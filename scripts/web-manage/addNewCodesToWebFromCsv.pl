#!/usr/bin/perl
use strict;
use warnings;
use feature qw(say);
use DBI;
use lib '..';
use connections;
use Text::CSV;
use List::Compare;
use String::Util 'trim';

say "Starting";

my $csv_filename = "codes.csv";
say "Reading sellers from CSV '$csv_filename'";

# it reads format <seller code>, <0/1>, <login> where 0/1 value has to be 0.
my %new_seller_codes_logins;
my @new_seller_codes;
# for existing sellers, where <0/1> is 1, keep login and new password
my %existing_seller_codes_logins;
my @existing_seller_codes;
{
	my $csv = Text::CSV->new({ sep_char => ',' });
	open (my $f, "<:utf8", $csv_filename) or die "Could not open text file '$csv_filename'";
	while (my $line = <$f>) {
		chomp $line;
	
		if ($csv->parse($line)) {
	
				my @fields = $csv->fields();
				if ($fields[1] == "0")
				{
					$new_seller_codes_logins{trim($fields[0])} = trim($fields[2]);
				} else {
					$existing_seller_codes_logins{trim($fields[0])} = trim($fields[2]);
				}
		}
	}
	close $f;
	
	if ((scalar keys %new_seller_codes_logins) + (scalar keys %existing_seller_codes_logins) == 0)
	{
		say "CSV did not contain any new or old logins.";
		die;
	}
	
	@new_seller_codes = sort keys %new_seller_codes_logins;
	@existing_seller_codes = sort keys %existing_seller_codes_logins;
	my $firstKey = $new_seller_codes[0];
	my $firstValue = $new_seller_codes_logins{ $firstKey };
	
	say "Read ". scalar @new_seller_codes ." codes and logins, first new is '$firstKey'='$firstValue'";
}

{
	my $dbweb = connections::web_connect();

	# reading 'seller codes' from web
	my @web_seller_codes;
	{
		my @sellers_arr = $dbweb->selectall_array("SELECT `info` FROM `Users`");
		@web_seller_codes = map {@$_} @sellers_arr;
	}
	
	# looking for duplicates
	{
		my $lc = List::Compare->new(\@new_seller_codes, \@web_seller_codes);
		my @intersection = $lc->get_intersection;
		if (scalar @intersection > 0)
		{
			say "There is some problem, some new codes are already on web:". join(" ", @intersection);
			die;
		}
	}
	
	# looking for update codes if all are in place
	{
		my $lc = List::Compare->new(\@existing_seller_codes, \@web_seller_codes);
		my @intersection = $lc->get_intersection;
		if (scalar @intersection != scalar @existing_seller_codes)
		{
			say "There is some problem, not all codes marked as existing are on the web, only these are online:". join(" ", @web_seller_codes);
			die;
		}
	}
	
	say "Going to insert into web. Confirm by pressing ENTER.";
	<STDIN>;
	
	# inserting
	if (scalar keys %new_seller_codes_logins > 0)
	{
		my $q = "INSERT INTO Users (PoslatTeraz, Provizia, isActive, Name, Mail, Kontakt, LoginStr, info) VALUES ";
		
		my @value_arr;
		for my $code (@new_seller_codes) {
			my $login = $new_seller_codes_logins{ $code };
			my $values = '(0, 0, 1, NULL, "", NULL, "'.$login.'", "'.$code.'")';
			push( @value_arr, $values );
		}
		
		$q .= join( ",", @value_arr );
		
		say "Insert sentence is ". length($q) ." characters long: ". $q;
		my $affected = $dbweb->do($q);
		say "Affected $affected rows.";
	}
	
	# updating existing
	if (scalar keys %existing_seller_codes_logins > 0)
	{
		my $q = "UPDATE Users SET info = CASE LoginStr ";
		
		my @value_arr;
		for my $code (@existing_seller_codes) {
			my $login = $existing_seller_codes_logins{ $code };
			my $values = "WHEN '".$login."' THEN '".$code."'";
			push( @value_arr, $values );
		}
		
		$q .= join( " ", @value_arr );
		$q .= " END";
		
		say "Update sentence is ". length($q) ." characters long.";
		my $affected = $dbweb->do($q);
		say "Affected $affected rows.";
	}
	
	say "Disconnecting";
	$dbweb->disconnect();
}

say "Exit.";


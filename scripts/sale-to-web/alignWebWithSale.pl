#!/usr/bin/perl
use strict;
use warnings;
use feature qw(say);
use DBI;
use lib '..';
use connections;
use String::Util 'trim';
use List::Compare;

say "Starting";

my $dbweb = connections::web_connect();
my $dbsale = connections::sale_connect();

# loop over all items, update item status

my (@sale_seller_codes, @web_seller_codes, @sale_trimmed_codes);
say "Retrieving sellers";

{
	my $sellers_arr_ref = $dbsale->selectall_arrayref(
		"SELECT \"SHORT_CODE\" FROM sellers");
	foreach (@$sellers_arr_ref)
	{
		my $code = @$_[0];
		my $codet = trim($code);
		push (@sale_seller_codes, $codet);
		if ( $code ne $codet )
		{
			push (@sale_trimmed_codes, $code);
		}
	}
	
	#	if (scalar @sale_trimmed_codes > 0)
	#	{
	#		say "Trimming sale codes: ". join( " ", @sale_trimmed_codes );
	#	foreach (@sale_trimmed_codes)
}
{
	my $sellers_arr_ref = $dbweb->selectall_arrayref(
		"SELECT info FROM Users");
	foreach (@$sellers_arr_ref)
	{
		my $code = @$_[0];
		push (@web_seller_codes, $code);
	}
}

# comparing sellers
my $lc = List::Compare->new(\@sale_seller_codes, \@web_seller_codes);
my @intersection = $lc->get_intersection;
my @missing_web = $lc->get_unique;
my @extra_web = $lc->get_complement;
{
	say "Found ". scalar @web_seller_codes ." sellers on web, ". scalar @sale_seller_codes ." sellers on sale, and ". scalar @intersection ." matching sellers by code";
	say "Codes missing on web: ". join( " ", @missing_web ) ." (". scalar @missing_web .")";
	say "Codes extra on web: ". join( " ", @extra_web ) ." (". scalar @extra_web .")";
	say "Going to remove/update web, continue? Press ENTER or break.";
	<STDIN>;
}

if (scalar @extra_web > 0) 
{
	say "Going to remove ". scalar @extra_web ." sellers and their items from web. Confirm by pressing ENTER.";
	<STDIN>;
	my $q = 'SELECT ID FROM Users WHERE info IN ("'. join( '","', @extra_web) .'")';
	my @seller_ids = map {@$_} $dbweb->selectall_array($q);
	if (scalar @seller_ids != scalar @extra_web)
	{ 
		# this is error state, debug and print out non-matches
		$q = 'SELECT info FROM Users WHERE ID IN ("'. join( '","', @seller_ids) .'")';
		my @found_codes = map {@$_} $dbweb->selectall_array($q);
		
		say "Did not find exact matching id's (". scalar @seller_ids .", " . scalar @found_codes .") for extra codes on web (". scalar @extra_web .")";
		my $cc = List::Compare->new('--unsorted', \@extra_web, \@found_codes);
		my @a = $cc->get_unique;
		my @b = $cc->get_complement;
		say "These were expected: '". join( " ", @a ) ."'(". scalar @a .") and these were found in query extra: '". join( " ", @b ) ."'(". scalar @b .")";
		
		die;
	}
	
	$q = "DELETE FROM Polozky WHERE UserID IN (". join( ",", @seller_ids) .")";
	my $deleted_items = $dbweb->do($q);
	$q = "DELETE FROM Users WHERE ID IN (". join( ",", @seller_ids) .")";
	my $deleted_sellers = $dbweb->do($q);
	say "Deleted $deleted_items items and $deleted_sellers sellers";
}

say "Updating seller names";
{
    # find in web DB which sellers do not have names
    my @ids;
	my $sellers_arr_ref = $dbweb->selectall_arrayref(
		"SELECT ID FROM Users WHERE Name IS NULL");

	if (scalar @$sellers_arr_ref > 0)
	{
        say scalar @$sellers_arr_ref . " sellers in web DB do not have a name";
        
        foreach (@$sellers_arr_ref)
        {
            push (@ids, @$_[0]);
        }
        
        my $idtext = join(",", @ids);
        
        # get those names if they are present in sale DB
        my $sellers_arr_ref = $dbsale->selectall_arrayref(
            'SELECT "ID", "NAME" FROM sellers WHERE "ID" IN ('.$idtext.') AND "NAME" IS NOT NULL');
        say scalar @$sellers_arr_ref . " sellers can be fixed by sale DB";
        say "Confirm by pressing ENTER.";
        <STDIN>;

        my $affected_rows = 0;
        foreach (@$sellers_arr_ref)
        {
            my $name = @$_[1];
            my $q = 'UPDATE Users SET Name = ? WHERE ID='. @$_[0];
            $affected_rows += $dbweb->do($q, undef, $name);
            if (defined $dbweb->err)
            {
                say "DB Err: " . $dbweb->err;
            }
        }

        say "Affected $affected_rows rows";
    }
    else
    {
        say "All seller names are filled in";
    }
}

say "Disconnecting";
$dbsale->disconnect();
$dbweb->disconnect();

say "Exit.";

=begin comment

sub updateForStatus
{
	my ($sale_value, $web_value) = @_;

	my $q = "SELECT i.\"ID\" FROM items i WHERE i.\"STATUS\" = $sale_value";
	my $qr = $dbsale->prepare($q);
	$qr->execute;
	if (defined $dbsale->err)
	{
	  say "DB Err: " . $dbsale->err;
	}
	my $updateItemLimit = 100;
	my $handledItems = 0;
	do
	{
		$handledItems = 0;
		$q = "UPDATE Polozky SET Predane = $web_value WHERE ID IN (";
		while ( ($handledItems < $updateItemLimit) and (my $row = $qr->fetchrow_arrayref) )
		{
			++$handledItems;
			my $id = @$row[0];
			$q .= "$id,";
		}
	
		$q = substr $q, 0, -1;
		$q .= ")";
		my $affected_rows = $dbweb->do($q);
		if (defined $dbweb->err)
		{
		  say "DB Err: " . $dbweb->err;
		}
	
		say "Affected $affected_rows rows";
	} while ($handledItems == $updateItemLimit);
}

=end comment

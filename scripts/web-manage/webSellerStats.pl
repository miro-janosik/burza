#!/usr/bin/perl
use strict;
use warnings;
use feature qw(say);
use DBI;
use lib '..';
use connections;

say "Starting";

my $dbh = connections::web_connect();

my $cmd1 = "SELECT u.ID, u.info, u.Name, COUNT(i.`UserID`) as itemsCount FROM `Users` u\
LEFT JOIN Polozky i ON u.`ID` = i.`UserID`\
GROUP BY u.`ID`\
ORDER BY u.`LoginStr`";
#my $qr1 = $dbh->prepare($cmd1);
#$qr1->execute;
#my $tbl_arr_ref = $qr1->fetchall_arrayref();
my $tbl_arr_ref = $dbh->selectall_arrayref($cmd1);
#my $tbl_arr_ref = $dbh->selectall_hashref($cmd1, {});

if (defined $dbh->err)
{
  say "DB Err: " . $dbh->err;
}

my @sellersOk;
my @sellersNoItems;
my @sellersNotAttending;

foreach my $s (@$tbl_arr_ref)
{
	my $itemCount = @$s[3];
	if ($itemCount == 0)
	{
		if ((!defined @$s[2]) || (@$s[2] eq ""))
		{
			push @sellersNotAttending, $s;
		}
		else
		{
			push @sellersNoItems, $s;
		}
	}
	else
	{
		push @sellersOk, $s;
	}
}

say "Sellers count: ". scalar @$tbl_arr_ref;
say "Sellers ok, count ". scalar @sellersOk;
foreach my $s (@sellersOk)
{
	say @$s[1] . "\t" . @$s[3];
}

say "Sellers without items, count ". scalar @sellersNoItems;
foreach my $s (@sellersNoItems)
{
	print @$s[1] . "[" . @$s[0] . "] ";
}
print "\n";

say "Sellers without items and name, count ". scalar @sellersNotAttending;
foreach my $s (@sellersNotAttending)
{
	print @$s[1] . "[" . @$s[0] . "] ";
}
print "\n";

say "Sellers to be removed (without items)";
print "(";
foreach my $s (@sellersNotAttending)
{
	print @$s[0] . ",";
}
foreach my $s (@sellersNoItems)
{
	print @$s[0] . ",";
}
print ")\n";

say "Disconnecting";
$dbh->disconnect();

say "Exit.";


#!/usr/bin/perl
use strict;
use warnings;
use feature qw(say);
use DBI;
use lib '..';
use connections;

say "Starting";

my $dbh = connections::connect();

my $cmd1 = "SELECT u.ID, u.info, COUNT(i.`UserID`) as itemsCount FROM `Users` u\
LEFT JOIN Polozky i ON u.`ID` = i.`UserID`\
GROUP BY i.`UserID`\
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

my $sellersWithoutItemsCount = 0;
foreach (@$tbl_arr_ref)
{
	my $itemCount = @$_[2];
	if ($itemCount == 0)
	{
		$sellersWithoutItemsCount++;
	}
}

say "Sellers count: ". scalar @$tbl_arr_ref . " without items: $sellersWithoutItemsCount";

say "Disconnecting";
$dbh->disconnect();

say "Exit.";


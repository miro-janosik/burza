#!/usr/bin/perl
use strict;
use warnings;
use feature qw(say);
use DBI;
use lib '..';
use connections;

say "Starting";

my $dbh = connections::web_connect();

my $cmd1 = "SELECT u.ID, COUNT(i.`UserID`) as itemsCount FROM `Users` u\
LEFT JOIN Polozky i ON u.`ID` = i.`UserID`\
GROUP BY u.`ID`\
ORDER BY u.`LoginStr`";

my $tbl_arr_ref = $dbh->selectall_arrayref($cmd1);

if (defined $dbh->err)
{
  say "DB Err: " . $dbh->err;
}

my @ids;

foreach my $s (@$tbl_arr_ref)
{
	my $itemCount = @$s[1];
	if ($itemCount == 0)
	{
		push @ids, @$s[0]
	}
}

say "Deleting ". scalar @ids . " sellers from ". scalar @$tbl_arr_ref;
my $idlist = join( ",", @ids );
say $idlist;
say "Press ENTER to confirm";
<STDIN>;

my $affected_rows = $dbh->do("DELETE FROM Users WHERE `ID` IN ($idlist)");
say "Affected $affected_rows rows";

say "Disconnecting";
$dbh->disconnect();

say "Exit.";


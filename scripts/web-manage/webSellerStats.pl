#!/usr/bin/perl
use strict;
use warnings;
use feature qw(say);
use DBI;
use lib '..';
use connections;

say "Starting";

my $dbh = connections::web_connect();

my $cmd2 = "SELECT COUNT(*) as Poloziek, SUM(p.Cena) as Suma FROM Polozky p";
my $tbl_suma = $dbh->selectall_arrayref($cmd2);
my $row1 = @$tbl_suma[0];
say "Poloziek: " . @$row1[0] . " Suma spolu: " . @$row1[1];

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

say "Sorting, and also requesting last added item date, will take time:";

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
		my $userId = @$s[0];
		my $cmdLast = "SELECT p.`Pridane` FROM Polozky p WHERE p.`UserID` = $userId ORDER BY p.`Pridane` DESC LIMIT 1";
		my $last_arr_ref = $dbh->selectall_arrayref($cmdLast);
		my $dateRow = @$last_arr_ref[0];
		my $date = @$dateRow[0];
		push @$s, $date;
		
		push @sellersOk, $s;
	}
}

say "Sellers count: ". scalar @$tbl_arr_ref;
say "Sellers ok, count ". scalar @sellersOk;
foreach my $s (@sellersOk)
{
	say @$s[1] . "\t" . @$s[3] . "\t" . @$s[4];
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


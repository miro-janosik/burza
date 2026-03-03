#!/usr/bin/perl
use strict;
use warnings;
use feature qw(say);
use DBI;
use lib '..';
use connections;

say "Starting";

# printing names causes warning, disabled as in https://stackoverflow.com/questions/47940662/how-to-get-rid-of-wide-character-in-print-at
binmode(STDOUT, ":encoding(UTF-8)");

my $dbh = connections::web_connect();

my $cmd2 = "SELECT COUNT(*) as Poloziek, SUM(p.Cena) as Suma FROM Polozky p";
my $tbl_suma = $dbh->selectall_arrayref($cmd2);
my $row1 = @$tbl_suma[0];
say "Poloziek: " . @$row1[0] . " Suma spolu: " . @$row1[1];

my $cmd1 = "SELECT u.ID, u.info, u.Name, COUNT(i.`UserID`) as itemsCount, u.VariantName, u.VariantActive FROM `Users` u \
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

foreach my $s (@sellersOk)
{
	my $s1 = @$s[1];
	my $s3 = @$s[3];
	my $sd = @$s[6];
	my $sn = @$s[2];
	my $sv = @$s[4];
	my $sva = @$s[5];
	my $sid = @$s[0];
	if (!defined $s1) { $s1 = ""; }
	if (!defined $s3) { $s3 = ""; }
	if (!defined $sd) { $sd = ""; }
	if (!defined $sn) { $sn = ""; }
	if ($sva) { $sv = "> " . $sv; } else { $sv = "  " . $sv; }

	printf("%s\t%s\t%s\t%-15s\t%s\t%s\n", $s1, $s3, $sd, $sv, $sid, $sn);
}

print "\n";
say "Sellers count: ". scalar @$tbl_arr_ref;
say "Sellers ok, count ". scalar @sellersOk;
say "Sellers with name without items, count ". scalar @sellersNoItems;
foreach my $s (@sellersNoItems)
{
	print @$s[1] . "[" . @$s[0] . "] ";
}
print "\n";

say "Sellers without items and name, count ". scalar @sellersNotAttending;
foreach my $s (@sellersNotAttending)
{
	my $s1 = @$s[1];
	my $s0 = @$s[0];

	if (!defined $s0) { $s0 = ""; }
	if (!defined $s1) { $s1 = ""; }

	print $s1 . "[" . $s0 . "] ";
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


#!/usr/bin/perl
use strict;
use warnings;
use feature qw(say);
use DBI;
use lib '..';
use connections;

say "Starting";

my $dbweb = connections::web_connect();
my $dbsale = connections::sale_connect();

# loop over all items, update item status

say "Updating items on stock";
updateForStatus(1, 0);
say "Updating items sold";
updateForStatus(2, 1);


say "Disconnecting";
$dbsale->disconnect();
$dbweb->disconnect();

say "Exit.";

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
		while ( (my $row = $qr->fetchrow_arrayref) and ($handledItems < $updateItemLimit) )
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

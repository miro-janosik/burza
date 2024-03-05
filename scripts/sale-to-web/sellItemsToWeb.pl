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
my $seller_id = 1219;

XXXX

my (@sale_seller_codes, @web_seller_codes, @sale_trimmed_codes);
say "Retrieving items";

{
	my $sale_items_arr_ref = $dbsale->selectall_arrayref(
		"SELECT * FROM items WHERE 'SELLER_ID' = $seller_id");
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

}

{
	my $sale_items_arr_ref = $dbsale->selectall_arrayref(
		"SELECT \"SHORT_CODE\" FROM sellers WHERE ");
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

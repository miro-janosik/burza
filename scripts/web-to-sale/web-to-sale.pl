#!/usr/bin/perl
use strict;
use warnings;
use feature qw(say);
use DBI;
use lib '..';
use connections;

say "Starting";

# sanitize string
sub sanitize
{
	my ($t) = @_;
	$t =~ s/"//g;
	$t =~ s/'//g;
	return $t;
}

my $dbsale = connections::sale_connect();

# see if local database is cleared
{
	my @counts = $dbsale->selectrow_array('SELECT COUNT(*) FROM "sellers"');
	my $sellers_count = $counts[0];
	   @counts = $dbsale->selectrow_array('SELECT COUNT(*) FROM "items"');
	my $items_count = $counts[0];

	if (($sellers_count + $items_count) > 0)
	{
		$dbsale->disconnect();
		say "Database contains items($items_count) or sellers ($sellers_count), delete them first";
		die;
	}
}

my $dbweb = connections::web_connect();

# copy sellers from web to local one
{
    my $qr = $dbweb->prepare(
"SELECT ID, \
info as shortcode, \
IFNULL( NULLIF(Name, ''), '-' ) as name, \
CONCAT( IFNULL(Mail, '-'), ' ', IFNULL(Kontakt, '-') ) as details \
FROM Users \
WHERE info is not null"
	);
	$qr->execute;

    my $updateItemLimit = 50;
    my $handledItems = 0;
	my $allHandledItems = 0;
    do
    {
		$handledItems = 0;

		my $q = 'INSERT INTO sellers ("ID", "SHORT_CODE", "NAME", "COMMENT") VALUES ';

		while ( (my $row = $qr->fetchrow_arrayref) and ($handledItems < $updateItemLimit) )
		{
			# say "A: @$row[2]";
			++$handledItems;
			$q .= "('".sanitize(@$row[0])."', '".sanitize(@$row[1])."', '".sanitize(@$row[2])."', '".sanitize(@$row[3])."'),";
		}

		$q = substr $q, 0, -1;
		$q .= ";";
		my $affected_rows = $dbsale->do($q);
		if (defined $dbweb->err)
		{
		  say "DB Err: " . $dbsale->err;
		}

		say "Affected $affected_rows rows";
		$allHandledItems += $affected_rows;
	} while ($handledItems == $updateItemLimit);

	say "Inserted $allHandledItems sellers";
}

# copy items from web to local one
{
        my $qr = $dbweb->prepare(
"SELECT
 p.ID,
 p.UserID,
 p.Cislo,
 p.Cena,
 CONCAT( REPLACE( IFNULL( NULLIF(p.Popis, ''), '-'), '\n', ' '), ' (', v.NAME, ')') as popis,
 p.Pridane
FROM Polozky p
JOIN Velkost v on p.Velkost = v.ID"
	);
	$qr->execute;

        my $updateItemLimit = 50;
        my $handledItems = 0;
	my $allHandledItems = 0;
        do
        {
		$handledItems = 0;

		my $q = 'INSERT INTO items ("ID","SELLER_ID","NUMBER","PRICE","NAME","MODIFIED") VALUES ';
                while ( (my $row = $qr->fetchrow_arrayref) and ($handledItems < $updateItemLimit) )
                {
                        ++$handledItems;
                        $q .= "('".sanitize(@$row[0])."', '".sanitize(@$row[1])."', '".sanitize(@$row[2])."', '".sanitize(@$row[3])."', '".sanitize(@$row[4])."', '".sanitize(@$row[5])."'),";
                }
        
                $q = substr $q, 0, -1;
                $q .= ";";
                my $affected_rows = $dbsale->do($q);
                if (defined $dbweb->err)
                {
                  say "DB Err: " . $dbsale->err;
                }
        
                say "Affected $affected_rows rows";
		$allHandledItems += $affected_rows;
        } while ($handledItems == $updateItemLimit);
	say "Inserted $allHandledItems items";
}

say "Disconnecting";
$dbsale->disconnect();
$dbweb->disconnect();


#!/usr/bin/perl
use strict;
use warnings;
use feature qw(say);
use DBI;
use lib '..';
use connections;

say "Starting";

my $dbsale = connections::sale_connect();

# see if local database is cleared
{
	my @counts = map {@$_} $dbsale->selectall_array('SELECT COUNT(*) FROM "sellers"');
	my $sellers_count = $counts[0];
	   @counts = map {@$_} $dbsale->selectall_array('SELECT COUNT(*) FROM "items"');
	my $items_count = $counts[0];

	if (($sellers_count + $items_count) > 0)
	{
		$dbsale->disconnect();
		say "Database contains items or sellers, delete them first";
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
			say "A: @$row[2]";
                        ++$handledItems;
                        $q .= "('@$row[0]', '@$row[1]', '@$row[2]', '@$row[3]'),";
                }
        
                $q = substr $q, 0, -1;
                $q .= ";"; say $q;
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
                        $q .= "('@$row[0]', '@$row[1]', '@$row[2]', '@$row[3]', '@$row[4]', '@$row[5]'),";
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


#!/usr/bin/perl
use strict;
use warnings;
use feature qw(say);
use DBI;
use lib '..';
use connections;

say "Starting";

my $dbsale = connections::sale_connect();

say "Deleting sale. Confirm by pressing ENTER";
<STDIN>;

say "Deleting sold_item, affected rows=". $dbsale->do("DELETE FROM sold_item");
say "Deleting sold, affected rows=". $dbsale->do("DELETE FROM sold");
say "Deleting items, affected rows=". $dbsale->do("DELETE FROM items");
say "Deleting sellers, affected rows=". $dbsale->do("DELETE FROM sellers");

say "Finished";

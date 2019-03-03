#!/usr/bin/perl
use strict;
use warnings;
use feature qw(say);
use DBI;
use lib '..';
use connections;

say "Starting";

my $dbweb = connections::web_connect();

{
	say "Removing sold items from web. Press ENTER to confirm";
	<STDIN>;
	
	my $affected_rows = $dbweb->do("DELETE FROM Polozky WHERE Predane = 1");
	say "Affected $affected_rows rows";
	
}

say "Disconnecting";
$dbweb->disconnect();

say "Exit.";

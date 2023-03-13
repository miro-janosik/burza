#!/usr/bin/perl
use strict;
use warnings;
use feature qw(say);
use DBI;
use lib '..';
use connections;

say "Starting";

my $dbweb = connections::web_connect();

my $cmd1 = "SELECT `loginStr`, `Name`, `Mail` FROM `Users` ORDER BY `loginStr`";
my $qr1 = $dbweb->prepare($cmd1);
$qr1->execute;
my $tbl_arr_ref = $qr1->fetchall_arrayref();
if (defined $qr1->err)
{
  say "DB Err: " . $qr1->err;
}

say "Disconnecting";
$dbweb->disconnect();

open (my $f, '>', "web_codes.txt") or die "Could not create text file";
binmode $f, ":utf8";

foreach (@$tbl_arr_ref)
{
	print $f @$_[0] . "\t" . @$_[1] . "\t" . @$_[2] . "\n";
}

say "Exported " . scalar @$tbl_arr_ref . " sellers";

close $f;
say "Exit.";


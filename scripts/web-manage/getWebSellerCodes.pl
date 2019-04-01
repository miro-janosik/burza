#!/usr/bin/perl
use strict;
use warnings;
use feature qw(say);
use DBI;
use lib '..';
use connections;

say "Starting";

my $dbweb = connections::web_connect();

my $cmd1 = "SELECT `info`, `loginStr` FROM `Users`";
my $qr1 = $dbweb->prepare($cmd1);
$qr1->execute;
#my $tbl_arr_ref = $qr1->fetchall_arrayref([0]);
my $tbl_arr_ref = $qr1->fetchall_arrayref();
if (defined $qr1->err)
{
  say "DB Err: " . $qr1->err;
}

say "Disconnecting";
$dbweb->disconnect();

open (my $f, '>', "web_codes.txt") or die "Could not create text file";

foreach (@$tbl_arr_ref)
{
	my $code = @$_[0];
	my $login = @$_[1];
	say " $code";
	print $f "$code\t$login\n";
}

close $f;
say "Exit.";


#!/usr/bin/perl
use strict;
use warnings;
use feature qw(say);
use POSIX qw(strftime);
#use DBI;
use Cwd 'abs_path';
# use lib '..';
#use connections;

say "Starting";

my $pth = "../../../bk/";
mkdir $pth;
my $bkfolder = abs_path($pth);
say "Backing up into folder $bkfolder";

while (1) 
{
#	say "Connecting";
#	my $dbsale = connections::sale_connect();
#
#	{
#		my @counts = $dbsale->selectrow_array('SELECT COUNT(*) FROM "sellers"');
#		my $sellers_count = $counts[0];
#		   @counts = $dbsale->selectrow_array('SELECT COUNT(*) FROM "items"');
#		my $items_count = $counts[0];
#
#		say "Database contains items($items_count) or sellers ($sellers_count).";
#	}

	my $dt = strftime "%F_%H%M", localtime;
	my $fname = "$bkfolder/burza-$dt.bak";

#	my $q = "BACKUP DATABASE burza TO DISK='$fname'";
#	$dbsale->do($q);
	my $cmd = "pg_dump burza -h 192.168.1.222 -p 5432 -U user -a > $fname";
	system($cmd);

	my $size = -s $fname;
	say "Back up generated, file $fname filesize $size";

#	say "Disconnecting";
#	$dbsale->disconnect();

	my $sleep_min = 10;
	say "Sleeping for $sleep_min minutes...";
	sleep($sleep_min * 60);
}


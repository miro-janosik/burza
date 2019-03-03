package connections;

use feature qw(say);
use DBI;

# server is mariadb / mysql
my $web_server = "mariadb55.websupport.sk";
my $web_login = "bwr884kg";
my $web_db = "bwr884kg";
my $web_password = "kE8bsC41Sg";
my $web_port = "3310";
my %web_attr = ( PrintError=>1,  # turn off error reporting via warn()
             RaiseError=>1 );   # turn on error reporting via die()
my $web_con = "DBI:mysql:database=$web_db;host=$web_server;port=$web_port;mysql_connect_timeout=5";

sub web_connect
{
	say "Connecting to web...";
	my $web_dbh = DBI->connect($web_con, $web_login, $web_password, \%web_attr)
	        or die("Error connecting to the database: $DBI::errstr\n");
	say "Connected to the MySQL database.";
	return $web_dbh;
}


# postgresql database
#sale_cmd="psql -d burza -h 192.168.1.222 -p 5432 -U user -w"
#sale_cmd="psql -d b1 -h 127.0.0.1 -p 5432 -U user"
# -f in_file.sql -o outfile.csv
# -w does not ask for password, it is stored in .pgpass

my $sale_server = "192.168.1.17";
my $sale_db = "b2";
my $sale_login = "user";
my $sale_password = "trencin";
my $sale_port = "5432";
my %sale_attr = ( PrintError=>1,  # turn off error reporting via warn()
             RaiseError=>1 );   # turn on error reporting via die()
my $sale_con = "DBI:Pg:database=$sale_db;host=$sale_server;port=$sale_port;connect_timeout=5";

sub sale_connect
{
	say "Connecting to sale...";
	my $sale_dbh = DBI->connect($sale_con, $sale_login, $sale_password, \%sale_attr)
	        or die("Error connecting to the database: $DBI::errstr\n");
	say "Connected to the PG database.";
	return $sale_dbh;
}


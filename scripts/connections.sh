# server is mariadb / mysql
web_cmd="mysql -h mariadb55.websupport.sk -u bwr884kg -P 3310 -pkE8bsC41Sg bwr884kg"

# postgresql database
#sale_cmd="psql -d burza -h 192.168.1.222 -p 5432 -U user -w"
sale_cmd="psql -d b2 -h 192.168.1.17 -p 5432 -U user"
# -f in_file.sql -o outfile.csv
# -w does not ask for password, it is stored in .pgpass


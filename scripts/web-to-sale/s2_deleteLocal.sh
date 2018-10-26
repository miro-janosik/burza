. ../connections.sh

echo "Deleting"
$sale_cmd -f ts_sale_deleteItemsSellers.sql
# sed 's/\t/,/g' t11_sellers.csv >t12_sellers.csv

echo "Done"

. ../connections.sh

echo "Importing"
$sale_cmd -f ts_sale_importCsvSellersItems.sql
# sed 's/\t/,/g' t11_sellers.csv >t12_sellers.csv

echo "Done"

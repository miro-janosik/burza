. ../connections.sh

echo "Getting items"
$sale_cmd < ts_selectItemsToCsv.sql
# sed 's/\t/,/g' t11_sellers.dat >t12_sellers.csv

echo "Done."

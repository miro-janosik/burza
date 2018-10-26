. ../connections.sh

echo "Getting sellers from web"
$web_cmd < ts_web_selectSellers.sql > t11_sellers.dat
# sed 's/\t/,/g' t11_sellers.dat >t12_sellers.csv

echo "Getting items from web"
$web_cmd < ts_web_selectItems.sql > t13_items.dat
echo "Fixing items"
sed 's// /g' t13_items.dat | sed 's/"/ /g' | sed 's/\\//g' >t14_items.dat

echo "Done."

. ../connections.sh

echo "Updating categories"
$sale_cmd < ts_sale_updateItemCats.sql 

echo "Done."

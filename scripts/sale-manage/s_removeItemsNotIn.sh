. ../connections.sh

echo "Removing items"
$sale_cmd < ts_sale_removeItemsNotIn.sql 

echo "Done."

. ../connections.sh

echo "Updating sellers"
$sale_cmd < ts_sale_fixSellerStatus.sql 

echo "Done."

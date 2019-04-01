. ../connections.sh

echo "Sellers not accepted with accepted items"
$sale_cmd < ts_sale_listSellersProblems.sql 

echo "Sellers accepted with not accepted items"
$sale_cmd < ts_sale_listSellersProblems2.sql 

echo "Done."

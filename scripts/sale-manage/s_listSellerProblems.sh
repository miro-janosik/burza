. ../connections.sh

echo "Sellers not accepted with accepted items"
$sale_cmd < ts_sale_listSellersProblems.sql 

echo "Sellers accepted with not accepted items"
$sale_cmd < ts_sale_listSellersProblems2.sql 

echo "Sellers not accepted with not accepted items"
$sale_cmd < ts_sale_listSellersProblems3.sql 

echo "Done."

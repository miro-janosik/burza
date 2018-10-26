. ../connections.sh

#flist="tmp1.dat"
#echo "Getting list of sellers without items"
#$sale_cmd < ts_sale_getSellersWithoutItems.sql >$flist

echo "Removing sellers"
$sale_cmd < ts_sale_removeSellersWithoutItems.sql

echo "Done."

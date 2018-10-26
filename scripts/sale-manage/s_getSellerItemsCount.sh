. ../connections.sh

ofile=sellerItemsCount.txt 

rm $ofile
echo "Getting seller item count into $ofile"
$sale_cmd < ts_sale_getSellerItemsCounts.sql >$ofile

echo "Adding info about sellers without items"
echo "Without any items:" >>$ofile;

$sale_cmd < ts_sale_getSellersWithoutItems.sql >>$ofile

echo "Done."

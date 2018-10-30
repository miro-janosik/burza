. ../connections.sh

echo "Updating items on web"
$web_cmd < ts_web_updateItemsByCsv.sql 

echo "Done."

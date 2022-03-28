
for i in {1..500};
do
	perl ./itemStatusSaleToWeb.pl
	echo sleeping in loop $i
	sleep 5m
done

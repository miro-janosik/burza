DROP TEMPORARY TABLE IF EXISTS tmp_item_status;

CREATE TEMPORARY TABLE tmp_item_status (id INTEGER, status INTEGER);

LOAD DATA LOCAL INFILE './t11_items.csv'
INTO TABLE tmp_item_status 
FIELDS TERMINATED BY ',' 
LINES TERMINATED BY '\n';

-- status=1 prijate; status2 = predane
UPDATE Polozky p 
INNER JOIN tmp_item_status ti on p.ID = ti.id
SET p.Predane=0
WHERE ti.status = 1;

UPDATE Polozky p 
INNER JOIN tmp_item_status ti on p.ID = ti.id
SET p.Predane=1
WHERE ti.status = 2;


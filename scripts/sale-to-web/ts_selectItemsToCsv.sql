\copy (select "ID", "STATUS" from items) TO './t11_items.csv' DELIMITER ',' CSV HEADER;

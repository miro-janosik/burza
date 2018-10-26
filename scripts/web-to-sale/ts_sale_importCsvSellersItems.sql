\copy sellers("ID", "SHORT_CODE", "NAME", "COMMENT") FROM './t11_sellers.dat' DELIMITER E'\t' CSV HEADER;

\copy items("ID", "SELLER_ID", "NUMBER", "PRICE", "NAME", "MODIFIED" ) FROM './t14_items.dat' DELIMITER E'\t' CSV HEADER;

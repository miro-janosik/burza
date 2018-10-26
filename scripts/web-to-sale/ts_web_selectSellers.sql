SELECT 
 ID, 
 info as shortcode,
 IFNULL( NULLIF(Name, ''), "-" ) as name, 
 CONCAT( IFNULL(Mail, "-"), " ", IFNULL(Kontakt, "-") ) as details
FROM Users
WHERE info is not null;


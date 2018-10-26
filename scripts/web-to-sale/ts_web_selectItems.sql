SELECT 
 p.ID, 
 p.UserID,
 p.Cislo, 
 p.Cena, 
 CONCAT( REPLACE( IFNULL( NULLIF(p.Popis, ''), "-"), '\n', ' '), " (", v.NAME, ")") as popis, 
 p.Pridane
FROM Polozky p
JOIN Velkost v on p.Velkost = v.ID


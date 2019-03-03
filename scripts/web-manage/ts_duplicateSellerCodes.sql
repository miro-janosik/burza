SELECT info,
 COUNT(info) AS NumOccurrences
FROM Users
GROUP BY info
HAVING ( NumOccurrences > 1 )


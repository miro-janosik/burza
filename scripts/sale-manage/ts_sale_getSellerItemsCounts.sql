SELECT s."SHORT_CODE", COUNT(i."ID") AS itemsCount
FROM "sellers" s 
LEFT JOIN "items" i on s."ID" = i."SELLER_ID"
GROUP BY s."ID"
ORDER BY s."SHORT_CODE"

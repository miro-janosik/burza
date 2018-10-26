SELECT si."SHORT_CODE"
FROM
(
SELECT s."SHORT_CODE", COUNT(i."ID") AS itemsCount
FROM "sellers" s 
LEFT JOIN "items" i on s."ID" = i."SELLER_ID"
GROUP BY s."ID"
) AS si
WHERE si.itemsCount = 0
ORDER BY si."SHORT_CODE"


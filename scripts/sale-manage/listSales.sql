SELECT s."DATE", COUNT(si."ID") AS itemsCount, sum(i."PRICE")
FROM "sold" s 
LEFT JOIN "sold_item" si on s."ID" = si."SOLD_ID"
LEFT JOIN "items" i on si."ITEM_ID" = i."ID"
GROUP BY s."ID"
ORDER BY s."DATE"
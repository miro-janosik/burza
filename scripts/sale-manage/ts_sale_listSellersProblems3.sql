select s."SHORT_CODE"
from items i
join sellers s on i."SELLER_ID" = s."ID"
where i."STATUS" = 0
and s."STATUS" = 0
GROUP BY s."SHORT_CODE"

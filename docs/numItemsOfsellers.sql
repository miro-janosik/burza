select sellers."SHORT_CODE", COUNT(items)
from items
left join sellers on items."SELLER_ID" = sellers."ID"
group by sellers."ID"
order by sellers."SHORT_CODE"

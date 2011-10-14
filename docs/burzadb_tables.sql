-- Table: item_category

-- DROP TABLE item_category;

CREATE TABLE item_category
(
  "ID" integer NOT NULL DEFAULT (-1),
  "LETTER" character(1),
  "NAME" text,
  "FEE_PER_ITEM" double precision,
  CONSTRAINT "item category id is unique" PRIMARY KEY ("ID")
)
WITH (
  OIDS=FALSE
);
ALTER TABLE item_category OWNER TO postgres;
COMMENT ON TABLE item_category IS 'Item categories.';


-- Table: items

-- DROP TABLE items;

CREATE TABLE items
(
  "ITEM_ID" integer NOT NULL DEFAULT (-1),
  "NAME" text,
  "SELLER_ID" integer NOT NULL DEFAULT (-1),
  "PRICE" double precision NOT NULL DEFAULT 0.0,
  "CHANGING" boolean NOT NULL DEFAULT false,
  "CHANGED_BY" text,
  "ITEM_CATEGORY_ID" integer DEFAULT (-1),
  "NUMBER" integer,
  "STATUS" integer NOT NULL DEFAULT 0,
  "MODIFIED" timestamp with time zone NOT NULL DEFAULT '2000-01-01 01:02:03'::timestamp without time zone,
  CONSTRAINT "item id is unique" PRIMARY KEY ("ITEM_ID")
)
WITH (
  OIDS=FALSE
);
ALTER TABLE items OWNER TO postgres;
COMMENT ON TABLE items IS 'List of all items of all sellers.';



-- Table: seller_category

-- DROP TABLE seller_category;

CREATE TABLE seller_category
(
  "ID" integer NOT NULL DEFAULT (-1),
  "LETTER" character(1),
  "NAME" text,
  "FEE_PER_SALE" double precision,
  CONSTRAINT "seller category id is unique" PRIMARY KEY ("ID")
)
WITH (
  OIDS=FALSE
);
ALTER TABLE seller_category OWNER TO postgres;
COMMENT ON TABLE seller_category IS 'Seller categories.';




-- Table: sellers

-- DROP TABLE sellers;

CREATE TABLE sellers
(
  "SELLER_ID" integer NOT NULL,
  "SHORT_CODE" text,
  "NAME" text,
  "COMMENT" text,
  "CHANGING" boolean NOT NULL DEFAULT false,
  "CHANGED_BY" text,
  "SELLER_CATEGORY_ID" integer DEFAULT (-1),
  "MODIFIED" timestamp with time zone NOT NULL DEFAULT '2000-01-01 01:02:03'::timestamp without time zone,
  CONSTRAINT "seller id is unique" PRIMARY KEY ("SELLER_ID")
)
WITH (
  OIDS=FALSE
);
ALTER TABLE sellers OWNER TO postgres;
COMMENT ON TABLE sellers IS 'List of sellers.';

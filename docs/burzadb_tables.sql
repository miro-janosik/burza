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

-- Create default content of the item_category table
INSERT INTO item_category(
            "ID", "LETTER", "NAME", "FEE_PER_ITEM")
    VALUES (0, 'a', 'odev', 0.0);


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
  "FEE" double precision DEFAULT 0, -- Fee is {value}.
  "FEE_SOLD_PERC" double precision DEFAULT 0, -- Fee is {value}% of the price of all sold items.
  "FEE_SOLD_ITEM" double precision DEFAULT 0, -- Fee is {value} for every sold item.
  "FEE_ITEM" double precision DEFAULT 0, -- Fee is {value} for every item.
  "FEE_ITEM_PERC" double precision DEFAULT 0, -- Fee is {value}% of price for every item.
  CONSTRAINT "seller category id is unique" PRIMARY KEY ("ID")
)
WITH (OIDS=FALSE);
ALTER TABLE seller_category OWNER TO postgres;
COMMENT ON TABLE seller_category IS 'Seller categories.';
COMMENT ON COLUMN seller_category."FEE" IS 'Fee is {value}.';
COMMENT ON COLUMN seller_category."FEE_SOLD_PERC" IS 'Fee is {value}% of the price of all sold items.';
COMMENT ON COLUMN seller_category."FEE_SOLD_ITEM" IS 'Fee is {value} for every sold item.';
COMMENT ON COLUMN seller_category."FEE_ITEM" IS 'Fee is {value} for every item.';
COMMENT ON COLUMN seller_category."FEE_ITEM_PERC" IS 'Fee is {value}% of price for every item.';

-- Create default content of 'seller category' table
INSERT INTO seller_category(
	"ID", "LETTER", "NAME", 
	"FEE", "FEE_SOLD_PERC", "FEE_SOLD_ITEM", "FEE_ITEM", "FEE_ITEM_PERC")
    VALUES 
    (1, 'p', 'Predajca', 0.0, 10.0, 0.00, 0.10, 0.0),
    (2, 's', 'Sympatizant', 0.0, 10.0, 0.00, 0.0, 0.0),
    (3, 'd', 'Organizator', 0.0, 0.0, 0.00, 0.0, 0.0);



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
  "STATUS" integer NOT NULL DEFAULT 0,
  CONSTRAINT "seller id is unique" PRIMARY KEY ("SELLER_ID")
)
WITH (
  OIDS=FALSE
);
ALTER TABLE sellers OWNER TO postgres;
GRANT ALL ON TABLE sellers TO postgres;
GRANT SELECT, UPDATE, INSERT, DELETE, REFERENCES ON TABLE sellers TO public;
COMMENT ON TABLE sellers IS 'List of sellers.';


-- !! TO BE TESTED !!
-- Create default usergroup + user, password 'trencin'
CREATE ROLE usergroup 
-- line below was not here, copied from user below to set password for group - is necessary at all?
LOGIN ENCRYPTED PASSWORD 'md59f1e62cb21e6b04c80c95afdbbb471de'
   VALID UNTIL 'infinity';
COMMENT ON ROLE usergroup IS 'Default user group';

-- default user with password 'trencin'
CREATE ROLE user LOGIN ENCRYPTED PASSWORD 'md59f1e62cb21e6b04c80c95afdbbb471de'
   VALID UNTIL 'infinity';
GRANT usergroup TO user;
COMMENT ON ROLE user IS 'default user, with password trencin';

GRANT ALL ON TABLE item_category TO GROUP usergroup;
GRANT ALL ON TABLE items TO GROUP usergroup;
GRANT ALL ON TABLE seller_category TO GROUP usergroup;
GRANT ALL ON TABLE sellers TO GROUP usergroup;


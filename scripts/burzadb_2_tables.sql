

-- Table: item_category
-- DROP TABLE item_category;
CREATE SEQUENCE item_category_id_seq;
GRANT SELECT, UPDATE, INSERT, DELETE, REFERENCES ON TABLE item_category_id_seq TO public;
CREATE TABLE item_category
(
  "ID" integer NOT NULL PRIMARY KEY DEFAULT nextval('item_category_id_seq'),
  "LETTER" character(1) not null,
  "NAME" text not null,
  "LIMIT_PRICE_MIN" double precision,
  "LIMIT_PRICE_MAX" double precision,
  
  "FEE_ITEM" double precision DEFAULT 0, -- Fee is {value} for every item. Even if not sold.
  "FEE_ITEM_PERC" double precision DEFAULT 0, -- Fee is {value}% of price for every item. Even if not sold.
  "FEE_SOLD_ITEM" double precision DEFAULT 0, -- Fee is {value} for every sold item.
  "FEE_SOLD_PERC" double precision DEFAULT 0 -- Fee is {value}% of the price of every sold item.
  
  -- CONSTRAINT "item category id is unique" PRIMARY KEY ("ID")
)
WITH (
  OIDS=FALSE
);
ALTER TABLE item_category OWNER TO postgres;
GRANT ALL ON TABLE item_category TO postgres;
GRANT SELECT, UPDATE, INSERT, DELETE, REFERENCES ON TABLE item_category TO public;
COMMENT ON TABLE item_category IS 'Item categories.';
COMMENT ON COLUMN item_category."FEE_ITEM" IS 'Fee is {value} for every item. Even if not sold.';
COMMENT ON COLUMN item_category."FEE_ITEM_PERC" IS 'Fee is {value}% of price for every item. Even if not sold.';
COMMENT ON COLUMN item_category."FEE_SOLD_ITEM" IS 'Fee is {value} for every sold item.';
COMMENT ON COLUMN item_category."FEE_SOLD_PERC" IS 'Fee is {value}% of the price of every sold item.';
-- Create default content of the item_category table
INSERT INTO item_category(
            "ID", "LETTER", "NAME", "LIMIT_PRICE_MIN", "LIMIT_PRICE_MAX",
            "FEE_ITEM", "FEE_ITEM_PERC", "FEE_SOLD_ITEM", "FEE_SOLD_PERC")
    VALUES 
    (3, 'z', 'Do 0.1',NULL, 0.1, 0.0, 0.0, 0.0, 0 ),
    (1, 'a', 'Do 15', 0.1, 15.0, 0.10, 0.0, 0.0, 10 ),
    (2, 'b', 'Nad 15',15.0, NULL, 1.00, 0.0, 0.0, 10 );

-- Table: seller_category
-- DROP TABLE seller_category;
CREATE SEQUENCE seller_category_id_seq;
GRANT SELECT, UPDATE, INSERT, DELETE, REFERENCES ON TABLE seller_category_id_seq TO public;
CREATE TABLE seller_category
(
  "ID" integer NOT NULL PRIMARY KEY DEFAULT nextval('seller_category_id_seq'),
  "LETTER" character(1),
  "NAME" text,
  "FEE" double precision DEFAULT 0, -- Fee is {value} for putting items on sale.

  "MUL_FEE_ITEM" double precision DEFAULT 1, -- Multiplier of item category FEE_ITEM
  "MUL_FEE_ITEM_PERC" double precision DEFAULT 1, -- Multiplier of item category FEE_ITEM_PERC
  "MUL_FEE_SOLD_ITEM" double precision DEFAULT 1, -- Multiplier of item category FEE_SOLD_ITEM
  "MUL_FEE_SOLD_PERC" double precision DEFAULT 1 -- Multiplier of item category FEE_SOLD_PERC
  -- CONSTRAINT "seller category id is unique" PRIMARY KEY ("ID")
)
WITH (OIDS=FALSE);
ALTER TABLE seller_category OWNER TO postgres;
GRANT ALL ON TABLE seller_category TO postgres;
GRANT SELECT, UPDATE, INSERT, DELETE, REFERENCES ON TABLE seller_category TO public;
COMMENT ON TABLE seller_category IS 'Seller categories.';
COMMENT ON COLUMN seller_category."FEE" IS 'Fee is {value} for putting items on sale.';
COMMENT ON COLUMN seller_category."MUL_FEE_ITEM" IS 'Multiplier of item category FEE_ITEM';
COMMENT ON COLUMN seller_category."MUL_FEE_ITEM_PERC" IS 'Multiplier of item category FEE_ITEM_PERC';
COMMENT ON COLUMN seller_category."MUL_FEE_SOLD_ITEM" IS 'Multiplier of item category FEE_SOLD_ITEM';
COMMENT ON COLUMN seller_category."MUL_FEE_SOLD_PERC" IS 'Multiplier of item category FEE_SOLD_PERC';

-- Create default content of 'seller category' table
INSERT INTO seller_category(
	"ID", "LETTER", "NAME", 
	"FEE", "MUL_FEE_ITEM", "MUL_FEE_ITEM_PERC", "MUL_FEE_SOLD_ITEM", "MUL_FEE_SOLD_PERC")
    VALUES 
    (1, 'p', 'Predajca', 0.0, 1.0, 1.0, 1.0, 1.0),
    (2, 's', 'Sympatizant', 0.0, 0.0, 0.0, 1.0, 1.0),
    (3, 'd', 'Organizator', 0.0, 0.0, 0.0, 0.0, 0.0);



-- Table: sellers
-- DROP TABLE sellers;
CREATE SEQUENCE sellers_id_seq;
GRANT SELECT, UPDATE, INSERT, DELETE, REFERENCES ON TABLE sellers_id_seq TO public;
CREATE TABLE sellers
(
  "ID" integer PRIMARY KEY NOT NULL DEFAULT nextval('sellers_id_seq'),
  "SHORT_CODE" text NOT NULL,
  "NAME" text NOT NULL,
  "COMMENT" text,
  "CHANGING" boolean NOT NULL DEFAULT false,
  "CHANGED_BY" text,
  "SELLER_CATEGORY_ID" integer DEFAULT (-1) CONSTRAINT "sellers_seller_category_id" references seller_category("ID"),
  "MODIFIED" timestamp with time zone NOT NULL DEFAULT '2000-01-01 01:02:03'::timestamp without time zone,
  "STATUS" integer NOT NULL DEFAULT 0
  -- CONSTRAINT "seller id is unique" PRIMARY KEY ("SELLER_ID")
)
WITH (
  OIDS=FALSE
);
ALTER TABLE sellers OWNER TO postgres;
GRANT ALL ON TABLE sellers TO postgres;
GRANT SELECT, UPDATE, INSERT, DELETE, REFERENCES ON TABLE sellers TO public;
COMMENT ON TABLE sellers IS 'List of sellers.';

-- Table: items
-- DROP TABLE items;
CREATE SEQUENCE items_id_seq;
GRANT SELECT, UPDATE, INSERT, DELETE, REFERENCES ON TABLE items_id_seq TO public;
CREATE TABLE items
(
  "ID" integer NOT NULL DEFAULT nextval('items_id_seq') PRIMARY KEY,
  "NAME" text,
  "SELLER_ID" integer NOT NULL DEFAULT (-1) references sellers("ID"),
  "PRICE" double precision NOT NULL DEFAULT 0.0,
  "CHANGING" boolean NOT NULL DEFAULT false,
  "CHANGED_BY" text,
  "ITEM_CATEGORY_ID" integer NOT NULL DEFAULT (-1) references item_category("ID"),
  "NUMBER" integer NOT NULL DEFAULT (-1),
  "STATUS" integer NOT NULL DEFAULT 0,
  "MODIFIED" timestamp with time zone NOT NULL DEFAULT '2000-01-01 01:02:03'::timestamp without time zone
  -- CONSTRAINT "item id is unique" PRIMARY KEY ("ITEM_ID")
)
WITH (
  OIDS=FALSE
);
ALTER TABLE items OWNER TO postgres;
GRANT ALL ON TABLE items TO postgres;
GRANT SELECT, UPDATE, INSERT, DELETE, REFERENCES ON TABLE items TO public;
COMMENT ON TABLE items IS 'List of all items of all sellers.';


-- DROP TABLE sold;
CREATE SEQUENCE sold_id_seq;
GRANT SELECT, UPDATE, INSERT, DELETE, REFERENCES ON TABLE sold_id_seq TO public;
CREATE TABLE sold
(
  "ID" integer NOT NULL DEFAULT nextval('sold_id_seq') PRIMARY KEY,
  "TITLE" text,
  "DATE" timestamp with time zone NOT NULL DEFAULT '2000-01-01 01:02:03'::timestamp without time zone
) WITH ( OIDS=FALSE );
ALTER TABLE sold OWNER TO postgres;
GRANT ALL ON TABLE sold TO postgres;
GRANT SELECT, UPDATE, INSERT, DELETE, REFERENCES ON TABLE sold TO public;
COMMENT ON TABLE sold IS 'Info about items sold to one person';


-- DROP TABLE sold_item;
CREATE SEQUENCE sold_item_id_seq;
GRANT SELECT, UPDATE, INSERT, DELETE, REFERENCES ON TABLE sold_item_id_seq TO public;
CREATE TABLE sold_item
(
  "ID" integer NOT NULL DEFAULT nextval('sold_item_id_seq') PRIMARY KEY,
  "SOLD_ID" integer NOT NULL references sold("ID"),
  "ITEM_ID" integer NOT NULL references items("ID")
) WITH ( OIDS=FALSE );
ALTER TABLE sold_item OWNER TO postgres;
GRANT ALL ON TABLE sold_item TO postgres;
GRANT SELECT, UPDATE, INSERT, DELETE, REFERENCES ON TABLE sold_item TO public;
COMMENT ON TABLE sold_item IS 'Info about items sold to one person';



-- Table: info
-- DROP TABLE info;

CREATE TABLE info
(
  "VERSION" integer NOT NULL,
  "TITLE" text,
  "DATE" timestamp with time zone NOT NULL DEFAULT '2000-01-01 01:02:03'::timestamp without time zone
)
WITH (
  OIDS=FALSE
);
ALTER TABLE info OWNER TO postgres;
GRANT ALL ON TABLE info TO postgres;
GRANT SELECT, UPDATE, INSERT, DELETE, REFERENCES ON TABLE info TO public;
COMMENT ON TABLE info IS 'Instance information.';

-- Create default content of 'info' table
INSERT INTO info(
	"VERSION", "TITLE")
    VALUES 
    (3, 'Burza');



-- !! TO BE TESTED !!
-- Create default usergroup + user, password 'trencin'
CREATE ROLE usergroup 
-- line below was not here, copied from user below to set password for group - is necessary at all?
LOGIN ENCRYPTED PASSWORD 'md59f1e62cb21e6b04c80c95afdbbb471de'
   VALID UNTIL 'infinity';
COMMENT ON ROLE usergroup IS 'Default user group';

-- default user with password 'trencin'
CREATE ROLE "user" LOGIN ENCRYPTED PASSWORD 'md53bc8db570e3bf0acd065d983b4fd7333'
   VALID UNTIL 'infinity';
GRANT usergroup TO "user";
COMMENT ON ROLE "user" IS 'default user, with password trencin';

GRANT ALL ON TABLE item_category TO GROUP usergroup;
GRANT ALL ON TABLE items TO GROUP usergroup;
GRANT ALL ON TABLE seller_category TO GROUP usergroup;
GRANT ALL ON TABLE sellers TO GROUP usergroup;
GRANT ALL ON TABLE info TO GROUP usergroup;



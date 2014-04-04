#include "datapostgrespqxx.h"

#include <exception>

// for snprintf
#include <stdio.h> 

#include "pqxx/pqxx"
#include "log.h"
#include "convfncs.h"
#include "bdialogs.h"
#include "errhandler.h"
#include "format.h"

using namespace Burza;
using namespace pqxx;

#define read_transaction work

/*
  broken_connection is an exception class that is thrown if libpqxx™ loses its connection to the back-end. 
  It is derived from the standard C++ exception std::runtime_error, and can generally be treated as such.

  sql_error is an exception class that is thrown if a query to the back-end fails. Like broken_connection 
  it is derived from std::runtime_error, but it also carries information about the failed query. This may be useful for debugging. 

  tablestream provides simple, relatively direct access to a table for either reading (through its child 
  class tablereader) or writing (through child class tablewriter) using PostgreSQL™'s COPY command. 
  This is typically much faster than issuing SELECT or UPDATE queries. 
  
  query format:
  column name always in "". String value always in ''
  SELECT * FROM sellers where "SHORT_CODE"='PU'
  
*/

enum columnIndexes {
	sellerColumn_seller_id = 0,
	sellerColumn_short_code,
	sellerColumn_name,
	sellerColumn_comment,
	sellerColumn_changing,
	sellerColumn_changed_by,
	sellerColumn_seller_category_id,
	sellerColumn_modified,
	sellerColumn_status,
  
	itemColumn_item_id = 0,
	itemColumn_name,
	itemColumn_seller_id,
	itemColumn_price,
	itemColumn_changing,
	itemColumn_changed_by,
	itemColumn_item_category_id,
	itemColumn_number,
	itemColumn_status,
	itemColumn_modified,

	sellerCategoryColumn_id = 0,
	sellerCategoryColumn_letter,
	sellerCategoryColumn_name,
	sellerCategoryColumn_fee,
	sellerCategoryColumn_multiplier_fee_item,
	sellerCategoryColumn_multiplier_fee_item_perc,
	sellerCategoryColumn_multiplier_fee_sold_item,
	sellerCategoryColumn_multiplier_fee_sold_perc,
	
	ItemCategoryColumn_id = 0,
	ItemCategoryColumn_letter,
	ItemCategoryColumn_name,
	ItemCategoryColumn_limit_price_min,
	ItemCategoryColumn_limit_price_max,
	ItemCategoryColumn_fee_item,
	ItemCategoryColumn_fee_item_perc,
	ItemCategoryColumn_fee_sold_item,
	ItemCategoryColumn_fee_sold_perc,
};

/*
string columnNameStrings[ columnNamesCount ] = {
	"SELLER_ID",
	"SHORT_CODE",
	"NAME",
	"COMMENT",
	"CHANGING",
	"CHANGED_BY",
	"SELLER_CATEGORY_ID",
	"MODIFIED"
};
*/

#define GetItemData( column_name_index, target_variable ) \
	if (!result[ column_name_index ].to( data.target_variable )) \
		  dataCorrupt = true;

#define GetItemDataNullable( column_name_index, target_variable, default_value ) \
	if (result[ column_name_index ].is_null()) data.target_variable = default_value; else\
	{ if (!result[ column_name_index ].to( data.target_variable )) dataCorrupt = true; }
	
DataPostgresPqxx::DataPostgresPqxx()
{
	ProgramSettings s = Centrala::Settings();
	format connectionString("user=%s password=%s dbname=%s hostaddr=%s port=%i",
		s.dbUser.c_str(), s.dbPassword.c_str(), s.dbName.c_str(), s.dbAddress.c_str(), s.dbPort);

	format logString("Connecting to postgresqldb: user=%s password=%s dbname=%s hostaddr=%s port=%i\n",
		s.dbUser.c_str(), "xxx", s.dbName.c_str(), s.dbAddress.c_str(), s.dbPort);
	LogS(InfoMajor, logString.str().c_str());

	// Make a connection to the database
	string connectionFailedString;
	try {
		dbConnection = new connection(connectionString.str());
	}
	catch(pqxx::broken_connection& e)
	{
		connectionFailedString = e.what();
	}
	catch(exception& e)
	{
		connectionFailedString = e.what();
	}
	catch(...)
	{
		connectionFailedString = "Unknown PSQL exception";
	}
	if (!connectionFailedString.empty()) {
		string str = "Connection to database failed: ";
		str += connectionFailedString;
		HandlePostgresqlException(new database_exception(str));
		throw new DataStorageException();
	}
}

DataPostgresPqxx::~DataPostgresPqxx()
{
	delete dbConnection;
}

SellerDataArray DataPostgresPqxx::GetSellers( void )
{
	SellerDataArray sellers;
	
	try {
	  sellers = QueryForSellers( "SELECT * FROM sellers", "getting sellers", false );
	}
	catch(exception& e)
	{
		HandlePostgresqlException(e);
	}
	
	return sellers;
}

SellerData DataPostgresPqxx::GetSeller(SELLER_ID id)
{
	SellerDataArray sellers;
	string query = ConvFuncs::FormatStdStr( "SELECT * FROM sellers WHERE \"ID\" = %i", id );
	try {
	  sellers = QueryForSellers( query, "getting sellers", true );
	}
	catch(WrongSellerException e)
	{
		throw;
	}

	return sellers[ 0 ];
}

ItemDataArray DataPostgresPqxx::GetSellerItems(SELLER_ID id)
{
	ItemDataArray items;
	string query = ConvFuncs::FormatStdStr( "SELECT * FROM items WHERE \"SELLER_ID\" = %i ORDER BY \"NUMBER\"", id );

	try {
		items = QueryForItems( query, "getting seller items", false);
	}
	catch(exception& e)
	{
		HandlePostgresqlException(e);
	}
	
	return items;
}


bool DataPostgresPqxx::ModifySellerStart(SELLER_ID id)
{
	return false;
}


void DataPostgresPqxx::ModifySellerEnd(SELLER_ID id, SellerData& seller)
{
	seller.id = id;
  
	try {
	  pqxx::work work1(*dbConnection, "update seller" );
	  string query = ConvFuncs::FormatStdStr( 
		  "UPDATE sellers SET %s WHERE \"ID\" = %i", SellerToQueryUpdateValuesString( seller ).c_str(), id );
	  
	  result sellersResult = work1.exec( query );
	  work1.commit();
	}
	catch(exception& ex) {
		HandlePostgresqlException( ex );
	}
}

bool DataPostgresPqxx::IsSellerModified(SELLER_ID id)
{
	return false;
}


SellerData* DataPostgresPqxx::GetSellerIfModified(SELLER_ID id)
{
	return NULL;
}


bool DataPostgresPqxx::IsItemModified(ITEM_ID id)
{
	return false;
}


ItemData* DataPostgresPqxx::GetItemIfModified(ITEM_ID id)
{
	return NULL;
}


ItemData DataPostgresPqxx::GetSellerItem(SELLER_ID id, int item_number)
{
	ItemDataArray items;
	string query = ConvFuncs::FormatStdStr( "SELECT * FROM items WHERE \"SELLER_ID\" = %i AND \"ID\" = %i", id, item_number );
	try {
		items = QueryForItems( query, "getting seller item by numer", true );
	}
	catch(exception& e)
	{
		HandlePostgresqlException( e );
	}
	
	for( int i=0; i<items.size(); ++i ) {
		if ( items[ i ].number == item_number ) {
			return items[ i ];
		}
	}
	
	throw new WrongItemException();
}

ItemData DataPostgresPqxx::GetItem(ITEM_ID id)
{
	ItemDataArray items;
	string query = ConvFuncs::FormatStdStr( "SELECT * FROM items WHERE \"ID\" = %i", id );
	
	try {
		items = QueryForItems( query, "getting item by id", true );
		
		if (! items.empty() ) {
			return items[ 0 ];
		}

		throw new WrongItemException();
	}
	catch(exception& e)
	{
		HandlePostgresqlException( e );
		throw new WrongItemException();
	}
}
	
ItemCategoryArray DataPostgresPqxx::GetItemsCategories( void )
{
	string query = ConvFuncs::FormatStdStr( "SELECT * FROM item_category" );
	
	try {
		ItemCategoryArray categories = QueryForItemCategories( query, "getting item categories", false );
		
		return categories;
	}
	catch(exception& e)
	{
		HandlePostgresqlException( e );
		throw new WrongItemException();
	}
}

ItemCategoryArray DataPostgresPqxx::GetItemsCategories(vector<ITEM_CATEGORY_ID>& itemCategoryIds)
{
	string idsText = ListOfIntsToStringCommaDelimited( itemCategoryIds );
	string query = string("SELECT * FROM item_category WHERE \"ID\" IN (") + idsText + ")";

	try {
		ItemCategoryArray categories = QueryForItemCategories( query, "getting item categories", false );
		
		return categories;
	}
	catch(exception& e)
	{
		HandlePostgresqlException( e );
		throw new WrongItemException();
	}
}


ItemCategoryArray DataPostgresPqxx::GetItemsCategoriesByLetter(vector<char>& letters)
{
	string query = ConvFuncs::FormatStdStr( "SELECT * FROM item_category" );
	
	try {
		ItemCategoryArray categories = QueryForItemCategories( query, "getting item categories", false );
		
		return categories;
	}
	catch(exception& e)
	{
		HandlePostgresqlException( e );
		throw new WrongItemException();
	}
}

SellerCategoryArray DataPostgresPqxx::GetSellersCategories( void )
{
	string query = ConvFuncs::FormatStdStr( "SELECT * FROM seller_category" );
	
	try {
		SellerCategoryArray categories = QueryForSellerCategories( query, "getting seller categories" );
		
		return categories;
	}
	catch(exception& e)
	{
		HandlePostgresqlException( e );
		throw new WrongItemException();
	}
}

SellerCategoryArray DataPostgresPqxx::GetSellersCategories(vector<SELLER_CATEGORY_ID>& ids)
{
	return GetSellersCategories();
}


SellerCategoryArray DataPostgresPqxx::GetSellersCategoriesByLetter(vector<char>& letters)
{
	SellerCategoryArray cats;
	SellerCategory cat;
	int i = 0, c = letters.size();
	
	for( ; i < c; ++i ) {
		cat.Reset();
		cat.letter = letters[ i ];
		cats.push_back(cat);
	}

	return cats;
}

SellerData DataPostgresPqxx::GetSellerByCode(string& code)
{
	SellerDataArray sellers;
	string query = ConvFuncs::FormatStdStr( "SELECT * FROM sellers WHERE \"SHORT_CODE\" = '%s'", code.c_str() );
	try {
		sellers = QueryForSellers( query, "getting seller by code", true );
	}
	catch(WrongSellerException e)
	{
		throw e;
	}
	catch(exception& e)
	{
		HandlePostgresqlException( e );
		throw WrongSellerException();
	}

	if (sellers.empty())
		throw WrongSellerException();

	return sellers[ 0 ];
}

void DataPostgresPqxx::RemoveSeller(SELLER_ID id)
{
	try {
	  pqxx::work dbGetSellers(*dbConnection, "remove seller by id" );
	  string query = ConvFuncs::FormatStdStr( "DELETE FROM sellers WHERE \"ID\" = %i", id );
	  result sellersResult = dbGetSellers.exec( query );
	  dbGetSellers.commit();
	}
	catch(exception& ex) {
		HandlePostgresqlException( ex );
	}
}

SELLER_ID DataPostgresPqxx::AddSeller(SellerData& seller)
{
	seller.id = GetNewSellerId();
	ConvFuncs::GetDateTime();
	seller.last_update = ConvFuncs::datetime;
  
	try {
		pqxx::work dbGetSellers(*dbConnection, "add seller" );
		string query = ConvFuncs::FormatStdStr( 
			"INSERT INTO sellers VALUES %s", SellerToQueryValuesString( seller ).c_str() );
		
		result sellersResult = dbGetSellers.exec( query );
		dbGetSellers.commit();
	}
	catch(exception& ex) {
		HandlePostgresqlException( ex );
	}
	
	return seller.id;
}

ITEM_ID DataPostgresPqxx::AddItem(ItemData& item, SELLER_ID id)
{
	ConvFuncs::GetDateTime();
	item.last_update = ConvFuncs::datetime;
	
	try {
		item.id = GetNewItemId();
		item.seller_id = id;
		
		pqxx::work dbGetSellers(*dbConnection, "add item" );
		string query = ConvFuncs::FormatStdStr( 
			"INSERT INTO items VALUES %s", ItemToQueryNewValuesString( item ).c_str() );
		
		result sellersResult = dbGetSellers.exec( query );
		dbGetSellers.commit();
	}
	catch(exception& ex) {
		HandlePostgresqlException( ex );
	}
	
	return item.id;
}

void DataPostgresPqxx::ModifyItem(ITEM_ID item_id, ItemData& item)
{
	try {
		pqxx::work dbGetSellers(*dbConnection, "modify item" );
		string query = ConvFuncs::FormatStdStr( 
			"UPDATE items SET %s WHERE \"ID\" = %i", ItemToQueryUpdateValuesString( item ).c_str(), item_id );
		
		result sellersResult = dbGetSellers.exec( query );
		dbGetSellers.commit();
	}
	catch(exception& ex) {
		HandlePostgresqlException( ex );
	}
}

void DataPostgresPqxx::RemoveItem(ITEM_ID id)
{
	try {
		pqxx::work dbGetSellers(*dbConnection, "remove item" );
		string query = ConvFuncs::FormatStdStr( 
		  "DELETE FROM items WHERE \"ID\" = %i", id );

		result sellersResult = dbGetSellers.exec( query );
		dbGetSellers.commit();
	}
	catch(exception& ex) {
		HandlePostgresqlException( ex );
		throw new WrongItemException();
	}
}

ITEM_ID DataPostgresPqxx::GetNewItemId( void )
{
	ItemDataArray items;
	string query = ConvFuncs::FormatStdStr( "SELECT * FROM items ORDER BY \"ID\" DESC LIMIT 1" );

	try {
		items = QueryForItems( query, "getting item with greatest ID", false);
	}
	catch(exception& e)
	{
		HandlePostgresqlException( e );
		throw e;
	}
	
	SELLER_ID biggestId = items.empty() ? 1 : items[ 0 ].id;
	return biggestId + 1;
}
	
string DataPostgresPqxx::TimestampToString(ConvFuncs::s_datetime& time )
{
	// 2011-01-01 01:02:03
	string res = ConvFuncs::FormatStdStr("%04i-%02i-%02i %02i:%02i:%02i",
		time.year, time.month, time.day,
		time.hour, time.minute, time.second );
		
	return res;
}

string DataPostgresPqxx::ValidateTimestampToString(ConvFuncs::s_datetime& time )
{
	if ((time.year < 1900) || (time.month == 0) || (time.day == 0) ||
		(time.hour > 23) || (time.minute > 59) || (time.second > 59)) 
	{
		LogS( InfoError, "Invalid date, reformatting to current date.");
		ConvFuncs::GetDateTime();
		time = ConvFuncs::datetime;
	}

	return TimestampToString( time );
}

ConvFuncs::s_datetime DataPostgresPqxx::StringToTimestamp( string time )
{
	ConvFuncs::s_datetime result;
	// 2011-01-01 01:02:03
	std::vector<std::string> parts, dateParts, timeParts;
	
	ConvFuncs::Tokenize(time, parts, " ");
	if ( parts.size() != 2 ) {
		HandleException(new runtime_error( "DataPostgresPqxx::StringToTimestamp: Could not convert time string because of incorrect format." ));
	}
	
	ConvFuncs::Tokenize( parts[ 0 ], dateParts, "-" );
	ConvFuncs::Tokenize( parts[ 1 ], timeParts, ":" );

	if (( dateParts.size() != 3 ) ||
		( timeParts.size() != 3 )) {
		HandleException(new runtime_error( "DataPostgresPqxx::StringToTimestamp: Could not convert time string because of incorrect format." ));
	}
	
	result.year		= atoi( dateParts[ 0 ].c_str() );
	result.month	= atoi( dateParts[ 1 ].c_str() );
	result.day		= atoi( dateParts[ 2 ].c_str() );

	result.hour		= atoi( timeParts[ 0 ].c_str() );
	result.minute	= atoi( timeParts[ 1 ].c_str() );
	result.second	= atoi( timeParts[ 2 ].c_str() );
	
	return result;
}

void DataPostgresPqxx::DbDataToSellerData( const result::tuple& result, SellerData& data )
{
	bool dataCorrupt = false;
	string timeString;
	
	GetItemData( sellerColumn_seller_id, id )
	GetItemData( sellerColumn_short_code, shortcode )
	GetItemData( sellerColumn_name, name )
	GetItemData( sellerColumn_comment, note )
	GetItemData( sellerColumn_seller_category_id, category )

	// seller status - integer - enum
	int statusInt;
	if (!result[ sellerColumn_status ].to( statusInt ))
		  dataCorrupt = true;
	data.status = (SellerStatus) statusInt;

	if (!result[ sellerColumn_modified ].to( timeString ))
		  dataCorrupt = true;

	if ( dataCorrupt ) {
		HandleException(new runtime_error("Data in DB corrupted!"));
	}
	
	data.last_update = StringToTimestamp( timeString );
}

SellerDataArray DataPostgresPqxx::QueryForSellers( string query, string queryName, bool singleSellers )
{
	// LogS( InfoMajor, "Query name: %s single: %s\n", queryName.c_str(), singleSellers ? "yes" : "no" );
	// LogS( InfoMajor, "Query: %s\n", query.c_str() );

	SellerDataArray sellers;
	result sellersResult;
	
	try {
		pqxx::read_transaction dbGetSellers(*dbConnection, queryName );
		sellersResult = dbGetSellers.exec( query );
		// dbGetSellers.commit();
	}
	catch(exception& ex) {
		HandlePostgresqlException( ex );
		return sellers;
	}

	SellerData sellerData;
	
	int i = 0, c = sellersResult.size();
	string timeString;

	if (( singleSellers ) && ( c != 1 )) {

		if ( c == 0 ) {
			throw WrongSellerException();
		}
		
		HandleException(new runtime_error( 
			ConvFuncs::FormatStdStr( 
			"DataPostgresPqxx::QueryForSellers no single item: '%s' => '%s'.", queryName.c_str(), query.c_str() )));
	}
	
	for( ; i < c; ++i ) {
		const result::tuple sellerResult = sellersResult[ i ];
		DbDataToSellerData( sellerResult, sellerData );
		
		sellers.push_back( sellerData );
	}
	
	return sellers;
}

void DataPostgresPqxx::DbDataToSellerCategoryData( const result::tuple& result, SellerCategory& data ) 
{
	bool dataCorrupt = false;
	
	GetItemData( sellerCategoryColumn_id, id )
	GetItemData( sellerCategoryColumn_name, name )
	GetItemData( sellerCategoryColumn_fee, fee_per_sale )
	GetItemData( sellerCategoryColumn_multiplier_fee_item, multiplier_fee_item )
	GetItemData( sellerCategoryColumn_multiplier_fee_item_perc, multiplier_fee_item_perc )
	GetItemData( sellerCategoryColumn_multiplier_fee_sold_item, multiplier_fee_sold_item )
	GetItemData( sellerCategoryColumn_multiplier_fee_sold_perc, multiplier_fee_sold_item_perc )

	// 'char' type is not supported
	const char* letterValue = result[ sellerCategoryColumn_letter ].c_str();
	if ((letterValue == NULL) || (strlen(letterValue) != 1)) {
		dataCorrupt = true;
	}
	else
		data.letter = letterValue[0];

	if ( dataCorrupt ) {
		HandleException(new runtime_error("Data in DB corrupted!"));
	}
}

SellerCategoryArray DataPostgresPqxx::QueryForSellerCategories( string query, string queryName )
{
	// LogS( InfoMajor, "Query name: %s single: %s\n", queryName.c_str() );
	// LogS( InfoMajor, "Query: %s\n", query.c_str() );

	result queryResult;
	SellerCategoryArray categories;
	
	try {
		pqxx::read_transaction dbGetSellers(*dbConnection, queryName );
		queryResult = dbGetSellers.exec( query );
		// dbGetSellers.commit();
	}
	catch(exception& ex) {
		HandlePostgresqlException( ex );
		return categories;
	}

	SellerCategory category;
	
	int i = 0, c = queryResult.size();
	string timeString;

	for( ; i < c; ++i ) {
		const result::tuple singleResult = queryResult[ i ];
		DbDataToSellerCategoryData( singleResult, category );
		
		categories.push_back( category );
	}
	
	return categories;
}

ItemDataArray DataPostgresPqxx::QueryForItems( string query, string queryName, bool singleItem )
{
	// LogS( InfoMajor, "Query name: %s single: %s\n", queryName.c_str(), singleItem ? "yes" : "no" );
	// LogS( InfoMajor, "Query: %s\n", query.c_str() );

	ItemDataArray items;
	result itemResult;
	
	try {
		pqxx::read_transaction work1(*dbConnection, queryName);
		itemResult = work1.exec( query );
		// work1.commit();
	}
	catch(exception& ex) {
		HandlePostgresqlException( ex );
		return items;
	}

	int i = 0, c = itemResult.size();
	if (( singleItem ) && ( c != 1 )) {
		HandleException(new runtime_error( 
			ConvFuncs::FormatStdStr( 
			"DataPostgresPqxx::QueryForItems no single item: '%s' => '%s'.", queryName.c_str(), query.c_str() )));
		return items;
	}
	
	i = 0;
	c = itemResult.size();
	ItemData item;
	
	for( ; i < c; ++i ) {
		DbDataToItemData( itemResult[ i ], item );
		items.push_back( item );
	}
	
	return items;
}

void DataPostgresPqxx::DbDataToItemData( const result::tuple& result, ItemData& data )
{
	bool dataCorrupt = false;
	string timeString;
	
	GetItemData( itemColumn_item_id, id )
	GetItemData( itemColumn_name, name )
	GetItemData( itemColumn_seller_id, seller_id )
	GetItemData( itemColumn_price, price )
	GetItemData( itemColumn_item_category_id, category )
	GetItemData( itemColumn_number, number )
	
	int statusInt;
	if (!result[ itemColumn_status ].to( statusInt ))
		  dataCorrupt = true;
	data.status = (ItemStatus) statusInt;

	if (!result[ itemColumn_modified ].to( timeString ))
		  dataCorrupt = true;
	
	if ( dataCorrupt ) {
		HandleException( new runtime_error("Data in DB corrupted!"));
	}
	
	data.last_update = StringToTimestamp( timeString );
}

string DataPostgresPqxx::SellerToQueryValuesString( SellerData& seller )
{
	string values = ConvFuncs::FormatStdStr( 
		"( %i, '%s', '%s', '%s', %s, '%s', %i, '%s')",
		seller.id, ApostropheToDouble( seller.shortcode ).c_str(), 
		ApostropheToDouble( seller.name ).c_str(),
		ApostropheToDouble( seller.note ).c_str(), "false", "-", 
		seller.category, ValidateTimestampToString( seller.last_update ).c_str() );
	return values;
}

string DataPostgresPqxx::ItemToQueryNewValuesString( ItemData& item )
{
	string values = ConvFuncs::FormatStdStr( 
		"( %i, '%s', %i, %0.4f, '%s', '%s', %i, %i, %i, '%s' )",
		item.id, ApostropheToDouble( item.name ).c_str(), item.seller_id, item.price, 
		"false", "-", item.category, 
		item.number, (int)item.status,
		ValidateTimestampToString( item.last_update ).c_str() );
	return values;
}

string DataPostgresPqxx::ItemToQueryUpdateValuesString( ItemData& item )
{
	string values = ConvFuncs::FormatStdStr( 
"\"NAME\"= '%s', \"SELLER_ID\"= '%i', \"PRICE\"= '%0.4f', \"CHANGING\"= '%s', \
\"CHANGED_BY\"= '%s', \"ITEM_CATEGORY_ID\"= '%i', \"NUMBER\"= '%i', \"STATUS\"= '%i', \
\"MODIFIED\"= '%s'",
		ApostropheToDouble( item.name ).c_str(), item.seller_id, item.price, 
		"false", "-", item.category, item.number, item.status,
		ValidateTimestampToString( item.last_update ).c_str() );
	return values;
}

string DataPostgresPqxx::SellerToQueryUpdateValuesString( SellerData& data )
{
	string values = ConvFuncs::FormatStdStr( 
		"\"SHORT_CODE\"='%s', \"NAME\"='%s', \"COMMENT\"='%s', \"CHANGING\"='%s', \"CHANGED_BY\"='%s', \"SELLER_CATEGORY_ID\"=%i, \"MODIFIED\"='%s', \"STATUS\"='%i'",
		ApostropheToDouble( data.shortcode ).c_str(), 
		ApostropheToDouble( data.name ).c_str(), 
		ApostropheToDouble( data.note ).c_str(), 
		"false", "", data.category, ValidateTimestampToString( data.last_update ).c_str(),
		data.status );

	return values;
}

void DataPostgresPqxx::DbDataToItemCategoryData( const result::tuple& result, ItemCategory& data )
{
	bool dataCorrupt = false;
	
	GetItemData( ItemCategoryColumn_id, id )
	
	std::string text;
	if (!result[ ItemCategoryColumn_letter ].to( text ))
		dataCorrupt = true;
	else
		data.letter = !text.empty() ? text[0] : 0;
		
	GetItemData( ItemCategoryColumn_name, name )
	GetItemDataNullable( ItemCategoryColumn_limit_price_min, limit_price_min, 0 )
	GetItemDataNullable( ItemCategoryColumn_limit_price_max, limit_price_max, 10e6 )
	
	GetItemDataNullable( ItemCategoryColumn_fee_item, fee_per_item, 0 )
	GetItemDataNullable( ItemCategoryColumn_fee_item_perc, fee_item_perc, 0 )
	GetItemDataNullable( ItemCategoryColumn_fee_sold_item, fee_per_sold_item, 0 )
	GetItemDataNullable( ItemCategoryColumn_fee_sold_perc, fee_sold_perc, 0 )

	if ( dataCorrupt ) {
		HandleException(new runtime_error("Data in DB corrupted!"));
	}
}

ItemCategoryArray DataPostgresPqxx::QueryForItemCategories( string query, string queryName, bool singleCategory )
{
	// LogS( InfoMajor, "Query name: %s single: %s\n", queryName.c_str(), singleSellers ? "yes" : "no" );
	// LogS( InfoMajor, "Query: %s\n", query.c_str() );

	ItemCategoryArray categories;
	result categoryResult;
	
	try {
		pqxx::read_transaction dbGetItemCategories( *dbConnection, queryName );
		categoryResult = dbGetItemCategories.exec( query );
		// dbGetItemCategories.commit();
	}
	catch(exception& ex) {
		HandlePostgresqlException( ex );
		return categories;
	}

	ItemCategory category;
	
	int i = 0, c = categoryResult.size();
	
	if (( singleCategory ) && ( c != 1 )) {

		if ( c == 0 ) {
			throw WrongItemCategoryException();
		}
		
		HandleException(new runtime_error( 
			ConvFuncs::FormatStdStr( 
			"DataPostgresPqxx::QueryForItemCategories no single item: '%s' => '%s'.", queryName.c_str(), query.c_str() )));
	}
	
	for( ; i < c; ++i ) {
		const result::tuple oneRow = categoryResult[ i ];
		DbDataToItemCategoryData( oneRow, category );
		
		categories.push_back( category );
	}
	
	return categories;
}

string DataPostgresPqxx::ItemCategoriesToQueryNewValuesString( ItemCategory& category )
{
	string letter;
	letter += category.letter;
	
	string values = ConvFuncs::FormatStdStr( 
		"( %i, '%s', '%s', %0.4f, %0.4f, %0.4f )",
		category.id, ApostropheToDouble( letter ).c_str(), ApostropheToDouble( category.name ).c_str(), 
		category.limit_price_min, category.limit_price_max,
		category.fee_per_item, category.fee_item_perc, category.fee_per_sold_item, category.fee_sold_perc
		);
	return values;
}

string DataPostgresPqxx::ApostropheToDouble( string str )
{
	string::iterator it;
	for( it = str.begin(); it != str.end(); ++it ) {
		if ( *it != '\'' )
			continue;
		
		// move after the apostrophe
		++it;
		
		// it will point to newly inserted char
		it = str.insert( it, '\'' );
	}

	return str;
}

void DataPostgresPqxx::HandlePostgresqlException( exception& e )
{
	ErrorHandling::HandlePostgresException(e);
}

void DataPostgresPqxx::HandleException( exception& e )
{
	ErrorHandling::HandleException(e);
}

std::string DataPostgresPqxx::ListOfIntsToStringCommaDelimited(std::vector<int>& ints)
{
	std::string result;
	char text[ 32 ];
	
	for (int i=0; i<ints.size(); ++i )
	{
		if ( i > 0 )
		{
			result += ", ";
		}
		
		// result += std::to_string( ints[ i ] ); - g++ current version does not support it. Enable after compiling on newer compilers
		snprintf( text, 31, "%i", ints[ i ] );
		result += text;
	}
	
	return result;
}
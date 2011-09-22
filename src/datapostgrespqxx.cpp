#include "datapostgrespqxx.h"

#include <exception>

#include "pqxx/pqxx"
#include "log.h"
#include "convfncs.h"
#include "bdialogs.h"
#include "errhandler.h"

using namespace Burza;
using namespace pqxx;

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

DataPostgresPqxx::DataPostgresPqxx()
{
	// Make a connection to the database
	try {
		dbConnection = new connection("user=postgres password=heslo dbname=burzadb hostaddr=127.0.0.1 port=5432");
	}
	catch(exception& e)
	{
		string str = "Connection to database failed: ";
		str += e.what();
		HandlePostgresqlException(new database_exception(str));
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
	string query = ConvFuncs::FormatStdStr( "SELECT * FROM sellers WHERE \"SELLER_ID\" = %i", id );
	try {
	  sellers = QueryForSellers( query, "getting sellers", true );
	}
	catch(exception& e)
	{
		HandlePostgresqlException(e);
	}

	return sellers[ 0 ];
}

ItemDataArray DataPostgresPqxx::GetSellerItems(SELLER_ID id)
{
	ItemDataArray items;
	string query = ConvFuncs::FormatStdStr( "SELECT * FROM items WHERE \"SELLER_ID\" = %i", id );

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
	  work work1(*dbConnection, "update seller" );
	  string query = ConvFuncs::FormatStdStr( 
		  "UPDATE sellers SET %s WHERE \"SELLER_ID\" = %i", SellerToQueryUpdateValuesString( seller ).c_str(), id );
	  
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
	string query = ConvFuncs::FormatStdStr( "SELECT * FROM items WHERE \"SELLER_ID\" = %i AND \"ITEM_ID\" = %i", id, item_number );
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
	ItemCategoryArray cats;
	ItemCategory cat;
	cats.push_back(cat);
	return cats;
}


SellerCategoryArray DataPostgresPqxx::GetSellersCategories( void )
{
	SellerCategoryArray cats;
	SellerCategory cat;
	cats.push_back(cat);
	return cats;
}


ItemCategoryArray DataPostgresPqxx::GetItemsCategories(vector<ITEM_CATEGORY_ID>& ids)
{
	ItemCategoryArray cats;
	ItemCategory cat;
	vector<ITEM_CATEGORY_ID>::iterator it;
	
	for( it = ids.begin(); it != ids.end(); ++it ) {
		cats.push_back(cat);
	}
	return cats;
}


ItemCategoryArray DataPostgresPqxx::GetItemsCategoriesByLetter(vector<char>& letters)
{
	ItemCategoryArray cats;
	ItemCategory cat;
	int i = 0, c = letters.size();
	
	for( ; i < c; ++i ) {
		cats.push_back(cat);
	}
	return cats;
}


SellerCategoryArray DataPostgresPqxx::GetSellersCategories(vector<SELLER_CATEGORY_ID>& ids)
{
	SellerCategoryArray cats;
	SellerCategory cat;
	int i = 0, c = ids.size();
	
	for( ; i < c; ++i ) {
		cats.push_back(cat);
	}
	return cats;
}


SellerCategoryArray DataPostgresPqxx::GetSellersCategoriesByLetter(vector<char>& letters)
{
	SellerCategoryArray cats;
	SellerCategory cat;
	int i = 0, c = letters.size();
	
	for( ; i < c; ++i ) {
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
	catch(exception& e)
	{
		HandlePostgresqlException( e );
		throw new WrongSellerException();
	}
	
	return sellers[ 0 ];
}

void DataPostgresPqxx::RemoveSeller(SELLER_ID id)
{
	try {
	  work dbGetSellers(*dbConnection, "remove seller by id" );
	  string query = ConvFuncs::FormatStdStr( "DELETE FROM sellers WHERE \"SELLER_ID\" = %i", id );
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
		work dbGetSellers(*dbConnection, "add seller" );
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
		
		work dbGetSellers(*dbConnection, "add item" );
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
		work dbGetSellers(*dbConnection, "modify item" );
		string query = ConvFuncs::FormatStdStr( 
			"UPDATE items SET %s WHERE \"ITEM_ID\" = %i", ItemToQueryUpdateValuesString( item ).c_str(), item_id );
		
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
		work dbGetSellers(*dbConnection, "remove item" );
		string query = ConvFuncs::FormatStdStr( 
		  "DELETE FROM items WHERE \"ITEM_ID\" = %i", id );

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
	string query = ConvFuncs::FormatStdStr( "SELECT * FROM items" );

	try {
		items = QueryForItems( query, "getting all items", false);
	}
	catch(exception& e)
	{
		HandlePostgresqlException( e );
		throw e;
	}
	
	SELLER_ID biggestId = -2;
	int i = 0, c = items.size();
	
	for( ; i < c; ++i ) {
		if ( items[ i ].id > biggestId) {
			biggestId = items[ i ].id;
		}
	}
	
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

#define GetItemData( column_name_index, target_variable ) \
	if (!result[ column_name_index ].to( data.target_variable )) \
		  dataCorrupt = true;

void DataPostgresPqxx::DbDataToSellerData( const result::tuple& result, SellerData& data )
{
	bool dataCorrupt = false;
	string timeString;
	
	GetItemData( sellerColumn_seller_id, id )
	GetItemData( sellerColumn_short_code, shortcode )
	GetItemData( sellerColumn_name, name )
	GetItemData( sellerColumn_comment, note )
	GetItemData( sellerColumn_seller_category_id, category )

	if (!result[ sellerColumn_modified ].to( timeString ))
		  dataCorrupt = true;
	
	if ( dataCorrupt ) {
		HandleException(new runtime_error("Data in DB corrupted!"));
	}
	
	data.last_update = StringToTimestamp( timeString );
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

ItemDataArray DataPostgresPqxx::QueryForItems( string query, string queryName, bool singleItem )
{
	ItemDataArray items;
	result itemResult;
	
	try {
		work work1(*dbConnection, queryName);
		itemResult = work1.exec( query );
		work1.commit();
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

SellerDataArray DataPostgresPqxx::QueryForSellers( string query, string queryName, bool singleSellers )
{
	SellerDataArray sellers;
	result sellersResult;
	
	try {
		work dbGetSellers(*dbConnection, queryName );
		sellersResult = dbGetSellers.exec( query );
		dbGetSellers.commit();
	}
	catch(exception& ex) {
		HandlePostgresqlException( ex );
		return sellers;
	}

	SellerData sellerData;
	
	int i = 0, c = sellersResult.size();
	string timeString;

	if (( singleSellers ) && ( c != 1 )) {
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

string DataPostgresPqxx::SellerToQueryValuesString( SellerData& seller )
{
	string values = ConvFuncs::FormatStdStr( 
		"( %i, '%s', '%s', '%s', %s, '%s', %i, '%s')",
		seller.id, seller.shortcode.c_str(), seller.name.c_str(),
		seller.note.c_str(), "false", "-", 
		-1, TimestampToString( seller.last_update ).c_str() );
	return values;
}

string DataPostgresPqxx::ItemToQueryNewValuesString( ItemData& item )
{
	string values = ConvFuncs::FormatStdStr( 
		"( %i, '%s', %i, %0.4f, '%s', '%s', %i, %i, %i, '%s' )",
		item.id, item.name.c_str(), item.seller_id, item.price, 
		"false", "-", -1, 
		item.number, (int)item.status,
		TimestampToString( item.last_update ).c_str() );
	return values;
}

string DataPostgresPqxx::ItemToQueryUpdateValuesString( ItemData& item )
{
	string values = ConvFuncs::FormatStdStr( 
"\"NAME\"= '%s', \"SELLER_ID\"= '%i', \"PRICE\"= '%0.4f', \"CHANGING\"= '%s', \
\"CHANGED_BY\"= '%s', \"ITEM_CATEGORY_ID\"= '%i', \"NUMBER\"= '%i', \"STATUS\"= '%i', \
\"MODIFIED\"= '%s'",
		item.name.c_str(), item.seller_id, item.price, 
		"false", "-", item.category, item.number, item.status,
		TimestampToString( item.last_update ).c_str() );
	return values;
}

string DataPostgresPqxx::SellerToQueryUpdateValuesString( SellerData& data )
{
	string values = ConvFuncs::FormatStdStr( 
		"\"SHORT_CODE\"='%s', \"NAME\"='%s', \"COMMENT\"='%s', \"CHANGING\"='%s', \"CHANGED_BY\"='%s', \"SELLER_CATEGORY_ID\"=%i, \"MODIFIED\"='%s'",
		data.shortcode.c_str(), data.name.c_str(), data.note.c_str(), 
		"false", "", data.category, TimestampToString( data.last_update ).c_str() );
	return values;
}

void DataPostgresPqxx::HandlePostgresqlException( exception& e )
{
	ErrorHandling::HandlePostgresException(e);
}

void DataPostgresPqxx::HandleException( exception& e )
{
	ErrorHandling::HandleException(e);
}

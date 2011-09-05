#ifndef _DataPostgresPqxx_h_
#define _DataPostgresPqxx_h_

#include "dataaccess.h"

#include <exception>
#include "pqxx/pqxx"
#include "convfncs.h"

using namespace pqxx;
using namespace std;

//! class notifying that there is problem with database
class database_exception : public runtime_error 
{
  public:
	database_exception(const string& what) : runtime_error( what ) { }
};

namespace Burza {

class DataPostgresPqxx : public DataAccessInterface
{
  private:
	DataPostgresPqxx(const DataPostgresPqxx&);
  public:
	DataPostgresPqxx();
	virtual ~DataPostgresPqxx();
  
  public:

	virtual SellerDataArray GetSellers( void );
	virtual SellerData& GetSeller(SELLER_ID id);
	virtual ItemDataArray GetSellerItems(SELLER_ID id);
	virtual bool ModifySellerStart(SELLER_ID id);
	virtual void ModifySellerEnd(SELLER_ID id, SellerData& seller);
	virtual void RemoveSeller(SELLER_ID id);
	virtual bool IsSellerModified(SELLER_ID id);
	virtual SellerData* GetSellerIfModified(SELLER_ID id);
	virtual bool IsItemModified(ITEM_ID id);
	virtual ItemData* GetItemIfModified(ITEM_ID id);
	virtual ItemData GetSellerItem(SELLER_ID id, int item_number);
	virtual ItemData GetItem(ITEM_ID id);
	
	virtual ItemCategoryArray GetItemsCategories( void );
	virtual SellerCategoryArray GetSellersCategories( void );
	virtual ItemCategoryArray GetItemsCategories(vector<ITEM_CATEGORY_ID>& ids);
	virtual ItemCategoryArray GetItemsCategoriesByLetter(vector<char>& letters);
	virtual SellerCategoryArray GetSellersCategories(vector<SELLER_CATEGORY_ID>& ids);
	virtual SellerCategoryArray GetSellersCategoriesByLetter(vector<char>& letters);	

	virtual SellerData& GetSellerByCode(string& code);
	virtual SELLER_ID AddSeller(SellerData& seller);
	virtual ITEM_ID AddItem(ItemData& item, SELLER_ID id);
	virtual void ModifyItem(ITEM_ID item_id, ItemData& item);
	virtual void RemoveItem(ITEM_ID id);
	virtual ITEM_ID GetNewItemId( void );
	
  protected:
	
	connection *dbConnection;

	//! function that converts database format string holding 'date + time + timezone' into a structure
	ConvFuncs::s_datetime StringToTimestamp( string time );

	//! function that converts time structure into database format string holding 'date + time + timezone'
	string TimestampToString(ConvFuncs::s_datetime& time );
	
	//! function that parses one result row that queried for a seller into the SellerData
	void DbDataToSellerData( const result::tuple& sellerResult, SellerData& sellerData );

	//! function that parses one result row that queried for a seller into the ItemData
	void DbDataToItemData( const result::tuple& result, ItemData& itemData );
	
	//! query for items
	//! singleItem: if returned value is not single item, runtime_exception will be thrown
	ItemDataArray QueryForItems( string query, string queryName, bool singleItem );

	SellerDataArray QueryForSellers( string query, string queryName, bool singleSellers );
	
	string SellerToQueryValuesString( SellerData& seller );
	string SellerToQueryUpdateValuesString( SellerData& data );
	string ItemToQueryNewValuesString( ItemData& item );
	string ItemToQueryUpdateValuesString( ItemData& item );
};

}

#endif
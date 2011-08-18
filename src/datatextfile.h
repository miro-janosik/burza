#ifndef _datatextfile_h_
#define _datatextfile_h_

#include "dataaccess.h"

namespace Burza {

class DataTextfile : public DataAccessInterface
{
  public:
	DataTextfile( string& datapath );
  
  public:
	//! Data functions
	
	SELLER_ID AddSeller(SellerData& seller);
	SellerDataArray GetSellers( void );
	SellerData& GetSeller(SELLER_ID id);
	ItemDataArray GetSellerItems(SELLER_ID id);
	bool ModifySellerStart(SELLER_ID id);
	void ModifySellerEnd(SELLER_ID id, SellerData& seller);
	void RemoveSeller(SELLER_ID id);
	ITEM_ID AddItem(ItemData& item, SELLER_ID id);
	void ModifyItem(ITEM_ID item_id, ItemData& item);
	void RemoveItem(ITEM_ID id);
	bool IsSellerModified(SELLER_ID id);
	SellerData* GetSellerIfModified(SELLER_ID id);
	bool IsItemModified(ITEM_ID id);
	ItemData* GetItemIfModified(ITEM_ID id);
	SellerData& GetSellerByCode(string& code);
	ItemData GetSellerItem(SELLER_ID id, int item_number);
	ItemData GetItem(ITEM_ID id);
	SELLER_ID GetNewSellerId( void );
	ITEM_ID GetNewItemId( void );
	
	ItemCategoryArray GetItemsCategories( void );
	SellerCategoryArray GetSellersCategories( void );
	ItemCategoryArray GetItemsCategories(vector<ITEM_CATEGORY_ID>& ids);
	ItemCategoryArray GetItemsCategoriesByLetter(vector<char>& letters);
	SellerCategoryArray GetSellersCategories(vector<SELLER_CATEGORY_ID>& ids);
	SellerCategoryArray GetSellersCategoriesByLetter(vector<char>& letters);	
	
  public:
	//! Seller has filled ID from the text file.
	//! Loaded items have filled seller_id, but not their own ID.
	SELLER_ID LoadFromTxtFile( string filename, SellerData& seller, ItemDataArray& items );
	
  private:
	//! class helper functions
  
	void SaveSellerToTxtFile( SellerData& seller, ItemDataArray& items );
	void LoadFromTxtFile( SELLER_ID id, SellerData& seller, ItemDataArray& items );
	string GetSellerFilenameExisting( SELLER_ID id );
	string GetSellerFilenameNew( SellerData& seller );
	void ReplaceCharactersInString( std::string& str, const char* from, char to );
	int GetSellerIndex( SELLER_ID id );
	int GetItemIndex( ITEM_ID id );
	
  private:
	//! class data


	string	DataPath;
	
	static char file_field_delimiters[];

	ItemCategoryArray	m_ItemCategories;
	SellerCategoryArray m_SellerCategories;
};

/*
Program: jeden button - Nastavenia/setup:

	SERVER CONNECTION
		SERVER_NAME/IP
		LOGIN
		PWD
	Registration key
		key (correct/incorrect)
		registered to
		host id
		
ak pri startupe nieje server, tak msg a pouzi lokal db

*/

class DataPostgress : public DataStorage
{
  
};

}

#endif
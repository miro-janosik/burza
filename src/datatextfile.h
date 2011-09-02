#ifndef _datatextfile_h_
#define _datatextfile_h_

#include "dataaccess.h"

namespace Burza {

class DataTextfile : public DataAccessInterface
{
  private:
	DataTextfile();
  public:
	DataTextfile( string& datapath );
	virtual ~DataTextfile();
  
  public:
	//! Data functions
	
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
	
  public:
	//! Seller has filled ID from the text file.
	//! Loaded items have filled seller_id, but not their own ID.
	SELLER_ID LoadFromTxtFile( string filename, SellerData& seller, ItemDataArray& items );
	
  protected:
	//! class helper functions
  
	void SaveSellerToTxtFile( SellerData& seller, ItemDataArray& items );
	void LoadFromTxtFile( SELLER_ID id, SellerData& seller, ItemDataArray& items );
	string GetSellerFilenameExisting( SELLER_ID id );
	string GetSellerFilenameNew( SellerData& seller );
	void ReplaceCharactersInString( std::string& str, const char* from, char to );
	
  protected:
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

}

#endif
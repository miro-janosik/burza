#ifndef _datatextfile_h_
#define _datatextfile_h_

#include "dataaccess.h"

namespace Burza {

class DataTextfile : public DataStorage
{
  public:
	DataTextfile( string datapath ) { DataPath = datapath; }
  
  public:
	//! Data functions
	
	SELLER_ID AddSeller(SellerData& seller);
	SellerDataArray GetSellers( void );
	SellerData& GetSeller(SELLER_ID id);
	ItemDataArray GetSellerItems(SELLER_ID id);
	bool ModifySellerStart(SELLER_ID id);
	void ModifySellerEnd(SELLER_ID id, SellerData& seller);
	void RemoveSeller(SELLER_ID id);
	ITEM_ID AddItem(ItemData& item);
	void ModifyItem(ITEM_ID item_id, ItemData& item);
	void RemoveItem(ITEM_ID id);
	bool IsSellerModified(SELLER_ID id);
	SellerData* GetSellerIfModified(SELLER_ID id);
	bool IsItemModified(ITEM_ID id);
	ItemData* GetItemIfModified(ITEM_ID id);
	SellerData& GetSellerByCode(string& code);
	
  private:
	//! class helper functions
  
	void SaveSellerToTxtFile( SellerData& seller, ItemDataArray& items );
	SELLER_ID LoadFromTxtFile( string filename, SellerData& seller, ItemDataArray& items );
	void LoadFromTxtFile( SELLER_ID id, SellerData& seller, ItemDataArray& items );
	string GetSellerFilenameExisting( SELLER_ID id );
	string GetSellerFilenameNew( SellerData& seller );
	
  private:
	//! class data
	
	string	DataPath;
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
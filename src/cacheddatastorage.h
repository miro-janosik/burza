#ifndef _cacheddatastorage_h_
#define _cacheddatastorage_h_

#include <vector>
#include <map>
#include "datatextfile.h"

using namespace std;

namespace Burza {

class CachedDataTextfile : public DataTextfile {
private:
	CachedDataTextfile();
public:
	CachedDataTextfile( string& datapath );
	virtual ~CachedDataTextfile();
	
	virtual SellerDataArray GetSellers( void );
	virtual SellerData GetSeller(SELLER_ID id);
	virtual SELLER_ID AddSeller(SellerData& seller);
	virtual void ModifySellerEnd(SELLER_ID id, SellerData& seller);
	virtual void RemoveSeller(SELLER_ID id);
	virtual SellerData GetSellerByCode(string& code);
	
	virtual ItemData GetItem(ITEM_ID id);
	virtual void RemoveItem(ITEM_ID id);
	virtual ItemData GetSellerItem(SELLER_ID id, int item_number);
	virtual ItemDataArray GetSellerItems(SELLER_ID id);
	virtual ITEM_ID AddItem(ItemData& item, SELLER_ID sellerId);
	virtual void ModifyItem(ITEM_ID item_id, ItemData& item);
	
	virtual void SaveSellerFile(SELLER_ID id);

	virtual SELLER_ID GetNewSellerId( void );
	virtual ITEM_ID GetNewItemId( void );
	
	virtual int GetSellerIndex( SELLER_ID id );
	virtual int GetItemIndex( ITEM_ID id );
	
private:
	
	typedef map<SELLER_ID, SellerData> SellerMap;
	typedef map<ITEM_ID, ItemData> ItemMap;
	
	SellerMap	sellers;
	ItemMap	items;
} ;

}

#endif

#ifndef _cacheddatastorage_h_
#define _cacheddatastorage_h_

#include <vector>
#include <map>
#include "datatextfile.h"

using namespace std;

namespace Burza {

class CachedDataTextfile : public DataTextfile {
public:
	CachedDataTextfile( string& datapath );
	
	SellerDataArray GetSellers( void );
	SellerData& GetSeller(SELLER_ID id);
	SELLER_ID AddSeller(SellerData& seller);
	void ModifySellerEnd(SELLER_ID id, SellerData& seller);
	void RemoveSeller(SELLER_ID id);

	ItemData GetItem(ITEM_ID id);
	
private:
	
	typedef map<SELLER_ID, SellerData> SellerMap;
	typedef map<ITEM_ID, ItemData> ItemMap;
	
	SellerMap	sellers;
	ItemMap	items;
} ;

}

#endif

#ifndef _cacheddatastorage_h_
#define _cacheddatastorage_h_

#include <vector>
#include <map>
#include "dataaccess.h"

using namespace std;

namespace Burza {

class CachedDataStorage : DataStorage {
  public:
	CachedDataStorage( DataStorage& storage );
	

	SellerDataArray GetSellers( void );
	SellerData& GetSeller(SELLER_ID id);
	SELLER_ID AddSeller(SellerData& seller);
	void ModifySellerEnd(SELLER_ID id, SellerData& seller);
	void RemoveSeller(SELLER_ID id);

private:	
	map<SELLER_ID, SellerData> sellers;
	map<ITEM_ID, ItemData> items;
} ;

}

#endif

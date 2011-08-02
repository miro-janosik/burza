#ifndef _cacheddatastorage_h_
#define _cacheddatastorage_h_

#include <vector>
#include "dataaccess.h"

using namespace std;

namespace Burza {

class CachedDataStorage : DataAccessInterface( DataStorage& storage ) {

	vector<SellerData> GetSellers( void );

	SellerData& GetSeller(SELLER_ID id);

	SELLER_ID AddSeller(SELLER& seller);

	void ModifySellerEnd(SELLER_ID id, SellerData& seller);

	void RemoveSeller(SELLER_ID id);

private:	
	map<SELLER_ID, Seller> sellers;
	map<ITEM_ID, Item> items;
} ;

}

#endif

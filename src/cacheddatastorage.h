#include <vector>
#include "dataaccess.h"

using namespace std;

namespace Burza {

class CachedDataStorage : DataAccessInterface( DataStorage& storage ) {

	vector<SellerData> GetSellers( void );

	SellerData& GetSeller(SELLER_ID id);

	SELLER_ID AddSeller(SELLER& seller)

	void ModifySellerEnd(SELLER_ID id, SellerData& seller);

	void RemoveSeller(int SELLER_ID)
	{
		UpdateSeller(int SELLER_ID);
		if (sellers.contains(SELLER_ID) == false) {
			throw new WrongSellerIdException();
		}

		dataAccess.RemoveSeller(SELLER_ID);
		sellers.Remove(SELLER_ID);
		items.RemoveWhere(SELLER_ID);
	}


private:
	map<SELLER_ID, Seller> sellers;
	map<ITEM_ID, Item> items;
} ;

}

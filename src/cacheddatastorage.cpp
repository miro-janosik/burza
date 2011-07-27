#include "cacheddatastorage.h"
#include "dataaccess.h"

using namespace Burza;
using namespace std;

vector<SellerData> CachedDataStorage::GetSellers( void )
{
	sellers.clear();
	sellers = dataAccess.GetSellers();

	return sellers;		
}

SellerData& CachedDataStorage::GetSeller(SELLER_ID id)
{
	if (sellers.contains(id)) {
		SellerData* s = dataAccess.GetSellerIfModified(id);
		if (s != NULL) {
			sellers[ id ] = *s;
		}
		return sellers[ id ];
	}

	if (dataAccess.SellersModified(sellersDate)) {
		GetSellers();
		if (sellers.contains(id)) {
			return sellers[ id ];
		}
	}
	
	throw new WrongSellerIdException();
}

SELLER_ID CachedDataStorage::AddSeller(SellerData& seller)
{
	// generate unique seller ID
	sellers[ id ] = seller;
	dataAccess.AddSeller(seller);
}

void CachedDataStorage::ModifySellerEnd(SELLER_ID id, SellerData& seller)
{
	vector<SellerData>::iterator it = sellers.find(id);
	
	if (it != sellers.end()) {
		GetSellers();
	} else {
		SellerData* s = dataAccess.GetSellerIfModified(id);
		if (s != NULL) {
			sellers[ id ] = *s;
		}
	}
}

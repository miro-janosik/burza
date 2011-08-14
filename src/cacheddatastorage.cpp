#include "cacheddatastorage.h"
#include "dataaccess.h"
#include <bits/algorithmfwd.h>

using namespace Burza;
using namespace std;

CachedDataTextfile::CachedDataTextfile( string& datapath ) 
  : DataTextfile( datapath )
{
	// preload all data
	SellerDataArray preloadedSellers = DataTextfile::GetSellers();
	ItemDataArray preloadedItems;

	for( int i=0; i<preloadedSellers.size(); ++i ) {
		sellers[ preloadedSellers[ i ].id ] = preloadedSellers[ i ];

		preloadedItems = DataTextfile::GetSellerItems( preloadedSellers[ i ].id );
		for( int j=0; j<preloadedItems.size(); ++j ) {
			items[ preloadedItems[ j ].id ] = preloadedItems[ j ];
		}
	}
}

SellerDataArray CachedDataTextfile::GetSellers( void )
{
	SellerDataArray outSellers;
	
	for( int i=0; i<sellers.size(); ++i ) {
		outSellers.push_back( sellers[ i ] );
	}
	
	return outSellers;
}

SellerData& CachedDataTextfile::GetSeller(SELLER_ID id)
{
	SellerMap::iterator it = sellers.find( id );
	if ( it == sellers.end() ) {
		throw new WrongSellerException();
	}
  
	return it->second;
	
	/*
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
	*/
}

SELLER_ID CachedDataTextfile::AddSeller(SellerData& seller)
{
	// generate unique seller ID
	SELLER_ID id = DataTextfile::AddSeller(seller);
	sellers[ id ] = seller;
}

void CachedDataTextfile::ModifySellerEnd(SELLER_ID id, SellerData& seller)
{
	/*
	SellerMap::iterator it = sellers.find( id );
	
	if (it == sellers.end()) {
		GetSellers();
	} else {
		SellerData* s = dataAccess.GetSellerIfModified(id);
		if (s != NULL) {
		}
	}
	*/
	
	sellers[ id ] = seller;
	
	DataTextfile::ModifySellerEnd( id, seller );
}

void CachedDataTextfile::RemoveSeller(SELLER_ID id)
{
	if ( sellers.find(id) == sellers.end()) {
		throw new WrongSellerException();
	}

	DataTextfile::RemoveSeller( id );
	sellers.erase( id );
	
	for( ItemMap::iterator it=items.begin(); it != items.end(); ++it ) {
		if ( it->second.seller_id == id ) {
			items.erase(it);
		}
	}
}

ItemData CachedDataTextfile::GetItem(ITEM_ID id)
{
	ItemMap::iterator it = items.find( id );
	if ( it != items.end() ) {
		return it->second;
	}
	
	throw new WrongItemException();
}

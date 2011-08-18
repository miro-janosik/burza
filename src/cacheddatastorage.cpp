#include "cacheddatastorage.h"
#include "dataaccess.h"
#include <bits/algorithmfwd.h>

#include "log.h"

using namespace Burza;
using namespace std;

CachedDataTextfile::CachedDataTextfile( string& datapath ) 
  : DataTextfile( datapath )
{
	// preload all data
	std::string		predajca_pat, filename;
	SellerData		seller;
	ItemDataArray	seller_items;
	SELLER_ID		id;

	sellers.clear();
	
	predajca_pat = DataPath;
	predajca_pat += "*.txt";
	
	bool found;
	found = ConvFuncsInstance->Find_FirstFile( predajca_pat.c_str() );
	while( found ) {
		found = ConvFuncsInstance->Find_NextFile();
		
		filename = DataPath;
		filename += ConvFuncsInstance->Find_CurrentFileName();
		
		try {
			seller_items.clear();
			id = LoadFromTxtFile( filename, seller, seller_items );

			sellers[ seller.id ] = seller;
			
			for( int j=0; j<seller_items.size(); ++j ) {
				
				// items don't have ID's stored
				seller_items[ j ].id = GetNewItemId();
				items[ seller_items[ j ].id ] = seller_items[ j ];
			}
		}
		catch(DataStorageException& e)
		{
			LogS( InfoError, "Error loading cached file, error: %s\n", e.what() );
			continue;
		}
	}
}

SellerDataArray CachedDataTextfile::GetSellers( void )
{
	SellerDataArray outSellers;
	SellerMap::iterator	ss, se;
	
	for( ss = sellers.begin(), se = sellers.end(); se != ss; ++ss ) {
		outSellers.push_back( ss->second );
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
	
	SellerMap::iterator it = sellers.find( id );
	if ( it == sellers.end() ) {
		throw new WrongSellerException();
	}
	
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
	
	Save
}

ItemData CachedDataTextfile::GetItem(ITEM_ID id)
{
	ItemMap::iterator it = items.find( id );
	if ( it != items.end() ) {
		return it->second;
	}
	
	throw new WrongItemException();
}

ItemData CachedDataTextfile::GetSellerItem(SELLER_ID id, int item_number)
{
	SellerMap::iterator sellerit = sellers.find( id );
	if ( sellerit == sellers.end() ) {
		throw new WrongSellerException();
	}
	
	for( ItemDataIterator itemit = items.begin(), itemend = items.end(); 
	  itemit != itemend; ++itemit ) {
		 if (( itemit->seller_id == id ) && ( itemit->number == item_number ))
			return *itemit;
	}
  
	throw new WrongItemException();
}

SellerDataArray CachedDataTextfile::GetSellers( void )
{
	SellerDataArray		result;
	SellerMap::iterator	sit, site;
	
	for( sit = sellers.begin(), site = sellers.end(); sit != site; ++sit ) {
		result.push_back( sit.second );
	}
	
	return result;
}

SellerData& CachedDataTextfile::GetSeller(SELLER_ID id)
{
	SellerMap::iterator	sit = sellers.find( id );
	if ( sit == sellers.end() ) {
		throw new WrongSellerException();
	}
	
	return sit.second;
}

void CachedDataTextfile::RemoveItem(ITEM_ID id);

void CachedDataTextfile::RemoveItem(ITEM_ID id)
{
	SELLER_ID seller_id = GetItem( id ).seller_id;
	items.erase( id );
	
	SaveSellerFile( seller_id );
}

void CachedDataTextfile::SaveSellerFile(SELLER_ID id);

void CachedDataTextfile::SaveSellerFile(SELLER_ID id)
{
	SellerData seller = GetSeller( seller_id );
	ItemDataArray items = GetSellerItems( seller_id );
	
	SaveSellerToTxtFile(seller, items);
}

SellerData& CachedDataTextfile::GetSellerByCode(string& code)

SellerData& CachedDataTextfile::GetSellerByCode(string& code)
{
	SellerMap::iterator it = sellers.begin(), ite = sellers.end();
	for( ; it != ite; ++it ) {
		if (it->shortcode == code) {
			return *it;
		}
	}
	
	throw WrongSellerException();
}

SELLER_ID CachedDataTextfile::GetNewSellerId( void )

SELLER_ID CachedDataTextfile::GetNewSellerId( void )
{
	SELLER_ID biggestId = 0;
	
	SellerDataIterator it = sellers.begin(), ite = sellers.end();
	for( ; it != ite; ++it ) {
		if (it->id > biggestId) {
			biggestId = it->id;
		}
	}
	
	return biggestId + 1;
}

ITEM_ID DataTextfile::GetNewItemId( void )

ITEM_ID CachedDataTextfile::GetNewItemId( void )
{
	ITEM_ID biggestId = 0;
	
	ItemMap::iterator itemit = items.begin(), itemite = items.end();
  
	for( ; itemit != itemite; ++itemit ) {
		if (itemit->id > biggestId) {
			biggestId = itemit->id;
		}
	}
	
	return biggestId + 1;
}

ItemDataArray CachedDataTextfile::GetSellerItems(SELLER_ID id)

ItemDataArray CachedDataTextfile::GetSellerItems(SELLER_ID id)
{
	SellerMap::iterator sellerit = sellers.find( id );
	if ( sellerit == sellers.end() ) {
		throw new WrongSellerException();
	}
	
	ItemDataArray result;
	
	for( ItemDataIterator itemit = items.begin(), itemend = items.end(); 
	  itemit != itemend; ++itemit ) {
		 if ( itemit->seller_id == id )
			result.push_back( *itemit );
	}
  
	return result;
}

SELLER_ID DataTextfile::AddSeller(SellerData& seller)

SELLER_ID CachedDataTextfile::AddSeller(SellerData& seller)
{
	assert( seller.id == SELLER_ID_NONE );
	
	seller.id = GetNewSellerId();
	sellers[ seller.id ] = seller;
	
	SaveSellerFile( seller_id );
	
	return seller.id;
}

ITEM_ID DataTextfile::AddItem(ItemData& item, SELLER_ID sellerId)
void DataTextfile::ModifyItem(ITEM_ID item_id, ItemData& item)

ITEM_ID CachedDataTextfile::AddItem(ItemData& item, SELLER_ID sellerId)
{
	assert( item.id == ITEM_ID_NONE );

	item.id = GetNewItemId();
	items[ item.id ] = item;
	
	SaveSellerFile( sellerId );
	return item.id;
}

void CachedDataTextfile::ModifyItem(ITEM_ID item_id, ItemData& item)
{
	ItemMap::iterator it = items.find( item_id );
	if ( it == items.end() ) {
		throw new WrongItemException();
	}
	
	// just to be sure, fill in the id.
	item.id = item_id;
	items[ item_id ] = item;
	
	SaveSellerFile( item.seller_id );
}


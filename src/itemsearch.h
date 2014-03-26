#ifndef _itemsearch_h_
#define _itemsearch_h_

#include <vector>
#include <string>
#include "dataaccess.h"

using namespace std;

namespace Burza {

class ItemSearch
{
public:
	ItemSearch(void);
	~ItemSearch(void);

	//! Loads the items from the data storage. Call if you think that data were updated.
	//! Also be sure to call it before looking for items, as only loaded items are searched through
	void FetchLatestData( void );

	//! Returns array of items that match the partOfName string
	//! partOfName: one or more words where each is separately 
	//! sellerId: if specified, then only items of that seller are searched.
	//! limitItemCount: limits number of found items
	//! Return: items are sorted, first are those most similair
	vector<ItemData> LookForItemsContainingWords( string& partOfName, SELLER_ID sellerId = SELLER_ID_NONE, int limitItemCount = -1 );

	//! searches items that are closest matching the [partOfName]
	vector<ItemData> LookForItemsDistance( string& partOfName, SELLER_ID sellerId );

	// Measures 'distance' or 'difference' of two words.
	// Algorythm taken from page 
	// http://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance
	template<class T> unsigned int levenshtein_distance(const T &s1, const T & s2);

	//! Helper function to get specified seller. Seller data are loaded
	//! together with item data through FetchLatestData()
	//! Throws WrongSellerException() if no such seller is in the 'sellers' array.
	SellerData GetSeller( SELLER_ID sellerId );

private:

	vector< ItemData > items;

	vector< SellerData > sellers;
};

}

#endif

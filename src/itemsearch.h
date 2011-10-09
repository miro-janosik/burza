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

	//! Returns array of items that match the partOfName string
	//! partOfName: one or more words where each is separately 
	//! sellerId: if specified, then only items of that seller are searched.
	//! Return: items are sorted, first are those most similair
	vector<ItemData> LookForItemsContainingWords( string& partOfName, SELLER_ID sellerId = SELLER_ID_NONE );

	vector<ItemData> LookForItemsStemmming( string& partOfName, SELLER_ID sellerId );

	//! Loads the items from the data storage. Call if you think that data were updated.
	void FetchLatestItemData( void );

	// Measures 'distance' or 'difference' of two words.
	// Algorythm taken from page 
	// http://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance
	template<class T> unsigned int levenshtein_distance(const T &s1, const T & s2);

private:

	vector< ItemData > items;
};

}

#endif

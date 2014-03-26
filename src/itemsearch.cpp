#include <vector>
#include <string>
#include <algorithm>
#ifdef _UNIX
	#include <stdexcept>
#endif

#include "itemsearch.h"
#include "dataaccess.h"
#include "burza.h"
#include "errhandler.h"
#include "stringencoding.h"
#include "convfncs.h"

using namespace Burza;
using namespace std;

ItemSearch::ItemSearch(void)
{
}

ItemSearch::~ItemSearch(void)
{
}

template<class T>
unsigned int ItemSearch::levenshtein_distance(const T &s1, const T & s2) 
{
    const size_t len1 = s1.size(), len2 = s2.size();
    vector<unsigned int> col(len2+1), prevCol(len2+1);

    for (unsigned int i = 0; i < prevCol.size(); i++)
        prevCol[i] = i;
    for (unsigned int i = 0; i < len1; i++) {
        col[0] = i+1;
        for (unsigned int j = 0; j < len2; j++)
            col[j+1] = min( min( 1 + col[j], 1 + prevCol[1 + j]),
                                prevCol[j] + (s1[i]==s2[j] ? 0 : 1) );
        col.swap(prevCol);
    }
    return prevCol[len2];
}

void ItemSearch::FetchLatestData( void )
{
	items.clear();
	sellers.clear();

	try {
		sellers = Centrala::Storage().GetSellers();

		SellerDataIterator it = sellers.begin(), ite = sellers.end();
		for( ; it != ite; ++it ) {
	
			ItemDataArray sellerItems = Centrala::Storage().GetSellerItems( it->id );

			for( int i=0; i < sellerItems.size(); ++i ) {
				
				StringEncoding::UTF8RemoveAccents( sellerItems[ i ].name );
				std::transform( sellerItems[ i ].name.begin(), sellerItems[ i ].name.end(), 
					sellerItems[ i ].name.begin(), static_cast<int (*)(int)>(tolower) );
				items.push_back( sellerItems[ i ] );
			}
		}
	}
	catch(const exception& e) {
		ErrorHandling::HandleException(e);
	}
	catch(...) {
		runtime_error e("ItemSearch::FetchLatestData");
		ErrorHandling::HandleException(e);
	}
}

vector<ItemData> ItemSearch::LookForItemsContainingWords( string& partOfName, SELLER_ID sellerId, int limitItemCount )
{
	vector<ItemData> result;

	if ( partOfName.size() <= 3 ) {
		return result;
	}

	if (limitItemCount == -1)
		limitItemCount = 10e6;
	
	const unsigned int limit = 5;
	unsigned int matches;
	vector<ItemData> preResults[ limit ];
	int i, c, w;
	vector<string> words;
	string itemText;

	StringEncoding::UTF8RemoveAccents( partOfName );
	std::transform( partOfName.begin(), partOfName.end(), partOfName.begin(), static_cast<int (*)(int)>(tolower) );
	ConvFuncs::Tokenize( partOfName, words );

	int numOfWordsOrLimit = words.size();
	if (numOfWordsOrLimit > limit)
		numOfWordsOrLimit = limit;
	
	for( c = items.size(), i = 0; i < c; ++i ) {

		matches = 0;
		itemText = items[ i ].name;
		itemText += " ";
		itemText += ConvFuncs::FormatCstr( "%0.2f", items[ i ].price );

		for( w = 0; w < words.size(); ++w ) {
			if ( itemText.find( words[ w ] ) != string::npos ) {

				matches++;
			}
		}

		if ( matches > 0 ) {
			if (matches >= limit) {
				matches = limit - 1;
			}

			preResults[ matches ].push_back( items[ i ] );
			
			// break in the middle if limit is reached
			if ( preResults[numOfWordsOrLimit].size() >= limitItemCount)
				break;
		}
	}

	/*
	// return all matches sorted by matches count
	for( int i = limit - 1; i >= 0; --i ) {
		for( int j = 0; j < preResults[ i ].size(); ++j ) {
			result.push_back( preResults[ i ][ j ] );
		}
	}
	*/

	// return the items that have most matches
	for( int i = limit - 1; i >= 0; --i ) {
		if ( ! preResults[ i ].empty() ) {
			
			if ( preResults[ i ].size() > limitItemCount )
				preResults[ i ].erase( preResults[ i ].begin() + limitItemCount, preResults[ i ].end() );
			
			return preResults[ i ];
		}
	}

	return result;
}

vector<ItemData> ItemSearch::LookForItemsDistance( string& partOfName, SELLER_ID sellerId )
{
	vector<ItemData> result;
	/*

	if ( partOfName.size() <= 3 ) {
		return result;
	}

	const unsigned int limit = 5;
	unsigned int distance;
	vector<ItemData> preResults[ limit ];
	int i, c, w;
	vector<string> words;

	StringEncoding::UTF8RemoveAccents( partOfName );
	
	for( c = items.size(), i = 0; i < c; ++i ) {

		distance = levenshtein_distance<string>( partOfName, items[ i ].name );

		if ( distance < limit ) {

			items[ i ].name += ConvFuncs::FormatStdStr( " - %i", distance );
			preResults[ distance ].push_back( items[ i ] );
		}
	}

	for( int i = 0; i < limit; ++i ) {
		for( int j = 0; j < preResults[ i ].size(); ++j ) {
			result.push_back( preResults[ i ][ j ] );
		}
	}
	*/

	return result;
}

SellerData ItemSearch::GetSeller( SELLER_ID sellerId )
{
	SellerDataIterator it;

	for( it = sellers.begin(); it != sellers.end(); ++it )
		if ( it->id == sellerId )
			return *it;

	throw WrongSellerException();
}

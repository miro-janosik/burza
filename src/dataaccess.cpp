#include "dataaccess.h"

using namespace Burza;

const SELLER_ID SELLER_ID_NONE = -1;
const ITEM_ID ITEM_ID_NONE = -1;
const SELLER_CATEGORY_ID SELLER_CATEGORY_ID_NONE = -1;
const ITEM_CATEGORY_ID ITEM_CATEGORY_ID_NONE = -1;

char DataAccessInterface::m_status_character_in_file[ ItemStatusCountWithUndefined + 1 ] = "ospvdnu";
string DataAccessInterface::m_status_string[ ItemStatusCountWithUndefined ] = {
  "Nezaradene",
  "Skladom",
  "Predane",
  "Vratene",
  "Dar",
  "Dohladat",
  "Nedefinovane",
};

SellerData::SellerData()
{
	name = "";
	shortcode = "";
	note = "";
	category = -1;
	status = SellerStatus::imported;
  
	ConvFuncs::GetDateTime();
	last_update = ConvFuncs::datetime;
	
	// left unset untill it will be stored to DB
	id = -1;
}

SellerData::SellerData(const SellerData& a)
{
	id			= a.id;
	name		= a.name;
	shortcode	= a.shortcode;
	note		= a.note;
	category	= a.category;
	last_update	= a.last_update;
	status		= a.status;
}

SellerData::SellerData(SELLER_ID _id, string _name, string _shortcode, string _note, int _category)
{
	name = _name;
	shortcode = _shortcode;
	note = _note;
	category = _category;
  
	ConvFuncs::GetDateTime();
	last_update = ConvFuncs::datetime;
	
	id = _id;
	status = SellerStatus::imported;
}

SellerData::SellerData(string _name, string _shortcode, string _note, int _category)
{
	name = _name;
	shortcode = _shortcode;
	note = _note;
	category = _category;
  
	ConvFuncs::GetDateTime();
	last_update = ConvFuncs::datetime;
	
	id = -1;
	status = SellerStatus::imported;
}

ItemData::ItemData()
{
	name = "";
	number = -1;
	price = -1.0;
	category = -1;
	status = not_in_stock_yet;
	seller_id = -1;
	id = -1;
}

ItemData::ItemData(string _name, int _number, double _price, int _category, ItemStatus _status)
{
	name = _name;
	number = _number;
	price = _price;
	category = _category;
	status = _status;
	seller_id = -1;
	id = -1;
}

ItemData::ItemData(const ItemData& a)
{
	id = a.id;
	name = a.name;
	number = a.number;
	price = a.price;
	category = a.category;
	status = a.status;
	seller_id = a.seller_id;
}

ItemDataArray DataAccessInterface::GetSellerItems(SELLER_ID id)
{
	throw NotImplementedException();
  /*
	ItemDataArray	result;
  
	for( ItemDataIterator it=items.begin(); it != items.end(); ++it ) {
		if ( it->seller_id == id ) {
			result.push_back(*it);
		}
	}
	
	return result;
  */
}

ItemData* DataAccessInterface::FindItemByNumber( ItemDataArray& list, int number )
{
	for( ItemDataIterator itemit = list.begin(), itemend = list.end(); 
	  itemit != itemend; ++itemit ) {
		 if (itemit->number == number )
			return & *itemit;
	}
	
	return NULL;
}

int DataAccessInterface::FindSellerIndexByCode( SellerDataArray& list, string code )
{
	int index;
	SellerDataIterator it, ite;
	
	for( it = list.begin(), ite = list.end(), index = 0; 
	  it != ite; 
	  ++it, ++index ) {
		  if ( stricmp( it->shortcode.c_str(), code.c_str() ) == 0 )
			return index;
	}
	
	return -1;
}

void DataAccessInterface::AddItems( ItemDataArray& items, SELLER_ID sellerId )
{
	ITEM_ID	id;
	
	for( ItemDataIterator itemit = items.begin(), itemend = items.end(); 
	  itemit != itemend; ++itemit ) {
		
		itemit->seller_id = sellerId;
		id = AddItem(*itemit, sellerId);
		itemit->id = id;
	}
}

ItemDataArray DataAccessInterface::GetItems(vector<ITEM_ID> ids)
{
	ItemDataArray items;
  
	for( int i=0; i<ids.size(); ++i ) {
		items.push_back( GetItem( ids[ i ] ) );
	}
	
	return items;
}

const char* DataAccessInterface::ItemStatus2String( ItemStatus status )
{
	if (( status < 0 ) || ( status >= ItemStatusCount ))
		return "?";
	
	return m_status_string[ status ].c_str();
}

char DataAccessInterface::ItemStatus2Char( ItemStatus status )
{
	if (( status < 0 ) || ( status >= ItemStatusCount ))
		return '?';
	
	return m_status_character_in_file[ status ];
}

ItemStatus DataAccessInterface::Char2ItemStatus( char c )
{
	for( int i=0; i < ItemStatusCount; ++i )
		if ( m_status_character_in_file[ i ] == c )
			return (Burza::ItemStatus) i;
	
	return undefined;
}

ConvFuncs::s_datetime DataAccessInterface::GetCurrentTime( void )
{
	ConvFuncs::GetDateTime();
	return ConvFuncs::datetime;
}

ItemCategoryArray DataAccessInterface::GetItemsCategories( ItemDataArray& items )
{
	vector<ITEM_CATEGORY_ID> ids;
	
	for( int i = 0; i < items.size(); ++i )
		ids.push_back( items[ i ].category );
	
	return GetItemsCategories( ids );
}

ItemCategory DataAccessInterface::GetItemCategory(ITEM_CATEGORY_ID id)
{
	vector<ITEM_CATEGORY_ID> ids; 
	ids.push_back( id );
	
	ItemCategoryArray categories = GetItemsCategories( ids );
	return categories[ 0 ];
}

ItemCategory DataAccessInterface::GetItemCategoryByLetter(char letter)
{
	vector<char> letters;
	letters.push_back( letter );
	
	ItemCategoryArray categories = GetItemsCategoriesByLetter( letters );
	return categories[ 0 ];
}

SellerCategory DataAccessInterface::GetSellerCategory(SELLER_CATEGORY_ID id)
{
	vector<SELLER_CATEGORY_ID> ids;
	ids.push_back( id );
	
	SellerCategoryArray categories = GetSellersCategories( ids );
	return categories[ 0 ];
}

SellerCategory DataAccessInterface::GetSellerCategoryByLetter(char letter)
{
	vector<char> letters;
	letters.push_back( letter );
	
	SellerCategoryArray categories = GetSellersCategoriesByLetter( letters );
	return categories[ 0 ];
}

SELLER_ID DataAccessInterface::GetNewSellerId( void )
{
	SellerDataArray sellers = GetSellers();
	
	SELLER_ID biggestId = 0;
	int i = 0, c = sellers.size();
	
	for( ; i < c; ++i ) {
		if ( sellers[ i ].id > biggestId) {
			biggestId = sellers[ i ].id;
		}
	}
	
	return biggestId + 1;
}

string DataAccessInterface::GenerateSellerUniqueCode( string sellerName, string firstTryCode, SELLER_ID sellerId )
{
	// if its empty try to generate it from name
	if ( firstTryCode.empty() ) {
		std::vector<std::string> tokens;
		ConvFuncs::Tokenize( sellerName, tokens );
		
		for( int i=0; i<tokens.size(); ++i ) {
			firstTryCode += tokens[ i ][ 0 ];
		}
	}
	
	// minimum length 2
	while ( firstTryCode.size() < 2 ) {
		firstTryCode += "A";
	}
	
	// run through all the sellers to see if its unique
	// if its not, generate first that is.
	int sellerIndex;
	int tries = 0;
	SellerDataArray	sellers = GetSellers();
	
	while( true ) {
		
		ConvFuncs::StdStringToUpper( firstTryCode );
		sellerIndex = DataAccessInterface::FindSellerIndexByCode( sellers, firstTryCode );

		// if there is no user with such code, we're OK
		if ( sellerIndex == -1 )
			break;

		// if there is seller with such code and that is me, its OK too
		if ( sellers[ sellerIndex ].id == sellerId )
			break;
		
		// inc characters, AB => AC, AZ => BA
		int char_pos = firstTryCode.size() - 1;
		while( char_pos >= 0 ) {
			if ( firstTryCode[ char_pos ] >= 'Z' ) {
				firstTryCode[ char_pos ] = 'A';
				char_pos--;
			} else {
				firstTryCode[ char_pos ]++;
				break;
			}
		}
		
		// Wooh, I could loop like this forever. Just help it if its too much
		tries++;
		if ( tries > 600 ) {
			firstTryCode += "A";
			tries = 0;
		}
	}
	
	return firstTryCode;
}

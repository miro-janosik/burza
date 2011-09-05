#include "dataaccess.h"

using namespace Burza;

const SELLER_ID SELLER_ID_NONE = -1;
const ITEM_ID ITEM_ID_NONE = -1;
const SELLER_CATEGORY_ID SELLER_CATEGORY_ID_NONE = -1;
const ITEM_CATEGORY_ID ITEM_CATEGORY_ID_NONE = -1;

char DataAccessInterface::m_status_character_in_file[ ItemStatusCount + 1 ] = "ospvdnu";
string DataAccessInterface::m_status_string[ ItemStatusCount ] = {
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
		 if (it->shortcode == code )
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
	
	SELLER_ID biggestId = -2;
	int i = 0, c = sellers.size();
	
	for( ; i < c; ++i ) {
		if ( sellers[ i ].id > biggestId) {
			biggestId = sellers[ i ].id;
		}
	}
	
	return biggestId + 1;
}

#include "dataaccess.h"

using namespace Burza;

const SELLER_ID SELLER_ID_NONE = -1;
const ITEM_ID ITEM_ID_NONE = -1;
const SELLER_CATEGORY_ID SELLER_CATEGORY_ID_NONE = -1;
const ITEM_CATEGORY_ID ITEM_CATEGORY_ID_NONE = -1;

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

SellerData::SellerData(string _name, string _shortcode, string _note, int _category);
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
	status = ItemStatus::not_in_stock_yet;
	seller_id = -1;
	id = -1;
}

ItemData::ItemData(string _name, int _number, double _price, int _category, int _status)
{
	name = _name;
	number = _number;
	price = _price;
	category = _category;
	status = _status;
	seller_id = -1;
	id = -1;
}

ItemDataArray DataAccessInterface::GetSellerItems(SELLER_ID id)
{
	ItemDataArray	result;
  
	for( ItemDataIterator it=items.begin(); it != items.end(); ++it ) {
		if ( it->seller_id == id ) {
			result.push_back(*it);
		}
	}
	
	return result;
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
	for( SellerDataIterator it = list.begin(), ite = list.end(), int index = 0; 
	  it != ite; 
	  ++it, ++index ) {
		 if (it->shortcode == code )
			return index;
	}
	
	return -1;
}

void DataAccessInterface::AddItems( ItemDataArray& items )
{
	ITEM_ID	id;
	
	for( ItemDataIterator itemit = list.begin(), itemend = list.end(); 
	  itemit != itemend; ++itemit ) {
		
		id = AddItem(*itemit);
		itemit->id = id;
	}
}

ItemDataArray& DataAccessInterface::GetItems(vector<ITEM_ID> ids)
{
	ItemDataArray items;
  
	for( int i=0; i<ids.size(); ++i ) {
		items.push_back( GetItem( ids[ i ] ) );
	}
	
	return items;
}

const char* DataAccessInterface::ItemStatus2String( ItemStatus status )
{
	if (( status < 0 ) || ( status >= pocet_stavov ))
		return "?";
	
	return m_string_stavu[ status ].c_str();
}

char DataAccessInterface::ItemStatus2Char( ItemStatus status )
{
	if (( status < 0 ) || ( status >= ItemStatusCount ))
		return '?';
	
	return m_znak_stavu_v_subore[ status ];
}

ItemStatus DataAccessInterface::Char2ItemStatus( char c )
{
	for( int i=0; i < pocet_stavov; ++i )
		if ( m_znak_stavu_v_subore[ i ] == c )
			return (Burza::ItemStatus) i;
	
	return undefined;
}


#include "dataaccess.h"

using namespace Burza;

SellerData::SellerData(string _name, string _shortcode, string _note, char _category);
{
	name = _name;
	shortcode = _shortcode;
	note = _note;
	category = _category;
  
	ConvFuncs::GetDateTime();
	last_update = ConvFuncs::datetime;
	
	unique_id = xx;
}

ItemData::ItemData(string _name, int _number, double _price, char _category, int _status, SELLER_ID _seller_id)
{
	name = _name;
	number = _number;
	price = _price;
	category = _category;
	status = _status;
	seller_id = _seller_id;
}

ItemDataArray DataTextfile::GetSellerItems(SELLER_ID id)
{
	ItemDataArray	result;
  
	for( ItemDataIterator it=items.begin(); it != items.end(); ++it ) {
		if ( it->seller_id == id ) {
			result.push_back(*it);
		}
	}
	
	return result;
}


#ifndef _fees_h_
#define _fees_h_

#include <string>
#include <vector>

using namespace std;
namespace Burza {

class SellerData;
class ItemDataArray;

struct Fee {
	string	name;
	double	price;

	Fee(const char* iName, double iPrice)
	{
		name = iName;
		price = iPrice;
	}
};

class SellerFees
{
public:
	SellerFees(void)
	{
		finalSumToGiveToSeller = 0.0;
	}

	std::vector< Fee > fees;

	double sumOfSoldItems;

	// sum minus all fees
	double finalSumToGiveToSeller;

	//! rounds the price to 5 cents ( 0.05 )
	static double RoundTo5Cents( double price );

	//! find out all the fees
	static SellerFees& CalculateSellerFees( SellerCategory& sellerCategory, ItemDataArray& items );
};

} // namespace Burza

#endif
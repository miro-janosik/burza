#ifndef _fees_h_
#define _fees_h_

#include <string>
#include <vector>

#include "dataaccess.h"

using namespace std;
namespace Burza {

enum FeeType {
	FeeUndefined,
	FeeWholeSoldSumPercentage,
	FeePerItem,  
} ;

struct Fee {
	string	name;
	double	price;
	FeeType type;

	Fee(const char* iName, double iPrice, FeeType iType)
	{
		name = iName;
		price = iPrice;
		type = iType;
	}
};

class SellerFees
{
public:
	SellerFees(void)
	{
		sumOfSoldItems = 0.0;
		finalSumToGiveToSeller = 0.0;
	}

	std::vector< Fee > fees;

	double sumOfSoldItems;

	// sum of sold items minus all fees. If sum is less than zero, then it is zero.
	double finalSumToGiveToSeller;

	//! rounds the price to 5 cents ( 0.05 )
	static double RoundTo5Cents( double price );

	//! find out all the fees
	static SellerFees CalculateSellerFees( SellerCategory& sellerCategory, ItemDataArray& items );
	
	//! returns seller fee of specified type or null
	Fee* GetFeeOfType(FeeType iType);
};

} // namespace Burza

#endif
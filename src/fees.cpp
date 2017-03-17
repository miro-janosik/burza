#include "fees.h"
#include "recipees.h"
#include "translate.h"
using namespace Burza;
using namespace std;

double SellerFees::RoundTo5Cents( double price )
{
	int a = ( price + 0.001 ) * 20;
	double b = (double)a / 20.0;

	return b;
}

ItemCategory& SellerFees::GetItemCategoryByPrice( ItemCategoryArray& itemCategories, float itemPrice )
{
	if (itemCategories.empty())
	{
		throw runtime_error("GetItemCategoryByPrice: No categories");
	}
	
	for (int i=0; i<itemCategories.size(); ++i)
	{
		if ((itemCategories[i].limit_price_min <= itemPrice) && 
			(itemPrice <= itemCategories[i].limit_price_max))
			return itemCategories[i];
	}
	
	return itemCategories[0];
}

SellerFees SellerFees::CalculateSellerFees( SellerCategory& sellerCat, ItemCategoryArray& itemCategories, ItemDataArray& sellerItems )
{
	int piecesPriceOverPayPerPiece = 0;
	SellerFees output;
	
	double sumSold = 0;
	for(int i=0; i<sellerItems.size(); ++i ) {
		if ( sellerItems[ i ].status == ::sold )
			sumSold += sellerItems[ i ].price;
		
		// I will not trust set category; I will select correct one by current price.
		ItemCategory c = SellerFees::GetItemCategoryByPrice( itemCategories, sellerItems[ i ].price );
		sellerItems[ i ].category = c.id;
	}
	
	sumSold = RoundTo5Cents( sumSold );
	double sumToReturn = sumSold;
	
	// do the loop for items of each category separately, to sum up the different category fees.
	for( int catIdx = 0; catIdx < itemCategories.size(); ++catIdx )
	{
		ItemCategory itemCat = itemCategories[ catIdx ];
		double soldSumForCategory = 0.0;
		
		double feePerItem = itemCat.fee_per_item * sellerCat.multiplier_fee_item;
		double feePerItemPerc = itemCat.fee_item_perc * sellerCat.multiplier_fee_item_perc / 100.0;
		double feePerSoldItem = itemCat.fee_per_sold_item * sellerCat.multiplier_fee_sold_item;
		double feePerSoldItemPerc = itemCat.fee_sold_perc * sellerCat.multiplier_fee_sold_item_perc / 100.0;

		double feeSumPerItem = 0.0;
		double feeSumPerItemPerc = 0.0;
		double feeSumPerSoldItem = 0.0;
		double feeSumPerSoldItemPerc = 0.0;
		
		int countOnStock = 0;
		int countSold = 0;
		
		for(int i=0; i<sellerItems.size(); ++i ) 
		{
			if ( sellerItems[ i ].category != itemCat.id )
				continue;
			
			// Fees of sold items
			if ( sellerItems[ i ].status == ::sold ) {
				++countSold;
				feeSumPerSoldItem += feePerSoldItem;
				feeSumPerSoldItemPerc += feePerSoldItemPerc * sellerItems[ i ].price;
			}
			
			// Fees for items in stock
			if ( sellerItems[ i ].status != ::not_in_stock_yet ) {
				++countOnStock;
				feeSumPerItem += feePerItem;
				feeSumPerItemPerc += feeSumPerItemPerc * sellerItems[ i ].price;
			}
		}
		
		feeSumPerItem = RoundTo5Cents( feeSumPerItem );
		feeSumPerItemPerc = RoundTo5Cents( feeSumPerItemPerc );
		feeSumPerSoldItem = RoundTo5Cents( feeSumPerSoldItem );
		feeSumPerSoldItemPerc = RoundTo5Cents( feeSumPerSoldItemPerc );
		
		sumToReturn -= feeSumPerItem;
		sumToReturn -= feeSumPerItemPerc;
		sumToReturn -= feeSumPerSoldItem;
		sumToReturn -= feeSumPerSoldItemPerc;
		
		string currencySign = Centrala::Settings().currencySign;
		
		if ( feeSumPerItem > 0 )
		{
			string str = ConvFuncs::FormatStdStr(
				_( "Manipulacny poplatok %0.2f %s za kazdy kus \"%s\" (%i ks)" ), 
				itemCat.fee_per_item, currencySign.c_str(), itemCat.name.c_str(), countOnStock );
			Fee fee( str.c_str(), feeSumPerItem, ::FeePerItemOnSale );
			output.fees.push_back( fee );
		}
		
		if ( feeSumPerItemPerc > 0 )
		{
			string str = ConvFuncs::FormatStdStr(
				_( "Manipulacny poplatok %0.2f% z ceny za kazdy kus \"%s\" (%i ks)" ), 
				itemCat.fee_item_perc, itemCat.name.c_str(), countOnStock );
			Fee fee( str.c_str(), feeSumPerItemPerc, ::FeePerItemOnSale );
			output.fees.push_back( fee );
		}
		
		if ( feeSumPerSoldItem > 0 )
		{
			string str = ConvFuncs::FormatStdStr(
				_( "Poplatok %0.2f %s za kazdy predaný kus \"%s\" (%i ks)" ), 
				itemCat.fee_per_sold_item, currencySign.c_str(), itemCat.name.c_str(), countSold );
			Fee fee( str.c_str(), feeSumPerSoldItem, ::FeePerSold );
			output.fees.push_back( fee );
		}
		
		if ( feeSumPerSoldItemPerc > 0 )
		{
			string str = ConvFuncs::FormatStdStr(
				_( "Poplatok - %0.2f% z ceny za kazdy predany kus \"%s\" (%i ks)" ), 
				itemCat.fee_sold_perc, itemCat.name.c_str(), countSold );
			Fee fee( str.c_str(), feeSumPerSoldItemPerc, ::FeePerSold );
			output.fees.push_back( fee );
		}
	}

	if ( sellerCat.fee_per_sale > 0 )
	{
		string str = ConvFuncs::FormatStdStr( _( "Manipulacny poplatok za ucast - %0.2f" ), sellerCat.fee_per_sale );
		sumToReturn -= sellerCat.fee_per_sale;

		Fee fee( str.c_str(), sellerCat.fee_per_sale, ::FeePerSale );
		output.fees.push_back( fee );
	}
	
	output.sumOfSoldItems = sumSold;
	output.finalSumToGiveToSeller = sumToReturn;
	return output;
}

Fees SellerFees::GetFeeOfType(FeeType iType)
{
	std::vector<Fee> result;
	
	for( int i=0; i<fees.size(); ++i) {
		if (fees[ i ].type == iType)
		  result.push_back( fees[ i ] );
	}
	
	return result;
}

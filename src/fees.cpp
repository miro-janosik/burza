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

SellerFees SellerFees::CalculateSellerFees( SellerCategory& sellerCat, ItemDataArray& sellerItems )
{
	double sum = 0.0;
	int piecesPriceOverPayPerPiece = 0;
	SellerFees output;

	for(int i=0; i<sellerItems.size(); ++i ) {

		if ( sellerItems[ i ].status == sold ) {
			sum += sellerItems[ i ].price;
		}
		
		// if item was accepted for sale, and it is over 'sellerCat.fee_item' it will be counted.
		if ( sellerCat.fee_item != 0 ) {
			
			switch ( sellerItems[ i ].status )
			{
			//case on_stock:
			//case sold:
			//case returned:
			case ::not_in_stock_yet:
				break;
			default:
				if ( sellerItems[ i ].price > sellerCat.fee_item + 0.0001 )
				{
					++piecesPriceOverPayPerPiece;
				}
				break;
			}
		}
	}
	
	sum = SellerFees::RoundTo5Cents( sum );
	output.sumOfSoldItems = sum;
	
	double finalSum = sum;

	if ( sellerCat.fee_whole_sold_sum_percentage != 0 )
	{
		double billForSold = sum * sellerCat.fee_whole_sold_sum_percentage / 100.0;
		billForSold = SellerFees::RoundTo5Cents( billForSold );
		finalSum -= billForSold;

		Fee fee( 
			ConvFuncs::FormatStdStr( _( "- %0.0f%% poplatok z predaných vecí (%0.0f%% z %0.2f)" ), 
				sellerCat.fee_whole_sold_sum_percentage,
				sellerCat.fee_whole_sold_sum_percentage,
				sum ).c_str(),
			billForSold );
		output.fees.push_back( fee );
	}

	if ( sellerCat.fee_item != 0 )
	{
		double billForPieces = SellerFees::RoundTo5Cents( piecesPriceOverPayPerPiece * sellerCat.fee_item );
		string str = ConvFuncs::FormatStdStr(
			_( "Manipulaèný poplatok - %0.2f za každý kus drahší ako %0.2f (%i ks)" ), 
				sellerCat.fee_item, sellerCat.fee_item, piecesPriceOverPayPerPiece );
		finalSum -= billForPieces;

		Fee fee( str.c_str(), billForPieces );
		output.fees.push_back( fee );
	}

	if ( finalSum < 0 ) {
		finalSum = 0;
	}

	output.finalSumToGiveToSeller = finalSum;
	return output;
}

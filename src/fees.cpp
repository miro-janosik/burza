#include "fees.h"
#include "recipees.h"

using namespace Burza;

double SellerFees::RoundTo5Cents( double price )
{
	int a = ( price + 0.001 ) * 20;
	double b = (double)a / 20.0;

	return b;
}

SellerFees& SellerFees::CalculateSellerFees( SellerCategory& sellerCat, ItemDataArray& sellerItems )
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
			case ItemStatus::not_in_stock_yet:
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
	
	sum = Recipees::RoundTo5Cents( sum );
	
	double finalSum = sum;

	if ( sellerCat.fee_whole_sold_sum_percentage != 0 )
	{
		double billForSold = sum * sellerCat.fee_whole_sold_sum_percentage / 100.0;
		billForSold = Recipees::RoundTo5Cents( billForSold );
		
		Fee fee( 
			ConvFuncs::FormatStdStr( _( "- %0.0f%% poplatok z predaných vecí (%0.0f%% z %0.2f)" ), 
				sellerCat.fee_whole_sold_sum_percentage,
				sellerCat.fee_whole_sold_sum_percentage,
				sum ),
			billForSold );
		output.fees.push_back( fee );
	}

	if ( sellerCat.fee_item != 0 )
	{
		double billForPieces = Recipees::RoundTo5Cents( piecesPriceOverPayPerPiece * sellerCat.fee_item );
		str = ConvFuncs::FormatStdStr(
			_( "Manipulaèný poplatok - %0.2f za každý kus drahší ako %0.2f (%i ks)" ), 
				sellerCat.fee_item, sellerCat.fee_item, piecesPriceOverPayPerPiece );
		
		Fee fee( str, billForPieces );
		output.fees.push_back( fee );
	}

	return output;
}

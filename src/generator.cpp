#include <string>

#include "generator.h"
#include "dataaccess.h"
#include "log.h"
#include "errhandler.h"
#include "stringencoding.h"

using namespace Burza;
using namespace std;

void DummyDataGenerator::GenerateSellersAndItems( 
	DataAccessInterface* storage, unsigned int numberOfSellers, unsigned int maxItems)
{
	try {
		LogS( InfoMajor, "GenerateSellersAndItems: Generating %i sellers with max %i items\n",
			numberOfSellers, maxItems );

		vector<string> colors, items, attributes;
		
		colors.push_back( "biela" );
		colors.push_back( "èierna" );
		colors.push_back( "zelená" );
		colors.push_back( "modrá" );
		colors.push_back( "oranžová" );
		colors.push_back( "èervená" );
		colors.push_back( "hnedá" );
		colors.push_back( "ružová" );
		colors.push_back( "šedá" );

		items.push_back( "èiapoèka" );
		items.push_back( "chlapèenská bunda" );
		items.push_back( "nohavice" );
		items.push_back( "ponožky" );
		items.push_back( "montérky" );
		items.push_back( "šál" );
		items.push_back( "sedaèka" );
		items.push_back( "bycikel" );
		items.push_back( "zimná kombinéza" );
		items.push_back( "prechodná bundièka" );
		items.push_back( "Látková plienka swaddlebees" );
		items.push_back( "Ruène pletený svetrík + èiapka" );
		items.push_back( "Deka na hranie" );
		items.push_back( "Body" );
		items.push_back( "topánoèky" );
		items.push_back( "overal" );
		items.push_back( "vetrovka" );

		attributes.push_back( "s retiazkou" );
		attributes.push_back( "s macíkom" );
		attributes.push_back( "so sloníkom" );
		attributes.push_back( "makyta" );
		attributes.push_back( "s kapucou" );
		attributes.push_back( "nenosené" );
		
		for( int sellerIdx = 0; sellerIdx < numberOfSellers; ++sellerIdx ) {
			
			string sellerCode = storage->GenerateSellerUniqueCode( "", "AA", SELLER_ID_NONE );
			string sellerName = "Predajca";
			string note = "Poznamka - ";
			note += sellerCode;

			SellerData seller( sellerName, sellerCode, note, 0 );
			SELLER_ID sellerId = storage->AddSeller( seller );

			int itemsCount = maxItems / 4 + rand() % ( maxItems * 3 / 4 );

			LogS( InfoMajor, " GenerateSellersAndItems: Adding seller '%s' - '%s', %i items\n",
				sellerCode.c_str(), sellerName.c_str(), itemsCount );

			int number = 0;
			string name;
			double price;

			for( int itemIdx = 0; itemIdx < itemsCount; ++itemIdx ) {
				
				++number;
				if (( rand() % 20 ) == 0 )
					++number;

				price = (double)(( rand() % 100 ) + 1) / 10;

				if (( rand() % 40 ) == 0 )
					price *= 10;

				name = "";

				// maybe a color?
				if (( rand() % 2 ) == 0 ) {
					name += colors[ rand() % colors.size() ];
					name += " ";
				}

				// item name
				name += items[ rand() % items.size() ];
				name += " ";

				// color or two
				if (( rand() % 2 ) == 0 ) {
					name += colors[ rand() % colors.size() ];
					name += " ";
				}
				if (( rand() % 2 ) == 0 ) {
					name += colors[ rand() % colors.size() ];
					name += " ";
				}

				// and maybe an attribute
				if (( rand() % 4 ) == 0 ) {
					name += attributes[ rand() % attributes.size() ];
					name += " ";
				}

				// remove empty char at end
				name.erase( name.end() - 1 );

				StringEncoding::ISO8859_2ToUTF8( name );

				ItemData item( name, number, price, 0, on_stock );

				storage->AddItem( item, sellerId );
			}
		}
	}
	catch(exception& e) {
		ErrorHandling::HandleException( e );
	}
}

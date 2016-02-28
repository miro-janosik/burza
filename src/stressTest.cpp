#include <thread>
#include <unistd.h>

#include "stressTest.h"
#include "dataaccess.h"
#include "log.h"
#include "burza.h"
#include "errhandler.h"

using namespace Burza;

void StressTest::StressTest1( SELLER_ID seller_id )
{
	try 
	{
		LogS( InfoError, "Starting stress test 1\n" );
		for (int loop = 0; loop < 41; ++loop)
		{
			LogS( InfoError, "Stress test loop start\n" );
			try {
				
				if (seller_id == SELLER_ID_NONE)
					return;

				SellerData seller; 
				seller = Centrala::Storage().GetSeller( seller_id );
				Centrala::Storage().ModifySellerEnd( seller_id, seller );
				LogS( InfoError, "Modified seller\n" );
				
				ItemDataArray items = Centrala::Storage().GetSellerItems( seller_id );
				if (items.size() >= 2) 
				{
					for (int i=0; i<2; ++i)
					{
						items[i].status = ItemStatus::lost;
						Centrala::Storage().ModifyItem( items[i].id, items[i] );
					}
				}
				LogS( InfoError, "Modified some items\n" );
				
				std::vector<ITEM_ID> itemIds;
				for(int i=0; i<items.size(); ++i)
					itemIds.push_back( items[ i ].id );
					
				Centrala::Storage().StoreSaleOfItems( itemIds, "StressTest1Sale");
				
				LogS( InfoError, "Sold %i items\n", itemIds.size() );
			}
			catch(exception& e)
			{
				LogS( InfoError, "Exception: %s\n", e.what() );
			}

			LogS( InfoError, "Stress test loop end\n" );
			// delay for 15ms
			usleep( 1000 * 15 );
		}
		
		LogS( InfoError, "Finished stress test 1\n" );
	}
	catch( std::exception& e ) {
		ErrorHandling::HandleException( e );
	}	
}

void StressTest::StressTest2( SELLER_ID seller_id )
{
	try 
	{
		std::thread t1(StressTest1, seller_id), t2(StressTest1, seller_id);
		t1.join();
		t2.join();
	}
	catch( std::exception& e ) {
		ErrorHandling::HandleException( e );
	}	
}

void StressTest::StressTest1x2( SELLER_ID seller_id )
{
	try 
	{
		StressTest1( seller_id );
		StressTest1( seller_id );
	}
	catch( std::exception& e ) {
		ErrorHandling::HandleException( e );
	}	
}

void StressTest::StressTest1x2Async( SELLER_ID seller_id )
{
	try 
	{
		std::thread t1(StressTest1x2, seller_id);
	}
	catch( std::exception& e ) {
		ErrorHandling::HandleException( e );
	}	
}

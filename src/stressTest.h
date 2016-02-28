#ifndef _stress_test_h_
#define _stress_test_h_

#include "dataaccess.h"

using namespace Burza;

class StressTest
{
public:
	// multiple times in the row request updates on user and make a sale
	static void StressTest1( SELLER_ID seller_id );
	
	// call StressTest1 in two threads at the same time
	static void StressTest2( SELLER_ID seller_id );
	
	static void StressTest1x2( SELLER_ID seller_id );
	static void StressTest1x2Async( SELLER_ID seller_id );
	
};

#endif

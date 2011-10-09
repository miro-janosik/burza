#ifndef _generator_h_
#define _generator_h_

namespace Burza {
	class DataAccessInterface;
};

class DummyDataGenerator
{
public:

	static void GenerateSellersAndItems( 
		Burza::DataAccessInterface* storage, 
		unsigned int numberOfSellers, unsigned int maxItems);
};

#endif
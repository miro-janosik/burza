#include "dataaccess.h"

namespace Burza {

class DataTextfile : public DataStorage
{
  public void SaveSellerToTxtFile( SellerData& seller );
  public void LoadFromTxtFile( SELLER_ID id, SellerData& seller, ItemDataArray& items );
  
  string	DataPath; = Centrala::Instancia()->Nastavenia()->m_data_dir_path;
};

/*
Program: jeden button - Nastavenia/setup:

	SERVER CONNECTION
		SERVER_NAME/IP
		LOGIN
		PWD
	Registration key
		key (correct/incorrect)
		registered to
		host id
		
ak pri startupe nieje server, tak msg a pouzi lokal db

*/

class DataPostgress : public DataStorage
{
  
};

}

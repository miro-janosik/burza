#include "dataaccess.h"

namespace Burza {

class DataTextfile : public DataStorage
{
  public void SaveSellerToTxtFile( SellerData& seller );
  
  string	DataPath; = Centrala::Instancia()->Nastavenia()->m_data_dir_path;
};

class DataPostgress : public DataStorage
{
  
};

}

#include <string>
#include "singleton.h"

using namespace std;

class Licence : public Singleton< Licence >
{
	friend class Singleton< Licence >;

private:
	Licence(void);
	~Licence(void);
public:

	void SetLicense( string _userName, string hash );

	void ValidateLicence( void );
	bool IsDemo( void ) { return !licenceValid; }

	unsigned int GetLimitSellers( void ) { return IsDemo() ? 2 : 10000; }
	unsigned int GetLimitItems( void ) { return IsDemo() ? 4 : 10000; }

	string GetLicenseUser( void ) { return userName; }

	static std::string forMoreInformation;
private:

	//! entered licensee user name
	string userName;

	//! hash of the license
	unsigned int hash;

	//! is license valid?
	bool licenceValid;

	//! serial number of hdd
	unsigned int hddSerialNumber;

	void GetHddSerialNumber( void );

	string GetLicenceRequestText( void );

	unsigned int StringToHash( string s );
	string HashToString( unsigned int h );

	unsigned int CreateLicenseHash( void );

};

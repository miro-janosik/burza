#include <string>
#include <vector>
#ifdef _UNIX
	#include <stdexcept>
#endif

#include "licence.h"
#include "translate.h"

using namespace std;

string encCharacters = "123456789ACDEFHJKLMNPRTUVWXYZ";
const unsigned int encCharactersSize = encCharacters.size();
std::string Licence::forMoreInformation = _( "Viac informácií o licenciách sa dozviete na http://burza.mypage.sk" );

// template<> X* Singleton<X> because of unix g++ compiler
template<> Licence* Singleton<Licence>::instance = NULL;

Licence::Licence(void)
{
	licenceValid = true;
	hddSerialNumber = 0;
}

Licence::~Licence(void)
{
}

void Licence::SetLicense( string _userName, string hash )
{
	userName = _userName;
	hash = StringToHash( hash );

	ValidateLicence();
}

string Licence::GetLicenceRequestText( void )
{
	if ( hddSerialNumber == 0 )
		GetHddSerialNumber();

	return 
_( "Prosím skopírujte tento text do e-mailu a zašlite ho na adresu 'burza@mypage.sk'.\
Tam Vám vygenerujeme nový registraèný kód ktorý Vám obratom zašleme." );
}

void Licence::GetHddSerialNumber( void )
{
	hddSerialNumber = 0;
}

void Licence::ValidateLicence( void )
{
	unsigned int goodHash = CreateLicenseHash();
	licenceValid = ( goodHash == hash );
}

unsigned int Licence::CreateLicenseHash( void )
{
	unsigned int userNameHash;
	unsigned int finalHash;
	
	// vector< unsigned int > shifted;
	// for( int i=0; i<userName.size(); ++i )
	//	shifted.push_back( userName[ i ] + 3 );

	userNameHash = 0;
	unsigned int v;
	for( unsigned int i=0; i < userName.size(); ++i ) {
		v = (( userName[ i ] + 3 ) * ((i % 4) + 1 )) % 0x8000000;
		userNameHash += v;
	}
	
	if ( hddSerialNumber == 0 )
		GetHddSerialNumber();

	finalHash = userNameHash;

	unsigned int hddSerialNumberShifted = hddSerialNumber ^ 12345;
	finalHash ^= hddSerialNumberShifted;

	return finalHash;
}

unsigned int Licence::StringToHash( string s )
{
	unsigned int v = 0;

	for( unsigned int i = 0; i < s.size(); ++i ) {

		unsigned int p;
		for( p = 0; p < encCharactersSize; ++p ) {
			if ( encCharacters[ p ] == s[ i ] )
				break;
		}

		if ( p < encCharactersSize )
			v = v * encCharactersSize + p;
	}

	return v;
}

string Licence::HashToString( unsigned int h )
{
	string res;
	unsigned int v;
	unsigned int charCount = 0;

	while( h > 0 ) {

		v = h % encCharactersSize;
		h /= encCharactersSize;

		res.insert( res.begin(), encCharacters[ v ] );
		charCount++;

		if ( charCount % 4 == 0 )
			res.insert( res.begin(), ' ' );
	}

	return res;
}

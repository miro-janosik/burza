// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----
#include "stringencoding.h"

#include <iconv.h>
#include <stdexcept>
#include <exception>
#include <errno.h>
#include <vector>

#include "convfncs.h"

#ifdef _UNIX
#include <string.h>
#endif

using namespace std;

namespace Burza {

StringEncoding::StringEncoding()
{
}

StringEncoding::~StringEncoding()
{
}

void StringEncoding::UTF8toASCII( std::string& str )
{
	std::string s;
	
	for( unsigned int i=0; i<str.size(); ++i ) {
		if (( str[ i ] & 0x80 ) == 0 )
			s += str[ i ];
	}
	str = s;
}

std::vector<char> StringEncoding::OneToAnother( const char* src, int sizeinbytes, const std::string& from, const std::string& to )
{
	iconv_t cd;     // conversion descriptor
	
	const unsigned int OutBufLen = 1024;
	char outBuffer[ OutBufLen + 1 ];
	const char* inBufferPtr = src;
	char* outBufferPtr = outBuffer;
	
	size_t inleft = sizeinbytes;
	size_t outleft = OutBufLen;
	int printErrno = -1;
	
	try {
		if ( sizeinbytes > OutBufLen ) {
			sizeinbytes = OutBufLen;
			// string too long
			throw std::runtime_error( "Input string too long." );
		}
		
		if ((cd = iconv_open( to.c_str(), from.c_str() )) == (iconv_t)(-1)) 
		{
			if (errno == EINVAL) {
				throw std::runtime_error( "Cannot open converter - conversion from to is not supported by the implementation." );
			}
		  
			printErrno = errno;
			throw std::runtime_error( "Cannot open converter" );
		}
		
		// return code of iconv()
		// (char**) cast because of g++ strict warning
		int rc = iconv(cd, 
#ifdef _UNIX
			(char**)
#endif
			&inBufferPtr, 
			&inleft, &outBufferPtr, &outleft);
		if (rc == -1) {
			printErrno = errno;
			iconv_close(cd);
			throw std::runtime_error( "Error in converting characters" );
		}
		iconv_close(cd);
	}
	catch(std::runtime_error& e) {
		std::string err;

		err = ConvFuncs::FormatStdStr( 
			"StringEncoding::OneToAnother(%s => %s): %s",
			from.c_str(), to.c_str(), e.what() );
		if ( printErrno != -1 ) {
			err += ConvFuncs::FormatStdStr( "(errno:%i)", printErrno );
		}

		throw std::runtime_error( err );
	}

	*outBufferPtr = 0;
	std::vector<char> outstr;
	outstr.insert( outstr.end(), outBuffer, outBufferPtr );
	return outstr;
}

void StringEncoding::UTF8ToISO8859_2( std::string& str )
{
	std::vector<char> result;
	result = OneToAnother( str.c_str(), str.size(), "UTF-8", "ISO_8859-2" );
	std::string resultstr( result.begin(), result.end() );
	str = resultstr;
}

void StringEncoding::ISO8859_2ToUTF8( std::string& str )
{
	std::vector<char> result;
	result = OneToAnother( str.c_str(), str.size(), "ISO_8859-2", "UTF-8" );
	std::string resultstr( result.begin(), result.end() );
	str = resultstr;
}

std::string StringEncoding::ISO8859_2ToUTF8ret( std::string str )
{
	std::vector<char> result;
	result = OneToAnother( str.c_str(), str.size(), "ISO_8859-2", "UTF-8" );
	return std::string( result.begin(), result.end() );
}

std::string StringEncoding::WStringToUTF8(std::wstring &str)
{
	std::vector<char> result;
	const char* cp = (const char*)str.c_str();
	int length;
	
	std::string sourceEncoding;
#ifdef _UNIX
	// changed "UCS-2-INTERNAL" to different because on unix it is not known
	// str.size() * 2, "WCHAR_T", "UTF-8"
	// Tried to use UCS-2 but it causes nice wstring to be converted to "S\0000v\000e\000t..."
	// sourceEncoding = "UTF-32";
	sourceEncoding = "WCHAR_T";
	length = str.size() * 4;
#else
	sourceEncoding = "UCS-2-INTERNAL";
	length = str.size() * 2;
#endif
	
	result = OneToAnother( cp, length, sourceEncoding, "UTF-8" );
	std::string resultstr( result.begin(), result.end() );
	
	return resultstr;
}

std::wstring StringEncoding::StringToWString(std::string &str)
{
	return wstring();
}

void StringEncoding::UTF8RemoveAccents( std::string& str )
{
	// plans - using library ICU
	string from =	"·¡‰ƒË»ÔœÈ…ÌÕÂ≈æºÚ“Û”Ù‘‡¿¯ÿöäùç˙⁄˝›ûé";
	string to =		"aAaAcCdDeEiIlLlLnNoOoOrRrRsStTuUyYzZ";
	string::size_type pos;

	for( string::iterator it = str.begin(); it != str.end(); ++it ) {
		pos = from.find( *it );
		if ( pos != string::npos ) {
			*it = to[ pos ];
		}
	}
}

void StringEncoding::ISO8859_2ToASCII( std::string& str )
{
	std::vector<char> result;
	result = OneToAnother( str.c_str(), str.size(), "ISO_8859-2", "ASCII" );
	std::string resultstr( result.begin(), result.end() );
	str = resultstr;
}


}

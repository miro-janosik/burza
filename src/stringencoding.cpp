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

void StringEncoding::UTF8toISO8859_2( std::string& str )
{
	iconv_t cd;     // conversion descriptor
	
	const unsigned int OutBufLen = 1024;
	char inBuffer[ OutBufLen + 1 ];
	char outBuffer[ OutBufLen + 1 ];
	const char* inBufferPtr = inBuffer;
	char* outBufferPtr = outBuffer;
	
	size_t inleft = str.size();
	size_t outleft = OutBufLen;
	
	unsigned int c = str.size();
	if ( c > OutBufLen ) {
		c = OutBufLen;
		// string too long
		throw std::runtime_error( "StringUTF8toISO8859_2: Input string too long." );
	}
	memcpy( inBuffer, str.c_str(), c );
	inBuffer[ c ] = 0;
	
	if ((cd = iconv_open( "ISO_8859-2", "UTF-8")) == (iconv_t)(-1)) 
	{
		throw std::runtime_error( "StringUTF8toISO8859_2: Cannot open converter" );
	}
	
	// return code of iconv()
	int rc = iconv(cd, &inBufferPtr, &inleft, &outBufferPtr, &outleft);
	if (rc == -1) {
		std::string errstr;
		errstr = "StringUTF8toISO8859_2: Error in converting characters;";
		
		if(errno == E2BIG)
				errstr += "errno == E2BIG";
		if(errno == EILSEQ)
				errstr += "errno == EILSEQ";
		if(errno == EINVAL)
				errstr +="errno == EINVAL";

		iconv_close(cd);
		throw std::runtime_error( errstr );
	}
	iconv_close(cd);
	
	*outBufferPtr = 0;
	str = outBuffer;
}

std::string StringEncoding::WStringToUTF8(std::wstring &str)
{
	iconv_t cd;     // conversion descriptor
	const unsigned int OutBufLen = 1024;
	char inBuffer[ OutBufLen + 1 ];
	char outBuffer[ OutBufLen + 1 ];
	const char* inBufferPtr = inBuffer;
	char* outBufferPtr = outBuffer;
	
	size_t inleft = str.size() * 2;
	size_t outleft = OutBufLen;
	
	unsigned int c = inleft;
	if ( c > OutBufLen ) {
		c = OutBufLen;
		// string too long
		throw std::runtime_error( "WStringToUTF8: Input string too long." );
	}

	if ((cd = iconv_open("UTF-8", "UCS-2-INTERNAL")) == (iconv_t)(-1)) 
	{
		throw std::runtime_error(
			ConvFuncs::FormatStdStr(
			"WStringToUTF8: Cannot open converter (errno: %i)", errno ));
	}
	
	memcpy( inBuffer, str.c_str(), c );
	inBuffer[ c ] = 0;
	
	// return code of iconv()
	int rc = iconv(cd, &inBufferPtr, &inleft, &outBufferPtr, &outleft);
	if (rc == -1) {
		std::string errstr;
		errstr = ConvFuncs::FormatStdStr(
			"WStringToUTF8: Error in converting characters (errno: %i)", errno );
		
		if(errno == E2BIG)
				errstr += "errno == E2BIG";
		if(errno == EILSEQ)
				errstr += "errno == EILSEQ";
		if(errno == EINVAL)
				errstr +="errno == EINVAL";

		iconv_close(cd);
		throw std::runtime_error( errstr );
	}
	iconv_close(cd);
	
	*outBufferPtr = 0;
	std::string result = outBuffer;
	return result;
}

std::wstring StringEncoding::StringToWString(std::string &str)
{
	return wstring();
}

}

// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#ifndef BURZASTRINGENCODING_H
#define BURZASTRINGENCODING_H

#include <string>

namespace Burza {

class StringEncoding {
public:
	StringEncoding();
	~StringEncoding();
	
	// Accepts string containing characters in UTF8 encoding.
	// That means it can also contain normal ASCII characters (in range 0x00 - 0x7F)
	// and those will remain in the string.
	// Removes all non-ASCII characters from the string.
	static void		UTF8toASCII( std::string& str );
	// Converts non-ASCII characters to ISO8859_2 chars.
	static void		UTF8toISO8859_2( std::string& str );
	/*
	struct EncodingTranslatorTable {
			unsigned char UTF8[2];
		unsigned char ISO8859_2;
	} ;
	
	typedef std::vector< EncodingTranslatorTable > t_EncodingTranslatorTables;
	t_EncodingTranslatorTables	m_EncodingTranslatorTables;
	
	m_EncodingTranslatorTables.
	*/
	
	static std::string		WStringToUTF8(std::wstring &str);
	static std::wstring		StringToWString(std::string &str);
	
};

}

#endif

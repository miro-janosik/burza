// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#ifndef BURZASTRINGENCODING_H
#define BURZASTRINGENCODING_H

#include <string>
#include <vector>

#include "useicu.h"

using namespace std;

namespace Burza {

class StringEncoding {
public:
	StringEncoding();
	~StringEncoding();
	
	// Accepts string containing characters in UTF8 encoding.
	// That means it can also contain normal ASCII characters (in range 0x00 - 0x7F)
	// and those will remain in the string.
	// Removes all non-ASCII characters from the string.
	static void		UTF8toASCII( string& str );
	// Converts non-ASCII characters to ISO8859_2 chars.
	static void		UTF8ToISO8859_2( string& str );
	static void		ISO8859_2ToUTF8( string& str );
	static string	ISO8859_2ToUTF8ret( string str );
	/*
	struct EncodingTranslatorTable {
			unsigned char UTF8[2];
		unsigned char ISO8859_2;
	} ;
	
	typedef vector< EncodingTranslatorTable > t_EncodingTranslatorTables;
	t_EncodingTranslatorTables	m_EncodingTranslatorTables;
	
	m_EncodingTranslatorTables.
	*/
	
	static string WStringToUTF8(wstring &str);
	static wstring StringToWString(string &str);

	static void UTF8RemoveAccents( string& str );

	static void ISO8859_2ToASCII( string& str );
	
	// Returns true if longText contains smallerText. Does not check strings for encodings or anything.
	static bool StringContains( string& longText, string& smallerText );
	
	// Returns string that is:
	// - lowercase 
	// - accents are removed
	static string ToComparable( string& text );
	
private:
	static std::vector<char> OneToAnother( const char* src, int sizeinbytes, const std::string& from, const std::string& to );
	
	static std::string desaxUTF8(const std::string& str);
	
	static Transliterator *icuAccentsRemover;
};

}

#endif

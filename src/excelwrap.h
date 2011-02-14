// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#ifndef EXCELWRAP_H
#define EXCELWRAP_H

#include <string>
#include "libxl.h"

/**
	@author mirex,,, <mirex@haluska>
*/

using namespace std;

namespace BurzaTool {
	
class TableCell {
private:
	string	m_text;
	
	string Get( void ) { return m_text; }
	void Set( string& s ) { m_text = s; }
} ;

class TableSheet {
	
	
} ;


class ExcelFileWrapper{
public:
	ExcelFileWrapper();
	~ExcelFileWrapper();
	
	void OpenFile( string filename ) throw;
	void ReadSheet( void ) throw;
	
	
};

} ;

#endif

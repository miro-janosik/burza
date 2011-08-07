// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#ifndef EXCELWRAP_H
#define EXCELWRAP_H

#include <string>
#include <list>
#include <vector>
#include <stdexcept>
#include <exception>

#include "table.h"
#include "burza.h"

/**
	@author mirex,,, <mirex@haluska>
*/

using namespace std;
using namespace BurzaTable;
using namespace Burza;

namespace BurzaExcel {
	
class ExcelFileWrapper{
public:
	ExcelFileWrapper();
	~ExcelFileWrapper();
	
	TableOfCells	m_sheet;
	
private:
	//! reads up xls file into m_table.
	void ReadSheetIntoTable( string filename ) throw( runtime_error );
	
public:
	void ReadSeller( string filename, SellerData& seller, ItemDataArray& items ) throw( runtime_error );
	
};

} ;

#endif

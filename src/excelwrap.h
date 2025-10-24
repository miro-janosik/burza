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

using namespace std;
using namespace BurzaTable;
using namespace Burza;

namespace BurzaExcel {
	
class ExcelFileWrapper {
public:
	ExcelFileWrapper();
	~ExcelFileWrapper();
	
	TableOfCells	m_sheet;
	
public:
	void ReadSeller( string filename, SellerData& seller, ItemDataArray& items ) noexcept( false );
	
	// Converts sheet file into format XML
	// If output file already exists then conversion is not performed and path is returned.
	// Returns: empty string if conversion was not succesfull
	// full path of new filename if succesfull.
	static string LibreOfficeConvertSheetToXls(const string& pathFileName);
	
protected:
	//! reads up xls file into m_table.
	virtual void ReadSheetIntoTable( string filename ) noexcept( false ) = 0;
	bool FindCellByText( const char* text, int& row, int& column);
	
};
/*
class ExcelFileXlWrapper : public ExcelFileWrapper {
protected:
	virtual void ReadSheetIntoTable( string filename ) noexcept( false );
};
*/

class BasicExcelFileWrapper : public ExcelFileWrapper {
	virtual void ReadSheetIntoTable( string filename ) noexcept( false );
};

} // namespace

#endif

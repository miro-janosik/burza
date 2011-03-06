// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#ifndef BURZAOPENOFFICEODSWRAP_H
#define BURZAOPENOFFICEODSWRAP_H

#include <string>
//for exceptions on linux
#include<stdexcept>
#include "burza.h"
#include "table.h"

using namespace std;
using namespace Burza;
using namespace BurzaTable;

namespace BurzaOpenOffice {

class OdsFileWrapper{
public:
	OdsFileWrapper();
	~OdsFileWrapper();
	
	TableOfCells	m_sheet;
	
private:
	//! reads up xls file into m_table.
	void ReadSheetIntoTable( string filename ) throw( runtime_error );
	
public:
	Seller ReadSeller( string filename ) throw();
	
};


}

#endif

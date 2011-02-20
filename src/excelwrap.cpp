// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#include "excelwrap.h"

#include <libxl.h>
#include <exception>
#include "convfncs.h"
#include "table.h"

using namespace BurzaExcel;
using namespace BurzaTable;
using namespace Burza;
using namespace libxl;
using namespace std;

ExcelFileWrapper::ExcelFileWrapper()
{
}


ExcelFileWrapper::~ExcelFileWrapper()
{
}

void ExcelFileWrapper::ReadSheetIntoTable( string filename ) throw()
{
	Book *xls_book = xlCreateBook();
	string result_str;
	
	try {
		Sheet	*xls_sheet;
		
		if ( xls_book->load( filename.c_str() ) == false )
			throw runtime_error( "File could not be load" );
		
		if ( xls_book->sheetCount() < 1 )
			throw runtime_error( "No sheets" );
		
		xls_sheet = xls_book->getSheet( 0 );
		if ( xls_sheet== NULL )
			throw runtime_error( "Error retrieving sheet" );
		
		int rmin = xls_sheet->firstRow();
		int rmax = xls_sheet->lastRow();
		int cmin = xls_sheet->firstCol();
		int cmax = xls_sheet->lastCol();
		string str;
		
		m_sheet.Resize( cmax-cmin, rmax-rmin );
		
		for( int y=rmin; y<rmax; y++ )
			for( int x=cmin; x<cmax; x++ ) {
				
				switch( xls_sheet->cellType( y, x ) ) {
				case CELLTYPE_NUMBER:
					str = ConvFuncs::FormatStdStr( "%0.2f", xls_sheet->readNum( y, x ) );
					break;
				case CELLTYPE_STRING:
					str = xls_sheet->readStr( y, x );
					break;
				case CELLTYPE_EMPTY:
				case CELLTYPE_BOOLEAN:
				case CELLTYPE_BLANK:
				case CELLTYPE_ERROR:
					str = "";
					break;
				}
				
				m_sheet.Cell( x, y ) = str;
			}
			
		result_str = "Ok";
	}
	catch( runtime_error& e ) {
		result_str = "runtime error '";
		result_str += e.what();
		result_str += "' msg: ";
		result_str += xls_book->errorMessage();
		xls_book->release();
		throw runtime_error( result_str );
	}
	catch( ... ) {
		result_str = "unknown error";
		result_str += " msg: ";
		result_str += xls_book->errorMessage();
		xls_book->release();
		throw runtime_error( result_str );
	}
	
}

Seller ExcelFileWrapper::ReadSeller( string filename )
{
	string error_str;
	error_str = "Could not read seller from file '";
	error_str += filename;
	error_str += "' because ";
	
	try {
		ReadSheetIntoTable( filename );
	} catch( runtime_error& e ) {
		error_str += e.what();
		throw runtime_error( error_str );
	}
	
	// check constant values
	if (( m_table.Cell( 0, 0 ) != "burza" )) {
		error_str += "file does not contain special marks";
		throw runtime_error( error_str );
	}
	
	Seller seller;
	Product product;
	
	// Read seller info
	seller.m_meno = m_table.Cell( 3, 2 );
	seller.m_kod = m_table.Cell( 4, 2 );
	seller.m_poznamka = m_table.Cell( 5, 2 );
	
	// read stock
	int num;
	int row = 8;
	string s;
	
	while( true ) {
		if (( m_table.Cell( row, 1 ) == "" ) || ( m_table.Cell( row, 2 ) == "" )) {
			break;
		}
		
		product.Reset();
		
		s = m_table.Cell( row, 1 );
		num = atoi( s.c_str() );
		
		if (( num < 1 ) || ( num > 50 )) {
			error_str += ConvFuncs::FormatStdStr( "item at line %i does not have item number but instead '%s'", 
				row, s.c_str() );
			throw runtime_error( error_str );
		}
		
		s = m_table.Cell( row, 2 );
		if ( m_table.Cell( row, 4 ) != "" ) {
			s += " (";
			s += m_table.Cell( row, 4 );
			s += ")";
		}
		
		seller.PridajTovar( num, s.c_str(),  atof( m_table.Cell( row, 3 ).c_str() ), 'a', caka_na_prijatie );
		
		row++;
	} ;
	
	return seller;
}



// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#include "excelwrap.h"

#include <exception>
#include "convfncs.h"
#include "table.h"

#include <libxl.h>
#include "ExcelFormat.h"
#include "stringencoding.h"
#include "translate.h"

using namespace BurzaExcel;
using namespace BurzaTable;
using namespace Burza;
using namespace libxl;
using namespace std;
using namespace YExcel;

ExcelFileWrapper::ExcelFileWrapper()
{
}


ExcelFileWrapper::~ExcelFileWrapper()
{
}

void ExcelFileXlWrapper::ReadSheetIntoTable( string filename ) throw( runtime_error )
{
	/*
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
		
		m_sheet.Resize( rmax-rmin, cmax-cmin );
		
		for( int row=rmin; row<rmax; ++row )
			for( int col=cmin; col<cmax; ++col ) {
				
				switch( xls_sheet->cellType( row, col ) ) {
				case CELLTYPE_NUMBER:
					str = ConvFuncs::FormatStdStr( "%0.2f", xls_sheet->readNum( row, col ) );
					break;
				case CELLTYPE_STRING:
					str = xls_sheet->readStr( row, col );
					break;
				case CELLTYPE_EMPTY:
				case CELLTYPE_BOOLEAN:
				case CELLTYPE_BLANK:
				case CELLTYPE_ERROR:
					str = "";
					break;
				}
				
				m_sheet.Cell( row, col ) = str;
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
	*/
}

void BasicExcelFileWrapper::ReadSheetIntoTable( string filename ) throw( runtime_error )
{
	BasicExcel xls;
	if (! xls.Load(filename.c_str())) {
		throw runtime_error("Could not open file.");
	}

	if (xls.GetTotalWorkSheets() < 1 ) {
		throw runtime_error("XLS contains no sheets");
	}

	BasicExcelWorksheet* sheet = xls.GetWorksheet(0);
	if ( sheet== NULL )
		throw runtime_error( "Error retrieving sheet" );
	
	int rmin = 0;
	int rmax = sheet->GetTotalRows();
	int cmin = 0;
	int cmax = sheet->GetTotalCols();
	string str;
	wstring wstr;

	m_sheet.Resize( rmax-rmin, cmax-cmin );
	
	for( int row=rmin; row<rmax; ++row )
		for( int col=cmin; col<cmax; ++col ) {

			const BasicExcelCell* cell = sheet->Cell( row, col );
			
			switch( cell->Type() ) {
			case BasicExcelCell::INT:
				str = ConvFuncs::FormatStdStr( "%i", cell->GetInteger() );
				break;
			case BasicExcelCell::DOUBLE:
				str = ConvFuncs::FormatStdStr( "%0.2f", cell->GetDouble() );
				break;
			case BasicExcelCell::STRING:
				str = cell->GetString();
				StringEncoding::ISO8859_2ToUTF8( str );
				break;
			case BasicExcelCell::WSTRING:
				wstr = cell->GetWString();
				str = StringEncoding::WStringToUTF8( wstr );
				break;
			default:
				str = "";
				break;
			}
			
			m_sheet.Cell( row, col ) = str;
		}
}

bool ExcelFileWrapper::FindCellByText( const char* text, int& row, int& column)
{
	int rows = m_sheet.Rows();
	int columns = m_sheet.Columns();

	for( int r = 0; r < rows; ++r )
		for( int c = 0; c < columns; ++c ) {
			if ( m_sheet.Cell( r, c ).GetText() == text ) {
				row = r;
				column = c;
				return true;
			}
		}
	
	row = column = -1;
	return false;
}

void ExcelFileWrapper::ReadSeller( string filename, SellerData& seller, ItemDataArray& items ) throw( runtime_error )
{
	string error_str;
	error_str = _("Nepodarilo sa mi naèíta súbor s predajcom '");
	error_str += filename;
	error_str += _("' pretože ");
	
	try {
		ReadSheetIntoTable( filename );
	} catch( runtime_error& e ) {
		error_str += e.what();
		throw runtime_error( error_str );
	}
	
	// check special marks in cells. They have to be present
	bool itemsOk = true;

	int infotableRow, infotableColumn;
	int codeRow, codeColumn;
	int numberRow, numberColumn;
	int nameRow, nameColumn;
	int priceRow, priceColumn;

	itemsOk &= FindCellByText( "!infotable!", infotableRow, infotableColumn);
	//itemsOk &= FindCellByText( "!code!", codeRow, codeColumn);
	itemsOk &= FindCellByText( "!number!", numberRow, numberColumn);
	itemsOk &= FindCellByText( "!name!", nameRow, nameColumn);
	itemsOk &= FindCellByText( "!price!", priceRow, priceColumn);

	if ((numberRow != nameRow) || (numberRow != priceRow)) {
		itemsOk = false;
	}

	if (!itemsOk) {
		error_str += _("súbor neobsahuje špeciálne znaky");
		throw runtime_error( error_str );
	}
	
	// Read seller info
	try {
		seller.name = m_sheet.Cell( infotableRow + 1, infotableColumn ).GetText();
		
		string note;
		note = m_sheet.Cell( infotableRow + 2, infotableColumn ).GetText();
		note += " ";
		note += m_sheet.Cell( infotableRow + 3, infotableColumn ).GetText();
		seller.note = note;

		seller.shortcode = m_sheet.Cell( infotableRow + 3, infotableColumn + 1 ).GetText();
	}
	catch(std::out_of_range& e) {
		error_str += _("zo súboru sa nepodarilo sa naèíta údaje predajcu");
		throw runtime_error( error_str );
	}

	// read stock
	int num;
	// from numberRow skip 2 lines - info codes + item table header
	int row = numberRow + 1;
	string s;
	
	while( true ) {
		row++;

		if (( m_sheet.Cell( row, numberColumn ) == "" ) &&
			( m_sheet.Cell( row, nameColumn ) == "" ) &&
			( m_sheet.Cell( row, priceColumn ) == "" )) 
			break;

		if (( m_sheet.Cell( row, nameColumn ) == "" ) || 
			( m_sheet.Cell( row, priceColumn ) == "" )) {
			continue;
		}
		
		s = m_sheet.Cell( row, numberColumn );
		num = atoi( s.c_str() );
		
		if (( num < 1 ) || ( num > 1000 )) {
			error_str += ConvFuncs::FormatStdStr( _("Tovar na riadku %i má namiesto èísla text '%s'"), 
				row, s.c_str() );
			throw runtime_error( error_str );
		}
		
		string price = m_sheet.Cell( row, priceColumn ).GetText();
		for( int i=0; i<price.size(); ++i ) {
			if ( price[ i ] == ',' )
				price[ i ] = '.';
		}

		items.push_back(ItemData(
			m_sheet.Cell( row, nameColumn ), 
			num, 
			atof( price.c_str() ), 
			0, 
			on_stock ));
	} ;
}

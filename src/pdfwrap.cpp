// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#include "pdfwrap.h"
#include <string>
#include "apdf.h"
#include "convfncs.h"

using namespace std;
using namespace Burza;

PdfWrapper::PdfWrapper()
{
	m_default_page_width = 400;
	m_default_page_height = 600;
}

int PdfWrapper::CreatePDF( string filename )
{
	m_pdf = HPDF_New(PdfErrorHandler, NULL); 
	if (!m_pdf) {
		throw runtime_error( "PDF ERROR: cannot create pdf object.\n" );
	}
	
	try {
		HPDF_SetCompressionMode (pdf, HPDF_COMP_ALL);
		
		// use index
		HPDF_SetPageMode(pdf, HPDF_PAGE_MODE_USE_OUTLINE);
		
		// get default font
		m_font_helvetica = HPDF_GetFont (pdf, "Helvetica", NULL);
	
		// load font object
		/*
		#ifdef __WIN32__
		font_name = HPDF_LoadType1FontFromFile (pdf, "type1\\a010013l.afm",
				"type1\\a010013l.pfb");
		#else
		font_name = HPDF_LoadType1FontFromFile (pdf, "type1/a010013l.afm",
				"type1/a010013l.pfb");
		#endif
		*/
		// create outline root. - something into index
		m_outline_root = HPDF_CreateOutline (pdf, NULL, "Burza", NULL);
		HPDF_Outline_SetOpened ( m_outline_root, HPDF_TRUE);
		
	} catch( ... ) {
		throw runtime_error( "PDF ERROR: error initializing PDF.\n" );
		HPDF_Free( m_pdf );
	}
	
	m_filename = filename;
	
	m_open = true;
}

void PdfWrapper::ClosePDF( void )
{
	if ( ! m_open ) {
		throw runtime_error( "PDF not open" );
	}
	
	try {
		// save the document to a file
		HPDF_SaveToFile( m_pdf, m_filename.c_str() );
	} catch (...) {
	}
	
	// clean up
	HPDF_Free( m_pdf );
	
	m_open = false;
}

HPDF_Error_Handler	PdfWrapper::PdfErrorHandler
{
	string s;
	
	s = CConvFncs::FormatStdStr( "PDF error: error_no=%04X, detail_no=%d\n", 
		(unsigned int)error_no, (int)detail_no );
	throw runtime_error( s );
}

void PdfWrapper::NewPage( string name )
{
	m_current_page = HPDF_AddPage( m_pdf );
	
	HPDF_Outline outline;
	HPDF_Destination dst;
	HPDF_Font font2;

	HPDF_Page_SetWidth( m_current_page, m_default_page_width );
	HPDF_Page_SetHeight( m_current_page, m_default_page_height );

	outline = HPDF_CreateOutline( m_pdf, m_outline_root, name.c_str(), NULL);
	dst = HPDF_Page_CreateDestination( m_current_page );
	HPDF_Destination_SetXYZ(dst, 0, HPDF_Page_GetHeight( m_current_page ), 1);
	// HPDF_Destination_SetFitB(dst);
	HPDF_Outline_SetDestination( outline, dst );

	HPDF_Page_SetFontAndSize( m_current_page, m_font_helvetica, 15);
}

void PdfWrapper::WriteSingleText( unsigned int x, unsigned int y, string text )
{
	HPDF_Page_BeginText( m_current_page );
	//HPDF_Page_SetFontAndSize( m_current_page, font, 20 );
	HPDF_Page_MoveTextPos( m_current_page, x, y );
	HPDF_Page_ShowText( m_current_page, text );
	HPDF_Page_EndText( m_current_page  );
}

void PdfWrapper::DrawTable( unsigned int x, unsigned int y, Table& t, TableColumnSpecs& c )
{
	unsigned int i, pos, table_width, table_height, cell_height;
	
	cell_height = 15;
	
	// count column width
	table_width = 0;
	for( i=0; i<c.m_columns.size(); ++i )
		table_width += c.m_columns[ i ].m_width;
	table_height = ( 1 + t.Rows() ) * cell_height;
	
	// draw lines
	HPDF_Page_SetLineWidth( m_current_page, 0.5);
	
	// 2 = header row + bottom line
	int py;
	for (i = 0; i < 2+t.Rows(); ++i )
	{
		py = y+ i * cell_height;
		
		HPDF_Page_MoveTo( m_current_page, x, py );
		HPDF_Page_LineTo( m_current_page, x + table_width, py );
		HPDF_Page_Stroke( m_current_page );
	}
	
	// vertical lines
	// do not draw header line, if it is merged with line to right
	int px = x;
	for (i = 0; i <= t.Columns(); ++i )
	{
		if ( t.Column[ i ].m_merge_header_cell_to_right ) {
			HPDF_Page_MoveTo( m_current_page, px, y + cell_height );
		} else {
			HPDF_Page_MoveTo( m_current_page, px, y );
		}
		HPDF_Page_LineTo( m_current_page, px, y + table_height );
		HPDF_Page_Stroke( m_current_page );
		
		// last vertical line
		if ( i < t.Columns() )
			px += t.Columns[ i ].m_width;
	}
	
	py = y;
	for ( int iy = 0; iy < 2+t.Rows(); ++iy ) {
		
		px = x;
		for ( int ix = 0; ix <= t.Columns(); ++ix ) {
			
			HPDF_Page_BeginText( m_current_page );
			HPDF_Page_MoveTextPos( m_current_page, px, py );
			HPDF_Page_ShowText( m_current_page, t.Cell( iy, ix ).c_str() );
			HPDF_Page_EndText( m_current_page );
		}
	}
	
}

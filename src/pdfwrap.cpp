// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#include "pdfwrap.h"
#include <string>
#include "hpdf.h"
#include "convfncs.h"

using namespace std;
using namespace Burza;
using namespace BurzaTable;

PdfWrapper::PdfWrapper()
{
	m_default_page_width = 400;
	m_default_page_height = 600;
	m_open = false;

	m_current_font = 0;
	m_current_font_size = 0;
	m_font_height_below_line = m_font_height_above_line = 0;
}

PdfWrapper::~PdfWrapper()
{
	if ( m_open )
		ClosePDF();
}

int PdfWrapper::CreatePDF( string filename )
{
	m_pdf = HPDF_New( &PdfErrorHandler, NULL); 
	if (!m_pdf) {
		throw runtime_error( "PDF ERROR: cannot create pdf object.\n" );
	}
	
	try {
		HPDF_SetCompressionMode( m_pdf, HPDF_COMP_ALL);
		
		// use index
		HPDF_SetPageMode( m_pdf, HPDF_PAGE_MODE_USE_OUTLINE);
		
		// get default font
		m_font_helvetica = HPDF_GetFont ( m_pdf, "Helvetica", NULL);
		SetCurrentFont( m_font_helvetica, 15 );
	
		// load font object
		/*
		#ifdef __WIN32__
		font_name = HPDF_LoadType1FontFromFile (m_pdf, "type1\\a010013l.afm",
				"type1\\a010013l.pfb");
		#else
		font_name = HPDF_LoadType1FontFromFile (m_pdf, "type1/a010013l.afm",
				"type1/a010013l.pfb");
		#endif
		*/
		// create outline root. - something into index
		m_outline_root = HPDF_CreateOutline ( m_pdf, NULL, "Burza", NULL);
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

void PdfWrapper::PdfErrorHandler( 
	HPDF_STATUS   error_no,
	HPDF_STATUS   detail_no,
	void         *user_data)
{
	string s;
	
	s = ConvFuncs::FormatStdStr( "PDF error: error_no=%04X, detail_no=%d\n", 
		(unsigned int)error_no, (int)detail_no );
	throw runtime_error( s );
}

void PdfWrapper::SetCurrentFont( HPDF_Font font, unsigned int size )
{
	m_current_font = font;
	m_current_font_size = size;
	
//	m_font_height_below_line = HPDF_Font_GetXHeight( font );
//	m_font_height_above_line = HPDF_Font_GetCapHeight( font );
	m_font_height_below_line = size / 3;
	m_font_height_above_line = size * 2 / 3;
	
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

	HPDF_Page_SetFontAndSize( m_current_page, m_current_font, m_current_font_size );
}

void PdfWrapper::WriteSingleText( unsigned int x, unsigned int y, string text )
{
	HPDF_Page_BeginText( m_current_page );
	//HPDF_Page_SetFontAndSize( m_current_page, font, m_current_font_size );
	HPDF_Page_MoveTextPos( m_current_page, x, y );
	HPDF_Page_ShowText( m_current_page, text.c_str() );
	HPDF_Page_EndText( m_current_page  );
}

void PdfWrapper::DrawTable( unsigned int x, unsigned int y, TableOfCells& t, TableColumnSpecs& c )
{
	unsigned int pos, table_width, table_height, cell_height;
	
	// everything has to be drawn upside down
	
	cell_height = 18;
	const unsigned int cell_text_left_margin = 4;
	const unsigned int cell_text_top_margin = 0;
	
	// count column width
	table_width = 0;
	for( unsigned int i=0; i<c.m_columns.size(); ++i )
		table_width += c.m_columns[ i ].m_width;
	table_height = ( 1 + t.Rows() ) * cell_height;
	
	// draw lines
	HPDF_Page_SetLineWidth( m_current_page, 0.5);
	
	// 2 = header row + bottom line
	int py;
	for ( unsigned int i = 0; i < 2+t.Rows(); ++i )
	{
		py = m_default_page_height - y - i * cell_height;
		
		HPDF_Page_MoveTo( m_current_page, x, py );
		HPDF_Page_LineTo( m_current_page, x + table_width, py );
		HPDF_Page_Stroke( m_current_page );
	}
	
	// vertical lines
	// do not draw header line, if it is merged with line to right
	int px = x;
	for ( unsigned int i = 0; i <= c.Columns(); ++i )
	{
		if ( c.m_columns[ i ].m_merge_header_cell_to_right ) {
			HPDF_Page_MoveTo( m_current_page, px, m_default_page_height - y - cell_height );
		} else {
			HPDF_Page_MoveTo( m_current_page, px, m_default_page_height - y );
		}
		HPDF_Page_LineTo( m_current_page, px, m_default_page_height - y - table_height );
		HPDF_Page_Stroke( m_current_page );
		
		// last vertical line
		if ( i < c.Columns() )
			px += c.m_columns[ i ].m_width;
	}
	
	py = m_default_page_height - y - cell_text_top_margin;
	px = x + cell_text_left_margin;
	// print out header text
	for ( unsigned int ix = 0; ix < t.Columns(); ++ix ) {
		
		if ( c.m_columns[ ix ].m_name.empty() )
			continue;
		
		WriteSingleTextIntoWidth( px, py, c.m_columns[ ix ].m_width, c.m_columns[ ix ].m_name );
		
		px += c.m_columns[ ix ].m_width;
	}
	
	// print out cell text
	for ( unsigned int iy = 0; iy < t.Rows(); ++iy ) {
		py -= cell_height;
		px = x + cell_text_left_margin;
		for ( unsigned int ix = 0; ix < t.Columns(); ++ix ) {
			
			if ( t.Cell( iy, ix ).GetText().empty() )
				continue;
			
			WriteSingleTextIntoWidth( px, py, c.m_columns[ ix ].m_width, t.Cell( iy, ix ).GetText() );
			
			px += c.m_columns[ ix ].m_width;
		}
	}
	
}

void PdfWrapper::WriteSingleTextIntoWidth( unsigned int x, unsigned int y,
	unsigned int width, string text )
{
	HPDF_UINT	len;
	
	// measure how many characters will fit
	len = HPDF_Font_MeasureText ( m_current_font,
		(const unsigned char*) text.c_str(), text.size(), width, m_current_font_size,
		0, 0, HPDF_FALSE, NULL );
	if ( len == 0 )
		throw runtime_error( "WriteSingleTextIntoWidth: HPDF_Font_MeasureText failed." );
	
	text.erase( len );
	
	HPDF_Page_BeginText( m_current_page );
	HPDF_Page_MoveTextPos( m_current_page, x, y - m_font_height_below_line - m_font_height_above_line );
	HPDF_Page_ShowText( m_current_page, text.c_str() );
	HPDF_Page_EndText( m_current_page );
}

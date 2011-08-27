// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#include "pdfwrap.h"
#include <string>
#include "hpdf.h"
#include "convfncs.h"
#include "stringencoding.h"

using namespace std;
using namespace Burza;
using namespace BurzaTable;

void PdfWrapperBase::PosToPercent( int posx, int posy, float& percentx, float& percenty )
{
	percentx = PosToPercentX( posx );
	percenty = PosToPercentY( posy ); 
}

void PdfWrapperBase::PercentToPos( float percentx, float percenty, int& posx, int& posy )
{
	posx = PercentToPosX( percentx );
	posy = PercentToPosY( percenty );
}

HPDFWrapper::HPDFWrapper()
{
	m_default_page_width = 400;
	m_default_page_height = 600;
	m_open = false;

	m_current_font = 0;
	m_current_font_size = 10;
	m_font_height_below_line = m_font_height_above_line = 0;
	m_current_page = NULL;
}

HPDFWrapper::~HPDFWrapper()
{
	if ( m_open )
		ClosePDF();
}

int HPDFWrapper::CreatePDF( string filename )
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
		SetFontFace( "Helvetica" );
		SetFontSize( 2 );

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

void HPDFWrapper::ClosePDF( void )
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
	m_current_page = NULL;
}

void HPDFWrapper::PdfErrorHandler( 
	HPDF_STATUS   error_no,
	HPDF_STATUS   detail_no,
	void         *user_data)
{
	string s;
	
	s = ConvFuncs::FormatStdStr( "PDF error: error_no=%04X, detail_no=%d\n", 
		(unsigned int)error_no, (int)detail_no );
	throw runtime_error( s );
}

void HPDFWrapper::SetFontFace( string name )
{
	HPDF_Font font = HPDF_GetFont( m_pdf, name.c_str(), "ISO8859-2" );
	SetCurrentFont( font, m_current_font_size );
}

void HPDFWrapper::SetFontSize( float size )
{
	SetCurrentFont( m_current_font, size );
}

void HPDFWrapper::SetCurrentFont( HPDF_Font font, float size )
{
	int sy = PercentToHeight( size );

	m_current_font = font;
	m_current_font_size = sy;
	
//	m_font_height_below_line = HPDF_Font_GetXHeight( font );
//	m_font_height_above_line = HPDF_Font_GetCapHeight( font );
	m_font_height_above_line = sy * 2 / 3;

	// set font on the actual page.
	if (m_current_page != NULL) {
		HPDF_Page_SetFontAndSize( m_current_page, m_current_font, m_current_font_size );
	}
}

float HPDFWrapper::GetFontHeight( void )
{
	return HeightToPercent( m_current_font_size + m_font_height_below_line );
}

float HPDFWrapper::GetFontHeightBase( void )
{
	return HeightToPercent( m_current_font_size );
}

void HPDFWrapper::NewPage( string name )
{
	m_current_page = HPDF_AddPage( m_pdf );
	
	HPDF_Outline outline;
	HPDF_Destination dst;
	HPDF_Font font2;

	HPDF_Page_SetWidth( m_current_page, m_default_page_width );
	HPDF_Page_SetHeight( m_current_page, m_default_page_height );

	string str = name;
	StringEncoding::UTF8toISO8859_2( str );
	outline = HPDF_CreateOutline( m_pdf, m_outline_root, str.c_str(), NULL);
	dst = HPDF_Page_CreateDestination( m_current_page );
	HPDF_Destination_SetXYZ(dst, 0, HPDF_Page_GetHeight( m_current_page ), 1);
	// HPDF_Destination_SetFitB(dst);
	HPDF_Outline_SetDestination( outline, dst );

	// set actual font on current page.
	HPDF_Page_SetFontAndSize( m_current_page, m_current_font, m_current_font_size );
}

int HPDFWrapper::PercentToPosX( float percentx )
{
	return percentx * m_default_page_width / 100;
}

int HPDFWrapper::PercentToPosY( float percenty )
{
	return m_default_page_height - ( percenty * m_default_page_height / 100 );
}

int HPDFWrapper::PercentToHeight( float percentHeight )
{
	return ( percentHeight * m_default_page_height / 100 );
}

float HPDFWrapper::PosToPercentX( int posx )
{
	return (float)posx / m_default_page_width * 100;
}

float HPDFWrapper::PosToPercentY( int posy )
{
	return (m_default_page_height - (float)posy) / m_default_page_height * 100; 
}

float HPDFWrapper::HeightToPercent( int height )
{
	return (float)height / m_default_page_height * 100; 
}

void HPDFWrapper::DrawTable( float fx, float fy, TableStyle& style, TableOfCells& t, TableColumnSpecs& c )
{
	int posxPx, posyPx, rowHeightPx;
	PercentToPos( fx, fy, posxPx, posyPx );
	rowHeightPx = PercentToHeight( style.rowHeight );
	
	unsigned int pos, table_width, tableHeightPx;
	
	// everything has to be drawn upside down
	
	const unsigned int cellTextLeftMarginPerc = GetFontHeight() / 6;
	const unsigned int cellTextTopMarginPerc = GetFontHeight() / 6;
	
	// count column width
	table_width = 0;
	for( unsigned int i=0; i<c.m_columns.size(); ++i ) {
		int w;
		w = PercentToPosX( c.m_columns[ i ].m_width );
		table_width += w;
	}
	tableHeightPx = PercentToHeight( ( 1 + t.Rows() ) * style.rowHeight );
	
	// draw lines
	HPDF_Page_SetLineWidth( m_current_page, 0.5);
	
	// 2 = header row + bottom line
	float py = fy;
	for ( unsigned int i = 0; i < 2+t.Rows(); ++i )
	{
		int y = PercentToPosY( py );
		HPDF_Page_MoveTo( m_current_page, posxPx, y );
		HPDF_Page_LineTo( m_current_page, posxPx + table_width, y );
		HPDF_Page_Stroke( m_current_page );
		
		py += style.rowHeight;
	}
	
	// vertical lines
	// do not draw header line, if it is merged with line to right
	float px = fx;
	for ( unsigned int i = 0; i <= c.Columns(); ++i )
	{
		int x = PercentToPosX( px );
		
		if (( i < c.Columns() ) &&
			( c.m_columns[ i ].m_merge_header_cell_to_right )) {
			HPDF_Page_MoveTo( m_current_page, x, posyPx - rowHeightPx );
		} else {
			HPDF_Page_MoveTo( m_current_page, x, posyPx );
		}
		HPDF_Page_LineTo( m_current_page, x, posyPx - tableHeightPx );
		HPDF_Page_Stroke( m_current_page );
		
		// last vertical line
		if ( i < c.Columns() ) {
			px += c.m_columns[ i ].m_width;
		}
	}
	
	float tx, ty;
	tx = fx + cellTextLeftMarginPerc;
	ty = fy + cellTextTopMarginPerc;
	
	// print out header text
	for ( unsigned int ix = 0; ix < c.m_columns.size(); ++ix ) {
		
		if ( c.m_columns[ ix ].m_name.empty() )
			continue;
		
		WriteSingleTextIntoWidth( tx, ty, c.m_columns[ ix ].m_width, c.m_columns[ ix ].m_name );
		
		tx += c.m_columns[ ix ].m_width;
	}
	
	// print out cell text
	for ( unsigned int iy = 0; iy < t.Rows(); ++iy ) {
		
		tx = fx + cellTextLeftMarginPerc;
		ty += style.rowHeight;
		
		for ( unsigned int ix = 0; ix < t.Columns(); ++ix ) {
			
			if ( t.Cell( iy, ix ).GetText().empty() )
				continue;
			
			WriteSingleTextIntoWidth( tx, ty, c.m_columns[ ix ].m_width, t.Cell( iy, ix ).GetText() );
			
			tx += c.m_columns[ ix ].m_width;
		}
	}
}

void HPDFWrapper::WriteSingleText( float x, float y, string text )
{
	int px, py;
	PercentToPos( x, y, px, py );

	string str = text;
	StringEncoding::UTF8toISO8859_2( str );
	
	HPDF_Page_BeginText( m_current_page );
	HPDF_Page_MoveTextPos( m_current_page, px, py );
	HPDF_Page_ShowText( m_current_page, str.c_str() );
	HPDF_Page_EndText( m_current_page  );
}

void HPDFWrapper::WriteSingleTextIntoWidth( float x, float y, float width, string text )
{
	int px, py;
	PercentToPos( x, y, px, py );

	string str = text;
	StringEncoding::UTF8toISO8859_2( str );
	
	int width_pixels = PercentToPosX( width );
	
	HPDF_UINT	len;
	
	// measure how many characters will fit
	len = HPDF_Font_MeasureText ( m_current_font,
		(const unsigned char*) str.c_str(), str.size(), width_pixels, m_current_font_size,
		0, 0, HPDF_FALSE, NULL );
	if ( len == 0 )
		throw runtime_error( "WriteSingleTextIntoWidth: HPDF_Font_MeasureText failed." );
	
	str.erase( len );
	
	HPDF_Page_BeginText( m_current_page );
	HPDF_Page_MoveTextPos( m_current_page, px, py - m_current_font_size );
	HPDF_Page_ShowText( m_current_page, str.c_str() );
	HPDF_Page_EndText( m_current_page );
}

void HPDFWrapper::FontPrintTest( void )
{
	int miny, maxy;
	miny = 20;
	maxy = miny + PercentToHeight( GetFontHeight() );
	
	HPDF_Page_MoveTo( m_current_page, 20, miny );
	HPDF_Page_LineTo( m_current_page, 100, miny );
	HPDF_Page_LineTo( m_current_page, 100, maxy );
	HPDF_Page_LineTo( m_current_page, 20, maxy );
	HPDF_Page_LineTo( m_current_page, 20, miny );
	HPDF_Page_Stroke( m_current_page );
	
	float ptx, pty, pwx;
	PosToPercent( 80, maxy, pwx, pty );
	PosToPercent( 20, maxy, ptx, pty );
	
	WriteSingleTextIntoWidth( ptx, pty, pwx, "AČgjhýĹq" );
	float fh = GetFontHeight();

	HPDF_Page_MoveTo( m_current_page, 30, PercentToPosY( pty ) );
	HPDF_Page_LineTo( m_current_page, 110, PercentToPosY( pty ) );
	HPDF_Page_MoveTo( m_current_page, 40, PercentToPosY( pty + fh ) );
	HPDF_Page_LineTo( m_current_page, 120, PercentToPosY( pty + fh ) );
	HPDF_Page_Stroke( m_current_page );
}

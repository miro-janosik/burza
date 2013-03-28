// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#ifndef BURZA_PDFWRAP_H
#define BURZA_PDFWRAP_H

#include <string>

#ifdef _WIN32
  #define HPDF_DLL
#else
  
#endif

#include "hpdf.h"
#include "table.h"

using namespace std;
using namespace BurzaTable;

namespace Burza {

class PdfWrapperBase {
public:
	PdfWrapperBase() {}
	
	virtual int CreatePDF( string filename ) = 0;
	virtual void ClosePDF( void ) = 0;
	
	virtual void NewPage( string name ) = 0;

	//! font name, for example "Helvetica"
	virtual void SetFontFace( string name ) = 0;
	//! font height in percentage of page (0 - 100%)
	virtual void SetFontSize( float size ) = 0;
	
	//! x, y : left top position on the page in percentage, 0 - 100%
	//! width : size x in page percentage, 0 - 100%
	//! text : text in unicode
	virtual void WriteSingleText( float x, float y, string text ) = 0;

	//! returns font height in percentage of page (0 - 100%)
	//! returns height of the font including descent and wovel space
	virtual float GetFontHeight( void ) = 0;
	//! returns height of the font from base line to top including wovel space
	virtual float GetFontHeightBase( void ) = 0;

	struct TableStyle {
		TableStyle();
		void Reset( void );

		//! rowHeight : height of the row in percentage of page height.
		float	rowHeight;

		//! rowHeight : height of the row in percentage of page height.
		float	headerRowHeight;

		//! size of the font
		float	fontSize;
	};
	
	//! x, y : left top position on the page in percentage, 0 - 100%
	//! column width is defined in TableColumnSpecs.
	//! text of headers and cells : text in unicode
	virtual void DrawTable( float x, float y, TableStyle& style, TableOfCells& t, TableColumnSpecs& c, int firstRow = -1, int lastRow = -1 ) = 0;
	
	//! converts position in page size percentage to position of the pdf engine
	virtual void PercentToPos( float percentx, float percenty, int& posx, int& posy );
	virtual int PercentToPosX( float percentx ) = 0;
	virtual int PercentToPosY( float percenty ) = 0;
	virtual int PercentToHeight( float percentHeight ) = 0;
	
	//! converts position from position of the engine to percentage position
	virtual void PosToPercent( int posx, int posy, float& percentx, float& percenty );
	virtual float PosToPercentX( int posx ) = 0;
	virtual float PosToPercentY( int posy ) = 0;
	virtual float HeightToPercent( int height ) = 0;

	virtual string GetCurrentPageTitle( void ) = 0;
};

class HPDFWrapper : public PdfWrapperBase {
public:
	HPDFWrapper();
	~HPDFWrapper();
	int CreatePDF( string filename );
	void ClosePDF( void );
  
	void NewPage( string name );
	
	void SetFontFace( string name );
	void SetFontSize( float size );
	
	float GetFontHeight( void );
	float GetFontHeightBase( void );
	
	void WriteSingleText( float x, float y, string text );
	void WriteSingleTextIntoWidth( float x, float y, float width, string text );
	
	void DrawTable( float x, float y, TableStyle& style, TableOfCells& t, TableColumnSpecs& c, int firstRow = -1, int lastRow = -1 );

	//! converts position in page size percentage to position of the engine
	virtual int PercentToPosX( float percentx );
	virtual int PercentToPosY( float percenty );
	virtual int PercentToHeight( float percentHeight );
	
	//! converts position from position of the engine to percentage position
	virtual float PosToPercentX( int posx );
	virtual float PosToPercentY( int posy );
	virtual float HeightToPercent( int height );
	
	virtual string GetCurrentPageTitle( void );

	static void ConvertUTF8ToISO8859_2orErrorString( std::string& str );
	static void ConvertUTF8ToIAsciiOrErrorString( std::string& str );

private:

	//! size : height of the font in percentage of page height
	void SetCurrentFont( HPDF_Font font, float height );
  
	HPDF_Doc m_pdf;
	
	string		m_filename;
	bool		m_open;
	HPDF_Font	m_font_helvetica;
	
	HPDF_Font	m_current_font;
	
	// height of the font. Height from bottom line to the top of the character including wovels.
	unsigned int m_current_font_size;
	// number of pixels below the line. From bottom line to bottom of g,j,q.
	unsigned int m_font_height_below_line;
	// number of pixels from top of characters to top of wovels
	unsigned int m_font_height_above_line;
	
	HPDF_Page	m_current_page;
	HPDF_Outline	m_outline_root;
	unsigned int	m_default_page_width, m_default_page_height;

//	HPDF_Error_Handler	PdfErrorHandler;
	static void PdfErrorHandler( 
		HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data);
	
	// function to test out size of font and its position on the page.
	void FontPrintTest( void );

	string		currentPageTitle;
} ;

}; // namespace

#endif
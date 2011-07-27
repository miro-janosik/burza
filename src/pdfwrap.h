// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#ifndef BURZA_PDFWRAP_H
#define BURZA_PDFWRAP_H

#include <string>
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
	virtual void WriteSingleText( float x, float y, string text ) = 0;
	virtual void DrawTable( float x, float y, TableOfCells& t, TableColumnSpecs& c ) = 0;
	
	//! converts position in page size percentage to position of the engine
	virtual void PercentToPos( float percentx, float percenty, int& posx, int& posy ) = 0;
	
	//! converts position from position of the engine to percentage position
	virtual void PosToPercent( int posx, int posy, float& percentx, float& percenty ) = 0;
};

class HPDFWrapper : public PdfWrapperBase {
public:
	HPDFWrapper();
	~HPDFWrapper();
	int CreatePDF( string filename );
	void ClosePDF( void );
	
	void SetCurrentFont( HPDF_Font font, float size );
	void NewPage( string name );
	void WriteSingleText( float x, float y, string text );
	void WriteSingleTextIntoWidth( float x, float y, float width, string text );
	
	void DrawTable( float x, float y, TableOfCells& t, TableColumnSpecs& c );

	//! converts position in page size percentage to position of the engine
	virtual void PercentToPos( float percentx, float percenty, int& posx, int& posy );
	
	//! converts position from position of the engine to percentage position
	virtual void PosToPercent( int posx, int posy, float& percentx, float& percenty );
	
private:
	HPDF_Doc m_pdf;
	
	string		m_filename;
	bool			m_open;
	HPDF_Font	m_font_helvetica;
	
	HPDF_Font	m_current_font;
	unsigned int m_current_font_size;
	unsigned int m_font_height_below_line, m_font_height_above_line;
	
	HPDF_Page	m_current_page;
	HPDF_Outline	m_outline_root;
	unsigned int	m_default_page_width, m_default_page_height;

//	HPDF_Error_Handler	PdfErrorHandler;
	static void PdfErrorHandler( 
		HPDF_STATUS error_no, HPDF_STATUS detail_no, void *user_data);
	
} ;

}; // namespace

#endif
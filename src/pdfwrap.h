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
	virtual void WriteSingleText( unsigned int x, unsigned int y, string text ) = 0;
	virtual void DrawTable( unsigned int x, unsigned int y, TableOfCells& t, TableColumnSpecs& c ) = 0;
	
};

class PdfWrapper : public PdfWrapperBase {
public:
	PdfWrapper();
	~PdfWrapper();
	int CreatePDF( string filename );
	void ClosePDF( void );
	
	void SetCurrentFont( HPDF_Font font, unsigned int size );
	void NewPage( string name );
	void WriteSingleText( unsigned int x, unsigned int y, string text );
	void WriteSingleTextIntoWidth( unsigned int x, unsigned int y, unsigned int width, string text );
	
	void DrawTable( unsigned int x, unsigned int y, TableOfCells& t, TableColumnSpecs& c );
	
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
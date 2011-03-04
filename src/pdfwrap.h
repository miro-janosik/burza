// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#ifndef BURZA_PDFWRAP_H
#define BURZA_PDFWRAP_H

#include <string>
#include "apdf.h"

using namespace std;

namespace Burza {

class PdfWrapper {
public:
	void CreatePDF( string filename );
	void ClosePDF( void );
	
	void NewPage( void );
	
	void WriteText( unsigned int x, unsigned int y, string text );
	
	void DrawTable( unsigned int x, unsigned int y, Table& t, TableColumnSpecs& c );
	
private:
	HPDF_Doc m_pdf;
	
	string		m_filename;
	bool			m_open;
	HDPF_Font	m_font_helvetica;
	HPDF_Page	m_current_page;
	HPDF_Outline	m_outline_root;
	unsigned int	m_default_page_width, m_default_page_height;

	static HPDF_Error_Handler	PdfErrorHandler;
	
} ;

}; // namespace

#endif
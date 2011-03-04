// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#include <string>
#include <vector>

#include <gtkmm.h>
#include <glib/gthread.h>
#include <gtkmm.h>
#include <gtkmm/main.h>

#include <assert.h>

#include "libglademm.h"
#include "convfile.h"
#include "convfncs.h"
#include "burza.h"
#include "bdialogs.h"

using namespace Burza;

Centrala* Centrala::m_instance = NULL;

//int _tmain(int argc, _TCHAR* argv[])
int main2( int argc, char **argv )
{
	Centrala		*pcentrala = Centrala::Instancia();
	
	ConvFuncs::SingletonInstanceCreate();
	
	Gtk::Main	kit( argc, argv );
	
	// do not use local settings, use international. I want '.' as decimal delimiter.
	setlocale(LC_ALL,"C");
	
	PdfWrapper	pdf;
	TableOfCells	table;
	TableColumnSpecs columns;
	
	table.Resize( 3, 3 );
	table.Cell( 0, 0 ) = "1.";
	table.Cell( 1, 0 ) = "2.";
	table.Cell( 2, 0 ) = "3.";
	
	table.Cell( 0, 1 ) = "abc";
	table.Cell( 1, 1 ) = "def";
	table.Cell( 2, 1 ) = "ghi";
	
	table.Cell( 0, 2 ) = "jkl";
	table.Cell( 1, 2 ) = "";
	table.Cell( 2, 2 ) = "mno";
	
	columns.AddColumn( "First", 120 );
	columns.AddColumn( "Second", 90 );
	columns.AddColumn( "Third!", 100 );
	
	pdf.CreatePDF( "/home/mirex/tmp/a.pdf" );
	pdf.NewPage();
	pdf.DrawTable( 50, 50, table, columns );
	pdf.ClosePDF();
	
	// ============
	
	pcentrala->Dialogy()->NahrajDialogy();
	
	pcentrala->Register()->NacitajVsetkychPredajcov();
	pcentrala->Dialogy()->VyplnPredajcovDoKomb();
	pcentrala->Dialogy()->RegisterZobrazPredajcuCezIndex( 0 );

	//Gtk::Main::run( * pcentrala->Dialogy()->m_pWndRegister );
	Gtk::Main::run();

	// setup update timer
	// m_timer_connection = Glib::signal_timeout().connect( sigc::ptr_fun( &on_timer ), 100 );

	// --=============================--
	
	ConvFuncs::SingletonInstanceDelete();
	
	Centrala::Zmaz();
	
	return 0;
}


// ==============================================
// ==============================================
// ==============================================

Product::Product()
{
	Reset();
}

void Product::Reset( void )
{
	m_cislo = -1;
	m_nazov = "?";
	m_cena = 0.0;
	m_kategoria = ' ';
	m_stav = caka_na_prijatie;
}

Seller::Seller()
{
	m_kod = "";
	m_meno = "?";
	m_poznamka = "";
}

void Seller::PridajTovar( int cislo, const char* nazov, double cena, char kategoria, e_product_status stav )
{
	Product	t;

	t.m_cena = cena;
	t.m_cislo = cislo;
	t.m_kategoria = kategoria;
	t.m_nazov = nazov;
	t.m_stav = stav;

	m_products.push_back( t );
}

void Seller::SaveToTxtFile( CConvFile& file )
{
	file.Write( m_kod.c_str() );
	file.Write( "|" );
	file.Write( m_meno.c_str() );
	file.Write( "|" );
	file.Write( m_poznamka.c_str() );
	file.Write( "\n" );
	
	string s;
	
	for( unsigned int i=0; i<m_products.size(); i++ ) {
		s = m_products[ i ].m_nazov.substr( 0, 40 );
		file.Print( "%i|%40s|%0.1f|%c|%c\n",
			m_products[ i ].m_cislo, s.c_str(),
			m_products[ i ].m_cena, m_products[ i ].m_kategoria,
			Centrala::Instancia()->Register()->Stav2Char( m_products[ i ].m_stav ) );
	}
}

void Seller::LoadFromTxtFile( CConvFile& file )
{
	static const int linebuffer_len = 512;
	char linebuffer[ linebuffer_len ];
	char namebuffer[ linebuffer_len ];
	char delimiters[] = "|\r\n";
	unsigned short strpos;
	
	const char* cp;
	Product		t;
	
	file.gets( linebuffer, linebuffer_len );
	m_kod = ConvFuncsInstance->StrItem( linebuffer, 0, delimiters );
	m_meno = ConvFuncsInstance->StrItem( linebuffer, 1, delimiters );
	m_poznamka = ConvFuncsInstance->StrItem( linebuffer, 2, delimiters );
	
	m_products.clear();
	
	while( 1 ) {
		if ( file.Eof() )
			break;
		if ( file.gets( linebuffer, linebuffer_len ) == 0 )
			break;
		
		cp = ConvFuncsInstance->StrItem( linebuffer, 0, delimiters, &strpos );
		// skip empty lines
		if ( cp == NULL )
			continue;
		t.m_cislo = atoi( cp );
		
		if ( strpos + 40 > strlen( linebuffer ))
			continue;
		
		strncpy( namebuffer, &linebuffer[ strpos ], 40 );
		t.m_nazov = namebuffer;
		cp = & linebuffer[ strpos + 40 ];
		
		t.m_cena = atof( ConvFuncsInstance->StrItem( cp, 0, delimiters ) );
		t.m_kategoria = * ConvFuncsInstance->StrItem( cp, 1, delimiters );
		t.m_stav = Centrala::Instancia()->Register()->Char2Stav( ( * ConvFuncsInstance->StrItem( cp, 2, delimiters ) ) );
		
		m_products.push_back( t );
	} ;
}

// ==============================================
// ==============================================
// ==============================================

void BurzaRegister::NacitajVsetkychPredajcov( void )
{
	std::string		predajca_dir, predajca_pat, predajca_fname;
			
	predajca_dir = ConvFuncsInstance->GetPathFromPathFilename(
		ConvFuncsInstance->GetExecutableFilePathName() );
	predajca_dir += "data/";
	
	predajca_pat = predajca_dir;
	predajca_pat += "*.txt";
	
	bool found;
	found = ConvFuncsInstance->Find_FirstFile( predajca_pat.c_str() );
	while( found ) {
		if ( ConvFuncsInstance->Find_CurrentIsDir() == false ) {
			predajca_fname = predajca_dir;
			predajca_fname += ConvFuncsInstance->Find_CurrentFileName();
			
			CConvFile	f;
			f.OpenFile( predajca_fname.c_str(), CConvFile::fm_text, CConvFile::fm_read );
			
			if ( f.IsOpened() == 1 ) {
				Seller	novy;
				novy.NacitajZTxtSuboru( f );
				m_predajcovia.push_back( novy );
				
				f.Close();
			}
		}
		
		found = ConvFuncsInstance->Find_NextFile();
	}
	
}

void BurzaRegister::PridajNovehoPredajcu( const char* kod, const char* meno, const char* poznamka )
{
	Seller	p;

	p.m_kod = kod;
	p.m_meno = meno;
	p.m_poznamka = poznamka;
	
	m_predajcovia.push_back( p );
}

BurzaRegister::t_predajcovia::iterator BurzaRegister::NajdiPredajcuPodlaKodu( const char* kod )
{
	std::vector< Seller >::iterator	it;
	Seller	*p;
	
	for( it = m_predajcovia.begin(); it != m_predajcovia.end(); it++ ) {
	
		p = &( *it );
	
		if ( p->m_kod == kod )
			return it;
	}
	
	return it;
}

// ==============================================
// ==============================================
// ==============================================

char BurzaRegister::Stav2Char( e_product_status stav )
{
	if (( stav < 0 ) || ( stav >= pocet_stavov ))
		return '?';
	
	return Centrala::Instancia()->Nastavenia()->m_znak_stavu_v_subore[ stav ];
}

const char* BurzaRegister::Stav2String( e_product_status stav )
{
	if (( stav < 0 ) || ( stav >= pocet_stavov ))
		return "?";
	
	return Centrala::Instancia()->Nastavenia()->m_string_stavu[ stav ].c_str();
}

e_product_status BurzaRegister::Char2Stav( char c )
{
	int i;
	NastaveniaProgramu	*n = & Centrala::Instancia()->m_nastavenia;
	
	for( i=0; i < pocet_stavov; ++i )
		if ( n->m_znak_stavu_v_subore[ i ] == c )
			return (Burza::e_product_status) i;
	
	return nedefinovane;
}

// ==============================================
// ==============================================

NastaveniaProgramu::NastaveniaProgramu()
{
	m_znak_stavu_v_subore[ sklad ] = 's';
	m_znak_stavu_v_subore[ predane ] = 'p';
	m_znak_stavu_v_subore[ vratene ] = 'v';
	m_znak_stavu_v_subore[ dar ] = 'd';
	m_znak_stavu_v_subore[ dohladat ] = 'n';
	
	m_string_stavu[ sklad ] = "Skladom";
	m_string_stavu[ predane ] = "Predane";
	m_string_stavu[ vratene ] = "Vratene";
	m_string_stavu[ dar ] = "Dar";
	m_string_stavu[ dohladat ] = "Dohladat";
	
}

// ==============================================
// ==============================================

Centrala::Centrala()
{
	mp_dialogs = new BurzaRegister();
}

Centrala* Centrala::Instancia( void )
{
	if ( m_instance == NULL ) {
		m_instance = new Centrala;
	}
	
	return m_instance;
}

void Centrala::Zmaz( void )
{
	if ( m_instance ) {
		delete mp_dialogs;
		
		delete m_instance;
		m_instance = NULL;
	}
}

BurzaDialogs* Centrala::Dialogy( void )
{
	return mp_dialogs;
}

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
#include "stringencoding.h"

// not required here, only for test
#include "pdfwrap.h"
#include "table.h"

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

char Seller::file_field_delimiters[] = "|*\r\n";

Seller::Seller()
{
	m_kod = "";
	m_meno = "?";
	m_poznamka = "";
	m_unique_id = Centrala::Instancia()->Register()->GenerateSellerUniqueId();
}

void Seller::PridajTovar( int cislo, const char* nazov, double cena, char kategoria, e_product_status stav )
{
	Product	t;

	t.m_cena = cena;
	t.m_cislo = cislo;
	t.m_kategoria = kategoria;
	t.m_nazov = nazov;
	t.m_stav = stav;
	
	// generate 'cislo'
	if ( t.m_cislo == -1 ) {
		
		int max = 0;
		for( int i=0; i<m_products.size(); ++i ) {
			if ( max < m_products[ i ].m_cislo )
				max = m_products[ i ].m_cislo;
		}
		
		t.m_cislo = max + 1;
	}
	
	m_products.push_back( t );
}

int Seller::VerifikujCisloTovaru( Product& p, int newNumber )
{
	int max = 0;
	bool valid = true;
	for( int i=0; i<m_products.size(); ++i ) {
		
		if ( max < m_products[ i ].m_cislo )
			max = m_products[ i ].m_cislo;
		
		if (( newNumber == m_products[ i ].m_cislo ) && ( &m_products[ i ] != &p )) {
			valid = false;
		}
	}
	
	if ( newNumber == -1 )
		valid = false;
	
	if ( ! valid ) {
		newNumber = max + 1;
	}
	
	p.m_cislo = newNumber;
	return newNumber;
}

void Seller::ZmazTovar( int cislo )
{
	for( int i=0; i<m_products.size(); ++i ) {
		if ( cislo == m_products[ i ].m_cislo ) {
			m_products.erase( m_products.begin() + i );
			return;
		}
	}
	
	throw runtime_error( "Seller::ZmazTovar : invalid number." );
}

int Seller::NajdiTovarIndex( int cislo )
{
	for( unsigned int i=0; i<m_products.size(); ++i ) {
		if ( cislo == m_products[ i ].m_cislo ) {
			return i;
		}
	}
	
	return -1;
}

void Seller::ReplaceCharactersInString( std::string& str, const char* from, char to )
{
	unsigned int len = strlen( from );
	for( unsigned int i=0; i<len; i++ ) {
		std::replace( str.begin(), str.end(), from[ i ], to );
	}
}

void Seller::SaveToTxtFile( CConvFile& file )
{
	ReplaceCharactersInString( m_meno, file_field_delimiters, '_' );
	ReplaceCharactersInString( m_poznamka, file_field_delimiters, '_' );
	
	file.Print( "%i", m_unique_id );
	file.Write( "*" );
	file.Write( m_kod.c_str() );
	file.Write( "*" );
	file.Write( m_meno.c_str() );
	file.Write( "*" );
	file.Write( m_poznamka.c_str() );
	file.Write( "\n" );
	
	string s;
	
	for( unsigned int i=0; i<m_products.size(); i++ ) {
		
		ReplaceCharactersInString( m_products[ i ].m_nazov, file_field_delimiters, '_' );
		
		file.Print( "%i*%s*%0.1f",
			m_products[ i ].m_cislo, m_products[ i ].m_nazov.c_str(), m_products[ i ].m_cena );
		file.Print( "*%c*%c\n",
			m_products[ i ].m_kategoria,
			Centrala::Instancia()->Register()->Stav2Char( m_products[ i ].m_stav ) );
	}
}

void Seller::Save( void )
{
	// File name is specified as "<seller id>-<short code>.txt".
	// Only seller id is unique, as short code could be changed.
	// so I have to look for file starting with seller id.
	
	string previous_filename, new_filename;
	string predajca_dir, predajca_pat;
	
	predajca_pat = predajca_dir;
	predajca_pat += "*.txt";
	
	// find previous filename
	predajca_pat = Centrala::Instancia()->Nastavenia()->m_data_dir_path;
	predajca_pat += ConvFuncs::FormatStdStr( "%i-*.txt", m_unique_id );
	bool previous_found;
	previous_found = ConvFuncsInstance->Find_FirstFile( predajca_pat.c_str() );
	if ( previous_found ) {
		previous_filename = ConvFuncsInstance->Find_CurrentFileName();
	}
	
	new_filename = ConvFuncs::FormatStdStr( "%i-%s-%s.txt", m_unique_id, m_kod.c_str(), m_meno.c_str() );
	
	// save
	predajca_pat = Centrala::Instancia()->Nastavenia()->m_data_dir_path;
	predajca_pat += new_filename;
	
	CConvFile	file;
	file.OpenFile( predajca_pat.c_str(), CConvFile::fm_text, CConvFile::fm_write );
	if ( ! file.OpenedWriteable() ) {
		
		string s1 = _( "Could not open output file '%s'" );
		string s2 = ConvFuncs::FormatStdStr( s1.c_str(), new_filename.c_str() );
		
		throw runtime_error( s2 );
	}
	
	SaveToTxtFile( file );
	file.Close();
	
	// compare new filename with previous
	// if different, erase previous file.
	if (( previous_found ) && ( new_filename != previous_filename )) {
		predajca_pat = Centrala::Instancia()->Nastavenia()->m_data_dir_path;
		predajca_pat += previous_filename;
		
		ConvFuncs::DoDeleteFile( predajca_pat.c_str() );
	}
}


void Seller::LoadFromTxtFile( CConvFile& file )
{
	// items cannot have fixed length, because source files are unicode (that means variable char length, either 1 or 2 chars)
	
	static const int linebuffer_len = 512;
	char linebuffer[ linebuffer_len ];
	char namebuffer[ linebuffer_len ];
	unsigned short strpos;
	
	const char* cp, *linep;
	Product		t;
	
	file.gets( linebuffer, linebuffer_len );
	m_unique_id = atoi( ConvFuncsInstance->StrItem( linebuffer, 0, file_field_delimiters ) );
	m_kod = ConvFuncsInstance->StrItem( linebuffer, 1, file_field_delimiters );
	m_meno = ConvFuncsInstance->StrItem( linebuffer, 2, file_field_delimiters );
	m_poznamka = ConvFuncsInstance->StrItem( linebuffer, 3, file_field_delimiters );
	
	m_products.clear();
	
	while( 1 ) {
		if ( file.Eof() )
			break;
		if ( file.gets( linebuffer, linebuffer_len ) == 0 )
			break;
		
		linep = linebuffer;
		cp = ConvFuncsInstance->StrItem( linebuffer, 0, file_field_delimiters, &strpos );
		// skip empty lines
		if ( cp == NULL )
			continue;
		t.m_cislo = atoi( cp );
		
		// item name
		linep += strpos;
		cp = ConvFuncsInstance->StrItem( linep, 0, file_field_delimiters, &strpos );
		t.m_nazov = cp;
		
		linep += strpos;
		
		t.m_cena = atof( ConvFuncsInstance->StrItem( linep, 0, file_field_delimiters ) );
		t.m_kategoria = * ConvFuncsInstance->StrItem( linep, 1, file_field_delimiters );
		t.m_stav = Centrala::Instancia()->Register()->Char2Stav( ( * ConvFuncsInstance->StrItem( linep, 2, file_field_delimiters ) ) );
		
		m_products.push_back( t );
	} ;
}

void Seller::PrintProductsToPDF( PdfWrapperBase& pdf )
{
	string str;
	
	str = m_kod;
	str += " - ";
	str += m_meno;
	
	StringEncoding::UTF8toISO8859_2( str );
	pdf.WriteSingleText( 10, 10, str.c_str() );
	
	TableOfCells	table;
	TableColumnSpecs columns;
	
	columns.AddColumn( "#", 40 );
	columns.AddColumn( _("nazov"), 200 );
	columns.AddColumn( _("cena"), 60 );
	columns.AddColumn( _("kat"), 40 );
	columns.AddColumn( _("stav"), 90 );
	
	BurzaRegister* reg = Centrala::Instancia()->Register();	
	
	unsigned int i, c;
	c = 5 + ( rand() % 70 );
	table.Resize( c, 5 );
	
	for( unsigned int j=0; j<c; ++j ) {
		
		i = j % m_products.size();
		
		table.Cell( j, 0 ) = ConvFuncs::FormatStdStr( "%i", m_products[ i ].m_cislo );
		str = m_products[ i ].m_nazov;
		StringEncoding::UTF8toISO8859_2( str );
		table.Cell( j, 1 ) = str;
		table.Cell( j, 2 ) = ConvFuncs::FormatStdStr( "%0.2f", m_products[ i ].m_cena );
		table.Cell( j, 3 ) = ConvFuncs::FormatStdStr( "%c", m_products[ i ].m_kategoria );
		table.Cell( j, 4 ) = reg->Stav2String( m_products[ i ].m_stav );
	}
	
	pdf.DrawTable( 50, 50, table, columns );
}

double Seller::CalculateFee( void )
{
	double fee = 0;
	t_items_in_categories itemCategories;
	t_items_in_categories::iterator icit;
	ProductCategory*	cat;
	
	itemCategories = GetItemsCategories();
	for( icit = itemCategories.begin(); icit != itemCategories.end(); ++icit ) {
		
		double itemFee;
		
		cat = Centrala::Instancia()->Register()->FindProductCategory( icit->first );
		if ( cat == NULL )
			itemFee = -100.0;
		else
			itemFee = cat->m_fee_per_item;
		
		fee += itemFee * icit->second;
	}
	
	SellerCategory* scat;
	scat = Centrala::Instancia()->Register()->FindSellerCategory( m_kategoria );
	if ( scat == NULL )
		fee -= 10000;
	else
		fee += scat->m_fee_per_sale;
}

Seller::t_items_in_categories Seller::GetItemsCategories( void )
{
	t_items_in_categories res;
	t_items_in_categories::iterator it;
	
	for( unsigned int i=0; i<m_products.size(); ++i ) {
		
		it = res.find( m_products[ i ].m_kategoria );
		
		if ( it == res.end() )
			res[ m_products[ i ].m_kategoria ] = 1;
		else
			it->second++;
		
	}
	
	return res;
}

// ==============================================
// ==============================================
// ==============================================

void BurzaRegister::NacitajVsetkychPredajcov( void )
{
	std::string		predajca_dir, predajca_pat, predajca_fname;
			
	predajca_dir = Centrala::Instancia()->Nastavenia()->m_data_dir_path;
	
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
				novy.LoadFromTxtFile( f );
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

BurzaRegister::t_predajcovia::iterator BurzaRegister::NajdiPredajcuPodlaKoduIt( const char* kod )
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

int BurzaRegister::NajdiPredajcuPodlaKoduIndex( const char* kod )
{
	std::vector< Seller >::iterator	it;
	int idx;
	
	for( idx = 0, it = m_predajcovia.begin(); it != m_predajcovia.end(); it++, idx++ ) {
		if ( stricmp( it->m_kod.c_str(), kod ) == 0 ) {
			return idx;
		}
	}
	
	return -1;
}

Seller* BurzaRegister::NajdiPredajcuPodlaKoduPtr( const char* kod )
{
	std::vector< Seller >::iterator	it;
	
	for( it = m_predajcovia.begin(); it != m_predajcovia.end(); it++ ) {
		if ( stricmp( it->m_kod.c_str(), kod ) == 0 ) {
			return &(*it);
		}
	}
	
	return NULL;
}

ProductCategory* BurzaRegister::FindProductCategory( char category )
{
	t_productCategories::iterator it = m_productCategories.find( category );
	if ( it == m_productCategories.end() )
		return NULL;
	return & it->second;
}

SellerCategory* BurzaRegister::FindSellerCategory( char seller_category )
{
	t_sellerCategories::iterator it = m_sellerCategories.find( seller_category );
	if ( it == m_sellerCategories.end() )
		return NULL;
	return & it->second;
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

unsigned int BurzaRegister::GenerateSellerUniqueId( void )
{
	unsigned int max = 0;
	for( int i=0; i<m_predajcovia.size(); ++i ) {
		if ( m_predajcovia[ i ].m_unique_id > max )
			max = m_predajcovia[ i ].m_unique_id;
	}
	
	return max+1;
}

void BurzaRegister::GenerateSellerUniqueCode( Seller& seller, std::string try_this_code )
{
	// if its empty try to generate it from name
	if ( try_this_code.empty() ) {
		std::vector<std::string> tokens;
		ConvFuncs::Tokenize( seller.m_meno, tokens );
		
		for( int i=0; i<tokens.size(); ++i ) {
			try_this_code += tokens[ i ][ 0 ];
		}
	}
	
	// minimum length 2
	while ( try_this_code.size() < 2 ) {
		try_this_code += "A";
	}
	
	// run through all the sellers to see if its unique
	// if its not, generate first that is.
	t_predajcovia::iterator it;
	int tries = 0;
	while( true ) {
		
		ConvFuncs::StdStringToUpper( try_this_code );
		it = NajdiPredajcuPodlaKoduIt( try_this_code.c_str() );
		if ( it == m_predajcovia.end() )
			break;
		
		// inc characters, AB => AC, AZ => BA
		int char_pos = try_this_code.size() - 1;
		while( char_pos >= 0 ) {
			if ( try_this_code[ char_pos ] >= 'Z' ) {
				try_this_code = 'A';
				char_pos--;
			} else {
				try_this_code[ char_pos ]++;
				break;
			}
		}
		
		// Wooh, I could loop like this forever. Just help it if its too much
		tries++;
		if ( tries > 100 ) {
			try_this_code += "A";
			tries = 0;
		}
	}
	
	seller.m_kod = try_this_code;
}

void BurzaRegister::FinishCheckout( BurzaRegister::t_sellerProducts& items )
{
	double sum = CalcPriceOfProducts( items );
	
	// mark items in register as sold
	BurzaRegister::t_sellerProducts::iterator it, ite;
	
	int pidx;
	Product* p;
	Seller* s;
	
	for( it = items.begin(), ite = items.end(); it != ite; ++it ) {
		s = NajdiPredajcuPodlaKoduPtr( it->sellerCode.c_str() );
		if ( s == NULL ) {
			throw runtime_error( "BurzaRegister::FinishCheckout: Seller not found" );
		}
		pidx = s->NajdiTovarIndex( it->productNumber );
		if ( pidx == -1 ) {
			throw runtime_error( "BurzaRegister::FinishCheckout: Item not found" );
		}
		sum = s->m_products[ pidx ].m_cena;
	}
	
	// prepare PDF receipt
	// and print sold items to receipt
	try {
		PdfWrapper	pdf;
		string pdfFileName = Centrala::Instancia->Nastavenia()->m_data_dir_path;
		pdfFileName += "ucet_";
		
		ConvFuncs::GetDateTime();
		ConvFuncs::s_datetime *dt = &ConvFuncs::datetime;
		pdfFileName += ConvFuncs::FormatStdStr( "%i_%i_%i-%i_%i",
			dt->year, dt->month, dt->day, dt->hour, dt->minute );
		pdfFileName += ".pdf";
		
		pdf.CreatePDF( pdfFileName );
		
		// print page title
		string title;
		title = _( "Účet vystavený " );
		title += ConvFuncs::FormatStdStr( "%i-%i-%i %i:%i",
			dt->year, dt->month, dt->day, dt->hour, dt->minute );
		
		StringEncoding::UTF8toISO8859_2( title );
		pdf.NewPage( title );
		
		// items in a table
		TableOfCells	table;
		TableColumnSpecs columns;
		unsigned int index;
		
		columns.AddColumn( "#", 40 );
		columns.AddColumn( _("číslo"), 60 );
		columns.AddColumn( _("názov"), 200 );
		columns.AddColumn( _("cena"), 60 );
		
		table.Resize( items.size(), 4 );
		
		for( it = items.begin(), ite = items.end(), index = 0; 
				it != ite; ++it, ++index ) {
			
			s = NajdiPredajcuPodlaKoduPtr( it->sellerCode.c_str() );
			if ( s == NULL ) {
				throw runtime_error( "BurzaRegister::FinishCheckout: Seller not found" );
			}
			pidx = s->NajdiTovarIndex( it->productNumber );
			if ( pidx == -1 ) {
				throw runtime_error( "BurzaRegister::FinishCheckout: Item not found" );
			}
			
			table.Cell( index, 0 ) = ConvFuncs::FormatStdStr( "%i", index + 1 );
			str = ConvFuncs::FormatStdStr( "%s-%i", it->sellerCode.c_str(), it->productNumber );
			StringEncoding::UTF8toISO8859_2( str );
			table.Cell( index, 1 ) = str;
			str = s->m_products[ pidx ].m_nazov;
			StringEncoding::UTF8toISO8859_2( str );
			table.Cell( index, 2 ) = str;
			table.Cell( index, 3 ) = ConvFuncs::FormatStdStr( "%0.2f", s->m_products[ pidx ].m_cena );
		}
		
		pdf.DrawTable( 50, 50, table, columns );
		
		pdf.ClosePDF();
		
	} catch( runtime_error& e ) {
		DisplayErrorMessageBox( "PDF error", e.what() );
	} catch( out_of_range& e ) {
		DisplayErrorMessageBox( "PDF out of range error", e.what() );
	} catch( ... ) {
		DisplayErrorMessageBox( "PDF error" );
	}

}

double BurzaRegister::CalcPriceOfProducts( BurzaRegister::t_sellerProducts& items )
{
	BurzaRegister::t_sellerProducts::iterator it, ite;
	
	double sum = 0;
	int pidx;
	Product* p;
	Seller* s;
	
	for( it = items.begin(), ite = items.end(); it != ite; ++it ) {
		s = NajdiPredajcuPodlaKoduPtr( it->sellerCode.c_str() );
		if ( s == NULL ) {
			throw runtime_error( "BurzaRegister::CalcPriceOfProducts: Seller not found" );
		}
		pidx = s->NajdiTovarIndex( it->productNumber );
		if ( pidx == -1 ) {
			throw runtime_error( "BurzaRegister::CalcPriceOfProducts: Item not found" );
		}
		sum = s->m_products[ pidx ].m_cena;
	}
	
	return sum;
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
	
	m_data_dir_path = ConvFuncsInstance->GetPathFromPathFilename(
		ConvFuncsInstance->GetExecutableFilePathName() );
	m_data_dir_path += "data/";

	m_string_mena = "∈";
}

// ==============================================
// ==============================================

Centrala::Centrala()
{
}

Centrala::~Centrala()
{
	delete mp_dialogs;
}

Centrala* Centrala::Instancia( void )
{
	if ( m_instance == NULL ) {
		m_instance = new Centrala;
		
		m_instance->mp_dialogs = new BurzaDialogs();
	}
	
	return m_instance;
}

void Centrala::Zmaz( void )
{
	if ( m_instance ) {
		
		delete m_instance;
		m_instance = NULL;
	}
}

BurzaDialogs* Centrala::Dialogy( void )
{
	return mp_dialogs;
}

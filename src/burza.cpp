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

#include <libglademm.h>

#include "convfile.h"
#include "convfncs.h"
#include "burza.h"
#include "bdialogs.h"
#include "stringencoding.h"

// not required here, only for test
#include "pdfwrap.h"
#include "table.h"
#include "datatextfile.h"

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

void BurzaRegister::BurzaRegister()
{
	m_storage = new DataTextfile();
}

void BurzaRegister::~BurzaRegister()
{
	delete m_storage;
}

void BurzaRegister::NacitajVsetkychPredajcov( void )
{
	SellerDataArray sellers = m_storage->GetSellers();
	
	for(SellerDataIterator sc=sellers.begin(), se=sellers.end(); sc != se; ++sc ) {
		Seller	s;
		s.m_kod 		= sc->shortcode;
		s.m_meno 		= sc->name;
		s.m_poznamka	= sc->note;
		s.m_kategoria	= sc->category;
		s.m_unique_id	= sc->unique_id;
		
		ItemDataArray items;
		items = m_storage->GetSellerItems( sc->unique_id );
		
		for( ItemDataIterator ic=items.begin(), ie=items.end(); ic != ie; ++ic ) {
			Product	p;
			
			p.m_cena		= ic->price;
			p.m_cislo		= ic->number;
			p.m_kategoria	= ic->category;
			p.m_nazov		= ic->name;
			p.m_stav		= ic->status;
			
			s.m_products.push_back(p);
		}
	}
}

void BurzaRegister::PridajNovehoPredajcu( const char* kod, const char* meno, const char* poznamka )
{
	Seller	p;

	p.m_kod = kod;
	p.m_meno = meno;
	p.m_poznamka = poznamka;
	
	m_predajcovia.push_back( p );
	
	m_storage->AddSeller(new SellerData(meno, shortcode_xx, poznamka, category_xx));
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

/*
		HPDFWrapper	pdf;
		string pdfFileName = Centrala::Instancia->Nastavenia()->m_data_dir_path;
		pdfFileName += "ucet_";
		
		ConvFuncs::GetDateTime();
		ConvFuncs::s_datetime *dt = &ConvFuncs::datetime;
		pdfFileName += ConvFuncs::FormatStdStr( "%i_%i_%i-%i_%i",
			dt->year, dt->month, dt->day, dt->hour, dt->minute );
		pdfFileName += ".pdf";
		
		pdf.CreatePDF( pdfFileName );
		

*/

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
	
}

void BurzaRegister::PrintRecipeeAtEndOfCheckout( BurzaRegister::t_sellerProducts& items, PdfWrapperBase& pdf )
{
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

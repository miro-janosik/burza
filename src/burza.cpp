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
#include "cacheddatastorage.h"
#include "translate.h"
#include "recipees.h"

using namespace Burza;

Centrala* Centrala::m_instance = NULL;

//int _tmain(int argc, _TCHAR* argv[])
int main2( int argc, char **argv )
{
	Centrala		*pcentrala = Centrala::Instance();

	pcentrala->Register().m_storage = new CachedDataTextfile( Centrala::Settings().m_data_dir_path );
	//TODO: dispose dataStorage in destructor somehow.
	//	DataTextfile dataStorage = new DataTextfile( Centrala::Instancia()->Nastavenia()->m_data_dir_path );
	//	m_storage = new CachedDataStorage(*dataStorage);
	
	ConvFuncs::SingletonInstanceCreate();
	
	Gtk::Main	kit( argc, argv );
	
	// do not use local settings, use international. I want '.' as decimal delimiter.
	setlocale(LC_ALL,"C");
	
	// ============
	
	Centrala::Dialogs().SetupDialogs();
	
	// Centrala::Register().NacitajVsetkychPredajcov();
	Centrala::Dialogs().FillSellerCombos();
	Centrala::Dialogs().RegisterDisplaySellerByIndex( 0 );

	try {
		//Gtk::Main::run( * pcentrala->Dialogy()->m_pWndRegister );
		Gtk::Main::run();
	}
	catch (DataStorageException& ex){
		BurzaDialogs::DisplayErrorMessageBox( ex.what(), _("Program failed to work, has to exit.") );
	}

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

/*
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

*/

double BurzaRegister::CalculateFee( SellerData& seller, ItemDataArray& items )
{
	double fee = 0;
	ItemCategoryArray	itemCategories;
	ItemCategoryIterator icit;
	
	itemCategories = m_storage->GetItemsCategories( items );
	for( icit = itemCategories.begin(); icit != itemCategories.end(); ++icit ) {
		
		fee += icit->fee_per_item;
	}
	
	SellerCategory scat = m_storage->GetSellerCategory(seller.category);
	
	fee += scat.seller_fee;
}

// ==============================================
// ==============================================
// ==============================================

BurzaRegister::BurzaRegister()
{
}

BurzaRegister::~BurzaRegister()
{
	delete m_storage;
}


	/*
void BurzaRegister::NacitajVsetkychPredajcov( void )
{
	m_storage->GetSellers();
	
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
	*/

void BurzaRegister::PridajNovehoPredajcu( const char* kod, const char* meno, const char* poznamka )
{
	/*
	Seller	p;

	p.m_kod = kod;
	p.m_meno = meno;
	p.m_poznamka = poznamka;
	
	m_predajcovia.push_back( p );
	*/
	
	SellerData newseller(meno, kod, poznamka, 'a');
	m_storage->AddSeller(newseller);
}

	/*
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
	
	string codeString( kod );
	SellerData seller = m_storage->GetSellerByCode(codeString);
	
	return x;
}
	*/

int BurzaRegister::NajdiPredajcuPodlaKoduIndex( const char* kod )
{
	SellerDataArray sellers = m_storage->GetSellers();
	string codeString(kod);
	
	return DataAccessInterface::FindSellerIndexByCode( sellers, codeString );
}

SellerData BurzaRegister::NajdiPredajcuPodlaKodu( const char* kod )
{
	string codeString(kod);
	SellerData seller = m_storage->GetSellerByCode(codeString);
	
	return seller;
}

// ==============================================
// ==============================================
// ==============================================

void BurzaRegister::GenerateSellerUniqueCode( SellerData& seller, std::string try_this_code )
{
	// if its empty try to generate it from name
	if ( try_this_code.empty() ) {
		std::vector<std::string> tokens;
		ConvFuncs::Tokenize( seller.name, tokens );
		
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
	int sellerIndex;
	int tries = 0;
	SellerDataArray	sellers = m_storage->GetSellers();
	
	while( true ) {
		
		ConvFuncs::StdStringToUpper( try_this_code );
		sellerIndex = DataAccessInterface::FindSellerIndexByCode( sellers, try_this_code );
		if ( sellerIndex == -1 )
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
	
	seller.shortcode = try_this_code;
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

void BurzaRegister::FinishCheckout( ItemDataArray& items )
{
	double sum = CalcPriceOfProducts( items );
	/*
	// mark items in register as sold
	ItemDataIterator it, ite;
	
	for( it = items.begin(), ite = items.end(); it != ite; ++it ) {

		ItemData item;
		try {
			item = m_storage->GetItem(it->id);
		}
		catch(WrongItemException ex) {
			throw runtime_error( "BurzaRegister::FinishCheckout: Item not found" );
		}
		
		item.status = sold;
		m_storage->ModifyItem(item.id, item);
	}
	*/
	try {
	  
	  Recipees recipees;
	  HPDFWrapper	pdf;
	  pdf.CreatePDF( "/home/mirex/tmp/a.pdf" );
	  
	  recipees.PrintRecipeeAtEndOfCheckout( items, pdf );
	  
	  pdf.ClosePDF();
	  
	} catch( runtime_error& e ) {
		Centrala::Dialogs().DisplayErrorMessageBox( "PDF error", e.what() );
	} catch( out_of_range& e ) {
		Centrala::Dialogs().DisplayErrorMessageBox( "PDF out of range error", e.what() );
	} catch( ... ) {
		Centrala::Dialogs().DisplayErrorMessageBox( "PDF error" );
	}
}

double BurzaRegister::CalcPriceOfProducts( ItemDataArray& items )
{
	ItemDataIterator it, ite;
	double sum = 0;
	
	for( it = items.begin(), ite = items.end(); it != ite; ++it ) {
		sum += it->price;
	}
	
	return sum;
}

int BurzaRegister::VerifyItemNumber( ItemData& item, int newNumber )
{
	int max = 0;
	bool valid = true;
	
	ItemDataArray items = Centrala::Storage().GetSellerItems( item.seller_id );
	
	for( int i=0; i<items.size(); ++i ) {
		
		if (( newNumber == items[ i ].number ) && ( items[ i ].id != item.id )) {
			valid = false;
		}

		if ( max < items[ i ].number ) {
			max = items[ i ].number;
		}
	}
	
	if ( newNumber == -1 )
		valid = false;
	
	if ( ! valid ) {
		newNumber = max + 1;
	}
	
	return newNumber;
}

// ==============================================
// ==============================================

NastaveniaProgramu::NastaveniaProgramu()
{
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

Centrala* Centrala::Instance( void )
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


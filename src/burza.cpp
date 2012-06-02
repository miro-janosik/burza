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
#include "translate.h"
#include "recipees.h"
#include "xmltool.h"
#include "errhandler.h"
#include "itemsearch.h"

#include "datatextfile.h"
#include "cacheddatastorage.h"
#include "datapostgrespqxx.h"

using namespace Burza;
using namespace XmlWrapperTool;

Centrala* Centrala::m_instance = NULL;

int main2( int argc, char **argv )
{
	Glib::init();

	Centrala		*pcentrala = Centrala::Instance();

	//TODO: dispose dataStorage in destructor somehow.
	//	DataTextfile dataStorage = new DataTextfile( Centrala::Instancia()->Nastavenia()->m_data_dir_path );
	//	m_storage = new CachedDataStorage(*dataStorage);
	
	ConvFuncs::SingletonInstanceCreate();
	
	Gtk::Main	kit( argc, argv );
	
	// do not use local settings, use international. I want '.' as decimal delimiter.
	setlocale(LC_ALL,"C");
	
	// ============

	std::string exceptionMessage;
	
	try {
		Centrala::Settings().LoadSettings();
		Centrala::Settings().Apply();

		Centrala::Dialogs().SetupDialogs();

		// Centrala::Register().NacitajVsetkychPredajcov();
		Centrala::Dialogs().initializing = true;

		Centrala::Dialogs().FillSellerCombos();
		Centrala::Dialogs().RegisterDisplaySellerByIndex( 0 );

		Centrala::Dialogs().initializing = false;

		Gtk::Main::run();
	}
	catch (std::exception& ex) {
		exceptionMessage = ex.what();
	}
	catch (DataStorageException& ex){
		exceptionMessage = ex.what();
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

	return -4.0;
}

// ==============================================
// ==============================================
// ==============================================

BurzaRegister::BurzaRegister()
{
	m_storage = NULL;
}

BurzaRegister::~BurzaRegister()
{
	delete m_storage;
}


void BurzaRegister::PridajNovehoPredajcu( const char* kod, const char* meno, const char* poznamka )
{	
	// if ( DemoLimitsCanAddSeller() ) 
	{
		SellerData newseller(meno, kod, poznamka, 'a');
		m_storage->AddSeller(newseller);
	}
}

int BurzaRegister::NajdiPredajcuPodlaKoduIndex( const char* kod )
{
	// obsolete
	assert( false );
	return -1;
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

void BurzaRegister::FinishCheckout( ItemDataArray& items )
{
	try {
	  
	  Recipees recipees;
	  HPDFWrapper	pdf;
	  string filename = Centrala::Settings().m_document_ouput_directory;
	  filename += ConvFuncs::FormatStdStr( "%s_%02i-%02i-%02i_%02i-%02i.pdf",
		_("Predaj"),
		ConvFuncs::datetime.year % 100, ConvFuncs::datetime.month, ConvFuncs::datetime.day,
		ConvFuncs::datetime.hour, ConvFuncs::datetime.minute );
	  pdf.CreatePDF( filename );

	  recipees.PrintRecipeeAtEndOfCheckout( items, pdf );
	  
	  pdf.ClosePDF();
	  
	} catch( runtime_error& e ) {
		Centrala::Dialogs().DisplayErrorMessageBox( "PDF error", e.what() );
	} catch( out_of_range& e ) {
		Centrala::Dialogs().DisplayErrorMessageBox( "PDF out of range error", e.what() );
	} catch( ... ) {
		Centrala::Dialogs().DisplayErrorMessageBox( "PDF error" );
	}

	// mark items in register as sold
	ItemDataIterator it, ite;
	
	for( it = items.begin(), ite = items.end(); it != ite; ++it ) {

		it->status = sold;
		m_storage->ModifyItem(it->id, *it);
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

int BurzaRegister::DisplayPdf( const char* fileName )
{
	system( fileName );
	return 0;
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
		m_instance->itemSearch = new ItemSearch();
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

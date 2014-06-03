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
#include "log.h"

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
#include "excelwrap.h"


using namespace Burza;
using namespace XmlWrapperTool;

Centrala* Centrala::m_instance = NULL;

int main2( int argc, char **argv )
{
	BiturnLogInstance()->SetLogFileName( "burza-log.txt" );
	BiturnLogInstance()->SetLoggingMood( BiturnLog::LoggingMoodHeavy );
	Log( InfoMajor, "=== Burza starting ===\n" );
	Log( InfoMajor, "main2 starting\n" );

	Log( InfoMajor, "Call to Glib::init\n" );
	Glib::init();

	Log( InfoMajor, "Creating central instance\n" );
	Centrala		*pcentrala = Centrala::Instance();

	//TODO: dispose dataStorage in destructor somehow.
	//	DataTextfile dataStorage = new DataTextfile( Centrala::Instancia()->Nastavenia()->m_data_dir_path );
	//	m_storage = new CachedDataStorage(*dataStorage);
	
	Log( InfoMajor, "Creating conv funcs\n" );
	ConvFuncs::SingletonInstanceCreate();
	
	Log( InfoMajor, "Gtk::Main\n" );
	Gtk::Main gtkkit( argc, argv );
	
	// do not use local settings, use international. I want '.' as decimal delimiter.
	// setlocale(LC_ALL,"C");
	setlocale(LC_NUMERIC,"C");
	
	// ============

	std::string exceptionMessage;
	
	try {
		Log( InfoMajor, "Loading and applying settings\n" );
		Centrala::Settings().LoadSettings();
		Centrala::Settings().Apply();

		Log( InfoMajor, "SetupDialogs\n" );
		Centrala::Dialogs().SetupDialogs();

		// Centrala::Register().NacitajVsetkychPredajcov();
		Centrala::Dialogs().initializing = true;

		Centrala::Dialogs().ReloadSellers();
		Centrala::Dialogs().RegisterDisplaySellerByIndex( 0 );

		Centrala::Dialogs().initializing = false;

		Log( InfoMajor, "Gtk::Main::run()\n" );
		Gtk::Main::run( *Centrala::Dialogs().m_pWndRegister );
	}
	catch (std::exception& ex) {
		exceptionMessage += "std::exception: ";
		exceptionMessage += ex.what();
	}
	catch (DataStorageException& ex){
		exceptionMessage += "DataStorageException: ";
		exceptionMessage += ex.what();
	}
	catch(...)
	{
		exceptionMessage += "Unknown exception.";
	}

	if (!exceptionMessage.empty())
	{
		std::string str = "Exception - " + exceptionMessage + "\n";
		Log( InfoError, str.c_str() );
	}
	
	// setup update timer
	// m_timer_connection = Glib::signal_timeout().connect( sigc::ptr_fun( &on_timer ), 100 );

	// --=============================--
	
	Log( InfoMajor, "Finishing application...\n" );
	ConvFuncs::SingletonInstanceDelete();
	
	Log( InfoMajor, "Removing central instance\n" );
	Centrala::Zmaz();
	
	Log( InfoMajor, "Finished.\n" );
	return 0;
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

SellerData BurzaRegister::NajdiPredajcuPodlaKodu( const char* kod )
{
	string codeString(kod);
	SellerData seller = m_storage->GetSellerByCode(codeString);
	
	return seller;
}

bool BurzaRegister::SellerExists(std::string shortcode)
{
	bool sellerExists = true;
	try {
		m_storage->GetSellerByCode( shortcode );
	}
	catch( WrongSellerException e) {
		sellerExists = false;
	}
	
	return sellerExists;
}

/*
SELLER_CATEGORY_ID BurzaRegister::GetDefaultSellerCategoryId( void )
{
	for( int i=0; i< categories.size(); ++i )
		if ( categories[ i ].id == id )
			return categories[ i ];

	throw WrongSellerCategoryException();
}
*/

// ==============================================
// ==============================================
// ==============================================

void BurzaRegister::FinishCheckout( ItemDataArray& items )
{
	try {
		
		Recipees recipees;
		HPDFWrapper	pdf;
		string filename = Centrala::Settings().m_document_ouput_directory;
		
		// check existance or create the folder.
		ConvFuncs *fncs = ConvFuncsInstance();
		if ( fncs->Find_FirstFile( filename ) == false ) {
			ConvFuncs::DoCreateFolder( filename.c_str() );
		}
		  
		filename += ConvFuncs::FormatStdStr( "%s_%02i-%02i-%02i_%02i-%02i.pdf",
		_("Predaj"),
		ConvFuncs::datetime.year % 100, ConvFuncs::datetime.month, ConvFuncs::datetime.day,
		ConvFuncs::datetime.hour, ConvFuncs::datetime.minute );
		pdf.CreatePDF( filename );

		recipees.PrintRecipeeAtEndOfCheckout( items, pdf );
		
		pdf.ClosePDF();
		
	} catch( std::exception& e ) {
		ErrorHandling::HandlePdfExceptions( e );
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

BurzaRegister::LoadSellerSheetFileResult BurzaRegister::LoadSellerSheetFile(std::string filename)
{
	LoadSellerSheetFileResult	result;

	// get extension type
	string::size_type pos = filename.rfind('.');
	string ext;
	bool knownType = false;

	if (pos != string::npos) {
		ext = filename.substr( pos + 1 );
		ConvFuncs::StdStringToLower( ext );
	}

	try {
		if ( ext == "txt" ) {
			knownType = true;
			CachedDataTextfile	textStorage( Centrala::Settings().m_data_dir_path );
			textStorage.LoadFromTxtFile(filename, result.seller, result.items );
			result.status = BurzaRegister::Loaded;
		}
		if ( ext == "xls" ) {
			knownType = true;
			BurzaExcel::BasicExcelFileWrapper	excelFile;
			excelFile.ReadSeller( filename, result.seller, result.items );
			result.status = BurzaRegister::Loaded;
		}
		if ( ext == "ods" ) {
			result.status = BurzaRegister::FormatNotSupported;
		}
		if ( ext == "xlsx" ) {
			result.status = BurzaRegister::FormatNotSupported;
		}

		if (knownType == false) {
			result.status = BurzaRegister::FormatNotKnown;
		}
	}
	catch( runtime_error& e ) {

		result.status = BurzaRegister::ErrorImporting;
		result.errorText = e.what();
	}

	result.itemsWithZeroPrice = 0;
	result.itemsWithPriceWithoutDecimalPart = 0;
	result.itemsWithoutName = 0;

	// check for basic errors in filled sheet.
	if ( result.status == BurzaRegister::Loaded ) {
		for( int i=0; i < result.items.size(); ++i ) {

			if ( result.items[ i ].name.empty() )
				++result.itemsWithoutName;

			double price = result.items[ i ].price;
			if ( price == 0 )
				++result.itemsWithZeroPrice;

			if ( price == (int)( price ) )
				++result.itemsWithPriceWithoutDecimalPart;
		}
	}

	return result;
}


void BurzaRegister::CreateFolders( string path, vector<string>& folderNames )
{
	for( int i=0; i<folderNames.size(); ++i)
		ConvFuncs::DoCreateFolder( path.c_str(), folderNames[ i ].c_str() );
}

ItemCategory& BurzaRegister::BestMatchingProductCategory( ItemData& item, ItemCategoryArray& categories )
{
	assert( ! categories.empty() );
	
	// try to find price match
	double price = item.price;
	for (int i=0; i<categories.size(); ++i)
	{
		if ((categories[i].limit_price_min <= price) && (price <= categories[i].limit_price_max))
			return categories[i];
	}
	
	return categories[0];
}

void BurzaRegister::SetItemBestMatchingProductCategory( ItemData& item, ItemCategoryArray& categories )
{
	ItemCategory category = BestMatchingProductCategory( item, categories );
	item.category = category.id;
}

ItemCategoryArray BurzaRegister::GetItemCategoriesForMatching( void )
{
	ItemCategoryArray categories = Centrala::Storage().GetItemsCategories();
	
	/* This is already done in the the GetItemsCategories
	for (int i=0; i<categories.size(); ++i)
	{
		if (categories[i].limit_price_min < 0)
			categories[i].limit_price_min = 0;
		if (categories[i].limit_price_max < 0)
			categories[i].limit_price_max = 10e6;
	}
	*/
	
	return categories;
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

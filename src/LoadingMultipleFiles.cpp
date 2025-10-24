#include <functional>

#include <gtkmm.h>

#include "convfncs.h"
#include "errhandler.h"
#include "burza.h"
#include "LoadingMultipleFiles.h"
#include "log.h"
#include "RepeatNotifierWorker.h"

#ifndef _
#define _(x) x
#endif

using namespace Burza;

LoadingMultipleFiles::LoadingMultipleFiles()
{
	// setup a progress notifier
	loadingRefreshNotificationDispatcher.connect(sigc::mem_fun(*this, &LoadingMultipleFiles::RefreshingNotificationAction));
}

void LoadingMultipleFiles::ConnectButtons( void )
{
	buttonOk->signal_clicked().connect( sigc::mem_fun( *this, &LoadingMultipleFiles::FinishedClickedOk ) );
}

void LoadingMultipleFiles::FinishedClickedOk()
{
	window->hide();
	window->unset_transient_for();
}

void LoadingMultipleFiles::Finished(const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gtk::FileDialog>& dialog)
{
	// Handle the response:
	try
	{
		auto folder = dialog->select_folder_finish(result);

		// Notice that this is a std::string, not a Glib::ustring.
		auto path = folder->get_path();
		ImportMultiSellersLog("File selected: ");
		ImportMultiSellersLog(path.c_str());

		// set back the import directory
		path += "/";
		Centrala::Settings().directoryImport = path;
		sourcePath = Centrala::Settings().directoryImport;
		
		ImportMultiSellersLog( "Selected folder: " + sourcePath );
	}
	catch (const Gtk::DialogError& err)
	{
		// Can be thrown by dialog->open_finish(result).
		ImportMultiSellersLog("No file selected. ");
		ImportMultiSellersLog(err.what());
	}
	catch (const Glib::Error& err)
	{
		ImportMultiSellersLog("Unexpected exception. ");
		ImportMultiSellersLog(err.what());
	}

}

bool LoadingMultipleFiles::AskForFolder( Gtk::Window& parentWindow )
{
	ImportMultiSellersLog( "Starting" );
	sourcePath = "";

	auto dialog = Gtk::FileDialog::create();

	std::string initPath = Centrala::Settings().directoryImport;
	if ( Centrala::Settings().directoryImport.empty() ) {
		initPath = Centrala::Settings().m_data_dir_path;
	}

	auto ghandle = Gio::File::create_for_path(initPath);
	dialog->set_initial_folder( ghandle );
	// dialog->set_initial_name( fileName );

	dialog->set_title("Select folder with sheets");
	// dialog., Gtk::FileChooser::Action::SELECT_FOLDER
  	dialog->select_folder(sigc::bind(
		sigc::mem_fun(*this, &LoadingMultipleFiles::Finished), dialog));

	return true;
}

void LoadingMultipleFiles::CreateOutputFolders( void )
{
	if (sourcePath.empty())
	{
		throw new runtime_error("CreateOutputFolders called without sourcePath");
	}
	
	BurzaRegister::CreateFolders( sourcePath, folderNames );
}

void LoadingMultipleFiles::DisplayWindow( Gtk::Window& parentWindow )
{
	window->show();
	window->set_transient_for( parentWindow );
	buttonInterrupt->set_sensitive( false );
	buttonOk->set_sensitive( false );
}

void LoadingMultipleFiles::SimpleNotification( int perc, string text )
{
	ImportMultiSellersLog( text );
	
	string displayedText = editLoadingText->get_buffer()->get_text();
	displayedText += text + "\n";
	editLoadingText->get_buffer()->set_text( displayedText );
}

void LoadingMultipleFiles::RunImport( Gtk::Window& parentWindow )
{
	CreateOutputFolders();
	DisplayWindow(parentWindow);
	
	buttonOk->set_sensitive( false );
	
	editLoadingText->get_buffer()->set_text( "" );
	
	// repeatNotifier.Start(std::chrono::milliseconds(400), [this] { loadingRefreshNotificationDispatcher.emit(); } );
	// loadSellersWorker.Start(std::bind(&LoadingMultipleFiles::LoadMultipleFiles, this, _1));
	LoadingMultipleFiles::LoadMultipleFiles( [this](int perc, string text) { SimpleNotification(perc, text); } );
	
	// Buttons:
	// stop (enabled only when import is running, only one clickable)
	// Return right away to return control to user; so user can click the 'stop'
	// run separate thread that does import + update of progress (perc + new text to log)
	// fix displayed text (Filename:  Import OK; Code AA, 80 items;
	// When progress is 100% update buttons enabled/disabled.
	// and some mutex to not to allow any other action, while running is not finished.
}

void LoadingMultipleFiles::RefreshingNotificationAction( void )
{
	auto data = loadSellersWorker.GetData();
	editLoadingText->get_buffer()->set_text( data.status );
	// editLoadingText->scroll
	
	if (data.finished)
	{
		repeatNotifier.Stop();
		buttonOk->set_sensitive( true );
	}
}

void LoadingMultipleFiles::LoadMultipleFiles( function<void(int, string)> updateStatus )
{
	string path = sourcePath;
	auto mp_register = & Centrala::Register();
	int finishedCount = 0, problemCount = 0;
	int percentFinished = 0;
	
	SellerCategoryArray categories = Centrala::Storage().GetSellersCategories();
	SELLER_CATEGORY_ID newSellerCategory = categories.empty() ? SELLER_CATEGORY_ID_NONE : categories.begin()->id;
	ItemCategoryArray itemCategories = Centrala::Register().GetItemCategoriesForMatching();
	
	try {
	
	updateStatus(percentFinished, "enumerating files");
	std::vector<std::string> fileNames;
	{
		bool found = ConvFuncsInstance->Find_FirstFile( path.c_str() );
		while ( found )
		{
			// find can find anything in the folder .. skip directories, use only files.
			if (ConvFuncsInstance->Find_CurrentIsDir()) {
				// find next file
				found = ConvFuncsInstance->Find_NextFile();
				continue;
			}
			
			string fileNameShort = ConvFuncsInstance->Find_CurrentFileName();
			fileNames.push_back(fileNameShort);

			// find next file
			found = ConvFuncsInstance->Find_NextFile();
		}
	}
	
	int filesCount = fileNames.size(), fileIndex = 0;
	for ( auto fileNameShort : fileNames)
	{
		string filename = path + fileNameShort;
		// true if read (even if there is error then print info about read data).
		bool readAndValid = false;
		string error, warning, info;
		
		percentFinished = fileIndex * 100 / filesCount;
		updateStatus(percentFinished, "Loading " + fileNameShort );
		string loadStatusShort;
		
		BurzaRegister::LoadSellerSheetFileResult loadedFile = mp_register->LoadSellerSheetFile( filename );
		switch( loadedFile.status ) {
			case BurzaRegister::Loaded:

				// even if file was loaded there may be some values that invalidate the content. Check these values.
				if ( Centrala::Register().SellerExists( loadedFile.seller.shortcode ) ) {
					error += ConvFuncs::FormatCstr( _( " Predajca s rovnakým kódom už existuje. Zmažte najskôr predajcu '%s'." ), loadedFile.seller.shortcode.c_str() );
					loadStatusShort += "Duplicate ";
					break;
				}

				if ( loadedFile.itemsWithZeroPrice > 0 )
				{
					error += ConvFuncs::FormatCstr(_(" Obsahuje %i ks tovaru s nulovou cenou"), loadedFile.itemsWithZeroPrice );
					loadStatusShort += "ZeroPrices ";
				}
				
				if ( loadedFile.itemsWithoutName > 0 )
				{
					error += ConvFuncs::FormatCstr(_(" Obsahuje %i ks tovaru bez mena"), loadedFile.itemsWithoutName );
					loadStatusShort += "NoItemNames ";
				}
				
				if ( error.empty() )
				{
					readAndValid = true;

					if ( loadedFile.itemsWithPriceWithoutDecimalPart > 0 )
						info += ConvFuncs::FormatCstr(_(" Obsahuje %i ks tovaru s okruhlou cenou"), loadedFile.itemsWithPriceWithoutDecimalPart );

					// imported items have to be checked by hand. Take note that these were not.
					loadedFile.seller.note += ""; // _(" za MC prijal: ?");

					loadedFile.seller.category = newSellerCategory;
					loadedFile.seller.status = Burza::imported;
					
					loadedFile.seller.id = Centrala::Storage().AddSeller( loadedFile.seller );
					
					// set correct category to items
					for( int i=0; i<loadedFile.items.size(); ++i)
						Centrala::Register().SetItemBestMatchingProductCategory( loadedFile.items[i], itemCategories );
					
					Centrala::Storage().AddItems( loadedFile.items, loadedFile.seller.id );
					
					loadStatusShort += "OK ";
				}
				break;
			case BurzaRegister::FormatNotKnown:
				error = _("Neznámy typ súboru. Podporované su 'ods', 'xls', 'xlsx', 'txt'");
				loadStatusShort += "UnsupportedType ";
				break;
			case BurzaRegister::FormatNotSupported:
				error = _("Nepodporovany format suboru");
				loadStatusShort += "UnknownType ";
				break;
			case BurzaRegister::ErrorImporting:
			default:
				error = "Error importing file." + loadedFile.errorText;
				loadStatusShort += "ErrorOther ";
				break;
		}
		
		string infoText = fileNameShort;
		infoText += ";";
		infoText += loadStatusShort;
		infoText += ";";
		infoText += loadedFile.seller.shortcode;
		infoText += ";";
		infoText += ConvFuncs::FormatCstr( "%i", loadedFile.items.size() );
		infoText += ";";
		
		if ( readAndValid ) {
			
			SellerCategoryArray categories = Centrala::Storage().GetSellersCategories();
			loadedFile.seller.category = categories.empty() ? SELLER_CATEGORY_ID_NONE : categories.begin()->id;
		}
		
		if ( !error.empty() )
		{
			updateStatus(percentFinished, _( " Chyba: " ) + error + ";" );
		}

		if ( !warning.empty() )
		{
			updateStatus(percentFinished, _( " Varovanie: " ) + warning + ";" );
		}

		if ( !info.empty() )
		{
			updateStatus(percentFinished, _( " Info: " ) + info + ";" );
		}

		// move file to dedicated folder:
		string targetFolder;
		if ( !error.empty() || !warning.empty() )
		{
			targetFolder = folderError;
		}
		else {
			targetFolder = folderFinished;
		}

		string dest = path + targetFolder + "/" + fileNameShort;
		if ( 0 == ConvFuncsInstance->DoMoveFile( filename.c_str(), dest.c_str() ) ) {
			updateStatus(percentFinished, _( " Chyba pri presune na miesto: " ) + dest );
			warning = "!!";
		}

		updateStatus(percentFinished, infoText );

		if ( warning.empty() && error.empty() )
			++finishedCount;
		else
			++problemCount;
	}
	
	updateStatus(100, ConvFuncs::FormatCstr(
		_("Dokončené ... správne načítaných: %i chýb: %i."), finishedCount, problemCount) );

	// DisplayOkMessageBox( 
	//	_( "Predajcovia načítaný"),
	//	ConvFuncs::FormatStdStr( _("Korektne načítaných hárkov: %i Chýb: %i."), 
	//		finishedCount, problemCount ));

	} 
	catch (const exception& e )
	{
		updateStatus(percentFinished, e.what() );
	}
}

void LoadingMultipleFiles::ImportMultiSellersLog(const char* text)
{
	LogS( InfoMajor, "ImportMultiSellers: %s\n", text );
}

void LoadingMultipleFiles::ImportMultiSellersLog(std::string text)
{
	ImportMultiSellersLog(text.c_str());
}

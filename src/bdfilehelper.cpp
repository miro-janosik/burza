#include "bdfilehelper.h"

#include <gtkmm-2.4/gtkmm/filechooserdialog.h>
#include <gtkmm.h>
#include <gtkmm/main.h>
#include <libglademm.h>
#include <string>

#include "convfncs.h"
#include "translate.h"
#include "burza.h"
#include "bdialogs.h"

using namespace std;
using namespace Burza;

string BurzaDialogsFileHelper::FormatDateTimeFileName( string prefix, string extension )
{
	// offer save dialog with location of the document
	ConvFuncs::GetDateTime();
	string offeredName = 
		ConvFuncs::FormatStdStr( "%s_%02i-%02i-%02i_%02i-%02i.%s",
		prefix.c_str(),
		ConvFuncs::datetime.year % 100, ConvFuncs::datetime.month, ConvFuncs::datetime.day,
		ConvFuncs::datetime.hour, ConvFuncs::datetime.minute,
		extension.c_str() );

	return offeredName;
}
 
string BurzaDialogsFileHelper::AskToSaveDocumentFile( string fileName )
{
	Gtk::FileChooserDialog dlg( _("Cielovy subor"), Gtk::FILE_CHOOSER_ACTION_SAVE );
	
	dlg.set_current_folder( Centrala::Settings().m_document_ouput_directory );
	dlg.set_current_name( fileName );
	dlg.set_do_overwrite_confirmation();
	
	//Add response buttons the the dialog:
	dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dlg.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);
	
	int result = dlg.run();
	if ( result != Gtk::RESPONSE_OK ) {
		return "";
	}
	
	string name = dlg.get_filename();
	return name;
}

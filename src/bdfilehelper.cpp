#include "bdfilehelper.h"

#include <gtkmm.h>
// #include <filechooserdialog.h>
#include <string>

#include "convfncs.h"
#include "translate.h"
#include "burza.h"
#include "bdialogs.h"
#include "log.h"

using namespace std;
using namespace Burza;

std::string BurzaDialogsFileHelper::m_selected_file;

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

void BurzaDialogsFileHelper::on_file_dialog_finish(const Glib::RefPtr<Gio::AsyncResult>& result,
  const Glib::RefPtr<Gtk::FileDialog>& dialog)
{
  // Handle the response:
  try
  {
    auto file = dialog->open_finish(result);

    // Notice that this is a std::string, not a Glib::ustring.
    BurzaDialogsFileHelper::m_selected_file = file->get_path();
  }
  catch (const Gtk::DialogError& err)
  {
    // Can be thrown by dialog->open_finish(result).
    LogDebug("No file selected. ");
	LogDebug(err.what());
  }
  catch (const Glib::Error& err)
  {
    LogDebug("Unexpected exception.");
	LogDebug(err.what());
  }
}

string BurzaDialogsFileHelper::AskToSaveDocumentFile( string fileName )
{
	BurzaDialogsFileHelper::m_selected_file = "?";
	
	auto dialog = Gtk::FileDialog::create();

	auto ghandle = Gio::File::create_for_path(Centrala::Settings().m_document_ouput_directory);
	dialog->set_initial_folder( ghandle );
	dialog->set_initial_name( fileName );

	// Show the dialog and wait for a user response:
	dialog->open(sigc::bind(sigc::ptr_fun(&BurzaDialogsFileHelper::on_file_dialog_finish), dialog));
	
	return BurzaDialogsFileHelper::m_selected_file;

	/* Gtk::FileDialog dlg( _("Cielovy subor"), Gtk::FileChooserDialog::Action::SAVE );
	
	auto ghandle = Gio::File::create_for_path(Centrala::Settings().m_document_ouput_directory);
	dlg.set_current_folder( ghandle );
	dlg.set_current_name( fileName );
	// dlg.set_do_overwrite_confirmation();
	
	//Add response buttons the the dialog:
	dlg.add_button( "cancel", Gtk::ResponseType::CANCEL);
	dlg.add_button( "ok", Gtk::ResponseType::OK);
	
	Centrala::GetApp()->add_window( dlg );
	dlg.show();
	int result = dlg.get_result();
	if ( result != Gtk::ResponseType::OK ) {
		return "";
	}
	
	string name = dlg.get_filename();

	return name;
	*/
}

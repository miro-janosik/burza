#ifndef _edit_message_box_h_
#define _edit_message_box_h_

#include <gtkmm.h>
#include <gtkmm/main.h>
#include <libglademm.h>
#include <assert.h>

#include "singleton.h"

struct s_messageEditBoxWindowControls {
	Glib::RefPtr<Gnome::Glade::Xml> xmlWindow;
	Gtk::Dialog		*dialog;

	Gtk::Label		*labelText;
	Gtk::Entry		*entryText;
	Gtk::Button		*buttonOk;
	Gtk::Button		*buttonCancel;
} ;


class EditMessageBox : public Singleton< EditMessageBox >
{
private:
	static bool initialized;

	s_messageEditBoxWindowControls controls;

public:
	//! init
	static void Initialize( s_messageEditBoxWindowControls& fetchedControls )
	{
		if (Instance().initialized) {
			return;
		}

		Instance().initialized = true;
		Instance().controls = fetchedControls;
	}
	
	static bool ShowDialog( std::string message, std::string& edit )
	{
		assert( instance != NULL );
		assert( instance->initialized );

		instance->controls.labelText->set_text( message );
		instance->controls.entryText->set_text( edit );

		int res = instance->controls.dialog->run();

		edit = instance->controls.entryText->get_text();
		instance->controls.dialog->hide();

		return res == 0;
	}

} ;

#endif

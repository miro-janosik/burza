#ifndef _edit_message_box_h_
#define _edit_message_box_h_

#include <gtkmm.h>
#include <assert.h>

#include "singleton.h"

struct s_messageEditBoxWindowControls {
	Gtk::Window		*dialog;

	Gtk::Label		*labelText;
	Gtk::Entry		*entryText;
	Gtk::Button		*buttonOk;
	Gtk::Button		*buttonCancel;
} ;


class EditMessageBox : public Singleton< EditMessageBox >
{
private:
	static bool initialized;
	int resultButtonIndex;

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
	
	static bool ShowDialog( std::string message, std::string& edit );
	
	void EditMessageBoxClickedOk( void )
	{
		EditMessageBox::Instance().resultButtonIndex = 0;
	}

	void EditMessageBoxClickedCancel( void )
	{
		EditMessageBox::Instance().resultButtonIndex = 1;
	}

} ;

#endif

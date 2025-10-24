#include <string>
#include <assert.h>

#include <gtkmm.h>

#include "bdhelper.h"
#include "convfncs.h"

using namespace std;

void BurzaDialogsHelper::ChangeGtkmmAppFontSize( int fontSize )
{
	if (( fontSize <= 0 ) || ( fontSize > 50 ))
	{
		assert( false );
	}

	string str = ConvFuncs::FormatStdStr(
		"style \"my-style\" { font_name = \"Mono %i\" } class \"*\" style \"my-style\"",
		fontSize
		);

	// gtk_rc_parse_string( str.c_str() ); => Gtk::CssProvider()
}

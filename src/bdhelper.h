#ifndef _BurzaDialogsHelper_h_
#define _BurzaDialogsHelper_h_

#include <string>
#include <exception>

class BurzaDialogsHelper
{
public:

	//! Installs the application's static CSS on the default display.
	//! Call once, after the Gtk::Application exists and before the windows
	//! are shown. Provides the classes used by BurzaDialogs, currently:
	//!   .burza-invalid  - highlights a widget holding invalid input
	static void InstallAppCss( void );

	//! Changes font size of the application
	static void ChangeGtkmmAppFontSize( int size );

	//! CSS class put on widgets whose content did not validate.
	static const char* cssClassInvalid;
};

#endif

#include <string>
#include <assert.h>

#include <gtkmm.h>

#include "bdhelper.h"
#include "convfncs.h"
#include "log.h"

using namespace std;

// gtkmm provides GTKMM_CHECK_VERSION in gtkmmconfig.h, but define it from the
// version macros if this gtkmm does not, so the guards below always work.
#ifndef GTKMM_CHECK_VERSION
#define GTKMM_CHECK_VERSION( maj, min, mic )                                  \
	(( GTKMM_MAJOR_VERSION > (maj) ) ||                                       \
	 ( GTKMM_MAJOR_VERSION == (maj) && GTKMM_MINOR_VERSION > (min) ) ||       \
	 ( GTKMM_MAJOR_VERSION == (maj) && GTKMM_MINOR_VERSION == (min) &&        \
	   GTKMM_MICRO_VERSION >= (mic) ))
#endif

const char* BurzaDialogsHelper::cssClassInvalid = "burza-invalid";

// GTK 4 replaced gtk_rc_parse_string() and Gtk::Widget::modify_bg()/modify_base()
// with CSS. Two providers are kept so that changing the font size does not wipe
// out the static rules, and vice versa.
namespace {

Glib::RefPtr<Gtk::CssProvider>	staticCssProvider;
Glib::RefPtr<Gtk::CssProvider>	fontCssProvider;

// Static rules, reproducing what HighlightWidgetRed() did under GTK 2:
//     modify_bg  (Gtk::STATE_NORMAL, Gdk::Color("red"))   -> widget background
//     modify_base(Gtk::STATE_NORMAL, Gdk::Color("pink"))  -> text-entry area
// so a Gtk::Label turns red and the inside of a Gtk::Entry turns pink.
//
// Three things matter for this to actually take effect:
//  * In GTK 4 an editable widget is a shell node ('entry', 'spinbutton',
//    'textview') with the editable surface in a child 'text' node, so the
//    "base" colour has to be set on that child.
//  * The theme usually paints entries with a background-image gradient, which
//    would cover a plain background-color, hence 'background-image: none'.
//  * This provider is installed at GTK_STYLE_PROVIDER_PRIORITY_APPLICATION,
//    which outranks the theme's PRIORITY_THEME in GTK's cascade, so these
//    rules win over Adwaita regardless of selector specificity.
const char* staticCss =
	// "bg" equivalent: the widget's own background.
	".burza-invalid {"
	"  background-color: red;"
	"  background-image: none;"
	"  color: black;"
	"}"
	// "base" equivalent: the editable surface inside entries and friends.
	"entry.burza-invalid > text,"
	"spinbutton.burza-invalid > text,"
	"textview.burza-invalid > text,"
	".burza-invalid > text {"
	"  background-color: pink;"
	"  background-image: none;"
	"  color: black;"
	"}"
	// Some themes put the selection/placeholder on further child nodes; keep
	// them legible against pink rather than leaving them theme-coloured.
	".burza-invalid > text > placeholder {"
	"  color: #555555;"
	"}";

void LoadCss( const Glib::RefPtr<Gtk::CssProvider>& provider, const string& css )
{
#if GTKMM_CHECK_VERSION( 4, 12, 0 )
	provider->load_from_string( css );
#else
	provider->load_from_data( css );
#endif
}

void AddProviderForDefaultDisplay( const Glib::RefPtr<Gtk::CssProvider>& provider )
{
	Glib::RefPtr<Gdk::Display> display = Gdk::Display::get_default();
	if ( ! display ) {
		Log( InfoWarn, "No default Gdk::Display, CSS not installed.\n" );
		return;
	}

#if GTKMM_CHECK_VERSION( 4, 10, 0 )
	Gtk::StyleProvider::add_provider_for_display( display, provider,
		GTK_STYLE_PROVIDER_PRIORITY_APPLICATION );
#else
	Gtk::StyleContext::add_provider_for_display( display, provider,
		GTK_STYLE_PROVIDER_PRIORITY_APPLICATION );
#endif
}

} // anonymous namespace

void BurzaDialogsHelper::InstallAppCss( void )
{
	if ( staticCssProvider ) {
		return;
	}

	try {
		staticCssProvider = Gtk::CssProvider::create();
		LoadCss( staticCssProvider, staticCss );
		AddProviderForDefaultDisplay( staticCssProvider );
	}
	catch( const Glib::Error& e ) {
		LogS( InfoError, "Could not install application CSS: %s\n", e.what() );
		staticCssProvider.reset();
	}
}

void BurzaDialogsHelper::ChangeGtkmmAppFontSize( int fontSize )
{
	if (( fontSize <= 0 ) || ( fontSize > 50 ))
	{
		assert( false );
		return;
	}

	// Equivalent of the old GTK 2 resource string
	//   style "my-style" { font_name = "Mono <size>" } class "*" style "my-style"
	string css = ConvFuncs::FormatStdStr(
		"* { font-family: Monospace; font-size: %ipt; }",
		fontSize
		);

	try {
		if ( ! fontCssProvider ) {
			fontCssProvider = Gtk::CssProvider::create();
			LoadCss( fontCssProvider, css );
			AddProviderForDefaultDisplay( fontCssProvider );
		}
		else {
			// Already attached to the display; reloading the data is enough.
			LoadCss( fontCssProvider, css );
		}
	}
	catch( const Glib::Error& e ) {
		LogS( InfoError, "Could not apply font size %i: %s\n",
			fontSize, e.what() );
	}
}

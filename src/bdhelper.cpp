#include <assert.h>
#include "bdhelper.h"

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

	gtk_rc_parse_string( str );
}

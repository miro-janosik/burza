#include "errhandler.h"
#include "bdialogs.h"
#include "translate.h"
#include "log.h"

using namespace Burza;

void ErrorHandling::HandleException(const std::exception& e)
{
	LogS( InfoError, "HandleException: %s", e.what() );
	BurzaDialogs::DisplayErrorMessageBox( e.what(), _("Program nemohol vykonat operaciu.") );
}

void ErrorHandling::HandlePostgresException(const std::exception& e)
{
	LogS( InfoError, "HandlePostgresException: %s", e.what() );
	BurzaDialogs::DisplayErrorMessageBox( e.what(), _("Problem s databazou, prosim skuste opat.") );
}

bool ErrorHandling::HandlePdfExceptions( std::exception& e )
{
	std::string title;
	bool result = true;

	try {
		throw e;
	} catch( runtime_error& e ) {
		title = "PDF runtime error";
	} catch( out_of_range& e ) {
		title = "PDF out of range error";
	} catch( ... ) {
		title = "Unknown PDF error";
		result = false;
	}

	LogS( InfoError, "HandlePdfExceptions: '%s' - '%s'", title.c_str(), e.what() );
	BurzaDialogs::DisplayErrorMessageBox( title , e.what() );
	return result;
}

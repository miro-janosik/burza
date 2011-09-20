#include "errhandler.h"
#include "bdialogs.h"
#include "translate.h"
#include "log.h"

using namespace Burza;

void ErrorHandling::HandleException(std::exception& e)
{
	LogS( InfoError, "HandleException: %s", e.what() );
	BurzaDialogs::DisplayErrorMessageBox( e.what(), _("Program nemohol vykonat operaciu.") );
}

void ErrorHandling::HandlePostgresException(std::exception& e)
{
	LogS( InfoError, "HandlePostgresException: %s", e.what() );
	BurzaDialogs::DisplayErrorMessageBox( e.what(), _("Problem s databazou, prosim skuste opat.") );
}
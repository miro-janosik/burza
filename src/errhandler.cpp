#include <stdexcept>

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

bool ErrorHandling::HandlePdfExceptions(const std::exception& e )
{
	std::string title;
	bool result = true;

	// Do NOT re-throw to classify: `throw e;` copies through the
	// const std::exception& and slices the object down to std::exception, so
	// the runtime_error / out_of_range handlers never matched and every PDF
	// problem was reported as "Unknown PDF error". dynamic_cast on the
	// reference keeps the real type.
	if ( dynamic_cast< const std::out_of_range* >( &e ) != NULL ) {
		title = "PDF out of range error";
	}
	else if ( dynamic_cast< const std::runtime_error* >( &e ) != NULL ) {
		title = "PDF runtime error";
	}
	else {
		title = "Unknown PDF error";
		result = false;
	}

	LogS( InfoError, "HandlePdfExceptions: '%s' - '%s'\n", title.c_str(), e.what() );
	BurzaDialogs::DisplayErrorMessageBox( e.what(), title );
	return result;
}

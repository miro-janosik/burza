#ifndef _BurzaDialogsFileHelper_h_
#define _BurzaDialogsFileHelper_h_

#include <string>
#include <exception>

class BurzaDialogsFileHelper
{
public:

	//! Returns string in format "prefix_YY-MM-DD_HH_MM.extension"
	static std::string FormatDateTimeFileName( std::string prefix, std::string extension );

	//! Opens "save" dialog in the 'documents' directory, asks user to save.
	//! Returns selected filename with full path, or string.empty() if cancelled
	static std::string AskToSaveDocumentFile( std::string fileName );
};

#endif
#include <string>
#include <vector>

namespace Burza {

struct SettingFontSizeValue {
	std::string	text;
	int size;

	SettingFontSizeValue( std::string _text, int _size )
	{
		text = _text;
		size = _size;
	}
};

class PredefinedSettingVariables {
public:

	//! array of font sizes displayed in the settings list
	std::vector< SettingFontSizeValue >	listOfFontSizes;

	PredefinedSettingVariables();
} ;

class ProgramSettings {
public:
	ProgramSettings();
public:
	void LoadSettings( void );
	void SaveSettings( void );
	void Apply( void );

	//! for example "e" (euro sign), "Kè" (czech krown),
	std::string		currencySign;

	//! for example "EUR", "CZK", "SKK"
	std::string		currencyInternationalShort;

	//! directory from where files are imported 
	std::string		m_data_dir_path;
	
	//! name of current computer. Useful when identifying PC that changes seller.
	std::string		m_computer_name;
	
	//! Name of the sale, it is written in output documents.
	std::string		m_name_of_the_sale;
	
	//! directory where files are output
	std::string		m_document_ouput_directory;

	std::string		directoryImport;

	//! PostgreSql connection
	bool			databaseInLocalFiles;

	std::string		dbAddress;
	std::string		dbUser;
	std::string		dbPassword;
	std::string		dbName;
	int				dbPort;

	std::string		licenseTo;
	std::string		licenseKey;

	// -1 = system default
	int				fontSize;

public:
	static	PredefinedSettingVariables	predefined;

private:
	//! current exe path
	std::string			exePath;
} ;

} ; // end namespace Burza

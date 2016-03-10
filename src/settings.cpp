#include <string>
#include "settings.h"
#include "bdhelper.h"
#include "translate.h"
#include "convfncs.h"
#include "xmltool.h"
#include "errhandler.h"
#include "burza.h"
#include "log.h"
#include "bdialogs.h"

#include "cacheddatastorage.h"
#include "datapostgrespqxx.h"

using namespace std;
using namespace Burza;
using namespace XmlWrapperTool;

PredefinedSettingVariables	ProgramSettings::predefined;

PredefinedSettingVariables::PredefinedSettingVariables()
{
	listOfFontSizes.push_back( SettingFontSizeValue( _( "Prednastavený" ), -1 ) );

	for( int i = 4; i < 20; i += 2 )
		listOfFontSizes.push_back( SettingFontSizeValue( ConvFuncs::FormatStdStr( "%i", i ), i ) );
}

ProgramSettings::ProgramSettings()
{
	exePath = ConvFuncsInstance->GetPathFromPathFilename(
		ConvFuncsInstance->GetExecutableFilePathName() );
	m_data_dir_path = exePath;
	m_data_dir_path += "data/";

	m_document_ouput_directory = exePath;
	m_document_ouput_directory += "/documents/";
	
	// currencySign = "∈"; Replaced while PDF printer can't print the character
	currencySign = "eur";
	currencyInternationalShort = "EUR";
	m_computer_name = "PC123";
	m_name_of_the_sale = "Jarna burza 2012";

	databaseInLocalFiles = true;
	fontSize = -1;

	LoadSettings();
}

void ProgramSettings::LoadSettings( void )
{
	CXmlTreeSingleNode configRoot;
	string confFilename = exePath + "burza.xml";
	if ( CXmlWrapperTools::ReadTreeFromDiskFile( confFilename.c_str(), configRoot ) == 1 ) {
		ErrorHandling::HandleException( *new std::runtime_error(_("Súbor s nastavením nenájdený")) );
	} else {

		CXmlTreeSingleNode* config = configRoot.GetChild("burza")->GetChild("config");
		if (config->IsValid()) {
			m_name_of_the_sale = config->GetAttribute("salename")->GetValue();

			fontSize = config->GetAttribute("fontSize")->GetValueInt();
		}

		CXmlTreeSingleNode* server = config->GetChild("server");
		if (server->IsValid()) {

			CXmlTreeSingleNode* node;

			node = server->GetAttribute("databaseInLocalFiles");
			if (node->IsValid())
				databaseInLocalFiles = node->ValueEqualNoCase( "true" );

			node = server->GetAttribute("address");
			if (node->IsValid())
				dbAddress = node->GetValue();

			node = server->GetAttribute("user");
			if (node->IsValid())
				dbUser = node->GetValue();

			node = server->GetAttribute("password");
			if (node->IsValid())
			{
				std::string encodedPassword = node->GetValue();
				// decode password from config file
				dbPassword = DecodePassword( encodedPassword );
			}

			node = server->GetAttribute("dbname");
			if (node->IsValid())
				dbName = node->GetValue();

			node = server->GetAttribute("port");
			if (node->IsValid())
				dbPort = node->GetValueInt();
		}

		CXmlTreeSingleNode* license = config->GetChild("License");
		if (license->IsValid()) {

			CXmlTreeSingleNode* node;
			
			node = license->GetAttribute("to");
			if (node->IsValid())
				licenseTo = node->GetValue();

			node = license->GetAttribute("key");
			if (node->IsValid())
				licenseKey = node->GetValue();
		}
	}

	// check if font size is valid
	int i;
	for( i = 0; i < predefined.listOfFontSizes.size(); ++i )
		if ( predefined.listOfFontSizes[ i ].size == fontSize )
			break;
	
	if ( i == predefined.listOfFontSizes.size() )
		fontSize = -1;
}

void ProgramSettings::SaveSettings( void )
{
	CXmlTreeSingleNode configRoot;
	string confFilename = exePath + "burza.xml";
	CConvFile configFile;

	CXmlTreeSingleNode* config = configRoot.AddChild("burza")->AddChild("config");
	config->AddAttribute( "salename", m_name_of_the_sale.c_str() );
	config->AddAttribute( "fontSize" )->SetValueInt( fontSize );
	
	CXmlTreeSingleNode* server = config->AddChild("server");
	
	server->AddAttribute("databaseInLocalFiles", databaseInLocalFiles ? "True" : "False" );
	
	// encode password before storing it to config file
	std::string encodedPassword = EncodePassword( dbPassword );
	server->AddAttribute("address", dbAddress.c_str() );
	server->AddAttribute("user", dbUser.c_str() );
	server->AddAttribute("password", encodedPassword.c_str() );
	server->AddAttribute("dbname", dbName.c_str() );
	server->AddAttribute("port")->SetValueInt( dbPort );

	CXmlTreeSingleNode* license = config->AddChild("License");
	
	license->AddAttribute("to", licenseTo.c_str() );
	license->AddAttribute("key", licenseKey.c_str() );

	configFile.OpenFile( confFilename.c_str(), CConvFile::fm_text, CConvFile::fm_write );
	if (! configFile.IsOpened() ) {
		ErrorHandling::HandleException( *new std::runtime_error(_("Súbor s nastavením sa nedá zapísať.")) );
		return;
	}

	if ( CXmlWrapperTools::WriteSimpleXmlFile( configRoot, configFile ) != 0 ) {
		ErrorHandling::HandleException( *new std::runtime_error(_("Súbor s nastavením sa nedá vyplniť XML údajmi.")) );
		return;
	}
}

void ProgramSettings::Apply( void )
{
	// change font size
	if ( fontSize != -1 )
		BurzaDialogsHelper::ChangeGtkmmAppFontSize( fontSize );
	
	// change data connection

	if ( Centrala::Register().m_storage != NULL ) {
		delete Centrala::Register().m_storage;
		Centrala::Register().m_storage = NULL;
	}

	string errorString;

	try {
		DataAccessInterface	*dai;

		if ( databaseInLocalFiles ) {
			dai = new CachedDataTextfile( Centrala::Settings().m_data_dir_path );
		} else {
			dai = new DataPostgresPqxx();
		}

		Centrala::Register().m_storage = dai;
	}
	catch(DataStorageException &e)
	{
		errorString = e.what();
	}
	catch(...) {
		errorString = "Error initializing data connection";
	}

	if ( ! errorString.empty() ) {
		LogS( InfoError, errorString.c_str() );
		BurzaDialogs::DisplayErrorMessageBox( errorString, "Error initing dialog." );
		throw runtime_error( errorString );
	}
}

static std::string base64_encode(const std::string &in) 
{
    std::string out;

    int val=0, valb=-6;
    for (unsigned char c : in) {
        val = (val<<8) + c;
        valb += 8;
        while (valb>=0) {
            out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val>>valb)&0x3F]);
            valb-=6;
        }
    }
    if (valb>-6) out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val<<8)>>(valb+8))&0x3F]);
    while (out.size()%4) out.push_back('=');
    return out;
}

static std::string base64_decode(const std::string &in) 
{
    std::string out;

    std::vector<int> T(256,-1);
    for (int i=0; i<64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i; 

    int val=0, valb=-8;
    for (unsigned char c : in) {
        if (T[c] == -1) break;
        val = (val<<6) + T[c];
        valb += 6;
        if (valb>=0) {
            out.push_back(char((val>>valb)&0xFF));
            valb-=8;
        }
    }
    return out;
}

std::string ProgramSettings::EncodePassword(const std::string&  decoded)
{
	return base64_encode(decoded);
}

std::string ProgramSettings::DecodePassword(const std::string&  encoded)
{
	return base64_decode(encoded);
}

std::string ProgramSettings::HidePartOfPassword(const std::string&  pwd)
{
	std::string partial = pwd.substr(0, 1);
	partial += "[...]";
	return partial;
}

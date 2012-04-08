// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#ifndef _burza_h_
#define _burza_h_

#include <vector>
#include <string>
#include <map>

#include "dataaccess.h"

using namespace std;

class CConvFile;
class BurzaDialogs;

int main2( int argc, char **argv );

namespace Burza {
	
	class PdfWrapperBase;
	class ItemSearch;
	
class NastaveniaProgramu {
public:
	NastaveniaProgramu();
public:
	void LoadSettings( void );
	void SaveSettings( void );

	//! for example "EUR", "Czech krown", "E"
	std::string		m_string_mena; 
	
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
	std::string		dbAddress;
	std::string		dbUser;
	std::string		dbPassword;
	std::string		dbName;
	int				dbPort;

	std::string		licenseTo;
	std::string		licenseKey;

private:
	//! current exe path
	string			exePath;
} ;

// ==============================================
// ==============================================

class BurzaRegister {
public:
	
	NastaveniaProgramu		m_nastavenia;
	
	DataAccessInterface		*m_storage;
	
	BurzaRegister();
	~BurzaRegister();
	
public:
	
	SellerDataArray	m_predajcovia;
	
	void PridajNovehoPredajcu( const char* kod, const char* meno, const char* poznamka );
	
	//! returns -1 if not found, or index of seller.
	int NajdiPredajcuPodlaKoduIndex( const char* kod );
	SellerData NajdiPredajcuPodlaKodu( const char* kod );
	ItemCategory* FindProductCategory( char product_category );
	SellerCategory* FindSellerCategory( char seller_category );
	
	double CalcPriceOfProducts( ItemDataArray& items );
	
	//! marks products as sold
	void FinishCheckout( ItemDataArray& items );
	
public:
	
	unsigned int GenerateSellerUniqueId( void );

	//! Returns valid number for item. Looks through seller item numbers, and if other item
	//! has the same number, new item number is calculated.
	int VerifyItemNumber( ItemData& item, int newNumber );
	
	double CalculateFee( SellerData& seller, ItemDataArray& items );

	//! displays specified PDF file in system-specific PDF viewer
	int DisplayPdf( const char* fileName );

  private:
	ItemCategory	temporaryItemCategory;
	SellerCategory	temporarySellerCategory;
	
} ;

// ==============================================
class Centrala {
private:
	Centrala();
	~Centrala();
	Centrala( const Centrala& );
// nech je to aj singleton ziskatelny odvsadial
	static Centrala	*m_instance;
public:
	// vytvorenie a ziskanie instancie
	static Centrala* Instance( void );
	// zmazanie instancie
	static void Zmaz( void );
public:
	
	static NastaveniaProgramu&	Settings( void ) { return Instance()->m_settings; }
	static BurzaDialogs& Dialogs( void ) { return * Instance()->mp_dialogs; }
	static BurzaRegister& Register( void ) { return Instance()->m_register; }
	static DataAccessInterface& Storage( void ) { return * Instance()->m_register.m_storage; }
	static ItemSearch& Search( void ) { return * Instance()->itemSearch; }
	
	NastaveniaProgramu		m_settings;
	BurzaDialogs			*mp_dialogs;
	BurzaRegister			m_register;
	ItemSearch				*itemSearch;
} ;
	
	
} ; // end namespace Burza

#endif
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
#include "settings.h"

using namespace std;

class CConvFile;
class BurzaDialogs;

int main2( int argc, char **argv );

namespace Burza {
	
	class PdfWrapperBase;
	class ItemSearch;
	class Fee;
	
class BurzaRegister {
	friend class ProgramSettings;
	friend class Centrala;
public:
	
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

	//! Returns list of fees that user has to pay
	Fees& CalculateSellerFees( SellerData& seller, ItemDataArray& items );

	//! displays specified PDF file in system-specific PDF viewer
	int DisplayPdf( const char* fileName );

  private:
	ItemCategory	temporaryItemCategory;
	SellerCategory	temporarySellerCategory;
	
	DataAccessInterface		*m_storage;
	
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
	
	static ProgramSettings&	Settings( void ) { return Instance()->m_settings; }
	static BurzaDialogs& Dialogs( void ) { return * Instance()->mp_dialogs; }
	static BurzaRegister& Register( void ) { return Instance()->m_register; }
	static DataAccessInterface& Storage( void ) { return * Instance()->m_register.m_storage; }
	static ItemSearch& Search( void ) { return * Instance()->itemSearch; }
	
	ProgramSettings		m_settings;
	BurzaDialogs			*mp_dialogs;
	BurzaRegister			m_register;
	ItemSearch				*itemSearch;
} ;
	
	
} ; // end namespace Burza

#endif
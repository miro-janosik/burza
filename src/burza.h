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

// translate i18n macro
#define _( text ) ( text )

int main2( int argc, char **argv );

namespace Burza {
	
	class PdfWrapperBase;
	
class NastaveniaProgramu {
public:
	NastaveniaProgramu();
	
public:
	
	std::string		m_string_mena; 
	
	std::string		m_data_dir_path;
} ;

/*
class SellerXXX {
public:
	std::string		m_kod;
	std::string		m_meno;
	std::string		m_poznamka;
	char					m_kategoria;
	// ID generated through Register::GenerateSellerUniqueCode()
	unsigned int	m_unique_id;
	
	typedef		std::vector< Product >	t_products;
	
	t_products	m_products;
	
	
	Seller();
	
	/// ! if [cislo] == -1  it is replaced by next number
	void PridajTovar( int cislo, const char* nazov, double cena, char kategoria, e_product_status stav );
	void ZmazTovar( int cislo );
	int NajdiTovarIndex( int cislo );
	void ZmenStavTovaru( int cislo, e_product_status stav );
	int VerifikujCisloTovaru( Product& p, int newNumber = -1 );
	
	void SaveToTxtFile( CConvFile& file );
	void LoadFromTxtFile( CConvFile& file );
	void Save( void );
	
	void ReplaceCharactersInString( std::string& str, std::string& from, char to );
	void ReplaceCharactersInString( std::string& str, const char* from, char to );
	
	void PrintProductsToPDF( PdfWrapperBase& pdf );
	double CalculateFee( void );
	
	// key: category character; value: number of items of that category
	typedef std::map< char, int > t_items_in_categories;
	
	
	static char file_field_delimiters[];
} ;
*/

// ==============================================
// ==============================================

class BurzaRegister {
public:
	
	NastaveniaProgramu		m_nastavenia;
	
	DataStorage		*m_storage;
	
	BurzaRegister();
	~BurzaRegister();
	
public:
	
	SellerDataArray	m_predajcovia;

	/* shouldn't be used anymore
	// typedef		std::vector< Seller >	t_predajcovia;
	typedef		std::map< char, ProductCategory >	t_productCategories;
	typedef		std::map< char, SellerCategory >	t_sellerCategories;

	t_productCategories	m_productCategories;
	t_sellerCategories	m_sellerCategories;
	
	struct SellerProduct {
		string sellerCode;
		int productNumber;
		
		bool operator==(const SellerProduct& other) const
		{
			return (( sellerCode == other.sellerCode ) && ( productNumber == other.productNumber ));
		}
	} ;
	*/
	
	// typedef std::vector< SellerProduct > t_sellerProducts;
	
	void PridajNovehoPredajcu( const char* kod, const char* meno, const char* poznamka );
	// void NacitajVsetkychPredajcov( void );
	
	// t_predajcovia::iterator NajdiPredajcuPodlaKoduIt( const char* kod );
	
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
	void GenerateSellerUniqueCode( SellerData& seller, std::string try_this_code );

	//! Returns valid number for item. Looks through seller item numbers, and if other item
	//! has the same number, new item number is calculated.
	int VerifyItemNumber( ItemData& item, int newNumber );
	
	double CalculateFee( SellerData& seller, ItemDataArray& items );

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
	static DataStorage& Storage( void ) { return * Instance()->m_register.m_storage; }
	
	NastaveniaProgramu		m_settings;
	BurzaDialogs			*mp_dialogs;
	BurzaRegister			m_register;
} ;
	
	
} ; // end namespace Burza

#endif
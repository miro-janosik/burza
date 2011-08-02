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
	
	enum e_product_status {
		caka_na_prijatie,
		sklad,
		predane,
		vratene,
		dar,
		dohladat,
		nedefinovane,
		pocet_stavov
	} ;
	
class Product {
	
public:
	int					m_cislo;
	std::string		m_nazov;
	double				m_cena;
	char					m_kategoria;
	e_product_status	m_stav;
	
	Product();
	void Reset( void );
} ;

class SellerCategory {
public:
	char			m_category;
	double		m_fee_per_sale;
};

class ProductCategory {
public:
	char			m_category;
	double		m_fee_per_item;
};

class NastaveniaProgramu {
public:
	NastaveniaProgramu();
	
public:
	char	m_znak_stavu_v_subore[ pocet_stavov ];
	std::string		m_string_stavu[ pocet_stavov ];
	
	std::string		m_string_mena; 
	
	std::string		m_data_dir_path;
} ;

class Seller {
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
	
	t_items_in_categories GetItemsCategories( void );
	
	static char file_field_delimiters[];
} ;

// ==============================================
// ==============================================

class BurzaRegister {
public:
	
	NastaveniaProgramu		m_nastavenia;
	
	DataStorage		*m_storage;
	
	BurzaRegister();
	~BurzaRegister();
	
public:
	typedef		std::vector< Seller >	t_predajcovia;
	typedef		std::map< char, ProductCategory >	t_productCategories;
	typedef		std::map< char, SellerCategory >	t_sellerCategories;
	
	
	t_predajcovia	m_predajcovia;
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
	
	typedef std::vector< SellerProduct > t_sellerProducts;
	
	void PridajNovehoPredajcu( const char* kod, const char* meno, const char* poznamka );
	void NacitajVsetkychPredajcov( void );
	
	// t_predajcovia::iterator NajdiPredajcuPodlaKoduIt( const char* kod );
	
	//! returns -1 if not found, or index of seller.
	int NajdiPredajcuPodlaKoduIndex( const char* kod );
	SellerData NajdiPredajcuPodlaKodu( const char* kod );
	ProductCategory* FindProductCategory( char product_category );
	SellerCategory* FindSellerCategory( char seller_category );
	double CalcPriceOfProducts( t_sellerProducts& items );
	
	//! marks products as sold
	void FinishCheckout( t_sellerProducts& items );
	
	//! creates checkout receipt
	void PrintRecipeeAtEndOfCheckout( BurzaRegister::t_sellerProducts& items, PdfWrapperBase& pdf );
	
public:
	char Stav2Char( e_product_status stav );
	const char* Stav2String( e_product_status stav );
	e_product_status Char2Stav( char c );
	
	
	unsigned int GenerateSellerUniqueId( void );
	void GenerateSellerUniqueCode( Seller& seller, std::string try_this_code );
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
	static Centrala* Instancia( void );
	// zmazanie instancie
	static void Zmaz( void );
public:
	
	NastaveniaProgramu*	Nastavenia( void ) { return & m_nastavenia; }
	BurzaDialogs*	Dialogy( void );
	BurzaRegister* Register( void ) { return & m_register; }
	
	NastaveniaProgramu		m_nastavenia;
	BurzaDialogs					*mp_dialogs;
	BurzaRegister					m_register;
} ;
	
	
} ; // end namespace Burza

#endif
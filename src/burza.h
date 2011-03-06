// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#ifndef _burza_h_
#define _burza_h_

#include <vector>
#include <string>

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

class NastaveniaProgramu {
public:
	NastaveniaProgramu();
	
public:
	char	m_znak_stavu_v_subore[ pocet_stavov ];
	std::string		m_string_stavu[ pocet_stavov ];

} ;

class Seller {
public:
	std::string		m_kod;
	std::string		m_meno;
	std::string		m_poznamka;
	
	typedef		std::vector< Product >	t_products;
	
	t_products	m_products;
	
	Seller();

	void PridajTovar( int cislo, const char* nazov, double cena, char kategoria, e_product_status stav );
	void ZmazTovar( int cislo );
	void ZmenStavTovaru( int cislo, e_product_status stav );
	
	void SaveToTxtFile( CConvFile& file );
	void LoadFromTxtFile( CConvFile& file );

	void PrintProductsToPDF( PdfWrapperBase& pdf );
} ;

// ==============================================
// ==============================================

class BurzaRegister {
public:
	
	NastaveniaProgramu		m_nastavenia;
	
public:
	typedef		std::vector< Seller >	t_predajcovia; 
	
	t_predajcovia	m_predajcovia;
	
	void PridajNovehoPredajcu( const char* kod, const char* meno, const char* poznamka );
	
	void NacitajVsetkychPredajcov( void );
	
	t_predajcovia::iterator NajdiPredajcuPodlaKodu( const char* kod );
	
public:
	char Stav2Char( e_product_status stav );
	const char* Stav2String( e_product_status stav );
	e_product_status Char2Stav( char c );
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
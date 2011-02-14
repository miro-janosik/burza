// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

int main2( int argc, char **argv );

#include <gtkmm.h>
#include "libglademm.h"

class CConvFile;

namespace Burza {
	
class ComboBoxTextDer;
	
	enum stav_tovaru {
		sklad,
		predane,
		vratene,
		dar,
		dohladat,
		nedefinovane,
		pocet_stavov
	} ;
	
class tovar {
	
public:
	int					m_cislo;
	std::string		m_nazov;
	double				m_cena;
	char					m_kategoria;
	stav_tovaru	m_stav;
} ;

class NastaveniaProgramu {
public:
	NastaveniaProgramu();
	
public:
	char	m_znak_stavu_v_subore[ pocet_stavov ];
	std::string		m_string_stavu[ pocet_stavov ];

} ;

class predajca {
public:
	std::string		m_kod;
	std::string		m_meno;
	std::string		m_poznamka;

	typedef		std::vector< tovar >	t_tovary;
	
	t_tovary	m_tovar;

	void PridajTovar( int cislo, const char* nazov, double cena, char kategoria, stav_tovaru stav );
	void ZmazTovar( int cislo );
	void ZmenStavTovaru( int cislo, stav_tovaru stav );
	
	void UlozDoTxtSuboru( CConvFile& file );
	void NacitajZTxtSuboru( CConvFile& file );
} ;

// ==============================================
// ==============================================

class BurzaRegister {
public:
	
	NastaveniaProgramu		m_nastavenia;
	
public:
	typedef		std::vector< predajca >	t_predajcovia; 
	
	t_predajcovia	m_predajcovia;
	
	void PridajNovehoPredajcu( const char* kod, const char* meno, const char* poznamka );
	
	void NacitajVsetkychPredajcov( void );
	
	t_predajcovia::iterator NajdiPredajcuPodlaKodu( const char* kod );
	
public:
	char Stav2Char( stav_tovaru stav );
	const char* Stav2String( stav_tovaru stav );
	stav_tovaru Char2Stav( char c );
} ;

// ComboBoxText is class derived from ComboBox, I have to provide special constructor
// so I can load it up from glade
class ComboBoxTextDer : public Gtk::ComboBoxText {
public:
	ComboBoxTextDer( BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

} ;

// ==============================================
// ==============================================

class BurzaDialogs {
	
public:
	BurzaDialogs();
	
	// returns 0 on success.
	int NahrajDialogy( void );
	
	// vyplni komba v Registri aj Pokladni
	void VyplnPredajcovDoKomb( void );
	
	void RegisterZobrazPredajcu( const char* kod );
	void RegisterZobrazPredajcuCezIndex( unsigned int idx );
	
	void PokladnaVyplnTovarDoKomba( void );
	void PokladnaZmenaPredajcu( void );
	
public:
	
	// "register" dialog functions
	void ButtonRegisterImportSeller( void );
	void ButtonRegisterAddSeller( void );
	void ButtonRegisterUpdateSeller( void );
	void ButtonRegisterAddItem( void );
	void ButtonRegisterRemoveItem( void );
	void ButtonRegisterCalcFee( void );
	void ButtonRegisterCalcRepayment( void );
	void ButtonSwitchToCashout( void );
	void ComboChangedSeller( void );
	void TreeSelectChangedItem( void );
	
	// "cash-out" dialog functions
	void ButtonSwitchToRegister( void );
	void ButtonCashoutAddItem( void );
	void ButtonCashoutFinished( void );
	void ButtonCashoutRemoveItem( void );
	void ComboCashoutLookupSeller( void );
	void ComboCashoutLookupItem( void );
	
	
public:
	BurzaRegister			m_register;
	
	Glib::RefPtr<Gnome::Glade::Xml> 	xmlWndRegister, xmlWndCashOut;
		
	Gtk::Window		*m_pWndRegister, *m_pWndCashOut;
	
	// register dialog
	ComboBoxTextDer	*m_pComboSeller;
	Gtk::Button		*m_pButtonImportSeller;
	Gtk::Button		*m_pButtonAddSeller;
	Gtk::Button		*m_pButtonUpdateSeller;
	Gtk::TreeView	*m_pTreeViewItems;
	Gtk::Button		*m_pButtonAddItem;
	Gtk::Button		*m_pButtonRemoveItem;
	Gtk::Button		*m_pButtonCalcFee;
	Gtk::Button		*m_pButtonCalcRepayment;
	Gtk::Label		*m_pLabelFee;
	Gtk::Label		*m_pLabelSellerNote;
	Gtk::Button		*m_pButtonCashout;
	
	Glib::RefPtr<Gtk::TreeSelection>	m_RegisterItemsSelection;
	Glib::RefPtr<Gtk::ListStore>	m_RegisterItemsListModel;
	
	// cash-out dialog
	Gtk::Button		*m_pButtonRegister;
	Gtk::TreeView	*m_pCashoutTreeViewList;
	Gtk::Entry			*m_pCashoutEntryLookupSeller;
	ComboBoxTextDer	*m_pCashoutComboLookupSeller;
	Gtk::Entry			*m_pCashoutEntryLookupItem;
	ComboBoxTextDer	*m_pCashoutComboLookupItem;
	Gtk::Label			*m_pCashoutLabelItemPrice;
	Gtk::Button		*m_pCashoutButtonAddItem;
	Gtk::Button		*m_pCashoutButtonRemoveItem;
	Gtk::Label			*m_pCashoutLabelSum;
	Gtk::Button		*m_pCashoutButtonFinish;
	
	// helper variables for dialogs
	
	Glib::RefPtr<Gtk::TreeSelection>	m_CashoutItemsSelection;
	Glib::RefPtr<Gtk::ListStore>	m_CashoutItemsListModel;
	
	// columns for items tree view
	class ItemsColumns : public Gtk::TreeModel::ColumnRecord {
	public:
		ItemsColumns()
			{ add(m_cislo); add(m_nazov); add(m_cena); add(m_kategoria); add(m_stav); add( m_predajca_idx ); add( m_tovar_idx ); }
		Gtk::TreeModelColumn<std::string> m_cislo;
		Gtk::TreeModelColumn<std::string> m_nazov;
		Gtk::TreeModelColumn<std::string> m_cena;
		Gtk::TreeModelColumn<std::string> m_kategoria;
		Gtk::TreeModelColumn<std::string> m_stav;
		Gtk::TreeModelColumn<int> m_predajca_idx;
		Gtk::TreeModelColumn<int> m_tovar_idx;
	};
	
	ItemsColumns	m_ItemsColumn;
	
	// predajca, ktory je vybrany v Pokladnicnom okne v kombo-boxe.
	predajca		*m_pCashoutVybranyPredajca;
	tovar		*m_pCashoutVybranyTovar;
	int		m_CashoutVybranyPredajcaIdx;
	int		m_CashoutVybranyTovarIdx;
	
	float		m_cashout_sum;
	
} ;

class Centrala {
private:
	Centrala();
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
	BurzaDialogs*	Dialogy( void ) { return & m_dialogy; }
	BurzaRegister* Register( void ) { return mp_register; }
	
	NastaveniaProgramu		m_nastavenia;
	BurzaDialogs					m_dialogy;
	
	BurzaRegister					*mp_register;
} ;

} ; // end namespace Burza

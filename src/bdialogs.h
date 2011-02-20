// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#include <gtkmm.h>
#include <gtkmm/main.h>
#include <libglademm.h>
#include <string>

#include "burza.h"

using namespace std;

// ComboBoxText is class derived from ComboBox, I have to provide special constructor
// so I can load it up from glade
class ComboBoxTextDer : public Gtk::ComboBoxText {
public:
	ComboBoxTextDer( BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);

} ;

class BurzaDialogs {
	
public:
	BurzaDialogs();
	~BurzaDialogs();
	
	// returns 0 on success.
	int NahrajDialogy( void );
	
	// vyplni komba v Registri aj Pokladni
	void VyplnPredajcovDoKomb( void );
	
	void RegisterZobrazPredajcu( const char* kod );
	void RegisterZobrazPredajcuCezIndex( unsigned int idx );
	
	void PokladnaVyplnTovarDoKomba( void );
	void PokladnaZmenaPredajcu( void );
	
	// Displays message box with message. Function returns only after you click OK on the box.
	void DisplayErrorMessageBox( string msg, string msg_title = NULL );
	
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
			{ add(m_cislo); add(m_nazov); add(m_cena); add(m_kategoria); add(m_stav); add( m_predajca_idx ); add( m_Product_idx ); }
		Gtk::TreeModelColumn<std::string> m_cislo;
		Gtk::TreeModelColumn<std::string> m_nazov;
		Gtk::TreeModelColumn<std::string> m_cena;
		Gtk::TreeModelColumn<std::string> m_kategoria;
		Gtk::TreeModelColumn<std::string> m_stav;
		Gtk::TreeModelColumn<int> m_predajca_idx;
		Gtk::TreeModelColumn<int> m_Product_idx;
	};
	
	ItemsColumns	m_ItemsColumn;
	
	// Seller, ktory je vybrany v Pokladnicnom okne v kombo-boxe.
	Seller		*m_pCashoutVybranyPredajca;
	Product		*m_pCashoutVybranyTovar;
	int		m_CashoutVybranyPredajcaIdx;
	int		m_CashoutVybranyTovarIdx;
	
	float		m_cashout_sum;
	
	string	m_working_directory;
} ;


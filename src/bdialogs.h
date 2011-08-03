// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#include <string>

#include <gtkmm.h>
#include <gtkmm/main.h>
#include <libglademm.h>

#include "burza.h"
#include "dataaccess.h"

using namespace std;

namespace Burza {
// class Seller;
// class Product;
class BurzaRegister;
};

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
	void DisplayErrorMessageBox( string msg, string msg_title = "" );
	void DisplayErrorMessageBox( const char* msg, const char* msg_title = NULL );
	//! returns true if Yes is pressed, false otherwise (you can expect that either No or 'x' was pressed.
	bool DisplayYesNoMessageBox( string message, string title );
	
	void StartDialogEditProduct( Burza::ItemData& product, Burza::SellerData& seller );
	void StartDialogEditSeller( Burza::SellerData& seller );
	void SellerModified( Burza::SellerData& seller );
	
public:
	
	// "register" dialog functions
	void ButtonRegisterImportSeller( void );
	void ButtonRegisterAddSeller( void );
	void ButtonRegisterUpdateSeller( void );
	void ButtonRegisterAddItem( void );
	void ButtonRegisterModifyItem( void );
	void ButtonRegisterRemoveItem( void );
	void ButtonRegisterCalcFee( void );
	void ButtonRegisterCalcRepayment( void );
	void ButtonSwitchToCashout( void );
	void ComboRegisterChangedSeller( void );
	void TreeSelectChangedItem( void );
	
	void RegisterGetSelectedItemNumbers( vector<unsigned int>& listOfItems );
	
	//! returns currently displayed seller in Register window, or NULL if none selected.
	Burza::SellerData* RegisterGetSelectedSeller( void );
	
	// Goes through selector's selected items in the list and returns array of indexes of selected items in the list.
	static void GetSelectedItemNumbers( Glib::RefPtr<Gtk::TreeSelection>& selector, vector<unsigned int>& listOfItems );
	
	// "cash-out" dialog callback functions
	void ButtonSwitchToRegister( void );
	void ButtonCashoutAddItem( void );
	void ButtonCashoutFinished( void );
	void ButtonCashoutRemoveItem( void );
	void ComboCashoutLookupSeller( void );
	void ComboCashoutLookupItem( void );
	void CashoutEditLookupSellerChanged( void );
	void CashoutEditLookupItemChanged( void );
	void CashoutComboLookupSellerChanged( void );
	void CashoutComboLookupItemChanged( void );
	void CashoutEditSearchChanged( void );
	void CashoutEditSearchEntryChanged( void );
	void CashoutSelectedItemChanged( void );
	
	// "cash-out" helper functions
	void CashoutZobrazPredajcu( const char* kod );
	void CheckoutCartChanged( void );
	
	// "item properties dialog functions
	//! Item editing finished. Fill item back to seller, and notify that seller was modified.
	void ButtonItemEditOk( void );
	void ButtonItemEditCancel( void );
	bool ButtonItemEditClose( GdkEventAny* );
	void ComboItemEditStatusChanged( void );
	
	// "seller" edit dialog
	void ButtonSellerEditOk( void );
	void ButtonSellerEditCancel( void );
	bool ButtonSellerEditClose( GdkEventAny* );
	
public:
	Burza::BurzaRegister			*mp_register;
	
	Glib::RefPtr<Gnome::Glade::Xml> 	xmlWndRegister, xmlWndCashOut, 
 			xmlWndModifySeller, xmlWndModifyItem;
		
	Gtk::Window		*m_pWndRegister, *m_pWndCashOut, 
 			*m_pWndSeller, *m_pWndItem;
	
	// register dialog
	ComboBoxTextDer	*m_pComboSeller;
	Gtk::Button		*m_pButtonImportSeller;
	Gtk::Button		*m_pButtonAddSeller;
	Gtk::Button		*m_pButtonUpdateSeller;
	Gtk::TreeView	*m_pTreeViewItems;
	Gtk::Button		*m_pButtonAddItem;
	Gtk::Button		*m_pButtonModifyItem;
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
	ComboBoxTextDer	*m_pCashoutComboSearch;
	Gtk::Entry			*m_pCashoutComboSearchEntry;
	
	// item details edit dialog
	Gtk::Entry			*m_pItemeditName;
	Gtk::Entry			*m_pItemeditNumber;
	Gtk::Entry			*m_pItemeditPrice;
	Gtk::Entry			*m_pItemeditCategory;
	ComboBoxTextDer	*m_pItemeditStatusCombo;
	Gtk::Button		*m_pItemeditOk;
	Gtk::Button		*m_pItemeditCancel;
	
	// seller details edit dialog
	Gtk::Entry			*m_pSellerEditName;
	Gtk::Entry			*m_pSellerEditCode;
	Gtk::Entry			*m_pSellerEditNote;
	Gtk::Button		*m_pSellerEditOk;
	Gtk::Button		*m_pSellerEditCancel;
	
	// helper variables for dialogs
	
	Glib::RefPtr<Gtk::TreeSelection>	m_CashoutItemsSelection;
	Glib::RefPtr<Gtk::ListStore>	m_CashoutItemsListModel;
	
	// columns for items tree view
	class ItemsColumns : public Gtk::TreeModel::ColumnRecord {
	public:
		ItemsColumns()
			{ add(m_number); add(m_name); add(m_price); add(m_category); add(m_state); add( m_seller_id ); add( m_item_id ); }
		Gtk::TreeModelColumn<std::string> m_number;
		Gtk::TreeModelColumn<std::string> m_name;
		Gtk::TreeModelColumn<std::string> m_price;
		Gtk::TreeModelColumn<std::string> m_category;
		Gtk::TreeModelColumn<std::string> m_state;
		Gtk::TreeModelColumn<int> m_seller_id;
		Gtk::TreeModelColumn<int> m_item_id;
	};
	
	ItemsColumns	m_ItemsColumn;
	
	// Seller, ktory je vybrany v Pokladnicnom okne v kombo-boxe.
	Burza::SellerData		*m_pCashoutVybranyPredajca;
	Burza::ItemData		*m_pCashoutVybranyTovar;
	int		m_CashoutVybranyPredajcaIdx;
	int		m_CashoutVybranyTovarIdx;
	
	float		m_cashout_sum;
	
	// Register
	Burza::SellerData*		m_pRegisterVybranyPredajca;
	
	// If user is editing some item, its pointer should be here, so it can be saved after modified.
	// edited product
	Burza::ItemData* m_productEdited;
	// edited seller
	Burza::SellerData* m_sellerEdited;
	
	enum edit_action {
		ea_add,
		ea_modify,
	} ;
	// what is going on with edited item
	edit_action	m_productEditAction;
	edit_action	m_sellerEditAction;
	
	// == Checkout ==
	Burza::ItemDataArray		m_shoppingCart;
	
} ;


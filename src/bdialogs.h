// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#include <string>

#include <gtkmm.h>
#include <gtkmm/main.h>
#include <libglademm.h>
#include <set>

#include "burza.h"
#include "dataaccess.h"

using namespace std;
using namespace Burza;

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
	int SetupDialogs( void );
	
	// vyplni komba v Registri aj Pokladni
	void FillSellerCombos( void );
	void ReloadSellers( void );
	
	void RegisterDisplaySeller( SellerData& seller );
	void RegisterDisplaySeller( const char* kod );
	void RegisterDisplaySellerByIndex( unsigned int idx );
	
	void CashoutSellerChanged( void );
	
	// Displays message box with message. Function returns only after you click OK on the box.
	static void DisplayErrorMessageBox( string msg, string msg_title = "" );
	static void DisplayErrorMessageBox( const char* msg, const char* msg_title = NULL );
	//! returns true if Yes is pressed, false otherwise (you can expect that either No or 'x' was pressed.
	bool DisplayYesNoMessageBox( string message, string title );
	void DisplayOkMessageBox( string message, string title );
	
	void RegisterStartDialogEditProduct( ITEM_ID item );
	void RegisterStartDialogEditSeller( SELLER_ID id );
	void RegisterSellerModified( Burza::SellerData& seller );
	
	//! highlights widget's background and edge color
	void HighlightWidgetRed( Gtk::Widget& widget );

	//! returns widget's color to normal
	void HighlightWidgetOff( Gtk::Widget& widget );

	//! stop highlighting items on the control
	void HighlightEditSellerOff( void );
	void HighlightEditItemOff( void );
	
	//! Compares two sellers by their short code and returns true if a > b
	static bool OrderSellerByShortCode(const SellerData& a, const SellerData& b);

public:
	
	// "register" dialog functions
	void RegisterButtonImportSeller( void );
	void RegisterButtonAddSeller( void );
	void RegisterButtonUpdateSeller( void );
	void RegisterButtonAddItem( void );
	void RegisterButtonModifyItem( void );
	void RegisterButtonRemoveItem( void );
	void RegisterButtonCalcFee( void );
	void RegisterButtonCalcRepayment( void );
	void RegisterButtonSwitchToCashout( void );
	void RegisterComboChangedSeller( void );
	void RegisterButtonReviewOfSellers( void );
	void RegisterButtonSettings( void );
	
	void CashoutItemsTreeSelectChanged( void );
	
	void RegisterGetSelectedItemIds( vector<Burza::ITEM_ID>& listOfIds );
	
	// Goes through selector's selected items in the list and returns array of indexes of selected items in the list.
	static void GetSelectedItemNumbers( Glib::RefPtr<Gtk::TreeSelection>& selector, vector<unsigned int>& listOfItems );
	
	// "cash-out" dialog callback functions
	void CashoutButtonSwitchToRegister( void );
	void CashoutButtonAddItem( void );
	void CashoutButtonFinished( void );
	void CashoutButtonRemoveItem( void );
	void CashoutComboLookupSeller( void );
	void CashoutComboLookupItem( void );
	void CashoutEditLookupSellerChanged( void );
	void CashoutEditLookupItemChanged( void );
	void CashoutComboLookupSellerChanged( void );
	void CashoutComboLookupItemChanged( void );
	void CashoutEditSearchChanged( void );
	void CashoutEditSearchEntryChanged( void );
	void CashoutSelectedItemChanged( void );

	void SettingsClickedButtonRequestLicence( void );
	void SettingsClickedButtonInstallLicence( void );
	void EditMessageBoxClickedOk( void );
	void EditMessageBoxClickedCancel( void );
	
	bool MessageEditBox( std::string message, std::string& edit);

	// "cash-out" helper functions
	void CashoutDisplaySeller( const char* shortcode );
	void CheckoutCartChanged( void );
	
	// "item properties dialog functions
	//! Item editing finished. Fill item back to seller, and notify that seller was modified.
	void RegisterButtonItemEditOk( void );
	void RegisterButtonItemEditCancel( void );
	bool RegisterButtonItemEditClose( GdkEventAny* );
	void RegisterComboItemEditStatusChanged( void );

	// "seller" edit dialog
	void RegisterButtonSellerEditOk( void );
	void RegisterButtonSellerEditCancel( void );
	bool RegisterButtonSellerEditClose( GdkEventAny* );
	
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
	Gtk::Button		*m_pRegisterButtonReviewOfSellers;
	Gtk::Button		*m_pButtonSettings;
	
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
	Gtk::Entry		*m_pSellerEditName;
	Gtk::Entry		*m_pSellerEditCode;
	Gtk::Entry		*m_pSellerEditNote;
	Gtk::Button		*m_pSellerEditOk;
	Gtk::Button		*m_pSellerEditCancel;
	
	struct {
		Glib::RefPtr<Gnome::Glade::Xml> xmlWindow;
		Gtk::Dialog		*window;

		Gtk::Label		*labelText;
		Gtk::Entry		*entryText;
		Gtk::Button		*buttonOk;
		Gtk::Button		*buttonCancel;
	} messageEditBoxWindowControls;

	struct {
		Glib::RefPtr<Gnome::Glade::Xml> xmlWindow;
		Gtk::Window		*window;

		Gtk::Label		*labelCurrentLicence;
		Gtk::Button		*buttonRequestLicence;
		Gtk::Button		*buttonInstallLicence;
	} settingsWindow;

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
	
	// == unknown ==
	
	ItemsColumns	m_ItemsColumn;
	
	// === register + cashout ===
	
	//! sellers displayed in the program, in combo boxes:
	//! m_pComboSeller, m_pCashoutComboLookupSeller
	//! non-sorted array so it is alignable to visual lists.
	SellerDataArray m_DisplayedSellers;
	
	// === register ===

	//! items displayed in the register item list.
	// ItemDataArray m_RegisterDisplayedItems;
	
	// Register
	SELLER_ID		m_RegisterSelectedSellerId;
	
	bool 	RegisterAnySelectedSeller( void );
	SellerData 	RegisterGetSelectedSeller( void );
	SELLER_ID	RegisterGetSelectedSellerId( void );
	
	// If user is editing some item, its pointer should be here, so it can be saved after modified.
	// edited product
	Burza::ItemData		m_EditedItem;
	// edited seller
	Burza::SellerData	m_EditedSeller;
	
	enum edit_action {
		ea_add,
		ea_modify,
	} ;
	// what is going on with edited item
	// edit_action	m_productEditAction;
	// edit_action	m_sellerEditAction;
	

	// === cashout ===

	float		m_cashout_sum;

	//! items displayed in the cashout
	ItemDataArray m_CashoutDisplayedItems;
	
	// Seller, ktory je vybrany v Pokladnicnom okne v kombo-boxe.
	SELLER_ID		m_CashoutSelectedSellerId;
	ITEM_ID			m_CashoutSelectedItemId;
	
	bool 	CashoutAnySelectedSeller( void ) 
		{ return m_CashoutSelectedSellerId != SELLER_ID_NONE; }
		
	SellerData 	CashoutGetSelectedSeller( void );
	
	SELLER_ID	CashoutGetSelectedSellerId( void )
		{ return m_CashoutSelectedSellerId; }


	bool 	CashoutAnySelectedItem( void ) 
		{ return m_CashoutSelectedItemId != ITEM_ID_NONE; }
		
	ItemData 	CashoutGetSelectedItem( void );
	
	ITEM_ID	CashoutGetSelectedItemId( void )
		{ return m_CashoutSelectedItemId; }

	typedef vector<ITEM_ID>		ShoppingCartArray;
	typedef ShoppingCartArray::iterator ShoppingCartIterator;
	
	//! array of items; array is non-sorted so it can be assigned to visual list.
	ShoppingCartArray			m_shoppingCart;

	// array of the items linked with the m_pCashoutComboSearch combo-box.
	// all the data that are in the combo box, are also here, in same order.
	ItemDataArray	cashoutComboSearchItemDatas;
	
} ;


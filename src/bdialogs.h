// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#include <string>
#include <set>

#include <gtkmm.h>
#include <gdk/gdk.h>

#include "burza.h"
#include "dataaccess.h"
#include "EditMessageBox.h"
#include "LoadingMultipleFiles.h"

using namespace std;
using namespace Burza;

	class CellItem_Bug
	{
	public:
	  CellItem_Bug();
	  CellItem_Bug(bool fixed);
	  CellItem_Bug(const CellItem_Bug& src);
	  ~CellItem_Bug();
	  CellItem_Bug& operator=(const CellItem_Bug& src);

	  bool m_fixed;
	};


class BurzaDialogs {

	
public:
	BurzaDialogs();
	~BurzaDialogs();
	
	// returns 0 on success.
	int SetupDialogs( void );
private:
	int LoadDialogs( Glib::RefPtr<Gtk::Builder>& builder );
public:

	// vyplni komba v Registri aj Pokladni
	void FillSellerCombos( void );
	void ReloadSellers( void );

	// Changes the seller combo-box for filtering by name.
	// Sellers matching the name will be first in the list
	// Also jumps to first seller
	// Behavior depends on length of partOfName. 
	// * length <= 0, 1 then no filtering is done
	// * length >= 2 then filter is applied
	void RegisterSortSellersByName( string& partOfName );
	
	void RegisterDisplaySeller( SellerData& seller );
	void RegisterDisplaySeller( const char* kod );
	void RegisterDisplaySellerByIndex( unsigned int idx );
	void RegisterDisplaySellerUpdateSellerStatus( Burza::SellerStatus status );
	
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
	void RegisterEditLookupSellerChanged( void );
	void RegisterButtonImportSeller( void );
	void RegisterButtonImportSeller_ContinueWithFileSelected( const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gtk::FileDialog>& dialog );
	void RegisterButtonImportMultiSellers( void );
	void RegisterButtonAddSeller( void );
	void RegisterButtonUpdateSeller( void );
	void RegisterButtonFinishAcceptingItems( void );
	void RegisterButtonAddItem( void );
	void RegisterButtonModifyItem( void );
	void RegisterButtonRemoveItem( void );
	void RegisterButtonAcceptUnaccept( void );
	void RegisterButtonCalcFee( void );
	void RegisterButtonCalcRepayment( void );
	void RegisterButtonSwitchToCashout( void );
	void RegisterComboChangedSeller( void );
	void RegisterButtonReviewOfSellers( void );
	//! Actions behind the "Iné akcie" menu. Registered as Gio::SimpleAction in
	//! the "burza" action group, so they take and return nothing.
	void RegisterButtonStats( void );
	void RegisterButtonSaveSellerOverview( void );
	void RegisterButtonSettings( void );
	void RegisterButtonFixItemCats( void );
	void RegisterSellerStatusRadioChanged( void );

	void CashoutItemsTreeSelectChanged( void );
	
	void RegisterGetSelectedItemIds( vector<Burza::ITEM_ID>& listOfIds );

	void CreateStatusImages( void );

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
	void SettingsClickedButtonOk( void );

	bool MessageEditBox( std::string message, std::string& edit);

	// "cash-out" helper functions
	void CashoutDisplaySeller( const char* shortcode );
	void CheckoutCartChanged( void );

	//! Checks for license limits, and displays message box if new can't be added.
	bool DemoLimitsCanAddItem( SELLER_ID sellerId );
	//! Checks for license limits, and displays message box if new can't be added.
	bool DemoLimitsCanAddSeller( void );
	//! Returns sellers, limited by numbers if in demo mode.
	SellerDataArray DemoLimitsGetSellers( void );
	//! Returns items, limited by numbers if in demo mode.
	ItemDataArray DemoLimitsGetSellerItems( SELLER_ID sellerId );

	//! When user does some action that navigates away from seller, and it
	//! is not accepted yet, ask about it.
	bool RegisterAskToFinishAcceptingSellerChanges( void );

	// "item properties dialog functions
	//! Item editing finished. Fill item back to seller, and notify that seller was modified.
	void RegisterButtonItemEditOk( void );
	void RegisterButtonItemEditCancel( void );
	bool RegisterButtonItemEditClose( void );
	void RegisterComboItemEditStatusChanged( void );

	// "seller" edit dialog
	void RegisterButtonSellerEditOk( void );
	void RegisterButtonSellerEditCancel( void );
	bool RegisterButtonSellerEditClose( void );
	
private:
	// helper methods:

	// Logs message from multi seller importer. Logs to file or to screen
	void ImportMultiSellersLog(const char* text);

	// Logs message from multi seller importer. Logs to file or to screen
	void ImportMultiSellersLog(string text);

	// Sort function that orders row by numeric value of the text
	// Used for number/price columns
	//int SortRegisterItemsNumberFunc(const Gtk::TreeIter& a, const Gtk::TreeIter& b);
	//int SortRegisterItemsPriceFunc(const Gtk::TreeIter& a, const Gtk::TreeIter& b);
	
public:
	Burza::BurzaRegister			*mp_register;
	
	//! value causing that actions bound to changes of GUI are not executed
	//! (for example checking if seller is already checked in, on change of seller combo.)
	bool initializing;

	//! The Builder OWNS every object it created, and get_widget() hands back a
	//! borrowed pointer. If the builder dies, everything it built that nothing
	//! else references is finalised, leaving all the pointers below dangling.
	//! Only m_pWndRegister gets an extra reference (Application::add_window),
	//! so this must stay alive for as long as the dialogs are used.
	Glib::RefPtr<Gtk::Builder>	m_refBuilder;

	Gtk::Window		*m_pWndRegister, *m_pWndCashOut, *m_pWndSeller, *m_pWndItem;

	// register dialog
	Gtk::DropDown	*m_pComboSeller;
	Gtk::Entry		*m_pEntryRegistryLookupSeller;
	Gtk::Button		*m_pButtonImportSeller;
	Gtk::Button		*m_pButtonImportMultipleSellers;
	Gtk::Button		*m_pButtonAddSeller;
	Gtk::Button		*m_pButtonUpdateSeller;
	Gtk::TreeView	*m_pTreeViewItems;
	Gtk::Button		*m_pButtonAddItem;
	Gtk::Button		*m_pButtonModifyItem;
	Gtk::Button		*m_pButtonRemoveItem;
	Gtk::Button		*m_pButtonAcceptUnaccept;
	Gtk::Button		*m_pButtonCalcFee;
	Gtk::Button		*m_pButtonCalcRepayment;
	Gtk::Label		*m_pLabelSellerType;
	Gtk::Label		*m_pLabelSellerNote;
	Gtk::Button		*m_pButtonCashout;
	Gtk::Button		*m_pRegisterButtonReviewOfSellers;
	Gtk::Button		*m_pButtonSettings;
	Gtk::MenuButton	*m_pRegisterOtherActions;
	Gtk::CheckButton	*m_pRegisterRadioSellerStatus1NotImported;
	Gtk::CheckButton	*m_pRegisterRadioSellerStatus2ImportedSelling;
	Gtk::CheckButton	*m_pRegisterRadioSellerStatus3ReturnedBack;
	
	Glib::RefPtr<Gtk::TreeSelection>	m_RegisterItemsSelection;
	Glib::RefPtr<Gtk::ListStore>	m_RegisterItemsListModel;

	Glib::RefPtr<Gio::Menu>		menuRegisterOtherActions;

	//! Holds the actions the "Iné akcie" menu items refer to ("burza." prefix).
	//! Must stay alive as long as the Register window does.
	Glib::RefPtr<Gio::SimpleActionGroup>	registerActionGroup;
	// The menu's entries are GMenu <item>s driven by actions in
	// registerActionGroup, not widgets, so there are no pointers to keep here.
	
	// cash-out dialog
	Gtk::Button		*m_pButtonRegister;
	Gtk::TreeView	*m_pCashoutTreeViewList;
	Gtk::Entry			*m_pCashoutEntryLookupSeller;
	Gtk::DropDown	*m_pCashoutComboLookupSeller;
	Gtk::Entry			*m_pCashoutEntryLookupItem;
	Gtk::DropDown	*m_pCashoutComboLookupItem;
	Gtk::Label			*m_pCashoutLabelItemPrice;
	Gtk::Button		*m_pCashoutButtonAddItem;
	Gtk::Button		*m_pCashoutButtonRemoveItem;
	Gtk::Label			*m_pCashoutLabelSum;
	Gtk::Button		*m_pCashoutButtonFinish;
	Gtk::DropDown	*m_pCashoutComboSearch;
	Gtk::Entry			*m_pCashoutComboSearchEntry;
	Gtk::Label			*m_pCashoutSearchFoundText;
	
	// item details edit dialog
	Gtk::Entry			*m_pItemeditName;
	Gtk::Entry			*m_pItemeditNumber;
	Gtk::Entry			*m_pItemeditPrice;
	// Gtk::Entry			*m_pItemeditCategory;
	Gtk::DropDown	*m_pItemeditCategoryCombo;
	Gtk::DropDown	*m_pItemeditStatusCombo;
	Gtk::Button		*m_pItemeditOk;
	Gtk::Button		*m_pItemeditCancel;
	
	// seller details edit dialog
	struct {
		Gtk::Entry		*editName;
		Gtk::Entry		*editCode;
		Gtk::Entry		*editNote;
		Gtk::Button		*buttonOk;
		Gtk::Button		*buttonCancel;
		Gtk::DropDown	*comboCategory;
		SellerCategoryArray categories;
	} sellerEdit;
	
	// these controls will be passed to the message box control
	s_messageEditBoxWindowControls	messageEditBoxWindowControls;

	struct {
		Gtk::Window		*dialog;

		Gtk::Label		*labelCurrentLicence;
		Gtk::Button		*buttonRequestLicence;
		Gtk::Button		*buttonInstallLicence;
		Gtk::Entry		*editBurzaName;
		Gtk::Button		*buttonOk;
		Gtk::DropDown	*comboFontSize;

		Gtk::CheckButton	*radio1file;
		Gtk::CheckButton	*radio2database;
		Gtk::Entry		*editDbAddress;
		Gtk::Entry		*editDbUser;
		Gtk::Entry		*editDbPassword;
		Gtk::Entry		*editDbPort;
	} settingsWindow;
	
	
	LoadingMultipleFiles	loadFileProgressWindow;

	// helper variables for dialogs
	
	Glib::RefPtr<Gtk::TreeSelection>	m_CashoutItemsSelection;
	Glib::RefPtr<Gtk::ListStore>	m_CashoutItemsListModel;
	
	// columns for items tree view
	class ItemsColumns : public Gtk::TreeModel::ColumnRecord {
	public:
		ItemsColumns()
		{	
			add(m_number); 
			add( m_state_pic ); add( m_state_checkbox );
			add(m_name); add(m_price); add(m_category); add(m_state); add( m_seller_id ); add( m_item_id ); 
		}
		Gtk::TreeModelColumn<std::string> m_number;
		Gtk::TreeModelColumn<std::string> m_name;
		Gtk::TreeModelColumn<std::string> m_price;
		Gtk::TreeModelColumn<std::string> m_category;
		Gtk::TreeModelColumn<std::string> m_state;
		Gtk::TreeModelColumn<int> m_seller_id;
		Gtk::TreeModelColumn<int> m_item_id;
		
		// space between "> >" because of gcc compile error
		Gtk::TreeModelColumn<Glib::RefPtr<Gdk::Pixbuf> > m_state_pic;
		Gtk::TreeModelColumn<bool> m_state_checkbox;
	};


	Glib::RefPtr<Gdk::Pixbuf>	registerColumnStateImages[ Burza::ItemStatusCountWithUndefined ];

	std::vector< CellItem_Bug > registerItemCheckBoxes;
	bool registerItemCheckBoxes2[ 100 ];

	void on_column_checkbox_edited( Glib::ustring text );

	// == register item list ==
	
	ItemsColumns	m_ItemsColumn;
	Gtk::TreeView::Column*	registerItemsStatusColumn;
	
	// === register + cashout ===
private:	
	class SellerDataWithFilterMark;
public:
	
	//! All sellers. Loaded for further processing
	SellerDataArray LoadedSellers; // vector<SellerDataWithFilterMark> Sellers;
	
	//! Sellers displayed in the program, in combo boxes:
	//! m_pComboSeller, m_pCashoutComboLookupSeller
	//! sorted array. It is alignable to visual lists.
	vector<SellerDataWithFilterMark> SellersOrdered;
	
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
	
	//! name of person(s) accepting incoming seller items;
	std::string		personAcceptingItems;

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
	
private:
	class SellerDataWithFilterMark : public SellerData
	{
	public:
		string mark;
		
		SellerDataWithFilterMark(const SellerData& other) : SellerData( other ) { }
	};
} ;


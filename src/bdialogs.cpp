// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#include <assert.h>
#include <exception>
#include <algorithm>
#include "bdialogs.h"
#include "convfncs.h"

#include "excelwrap.h"
#include "pdfwrap.h"
#include "recipees.h"
#include "datatextfile.h"
#include "cacheddatastorage.h"
#include "translate.h"
#include "itemsearch.h"
#include "errhandler.h"
#include "EditMessageBox.h"
#include "stringencoding.h"
#include "bdfilehelper.h"

#include "generator.h"
#include "licence.h"

#include <gtkmm-2.4/gtkmm/filechooserdialog.h>

using namespace Burza;
using namespace BurzaExcel;
using namespace std;

ComboBoxTextDer::ComboBoxTextDer( BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::ComboBoxText(cobject)
{ }

// ==============================================
// ==============================================
// ==============================================

BurzaDialogs::BurzaDialogs()
{
	mp_register = & Centrala::Register();
	
	m_RegisterSelectedSellerId = SELLER_ID_NONE;
	
	m_CashoutSelectedSellerId = SELLER_ID_NONE;
	m_CashoutSelectedItemId = ITEM_ID_NONE;
	
	m_cashout_sum = 0.0;

	ConvFuncs::SingletonInstanceCreate();
}

BurzaDialogs::~BurzaDialogs()
{
	ConvFuncs::SingletonInstanceDelete();
}

bool BurzaDialogs::RegisterAnySelectedSeller( void )
{ 
	return m_RegisterSelectedSellerId != SELLER_ID_NONE; 
}

SellerData BurzaDialogs::RegisterGetSelectedSeller( void )
{ 
	try {
		return Centrala::Storage().GetSeller( m_RegisterSelectedSellerId );
	}
	catch(exception& e)
	{
		ErrorHandling::HandleException(e);
	}
}

SELLER_ID BurzaDialogs::RegisterGetSelectedSellerId( void )
{
	return m_RegisterSelectedSellerId;
}

SellerData BurzaDialogs::CashoutGetSelectedSeller( void )
{ 
	try {
		return Centrala::Storage().GetSeller( m_CashoutSelectedSellerId );
	}
	catch(exception& e)
	{
		ErrorHandling::HandleException(e);
	}
}

ItemData BurzaDialogs::CashoutGetSelectedItem( void )
{
	try {
		return Centrala::Storage().GetItem( m_CashoutSelectedItemId );
	}
	catch(exception& e)
	{
		ErrorHandling::HandleException(e);
	}
}

int BurzaDialogs::SetupDialogs( void )
{
	std::string		glade_fname;
	
	glade_fname = Centrala::Settings().m_data_dir_path;
	glade_fname += "burza.glade";
	
	std::string		error;
	
	error = "";
	
	try {

	xmlWndRegister = Gnome::Glade::Xml::create( glade_fname.c_str(), "Register" );
	if ( ! xmlWndRegister )
		error = "BiturnDialogsGtkmm::Init() error loading glade item from xml. 'Register'";
	
	xmlWndCashOut = Gnome::Glade::Xml::create( glade_fname.c_str(), "Pokladna" );
	if ( ! xmlWndCashOut )
		error = "BiturnDialogsGtkmm::Init() error loading glade item from xml. 'Pokladna'";
	
	xmlWndModifySeller = Gnome::Glade::Xml::create( glade_fname.c_str(), "Predajca" );
	if ( ! xmlWndModifySeller )
		error = "BiturnDialogsGtkmm::Init() error loading glade item from xml. 'Predajca'";
	
	xmlWndModifyItem = Gnome::Glade::Xml::create( glade_fname.c_str(), "Tovar" );
	if ( ! xmlWndModifyItem )
		error = "BiturnDialogsGtkmm::Init() error loading glade item from xml. 'Tovar'";
	
	xmlWndRegisterPopupMenu = Gnome::Glade::Xml::create( glade_fname.c_str(), "menuRegisterOtherActions" );
	if ( ! xmlWndRegisterPopupMenu )
		error = "BiturnDialogsGtkmm::Init() error loading glade item from xml. 'menuRegisterOtherActions'";
	
	xmlWndRegister->get_widget( "Register", m_pWndRegister );
	xmlWndCashOut->get_widget( "Pokladna", m_pWndCashOut );
	xmlWndModifySeller->get_widget( "Predajca", m_pWndSeller );
	xmlWndModifyItem->get_widget( "Tovar", m_pWndItem );
	
	if (( m_pWndRegister == NULL ) ||
		( m_pWndCashOut == NULL ) ||
		( m_pWndSeller == NULL ) ||
		( m_pWndItem == NULL )) {
		error = "Dialog file is corrupt [3].";
		throw;
	}
	
	m_pWndRegister->show();
	m_pWndRegister->maximize();
	
	xmlWndRegister->get_widget_derived( "combobox seller", m_pComboSeller );
	xmlWndRegister->get_widget( "button import seller", m_pButtonImportSeller );
	xmlWndRegister->get_widget( "button add seller", m_pButtonAddSeller );
	xmlWndRegister->get_widget( "button update seller", m_pButtonUpdateSeller );
	xmlWndRegister->get_widget( "button_finish_accepting_items", m_pButtonFinishAcceptingItems );
	xmlWndRegister->get_widget( "treeview items", m_pTreeViewItems );
	xmlWndRegister->get_widget( "button add item", m_pButtonAddItem );
	xmlWndRegister->get_widget( "button modify item", m_pButtonModifyItem );
	xmlWndRegister->get_widget( "button remove item", m_pButtonRemoveItem );
	xmlWndRegister->get_widget( "button toggle accepted", m_pButtonAcceptUnaccept );
	// xmlWndRegister->get_widget( "button calc fee", m_pButtonCalcFee );
	xmlWndRegister->get_widget( "button calc repayment", m_pButtonCalcRepayment );
	xmlWndRegister->get_widget( "label fee", m_pLabelFee );
	xmlWndRegister->get_widget( "label seller note", m_pLabelSellerNote );
	xmlWndRegister->get_widget( "button cashout", m_pButtonCashout );
	// xmlWndRegister->get_widget( "button_review_of_sellers", m_pRegisterButtonReviewOfSellers );
	// xmlWndRegister->get_widget( "button_settings", m_pButtonSettings );
	xmlWndRegister->get_widget( "button_other_actions", m_pRegisterOtherActions );
	
	xmlWndRegisterPopupMenu->get_widget( "menuRegisterOtherActions", menuRegisterOtherActions );
	xmlWndRegisterPopupMenu->get_widget( "menuItemSettings", menuItemSettings );
	xmlWndRegisterPopupMenu->get_widget( "menuItemSaveSellerOverview", menuItemSaveSellerOverview );
	xmlWndRegisterPopupMenu->get_widget( "menuitemStats", menuitemStats );
	
	if (( m_pComboSeller == NULL ) ||
		( m_pButtonImportSeller == NULL ) ||
		( m_pButtonAddSeller == NULL ) ||
		( m_pButtonUpdateSeller == NULL ) ||
		( m_pButtonFinishAcceptingItems == NULL ) ||
		( m_pTreeViewItems == NULL ) ||
		( m_pButtonAddItem == NULL ) ||
		( m_pButtonModifyItem == NULL ) ||
		( m_pButtonRemoveItem == NULL ) ||
		( m_pButtonAcceptUnaccept == NULL ) ||
		// ( m_pButtonCalcFee == NULL ) ||
		( m_pButtonCalcRepayment == NULL ) ||
		( m_pLabelFee == NULL ) ||
		( m_pLabelSellerNote == NULL ) ||
		( m_pButtonCashout  == NULL ) ||
		// ( m_pRegisterButtonReviewOfSellers == NULL ) ||
		// ( m_pButtonSettings == NULL ) ||
		( m_pRegisterOtherActions == NULL ) ||
		( menuRegisterOtherActions == NULL ) ||
		( menuItemSettings == NULL ) ||
		( menuItemSaveSellerOverview == NULL ) ||
		( menuitemStats == NULL )) {
		error = "Dialog file is corrupt [1].";
		throw;
	}
	
	m_pButtonImportSeller->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonImportSeller ) );
	m_pButtonAddSeller->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonAddSeller ) );
	m_pButtonUpdateSeller->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonUpdateSeller ) );
	m_pButtonFinishAcceptingItems->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonFinishAcceptingItems ) );
	m_pButtonAddItem->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonAddItem ) );
	m_pButtonModifyItem->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonModifyItem ) );
	m_pButtonRemoveItem->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonRemoveItem ) );
	m_pButtonAcceptUnaccept->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonAcceptUnaccept ) );
	// m_pButtonCalcFee->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonCalcFee ) );
	m_pButtonCalcRepayment->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonCalcRepayment ) );
	m_pButtonCashout->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonSwitchToCashout ) );
	// m_pRegisterButtonReviewOfSellers->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonReviewOfSellers ) );
	// m_pButtonSettings->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonSettings ) );
	m_pRegisterOtherActions->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonOtherActions ) );

	menuItemSettings->signal_button_release_event().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonSettings ) );
	menuItemSaveSellerOverview->signal_button_release_event().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonSaveSellerOverview ) );
	menuitemStats->signal_button_release_event().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonStats ) );

	m_pComboSeller->signal_changed().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterComboChangedSeller ) );
	
	m_RegisterItemsSelection = m_pTreeViewItems->get_selection();
	m_RegisterItemsSelection->set_mode( Gtk::SELECTION_MULTIPLE );
	
	// setup item list
	m_RegisterItemsListModel = Gtk::ListStore::create( m_ItemsColumn );
	m_pTreeViewItems->set_model( m_RegisterItemsListModel );

	m_pTreeViewItems->append_column( "#", m_ItemsColumn.m_number );

	// m_pTreeViewItems->append_column( "stav", m_ItemsColumn.m_state );
	registerItemsStatusColumn = Gtk::manage( new Gtk::TreeView::Column( "stav" ) );
	registerItemsStatusColumn->pack_start( m_ItemsColumn.m_state_pic , false); //false = don’t expand.
	registerItemsStatusColumn->pack_start( m_ItemsColumn.m_state );
	m_pTreeViewItems->append_column( * registerItemsStatusColumn );

	m_pTreeViewItems->append_column_editable( "prebrate", m_ItemsColumn.m_state_checkbox );

	m_pTreeViewItems->append_column( "nazov", m_ItemsColumn.m_name );
	m_pTreeViewItems->append_column( "cena", m_ItemsColumn.m_price );
	m_pTreeViewItems->append_column( "kategoria", m_ItemsColumn.m_category );
	
	//Make all the columns reorderable:
	//This is not necessary, but it's nice to show the feature.
	//You can use TreeView::set_column_drag_function() to more
	//finely control column drag and drop.
	for(guint i = 0; i <5; i++)
	{
		Gtk::TreeView::Column * pColumn = m_pTreeViewItems->get_column( i );
		if ( pColumn == NULL ) {
			assert( false );
			continue;
		}
		
		pColumn->set_reorderable( false );
		pColumn->set_resizable();
		int w = pColumn->get_width();
		pColumn->set_min_width( 200 );
		//pColumn->set_fixed_width( 200 );
		pColumn->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );
		//pColumn->set_sizing( Gtk::TREE_VIEW_COLUMN_GROW_ONLY );
	}
	
	// set column sorting and width
	Gtk::TreeView::Column * pColumn;
	
	pColumn = m_pTreeViewItems->get_column( 0 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_number );
		pColumn->set_min_width( 50 );
	}
	pColumn = m_pTreeViewItems->get_column( 1 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_state );
		pColumn->set_min_width( 100 );
	}

	pColumn = m_pTreeViewItems->get_column( 2 );
	if( pColumn ) {
		//pColumn->set_sort_column( m_ItemsColumn.m_state_checkbox );
		pColumn->set_min_width( 50 );
		pColumn->set_clickable();

		Gtk::CellRenderer* pCellRenderer = pColumn->get_first_cell_renderer();
		Gtk::CellRendererToggle* pCellRenderToggle = dynamic_cast<Gtk::CellRendererToggle*>(pCellRenderer);
		if(pCellRenderToggle) {
			pCellRenderToggle->signal_toggled().connect( 
				sigc::mem_fun(*this, &BurzaDialogs::on_column_checkbox_edited) );
		}
	}

	pColumn = m_pTreeViewItems->get_column( 3 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_name );
		pColumn->set_min_width( 400 );
	}
	pColumn = m_pTreeViewItems->get_column( 4 );
	if( pColumn ) {
		//pColumn->set_sort_column( m_ItemsColumn.m_nazov );
		pColumn->set_min_width( 80 );
	}
	pColumn = m_pTreeViewItems->get_column( 5 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_category );
		pColumn->set_min_width( 50 );
	}

	// === --- === cash-out === --- ===
	
	if ( m_pWndCashOut == NULL ) {
		error = "Dialog file is corrupt [2].";
		throw;
	}
	
	m_pWndCashOut->hide();

	xmlWndCashOut->get_widget( "treeview cashout items", m_pCashoutTreeViewList );
	xmlWndCashOut->get_widget( "entry lookup seller", m_pCashoutEntryLookupSeller );
	xmlWndCashOut->get_widget_derived( "combo lookup seller", m_pCashoutComboLookupSeller );
	xmlWndCashOut->get_widget( "entry lookup item", m_pCashoutEntryLookupItem );
	xmlWndCashOut->get_widget_derived( "combo lookup item", m_pCashoutComboLookupItem );
	xmlWndCashOut->get_widget( "label item price", m_pCashoutLabelItemPrice );
	xmlWndCashOut->get_widget( "button cashout add item", m_pCashoutButtonAddItem );
	xmlWndCashOut->get_widget( "button cashout remove item", m_pCashoutButtonRemoveItem );
	xmlWndCashOut->get_widget( "label sum", m_pCashoutLabelSum );
	xmlWndCashOut->get_widget( "label cashout finished", m_pCashoutButtonFinish );
	xmlWndCashOut->get_widget( "button register", m_pButtonRegister );
	xmlWndCashOut->get_widget_derived( "comboboxentry_search", m_pCashoutComboSearch );
	m_pCashoutComboSearchEntry = dynamic_cast<Gtk::Entry*>( m_pCashoutComboSearch->get_child() );
	
	if (( m_pButtonRegister == NULL ) ||
		( m_pCashoutTreeViewList == NULL ) ||
		( m_pCashoutEntryLookupSeller == NULL ) ||
		( m_pCashoutComboLookupSeller == NULL ) ||
		( m_pCashoutEntryLookupItem == NULL ) ||
		( m_pCashoutComboLookupItem == NULL ) ||
		( m_pCashoutLabelItemPrice == NULL ) ||
		( m_pCashoutButtonAddItem == NULL ) ||
		( m_pCashoutButtonRemoveItem == NULL ) ||
		( m_pCashoutLabelSum == NULL ) ||
		( m_pCashoutButtonFinish == NULL ) ||
	   ( m_pCashoutComboSearch == NULL ) ||
	   ( m_pCashoutComboSearchEntry == NULL )) {
		error = "Dialog file is corrupt [4].";
		throw;
	}
	
	m_pButtonRegister->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::CashoutButtonSwitchToRegister ) );
	m_pCashoutButtonAddItem->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::CashoutButtonAddItem ) );
	m_pCashoutButtonRemoveItem->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::CashoutButtonRemoveItem ) );
	m_pCashoutButtonFinish->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::CashoutButtonFinished ) );
	
	m_CashoutItemsSelection = m_pCashoutTreeViewList->get_selection();
	m_CashoutItemsSelection->set_mode( Gtk::SELECTION_SINGLE );
	m_CashoutItemsSelection->signal_changed().connect( sigc::mem_fun( this, &BurzaDialogs::CashoutItemsTreeSelectChanged ) );
	
	m_pCashoutEntryLookupSeller->signal_changed().connect( sigc::mem_fun( this, &BurzaDialogs::CashoutEditLookupSellerChanged ) );
	m_pCashoutEntryLookupItem->signal_changed().connect( sigc::mem_fun( this, &BurzaDialogs::CashoutEditLookupItemChanged ) );
	m_pCashoutComboLookupSeller->signal_changed().connect( sigc::mem_fun( *this, &BurzaDialogs::CashoutComboLookupSellerChanged ) );
	m_pCashoutComboLookupItem->signal_changed().connect( sigc::mem_fun( *this, &BurzaDialogs::CashoutComboLookupItemChanged ) );
	
	m_pCashoutComboSearch->signal_changed().connect( sigc::mem_fun( this, &BurzaDialogs::CashoutEditSearchChanged ) );
	m_pCashoutComboSearchEntry->signal_changed().connect( sigc::mem_fun( this, &BurzaDialogs::CashoutEditSearchEntryChanged ) );

	// setup item list
	m_CashoutItemsListModel = Gtk::ListStore::create( m_ItemsColumn );
	m_pCashoutTreeViewList->set_model( m_CashoutItemsListModel );
	m_pCashoutTreeViewList->append_column( "#", m_ItemsColumn.m_number );
	m_pCashoutTreeViewList->append_column( "stav", m_ItemsColumn.m_state );
	m_pCashoutTreeViewList->append_column( "nazov", m_ItemsColumn.m_name );
	m_pCashoutTreeViewList->append_column( "cena", m_ItemsColumn.m_price );
	m_pCashoutTreeViewList->append_column( "kategoria", m_ItemsColumn.m_category );
	
	//Make all the columns reorderable:
	//This is not necessary, but it's nice to show the feature.
	//You can use TreeView::set_column_drag_function() to more
	//finely control column drag and drop.
	for(guint i = 0; i <5; i++)
	{
		Gtk::TreeView::Column * pColumn = m_pCashoutTreeViewList->get_column( i );
		if ( pColumn == NULL ) {
			assert( false );
			continue;
		}
		
		pColumn->set_reorderable( false );
		pColumn->set_resizable();
		int w = pColumn->get_width();
		pColumn->set_min_width( 200 );
		//pColumn->set_fixed_width( 200 );
		pColumn->set_sizing( Gtk::TREE_VIEW_COLUMN_FIXED );
		//pColumn->set_sizing( Gtk::TREE_VIEW_COLUMN_GROW_ONLY );
	}
	
	// set column sorting and width
	//Gtk::TreeView::Column * pColumn;
	
	pColumn = m_pCashoutTreeViewList->get_column( 0 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_number );
		pColumn->set_min_width( 50 );
	}
	pColumn = m_pCashoutTreeViewList->get_column( 1 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_state );
		pColumn->set_min_width( 100 );
	}
	pColumn = m_pCashoutTreeViewList->get_column( 2 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_name );
		pColumn->set_min_width( 400 );
	}
	pColumn = m_pCashoutTreeViewList->get_column( 3 );
	if( pColumn ) {
		//pColumn->set_sort_column( m_ItemsColumn.m_nazov );
		pColumn->set_min_width( 80 );
	}
	pColumn = m_pCashoutTreeViewList->get_column( 4 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_category );
		pColumn->set_min_width( 50 );
	}
	
	// == item details edit dialog ==
	xmlWndModifyItem->get_widget( "itemName", m_pItemeditName );
	xmlWndModifyItem->get_widget( "itemNumber", m_pItemeditNumber );
	xmlWndModifyItem->get_widget( "itemPrice", m_pItemeditPrice );
	xmlWndModifyItem->get_widget( "itemCategory", m_pItemeditCategory );
	xmlWndModifyItem->get_widget_derived( "itemStatus_combobox", m_pItemeditStatusCombo );
	xmlWndModifyItem->get_widget( "itemButton_ok", m_pItemeditOk );
	xmlWndModifyItem->get_widget( "itemButton_cancel", m_pItemeditCancel );
	
	if (( m_pItemeditName == NULL ) ||
		( m_pItemeditNumber == NULL ) ||
		( m_pItemeditPrice == NULL ) ||
		( m_pItemeditCategory == NULL ) ||
		( m_pItemeditStatusCombo == NULL ) ||
		( m_pItemeditOk == NULL ) ||
		( m_pItemeditCancel == NULL )) {
		error = "Dialog file is corrupt [5].";
		throw;
	}
	
	m_pItemeditOk->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonItemEditOk ) );
	m_pItemeditCancel->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonItemEditCancel ) );
	m_pWndItem->signal_delete_event().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonItemEditClose ) );
	m_pItemeditStatusCombo->signal_changed().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterComboItemEditStatusChanged ) );
	
	// == seller details edit dialog == 

	xmlWndModifySeller->get_widget( "sellerName", m_pSellerEditName );
	xmlWndModifySeller->get_widget( "sellerCode", m_pSellerEditCode );
	xmlWndModifySeller->get_widget( "sellerNote", m_pSellerEditNote );
	xmlWndModifySeller->get_widget( "sellerButton_ok", m_pSellerEditOk );
	xmlWndModifySeller->get_widget( "sellerButton_cancel", m_pSellerEditCancel );
	
	if (( m_pSellerEditName == NULL ) ||
		( m_pSellerEditCode == NULL ) ||
		( m_pSellerEditNote == NULL ) ||
		( m_pSellerEditOk == NULL ) ||
		( m_pSellerEditCancel == NULL )) {
		error = "Dialog file is corrupt [6=Seller edit].";
		throw;
	}
	
	m_pSellerEditOk->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonSellerEditOk ) );
	m_pSellerEditCancel->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonSellerEditCancel ) );
	m_pWndSeller->signal_delete_event().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonSellerEditClose ) );

	// === --- === --- === edit message box === --- === --- ===

	messageEditBoxWindowControls.xmlWindow = Gnome::Glade::Xml::create( glade_fname.c_str(), "MessageEditBox" );
	if ( ! messageEditBoxWindowControls.xmlWindow ) {
		error = "BiturnDialogsGtkmm::Init() error loading glade item from xml. 'MessageEditBox'";
		throw;
	}

	messageEditBoxWindowControls.xmlWindow->get_widget( "MessageEditBox", messageEditBoxWindowControls.dialog );
	messageEditBoxWindowControls.xmlWindow->get_widget( "label_text", messageEditBoxWindowControls.labelText );
	messageEditBoxWindowControls.xmlWindow->get_widget( "entry_text", messageEditBoxWindowControls.entryText );
	messageEditBoxWindowControls.xmlWindow->get_widget( "button_ok", messageEditBoxWindowControls.buttonOk );
	messageEditBoxWindowControls.xmlWindow->get_widget( "button_cancel", messageEditBoxWindowControls.buttonCancel );

	if (( messageEditBoxWindowControls.dialog == NULL ) ||
		( messageEditBoxWindowControls.labelText == NULL ) ||
		( messageEditBoxWindowControls.entryText == NULL ) ||
		( messageEditBoxWindowControls.buttonOk == NULL ) ||
		( messageEditBoxWindowControls.buttonCancel == NULL )) {
		error = "Dialog file is corrupt [MessageEditBox].";
		throw;
	}
	
	messageEditBoxWindowControls.buttonOk->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::EditMessageBoxClickedOk ) );
	messageEditBoxWindowControls.buttonCancel->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::EditMessageBoxClickedCancel ) );

	EditMessageBox::Initialize( messageEditBoxWindowControls );

	// === --- === --- === Nastavenia === --- === --- ===

	settingsWindow.xmlWindow = Gnome::Glade::Xml::create( glade_fname.c_str(), "Nastavenia" );
	if ( ! settingsWindow.xmlWindow ) {
		error = "BiturnDialogsGtkmm::Init() error loading glade item from xml. 'Nastavenia'";
		throw;
	}

	settingsWindow.xmlWindow->get_widget( "Nastavenia", settingsWindow.dialog );
	settingsWindow.xmlWindow->get_widget( "label_current_licence", settingsWindow.labelCurrentLicence );
	settingsWindow.xmlWindow->get_widget( "button_request_licence", settingsWindow.buttonRequestLicence );
	settingsWindow.xmlWindow->get_widget( "button_install_licence", settingsWindow.buttonInstallLicence );
	settingsWindow.xmlWindow->get_widget( "entry_name_of_burza", settingsWindow.editBurzaName );
	settingsWindow.xmlWindow->get_widget( "button_ok", settingsWindow.buttonOk );

	if (( settingsWindow.dialog == NULL ) ||
		( settingsWindow.labelCurrentLicence == NULL ) ||
		( settingsWindow.buttonRequestLicence == NULL ) ||
		( settingsWindow.editBurzaName == NULL ) ||
		( settingsWindow.buttonOk == NULL ) ||
		( settingsWindow.buttonInstallLicence == NULL )) {
		error = "Dialog file is corrupt [Nastavenia].";
		throw;
	}
	
	settingsWindow.buttonRequestLicence->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::SettingsClickedButtonRequestLicence ) );
	settingsWindow.buttonInstallLicence->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::SettingsClickedButtonInstallLicence ) );
	settingsWindow.buttonOk->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::SettingsClickedButtonOk ) );

	// === --- === --- === throw - catch  === --- === --- ===
	
	}
	catch( ... ) {
	}
	
	if ( ! error.empty() ) {
		DisplayErrorMessageBox( error, "Error initing dialog." );
		return -1;
	}

	CreateStatusImages();

	return 0;
}

void BurzaDialogs::RegisterDisplaySeller( const char* kod )
{
	SellerData seller;
	
	try {
		string codeString(kod);
		seller = Centrala::Storage().GetSellerByCode(codeString);
		RegisterDisplaySeller(seller);
	}
	catch(WrongSellerException) {
	}
}

void BurzaDialogs::RegisterDisplaySellerByIndex( unsigned int idx )
{
	if ( idx >= m_DisplayedSellers.size() ) {
		m_RegisterSelectedSellerId = SELLER_ID_NONE;

		DisplayOkMessageBox( _("Neexistuje žiaden predajca"), "" );
		return;
	}

	RegisterDisplaySeller(m_DisplayedSellers[ idx ]);
}




CellItem_Bug::CellItem_Bug()
{
  m_fixed = false;
}

CellItem_Bug::CellItem_Bug(bool fixed)
{
  m_fixed = fixed;
}

CellItem_Bug::CellItem_Bug(const CellItem_Bug& src)
{
  operator=(src);
}

CellItem_Bug::~CellItem_Bug()
{
}

CellItem_Bug& CellItem_Bug::operator=(const CellItem_Bug& src)
{
  m_fixed = src.m_fixed;

  return *this;
}





void BurzaDialogs::RegisterDisplaySeller(SellerData& seller)
{
	m_RegisterSelectedSellerId = seller.id;
	
	m_pLabelSellerNote->set_label( seller.note );
	
	// fill items
	ItemDataArray	items = DemoLimitsGetSellerItems(seller.id);
	ItemCategoryArray	categories = Centrala::Storage().GetItemsCategories(items);
	Gtk::TreeModel::Row row;
	
	m_RegisterItemsListModel->clear();
	
	/* remove also properties: registerItemCheckBoxes.clear();
	
	for( int i=0; i<items.size(); i++ ) {
		registerItemCheckBoxes.push_back( new CellItem_Bug() );
		registerItemCheckBoxes2[ i ] = false;
	}
	*/

	for( int i=0; i<items.size(); i++ ) {
		
		row = * (m_RegisterItemsListModel->append());
		
		row[ m_ItemsColumn.m_number ] = ConvFuncsInstance->FormatCstr( "%i", items[i].number );
		row[ m_ItemsColumn.m_name ] = items[i].name;
		row[ m_ItemsColumn.m_price ] = ConvFuncsInstance->FormatCstr( "%0.2f", items[i].price );
		row[ m_ItemsColumn.m_category ] = ConvFuncsInstance->FormatCstr( "%c", categories[i].letter );

		row[ m_ItemsColumn.m_state ] = ConvFuncsInstance->FormatCstr( "%s", 
			DataAccessInterface::ItemStatus2String( items[i].status ) );
		row[ m_ItemsColumn.m_state_pic ] = registerColumnStateImages[ items[i].status ];
		row[ m_ItemsColumn.m_state_checkbox ] = (items[i].status == Burza::on_stock);
	}
	
	int idx;
	
	for( idx = 0; idx < m_DisplayedSellers.size(); idx++ ) {
		if ( m_DisplayedSellers[ idx ].id == seller.id ) {
			break;
		}
	}
	
	if ( idx >= m_DisplayedSellers.size() ) {
		idx = -1;
	}
	m_pComboSeller->set_active( idx );
	// i = m_pComboSeller->get_active_row_number();
}

void BurzaDialogs::ReloadSellers( void )
{
	FillSellerCombos();
}

void BurzaDialogs::FillSellerCombos( void )
{
	m_DisplayedSellers = DemoLimitsGetSellers();
	std::string		str;

	sort( m_DisplayedSellers.begin(), m_DisplayedSellers.end(), OrderSellerByShortCode );
	
	m_pComboSeller->clear();
	m_pCashoutComboLookupSeller->clear();
	
	for( int i=0; i<m_DisplayedSellers.size(); ++i ) {
		str = m_DisplayedSellers[ i ].shortcode;
		str += " - ";
		str += m_DisplayedSellers[ i ].name;

		m_pComboSeller->append_text( str );
		m_pCashoutComboLookupSeller->append_text( str );
	}
}

void BurzaDialogs::RegisterButtonImportSeller( void )
{
	if (DemoLimitsCanAddSeller() == false)
		return;

	Gtk::FileChooserDialog dialog("Select importer file", Gtk::FILE_CHOOSER_ACTION_OPEN );

	dialog.set_transient_for( *m_pWndRegister );
	//Add response buttons the the dialog:
	dialog.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );
	dialog.add_button( Gtk::Stock::OK, Gtk::RESPONSE_OK );
	dialog.set_filename( Centrala::Settings().m_data_dir_path );
	
	if ( Centrala::Settings().directoryImport.empty() ) {
		dialog.set_current_folder( Centrala::Settings().m_data_dir_path );
	} else {
		dialog.set_current_folder( Centrala::Settings().directoryImport );
	}

	Gtk::FileFilter	xls_filter;
	xls_filter.set_name( "Sheet files." );
	xls_filter.add_pattern( "*.ods" );
	xls_filter.add_pattern( "*.xls" );
	xls_filter.add_pattern( "*.xlsx" );
	xls_filter.add_pattern( "*.txt" );
	dialog.add_filter( xls_filter );
	
	int result = dialog.run();
	//Handle the response:
	switch(result) {
	case(Gtk::RESPONSE_OK):
		break;
	case(Gtk::RESPONSE_CANCEL):
	case Gtk::RESPONSE_DELETE_EVENT:
	default:
		//DisplayErrorMessageBox( "BiturnDialogsGtkmm::on_clicked_ViewDirs() Unexpected button clicked.\n" );
		return;
	}
	
	// set back the import directory
	Centrala::Settings().directoryImport = dialog.get_current_folder();
	Centrala::Settings().directoryImport += "/";

	string filename = dialog.get_filename();
	// s += "data/harok-burza.xls";
	string::size_type pos = filename.rfind('.');
	string ext;
	bool knownType = false;

	if (pos != string::npos) {
		ext = filename.substr( pos + 1 );
		ConvFuncs::StdStringToLower( ext );
	}
	
	SellerData seller;
	ItemDataArray items;
	
	try {
		if ( ext == "txt" ) {
			knownType = true;
			CachedDataTextfile	textStorage( Centrala::Settings().m_data_dir_path );
			textStorage.LoadFromTxtFile(filename, seller, items );
		}
		if ( ext == "xls" ) {
			knownType = true;
			BurzaExcel::BasicExcelFileWrapper	excelFile;
			excelFile.ReadSeller( filename, seller, items );
		}
		if ( ext == "ods" ) {
			DisplayErrorMessageBox( _("Nepodporovany format suboru") );
		}
		if ( ext == "xlsx" ) {
			DisplayErrorMessageBox( _("Nepodporovany format suboru") );
		}

		if (knownType == false) {
			DisplayErrorMessageBox( _("Neznámy typ súboru. Podporované su 'ods', 'xls', 'xlsx', 'txt'") );
			return;
		}
	}
	catch( runtime_error& e ) {
		DisplayErrorMessageBox( string( "Error importing file." ) + e.what(), "Error importing file." );
		return;
	}

	DataAccessInterface	&storage = Centrala::Storage();
	
	bool sellerExists = true;
	try {
		storage.GetSellerByCode( seller.shortcode );
	}
	catch( WrongSellerException e) {
		sellerExists = false;
	}
	
	if ( sellerExists ) {
		DisplayErrorMessageBox( 
			ConvFuncs::FormatCstr( _( "Predajca s rovnakým kódom už existuje. Zmažte najskôr predajcu '%s'." ),
				seller.shortcode.c_str() ) );
		return;
	}

	// imported items have to be checked by hand. Take note that these were not.
	seller.note += _(" za MC prijal: !zatiaľ neprebrané!");
	
	seller.id = storage.AddSeller( seller );
	storage.AddItems( items, seller.id );

	DisplayOkMessageBox( 
		_( "Predajca načítaný"),
		ConvFuncs::FormatStdStr( _("Kód: %s Meno: %s Počet položiek: %i"), 
			seller.shortcode.c_str(), seller.name.c_str(), items.size() ));

	ReloadSellers();
	RegisterDisplaySeller( seller );
}

void BurzaDialogs::RegisterButtonAddSeller( void )
{
	if ( DemoLimitsCanAddSeller() ) {

		// m_sellerEditAction = ea_add;
		RegisterStartDialogEditSeller( SELLER_ID_NONE );
	}
}

void BurzaDialogs::RegisterButtonUpdateSeller( void )
{
	// m_sellerEditAction = ea_modify;
	if ( RegisterAnySelectedSeller() )
		RegisterStartDialogEditSeller( RegisterGetSelectedSellerId() );
}

void BurzaDialogs::RegisterButtonFinishAcceptingItems( void )
{
	std::string preberajuci = personAcceptingItems;

	// popup - name who accepted the items
	bool clickedOk = EditMessageBox::ShowDialog( 
		_( "Naozaj chcete ukončiť preberanie tovaru tohto predajcu? Zadajte meno preberajúcich:" ), 
		preberajuci );

	if ( ! clickedOk )
		return;

	// mark seller as 'accepted'
	SellerData seller;

	try {
		seller = Centrala::Storage().GetSeller( m_RegisterSelectedSellerId );
		Centrala::Storage().ModifySellerStart( m_RegisterSelectedSellerId );
		
		seller.note += _( "Prijatý " ) + preberajuci;
		seller.status = SellerStatus::received;

		Centrala::Storage().ModifySellerEnd( m_RegisterSelectedSellerId, seller );
	}
	catch(exception& e)
	{
		ErrorHandling::HandleException(e);
	}
	
	RegisterSellerModified( seller );

	personAcceptingItems = preberajuci;
}

void BurzaDialogs::RegisterButtonAddItem( void )
{
	if ( !RegisterAnySelectedSeller() )
		return;

	if ( DemoLimitsCanAddItem( RegisterGetSelectedSellerId() ) ) {
		RegisterStartDialogEditProduct( ITEM_ID_NONE );
	}
}

void BurzaDialogs::RegisterButtonModifyItem( void )
{
	if ( !RegisterAnySelectedSeller() )
		return;
	
	vector< ITEM_ID > listOfItems;
	RegisterGetSelectedItemIds( listOfItems );
	
	if ( listOfItems.size() != 1 )
		return;
	
	RegisterStartDialogEditProduct( listOfItems[0] );
}

void BurzaDialogs::RegisterButtonRemoveItem( void )
{
	if ( !RegisterAnySelectedSeller() )
		return;
	
	// to avoid pointing to array that removes under our feet I'll make list of item numbers to delete
	vector< ITEM_ID > productIdsToDelete;
	
	RegisterGetSelectedItemIds( productIdsToDelete );
	
	// ask user if he is sure
	string str = ConvFuncs::FormatStdStr( _( "Ste si istý že chcete odstrániť %i ks tovaru?" ), productIdsToDelete.size() );
	if ( DisplayYesNoMessageBox( str, _("Odstránenie tovaru") ) == false )
		return;
	
	// delete items
	for( unsigned int i=0; i<productIdsToDelete.size(); ++i ) {
		Centrala::Storage().RemoveItem( productIdsToDelete[ i ] );
	}
	
	SellerData seller;

	try {
		seller = Centrala::Storage().GetSeller( m_RegisterSelectedSellerId );
	}
	catch(exception& e)
	{
		ErrorHandling::HandleException(e);
	}

	RegisterDisplaySeller( seller );
}

void BurzaDialogs::RegisterButtonAcceptUnaccept( void )
{
	if ( !RegisterAnySelectedSeller() )
		return;
	
	// ask user if he is sure
	if ( DisplayYesNoMessageBox( "Ste si istý že chcete preznacit tovar?", _("Oznacenie / odznacenie tovaru") ) == false )
		return;

	ItemDataArray items = Centrala::Storage().GetSellerItems(m_RegisterSelectedSellerId);
	
	if ( items.empty() )
		return;

	ItemStatus newValue = ( items[ 0 ].status == Burza::on_stock ) ? Burza::not_in_stock_yet : Burza::on_stock;

	for( int i=0; i<items.size(); ++i ) {
		
		items[ i ].status = newValue;
		
		Centrala::Storage().ModifyItem( items[ i ].id, items[ i ] );
	}

	// update seller
	SellerData seller;

	try {
		seller = Centrala::Storage().GetSeller( m_RegisterSelectedSellerId );
	}
	catch(exception& e)
	{
		ErrorHandling::HandleException(e);
	}

	RegisterDisplaySeller( seller );
}

void BurzaDialogs::RegisterButtonCalcFee( void )
{
	if ( !RegisterAnySelectedSeller() )
		return;
	
	ItemDataArray items;
	SellerData seller;
	try {
		items = Centrala::Storage().GetSellerItems( m_RegisterSelectedSellerId );
		seller = Centrala::Storage().GetSeller( m_RegisterSelectedSellerId );
	}
	catch(exception& e)
	{
		ErrorHandling::HandleException(e);
	}

	double fee = Centrala::Register().CalculateFee( seller, items );
	
	m_pLabelFee->set_text( ConvFuncs::FormatStdStr( "%0.2f %s", fee, 
		Centrala::Settings().m_string_mena.c_str() ) );
}

void BurzaDialogs::RegisterButtonCalcRepayment( void )
{
	m_pLabelFee->set_text( string( "-" ) + Centrala::Settings().m_string_mena );
	
	if ( !RegisterAnySelectedSeller() ) {
		return;
	}
	
	SellerData seller = RegisterGetSelectedSeller();
	
	ConvFuncs::GetDateTime();
	string offeredName = 
		ConvFuncs::FormatStdStr( "%s_%s_%02i-%02i-%02i_%02i-%02i.pdf",
		_("PoPredaji"), seller.shortcode.c_str(),
		ConvFuncs::datetime.year % 100, ConvFuncs::datetime.month, ConvFuncs::datetime.day,
		ConvFuncs::datetime.hour, ConvFuncs::datetime.minute );

	string filename = Centrala::Settings().m_document_ouput_directory;
	filename += "/";
	filename += offeredName;

	bool storingFinished = false;

	try {
		Recipees recipees;
		HPDFWrapper	pdf;
		pdf.CreatePDF( filename );

		recipees.PrintSellerReturnSheet( pdf, RegisterGetSelectedSellerId() );

		pdf.ClosePDF();

		storingFinished = true;

	} catch( runtime_error& e ) {
		DisplayErrorMessageBox( "PDF error", e.what() );
	} catch( out_of_range& e ) {
		DisplayErrorMessageBox( "PDF out of range error", e.what() );
	} catch( ... ) {
		DisplayErrorMessageBox( "PDF error" );
	}

	if (storingFinished) {
		Centrala::Register().DisplayPdf( filename.c_str() );
	}
}

void BurzaDialogs::RegisterButtonSwitchToCashout( void )
{
	m_pWndRegister->hide();
	m_pWndCashOut->show();
	m_pWndCashOut->maximize();
	
	m_pCashoutComboLookupSeller->clear();
	m_pCashoutComboLookupItem->clear();
	m_pCashoutEntryLookupSeller->set_text( "" );
	m_pCashoutEntryLookupItem->set_text( "" );
	m_pCashoutComboSearch->clear();
	m_pCashoutComboSearchEntry->set_text( "" );

	ReloadSellers();
	
	// set focus chaing = set order of tab-stops on items. Weird, but has to be done in the code.
	// Default tab-order
	vector<Gtk::Widget*> cashoutFocusList;
	
	cashoutFocusList.push_back( m_pCashoutEntryLookupSeller );
	cashoutFocusList.push_back( m_pCashoutEntryLookupItem );
	cashoutFocusList.push_back( m_pCashoutButtonAddItem );
	cashoutFocusList.push_back( m_pCashoutButtonFinish );
	
	cashoutFocusList.push_back( m_pCashoutComboLookupSeller );
	cashoutFocusList.push_back( m_pCashoutComboLookupItem );
	cashoutFocusList.push_back( m_pCashoutComboSearchEntry );
	cashoutFocusList.push_back( m_pCashoutComboSearch );
	
	cashoutFocusList.push_back( m_pTreeViewItems );
	cashoutFocusList.push_back( m_pCashoutButtonRemoveItem );
	cashoutFocusList.push_back( m_pButtonRegister );
	
	m_pWndCashOut->set_focus_chain( cashoutFocusList );

	Centrala::Search().FetchLatestData();
}

bool BurzaDialogs::OrderSellerByShortCode(const SellerData& a, const SellerData& b)
{ 
	string acode = a.shortcode;
	string bcode = b.shortcode;
	std::transform( acode.begin(), acode.end(), acode.begin(), static_cast<int (*)(int)>(tolower) );
	std::transform( bcode.begin(), bcode.end(), bcode.begin(), static_cast<int (*)(int)>(tolower) );

	return (acode.compare( bcode ) < 0); 
}

void BurzaDialogs::CashoutButtonSwitchToRegister( void )
{
	m_pWndRegister->show();
	m_pWndCashOut->hide();
	
	m_pWndRegister->unfullscreen();
	m_pWndRegister->maximize();
	m_pWndRegister->reset_shapes();
	
	ReloadSellers();
	m_pLabelFee->set_text( "" );
}

void BurzaDialogs::RegisterComboChangedSeller( void )
{
	int i = m_pComboSeller->get_active_row_number();
	
	if ( i == -1 )
		return;
	
	if ( i >= m_DisplayedSellers.size() ) {
		ReloadSellers();
		return;
	}
	
	RegisterDisplaySellerByIndex( i );
}

void BurzaDialogs::CashoutItemsTreeSelectChanged( void )
{
	
}

void BurzaDialogs::GetSelectedItemNumbers( Glib::RefPtr<Gtk::TreeSelection>& selector, vector<unsigned int>& listOfItems )
{
	listOfItems.clear();
	
	// count selected items
	Gtk::TreeSelection::ListHandle_Path	selected_items = selector->get_selected_rows();
	if ( selected_items.empty() )
		return;
	
	for( Gtk::TreeSelection::ListHandle_Path::iterator it=selected_items.begin();
			it != selected_items.end(); ++it ) {
		assert( (*it).size() == 1 );
		listOfItems.push_back( (*it)[ 0 ] );
	}
}

void BurzaDialogs::RegisterGetSelectedItemIds( vector<ITEM_ID>& listOfIds )
{
	if ( !RegisterAnySelectedSeller() ) {
		throw runtime_error("RegisterGetSelectedItemIds cannot work without selected seller");
	}
  
	listOfIds.clear();
	
	vector<unsigned int>	selectedIndexes;
	
	// count selected items
	GetSelectedItemNumbers( m_RegisterItemsSelection, selectedIndexes );
	
	ItemDataArray items = Centrala::Storage().GetSellerItems(m_RegisterSelectedSellerId);
	
	for( int i=0; i<selectedIndexes.size(); ++i ) {
		
		if ( items.size() < selectedIndexes[ i ] ) {
			assert( items.size() < selectedIndexes[ i ] );
			return;
		}
		
		listOfIds.push_back( items[ selectedIndexes[ i ] ].id );
	}
}

void BurzaDialogs::CashoutButtonAddItem( void )
{
	if ( ! CashoutAnySelectedItem() ) {
		return;
	}
	
	ShoppingCartIterator it;
	for( it = m_shoppingCart.begin(); it != m_shoppingCart.end(); ++it ) {
		if ( *it == CashoutGetSelectedItemId() )
			break;
	}

	if ( it != m_shoppingCart.end() ) {
		return;
	}
	
	if ( CashoutGetSelectedItem().status != on_stock ) {
		if ( DisplayYesNoMessageBox( 
			_("Tento tovar nieje označený že je na sklade"), 
			_("Chcete ho naozaj predať?") ) == false )
			return;
	}

	// ked nieje, tak pridaj
	ItemData item = CashoutGetSelectedItem();
	ItemCategory itemcat = Centrala::Storage().GetItemCategory( item.category );
	SellerData seller = Centrala::Storage().GetSeller( item.seller_id );
	Gtk::TreeModel::Row row;
	
	row = * (m_CashoutItemsListModel->append());
	
	row[ m_ItemsColumn.m_number ] = ConvFuncsInstance->FormatCstr( "%s - %i", seller.shortcode.c_str(), item.number );
	row[ m_ItemsColumn.m_name ] = item.name;
	row[ m_ItemsColumn.m_price ] = ConvFuncsInstance->FormatCstr( "%0.2f", item.price );
	row[ m_ItemsColumn.m_category ] = ConvFuncsInstance->FormatCstr( "%c", itemcat.letter );
	row[ m_ItemsColumn.m_state ] = ConvFuncsInstance->FormatCstr( "%s", 
		DataAccessInterface::ItemStatus2String( item.status ) );
	row[ m_ItemsColumn.m_seller_id ] = item.seller_id;
	row[ m_ItemsColumn.m_item_id ] = item.id;
	
	m_shoppingCart.push_back( item.id );
	
	// clear fields
	m_pCashoutEntryLookupSeller->set_text( "" );
	m_pCashoutEntryLookupItem->set_text( "" );
	m_pCashoutEntryLookupSeller->grab_focus();

	CheckoutCartChanged();
}

void BurzaDialogs::CheckoutCartChanged( void )
{
	ItemDataArray items = Centrala::Storage().GetItems( m_shoppingCart );
	m_cashout_sum = Centrala::Register().CalcPriceOfProducts( items );
	
	std::string	str;
	str = ConvFuncsInstance->FormatCstr( _("Suma: %0.2f" ), m_cashout_sum );
	str += " ";
	str += Centrala::Settings().m_string_mena;
	m_pCashoutLabelSum->set_text( str );
}

void BurzaDialogs::CashoutButtonFinished( void )
{
	if ( ! DisplayYesNoMessageBox( _("Dokončiť nákup?"), "" )) {
		return;
	}
	
	ItemDataArray items = Centrala::Storage().GetItems( m_shoppingCart );
	mp_register->FinishCheckout( items );
	
	m_CashoutItemsListModel->clear();
	m_shoppingCart.clear();
	CheckoutCartChanged();
}

void BurzaDialogs::CashoutButtonRemoveItem( void )
{
	vector<unsigned int> itemIndexes;
	
	// get selected items into list
	GetSelectedItemNumbers( m_CashoutItemsSelection, itemIndexes );
	
	// sort indexes of the items from biggest index to smallest, so they 
	// are removed correctly from the visual list
	std::sort(itemIndexes.begin(), itemIndexes.end());
	std::reverse(itemIndexes.begin(), itemIndexes.end());
	
	// remove items
	for(int i=0; i<itemIndexes.size(); ++i ) {
		
		Gtk::TreeModel::iterator it = m_CashoutItemsListModel->children()[ itemIndexes[ i ] ];
		m_CashoutItemsListModel->erase( it );
		m_shoppingCart.erase( m_shoppingCart.begin() + itemIndexes[ i ] );
	}
	
	CheckoutCartChanged();
}

void BurzaDialogs::CashoutComboLookupSellerChanged( void )
{
	int i = m_pCashoutComboLookupSeller->get_active_row_number();
	
	if ( i == -1 ) {
	  	m_CashoutSelectedSellerId = SELLER_ID_NONE;
		return;
	}
	
	if ( i >= m_DisplayedSellers.size() ) {
		assert( false );
		return;
	}
	
	m_CashoutSelectedSellerId = m_DisplayedSellers[ i ].id;
	CashoutSellerChanged();
}

void BurzaDialogs::CashoutComboLookupItemChanged( void )
{
	m_CashoutSelectedItemId = ITEM_ID_NONE;
  
	if ( ! CashoutAnySelectedSeller() )
		return;
	
	int i = m_pCashoutComboLookupItem->get_active_row_number();
	if ( i == -1 )
		return;
	
	if ( i >= m_CashoutDisplayedItems.size() ) {
		assert( false );
		return;
	}
	
	m_CashoutSelectedItemId = m_CashoutDisplayedItems[ i ].id;
	
	CashoutSelectedItemChanged();
}

void BurzaDialogs::CashoutSelectedItemChanged( void )
{
	if ( ! CashoutAnySelectedItem() ) {
		m_pCashoutLabelItemPrice->set_text( "" );
		return;
	}
	
	std::string		str;
	str = ConvFuncsInstance->FormatCstr( "%0.2f %s", 
		CashoutGetSelectedItem().price, 
		DataAccessInterface::ItemStatus2String( CashoutGetSelectedItem().status ) );
	m_pCashoutLabelItemPrice->set_text( str );

	if ( CashoutGetSelectedItem().status == on_stock ) {
		HighlightWidgetOff( *m_pCashoutLabelItemPrice );
	} else {
		HighlightWidgetRed( *m_pCashoutLabelItemPrice );
	}
}

void BurzaDialogs::CashoutSellerChanged( void )
{
	m_pCashoutComboLookupItem->clear();
	m_CashoutDisplayedItems.clear();
	
	if ( ! CashoutAnySelectedSeller() )
		return;
	
	std::string		str;
	
	ItemDataArray items = DemoLimitsGetSellerItems( CashoutGetSelectedSellerId() );

	// XXX ERROR: GTKMM can't handle too long items in combo or some chars probably?
	// so have to cut down.
	const int maxStrLen = 70;
	
	for( int i = 0; i < items.size(); ++i ) {
		str = ConvFuncsInstance->FormatCstr( "%i - %s", 
			items[ i ].number,
			items[ i ].name.c_str() );

		if (str.size() > maxStrLen )
			str.erase( str.begin() + maxStrLen, str.end() );

		m_pCashoutComboLookupItem->append_text( str );
		m_CashoutDisplayedItems.push_back( items[i] );
	}
}

void BurzaDialogs::DisplayErrorMessageBox( string msg1, string msg2 )
{
	Gtk::MessageDialog dialog( "" );
	
	if ( msg2.empty() )
		dialog.set_message( "error" );
	else
		dialog.set_message( msg2 );
	
	dialog.set_secondary_text( msg1 );
	dialog.run();
}

void BurzaDialogs::DisplayErrorMessageBox( const char* msg, const char* msg_title )
{
	string msg2, msg_title2;
	msg2 = ( msg == NULL ) ? "no msg" : msg;
	msg_title2 = ( msg_title == NULL ) ? "" : msg_title;
	DisplayErrorMessageBox( msg2, msg_title2 );
}

bool BurzaDialogs::DisplayYesNoMessageBox( string message, string title )
{
	Gtk::MessageDialog dialog( message, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true );
	
	dialog.set_title( title );
	dialog.set_secondary_text( title );
	int res = dialog.run();
	return ( res == Gtk::RESPONSE_YES );
}

void BurzaDialogs::DisplayOkMessageBox( string message, string title )
{
	Gtk::MessageDialog dialog( message, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK, true );
	
	dialog.set_title( title );
	dialog.set_secondary_text( title );
	int res = dialog.run();
}

void BurzaDialogs::RegisterStartDialogEditProduct( ITEM_ID itemid )
{
	ItemData item;
	item.seller_id = RegisterGetSelectedSellerId();
	
	if ( itemid != ITEM_ID_NONE ) {
		try {
			item = Centrala::Storage().GetItem( itemid );
		}
		catch(WrongItemException*)
		{
			throw runtime_error( "BurzaDialogs::RegisterStartDialogEditProduct item not in list" );
		}
	} else {
		item.category = 0;
		item.name = _("Názov");
		item.status = on_stock;
	}

	ItemCategory cat = Centrala::Storage().GetItemCategory( item.category );
	
	m_pItemeditName->set_text( item.name );
	m_pItemeditNumber->set_text( ConvFuncs::FormatCstr( "%i", item.number ) );
	m_pItemeditPrice->set_text( ConvFuncs::FormatCstr( "%0.2f", item.price ) );
	m_pItemeditCategory->set_text( ConvFuncs::FormatCstr( "%c", cat.letter ) );
	
	m_pItemeditStatusCombo->clear();
	for( int i = not_in_stock_yet; i < ItemStatusCount; ++i ) {
		m_pItemeditStatusCombo->append_text( 
			DataAccessInterface::ItemStatus2String( (ItemStatus)i ) );
	}
	m_pItemeditStatusCombo->set_active( item.status );
	
	m_EditedItem = item;
	try {
		m_EditedSeller = Centrala::Storage().GetSeller( item.seller_id );
	}
	catch(exception& e)
	{
		ErrorHandling::HandleException(e);
	}
	
	m_pWndItem->show();
	HighlightEditItemOff();
}

void BurzaDialogs::HighlightWidgetRed( Gtk::Widget& widget )
{
	widget.signal_focus_in_event();
	widget.drag_highlight();
	Gdk::Color red( "red" );
	Gdk::Color pink( "pink" );
	widget.modify_bg(Gtk::STATE_NORMAL, red);
	widget.modify_base(Gtk::STATE_NORMAL, pink);
}

void BurzaDialogs::HighlightWidgetOff( Gtk::Widget& widget )
{
	Gdk::Color white( "white" );
	Gdk::Color black( "black" );
	widget.modify_bg(Gtk::STATE_NORMAL, black);
	widget.modify_base(Gtk::STATE_NORMAL, white);
}

void BurzaDialogs::RegisterButtonItemEditOk( void )
{
	// validate
	bool validate_needed = false;
	HighlightEditItemOff();
	string str;
	
	if ( m_pItemeditName->get_text().empty() ) {
		m_pItemeditName->set_text( _("Popis predmetu") );

		HighlightWidgetRed( *m_pItemeditName );
		validate_needed = true;
	}
	
	str = m_pItemeditPrice->get_text();
	if ( ConvFuncsInstance->IsNumber( str.c_str() ) == 0 ) {
		
		ConvFuncsInstance->StrReplaceChars( str, ",", "." );
		
		if ( ConvFuncsInstance->IsNumber( str.c_str() ) == 1 ) {
			m_pItemeditPrice->set_text( str );
		} else {
			m_pItemeditPrice->set_text( "1.0" );
		}

		HighlightWidgetRed( *m_pItemeditPrice );
		validate_needed = true;
	} else {
		
		if ( atof( str.c_str() ) < 0 ) {
			m_pItemeditPrice->set_text( "1.0" );
			HighlightWidgetRed( *m_pItemeditPrice );
			validate_needed = true;
		}
	}
	
	if ( m_pItemeditName->get_text().empty() ) {
		m_pItemeditName->set_text( _("Popis predmetu") );
		HighlightWidgetRed( *m_pItemeditName );
		validate_needed = true;
	}
	
	// number
	str = m_pItemeditNumber->get_text();
	int num = atoi( str.c_str() );
	if ( num < 1 ) {
		num = 1;
	}
	
	num = Centrala::Register().VerifyItemNumber( m_EditedItem, num );
	string str2 = ConvFuncs::FormatStdStr( "%i", num );
	if ( str != str2 ) {
		m_pItemeditNumber->set_text( str2 );
		HighlightWidgetRed( *m_pItemeditNumber );
		validate_needed = true;
	}
	
	if ( validate_needed ) {
		return;
	}
	
	m_pWndItem->hide();
	
	m_EditedItem.name = m_pItemeditName->get_text();
	m_EditedItem.number =  atoi( m_pItemeditNumber->get_text().c_str() );
	m_EditedItem.price = atof(  m_pItemeditPrice->get_text().c_str() );
	
	
	if ( m_pItemeditCategory->get_text().empty() )
		m_EditedItem.category = 0;
	else
		m_EditedItem.category = 0;

	int i = m_pItemeditStatusCombo->get_active_row_number();
	if ( i != -1 ) {
		m_EditedItem.status = (ItemStatus) i;
	}

	// add new item, or modify edited item
	int action = ( m_EditedItem.id == ITEM_ID_NONE ) ? ea_add : ea_modify;
	
	switch ( action ) {
	case ea_add:

		// validate if we can add item 
		if ( DemoLimitsCanAddItem( RegisterGetSelectedSellerId() ) ) {

			m_EditedItem.id = Centrala::Storage().AddItem(m_EditedItem, m_EditedSeller.id);

			// after adding new item - start adding another one.
			RegisterStartDialogEditProduct( ITEM_ID_NONE );
		}

		break;
		
	case ea_modify:
		Centrala::Storage().ModifyItem(m_EditedItem.id, m_EditedItem);
		break;
	}

	RegisterDisplaySeller( m_EditedSeller );
}

void BurzaDialogs::RegisterButtonItemEditCancel( void )
{
	m_pWndItem->hide();
	m_EditedItem.id = ITEM_ID_NONE;
	m_EditedSeller.id = SELLER_ID_NONE;
}

bool BurzaDialogs::RegisterButtonItemEditClose( GdkEventAny* )
{
	RegisterButtonItemEditCancel(); 
	return true;
}

void BurzaDialogs::HighlightEditItemOff( void )
{
	HighlightWidgetOff( *m_pItemeditName );
	HighlightWidgetOff( *m_pItemeditPrice );
	HighlightWidgetOff( *m_pItemeditName );
	HighlightWidgetOff( *m_pItemeditNumber );
}

void BurzaDialogs::HighlightEditSellerOff( void )
{
	HighlightWidgetOff( *m_pSellerEditName );
	HighlightWidgetOff( *m_pSellerEditCode );
	HighlightWidgetOff( *m_pSellerEditNote );
}

void BurzaDialogs::RegisterComboItemEditStatusChanged( void )
{
}

void BurzaDialogs::RegisterStartDialogEditSeller( SELLER_ID id )
{
	SellerData seller; 
	
	if ( id != SELLER_ID_NONE ) {
		try {
			seller = Centrala::Storage().GetSeller( id );
		}
		catch(exception& e)
		{
			ErrorHandling::HandleException(e);
		}

		Centrala::Storage().ModifySellerStart( id );
	} else {
		seller.name = _( "Nový predajca" );
		seller.note = _( "za MC prijal: !zatiaľ neprebrané!" );
		seller.category = 0;
		seller.id = SELLER_ID_NONE;
	}
	
	m_pSellerEditName->set_text( seller.name );
	m_pSellerEditCode->set_text( seller.shortcode );
	m_pSellerEditNote->set_text( seller.note );
	
	HighlightEditSellerOff();

	m_EditedSeller = seller;
	m_pWndSeller->show();
}

void BurzaDialogs::RegisterButtonSellerEditOk( void )
{
	// validate
	string str;
	
	if ( m_pSellerEditName->get_text().empty() ) {
		m_pSellerEditName->set_text( _( "Nový predajca" ) );
		HighlightWidgetRed( *m_pSellerEditName );
		return;
	}
	
	string sellerOriginalCode = m_EditedSeller.shortcode;
	str = m_pSellerEditCode->get_text();
	// check new code and set it to seller
	str = Centrala::Storage().GenerateSellerUniqueCode( m_EditedSeller.name, str, m_EditedSeller.id );
	if ( m_pSellerEditCode->get_text() != str ) {
		m_EditedSeller.shortcode = str;
		m_pSellerEditCode->set_text( m_EditedSeller.shortcode );
		HighlightWidgetRed( *m_pSellerEditCode );
		return;
	}
	
	m_EditedSeller.name = m_pSellerEditName->get_text();
	m_EditedSeller.note = m_pSellerEditNote->get_text();
	m_EditedSeller.shortcode = m_pSellerEditCode->get_text();
	
	m_pWndSeller->hide();
	
	int action = m_EditedSeller.id == SELLER_ID_NONE ? ea_add : ea_modify;
	if ( action == ea_add ) {

		m_EditedSeller.id = Centrala::Storage().AddSeller( m_EditedSeller );

	} else {
		Centrala::Storage().ModifySellerEnd( m_EditedSeller.id, m_EditedSeller );
	}
	
	RegisterSellerModified( m_EditedSeller );
	
	// start adding items right away !
	m_RegisterSelectedSellerId = m_EditedSeller.id;

	if ( action == ea_add ) {
		// uses m_EditedSeller
		RegisterStartDialogEditProduct( ITEM_ID_NONE );
	}
}

void BurzaDialogs::RegisterButtonSellerEditCancel( void )
{
	m_pWndSeller->hide();
}

bool BurzaDialogs::RegisterButtonSellerEditClose( GdkEventAny* )
{
	RegisterButtonSellerEditCancel(); 
	return true; 
}

void BurzaDialogs::RegisterSellerModified( SellerData& seller )
{
	ReloadSellers();
	RegisterDisplaySeller( seller.shortcode.c_str() );
}

void BurzaDialogs::CashoutEditLookupSellerChanged( void )
{
	string str = m_pCashoutEntryLookupSeller->get_text();
	CashoutDisplaySeller( str.c_str() );
}

void BurzaDialogs::CashoutDisplaySeller( const char* shortcode )
{
	int idx = DataAccessInterface::FindSellerIndexByCode( m_DisplayedSellers, shortcode );
	
	m_pCashoutComboLookupItem->clear();
	m_CashoutDisplayedItems.clear();
	
	if ( idx == -1 ) {
		m_pCashoutComboLookupSeller->set_active( -1 );
		m_CashoutSelectedSellerId = SELLER_ID_NONE;
	} else {
		m_pCashoutComboLookupSeller->set_active( idx );
		m_CashoutSelectedSellerId = m_DisplayedSellers[ idx ].id;
	}
	
	CashoutSellerChanged();
}

void BurzaDialogs::CashoutEditLookupItemChanged( void )
{
	if ( m_CashoutSelectedSellerId == SELLER_ID_NONE )
		return;
	
	// set it into combo
	int num = atoi( m_pCashoutEntryLookupItem->get_text().c_str() );
	int idx;
	
	for( idx = 0; idx < m_CashoutDisplayedItems.size(); ++idx ) {
		if ( m_CashoutDisplayedItems[ idx ].number == num )
			break;
	}
	
	if ( idx >= m_CashoutDisplayedItems.size() ) {
		idx = -1;
	}
	
	m_pCashoutComboLookupItem->set_active( idx );
	// and let combo to do the work.
	CashoutComboLookupItemChanged();
}

void BurzaDialogs::CashoutEditSearchChanged( void )
{
	int selectedItemIdx = m_pCashoutComboSearch->property_active();

	if ( selectedItemIdx == -1 ) {
		return;
	}

	// get the selected item data
	if (( selectedItemIdx < 0 ) || ( selectedItemIdx >= cashoutComboSearchItemDatas.size() )) {
		assert( false );
	}

	ItemData item = cashoutComboSearchItemDatas[ selectedItemIdx ];

	try {
		SellerData seller = Centrala::Storage().GetSeller( item.seller_id );
		
		m_pCashoutEntryLookupSeller->set_text( seller.shortcode );
		m_pCashoutEntryLookupItem->set_text( ConvFuncs::FormatCstr( "%i", item.number ) );
	}
	catch(exception& e)
	{
		ErrorHandling::HandleException(e);
	}
}

void BurzaDialogs::CashoutEditSearchEntryChanged( void )
{
	vector<ItemData> foundItems;
	string str;
	ItemSearch& search = Centrala::Search();
	SellerData sellerOfItem;

	m_pCashoutComboSearch->clear();
	cashoutComboSearchItemDatas.clear();

	string text = m_pCashoutComboSearchEntry->get_text();
	foundItems = search.LookForItemsContainingWords( text );

	for( int i=0; i<foundItems.size(); ++i ) {

		sellerOfItem = search.GetSeller( foundItems[ i ].seller_id );

		str = ConvFuncs::FormatStdStr("%s - %i - %s - %0.2f - %s",
			sellerOfItem.shortcode.c_str(), 
			foundItems[ i ].number, 
			foundItems[ i ].name.c_str(),
			foundItems[ i ].price,
			DataAccessInterface::ItemStatus2String( foundItems[ i ].status ));

		m_pCashoutComboSearch->append_text( str );
		cashoutComboSearchItemDatas.push_back( foundItems[ i ] );
	}

	/*
	if ( ! foundItems.empty() ) {
		m_pCashoutComboSearch->popup();
	}
	m_pCashoutComboSearchEntry->activate();
	m_pCashoutComboSearchEntry->grab_focus();
	*/
}

/*
void BurzaDialogs::RegisterButtonSettings( void )
{
	settingsWindow.dialog->run();
}
*/

void BurzaDialogs::RegisterButtonOtherActions( void )
{
	menuRegisterOtherActions->popup( 0, 0 );
}

bool BurzaDialogs::RegisterButtonStats( GdkEventButton* button )
{
	string offeredName = BurzaDialogsFileHelper::FormatDateTimeFileName( _("Predajcovia"), "pdf" );
	string name = BurzaDialogsFileHelper::AskToSaveDocumentFile( offeredName );
	if ( name.empty() )
		return false;

	try {
		HPDFWrapper	pdf;
		pdf.CreatePDF( name );
		Recipees::PrintSummaryOfSellers( pdf );
		pdf.ClosePDF();
		
		Centrala::Register().DisplayPdf( name.c_str() );
	} 
	catch( std::exception& e ) {
		BurzaDialogsFileHelper::HandlePdfExceptions( e );
	}

	return true;
}

bool BurzaDialogs::RegisterButtonSaveSellerOverview( GdkEventButton* button )
{
	RegisterButtonReviewOfSellers();
	return true;
}

bool BurzaDialogs::RegisterButtonSettings( GdkEventButton* button )
{
	settingsWindow.dialog->run();
	return true;
}

void BurzaDialogs::SettingsClickedButtonRequestLicence( void )
{
//	MessageEditBox
	std::string vlastnik = "[vlastník]";
	MessageEditBox( "Prosím zadajte názov vlastníka licencie", vlastnik );
}

void BurzaDialogs::SettingsClickedButtonInstallLicence( void )
{

}

void BurzaDialogs::EditMessageBoxClickedOk( void )
{

}

void BurzaDialogs::EditMessageBoxClickedCancel( void )
{

}

bool BurzaDialogs::MessageEditBox( std::string message, std::string& edit)
{
	return EditMessageBox::ShowDialog( message, edit );
}

void BurzaDialogs::CreateStatusImages( void )
{
	#define ColorToPixel( red, green, blue ) ((guint32)( red << 24 | green << 16 | blue << 8 ))
	int sizex = 10;
	int sizey = 24;

	for( int i=0; i < ItemStatusCount; ++i ) 
	{
		registerColumnStateImages[ i ] = Gdk::Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB, FALSE, 8, sizex, sizey);
	}

	registerColumnStateImages[ not_in_stock_yet ]->fill( ColorToPixel( 50, 50, 255 ) );
	registerColumnStateImages[ on_stock ]->fill( ColorToPixel( 100, 255, 100 ) );
	registerColumnStateImages[ sold ]->fill( ColorToPixel( 128, 128, 128 ) );
	registerColumnStateImages[ returned ]->fill( ColorToPixel( 50, 50, 50 ) );
	registerColumnStateImages[ given_as_gift ]->fill( ColorToPixel( 255, 0, 255 ) );
	registerColumnStateImages[ lost ]->fill( ColorToPixel( 255, 0, 255 ) );
	registerColumnStateImages[ undefined ]->fill( ColorToPixel( 255, 0, 0 ) );
}

void BurzaDialogs::on_column_checkbox_edited( Glib::ustring rowNumberText )
{
	int row = atoi( rowNumberText.c_str() );

	ItemDataArray items = Centrala::Storage().GetSellerItems(m_RegisterSelectedSellerId);
	if ( row > items.size() ) {
		string s;
		DisplayErrorMessageBox( "chyba" );
		return;
	}

	ItemData item = items[ row ];

	item.status = 
		( item.status == Burza::on_stock ) ?
		Burza::not_in_stock_yet : Burza::on_stock;
	
	Centrala::Storage().ModifyItem(item.id, item);
	RegisterDisplaySeller( RegisterGetSelectedSeller() );
}

void BurzaDialogs::SettingsClickedButtonOk( void )
{
	Centrala::Settings().m_name_of_the_sale = settingsWindow.editBurzaName->get_text();
		
	Centrala::Settings().SaveSettings();
}

void BurzaDialogs::RegisterButtonReviewOfSellers( void )
{
	// offer save dialog with location of the document
	string offeredName = BurzaDialogsFileHelper::FormatDateTimeFileName( _("PrehladPredajcov"), "pdf" );
	string name = BurzaDialogsFileHelper::AskToSaveDocumentFile( offeredName );
	if ( name.empty() )
		return;
	
	bool storingFinished = false;

	try {
		Recipees recipees;
		HPDFWrapper	pdf;
		pdf.CreatePDF( name );
		
		for( unsigned int i=0; i<m_DisplayedSellers.size(); i++ ) {
			recipees.PrintSellerReturnSheet( pdf, m_DisplayedSellers[ i ].id );
		}
		
		pdf.ClosePDF();

		storingFinished = true;
		
	}
	catch( std::exception& e ) {
		BurzaDialogsFileHelper::HandlePdfExceptions( e );
	}

	if (storingFinished) {
		Centrala::Register().DisplayPdf( name.c_str() );
	}
}

bool BurzaDialogs::DemoLimitsCanAddSeller( void )
{
	if ( !Licence::Instance().IsDemo() )
		return true;

	int sellerLimit = Licence::Instance().GetLimitSellers();
	if ( Centrala::Storage().GetSellers().size() < sellerLimit )
		return true;

	string str = ConvFuncs::FormatStdStr( 
		_( "Vo verzii DEMO je možné pracovať s maximálne %i predajcami." ), sellerLimit );
	DisplayOkMessageBox( str, _( "Obmedzená demo verzia" ) );

	return false;
}

bool BurzaDialogs::DemoLimitsCanAddItem( SELLER_ID sellerId )
{
	if (!Licence::Instance().IsDemo())
		return true;

	int itemLimit = Licence::Instance().GetLimitItems();

	if ( Centrala::Storage().GetSellerItems( sellerId ).size() < itemLimit )
		return true;

	string str = ConvFuncs::FormatStdStr( 
		_( "Vo verzii DEMO je možné pracovať s maximálne %i kusmi tovaru pre predajcu." ), itemLimit );
	DisplayOkMessageBox( str, _( "Obmedzená demo verzia" ) );

	return false;
}

SellerDataArray& BurzaDialogs::DemoLimitsGetSellers( void )
{
	SellerDataArray sellers = Centrala::Storage().GetSellers();

	if ( Licence::Instance().IsDemo() ) {

		int sellerLimit = Licence::Instance().GetLimitSellers();
		if ( sellers.size() > sellerLimit )
			sellers.erase( sellers.begin() + sellerLimit, sellers.end() );

		string str = ConvFuncs::FormatStdStr( 
			_( "Vo verzii DEMO je možné pracovať s maximálne %i predajcami." ),
			sellerLimit );
		SellerData seller( str, "XX", str, 0 );

		sellers.push_back( seller );
	}

	return sellers;
}

ItemDataArray& BurzaDialogs::DemoLimitsGetSellerItems( SELLER_ID sellerId )
{
	ItemDataArray items = Centrala::Storage().GetSellerItems( sellerId );

	if ( Licence::Instance().IsDemo() ) {

		int itemLimit = Licence::Instance().GetLimitItems();
		if ( items.size() > itemLimit ) {
			items.erase( items.begin() + itemLimit, items.end() );

			string str = ConvFuncs::FormatStdStr( 
				_( "Vo verzii DEMO je možné pracovať s maximálne %i tovarmi." ),
				itemLimit );

			ItemData item( str, -1, 1.0, 0, ItemStatus::given_as_gift );
			items.push_back( item );
		}
	}

	return items;
}

bool BurzaDialogs::AskToFinishAcceptingSellerChanges( void )
{
	// ask only if user is not accepted yet
	SellerData seller = RegisterGetSelectedSeller();
	if (seller.status != SellerStatus::imported) 
		return true;

	// first ask if user wants to finish
	if ( DisplayYesNoMessageBox( 
			_( "Chceš zmeniť predajcu na 'prijatého'?" ), 
			_( "Importovaný na prijatý." ) ) == false )
		return false;

	// then ask for name of person who finished it.
	if ( false == MessageEditBox( _( "Kto prebral tovar tohoto predajcu?" ), 
		personAcceptingItems ) )
		return false;
	
	
}

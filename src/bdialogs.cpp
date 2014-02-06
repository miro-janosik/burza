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
#include "log.h"

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

	initializing = true;
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
	if (m_RegisterSelectedSellerId != SELLER_ID_NONE )
	{
		try {
			return Centrala::Storage().GetSeller( m_RegisterSelectedSellerId );
		}
		catch(exception& e)
		{
			ErrorHandling::HandleException(e);
		}
	}
	
	throw runtime_error("No seller selected.");
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
		error = "Dialog file is corrupt [windows].";
		throw;
	}
	
	m_pWndRegister->show();
	m_pWndRegister->maximize();
	
	xmlWndRegister->get_widget_derived( "combobox seller", m_pComboSeller );
	xmlWndRegister->get_widget( "entry_find_seller", m_pEntryRegistryLookupSeller );
	xmlWndRegister->get_widget( "button import seller", m_pButtonImportSeller );
	xmlWndRegister->get_widget( "button import multi seller", m_pButtonImportMultipleSellers );
	xmlWndRegister->get_widget( "button add seller", m_pButtonAddSeller );
	xmlWndRegister->get_widget( "button update seller", m_pButtonUpdateSeller );
	xmlWndRegister->get_widget( "treeview items", m_pTreeViewItems );
	xmlWndRegister->get_widget( "button add item", m_pButtonAddItem );
	xmlWndRegister->get_widget( "button modify item", m_pButtonModifyItem );
	xmlWndRegister->get_widget( "button remove item", m_pButtonRemoveItem );
	xmlWndRegister->get_widget( "button toggle accepted", m_pButtonAcceptUnaccept );
	// xmlWndRegister->get_widget( "button calc fee", m_pButtonCalcFee );
	xmlWndRegister->get_widget( "button calc repayment", m_pButtonCalcRepayment );
	xmlWndRegister->get_widget( "label seller type", m_pLabelSellerType );
	xmlWndRegister->get_widget( "label seller note", m_pLabelSellerNote );
	xmlWndRegister->get_widget( "button cashout", m_pButtonCashout );
	// xmlWndRegister->get_widget( "button_review_of_sellers", m_pRegisterButtonReviewOfSellers );
	// xmlWndRegister->get_widget( "button_settings", m_pButtonSettings );
	xmlWndRegister->get_widget( "button_other_actions", m_pRegisterOtherActions );
	xmlWndRegister->get_widget( "radiobuttonSellerStateImported", m_pRegisterRadioSellerStatus1NotImported );
	xmlWndRegister->get_widget( "radiobuttonSellerStateSelling", m_pRegisterRadioSellerStatus2ImportedSelling );
	xmlWndRegister->get_widget( "radiobuttonSellerStateReturned", m_pRegisterRadioSellerStatus3ReturnedBack );
	
	xmlWndRegisterPopupMenu->get_widget( "menuRegisterOtherActions", menuRegisterOtherActions );
	xmlWndRegisterPopupMenu->get_widget( "menuItemSettings", menuItemSettings );
	xmlWndRegisterPopupMenu->get_widget( "menuItemSaveSellerOverview", menuItemSaveSellerOverview );
	xmlWndRegisterPopupMenu->get_widget( "menuitemStats", menuitemStats );
	
	if (( m_pComboSeller == NULL ) ||
		( m_pEntryRegistryLookupSeller == NULL ) ||
		( m_pButtonImportSeller == NULL ) ||
		( m_pButtonImportMultipleSellers == NULL ) ||
		( m_pButtonAddSeller == NULL ) ||
		( m_pButtonUpdateSeller == NULL ) ||
		( m_pTreeViewItems == NULL ) ||
		( m_pButtonAddItem == NULL ) ||
		( m_pButtonModifyItem == NULL ) ||
		( m_pButtonRemoveItem == NULL )) {
		error = "Dialog file is corrupt [register 1].";
		throw;
	}

	if (( m_pButtonAcceptUnaccept == NULL ) ||
		// ( m_pButtonCalcFee == NULL ) ||
		( m_pButtonCalcRepayment == NULL ) ||
		( m_pLabelSellerType == NULL ) ||
		( m_pLabelSellerNote == NULL ) ||
		( m_pButtonCashout  == NULL ) ||
		// ( m_pRegisterButtonReviewOfSellers == NULL ) ||
		// ( m_pButtonSettings == NULL ) ||
		( m_pRegisterOtherActions == NULL ) ||
		( menuRegisterOtherActions == NULL ) ||
		( menuItemSettings == NULL ) ||
		( menuItemSaveSellerOverview == NULL ) ||
		( menuitemStats == NULL )) {
		error = "Dialog file is corrupt [register 2].";
		throw;
	}

	if (( m_pRegisterRadioSellerStatus1NotImported == NULL ) ||
		( m_pRegisterRadioSellerStatus2ImportedSelling == NULL ) ||
		( m_pRegisterRadioSellerStatus3ReturnedBack == NULL )) {
		error = "Dialog file is corrupt [register 3].";
		throw;
	}

	
	m_pButtonImportSeller->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonImportSeller ) );
	m_pButtonImportMultipleSellers->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonImportMultiSellers ) );
	m_pButtonAddSeller->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonAddSeller ) );
	m_pButtonUpdateSeller->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonUpdateSeller ) );
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
	m_pEntryRegistryLookupSeller->signal_changed().connect( sigc::mem_fun( this, &BurzaDialogs::RegisterEditLookupSellerChanged ) );

	m_pRegisterRadioSellerStatus1NotImported->signal_toggled().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterSellerStatusRadioChanged ) );
	m_pRegisterRadioSellerStatus2ImportedSelling->signal_toggled().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterSellerStatusRadioChanged ) );
	m_pRegisterRadioSellerStatus3ReturnedBack->signal_toggled().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterSellerStatusRadioChanged ) );

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
		pColumn->set_sort_column( m_ItemsColumn.m_price );
		pColumn->set_min_width( 80 );
	}
	pColumn = m_pTreeViewItems->get_column( 5 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_category );
		pColumn->set_min_width( 50 );
		pColumn->set_max_width( 200 );
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
	xmlWndCashOut->get_widget( "search_found_text", m_pCashoutSearchFoundText );
	
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
	   ( m_pCashoutComboSearchEntry == NULL ) ||
	   ( m_pCashoutSearchFoundText == NULL )) {
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
	
	// m_pCashoutComboSearch->set(); xalign = left;

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
	// xmlWndModifyItem->get_widget( "itemCategory", m_pItemeditCategory );
	xmlWndModifyItem->get_widget_derived( "itemCategoryCombo", m_pItemeditCategoryCombo );
	xmlWndModifyItem->get_widget_derived( "itemStatus_combobox", m_pItemeditStatusCombo );
	xmlWndModifyItem->get_widget( "itemButton_ok", m_pItemeditOk );
	xmlWndModifyItem->get_widget( "itemButton_cancel", m_pItemeditCancel );
	
	if (( m_pItemeditName == NULL ) ||
		( m_pItemeditNumber == NULL ) ||
		( m_pItemeditPrice == NULL ) ||
		( m_pItemeditCategoryCombo == NULL ) ||
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

	xmlWndModifySeller->get_widget( "sellerName", sellerEdit.editName );
	xmlWndModifySeller->get_widget( "sellerCode", sellerEdit.editCode );
	xmlWndModifySeller->get_widget( "sellerNote", sellerEdit.editNote );
	xmlWndModifySeller->get_widget_derived( "sellerCategory", sellerEdit.comboCategory );
	xmlWndModifySeller->get_widget( "sellerButton_ok", sellerEdit.buttonOk );
	xmlWndModifySeller->get_widget( "sellerButton_cancel", sellerEdit.buttonCancel );
	
	if (( sellerEdit.editName == NULL ) ||
		( sellerEdit.editCode == NULL ) ||
		( sellerEdit.comboCategory == NULL ) ||
		( sellerEdit.editNote == NULL ) ||
		( sellerEdit.buttonOk == NULL ) ||
		( sellerEdit.buttonCancel == NULL )) {
		error = "Dialog file is corrupt [6=Seller edit].";
		throw;
	}
	
	sellerEdit.buttonOk->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonSellerEditOk ) );
	sellerEdit.buttonCancel->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonSellerEditCancel ) );
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
	
	EditMessageBox& msgBox = EditMessageBox::Instance();
	messageEditBoxWindowControls.buttonOk->signal_clicked().connect( sigc::mem_fun( msgBox, &EditMessageBox::EditMessageBoxClickedOk ) );
	messageEditBoxWindowControls.buttonCancel->signal_clicked().connect( sigc::mem_fun( msgBox, &EditMessageBox::EditMessageBoxClickedCancel ) );

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
	settingsWindow.xmlWindow->get_widget_derived( "combo_font_size", settingsWindow.comboFontSize );
	settingsWindow.xmlWindow->get_widget( "radio_1file", settingsWindow.radio1file );
	settingsWindow.xmlWindow->get_widget( "radio_2db", settingsWindow.radio2database );
	settingsWindow.xmlWindow->get_widget( "entry_dbaddress", settingsWindow.editDbAddress );
	settingsWindow.xmlWindow->get_widget( "entry_dbuser", settingsWindow.editDbUser );
	settingsWindow.xmlWindow->get_widget( "entry_dbpassword", settingsWindow.editDbPassword );
	settingsWindow.xmlWindow->get_widget( "entry_dbport", settingsWindow.editDbPort );

	if (( settingsWindow.dialog == NULL ) ||
		( settingsWindow.labelCurrentLicence == NULL ) ||
		( settingsWindow.buttonRequestLicence == NULL ) ||
		( settingsWindow.editBurzaName == NULL ) ||
		( settingsWindow.buttonOk == NULL ) ||
		( settingsWindow.buttonInstallLicence == NULL ) ||
		( settingsWindow.comboFontSize == NULL )) {
		error = "Dialog file is corrupt [Nastavenia].";
		throw;
	}

	if (( settingsWindow.radio1file == NULL ) ||
		( settingsWindow.radio2database == NULL ) ||
		( settingsWindow.editDbAddress == NULL ) ||
		( settingsWindow.editDbUser == NULL ) ||
		( settingsWindow.editDbPassword == NULL ) ||
		( settingsWindow.editDbPort == NULL )) {
		error = "Dialog file is corrupt [Nastavenia] part 2.";
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
	if ( idx >= SellersOrdered.size() ) {
		m_RegisterSelectedSellerId = SELLER_ID_NONE;

		DisplayOkMessageBox( _("Neexistuje žiaden predajca"), "" );
		return;
	}

	RegisterDisplaySeller(SellersOrdered[ idx ]);
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
	if ( m_RegisterSelectedSellerId != seller.id ) {
		if (!initializing)
			RegisterAskToFinishAcceptingSellerChanges();
	}

	m_RegisterSelectedSellerId = seller.id;
	
	m_pLabelSellerNote->set_label( seller.note );
	
	// fill items
	ItemDataArray	items = DemoLimitsGetSellerItems(seller.id);
	
	ItemCategoryArray	categories = Centrala::Storage().GetItemsCategories( items );
	ItemCategory itemCategory;
	
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

		row[ m_ItemsColumn.m_category ] = ConvFuncsInstance->FormatCstr( "%s", categories[i].name.c_str() );

		row[ m_ItemsColumn.m_state ] = ConvFuncsInstance->FormatCstr( "%s", 
			DataAccessInterface::ItemStatus2String( items[i].status ) );
		row[ m_ItemsColumn.m_state_pic ] = registerColumnStateImages[ items[i].status ];
		row[ m_ItemsColumn.m_state_checkbox ] = (items[i].status == Burza::on_stock);
	}
	
	int idx;
	
	for( idx = 0; idx < SellersOrdered.size(); idx++ ) {
		if ( SellersOrdered[ idx ].id == seller.id ) {
			break;
		}
	}
	
	if ( idx >= SellersOrdered.size() ) {
		idx = -1;
	}
	m_pComboSeller->set_active( idx );
	// i = m_pComboSeller->get_active_row_number();

	RegisterDisplaySellerUpdateSellerStatus( seller.status );

	SellerCategory sellerCat = Centrala::Storage().GetSellerCategory( seller.category );
	m_pLabelSellerType->set_text( sellerCat.name );
}

void BurzaDialogs::RegisterSortSellersByName(string& partOfName)
{
	sort( SellersOrdered.begin(), SellersOrdered.end(), OrderSellerByShortCode );
	
	for( int i=0; i<SellersOrdered.size(); ++i )
	{
		SellersOrdered[ i ].mark = "";
	}
	
	if (partOfName.size() <= 1 )
	{
	}
	else
	{
		string key = StringEncoding::ToComparable( partOfName );
		
		vector<SellerDataWithFilterMark> orderedSellers, otherSellers;
		
		// find sellers matching the key
		string text;
		for( int i=0; i<SellersOrdered.size(); ++i )
		{
			text = StringEncoding::ToComparable( SellersOrdered[ i ].name );
			if ( StringEncoding::StringContains( text, key ) )
			{
				SellersOrdered[ i ].mark = ">>  ";
				orderedSellers.push_back( SellersOrdered[ i ] );
				continue;
			}

			text = StringEncoding::ToComparable( SellersOrdered[ i ].shortcode );
			if ( StringEncoding::StringContains( text, key ) )
			{
				SellersOrdered[ i ].mark = ">>>  ";
				orderedSellers.push_back( SellersOrdered[ i ] );
				continue;
			}
			
			otherSellers.push_back( SellersOrdered[ i ] );
		}

		// Now get them together, add ordered first
		SellersOrdered.clear();
		for( int i=0; i<orderedSellers.size(); ++i )
		{
			SellersOrdered.push_back( orderedSellers[ i ] );
		}
		
		// Add rest of sellers at the end.
		for( int i=0; i<otherSellers.size(); ++i )
		{
			SellersOrdered.push_back( otherSellers[ i ] );
		}
	}
	
	FillSellerCombos();
}

void BurzaDialogs::RegisterDisplaySellerUpdateSellerStatus( Burza::SellerStatus status )
{
	switch ( status ) {
	case imported:
		m_pRegisterRadioSellerStatus1NotImported->set_active( true );
		break;
	case receivedSelling:
		m_pRegisterRadioSellerStatus2ImportedSelling->set_active( true );
		break;
	case handedOut:
		m_pRegisterRadioSellerStatus3ReturnedBack->set_active( true );
		break;
	}
}

void BurzaDialogs::ReloadSellers( void )
{
	LoadedSellers = DemoLimitsGetSellers();
	
	// transform Seller to SellerDataWithFilterMark
	SellersOrdered.clear();
	for( int i=0; i<LoadedSellers.size(); ++i )
		SellersOrdered.push_back( LoadedSellers[i] );
	
	// order sellers without criteria => by short code
	string nothing;
	RegisterSortSellersByName( nothing );
	
	FillSellerCombos();
}

void BurzaDialogs::FillSellerCombos( void )
{
	m_pComboSeller->clear();
	m_pCashoutComboLookupSeller->clear();
	
	std::string		str;
	for( int i=0; i<SellersOrdered.size(); ++i ) {
		str = SellersOrdered[ i ].mark;
		str += SellersOrdered[ i ].shortcode;
		str += " - ";
		str += SellersOrdered[ i ].name;

		m_pComboSeller->append_text( str );
		m_pCashoutComboLookupSeller->append_text( str );
	}
}

void BurzaDialogs::RegisterEditLookupSellerChanged( void )
{
	if ( SellersOrdered.empty() )
		return;
	
	string str = m_pEntryRegistryLookupSeller->get_text();
	
	SellerDataArray sellersBase;
	for( int i=0; i<SellersOrdered.size(); ++i )
		sellersBase.push_back(SellersOrdered[i]);
	
	int idx = DataAccessInterface::FindSellerIndexByCode( sellersBase, str );
	if ( idx != -1 ) {
		RegisterDisplaySeller( SellersOrdered[ idx ] );
	}
	else 
	{
		RegisterSortSellersByName( str );
		RegisterDisplaySeller( SellersOrdered[ 0 ] );
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

	BurzaRegister::LoadSellerSheetFileResult loadedFile = mp_register->LoadSellerSheetFile( filename );
	switch( loadedFile.status ) {
		case BurzaRegister::Loaded:
		// nested for the local variable [categories] inside case, error on gcc
		{
			if ( Centrala::Register().SellerExists( loadedFile.seller.shortcode ) ) {
				DisplayErrorMessageBox( 
					ConvFuncs::FormatCstr( _( "Predajca s rovnakým kódom už existuje. Zmažte najskôr predajcu '%s'." ),
						loadedFile.seller.shortcode.c_str() ) );
				return;
			}

			// imported items have to be checked by hand. Take note that these were not.
			loadedFile.seller.note += ""; // _(" za MC prijal: ?");
			
			SellerCategoryArray categories = Centrala::Storage().GetSellersCategories();
			loadedFile.seller.category = categories.empty() ? SELLER_CATEGORY_ID_NONE : categories.begin()->id;

			loadedFile.seller.id = Centrala::Storage().AddSeller( loadedFile.seller );
			
			// set correct category to items
			ItemCategoryArray itemCategories = Centrala::Register().GetItemCategoriesForMatching();
			for( int i=0; i<loadedFile.items.size(); ++i)
				Centrala::Register().SetItemBestMatchingProductCategory( loadedFile.items[i], itemCategories );
			
			Centrala::Storage().AddItems( loadedFile.items, loadedFile.seller.id );

			DisplayOkMessageBox( 
				_( "Predajca načítaný"),
				ConvFuncs::FormatStdStr( _("Kód: %s Meno: %s Počet položiek: %i"), 
					loadedFile.seller.shortcode.c_str(), loadedFile.seller.name.c_str(), loadedFile.items.size() ));
		}
			break;

		case BurzaRegister::FormatNotKnown:
			DisplayErrorMessageBox( _("Neznámy typ súboru. Podporované su 'ods', 'xls', 'xlsx', 'txt'") );
			return;
		case BurzaRegister::FormatNotSupported:
			DisplayErrorMessageBox( _("Nepodporovany format suboru") );
			return;
		case BurzaRegister::ErrorImporting:
		default:
			DisplayErrorMessageBox( string( "Error importing file." ) + loadedFile.errorText, "Error importing file." );
			break;
	}

	ReloadSellers();
	RegisterDisplaySeller( loadedFile.seller );
}

void BurzaDialogs::RegisterButtonImportMultiSellers( void )
{
	ImportMultiSellersLog( "Starting" );

	if (DemoLimitsCanAddSeller() == false)
		return;
	
	Gtk::FileChooserDialog dialog("Select folder with sheets", Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER );

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
	string path = Centrala::Settings().directoryImport;
	ImportMultiSellersLog( "Selected folder: " + path );

	// create output folders
	
	const char folderFinished[] = "finished";
	const char folderWarning[] = "warning";
	const char folderError[] = "error";
	vector<string> folderNames;
	folderNames.push_back( folderFinished );
	folderNames.push_back( folderWarning );
	folderNames.push_back( folderError );

	BurzaRegister::CreateFolders( path, folderNames );

	SellerCategoryArray categories = Centrala::Storage().GetSellersCategories();
	SELLER_CATEGORY_ID newSellerCategory = categories.empty() ? SELLER_CATEGORY_ID_NONE : categories.begin()->id;
	ItemCategoryArray itemCategories = Centrala::Register().GetItemCategoriesForMatching();
	
	// == loop through the files ==
	int finishedCount = 0, problemCount = 0;
	
	bool found = ConvFuncsInstance->Find_FirstFile( path.c_str() );
	while ( found )
	{
		// find can find anything in the folder .. skip directories, use only files.
		if (ConvFuncsInstance->Find_CurrentIsDir()) {
			// find next file
			found = ConvFuncsInstance->Find_NextFile();			
			continue;
		}

		string filename = path + ConvFuncsInstance->Find_CurrentFileName();

		// true if read (even if there is error then print info about read data).
		bool readAndValid = false;
		string error, warning, info;

		BurzaRegister::LoadSellerSheetFileResult loadedFile = mp_register->LoadSellerSheetFile( filename );
		switch( loadedFile.status ) {
			case BurzaRegister::Loaded:

				// even if file was loaded there may be some values that invalidate the content. Check these values.
				if ( Centrala::Register().SellerExists( loadedFile.seller.shortcode ) ) {
					error += ConvFuncs::FormatCstr( _( " Predajca s rovnakým kódom už existuje. Zmažte najskôr predajcu '%s'." ), loadedFile.seller.shortcode.c_str() );
					break;
				}

				if ( loadedFile.itemsWithZeroPrice > 0 )
					error += ConvFuncs::FormatCstr(_(" Obsahuje %i ks tovaru s nulovou cenou"), loadedFile.itemsWithZeroPrice );
				
				if ( loadedFile.itemsWithoutName > 0 )
					error += ConvFuncs::FormatCstr(_(" Obsahuje %i ks tovaru bez mena"), loadedFile.itemsWithoutName );

				if ( error.empty() )
				{
					readAndValid = true;

					if ( loadedFile.itemsWithPriceWithoutDecimalPart > 0 )
						info += ConvFuncs::FormatCstr(_(" Obsahuje %i ks tovaru s okruhlou cenou"), loadedFile.itemsWithPriceWithoutDecimalPart );

					// imported items have to be checked by hand. Take note that these were not.
					loadedFile.seller.note += ""; // _(" za MC prijal: ?");

					loadedFile.seller.category = newSellerCategory;
					loadedFile.seller.status = Burza::imported;
					
					loadedFile.seller.id = Centrala::Storage().AddSeller( loadedFile.seller );
					
					// set correct category to items
					for( int i=0; i<loadedFile.items.size(); ++i)
						Centrala::Register().SetItemBestMatchingProductCategory( loadedFile.items[i], itemCategories );
					
					Centrala::Storage().AddItems( loadedFile.items, loadedFile.seller.id );
				}
				break;
			case BurzaRegister::FormatNotKnown:
				error = _("Neznámy typ súboru. Podporované su 'ods', 'xls', 'xlsx', 'txt'");
				break;
			case BurzaRegister::FormatNotSupported:
				error = _("Nepodporovany format suboru");
				break;
			case BurzaRegister::ErrorImporting:
			default:
				error = "Error importing file." + loadedFile.errorText;
				break;
		}

		string infoText = _("Spracovaný súbor");
		infoText += " '";
		infoText += ConvFuncsInstance->Find_CurrentFileName();
		infoText += "';";

		if ( readAndValid ) {
			infoText += 
				ConvFuncs::FormatCstr(_(" načítaný, kód '%s', %i kusov tovaru;"),
				loadedFile.seller.shortcode.c_str(),
				loadedFile.items.size() );
			
			SellerCategoryArray categories = Centrala::Storage().GetSellersCategories();
			loadedFile.seller.category = categories.empty() ? SELLER_CATEGORY_ID_NONE : categories.begin()->id;
		}

		if ( !error.empty() )
		{
			infoText += _( " Chyba: " ) + error + ";";
		}

		if ( !warning.empty() )
		{
			infoText += _( " Varovanie: " ) + warning + ";";
		}

		if ( !info.empty() )
		{
			infoText += _( " Info: " ) + info + ";";
		}

		// move file to dedicated folder:
		string targetFolder;
		if ( !error.empty() )
		{
			targetFolder = folderError;
		}
		else if ( !warning.empty() )
		{
			targetFolder = folderWarning;
		}
		else {
			targetFolder = folderFinished;
		}

		string dest = path + targetFolder + "/" + ConvFuncsInstance->Find_CurrentFileName();
		if ( 0 == ConvFuncsInstance->DoMoveFile( filename.c_str(), dest.c_str() ) ) {
			infoText += _( " Chyba pri presune na miesto: " ) + dest;
			warning = "!!";
		}

		ImportMultiSellersLog( infoText );

		if ( warning.empty() && error.empty() )
			++finishedCount;
		else
			++problemCount;

		// find next file
		found = ConvFuncsInstance->Find_NextFile();
	}

	ImportMultiSellersLog( ConvFuncs::FormatCstr(
		_("Dokončené ... správne načítaných: %i chýb: %i."), finishedCount, problemCount) );

	DisplayOkMessageBox( 
		_( "Predajcovia načítaný"),
		ConvFuncs::FormatStdStr( _("Korektne načítaných hárkov: %i Chýb: %i."), 
			finishedCount, problemCount ));

	ReloadSellers();
}

void BurzaDialogs::ImportMultiSellersLog(const char* text)
{
	LogS( InfoMajor, "ImportMultiSellers: " );
	LogS( InfoMajor, text );
	LogS( InfoMajor, "\n" );
}

void BurzaDialogs::ImportMultiSellersLog(std::string text)
{
	ImportMultiSellersLog(text.c_str());
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

	RegisterAskToFinishAcceptingSellerChanges();

	/* this all is done in the call to RegisterAskToFinishAcceptingSellerChanges()
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
		seller.status = receivedSelling;

		Centrala::Storage().ModifySellerEnd( m_RegisterSelectedSellerId, seller );
	}
	catch(exception& e)
	{
		ErrorHandling::HandleException(e);
	}
	
	RegisterSellerModified( seller );

	personAcceptingItems = preberajuci;
	*/
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
	
	SellerData seller = RegisterGetSelectedSeller();
	ItemDataArray items = Centrala::Storage().GetSellerItems( RegisterGetSelectedSellerId() );
	
	if ( items.empty() )
		return;

	ItemStatus statusFrom, statusTo;

	switch ( seller.status ) {
	case imported:
		statusFrom = Burza::not_in_stock_yet;
		statusTo = Burza::on_stock;
		break;
	case receivedSelling:
		statusFrom = Burza::on_stock;
		statusTo = Burza::returned;
		break;
	}

	// ask user if he is sure
	string newStatusText = _("Tovar sa označí na ");
	newStatusText += DataAccessInterface::ItemStatus2String( statusTo );

	if ( DisplayYesNoMessageBox( 
		_("Ste si istý že chcete preznacit tovar?"), 
		newStatusText ) == false )
		return;

	for( int i=0; i<items.size(); ++i ) {
		
		if ( items[ i ].status == statusFrom )
		{
			items[ i ].status = statusTo;
			Centrala::Storage().ModifyItem( items[ i ].id, items[ i ] );
		}
	}
	
	// update seller

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
}

void BurzaDialogs::RegisterButtonCalcRepayment( void )
{
	// m_pLabelFee->set_text( string( "-" ) + Centrala::Settings().currencySign );
	
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

	} catch( std::exception& e ) {
		ErrorHandling::HandlePdfExceptions( e );
	}

	if (storingFinished) {
		Centrala::Register().DisplayPdf( filename.c_str() );
	}
}

void BurzaDialogs::RegisterButtonSwitchToCashout( void )
{
	if (!initializing)
		RegisterAskToFinishAcceptingSellerChanges();

	initializing = true;

	// m_pWndRegister->hide();
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

	initializing = false;
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
	// m_pWndRegister->show();
	m_pWndCashOut->hide();
	
	/*
	m_pWndRegister->unfullscreen();
	m_pWndRegister->maximize();
	m_pWndRegister->reset_shapes();
	*/
	
	initializing = true;

	ReloadSellers();

	initializing = false;
}

void BurzaDialogs::RegisterComboChangedSeller( void )
{
	int i = m_pComboSeller->get_active_row_number();
	
	if ( i == -1 )
		return;
	
	if ( i >= SellersOrdered.size() ) {
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
	str += Centrala::Settings().currencySign;
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
	
	if ( i >= SellersOrdered.size() ) {
		assert( false );
		return;
	}
	
	m_CashoutSelectedSellerId = SellersOrdered[ i ].id;
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

	m_pItemeditName->set_text( item.name );
	m_pItemeditNumber->set_text( ConvFuncs::FormatCstr( "%i", item.number ) );
	m_pItemeditPrice->set_text( ConvFuncs::FormatCstr( "%0.2f", item.price ) );
	
	m_pItemeditStatusCombo->clear();
	for( int i = not_in_stock_yet; i < ItemStatusCount; ++i ) {
		m_pItemeditStatusCombo->append_text( 
			DataAccessInterface::ItemStatus2String( (ItemStatus)i ) );
	}
	m_pItemeditStatusCombo->set_active( item.status );

	// fill 'category' combo
	ItemCategoryArray categories = Centrala::Storage().GetItemsCategories();

	int selectedCategoryIndex = -1;
	m_pItemeditCategoryCombo->clear();
	for( int i = 0; i < categories.size(); ++i ) {
		m_pItemeditCategoryCombo->append_text( categories[i].name );
		
		if ( categories[i].id == item.category )
			selectedCategoryIndex = i;
	}
	
	if ( selectedCategoryIndex != -1 )
		m_pItemeditCategoryCombo->set_active( selectedCategoryIndex );
	
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
	
	/*
	if ( m_pItemeditCategory->get_text().empty() )
		m_EditedItem.category = 0;
	else
		m_EditedItem.category = 0;
	*/

	int i = m_pItemeditStatusCombo->get_active_row_number();
	if ( i != -1 ) {
		m_EditedItem.status = (ItemStatus) i;
	}

	// item category
	int itemCategoryIndex = m_pItemeditCategoryCombo->get_active_row_number();
	ItemCategoryArray itemCategories = Centrala::Storage().GetItemsCategories();
	int itemCategoryId = -1;
	if (itemCategories.size() > itemCategoryIndex)
	{
		itemCategoryId = itemCategories[ itemCategoryIndex ].id;
	}
	
	if ( itemCategoryId != -1 ) {
		m_EditedItem.category = itemCategoryId;
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
	HighlightWidgetOff( *sellerEdit.editName );
	HighlightWidgetOff( *sellerEdit.editCode );
	HighlightWidgetOff( *sellerEdit.editNote );
}

void BurzaDialogs::RegisterComboItemEditStatusChanged( void )
{
}

void BurzaDialogs::RegisterStartDialogEditSeller( SELLER_ID id )
{
	SellerData seller; 
	vector<SELLER_CATEGORY_ID> ids;
	SellerCategoryArray categories = Centrala::Storage().GetSellersCategories();
	
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
		seller.note = ""; // _(" za MC prijal: ?");
		seller.id = SELLER_ID_NONE;
		
		SellerCategoryArray categories = Centrala::Storage().GetSellersCategories();
		seller.category = categories.empty() ? SELLER_CATEGORY_ID_NONE : categories.begin()->id;
	}
	
	sellerEdit.editName->set_text( seller.name );
	sellerEdit.editCode->set_text( seller.shortcode );
	sellerEdit.editNote->set_text( seller.note );

	sellerEdit.comboCategory->clear();
	sellerEdit.categories = categories;
	int index = 0;
	for( int i=0; i < categories.size(); ++i ) {
		sellerEdit.comboCategory->append_text( categories[ i ].name );

		if ( categories[ i ].id == seller.category )
			index = i;
	}

	sellerEdit.comboCategory->set_active( index );

	HighlightEditSellerOff();

	m_EditedSeller = seller;
	m_pWndSeller->show();
}

void BurzaDialogs::RegisterButtonSellerEditOk( void )
{
	// validate
	string str;
	
	if ( sellerEdit.editName->get_text().empty() ) {
		sellerEdit.editName->set_text( _( "Nový predajca" ) );
		HighlightWidgetRed( *sellerEdit.editName );
		return;
	}
	
	string sellerOriginalCode = m_EditedSeller.shortcode;
	str = sellerEdit.editCode->get_text();
	// check new code and set it to seller
	str = Centrala::Storage().GenerateSellerUniqueCode( m_EditedSeller.name, str, m_EditedSeller.id );
	if ( sellerEdit.editCode->get_text() != str ) {
		m_EditedSeller.shortcode = str;
		sellerEdit.editCode->set_text( m_EditedSeller.shortcode );
		HighlightWidgetRed( *sellerEdit.editCode );
		return;
	}
	
	m_EditedSeller.name = sellerEdit.editName->get_text();
	m_EditedSeller.note = sellerEdit.editNote->get_text();
	m_EditedSeller.shortcode = sellerEdit.editCode->get_text();
	
	// get category
	int categoryIndex = sellerEdit.comboCategory->get_active_row_number();
	assert( categoryIndex < sellerEdit.categories.size() );
	m_EditedSeller.category = sellerEdit.categories[ categoryIndex ].id;

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
	SellerDataArray SellersSearchable;
	for( int i=0; i<SellersOrdered.size(); ++i )
		SellersSearchable.push_back( SellersOrdered[ i ] );
		
	int idx = DataAccessInterface::FindSellerIndexByCode( SellersSearchable, shortcode );
	
	m_pCashoutComboLookupItem->clear();
	m_CashoutDisplayedItems.clear();
	
	if ( idx == -1 ) {
		m_pCashoutComboLookupSeller->set_active( -1 );
		m_CashoutSelectedSellerId = SELLER_ID_NONE;
	} else {
		m_pCashoutComboLookupSeller->set_active( idx );
		m_CashoutSelectedSellerId = SellersOrdered[ idx ].id;
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
	m_pCashoutSearchFoundText->set_text("");

	string text = m_pCashoutComboSearchEntry->get_text();
	foundItems = search.LookForItemsContainingWords( text );

	m_pCashoutSearchFoundText->set_text(ConvFuncs::FormatCstr(_("Nájdených: %i"), foundItems.size()));
	
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
	string offeredName = BurzaDialogsFileHelper::FormatDateTimeFileName( _("Statistika"), "pdf" );
	string name = BurzaDialogsFileHelper::AskToSaveDocumentFile( offeredName );
	if ( name.empty() )
		return false;

	try {
		HPDFWrapper	pdf;
		pdf.CreatePDF( name );
		Recipees::PrintStats( pdf );
		pdf.ClosePDF();
		
		Centrala::Register().DisplayPdf( name.c_str() );
	} 
	catch( std::exception& e ) {
		ErrorHandling::HandlePdfExceptions( e );
	}

	return true;
}

bool BurzaDialogs::RegisterButtonSaveSellerOverview( GdkEventButton* button )
{
	RegisterButtonReviewOfSellers();
	return true;
}

// ======================================================
// ======================================================

bool BurzaDialogs::RegisterButtonSettings( GdkEventButton* button )
{
	ProgramSettings* settings = & Centrala::Settings();

	// license
	string licenseText = Licence::Instance().IsDemo() ? _( "DEMO" ) : 
		Licence::Instance().GetLicenseUser();
	settingsWindow.labelCurrentLicence->set_text( licenseText );

	// setup 'font' combo
	settingsWindow.comboFontSize->clear();
	int selectedFontIndex = 0;

	for( int i = 0; i < ProgramSettings::predefined.listOfFontSizes.size(); ++i ) {
		settingsWindow.comboFontSize->append_text( 
			ProgramSettings::predefined.listOfFontSizes[ i ].text );
		if ( settings->fontSize == ProgramSettings::predefined.listOfFontSizes[ i ].size )
			selectedFontIndex = i;
	}

	settingsWindow.comboFontSize->set_active( selectedFontIndex );

	// name of "Burza"
	settingsWindow.editBurzaName->set_text( settings->m_name_of_the_sale );

	// database / file settings
	settingsWindow.radio1file->set_active( settings->databaseInLocalFiles );
	settingsWindow.radio2database->set_active( ! settings->databaseInLocalFiles );

	settingsWindow.editDbAddress->set_text( settings->dbAddress );
	settingsWindow.editDbUser->set_text( settings->dbUser );
	settingsWindow.editDbPassword->set_text( settings->dbPassword );
	settingsWindow.editDbPort->set_text( ConvFuncs::FormatStdStr( "%i", settings->dbPort ) );

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

void BurzaDialogs::SettingsClickedButtonOk( void )
{
	settingsWindow.dialog->response( Gtk::RESPONSE_OK );
	settingsWindow.dialog->hide();

	ProgramSettings	*settings = & Centrala::Settings();

	// name of "Burza"
	settings->m_name_of_the_sale = settingsWindow.editBurzaName->get_text();

	// font size
	int fontSizeSelectionIndex = settingsWindow.comboFontSize->get_active_row_number();
	int newFontSize = -1;
	if ( fontSizeSelectionIndex != -1 ) {
		newFontSize = ProgramSettings::predefined.listOfFontSizes[ fontSizeSelectionIndex ].size;
	}

	bool fontSizeChanged = false;
	if ( newFontSize != settings->fontSize ) {
		settings->fontSize = newFontSize;
		fontSizeChanged = true;
	}

	// database / file settings
	settings->databaseInLocalFiles = settingsWindow.radio1file->get_active();

	settings->dbAddress = settingsWindow.editDbAddress->get_text();
	settings->dbUser = settingsWindow.editDbUser->get_text();
	settings->dbPassword = settingsWindow.editDbPassword->get_text();
	settings->dbPort = atoi( settingsWindow.editDbPort->get_text().c_str() );

	settings->SaveSettings();
	settings->Apply();

	if ( fontSizeChanged ) {
		DisplayOkMessageBox( _("Zmena veľkosti písma sa prejaví pri ďalšom spustení programu") , _( "Veľkosť písma zmenená" ));
	}
}

// ======================================================
// ======================================================

bool BurzaDialogs::MessageEditBox( std::string message, std::string& edit)
{
	return EditMessageBox::ShowDialog( message, edit );
}

void BurzaDialogs::CreateStatusImages( void )
{
	#define ColorToPixel( red, green, blue ) ((guint32)( red << 24 | green << 16 | blue << 8 ))
	int sizex = 10;
	int sizey = 24;

	for( int i=0; i < ItemStatusCountWithUndefined; ++i ) 
	{
		registerColumnStateImages[ i ] = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, FALSE, 8, sizex, sizey);
	}

	registerColumnStateImages[ not_in_stock_yet ]->fill( ColorToPixel( 50, 50, 50 ) );
	registerColumnStateImages[ on_stock ]->fill( ColorToPixel( 100, 255, 100 ) );
	registerColumnStateImages[ sold ]->fill( ColorToPixel( 255, 190, 50 ) );
	registerColumnStateImages[ returned ]->fill( ColorToPixel( 50, 50, 255 ) );
	registerColumnStateImages[ given_as_gift ]->fill( ColorToPixel( 255, 0, 255 ) );
	registerColumnStateImages[ lost ]->fill( ColorToPixel( 255, 80, 80 ) );
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

	SellerData seller = RegisterGetSelectedSeller();
	ItemData item = items[ row ];

	switch (seller.status)
	{
	case imported:

		switch ( item.status )
		{
		case on_stock:
			item.status = not_in_stock_yet;
			break;
		case not_in_stock_yet:
			item.status = on_stock;
			break;
		default:
			// any other state can't be flipped by checkbox
			return;
		}
		break;
	case receivedSelling:

		switch ( item.status )
		{
		case on_stock:
			item.status = returned;
			break;
		case returned:
			item.status = on_stock;
			break;
		default:
			// any other state can't be flipped by checkbox
			return;
		}

		break;
	}

	Centrala::Storage().ModifyItem(item.id, item);
	
	RegisterDisplaySeller( seller );
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
		
		for( unsigned int i=0; i<SellersOrdered.size(); i++ ) {
			recipees.PrintSellerReturnSheet( pdf, SellersOrdered[ i ].id );
		}
		
		pdf.ClosePDF();

		storingFinished = true;
		
	}
	catch( std::exception& e ) {
		ErrorHandling::HandlePdfExceptions( e );
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
	str += "\n";
	str += Licence::forMoreInformation;
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
	str += "\n";
	str += Licence::forMoreInformation;
	DisplayOkMessageBox( str, _( "Obmedzená demo verzia" ) );

	return false;
}

SellerDataArray BurzaDialogs::DemoLimitsGetSellers( void )
{
	SellerDataArray sellers = Centrala::Storage().GetSellers();

	if ( Licence::Instance().IsDemo() ) {

		int sellerLimit = Licence::Instance().GetLimitSellers();
		if ( sellers.size() > sellerLimit )
			sellers.erase( sellers.begin() + sellerLimit, sellers.end() );
	}

	return sellers;
}

ItemDataArray BurzaDialogs::DemoLimitsGetSellerItems( SELLER_ID sellerId )
{
	ItemDataArray items = Centrala::Storage().GetSellerItems( sellerId );

	if ( Licence::Instance().IsDemo() ) {

		int itemLimit = Licence::Instance().GetLimitItems();
		if ( items.size() > itemLimit ) {
			items.erase( items.begin() + itemLimit, items.end() );

			/*
			string str = ConvFuncs::FormatStdStr( 
				_( "Vo verzii DEMO je možné pracovať s maximálne %i tovarmi." ),
				itemLimit );
			str += "\n";
			str += Licence::forMoreInformation;

			ItemData item( str, -1, 1.0, 0, given_as_gift );
			items.push_back( item );
			*/
		}
	}

	return items;
}

bool BurzaDialogs::RegisterAskToFinishAcceptingSellerChanges( void )
{
	if (! RegisterAnySelectedSeller())
	{
		return true;
	}
  
	return true;

	// ask only if user is not accepted yet
	SellerData seller = RegisterGetSelectedSeller();
	if (seller.status != imported) 
		return true;

	DisplayOkMessageBox( 
		ConvFuncs::FormatStdStr( _( "Predajca '%s - %s' zatiaľ nieje prijatý." ), 
			seller.shortcode.c_str(), 
			seller.name.c_str() ), 
		_("Nieje prijatý") );

	/*
	// first ask if user wants to finish
	if ( DisplayYesNoMessageBox( 
			_( "Chceš zmeniť predajcu na 'prijatého'?" ), 
			_( "Importovaný na prijatý." ) ) == false )
		return false;

	// then ask for name of person who finished it.
	if ( false == MessageEditBox( _( "Kto prebral tovar tohoto predajcu?" ), 
		personAcceptingItems ) )
		return false;

	try {
		Centrala::Storage().ModifySellerStart( seller.id );
		
		seller.status = receivedSelling;
		seller.note += _( " Prijal:" ) + personAcceptingItems;

		Centrala::Storage().ModifySellerEnd( seller.id, seller );
	}
	catch(exception& e)
	{
		ErrorHandling::HandleException(e);
	}
	
	RegisterSellerModified( seller );
	*/

	return true;
}

void BurzaDialogs::RegisterSellerStatusRadioChanged( void )
{
	SellerStatus newState;

	if ( m_pRegisterRadioSellerStatus1NotImported->get_active() ) {
		newState = imported;
	}
	else if ( m_pRegisterRadioSellerStatus2ImportedSelling->get_active() ) {
		newState = receivedSelling;
	}
	else if ( m_pRegisterRadioSellerStatus3ReturnedBack->get_active() ) {
		newState = handedOut;
	}
	
	if ( !RegisterAnySelectedSeller() )
		return;

	SellerData seller = RegisterGetSelectedSeller();
	if ( seller.status == newState ) {
		return;
	}

	// check if it is possible to switch state
	bool returnStateBack = false;
	bool addComment = false;

	switch ( seller.status ) {
	case imported:
		
		switch ( newState ) {
		case receivedSelling:
			{
				// changing from 'imported' to 'selling' can be done only
				// if all items are marked as 'received on stock'.
				int itemInvalidStatePcs = 0;

				ItemDataArray items = DemoLimitsGetSellerItems(seller.id);
				for( int i=0; i < items.size(); ++i ) {

					switch ( items[ i ].status ) {
					case on_stock:
					case returned:
					case given_as_gift:
					case lost:
						break;
					default:
						++itemInvalidStatePcs;
						break;
					}
				}

				if ( itemInvalidStatePcs > 0 ) {
					DisplayErrorMessageBox(
						ConvFuncs::FormatStdStr( _( "%i kus(ov) tovaru nieje prebratých."), itemInvalidStatePcs ),
						_( "Prosím opravte" ) );

					returnStateBack = true;
				}

				addComment = true;
			}
			break;
		case handedOut:
			returnStateBack = true;
			break;
		default:
			assert( false );
			return;
		}

		break;

	case receivedSelling:

		switch ( newState ) {
		case imported:
			break;

		case handedOut:
			{
				// changing from 'selling' to 'handed out' can be done only
				// if no items are left to sale.
				int itemInvalidStatePcs = 0;

				ItemDataArray items = DemoLimitsGetSellerItems(seller.id);
				for( int i=0; i < items.size(); ++i ) {

					switch ( items[ i ].status ) {
					case sold:
					case returned:
					case given_as_gift:
					case lost:
						break;
					default:
						++itemInvalidStatePcs;
						break;
					}
				}

				if ( itemInvalidStatePcs > 0 ) {
					DisplayErrorMessageBox(
						ConvFuncs::FormatStdStr( _( "%i kus(ov) tovaru nieje predaných/vrátených/stratených/darovaných."), itemInvalidStatePcs ),
						_( "Prosím opravte" ) );

					returnStateBack = true;
				}

				addComment = true;
			}
			break;

		default:
			assert( false );
			return;
		}

		break;

	case handedOut:

		switch ( newState ) {
		case imported:
			returnStateBack = true;
			break;
		case receivedSelling:
			break;
		default:
			assert( false );
			return;
		}
		
		break;

	default:
		assert( false );
		return;
	}
	
	if (returnStateBack) {
		RegisterDisplaySellerUpdateSellerStatus( seller.status );
		return;
	}

	// Change status of the seller
	

	// ask for name of person who finished it.
	if ( addComment )
	{
		do {
			if ( false == MessageEditBox( _( "Kto prebral tovar tohoto predajcu?" ), personAcceptingItems ) ) 
			{
				RegisterDisplaySellerUpdateSellerStatus( seller.status );
				return;
			}
		} while( personAcceptingItems.empty() );
	}

	try {
		Centrala::Storage().ModifySellerStart( seller.id );
		
		seller.status = newState;

		if ( addComment )
		{
			string activity = "?";
			switch ( newState ) {
				case receivedSelling:
					activity = _( "Prijal" );
					break;
				case handedOut:
					activity = _( "Vydal" );
					break;
			}
			
			seller.note += " " + activity + ":" + personAcceptingItems;
		}

		Centrala::Storage().ModifySellerEnd( seller.id, seller );
	}
	catch(exception& e)
	{
		ErrorHandling::HandleException(e);
	}
	
	RegisterSellerModified( seller );
}

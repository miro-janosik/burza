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
#include "bdhelper.h"
#include "bdfilehelper.h"
#include "LoadingMultipleFiles.h"

#include "generator.h"
#include "licence.h"

#include "stressTest.h"

#include <gtkmm.h>

using namespace Burza;
using namespace BurzaExcel;
using namespace std;

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

	// HandleException has already told the user. Callers run inside GTK signal
	// handlers, so an exception must not escape from here; return an empty
	// seller instead. Without any return the function falls off its end, which
	// is undefined behaviour - GCC emits ud2 and the program dies with SIGILL.
	return SellerData();
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

	// See CashoutGetSelectedSeller above. The default ItemData has
	// status == not_in_stock_yet and id == -1, so callers take their
	// "not on stock" path rather than selling an unknown item.
	return ItemData();
}

int BurzaDialogs::SetupDialogs( void )
{
	std::string		ui_fname;
	ui_fname = Centrala::Settings().m_data_dir_path;
	ui_fname += "burza.ui";
	
	std::string		error = "";

	// CSS replaces the GTK 2 modify_bg()/modify_base() styling; install it
	// before the windows exist so the first draw is already styled.
	BurzaDialogsHelper::InstallAppCss();

	// Load the GtkBuilder file and instantiate its widgets.
	// The builder is kept in a member, not a local: it owns the objects it
	// builds, so letting it go out of scope here would finalise every window
	// except Register (which Application::add_window keeps a reference to) and
	// turn all the widget pointers fetched below into dangling ones.
	m_refBuilder = Gtk::Builder::create();
	try
	{
		m_refBuilder->add_from_file(ui_fname);

		LoadDialogs(m_refBuilder);
	}
	catch(const Glib::FileError& ex)
	{
		error = "FileError: ";
		error += ex.what();
	}
	catch(const Glib::MarkupError& ex)
	{
		error = "MarkupError: ";
		error += ex.what();
	}
	catch(const Gtk::BuilderError& ex)
	{
		error = "BuilderError: ";
		error += ex.what();
	}

	if (!error.empty())
	{
		error += '\n';
		Log(InfoError, error.c_str());
		return 1;
	}

	return 0;
}

int BurzaDialogs::LoadDialogs( Glib::RefPtr<Gtk::Builder>& refBuilder )
{
	std::string error = "";

	try {

	m_pWndRegister = refBuilder->get_widget<Gtk::Window>( "Register" );
	if ( ! m_pWndRegister )
		error = "LoadDialogs: missing widget in UI file: 'Register'";
	
	m_pWndCashOut = refBuilder->get_widget<Gtk::Window>( "Pokladna" );
	if ( ! m_pWndCashOut )
		error = "LoadDialogs: missing widget in UI file: 'Pokladna'";
	
	m_pWndSeller = refBuilder->get_widget<Gtk::Window>( "Predajca" );
	if ( ! m_pWndSeller )
		error = "LoadDialogs: missing widget in UI file: 'Predajca'";
	
	m_pWndItem = refBuilder->get_widget<Gtk::Window>( "Tovar" );
	if ( ! m_pWndItem )
		error = "LoadDialogs: missing widget in UI file: 'Tovar'";
	
	//xmlWndRegisterPopupMenu = refBuilder->get_widget<Gtk::Window>( "menuRegisterOtherActions" );
	//if ( ! xmlWndRegisterPopupMenu )
	//	error = "LoadDialogs: missing widget in UI file: 'menuRegisterOtherActions'";
	
	loadFileProgressWindow.window = refBuilder->get_widget<Gtk::Window>( "LoadFilesProgress" );
	if ( ! loadFileProgressWindow.window )
		error = "LoadDialogs: missing widget in UI file: 'LoadFilesProgress'";
	
	if (( m_pWndRegister == NULL ) ||
		( m_pWndCashOut == NULL ) ||
		( m_pWndSeller == NULL ) ||
		( m_pWndItem == NULL ) ||
		( loadFileProgressWindow.window == NULL )) {
		error = "Dialog file is corrupt [windows].";
		throw std::runtime_error( error );
	}
	
	// GTK 4 removed window icons set from a file path; icons come from an icon
	// theme instead. data/icons holds burza.png in hicolor layout, so adding
	// that directory to the search path makes the name "burza" resolvable.
	// (The source burza.ico only has a 16x16 frame, so the icon is small - a
	// larger one should be drawn and dropped into data/icons some day.)
	{
		std::string iconPath = Centrala::Settings().m_data_dir_path + "icons";
		auto iconTheme = Gtk::IconTheme::get_for_display( Gdk::Display::get_default() );
		if ( iconTheme )
			iconTheme->add_search_path( iconPath );
	}

	m_pWndRegister->set_icon_name( "burza" );
	m_pWndCashOut->set_icon_name( "burza" );
	m_pWndSeller->set_icon_name( "burza" );
	m_pWndItem->set_icon_name( "burza" );

	// The .ui references logo_mc.gif relatively, which Cambalache resolves
	// against the project directory but GTK resolves against the process's
	// working directory. Set it from the data path so it loads wherever the
	// program is started from.
	{
		std::string logo = Centrala::Settings().m_data_dir_path + "logo_mc.gif";
		for ( const char* id : { "image1", "image2" } ) {
			Gtk::Image* pLogo = refBuilder->get_widget<Gtk::Image>( id );
			if ( pLogo )
				pLogo->set( logo );
		}
	}

	// It's not possible to delete widgets after app->run() has returned.
	// Delete the dialog with its child widgets before app->run() returns.
	m_pWndRegister->signal_hide().connect([] () { Log("window main close", InfoMinor); });

	Centrala::GetApp()->add_window(*m_pWndRegister);
	// m_pWndRegister->set_visible(true);
	m_pWndRegister->maximize();
	
	m_pComboSeller = refBuilder->get_widget<Gtk::DropDown>("combobox seller");
	m_pEntryRegistryLookupSeller = refBuilder->get_widget<Gtk::Entry>("entry_find_seller");
	m_pButtonImportSeller = refBuilder->get_widget<Gtk::Button>("button import seller");
	m_pButtonImportMultipleSellers = refBuilder->get_widget<Gtk::Button>("button import multi seller");
	m_pButtonAddSeller = refBuilder->get_widget<Gtk::Button>("button add seller");
	m_pButtonUpdateSeller = refBuilder->get_widget<Gtk::Button>("button update seller");
	m_pTreeViewItems = refBuilder->get_widget<Gtk::TreeView>("treeview items");
	m_pButtonAddItem = refBuilder->get_widget<Gtk::Button>("button add item");
	m_pButtonModifyItem = refBuilder->get_widget<Gtk::Button>("button modify item");
	m_pButtonRemoveItem = refBuilder->get_widget<Gtk::Button>("button remove item");
	m_pButtonAcceptUnaccept = refBuilder->get_widget<Gtk::Button>("button toggle accepted");
	// m_pButtonCalcFee = refBuilder->get_widget<Gtk::Button>("button calc fee");
	m_pButtonCalcRepayment = refBuilder->get_widget<Gtk::Button>("button calc repayment");
	m_pLabelSellerType = refBuilder->get_widget<Gtk::Label>("label seller type");
	m_pLabelSellerNote = refBuilder->get_widget<Gtk::Label>("label seller note");
	m_pButtonCashout = refBuilder->get_widget<Gtk::Button>("button cashout");
	// m_pRegisterButtonReviewOfSellers = refBuilder->get_widget<Gtk::Button>("button_review_of_sellers");
	// m_pButtonSettings = refBuilder->get_widget<Gtk::Button>("button_settings");
	m_pRegisterOtherActions = refBuilder->get_widget<Gtk::MenuButton>("button_other_actions");
	m_pRegisterRadioSellerStatus1NotImported = refBuilder->get_widget<Gtk::CheckButton>("radiobuttonSellerStateImported");
	m_pRegisterRadioSellerStatus2ImportedSelling = refBuilder->get_widget<Gtk::CheckButton>("radiobuttonSellerStateSelling");
	m_pRegisterRadioSellerStatus3ReturnedBack = refBuilder->get_widget<Gtk::CheckButton>("radiobuttonSellerStateReturned");

	m_pRegisterRadioSellerStatus2ImportedSelling->set_group(*m_pRegisterRadioSellerStatus1NotImported);
	m_pRegisterRadioSellerStatus3ReturnedBack->set_group(*m_pRegisterRadioSellerStatus1NotImported);
	
	// The menu's four entries are GMenu <item>s addressed by action name
	// ("burza.menuItemSettings" and friends, registered below), not widgets, so
	// there is nothing to look up for them.
	menuRegisterOtherActions = refBuilder->get_object<Gio::Menu>("menuRegisterOtherActions");

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
		throw std::runtime_error( error );
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
		( menuRegisterOtherActions.get() == NULL )
		) {
		error = "Dialog file is corrupt [register 2].";
		throw std::runtime_error( error );
	}

	if (( m_pRegisterRadioSellerStatus1NotImported == NULL ) ||
		( m_pRegisterRadioSellerStatus2ImportedSelling == NULL ) ||
		( m_pRegisterRadioSellerStatus3ReturnedBack == NULL )) {
		error = "Dialog file is corrupt [register 3].";
		throw std::runtime_error( error );
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
	// The "Iné akcie" button is a Gtk::MenuButton and shows menuRegisterOtherActions
	// by itself (menu-model is set in the .ui), so it needs no clicked handler.
	// Its items refer to actions "burza.<name>", which have to exist or GTK
	// renders every entry insensitive.
	registerActionGroup = Gio::SimpleActionGroup::create();
	registerActionGroup->add_action( "menuItemSettings",
		sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonSettings ) );
	registerActionGroup->add_action( "menuItemSaveSellerOverview",
		sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonSaveSellerOverview ) );
	registerActionGroup->add_action( "menuitemStats",
		sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonStats ) );
	registerActionGroup->add_action( "menuitemFixItemCats",
		sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonFixItemCats ) );
	m_pWndRegister->insert_action_group( "burza", registerActionGroup );
	m_pComboSeller->property_selected().signal_changed().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterComboChangedSeller ) );
	m_pEntryRegistryLookupSeller->signal_changed().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterEditLookupSellerChanged ) );

	m_pRegisterRadioSellerStatus1NotImported->signal_toggled().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterSellerStatusRadioChanged ) );
	m_pRegisterRadioSellerStatus2ImportedSelling->signal_toggled().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterSellerStatusRadioChanged ) );
	m_pRegisterRadioSellerStatus3ReturnedBack->signal_toggled().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterSellerStatusRadioChanged ) );

	m_RegisterItemsSelection = m_pTreeViewItems->get_selection();
	m_RegisterItemsSelection->set_mode( Gtk::SelectionMode::MULTIPLE );
	
	// setup item list
	m_RegisterItemsListModel = Gtk::ListStore::create( m_ItemsColumn );
	m_pTreeViewItems->set_model( m_RegisterItemsListModel );

	m_pTreeViewItems->append_column( "#", m_ItemsColumn.m_number );

	// m_pTreeViewItems->append_column( "stav", m_ItemsColumn.m_state );
	registerItemsStatusColumn = Gtk::manage( new Gtk::TreeView::Column( "stav" ) );
	registerItemsStatusColumn->pack_start( m_ItemsColumn.m_state_pic , false); //false = don’t expand.
	registerItemsStatusColumn->pack_start( m_ItemsColumn.m_state );
	m_pTreeViewItems->append_column( * registerItemsStatusColumn );

	// Checkbox column with toggling support
	auto toggle_renderer = Gtk::make_managed<Gtk::CellRendererToggle>();
	toggle_renderer->set_activatable(true);

	auto registerItemsStatusCheckboxColumn = Gtk::make_managed<Gtk::TreeViewColumn>("prebrate");
	registerItemsStatusCheckboxColumn->pack_start(*toggle_renderer);
	registerItemsStatusCheckboxColumn->add_attribute(toggle_renderer->property_active(),
						m_ItemsColumn.m_state_checkbox);

	m_pTreeViewItems->append_column( *registerItemsStatusCheckboxColumn );

	toggle_renderer->signal_toggled().connect(
		sigc::mem_fun(*this, &BurzaDialogs::on_column_checkbox_edited));

	/*
	auto registerItemsStatusCheckboxColumn = Gtk::manage( new Gtk::TreeView::Column( "prebrate" ) );

	registerItemsStatusCheckboxColumn->pack_start( m_ItemsColumn.m_state_checkbox );
	registerItemsStatusCheckboxColumn->get_cells().front()->signal_editing_started

	
	m_pTreeViewItems->connect_property_changed
	registerItemsStatusCheckboxColumn
	_connect_auto_store_editable_signal_handler<ColumnType>(this, pCellRender, model_column);

	m_pTreeViewItems->append_column( *registerItemsStatusCheckboxColumn );
	CellRenderer* pCellRender = registerItemsStatusCheckboxColumn->get_first_cell();
	*/

	// m_pTreeViewItems->append_column_editable( "prebrate", m_ItemsColumn.m_state_checkbox );

	m_pTreeViewItems->append_column( "nazov", m_ItemsColumn.m_name );
	m_pTreeViewItems->append_column( "cena", m_ItemsColumn.m_price );
	m_pTreeViewItems->append_column( "kategoria", m_ItemsColumn.m_category );
	
	// m_RegisterItemsListModel->set_sort_func(m_ItemsColumn.m_number, sigc::mem_fun(*this, &BurzaDialogs::SortRegisterItemsNumberFunc));
	// m_RegisterItemsListModel->set_sort_func(m_ItemsColumn.m_price, sigc::mem_fun(*this, &BurzaDialogs::SortRegisterItemsPriceFunc));
	
	m_RegisterItemsListModel->unset_default_sort_func();

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
		pColumn->set_sizing( Gtk::TreeViewColumn::Sizing::FIXED );
		//pColumn->set_sizing( Gtk::TREE_VIEW_COLUMN_GROW_ONLY );
	}
	
	// set column sorting and width
	Gtk::TreeView::Column * pColumn;
	
	pColumn = m_pTreeViewItems->get_column( 0 );
	if( pColumn ) {
		// pColumn->set_sort_column( m_ItemsColumn.m_number );
		pColumn->set_min_width( 50 );
	}
	pColumn = m_pTreeViewItems->get_column( 1 );
	if( pColumn ) {
		// pColumn->set_sort_column( m_ItemsColumn.m_state );
		pColumn->set_min_width( 100 );
	}

	pColumn = m_pTreeViewItems->get_column( 2 );
	if( pColumn ) {
		//pColumn->set_sort_column( m_ItemsColumn.m_state_checkbox );
		pColumn->set_min_width( 50 );
		// pColumn->set_clickable();
		// The toggled handler is connected where the column is created, above.
	}

	pColumn = m_pTreeViewItems->get_column( 3 );
	if( pColumn ) {
		// pColumn->set_sort_column( m_ItemsColumn.m_name );
		pColumn->set_min_width( 400 );
	}
	pColumn = m_pTreeViewItems->get_column( 4 );
	if( pColumn ) {
		// pColumn->set_sort_column( m_ItemsColumn.m_price );
		pColumn->set_min_width( 80 );
	}
	pColumn = m_pTreeViewItems->get_column( 5 );
	if( pColumn ) {
		// pColumn->set_sort_column( m_ItemsColumn.m_category );
		pColumn->set_min_width( 50 );
		pColumn->set_max_width( 200 );
	}

	// === --- === cash-out === --- ===
	
	if ( m_pWndCashOut == NULL ) {
		error = "Dialog file is corrupt [2].";
		throw std::runtime_error( error );
	}
	
	m_pWndCashOut->hide();

	m_pCashoutTreeViewList = refBuilder->get_widget<Gtk::TreeView>("treeview cashout items");
	m_pCashoutEntryLookupSeller = refBuilder->get_widget<Gtk::Entry>("entry lookup seller");
	m_pCashoutComboLookupSeller = refBuilder->get_widget<Gtk::DropDown>("combo lookup seller");
	m_pCashoutEntryLookupItem = refBuilder->get_widget<Gtk::Entry>("entry lookup item");
	m_pCashoutComboLookupItem = refBuilder->get_widget<Gtk::DropDown>("combo lookup item");
	m_pCashoutLabelItemPrice = refBuilder->get_widget<Gtk::Label>("label item price");
	m_pCashoutButtonAddItem = refBuilder->get_widget<Gtk::Button>("button cashout add item");
	m_pCashoutButtonRemoveItem = refBuilder->get_widget<Gtk::Button>("button cashout remove item");
	m_pCashoutLabelSum = refBuilder->get_widget<Gtk::Label>("label sum");
	m_pCashoutButtonFinish = refBuilder->get_widget<Gtk::Button>("label cashout finished");
	m_pButtonRegister = refBuilder->get_widget<Gtk::Button>("button register");
	m_pCashoutComboSearch = refBuilder->get_widget<Gtk::DropDown>("comboboxentry_search");
	// The old UI used a GtkComboBoxEntry (dropdown with a built-in entry) and
	// fetched the inner entry with get_child(). Gtk::DropDown has no entry, so
	// the .ui now carries a separate "entry_search_item" above the dropdown.
	// Until this was fetched, m_pCashoutComboSearchEntry stayed uninitialised
	// and any use of it was undefined behaviour.
	m_pCashoutComboSearchEntry = refBuilder->get_widget<Gtk::Entry>("entry_search_item");
	m_pCashoutSearchFoundText = refBuilder->get_widget<Gtk::Label>("search_found_text");
	
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
	   // ( m_pCashoutComboSearchEntry == NULL ) ||
	   ( m_pCashoutSearchFoundText == NULL )) {
		error = "Dialog file is corrupt [4].";
		throw std::runtime_error( error );
	}
	
	m_pButtonRegister->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::CashoutButtonSwitchToRegister ) );
	m_pCashoutButtonAddItem->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::CashoutButtonAddItem ) );
	m_pCashoutButtonRemoveItem->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::CashoutButtonRemoveItem ) );
	m_pCashoutButtonFinish->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::CashoutButtonFinished ) );
	
	m_CashoutItemsSelection = m_pCashoutTreeViewList->get_selection();

	// These were commented out with a mixture of wrong APIs: the selection mode
	// enum came from Gtk::CellRendererAccel, Gtk::TreeSelection has
	// signal_changed() rather than property_selected(), and the search field is
	// an entry (signal_changed()), not a dropdown. sigc++3 also needs *this.
	m_CashoutItemsSelection->set_mode( Gtk::SelectionMode::SINGLE );
	m_CashoutItemsSelection->signal_changed().connect(
		sigc::mem_fun( *this, &BurzaDialogs::CashoutItemsTreeSelectChanged ) );

	m_pCashoutComboSearch->property_selected().signal_changed().connect(
		sigc::mem_fun( *this, &BurzaDialogs::CashoutEditSearchChanged ) );

	if ( m_pCashoutComboSearchEntry )
		m_pCashoutComboSearchEntry->signal_changed().connect(
			sigc::mem_fun( *this, &BurzaDialogs::CashoutEditSearchEntryChanged ) );

	m_pCashoutEntryLookupSeller->signal_changed().connect( sigc::mem_fun( *this, &BurzaDialogs::CashoutEditLookupSellerChanged ) );
	m_pCashoutEntryLookupItem->signal_changed().connect( sigc::mem_fun( *this, &BurzaDialogs::CashoutEditLookupItemChanged ) );
	m_pCashoutComboLookupSeller->property_selected().signal_changed().connect( sigc::mem_fun( *this, &BurzaDialogs::CashoutComboLookupSellerChanged ) );
	m_pCashoutComboLookupItem->property_selected().signal_changed().connect( sigc::mem_fun( *this, &BurzaDialogs::CashoutComboLookupItemChanged ) );
	
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
		pColumn->set_sizing( Gtk::TreeViewColumn::Sizing::FIXED );
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
	m_pItemeditName = refBuilder->get_widget<Gtk::Entry>("itemName");
	m_pItemeditNumber = refBuilder->get_widget<Gtk::Entry>("itemNumber");
	m_pItemeditPrice = refBuilder->get_widget<Gtk::Entry>("itemPrice");
	// m_pItemeditCategory = refBuilder->get_widget<Gtk::Entry>("itemCategory");
	m_pItemeditCategoryCombo = refBuilder->get_widget<Gtk::DropDown>("itemCategoryCombo");
	m_pItemeditStatusCombo = refBuilder->get_widget<Gtk::DropDown>("itemStatus_combobox");
	m_pItemeditOk = refBuilder->get_widget<Gtk::Button>("itemButton_ok");
	m_pItemeditCancel = refBuilder->get_widget<Gtk::Button>("itemButton_cancel");
	
	if (( m_pItemeditName == NULL ) ||
		( m_pItemeditNumber == NULL ) ||
		( m_pItemeditPrice == NULL ) ||
		( m_pItemeditCategoryCombo == NULL ) ||
		( m_pItemeditStatusCombo == NULL ) ||
		( m_pItemeditOk == NULL ) ||
		( m_pItemeditCancel == NULL )) {
		error = "Dialog file is corrupt [5].";
		throw std::runtime_error( error );
	}
	
	m_pItemeditOk->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonItemEditOk ) );
	m_pItemeditCancel->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonItemEditCancel ) );
	m_pWndItem->signal_close_request().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonItemEditClose ), false );
	m_pItemeditStatusCombo->property_selected().signal_changed().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterComboItemEditStatusChanged ) );
	
	// == seller details edit dialog == 

	sellerEdit.editName = refBuilder->get_widget<Gtk::Entry>("sellerName");
	sellerEdit.editCode = refBuilder->get_widget<Gtk::Entry>("sellerCode");
	sellerEdit.editNote = refBuilder->get_widget<Gtk::Entry>("sellerNote");
	sellerEdit.comboCategory = refBuilder->get_widget<Gtk::DropDown>("sellerCategory");
	sellerEdit.buttonOk = refBuilder->get_widget<Gtk::Button>("sellerButton_ok");
	sellerEdit.buttonCancel = refBuilder->get_widget<Gtk::Button>("sellerButton_cancel");
	
	if (( sellerEdit.editName == NULL ) ||
		( sellerEdit.editCode == NULL ) ||
		( sellerEdit.comboCategory == NULL ) ||
		( sellerEdit.editNote == NULL ) ||
		( sellerEdit.buttonOk == NULL ) ||
		( sellerEdit.buttonCancel == NULL )) {
		error = "Dialog file is corrupt [6=Seller edit].";
		throw std::runtime_error( error );
	}
	
	sellerEdit.buttonOk->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonSellerEditOk ) );
	sellerEdit.buttonCancel->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonSellerEditCancel ) );
	m_pWndSeller->signal_close_request().connect( sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonSellerEditClose ), false );

	// === --- === --- === edit message box === --- === --- ===

	messageEditBoxWindowControls.dialog = refBuilder->get_widget<Gtk::Window>("MessageEditBox");
	messageEditBoxWindowControls.labelText = refBuilder->get_widget<Gtk::Label>("msg_label_text");
	messageEditBoxWindowControls.entryText = refBuilder->get_widget<Gtk::Entry>("entry_text");
	messageEditBoxWindowControls.buttonOk = refBuilder->get_widget<Gtk::Button>("button_ok");
	messageEditBoxWindowControls.buttonCancel = refBuilder->get_widget<Gtk::Button>("button_cancel");

	if (( messageEditBoxWindowControls.dialog == NULL ) ||
		( messageEditBoxWindowControls.labelText == NULL ) ||
		( messageEditBoxWindowControls.entryText == NULL ) ||
		( messageEditBoxWindowControls.buttonOk == NULL ) ||
		( messageEditBoxWindowControls.buttonCancel == NULL )) {
		error = "Dialog file is corrupt [MessageEditBox].";
		throw std::runtime_error( error );
	}
	
	EditMessageBox& msgBox = EditMessageBox::Instance();
	messageEditBoxWindowControls.buttonOk->signal_clicked().connect( sigc::mem_fun( msgBox, &EditMessageBox::EditMessageBoxClickedOk ) );
	messageEditBoxWindowControls.buttonCancel->signal_clicked().connect( sigc::mem_fun( msgBox, &EditMessageBox::EditMessageBoxClickedCancel ) );

	EditMessageBox::Initialize( messageEditBoxWindowControls );

	// === --- === --- === Nastavenia === --- === --- ===

	settingsWindow.dialog = refBuilder->get_widget<Gtk::Window>("Nastavenia");
	settingsWindow.labelCurrentLicence = refBuilder->get_widget<Gtk::Label>("label_current_licence");
	settingsWindow.buttonRequestLicence = refBuilder->get_widget<Gtk::Button>("button_request_licence");
	settingsWindow.buttonInstallLicence = refBuilder->get_widget<Gtk::Button>("button_install_licence");
	settingsWindow.editBurzaName = refBuilder->get_widget<Gtk::Entry>("entry_name_of_burza");
	settingsWindow.buttonOk = refBuilder->get_widget<Gtk::Button>("settingsButton_ok");
	settingsWindow.comboFontSize = refBuilder->get_widget<Gtk::DropDown>("combo_font_size");
	settingsWindow.radio1file = refBuilder->get_widget<Gtk::CheckButton>("radio_1file");
	settingsWindow.radio2database = refBuilder->get_widget<Gtk::CheckButton>("radio_2db");
	settingsWindow.editDbAddress = refBuilder->get_widget<Gtk::Entry>("entry_dbaddress");
	settingsWindow.editDbUser = refBuilder->get_widget<Gtk::Entry>("entry_dbuser");
	settingsWindow.editDbPassword = refBuilder->get_widget<Gtk::Entry>("entry_dbpassword");
	settingsWindow.editDbPort = refBuilder->get_widget<Gtk::Entry>("entry_dbport");

	if (( settingsWindow.dialog == NULL ) ||
		( settingsWindow.labelCurrentLicence == NULL ) ||
		( settingsWindow.buttonRequestLicence == NULL ) ||
		( settingsWindow.editBurzaName == NULL ) ||
		( settingsWindow.buttonOk == NULL ) ||
		( settingsWindow.buttonInstallLicence == NULL ) ||
		( settingsWindow.comboFontSize == NULL )) {
		error = "Dialog file is corrupt [Nastavenia].";
		throw std::runtime_error( error );
	}

	if (( settingsWindow.radio1file == NULL ) ||
		( settingsWindow.radio2database == NULL ) ||
		( settingsWindow.editDbAddress == NULL ) ||
		( settingsWindow.editDbUser == NULL ) ||
		( settingsWindow.editDbPassword == NULL ) ||
		( settingsWindow.editDbPort == NULL )) {
		error = "Dialog file is corrupt [Nastavenia] part 2.";
		throw std::runtime_error( error );
	}

	settingsWindow.radio2database->set_group( *settingsWindow.radio1file );

	settingsWindow.buttonRequestLicence->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::SettingsClickedButtonRequestLicence ) );
	settingsWindow.buttonInstallLicence->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::SettingsClickedButtonInstallLicence ) );
	settingsWindow.buttonOk->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::SettingsClickedButtonOk ) );

	// ============== Load file progress window =================
	
	loadFileProgressWindow.window = refBuilder->get_widget<Gtk::Window>("LoadFilesProgress");

	loadFileProgressWindow.editLoadingText = refBuilder->get_widget<Gtk::TextView>("loading_text");
	loadFileProgressWindow.buttonOk = refBuilder->get_widget<Gtk::Button>("buttonFinished");
	loadFileProgressWindow.buttonInterrupt = refBuilder->get_widget<Gtk::Button>("buttonInterrupt");

	if (( loadFileProgressWindow.editLoadingText == NULL ) ||
		( loadFileProgressWindow.buttonOk == NULL ) ||
		( loadFileProgressWindow.buttonInterrupt == NULL )) {
		error = "Dialog file is corrupt [LoadFilesProgress].";
		throw std::runtime_error( error );
	}	
	
	loadFileProgressWindow.ConnectButtons();
	
	// === --- === --- === throw - catch  === --- === --- ===
	
	}
	catch(exception& e) {
		error = "Error loading widgets:";
		error += e.what();

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
	m_pComboSeller->set_selected( idx );
	// i = m_pComboSeller->get_selected();

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
	std::vector<Glib::ustring> sellers;

	std::string		str;
	for( int i=0; i<SellersOrdered.size(); ++i ) {
		str = SellersOrdered[ i ].mark;
		str += SellersOrdered[ i ].shortcode;
		str += " - ";
		str += SellersOrdered[ i ].name;

		sellers.push_back( str );
	}

	auto sellerList = Gtk::StringList::create(sellers);
	m_pComboSeller->set_model(sellerList);
	m_pComboSeller->set_selected(0);

	m_pCashoutComboLookupSeller->set_model(sellerList);
	m_pCashoutComboLookupSeller->set_selected(0);
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
		// do not display if it is not exact match. Displaying takes time. 
		// See issue #40 RegisterDisplaySeller( SellersOrdered[ 0 ] );
	}
}

void BurzaDialogs::RegisterButtonImportSeller( void )
{
	if (DemoLimitsCanAddSeller() == false)
		return;

	auto dialog = Gtk::FileDialog::create();
	dialog->set_title("Select importer file");
	/*
	dialog.set_transient_for( *m_pWndRegister );
	//Add response buttons the the dialog:
	dialog.add_button( "cancel", Gtk::ResponseType::CANCEL);
	dialog.add_button( "ok", Gtk::ResponseType::OK);
	dialog.set_filename( Centrala::Settings().m_data_dir_path );
	*/

	std::string path = "";
	if ( Centrala::Settings().directoryImport.empty() ) {
		path = Centrala::Settings().m_data_dir_path;
	} else {
		path = Centrala::Settings().directoryImport;
	}

	auto ghandle = Gio::File::create_for_path(path);
	dialog->set_initial_folder( ghandle );

	// Add filters, so that only certain file types can be selected:
	auto filters = Gio::ListStore<Gtk::FileFilter>::create();
	auto xls_filter = Gtk::FileFilter::create();
	xls_filter->set_name("Sheet files.");
	xls_filter->add_pattern( "*.ods" );
	xls_filter->add_pattern( "*.xls" );
	xls_filter->add_pattern( "*.xlsx" );
	xls_filter->add_pattern( "*.txt" );
	filters->append(xls_filter);
	dialog->set_filters(filters);

	dialog->open( *m_pWndRegister, sigc::bind(
		sigc::mem_fun( *this, &BurzaDialogs::RegisterButtonImportSeller_ContinueWithFileSelected ),
		dialog ) );
}

void BurzaDialogs::RegisterButtonImportSeller_ContinueWithFileSelected(
	const Glib::RefPtr<Gio::AsyncResult>& result, const Glib::RefPtr<Gtk::FileDialog>& dialog )
{
	std::string foldername;
	std::string filename;
  	try
  	{
		auto gfile = dialog->open_finish(result);
	    foldername = gfile->get_basename();
	    filename = gfile->get_path();
	}
	catch (const Gtk::DialogError& err)
	{
		Log("No file selected", InfoError);
		return;
	}
	catch (const Glib::Error& err)
	{
		std::string e = "Error selecting file. ";
		e += err.what();
		Log(e.c_str(), InfoError);
		return;
	}
	
	// set back the import directory
	Centrala::Settings().directoryImport = foldername;
	Centrala::Settings().directoryImport += "/";

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
	if (DemoLimitsCanAddSeller() == false)
		return;

	if (!loadFileProgressWindow.AskForFolder( *m_pWndRegister ))
	{
		return;
	}
	
	loadFileProgressWindow.RunImport( *m_pWndRegister );
	ReloadSellers();
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
	
	auto emptyList = Gtk::StringList::create();
	m_pCashoutComboLookupSeller->set_model(emptyList);
	m_pCashoutComboLookupItem->set_model(emptyList);
	m_pCashoutEntryLookupSeller->set_text( "" );
	m_pCashoutEntryLookupItem->set_text( "" );
	m_pCashoutComboSearch->set_model( Gtk::StringList::create( {} ) );
	if ( m_pCashoutComboSearchEntry )
		m_pCashoutComboSearchEntry->set_text( "" );

	ReloadSellers();

	// GTK 4 removed Gtk::Container::set_focus_chain - tab order now follows the
	// order widgets appear in the .ui, so the list that used to be built here
	// has been dropped. Reorder the widgets in Cambalache to change tab order.
	m_pCashoutEntryLookupSeller->grab_focus();

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
	int i = m_pComboSeller->get_selected();
	
	if ( i == GTK_INVALID_LIST_POSITION )
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
	std::vector<Gtk::TreeModel::Path>	selected_items = selector->get_selected_rows();
	if ( selected_items.empty() )
		return;
	
	for( auto it=selected_items.begin(); it != selected_items.end(); ++it ) {
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
	Gtk::TreeModel::iterator rowIterator;
	
	rowIterator = m_CashoutItemsListModel->append();
	row = *(rowIterator);
	
	row[ m_ItemsColumn.m_number ] = ConvFuncsInstance->FormatCstr( "%s - %i", seller.shortcode.c_str(), item.number );
	row[ m_ItemsColumn.m_name ] = item.name;
	row[ m_ItemsColumn.m_price ] = ConvFuncsInstance->FormatCstr( "%0.2f", item.price );
	row[ m_ItemsColumn.m_category ] = ConvFuncsInstance->FormatCstr( "%c", itemcat.letter );
	row[ m_ItemsColumn.m_state ] = ConvFuncsInstance->FormatCstr( "%s", 
		DataAccessInterface::ItemStatus2String( item.status ) );
	row[ m_ItemsColumn.m_seller_id ] = item.seller_id;
	row[ m_ItemsColumn.m_item_id ] = item.id;
	
	Gtk::TreeModel::Path rowPath( rowIterator );
	m_pCashoutTreeViewList->scroll_to_row( rowPath );
	
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
	
	// Store sum before checkout is finished.
	double previousCashoutSum = m_cashout_sum;
	
	ItemDataArray items = Centrala::Storage().GetItems( m_shoppingCart );
	mp_register->FinishCheckout( items );
	
	m_CashoutItemsListModel->clear();
	m_shoppingCart.clear();
	CheckoutCartChanged();

	// add a text to the sum displaying sum of previous cashout
	std::string	str;
	str = m_pCashoutLabelSum->get_text();
	str += ConvFuncsInstance->FormatCstr( _(" (Predchádzajúci nákup: %0.2f" ), previousCashoutSum );
	str += " ";
	str += Centrala::Settings().currencySign;
	str += ")";
	m_pCashoutLabelSum->set_text( str );
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
	
	// remove items. Indexes are in descending order (sorted and reversed
	// above), so erasing one does not shift the ones still to be removed.
	for(int i=0; i<itemIndexes.size(); ++i ) {

		unsigned int index = itemIndexes[ i ];

		// Addressing the row by path is the gtkmm 4 way; children()[index]
		// is gone, which is why this loop had been emptied out.
		Gtk::TreeModel::Path path;
		path.push_back( index );
		Gtk::TreeModel::iterator it = m_CashoutItemsListModel->get_iter( path );
		if ( it )
			m_CashoutItemsListModel->erase( it );

		if ( index < m_shoppingCart.size() )
			m_shoppingCart.erase( m_shoppingCart.begin() + index );
		else
			LogS( InfoWarn, "CashoutButtonRemoveItem: index %u outside cart of %zu\n",
				index, m_shoppingCart.size() );
	}
	
	CheckoutCartChanged();
}

void BurzaDialogs::CashoutComboLookupSellerChanged( void )
{
	int i = m_pCashoutComboLookupSeller->get_selected();
	
	if ( i == GTK_INVALID_LIST_POSITION ) {
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
	
	int i = m_pCashoutComboLookupItem->get_selected();
	if ( i == GTK_INVALID_LIST_POSITION )
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
	auto emptyList = Gtk::StringList::create();
	m_pCashoutComboLookupItem->set_model(emptyList);
	m_CashoutDisplayedItems.clear();
	
	if ( ! CashoutAnySelectedSeller() )
		return;
	
	std::string		str;
	
	ItemDataArray items = DemoLimitsGetSellerItems( CashoutGetSelectedSellerId() );

	// XXX ERROR: GTKMM can't handle too long items in combo or some chars probably?
	// so have to cut down.
	const int maxStrLen = 70;
	
	std::vector<Glib::ustring> strings;
		
	for( int i = 0; i < items.size(); ++i ) {
		str = ConvFuncsInstance->FormatCstr( "%i - %s", 
			items[ i ].number,
			items[ i ].name.c_str() );

		if (str.size() > maxStrLen )
			str.erase( str.begin() + maxStrLen, str.end() );

		strings.push_back( str );
		m_CashoutDisplayedItems.push_back( items[i] );
	}

	auto stringList = Gtk::StringList::create(strings);
	m_pCashoutComboLookupItem->set_model(stringList);
	m_pCashoutComboLookupItem->set_selected(0);
}

void BurzaDialogs::DisplayErrorMessageBox( string msg1, string msg2 )
{
	// This used to build a Gtk::MessageDialog and then throw instead of
	// showing it ("//X Should display it instead"), left over from the gtkmm 4
	// port. Throwing from here was fatal: callers invoke it from inside a catch
	// handler, so the new exception escaped the handler and then the GTK signal
	// handler, and glibmm aborts the program on that.
	//
	// Gtk::AlertDialog replaces Gtk::MessageDialog in GTK 4 and is shown
	// asynchronously - there is no run() to block on any more, so this returns
	// immediately and the dialog closes itself when the user dismisses it.
	try {
		auto dialog = Gtk::AlertDialog::create();
		dialog->set_message( msg2.empty() ? std::string( "error" ) : msg2 );
		dialog->set_detail( msg1 );
		dialog->set_modal( false );

		// Parent it on the main window when there is one; during start-up
		// failures there may not be.
		Gtk::Window* parent = NULL;
		if ( Centrala::Instance() && Centrala::Instance()->mp_dialogs )
			parent = Centrala::Dialogs().m_pWndRegister;

		if ( parent )
			dialog->show( *parent );
		else
			dialog->show();
	}
	catch( ... ) {
		// Never let the error reporter become the error.
	}

	LogS( InfoError, "MessageBox: '%s' - '%s'\n", msg2.c_str(), msg1.c_str() );
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
	// This used to be stubbed out and simply "return true", which silently
	// confirmed every question it was asked - deleting items, selling goods
	// not marked as on stock, and finishing a purchase - without the user ever
	// seeing a dialog.
	//
	// GTK 4 removed Gtk::Dialog::run(), so there is no blocking dialog any
	// more: Gtk::AlertDialog::choose() is asynchronous. To keep this function's
	// synchronous bool contract (five call sites depend on it) a nested
	// Glib::MainLoop is run until the answer arrives. The dialog is modal, so
	// the window underneath cannot be operated while it waits.
	//
	// Anything other than an explicit "Áno" - closing the dialog, Escape, an
	// error - counts as "no", so a failure can never silently approve a
	// destructive action.

	const int buttonNo = 0;
	const int buttonYes = 1;
	int answer = buttonNo;

	try {
		auto dialog = Gtk::AlertDialog::create();

		// AlertDialog shows 'message' prominently and 'detail' below it.
		if ( title.empty() ) {
			dialog->set_message( message );
		} else {
			dialog->set_message( title );
			dialog->set_detail( message );
		}

		dialog->set_buttons( { _("Nie"), _("Áno") } );
		dialog->set_default_button( buttonYes );
		dialog->set_cancel_button( buttonNo );
		dialog->set_modal( true );

		auto loop = Glib::MainLoop::create( false );
		bool finished = false;

		auto onAnswer = [ &dialog, &answer, &loop, &finished ]
			( Glib::RefPtr<Gio::AsyncResult>& result )
		{
			try {
				answer = dialog->choose_finish( result );
			}
			catch( const Glib::Error& ) {
				// dismissed without choosing
				answer = buttonNo;
			}
			finished = true;
			if ( loop->is_running() )
				loop->quit();
		};

		if ( m_pWndRegister )
			dialog->choose( *m_pWndRegister, onAnswer );
		else
			dialog->choose( onAnswer );

		// choose() may already have completed (it can answer synchronously if
		// it fails immediately); only block when it has not.
		if ( ! finished )
			loop->run();
	}
	catch( const std::exception& e ) {
		LogS( InfoError, "DisplayYesNoMessageBox failed: %s\n", e.what() );
		return false;
	}

	return ( answer == buttonYes );
}

void BurzaDialogs::DisplayOkMessageBox( string message, string title )
{
	// Shown asynchronously - there is no run() in GTK 4. Nothing here needs an
	// answer, so unlike DisplayYesNoMessageBox this does not wait for one.
	try {
		auto dialog = Gtk::AlertDialog::create();
		dialog->set_buttons( { _("OK") } );

		// Button indices are 0-based: the single OK button is 0. The old code
		// asked for button 2 of a one-button dialog.
		dialog->set_default_button( 0 );
		dialog->set_cancel_button( 0 );

		if ( title.empty() ) {
			dialog->set_message( message );
		} else {
			dialog->set_message( title );
			dialog->set_detail( message );
		}

		dialog->set_modal( false );

		if ( m_pWndRegister )
			dialog->show( *m_pWndRegister );
		else
			dialog->show();
	}
	catch( const std::exception& e ) {
		LogS( InfoError, "DisplayOkMessageBox failed: %s\n", e.what() );
	}
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
	
	std::vector<Glib::ustring> states_strings;
	for( int i = not_in_stock_yet; i < ItemStatusCount; ++i ) {
		states_strings.push_back( DataAccessInterface::ItemStatus2String( (ItemStatus)i ) );
	}
	
	auto listStates = Gtk::StringList::create(states_strings);
	m_pItemeditStatusCombo->set_model(listStates);
	m_pItemeditStatusCombo->set_selected( item.status );

	// fill 'category' combo
	ItemCategoryArray categories = Centrala::Storage().GetItemsCategories();

	int selectedCategoryIndex = -1;

	std::vector<Glib::ustring> category_strings;
	for( int i = 0; i < categories.size(); ++i ) {
		category_strings.push_back( categories[i].name );
		
		if ( categories[i].id == item.category )
			selectedCategoryIndex = i;
	}

	auto list = Gtk::StringList::create(category_strings);
	m_pItemeditCategoryCombo->set_model(list);

	// select a default category, if none selected
	if ( selectedCategoryIndex == -1 ) {
		selectedCategoryIndex = 0;
	}
	
	if ( selectedCategoryIndex != -1 )
		m_pItemeditCategoryCombo->set_selected( selectedCategoryIndex );
	
	m_EditedItem = item;
	try {
		m_EditedSeller = Centrala::Storage().GetSeller( item.seller_id );
	}
	catch(exception& e)
	{
		ErrorHandling::HandleException(e);
	}
	
	m_pWndItem->set_transient_for( *m_pWndRegister );
	m_pWndItem->show();
	HighlightEditItemOff();
}

void BurzaDialogs::HighlightWidgetRed( Gtk::Widget& widget )
{
	// The rule for this class is in BurzaDialogsHelper::InstallAppCss().
	widget.add_css_class( BurzaDialogsHelper::cssClassInvalid );
}

void BurzaDialogs::HighlightWidgetOff( Gtk::Widget& widget )
{
	widget.remove_css_class( BurzaDialogsHelper::cssClassInvalid );
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

	int i = m_pItemeditStatusCombo->get_selected();
	if ( i != GTK_INVALID_LIST_POSITION ) {
		m_EditedItem.status = (ItemStatus) i;
	}

	// item category
	int itemCategoryIndex = m_pItemeditCategoryCombo->get_selected();
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

bool BurzaDialogs::RegisterButtonItemEditClose( void )
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

	// Gtk::DropDown replaced Gtk::ComboBoxText, so there is no clear() or
	// append_text() any more - it is driven by a model. Same pattern as
	// FillSellerCombos().
	sellerEdit.categories = categories;
	std::vector<Glib::ustring> categoryNames;
	int index = 0;
	for( int i=0; i < categories.size(); ++i ) {
		categoryNames.push_back( categories[ i ].name );

		if ( categories[ i ].id == seller.category )
			index = i;
	}

	auto categoryList = Gtk::StringList::create( categoryNames );
	sellerEdit.comboCategory->set_model( categoryList );
	sellerEdit.comboCategory->set_selected( index );

	HighlightEditSellerOff();

	m_EditedSeller = seller;
	m_pWndSeller->set_transient_for( *m_pWndRegister );
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
	
	// get category. Gtk::DropDown::get_selected() returns
	// GTK_INVALID_LIST_POSITION (not -1) when nothing is selected, which as a
	// signed int is -1 and would index the array out of range - the assert
	// below only caught it in debug builds.
	guint categoryIndex = sellerEdit.comboCategory->get_selected();
	if (( categoryIndex == GTK_INVALID_LIST_POSITION ) ||
		( categoryIndex >= sellerEdit.categories.size() )) {
		HighlightWidgetRed( *sellerEdit.comboCategory );
		DisplayErrorMessageBox( _("Vyberte kategóriu predajcu."), _("Chýba kategória") );
		return;
	}
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

bool BurzaDialogs::RegisterButtonSellerEditClose( void )
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
	
	auto emptyList = Gtk::StringList::create();
	m_pCashoutComboLookupItem->set_model(emptyList);
	m_CashoutDisplayedItems.clear();

	if ( idx == -1 ) {
		m_pCashoutComboLookupSeller->set_selected( -1 );
		m_CashoutSelectedSellerId = SELLER_ID_NONE;
	} else {
		m_pCashoutComboLookupSeller->set_selected( idx );
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
	
	m_pCashoutComboLookupItem->set_selected( idx );
	// and let combo to do the work.
	CashoutComboLookupItemChanged();
}

void BurzaDialogs::CashoutEditSearchChanged( void )
{
	// property_active() was the ComboBox API; a DropDown reports its row with
	// get_selected(), which yields GTK_INVALID_LIST_POSITION when unset.
	guint selected = m_pCashoutComboSearch->get_selected();

	if ( selected == GTK_INVALID_LIST_POSITION ) {
		return;
	}

	if ( selected >= cashoutComboSearchItemDatas.size() ) {
		LogS( InfoWarn, "CashoutEditSearchChanged: row %u outside %zu results\n",
			selected, cashoutComboSearchItemDatas.size() );
		return;
	}

	int selectedItemIdx = (int) selected;

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
	std::vector<Glib::ustring> searchResultNames;

	// Reset the result list. A DropDown is cleared by giving it an empty
	// model, the same way RegisterButtonSwitchToCashout does.
	m_pCashoutComboSearch->set_model( Gtk::StringList::create( {} ) );
	cashoutComboSearchItemDatas.clear();
	m_pCashoutSearchFoundText->set_text( "" );

	if ( ! m_pCashoutComboSearchEntry )
		return;

	string text = m_pCashoutComboSearchEntry->get_text();
	const int itemsLimit = 60;
	
	foundItems = search.LookForItemsContainingWords( text, SELLER_ID_NONE, itemsLimit + 1 );
	
	string itemsCountText = _("Nájdených: ");
	
	itemsCountText += ConvFuncs::FormatStdStr("%i", foundItems.size() > itemsLimit ? itemsLimit : foundItems.size() );
	if ( foundItems.size() > itemsLimit ) {
		itemsCountText += " +";
	}
	
	m_pCashoutSearchFoundText->set_text(itemsCountText);
	
	for( int i=0; i<foundItems.size(); ++i ) {

		sellerOfItem = search.GetSeller( foundItems[ i ].seller_id );

		str = ConvFuncs::FormatStdStr("%s - %i - %s - %0.2f - %s",
			sellerOfItem.shortcode.c_str(), 
			foundItems[ i ].number, 
			foundItems[ i ].name.c_str(),
			foundItems[ i ].price,
			DataAccessInterface::ItemStatus2String( foundItems[ i ].status ));

		searchResultNames.push_back( str );
		cashoutComboSearchItemDatas.push_back( foundItems[ i ] );
	}

	// Fill the DropDown in one go - it is model driven, so there is no
	// append_text() to call per row.
	m_pCashoutComboSearch->set_model( Gtk::StringList::create( searchResultNames ) );
}

void BurzaDialogs::RegisterButtonStats( void )
{
	string offeredName = BurzaDialogsFileHelper::FormatDateTimeFileName( _("Statistika"), "pdf" );
	string name = BurzaDialogsFileHelper::AskToSaveDocumentFile( offeredName );
	if ( name.empty() )
		return;

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
}

void BurzaDialogs::RegisterButtonSaveSellerOverview( void )
{
	RegisterButtonReviewOfSellers();
}

void BurzaDialogs::RegisterButtonFixItemCats( void )
{
	// NOTE: this calls RegisterButtonReviewOfSellers(), exactly as
	// RegisterButtonSaveSellerOverview() does. That looks like a copy-paste
	// left over from before the port - "Oprav kategórie tovaru" presumably
	// ought to fix item categories, not produce the seller overview. Behaviour
	// kept as it was; worth deciding separately.
	RegisterButtonReviewOfSellers();
}

// ======================================================
// ======================================================

void BurzaDialogs::RegisterButtonSettings( void )
{
	ProgramSettings* settings = & Centrala::Settings();

	// license
	string licenseText = Licence::Instance().IsDemo() ? _( "DEMO" ) : 
		Licence::Instance().GetLicenseUser();
	settingsWindow.labelCurrentLicence->set_text( licenseText );

	// setup 'font' combo. Gtk::DropDown has no clear()/append_text() - it takes
	// a model - so this is built as a Gtk::StringList, like FillSellerCombos().
	int selectedFontIndex = 0;
	std::vector<Glib::ustring> fontSizeNames;

	for( int i = 0; i < ProgramSettings::predefined.listOfFontSizes.size(); ++i )
	{
		fontSizeNames.push_back( ProgramSettings::predefined.listOfFontSizes[ i ].text );
		if ( settings->fontSize == ProgramSettings::predefined.listOfFontSizes[ i ].size )
			selectedFontIndex = i;
	}

	settingsWindow.comboFontSize->set_model( Gtk::StringList::create( fontSizeNames ) );
	settingsWindow.comboFontSize->set_selected( selectedFontIndex );

	// name of "Burza"
	settingsWindow.editBurzaName->set_text( settings->m_name_of_the_sale );

	// database / file settings
	settingsWindow.radio1file->set_active( settings->databaseInLocalFiles );
	settingsWindow.radio2database->set_active( ! settings->databaseInLocalFiles );

	settingsWindow.editDbAddress->set_text( settings->dbAddress );
	settingsWindow.editDbUser->set_text( settings->dbUser );
	settingsWindow.editDbPort->set_text( ConvFuncs::FormatStdStr( "%i", settings->dbPort ) );
	
	auto passwordPartial = ProgramSettings::HidePartOfPassword( settings->dbPassword );
	settingsWindow.editDbPassword->set_text( passwordPartial );
	
	// Gtk::Dialog::run() no longer exists in GTK 4 - show the window instead.
	// SettingsClickedButtonOk() hides it again.
	settingsWindow.dialog->set_transient_for( *m_pWndRegister );
	settingsWindow.dialog->set_modal( true );
	settingsWindow.dialog->set_visible( true );
}

void BurzaDialogs::SettingsClickedButtonRequestLicence( void )
{
	std::string vlastnik = "[vlastník]";
	MessageEditBox( "Prosím zadajte názov vlastníka licencie", vlastnik );
}

void BurzaDialogs::SettingsClickedButtonInstallLicence( void )
{

}

void BurzaDialogs::SettingsClickedButtonOk( void )
{
	settingsWindow.dialog->set_visible( false );

	ProgramSettings	*settings = & Centrala::Settings();

	// name of "Burza"
	settings->m_name_of_the_sale = settingsWindow.editBurzaName->get_text();

	// font size
	int fontSizeSelectionIndex = settingsWindow.comboFontSize->get_selected();
	int newFontSize = -1;
	if ( fontSizeSelectionIndex != GTK_INVALID_LIST_POSITION ) {
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
	settings->dbPort = atoi( settingsWindow.editDbPort->get_text().c_str() );
	
	// if user did not modify the password then it still may be the 'partial' password text
	// that was filled into the textbox. In that case do not store the password, because
	// it was not modified.
	string possiblyPartialPassword = settingsWindow.editDbPassword->get_text();
	auto passwordPartial = ProgramSettings::HidePartOfPassword( settings->dbPassword );
	if (possiblyPartialPassword != passwordPartial)
	{
		settings->dbPassword = possiblyPartialPassword;
	}
	
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
		registerColumnStateImages[ i ] = Gdk::Pixbuf::create(
			Gdk::Colorspace::RGB, FALSE, 8, sizex, sizey);
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
	if (( row < 0 ) || ( (size_t) row >= items.size() )) {
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
		    if (SellersOrdered[i].status > 0 )
			recipees.PrintSellerReturnSheet( pdf, SellersOrdered[ i ].id );
		}
		
		pdf.ClosePDF();

		storingFinished = true;
		
	}
	catch( const std::exception & e ) {
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
			{
				// changing from 'selling' to 'imported' can be done only
				// if no items are sold yet
				int itemInvalidStatePcs = 0;
				
				ItemDataArray items = DemoLimitsGetSellerItems(seller.id);
				for( int i=0; i < items.size(); ++i ) {

					switch ( items[ i ].status ) {
					case not_in_stock_yet:
					case on_stock:
						break;
					default:
						++itemInvalidStatePcs;
						break;
					}
				}
				
				if ( itemInvalidStatePcs > 0 ) {
					DisplayErrorMessageBox(
						ConvFuncs::FormatStdStr( _( "%i kus(ov) tovaru už nieje prijatých."), itemInvalidStatePcs ),
						_( "Prosím opravte" ) );
					
					returnStateBack = true;
				}
			}
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
			{
				// changing from 'selling' to 'handed out' can be done only
				// if no items are left to sale.
				int itemInvalidStatePcs = 0;

				ItemDataArray items = DemoLimitsGetSellerItems(seller.id);
				for( int i=0; i < items.size(); ++i ) {

					switch ( items[ i ].status ) {
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
						ConvFuncs::FormatStdStr( _( "%i kus(ov) tovaru nieje vrátených/stratených/darovaných."), itemInvalidStatePcs ),
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
/*
int BurzaDialogs::SortRegisterItemsNumberFunc(const Gtk::TreeIter& a, const Gtk::TreeIter& b)
{
	string vala = (*a)[ m_ItemsColumn.m_number ];
	string valb = (*b)[ m_ItemsColumn.m_number ];
	
	double numa, numb;
	if (!ConvFuncs::StringToDouble(vala, numa) || !ConvFuncs::StringToDouble(valb, numb))
	{
		return 0;
	}
	
	return numa - numb;
}

int BurzaDialogs::SortRegisterItemsPriceFunc(const Gtk::TreeIter& a, const Gtk::TreeIter& b)
{
	string vala = (*a)[ m_ItemsColumn.m_price ];
	string valb = (*b)[ m_ItemsColumn.m_price ];
	
	double numa, numb;
	if (!ConvFuncs::StringToDouble(vala, numa) || !ConvFuncs::StringToDouble(valb, numb))
	{
		return 0;
	}
	
	double dif = numa - numb;
	return (dif == 0 ? 0 : (dif > 0 ? 1 : -1));
}
*/
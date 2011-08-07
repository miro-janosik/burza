// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#include <assert.h>
#include <exception>
#include "bdialogs.h"
#include "convfncs.h"

#include "excelwrap.h"
#include "pdfwrap.h"
#include "recipees.h"
#include "datatextfile.h"

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
	
	m_SelectedSellerId = SELLER_ID_NONE;
	
	m_pCashoutVybranyPredajca = NULL;
	m_CashoutVybranyPredajcaIdx = -1;
	m_pCashoutVybranyTovar = NULL;
	m_CashoutVybranyTovarIdx = -1;
	
	m_cashout_sum = 0.0;
	
	m_sellerEdited = NULL;
	
	ConvFuncs::SingletonInstanceCreate();
}

BurzaDialogs::~BurzaDialogs()
{
	ConvFuncs::SingletonInstanceDelete();
}

bool BurzaDialogs::RegisterAnySelectedSeller( void )
{ 
	return m_SelectedSellerId != SELLER_ID_NONE; 
}

SellerData BurzaDialogs::RegisterGetSelectedSeller( void )
{ 
	return Centrala::Storage().GetSeller( m_SelectedSellerId );
}

SELLER_ID BurzaDialogs::RegisterGetSelectedSellerId( void )
{
	return m_SelectedSellerId;
}

SellerData BurzaDialogs::CashoutGetSelectedSeller( void )
{ 
	return Centrala::Storage().GetSeller( m_CashoutSelectedSellerId );
}

int BurzaDialogs::NahrajDialogy( void )
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
	xmlWndRegister->get_widget( "treeview items", m_pTreeViewItems );
	xmlWndRegister->get_widget( "button add item", m_pButtonAddItem );
	xmlWndRegister->get_widget( "button modify item", m_pButtonModifyItem );
	xmlWndRegister->get_widget( "button remove item", m_pButtonRemoveItem );
	xmlWndRegister->get_widget( "button calc fee", m_pButtonCalcFee );
	xmlWndRegister->get_widget( "button calc repayment", m_pButtonCalcRepayment );
	xmlWndRegister->get_widget( "label fee", m_pLabelFee );
	xmlWndRegister->get_widget( "label seller note", m_pLabelSellerNote );
	xmlWndRegister->get_widget( "button cashout", m_pButtonCashout );
	
	if (( m_pComboSeller == NULL ) ||
		( m_pButtonImportSeller == NULL ) ||
		( m_pButtonAddSeller == NULL ) ||
		( m_pButtonUpdateSeller == NULL ) ||
		( m_pTreeViewItems == NULL ) ||
		( m_pButtonAddItem == NULL ) ||
		( m_pButtonModifyItem == NULL ) ||
		( m_pButtonRemoveItem == NULL ) ||
		( m_pButtonCalcFee == NULL ) ||
		( m_pButtonCalcRepayment == NULL ) ||
		( m_pLabelFee == NULL ) ||
		( m_pLabelSellerNote == NULL ) ||
		( m_pButtonCashout  == NULL )) {
		
		error = "Dialog file is corrupt [1].";
		throw;
	}
	
	m_pButtonImportSeller->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonRegisterImportSeller ) );
	m_pButtonAddSeller->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonRegisterAddSeller ) );
	m_pButtonUpdateSeller->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonRegisterUpdateSeller ) );
	m_pButtonAddItem->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonRegisterAddItem ) );
	m_pButtonModifyItem->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonRegisterModifyItem ) );
	m_pButtonRemoveItem->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonRegisterRemoveItem ) );
	m_pButtonCalcFee->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonRegisterCalcFee ) );
	m_pButtonCalcRepayment->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonRegisterCalcRepayment ) );
	m_pButtonCashout->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonSwitchToCashout ) );
	
	m_pComboSeller->signal_changed().connect( sigc::mem_fun( *this, &BurzaDialogs::ComboRegisterChangedSeller ) );
	
	m_RegisterItemsSelection = m_pTreeViewItems->get_selection();
	m_RegisterItemsSelection->set_mode( Gtk::SELECTION_MULTIPLE );
	
	// setup item list
	m_RegisterItemsListModel = Gtk::ListStore::create( m_ItemsColumn );
	m_pTreeViewItems->set_model( m_RegisterItemsListModel );
	m_pTreeViewItems->append_column( "#", m_ItemsColumn.m_number );
	m_pTreeViewItems->append_column( "nazov", m_ItemsColumn.m_name );
	m_pTreeViewItems->append_column( "cena", m_ItemsColumn.m_price );
	m_pTreeViewItems->append_column( "kategoria", m_ItemsColumn.m_category );
	m_pTreeViewItems->append_column( "stav", m_ItemsColumn.m_state );
	
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
		pColumn->set_sort_column( m_ItemsColumn.m_name );
		pColumn->set_min_width( 400 );
	}
	pColumn = m_pTreeViewItems->get_column( 2 );
	if( pColumn ) {
		//pColumn->set_sort_column( m_ItemsColumn.m_nazov );
		pColumn->set_min_width( 80 );
	}
	pColumn = m_pTreeViewItems->get_column( 3 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_category );
		pColumn->set_min_width( 50 );
	}
	pColumn = m_pTreeViewItems->get_column( 4 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_state );
		pColumn->set_min_width( 100 );
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
	
	m_pButtonRegister->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonSwitchToRegister ) );
	m_pCashoutButtonAddItem->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonCashoutAddItem ) );
	m_pCashoutButtonRemoveItem->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonCashoutRemoveItem ) );
	m_pCashoutButtonFinish->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonCashoutFinished ) );
	
	m_CashoutItemsSelection = m_pCashoutTreeViewList->get_selection();
	m_CashoutItemsSelection->set_mode( Gtk::SELECTION_SINGLE );
	m_CashoutItemsSelection->signal_changed().connect( sigc::mem_fun( this, &BurzaDialogs::TreeSelectChangedItem ) );
	
	m_pCashoutEntryLookupSeller->signal_changed().connect( sigc::mem_fun( this, &BurzaDialogs::CashoutEditLookupSellerChanged ) );
	m_pCashoutEntryLookupItem->signal_changed().connect( sigc::mem_fun( this, &BurzaDialogs::CashoutEditLookupItemChanged ) );
	m_pCashoutComboLookupSeller->signal_changed().connect( sigc::mem_fun( *this,
		&BurzaDialogs::CashoutComboLookupSellerChanged ) );
	m_pCashoutComboLookupItem->signal_changed().connect( 
		sigc::mem_fun( *this, &BurzaDialogs::CashoutComboLookupItemChanged ) );
	
	m_pCashoutComboSearch->signal_changed().connect( sigc::mem_fun( this, &BurzaDialogs::CashoutEditSearchChanged ) );
	m_pCashoutComboSearchEntry->signal_changed().connect( sigc::mem_fun( this, &BurzaDialogs::CashoutEditSearchEntryChanged ) );

	// setup item list
	m_CashoutItemsListModel = Gtk::ListStore::create( m_ItemsColumn );
	m_pCashoutTreeViewList->set_model( m_CashoutItemsListModel );
	m_pCashoutTreeViewList->append_column( "#", m_ItemsColumn.m_number );
	m_pCashoutTreeViewList->append_column( "nazov", m_ItemsColumn.m_name );
	m_pCashoutTreeViewList->append_column( "cena", m_ItemsColumn.m_price );
	m_pCashoutTreeViewList->append_column( "kategoria", m_ItemsColumn.m_category );
	m_pCashoutTreeViewList->append_column( "stav", m_ItemsColumn.m_state );
	
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
		pColumn->set_sort_column( m_ItemsColumn.m_name );
		pColumn->set_min_width( 400 );
	}
	pColumn = m_pCashoutTreeViewList->get_column( 2 );
	if( pColumn ) {
		//pColumn->set_sort_column( m_ItemsColumn.m_nazov );
		pColumn->set_min_width( 80 );
	}
	pColumn = m_pCashoutTreeViewList->get_column( 3 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_category );
		pColumn->set_min_width( 50 );
	}
	pColumn = m_pCashoutTreeViewList->get_column( 4 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_state );
		pColumn->set_min_width( 100 );
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
	
	m_pItemeditOk->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonItemEditOk ) );
	m_pItemeditCancel->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonItemEditCancel ) );
	m_pWndItem->signal_delete_event().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonItemEditClose ) );
	m_pItemeditStatusCombo->signal_changed().connect( sigc::mem_fun( *this, &BurzaDialogs::ComboItemEditStatusChanged ) );
	
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
	
	m_pSellerEditOk->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonSellerEditOk ) );
	m_pSellerEditCancel->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonSellerEditCancel ) );
	m_pWndSeller->signal_delete_event().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonSellerEditClose ) );
	
		// === --- === --- === throw - catch  === --- === --- ===
	
	}
	catch( ... ) {
	}
	
	if ( ! error.empty() ) {
		DisplayErrorMessageBox( error, "Error initing dialog." );
		return -1;
	}
	
	
}

void BurzaDialogs::RegisterZobrazPredajcu( const char* kod )
{
	SellerData seller;
	
	try {
		string codeString(kod);
		seller = Centrala::Storage().GetSellerByCode(codeString);
		RegisterZobrazPredajcu(seller);
	}
	catch(WrongSellerException) {
	}
}

void BurzaDialogs::RegisterZobrazPredajcuCezIndex( unsigned int idx )
{
	SellerDataArray	sellers = Centrala::Storage().GetSellers();

	if ( idx >= sellers.size() ) {
		m_SelectedSellerId = SELLER_ID_NONE;
		assert( false );
		return;
	}

	RegisterZobrazPredajcu(sellers[ idx ]);
}

void BurzaDialogs::RegisterZobrazPredajcu(SellerData& seller)
{
	m_SelectedSellerId = seller.id;
	
	m_pLabelSellerNote->set_label( seller.note );
	
	// fill items
	ItemDataArray	items = Centrala::Storage().GetSellerItems(seller.id);
	ItemCategoryArray	categories = Centrala::Storage().GetItemsCategories(items);

	Gtk::TreeModel::Row row;
	
	m_RegisterItemsListModel->clear();
	
	for( int i=0; i<items.size(); i++ ) {
		
		row = * (m_RegisterItemsListModel->append());
		
		row[ m_ItemsColumn.m_number ] = ConvFuncsInstance->FormatCstr( "%i", items[i].number );
		row[ m_ItemsColumn.m_name ] = items[i].name;
		row[ m_ItemsColumn.m_price ] = ConvFuncsInstance->FormatCstr( "%0.2f", items[i].price );
		row[ m_ItemsColumn.m_category ] = ConvFuncsInstance->FormatCstr( "%c", categories[i].letter );
		row[ m_ItemsColumn.m_state ] = ConvFuncsInstance->FormatCstr( "%s", DataAccessInterface::Stav2String( items[i].status ) );
	}
	
	SellerDataArray sellers = Centrala::Storage().GetSellers();
	int idx;
	
	for( idx = 0; idx < sellers.size(); idx++ ) {
		if ( sellers[ idx ].id == seller.id ) {
			break;
		}
	}
	
	if ( idx >= sellers.size() ) {
		idx = -1;
	}
	m_pComboSeller->set_active( idx );
	// i = m_pComboSeller->get_active_row_number();
}

void BurzaDialogs::VyplnPredajcovDoKomb( void )
{
	m_DisplayedSellers = Centrala::Storage().GetSellers();
	std::string		str;
	
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

void BurzaDialogs::ButtonRegisterImportSeller( void )
{
	ExcelFileWrapper	x;
	Gtk::FileChooserDialog dialog("Select importer file", Gtk::FILE_CHOOSER_ACTION_OPEN );

	dialog.set_transient_for( *m_pWndRegister );
	//Add response buttons the the dialog:
	dialog.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );
	dialog.add_button( Gtk::Stock::OK, Gtk::RESPONSE_OK );
	dialog.set_filename( Centrala::Settings().m_data_dir_path );
	
	Gtk::FileFilter	xls_filter;
	xls_filter.set_name( "Excel XLS files." );
	xls_filter.add_pattern( "*.xls" );
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
	
	string filename = dialog.get_filename();
	// s += "data/harok-burza.xls";
	
	SellerData seller;
	ItemDataArray items;
	
	try {
		DataTextfile	textStorage( Centrala::Settings().m_data_dir_path );
		textStorage.LoadFromTxtFile(filename, seller, items );
	}
	catch( runtime_error& e ) {
		DisplayErrorMessageBox( string( "Error importing file." ) + e.what(), "Error importing file." );
		return;
	}
	
	DataStorage	&storage = Centrala::Storage();
	
	bool sellerExists = true;
	try {
		storage.GetSellerByCode( seller.shortcode );
	}
	catch( WrongSellerException ) {
		sellerExists = false;
	}
	
	if ( sellerExists ) {
		DisplayErrorMessageBox( 
			ConvFuncs::FormatCstr( _( "Error seller with that code already exists. Erase previous '%s' seller." ),
				seller.shortcode.c_str() ) );
		return;
	}
	
	seller.id = storage.AddSeller( seller );
	storage.AddItems( items, seller.id );
}

void BurzaDialogs::ButtonRegisterAddSeller( void )
{
	SELLER_ID sellerid = ::Storage().AddSeller(SellerData(_("Nový predajca"), "", "", 0));
	
	m_sellerEditAction = ea_add;
	StartDialogEditSeller( sellerid );
}

void BurzaDialogs::ButtonRegisterUpdateSeller( void )
{
	m_sellerEditAction = ea_modify;
	if ( RegisterAnySelectedSeller() )
		StartDialogEditSeller( GetSelectedSellerId() );
}

void BurzaDialogs::ButtonRegisterAddItem( void )
{
	if ( !RegisterAnySelectedSeller() )
		return;
	
	ITEM_ID newitemid = Centrala::Storage().AddItem( ItemData("", -1, 0.0, 0, not_in_stock_yet), m_SelectedSellerId );
	
	m_productEditAction = ea_add;
	StartDialogEditProduct( newitemid );
}

void BurzaDialogs::ButtonRegisterModifyItem( void )
{
	if ( !RegisterAnySelectedSeller() )
		return;
	
	vector< ITEM_ID > listOfItems;
	RegisterGetSelectedItemIds( listOfItems );
	
	if ( listOfItems.size() != 1 )
		return;
	
	m_productEditAction = ea_modify;
	StartDialogEditProduct( listOfItems[0] );
}

void BurzaDialogs::ButtonRegisterRemoveItem( void )
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
	
	RegisterZobrazPredajcu( m_SelectedSellerId );
}

void BurzaDialogs::ButtonRegisterCalcFee( void )
{
	if ( !RegisterAnySelectedSeller() )
		return;
	
	ItemDataArray items = Centrala::Storage().GetSellerItems( m_SelectedSellerId );
	
	double fee = Centrala::Register().CalculateFee( m_SelectedSellerId, items );
	
	m_pLabelFee->set_text( ConvFuncs::FormatStdStr( "%0.2f %s", fee, 
		Centrala::Settings().m_string_mena.c_str() ) );
}

void BurzaDialogs::ButtonRegisterCalcRepayment( void )
{
	m_pLabelFee->set_text( string( " -4500.00 ") + Centrala::Settings().m_string_mena );
	
	if ( !RegisterAnySelectedSeller() ) {
		return;
	}

	try {
	  Recipees recipees;
	  HPDFWrapper	pdf;
	  pdf.CreatePDF( "/home/mirex/tmp/a.pdf" );
	  /*
	  for( unsigned int i=0; i<150; i++ ) {
		  pdf.NewPage( ConvFuncs::FormatCstr( "index %i", i ) );
		  pseller->PrintProductsToPDF( pdf );
	  }
	  */

	  recipees.PrintSellerStatus( pdf, m_SelectedSellerId );
	  
	  pdf.ClosePDF();
	
	} catch( runtime_error& e ) {
		DisplayErrorMessageBox( "PDF error", e.what() );
	} catch( out_of_range& e ) {
		DisplayErrorMessageBox( "PDF out of range error", e.what() );
	} catch( ... ) {
		DisplayErrorMessageBox( "PDF error" );
	}
}

void BurzaDialogs::ButtonSwitchToCashout( void )
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

	VyplnPredajcovDoKomb();
	
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
}

void BurzaDialogs::ButtonSwitchToRegister( void )
{
	m_pWndRegister->show();
	m_pWndCashOut->hide();
	m_pWndRegister->maximize();
	
	VyplnPredajcovDoKomb();
	m_pLabelFee->set_text( "" );
}

void BurzaDialogs::ComboRegisterChangedSeller( void )
{
	int i = m_pComboSeller->get_active_row_number();
	
	if ( i == -1 )
		return;
	
	if ( i >= m_DisplayedSellers.size() ) {
		ReloadSellers();
		return;
	}
	
	RegisterZobrazPredajcuCezIndex( i );
}

void BurzaDialogs::TreeSelectChangedItem( void )
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
		throw runtime_error();
	}
  
	listOfIds.clear();
	
	vector<unsigned int>	selectedIndexes;
	
	// count selected items
	GetSelectedItemNumbers( m_RegisterItemsSelection, selectedIndexes );
	
	for( int i=0; i<selectedIndexes.size(); ++i ) {
		
		if ( m_RegisterDisplayedItems.size() < selectedIndexes[ i ] ) {
			assert( m_RegisterDisplayedItems.size() < selectedIndexes[ i ] );
			return;
		}
		
		listOfIds.push_back( m_RegisterDisplayedItems[ selectedIndexes[ i ] ].id );
	}
}

void BurzaDialogs::ButtonCashoutAddItem( void )
{
	if ( ! CashoutAnySelectedItem() )
		return;
	
	ShoppingCartIterator it = m_shoppingCart.find( CashoutGetSelectedItemId() );
	if ( it != m_shoppingCart.end() ) {
		return;
	}
	
	// ked nieje, tak pridaj
	ItemData item = CashoutGetSelectedItem();
	ItemCategory itemcat = Centrala::Storage().GetItemCategory( item.category );
	
	row = * (m_CashoutItemsListModel->append());
	
	row[ m_ItemsColumn.m_number ] = ConvFuncsInstance->FormatCstr( "%i", item.number );
	row[ m_ItemsColumn.m_name ] = item.name;
	row[ m_ItemsColumn.m_price ] = ConvFuncsInstance->FormatCstr( "%0.2f", item.price );
	row[ m_ItemsColumn.m_category ] = ConvFuncsInstance->FormatCstr( "%c", itemcat.letter );
	row[ m_ItemsColumn.m_state ] = ConvFuncsInstance->FormatCstr( "%s", 
		DataAccessInterface::ItemStatus2String( item.status ) );
	row[ m_ItemsColumn.m_seller_id ] = item.seller_id;
	row[ m_ItemsColumn.m_item_id ] = item.id;
	
	m_shoppingCart.insert( item.id );
	
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

void BurzaDialogs::ButtonCashoutFinished( void )
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

void BurzaDialogs::ButtonCashoutRemoveItem( void )
{
	vector<unsigned int> listOfItems;
	ItemDataArray listOfProducts;
	
	// get selected items into list
	GetSelectedItemNumbers( m_CashoutItemsSelection, listOfItems );
	
	// get items into list
	for(int i=0; i<listOfItems.size(); ++i ) {
		listOfProducts.push_back( m_shoppingCart[ listOfItems[ i ] ] );
	}
	
	// sort indexes of the items from biggest index to smallest, so they 
	// are removed correctly from the visual list
	std::sort(listOfItems.begin(), listOfItems.end());
	std::reverse(listOfItems.begin(), listOfItems.end());
	
	// remove items
	for(int i=0; i<listOfProducts.size(); ++i ) {
		
		m_CashoutItemsListModel->erase( listOfItems[ i ] );
		m_shoppingCart.erase( listOfProducts[ i ] );
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
	
	m_CashoutSelectedSellerId = m_DisplayedSellers[ i ];
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
}

void BurzaDialogs::CashoutSellerChanged( void )
{
	m_pCashoutComboLookupItem->clear();
	
	if ( ! CashoutAnySelectedSeller() )
		return;
	
	std::string		str;
	
	ItemDataArray items = Centrala::Storage().GetSellerItems( CashoutGetSelectedSellerId() );
	
	for( int i = 0; i < items.size(); ++i ) {
		str = ConvFuncsInstance->FormatCstr( "%i - %s", 
			items[ i ].number,
			items[ i ].name.c_str() );
		m_pCashoutComboLookupItem->append_text( str );
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

void BurzaDialogs::StartDialogEditProduct( ITEM_ID itemid )
{
	ItemData item;
	
	try {
		item = Centrala::Storage().GetItem( itemid );
	}
	catch(WrongItemException)
	{
		throw runtime_error( "BurzaDialogs::ButtonRegisterModifyItem item not in list" );
	}
	
	ItemCategory cat = Centrala::Storage().GetItemCategory( item.category );
	
	m_pItemeditName->set_text( item.name );
	m_pItemeditNumber->set_text( ConvFuncs::FormatCstr( "%i", item.number ) );
	m_pItemeditPrice->set_text( ConvFuncs::FormatCstr( "%0.2f", item.price ) );
	m_pItemeditCategory->set_text( ConvFuncs::FormatCstr( "%c", cat.letter ) );
	
	m_pItemeditStatusCombo->clear();
	for( ItemStatus i = not_in_stock_yet; i < ItemStatusCount; ++i ) {
		m_pItemeditStatusCombo->append_text( 
			DataAccessInterface::ItemStatus2String( i ) );
	}
	m_pItemeditStatusCombo->set_active( item.status );
	
	m_EditedItem = item;
	m_pWndItem->show();
}

void BurzaDialogs::ButtonItemEditOk( void )
{
	assert( m_EditedItem.id != ITEM_ID_NONE );
	
	// validate
	bool validate_needed = false;
	string str;
	
	if ( m_pItemeditName->get_text().empty() ) {
		m_pItemeditName->set_text( _("item name") );
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
		validate_needed = true;
	} else {
		
		if ( atof( str.c_str() ) < 0 ) {
			m_pItemeditPrice->set_text( "1.0" );
			validate_needed = true;
		}
	}
	
	if ( m_pItemeditName->get_text().empty() ) {
		m_pItemeditName->set_text( _("item name") );
		validate_needed = true;
	}
	
	// number
	str = m_pItemeditNumber->get_text();
	int num = atoi( str.c_str() );
	
	num = Centrala::Register().VerifyItemNumber( m_EditedItem.id, num );
	string str2 = ConvFuncs::FormatStdStr( "%i", num );
	if ( str != str2 ) {
		m_pItemeditNumber->set_text( str2 );
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
		m_EditedItem.category = ITEM_CATEGORY_ID_NONE;
	else
	  asdfasdf asdfasdfads
		m_productEdited->m_kategoria = m_pItemeditCategory->get_text().c_str()[0];
	
	int i = m_pItemeditStatusCombo->get_active_row_number();
	if ( i != -1 ) {
		m_productEdited->m_stav = (e_product_status) i;
	}
	
	SellerModified( m_sellerEdited );
	
	Seller* sellerThatWasModified = m_sellerEdited;
	m_productEdited = NULL;
	m_sellerEdited = NULL;

	// add new item in row.
	if ( m_productEditAction == ea_add ) {
		
		sellerThatWasModified->PridajTovar( -1, "", 0.0, 'a', caka_na_prijatie );
		Product* p = & m_pRegisterVybranyPredajca->m_products[ 
			m_pRegisterVybranyPredajca->m_products.size() - 1 ];
		
		m_productEditAction = ea_add;
		StartDialogEditProduct( p, sellerThatWasModified );
	}
	
	SellerData seller = Centrala::Storage().GetSeller( m_EditedItem.id );
}

void BurzaDialogs::ButtonItemEditCancel( void )
{
	if ( m_productEditAction == ea_add )
		m_sellerEdited->m_products.erase( m_sellerEdited->m_products.end() -1 );
	
	m_pWndItem->hide();
	m_productEdited = NULL;
	m_sellerEdited = NULL;
}


bool BurzaDialogs::ButtonItemEditClose( GdkEventAny* )
{
	ButtonItemEditCancel(); 
	return true; 
}

void BurzaDialogs::ComboItemEditStatusChanged( void )
{
}

void BurzaDialogs::StartDialogEditSeller( SELLER_ID id )
{
	m_pSellerEditName->set_text( seller.name );
	m_pSellerEditCode->set_text( seller.shortcode );
	m_pSellerEditNote->set_text( seller.note );
	
	assert( m_sellerEdited == NULL );
	m_sellerEdited = seller;
	m_pWndSeller->show();
}

void BurzaDialogs::ButtonSellerEditOk( void )
{
	assert( m_sellerEdited != NULL );
	
	// validate
	string str;
	
	if ( m_pSellerEditName->get_text().empty() ) {
		m_pSellerEditName->set_text( _( "Seller" ) );
		return;
	}
	
	string sellerOriginalCode = m_sellerEdited->m_kod;
	str = m_pSellerEditCode->get_text();
	// check new code and set it to seller
	mp_register->GenerateSellerUniqueCode( *m_sellerEdited, str );
	if ( m_sellerEdited->m_kod != str ) {
		m_pSellerEditCode->set_text( m_sellerEdited->m_kod );
		m_sellerEdited->m_kod = sellerOriginalCode;
		return;
	}
	
	m_sellerEdited->m_meno = m_pSellerEditName->get_text();
	m_sellerEdited->m_poznamka = m_pSellerEditNote->get_text();
	
	m_pWndSeller->hide();
	
	SellerModified( m_sellerEdited );
	Seller* sellerThatWasModified = m_sellerEdited;
	m_sellerEdited = NULL;
	
	if ( m_sellerEditAction == ea_add ) {
		
		sellerThatWasModified->PridajTovar( -1, "", 0.0, 'a', caka_na_prijatie );
		Product* p = & m_pRegisterVybranyPredajca->m_products[ 
			m_pRegisterVybranyPredajca->m_products.size() - 1 ];
		
		m_productEditAction = ea_add;
		
		StartDialogEditProduct( p, sellerThatWasModified );
	}
}

void BurzaDialogs::ButtonSellerEditCancel( void )
{
	if ( m_sellerEditAction == ea_add )
		Centrala::Instancia()->Register()->m_predajcovia.erase( 
			Centrala::Instancia()->Register()->m_predajcovia.end() -1 );
	
	m_sellerEdited = NULL;
	m_pWndSeller->hide();
}

bool BurzaDialogs::ButtonSellerEditClose( GdkEventAny* )
{
	ButtonSellerEditCancel(); 
	return true; 
}

void BurzaDialogs::SellerModified( SellerData& seller )
{
	seller->Save();
	VyplnPredajcovDoKomb();
	RegisterZobrazPredajcu( seller.shortcode.c_str() );
}

void BurzaDialogs::CashoutEditLookupSellerChanged( void )
{
	string str = m_pCashoutEntryLookupSeller->get_text();
	CashoutZobrazPredajcu( str.c_str() );
}

void BurzaDialogs::CashoutZobrazPredajcu( const char* kod )
{
	int idx = mp_register->NajdiPredajcuPodlaKoduIndex( kod );
	
	m_pCashoutComboLookupItem->clear();
	
	if ( idx == -1 ) {
		m_pCashoutComboLookupSeller->set_active( -1 );
		m_pCashoutVybranyPredajca = NULL;
	} else {
		m_pCashoutComboLookupSeller->set_active( idx );
		m_pCashoutVybranyPredajca = & mp_register->m_predajcovia[ idx ];
	}
	
	CashoutSellerChanged();
}

void BurzaDialogs::CashoutEditLookupItemChanged( void )
{
	if ( m_pCashoutVybranyPredajca == NULL )
		return;
	
	// set it into combo
	int num = atoi( m_pCashoutEntryLookupItem->get_text().c_str() );
	int idx = m_pCashoutVybranyPredajca->NajdiTovarIndex( num );
	m_pCashoutComboLookupItem->set_active( idx );
	// and let combo to do the work.
	CashoutComboLookupItemChanged();
}

void BurzaDialogs::CashoutEditSearchChanged( void )
{
	// m_pCashoutComboLookupItem
	//m_pCashoutComboSearch
}

void BurzaDialogs::CashoutEditSearchEntryChanged( void )
{
}


/*
	BurzaRegister::t_predajcovia::iterator	it;
	Seller *pseller;
	//it = Centrala::Instancia()->Register()->NajdiPredajcuPodlaKoduIt( "mj" );
	pseller = & Centrala::Instancia()->Register()->m_predajcovia[ 0 ];
	
	try {
	HPDFWrapper	pdf;
	pdf.CreatePDF( "/home/mirex/tmp/a.pdf" );
	for( unsigned int i=0; i<150; i++ ) {
		pdf.NewPage( ConvFuncs::FormatCstr( "index %i", i ) );
		pseller->PrintProductsToPDF( pdf );
	}
	pdf.ClosePDF();
	
	} catch( runtime_error& e ) {
		DisplayErrorMessageBox( "PDF error", e.what() );
	} catch( out_of_range& e ) {
		DisplayErrorMessageBox( "PDF out of range error", e.what() );
	} catch( ... ) {
		DisplayErrorMessageBox( "PDF error" );
	}
	
*/

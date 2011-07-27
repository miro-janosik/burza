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
	mp_register = Centrala::Instancia()->Register();
	
	m_pRegisterVybranyPredajca = NULL;
	
	m_pCashoutVybranyPredajca = NULL;
	m_CashoutVybranyPredajcaIdx = -1;
	m_pCashoutVybranyTovar = NULL;
	m_CashoutVybranyTovarIdx = -1;
	
	m_cashout_sum = 0.0;
	
	m_productEdited = NULL;
	m_sellerEdited = NULL;
	
	ConvFuncs::SingletonInstanceCreate();
}

BurzaDialogs::~BurzaDialogs()
{
	ConvFuncs::SingletonInstanceDelete();
}

int BurzaDialogs::NahrajDialogy( void )
{
	std::string		glade_fname;
	
	glade_fname = Centrala::Instancia()->Nastavenia()->m_data_dir_path;
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
	m_pTreeViewItems->append_column( "#", m_ItemsColumn.m_cislo );
	m_pTreeViewItems->append_column( "nazov", m_ItemsColumn.m_nazov );
	m_pTreeViewItems->append_column( "cena", m_ItemsColumn.m_cena );
	m_pTreeViewItems->append_column( "kategoria", m_ItemsColumn.m_kategoria );
	m_pTreeViewItems->append_column( "stav", m_ItemsColumn.m_stav );
	
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
		pColumn->set_sort_column( m_ItemsColumn.m_cislo );
		pColumn->set_min_width( 50 );
	}
	pColumn = m_pTreeViewItems->get_column( 1 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_nazov );
		pColumn->set_min_width( 400 );
	}
	pColumn = m_pTreeViewItems->get_column( 2 );
	if( pColumn ) {
		//pColumn->set_sort_column( m_ItemsColumn.m_nazov );
		pColumn->set_min_width( 80 );
	}
	pColumn = m_pTreeViewItems->get_column( 3 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_kategoria );
		pColumn->set_min_width( 50 );
	}
	pColumn = m_pTreeViewItems->get_column( 4 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_stav );
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
	m_pCashoutTreeViewList->append_column( "#", m_ItemsColumn.m_cislo );
	m_pCashoutTreeViewList->append_column( "nazov", m_ItemsColumn.m_nazov );
	m_pCashoutTreeViewList->append_column( "cena", m_ItemsColumn.m_cena );
	m_pCashoutTreeViewList->append_column( "kategoria", m_ItemsColumn.m_kategoria );
	m_pCashoutTreeViewList->append_column( "stav", m_ItemsColumn.m_stav );
	
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
		pColumn->set_sort_column( m_ItemsColumn.m_cislo );
		pColumn->set_min_width( 50 );
	}
	pColumn = m_pCashoutTreeViewList->get_column( 1 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_nazov );
		pColumn->set_min_width( 400 );
	}
	pColumn = m_pCashoutTreeViewList->get_column( 2 );
	if( pColumn ) {
		//pColumn->set_sort_column( m_ItemsColumn.m_nazov );
		pColumn->set_min_width( 80 );
	}
	pColumn = m_pCashoutTreeViewList->get_column( 3 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_kategoria );
		pColumn->set_min_width( 50 );
	}
	pColumn = m_pCashoutTreeViewList->get_column( 4 );
	if( pColumn ) {
		pColumn->set_sort_column( m_ItemsColumn.m_stav );
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
	// najskor najdi predajcu
	BurzaRegister::t_predajcovia::iterator	it;
	int	idx;
	
	for( idx = 0, it = mp_register->m_predajcovia.begin(); it != mp_register->m_predajcovia.end(); it++, idx++ ) {
		if ( it->m_kod == kod )
			break;
	}
	
	if ( it == mp_register->m_predajcovia.end() )
		return;
	
	RegisterZobrazPredajcuCezIndex( idx );
}

void BurzaDialogs::RegisterZobrazPredajcuCezIndex( unsigned int idx )
{
	if ( idx >= mp_register->m_predajcovia.size() ) {
		m_pRegisterVybranyPredajca = NULL;
		assert( false );
		return;
	}
	
	BurzaRegister::t_predajcovia::iterator	it;
	it = mp_register->m_predajcovia.begin() + idx;
	
	Seller	*pr = & (*it );
	m_pRegisterVybranyPredajca = pr;
			
	m_pComboSeller->set_active( idx );
	// i = m_pComboSeller->get_active_row_number();
	
	m_pLabelSellerNote->set_label( pr->m_poznamka );
	
	// vypln Product
	Gtk::TreeModel::Row row;
	Product	*t;
	
	m_RegisterItemsListModel->clear();
	
	for( int i=0; i<pr->m_products.size(); i++ ) {
		
		t = & pr->m_products[ i ];
		row = * (m_RegisterItemsListModel->append());
		
		row[ m_ItemsColumn.m_cislo ] = ConvFuncsInstance->FormatCstr( "%i", t->m_cislo );
		row[ m_ItemsColumn.m_nazov ] = t->m_nazov;
		row[ m_ItemsColumn.m_cena ] = ConvFuncsInstance->FormatCstr( "%0.2f", t->m_cena );
		row[ m_ItemsColumn.m_kategoria ] = ConvFuncsInstance->FormatCstr( "%c", t->m_kategoria );
		row[ m_ItemsColumn.m_stav ] = ConvFuncsInstance->FormatCstr( "%s", mp_register->Stav2String( t->m_stav ) );
	}
	
}

void BurzaDialogs::VyplnPredajcovDoKomb( void )
{
	
	BurzaRegister::t_predajcovia::iterator	it;
	std::string		str;
	
	m_pComboSeller->clear();
	
	for( it = mp_register->m_predajcovia.begin(); it != mp_register->m_predajcovia.end(); it++ ) {
		str = it->m_kod;
		str += " - ";
		str += it->m_meno;
		m_pComboSeller->append_text( str );
	}
	
	// ---
	m_pCashoutComboLookupSeller->clear();
	
	for( it = mp_register->m_predajcovia.begin(); it != mp_register->m_predajcovia.end(); it++ ) {
		str = it->m_kod;
		str += " - ";
		str += it->m_meno;
		m_pCashoutComboLookupSeller->append_text( str );
	}
}

void BurzaDialogs::ButtonRegisterImportSeller( void )
{
	ExcelFileWrapper	x;
	Seller seller;
	Gtk::FileChooserDialog dialog("Select importer file", Gtk::FILE_CHOOSER_ACTION_OPEN );

	dialog.set_transient_for( *m_pWndRegister );
	//Add response buttons the the dialog:
	dialog.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );
	dialog.add_button( Gtk::Stock::OK, Gtk::RESPONSE_OK );
	dialog.set_filename( Centrala::Instancia()->Nastavenia()->m_data_dir_path );
	
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
	
	string s = dialog.get_filename();
	// s += "data/harok-burza.xls";
	
	try {
		seller = x.ReadSeller( s );
	}
	catch( runtime_error& e ) {
		DisplayErrorMessageBox( string( "Error importing file." ) + e.what(), "Error importing file." );
		return;
	}
	
	BurzaRegister	*reg = Centrala::Instancia()->Register();
	BurzaRegister::t_predajcovia::iterator	it;	
	it = reg->NajdiPredajcuPodlaKoduIt( seller.m_kod.c_str() );
	if ( it != reg->m_predajcovia.end() ) {
		DisplayErrorMessageBox( 
			ConvFuncs::FormatCstr( _( "Error seller with that code already exists. Erase previous '%s' seller." ),
				seller.m_kod.c_str() ) );
		return;
	}
	
	reg->m_predajcovia.push_back(seller );
}

void BurzaDialogs::ButtonRegisterAddSeller( void )
{
	mp_register->PridajNovehoPredajcu( "", _("Nový predajca"), "" );
	
	m_sellerEditAction = ea_add;
	StartDialogEditSeller( &mp_register->m_predajcovia[ 
		mp_register->m_predajcovia.size() - 1 ] );
}

void BurzaDialogs::ButtonRegisterUpdateSeller( void )
{
	m_sellerEditAction = ea_modify;
	if ( m_pRegisterVybranyPredajca != NULL )
		StartDialogEditSeller( m_pRegisterVybranyPredajca );
}

void BurzaDialogs::ButtonRegisterAddItem( void )
{
	if ( m_pRegisterVybranyPredajca == NULL )
		return;
	
	m_pRegisterVybranyPredajca->PridajTovar( -1, "", 0.0, 'a', caka_na_prijatie );
	Product* p = & m_pRegisterVybranyPredajca->m_products[ 
		m_pRegisterVybranyPredajca->m_products.size() - 1 ];
	
	m_productEditAction = ea_add;
	StartDialogEditProduct( p, m_pRegisterVybranyPredajca );
}

void BurzaDialogs::ButtonRegisterModifyItem( void )
{
	if ( m_pRegisterVybranyPredajca == NULL )
		return;
	
	vector< unsigned int > listOfItems;
	RegisterGetSelectedItemNumbers( listOfItems );
	
	if ( listOfItems.size() != 1 )
		return;
	
	int idx= m_pRegisterVybranyPredajca->NajdiTovarIndex( listOfItems[ 0 ] );
	if ( idx == -1 )
		throw runtime_error( "BurzaDialogs::ButtonRegisterModifyItem item not in list" );
	
	Product* p = & m_pRegisterVybranyPredajca->m_products[ idx ];
	
	m_productEditAction = ea_modify;
	StartDialogEditProduct( p, m_pRegisterVybranyPredajca );
}

void BurzaDialogs::ButtonRegisterRemoveItem( void )
{
	if ( m_pRegisterVybranyPredajca == NULL )
		return;
	
	// to avoid pointing to array that removes under our feet I'll make list of item numbers to delete
	vector< unsigned int > productNumbersToDelete;
	
	RegisterGetSelectedItemNumbers( productNumbersToDelete );
	
	// ask user if he is sure
	string str = ConvFuncs::FormatStdStr( _( "Ste si istý že chcete odstrániť %i ks tovaru?" ), productNumbersToDelete.size() );
	if ( DisplayYesNoMessageBox( str, _("Odstránenie tovaru") ) == false )
		return;
	
	// delete items
	for( unsigned int i=0; i<productNumbersToDelete.size(); ++i ) {
		m_pRegisterVybranyPredajca->ZmazTovar( productNumbersToDelete[ i ] );
	}
	
	RegisterZobrazPredajcu( m_pRegisterVybranyPredajca->m_kod.c_str() );
}

void BurzaDialogs::ButtonRegisterCalcFee( void )
{
	if ( m_pRegisterVybranyPredajca == NULL )
		return;
	
	double fee = m_pRegisterVybranyPredajca->CalculateFee();
	
	m_pLabelFee->set_text( ConvFuncs::FormatStdStr( "%0.2f %s", fee, 
		mp_register->m_nastavenia.m_string_mena.c_str() ) );
}

void BurzaDialogs::ButtonRegisterCalcRepayment( void )
{
	m_pLabelFee->set_text( string( " -4500.00 ") + mp_register->m_nastavenia.m_string_mena );
	
	Recipees recipees;
	Seller* seller = RegisterGetSelectedSeller();
	if (seller == NULL) {
		return;
	}

	try {
	  HPDFWrapper	pdf;
	  pdf.CreatePDF( "/home/mirex/tmp/a.pdf" );
	  /*
	  for( unsigned int i=0; i<150; i++ ) {
		  pdf.NewPage( ConvFuncs::FormatCstr( "index %i", i ) );
		  pseller->PrintProductsToPDF( pdf );
	  }
	  */

	  recipees.PrintSellerStatus( pdf, *seller );
	  
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

void BurzaDialogs::RegisterGetSelectedItemNumbers( vector<unsigned int>& listOfItems )
{
	listOfItems.clear();
	
	vector<unsigned int>	selectedIndexes;
	
	// count selected items
	GetSelectedItemNumbers( m_RegisterItemsSelection, selectedIndexes );
	
	for( int i=0; i<selectedIndexes.size(); ++i ) {
		
		listOfItems.push_back( 
			m_pRegisterVybranyPredajca->m_products[ selectedIndexes[ i ] ].m_cislo );
	}
}

Seller* BurzaDialogs::RegisterGetSelectedSeller( void )
{
  return m_pRegisterVybranyPredajca;
}

void BurzaDialogs::ButtonCashoutAddItem( void )
{
	if ( m_pCashoutVybranyTovar == NULL )
		return;
	
	// pozri ci tam uz taky item nahodou nieje
	Gtk::TreeModel::Children	::iterator	it, itb, ite;
	Gtk::TreeModel::Row row;
	
	itb = m_CashoutItemsListModel->children().begin();
	ite = m_CashoutItemsListModel->children().end();
	
	for( it=itb; it!=ite; ++it ) {
		row = *it;
		if (( row[ m_ItemsColumn.m_predajca_idx ] == m_CashoutVybranyPredajcaIdx ) &&
			( row[ m_ItemsColumn.m_Product_idx ] == m_CashoutVybranyTovarIdx )) {
		return;
		}
	}
	
	// ked nieje, tak pridaj
	Product	*t = m_pCashoutVybranyTovar;
	row = * (m_CashoutItemsListModel->append());
	
	row[ m_ItemsColumn.m_cislo ] = ConvFuncsInstance->FormatCstr( "%i", t->m_cislo );
	row[ m_ItemsColumn.m_nazov ] = t->m_nazov;
	row[ m_ItemsColumn.m_cena ] = ConvFuncsInstance->FormatCstr( "%0.2f", t->m_cena );
	row[ m_ItemsColumn.m_kategoria ] = ConvFuncsInstance->FormatCstr( "%c", t->m_kategoria );
	row[ m_ItemsColumn.m_stav ] = ConvFuncsInstance->FormatCstr( "%s", mp_register->Stav2String( t->m_stav ) );
	row[ m_ItemsColumn.m_predajca_idx ] = m_CashoutVybranyPredajcaIdx;
	row[ m_ItemsColumn.m_Product_idx ] = m_CashoutVybranyTovarIdx;
	
	Burza::BurzaRegister::SellerProduct	sp;
	sp.sellerCode = m_pCashoutVybranyPredajca->m_kod;
	sp.productNumber = m_pCashoutVybranyTovar->m_cislo;
	m_shoppingCart.push_back( sp );
	
	CheckoutCartChanged();
}

void BurzaDialogs::CheckoutCartChanged( void )
{
	m_cashout_sum = mp_register->CalcPriceOfProducts( m_shoppingCart );
	
	std::string	str;
	str = ConvFuncsInstance->FormatCstr( _("Suma: %0.2f" ), m_cashout_sum );
	str += " ";
	str += Centrala::Instancia()->Nastavenia()->m_string_mena;
	m_pCashoutLabelSum->set_text( str );
}

void BurzaDialogs::ButtonCashoutFinished( void )
{
	if ( ! DisplayYesNoMessageBox( _("Dokončiť nákup?"), "" )) {
		return;
	}
	
	mp_register->FinishCheckout( m_shoppingCart );
	
	m_CashoutItemsListModel->clear();
	
	m_shoppingCart.clear();
	CheckoutCartChanged();
}

void BurzaDialogs::ButtonCashoutRemoveItem( void )
{
	vector<unsigned int> listOfItems;
	vector<BurzaRegister::SellerProduct> listOfProducts;
	
	// get selected items into list
	GetSelectedItemNumbers( m_CashoutItemsSelection, listOfItems );
	
	// get items into list
	for(int i=0; i<listOfItems.size(); ++i ) {
		listOfProducts.push_back( m_shoppingCart[ listOfItems[ i ] ] );
	}
	
	// remove items
	for(int i=0; i<listOfProducts.size(); ++i ) {
		
		for( int p=0; p<m_shoppingCart.size(); ++p ) {
			
			if ( m_shoppingCart[ p ] == listOfProducts[ i ] ) {
				m_shoppingCart.erase( m_shoppingCart.begin() + p );
				break;
			}
		}
	}
	
	CheckoutCartChanged();
}

void BurzaDialogs::CashoutComboLookupSellerChanged( void )
{
	m_pCashoutVybranyPredajca = NULL;
	m_CashoutVybranyPredajcaIdx = -1;
	
	int i = m_pCashoutComboLookupSeller->get_active_row_number();
	
	if ( i == -1 )
		return;
	
	if ( i >= mp_register->m_predajcovia.size() ) {
		assert( false );
		return;
	}
	
	m_pCashoutVybranyPredajca = & (*( mp_register->m_predajcovia.begin() + i ));
	m_CashoutVybranyPredajcaIdx = i;
	
	PokladnaZmenaPredajcu();
}

void BurzaDialogs::CashoutComboLookupItemChanged( void )
{
	m_pCashoutVybranyTovar = NULL;
	m_CashoutVybranyTovarIdx = -1;
	
	if ( m_pCashoutVybranyPredajca == NULL )
		return;
	
	int i = m_pCashoutComboLookupItem->get_active_row_number();
	if ( i == -1 )
		return;
	
	if ( i >= m_pCashoutVybranyPredajca->m_products.size() ) {
		assert( false );
		return;
	}
	
	m_pCashoutVybranyTovar = & ( * (m_pCashoutVybranyPredajca->m_products.begin() + i ));
	m_CashoutVybranyTovarIdx = i;
	
	CashoutSelectedItemChanged();
}

void BurzaDialogs::CashoutSelectedItemChanged( void )
{
	if ( m_pCashoutVybranyTovar == NULL ) {
		m_pCashoutLabelItemPrice->set_text( "" );
		return;
	}
	
	std::string		str;
	
	str = ConvFuncsInstance->FormatCstr( "%0.2f %s", 
		m_pCashoutVybranyTovar->m_cena, 
		mp_register->Stav2String( m_pCashoutVybranyTovar->m_stav ) );
	m_pCashoutLabelItemPrice->set_text( str );
}

void BurzaDialogs::PokladnaZmenaPredajcu( void )
{
	m_pCashoutComboLookupItem->clear();
	
	if ( m_pCashoutVybranyPredajca == NULL )
		return;
	
	Seller::t_products::iterator	it;
	std::string		str;
	
	for( it = m_pCashoutVybranyPredajca->m_products.begin(); it != m_pCashoutVybranyPredajca->m_products.end(); it++ ) {
		str = ConvFuncsInstance->FormatCstr( "%i - %s", it->m_cislo, it->m_nazov.c_str() );
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

void BurzaDialogs::StartDialogEditProduct( Product* product, Seller* seller )
{
	m_pItemeditName->set_text( product->m_nazov );
	m_pItemeditNumber->set_text( ConvFuncs::FormatCstr( "%i", product->m_cislo ) );
	m_pItemeditPrice->set_text( ConvFuncs::FormatCstr( "%0.2f", product->m_cena ) );
	m_pItemeditCategory->set_text( ConvFuncs::FormatCstr( "%c", product->m_kategoria ) );
	
	m_pItemeditStatusCombo->clear();
	
	for( int i = 0; i < pocet_stavov; ++i ) {
		m_pItemeditStatusCombo->append_text( 
			mp_register->Stav2String( (e_product_status) i ) );
	}
	m_pItemeditStatusCombo->set_active( product->m_stav );
	
	assert( m_productEdited == NULL );
	m_productEdited = product;
	m_sellerEdited = seller;
	m_pWndItem->show();
}

void BurzaDialogs::ButtonItemEditOk( void )
{
	assert( m_productEdited != NULL );
	assert( m_sellerEdited != NULL );
	
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
	
	num = m_sellerEdited->VerifikujCisloTovaru( *m_productEdited, num );
	string str2 = ConvFuncs::FormatStdStr( "%i", num );
	if ( str != str2 ) {
		m_pItemeditNumber->set_text( str2 );
		validate_needed = true;
	}
	
	if ( validate_needed ) {
		return;
	}
	
	m_pWndItem->hide();
	
	m_productEdited->m_nazov = m_pItemeditName->get_text();
	m_productEdited->m_cislo =  atoi( m_pItemeditNumber->get_text().c_str() );
	m_productEdited->m_cena = atof(  m_pItemeditPrice->get_text().c_str() );
	if ( m_pItemeditCategory->get_text().empty() )
		m_productEdited->m_kategoria = ' ';
	else
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

void BurzaDialogs::StartDialogEditSeller( Seller* seller )
{
	m_pSellerEditName->set_text( seller->m_meno );
	m_pSellerEditCode->set_text( seller->m_kod );
	m_pSellerEditNote->set_text( seller->m_poznamka );
	
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

void BurzaDialogs::SellerModified( Seller* seller )
{
	seller->Save();
	VyplnPredajcovDoKomb();
	RegisterZobrazPredajcu( seller->m_kod.c_str() );
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
	
	PokladnaZmenaPredajcu();
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

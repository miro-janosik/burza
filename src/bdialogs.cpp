// -----
// Program: Burza
// Autor: Miro Janosik
// Vytvorene pre MC Srdiecko Trencin
// -----

#include <assert.h>
#include "bdialogs.h"
#include "convfncs.h"

#include "excelwrap.h"

using namespace Burza;
using namespace BurzaExcel;

ComboBoxTextDer::ComboBoxTextDer( BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
	: Gtk::ComboBoxText(cobject)
{ }


// ==============================================
// ==============================================
// ==============================================

BurzaDialogs::BurzaDialogs()
{
	m_pCashoutVybranyPredajca = NULL;
	m_CashoutVybranyPredajcaIdx = -1;
	m_pCashoutVybranyTovar = NULL;
	m_CashoutVybranyTovarIdx = -1;
	
	m_cashout_sum = 0.0;
	
	ConvFuncs::SingletonInstanceCreate();
	
	m_working_directory = 
		ConvFuncsInstance->GetPathFromPathFilename(
			ConvFuncsInstance->GetExecutableFilePathName() );
}

BurzaDialogs::~BurzaDialogs()
{
	ConvFuncs::SingletonInstanceDelete();
}

int BurzaDialogs::NahrajDialogy( void )
{
	std::string		glade_fname;
	
	glade_fname = ConvFuncsInstance->GetPathFromPathFilename(
		ConvFuncsInstance->GetExecutableFilePathName() );
	glade_fname += "data/burza.glade";
	
	std::string		error;
	
	error = "";
	
	try {

	xmlWndRegister = Gnome::Glade::Xml::create( glade_fname.c_str(), "Register" );
	if ( ! xmlWndRegister )
		error = "BiturnDialogsGtkmm::Init() error loading glade item from xml. 'Register'";
	
	xmlWndCashOut = Gnome::Glade::Xml::create( glade_fname.c_str(), "Pokladna" );
	if ( ! xmlWndCashOut )
		error = "BiturnDialogsGtkmm::Init() error loading glade item from xml. 'Pokladna'";
	
	xmlWndRegister->get_widget( "Register", m_pWndRegister );
	xmlWndCashOut->get_widget( "Pokladna", m_pWndCashOut );
	
	if ( m_pWndRegister == NULL ) {
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
	m_pButtonRemoveItem->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonRegisterRemoveItem ) );
	m_pButtonCalcFee->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonRegisterCalcFee ) );
	m_pButtonCalcRepayment->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonRegisterCalcRepayment ) );
	m_pButtonCashout->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonSwitchToCashout ) );
	
	m_pComboSeller->signal_changed().connect( sigc::mem_fun( *this, &BurzaDialogs::ComboChangedSeller ) );
	
	m_RegisterItemsSelection = m_pTreeViewItems->get_selection();
	m_RegisterItemsSelection->set_mode( Gtk::SELECTION_SINGLE );
	m_RegisterItemsSelection->signal_changed().connect( sigc::mem_fun( this, &BurzaDialogs::TreeSelectChangedItem ) );
	
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
		( m_pCashoutButtonFinish == NULL )) {
		error = "Dialog file is corrupt [4].";
		throw;
	}
	
	m_pButtonRegister->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonSwitchToRegister ) );
	m_pCashoutButtonAddItem->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonCashoutAddItem ) );
	m_pCashoutButtonRemoveItem->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonCashoutRemoveItem ) );
	m_pCashoutButtonFinish->signal_clicked().connect( sigc::mem_fun( *this, &BurzaDialogs::ButtonCashoutFinished ) );
	
	m_pCashoutComboLookupSeller->signal_changed().connect( sigc::mem_fun( *this, &BurzaDialogs::ComboCashoutLookupSeller ) );
	m_pCashoutComboLookupItem->signal_changed().connect( sigc::mem_fun( *this, &BurzaDialogs::ComboCashoutLookupItem ) );
	
	m_CashoutItemsSelection = m_pTreeViewItems->get_selection();
	m_CashoutItemsSelection->set_mode( Gtk::SELECTION_SINGLE );
	m_CashoutItemsSelection->signal_changed().connect( sigc::mem_fun( this, &BurzaDialogs::TreeSelectChangedItem ) );
	
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
	
	for( idx = 0, it = m_register.m_predajcovia.begin(); it != m_register.m_predajcovia.end(); it++ ) {
		if ( it->m_kod == kod )
			break;
	}
	
	if ( it == m_register.m_predajcovia.end() )
		return;
	
	RegisterZobrazPredajcuCezIndex( idx );
}

void BurzaDialogs::RegisterZobrazPredajcuCezIndex( unsigned int idx )
{
	if ( idx >= m_register.m_predajcovia.size() ) {
		assert( false );
		return;
	}
	
	BurzaRegister::t_predajcovia::iterator	it;
	it = m_register.m_predajcovia.begin() + idx;
	
	Seller	*pr = & (*it );
	
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
		row[ m_ItemsColumn.m_stav ] = ConvFuncsInstance->FormatCstr( "%s", m_register.Stav2String( t->m_stav ) );
	}
	
}

void BurzaDialogs::VyplnPredajcovDoKomb( void )
{
	
	BurzaRegister::t_predajcovia::iterator	it;
	std::string		str;
	
	m_pComboSeller->clear();
	
	for( it = m_register.m_predajcovia.begin(); it != m_register.m_predajcovia.end(); it++ ) {
		str = it->m_kod;
		str += " - ";
		str += it->m_meno;
		m_pComboSeller->append_text( str );
	}
	
	// ---
	m_pCashoutComboLookupSeller->clear();
	
	for( it = m_register.m_predajcovia.begin(); it != m_register.m_predajcovia.end(); it++ ) {
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

	dialog.set_transient_for( * xmlWndRegister );
	//Add response buttons the the dialog:
	dialog.add_button( Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL );
	dialog.add_button( Gtk::Stock::OK, Gtk::RESPONSE_OK );
	dialog.set_filename( m_working_directory );
	
	int result = dialog.run();
	//Handle the response:
	switch(result) {
	case(Gtk::RESPONSE_OK):
		break;
	case(Gtk::RESPONSE_CANCEL):
		return;
	default:
		DisplayErrorMessageBox( "BiturnDialogsGtkmm::on_clicked_ViewDirs() Unexpected button clicked.\n" );
		return;
	}
	
	string s = dialog.get_filename();
	// s += "data/harok-burza.xls";
	
	try {
		seller = x.ReadSeller( s );
	}
	catch( runtime_exception& e ) {
		DisplayErrorMessageBox( "Error importing file." + e.what(), "Error importing file." );
		return;
	}
	
	BurzaRegister	*reg = Centrala::Instancia()->Register();
	t_predajcovia::iterator	it;	
	it = reg->NajdiPredajcuPodlaKodu( seller.m_kod );
	if ( it != reg->m_predajcovia.end() ) {
		DisplayErrorMessageBox( "Error seller with that code already exists. Erase previous '" + 
			seller.m_kod + "' seller." );
		return;
	}
	
	reg->m_predajcovia.push_back(seller );
}
	kdevelop program translations
geocache olomouc
mapa milan
2% materina duska

void BurzaDialogs::ButtonRegisterAddSeller( void )
{
	
}

void BurzaDialogs::ButtonRegisterUpdateSeller( void )
{
	
}

void BurzaDialogs::ButtonRegisterAddItem( void )
{
	
}

void BurzaDialogs::ButtonRegisterRemoveItem( void )
{
	
}

void BurzaDialogs::ButtonRegisterCalcFee( void )
{
	
}

void BurzaDialogs::ButtonRegisterCalcRepayment( void )
{
	
}

void BurzaDialogs::ButtonSwitchToCashout( void )
{
	m_pWndRegister->hide();
	m_pWndCashOut->show();
	m_pWndCashOut->maximize();
}

void BurzaDialogs::ButtonSwitchToRegister( void )
{
	m_pWndRegister->show();
	m_pWndCashOut->hide();
	m_pWndRegister->maximize();
}

void BurzaDialogs::ComboChangedSeller( void )
{
	int i = m_pComboSeller->get_active_row_number();
	
	if ( i == -1 )
		return;
	
	RegisterZobrazPredajcuCezIndex( i );
}

void BurzaDialogs::TreeSelectChangedItem( void )
{
	
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
	row[ m_ItemsColumn.m_stav ] = ConvFuncsInstance->FormatCstr( "%s", m_register.Stav2String( t->m_stav ) );
	row[ m_ItemsColumn.m_predajca_idx ] = m_CashoutVybranyPredajcaIdx;
	row[ m_ItemsColumn.m_Product_idx ] = m_CashoutVybranyTovarIdx;
	
	m_cashout_sum += t->m_cena;
	
	std::string	str;
	str = ConvFuncsInstance->FormatCstr( "Suma: %0.2f ∈", m_cashout_sum );
	m_pCashoutLabelSum->set_text( str );
}

void BurzaDialogs::ButtonCashoutFinished( void )
{
	// mark items in register
	Gtk::TreeModel::Children	::iterator	it, itb, ite;
	Gtk::TreeModel::Row row;
	
	itb = m_CashoutItemsListModel->children().begin();
	ite = m_CashoutItemsListModel->children().end();
	
	for( it=itb; it!=ite; ++it ) {
		row = *it;
		
		assert( row[ m_ItemsColumn.m_predajca_idx ] < m_register.m_predajcovia.size() );
		
		assert( row[ m_ItemsColumn.m_Product_idx ] < 
			m_register.m_predajcovia[ row[ m_ItemsColumn.m_predajca_idx ] ].m_products.size() );
		
		m_register.m_predajcovia[ row[ m_ItemsColumn.m_predajca_idx ] ].m_products[ 
 			row[ m_ItemsColumn.m_Product_idx ] ].m_stav = predane;
		
	}
	
	m_CashoutItemsListModel->clear();
	
	m_cashout_sum = 0.0;
	m_pCashoutLabelSum->set_text( "0 ∈" );
}

void BurzaDialogs::ButtonCashoutRemoveItem( void )
{
	
}

void BurzaDialogs::ComboCashoutLookupSeller( void )
{
	m_pCashoutVybranyPredajca = NULL;
	m_CashoutVybranyPredajcaIdx = -1;
	
	int i = m_pCashoutComboLookupSeller->get_active_row_number();
	
	if ( i == -1 )
		return;
	
	if ( i >= m_register.m_predajcovia.size() ) {
		assert( false );
		return;
	}
	
	m_pCashoutVybranyPredajca = & (*( m_register.m_predajcovia.begin() + i ));
	m_CashoutVybranyPredajcaIdx = i;
	
	PokladnaZmenaPredajcu();
}

void BurzaDialogs::ComboCashoutLookupItem( void )
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
	
	std::string		str;
	
	str = ConvFuncsInstance->FormatCstr( "%0.2f %s", 
		m_pCashoutVybranyTovar->m_cena, 
		m_register.Stav2String( m_pCashoutVybranyTovar->m_stav ) );
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
	Gtk::MessageDialog dialog;
	
	if ( msg2 == NULL )
		dialog.set_message( "error" );
	else
		dialog.set_message( msg2 );
	
	dialog.set_secondary_text( msg1 );
	dialog.run();
}

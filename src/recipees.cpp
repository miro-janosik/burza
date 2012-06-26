#include "recipees.h"

#include <string>
#include <algorithm>
#include "table.h"
#include "burza.h"
#include "pdfwrap.h"
#include "convfncs.h"
#include "stringencoding.h"
#include "translate.h"
#include "errhandler.h"
#include "log.h"
#include "fees.h"

using namespace Burza;
using namespace BurzaTable;
using namespace std;

float Recipees::marginTop = 8;
float Recipees::marginBottom = 80;
float Recipees::marginLeft = 10;
float Recipees::marginRight = 90;
PdfWrapperBase *Recipees::currentPdf = NULL;

void Recipees::PrintSellerStatus( PdfWrapperBase& pdf, SELLER_ID id )
{
	SellerData seller;
	SellerCategory sellerCat;

	try {
		seller = Centrala::Storage().GetSeller( id );
		sellerCat = Centrala::Storage().GetSellerCategory( seller.category );
	}
	catch(exception& e)
	{
		ErrorHandling::HandleException(e);
	}
	
	pdf.NewPage( ConvFuncs::FormatStdStr( _("%s - %s"), seller.shortcode.c_str(), seller.name.c_str() ) );
	AddProgramPageHeaderAndFooter( pdf );

	pdf.SetFontSize(1.2);
	double ypos = marginTop;
	
	pdf.WriteSingleText( 10, ypos, ConvFuncs::FormatStdStr(_("Predávajúci: %s - %s (%s)"), 
		seller.shortcode.c_str(), seller.name.c_str(),
		sellerCat.name.c_str() ));
	ypos += 1.6;
	pdf.WriteSingleText( 10, ypos, ConvFuncs::FormatStdStr(_("( %s )"), seller.note.c_str() ));
	ypos += 1.6;
	pdf.WriteSingleText( 10, ypos, ConvFuncs::FormatStdStr(_("Stav dňa: %s"), ConvFuncsInstance->CurDateTimeString() ));
	ypos += 1.6;

	TableOfCells	items;
	TableColumnSpecs	columns;
	string str;

	columns.AddColumn( "#", 5 );
	columns.AddColumn( _("číslo"), 5 );
	columns.AddColumn( _("názov"), 40 );
	columns.AddColumn( _("cena"), 10 );
	columns.AddColumn( _("kat"), 5 );
	columns.AddColumn( _("stav"), 15 );
	
	ItemDataArray sellerItems = Centrala::Storage().GetSellerItems( id );
	
	items.Resize(sellerItems.size(), columns.m_columns.size());
	
	for(int i=0; i<sellerItems.size(); ++i ) {
		
		items.Cell( i, 0 ) = ConvFuncs::FormatStdStr( "%i", i + 1 );
		items.Cell( i, 1 ) = ConvFuncs::FormatStdStr( "%i", sellerItems[ i ].number );
		
		items.Cell( i, 2 ) = sellerItems[ i ].name;
		
		items.Cell( i, 3 ) = ConvFuncs::FormatStdStr( "%0.2f", sellerItems[ i ].price );

		items.Cell( i, 4 ) = ConvFuncs::FormatStdStr( "%c",
			Centrala::Storage().GetSellerCategory( sellerItems[ i ].category ).letter );
		items.Cell( i, 5 ) = DataAccessInterface::ItemStatus2String( sellerItems[ i ].status );
	}

	DrawTableSplitOnPages( pdf, 10, ypos, items, columns );
}

void Recipees::AddProgramPageHeaderAndFooter( PdfWrapperBase& pdf )
{
	pdf.SetFontSize( 1 );
	
	// header
	pdf.WriteSingleText( 5, 5, 
		ConvFuncs::FormatStdStr(_("Dokument vytvorený '%s' na burze '%s'."), 
		ConvFuncsInstance->CurDateTimeString(),
		Centrala::Settings().m_name_of_the_sale.c_str()));
	
	// center free area is in vertical range "7% - 90%"
	
	//footer
	pdf.WriteSingleText( 5, 90, 
		ConvFuncs::FormatStdStr(_("Dokument vytvorený '%s' na počítači '%s'."), 
		ConvFuncsInstance->CurDateTimeString(),
		Centrala::Settings().m_computer_name.c_str()));
	pdf.WriteSingleText( 5, 92, 
		ConvFuncs::FormatStdStr(_("Program má licenciu '%s'."), 
		"Demo - zdarma"));
	pdf.WriteSingleText( 5, 94, 
		ConvFuncs::FormatStdStr(_("Program %s, verzia z dňa %s. Autor %s v spolupráci s Materským Centrom %s"), 
		"Burza",
		__DATE__,
		"Miro Jánošík",
		"Srdiečko"));
}

float Recipees::DrawTableSplitOnPages( PdfWrapperBase& pdf, float fx, float fy, TableOfCells& table, TableColumnSpecs& columns )
{
	PdfWrapperBase::TableStyle	style;
	style.fontSize = 1.3;
	style.headerRowHeight = style.fontSize * 1.5;
	style.rowHeight = style.fontSize * 1.2;

	int firstRowToPrint, rowsToPrint, lastRowToPrint;
	firstRowToPrint = 0;
	
	float yEndOfTable = -1;

	do {
		rowsToPrint = ( marginBottom - fy ) / style.rowHeight;
		lastRowToPrint = firstRowToPrint + rowsToPrint;
		if ( lastRowToPrint >= table.Rows() ) {
			lastRowToPrint = table.Rows() - 1;
		}
		
		pdf.DrawTable( fx, fy, style, table, columns, firstRowToPrint, lastRowToPrint );
		
		if ( lastRowToPrint >= table.Rows() - 1 ) {
			yEndOfTable = fy + ( lastRowToPrint - firstRowToPrint + 1 ) * style.rowHeight;
			
			// add header row on first page
			if ( firstRowToPrint == 0 ) {
				yEndOfTable += style.headerRowHeight;
			}
			break;
		}

		pdf.NewPage( pdf.GetCurrentPageTitle() );
		AddProgramPageHeaderAndFooter( pdf );
		fy = marginTop;
		firstRowToPrint = lastRowToPrint + 1;
	} while( true );

	return yEndOfTable;
}

void Recipees::PrintRecipeeAtEndOfCheckout( ItemDataArray& items, PdfWrapperBase& pdf )
{
	string errorMessage;
	
	// prepare PDF receipt
	// and print sold items to receipt
	try {
		// print page title
		string title;
		title = _( "Účet vystavený " );
		ConvFuncs::GetDateTime();
		ConvFuncs::s_datetime *dt = &ConvFuncs::datetime;
		title += ConvFuncs::FormatStdStr( "%i-%i-%i %i:%i",
			dt->year, dt->month, dt->day, dt->hour, dt->minute );
		
		pdf.NewPage( title );
		AddProgramPageHeaderAndFooter( pdf );
		
		// items in a table
		TableOfCells	table;
		TableColumnSpecs columns;
		unsigned int index;
		
		columns.AddColumn( "#", 5 );
		columns.AddColumn( _("číslo"), 10 );
		columns.AddColumn( _("názov"), 30 );
		columns.AddColumn( _("cena"), 20 );
		
		table.Resize( items.size(), 4 );
		
		SellerData seller;
		int pidx;
		string str;
		BurzaRegister	*pregister = & Centrala::Register();
		double sum = 0;
		
		ItemDataIterator it, ite;
		for( it = items.begin(), ite = items.end(), index = 0; 
				it != ite; ++it, ++index ) {
			
			try {
				seller = Centrala::Storage().GetSeller( it->seller_id );
			}
			catch(WrongSellerException) {
				throw runtime_error( "BurzaRegister::FinishCheckout: Seller not found" );
			}

			table.Cell( index, 0 ) = ConvFuncs::FormatStdStr( "%i", index + 1 );
			str = ConvFuncs::FormatStdStr( "%s-%i", seller.shortcode.c_str(), it->number );
			table.Cell( index, 1 ) = str;
			str = it->name;
			table.Cell( index, 2 ) = str;
			table.Cell( index, 3 ) = ConvFuncs::FormatStdStr( "%0.2f", it->price );

			sum += it->price;
		}
		
		index++;
		table.Resize( index+1, 4 );
		table.Cell( index, 2 ) = _( "Suma" );
		table.Cell( index, 3 ) = ConvFuncs::FormatStdStr( "%0.2f", sum );

		DrawTableSplitOnPages( pdf, 10, 20, table, columns );
		
	} catch( runtime_error& e ) {
		errorMessage = "PDF error:";
		errorMessage += e.what();
	} catch( out_of_range& e ) {
		errorMessage = "PDF out of range error";
		errorMessage += e.what();
	} catch( ... ) {
		errorMessage = "PDF error(unknown)";
	}
	
	if (!errorMessage.empty()) {
		throw runtime_error( errorMessage );
	}
}

void Recipees::PrintSellerReturnSheet( PdfWrapperBase& pdf, SELLER_ID id )
{
	SellerData seller;
	SellerCategory sellerCat;

	try {
		seller = Centrala::Storage().GetSeller( id );
		sellerCat = Centrala::Storage().GetSellerCategory( seller.category );
	}
	catch(exception& e)
	{
		ErrorHandling::HandleException(e);
	}
	
	pdf.NewPage( ConvFuncs::FormatStdStr( _("%s - %s"), seller.shortcode.c_str(), seller.name.c_str() ) );
	AddProgramPageHeaderAndFooter( pdf );
	
	pdf.SetFontSize(1.2);
	double ypos = marginTop;
	
	pdf.WriteSingleText( 10, ypos, ConvFuncs::FormatStdStr(_("Predávajúci: %s - %s (%s)"), 
		seller.shortcode.c_str(), seller.name.c_str(),
		sellerCat.name.c_str() ));
	ypos += 1.6;
	pdf.WriteSingleText( 10, ypos, ConvFuncs::FormatStdStr(_("( %s )"), seller.note.c_str() ));
	ypos += 1.6;
	pdf.WriteSingleText( 10, ypos, ConvFuncs::FormatStdStr(_("Stav dňa: %s"), ConvFuncsInstance->CurDateTimeString() ));
	ypos += 1.6;

	TableOfCells	items;
	TableColumnSpecs	columns;
	string str;

	columns.AddColumn( _("číslo"), 5 );
	columns.AddColumn( _("názov"), 40 );
	columns.AddColumn( _("cena"), 7 );
	columns.AddColumn( _("kat"), 4 );
	columns.AddColumn( _("stav"), 15 );
	columns.AddColumn( _("predane"), 7 );
	
	ItemDataArray sellerItems = Centrala::Storage().GetSellerItems( id );

	items.Resize(sellerItems.size(), columns.m_columns.size());
	double sum = 0;

	for(int i=0; i<sellerItems.size(); ++i ) {
		
		items.Cell( i, 0 ) = ConvFuncs::FormatStdStr( "%i", sellerItems[ i ].number );
		
		items.Cell( i, 1 ) = sellerItems[ i ].name;
		
		items.Cell( i, 2 ) = ConvFuncs::FormatStdStr( "%0.2f", sellerItems[ i ].price );

		items.Cell( i, 3 ) = ConvFuncs::FormatStdStr( "%c",
			Centrala::Storage().GetSellerCategory( sellerItems[ i ].category ).letter );
		items.Cell( i, 4 ) = DataAccessInterface::ItemStatus2String( sellerItems[ i ].status );

		if ( sellerItems[ i ].status == sold ) {
			items.Cell( i, 5 ) = ConvFuncs::FormatStdStr( "%0.2f", sellerItems[ i ].price );
			sum += sellerItems[ i ].price;
		}
	}

	ypos = DrawTableSplitOnPages( pdf, 10, ypos, items, columns );
	
	// position for another table is:
	ypos += 0;

	// sums
	TableOfCells	sumCells;
	TableColumnSpecs	sumColumns;
	
	sumColumns.AddColumn( "", 50 );
	sumColumns.AddColumn( "", 20 );

	// add sum of the items that were sold
	SellerFees fees = SellerFees::CalculateSellerFees( sellerCat, sellerItems );

	sumCells.Resize( 6 + fees.fees.size() * 2, sumColumns.m_columns.size() );

	int row = 0;
	sumCells.Cell( row, 0 ) = _( "Súčet predaných vecí" );
	sumCells.Cell( row, 1 ) = ConvFuncs::FormatStdStr( "%0.2f", sum );
	double finalSum = fees.sumOfSoldItems;

	for( int i = 0; i < fees.fees.size(); ++i )
	{
		++row;
		sumCells.Cell( row, 0 ) = fees.fees[ i ].name;
		sumCells.Cell( row, 1 ) = ConvFuncs::FormatStdStr( "- %0.2f", fees.fees[ i ].price );

		finalSum -= fees.fees[ i ].price;
		++row;
		sumCells.Cell( row, 1 ) = ConvFuncs::FormatStdStr( "%0.2f", finalSum );
	}

	++row;
	++row;
	sumCells.Cell( row, 0 ) = _( "Celkom vyplatené (eur) :" );
	sumCells.Cell( row, 1 ) = ConvFuncs::FormatStdStr( "%0.2f", finalSum );
	
	++row;
	++row;
	sumCells.Cell( row, 0 ) = _( "Nepredané veci a peniaze prevzal : " );

	DrawTableSplitOnPages( pdf, 15, ypos, sumCells, sumColumns );

	// nepredané veci a peniaze prevzal : 		
}

float Recipees::AdvanceYpos( float ypos, float percent )
{
	ypos += percent;
	if ( ypos >= marginBottom ) {
		
		currentPdf->NewPage( currentPdf->GetCurrentPageTitle() );
		AddProgramPageHeaderAndFooter( *currentPdf );

		ypos = marginTop;
	}

	return ypos;
}


bool SortSellerDataPtrsByShortcode(SellerData* a, SellerData *b) { 
	return (stricmp( a->shortcode.c_str(), b->shortcode.c_str()) < 0); }

string GetSellerCategoryNameById( SellerCategoryArray& categories, SELLER_CATEGORY_ID id )
{
	for( int i=0; i< categories.size(); ++i )
		if ( categories[ i ].id == id )
			return categories[ i ].name;

	return "?";
}

void Recipees::PrintStats( PdfWrapperBase& pdf )
{
	SellerDataArray sellers;
	SellerCategoryArray categories;

	try {
		sellers = Centrala::Storage().GetSellers();
		categories = Centrala::Storage().GetSellersCategories();
	}
	catch(exception& e)
	{
		ErrorHandling::HandleException(e);
	}

	currentPdf = &pdf;

	// order sellers - first by status, then by code
	vector< SellerData* >	allSorted;
	vector< SellerData* >	imported;
	vector< SellerData* >	received;
	vector< SellerData* >	handedOut;

	// sort by shortcode
	for( unsigned int i = 0; i < sellers.size(); ++i ) {
		allSorted.push_back( & sellers[ i ] );
	}

	std::sort( allSorted.begin(), allSorted.end(), SortSellerDataPtrsByShortcode );
	
	for( unsigned int i = 0; i < allSorted.size(); ++i )
	{
		switch ( allSorted[ i ]->status ) {
		case SellerStatus::imported:
			imported.push_back( allSorted[ i ] );
			break;
		case SellerStatus::receivedSelling:
			received.push_back( allSorted[ i ] );
			break;
		case SellerStatus::handedOut:
			handedOut.push_back( allSorted[ i ] );
			break;
		}
	}

	// start printing
	pdf.NewPage( _( "Zhrnutie predajcov" ) );
	AddProgramPageHeaderAndFooter( pdf );

	double ypos = marginTop;
	double fontSize = 1.2;
	double lineSpacing = fontSize + 0.4;

	pdf.SetFontSize(fontSize);

	pdf.WriteSingleText( 10, ypos, ConvFuncs::FormatStdStr(_("Počet predávajúcich dohromady: %i"), allSorted.size() ));
	ypos += lineSpacing;
	pdf.WriteSingleText( 10, ypos, ConvFuncs::FormatStdStr(_("Stav dňa: %s"), ConvFuncsInstance->CurDateTimeString() ));
	ypos += lineSpacing;

	TableOfCells	cells;
	TableColumnSpecs	columns;
	string str;

	columns.AddColumn( _("Kód"), 5 );
	columns.AddColumn( _("Meno"), 35 );
	columns.AddColumn( _("Poznámka"), 35 );
	columns.AddColumn( _("Kategória"), 10 );

	unsigned int row = 0;

	cells.Resize( 3 + allSorted.size(), columns.m_columns.size());

	// received, imported, but items are not taken in
	cells.Cell( row, 1 ).SetText( ConvFuncs::FormatStdStr(_( "Neprebratí - predajcov: %i" ), imported.size() ) );
	++row;

	for( unsigned int i = 0; i < imported.size(); ++i ) {
		cells.Cell( row, 0 ).SetText( imported[ i ]->shortcode );
		cells.Cell( row, 1 ).SetText( imported[ i ]->name );
		cells.Cell( row, 2 ).SetText( imported[ i ]->note );
		cells.Cell( row, 3 ).SetText( GetSellerCategoryNameById( categories, imported[ i ]->category ) );

		++row;
	}

	// items taken from seller
	cells.Cell( row, 1 ).SetText( ConvFuncs::FormatStdStr(_( "Tovar prebratý - predajcov: %i" ), received.size() ) );
	++row;

	for( unsigned int i = 0; i < received.size(); ++i ) {
		cells.Cell( row, 0 ).SetText( received[ i ]->shortcode );
		cells.Cell( row, 1 ).SetText( received[ i ]->name );
		cells.Cell( row, 2 ).SetText( received[ i ]->note );
		cells.Cell( row, 3 ).SetText( GetSellerCategoryNameById( categories, received[ i ]->category ) );
		++row;
	}

	// remaining items handed out back
	cells.Cell( row, 1 ).SetText( ConvFuncs::FormatStdStr(_( "Tovar vydaný späť - predajcov: %i" ), handedOut.size() ) );
	++row;

	for( unsigned int i = 0; i < handedOut.size(); ++i ) {
		cells.Cell( row, 0 ).SetText( handedOut[ i ]->shortcode );
		cells.Cell( row, 1 ).SetText( handedOut[ i ]->name );
		cells.Cell( row, 2 ).SetText( handedOut[ i ]->note );
		cells.Cell( row, 3 ).SetText( GetSellerCategoryNameById( categories, handedOut[ i ]->category ) );
		++row;
	}

	ypos = DrawTableSplitOnPages( pdf, 15, ypos, cells, columns );
	float textPosX = 15;

	ypos = AdvanceYpos( ypos, 2 * lineSpacing );
	pdf.WriteSingleText( textPosX, ypos, _( "Ďalšie štatistické údaje:" ) );

	// number of sellers, number of items, cost of items, average per seller, cost per seller
	// price of items not brought in, items on stock, items handed out, items sold, items lost
	int numberOfItems = 0;
	float costOfAllItems = 0.0;
	int numberOfItemsInStatus[ ItemStatus::ItemStatusCountWithUndefined ];
	float costOfItemsInStatus[ ItemStatus::ItemStatusCountWithUndefined ];
	float price;
	ItemStatus status;

	for( int i = 0; i < ItemStatus::ItemStatusCount; ++i ) {
		numberOfItemsInStatus[ i ] = 0;
		costOfItemsInStatus[ i ] = 0.0;
	}

	LogS( InfoMajor, "Counting up sum\n" );

	for( int s = 0; s < sellers.size(); ++s ) {

		ItemDataArray items = Centrala::Storage().GetSellerItems( sellers[ s ].id );

		for( int i = 0; i < items.size(); ++i ) {
			
			price = items[ i ].price;
			status = items[ i ].status;

			costOfAllItems += price;

			numberOfItemsInStatus[ status ]++;
			costOfItemsInStatus[ status ] += price;
		}

		numberOfItems += items.size();

		LogS( InfoMajor, "After seller %s - %s : %i\n", 
			sellers[ s ].shortcode.c_str(), sellers[ s ].name.c_str(), (int)costOfAllItems );
	}

	ypos = AdvanceYpos( ypos, lineSpacing );
	pdf.WriteSingleText( textPosX, ypos, ConvFuncs::FormatStdStr(_( "Počet predajcov: %i" ), sellers.size()) );

	ypos = AdvanceYpos( ypos, lineSpacing );
	pdf.WriteSingleText( textPosX, ypos, ConvFuncs::FormatStdStr(_( "Kusov tovaru: %i" ), numberOfItems) );

	ypos = AdvanceYpos( ypos, lineSpacing );
	pdf.WriteSingleText( textPosX, ypos, ConvFuncs::FormatStdStr(_( "Cena tovaru spolu: %0.0f %s" ), 
		costOfAllItems, Centrala::Settings().currencyInternationalShort.c_str() ) );

	ypos = AdvanceYpos( ypos, lineSpacing );
	pdf.WriteSingleText( textPosX, ypos, ConvFuncs::FormatStdStr(_( "Priemerne kusov tovaru na predajcu: %i" ), 
		numberOfItems / sellers.size() ) );
	
	ypos = AdvanceYpos( ypos, lineSpacing );
	pdf.WriteSingleText( textPosX, ypos, ConvFuncs::FormatStdStr(_( "Priemerná cena tovaru na predajcu: %0.0f %s" ), 
		costOfAllItems / sellers.size(), Centrala::Settings().currencyInternationalShort.c_str() ) );

	for( int i = 0; i < ItemStatus::ItemStatusCount; ++i ) {

		ypos = AdvanceYpos( ypos, lineSpacing );
		pdf.WriteSingleText( textPosX, ypos, ConvFuncs::FormatStdStr(_( "Kusov v '%s' : %i , cena tovaru : %0.0f %s" ), 
			DataAccessInterface::ItemStatus2String( Burza::ItemStatus( i ) ), 
			numberOfItemsInStatus[ i ],
			costOfItemsInStatus[ i ],
			Centrala::Settings().currencyInternationalShort.c_str() ) );
	}

	currentPdf = NULL;
}

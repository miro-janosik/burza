#include "recipees.h"

#include <string>
#include "table.h"
#include "burza.h"
#include "pdfwrap.h"
#include "convfncs.h"
#include "stringencoding.h"
#include "translate.h"

using namespace Burza;
using namespace BurzaTable;
using namespace std;

float Recipees::marginTop = 10;
float Recipees::marginBottom = 80;
float Recipees::marginLeft = 10;
float Recipees::marginRight = 90;

void Recipees::PrintSellerStatus( PdfWrapperBase& pdf, SELLER_ID id )
{
	SellerData seller = Centrala::Storage().GetSeller( id );
	
	pdf.NewPage( ConvFuncs::FormatStdStr( _("Predávajúci %s"), seller.name.c_str() ) );
	AddProgramPageHeaderAndFooter( pdf );
	
	pdf.SetFontSize(2);
	pdf.WriteSingleText( 10, 10, ConvFuncs::FormatStdStr(_("Predávajúci: %s - %s"), 
		seller.name.c_str(), seller.shortcode.c_str() ));
	pdf.WriteSingleText( 10, 13, ConvFuncs::FormatStdStr(_("Poznámka: %s"), seller.note.c_str() ));
	pdf.WriteSingleText( 10, 16, ConvFuncs::FormatStdStr(_("Stav dňa: %s"), ConvFuncsInstance->CurDateTimeString() ));

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

	DrawTableSplitOnPages( pdf, 10, 20, items, columns );
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

void Recipees::DrawTableSplitOnPages( PdfWrapperBase& pdf, float fx, float fy, TableOfCells& table, TableColumnSpecs& columns )
{
	PdfWrapperBase::TableStyle	style;
	style.fontSize = 1.3;
	style.headerRowHeight = style.fontSize * 1.5;
	style.rowHeight = style.fontSize * 1.2;

	int firstRowToPrint, rowsToPrint, lastRowToPrint;
	firstRowToPrint = 0;
	
	do {
		rowsToPrint = ( marginBottom - fy ) / style.rowHeight;
		lastRowToPrint = firstRowToPrint + rowsToPrint;
		if ( lastRowToPrint >= table.Rows() ) {
			lastRowToPrint = table.Rows() - 1;
		}
		
		pdf.DrawTable( fx, fy, style, table, columns, firstRowToPrint, lastRowToPrint );
		
		if ( lastRowToPrint >= table.Rows() - 1 ) {
			break;
		}

		pdf.NewPage( pdf.GetCurrentPageTitle() );
		AddProgramPageHeaderAndFooter( pdf );
		fy = marginTop;
		firstRowToPrint = lastRowToPrint + 1;
	} while( true );
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
		
		//StringEncoding::UTF8toISO8859_2( title );
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
		}
		
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
	PrintSellerStatus( pdf, id );
}

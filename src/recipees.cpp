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

void Recipees::PrintSellerStatus( PdfWrapperBase& pdf, SELLER_ID id )
{
	SellerData seller = Centrala::Storage().GetSeller( id );
	
	pdf.NewPage( ConvFuncs::FormatStdStr( _("Predávajúci %s"), seller.name.c_str() ) );
	AddProgramPageFooter( pdf );
	pdf.WriteSingleText( 10, 20, ConvFuncs::FormatStdStr(_("Predávajúci %s - %s"), 
		seller.name.c_str(), seller.shortcode.c_str() ));
	pdf.WriteSingleText( 10, 40, ConvFuncs::FormatStdStr(_("Poznámka %s"), seller.note.c_str() ));
	pdf.WriteSingleText( 10, 40, ConvFuncs::FormatStdStr(_("Stav dňa %s"), ConvFuncsInstance->CurDateTimeString() ));

	TableOfCells	items;
	TableColumnSpecs	columns;
	string str;

	columns.AddColumn( _("číslo"), 60 );
	columns.AddColumn( _("názov"), 200 );
	columns.AddColumn( _("cena"), 60 );
	
	ItemDataArray sellerItems = Centrala::Storage().GetSellerItems( seller.id );
	
	items.Resize(sellerItems.size(), columns.m_columns.size());
	
	for(int i=0; i<sellerItems.size(); ++i ) {
		items.Cell(i, 0) = sellerItems[ i ].name;
		
		items.Cell( i, 0 ) = ConvFuncs::FormatStdStr( "%i", i + 1 );
		
		str = sellerItems[ i ].name;
		StringEncoding::UTF8toISO8859_2( str );
		items.Cell( i, 1 ) = str;

		items.Cell( i, 2 ) = ConvFuncs::FormatStdStr( "%0.2f", sellerItems[ i ].price );
	}

	pdf.DrawTable( 10, 60, items, columns );
}

void Recipees::AddProgramPageFooter( PdfWrapperBase& pdf )
{
	pdf.WriteSingleText( 10, 200, 
		ConvFuncs::FormatStdStr(_("Program %s produkované dňa %s. Autor %s v spolupráci s Materským Centrom %"), 
		"Burza",
		ConvFuncsInstance->CurDateTimeString(),
		"Miro Jánošík",
		"Srdiečko"));
	
	pdf.WriteSingleText(10, 220, 
		ConvFuncs::FormatStdStr(_("Program ma licenciu '%s'."), 
		"Demo - zdarma"));
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
		
		StringEncoding::UTF8toISO8859_2( title );
		pdf.NewPage( title );
		AddProgramPageFooter( pdf );
		
		// items in a table
		TableOfCells	table;
		TableColumnSpecs columns;
		unsigned int index;
		
		columns.AddColumn( "#", 40 );
		columns.AddColumn( _("číslo"), 60 );
		columns.AddColumn( _("názov"), 200 );
		columns.AddColumn( _("cena"), 60 );
		
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
			StringEncoding::UTF8toISO8859_2( str );
			table.Cell( index, 1 ) = str;
			str = it->name;
			StringEncoding::UTF8toISO8859_2( str );
			table.Cell( index, 2 ) = str;
			table.Cell( index, 3 ) = ConvFuncs::FormatStdStr( "%0.2f", it->price );
		}
		
		pdf.DrawTable( 50, 50, table, columns );
		
		// pocet poloziek: suma:
		pdf.ClosePDF();
		
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

void Recipees::PrintProductsToPDF( PdfWrapperBase& pdf, SellerData& seller )
{
	string str;
	
	str = seller.shortcode;
	str += " - ";
	str += seller.name;
	
	StringEncoding::UTF8toISO8859_2( str );
	pdf.WriteSingleText( 10, 10, str.c_str() );
	
	TableOfCells	table;
	TableColumnSpecs columns;
	
	columns.AddColumn( "#", 40 );
	columns.AddColumn( _("nazov"), 200 );
	columns.AddColumn( _("cena"), 60 );
	columns.AddColumn( _("kat"), 40 );
	columns.AddColumn( _("stav"), 90 );
	
	BurzaRegister* reg = & Centrala::Register();	
	
	unsigned int i, c;
	c = 5 + ( rand() % 70 );
	table.Resize( c, 5 );
	
	ItemDataArray items = Centrala::Storage().GetSellerItems( seller.id );
	
	for( unsigned int j=0; j<c; ++j ) {
		
		i = j % items.size();
		
		table.Cell( j, 0 ) = ConvFuncs::FormatStdStr( "%i", items[ i ].number );
		str = items[ i ].name;
		StringEncoding::UTF8toISO8859_2( str );
		table.Cell( j, 1 ) = str;
		table.Cell( j, 2 ) = ConvFuncs::FormatStdStr( "%0.2f", items[ i ].price );
		table.Cell( j, 3 ) = ConvFuncs::FormatStdStr( "%c",
			Centrala::Storage().GetItemCategory( items[ i ].category ).letter );
		table.Cell( j, 4 ) = DataAccessInterface::ItemStatus2String( items[ i ].status );
	}
	
	pdf.DrawTable( 50, 50, table, columns );
}


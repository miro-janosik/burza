#include "recipees.h"

#include <string>
#include "table.h"
#include "burza.h"
#include "pdfwrap.h"
#include "convfncs.h"
#include "stringencoding.h"

using namespace Burza;
using namespace BurzaTable;
using namespace std;

void Recipees::PrintSellerStatus( PdfWrapperBase& pdf, Seller& seller )
{
	pdf.NewPage( ConvFuncs::FormatStdStr( _("Predávajúci %s"), seller.m_meno.c_str() ) );
	AddProgramPageFooter( pdf );
	pdf.WriteSingleText( 10, 20, ConvFuncs::FormatStdStr(_("Predávajúci %s - %s"), 
		seller.m_meno.c_str(), seller.m_kod.c_str() ));
	pdf.WriteSingleText( 10, 40, ConvFuncs::FormatStdStr(_("Poznámka %s"), seller.m_poznamka.c_str() ));
	pdf.WriteSingleText( 10, 40, ConvFuncs::FormatStdStr(_("Stav dňa %s"), ConvFuncsInstance->CurDateTimeString() ));

	TableOfCells	items;
	TableColumnSpecs	columns;
	string str;

	columns.AddColumn( _("číslo"), 60 );
	columns.AddColumn( _("názov"), 200 );
	columns.AddColumn( _("cena"), 60 );
	
	items.Resize(seller.m_products.size(), columns.m_columns.size());
	
	for(int i=0; i<seller.m_products.size(); ++i ) {
		items.Cell(i, 0) = seller.m_products[ i ].m_nazov;
		
		items.Cell( i, 0 ) = ConvFuncs::FormatStdStr( "%i", i + 1 );
		
		str = seller.m_products[ i ].m_nazov;
		StringEncoding::UTF8toISO8859_2( str );
		items.Cell( i, 1 ) = str;

		items.Cell( i, 2 ) = ConvFuncs::FormatStdStr( "%0.2f", seller.m_products[ i ].m_cena );
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

void Recipees::PrintRecipeeAtEndOfCheckout( BurzaRegister::t_sellerProducts& items, PdfWrapperBase& pdf )
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
		
		Seller* s;
		int pidx;
		string str;
		BurzaRegister	*pregister = Centrala::Instancia()->Register();
		
		BurzaRegister::t_sellerProducts::iterator it, ite;
		for( it = items.begin(), ite = items.end(), index = 0; 
				it != ite; ++it, ++index ) {
			
			s = pregister->NajdiPredajcuPodlaKoduPtr( it->sellerCode.c_str() );
			if ( s == NULL ) {
				throw runtime_error( "BurzaRegister::FinishCheckout: Seller not found" );
			}
			pidx = s->NajdiTovarIndex( it->productNumber );
			if ( pidx == -1 ) {
				throw runtime_error( "BurzaRegister::FinishCheckout: Item not found" );
			}
			
			table.Cell( index, 0 ) = ConvFuncs::FormatStdStr( "%i", index + 1 );
			str = ConvFuncs::FormatStdStr( "%s-%i", it->sellerCode.c_str(), it->productNumber );
			StringEncoding::UTF8toISO8859_2( str );
			table.Cell( index, 1 ) = str;
			str = s->m_products[ pidx ].m_nazov;
			StringEncoding::UTF8toISO8859_2( str );
			table.Cell( index, 2 ) = str;
			table.Cell( index, 3 ) = ConvFuncs::FormatStdStr( "%0.2f", s->m_products[ pidx ].m_cena );
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

void ::PrintProductsToPDF( PdfWrapperBase& pdf, SellerData& seller )
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
	
	BurzaRegister* reg = Centrala::Instancia()->Register();	
	
	unsigned int i, c;
	c = 5 + ( rand() % 70 );
	table.Resize( c, 5 );
	
	for( unsigned int j=0; j<c; ++j ) {
		
		i = j % m_products.size();
		
		table.Cell( j, 0 ) = ConvFuncs::FormatStdStr( "%i", m_products[ i ].m_cislo );
		str = m_products[ i ].m_nazov;
		StringEncoding::UTF8toISO8859_2( str );
		table.Cell( j, 1 ) = str;
		table.Cell( j, 2 ) = ConvFuncs::FormatStdStr( "%0.2f", m_products[ i ].m_cena );
		table.Cell( j, 3 ) = ConvFuncs::FormatStdStr( "%c", m_products[ i ].m_kategoria );
		table.Cell( j, 4 ) = reg->Stav2String( m_products[ i ].m_stav );
	}
	
	pdf.DrawTable( 50, 50, table, columns );
}


#ifndef RECIPEES_H
#define RECIPEES_H

#include "burza.h"
#include "pdfwrap.h"

namespace Burza {
  
class Recipees
{
public:
  void PrintSellerStatus( PdfWrapperBase& pdf, SELLER_ID id );

  void PrintRecipeeAtEndOfCheckout( ItemDataArray& items, PdfWrapperBase& pdf );
  
  void PrintSellerReturnSheet( PdfWrapperBase& pdf, SELLER_ID id );

  void PrintSummaryOfSellers( PdfWrapperBase& pdf );

private:

  void AddProgramPageHeaderAndFooter( PdfWrapperBase& pdf );

  //! returns Y position on page in percent, where table ended.
  float DrawTableSplitOnPages( PdfWrapperBase& pdf, float fx, float fy, TableOfCells& table, TableColumnSpecs& columns );
 
  static double RoundTo5Cents( double price );

private:
	//! margin in percentage of page. 0 means no margin
	static float marginTop;
	static float marginLeft;

	//! margin in percentage of page. 100 means no margin
	static float marginBottom;
	static float marginRight;
};

};

#endif // RECIPEES_H

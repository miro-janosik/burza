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

  static void PrintStats( PdfWrapperBase& pdf );

private:

  static void AddProgramPageHeaderAndFooter( PdfWrapperBase& pdf );

  //! returns Y position on page in percent, where table ended.
  float static DrawTableSplitOnPages( PdfWrapperBase& pdf, float fx, float fy, TableOfCells& table, TableColumnSpecs& columns );
 
  //! Takes Y position, advances it by some percents (down), returns the position.
  //! If position would be over bottom margin, new page is created and Y is moved
  //! to start of page.
  static float AdvanceYpos( float ypos, float percent );

private:
	//! margin in percentage of page. 0 means no margin
	static float marginTop;
	static float marginLeft;

	//! margin in percentage of page. 100 means no margin
	static float marginBottom;
	static float marginRight;

	static PdfWrapperBase *currentPdf;
};

};

#endif // RECIPEES_H

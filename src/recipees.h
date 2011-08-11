#ifndef RECIPEES_H
#define RECIPEES_H

#include "burza.h"
#include "pdfwrap.h"

namespace Burza {
  
class Recipees
{
public:
  void PrintSellerStatus( PdfWrapperBase& pdf, SELLER_ID id );

  void AddProgramPageFooter( PdfWrapperBase& pdf );

  void PrintRecipeeAtEndOfCheckout( ItemDataArray& items, PdfWrapperBase& pdf );

  void PrintProductsToPDF( PdfWrapperBase& pdf, SellerData& seller );
  
};

};

#endif // RECIPEES_H

#ifndef RECIPEES_H
#define RECIPEES_H

#include "burza.h"
#include "pdfwrap.h"

namespace Burza {
  
class Recipees
{
public:
  void PrintSellerStatus( PdfWrapperBase& pdf, Seller& seller );

  void AddProgramPageFooter( PdfWrapperBase& pdf );

  void PrintRecipeeAtEndOfCheckout( BurzaRegister::t_sellerProducts& items, PdfWrapperBase& pdf );
  
};

};

#endif // RECIPEES_H

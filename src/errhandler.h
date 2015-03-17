#ifndef _error_handling_h_
#define _error_handling_h_

#include <exception>

namespace Burza {

class ErrorHandling {
public:
	
  //! handles the exception. If exception can't be handled it will be thrown again
  static void HandlePostgresException(const std::exception& e);
  
  //! handles the exception. If exception can't be handled it will be thrown again
  static void HandleException(const std::exception& e);
  
  //! Logs and displays message box.
  //! returns false if not handled.
  static bool HandlePdfExceptions(const std::exception& e );

} ;

}

#endif
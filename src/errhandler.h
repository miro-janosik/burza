#ifndef _error_handling_h_
#define _error_handling_h_

#include <exception>

namespace Burza {

class ErrorHandling {
public:
	
  //! handles the exception. If exception can't be handled it will be thrown again
  static void HandlePostgresException(std::exception& e);
  
  //! handles the exception. If exception can't be handled it will be thrown again
  static void HandleException(std::exception& e);
  
} ;

}

#endif
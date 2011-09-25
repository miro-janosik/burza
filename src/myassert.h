#ifndef _myassert_h_
#define _myassert_h_

// assert( false ) messes things up from time I switched to Gtkmm in MSVC. When it asserts
// it jumps somewhere in Log() code, messes up stack and dunno what happened.
// assert needs to be replaced with something that shows place of problems.

//#include <assert.h>

#include "log.h"

// print out log message, try to initiate 'breakpoint' by writing to invalid memory
#ifdef _DEBUG
	#define assert( a ) {  if ( ! ( a ) ) { LogS( InfoError, "!!! ASSERT at line %i in %s\n", __LINE__, __FILE__ ); int* p = NULL; *p = 666; exit( 0 ); } }
#else
	#define assert( a )
#endif

/*
inline void assert( bool a )
{ 
	if ( ! ( a ) ) { 
		// TRACE2( " ASSERT at line %s in %s\n", __LINE__, __FILE__ );
		LogS( InfoError, "!!! ASSERT at line %s in %s\n", __LINE__, __FILE__ ); 
		exit( 0 ); 
	} 
}

*/

#endif

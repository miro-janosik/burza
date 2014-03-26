#ifndef _convfiletypes_h_
#define _convfiletypes_h_

namespace ConvFile {
	
#ifndef uint32_t

#ifdef _MSC_VER
	// MS Visual Studio is missing these types
	typedef unsigned long uint32_t;
	typedef long int32_t;
	typedef unsigned short uint16_t;
	typedef short int16_t;
	typedef unsigned char uint8_t;
	typedef char int8_t;
#else
	// Use the C99 official header
	#include <stdint.h>
#endif

	// additionally define missing types.
	typedef float float32_t;
	typedef double float64_t;
#endif

}

#endif

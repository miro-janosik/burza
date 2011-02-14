#pragma once

#ifndef _ConvFile_h_
#define _ConvFile_h_

/*  CConvFile class for disk file or temporary memory file in-one manipulation
*  it is probably something like CFile and CMemFile from MFC
*  anyone is free to use it, and some notice would be nice.
*  created by Mirex. Check out my homepage at http://mirex.mypage.sk
*  version 1.0, - 19.january 2004
*  version 2.0
*  version 3.0  - 20.april 2006
    EXTENSION 1:
	Support for reading and writing of numbers in Little Endian and Big Endian 
	format depending on the current machine settings.
*	version 4.0 - 16.june 2008
	added possibility to throw on read error. Throws only if [throws_on_error] is true
	Set it by SetThrowOnError().
	Thrown item is [CConvFile::e_error_code], value is from enum [e_error_code]
	
*/

#include <stdio.h>

/*!	Basic class. It supports working with FILE* files and temporary memory space
	Supports all basic things a file should have - reading and writing of bytes,
	seeking, reading and writing strings.
*/

class CConvFileBits;

class CConvFile {

	friend class CConvFileBits;
	
private:
	// it is forbidden to copy the file this way. Use CopyFrom instead.
	CConvFile( const CConvFile & copy );

public:

	CConvFile() { opened = o_closed; throws_on_error = 0; }
	virtual ~CConvFile() { Close(); }

	// if function fails see variable GetErrorCode() for reason

	enum e_filemode_rw {
		fm_read,
		fm_write,
		fm_read_write,
	} ;

	enum e_filemode_binary {
		fm_binary,
		fm_text,
	} ;

	// mode: fm_binary / fm_text,
	// read: fm_read / fm_write / fm_read_write
	// returns 1 on success, 0 on fail
	int OpenFile( const char *filename, e_filemode_binary mode, e_filemode_rw readwrite );

	// shortcut for opening mostly used binary files
	int OpenFile( const char *filename, e_filemode_rw readwrite )
		{ return OpenFile( filename, fm_binary, readwrite ); }
	
	// read: fm_read / fm_write / fm_read_write
	// returns 1 on success, 0 on fail
	// creates own memory buffer, copies data from *memory
	int OpenMemory( void* memory, unsigned long size, e_filemode_rw read );

	// close
	// returns 1 on success, 0 on fail
	int Close( void );
	
	enum e_seek {
		start_of_file,
		cur_pos,
		end_of_file,
	} ;

	// parameter [from] should contain
	//   ::start_of_file, ::cur_pos  or ::end_of_file, values from enum [e_seek] 
	// returns 1 on success, 0 on fail
	int Seek( long pos, e_seek from );

	// returns file position from start of file, returns 0 on error
	unsigned long Tell( void );

	// check if positioned at end of file
	// returns 0 if not end of file, 1 if end of file, -1 on fail
	int Eof( void );

	// returns file size in bytes, 0 on error
	unsigned long FileSize( void );

	// reading from file, returns size of data read, 0 on error
	int Read( unsigned long size, void* ptr );

	// reads text from current position to the first end of line
	// or to the end of the buffer supplied, which comes first
	// buffer is filled with the text including the end of line characters
	//@returns: NULL on error, pointer to buffer parameter on success
	char* gets( char* buffer, int buffer_size );

	// writing to binary file, returns number of bytes written, 0 on error
	int Write( unsigned long size, const void* ptr );

	// write text string, returns number of chars written, 0 on error
	int Write( const char* text );

	// write formated text string, as through printf ... max string length is [MaxPrintStrLen] chars
	// returns number of characters written, 0 on error
	int Print( const char* format, ... );

	// returns 1 if opened, 0 if closed
	int IsOpened( void );

	//returns o_closed, o_file or o_memory
	int OpenedType( void );

	//returns fm_binary or fm_text, -1 if file is not opened
	int OpenedMode( void );

	//returns fm_read, fm_write or fm_read_write
	// -1 if file is not opened
	int OpenedForReading( void );

	// returns true if open in fm_read or fm_read_write mode
	// returns false otherwise, or if file is closed
	bool OpenedReadable( void );

	// returns true if open in fm_write or fm_read_write mode
	// returns false otherwise, or if file is closed
	bool OpenedWriteable( void );

	// copies contents from other file. return 1=succes, 0=failure
	int CopyFrom( CConvFile& input );

	// grows file to [size] bytes
	// returns 1 on success, 0 on fail
	int ResizeFile( unsigned long size );

	// returns current error code, value from [e_error_code]
	int	GetErrorCode( void ) { return error_code; }

	//! returns string of current error code, string is from [m_error_code_strings]
	const char* GetErrorCodeString( void ) { return m_error_code_strings[ error_code ]; }

	void SetThrowOnError( bool enabled ) { throws_on_error = enabled; }
	
	enum {
		MaxPrintStrLen = 8192,
	} ;

	enum e_opened {
		o_closed,
		o_file,
		o_memory,
	} ;

	enum e_error_code {
		e_no_error,
		e_no_file,
		e_bad_parameter,
		e_bad_parameter1,
		e_bad_parameter2,
		e_bad_parameter3,
		e_bad_parameter4,
		e_no_memory,
		e_not_opened,
		e_file_error,
		e_out_of_file,
		e_could_not_read,
		e_not_opened_for_writing,
		e_media_error,

		e_error_code_count
	} ;

	static char	m_error_code_strings[ e_error_code_count ][ 40 ];

private:

	//! opened file handle
	FILE			*f;

	//! opened memory handle
	union {
		void			*mem;
		unsigned char	*mem_uch;
		char			*mem_ch;
	} ;

		
	int				opened;			// e_opened
	e_filemode_binary	filemode;		// fm_binary or fm_text
	e_filemode_rw	reading;		// fm_read or fm_write or fm_read_write
	unsigned long	filesize;
	unsigned long	filepos;

	int				throws_on_error; // to throw or not to throw

	//error messages; one value of e_error_code, use GetErrorCode()
	e_error_code	error_code;


} ;
// end of CConvFile class


/*! === CConvFile EXTENSION 1 ===
	CConvFileWithEndianSwitchSupport

	Support for reading and writing of numbers in Little Endian and Big Endian 
	format depending on the current machine settings.
	Define either CONVFILE_PLATFORM_LE (if you're on Little endian platform) 
	or CONVFILE_PLATFORM_BE (if you're on Big endian platform) to enable 
	these functions. 

	If you won't enable either data will be written in plain form.
	warning1: you can create a instance of CConvFileWithEndianSwitchSupport with 
		CConvFile as parameter, but then be cautious, because after that both 
		streams share same file or memory so the usage should be carefull.
		Especially writing to the stream should be used only in one stream.
		Maybe in next version this action will cause disabling of original 
		CConvFile instance.
	conclusion: original file is closed when a copy is made this way
*/


#ifndef uint32_t

#ifdef _MSC_VER
	// MSVC is missing these types
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

	typedef float float32_t;
	typedef double float64_t;
#endif



class CConvFileWithEndianSwitchSupport : virtual public CConvFile {

private:
	CConvFileWithEndianSwitchSupport( const CConvFileWithEndianSwitchSupport& copy );

public:
	CConvFileWithEndianSwitchSupport () : CConvFile() {
		file_endian_type = fet_undefined;
	}

	virtual ~CConvFileWithEndianSwitchSupport() {
	}

public:

	// macros for swapping on read/write
	#define ConvFileWriteDataSwapEndian( size, source )\
		{\
		unsigned char	*ucsource = (unsigned char*) &source; \
		int	si;\
		for( si=0; si<size; si++ ) \
			Write( 1, &ucsource[ size - si - 1 ] );\
		}

	#define ConvFileReadDataSwapEndian( size, source )\
		{\
		unsigned char	*ucsource = (unsigned char*) &source; \
		int	si;\
		for( si=0; si<size; si++ ) \
			Read( 1, &ucsource[ size - si - 1 ] );\
		}


	// macros depending on the platform definition
	#if defined(CONVFILE_PLATFORM_LE)

	#ifndef CONVFILE_MSG1
	#define CONVFILE_MSG1

	#ifdef _MSC_VER
		#pragma message( "CConvFile with Endian support is compiled as for LE hardware" )
	#else
		#warning CConvFile with Endian support is compiled as for LE hardware
	#endif

	#endif

		#define ConvFileWriteDataLE( size, source ) Write( size, &source )
		#define ConvFileReadDataLE( size, source ) Read( size, &source )
		#define ConvFileWriteDataBE( size, source ) ConvFileWriteDataSwapEndian( size, source )
		#define ConvFileReadDataBE( size, source ) ConvFileReadDataSwapEndian( size, source )

	#elif defined(CONVFILE_PLATFORM_BE)

	#ifndef CONVFILE_MSG1
	#define CONVFILE_MSG1

	#ifdef _MSC_VER
		#pragma message( "CConvFile with Endian support is compiled as for BE hardware" )
	#else
		#warning CConvFile with Endian support is compiled as for BE hardware
	#endif
	
	#endif

		#define ConvFileWriteDataLE( size, source ) ConvFileWriteDataSwapEndian( size, source )
		#define ConvFileReadDataLE( size, source ) ConvFileReadDataSwapEndian( size, source )
		#define ConvFileWriteDataBE( size, source ) Write( size, &source )
		#define ConvFileReadDataBE( size, source ) Read( size, &source )

	#else

	#ifdef _MSC_VER
		#pragma message( "CConvFile with Endian support is disabled, no CONVFILE_PLATFORM_BE or CONVFILE_PLATFORM_LE is defined" )
	#else
		#warning CConvFile with Endian support is disabled, no CONVFILE_PLATFORM_BE or CONVFILE_PLATFORM_LE is defined
	#endif

		// if no platform is defined. no switching is performed
		#define ConvFileWriteDataLE( size, source ) Write( size, &source )
		#define ConvFileReadDataLE( size, source ) Read( size, &source )
		#define ConvFileWriteDataBE( size, source ) Write( size, &source )
		#define ConvFileReadDataBE( size, source ) Read( size, &source )
		
	#endif

	// Little Endian functions

	void WriteLeInt32( int32_t v ) { ConvFileWriteDataLE( 4, v ); }
	void WriteLeInt16( int16_t v ) { ConvFileWriteDataLE( 2, v ); }
	void WriteLeUint32( uint32_t v ) { ConvFileWriteDataLE( 4, v ); }
	void WriteLeUint16( uint16_t v ) { ConvFileWriteDataLE( 2, v ); }
	void WriteLeFloat32( float32_t v ) { ConvFileWriteDataLE( 4, v ); }
	void WriteLeFloat64( float64_t v ) { ConvFileWriteDataLE( 8, v ); }
	
	int32_t   ReadLeInt32( void ) { int32_t v; ConvFileReadDataLE( 4, v ); return v; }
	int16_t   ReadLeInt16( void ) { int16_t v; ConvFileReadDataLE( 2, v ); return v; }
	uint32_t  ReadLeUint32( void ) { uint32_t v; ConvFileReadDataLE( 4, v ); return v; }
	uint16_t  ReadLeUint16( void ) { uint16_t v; ConvFileReadDataLE( 2, v ); return v; }
	float32_t ReadLeFloat32( void ) { float32_t v; ConvFileReadDataLE( 4, v ); return v; }
	float64_t ReadLeFloat64( void ) { float64_t v; ConvFileReadDataLE( 8, v ); return v; }

	// Big Endian functions

	void WriteBeInt32( int32_t v ) { ConvFileWriteDataBE( 4, v ); }
	void WriteBeInt16( int16_t v ) { ConvFileWriteDataBE( 2, v ); }
	void WriteBeUint32( uint32_t v ) { ConvFileWriteDataBE( 4, v ); }
	void WriteBeUint16( uint16_t v ) { ConvFileWriteDataBE( 2, v ); }
	void WriteBeFloat32( float32_t v ) { ConvFileWriteDataBE( 4, v ); }
	void WriteBeFloat64( float64_t v ) { ConvFileWriteDataBE( 8, v ); }
	
	int32_t   ReadBeInt32( void ) { int32_t v; ConvFileReadDataBE( 4, v ); return v; }
	int16_t   ReadBeInt16( void ) { int16_t v; ConvFileReadDataBE( 2, v ); return v; }
	uint32_t  ReadBeUint32( void ) { uint32_t v; ConvFileReadDataBE( 4, v ); return v; }
	uint16_t  ReadBeUint16( void ) { uint16_t v; ConvFileReadDataBE( 2, v ); return v; }
	float32_t ReadBeFloat32( void ) { float32_t v; ConvFileReadDataBE( 4, v ); return v; }
	float64_t ReadBeFloat64( void ) { float64_t v; ConvFileReadDataBE( 8, v ); return v; }


public:

	// you can also predefine the file's endian type by calling function
	// SetFileEndianType() and use simplified Read/Write functions below
	// these functions will determine whether data should be swapped or not

	enum e_file_endian_type {
		fet_undefined,
		fet_little_endian,
		fet_big_endian,
	} ;

	void SetFileEndianType( e_file_endian_type type ) 
		{ file_endian_type = type; }


	// macros taking care of this
	#if defined(CONVFILE_PLATFORM_LE)

		#define ConvFileWriteDataFileDependant( size, source ) {\
			if ( file_endian_type == fet_big_endian )\
				ConvFileWriteDataSwapEndian( size, source )\
			else\
				Write( size, &source ); }
		
		#define ConvFileReadDataFileDependant( size, source ) {\
			if ( file_endian_type == fet_big_endian )\
				ConvFileReadDataSwapEndian( size, source )\
			else\
				Read( size, &source ); }

	#elif defined(CONVFILE_PLATFORM_BE)

		#define ConvFileWriteDataFileDependant( size, source ) {\
			if ( file_endian_type == fet_little_endian )\
				ConvFileWriteDataSwapEndian( size, source )\
			else\
				Write( size, &source ); }
		
		#define ConvFileReadDataFileDependant( size, source ) {\
			if ( file_endian_type == fet_little_endian )\
				ConvFileReadDataSwapEndian( size, source )\
			else\
				Read( size, &source ); }
				
	#else

		#define ConvFileWriteDataFileDependant( size, source )\
			Write( size, &source );
		
		#define ConvFileReadDataFileDependant( size, source )\
			Read( size, &source );

	#endif

	void WriteInt32( int32_t v ) { ConvFileWriteDataFileDependant( 4, v ); }
	void WriteInt16( int16_t v ) { ConvFileWriteDataFileDependant( 2, v ); }
	void WriteInt8( int8_t v )   { Write( 1, &v ); }
	void WriteUint32( uint32_t v ) { ConvFileWriteDataFileDependant( 4, v ); }
	void WriteUint16( uint16_t v ) { ConvFileWriteDataFileDependant( 2, v ); }
	void WriteUint8( uint8_t v )   { Write( 1, &v ); }
	void WriteFloat32( float32_t v ) { ConvFileWriteDataFileDependant( 4, v ); }
	void WriteFloat64( float64_t v ) { ConvFileWriteDataFileDependant( 8, v ); }

	void ReadInt32( int32_t &v ) { ConvFileReadDataFileDependant( 4, v ); }
	void ReadInt16( int16_t &v ) { ConvFileReadDataFileDependant( 2, v ); }
	void ReadInt8( int8_t &v )  { Read( 1, &v ); }
	void ReadUint32( uint32_t &v ) { ConvFileReadDataFileDependant( 4, v ); }
	void ReadUint16( uint16_t &v ) { ConvFileReadDataFileDependant( 2, v ); }
	void ReadUint8( uint8_t &v )  { Read( 1, &v ); }
	void ReadFloat32( float32_t &v ) { ConvFileReadDataFileDependant( 4, v ); }
	void ReadFloat64( float64_t &v ) { ConvFileReadDataFileDependant( 8, v ); }
	
	int32_t   ReadInt32( void ) { int32_t v; ConvFileReadDataFileDependant( 4, v ); return v; }
	int16_t   ReadInt16( void ) { int16_t v; ConvFileReadDataFileDependant( 2, v ); return v; }
	int8_t    ReadInt8( void )  { int8_t v; Read( 1, &v ); return v; }
	uint32_t  ReadUint32( void ) { uint32_t v; ConvFileReadDataFileDependant( 4, v ); return v; }
	uint16_t  ReadUint16( void ) { uint16_t v; ConvFileReadDataFileDependant( 2, v ); return v; }
	uint8_t	  ReadUint8( void )  { uint8_t v; Read( 1, &v ); return v; }
	float32_t ReadFloat32( void ) { float32_t v; ConvFileReadDataFileDependant( 4, v ); return v; }
	float64_t ReadFloat64( void ) { float64_t v; ConvFileReadDataFileDependant( 8, v ); return v; }

private:
	int file_endian_type;

} ;
// end of CConvFileWithEndianSwitchSupport class

/*! === CConvFile EXTENSION 2 ===
	CConvFileWideString
	This one adds wideString reading support

*/

class CConvFileWideString : virtual public CConvFile {
private:
	CConvFileWideString( const CConvFileWideString& copy );

public:
	CConvFileWideString () : CConvFile() {
	}

	// get one text line until eol (with eol character)
	// or get how much fits into buffer
	// return: number of characters read
	int GetsWidestringIntoChar( char* buffer, int buffer_size ) {

		char	chars[ 2 ];
		int		buf_pos = 0;

		buffer_size -= 2;

		while( 1 ) {

			if ( buf_pos >= buffer_size )
				break;

			if ( Read( 2, chars ) < 2 )
				break;

			if ( chars[ 0 ] == '\n' ) {
				buffer[ buf_pos++ ] = chars[ 0 ];
				break;
			}
			if ( chars[ 0 ] == '\r' )
				continue;

			buffer[ buf_pos++ ] = chars[ 0 ];

		} ;

		buffer[ buf_pos++ ] = 0;

		return buf_pos - 1;
	}

} ;
// CConvFileWideString


/*! === CConvFile EXTENSION 3 ===
	CConvFileBits
	Support for reading the file as stream of bits
	You can read also byte-data, but it does not have to be
	aligned to byte boundaries.
*/

// class for reading bits from CConvFile
class CConvFileBits : virtual public CConvFile {
public:

	// Seeks file. 
	//@offset: offset in bytes
	//@bit: offset in bits <0-8>
	void SeekBits( long offset, long bit ) {
		Seek( offset, CConvFile::start_of_file );
		bitposition = bit;
	}

	// returns file position
	//@offset: offset in bytes
	//@bit: offset in bits <0-8>
	void TellBits( long &offset, long &bit ) {
		offset = Tell();
		bit = bitposition;
	}

	// reads data from current file position
	//@count: number of bits to read. Number from 1 to 64.
	//@return: 1 on success, 0 on failure. See GetErrorCode() on failure
	int ReadBits( unsigned int count, void* buffer ) {
		unsigned long	bytes, restbits;
		unsigned char	tmpbuffer[ 5 ];
		char			bufp, countp, outbufp;
		int				position = Tell();

		// check inputs
		if (( count < 1 ) || ( count > 64 )) {
			error_code = e_bad_parameter1;
			return 0;
		}

		// calculate byte size
		bytes = bitposition + count;
		bytes = bytes / 8 + 1;

		restbits = 8 - (unsigned char)bitposition;

		// read bytes
		if ( Read( bytes, tmpbuffer ) == 0 )
			return 0;
		
		// transform into bits
		bufp = outbufp = 0;
		countp = count;
		
		while ( countp > 0 ) {

			// combine bits on edge into one
			((unsigned char*)buffer)[ outbufp ] = 
				( tmpbuffer[ bufp ] >> bitposition ) + 
				( tmpbuffer[ bufp+1 ] << restbits ); 
				
			// clear rest if its last byte and its not whole 8 bits
			if ( countp < 8 ) {
				((unsigned char*)buffer)[ outbufp ] <<= ( 8 - countp );
				((unsigned char*)buffer)[ outbufp ] >>= ( 8 - countp );
			}
			
			countp -= 8;
			outbufp++;
			bufp++;
		} ;

		// move file positions
		bitposition += count;
		while( bitposition > 7 ) {
			position ++;
			bitposition -= 8;
		}
		Seek( position, CConvFile::start_of_file );
		
		return 1;
	}

	// moves file position to the nearest byte boundary
	int AlignPosToByte( void ) {
		if ( bitposition == 0 )
			return 1;

		Seek( 1, CConvFile::cur_pos );
		bitposition = 0;
		return 1;
	}

private:
	long	bitposition;
} ;


/*! All classes combined into single class, which contains them all
	use this class if you're not sure which of the extensions you need
*/

class CConvFileCombined : 
	virtual public CConvFileWithEndianSwitchSupport, 
	virtual public CConvFileWideString,
	virtual public CConvFileBits {

} ;

#endif

#include "convfile.h"

#include <memory.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

//needed in unix
#include <malloc.h>

/*
#ifndef true
	#define true 1
#endif
#ifndef false
	#define false 0
#endif
*/

#ifdef _MSC_VER
	#define vsnprintf _vsnprintf
#endif

/*  CConvFile class for disk file or temporary memory file in-one manipulation
*  it is probably something like CFile and CMemFile from MFC
*  anyone is free to use it, and some notice would be nice.
*  created by Mirex. Check out my homepage at http://mirex.mypage.sk
*  version 1.0, - 19.january 2004
*/

using namespace ConvFile;

char CConvFile::m_error_code_strings[ CConvFile::e_error_code_count ][ 40 ] = 
{
		"e_no_error",
		"e_no_file",
		"e_bad_parameter",
		"e_bad_parameter1",
		"e_bad_parameter2",
		"e_bad_parameter3",
		"e_bad_parameter4",
		"e_no_memory",
		"e_not_opened",
		"e_file_error",
		"e_out_of_file",
		"e_could_not_read",
		"e_not_opened_for_writing",
		"e_media_error",
} ;

int CConvFile::OpenFile( const char *filename, e_filemode_binary mode, e_filemode_rw readwrite )
{
	char	str[ 10 ];

	if ( opened != o_closed )
		return 0;

	if (( mode != fm_binary ) && ( mode != fm_text )) {
		assert( false );
		error_code = e_bad_parameter2;
		return 0;
	}

	if (( readwrite != fm_read ) && ( readwrite != fm_write ) &&
			( readwrite != fm_read_write )) {
		assert( false );
		error_code = e_bad_parameter3;
		return 0;
	}


	str[0] = 0;

	if ( readwrite == fm_read_write ) {
		f = fopen( filename, "rb" );
		if ( f == NULL )
			strcat( str, "w+" );
		else {
			fclose( f );
			strcat( str, "r+" );
		}
	} else
		strcat( str, ( readwrite == fm_read ) ? "r" : "w" );

	strcat( str, ( mode == fm_binary ) ? "b" : "t" );

	f = fopen( filename, str );
	if ( f == NULL ) {
		error_code = e_no_file;
		return 0;
	}

	filemode	= mode;
	reading		= readwrite;
	opened		= o_file;
	filepos		= 0;

	fseek( f, 0, SEEK_END );
	filesize = ftell( f );
	fseek( f, 0, SEEK_SET );

	error_code = e_no_error;
	return 1;
}

// memory
int CConvFile::OpenMemory( void* memory, unsigned long size, e_filemode_rw read )
{
	if ( opened != o_closed )
		return 0;

	if (( read != fm_read ) && ( read != fm_write ) && ( read != fm_read_write )) {
		assert( false );
		error_code = e_bad_parameter3;
		return 0;
	}

	if ( size == 0 )
		mem = malloc( 1 );
	else
		mem = malloc( size );
	if ( mem == NULL ) {
		error_code = e_no_memory;
		return 0;
	}

	reading		= read;
	opened		= o_memory;
	filesize	= size;
	filepos		= 0;

	if ( memory == NULL )
		memset( mem, 0, size );
	else
		memcpy( mem, memory, size );

	error_code = e_no_error;
	return 1;
}


// close
int CConvFile::Close( void )
{
	int	res;

	switch ( opened ) {
	case o_closed:
		error_code = e_not_opened;
		return 0;
		break;
	case o_file:
		res = fclose( f );
		break;
	case o_memory:
		free( mem );
		break;
	}

	opened = o_closed;
	error_code = e_no_error;
	return 1;
}


int CConvFile::Seek( long pos, e_seek from )
{
	int		res, p;
	
	assert( opened != o_closed );
	if ( opened == o_closed ) {
		error_code = e_not_opened;
		return 0;
	}

	error_code = e_no_error;

	switch( from ) {
	case start_of_file:
		p = pos;
		break;
	case cur_pos:
		p = filepos + pos;
		break;
	case end_of_file:
		p = filesize + pos;
		break;
	default:
		assert( false );
		error_code = e_bad_parameter2;
		return 0;
	}

	if (( p < 0 ) || ( p > filesize )) {
		error_code = e_out_of_file;
		return 0;
	}

	if ( opened == o_file ) {

		res = fseek( f, p, SEEK_SET );
		if ( res == 0 ) {
		
			filepos = ftell( f );
			
			if ( filepos == p )
				return 1;
		}
		error_code = e_file_error;
		return 0;
	}

	if ( opened == o_memory ) {

		filepos = p;
		error_code = e_no_error;
		return 1;
	}

	error_code = e_no_file;
	return 0;
}


// tell file position
unsigned long CConvFile::Tell( void )
{
	assert( opened != o_closed );
	if ( opened == o_closed ) {
		error_code = e_not_opened;
		return 0;
	}

	error_code = e_no_error;
	return filepos;
}

int CConvFile::ResizeFile( unsigned long size )
{
	void*	newmem;
	int		res;

	error_code = e_no_error;

	if ( size == filesize ) {
		return 1;
	}

	switch( opened ) {
	case o_closed:
		assert( opened != o_closed );
		error_code = e_not_opened;
		if ( throws_on_error ) throw error_code;
		return 0;

	case o_file:
		if ( reading == fm_read ) {
			error_code = e_not_opened_for_writing;
			if ( throws_on_error ) throw error_code;
			return 0;
		}
		
		if ( size > filesize ) {
			res = fseek( f, size, SEEK_SET );
			if ( res == 0 ) {
				filesize = ftell( f );
				res = fseek( f, filepos, SEEK_SET );
				return 1;
			} else {
				error_code = e_out_of_file;
				if ( throws_on_error ) throw error_code;
				return 0;
			}
		} else {
			error_code = e_not_opened_for_writing;
			if ( throws_on_error ) throw error_code;
			return 0;
		}
		break;
	case o_memory:
		newmem = realloc( mem, size );
		if ( newmem == NULL ) {
			error_code = e_no_memory;
			if ( throws_on_error ) throw error_code;
			return 0;
		}
		mem = newmem;
		filesize = size;
		return 1;
	default:
		error_code = e_not_opened;
		if ( throws_on_error ) throw error_code;
		return 0;
	}

}

int CConvFile::Eof( void )
{
	if ( opened == o_closed ) {
		error_code = e_not_opened;
		return -1;
	}

	error_code = e_no_error;
	return ( filepos >= filesize );
}

unsigned long CConvFile::FileSize( void )
{
	assert( opened != 0 );
	if ( opened == 0 ) {
		error_code = e_not_opened;
		return 0;
	}

	error_code = e_no_error;

	return filesize;
}

int CConvFile::Read( unsigned long size, void* ptr )
{
	int		res;

	error_code = e_no_error;

	switch( opened ) {
	case o_closed:
		assert( false );
		error_code = e_not_opened;
		return 0;
		break;
	case o_file:
		res = fread( ptr, 1, size, f );
		filepos = ftell( f );
		if ( res != size ) {
			error_code = e_could_not_read;
			if ( throws_on_error ) throw error_code;
		}
		return res;
		break;
	case o_memory:
		if ( filesize - filepos < size )
			size = filesize - filepos;
		memcpy( ptr, (unsigned char*)mem + filepos, size );
		filepos += size;
		return size;
		break;
	}

	error_code = e_no_file;
	return 0;
}

char* CConvFile::gets( char* buffer, int buffer_size )
{
	int		res, i, bp;
	char*	chp;

	if ( buffer == NULL ) {
		assert( false );
		return NULL;
	}

	error_code = e_no_error;
//	\n = 0xA, \r = 0xD

	switch( opened ) {
	case o_closed:
		assert( false );
		error_code = e_not_opened;
		return NULL;
		break;
	case o_file:
		chp = fgets( buffer, buffer_size, f );
		if ( chp == NULL ) {
			error_code = e_could_not_read;
		}

		filepos = ftell( f );
		return chp;
		break;
	case o_memory:

		buffer_size--;

		i = filepos;
		bp = 0;

		while (( i < filesize ) && ( bp < buffer_size )) {

			buffer[ bp++ ] = mem_ch[ i ];

			if (( mem_uch[ i ] == '\n' ) || ( mem_uch[ i ] == '\r' )) {
				if ( mem_uch[ i ] == '\r' )
					buffer[ bp-1 ] = '\n';
				i++;
				if ( ( i < filesize ) && (( mem_uch[ i ] == '\r' ) || ( mem_uch[ i ] == '\n' )) )
					i++;
				break;
			}

			i++;
		} ;

		filepos += i;
		buffer[ bp ] = 0;

		if ( bp == 0 ) {
			error_code = e_could_not_read;
			return NULL;
		}
		return buffer;
		break;
	}

	error_code = e_no_file;
	return NULL;
}

int CConvFile::Write( unsigned long size, const void* ptr )
{
	void	*v;
	int		res;

	if ( reading == fm_read ) {
		error_code = e_not_opened_for_writing;
		return 0;
	}

	error_code = e_no_error;

	switch( opened ) {
	case o_closed:
		assert( false );
		error_code = e_not_opened;
		return 0;
		break;
	case o_file:
		res = fwrite( ptr, 1, size, f );
		filepos = ftell( f );
		if ( filepos > filesize )
			filesize = filepos;
		return res;
		break;
	case o_memory:
		if ( filepos + size > filesize ) {
			if ( ResizeFile( filepos + size ) == 0 )
				return 0;
		}

		memcpy( (unsigned char*)mem + filepos, ptr, size );
		filepos += size;
		return size;
		break;
	}

	error_code = e_no_file;
	return 0;
}


int CConvFile::Write( const char* text )
{
	int		i = 0, j;

	i = strlen( text );

	//pass control to binary write, it sets error code and return number
	j = Write( i, text );
	return j;
}

int CConvFile::Print( const char* format, ... )
{
	char	str[ MaxPrintStrLen ];
	va_list	marker;
	int		i;

	va_start( marker, format );
	i = vsnprintf( str, MaxPrintStrLen, format, marker );
	if ( i < 0 ) {
		error_code = e_no_memory;
		return 0;
	}
	if ( i >= MaxPrintStrLen ) {
		error_code = e_no_memory;
		return 0;
	}

	if ( strlen( str ) == 0 )
		return 0;

	return Write( str );
}

int CConvFile::IsOpened( void )
{
	error_code = e_no_error;

	if ( opened == o_closed )
		return 0;
	return 1;
}

int CConvFile::OpenedType( void )
{
	error_code = e_no_error;
	return opened;
}

int CConvFile::OpenedMode( void )
{
	error_code = e_no_error;

	if ( opened == o_closed )
		return -1;
	return filemode;
}

int CConvFile::OpenedForReading( void )
{
	error_code = e_no_error;

	if ( opened == o_closed )
		return -1;
	return reading;
}

bool CConvFile::OpenedReadable( void )
{
	error_code = e_no_error;

	if ( opened == o_closed )
		return false;

	if (( reading == fm_read ) || ( reading == fm_read_write ))
		return true;
	
	return false;
}

bool CConvFile::OpenedWriteable( void )
{
	error_code = e_no_error;

	if ( opened == o_closed )
		return false;

	if (( reading == fm_write ) || ( reading == fm_read_write ))
		return true;
	
	return false;
}

int CConvFile::CopyFrom( CConvFile& input )
{
	const int BUF_SIZE = 10000;

	unsigned char buf[ BUF_SIZE ];
	unsigned long p, s, ss;
	int  res;

	res = input.OpenedForReading();
	if (( res != fm_read ) && ( res != fm_read_write )) {
		error_code = e_could_not_read;
		return 0;
	}
	res = OpenedForReading();
	if (( res != fm_write ) && ( res != fm_read_write )) {
		error_code = e_not_opened_for_writing;
		return 0;
	}

	input.Seek( 0, start_of_file );
	Seek( 0, start_of_file );

	ss = input.FileSize();

	p = 0; s = BUF_SIZE;
	while( p < ss ) {

		if ( p + s > ss )
			s = ss - p;

		input.Read( s, buf );
		Write( s, buf );

		p += s;
	}

	return 1;
}


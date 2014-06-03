#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <string>
// for std transform
#include <algorithm>
//for exceptions on linux
#include<stdexcept>
#include <fcntl.h>
// define either _DOS, _WIN32, _MAC, or _UNIX

#include <fstream>

// system includes
#if ( defined ( _DOS ) || defined( _WIN32 ) )
	#include <conio.h>
	#include <dos.h>
#endif

#ifdef _WIN32
//	#include <afxwin.h>
	#include <windows.h>
	// bugfix to be able to use gtkmm 
	#undef max
#endif

// use compiler flag -D_UNIX on unix
#ifdef _UNIX
	// commented on 30.5.2011 should not include both cstdlib and #include <stdlib.h>
	#include <cstdlib>
	#include <time.h>
	#include <sys/types.h>
	#include <sys/time.h>
	#include <sys/stat.h>	// S_ISDIR
	#include <unistd.h>
	#include <ctime>

	// for DirChangeNotification functionality
	// commented on 30.5.2011 #define _GNU_SOURCE	// needed to get the defines
	#include <fcntl.h>	// in glibc 2.2 this has the needed values defined
	#include <signal.h>
	#include <stdio.h>
	
	#include <sys/inotify.h>
	#include <sys/types.h>
	#include <poll.h>
	#include <errno.h>
#endif

// OS dependant defines
#ifdef _MSC_VER
	#define snprintf _snprintf
#endif

// conv fncs includes
#include "convfncs.h"
#include "log.h"

#ifdef CONV_FNCS_CONV_FILE_USED
#include "convfile.h"
#endif

#ifdef CONV_FNCS_LEVELING_INCLUDED
	#include "leveling.h"
#endif

#ifdef CONV_FNCS_FORM_INCLUDED
	#include "_form.h"
#endif

using namespace std;

ConvFuncs::s_datetime ConvFuncs::datetime;
char ConvFuncs::DateTimeString[ ConvFuncs::DateTimeString_len ];
ConvFuncs *ConvFuncs::pSingleton = NULL;
int ConvFuncs::m_singleton_init_counter = 0;
const double ConvFuncs::RAD = (180 / 3.14159276);
	
// X: using static variables for static methods may cause problems in multithreading
char ConvFuncs::formater_cstr_buffer[ ConvFuncs::formater_string_buffer_len ];
std::string	ConvFuncs::formater_stdstring;

//ConvFuncs::DirChangeNotificationHandler	*ConvFuncs::m_DirChangeNotificationHandler = NULL;
//int	ConvFuncs::m_DirChangeNotificationFileDescriptor = -1;

ConvFuncs::ConvFuncs()
{ 
	FindFileHandle = -1;
#if ( defined( _MAC ) || defined( _UNIX ))
	DirPtr = NULL;
#endif
	
	Reset(); 
}

void ConvFuncs::Reset( void )
{
#ifdef CONV_FNCS_FORM_INCLUDED
	FormSkipOddByte = -1; FormByteOrdering = byte_ordering_undefined;
#endif
#if (( defined _WIN32 && defined _MFC ) || ( defined _UNIX ))
	srand( time(NULL) );
#endif

	ActualByteOrdering = byte_ordering_undefined;
#ifdef ENDIAN_INTEL
	ActualByteOrdering = byte_ordering_intel;
#endif
#ifdef ENDIAN_MOTORLA
	ActualByteOrdering = byte_ordering_motorla;
#endif
	SwapSourceOrdering = ( ActualByteOrdering == byte_ordering_intel ) ? byte_ordering_motorla : byte_ordering_intel;

#ifdef CONV_FNCS_LEVELING_INCLUDED
	level = 0;
	ResetLeveling();
#endif

	//find file
//#if ( defined _WIN32 && defined _MFC )
#ifdef _WIN32
	if ( FindFileHandle != -1 ) {
		_findclose( FindFileHandle );
		FindFileHandle = -1;
	}
#endif
#if ( defined( _MAC ) || defined( _UNIX ))
	if ( DirPtr != NULL ) {
		closedir( DirPtr );
		memset( &DirEntry, 0, sizeof( DirEntry ) );
		DirPtr = NULL;
	}
#endif
		
	m_DirChangeNotificationInotifyDescriptor = -1;
	m_DirChangeNotificationDirRequestDescriptor = -1;
}

ConvFuncs::~ConvFuncs()
{
#ifdef _WIN32
	if ( FindFileHandle != -1 ) {
		_findclose( FindFileHandle );
	}
#elif ( defined( _MAC ) || defined( _UNIX ))
	// release resources first
	if ( DirPtr != NULL ) {
		closedir( DirPtr );
		memset( &DirEntry, 0, sizeof( DirEntry ) );
		DirPtr = NULL;
	}
#endif
	
	DirChangeNotificationUnsubscribe();
}

//reading while not eof, even when reading stops for some reason.(fread)
//sometimes there is a problem, when i put fread( &, 1, x>4090, file )
//that it wont read more than 4090 bytes at once.
unsigned long ConvFuncs::SkipBigData( unsigned long size, FILE *file )
{
const unsigned long cut = 2000;
	unsigned long	red = 0;
	unsigned char	buf[ cut ];

	if ( size == 0 )
		return 0;

	do {
		red += (unsigned long)
			fread( &buf, 1,
				(size-red>cut)?(size_t)cut: (size_t)(size-red) , file );
	} while (( red < size ) && (! feof( file ) ) && (! ferror( file )) );

	return red;
}

#ifdef CONV_FNCS_CONV_FILE_USED

unsigned long ConvFuncs::SkipBigData( unsigned long size, CConvFile *cfile )
{
const unsigned long cut = 2000;
	unsigned long	red = 0;
	unsigned char	buf[ cut ];

	if ( size == 0 )
		return 0;

	do {
		red += (unsigned long)
			cfile->Read(
				(size-red>cut)?(size_t)cut: (size_t)(size-red), &buf );
	} while (( red < size ) && (! cfile->Eof() ) && ( cfile->GetErrorCode() == CConvFile::e_no_error ));

	return red;
}

#endif

int ConvFuncs::WaitForKey( void )
{
#if ( defined( _WIN32 ) || defined( _DOS ))
	while ( ! kbhit() ) ;
	return ( getch() );
#else
#warning "Implementations for other platforms missing!"
	assert( false );
	return 0;
#endif
}

void ConvFuncs::RotateH( double *x, double *y, double h )
{
        double  c, s;
        double  xt, yt;

        c = cos( h /RAD );
        s = sin( h /RAD );

        xt = *x * c - *y * s;
        yt = *x * s + *y * c;

        *x = xt; *y = yt;
}

void ConvFuncs::RotateHPR( double *x, double *y, double *z, double h, double p, double r )
{
	// x+ = forward, z+ = top, y+ = left
	//R
	RotateH( y, z, r );
	//P
	RotateH( x, z, p );
	//H
	RotateH( x, y, h );
}

void ConvFuncs::RotateZXY( double *x, double *y, double *z, double rx, double ry, double rz )
{
	// x+ = forward, z+ = top, y+ = left
	//H
	RotateH( x, y, rx );
	//R
	RotateH( y, z, rz );
	//P
	RotateH( x, z, ry );
}

void ConvFuncs::RotateNewHPR( double *x, double *y, double *z, double h, double p, double r )
{
	// y+ = top, z- = forward, x+ = right
	//R
	RotateH( x, y, r );
	//P
	*z = - *z;
	RotateH( z, y, p );
	*z = - *z;
	//H
	RotateH( z, x, h );
}

/*
void ConvFuncs::Rotate( double *x, double *y, double *z, double ux, double uy, double uz )
{
	double	cx, cy, cz;
	double	sx, sy, sz;
	double	tx, ty, tz;

	cx = cos( ux /RAD ); sx = sin( ux /RAD );
	cy = cos( uy /RAD ); sy = sin( uy /RAD );
	cz = cos( uz /RAD ); sz = sin( uz /RAD );

	tx = cy*cz * *x 				+ cy*cz * *y				+ (-sy) * *z;
	ty = ((-cx)*sz+ sx*sy*cz) * *x	+ (cx*cz+sx*sy*sz) * *y		+ sx*cy * *z;
	tz = (sx*sz+cx*sy*cz) * *x		+ (-sx*cz+cx*sy*sz) * *y	+ cx*cy * *z;

	*x = tx;
	*y = ty;
	*z = tz;
}
*/

//reads string from file, and fills it into [str] as much as can fit.
//return: size of data red
unsigned short ConvFuncs::ReadAscizFromBinary( FILE *fin, char *str, unsigned short maxlen )
{
	unsigned short	pos = 0, red = 0;
	char			c;

	do {
		fread( &c, 1, 1, fin );
		red++;
		if ( pos < maxlen - 1 ) {
			str[ pos++ ] = c;
		}
	} while (( c != '\0' ) && ( ! feof( fin ) ));

	if ( pos == maxlen - 1 )
		str[ pos ] = '\0';

	return red;
}

#ifdef CONV_FNCS_CONV_FILE_USED

unsigned short ConvFuncs::ReadAscizFromBinary( CConvFile *file, char *str, unsigned short maxlen )
{
	unsigned short	pos = 0, red = 0;
	char			c;

	do {
		file->Read( 1, &c );
		red++;
		if ( pos < maxlen ) {
			str[ pos++ ] = c;
		}
	} while (( c != '\0' ) && ( ! file->Eof() ));

	if ( pos == maxlen )
		str[ pos-1 ] = '\0';

	return red;
}

#endif

char *ConvFuncs::StrItem( const char *str, char num, const char *separators_pnt, unsigned short *pos )
{
#define ResultMaxLen 255
	static char		result[ ResultMaxLen ];
    unsigned short	sp, rp, i;
	char			default_separators[] = " \n\r\t";
    const char		*separators;

	if ( separators_pnt == NULL )
		separators = default_separators;
	else
		separators = separators_pnt;

	//find NUM space
    sp = 0;
    do {
		//if i find gap dec counter and skip whole gap
		for( i=0; i<strlen( separators ); i++ )
			if ( str[sp] == separators[i] )
				break;
		if ( i < strlen( separators ) ) {
			//posuniem cislo itemu, avsak nie ak to bol prvy znak => odstranim medzery na zaciatku stringu
			if (( num > 0 ) && ( sp > 0 ))
				num--;
			do {
				for( i=0; i<strlen( separators ); i++ )
					if ( str[sp] == separators[i] )
						break;
				if ( i < strlen( separators ) )
					sp++;
			} while ( i < strlen( separators ) );
		}
		else
			if ( num != 0 )
				sp++;

	} while (( num > 0 ) && ( str[sp] != '\0' ));
    //copy to next space
    rp = 0;
    while ( (str[sp] != '\0') && ( strchr( separators, str[sp] ) == NULL ) && ( rp < ResultMaxLen-1 ))
      result[rp++] = str[sp++];
    result[ rp ] = '\0';

	if ( pos != NULL )
		*pos = sp;

    return result;
}

//pri eofe sa vrati "", inak slovo
char *ConvFuncs::ReadWord( FILE *fin )
{
#define LineBufSize 100
	static char LineBuf[ LineBufSize ];
	static char	result[ LineBufSize ];
	int			p, s;
	bool		nasielsom = false;

// oddelovace, alebo aj toto su slova: "{" "}" "\n" ";" ":" "(text\ntext)" "#" "//" "//*" "*//" "[" "]" '"'
#define SpecialnychSlov 16
char SpecialneSlova[ SpecialnychSlov ][10] =
{
	"<",	">",
	"{",	"}",
	"(",	")",
	"/*",	"*/",
	"[",	"]",
	"//\0",
	"\n",
	";",
	"#",
	":",
	"\"",
};

	do {
		//odstranim medzery na zaciatku
		p = 0;
		while ( ((LineBuf[ p ] == ' ') || ( LineBuf[ p ] == '\t' )) && ( p < (int)strlen( LineBuf ) ) )
			p++;

		if ( p > 0 ) {
			memcpy( LineBuf, &LineBuf[p], strlen( &LineBuf[p] )+1 );
		}

		//doplnim si buffer
		fgets( &LineBuf[ strlen( LineBuf ) ], LineBufSize - strlen( LineBuf ), fin );
	} while ( p != 0 );

	p=0;
	do {
		for( s=0; s<SpecialnychSlov; s++ )
			if ( strstr( &LineBuf[p], SpecialneSlova[ s ] ) == &LineBuf[p] )
				break;
		//ak som nasiel nejake specialne slovo
		if ( s < SpecialnychSlov ) {
			//tak tu konci to predchadzajuce slovo, alebo ak som na 0 tak tu zacina
			if ( p == 0 ) {
				strcpy( result, SpecialneSlova[ s ] );
			}
			else {
				memcpy( result, LineBuf, p );
				result[ p ] = '\0';
			}
			nasielsom = true;
		}

		if ( LineBuf[ p ] == ' ' ) {
			nasielsom = true;
			memcpy( result, LineBuf, p );
			result[ p ] = '\0';
		}

		p++;
	} while ( (! nasielsom) && ( p < (int)strlen( LineBuf ) ) );

	if ( ! nasielsom )
		strcpy( result, "" );

	//este odstranim vysledok z buffera
	memcpy( LineBuf, &LineBuf[ strlen( result ) ], LineBufSize- strlen( result ) + 1 );

	return result;
}

void ConvFuncs::ReadLine( FILE *file )
{
	Line[0]='\0';
	fgets( Line, MaxLineLen, file );
}

#ifdef CONV_FNCS_CONV_FILE_USED

void ConvFuncs::ReadLine( CConvFile *file )
{
	Line[0]='\0';
	file->gets( Line, MaxLineLen );
}

#endif

//ReadLineSkip reads line, but skips empty lines before it
void ConvFuncs::ReadLineSkip( FILE *file )
{
	Line[0]='\0';
	do
		fgets( Line, MaxLineLen, file );
	while ( ((strlen( Line ) == 0) || ( Line[0] == '\n' ) || ( Line[0] == '\r' )) && (! feof( file )) );
}

#ifdef CONV_FNCS_CONV_FILE_USED

//ReadLineSkip reads line, but skips empty lines before it
void ConvFuncs::ReadLineSkip( CConvFile *file )
{
	Line[0]='\0';
	do
		file->gets( Line, MaxLineLen );
	while ( ((strlen( Line ) == 0) || ( Line[0] == '\n' ) || ( Line[0] == '\r' )) && (! file->Eof()) );
}

#endif

//reads file & tries to keep Line full.
void ConvFuncs::ReadLineFull( FILE *file )
{
	int		pos = strlen( Line );
	int		c;

	while (( pos < MaxLineLen-1 ) && ( ! feof( file ) )) {
		c = getc( file );
		if ( c == EOF )
			break;
		Line[ pos++ ] = c;
	}
	Line[ pos ] = 0;
}

#ifdef CONV_FNCS_CONV_FILE_USED

void ConvFuncs::ReadLineFull( CConvFile *file )
{
	int		pos = strlen( Line );
	char	c;

	while (( pos < MaxLineLen-1 ) && ( file->Eof() )) {
		file->Read( 1, &c );
		Line[ pos++ ] = c;
	}
	Line[ pos ] = 0;
}

#endif

//vymazem z Line vsetko od [remark_str] po eol
void ConvFuncs::RemoveRemarkFromLine( char *remark_str )
{
	char			*res;
	unsigned int	s, e, i, l;

	do {
		res = strstr( Line, remark_str );
		if ( res == NULL )
			break;

		e = s = res - Line;

		while (( e < strlen( Line ) ) && ( Line[ e ] != '\n' ) && ( Line[ e ] != '\r' ))
			e++;

		if ( e == strlen( Line ) )
			break;

		l = strlen( Line );
		for( i=e; i<=l; i++ )
			Line[ i - (e-s) ] = Line[ i ];
	} while ( true );

}

void ConvFuncs::RemoveLineLeadSpaces( void )
{
	char	Line2[ MaxLineLen ];
	int		pos = 0;

	while ( Line[ pos ] == ' ' )
		pos++;

	if ( pos != 0 ) {
		strcpy( Line2, Line );
		strcpy( Line, &Line2[ pos ] );
	}

}

char* ConvFuncs::RemoveLeadingAndTrailingChars( char* str, char* chars )
{
	static char		result[ ResultMaxLen ];
	unsigned char	i, p, s;

	p = 0;
	do {
		for( i=0; i<strlen( chars ); i++ )
			if ( str[ p ] == chars[ i ] )
				break;
		p++;
	} while ( i < strlen( chars ) );

	s = p - 1;

	p = strlen( str );
	do {
		p--;
		for( i=0; i<strlen( chars ); i++ )
			if ( str[ p ] == chars[ i ] )
				break;
	} while ( i < strlen( chars ) );

	memcpy( result, &str[ s ], p - s + 1 );
	result[ p - s + 1 ] = 0;

	return result;
}

int ConvFuncs::StrReplaceChars( char *str, const char* oldchars, const char *newchars )
{
	unsigned long	i, j, len, chlen, res;

	res = 0;
	len = strlen( str );

	chlen = strlen( oldchars );
	if ( chlen != strlen( newchars ) ) {
		assert( false );
		return 0;
	}

	for( i=0; i<len; i++ ) {

		for( j=0; j<chlen; j++ )
			if ( str[ i ] == oldchars[ j ] ) {
				str[ i ] = newchars[ j ];
				res++;
				j = chlen;
			}
	}

	return res;
}

int ConvFuncs::StrReplaceChars( std::string& str, const char* oldchars, const char *newchars )
{
	char* s = new char[ str.size() + 1 ];
	int		i;
	
	strcpy( s, str.c_str() );
	
	i = StrReplaceChars( s, oldchars, newchars );
	str = s;
	
	delete[] s;
	return i;
}

int ConvFuncs::IsNumber( const char *text )
{
	char		FloatNumberChars[] = "0123456789.";
	int			i;
	int			num_dots;

	if ( strlen(text) == 0 )
		return 0;

	i = 0;
	num_dots = 0;
	
	if ( text[ i ] == '-' )
		i++;
	
	while (( i < (int)strlen( text )) && ( strchr( FloatNumberChars, text[i] ) != NULL )) {
		if ( text[ i ] == '.' )
			num_dots++;
		i++;
	}
	return (( i == (int)strlen( text ) ) && ( num_dots <= 1 )) ? 1 : 0;
}

//upravi vektor tak aby mal velkost 1
void ConvFuncs::VektorNaJednotkovy( double *vx, double *vy, double *vz )
{
	double	c;

	c = *vx * *vx + *vy * *vy + *vz * *vz;
	if ( c == 0 ) {
		*vx = 1;
		return;
	}

	c = sqrt( c );

	*vx = *vx / c;
	*vy = *vy / c;
	*vz = *vz / c;
}

void ConvFuncs::TriangleNormal( double *rx, double *ry, double *rz,
									double ax, double ay, double az,
									double bx, double by, double bz,
									double cx, double cy, double cz )
{
	*rx = ((by - ay) * (cz - az) - (cy - ay) * (bz - az));
	*ry = ((cx - ax) * (bz - az) - (bx - ax) * (cz - az));
	*rz = ((bx - ax) * (cy - ay) - (cx - ax) * (by - ay));
}

//call this before CheckTime
void ConvFuncs::StartCheckTime( void )
{
#if ( defined _WIN32 && defined _MFC )
	GetSystemTime( &CheckTimeStart );
#endif
#ifdef _UNIX
	gettimeofday(&CheckTimeStart, NULL);
	CheckTimeStartD =  CheckTimeStart.tv_sec + ( CheckTimeStart.tv_usec / 1000000.0 );
#endif
}

//return: 0=nothing, 1=passed
int ConvFuncs::CheckTime( float seconds )
{
#if ( defined _WIN32 && defined _MFC )
	float time1, time2;
	float timeDelta;

	GetSystemTime( &CheckTimeNow );

	time1 = CheckTimeNow.wMinute * 60 + CheckTimeNow.wSecond + (float)CheckTimeNow.wMilliseconds / 1000;
	time2 = CheckTimeStart.wMinute * 60 + CheckTimeStart.wSecond + (float)CheckTimeStart.wMilliseconds / 1000;
	timeDelta = time1 - time2;
	if ( timeDelta < 0 )
		timeDelta += 60*60; // one hour;
	if ( timeDelta > seconds )
		return 1;
	else
		return 0;
#endif

#ifdef _UNIX
	double timeDelta;
	
	gettimeofday( &CheckTimeNow, NULL );
	double time2 = CheckTimeNow.tv_sec + ( CheckTimeNow.tv_usec / 1000000.0 );
	timeDelta = time2 - CheckTimeStartD;
	if ( timeDelta > seconds )
		return 1;
	else
		return 0;
#endif
	
	return 1;
}

#define MaxSwapBytes 64
void ConvFuncs::Swap( void* data, int LengthInBytes )
{
	if (( LengthInBytes > MaxSwapBytes ) || ( LengthInBytes <= 0 )) {
		LogS( InfoError, "Swap: too many bytes (%i) !\n", LengthInBytes );
		assert( false );
		return;
	}

	int				i;
	unsigned char	*normal, swapped[ MaxSwapBytes ];

	normal = (unsigned char*) data;

	for( i=0; i<LengthInBytes; i++ )
		swapped[ i ] = normal[ LengthInBytes - 1 - i ];

	memcpy( data, swapped, LengthInBytes );

}

void ConvFuncs::Swapso( void* data, int LengthInBytes )
{
	assert( ActualByteOrdering != byte_ordering_undefined );
	if ( SwapSourceOrdering == ActualByteOrdering )
		return;

	Swap( data, LengthInBytes );
}

void ConvFuncs::SwapsoSetSourceOrdering( int source_ordering )
{
	SwapSourceOrdering = source_ordering;
}

// motorla <=> intel
unsigned long ConvFuncs::SwapULong( unsigned long in )
{
	union {
		unsigned char	c[4];
		unsigned long	l;
	} normal, swaped;

	normal.l = in;
	swaped.c[0] = normal.c[3];
	swaped.c[1] = normal.c[2];
	swaped.c[2] = normal.c[1];
	swaped.c[3] = normal.c[0];
	return swaped.l;

}

long ConvFuncs::SwapSLong( long in )
{
	union {
		unsigned char	c[4];
		long			l;
	} normal, swaped;

	normal.l = in;
	swaped.c[0] = normal.c[3];
	swaped.c[1] = normal.c[2];
	swaped.c[2] = normal.c[1];
	swaped.c[3] = normal.c[0];
	return swaped.l;

}

unsigned short ConvFuncs::SwapShort( unsigned short in )
{
	union {
		char			c[2];
		unsigned short	s;
	} normal, swaped;

	normal.s = in;
	swaped.c[0] = normal.c[1];
	swaped.c[1] = normal.c[0];
	return swaped.s;

}

/* Use StrItem()
//if I have string X:-12.334\t and give param ':' then it will return -12.334
char *GetNumberFromString( char *str, char starts_after_this )
{
	#define ResultLen 100
	static char	result[ ResultLen ];
	int			i;
	char		FloatNumberChars[] = "0123456789.-";

	if ( strlen( str ) >= ResultLen ) {
		LogS( InfoError, "GetNumberFromString: string too long [%s]", str );
		return str;
	}

	result[0] = '\0';

	//find that char
	i = 0;
	while (( str[i] != starts_after_this ) && ( str[i] != '\0' ))
		i++;

	if ( str[i] == '\0' )
		return result;

	strcpy( result, &str[i] );

	//find end of number
	i=0;
	while ( strchr( FloatNumberChars, result[i] ) != NULL )
		i++;

	result[i] = '\0';

	return result;
}
*/

fract ConvFuncs::Float2Fract( double f )
{
        return ( (fract) SwapSLong( (unsigned long) (f * 65536l) ) );
}

double ConvFuncs::Fract2Float( fract f )
{
        return ( ((float) SwapSLong((unsigned long)f) ) / 65536l );
}

// if str=<bla ks"aejt aswe"qasdf> it returns <aejt aswe>
char *ConvFuncs::GetNameFromQuotes( char *str )
{
static	char		res[ MaxLineLen ];
	unsigned short	start, end;

	start = 0;
	while (( str[ start ] != '"' ) && ( start < strlen( str ) ))
		start++;

	end = start + 1;
	while (( str[ end ] != '"' ) && ( end < strlen( str ) ))
		end++;

	memcpy( res, &str[ start + 1 ], end-1 );
	res[ end - start - 1 ] = 0;

	return res;
}

unsigned long ConvFuncs::FindTwoSame( unsigned long a1, unsigned long a2, unsigned long b1, unsigned long b2 )
{
	if (( a2 == b1 ) || ( a2 == b2 ))
		return a2;
	if (( a1 == b1 ) || ( a1 == b2 ))
		return a1;
	Log( "FindTwoSame: Can't find two same !\n", InfoError );
	return -1;
}

double ConvFuncs::Distance3( double x, double y, double z )
{
	return sqrt( x*x + y*y + z*z );
}

double ConvFuncs::VectorLength2( double x, double y )
{
	return sqrt( x*x + y*y );
}

char* ConvFuncs::CreateUniqueFilename( char *path )
{
#define CreateUniqueFNmaxloop 100
	int		i;
	FILE	*f;
static	char	str[ BBL_MaxFileNameLen ];

	i = 0;

	do {
		sprintf( str, "%s%cF%lu.tmp", path, path_good_slash_char, ((unsigned long)rand()) % 10000000 );
		f = fopen( str, "rb" );
		if ( f == NULL )
			break;
		fclose( f );
		i++;
	} while ( i < CreateUniqueFNmaxloop );

	if ( i == CreateUniqueFNmaxloop )
		str[ 0 ] = 0;

	return str;
	//i'll create file
//	f = fopen( name, "wb" );
//	fclose( f );
}

// zadas bod v rovine a vrati to jeho uhly voci bodu 0, (uhol 0=x+, uhol 90=y+)
double ConvFuncs::PointAngle2d( double x, double y )
{
	double	a;

	if ( x == 0 ) {
		if ( y == 0 )
			a = 0;
		else
			a = ( y < 0 ) ? -90 : 90;
	}
	else {
		a = atan( y / x ) * RAD;

		if ( x < 0 )
			a += 180;

	}

	if ( a < 0 )
		a += 360;

	return a;
}

// zadas bod v priestore a vrati to jeho uhly voci bodu 0
void ConvFuncs::PointAngle3d( double x, double y, double z, double *h, double *p )
{
	double	xy;

	*h = PointAngle2d( x, y );
	xy = sqrt( x*x + y*y );
	*p = PointAngle2d( xy, z );
}

double ConvFuncs::AngleIntoBound360( double a )
{
	while ( a > 360 )
		a -= 360;
	while ( a < 0 )
		a += 360;

	return a;
}

double ConvFuncs::Angle3Points2d( double ax, double ay, double xx, double xy, double bx, double by )
{
	double	u;

	u = PointAngle2d( ax - xx, ay - xy ) - PointAngle2d( bx - xx, by - xy );
	u = AngleIntoBound360( u );

	return u;
}

//returns nearest equal or bigger number which is 2 ^ x (where x is integer number )
unsigned long ConvFuncs::FixNumTo2n( unsigned long in_num )
{

	int				i = 0, c = 0;
	unsigned long	in = in_num;

	while ( in > 0 ) {
		i++;
		c += in & 1;
		in >>= 1;
	}

	if ( c == 1 )
		return in_num;

	in = 1;
	in <<= i;

	return in;
}

int ConvFuncs::PointInRectLong( long px, long py, long x1, long y1, long x2, long y2 )
{
	long	a;

	// make sure that coord 1 are minimum coords
	if ( x2 < x1 ) {
		a = x1; x1 = x2; x2 = a;
	}
	if ( y2 < y1 ) {
		a = y1; y1 = y2; y2 = a;
	}

	if (( x1 <= px ) && ( px <= x2 ) && ( y1 <= py ) && ( py <= y2 ))
		return 1;
	else
		return 0;
}

const char* ConvFuncs::GetFileFromPathFilename( const char *str )
{
	int		pos, size;
static	char	result[ BBL_MaxFileNameLen ];

	pos = strlen( str );

	while (( pos > 0 ) && ( str[ pos-1 ] != '\\' ) && ( str[ pos-1 ] != '/' ))
		pos--;

	size = strlen( &str[ pos ] );
	if ( (BBL_MaxFileNameLen-1) < size )
		size = BBL_MaxFileNameLen-1;

	memcpy( result, &str[ pos ], size );
	result[ size ] = 0;
	return result;
}

const char* ConvFuncs::GetPathFromPathFilename( const char *str )
{
	int		pos;
static	char	result[ BBL_MaxFileNameLen ];

	pos = strlen( str );

	while (( pos > 0 ) && ( str[ pos-1 ] != '\\' ) && ( str[ pos-1 ] != '/' ))
		pos--;

	if ( (BBL_MaxFileNameLen-1) < pos )
		pos = BBL_MaxFileNameLen-1;

	memcpy( result, str, pos );
	result[ pos ] = 0;
	return result;
}

const char *ConvFuncs::FindOniFileById( unsigned long id, char* path )
{
	bool		res;
	char		name[7];
	char		str[ BBL_MaxFileNameLen ];

	LogS( InfoMinor, "searching for oni file with id=%lu..", id );

	if ( id > 99999 ) {
		Log( "id too big\n", InfoError );
		return NULL;
	}

	sprintf( name, "%05lu-", id );
#ifdef _WIN32
	sprintf( str, "%s\\%s*.*", path, name );
	res = Find_FirstFile( str );
#else
	res = Find_FirstFile( path );
	while ( res ) {
		if ( memcmp( name, Find_CurrentFileName(), 6 ) == 0 )
			break;
		
		res = Find_NextFile();
	} ;
#endif
	
	if ( res == false ) {
		Log( "None found\n", InfoError );
		return NULL;
	}
	else {
		LogS( InfoMinor, "found [%s]\n", Find_CurrentFileName() );
		return Find_CurrentFileName();
	}
}

bool ConvFuncs::Find_FirstFile( const char* pattern )
{
#ifdef _WIN32

	string patternCopy = pattern;
	char lastChar = *(patternCopy.rbegin());
	if (( lastChar == '/' ) || ( lastChar == '\\' ))
		patternCopy += "*";

	if ( FindFileHandle != -1 ) {
		_findclose( FindFileHandle );
	}
	FindFileResult = FindFileHandle = _findfirst( patternCopy.c_str(), &FindFileData );
	FindFileFirstFile = 1;

	return ( FindFileHandle != -1 );
#elif ( defined( _MAC ) || defined( _UNIX ))
	// release resources first
	if ( DirPtr != NULL ) {
		closedir( DirPtr );
		memset( &DirEntry, 0, sizeof( DirEntry ) );
		DirPtr = NULL;
	}

	// separate filename to directory and filename
	std::string		str_dir, str_file;
	str_file = GetFileFromPathFilename( pattern );
	str_dir = GetPathFromPathFilename( pattern );
	
	DirPtr = opendir( str_dir.c_str() );
	if ( DirPtr == NULL )
		return false;
	
	// find out file mask
	// supported: "*", "*.*", "", "*ending" "starting*", "starting*ending", "full"
	m_find_file_pattern.Reset();
	
	// empty, * or *.* means all files
	if (( str_file.empty() ) || ( str_file == "*" ) || ( str_file == "*.*" )) {
		m_find_file_pattern.any = true;
	} else {
		m_find_file_pattern.any = false;
		
		if ( *(str_file.begin()) == '*' ) {
			m_find_file_pattern.asterisk_start = true;
			str_file.erase( str_file.begin() );
		}
		if ( *(str_file.rbegin()) == '*' ) {
			m_find_file_pattern.asterisk_end = true;
			str_file.erase( str_file.end() - 1 );
		}
		
		if (( m_find_file_pattern.asterisk_start == false ) &&
		    ( m_find_file_pattern.asterisk_end == false )) {
			// count if there are any stars inside
			size_t pos = 0;
			unsigned int count = 0;
			do {
				pos = str_file.find( '*', pos );
				if ( pos == string::npos )
					break;
				
				count++;
				pos++;
			} while( true );
			
			// if there are more than 1 stars in string then there is something wrong.
			if ( count > 1 )
				throw runtime_error( "Too many * in the search string " );
			
			if ( count == 1 ) {
				m_find_file_pattern.asterisk_center = true;
				
				pos = str_file.find( '*' );
				m_find_file_pattern.str2 = str_file.substr( pos+1 );
				str_file.erase( pos );
			}
			else
			{
				m_find_file_pattern.full_name = true;
			}
		}
			
		m_find_file_pattern.str = str_file;
	}
	
	return Find_NextFile();
	
#else
	#warning Find_FirstFile() No implementation !
	return false;
#endif
}

bool ConvFuncs::Find_NextFile( void )
{
#ifdef _WIN32
	if ( FindFileFirstFile == 1 )
		FindFileFirstFile = 0;
	
	FindFileResult = _findnext( FindFileHandle, &FindFileData );
	
	return( FindFileResult != -1 );
#elif ( defined( _MAC ) || defined( _UNIX ))
	if ( DirPtr == NULL ) {
		return false;
	}
	
	bool	res = true;
	struct dirent*	DirEntryPtr;
	int result;
	
	while ( res ) {
		
		// find next file
		result = readdir_r( DirPtr, &DirEntry, &DirEntryPtr );
		if  (( result != 0) || (DirEntryPtr == NULL ))
			return false;
		
		// see if it matches find pattern
		if ( m_find_file_pattern.any )
			return true;
		
		if ( m_find_file_pattern.full_name ) {
			if ( m_find_file_pattern.case_sensitive )
				if ( strcmp( m_find_file_pattern.str.c_str(), DirEntry.d_name ))
					return true;
			else
				if ( stricmp( m_find_file_pattern.str.c_str(), DirEntry.d_name ))
					return true;
			
			continue;
		}
		
		if (( m_find_file_pattern.asterisk_start ) && ( m_find_file_pattern.asterisk_end )) {
			if ( m_find_file_pattern.case_sensitive )
				if ( strstr( DirEntry.d_name, m_find_file_pattern.str.c_str() ) != NULL )
					return true;
			else
				if ( strcasestr( DirEntry.d_name, m_find_file_pattern.str.c_str() ) != NULL )
					return true;
			
			continue;
		}
		if ( m_find_file_pattern.asterisk_end ) {
			std::string		str;
			
			str = DirEntry.d_name;
			if ( str.size() < m_find_file_pattern.str.size() )
				continue;
			
			str.erase( str.begin() + m_find_file_pattern.str.size(), str.end() );
			if ( m_find_file_pattern.case_sensitive )
				if ( strcmp( str.c_str(), m_find_file_pattern.str.c_str() ) == 0 )
					return true;
			else
				if ( stricmp( str.c_str(), m_find_file_pattern.str.c_str() ) == 0 )
					return true;
			
			continue;
		}
		
		if ( m_find_file_pattern.asterisk_start ) {
			std::string		str;
			
			str = DirEntry.d_name;
			if ( str.size() < m_find_file_pattern.str.size() )
				continue;
			
			str.erase( str.begin(), str.begin() + ( str.size() - m_find_file_pattern.str.size() ) );
			if ( m_find_file_pattern.case_sensitive )
				if ( strcmp( str.c_str(), m_find_file_pattern.str.c_str() ) == 0 )
					return true;
			else
				if ( stricmp( str.c_str(), m_find_file_pattern.str.c_str() ) == 0 )
					return true;
			
			continue;
		}
		
		if ( m_find_file_pattern.asterisk_center ) {
			std::string		str;
			
			// check start of string with 'str' as in case of 'something*'
			str = DirEntry.d_name;
			if ( str.size() < m_find_file_pattern.str.size() )
				continue;
			if ( str.size() < m_find_file_pattern.str2.size() )
				continue;
			
			str.erase( m_find_file_pattern.str.size() );
			if ( m_find_file_pattern.case_sensitive )
				if ( strcmp( str.c_str(), m_find_file_pattern.str.c_str() ) != 0 )
					continue;
			else
				if ( stricmp( str.c_str(), m_find_file_pattern.str.c_str() ) != 0 )
					continue;
			
			// check end of string with 'str2' as in case of '*something'
			str = DirEntry.d_name;
			
			str.erase( 0, ( str.size() - m_find_file_pattern.str2.size() ) );
			if ( m_find_file_pattern.case_sensitive )
				if ( strcmp( str.c_str(), m_find_file_pattern.str2.c_str() ) != 0 )
					continue;
			else
				if ( stricmp( str.c_str(), m_find_file_pattern.str2.c_str() ) != 0 )
					continue;
			
			return true;
		}
		
		// what am I looking for then ?
		assert( false );
		return false;
	} ;
	
	return false;
#else
	#warning Find_NextFile() No implementation !
	return false;
#endif
}

const char*	ConvFuncs::Find_CurrentFileName( void )
{
#ifdef _WIN32
	return FindFileData.name;
#elif ( defined( _MAC ) || defined( _UNIX ))
	return DirEntry.d_name;
#else
	return NULL;
#endif
}

int ConvFuncs::Find_CurrentIsDir( void )
{
#ifdef _WIN32
	return (( FindFileData.attrib & _A_SUBDIR ) > 0 );
#elif ( defined( _MAC ) || defined( _UNIX ))
	if ( DirEntry.d_type == DT_DIR )
		return 1;
	if ( DirEntry.d_type == DT_LNK ) {
		
		/*
		struct stat	s;
		stat( DirEntry.d_name, &s );
		if ( s.st_mode == S_ISDIR )
			return 1;
		*/
		return 1;
	}
	
	return 0;
#else
	return 0;
#endif
}

int ConvFuncs::Find_CurrentIsReadOnly( void )
{
#ifdef _WIN32
	return (( FindFileData.attrib & _A_RDONLY ) > 0 );
#elif ( defined( _MAC ) || defined( _UNIX ))
	#warning "TODO: Check file attributes, but not from direntry structure."
	return true;	// !!! TODO
#else
	return true;
#endif
}

int ConvFuncs::DoMoveFile( const char* src, const char* dest )
{
	
#if (( defined _UNIX ) || defined ( _WIN32 ))
	return ( rename( src, dest ) == 0 );
#endif

	assert( false );
	return 0;
}

int ConvFuncs::DoDeleteFile( const char* src )
{
#if (( defined _UNIX ) || defined ( _WIN32 ))
	return ( remove( src ) == 0 );
#endif

	assert( false );
	return 0;
}

bool ConvFuncs::DoCopyFile(std::string src, std::string dest)
{
#if (( defined _UNIX ) || defined ( _WIN32 ))
	
	std::ifstream  srcf( src.c_str(), std::ios::binary );
	if (!srcf.is_open()) {
		LogS( InfoWarn, "File could not be open: '%s'", src.c_str() );
		return false;
	}
	 
	std::ofstream  dstf( dest.c_str(), std::ios::binary);
	if (!dstf.is_open()) {
		LogS( InfoWarn, "Output file could not be open: '%s'", dest.c_str() );
		return false;
	}

	dstf << srcf.rdbuf();

	dstf.close();
	srcf.close();
	return true;

#endif

	assert( false );
	return false;
}

// Return true if the folder exists, false otherwise
bool folderExists(const char* folderName) {

#if (defined ( _WIN32 ))
    if (_access(folderName, 0) == -1) {
        //File not found
        return false;
    }

    DWORD attr = GetFileAttributes((LPCSTR)folderName);
    if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        // File is not a directory
        return false;
    }

    return true;
#endif

#if ( defined _UNIX )
	// if folder exists return
	struct stat	st;
	if (stat(folderName, &st) == 0)
    {
		if (!S_ISDIR(st.st_mode))
		{
			// filename exists, but it is not a directory
			return false;
		}
		
		return true;
	}

	return false;
	
#endif

	return false;
}

bool ConvFuncs::DoCreateFolder( const char* parentPath, const char* folderName )
{
	std::string fullPath = parentPath;
	fullPath += "/";
	fullPath += folderName;
	DoCreateFolder( fullPath );
}

bool ConvFuncs::DoCreateFolder( const char* fullPath)
{
	if (folderExists(fullPath.c_str()))
		return true;

	if (!folderExists(parentPath))
		return false;

#if (defined ( _WIN32 ))
	return CreateDirectory(fullPath.c_str(), NULL) != 0;
#endif

#if ( defined _UNIX )
	return ((mkdir(fullPath.c_str(), 0777) == 0) || (errno == EEXIST));
#endif
	
	assert( false );
	return 0;
}


#ifdef UNICODE
const wchar_t*	ConvFuncs::GetExecutableFilePathName( void )
#else
const char* ConvFuncs::GetExecutableFilePathName( void )
#endif
{
	enum {
		str_len = 512,
	} ;
#ifdef UNICODE
	static wchar_t	filename[ str_len ];
#else
	static char filename[ str_len ];
#endif

	filename[ 0 ] = 0;
	
#ifdef _UNIX
	pid_t	pid = getpid();
	char	buf[ 128 ];
	
	snprintf( buf, 127, "/proc/%i/exe", pid );
	
	readlink( buf, filename, 511 );
	
/*
Linux:
/proc/<pid>/exe

Solaris:
/proc/<pid>/object/a.out (filename only)
/proc/<pid>/path/a.out (complete pathname)

*BSD (and maybe Darwing too):
/proc/<pid>/file
*/

#endif

#ifdef _WIN32
	GetModuleFileName( NULL, filename, str_len );
#endif

	return filename;
}

char* ConvFuncs::CurDateTimeString( void )
{
	GetDateTime();
	
	/*
	char	DaysOfWeek[7][4] =
	{ "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	char	MonthsOfYear[12][4] =
	{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

	const char* dayText = DaysOfWeek[ datetime.dayofweek ];
	const char* monthText = MonthsOfYear[ datetime.month ];
	*/
	
	// Get localized texts by system calls
	struct tm date;
	date.tm_mon = datetime.month;
	date.tm_wday = datetime.dayofweek;
	
	char dayText[ 512 ];
	char monthText[ 512 ];

#ifdef _UNIX
	strftime( dayText, 511, "%a", &date );
	strftime( monthText, 511, "%b", &date );
#else
	snprintf( dayText, 511, "" );
	snprintf( monthText, 511, "%i", date.tm_mon + 1 );
#endif

	// format all together
	snprintf( DateTimeString, DateTimeString_len, "%s %i %s %i %02i:%02i",
		( datetime.dayofweek == -1 ) ? "-" : dayText,
		datetime.day,
		monthText,
		datetime.year,
		datetime.hour, datetime.minute );

	return DateTimeString;
}

void ConvFuncs::GetDateTime( void )
{
	datetime.Reset();
	
#ifdef _WIN32
	SYSTEMTIME		time;

	GetSystemTime( &time );

	TIME_ZONE_INFORMATION	timeZoneInformation;
	GetTimeZoneInformation( &timeZoneInformation );

	DWORD timeAdjustment;
	DWORD timeIncrement;
	BOOL adjustmentDisabled;
	GetSystemTimeAdjustment( &timeAdjustment, &timeIncrement, &adjustmentDisabled );
	int differenceInHours = - timeZoneInformation.Bias / 60 - timeZoneInformation.DaylightBias / 60;

	// this may cause wrapping over the 24 hours or below 0 hours. Have to be careful on that
	double vtime;
	double differenceInDays = differenceInHours / 24.0;
	SystemTimeToVariantTime( &time, &vtime );
	vtime += differenceInDays;
	VariantTimeToSystemTime( vtime, &time );

	datetime.year		=	time.wYear;
	datetime.month		=	time.wMonth;
	datetime.day		=	time.wDay;
	datetime.hour		=	time.wHour;
	datetime.minute		=	time.wMinute;
	datetime.second		=	time.wSecond;
	
	datetime.dayofweek	=	time.wDayOfWeek;
#endif
#ifdef _DOS
	struct date     d;
    struct time     t;

    getdate( &d );
    gettime( &t );
	
	datetime.year		=	d.da_year;
	datetime.month		=	d.da_mon;
	datetime.day		=	d.da_day;
	datetime.hour		=	t.ti_hour;
	datetime.minute		=	r.ti_min;
	datetime.second		=	t.ti_sec;
	
#endif
#ifdef _UNIX

	time_t	time_integer = time( NULL );
	struct tm*	time_struct;
	
	time_struct = localtime( &time_integer );
	
	datetime.year		=	1900 + time_struct->tm_year;
	datetime.month		=	time_struct->tm_mon + 1;
	datetime.day		=	time_struct->tm_mday;
	datetime.hour		=	time_struct->tm_hour;
	datetime.minute		=	time_struct->tm_min;
	datetime.second		=	time_struct->tm_sec;
	
	datetime.dayofweek	=	time_struct->tm_wday;
	datetime.dayofyear	=	time_struct->tm_yday;
#endif

}


//returns length of copied string
int ConvFuncs::StrCpyLen( char* dest, const char* source, int dest_size )
{
	int		pos;

	pos = 0;
	dest_size--;
	while (( source[ pos ] != 0 ) && ( pos < dest_size )) {
		dest[ pos ] = source[ pos ];
		pos++;
	}
	dest[ pos ] = 0;

	return pos;
}

const char*	 ConvFuncs::FormatCstr( const char* format, ... )
{
	va_list	marker;

	va_start( marker, format );

	int res = vsnprintf( formater_cstr_buffer, formater_string_buffer_len-1,
		format, marker );

	return formater_cstr_buffer;
}

const std::string ConvFuncs::FormatStdStr( const char* format, ... )
{
	va_list	marker;

	va_start( marker, format );

	int res = vsnprintf( formater_cstr_buffer, formater_string_buffer_len-1,
		format, marker );

	// X: this code is commented out becuase it somehow caused segfault on 2013-08-04 on XUbuntu 12.
	// X: Somehow formater_cstr_buffer was not initialized even though it was static class variable.
	// formater_stdstring = formater_cstr_buffer;
	// return formater_stdstring;
	
	std::string s = formater_cstr_buffer;
	return s;
}

const char* ConvFuncs::strupr_convfncs( char* input )
{
	char* p = input;
	while ( *p = toupper(*p)) p++;

	return input;
}

double ConvFuncs::PointPositionToLine( ConvFuncs::float3& P, ConvFuncs::float3& A, ConvFuncs::float3& B )
{
	float3 ba;
	
	ba[0] = B[0] - A[0];
	ba[1] = B[1] - A[1];
	ba[2] = B[2] - A[2];
	
	double t;
	
	t = ( ba[0]*(P[0]-A[0]) + ba[1]*(P[1]-A[1]) + ba[2]*(P[2]-A[2]) ) /
				( ba[0]*ba[0] + ba[1]*ba[1] + ba[2]*ba[2] );
	
	//delim vzdialenostou bodov AB^2. kedze vzdialenost sa sqrt-uje, tak sqrt a ^2 sa vyrusia
	return t;
}

double	ConvFuncs::PointDistanceToLine( ConvFuncs::float3& P, ConvFuncs::float3& A, ConvFuncs::float3& B, double* position )
{
	float3	p1;
	double	d;
	
	d = PointPositionToLine( P, A, B );
	// calculate point representation on the line
	for( int i=0; i<3; i++ )
		p1[ i ] = A[ i ] + ( B[ i ] - A[ i ] ) * d;
	
	if ( position != NULL )
		*position = d;

	return Distance3( p1[ 0 ] - P[ 0 ], p1[ 1 ] - P[ 1 ], p1[ 2 ] - P[ 2 ] );
}

int ConvFuncs::IntersectLines2d( float2& A, float2& Av, float2& B, float2& Bv, float& Ap, float& Bp )
{
	//skusim ci niesu rovnobezne
	if ( Av[ 0 ] * Bv[ 1 ] == Av[ 1 ] * Bv[ 0 ] )
		return 0;
	//skusim ci niesu nulove vektory
	if ( ((Av[ 0 ] == 0) && (Av[ 1 ] == 0)) || ((Bv[ 0 ] == 0) && (Bv[ 1 ] == 0)) )
		return 0;

	Bp = ( (A[ 0 ]-B[ 0 ])*Av[ 1 ] - (A[ 1 ]-B[ 1 ])*Av[ 0 ] ) / (Bv[ 0 ]*Av[ 1 ] - Bv[ 1 ]*Av[ 0 ]);
	if ( Av[ 0 ] != 0 )
		Ap = (B[ 0 ] + Bv[ 0 ] * Bp - A[ 0 ]) / Av[ 0 ];
	else
		Ap = (B[ 1 ] + Bv[ 1 ] * Bp - A[ 1 ]) / Av[ 1 ];

	return 1;
}
/* Teoria(PriesekPriamok):
   //body na priamkach a, b
   xa = ax + avx * k;  xb = bx + bvx * j
   ya = ay + avy * k;  yb = by + bvy * j
   //kedze idem hladat priesek (bod) tak sa tam musia rovnat
   ax + avx * k = bx + bvx * j
   ay + avy * k = by + bvy * j
   //teraz idem hladat k & j. Takze jedno vylucim, podla toho ktore sa da
   *  avy
   * -avx
   //prenasobim to aby sa vylucilo K
   ax * avy + avx * avy * k == bx * avy + bvx * avy * j
   ay *-avx + avy *-avx * k == by *-avx + bvy *-avx * j
   //takze teraz sa oba riadky scitaju spolu, K sa vylucilo
   ax * avy + ay *-avx == bx * avy + by *-avx + bvx * avy * j + bvy *-avx * j
   //teraz su uz vsetko konstanty okrem j(neznama). tak to prevediem na jednu stranu
   (ax*avy + ay*-avx - bx*avy - by*-avx) / (bvx*avy + bvy*-avx) == j
   //zjednodusim
   ( (ax-bx)*avy + (ay-by)*-avx ) / (bvx*avy + bvy*-avx) == j
	ak (bvx*avy + bvy*-avx) == 0 => su rovnobezne. to sa vyluci na zaciatku
   //mam j, potrebujem este k. zoberiem prvu hornu rovnicu a dosadim
   ax + avx * k = bx + bvx * j
   k = (bx + bvx * j - ax) / avx (ak by bolo avx 0, tak pouzijem spodnu)
   ay + avy * k = by + bvy * j
   k = (by + bvy * j - ay) / avy (ak by boli obe 0, tak by bol vektor nulovy)

*/

void ConvFuncs::StdStringToLower( std::string& str )
{
	std::transform( str.begin(), str.end(), str.begin(), static_cast<int (*)(int)>(tolower) );
}

void ConvFuncs::StdStringToUpper( std::string& str )
{
	std::transform( str.begin(), str.end(), str.begin(), static_cast<int (*)(int)>(toupper) );
}

const char* ConvFuncs::FixupPathString( const char *str, bool ending_slash )
{
	static	std::string	result;
	static	std::string::iterator it;
	
	result = str;
	if ( result.empty() )
		return result.c_str();
	
	// fix slashes
	for( it=result.begin(); it!=result.end(); it++ ) {
		if ( *it == path_bad_slash_char )
			*it = path_good_slash_char;
	}
	// remove trailing slashes
	
	it = result.end() - 1;
	if ( *it == path_good_slash_char ) {
		while (( it != result.begin() ) && ( *it == path_good_slash_char ))
			it--;
		result.erase( ++it, result.end() );
	}
	// add one trailing slash
	if ( ending_slash ) {
		result += path_good_slash_char;
	}
	
	return result.c_str();
}

const char* ConvFuncs::FixupPathFilenameString( const char *str )
{
	static	std::string	result;
	static	std::string::iterator it;
	
	result = str;
	if ( result.empty() )
		return result.c_str();
	
	// fix slashes
	for( it=result.begin(); it!=result.end(); it++ ) {
		if ( *it == path_bad_slash_char )
			*it = path_good_slash_char;
	}
	
#ifdef _UNIX
	// in unix remove dot if there is no extension
	it = result.end() - 1;
	if ( *it == '.' )
		result.erase( it );
#endif
	
	return result.c_str();
}

bool ConvFuncs::DirChangeNotificationSubscribe( const char* directory )
{
#ifdef _UNIX
	/*
	if ( m_DirChangeNotificationFileDescriptor >= 0 ) {
		LogS( InfoError, "ConvFuncs::DirChangeNotificationSubscribe() there is already some subscriber, it was unsubscribed now.\n" );
		DirChangeNotificationUnsubscribe();
	}
	
	struct sigaction act;

	act.sa_sigaction = ConvFuncs::DirChangeNotificationInternalHandler;
	sigemptyset( &act.sa_mask );
	act.sa_flags = SA_SIGINFO;
	sigaction( SIGRTMIN + 1, &act, NULL );
	
	m_DirChangeNotificationFileDescriptor = open( directory, O_RDONLY );
	if ( m_DirChangeNotificationFileDescriptor < 0 ) {
		LogS( InfoError, "ConvFuncs::DirChangeNotificationSubscribe() unable to open directory '%s'\n", directory );
		return false;
	}
	fcntl( m_DirChangeNotificationFileDescriptor, F_SETSIG, SIGRTMIN + 1);
	fcntl( m_DirChangeNotificationFileDescriptor, F_NOTIFY, DN_MODIFY|DN_CREATE|DN_DELETE|DN_RENAME|DN_MULTISHOT);
	
	m_DirChangeNotificationHandler = h;
	
	return true;
	*/
	
	if ( m_DirChangeNotificationDirRequestDescriptor != -1 ) {
		LogS( InfoError, "ConvFuncs::DirChangeNotificationSubscribe() there is already some subscriber, it was unsubscribed now.\n" );
		DirChangeNotificationUnsubscribe();
	}
	
	if ( m_DirChangeNotificationInotifyDescriptor == -1 ) {
		m_DirChangeNotificationInotifyDescriptor = inotify_init(); // O_NONBLOCK );
		if ( m_DirChangeNotificationInotifyDescriptor < 0 ) {
			LogS( InfoError, "ConvFuncs::DirChangeNotificationSubscribe() could not init 'inotify'\n" );
			return false;
		}
	
		long	flags;
		
		flags = fcntl( m_DirChangeNotificationInotifyDescriptor, F_GETFL );
		flags |= O_NONBLOCK;
		if ( fcntl( m_DirChangeNotificationInotifyDescriptor, F_SETFL, flags ) == -1 ) {
			LogS( InfoError, "ConvFuncs::DirChangeNotificationSubscribe() could not modify to non-block. Stopping.\n" );
			// I don't want to be blocked, that would hang program.
			DirChangeNotificationUnsubscribe();
			return false;
		}
		
	}
	
	m_DirChangeNotificationDirRequestDescriptor = inotify_add_watch( 
		m_DirChangeNotificationInotifyDescriptor, directory, 
		IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO );
	if ( m_DirChangeNotificationDirRequestDescriptor < 0 ) {
		LogS( InfoError, "ConvFuncs::DirChangeNotificationSubscribe() unable to open directory '%s'\n", directory );
		return false;
	}
	return true;

#endif
#ifdef _WIN_NEDOKONCENE
public:
    FileChangeEvent (char const * folder, BOOL recursive, DWORD notifyFlags)
    {
        _handle = FindFirstChangeNotification (folder, recursive, 
		                             notifyFlags);
        if (INVALID_HANDLE_VALUE == _handle)
            throw WinException ("Cannot create change notification handle");
    }
    ~FileChangeEvent ()
    {
        if (INVALID_HANDLE_VALUE != _handle)
            FindCloseChangeNotification (_handle);
    }

    operator HANDLE () const { return _handle; }
    BOOL ContinueNotification ()
    {
        return FindNextChangeNotification (_handle);
    }

private:
    HANDLE _handle;
};

class FolderChangeEvent : public FileChangeEvent
{
public:
    FolderChangeEvent (char const * folder)
        : FileChangeEvent (folder, FALSE, FILE_NOTIFY_CHANGE_FILE_NAME)
    {}
};

class TreeChangeEvent : public FileChangeEvent
{
public:
    TreeChangeEvent (char const * root)
        : FileChangeEvent (root, TRUE,
                           FILE_NOTIFY_CHANGE_FILE_NAME
                         | FILE_NOTIFY_CHANGE_DIR_NAME)
    {}
#endif

	return false;
}

bool ConvFuncs::DirChangeNotificationUnsubscribe( void )
{
#ifdef _UNIX
	/*
	if ( m_DirChangeNotificationFileDescriptor < 0 )
		return true;
	
	close( m_DirChangeNotificationFileDescriptor );
	m_DirChangeNotificationFileDescriptor = -1;
	
	sigaction( SIGRTMIN + 1, NULL, NULL );
	
	m_DirChangeNotificationHandler = NULL;
	
	return true;
	*/

	if ( m_DirChangeNotificationInotifyDescriptor == -1 )
		return true;
	
	if ( m_DirChangeNotificationDirRequestDescriptor != -1 ) {
		inotify_rm_watch( m_DirChangeNotificationInotifyDescriptor, m_DirChangeNotificationDirRequestDescriptor );
		m_DirChangeNotificationDirRequestDescriptor = -1;
	}
	
	if ( m_DirChangeNotificationInotifyDescriptor != -1 ) {
		close( m_DirChangeNotificationInotifyDescriptor );
		m_DirChangeNotificationInotifyDescriptor = -1;
	}
	
	return true;
#endif
	return false;
}

/*
void ConvFuncs::DirChangeNotificationInternalHandler( int sig, siginfo_t *si, void *data )
{
	if ( ConvFuncs::m_DirChangeNotificationHandler != NULL ) {
		ConvFuncs::m_DirChangeNotificationHandler();
	} else {
		LogS( InfoError, "ConvFuncs::DirChangeNotificationInternalHandler() notified about change, although noone subscribed\n" );
	}
	
}
*/

bool ConvFuncs::DirChangeNotificationCheck( s_DirChangeNotificationCheckStatus& status )
{
#ifdef _UNIX
	// return next item from array if possible
	if ( ! m_DirChangeNotificationsArray.empty() ) {
		status = m_DirChangeNotificationsArray[ 0 ];
		m_DirChangeNotificationsArray.erase( m_DirChangeNotificationsArray.begin() );
		return true;
	}
	
	static const int buf_size = 1024;
	inotify_event	*pevent;
	char		buf[ buf_size ];
	int		len, l, pos;
	s_DirChangeNotificationCheckStatus		s1;
	
	s1.event_received = false;
	s1.dir_files_changed = false;
	
	// check if there is something in stream
	pollfd	pfd;
	
	pfd.fd = m_DirChangeNotificationInotifyDescriptor;
	pfd.events = POLLIN;
		
	if ( poll( &pfd, 1, 0 ) <= 0 ) {
		
		status.event_received = false;
		status.dir_files_changed = false;
		status.filename = "";
		status.action = dcnfs_unknown;
		
		return false;
	}
	
	// add new items to array
	s1.event_received = true;
	s1.dir_files_changed = true;
	pos = 0;
	len = 0;
	do {
		// read() is messed up, it can't read only few bytes, it has to read all at once.
		//len = read( m_DirChangeNotificationInotifyDescriptor, &event, sizeof( inotify_event ) );
		
		l = read( m_DirChangeNotificationInotifyDescriptor, & buf[ len ], buf_size-1 );
		/*
		if ( (int) l < (int) sizeof( inotify_event ) ) {
			int err = errno;
			return false;
		}
		*/
		len += l;
		if ( l <= 0 )
			break;
		
		while( pos + sizeof( inotify_event ) < len ) {
			pevent = (inotify_event*) &buf[ pos ];
			pos += sizeof( inotify_event );
			s1.filename = "";
		
			// see for filename
			if ( pevent->len > 0 ) {
				s1.filename = &buf[ pos ];
				pos += pevent->len;
			}
		
		//char *pathname                  = find_path_name(list, pevent->wd);
		// free(pathname);
		
		//if (pevent->len)
		//strncpy (action, pevent->name, ACTION_LENGTH -1);
		//else
		//strncpy(action, pathname, FILENAME_MAX -1);
		
			if ( pevent->mask & ( IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVE_SELF | IN_MOVED_FROM | IN_MOVED_TO )) {
		
			//printf ("Action: [%s]\n\twd=%d mask=%d cookie=%d len=%d dir=%s ", 
			//action, pevent->wd, pevent->mask, pevent->cookie, pevent->len, (pevent->mask & IN_ISDIR) ? "yes":"no");
				if ( pevent->mask & ( IN_CREATE | IN_MOVED_TO ))
					s1.action = dcnfs_created;
				if ( pevent->mask & ( IN_DELETE | IN_MOVED_FROM ))
					s1.action = dcnfs_deleted;
				
				m_DirChangeNotificationsArray.push_back( s1 );
			}
		};
		
		// move rest of buffer to the start.
		memcpy( buf, & buf[ pos ], len - pos );
		len = len - pos;
		pos = 0;
		
	} while( 1 );
	
	if ( ! m_DirChangeNotificationsArray.empty() ) {
		status = m_DirChangeNotificationsArray[ 0 ];
		m_DirChangeNotificationsArray.erase( m_DirChangeNotificationsArray.begin() );
		return true;
	} else
		return false;
#endif

	return false;
}

//! Gets words from the string into array.
//! Does not clear tokens array.
void ConvFuncs::Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters )
{
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

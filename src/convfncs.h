#ifndef _convfncs_h_
#define _convfncs_h_

// su tu rozne pomocne funkcie
//  a taktiez Leveling a Form

// define either _DOS, _WIN32, _MAC, or _UNIX

// use defines:
#define CONV_FNCS_CONV_FILE_USED
//#define CONV_FNCS_FORM_INCLUDED
//#define CONV_FNCS_LEVELING_INCLUDED

#include <string>
#include <vector>

#if ( defined( _WIN32 ) || defined( _DOS ))
/*
	#ifdef _WIN32
		#if ( _MFC_VER == 1400 )
			#include <afxwin.h>
		#else
			// 1500, VC9 Expr.
			#define WIN32_LEAN_AND_MEAN
			#include <windows.h>
		#endif
*/
	#ifdef _MFC
		#include <afxwin.h>
	#else
		#include <dos.h>
		
		// #include <stdint.h> // needed by stdlib.h - not present in WIN
		#include <stdlib.h>
	#endif
	#include <io.h>
#else
	#include <ctime>
	#include <dirent.h>
#endif

#ifdef _UNIX
//	#define srand std::srand
	#include <cstdlib>

	// for DirChangeNotification functionality
	#include <signal.h>
#endif


#ifdef CONV_FNCS_CONV_FILE_USED
#include "convfile.h"
using namespace ConvFile;
#endif

#if _WIN32
	#define PATH_DELIMITER	'\\'
#else
	#define PATH_DELIMITER	'/'
#endif
typedef long fract;

#ifdef CONV_FNCS_FORM_INCLUDED
#define FORM 0x4D524F46l
// "FORM" <- for back compatibility to dos cpp, where ulong cant be written as 'FORM' (four chars)
#endif

#ifndef BBL_MaxFileNameLen
#define BBL_MaxFileNameLen 512
#endif

#define ENDIAN_INTEL yes
//#define ENDIAN_MOTORLA yes

#ifndef snprintf
	#ifdef _WIN32

		#define snprintf _snprintf
		#define vsnprintf _vsnprintf
	#endif
#endif

#ifndef stricmp

	#ifdef _MAC
		#define stricmp(a, b) strcasecmp((a),(b))
	#endif
	
	#ifdef _UNIX
		#include <strings.h>
		#define stricmp(a, b) strcasecmp((a),(b))
	#endif

#endif

#ifndef strupr
	#define strupr ConvFuncs::strupr_convfncs
#endif

// compilers other than MSVS do not contain function "string to upper"
#ifndef _MSC_VER // strupr

#ifdef _UNIX
	#include <ctype.h>
#endif

#endif

#ifdef CONV_FNCS_CONV_FILE_USED
#include "convfile.h"
#endif

class ConvFuncs {
public:

	ConvFuncs();
	~ConvFuncs();

	void Reset( void );

	static const double RAD;

		//util for reading line from text file
	#define MaxLineLen 1000

	enum e_byte_ordering {
		byte_ordering_motorla,
		byte_ordering_intel,
		byte_ordering_undefined,
	};

	char			Line[ MaxLineLen ];

private:

#if ( defined _WIN32 && defined _MFC )
	SYSTEMTIME		CheckTimeStart, CheckTimeNow;
#endif
#ifdef _UNIX
	timeval 		CheckTimeStart, CheckTimeNow;
	double			CheckTimeStartD;
#endif
#ifdef _WIN32
	_finddata_t		FindFileData;
#endif

#if ( defined( _MAC ) || defined( _UNIX ))
	DIR*			DirPtr;
	struct dirent	DirEntry;
	struct s_find_file_pattern {
		bool		any;
		bool		asterisk_start;
		bool		asterisk_end;
		bool		asterisk_center;
		bool		full_name;
		bool		case_sensitive;
		std::string		str;
		std::string		str2;
		
		void Reset( void ) {
			any = true;
			asterisk_start = false;
			asterisk_end = false;
			asterisk_center = false;
			full_name = false;
			case_sensitive = true;
			str = "";
		}
	} m_find_file_pattern;
#endif

	long			FindFileHandle, FindFileResult, FindFileFirstFile;
	enum {
		DateTimeString_len = 512,
	} ;
	static char		DateTimeString[ DateTimeString_len ];
	int				ActualByteOrdering, SwapSourceOrdering;
public:

	//math-geometry
//	void			Rotate( double *x, double *y, double *z, double ux, double uy, double uz );
	static void		RotateHPR( double *x, double *y, double *z, double h, double p, double r );
	static void		RotateNewHPR( double *x, double *y, double *z, double h, double p, double r );
	static void		RotateZXY( double *x, double *y, double *z, double rx, double ry, double rz );
	static void		RotateH( double *x, double *y, double h );
	static void		VektorNaJednotkovy( double *vx, double *vy, double *vz );
	static void		TriangleNormal( double *rx, double *ry, double *rz, double ax, double ay, double az,
									double bx, double by, double bz, double cx, double cy, double cz );
	static double	Distance3( double x, double y, double z );
	static double	VectorLength2( double x, double y );
	static void		PointAngle3d( double x, double y, double z, double *h, double *p );
	static double	PointAngle2d( double x, double y );
	static double	AngleIntoBound360( double a );
	static double	Angle3Points2d( double ax, double ay, double xx, double xy, double bx, double by );
	//returns nearest equal or bigger number which is 2 ^ x (where x is integer number )
	static unsigned long	FixNumTo2n( unsigned long in_num );
	static int		PointInRectLong( long px, long py, long x1, long y1, long x2, long y2 );

/*
        vzdialenost bodu od priamky v priestore:
	bod P, priamka dana (bodmi A, B) == (bodom A a vektorom q)
	t1 = ( (Bx - Ax)*(Px - Ax) + (By-Ay)*(Py-Ay) + (Bz-Az)*(Pz-Az) ) / ( |AB| * |AB| )
	kolmy priemet bodu P na usecku AB,  I = A + q*t1;
*/
	//! Line is defined by its start point A and end point B
	//! Point is defined by point position P
	
	typedef			float	float3[3];
	typedef			float	float2[2];
	//! you can calculate point's image on the line, where the image is closest point on the line to the point P
	//! image's position is A + ( B - A ) * returned value.
	double 			PointPositionToLine( float3& P, float3& A, float3& B );
	//! returns distance from point to line
	//! position : if not NULL its filled with value returned from PointPositionToLine()
	double 			PointDistanceToLine( float3& P, float3& A, float3& B, double* position = NULL );
	
	//! Input: two lines defined by point and vector
	//! Output: intersection of lines, point of intersection is [A+Av*Ap] and also [B+Bv*Bp]
	//! Intersection position on line A is a multiply Ap of vector Av
	//! on line B it is multiply Bp of vector Bv
	//! If Ap or Bp is in range = <0..1>, then one line intersect segment of other line
	//! Returns 1 if lines are intersecting, 0 if parallel or null vector
	int IntersectLines2d( float2& A, float2& Av, float2& B, float2& Bv, float& Ap, float& Bp );

	//file reading

	//reads string from file until zero char, and fills it into [str] as
	// much as can fit.
	//return: size of data red including zero char
	unsigned short	ReadAscizFromBinary( FILE *fin, char *str, unsigned short maxlen );
	char*			ReadWord( FILE *fin );
	void			ReadLine( FILE *file );
	void			ReadLineSkip( FILE *file );
	void			ReadLineFull( FILE *file );
	unsigned long	SkipBigData( unsigned long size, FILE *file );
#ifdef CONV_FNCS_CONV_FILE_USED
	unsigned short	ReadAscizFromBinary( CConvFile *file, char *str, unsigned short maxlen );
	void			ReadLine( CConvFile *file );
	void			ReadLineSkip( CConvFile *file );
	void			ReadLineFull( CConvFile *file );
	unsigned long	SkipBigData( unsigned long size, CConvFile *cfile );
#endif

	//file utils
	char*			CreateUniqueFilename( char *path );
	const char*			FindOniFileById( unsigned long id, char* path );

	//! Finds first file in the directory
	//! You have to search with search pattern, for example: "c:/dir/directory/*.txt"
	//! Pattern can be "*txt" or "asdf*", "*text*" or "text*ext". To look for all files use "*" or "*.*" or ""
	//! for example "/home/user/".
	//! @returns true if respective file found, false if not. Use other Find_* functions to get 
	//! properties of found files/directories and to retrieve next files/directories
	//! use: res = Find_FirstFile( "/directory/*" ); while( res ) { ... use ... ; res = Find_NextFile(); }
	//!
	//! comment: changed name of functions from FindFirstFile to Find_FirstFile, because 
	//! MSVC 9.0 express has some defines called FindFirstFile and FindNextFile which caused conflicts
	bool			Find_FirstFile( const char* dir_with_filename );
	//! finds every next file in the search
	//! returns true if found
	bool			Find_NextFile( void );
	//! use functions below to retrieve attributes of found file. Call these only after
	//! successful calls to Find_FirstFile() or Find_NextFiles()
	//! returns filename without path.
	const char*		Find_CurrentFileName( void );
	int				Find_CurrentIsDir( void );
	int				Find_CurrentIsReadOnly( void );

	//! returns string filled with full path and filename of executable
	//! works for unix and win32
#ifdef UNICODE
	const wchar_t*	GetExecutableFilePathName( void );
#else
	const char* GetExecutableFilePathName( void );
#endif

	// moves file to different directory and renames file
	//! comment: changed name of functions because MSVC 9.0 express has some 
	//! defines which caused conflicts
	//! returns 0 on failure.
	static int			DoMoveFile( const char* src, const char* dest );
	static int			DoDeleteFile( const char* src );

	//! Copies file from source to destination file.
	//! Returns true if successful
	static bool DoCopyFile( std::string src, std::string dest );
	
	//! Creates a folder in specified path
	static bool	DoCreateFolder( std::string parentPath, std::string folderName );
	static bool DoCreateFolder( std::string fullPath );

	static bool DoCreateFolder( const char* fullPath);
	
	//string utils
#ifdef _UNIX
	static const char path_good_slash_char = '/';
	static const char path_bad_slash_char = '\\';
#else
	static const char path_good_slash_char = '\\';
	static const char path_bad_slash_char = '/';
#endif
	
	
	/*returns [num]th item (or word) from string
	 if [num]th item is out of string, '\0' is returned
	example: str ="aa bb cde fdg", num = 2, return = "cde"
	if pos is defined, there is returned position in string right after this item (points to first whitespace after it)
	if separators string is not defined, default are used ( space, eol, tab )
	Result length is limited by [ResultMaxLen]
	*/
	char*			StrItem( const char *str, char num, const char *separators_pnt = NULL, unsigned short *pos = NULL );
	
	//! Gets words from the string into array.
	//! Does not clear tokens array.
	static void Tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters = " ");
	
	// returns 1 if string represents a number (int or float)
	// returns 0 otherwise
	int				IsNumber( const char *text );
	void			RemoveRemarkFromLine( char *remark_str );
	void			RemoveLineLeadSpaces( void );
	char*			GetNameFromQuotes( char *str );// if str=<bla ks"aejt aswe"qasdf> it returns <aejt aswe>
	const char*		GetFileFromPathFilename( const char *str );
	// returns path with ending slash
	const char*		GetPathFromPathFilename( const char *str );
	//! fixes path string - removes extra ending slashes etc.
	//! function does not check if file or directory exists, it only modifies the string
	const char*		FixupPathString( const char *str, bool ending_slash );
	//! fixes file string - corrects slashes, removes extra ending dot if no extension on Unix etc.
	//! function does not check if file or directory exists, it only modifies the string
	const char*		FixupPathFilenameString( const char *str );
	char*			RemoveLeadingAndTrailingChars( char* str, char* chars );
	//return: no of replaced chars
	int				StrReplaceChars( char *str, const char* oldchars, const char *newchars );
	//return: no of replaced chars
	int				StrReplaceChars( std::string& str, const char* oldchars, const char *newchars );
	//StrCpyLen is safer then strcpy() because it checks length of destination buffer
	//return: length of copied string
	static int		StrCpyLen( char* dest, const char* source, int dest_size );
	// returns pointer to local buffer holding result string.
	static const char*			FormatCstr( const char* format, ... );
	// returns reference to local buffer holding result string.
	static const std::string	FormatStdStr( const char* format, ... );

	static const char* strupr_convfncs( char* input );

	static void		StdStringToLower( std::string& str );
	static void		StdStringToUpper( std::string& str );
	
	//various utils
	int				WaitForKey( void );
	void			StartCheckTime( void );
	int				CheckTime( float seconds );
	void			Swap( void* data, int LengthInBytes );
	// sets source ordering for Swapso function data;
	// source_ordering: use byte_ordering_motorla or byte_ordering_intel
	void			SwapsoSetSourceOrdering( int source_ordering );
	// calls Swap, depending on data source_ordering and actual ordering,
	// which depends on machine where it was compiled
	void			Swapso( void* data, int LengthInBytes );
	unsigned long	SwapULong( unsigned long in );
	long			SwapSLong( long in );
	unsigned short	SwapShort( unsigned short in );
	fract			Float2Fract( double f );
	double			Fract2Float( fract f );
	unsigned long	FindTwoSame( unsigned long a1, unsigned long a2, unsigned long b1, unsigned long b2 );
	// calls function GetDateTime() to fetch current time. Formats the date 
	// and time into string. returns pointer to DateTimeString
	static char*	CurDateTimeString( void );
	
	// retrieves current date and time and fills it into structure [datetime]
	static void		GetDateTime( void );

	struct s_datetime {
		int		year;		// year since year 0
		int		month;		// 0 - 11
		int		day;		// 1 - 31
		
		int		hour;		// 0 - 23
		int		minute;		// 0 - 59
		int		second;		// 0 - 59
		int		msec;		// 0 - 999
	
		int		dayofweek;	// 0-6, -1 if unknown
		int		dayofyear;	// 0-365, -1 if unknown
	
		void Reset( void )
		{ 
			year = month = day = hour = minute = second = msec = 0;
			dayofweek = dayofyear = -1;
		}
		
		s_datetime() { Reset(); }
	} ;

	static	s_datetime		datetime;

	// ---- leveling ----

	#define			MaxLevels	250
	#define			MaxLevelIdentLen 40

#ifdef CONV_FNCS_LEVELING_INCLUDED
private:
	char			LevelIdent[ MaxLevels ][ MaxLevelIdentLen ];
	long			LevelLong[ MaxLevels ];
public:
	long			LevelLen[ MaxLevels ];
	int				level;
	char			LevelSpace[ 100 ];

	void			MakeLevelString( void );
	void			NewLevel( unsigned long length );
	void			NewLevelAddToSublevels( unsigned long length );
	void			LevelRead( unsigned long length );
	void			SetLevelIdent( char *ident );		//call before NewLevel
	void			SetLevelLong( long ident );			//call before NewLevel
	int				IsLevelIdent( char *ident, int relative = -1 );
	char*			GetLevelIdent( int relative = -1 );
	long			GetLevelLong( int relative = -1 );
	void			ResetLeveling( void );
#endif

	// -- buffer --
	enum {
		formater_string_buffer_len = 512,
	} ;

	static char		formater_cstr_buffer[ formater_string_buffer_len ];
	static std::string	formater_stdstring;

	// --- form ----
#ifdef CONV_FNCS_FORM_INCLUDED

	//header structure
#pragma pack( push )
#pragma pack( 1 )

	struct {
		union {
			char			Ident[4];
			unsigned long	IdentLong;
		} ;
		unsigned long	Length;
	} FormChunkHead;

#pragma pack( pop )

	unsigned long	FormChunkHeadEvenLength;
	unsigned char	FormOne;			//buffer, for reading one byte
	int				FormSkipOddByte;
	//data length in file is always even, when .Length is odd one byte is added
	//some forms don't skip one even byte when data length is odd (set FormSkipOddByte = 0)

	int				FormByteOrdering;

	// writing fncs
	#define MaxFormChunkLevel 20

	struct {
		unsigned long	Offset;
		unsigned long	DataType;
	} FormChunkLevel[ MaxFormChunkLevel ];

	unsigned short	FormWritingLevel;

	int				FormReadChunkHead( FILE *fin );
	void			FormSkipOdd( FILE *fin );
	int				FormSkipChunkData( FILE *fin );
	void			FormChunkStart( FILE *fout, unsigned long chunk_type );
	void			FormChunkFinished( FILE *fout );

#ifdef CONV_FNCS_CONV_FILE_USED
	int				FormReadChunkHead( CConvFile *cfin );
	void			FormSkipOdd( CConvFile *cfin );
	int				FormSkipChunkData( CConvFile *cfin );
	void			FormChunkStart( CConvFile *cfout, unsigned long chunk_type );
	void			FormChunkFinished( CConvFile *cfout );
#endif

#endif // #ifdef CONV_FNCS_FORM_INCLUDED

	//! subscribe for notifications about change of files in the directory. Your program can subscribe only
	//! one handler for the notification. If you want to watch for different directory then unsubscribe and
	//! subscribe again for new directory.
	//@returns: true on success, false on failure. Reason of failure is printed into logs.
	bool DirChangeNotificationSubscribe( const char* directory );
	//@returns: true on success, false on failure. Reason of failure is printed into logs.
	bool DirChangeNotificationUnsubscribe( void );
	
	enum e_DirChangeNotificationFileStatus {
		dcnfs_unknown,
		dcnfs_created,
		dcnfs_deleted,
	} ;
	
	struct s_DirChangeNotificationCheckStatus {
		// if true there was notification that is received, it is possible 
		// that there is another notification waiting 
		bool		event_received;
		// if true then directory content have changed (there is some file created or deleted)
		bool		dir_files_changed;
		// name of the file that was changed
		std::string		filename;
		// actions on the file
		e_DirChangeNotificationFileStatus				action;
	} ;
	
	std::vector< s_DirChangeNotificationCheckStatus >	m_DirChangeNotificationsArray;
	
	//! returns true if any event was received and there might be another event in queue
	//! in that case [status] is filled with notification data.
	//! if there is nothing new 'false' is returned.
	bool DirChangeNotificationCheck( s_DirChangeNotificationCheckStatus& status );
	
private:
	int			m_DirChangeNotificationInotifyDescriptor;
	int			m_DirChangeNotificationDirRequestDescriptor;

//! in case you want to use only single instance of ConvFncs then use this singleton mechanism
private:
	static ConvFuncs	*pSingleton;
	static int 			m_singleton_init_counter;
public:
	static void SingletonInstanceCreate() 
		{ if ( pSingleton == NULL ) pSingleton = new ConvFuncs; m_singleton_init_counter++; }
	static void SingletonInstanceDelete() 
		{ if ( m_singleton_init_counter == 0 ) return; m_singleton_init_counter--; 
			if ( m_singleton_init_counter == 0 ) { delete pSingleton; pSingleton = NULL; } }
	inline static ConvFuncs*	SingletonInstanceGet() { return pSingleton; }

} ;

// shortcut macro
#define ConvFuncsInstance ConvFuncs::SingletonInstanceGet()

#endif

#ifndef _logging_h_
#define _logging_h_

// -- Biturn logging header file                    --
// --  created by mirex                             --
// -- Do not make any changes to this file          --

// can be used for logging of progress of loading / saving / conversion

// define LOG_INTO_WIN32_CONTROLS if you want to have have printouts either
// to standard output or to WIN32 controls.
// define LOG_INTO_GTKMM_CONTROLS if you want to specify output to 
// GTKmm controls.

/*	logging.
	First set logging mood by SetLoggingMood() and
	set logging file by SetLogFileName()

	and then use Log() or LogS() functions

	major info (starting conversion, errors) write to both, 
	minor info (conversion progress) only to log
*/

// #define LOG_INTO_WIN32_CONTROLS
//#define LOG_INTO_GTKMM_CONTROLS

#ifdef LOG_INTO_WIN32_CONTROLS
#include <afxwin.h>
#include <afxcmn.h>
#endif

#ifdef LOG_INTO_GTKMM_CONTROLS
#include <gtkmm.h>
#endif

#include <string>

// ===    global functions, that are not encapsulated in class, are used by whole program  ===

void Log( const char *text, int type );
void Log( int type, const char *text );
void LogS( int type, const char *format, ... );

#ifdef _DEBUG
#define LogDebug( a ) Log( a, InfoMajor )
#else
#define LogDebug( a )
#endif

#define InfoMajor BiturnLog::LogInfoMajor
#define InfoMinor BiturnLog::LogInfoMinor
#define InfoWarn  BiturnLog::LogInfoWarn
#define InfoError BiturnLog::LogInfoError
#define InfoDebug BiturnLog::LogInfoDebug

class BiturnLog;

BiturnLog* BiturnLogInstance( void );


class BiturnLog {

public:

	enum {
		LogWindowLinesMax		= 400,
		LogStrLength			= 4096,

		toNone					= 0,
		toLog					= 1,
		toScreen				= 2,
		toBoth					= ( toLog + toScreen ),

		LoggingMoodNothing		= 0,	// nothing
		LoggingMoodLight		= 1,	// major & error logs to screen
		LoggingMoodNormal		= 2,	// all to screen, errors to file
		LoggingMoodHeavy		= 3,	// all to screen & file 
		LoggingMoodDebug		= 4,	// all to file

		LogInfoMinor		= 21,
		LogInfoMajor		= 22,
		LogInfoError		= 23,
		LogInfoDebug		= 24,
		LogInfoWarn			= LogInfoMinor,

		log_max_details_list_rows	= 300,
		MaxLogFileNameLen			= 512,
		LastLogLinesCount			= 2,
		LastLogLineLength			= 1024,
	} ;

	char			last_log_line[ LogStrLength ];

	void			SetLogFileName( const char* name );
	void			ResetLogFile( void );

	// type = one of InfoMinor / InfoMajor / InfoError / InfoDebug states
	void			Log( const char *text, int type );
	void			LogS( int type, const char *format, ... );
	void			LogV( int type, const char *format, va_list list );
	void			ClearLog( void );

	// mood = one of LoggingMoodNothing / LoggingMoodLight / LoggingMoodNormal / LoggingMoodHeavy / LoggingMoodDebug
	void			SetLoggingMood( int mood );
	int				GetLoggingMood( void );
	
	// can be used for displaying of logs in win32 environment
#ifdef LOG_INTO_WIN32_CONTROLS
	void			SetLogOutputWnd( CEdit	*wnd );
	void			SetLogOutputWnd( CStatic *wnd );
	void			SetLogOutputWnd( CListCtrl *wnd );
#endif
#ifdef LOG_INTO_GTKMM_CONTROLS
	void			SetLogOutputWnd( Gtk::TextView *view );
#endif
	char*			GetLastLogLine( void );


	static BiturnLog*	GetSingletonInstance( void );
	static void DeleteSingletonInstance( void );

	BiturnLog() {
		LoggingSucces = 1;
		LogFileName = "xx.log";
		LoggingMood = LoggingMoodLight;

		last_log_line[ 0 ] = 0;
		
		for( int i=0; i<LastLogLinesCount; i++ )
			LastLogLine[ i ][ 0 ] = 0;


#ifdef LOG_INTO_WIN32_CONTROLS
		LogOutputWndOneLine		= 0;
		LogOutputWnd			= NULL;
#endif
#ifdef LOG_INTO_GTKMM_CONTROLS
		LogOutputTextView		= 0;
#endif
		
	}

private:
	static BiturnLog*	instance;

	int				LoggingMood;		//LoggingMoods
	int				LogOutputType;

	int				LoggingSucces;

	std::string		LogFileName;

	char			LastLogLine[ LastLogLinesCount ][ LastLogLineLength ];


#ifdef LOG_INTO_WIN32_CONTROLS
	static union {
		CWnd*			LogOutputWnd;
		CEdit*			LogOutputWndEdit;
		CStatic*		LogOutputWndStatic;
		CListCtrl*		LogOutputWndListCtrl;
	} ;

	int				LogOutputWndOneLine;
#endif
#ifdef LOG_INTO_GTKMM_CONTROLS
	Gtk::TextView*		LogOutputTextView;
	
	Glib::RefPtr< Gtk::TextBuffer::Tag > m_TextBufferTag_Minor;
	Glib::RefPtr< Gtk::TextBuffer::Tag > m_TextBufferTag_Major;
	Glib::RefPtr< Gtk::TextBuffer::Tag > m_TextBufferTag_Error;
	
	Glib::RefPtr<Gtk::TextBuffer::TagTable>	m_TextBufferTagTable;
#endif
		
	char			logstr[ LogStrLength ];


} ;


#endif

#include <stdio.h>
#include <assert.h>

#include <stdarg.h>
// needed for _UNIX gcc
#include <time.h>

#include "log.h"

#ifdef _MSC_VER

#ifndef vsnprintf
#define vsnprintf _vsnprintf
#endif

#endif

#ifdef _WIN32
	// 1500, VC9 Expr. - lean and mean needed in express
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	// bugfix to be able to use gtkmm 
	#undef max
#endif

BiturnLog* BiturnLog::instance = NULL;

#ifndef true
	#define true 1
	#define false 0
#endif

void Log( const char *text, int type )
{
	BiturnLog::GetSingletonInstance()->Log( text, type );
}

void Log( int type, const char *text )
{
	BiturnLog::GetSingletonInstance()->Log( text, type );
}

void LogS( int type, const char *format, ... )
{
	va_list		l;

	va_start( l, format );

	BiturnLog::GetSingletonInstance()->LogV( type, format, l );
}

BiturnLog* BiturnLog::GetSingletonInstance( void )
{
	if ( instance == NULL ) {
		instance = new BiturnLog;
	}

	return instance;
}

void BiturnLog::DeleteSingletonInstance( void )
{
	if ( instance != NULL ) {
		delete instance;
		instance = NULL;
	}
}

void BiturnLog::ResetLogFile( void )
{
	FILE	        *flog;

    flog = fopen( LogFileName.c_str(), "wt" );
    if ( flog == NULL )
        LoggingSucces = 0;
    else {
#ifdef _WIN32
		SYSTEMTIME		time;
//        fprintf( flog, "Conv by MireX %i.%iW\n", VerMajor, VerMinor );
//        fprintf( flog, "Conv by MireX W\n" );
		GetSystemTime( &time );
		fprintf( flog, "Log reset on %i.%i.%i %02i:%02i\n",
			time.wDay, time.wMonth, time.wYear,
			time.wHour, time.wMinute );
#endif
#ifdef _DOS
//        fprintf( flog, "Conv by MireX %i.%iD\n", VerMajor, VerMinor );
        getdate( &d );
        gettime( &t );
        fprintf( flog, "Log reset on %i.%i.%i %02i:%02i\n",
                  d.da_day, d.da_mon, d.da_year,
                  t.ti_hour, t.ti_min );
#endif
#ifdef _UNIX
	time_t	tim;
	tm		*t;

	tim = time( NULL );
    t = localtime( &tim );

	fprintf( flog, "Log reset on %i.%i.%i %02i:%02i\n",
		t->tm_mday,
		t->tm_mon,
		t->tm_year + 1900,
		t->tm_hour, t->tm_min );
#endif

        fclose( flog );
    }

}

void BiturnLog::Log( const char *text, int type )
{
	assert( (type >= InfoMinor) && (type <=InfoDebug) );

	FILE	*flog;
	int		whereto = toNone;
	unsigned int	i, j;
	int				lines;

	//choose where to
	switch ( LoggingMood ) {
	case LoggingMoodNothing:
		whereto = toNone;
		break;
	case LoggingMoodLight:
		if (( type == InfoMajor ) || ( type == InfoError )) 
			whereto = toBoth; 
		else
			whereto = toNone;
		break;
	case LoggingMoodNormal:
		if ( type == InfoError ) 
			whereto = toBoth; 
		else
			if ( type == InfoDebug )
				whereto = toNone;
			else
				whereto = toScreen;
		break;
	case LoggingMoodHeavy:
		whereto = toBoth;
		break;
	case LoggingMoodDebug:
		whereto = toLog;
		break;
	default:
		assert( false );
		break;
	}

	//log to file
	if ( whereto & toLog ) {
		flog = fopen( LogFileName.c_str(), "at+" );
		
		if ( flog == NULL )
			LoggingSucces = 0;
		else {
			fprintf( flog, text );
			fclose( flog );
		}
	}

	//display to screen
	if ( whereto & toScreen ) {
#ifdef LOG_INTO_WIN32_CONTROLS
	#ifdef WIN32
		CString			cstr;
		CString			TextBuf;
		unsigned int	i, j;
		int				lines, last; 
		unsigned long	color;
		CSize			csz;

		if ( LogOutputWnd != NULL ) {

			if ( LogOutputType == 1 ) {
				cstr = "";
				for( i=0; i<strlen( text ); i++ )
					if ( text[ i ] == '\n' ) {
						cstr += "\xD\xA";
					}
					else
						cstr += text[ i ];
				LogOutputWnd->GetWindowText( TextBuf );

				if (( LogOutputWndOneLine == 1 ) &&
					( TextBuf.Mid( TextBuf.GetLength() - 2, 2 ) == "\xD\xA" ))
					TextBuf = "";

				TextBuf += cstr;
				LogOutputWnd->SetWindowText( TextBuf.GetBuffer(0) );

				if ( LogOutputWndOneLine == 0 ) {
					lines = LogOutputWndEdit->GetLineCount();
			
					if ( lines > LogWindowLinesMax ) {
						i = 0;
						while ( lines-- > LogWindowLinesMax/2 ) {
							i = TextBuf.Find( "\xD\xA", i ) + 2;
						} ;
						TextBuf.Delete( 0, i );
						LogOutputWndEdit->SetWindowText( TextBuf.GetBuffer(0) );
					}

					LogOutputWndEdit->LineScroll( LogOutputWndEdit->GetLineCount() );
				}

				LogOutputWnd->GetParent()->UpdateData( false );
			}
//			AfxGetMainWnd()->UpdateData( false );

			if ( LogOutputType == 2 ) {
				// m_ListDetails
				if ( type == InfoMinor )
					color = RGB( 150, 150, 150 );
				if ( type == InfoMajor )
					color = RGB( 150, 150, 255 );
				if ( type == InfoError )
					color = RGB( 250, 100, 100 );

				last = ( strlen( last_log_line ) > 0 );
				lines = 0;

				for( i=0; i<strlen( text ) + 1; i++ ) {

					if (( text[ i ] == '\n' ) || ( text[ i ] == 0 )) {

						if ( strlen( last_log_line ) == 0 )
							continue;

						j = LogOutputWndListCtrl->GetItemCount();
						if ( last ) {
							j--;
							LogOutputWndListCtrl->SetItemText( j, 0, last_log_line );
							last = 0;
						}
						else
							LogOutputWndListCtrl->InsertItem( j, last_log_line );
						
						LogOutputWndListCtrl->SetItemData( j, color );
						
						if ( text[ i ] != 0 )
							last_log_line[ 0 ] = 0;
						
						lines++;
					}
					else {
						j = strlen( last_log_line );
						last_log_line[ j++ ] = text[ i ];
						last_log_line[ j ] = 0;
					}
				}

//				csz.cx = 0; csz.cy = 16 * lines;
//				LogOutputWndListCtrl->Scroll( csz );
				LogOutputWndListCtrl->EnsureVisible( 
					LogOutputWndListCtrl->GetItemCount() - 1, false );


				i = LogOutputWndListCtrl->GetItemCount();
				while ( i > log_max_details_list_rows ) {
					LogOutputWndListCtrl->DeleteItem( 0 );
					i--;
				}
			}
		}

	#else
		printf( text );
	#endif
#endif
	
#ifdef LOG_INTO_GTKMM_CONTROLS

		// remove non-utf8 chars
		char	text2[ 512 ];
		strncpy( text2, text, 511 );
		text2[ 511 ] = 0;
		for( char* cp = text2; *cp != 0; cp++ ) {
			if ((( *cp < 32 ) || ( *cp > 127 )) && ( *cp != 0xD ) && ( *cp != 0xA ))
				*cp = '?';
		}

		if ( LogOutputTextView != NULL ) {
			
			// Log can be called from various threads. So lock up.
			gdk_threads_enter();
			
			Glib::RefPtr< Gtk::TextBuffer >	buf;
			buf = LogOutputTextView->get_buffer();
			
			Gtk::TextBuffer::iterator	iter;
			int	lines;
			
			// erase text if there are too many lines
			lines = buf->get_line_count();
			
			if ( lines > LogWindowLinesMax ) {
				iter = buf->get_iter_at_line( LogWindowLinesMax / 2 );
				buf->erase( buf->begin(), iter );
			}
			
			// add new text
			iter = buf->end();
			switch( type ) {
			case InfoMinor:
				buf->insert_with_tag( iter, text2, m_TextBufferTag_Minor );
				break;
			case InfoMajor:
				buf->insert_with_tag( iter, text2, m_TextBufferTag_Major );
				break;
			case InfoError:
			default:
				buf->insert_with_tag( iter, text2, m_TextBufferTag_Error );
				break;
			}
			
			//iter = buf->last();
			//iter = buf->get_iter_at_line( buf->get_line_count() - 1 );
			//LogOutputTextView->scroll_to( iter );
			iter = buf->end();
			LogOutputTextView->scroll_to( iter );
		
			//Glib::MainContext::get_default()->iteration( false );
		
			gdk_threads_leave();
		}
#endif
	}

	//store log line into LastLogLine
	i = 0; j = strlen( LastLogLine[ 0 ] );
	while (( i < strlen( text ) ) &&
			( j < LastLogLineLength )) {
		if ( text[ i ] == '\n' ) {
			LastLogLine[ 0 ][ j ] = 0;
			for( lines=1; lines<LastLogLinesCount; lines++ )
				memcpy( &LastLogLine[ lines ], &LastLogLine[ lines-1 ], LastLogLineLength );
			LastLogLine[ 0 ][ 0 ] = 0;
			j = 0; i++;
			continue;
		}

		LastLogLine[ 0 ][ j ] = text[ i ];
		j++; i++;
	}
}

void BiturnLog::LogS( int type, const char *format, ... )
{
	va_list		l;

	va_start( l, format );

	LogV( type, format, l );
}

void BiturnLog::LogV( int type, const char *format, va_list list )
{
	int i = vsnprintf( logstr, LogStrLength-1, format, list );
	assert( i < LogStrLength );

	Log( logstr, type );
}

void BiturnLog::ClearLog( void )
{
#ifdef LOG_INTO_WIN32_CONTROLS
	#ifdef WIN32

		if ( LogOutputType == 1 )
			LogOutputWnd->SetWindowText( "" );
		if ( LogOutputType == 2 )
			LogOutputWndListCtrl->DeleteAllItems();

	#endif
#endif
#ifdef LOG_INTO_GTKMM_CONTROLS
	if ( LogOutputTextView != NULL ) {
		Glib::RefPtr< Gtk::TextBuffer >	buf;
		buf = LogOutputTextView->get_buffer();
		
		buf->erase( buf->begin(), buf->end() );
	}
#endif
	
	LastLogLine[ 0 ][ 0 ] = 0;
}

void BiturnLog::SetLoggingMood( int mood )
{
	switch ( mood ) {
	case LoggingMoodNothing:
	case LoggingMoodLight:
	case LoggingMoodNormal:
	case LoggingMoodHeavy:
	case LoggingMoodDebug:
		LoggingMood = mood;
		break;
	default:
		assert( false );
		break;
	}
}

int BiturnLog::GetLoggingMood( void )
{
	return LoggingMood;
}

#ifdef LOG_INTO_WIN32_CONTROLS
void BiturnLog::SetLogOutputWnd( CEdit *wnd )
{ 
	 LogOutputWndEdit = wnd;
	 LogOutputWndOneLine = 0;
	 LogOutputType = 0;
}

void BiturnLog::SetLogOutputWnd( CStatic *wnd )
{ 
	 LogOutputWndStatic = wnd;
	 LogOutputWndOneLine = 1;
	 LogOutputType = 1;
}

void BiturnLog::SetLogOutputWnd( CListCtrl *wnd )
{ 
	 LogOutputWndListCtrl = wnd;
	 LogOutputType = 2;
}
#endif

#ifdef LOG_INTO_GTKMM_CONTROLS
void BiturnLog::SetLogOutputWnd( Gtk::TextView *view )
{
	LogOutputTextView = view;
	
	Gdk::Color	teal;
	teal.set_rgb_p( 0.7, 0.7, 1.0 );
	
	m_TextBufferTag_Minor = Gtk::TextBuffer::Tag::create( "Minor" );
	m_TextBufferTag_Minor->property_background() = "white";
	m_TextBufferTag_Major = Gtk::TextBuffer::Tag::create( "Major" );
	m_TextBufferTag_Major->property_background_gdk() = teal;
	m_TextBufferTag_Error = Gtk::TextBuffer::Tag::create( "Error" );
	m_TextBufferTag_Error->property_background() = "red";
	
	//m_TextBufferTagTable =  Gtk::TextBuffer::TagTable::create();
	
	Glib::RefPtr<Gtk::TextBuffer> refBuffer = view->get_buffer();
	m_TextBufferTagTable = refBuffer->get_tag_table();
	
	m_TextBufferTagTable->add( m_TextBufferTag_Minor );
	m_TextBufferTagTable->add( m_TextBufferTag_Major );
	m_TextBufferTagTable->add( m_TextBufferTag_Error );
	
}
#endif

void BiturnLog::SetLogFileName( const char* name )
{
	LogFileName = name;
}

char* BiturnLog::GetLastLogLine( void )
{
	if ( strlen( LastLogLine[ 0 ] ) > 0 )
		return LastLogLine[ 0 ];
	else
		return LastLogLine[ 1 ];
}

BiturnLog* BiturnLogInstance( void )
{
	return BiturnLog::GetSingletonInstance();
}

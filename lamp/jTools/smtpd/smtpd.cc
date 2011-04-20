/*	========
 *	smtpd.cc
 *	========
 */

// Standard C++
#include <algorithm>
#include <functional>
#include <list>
#include <memory>
#include <vector>

// Standard C/C++
#include <cstdio>
#include <cstdlib>

// Standard C
#include <time.h>

// POSIX
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

// iota
#include "iota/decimal.hh"
#include "iota/strings.hh"

// plus
#include "plus/pointer_to_function.hh"
#include "plus/string/concat.hh"

// text-input
#include "text_input/feed.hh"
#include "text_input/get_line_from_feed.hh"

// nucleus
#include "nucleus/scribe.hh"
#include "nucleus/shared.hh"

// Io
#include "io/io.hh"
#include "io/spew.hh"

// Nitrogen
#include "Nitrogen/DateTimeUtils.hh"
#include "Nitrogen/Folders.hh"

// Io: MacFiles
#include "MacFiles/Classic.hh"

// poseven
#include "poseven/extras/fd_reader.hh"
#include "poseven/functions/write.hh"
#include "poseven/types/exit_t.hh"

// FSContents
#include "FSContents.h"

// Orion
#include "Orion/Main.hh"


namespace io
{
	
	inline FSSpec path_descent( const Nitrogen::FSDirSpec& dir, const unsigned char* name )
	{
		return Nitrogen::FSMakeFSSpec( dir, name );
	}
	
	inline FSSpec path_descent( const FSSpec& dir, const unsigned char* name )
	{
		return path_descent( Nitrogen::FSpMake_FSDirSpec( dir ), name );
	}
	
}

namespace tool
{
	
	namespace n = nucleus;
	namespace N = Nitrogen;
	namespace p7 = poseven;
	
	using namespace io::path_descent_operators;
	
	
	// E.g. "19840124.183000"
	static plus::string DateFormattedForFilename( const time_t& now, int serial )
	{
		const struct tm* t = gmtime( &now );
		
		plus::string result;
		
		char* p = result.reset( STRLEN( "YYYYMMDD.hhmmss-nn" ) );
		
		iota::fill_unsigned_decimal( t->tm_year + 1900, &p[0], 4 );
		iota::fill_unsigned_decimal( t->tm_mon  +    1, &p[4], 2 );
		iota::fill_unsigned_decimal( t->tm_mday,        &p[6], 2 );
		
		p[8] = '.';
		
		iota::fill_unsigned_decimal( t->tm_hour, &p[ 9], 2 );
		iota::fill_unsigned_decimal( t->tm_min,  &p[11], 2 );
		iota::fill_unsigned_decimal( t->tm_sec,  &p[13], 2 );
		
		p[15] = '-';
		
		iota::fill_unsigned_decimal( serial, &p[16], 2 );
		
		return result;
	}
	
	static plus::string MakeMessageName()
	{
		static time_t stamp = 0;
		static int serial = 0;
		
		const time_t now = time( NULL );
		
		if ( stamp == now )
		{
			++serial;
		}
		else
		{
			stamp = now;
			serial = 1;
		}
		
		return DateFormattedForFilename( now, serial );
	}
	
	static inline unsigned int IP( unsigned char a,
	                               unsigned char b,
	                               unsigned char c,
	                               unsigned char d )
	{
		return (a << 24) | (b << 16) | (c << 8) | d;
	}
	
	
	static plus::string GetForwardPath( const plus::string& rcptLine )
	{
		return rcptLine.substr( STRLEN( "RCPT TO:" ), rcptLine.npos );
	}
	
	static plus::string GetReversePath( const plus::string& fromLine )
	{
		return fromLine.substr( STRLEN( "MAIL FROM:" ), fromLine.npos );
	}
	
	static void CreateOneLiner( const FSSpec& file, const plus::string& line )
	{
		typedef n::POD_vector_scribe< plus::string > scribe;
		
		plus::string output = line + "\n";
		
		io::spew_file< scribe >( N::FSpCreate( file,
		                                       Mac::FSCreator( 'R*ch' ),
		                                       Mac::FSType   ( 'TEXT' ) ),
		                         output );
	}
	
	static void CreateDestinationFile( const N::FSDirSpec& destFolder, const plus::string& dest )
	{
		CreateOneLiner( destFolder / dest.substr( 0, 31 ),
		                dest );
	}
	
	static N::FSDirSpec QueueDirectory()
	{
		return N::FSpMake_FSDirSpec( io::system_root< N::FSDirSpec >() / "j" / "var" / "spool" / "jmail" / "queue" );
	}
	
	
	class PartialMessage
	{
		private:
			N::FSDirSpec dir;
			n::owned< N::FSFileRefNum > out;
		
		private:
			// non-copyable
			PartialMessage           ( const PartialMessage& );
			PartialMessage& operator=( const PartialMessage& );
		
		public:
			PartialMessage( const FSSpec& dir );
			
			~PartialMessage();
			
			N::FSDirSpec Dir() const  { return dir; }
			void WriteLine( const plus::string& line );
			
			void Finished();
	};
	
	PartialMessage::PartialMessage( const FSSpec& dirLoc )
	:
		dir( N::FSpDirCreate( dirLoc ) ),
		out( io::open_for_writing( N::FSpCreate( dir / "Message",
		                                         Mac::FSCreator( 'R*ch' ),
		                                         Mac::FSType   ( 'TEXT' ) ) ) )
	{
		//
	}
	
	PartialMessage::~PartialMessage()
	{
		if ( dir.dirID != 0 )
		{
			io::recursively_delete_directory( dir );
		}
	}
	
	void PartialMessage::WriteLine( const plus::string& line )
	{
		//static unsigned int lastFlushKBytes = 0;
		plus::string terminatedLine = line + "\r\n";
		
		io::write( out, terminatedLine.data(), terminatedLine.size() );
	}
	
	void PartialMessage::Finished()
	{
		dir.dirID = Mac::FSDirID();  // 0
	}
	
	
	plus::string myHello;
	plus::string myFrom;
	std::list< plus::string > myTo;
	
	static std::auto_ptr< PartialMessage > myMessage;
	
	bool dataMode = false;
	
	
	static void QueueMessage()
	{
		N::FSDirSpec dir = myMessage->Dir();
		
		// Create the Destinations subdirectory.
		N::FSDirSpec destFolder = N::FSpDirCreate( dir / "Destinations" );
		
		// Create the destination files.
		std::for_each( myTo.begin(),
		               myTo.end(),
		               std::bind1st( plus::ptr_fun( CreateDestinationFile ),
		                             destFolder ) );
		
		// Create the Return-Path file.
		// Write this last so the sender won't delete the message prematurely.
		CreateOneLiner( dir / "Return-Path", 
		                myFrom );
		
	}
	
	static void DoCommand( const plus::string& command )
	{
		plus::string word = command.substr( 0, command.find(' ') );
		
		if ( false )
		{
			//
		}
		else if ( word == "MAIL" )
		{
			myFrom = GetReversePath( command );
			
			p7::write( p7::stdout_fileno, STR_LEN( "250 Sender ok, probably"  "\r\n" ) );
		}
		else if ( word == "RCPT" )
		{
			myTo.push_back( GetForwardPath( command ) );
			
			p7::write( p7::stdout_fileno, STR_LEN( "250 Recipient ok, I guess"  "\r\n" ) );
		}
		else if ( word == "DATA" )
		{
			myMessage.reset( new PartialMessage( QueueDirectory() / MakeMessageName() ) );
			dataMode  = true;
			
			p7::write( p7::stdout_fileno, STR_LEN( "354 I'm listening"  "\r\n" ) );
		}
		else if ( word == "HELO" )
		{
			myHello = command;
			
			p7::write( p7::stdout_fileno, STR_LEN( "250 Hello there"  "\r\n" ) );
		}
		else if ( word == "RSET" )
		{
			myFrom = "";
			myTo.clear();
			//myData.clear();
			
			p7::write( p7::stdout_fileno, STR_LEN( "250 Reset"  "\r\n" ) );
		}
		else if ( word == "QUIT" )
		{
			//isComplete = true;
			p7::write( p7::stdout_fileno, STR_LEN( "221 Closing connection"  "\r\n" ) );
			
			throw p7::exit_success;
		}
		else
		{
			if ( word != "EHLO" )
			{
				std::fprintf( stderr, "Unrecognized command '%s'\n", word.c_str() );
			}
			
			p7::write( p7::stdout_fileno, STR_LEN( "500 Unrecognized command"  "\r\n" ) );
		}
	}
	
	static void DoData( const plus::string& data )
	{
		myMessage->WriteLine( data );
		
		if ( data == "." )
		{
			p7::write( p7::stderr_fileno, STR_LEN( "done\n" ) );
			
			dataMode = false;
			
			bool queued = false;
			
			try
			{
				QueueMessage();
				myMessage->Finished();
				queued = true;
			}
			catch ( ... )
			{
				
			}
			
			const char* message = queued ? "250 Message accepted"      "\r\n"
			                             : "554 Can't accept message"  "\r\n";
			
			p7::write( p7::stdout_fileno, message, std::strlen( message ) );
			
			myMessage.reset();
		}
		else
		{
			//
		}
	}
	
	static void DoLine( const plus::string& line )
	{
		if ( dataMode )
		{
			DoData( line );
		}
		else
		{
			DoCommand( line );
		}
	}
	
	int Main( int argc, char** argv )
	{
		sockaddr_in peer;
		socklen_t peerlen = sizeof peer;
		
		if ( getpeername( 0, (sockaddr*)&peer, &peerlen ) == 0 )
		{
			std::fprintf( stderr, "Connection from %s, port %d\n",
			                                       inet_ntoa( peer.sin_addr ),
			                                                peer.sin_port );
		}
		
		const size_t max_hostname_length = 255;
		
		const size_t buffer_size = STRLEN( "220 " )
		                         + max_hostname_length
		                         + STRLEN( " ready" "\r\n" )
		                         + 1;
		
		char hostname[256] = "smtpd";
		
		char message_buffer[ buffer_size ] = "220 smtpd";
		
		(void) gethostname( message_buffer + STRLEN( "220 " ),
		                    max_hostname_length + 1 );
		
		const size_t partial_size = strlen( message_buffer );
		
		memcpy( message_buffer + partial_size, STR_LEN( " ready" "\r\n" ) + 1 );
		
		const size_t message_size = partial_size + STRLEN( " ready" "\r\n" );
		
		p7::write( p7::stdout_fileno, message_buffer, message_size );
		
		text_input::feed feed;
		
		p7::fd_reader reader( p7::stdin_fileno );
		
		while ( const plus::string* s = get_line_from_feed( feed, reader ) )
		{
			plus::string line( s->begin(), s->end() - 1 );
			
			DoLine( line );
		}
		
		return 0;
	}

}


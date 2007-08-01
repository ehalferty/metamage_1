/*	======
 *	cat.cc
 *	======
 */

// Standard C/C++
#include <cerrno>

// Standard C/C++
#include <cstdio>

// POSIX
#include <fcntl.h>
#include <unistd.h>

// POSeven
#include "POSeven/FileDescriptor.hh"

// Orion
#include "Orion/Main.hh"
#include "Orion/StandardIO.hh"


namespace P7 = POSeven;
namespace O = Orion;


static void DumpFile( P7::FileDescriptor in );

static bool PathnameMeansStdIn( const char* pathname )
{
	return    pathname[0] == '-'
	       && pathname[1] == '\0';
}

int O::Main( int argc, const char *const argv[] )
{
	// Check for sufficient number of args
	if ( argc < 2 )
	{
		Io::Err << "cat: missing arguments\n";
		
		return 1;
	}
	
	// Print each file in turn.  Return whether any errors occurred.
	int fail = 0;
	
	for ( int index = 1; index < argc; index++ )
	{
		const char* pathname = argv[ index ];
		
		int fd = PathnameMeansStdIn( pathname ) ? 0
		                                        : open( pathname, 0 );
		
		if ( fd == -1 )
		{
			std::fprintf( stderr, "%s: %s: %s\n", argv[0], pathname, std::strerror( errno ) );
			
			continue;
		}
		
		try
		{
			DumpFile( P7::FileDescriptor( fd ) );
		}
		catch ( ... )
		{
			Io::Err << "cat: " << pathname << ": DumpFile() failed\n";
			++fail;
		}
		
		if ( fd != 0 )
		{
			(void) close( fd );
		}
	}
	
	return (fail == 0) ? 0 : 1;
}

void DumpFile( P7::FileDescriptor in )
{
	while ( true )
	{
		try
		{
			enum { blockSize = 4096 };
			
			char data[ blockSize ];
			
			int bytes = io::read( in, data, blockSize );
			
			write( STDOUT_FILENO, data, bytes );
		}
		catch ( const io::end_of_input& err )
		{
			break;
		}
		catch ( const io::no_input_pending& )
		{
			sleep( 0 );
		}
	}
}


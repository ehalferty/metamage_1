/*	========
 *	mkdir.cc
 *	========
 */

// Standard C
#include <errno.h>

// POSIX
#include "sys/stat.h"

// Nitrogen / Carbon
#include "Nitrogen/Files.h"
#include "Nitrogen/OSStatus.h"

// Genie
#include "Genie/pathnames.hh"
#include "Genie/Process.hh"
#include "Genie/SystemCallRegistry.hh"
#include "Genie/Yield.hh"


namespace Genie
{
	
	namespace N = Nitrogen;
	namespace NN = Nucleus;
	
	static int mkdir( const char* pathname, mode_t /*mode*/ )
	{
		try
		{
			N::FSDirSpec current = NN::Convert< N::FSDirSpec >( CurrentProcess().CurrentDirectory() );
			
			FSSpec spec = ResolveUnixPathname( pathname, current );
			
			N::FSpDirCreate( spec );
			
			return 0;
		}
		catch ( N::DupFNErr& )
		{
			return CurrentProcess().SetErrno( EEXIST );
		}
		catch ( ... )
		{
		}
		
		return CurrentProcess().SetErrno( EINVAL );
	}
	
	REGISTER_SYSTEM_CALL( mkdir );
	
}


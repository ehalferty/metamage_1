// dup2.cc
// -------
//
// Maintained by Joshua Juran

// Part of the Nitrogen project.
//
// Written 2009 by Joshua Juran.
//
// This code was written entirely by the above contributor, who places it
// in the public domain.


#include "poseven/functions/dup2.hh"

// poseven
#include "poseven/types/errno_t.hh"


namespace poseven
{
	
	void dup2( fd_t old_fd, fd_t new_fd )
	{
		throw_posix_result( ::dup2( old_fd, new_fd ) );
	}
	
}

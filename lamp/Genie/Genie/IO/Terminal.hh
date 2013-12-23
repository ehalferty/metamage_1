/*	===========
 *	Terminal.hh
 *	===========
 */

#ifndef GENIE_IO_TERMINAL_HH
#define GENIE_IO_TERMINAL_HH

// plus
#include "plus/string.hh"

// Genie
#include "Genie/IO/Base.hh"


namespace Genie
{
	
	const pid_t no_pgid = 0x7fffffff;
	
	
	class TerminalHandle : public IOHandle
	{
		private:
			const plus::string  itsTTYName;
			IOPtr               itsTTY;
			pid_t               its_process_group_id;
			
			IOHandle* Next() const  { return itsTTY.get(); }
		
		public:
			TerminalHandle( const plus::string& tty_name );
			
			~TerminalHandle();
			
			void Attach( vfs::filehandle* target )  { itsTTY = target; }
			
			FSTreePtr GetFile();
			
			pid_t getpgrp() const  { return its_process_group_id; }
			
			void setpgrp( pid_t pgid )  { its_process_group_id = pgid; }
			
			void IOCtl( unsigned long request, int* argp );
			
			void Disconnect();
	};
	
}

#endif


/*
	trace.cc
	--------
*/

#include "trace.hh"

// POSIX
#include <unistd.h>

// gear
#include "gear/parse_decimal.hh"

// vdb
#include "print_registers.hh"


#define PROMPT "vdb> "

#define TROFF_WARNING "Warning:  Tracing is off.  Enter 't' to reenable."

#define PRINT( s )  write( STDERR_FILENO, s "\n", sizeof s )


static unsigned n_steps;

static uint32_t step_over;

static void debugger_loop( registers& regs )
{
	static uint32_t troff_address = 0;
	
	if ( !troff_address )
	{
		troff_address = regs.pc + 2;  // stop after the JSR (A1)
		
		return;
	}
	
	if ( regs.pc == troff_address )
	{
		regs.sr &= 0x3FFF;  // clear Trace bits
		
		return;
	}
	
	if ( n_steps > 0 )
	{
		--n_steps;
		
		return;
	}
	
	if ( step_over )
	{
		if ( regs.pc < step_over  ||  regs.pc > step_over + 22 )
		{
			return;
		}
		
		step_over = 0;
	}
	
	if ( (regs.sr & 0xC000) == 0 )
	{
		PRINT( TROFF_WARNING );
	}
	
	char buffer[ 16 ];
	
	for ( ;; )
	{
		write( STDERR_FILENO, PROMPT, sizeof PROMPT - 1 );
		
		ssize_t n_read = read( STDIN_FILENO, buffer, sizeof buffer );
		
		if ( n_read <= 0 )
		{
			// FIXME
			break;
		}
		
		buffer[ n_read - 1 ] = '\0';
		
		const char* p = buffer;
		
		switch ( *p )
		{
			case 'c':
			case 'g':
				regs.sr &= 0x3FFF;  // clear Trace bits
				return;
			
			case 's':
				n_steps = 0;
				
				{
					while ( *++p != '\0' )  if ( *p != ' ' )  break;
					
					const char* q = p;
					
					const unsigned steps = gear::parse_unsigned_decimal( &q );
					
					if ( q != p  &&  steps > 0 )
					{
						n_steps = steps - 1;
					}
				}
				
				return;
			
			case 'o':
				step_over = regs.pc;
				return;
			
			case 'r':
				print_registers( regs );
				break;
			
			case 't':
				regs.sr |= 0x8000;
				break;
			
			case 'x':
				_exit(0);
			
			default:
				break;
		}
	}
}

static asm void trace_handler()
{
	MOVEM.L  D0-D7/A0-A7,-(SP)
	
	MOVE.W   #0x2000,D3
	AND.W    64(SP),D3
	BNE.S    no_USP_load
	
	MOVE     USP,A0
	MOVE.L   A0,60(SP)  // copy user SP to registers
	
no_USP_load:
	
	MOVE.L  SP,-(SP)
	
	JSR  debugger_loop
	
	ADDQ.L   #4,SP
	
	TST.W    D3
	BNE.S    no_USP_store
	
	MOVEA.L  60(SP),A0  // update USP
	MOVE     A0,USP
	
no_USP_store:
	
	MOVEM.L  (SP)+,D0-D7/A0-A6  // update registers (not A7)
	
	ADDQ.L   #4,SP
	
	RTE
}

void* trace_vector : 0x00000024;

asm int set_trace_handler()
{
	JSR     0xFFFFFFFA ;  // enter_supervisor_mode()
	
	BMI.S   bail ;  // D0 is -1 if branch taken
	
	LEA     trace_handler,A0
	MOVE.L  A0,trace_vector
	
	MOVE    D0,SR
	MOVEQ   #0,D0
	
bail:
	RTS
}

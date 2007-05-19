/*	===============
 *	GenieProcess.hh
 *	===============
 *	
 *	Implemented by GenieProcess.cc
 */

#ifndef GENIE_PROCESS_HH
#define GENIE_PROCESS_HH

// Universal Interfaces
#include <AEDataModel.h>

// Standard C++
#include <map>
#include <string>

// Boost
#include <boost/shared_ptr.hpp>

// Nucleus
#include "Nucleus/Owned.h"

// Nitrogen
#include "Nitrogen/CodeFragments.h"

// Nitrogen Extras / Utilities
#include "Utilities/Threads.h"

// ShellShock
#include "ShellShock/VarArray.hh"

// Genie
#include "Genie/BinaryImage.hh"
#include "Genie/FileDescriptor.hh"
#include "Genie/FileSystem/FSTree.hh"


namespace Genie
{
	
	namespace N = Nitrogen;
	namespace NN = Nucleus;
	namespace NX = NitrogenExtras;
	namespace Sh = ShellShock;
	
	
	typedef void ( *sig_t )( int );
	
	typedef std::map< int, FileDescriptor > FileDescriptorMap;
	
	class NotExecutableError {};
	
	
	typedef int (*MainProcPtr)( int, char const* const*, char const* const* );
	
	class Process;
	
	struct ThreadContext
	{
		Process* processContext;
		MainProcPtr externalMain;
		const char* const* argv;
		const char* const* envp;
		
		ThreadContext()
		:
			processContext(),
			externalMain  (),
			argv          (),
			envp          ()
		{}
		
		ThreadContext( Process*            process,
		               MainProcPtr         externalMain,
		               const char* const*  argv,
		               const char* const*  envp )
		:
			processContext( process       ),
			externalMain  ( externalMain  ),
			argv          ( argv          ),
			envp          ( envp          )
		{}
	};
	
	struct ExternalProcessExecutor
	{
		int operator()( ThreadContext& context ) const;
	};
	
	struct Thread_Details
	{
		typedef ThreadContext  Parameter;
		typedef int            Result;
		
		typedef ExternalProcessExecutor Function;
	};
	
	typedef NX::Thread< Thread_Details > Thread;
	
	typedef int*     ErrnoDataPtr;
	typedef char***  EnvironDataPtr;
	
	class TTYHandle;
	
	enum ProcessLifeStage
	{
		kProcessStarting,
		kProcessLive,
		kProcessTerminated,
		kProcessReleased
	};
	
	enum ProcessInterdependence
	{
		kProcessIndependent,
		kProcessForking,
		kProcessForked
	};
	
	enum ProcessSchedule
	{
		kProcessRunning,
		kProcessSleeping,
		kProcessStopped,
		kProcessFrozen,
		kProcessUnscheduled
	};
	
	typedef void (*CleanupHandlerProc)();
	
#if TARGET_CPU_68K
	
	typedef CleanupHandlerProc CleanupHandler;
	
#else
	
	struct CleanupHandler
	{
		CleanupHandler& operator=( CleanupHandlerProc )  { return *this; }
		
		operator CleanupHandlerProc() const  { return NULL; }
		
		void operator()() const  {}
	};
	
#endif
	
	class Process
	{
		public:
			enum
			{
				kNoProcessPID   = 0,
				kRootProcessPID = 1
			};
		
		private:
			pid_t itsPPID;
			pid_t itsPID;
			pid_t itsPGID;
			pid_t itsSID;
			
			UInt64 itsAlarmClock;
			
			UInt32 itsPendingSignals;
			UInt32 itsPreviousSignals;
			
			std::string itsName;
			
			FSTreePtr itsCWD;
			
			TTYHandle* itsControllingTerminal;
			
			FileDescriptorMap itsFileDescriptors;
			
			ProcessLifeStage        itsLifeStage;
			ProcessInterdependence  itsInterdependence;
			ProcessSchedule         itsSchedule;
			
			int itsResult;
			
			std::map< int, sig_t > itsSignalMap;
		
		private:
			FSSpec itsProgramFile;
			BinaryImage itsFragmentImage;
			BinaryImage itsOldFragmentImage;
			NN::Owned< CFragConnectionID > itsFragmentConnection;
			NN::Owned< CFragConnectionID > itsOldFragmentConnection;
			
			std::auto_ptr< Sh::StringArray > itsArgvStorage;
			std::auto_ptr< Sh::VarArray    > itsEnvironStorage;
			std::auto_ptr< Thread > itsThread;
			
			CleanupHandler itsCleanupHandler;
			
			ErrnoDataPtr   itsErrnoData;
			EnvironDataPtr itsEnvironData;
			
			std::string itsLastEnv;
		
		private:
			void Orphan();
		
		public:
			struct RootProcess {};
			
			Process( RootProcess );
			Process( pid_t ppid );
			
			~Process();
			
			void SetCleanupHandler( CleanupHandlerProc cleanup )  { itsCleanupHandler = cleanup; }
			
			pid_t GetPPID() const  { return itsPPID; }
			pid_t GetPID()  const  { return itsPID;  }
			pid_t GetPGID() const  { return itsPGID; }
			pid_t GetSID()  const  { return itsSID;  }
			
			void SetPGID( pid_t pgid )  { itsPGID = pgid; }
			void SetSID ( pid_t sid  )  { itsSID  = sid;  }
			
			ProcessLifeStage  GetLifeStage() const  { return itsLifeStage; }
			ProcessSchedule   GetSchedule () const  { return itsSchedule;  }
			
			int Result() const  { return itsResult; }
			
			void Release();
			
			void SetSchedule( ProcessSchedule schedule )  { itsSchedule = schedule; }
			
			void Result( int result )  { itsResult = result; }
			
			TTYHandle* ControllingTerminal() const  { return itsControllingTerminal; }
			
			void SetControllingTerminal( TTYHandle* terminal )  { itsControllingTerminal = terminal; }
			
			FSTreePtr GetCWD() const  { return itsCWD; }
			
			void ChangeDirectory( const FSTreePtr& newCWD );
			
			std::string ProgramName()  const  { return itsName;   }
			
			void ProgramName( const std::string& name )  { itsName = name; }
			
			FSSpec ProgramFile() const  { return itsProgramFile; }
			
			FileDescriptorMap& FileDescriptors()  { return itsFileDescriptors; }
			
			char** Environ() const  { return itsEnvironStorage->GetPointer(); }
			
			int const* ErrnoData()  const { return itsErrnoData; }
			int      * ErrnoData()        { return itsErrnoData; }
			
			char** const* EnvironData()  const { return itsEnvironData; }
			char**      * EnvironData()        { return itsEnvironData; }
			
			char* GetEnv( const char* name );
			void SetEnv( const char* name, const char* value, bool overwrite );
			void PutEnv( const char* string );
			void UnsetEnv( const char* name );
			void ClearEnv();
			
			unsigned int SetAlarm( unsigned int seconds );
			
			sig_t SetSignalAction( int signal, sig_t signalAction );
			
			void Raise( int signal );
			
			void ResumeAfterFork();
			
			void Terminate();
			void Terminate( int result );
		
		public:
			bool Forked() const  { return itsThread.get() == NULL; }
			
			NN::Owned< N::ThreadID > Exec( const FSSpec&       progFile,
			                               const char* const   argv[],
			                               const char* const*  envp );
			
			void InitThread();
			
			int SetErrno( int errorNumber );
			
			void Stop();
			void Continue();
			
			bool HandlePendingSignals();
	};
	
	struct NoSuchProcess  {};
	
	class GenieProcessTable
	{
		public:
			typedef std::map< pid_t, boost::shared_ptr< Process > > ProcessMap;
			typedef ProcessMap::const_iterator const_iterator;
			typedef ProcessMap::iterator iterator;
		
		private:
			ProcessMap itsProcesses;
			pid_t itsNextPID;
		
		public:
			GenieProcessTable();
			
			~GenieProcessTable()
			{
				//ASSERT( itsProcesses.size() == 1 );
			}
			
			pid_t NewProcess( Process* process );
			
			int RemoveProcess( iterator it );
			int RemoveProcess( pid_t pid );
			
			void KillAll();
			
			ProcessMap const& Map() const  { return itsProcesses; }
			ProcessMap      & Map()        { return itsProcesses; }
			
			const_iterator begin() const  { return itsProcesses.begin(); }
			const_iterator end  () const  { return itsProcesses.end  (); }
			
			iterator begin()  { return itsProcesses.begin(); }
			iterator end  ()  { return itsProcesses.end  (); }
	};
	
	extern GenieProcessTable gProcessTable;
	
	void SendSignalToProcessesControlledByTerminal( int sig, TTYHandle* terminal );
	
	Process& GetProcess( pid_t pid );
	
	Process* FindProcess( pid_t pid );
	
}

#endif


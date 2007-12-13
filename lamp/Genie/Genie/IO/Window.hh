/*	=========
 *	Window.hh
 *	=========
 */

#ifndef GENIE_IO_WINDOW_HH
#define GENIE_IO_WINDOW_HH

// Standard C++
#include <string>

// Nitrogen
#include "Nitrogen/MacWindows.h"
#include "Nitrogen/Str.h"

// Pedestal
#include "Pedestal/Window.hh"

// Genie
#include "Genie/IO/Terminal.hh"


namespace Genie
{
	
	class WindowHandle : public TerminalHandle
	{
		private:
			int itsWindowSalvagePolicy;
		
		public:
			WindowHandle( const std::string& name );
			
			virtual ~WindowHandle();
			
			virtual void IOCtl( unsigned long request, int* argp );
			
			virtual Pedestal::WindowCore& GetWindowCore() = 0;
			
			virtual Nitrogen::WindowRef GetWindowRef() const = 0;
			
			bool ShouldBeSalvaged() const;
			
			Nitrogen::Str255 GetTitle() const;
			
			void SetTitle( ConstStr255Param title );
			
			Point GetPosition() const;
			
			void SetPosition( Point position );
			
			bool IsVisible() const;
			
			void Show() const;
			void Hide() const;
	};
	
	typedef std::map< ::WindowRef, boost::weak_ptr< IOHandle > > WindowMap;
	
	const WindowMap& GetWindowMap();
	
	void AddWindowToMap( ::WindowRef window, const boost::shared_ptr< IOHandle >& handle );
	
	TerminalHandle& GetWindowFromMap( ::WindowRef window );
	
	const boost::shared_ptr< Pedestal::WindowCloseHandler >& GetTerminalCloseHandler();
	
}

#endif


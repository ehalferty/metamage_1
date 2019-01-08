/*
	menus.hh
	--------
*/

#include "mac_app/menus.hh"

// Mac OS X
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

// Mac OS
#if ! TARGET_API_MAC_CARBON
#ifndef __DEVICES__
#include <Devices.h>
#endif
#endif
#ifndef __MENUS__
#include <Menus.h>
#endif

// mac-sys-utils
#include "mac_sys/has/Aqua_menus.hh"


namespace mac {
namespace app {

enum
{
	kAppleMenuID = 1,
	kFileMenuID  = 2,
};

void install_menus()
{
	short id = 128;
	
	while ( MenuRef menu = GetMenu( id++ ) )
	{
		InsertMenu( menu, 0 );
	}
	
	DrawMenuBar();
	
	if ( ! TARGET_API_MAC_CARBON )
	{
		const ResType kDeskAccessoryResourceType = 'DRVR';
		
		MenuRef appleMenu = GetMenuHandle( kAppleMenuID );
		
		AppendResMenu( appleMenu, kDeskAccessoryResourceType );
	}
	
	if ( mac::sys::has_Aqua_menus() )
	{
		MenuRef fileMenu = GetMenuHandle( kFileMenuID );
		
		SInt16 last = CountMenuItems( fileMenu );
		
		// Delete "Quit" and the separator above it.
		
		DeleteMenuItem( fileMenu, last     );
		DeleteMenuItem( fileMenu, last - 1 );
	}
}

void open_DA_from_menu( short item )
{
	static MenuRef appleMenu = GetMenuHandle( kAppleMenuID );
	
	Str255 itemText;
	
	GetMenuItemText( appleMenu, item, itemText );
	
#if CALL_NOT_IN_CARBON
	
	OpenDeskAcc( itemText );
	
#endif
}

}
}
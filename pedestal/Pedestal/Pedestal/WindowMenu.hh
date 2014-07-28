/*
	WindowMenu.hh
	-------------
*/

#ifndef PEDESTAL_WINDOWMENU_HH
#define PEDESTAL_WINDOWMENU_HH

// Mac OS X
#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

// Mac OS
#ifndef __MACWINDOWS__
#include <MacWindows.h>
#endif


namespace Pedestal
{
	
	void window_created( WindowRef w );
	void window_removed( WindowRef w );
	
}

#endif

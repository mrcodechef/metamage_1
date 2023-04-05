/*
	find_SDK_dir.cc
	---------------
*/

#include "one_path/find_SDK_dir.hh"

// Standard C
#include <stdlib.h>

// gear
#include "gear/find.hh"

// plus
#include "plus/var_string.hh"

// poseven
#include "poseven/types/errno_t.hh"

// one_path
#include "one_path/find_MPW_dir.hh"


namespace p7 = poseven;


static const char env_sdk_dir[] = "SDK_DIR";
static const char txt_intfs_libs[] = "Interfaces&Libraries";


plus::string find_SDK_dir()
{
	if ( const char* sdk_dir = getenv( env_sdk_dir ) )
	{
		return sdk_dir;
	}
	
	plus::var_string pathname = find_MPW_dir().move();
	
	const char* p = pathname.data();
	
	if ( const char* it = gear::find_last_match( p, pathname.size(), '/' ) )
	{
		pathname.resize( it + 1 - p );
		
		pathname += txt_intfs_libs;
		
		return pathname.move();
	}
	
	throw p7::errno_t( ENOENT );
}
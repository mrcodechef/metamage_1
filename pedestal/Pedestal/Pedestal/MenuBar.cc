/*	==========
 *	MenuBar.cc
 *	==========
 */

#include "Pedestal/MenuBar.hh"

// more-libc
#include "more/string.h"

// gear
#include "gear/quad.hh"

// mac-sys-utils
#include "mac_sys/has/Aqua_menus.hh"

// Pedestal
#include "Pedestal/BundleStrings.hh"
#include "Pedestal/MenuItemCommands.hh"
#include "Pedestal/OwnerResource.hh"


#define STRLEN( s )  (sizeof "" s - 1)
#define STR_LEN( s )  "" s, (sizeof s - 1)


namespace Pedestal
{
	
	static CommandCode TakeCodeFromItemText( Str255 ioItemText )
	{
		int len = ioItemText[ 0 ];
		
		if ( len < 7 )  return kCmdNone;
		
		if ( ioItemText[ len     ] != ']' )  return kCmdNone;
		if ( ioItemText[ len - 5 ] != '[' )  return kCmdNone;
		
		ioItemText[ 0 ] -= 7;
		
		return CommandCode( gear::decode_quad( &ioItemText[ len - 4 ] ) );
	}
	
	static inline
	CommandCode ExtractItemCmdCode( MenuRef menu, short item )
	{
		Str255 itemText;
		GetMenuItemText( menu, item, itemText );
		
		CommandCode code = TakeCodeFromItemText( itemText );
		
		if ( code != kCmdNone )
		{
			SetMenuItemText( menu, item, itemText );
		}
		
		return code;
	}
	
	void AddMenu( MenuRef menu )
	{
		const long shifted_menuID = GetMenuID( menu ) << 16;
		
		const UInt16 count = CountMenuItems( menu );
		
		for ( int i = count;  i > 0;  i-- )
		{
			const CommandCode code = ExtractItemCmdCode( menu, i );
			
			SetMenuItemCommandCode( shifted_menuID | i, code );
		}
	}
	
	
	void FixUpAboutMenuItem( MenuRef appleMenu )
	{
		Str255 about;
		
		GetMenuItemText( appleMenu, 1, about );
		
		// Check for a trailing NUL byte.
		
		if ( about[ about[ 0 ] ] != '\0' )
		{
			/*
				The project supplied its own Apple menu resource, so just
				return.
			*/
			
			return;
		}
		
		if ( TARGET_API_MAC_CARBON )
		{
			if ( CFStringRef name = GetBundleName() )
			{
				// Remove trailing NUL and replace ellipsis with space.
				
				about[ --about[ 0 ] ] = ' ';
				
				int max_length = about[ 0 ] + CFStringGetLength( name );
				
				CFMutableStringRef text;
				text = CFStringCreateMutable( NULL, max_length );
				
				const CFStringEncoding macRoman = kCFStringEncodingMacRoman;
				
				if ( text )
				{
					CFStringAppendPascalString( text, about, macRoman );
					CFStringAppend( text, name );
					
					SetMenuItemTextWithCFString( appleMenu, 1, text );
					
					CFRelease( text );
				}
				
				return;
			}
		}
		
		Str255 name;
		
		if ( GetOwnerResourceName( GetCreatorFromBNDL(), name ) )
		{
			const Byte* name_p = name;
			
			const UInt8 len = *name_p++;
			
			if ( len != 0  &&  about[ 0 ] + len <= 255 )
			{
				unsigned char* p = about + about[ 0 ];
				
				p[ -1 ] = ' ';
				
				p = (unsigned char*) mempcpy( p, name_p, len );
				
				about[ 0 ] = p - about;
				
				if ( mac::sys::has_Aqua_menus() )
				{
					--about[ 0 ];
				}
				else
				{
					*p++ = '\xC9';  // ellipsis
				}
				
				SetMenuItemText( appleMenu, 1, about );
			}
		}
	}
	
}

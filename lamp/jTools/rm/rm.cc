/*	=====
 *	rm.cc
 *	=====
 */

// Standard C/C++
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

// POSIX
#include <unistd.h>

// Iota
#include "iota/strings.hh"

// Orion
#include "Orion/Main.hh"


namespace tool
{
	
	int Main( int argc, iota::argv_t argv )
	{
		// Check for sufficient number of args
		if ( argc < 2 )
		{
			(void) write( STDERR_FILENO, STR_LEN( "rm: missing arguments\n" ) );
			
			return 1;
		}
		
		int exit_status = EXIT_SUCCESS;
		
		for ( int index = 1;  index < argc;  ++index )
		{
			const char* pathname = argv[ index ];
			
			int unlinked = unlink( pathname );
			
			if ( unlinked == -1 )
			{
				exit_status = EXIT_FAILURE;
				
				std::fprintf( stderr, "rm: %s: %s\n", pathname, std::strerror( errno ) );
			}
		}
		
		return exit_status;
	}
	
}


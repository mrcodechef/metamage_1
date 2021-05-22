/*
	environ_store.hh
	----------------
*/

#ifndef KEROSENE_ENVIRONSTORE_HH
#define KEROSENE_ENVIRONSTORE_HH

// Standard C/C++
#include <cstddef>


namespace _relix_libc
{
	
	class environ_store
	{
		private:
			// Non-copyable
			environ_store           ( const environ_store& );
			environ_store& operator=( const environ_store& );
			
		private:
			char* find_space_or_reallocate( std::size_t extra_space );
			void update_environ();
			void preallocate();
			
			void erase( char* var );
		
		public:
			environ_store( char** envp );
			
			char* get( const char* name );
			void set( const char* name, const char* value, bool overwriting );
			void put( char* string );
			void unset( const char* name );
			void clear();
	};
	
}

#endif

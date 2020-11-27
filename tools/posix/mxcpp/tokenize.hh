/*
	tokenize.hh
	-----------
*/

#ifndef MXCPP_TOKENIZE_HH
#define MXCPP_TOKENIZE_HH

// Standard C++
#include <vector>

// plus
#include "plus/string.hh"


namespace tool
{
	
	class token_list
	{
		private:
			std::vector< plus::string > its_vector;
		
		public:
			size_t size() const  { return get().size(); }
			
			plus::string& operator[]( size_t i )
			{
				return get()[ i ];
			}
			
			plus::string const& operator[]( size_t i ) const
			{
				return get()[ i ];
			}
			
			std::vector< plus::string >      & get()        { return its_vector; }
			std::vector< plus::string > const& get() const  { return its_vector; }
	};
	
	inline bool operator==( const token_list& a, const token_list& b )
	{
		return a.get() == b.get();
	}
	
	inline bool operator!=( const token_list& a, const token_list& b )
	{
		return !( a == b );
	}
	
	void tokenize( const plus::string& input, token_list& output );
	
	token_list tokenize( const plus::string& input );
	
}

#endif

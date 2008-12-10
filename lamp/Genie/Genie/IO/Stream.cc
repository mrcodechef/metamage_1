/*	=========
 *	Stream.cc
 *	=========
 */

#include "Genie/IO/Stream.hh"

// Standard C++
#include <algorithm>

// POSIX
#include "sys/ioctl.h"

// Nucleus
#include "Nucleus/NAssert.h"

// POSeven
#include "POSeven/Errno.hh"

// Genie
#include "Genie/Process.hh"


namespace Genie
{
	
	namespace p7 = poseven;
	
	
	StreamHandle::~StreamHandle()
	{
	}
	
	ssize_t StreamHandle::SysRead( char* data, std::size_t byteCount )
	{
		p7::throw_errno( EPERM );
		
		return 0;
	}
	
	ssize_t StreamHandle::SysWrite( const char* data, std::size_t byteCount )
	{
		p7::throw_errno( EPERM );
		
		return 0;
	}
	
	void StreamHandle::TryAgainLater() const
	{
		Genie::TryAgainLater( IsNonblocking() );
	}
	
	// Return a reference to the peek buffer with at least minBytes of data in it.
	// We Read() until we have enough data.  If there're not enough data available,
	// then we either block, or throw EWOULDBLOCK if the stream is non-blocking.
	
	const std::string* StreamHandle::Peek( ByteCount minBytes )
	{
		if ( IsDisconnected() )
		{
			p7::throw_errno( EIO );
		}
		
		while ( itsPeekBuffer.size() < minBytes )
		{
			const ByteCount kBufferLength = 4096;
			
			char data[ kBufferLength ];
			
			ssize_t bytes = SysRead( data, kBufferLength );  // will block, throw, or return non-negative
			
			if ( bytes == 0 )
			{
				return NULL;
			}
			
			itsPeekBuffer.append( data, bytes );
		}
		
		return &itsPeekBuffer;
	}
	
	unsigned int StreamHandle::Poll() const
	{
		if ( IsDisconnected() )
		{
			p7::throw_errno( EIO );
		}
		
		return SysPoll() | (itsPeekBuffer.empty() ? 0 : kPollRead);
	}
	
	ssize_t StreamHandle::Read( char* data, std::size_t byteCount )
	{
		if ( IsDisconnected() )
		{
			p7::throw_errno( EIO );
		}
		
		ByteCount bytesRead = std::min( itsPeekBuffer.size(), byteCount );
		
		if ( data != NULL )
		{
			// copy data out of peek buffer
			std::copy( itsPeekBuffer.begin(),
			           itsPeekBuffer.begin() + bytesRead,
			           data );
			
			// advance dest mark for further reads
			data += bytesRead;
		}
		
		if ( bytesRead > 0 )
		{
			// slide unread data in peek buffer to beginning
			std::copy( itsPeekBuffer.begin() + bytesRead,
			           itsPeekBuffer.end(),
			           itsPeekBuffer.begin() );
			
			itsPeekBuffer.resize( itsPeekBuffer.size() - bytesRead );
			
			// adjust request size
			byteCount -= bytesRead;
		}
		
		if ( data == NULL )
		{
			if ( bytesRead == 0 )
			{
				p7::throw_errno( EFAULT );
			}
			
			return bytesRead;
		}
		
		try
		{
			ssize_t result = SysRead( data, byteCount );
			
			bytesRead += result;
		}
		catch ( ... )
		{
			if ( bytesRead == 0 )
			{
				throw;
			}
		}
		
		return bytesRead;
	}
	
	ssize_t StreamHandle::Write( const char* data, std::size_t byteCount )
	{
		if ( IsDisconnected() )
		{
			p7::throw_errno( EIO );
		}
		
		return SysWrite( data, byteCount );
	}
	
	void StreamHandle::IOCtl( unsigned long request, int* argp )
	{
		switch ( request )
		{
			case FIONBIO:
				if ( *argp )
				{
					SetNonblocking();
				}
				else
				{
					ClearNonblocking();
				}
				
				break;
			
			case FIONREAD:
				// not implemented
				return p7::throw_errno( EINVAL );
			
			default:
				IOHandle::IOCtl( request, argp );
				break;
		}
	}
	
}


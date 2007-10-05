/*	====================
 *	local-edit-client.cc
 *	====================
 */

// Standard C/C++
#include <cerrno>
#include <cstdlib>

// Standard C++
#include <string>

// POSIX
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

// Iota
#include "iota/strings.hh"

// POSeven
#include "POSeven/Errno.hh"
#include "POSeven/FileDescriptor.hh"
#include "POSeven/Open.hh"

// Arcana
#include "HTTP.hh"
#include "MD5.hh"

// Orion
#include "Orion/GetOptions.hh"
#include "Orion/Main.hh"


namespace NN = Nucleus;
namespace P7 = POSeven;
namespace p7 = poseven;
namespace O = Orion;


static MD5::Result MD5DigestFile( p7::fd_t input )
{
	MD5::Engine md5;
	
	char buffer[ 4096 ];
	
	try
	{
		while ( std::size_t bytes_read = p7::read( input, buffer, 4096 ) )
		{
			if ( bytes_read == 4096 )
			{
				for ( const char* p = buffer;  p < buffer + 4096;  p += 64 )
				{
					md5.DoBlock( p );
				}
			}
			else
			{
				const char* end_of_blocks = buffer + bytes_read - bytes_read % 64;
				
				for ( const char* p = buffer;  p < end_of_blocks;  p += 64 )
				{
					md5.DoBlock( p );
				}
				
				md5.Finish( end_of_blocks, bytes_read % 64 * 8 );
				
				return md5.GetResult();
			}
		}
	}
	catch ( const io::end_of_input& )
	{
		md5.Finish( buffer, 0 );
	}
	
	return md5.GetResult();
}

static void EncodeBase64Triplet( const unsigned char* triplet, unsigned char* buffer )
{
	const char* code = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	
	const unsigned char mask = 0x3f;
	
	buffer[0] = code[                            (triplet[0] >> 2) ];
	buffer[1] = code[ mask & (triplet[0] << 4) | (triplet[1] >> 4) ];
	buffer[2] = code[ mask & (triplet[1] << 2) | (triplet[2] >> 6) ];
	buffer[3] = code[ mask & (triplet[2] << 0)                     ];
}

static std::string EncodeBase64( const unsigned char* begin, const unsigned char* end )
{
	std::string result;
	
	result.reserve( (end - begin + 2) * 4 / 3 );
	
	unsigned char buffer[4];
	
	while ( begin < end - 2 )
	{
		EncodeBase64Triplet( begin, buffer );
		
		result.append( (char*) buffer, 4 );
		
		begin += 3;
	}
	
	if ( begin < end )
	{
		unsigned char final[3] = { 0, 0, 0 };
		
		std::copy( begin, end, final );
		
		EncodeBase64Triplet( final, buffer );
		
		if ( end - begin == 1 )
		{
			buffer[2] = '=';
		}
		
		buffer[3] = '=';
		
		result.append( (char*) buffer, 4 );
	}
	
	return result;
}

static void CommitFileChangesWithBackup( const char*  changed,
                                         const char*  original,
                                         const char*  backup )
{
	P7::ThrowPOSIXResult( rename( changed, original ) );  // FIXME
}


int O::Main( int argc, argv_t argv )
{
	bool dumpHeaders = false;
	
	char const *const defaultOutput = "/dev/fd/1";
	
	const char* outputFile = defaultOutput;
	
	O::BindOption( "-i", dumpHeaders );
	O::BindOption( "-o", outputFile  );
	
	O::GetOptions( argc, argv );
	
	char const *const *freeArgs = O::FreeArguments();
	
	std::size_t argCount = O::FreeArgumentCount();
	
	const char* filename = argCount > 0 ? freeArgs[0] : "/dev/null";
	
	NN::Owned< p7::fd_t > message_body = p7::open( filename, O_RDONLY );
	
	MD5::Result digest = MD5DigestFile( message_body );
	
	std::string old_digest_b64 = EncodeBase64( digest.data, digest.data + 16 );
	
	//p7::lseek( message_body, 0, 0 );
	lseek( message_body, 0, 0 );
	
	const p7::fd_t socket_in  = p7::fd_t( 6 );
	const p7::fd_t socket_out = p7::fd_t( 7 );
	
	const char* method = "APPLY";
	
	const char* urlPath = "/cgi-bin/local-edit-server";
	
	std::string contentLengthHeader;
	
	try
	{
		contentLengthHeader = HTTP::GetContentLengthHeaderLine( message_body );
	}
	catch ( ... )
	{
	}
	
	std::string message_header =   HTTP::RequestLine( method, urlPath )
	                             //+ HTTP::HeaderLine( "Host", hostname )
	                             + HTTP::HeaderLine( "Content-MD5", old_digest_b64 )
	                             + contentLengthHeader
	                             + "\r\n";
	
	HTTP::SendMessage( socket_out, message_header, message_body );
	
	p7::close( message_body );
	
	shutdown( socket_out, SHUT_WR );
	
	HTTP::ResponseReceiver response;
	
	response.ReceiveHeaders( socket_in );
	
	if ( dumpHeaders )
	{
		const std::string& message = response.GetMessageStream();
		
		p7::write( p7::stdout_fileno, message.data(), message.size() );
	}
	
	unsigned result_code = response.GetResultCode();
	
	if ( result_code == 200 )
	{
		NN::Owned< p7::fd_t > output = p7::open( outputFile, O_WRONLY | O_TRUNC | O_CREAT, 0400 );
		
		const std::string& partial_content = response.GetPartialContent();
		
		if ( !partial_content.empty() )
		{
			p7::write( output, partial_content.data(), partial_content.size() );
		}
		
		HTTP::SendMessageBody( output, socket_in );
		
		output = p7::open( outputFile, O_RDONLY );
		
		digest = MD5DigestFile( output );
		
		p7::close( output );
		
		std::string new_digest_b64 = EncodeBase64( digest.data, digest.data + 16 );
		
		std::string received_digest_b64 = response.GetHeader( "Content-MD5", "" );
		
		if ( new_digest_b64 != received_digest_b64 )
		{
			std::fprintf( stderr, "MD5 digest mismatch\n" );
			
			unlink( outputFile );
			
			return EXIT_FAILURE;
		}
		
		// FIXME:  test for a regular file
		if ( outputFile != defaultOutput )
		{
			p7::write( p7::stdout_fileno, STR_LEN( "Hit return to confirm or Control-D to cancel: " ) );
			
			while ( true )
			{
				char c;
				
				int bytes_read = read( p7::stdin_fileno, &c, sizeof c );
				
				if ( bytes_read == -1 )
				{
					std::perror( "local-edit-client: read" );
					
					// I'm not sure what the scenario is here.
					// (EINTR on handled signal?  EIO on disconnected terminal?)
					// Leave tmp file for recovery.
					return EXIT_FAILURE;
				}
				
				if ( bytes_read == 0 )
				{
					p7::write( p7::stdout_fileno, STR_LEN( "\n" "canceled\n" ) );
					
					unlink( outputFile );
					
					break;
				}
				
				if ( c == '\n' )
				{
					CommitFileChangesWithBackup( outputFile, filename, NULL );
					
					break;
				}
			}
		}
		
	}
	else if ( result_code == 304 )
	{
		// Not modified
	}
	else
	{
		std::fprintf( stderr, "%s\n", response.GetResult().c_str() );
		
		return EXIT_FAILURE;
	}
	
	shutdown( socket_in, SHUT_RD );
	
	return 0;
}


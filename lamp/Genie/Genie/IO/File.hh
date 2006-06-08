/*	=======
 *	File.hh
 *	=======
 */

#ifndef GENIE_IO_FILE_HH
#define GENIE_IO_FILE_HH

// Genie
#include "Genie/IO/Stream.hh"

// <sys/stat.h>
struct stat;


namespace Genie
{
	
	class FileHandle : public StreamHandle
	{
		private:
			bool isBlocking;
		
		public:
			static TypeCode Type()  { return kFileType; }
			
			virtual TypeCode ActualType() const  { return Type(); }
			
			FileHandle() : isBlocking( true )  {}
			
			virtual ~FileHandle()  {}
			
			bool IsBlocking() const  { return isBlocking; }
			
			void SetBlocking   ()  { isBlocking = true;  }
			void SetNonBlocking()  { isBlocking = false; }
			
			virtual void Stat( struct ::stat* sb ) const = 0;
	};
	
}

#endif


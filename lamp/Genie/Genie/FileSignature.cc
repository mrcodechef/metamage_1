/*	======================
 *	Genie/FileSignature.cc
 *	======================
 */

#include "Genie/FileSignature.hh"

// Standard C
#include <string.h>

// plus
#include "plus/string.hh"

// MacFeatures
#include "MacFeatures/Features.hh"


namespace Genie
{
	
	namespace N = Nitrogen;
	
	
	using MacFeatures::Has_OSXSystem;
	
	
	enum
	{
		sigBBEdit       = 'R*ch',
		sigTextWrangler = '!Rch'
	};
	
	::OSType gTextFileCreator = Has_OSXSystem() ? sigTextWrangler
	                                            : sigBBEdit;
	
	
	struct ExtensionToTypeRecord
	{
		const char* extension;
		::OSType type;
		const char* comment;
	};
	
	const ExtensionToTypeRecord gExtensionToTypeMappingInput[] =
	{
		// Notes
		// -----
		// .sit is ambiguous: 'SIT!', 'SITD', and 'SIT5' are all possible translations.
		// My copy of Internet Config uses 'SITD'.
		
		// .bin is ambiguous, used for MacBinary and disk images.
		// We'll need to have a generic binary type and a means of autodetection.
		// UPDATE:  To avoid pain, we're going with .mbin / 'mBIN' for new archives.
		
		// General
		{ "rsrc", 'rsrc', "resource file" },
		
		// Archive formats
		{ "mBin", 'BIN+', "MacBinary III+" " archive" },
		{ "mbin", 'mBIN', "MacBinary III"  " archive" },
		{ "mbi",  'mBIN', "MacBinary III"  " archive" },
		{ "zip",  'ZIP ', "zip"            " archive" },
		{ "tar",  'TARF', "TAR"            " archive" },
		{ "lha",  'LHA ', "LHA"            " archive" },
		{ "rar",  'RARf', "RAR"            " archive" },
		{ "sit",  'SITD', "StuffIt"        " archive" },
		
		// Single-file encoding formats
		{ "as",  'ApSi', "AppleSingle"  "-encoded file" },
		{ "bin", 'BINA', "MacBinary"    "-encoded file" },
		
		// Single-file compression formats
		{ "Z",   'ZIVU', "Z"     "-compressed file" },
		{ "gz",  'Gzip', "gzip"  "-compressed file" },
		{ "bz2", 'Bzp2', "bzip2" "-compressed file" },
		
		// Compressed archive coalesced extensions
		{ "taz", 'ZIVU', "Z"    "-compressed " "tarball" },
		{ "tgz", 'Gzip', "gzip" "-compressed " "tarball" },
		{ "mbz", 'Gzip', "gzip" "-compressed " "MacBinary archive" },
		
		// PDF
		{ "pdf", 'PDF ', "PDF" " document" },
		
		// Image formats
		{ "gif",  'GIFf', "GIF"  " image" },
		{ "png",  'PNGf', "PNG"  " image" },
		{ "bmp",  'BMPf', "BMP"  " image" },
		{ "jpg",  'JPEG', "JPEG" " image" },
		{ "tiff", 'TIFF', "TIFF" " image" },
		{ "pict", 'PICT', "PICT" " image" },
		
		{ "mov", 'MooV', "QuickTime" " movie" },
		
		{ "mp3", 'MPEG', "MP3" " audio" },
		
		// Application-specific binary formats
		{ "lib", 'MPLF', "Metrowerks static library" },
	};
	
	struct FileSignature
	{
		::OSType creator;
		::OSType type;
	};
	
	const FileSignature gDefaultCreatorForTypeInput[] =
	{
		{ 'RSED', 'rsrc' },
		{ 'mBin', 'mBIN' },
		{ 'mBin', 'BIN+' },
		{ 'CARO', 'PDF ' },
		{ 'ogle', 'PICT' },
		{ 'ogle', 'PNGf' },
		{ 'ogle', 'GIFf' },
		{ 'ogle', 'TIFF' },
		{ 'ogle', 'JPEG' },
		{ 'TVOD', 'MPEG' },
		{ 'TVOD', 'MooV' },
		{ 'CWIE', 'MPLF' },
	};
	
	
	static bool operator==( const ExtensionToTypeRecord& record, const char* extension )
	{
		return strcmp( record.extension, extension ) == 0;
	}
	
	static inline bool operator==( const FileSignature& signature, ::OSType type )
	{
		return signature.type == type;
	}
	
	static const ExtensionToTypeRecord* FindExtensionToTypeRecord( const char* extension )
	{
		const ExtensionToTypeRecord* begin = gExtensionToTypeMappingInput;
		const ExtensionToTypeRecord* end   = begin + sizeof gExtensionToTypeMappingInput / sizeof gExtensionToTypeMappingInput[0];
		
		const ExtensionToTypeRecord* it = std::find( begin, end, extension );
		
		return it != end ? it : NULL;
	}
	
	static const FileSignature* FindFileSignature( ::OSType type )
	{
		const FileSignature* begin = gDefaultCreatorForTypeInput;
		const FileSignature* end   = begin + sizeof gDefaultCreatorForTypeInput / sizeof gDefaultCreatorForTypeInput[0];
		
		const FileSignature* it = std::find( begin, end, type );
		
		return it != end ? it : NULL;
	}
	
	
	static N::OSType GetCreatorForType( N::OSType type )
	{
		if ( const FileSignature* it = FindFileSignature( type ) )
		{
			return N::OSType( it->creator );
		}
		
		return N::OSType( '\?\?\?\?' );
	}
	
	Mac::FSSignature PickFileSignatureForName( const plus::string& name )
	{
		std::size_t dot = name.find_last_of( "." );
		
		if ( dot != name.npos )
		{
			const char* extension = &name[ dot + 1 ];
			
			if ( const ExtensionToTypeRecord* it = FindExtensionToTypeRecord( extension ) )
			{
				N::OSType type = N::OSType( it->type );
				
				Mac::FSSignature signature( GetCreatorForType( type ), type );
				
				return signature;
			}
		}
		
		return Mac::FSSignature( TextFileCreator(), N::OSType( 'TEXT' ) );
	}
	
}


/*	==========
 *	Project.cc
 *	==========
 */

#include "A-line/Project.hh"

// Standard C++
#include <algorithm>
#include <functional>
#include <set>
#include <vector>

// POSeven
#include "POSeven/Pathnames.hh"
#include "POSeven/Stat.hh"

// Nitrogen
#include "Nitrogen/Aliases.h"
#include "Nitrogen/Files.h"
#include "Nitrogen/MacErrors.h"
#include "Nitrogen/OSStatus.h"

// GetPathname
#include "GetPathname.hh"

// Nitrogen Extras / Templates
#include "Templates/FunctionalExtensions.h"
#include "Templates/PointerToFunction.h"

// BitsAndBytes
#include "StringFilters.hh"
#include "StringPredicates.hh"

// Divergence
#include "Divergence/Utilities.hh"

// Orion
#include "Orion/StandardIO.hh"

// CompileDriver
#include "CompileDriver/ProjectConfig.hh"
#include "CompileDriver/Subprojects.hh"

// A-line
#include "A-line/A-line.hh"
#include "A-line/Exceptions.hh"
#include "A-line/DeepFiles.hh"
#include "A-line/Locations.hh"
#include "A-line/ProjectCommon.hh"
#include "A-line/SourceDotList.hh"


namespace ALine
{
	
	namespace N = Nitrogen;
	namespace NN = Nucleus;
	namespace Div = Divergence;
	namespace CD = CompileDriver;
	
	namespace ext = N::STLExtensions;
	
	using namespace io::path_descent_operators;
	
	using BitsAndBytes::eos;
	using BitsAndBytes::q;
	
	
	inline N::Str255 GetFileName( const FSSpec& file )
	{
		return file.name;
	}
	
	static std::string FindIncludeInFolder( const std::string& folder, IncludePath includePath )
	{
		// This will throw if folder or any subfolders are missing.
		std::string file = folder / includePath;
		
		if ( !io::item_exists( file ) )
		{
			throw N::FNFErr();
		}
		
		return file;
	}
	
	bool Project::FindInclude( const std::string& includePath )
	{
		typedef std::vector< std::string >::const_iterator Iter;
		
		for ( Iter it = sourceDirs.begin();  it != sourceDirs.end();  ++it )
		{
			// FIXME:  Use a trapped function
			try
			{
				std::string file = FindIncludeInFolder( *it, includePath );
				
				AddInclude( includePath, file );
				
				return true;
			}
			catch ( ... )
			{
			}
		}
		
		return false;
	}
	
	static std::string First( const std::vector< std::string >& v )
	{
		return ( v.size() > 0 ) ? v[ 0 ] : std::string();
	}
	
	static std::string FindSearchDir( const std::string& cwdPath, const std::string& pathname )
	{
		try
		{
			std::string dirPath = cwdPath + "/" + pathname;
			
			if ( io::directory_exists( dirPath ) )
			{
				return dirPath;
			}
			
			Io::Err << "Missing search directory " << q( pathname ) << "\n";
			throw;
		}
		catch ( ... )
		{
			Io::Err << "Unrecognized exception for " << q( pathname ) << "\n";
			throw;
		}
		
		// Not reached
		return "";
	}
	
	
	static ProductType ReadProduct( const std::string& productName )
	{
		if ( productName == "app" )
		{
			return productApplication;
		}
		else if ( productName == "lib" )
		{
			return productStaticLib;
		}
		else if ( productName == "shlib" )
		{
			return productSharedLib;
		}
		else if ( productName == "wish" )
		{
			return productWish;
		}
		else if ( productName == "init" )
		{
			return productINIT;
		}
		else if ( productName == "driver" )
		{
			return productDriver;
		}
		
		return productNotBuilt;
	}
	
	Project::Project( const std::string& proj )
	:
		projName  ( proj ),
		projFolder( CD::GetProjectFolder( proj, Options().platform ) ),
		projFolderPath( GetPOSIXPathname( projFolder ) ),
		product   ( productNotBuilt )
	{
		const CD::ProjectData* initialProjectData = &CD::GetProjectData( projName, Options().platform );
		
		// We need to merge both the project's platform requirements (unconditionally)
		// and the default platform settings (conditionally) with the settings
		// already determined by the command-line options.
		// The order we do this in is important.
		// We don't want building a code resource to fail because 68K/A4 conflicts
		// with the default settings, so the special-case options set for code
		// resources have to be applied before the defaults.  However,
		// if a project has multiple implementations (as evidenced by the presence
		// of the 'platform' directive or a non-default platform value), then we
		// need to apply the platform defaults first, so A-line doesn't just grab
		// the first implementation it finds.  For example, "A-line OpenSSL" should
		// include OpenSSL-include and import OpenSSLLib when run from Genie, and
		// do nothing when run from the A-line Mach-O tool.
		// The defaults will only have effect (if at all) when first applied.
		
		// If this project has platform restrictions,
		if ( initialProjectData->platform != CD::Platform() )
		{
			// Apply the defaults first so we can select an appropriate project alternative.
			CD::ApplyPlatformDefaults( Options().platform );
			
			// Now that we (potentially) have new platform restrictions,
			// call GetProjectData() again to get the alternative that matches.
			initialProjectData = &CD::GetProjectData( projName, Options().platform );
			
			// The platform restrictions are already known to be compatible or we
			// wouldn't have gotten here.
			Options().platform &= initialProjectData->platform;
		}
		
		const CD::ProjectData& projectData = *initialProjectData;
		
		CD::ConfData config( projectData.confData );
		
		if ( config.size() > 0 )
		{
			progName = First( config[ "program" ] );
			
			product = ReadProduct( First( config[ "product" ] ) );
			
			if (    product == productINIT
			     || product == productDriver )
			{
				Options().platform &= CD::Platform( CD::arch68K,
				                                    CD::runtimeA4CodeResource,
				                                    CD::apiMacToolbox );
			}
			
			typedef std::vector< ProjName >::const_iterator vPN_ci;
			
			// Figure out which projects we use
			const std::vector< ProjName >& moreUsedProjects = config[ "use" ];
			
			std::vector< ProjName > usedProjects = config[ "uses" ];
			
			std::size_t usedCount = usedProjects.size();
			
			usedProjects.resize( usedCount + moreUsedProjects.size() );
			
			std::copy( moreUsedProjects.begin(),
			           moreUsedProjects.end(),
			           usedProjects.begin() + usedCount );
			
			std::set< ProjName > allUsedSet;
			
			// For each project named in the 'uses' directive:
			for ( vPN_ci it = usedProjects.begin();  it != usedProjects.end();  ++it )
			{
				try
				{
					GetProject( *it );  // Recursively creates projects
				}
				catch ( NoSuchProject )
				{
					throw NoSuchUsedProject( projName, *it );
				}
				catch ( CD::NoSuchProject )
				{
					throw NoSuchUsedProject( projName, *it );
				}
				
				Project& used = GetProject( *it );
				
				// Find out which projects it uses
				const std::vector< ProjName >& subUsed = used.AllUsedProjects();
				
				// For each project even indirectly used by this one:
				for ( vPN_ci it2 = subUsed.begin();  it2 != subUsed.end();  ++it2 )
				{
					const ProjName& name = *it2;
					
					// If it isn't already in the collection,
					if ( allUsedSet.count( name ) == 0 )
					{
						// Add it to the collection.
						allUsedSet.insert( name );
						allUsedProjects.push_back( name );
					}
				}
			}
			
			// Check for special options.
			// Currently, there's cwd-source, which indicates that we should
			// pass '-cwd source' to CodeWarrior.  OpenSSL needs it.
			const std::vector< std::string >& options = config[ "options" ];
			
			needsCwdSourceOption = std::find( options.begin(),
			                                  options.end(),
			                                  std::string( "cwd-source" ) )
			                       != options.end();
			
			// Locate source files
			const std::vector< std::string >& search = config[ "search" ];
			
			// If search folders are specified,
			if ( search.size() > 0 )
			{
				sourceDirs.resize( search.size() );
				
				// Find and record them.
				std::transform( search.begin(),
				                search.end(),
				                sourceDirs.begin(),
				                std::bind1st( N::PtrFun( FindSearchDir ),
					                          projFolderPath ) );
			}
			else
			{
				std::string sourceDir;
				
				// Otherwise, just use a default location.
				try
				{
					sourceDir = ProjectSourcesPath( projFolderPath );
				}
				catch ( ... )
				{
					sourceDir = projFolderPath;
				}
				
				sourceDirs.push_back( sourceDir );
			}
		}
		
		// Make sure we're in the list too, and make sure we're last.
		allUsedProjects.push_back( proj );
		
		//printf("%s recursively uses %d projects.\n", proj.c_str(), allUsedProjects.size());
		
		// If this project precompiles a header, this is the relative path to it.
		precompiledHeaderSource  = First( config[ "precompile" ] );
		
		// The creator code for linked output files.  Mac only.
		creator = First( config[ "creator"    ] );
		
		//myExtraSources = config[ "sources"    ];  // Extra sources to compile.  Deprecated.
		myImports      = config[ "imports"    ];  // Libraries to import.
		myFrameworks   = config[ "frameworks" ];  // Frameworks to include when building for OS X.
		rsrcFiles      = config[ "rsrc"       ];  // Resource files from which to copy resources.
		rezFiles       = config[ "rez"        ];  // Rez files to compile.
	}
	
	static bool IsCompilableExtension( const std::string& ext )
	{
		if ( ext == ".c"   )  return true;
		if ( ext == ".cc"  )  return true;
		if ( ext == ".cp"  )  return true;
		if ( ext == ".cpp" )  return true;
		if ( ext == ".c++" )  return true;
		
		return false;
	}
	
	static bool IsCompilableFilename( const std::string& filename )
	{
		std::size_t lastDot = filename.find_last_of( "." );
		
		return    lastDot != std::string::npos
		       && IsCompilableExtension( filename.substr( lastDot, std::string::npos ) );
	}
	
	
	template < class Container >
	class MembershipTest 
	:
		public std::unary_function< const typename Container::key_type&, bool >
	{
		private:
			const Container& container;
		
		public:
			MembershipTest( const Container& con ) : container( con )  {}
			
			bool operator()( const typename Container::key_type& element ) const
			{
				return    std::find( container.begin(),
				                     container.end(),
				                     element )
				       != container.end();
			}
	};
	
	template < class Container >
	MembershipTest< Container > Membership( const Container& container )
	{
		return MembershipTest< Container >( container );
	}
	
	template < class Exception, class Function >
	struct ExceptionConverter
	{
		typedef typename Function::result_type   result_type;
		typedef typename Function::argument_type argument_type;
		
		ExceptionConverter( const Function& f ) : f( f )  {}
		
		result_type operator()( const argument_type& arg ) const
		{
			try
			{
				return f( arg );
			}
			catch ( ... )
			{
				throw Exception( arg );
			}
			
			// Not reached
			return result_type();
		}
		
		Function f;
	};
	
	template < class Exception, class Function >
	ExceptionConverter< Exception, Function > ConvertException( const Function& f )
	{
		return ExceptionConverter< Exception, Function >( f );
	}
	
	static std::string FindFileInDir( const std::string& filename, const std::string& dir )
	{
		std::string result = dir / filename;
		
		if ( io::item_exists( result ) )
		{
			return result;
		}
		
		throw N::FNFErr();
	}
	
	static std::string FindSourceFileInDirs( const std::string& relative_path, const std::vector< std::string >& sourceDirs )
	{
		typedef std::vector< std::string >::const_iterator dir_iter;
		
		for ( dir_iter it = sourceDirs.begin();  it != sourceDirs.end();  ++it )
		{
			std::string dir = *it;
			
			try
			{
				const char* path = relative_path.c_str();
				
				while ( const char* slash = std::strchr( path, '/' ) )
				{
					dir = dir / std::string( path, slash );
					
					path = slash + 1;
				}
				
				return FindFileInDir( path, dir );
			}
			catch ( ... )
			{
			}
		}
		
		std::fprintf( stderr, "Missing source file %s\n", relative_path.c_str() );
		
		throw N::FNFErr();
	}
	
	void Project::Study()
	{
		// Add the includes directory
		AddIncludeDir( projName );
		
		if ( product == productNotBuilt )  return;
		
		// First try files explicitly specified on the command line
		std::vector< std::string > sourceList = Options().files;
		
		// None?  Try a Source.list file
		if ( sourceList.size() == 0 )
		{
			std::string sourceDotListfile = SourceDotListFile( projFolderPath );
			
			if ( io::item_exists( sourceDotListfile ) )
			{
				sourceList = ReadSourceDotList( sourceDotListfile );
			}
		}
		
		// We have filenames -- now, find them
		if ( sourceList.size() > 0 )
		{
			typedef std::vector< std::string >::const_iterator str_iter;
			
			for ( str_iter it = sourceList.begin();  it != sourceList.end();  ++it )
			{
				const std::string& sourceName = *it;
				
				mySources.push_back( FindSourceFileInDirs( sourceName, sourceDirs ) );
			}
		}
		else
		{
			// Still nothing?  Just enumerate everything in the source directory.
			
			typedef std::vector< std::string >::const_iterator Iter;
			
			// Enumerate our source files
			std::vector< std::string > sources;
			for ( Iter it = sourceDirs.begin();  it != sourceDirs.end();  ++it )
			{
				std::vector< FSSpec > deepSources = DeepFiles
				(
					Div::ResolvePathToFSSpec( it->c_str() ), 
					ext::compose1
					(
						std::ptr_fun( IsCompilableFilename ), 
						ext::compose1
						(
							NN::Converter< std::string, const unsigned char* >(), 
							N::PtrFun( GetFileName )
						)
					)
				);
				
				sources.resize( sources.size() + deepSources.size() );
				
				std::transform( deepSources.begin(),
				                deepSources.end(),
				                sources.end() - deepSources.size(),
				                std::ptr_fun( static_cast< std::string (*)(const FSSpec&) >( GetPOSIXPathname ) ) );
			}
			
			// FIXME:  Doesn't deal with duplicates
			std::swap( mySources, sources );
		}
		
		/*
		try
		{
			std::transform
			(
				mySources.begin(), 
				mySources.end(), 
				mySources.begin(), 
				ConvertException< FSSpec >
				(
					std::bind2nd
					(
						N::PtrFun( N::ResolveAliasFile ), 
						true
					)
				)
			);
		}
		catch ( FSSpec& alias )
		{
			throw BadSourceAlias( *this, alias );
		}
		*/
		
		// Locate resources
		try
		{
			std::vector< std::string > rezFiles = DeepFiles( projFolderPath / "Resources" );
			
			std::for_each
			(
				rezFiles.begin(), 
				rezFiles.end(), 
				std::ptr_fun( AddRezFile )
			);
		}
		catch ( ... )
		{
			
		}
		
	}
	
}


/*	===========================
 *	FSTree_sys_mac_user_home.cc
 *	===========================
 */

#include "Genie/FileSystem/FSTree_sys_mac_user_home.hh"

// Nitrogen
#include "Nitrogen/Folders.h"

// Io: MacFiles
#include "MacFiles.hh"

// Genie
#include "Genie/FileSystem/FSSpec.hh"
#include "Genie/Utilities/AsyncIO.hh"


namespace Genie
{
	
	namespace N = Nitrogen;
	namespace NN = Nucleus;
	
	
	static N::FSDirSpec GetAppFolder()
	{
		// FIXME:  Assumes that app doesn't call HSetVol().
		// (We don't, but still...)
		return N::FSDirSpec( FSMakeFSSpec< FNF_Throws >( N::FSVolumeRefNum(), N::FSDirID(), "\p" ) );
	}
	
	static N::FSDirSpec GetUsersFolder( N::FSVolumeRefNum vRefNum )
	{
		try
		{
			return N::FindFolder( vRefNum, N::kUsersFolderType, false );
		}
		catch ( ... )
		{
		}
		
		return N::FSDirSpec( FSMakeFSSpec< FNF_Throws >( vRefNum, N::fsRtDirID, "\pUsers" ) );
	}
	
	static N::FSDirSpec FindUserHomeFolder()
	{
		N::FSDirSpec appFolder = GetAppFolder();
		
		N::FSDirSpec users = GetUsersFolder( appFolder.vRefNum );
		
		N::FSDirSpec parent = appFolder;
		N::FSDirSpec child;
		
		do
		{
			child = parent;
			
			parent = io::get_preceding_directory( child );
		}
		while ( parent != users );
		
		return child;
	}
	
	static N::FSDirSpec GetUserHomeFolder()
	{
		try
		{
			return N::FindFolder( N::kOnAppropriateDisk, N::kCurrentUserFolderType, false );
		}
		catch ( ... )
		{
		}
		
		return FindUserHomeFolder();
	}
	
	void CheckForHomeFolder()
	{
		GetUserHomeFolder();
	}
	
	
	std::string FSTree_sys_mac_user_home::ReadLink() const
	{
		return ResolveLink()->Pathname();
	}
	
	FSTreePtr FSTree_sys_mac_user_home::ResolveLink() const
	{
		return FSTreeFromFSDirSpec( GetUserHomeFolder() );
	}
	
}


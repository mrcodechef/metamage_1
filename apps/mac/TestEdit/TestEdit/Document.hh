/*
	Document.hh
	-----------
*/

#ifndef TESTEDIT_DOCUMENT_HH
#define TESTEDIT_DOCUMENT_HH

// plus
#include "plus/ref_count.hh"
#include "plus/string.hh"

// TestEdit
#include "TestEdit/Window.hh"


namespace TestEdit
{
	
	class Document : public plus::ref_count< Document >
	{
		private:
			nucleus::owned< WindowRef >  itsWindow;
			bool                         itHasFile;
			bool                         itIsDirty;
		
		public:
			Document();
			Document( const FSSpec& file );
			Document( const FSRef & file );
			
			WindowRef GetWindowRef() const  { return itsWindow; }
			
			plus::string GetName() const;
	};
	
}

#endif

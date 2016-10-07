/*	============
 *	PedWindow.cc
 *	============
 */

#include "Pedestal/Window.hh"

// MacGlue
#include "MacGlue/MacGlue.hh"

// nucleus
#include "nucleus/saved.hh"

// Nitrogen
#include "Nitrogen/Quickdraw.hh"

// ClassicToolbox
#include "ClassicToolbox/MacWindows.hh"

// MacFeatures
#include "MacFeatures/ColorQuickdraw.hh"

// Pedestal
#include "Pedestal/View.hh"
#include "Pedestal/WindowMenu.hh"


namespace MacGlue
{
	
	DECLARE_MAC_GLUE( NewWindow  );
	DECLARE_MAC_GLUE( NewCWindow );
	
}

namespace Pedestal
{
	
	namespace n = nucleus;
	namespace N = Nitrogen;
	
	
#if ! OPAQUE_TOOLBOX_STRUCTS
	
	struct WindowStorage
	{
		WindowRecord  window;
	};
	
	static
	WindowStorage* RecoverWindowStorage( WindowRef window )
	{
		if ( GetWindowKind( window ) != kApplicationWindowKind )
		{
			return NULL;
		}
		
		void* address = (char*) window - offsetof( WindowStorage, window );
		
		return (WindowStorage*) address;
	}
	
	static
	pascal void DestroyWindow( WindowRef window )
	{
		WindowStorage* storage = RecoverWindowStorage( window );
		
		CloseWindow( window );
		
		delete storage;
	}
	
#endif
	
	static
	void set_window_owner( WindowRef window, Window* owner )
	{
		N::SetWRefCon( window, owner );
	}
	
	Window* get_window_owner( WindowRef window )
	{
		return N::GetWRefCon( window );
	}
	
	typedef pascal WindowRef (*NewWindow_ProcPtr)( void*             storage,
	                                               const Rect*       bounds,
	                                               ConstStr255Param  title,
	                                               Boolean           visible,
	                                               short             procID,
	                                               WindowRef         behind,
	                                               Boolean           goAwayFlag,
	                                               long              refCon );
	
	using MacFeatures::Has_ColorQuickdraw;
	
	static const NewWindow_ProcPtr gNewWindow = Has_ColorQuickdraw() ? &MacGlue::NewCWindow
	                                                                 : &MacGlue::NewWindow;
	
	
	void ResizeWindow( WindowRef window, Point newSize )
	{
		InvalidateWindowGrowBox( window );  // assume grow box present on resize
		
		N::SizeWindow( window, newSize.h, newSize.v, true );
		
		if ( Window* base = get_window_owner( window ) )
		{
			// Don't rely on the requested size because it might have been tweaked
			Rect bounds = N::GetPortBounds( N::GetWindowPort( window ) );
			
			// Shotgun approach -- invalidate the whole window.
			// Clients can validate regions if they want.
			N::InvalRect( bounds );
			
			// Inform the window's contents that it has been resized
			base->Resized( bounds.right, bounds.bottom );
		}
	}
	
	void SetWindowSize( WindowRef window, Point size )
	{
		n::saved< N::Port > savePort;
		
		N::SetPortWindowPort( window );
		
		ResizeWindow( window, size );
	}
	
	
	static
	n::owned< WindowRef > CreateWindow( const Rect&         bounds,
	                                    ConstStr255Param    title,
	                                    bool                visible,
	                                    N::WindowDefProcID  procID,
	                                    WindowRef           behind,
	                                    bool                goAwayFlag )
	{
		typedef nucleus::disposer_class< WindowRef >::type Disposer;
		
		Disposer disposer;
		
		WindowRef substorage = NULL;
		
	#if ! TARGET_API_MAC_CARBON
		
		disposer = &DestroyWindow;
		
		WindowStorage* storage = new WindowStorage;
		
		substorage = &storage->window.port;
		
	#endif
		
		WindowRef window = gNewWindow( substorage,
		                               &bounds,
		                               title,
		                               visible,
		                               procID,
		                               behind,
		                               goAwayFlag,
		                               0 );
		
		//N::SetWindowKind( window, kPedestalWindowKind );
		N::SetPortWindowPort( window );
		
		return n::owned< WindowRef >::seize( window, disposer );
	}
	
	static inline
	n::owned< WindowRef > CreateWindow( const NewWindowContext& context )
	{
		return CreateWindow( context.bounds,
		                     context.title,
		                     context.visible,
		                     context.procID,
		                     context.behind,
		                     context.goAwayFlag );
	}
	
	static Rect GrowBoxBounds( WindowRef window )
	{
		Rect bounds = N::GetPortBounds( N::GetWindowPort( window ) );
		
		bounds.left = bounds.right - 15;
		bounds.top = bounds.bottom - 15;
		
		return bounds;
	}
	
	void InvalidateWindowGrowBox( WindowRef window )
	{
		N::InvalRect( GrowBoxBounds( window ) );
	}
	
	
	Window::Window( const NewWindowContext& context )
	:
		itsWindowRef( CreateWindow( context ) ),
		itsDefProcID( context.procID )
	{
		set_window_owner( itsWindowRef, this );
		
		window_created( itsWindowRef.get() );
	}
	
	Window::~Window()
	{
		window_removed( itsWindowRef.get() );
	}
	
	
	void Window::Activate( bool activating )
	{
		GetView()->Activate( activating );
		
		InvalidateGrowBox();
	}
	
	void Window::Resized( short width, short height )
	{
		Rect bounds = { 0, 0, height, width };
		
		GetView()->SetBounds( bounds );
		
		InvalidateGrowBox();
		
		if ( const WindowResizeHandler* handler = itsResizeHandler.get() )
		{
			(*handler)( Get() );
		}
	}
	
	void Window::MouseDown( const EventRecord& event )
	{
		// FIXME:  The window may want clicks even if it's not in front.
		if ( Get() != Nitrogen::FrontWindow() )
		{
			Nitrogen::SelectWindow( Get() );
		}
		else
		{
			GetView()->MouseDown( event );
		}
	}
	
	void Window::Update()
	{
		GetView()->Draw( Nitrogen::GetPortBounds( Nitrogen::GetWindowPort( Get() ) ), true );
		
		if ( !TARGET_API_MAC_CARBON && HasGrowIcon() )
		{
			n::saved< N::Clip > savedClip;
			
			N::ClipRect( GrowBoxBounds( Get() ) );
			
			N::DrawGrowIcon( Get() );
		}
	}
	
	
	#if 0
	
	static Rect CalcWindowStructureDiff()
	{
		Rect r = { -200, -200, -100, -100 };
		unsigned char* title = "\pTest";
		bool vis = true;
		int procID = 0;
		WindowRef front = kFirstWindowOfClass;
		bool goAway = true;
		int refCon = 0;
		WindowRef windowPtr = ::NewWindow(NULL, &r, title, vis, procID, front, goAway, refCon);
		
		VRegion region;
		::GetWindowStructureRgn(windowPtr, region);
		::DisposeWindow(windowPtr);
		RgnHandle rgnH = region;
		Rect bounds = (**rgnH).rgnBBox;
		
		return SetRect(
			bounds.left   - r.left, 
			bounds.top    - r.top, 
			bounds.right  - r.right, 
			bounds.bottom - r.bottom
		);
	}
	
	static Rect WindowStructureDiff()
	{
		static Rect diff = CalcWindowStructureDiff();
		return diff;
	}
	
	#endif
	
}

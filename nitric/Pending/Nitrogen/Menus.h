// Menus.h

#ifndef NITROGEN_MENUS_H
#define NITROGEN_MENUS_H

#ifndef NITROGEN_FRAMEWORKHEADER_H
#include "Nitrogen/FrameworkHeader.h"
#endif
#ifndef __MENUS__
#include FRAMEWORK_HEADER(HIToolbox,Menus.h)
#endif
#ifndef NITROGEN_OWNED_H
#include "Nitrogen/Owned.h"
#endif
#ifndef NITROGEN_FLAGTYPE_H
#include "Nitrogen/FlagType.h"
#endif
#ifndef NITROGEN_SELECTORTYPE_H
#include "Nitrogen/SelectorType.h"
#endif
#ifndef NITROGEN_IDTYPE_H
#include "Nitrogen/IDType.h"
#endif
#ifndef NITROGEN_MACTYPES_H
#include "Nitrogen/MacTypes.h"
#endif
#ifndef NITROGEN_EVENTS_H
#include "Nitrogen/Events.h"
#endif
#ifndef NITROGEN_QUICKDRAW_H
#include "Nitrogen/Quickdraw.h"
#endif
#ifndef NITROGEN_FONTS_H
#include "Nitrogen/Fonts.h"
#endif
#ifndef NITROGEN_TEXTCOMMON_H
#include "Nitrogen/TextCommon.h"
#endif
#ifndef NITROGEN_PROCESSES_H
#include "Nitrogen/Processes.h"
#endif
#ifndef NITROGEN_APPLEEVENTS_H
#include "Nitrogen/AppleEvents.h"
#endif
#ifndef NITROGEN_COLLECTIONS_H
#include "Nitrogen/Collections.h"
#endif
#ifndef NITROGEN_MACERRORS_H
#include "Nitrogen/MacErrors.h"
#endif
#ifndef NITROGEN_CFSTRING_H
#include "Nitrogen/CFString.h"
#endif
#ifndef NITROGEN_CFUUID_H
#include "Nitrogen/CFUUID.h"
#endif
#ifndef NITROGEN_RESOURCES_H
#include "Nitrogen/Resources.h"
#endif

#ifdef GetMenuRef
#undef GetMenuRef

inline MenuRef GetMenuRef( MenuID menuID )  { return GetMenuHandle( menuID ); }

#endif

namespace Nitrogen
  {
   class MenuAttributesTag {};
   typedef FlagType< MenuAttributesTag, ::MenuAttributes, 0 > MenuAttributes;

   class MenuItemAttributesTag {};
   typedef FlagType< MenuItemAttributesTag, ::MenuItemAttributes, 0 > MenuItemAttributes;
    
   class MenuTrackingModeTag {};
   typedef SelectorType< MenuTrackingModeTag, ::MenuTrackingMode, 0 > MenuTrackingMode;

   class MenuEventOptionsTag {};
   typedef FlagType< MenuEventOptionsTag, ::MenuEventOptions, 0 > MenuEventOptions;
   
   class MenuIDTag {};
   typedef IDType< MenuIDTag, ::MenuID, 0 > MenuID;
   
   template <> struct Disposer< MenuID >: public std::unary_function< MenuID, void >
     {
      void operator()( MenuID id ) const
        {
         ::DeleteMenu( id );
        }
     };

   using ::MenuItemIndex;
   
   class MenuCommandTag {};
   typedef SelectorType< MenuCommandTag, ::MenuCommand, 0 > MenuCommand;
   
   using ::MenuRef;
   
   template <> struct Disposer< MenuRef >: public std::unary_function< MenuRef, void >
     {
      void operator()( MenuRef m ) const
        {
         ::DisposeMenu( m );
        }
     };

   /* ... */
   
   class MenuItemDataFlagsTag {};
   typedef FlagType< MenuItemDataFlagsTag, ::MenuItemDataFlags, 0 > MenuItemDataFlags;
   
   using ::MenuItemDataRec;
   using ::MenuItemDataPtr;
   
   class MenuItemIDTag {};
   typedef IDType< MenuItemIDTag, ::MenuItemID, 0 > MenuItemID;
   
   /* ... */
   
   #pragma mark -
   #pragma mark � Menu Manipulation �
   
   class NewMenu_Failed {};
   
   // 1549
   Owned<MenuRef> NewMenu( MenuID           menuID,
                           ConstStr255Param menuTitle );
   
	inline MenuRef CheckResource( MenuRef menu )
	{
		CheckResource( reinterpret_cast< ::Handle >( menu ) );
		return menu;
	}
	
   // 1563
   inline MenuRef MacGetMenu( ResID resourceID )             { return CheckResource( ::MacGetMenu( resourceID ) ); }
   
   // 1578
   inline void DisposeMenu( Owned<MenuRef> /* theMenu */ )          {}
   
   using ::CalcMenuSize;
   using ::CountMenuItems;
   
   struct MenuFont
     {
      FontID fontID;
      UInt16 fontSize;
     };
   
   MenuFont GetMenuFont( MenuRef menu );

   void SetMenuFont( MenuRef menu, FontID inFontID, UInt16 inFontSize );
   inline void SetMenuFont( MenuRef menu, MenuFont font )         { SetMenuFont( menu, font.fontID, font.fontSize ); }
   
   inline bool GetMenuExcludesMarkColumn( MenuRef menu )          { return ::GetMenuExcludesMarkColumn( menu ); }
   void SetMenuExcludesMarkColumn( MenuRef menu, bool excludesMark ); 
   
   /* ... RegisterMenuDefinition */
   
   Owned<MenuRef> CreateNewMenu( MenuID inMenuID, MenuAttributes inMenuAttributes = MenuAttributes() );

   /* ... CreateCustomMenu */
   
   inline bool IsValidMenu( MenuRef inMenu )                      { return ::IsValidMenu( inMenu ); }
   
   /* ... */
   
   #pragma mark -
   #pragma mark � Menu Item Insertion �
   
   // 2224
   using ::MacAppendMenu;
   
   // 2241
   inline void InsertResMenu( MenuRef menu, ResType type, SInt16 afterItem )  { ::InsertResMenu( menu, type, afterItem ); }
   
   // 2256
   inline void AppendResMenu( MenuRef menu, ResType type )  { ::AppendResMenu( menu, type ); }
   
   // 2270
   inline void MacInsertMenuItem( MenuRef menu, ConstStr255Param itemString, SInt16 afterItem )  { ::MacInsertMenuItem( menu, itemString, afterItem ); }
   
   // 2288
   inline void DeleteMenuItem( MenuRef menu, SInt16 item )  { ::DeleteMenuItem( menu, item ); }
   
   MenuItemIndex AppendMenuItemTextWithCFString( MenuRef            inMenu,
                                                 CFStringRef        inString,
                                                 MenuItemAttributes inAttributes,
                                                 MenuCommand        inCommandID );

   void InsertMenuItemTextWithCFString( MenuRef            inMenu,
                                        CFStringRef        inString,
                                        MenuItemIndex      inAfterItem,
                                        MenuItemAttributes inAttributes,
                                        MenuCommand        inCommandID );

   /* ... */
   
   inline void SetItemMark( MenuRef theMenu, short item, char markChar )  { ::SetItemMark( theMenu, item, markChar ); }
   
   /* ... */
   
   using ::MacEnableMenuItem;
   using ::DisableMenuItem;

   #pragma mark -
   #pragma mark � Menu Bar �
   
   using ::GetMBarHeight;
   using ::MacDrawMenuBar;
   using ::InvalMenuBar;
   
   inline void HiliteMenu( MenuID menuID = MenuID( 0 ) )  { ::HiliteMenu( menuID ); }
   
   inline MenuRef GetMenuHandle( MenuID menuID )  { return ::GetMenuHandle( menuID ); }
   inline MenuRef GetMenuRef   ( MenuID menuID )  { return ::GetMenuRef   ( menuID ); }
   
   Owned< MenuID > MacInsertMenu( MenuRef menu, MenuID beforeID = MenuID( 0 ) );
   inline void MacDeleteMenu( Owned< MenuID > menuID )  {}
   
   /* ... */

   #pragma mark -
   #pragma mark � Menu Item Accessors �
   
   using ::SetMenuItemText;
   
   Str255 GetMenuItemText( MenuRef menu, SInt16 item );
   
   #pragma mark -
   #pragma mark � Attributes �
   
   // 4315
   void ChangeMenuItemAttributes( MenuRef            menu,
                                  MenuItemIndex      item,
                                  MenuItemAttributes setTheseAttributes,
                                  MenuItemAttributes clearTheseAttributes );

   /* ... */
	
	// 5826
	inline MenuID GetMenuID( MenuRef menu )  { return MenuID( ::GetMenuID( menu ) ); }
	
   void RegisterMenuManagerErrors();
  }

#endif

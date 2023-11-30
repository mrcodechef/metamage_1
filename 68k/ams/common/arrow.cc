/*
	arrow.cc
	--------
*/

#include "arrow.hh"

// Mac OS
#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif


#define PACK16( _15, _14, _13,_12,  \
                _11, _10,  _9, _8,  \
                 _7,  _6,  _5, _4,  \
                 _3,  _2,  _1, _0 ) \
	(                            \
		+ (_15 << 15)  \
		| (_14 << 14)  \
		| (_13 << 13)  \
		| (_12 << 12)  \
		| (_11 << 11)  \
		| (_10 << 10)  \
		| ( _9 <<  9)  \
		| ( _8 <<  8)  \
		| ( _7 <<  7)  \
		| ( _6 <<  6)  \
		| ( _5 <<  5)  \
		| ( _4 <<  4)  \
		| ( _3 <<  3)  \
		| ( _2 <<  2)  \
		| ( _1 <<  1)  \
		| ( _0 <<  0)  \
	)

#define _ 0
#define X 1

#define ARROW_DATA  \
{  \
	PACK16( _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_ ),  \
	PACK16( _,X,_,_,_,_,_,_,_,_,_,_,_,_,_,_ ),  \
	PACK16( _,X,X,_,_,_,_,_,_,_,_,_,_,_,_,_ ),  \
	PACK16( _,X,X,X,_,_,_,_,_,_,_,_,_,_,_,_ ),  \
	PACK16( _,X,X,X,X,_,_,_,_,_,_,_,_,_,_,_ ),  \
	PACK16( _,X,X,X,X,X,_,_,_,_,_,_,_,_,_,_ ),  \
	PACK16( _,X,X,X,X,X,X,_,_,_,_,_,_,_,_,_ ),  \
	PACK16( _,X,X,X,X,X,X,X,_,_,_,_,_,_,_,_ ),  \
	PACK16( _,X,X,X,X,X,X,X,X,_,_,_,_,_,_,_ ),  \
	PACK16( _,X,X,X,X,X,_,_,_,_,_,_,_,_,_,_ ),  \
	PACK16( _,X,X,_,X,X,_,_,_,_,_,_,_,_,_,_ ),  \
	PACK16( _,X,_,_,_,X,X,_,_,_,_,_,_,_,_,_ ),  \
	PACK16( _,_,_,_,_,X,X,_,_,_,_,_,_,_,_,_ ),  \
	PACK16( _,_,_,_,_,_,X,X,_,_,_,_,_,_,_,_ ),  \
	PACK16( _,_,_,_,_,_,X,X,_,_,_,_,_,_,_,_ ),  \
	PACK16( _,_,_,_,_,_,_,_,_,_,_,_,_,_,_,_ ),  \
}

#define ARROW_MASK  \
{  \
	PACK16( X,X,_,_,_,_,_,_,_,_,_,_,_,_,_,_ ),  \
	PACK16( X,X,X,_,_,_,_,_,_,_,_,_,_,_,_,_ ),  \
	PACK16( X,X,X,X,_,_,_,_,_,_,_,_,_,_,_,_ ),  \
	PACK16( X,X,X,X,X,_,_,_,_,_,_,_,_,_,_,_ ),  \
	PACK16( X,X,X,X,X,X,_,_,_,_,_,_,_,_,_,_ ),  \
	PACK16( X,X,X,X,X,X,X,_,_,_,_,_,_,_,_,_ ),  \
	PACK16( X,X,X,X,X,X,X,X,_,_,_,_,_,_,_,_ ),  \
	PACK16( X,X,X,X,X,X,X,X,X,_,_,_,_,_,_,_ ),  \
	PACK16( X,X,X,X,X,X,X,X,X,X,_,_,_,_,_,_ ),  \
	PACK16( X,X,X,X,X,X,X,X,X,X,X,_,_,_,_,_ ),  \
	PACK16( X,X,X,X,X,X,X,_,_,_,_,_,_,_,_,_ ),  \
	PACK16( X,X,X,_,X,X,X,X,_,_,_,_,_,_,_,_ ),  \
	PACK16( X,X,_,_,X,X,X,X,_,_,_,_,_,_,_,_ ),  \
	PACK16( X,_,_,_,_,X,X,X,X,_,_,_,_,_,_,_ ),  \
	PACK16( _,_,_,_,_,X,X,X,X,_,_,_,_,_,_,_ ),  \
	PACK16( _,_,_,_,_,_,X,X,X,_,_,_,_,_,_,_ ),  \
}

const Cursor arrow =
{
	ARROW_DATA,
	ARROW_MASK,
	{ 1, 1 }
};
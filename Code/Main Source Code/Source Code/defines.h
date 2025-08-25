#ifndef DEFINES_H
#define DEFINES_H

#include "windows.h"
#include "ctext.h"
#include "appName.h"

#define EXPORT extern "C" __declspec(dllexport)
//#define EXPORT extern  __declspec(dllexport)
// To change the name of the program change these #defines

#define MAX_STR 500

#define MAX_NAME 50 // Maximum length for a macro name or procedure

#define MAX_RETURN_VARIABLES 50   // Maximum number of variables which can be returned from a procedure.
#define MAX_ARGUMENT_VARIABLES 50 // Maximum number of variables which can be passed to a procedure.


#define NO_SOURCE      0
#define CLI_SOURCE     1
#define MACRO_SOURCE   2
#define CONTROL_SOURCE 3
#define VARIABLE_SOURCE 4

namespace WindowLayout{
	enum Constraint {
		LEFT_ALIGN = 1, 
		RIGHT_ALIGN, 
		TOP_ALIGN, 
		BASE_ALIGN, 
		VERT_ALIGN, 
		HORIZ_ALIGN,
		HORIZ_DISTRIBUTE, 
		VERT_DISTRIBUTE, 
		CENTRE_ALIGN, 
		WINDOW_HORIZ_DISTRIBUTE, 
		WINDOW_VERT_DISTRIBUTE, 
		FIT_VERT_DISTRIBUTE, 
		FIT_HORIZ_DISTRIBUTE, 
		EQUAL_VERT_DISTRIBUTE, 
		EQUAL_HORIZ_DISTRIBUTE, 
		LEFT_ATTACH, 
		RIGHT_ATTACH, 
		TOP_ATTACH, 
		BASE_ATTACH
	};

	enum Orientation{
		HORIZONTAL = 1,
		VERTICAL 
	};
}

#define QUOTE ('"')


#define DISPLAY_IMAGE				   1
#define DISPLAY_CONTOURS			   2
#define DISPLAY_INTERP_CONTOURS	   4
#define DISPLAY_VECTORS	            8
#define DISPLAY_WATERFALL	         16

// For GetVariable
#define ALL_VAR                1
#define NOT_ALIAS              2
#define NOT_ALIAS_SPECIFY_TYPE 4
#define DO_NOT_RESOLVE         8
 
// For CopyVariable
#define FULL_COPY      1
#define RESPECT_ALIAS  2
#define MAKE_ALIAS     3

// Type of colormap
#define NORMAL_CMAP     0
#define PLUS_MINUS_CMAP 1

// Font styles
#define FONT_NORMAL    0
#define FONT_ITALIC    1
#define FONT_BOLD      2
#define FONT_UNDERLINE 4

#define FONT_NAME_LENGTH 100

// Syntax colouring styles
#define NO_STYLE       0
#define MACRO_STYLE    1
#define ASM_STYLE      2
#define PAR_STYLE      3

// Class types
#define CLASS_ITEM         0
#define OBJECT_CLASS       1
#define WINDOW_CLASS       2
#define PLOT_CLASS         3
#define XLABEL_CLASS       4
#define YLABEL_CLASS       5
#define YLABEL_LEFT_CLASS  6
#define YLABEL_RIGHT_CLASS 7
#define TITLE_CLASS        8
#define TRACE_CLASS        9
#define AXES_CLASS         10
#define GRID_CLASS         11
#define USER_CLASS         12
#define INSET_CLASS			13

// Readonly
#define gReadOnly  0

typedef struct 
{
   float left;
   float right;
   float top;
   float bottom;
}FloatRect;

typedef struct 
{
   float x;
   float y;
   float w;
   float h;
}FloatRect2;

typedef struct
{
   float r;
   float i;
}complex;


// Procedure structure (for cacheing procedure)

typedef struct
{
   char *procName;
   char *macroName;
   char *macroPath;
   char *procedure;
   long startLine;
}
ProcedureInfo;



typedef struct
{
   float xs,xo;  // ww*xs+xo
   float ys,yo;  // wh*ys+yo
   float ws,wo;  // ww*ws+wo
   float hs,ho;  // ww*hs+ho
   bool region;  // Region or window based
}
ObjPos;

#define WINDATA    0
#define OBJDATA    1
#define AXOBJECT   2

typedef struct
{
   char* data;
   short winType;
}
WinInfo;

typedef struct
{
   short type; // Type of object 
   void *data; // Pointer to class object
   long code;  // Validation code
}
ClassData;

typedef struct
{
   short type;
   char *name;
   char *args;
}
ClassItem;

typedef struct
{
	CText path;
	CText macro;
	CText procedure;
	CText command;
	CText description;
	long lineNr;
	CText lastError;
	CText type;
	bool errorFound;
	bool blocked;
}
CommandInfo;


typedef enum {
	NO_VAXIS_SIDE,
	LEFT_VAXIS_SIDE,
	RIGHT_VAXIS_SIDE
} VerticalAxisSide;


#define templateFlag   (OFN_EXPLORER | OFN_ENABLETEMPLATE | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_ENABLEHOOK | OFN_NONETWORKBUTTON | OFN_ENABLESIZING)
#define noTemplateFlag  (OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_ENABLEHOOK | OFN_NONETWORKBUTTON | OFN_ENABLESIZING)

#define IsWhiteSpace(c)  (c == ' ' || c == '\t')

#define BEFORE_PROCEDURE " +-*/%^=(,[$\t<>"
#define AFTER_PROCEDURE " ()"
#define BEFORE_OPERAND   " .$:+-*/%^=(,<>\\[\"\t\n"
#define AFTER_OPERAND   " .$:+-*/%^=(),<>\\[]\"\t\n"

//#define GetWinDataClass(win)  ((WinData*)GetWindowLong(win,GWL_USERDATA))
#define GetWinDataClass(hWnd)  rootWin->FindWinByHWND(hWnd)
#define PI 3.1415927

//#ifndef __MWERKS__
//   #define WM_MOUSEWHEEL  0x020A
#define isnan _isnan
//#endif


#define MAX_FLOAT 3.4e38

#define ABORT_COLOR_CHOICE 0x01000000

#define MACRO_RUNNING            0
#define MACRO_ABORTED            1

#define CLASS_MEMBER   0
#define CLASS_FUNCTION 1
#define CLASS_ARRAY    2

#define RGBA(r,g,b,a)        ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)|(((DWORD)(BYTE)(a))<<24)))
#define GetAValue(rgba)      (LOBYTE((rgba)>>24))

#endif // define DEFINES_H

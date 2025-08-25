#ifndef GLOBALS_H
#define GLOBALS_H

#include <commdlg.h>
#include <vector>
#include "defines.h"

class Interface;

const short AXES_FONT      = 1;
const short LABEL_FONT     = 2;
const short TITLE_FONT     = 3;
const short SYMBOL_FONT    = 4;

const short AXES_DLG       = 0;
const short RANGE_DLG      = 1;
const short PLOT_COLOR_DLG = 2;
const short TRACE_DLG      = 3;
const short IMPORT_DLG     = 4;
const short EXPORT_DLG     = 5;
const short PRINT_DLG      = 6;

const short OK                = 0;
const short ERR               = -1;
const short INVALID_ARG       = -3;
const short THROW             = -4;
const short ABORT             = -5;
const short RETURN_FROM_MACRO = -6;
const short RETURN_FROM_DLL   = -7;
const short STOP              = -8;
const short CMD_NOT_FOUND     = -10;


const short PRINT = 1;
const short DISPLAY = 2;
const short IMAGEFILE = 3;
const short CLIPBOARD = 4;

const short COLOR_PLOT_BK = 1;
const short COLOR_PLOT_TRACE = 2;

const short BLACK_AND_WHITE = 1;
const short COLOUR      = 2;

const long XYDATA = 2;    // Only Y array present
const long ASCII  = 4;    // Ascii (text) data
const long BINARY = 8;    // Binary data
const long BIG_ENDIAN = 16; // Byte reversed e.g. Mac
const long FLOAT_32 = 32; // 32 bit floating point data
const long FLOAT_64 = 64; // 64 bit floating point data
const long INT_32   = 128; // 32 bit signed integer data
const long INT_16   = 256; // 16 bit signed integer data
const long INT_8   = 512;  // 8 bit unsigned integer data
const long SPACE_DELIMIT = 1024; // add spaces between ascii x-y pairs
const long TAB_DELIMIT   = 2048; // add tabs between ascii x-y pairs
const long COMMA_DELIMIT = 4096; // add commas between ascii x-y pairs
const long REAL_DATA = 8192;          // data is real
const long COMP_DATA = 16384;         // data is complex
const long X_COMP_DATA = 32768;       // data is x-complex
const long OVERWRITE_MODE = 65536;    // overwrite data
const long APPEND_MODE = 131072;       // append data
const long RETURN_DELIMIT = 262144;    // add returns between ascii x-y pairs
const long DOUBLE_DATA = 524288;          // data is double


EXPORT extern void TextMessage(const char *const text,...);
EXPORT extern void ErrorMessage(const char *const text,...);
EXPORT extern void Error(Interface *itfc, char *text,...);

extern HCURSOR SetMyCursor(HCURSOR);

extern CRITICAL_SECTION cs1DPlot;
extern CRITICAL_SECTION cs2DPlot;
extern CRITICAL_SECTION csVariable;
extern CRITICAL_SECTION csAssignVariable;
extern CRITICAL_SECTION csCache;
extern CRITICAL_SECTION csThread;

enum MouseMode {DO_NOTHING, SHOW_DATA, SELECT_RECT, SELECT_COLUMN, SELECT_ROW, MOVE_PLOT, USER_DEFINED};

/* 
 Use as the target function of std::for_each to delete
 object pointers from collections
*/
struct delete_object
{
  template <typename T>
  void operator()(T *ptr){ delete ptr;}
};

extern COLORREF gMainWindowColor;

extern bool gUsingWine;

#endif // define GLOBALS_H
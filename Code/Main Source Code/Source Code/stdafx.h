#ifndef STDAFX_H
#define STDAFX_H

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#define _CRT_SECURE_NO_DEPRECATE 1
#pragma once


#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
//#define _WIN32_WINNT WINVER
#pragma pack(push,8)
#include <windows.h>
#pragma pack(pop)

//#define WINVER 0x0602

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <assert.h>


#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
//
//#if defined _M_IX86
//#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.GdiPlus' version='1.1.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
//#elif defined _M_IA64
//#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.GdiPlus' version='1.1.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
//#elif defined _M_X64
//#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.GdiPlus' version='1.1.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
//#endif


#endif // define STDAFX_H

BOOL MyInvalidateRect(HWND,const RECT*,BOOL);
void MySetFocus(HWND hWnd);
void MySetCursor(HCURSOR curs);
BOOL MyUpdateWindow(HWND);
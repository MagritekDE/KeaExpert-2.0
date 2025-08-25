#ifndef DEFINEWINDOWS_H
#define DEFINEWINDOWS_H

#include "defines.h"

class Interface;

// Default windows used in program

extern HWND prospaWin;
//extern HWND plot1DWin;
//extern HWND plot2DWin;
//extern HWND openGlWin;
//extern HWND plot3DWin;
extern HWND cliWin;
//extern HWND tbWnd1D;
//extern HWND tbWnd2D;
extern HWND tbWnd3D;
//extern HWND statusWnd1D;
//extern HWND statusWnd2D;
extern HWND statusWnd3D;
extern HWND cliEditWin;
//extern HWND editWin;
extern HWND editStatusWnd;
//extern HWND editSyntaxWnd;
extern HWND cliStatusWnd;
extern HWND cliSyntaxWnd;

extern HCURSOR RectangleCursor;
extern HCURSOR OneDCursor;
extern HCURSOR TwoDCursor;
extern HCURSOR HorizDivCursor;
extern HCURSOR VertDivCursor;
extern HCURSOR CtrlCursor;
extern HCURSOR CtrlCursor2;
extern HCURSOR CrossCursor;
extern HCURSOR SquareCursor;

extern FloatRect2 prospaRect;
extern FloatRect2 plot1DRect;
extern FloatRect2 plot2DRect;
extern FloatRect2 plot3DRect;
extern FloatRect2 cliRect;
extern FloatRect2 editRect;

extern HPEN regionPen;
extern HPEN bkgPen;

int  MoveAppWindow(Interface* itfc ,char[]);
int MiscInit(void);

#endif // define DEFINEWINDOWS_H
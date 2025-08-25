#include "stdafx.h"
#include "defineWindows.h"
#include "globals.h"
#include "interface.h"
#include "main.h"
#include "mymath.h"
#include "plot.h"
#include "plotwindefaults.h"
#include "prospaResource.h"
#include "rgbColors.h"
#include "scanstrings.h"
#include "TracePar.h"
#include <commctrl.h>
#include "memoryLeak.h"

#define NUM1DBUTTONS 24
#define NUM2DBUTTONS 21

#define WS_OL_NOMIN (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX)
#define WS_CLD_NOMIN (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX)
//#define WS_CLD_NOMIN (WS_CHILD | WS_CAPTION | WS_THICKFRAME | WS_CLIPSIBLINGS | WS_SYSMENU | WS_MAXIMIZEBOX)

HWND prospaWin;
HWND cliWin;
HWND cliEditWin;
HWND cur3DWin;

FloatRect2 prospaRect;
FloatRect2 plot1DRect;
FloatRect2 plot2DRect;
FloatRect2 plot3DRect;
FloatRect2 cliRect;
FloatRect2 editRect;

HCURSOR RectangleCursor;
HCURSOR CrossCursor;
HCURSOR SquareCursor;
HCURSOR CtrlCursor;
HCURSOR CtrlCursor2;
HCURSOR OneDCursor;
HCURSOR TwoDCursor;
HCURSOR HorizDivCursor;
HCURSOR VertDivCursor;

HPEN regionPen;
HPEN bkgPen;

int statusParts[6];
static char winName[] = "Command line interface";
PlotWinDefaults *pwd; // Defaults for plot windows

void SetRectangle(FloatRect2 *rect, float x, float y, float w, float h);

int MiscInit()
{
   
// Define 1D and 2D plot regions ***********************************
   pwd = new PlotWinDefaults;

// Some window dimensions
   resizableWinBorderSize = GetSystemMetrics(SM_CYSIZEFRAME);
   fixedSizeWinBorderSize = GetSystemMetrics(SM_CYFIXEDFRAME);
   titleBarNMenuHeight = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU) + GetSystemMetrics(SM_CYSIZEFRAME);
   titleBarHeight = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CXFIXEDFRAME);

#if(_WIN32_WINNT >= 0x0600)
   resizableWinBorderSize += GetSystemMetrics(SM_CXPADDEDBORDER);
   fixedSizeWinBorderSize += GetSystemMetrics(SM_CXPADDEDBORDER);
   titleBarNMenuHeight += GetSystemMetrics(SM_CXPADDEDBORDER);
   titleBarHeight += GetSystemMetrics(SM_CXPADDEDBORDER);
#endif

// Load some cursors ***********************************************
   RectangleCursor = LoadCursor(prospaInstance,"RectangleCursor");
   OneDCursor = LoadCursor(prospaInstance,"OneDCursor");
   TwoDCursor = LoadCursor(prospaInstance,"TwoDCursor");
   CrossCursor = LoadCursor(prospaInstance,"CURSOR_CROSS");
   SquareCursor = LoadCursor(prospaInstance,"CURSOR_SQUARE");
   CtrlCursor = LoadCursor(prospaInstance,"CONTROLCURSOR");
   CtrlCursor2 = LoadCursor(prospaInstance,"CONTROLCURSOR2");
   VertDivCursor = LoadCursor(prospaInstance,"VertDividerCursor");
   HorizDivCursor = LoadCursor(prospaInstance,"HorizDividerCursor");

   regionPen = CreatePen(PS_DOT,0,RGB(0,0,0));
   bkgPen = CreatePen(PS_SOLID,0,GetSysColor(COLOR_BTNFACE));

   return(OK);						 
}


/*******************************************************************
  Set plot window parameter defaults
********************************************************************/

PlotWinDefaults::PlotWinDefaults()
{
   autoXScale = true;
   autoYScale = true;

// Ticks
   axesMode = PLOT_AXES_BOX;
	ProspaFont ticksFont(DEFAULT_FONT_NAME, 0x000000, 10, 0);
	this->ticks = new Ticks(10,5,20,5, &ticksFont, RGB_BLACK);
// Trace and symbols
   tracePar.setRealColor        (0x00FFFF);
   tracePar.setImagColor        (0x008080);

   tracePar.setBarColor         (0x0000FF);
   tracePar.setRealStyle        (0);
   tracePar.setImagStyle        (2);
   tracePar.setShowErrorBars    (false);
   tracePar.setBarFixedHeight   (0);
   tracePar.setErrorBarsStored  (false);
   tracePar.setTraceType        (PLOT_TRACE_LINES);
   tracePar.setTraceWidth       (0);
   tracePar.setRealSymbolColor  (0x0000FF);
   tracePar.setImagSymbolColor  (0x000080);
   tracePar.setSymbolType       (PLOT_SYMBOL_NO_SYMBOL);
   tracePar.setSymbolSize       (3);

   bkColor       = 0x640000;
   borderColor   = GetSysColor(COLOR_BTNFACE); //0xB19983;
   gridColor     = 0x8C0000;
   fineGridColor = 0x780000;

// Labels
   strcpy(axesFontName,DEFAULT_FONT_NAME);
   strcpy(labelFontName,DEFAULT_FONT_NAME);
   strcpy(titleFontName,DEFAULT_FONT_NAME);
   labelFontColor    = 0x000000; 
   titleFontColor    = 0x000000;  
   labelFontStyle    = 0; 
   titleFontStyle    = 0;  
   titleFontSize  = 12;
   labelFontSize  = 10;
   
// Antialiasing
	antiAliasing = true;

// Zoom rectangle
	zoomBkgColor = RGBA(255,128,64,100);    // Zoom rectangle color
	zoomBorderColor = RGBA(168,54,0,200);   // Zoom rectangle color
   zoomRectMode = "solidrect";             // dotted zoom rect or not
}

PlotWinDefaults::~PlotWinDefaults()
{
	delete ticks;
}


/*******************************************************************
  Modify plot default parameters
*******************************************************************/

void PlotWinDefaults::Modify(int parameter, COLORREF col)
{
   switch(parameter)
   {
      case(ID_PREF_BORDER_COLOR):
         pwd->borderColor = col;   
         break;      
      case(ID_PREF_BG_COLOR):
         pwd->bkColor = col;
         break;                  
      case(ID_PREF_AXES_COLOR):
			pwd->axesColor = col; 
         break;                  
      case(ID_PREF_AXES_FONT_COLOR):
         pwd->ticks->setFontColor(col);  
         break;                   
      case(ID_PREF_GRID_COLOR):
         pwd->gridColor = col;
         break;  
      case(ID_PREF_FINE_GRID_COLOR):
         pwd->fineGridColor = col;
         break; 
      case(ID_PREF_TITLE_COLOR):
         pwd->titleFontColor = col;
         break; 
      case(ID_PREF_LABEL_COLOR):
         pwd->labelFontColor = col;
         break;                              
   }          
}

/*********************************************************************************************
   Get and set the axes type
*********************************************************************************************/

char* PlotWinDefaults::GetAxesTypeStr()
{
   if(axesMode == PLOT_AXES_CORNER)
      return("corner");
   else if(axesMode == PLOT_AXES_CROSS)
      return("cross");
   else if(axesMode == PLOT_AXES_BOX)
      return("box");
   else if(axesMode == PLOT_X_AXIS)
      return("xaxis");
   else if(axesMode == PLOT_X_AXIS_BOX)
      return("xaxis_box");
   else if(axesMode == PLOT_X_AXIS_CROSS)
      return("xaxis_cross");
   else if(axesMode == PLOT_Y_AXIS)
      return("yaxis");
   else if(axesMode == PLOT_Y_AXIS_BOX)
      return("yaxis_box");
   else if(axesMode == PLOT_Y_AXIS_CROSS)
      return("yaxis_cross");
   else if(axesMode == PLOT_AXIS_BOX_Y_INDEPENDENT)
      return("box_y_independent");
   else if(axesMode == PLOT_NO_AXES)
      return("none");
   
   return("error");
}


bool PlotWinDefaults::SetAxesType(CText type)
{
   if(type == "corner")
      axesMode = PLOT_AXES_CORNER;
   else if(type == "cross")
      axesMode = PLOT_AXES_CROSS;
   else if(type == "box")
      axesMode = PLOT_AXES_BOX;
   else if(type == "xaxis")
      axesMode = PLOT_X_AXIS;
   else if(type == "xaxis_box")
      axesMode = PLOT_X_AXIS_BOX;
   else if(type == "xaxis_cross")
      axesMode = PLOT_X_AXIS_CROSS;
    else if(type == "yaxis")
      axesMode = PLOT_Y_AXIS;
   else if(type == "yaxis_box")
      axesMode = PLOT_Y_AXIS_BOX;
   else if(type == "yaxis_cross")
      axesMode = PLOT_Y_AXIS_CROSS;
   else if(type == "box_y_independent")
      axesMode = PLOT_AXIS_BOX_Y_INDEPENDENT;
   else if(type == "none")
      axesMode = PLOT_NO_AXES;
   else
      return(1);
   return(0);
}
    


/*********************************************************************************************
   Resize and move an application window
*********************************************************************************************/

int MoveAppWindow(Interface* itfc ,char args[])
{
   short nrArgs;
   float xf,yf,wf,hf;
   long sx,sy,sw,sh;
   long x,y,w,h;
   long width,height;
   RECT r,rp;
   
// Get initial values for the prospa window (the dimensions of the
//  prospa window relative to the whole screen
   GetWindowRect(prospaWin,&rp);
   w = rp.right-rp.left;
   h = rp.bottom-rp.top;
   x = rp.left;
   y = rp.top;
   SystemParametersInfo(SPI_GETWORKAREA,0,&r,0);
   sx = r.left;
   sy = r.top;
   sw = r.right-r.left;
   sh = r.bottom-r.top;
   xf = (x-sx)/(float)sw;
   yf = (y-sy)/(float)sh;
   wf = w/float(sw);
   hf = h/float(sh);
   strcpy(winName,"prospa");

// Get window name and position from user *************   
   if((nrArgs = ArgScan(itfc,args,5,"name,x,y,w,h","eeeee","sffff",winName,&xf,&yf,&wf,&hf)) < 0)
     return(nrArgs);  

// Get screen dimensions **************
   HDC hdc = GetDC(prospaWin);
   ReleaseDC(prospaWin,hdc);

// Get free window space (minus toolbars)
   if(!strcmp(winName,"prospa"))
   {
      SystemParametersInfo(SPI_GETWORKAREA,0,&r,0);
      width = r.right-r.left;
      height = r.bottom-r.top;
   }
   else
   {
      GetWindowRect(prospaWin,&r);
      width = r.right-r.left-2*resizableWinBorderSize;
      height = r.bottom-r.top-(resizableWinBorderSize + titleBarNMenuHeight);
      r.top += titleBarNMenuHeight;
      r.left += resizableWinBorderSize;
      r.right -= resizableWinBorderSize;
      r.bottom -= (resizableWinBorderSize + titleBarNMenuHeight);
   }

// Make sure we aren't outside the screen boundaries  
   if(xf < 0) xf = 0;
   if(yf < 0) yf = 0;
   if(wf > 1) wf = 1;
   if(hf > 1) hf = 1;
   if(xf + wf > 1) xf = 1-wf;
   if(yf + hf > 1) yf = 1-hf;

// Scale window dimensions to screen ***
   x = nint(xf*width)+r.left;
   y = nint(yf*height)+r.top;
   w = nint(wf*width);
   h = nint(hf*height);
      
// Position the windows ***************   
   if(!strcmp(winName,"prospa"))
   {
      SetRectangle(&prospaRect,xf,yf,wf,hf);
		MoveWindow(prospaWin,x,y,w,h,true);
   }
   else
   {
      ErrorMessage("invalid window name '%s'",winName);
      return(ERR);
   }

	itfc->nrRetValues = 0;
   return(OK);
}

void SetRectangle(FloatRect2 *rect, float x, float y, float w, float h)
{
	rect->x = x;
	rect->y = y;
	rect->w = w;
	rect->h = h;
}


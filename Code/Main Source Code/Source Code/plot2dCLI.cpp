#define WINVER _WIN32_WINNT 
#include "stdafx.h"
#include "plot1DCLI.h"
#include "plot2DCLI.h"
#include "allocate.h"
#include "cArg.h"
#include "guiWindowClass.h"
#include "interface.h"
#include "defineWindows.h"
#include "dialog_utilities.h"
#include "edit_class.h"
#include "globals.h"
#include "guiModifyObjectParameters.h"
#include "guiWindowsEvents.h"
#include "main.h"
#include "message.h"
#include "metafile.h"
#include "mymath.h"
#include "plot.h"
#include "plot2dEvents.h"
#include "plot2dFiles.h"
#include "plot_dialog_1d.h"
#include "PlotWindow.h"
#include "print.h"
#include "process.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "string_utilities.h"
#include "variablesOther.h"
#include <algorithm>
#include "memoryLeak.h"

using std::find;

// Locally defined procedures

short ProcessDragEvents(Interface* itfc, Plot*, HCURSOR, HCURSOR, short, HWND, char*);
short ProcessDragAndScrollEvents(Interface* itfc, Plot*, HCURSOR, HCURSOR, short, HWND, char*);
short ProcessScrollEvents(Interface* itfc, Plot*, HCURSOR, HCURSOR, short, HWND, char*);
short ProcessMoveEvents(Interface *itfc,Plot*,HCURSOR,HCURSOR,short,HWND,char*);

// Externally visible variables

float **gDefaultColorMap = NULL;
long gDefaultColorMapLength = 0;

// Allow Prospa to save 2D images in old formats
int ImageFileSaveVersion(Interface* itfc, char args[])
{
   short n;
   extern long plot2DSaveVersion;

   long version = nint(fileVersionConstantToNumber(plot2DSaveVersion,2));

   if((n = ArgScan(itfc,args,1,"version","e","l",&version)) < 0)
      return(n);

   version =  fileVersionNumberToConstant(version,2);
   if(version == -1)
      return(ERR);

   plot2DSaveVersion =  version;

   return(OK);
}


int Clear2D(Interface* itfc, char args[])
{
   if(!Plot2D::curPlot())
   {
      ErrorMessage("no 2D plot defined");
      return(ERR);
   }

   Plot2D::curPlot()->plotParent->MakeMultiPlot(1,1);   
   Plot2D::curPlot()->initialiseMenuChecks("clear");
   MyInvalidateRect(Plot2D::curPlot()->win,NULL,false);
   return(OK);
}

int SetWindowMargins(Interface *itfc, char args[])
{
   short left,right,top,base;
   short n;

	if(!Plot::curPlot())
   {
      ErrorMessage("no plot defined");
      return(ERR);
   }
	Margins& originalMargins = Plot::curPlot()->getMargins();
	left  = originalMargins.left();
   right = originalMargins.right();
   top   = originalMargins.top();
   base  = originalMargins.base();
   
   if((n = ArgScan(itfc,args,1,"left,right,top,base","eeee","dddd",&left,&right,&top,&base)) < 0)
      return(n);

   PlotWindow *pp = Plot::curPlot()->plotParent;

	Margins m(left,right,top,base);

	for(Plot* p: pp->plotList())
   {
		p->setMargins(m);
   }

// Make sure window is redrawn
	MyInvalidateRect(Plot::curPlot()->win,NULL,false);

	itfc->nrRetValues = 0;
   return(OK);
}

int DefaultWindowMargins(Interface *itfc, char args[])
{
   short left,right,top,base;
   short n;
	static short dim;

	if(!Plot::curPlot())
   {
      ErrorMessage("no plot defined");
      return(ERR);
   }
	if(dim == 1)
	{
		left  = defaultMargins1D.left();
		right = defaultMargins1D.right();
		top   = defaultMargins1D.top();
		base  = defaultMargins1D.base();
	}
	else
 	{
		left  = defaultMargins2D.left();
		right = defaultMargins2D.right();
		top   = defaultMargins2D.top();
		base  = defaultMargins2D.base();
	}  
   if((n = ArgScan(itfc,args,1,"dim,left,right,top,base","eeeee","ddddd",&dim,&left,&right,&top,&base)) < 0)
      return(n);
	

	if(n == 1)
	{
		if(dim == 1)
		{
			left  = defaultMargins1D.left();
			right = defaultMargins1D.right();
			top   = defaultMargins1D.top();
			base  = defaultMargins1D.base();
		}
		else
 		{
			left  = defaultMargins2D.left();
			right = defaultMargins2D.right();
			top   = defaultMargins2D.top();
			base  = defaultMargins2D.base();
		} 
		itfc->retVar[1].MakeAndSetFloat(left);
		itfc->retVar[2].MakeAndSetFloat(right);
		itfc->retVar[3].MakeAndSetFloat(top);
		itfc->retVar[4].MakeAndSetFloat(base);
		itfc->nrRetValues = 4;
		return(OK);
	}

	Margins m(left,right,top,base);

//   PlotWindow *pp = Plot::curPlot()->plotParent;
//
//	for(Plot* p: pp->plotList())
//   {
//		p->setMargins(m);
//   }
//
//// Make sure window is redrawn
//	MyInvalidateRect(Plot::curPlot()->win,NULL,false);

	if(dim == 1)
	   defaultMargins1D = m;
	else if(dim == 2)
	  defaultMargins2D = m;
	else
	{
		ErrorMessage("Invalid dimensions (1/2)");
		return(ERR);
	}

	itfc->nrRetValues = 0;
   return(OK);
}

/************************************************************************
    Extract the data range (current/full/stored) in the 2D plot 

  Syntax: (max,min) = getdatarange("current")  or
          (max,min) = getdatarange("full") or
          (max,min) = getdatarange("displayed")

************************************************************************/

int GetDataRange(Interface* itfc ,char args[])
{
   short r;
   static CText mode;
   float minVal,maxVal;
	Plot2D* cur2DPlot = Plot2D::curPlot();

// Get desired mode *********************************
   if((r = ArgScan(itfc,args,1,"mode","e","t",&mode)) < 0)
      return(r); 

// Check for valid 2D plot **************************
   if(!cur2DPlot)
   {
      ErrorMessage("no current 2D plot");
      return(ERR);
   }
 
// Extract data range in complete data set **********
   if(mode == "full") 
   {
      cur2DPlot->FindFullMatrixRange(minVal,maxVal);	          
   }
// Extract data range from visible data set *********
   else if(mode == "current")
   {
      cur2DPlot->FindMatrixRange(minVal,maxVal);      
   } 
// Extract currently displayed data range ***********
   else if(mode == "displayed")
   {
      minVal = cur2DPlot->minVal();
      maxVal = cur2DPlot->maxVal();     
   } 
// Invalid mode passed to command ********************
   else
   {
      ErrorMessage("invalid parameter. Must be one of {full,current,displayed}");
      return(ERR);
   }   

// Return max and min values *************************
   itfc->retVar[1].MakeAndSetFloat(minVal);
	itfc->retVar[2].MakeAndSetFloat(maxVal); 
   itfc->nrRetValues = 2; 
   
   return(OK);
}

// Zoom into the current 2D plot
int Zoom2D(Interface *itfc, char args[])
{
	Plot2D* cur2DPlot = Plot2D::curPlot();
   if(!cur2DPlot)
   {
      ErrorMessage("2D plot not defined");
      return(ERR);
   }

   short r = cur2DPlot->Zoom(itfc,args);
   if(r != -2)
      itfc->nrRetValues = 0;
	
   return(OK); 
}


/*****************************************************************************
*                   Display a 2D matrix as an intensity image 
*****************************************************************************/

int DisplayMatrixAsImage(Interface* itfc ,char arg[])
{
   short r;
	MSG msg;

	Plot2D* cur2DPlot = Plot2D::curPlot();

   if(!cur2DPlot)
   {
      ErrorMessage("no 2D plot window present");
      return(ERR);
   }

   while(cur2DPlot->plotParent->isBusy())
      PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);

   EnterCriticalSection(&cs2DPlot);

   if(!cur2DPlot->plotParent->isBusy())
	{
		cur2DPlot->plotParent->setBusy(true);
      LeaveCriticalSection(&cs2DPlot);
      r = cur2DPlot->Draw2DImage(itfc, arg);
	   cur2DPlot->makeCurrentPlot();
      cur2DPlot->plotParent->setBusyWithCriticalSection(false);
   }
   else
		LeaveCriticalSection(&cs2DPlot);

   itfc->nrRetValues = 0;

   return(r);
}


/*****************************************************************************
* Specify whether images in the 2D windows should be immediately draw or not
*****************************************************************************/

int DrawImage(Interface* itfc ,char args[])
{
   short nrArgs;
   CText draw;
	Plot2D* cur2DPlot = Plot2D::curPlot();

// Check for valid 2D plot **************************
   if(!cur2DPlot)
   {
      ErrorMessage("no current 2D plot");
      return(ERR);
   }

   if((nrArgs = ArgScan(itfc,args,1,"true/false","e","t",&draw)) < 0)
     return(nrArgs);
   
   if(draw == "true" || draw == "yes" || draw == "on")
   {
      cur2DPlot->plotParent->updatePlots(true);
   }
   else if(draw == "false" || draw == "no" || draw == "off")
   {
      cur2DPlot->plotParent->updatePlots(false);
   }
   else
   {
      ErrorMessage("invalid parameter");
      return(ERR);
   }

   cur2DPlot->DisplayAll(false); 

   return(0);     
}



/*****************************************************************************
* Specify a colormap which will be used to redraw the current 2D plot
*****************************************************************************/

int SetColorMap(Interface* itfc ,char arg[])
{
   short r;
   Variable colorVar;
	MSG msg;

	Plot2D* cur2DPlot = Plot2D::curPlot();

   if(!cur2DPlot) 
   {
      ErrorMessage("no current 2D plot");
      return(ERR);
   }

// Get current colormap if present
   if(cur2DPlot->colorMap())
      colorVar.MakeAndLoadMatrix2D(cur2DPlot->colorMap(),3,cur2DPlot->colorMapLength());

// Get new colormap name ******************************  
   if((r = ArgScan(itfc,arg,1,"colormap","e","v",&colorVar)) < 0)
     return(r);

   if(cur2DPlot->SetColorMap(&colorVar) == ERR)
      return(ERR);

// Make sure window is redrawn ***********************  
   cur2DPlot->DisplayAll(false);               
   
	itfc->nrRetValues = 0;
   return(OK);
}  


/*****************************************************************************
* Get the current colormap and return as a matrix
*****************************************************************************/

int GetColorMap(Interface *itfc, char args[])
{
	
	Plot2D* cur2DPlot = Plot2D::curPlot();

   if(!cur2DPlot)
   {
      ErrorMessage("no 2D plot defined");
      return(ERR);
   }   

   if(cur2DPlot->colorMap() && cur2DPlot->colorMapLength()>0)
   {
      itfc->retVar[1].MakeAndLoadMatrix2D(cur2DPlot->colorMap(),3,cur2DPlot->colorMapLength());
      itfc->nrRetValues = 1;
   }
   else
   {
      ErrorMessage("no colormap defined");
      return(ERR);
   }           
    
   return(OK);
}  


int ContourPlot(Interface* itfc ,char args[])
{
   short nrArgs;
   short contMode = DISPLAY_CONTOURS;
   long nr;
   float *levels = NULL;
   Variable levelVar,fixedColorVar;
	Plot2D* cur2DPlot = Plot2D::curPlot();
   short fixedLevels = 0;

// Check there is a current 2D plot *************
   if(!cur2DPlot)
   {
      ErrorMessage("no 2D plot defined");
      return(ERR);
   }

// Initialize defaults **************************
   nr = cur2DPlot->nrContourLevels;
   contMode = cur2DPlot->drawMode();
   levels = cur2DPlot->contourLevels;
   if(levels)
      levelVar.MakeMatrix2DFromVector(levels,nr,1);
   else
      levelVar.MakeAndSetFloat(nr);
   COLORREF col = cur2DPlot->fixedContourColor();
   float *colorf = new float[3];
   colorf[0] = GetRValue(col);
   colorf[1] = GetGValue(col);
   colorf[2] = GetBValue(col);
   fixedColorVar.MakeMatrix2DFromVector(colorf,3,1);
   delete [] colorf;
   fixedLevels = (short)cur2DPlot->useFixedContourColor();
                       
// Get number of number of levels and the color scheme *************
   if((nrArgs = ArgScan(itfc,args,1,"number/levels, [[mode], fixed_color, use_fixed_levels]","eeee","vdvd",&levelVar,&contMode,&fixedColorVar,&fixedLevels)) < 0)
     return(nrArgs);  

// Clear levels if necessary
   if(levels)
   {
      delete [] levels;
      levels = NULL;
      cur2DPlot->contourLevels = NULL;
   }

// Get levels
   if(levelVar.GetType() == MATRIX2D && levelVar.GetDimY() == 1)
   {
      levels = new float[levelVar.GetDimX()];
      for(int x = 0; x < levelVar.GetDimX(); x++)
         levels[x] = levelVar.GetMatrix2D()[0][x];
      nr = levelVar.GetDimX();
   }
   else if(levelVar.GetType() == FLOAT32)
   {
      nr = nint(levelVar.GetReal());
   }
   else
   {
      ErrorMessage("Invalid first argument, should be integer or level array");
      return(ERR);
   }
   cur2DPlot->setUseFixedContourColor(false);

// Get fixed color
   if(nrArgs >= 3)
   {
      if(fixedColorVar.GetType() == MATRIX2D && fixedColorVar.GetDimX() == 3 && fixedColorVar.GetDimY() == 1 && fixedLevels >= 0)
      {
         float *fcol = fixedColorVar.GetMatrix2D()[0];
         COLORREF col = RGB(nint(fcol[0]),nint(fcol[1]),nint(fcol[2]));
         cur2DPlot->setFixedContourColor(col);
         cur2DPlot->setUseFixedContourColor(fixedLevels>0);
      }
      else
      {
         ErrorMessage("Invalid fixed contour color");
         return(ERR);
      }
   }

// Set mode
   cur2DPlot->setDrawMode(contMode);
   
// Determine levels to plot *********************
   cur2DPlot->nrContourLevels = nr;
   if(cur2DPlot->contourLevels)
      delete [] cur2DPlot->contourLevels;
   cur2DPlot->contourLevels = levels;

// Make sure window is redrawn *********************** 
   cur2DPlot->DisplayAll(false); 
 
	itfc->nrRetValues = 0;
   return(0);
}

 
/*****************************************************************
             Set intensity range for imaging
******************************************************************/

int SetImageRange(Interface* itfc ,char arg[])
{
   short r;
   float minV,maxV;
	
	Plot2D* cur2DPlot = Plot2D::curPlot();

// Check there is a current 2D plot *************
   if(!cur2DPlot)
   {
      ErrorMessage("No 2D plot defined");
      return(ERR);
   }

// Default range
   minV = cur2DPlot->minVal();
   maxV = cur2DPlot->maxVal();
      
// Extract range
   if((r = ArgScan(itfc,arg,2,"min,max","ee","ff",&minV,&maxV)) < 0)
      return(r); 
      
   if(minV > maxV)
   {
      ErrorMessage("Invalid range");
      return(ERR);
   }

// Set new range
   cur2DPlot->setMinVal(minV);
   cur2DPlot->setMaxVal(maxV);

// Redraw data set
   cur2DPlot->DisplayAll(false); 
         
	return(0);
}

/*********************************************************************************************
  Display a row, column or cross cursor in the 1D or 2D window and call a procedure each time
  the mouse moves.
 ********************************************************************************************/

void RemoveOldCursor(HDC hdc, long plotLeft, long plotWidth, long plotTop, long plotHeight, long x0, long y0, short cursor);

// Check
int TrackCursor(Interface* itfc ,char args[])
{
   const short ARROW = 0;
   const short CROSS = 1;
   const short ROW   = 2;
   const short COL   = 3;
   short r;
   CText proc;
   CText which;
   CText type;
   Plot *plot;
   HWND status;
   HCURSOR outofboundsCursor = OneDCursor;
   HCURSOR trackingCursor;
   short cursor;
   WinData* win;
   CText mode = "move"; // Move or drag cursor
   extern bool gScrollWheelEvent;
   
// Get user arguments
   if((r = ArgScan(itfc,args,3,"1D/2D, cursor, procedure, [mode]","eeee","tttt",&which,&type,&proc,&mode)) < 0)
      return(r); 

// Check on window type
   if(which == "1D" || which == "1d")
   {
		plot = Plot1D::curPlot();
      win = GetWinDataClass(GetParent(plot->plotParent->hWnd));

		if(!(Plot1D::curPlot()))
	   {
	      ErrorMessage("no current 1D plot");
	      return(ERR);
	   }
	   outofboundsCursor = OneDCursor;	   
	}
   else if(which == "2D" || which == "2d")
   {
		plot = Plot2D::curPlot();
      win = GetWinDataClass(GetParent(plot->plotParent->hWnd));

		if(!Plot2D::curPlot() || (!(Plot2D::curPlot()->mat()) && !(Plot2D::curPlot()->cmat())))
	   {
	      ErrorMessage("no current 2D plot");
	      return(ERR);
	   }
	   outofboundsCursor = TwoDCursor;	   
	}
	else
	{
		ErrorMessage("Wrong window type requested for operation.");
		return(ERR);
	}

// Check on cursor type
	if(type == "cross")
	{
	   trackingCursor = LoadCursor(NULL,IDC_ARROW);
	   cursor = CROSS;
	}
	else if(type == "col")
	{
	   trackingCursor = LoadCursor(NULL,IDC_ARROW);
	   cursor = COL;
	}
	else if(type == "row")
	{
	   trackingCursor = LoadCursor(NULL,IDC_ARROW);
	   cursor = ROW;
	}
	else if(type == "arrow")
	{
	   trackingCursor = LoadCursor(NULL,IDC_ARROW);
	   cursor = ARROW;
	}
	else
	{
	   ErrorMessage("invalid cursor type");
	   return(ERR);
	}		

   SetCursor(outofboundsCursor);
   
// Record wait cursor blocking state
   bool blockState = gBlockWaitCursor;
// Prevent wait cursor from appearing
   gBlockWaitCursor = true;		

// Drag option - wait until user presses left key to start tracking. Releasing key exits
   if(mode == "drag")
   {
      if(win->statusbox)
      {
         status = win->statusbox->hWnd;
         UpdateStatusWindow(status,1,"Drag mouse to action, release left button to exit");
      }
      else
      {
         status = NULL;
      }
      r = ProcessDragEvents(itfc,plot,trackingCursor,outofboundsCursor,cursor,status,proc.Str());

   }
   //else if (mode == "dragandscroll")
   //{
   //   if (win->statusbox)
   //   {
   //      status = win->statusbox->hWnd;
   //      UpdateStatusWindow(status, 1, "Drag or shift-scroll mouse to move data set, escape to exit");
   //   }
   //   else
   //   {
   //      status = NULL;
   //   }
   //   r = ProcessDragAndScrollEvents(itfc, plot, trackingCursor, outofboundsCursor, cursor, status, proc.Str());

   //}
   //else if (mode == "scroll")
   //{
   //   if (win->statusbox)
   //   {
   //      status = win->statusbox->hWnd;
   //      UpdateStatusWindow(status, 1, "Scroll mouse to scale data set, escape to exit");
   //   }
   //   else
   //   {
   //      status = NULL;
   //   }
   //   r = ProcessScrollEvents(itfc, plot, trackingCursor, outofboundsCursor, cursor, status, proc.Str());

   //}
// Move option - track while moving mouse, wait until user presses left key to exit
   else if(mode == "move")
   {
      if(win->statusbox)
      {
         status = win->statusbox->hWnd;
         UpdateStatusWindow(status,1,"Move mouse to action, press left button to exit");
      }
      else
      {
         status = NULL;
      }

      r = ProcessMoveEvents(itfc,plot,trackingCursor,outofboundsCursor,cursor,status,proc.Str());
   }
   else
   {
      ErrorMessage("invalid mode option");
      return(ERR);
   }

   gBlockWaitCursor = blockState;


   return(0);
}

/*********************************************************************************************
  User will click with left mouse button and then drag the cursor, releasing when finished.
  During this operation the callback procedure proc will be called each time the mouse it
  moved.
*********************************************************************************************/
bool gBlockScroll = false;

short ProcessDragAndScrollEvents(Interface* itfc, Plot* plot, HCURSOR trackingCursor, HCURSOR outofboundsCursor,
   short cursor, HWND status, char* proc)
{
   const short ARROW = 0;
   const short CROSS = 1;
   const short ROW = 2;
   const short COL = 3;

   short ret;

   MSG msg;
   long x1, y1, x0 = -1, y0 = -1;
   bool drawing = false;
   bool finished = false;

   // Note screen plot-limits	
   long pLeft = plot->GetLeft();
   long pRight = plot->GetRight();
   long pTop = plot->GetTop();
   long pBase = plot->GetTop() + plot->GetHeight();
   long pWidth = plot->GetWidth();
   long pHeight = plot->GetHeight();

   long xReset = 0;

   extern bool gScrollWheelEvent;

   char temp[MAX_STR];

   HWND hWnd = plot->win;
   HDC hdc = GetDC(hWnd);
   HPEN rubberPen = CreatePen(PS_DOT, 0, RGB(0, 0, 0));
   SelectObject(hdc, rubberPen);
   SetROP2(hdc, R2_XORPEN);
   gBlockWaitCursor = true;
   // Force window to capture mouse events
   SetCapture(hWnd);

   // Wait until user presses down key
   while (1)
   {
      // Check for an ecsape press - this will abort tracking macro
      if ((GetAsyncKeyState(VK_ESCAPE) & 0x08000))
      {
         ret = OK;
         goto ex;
      }

      if (PeekMessage(&msg, hWnd, NULL, NULL, PM_REMOVE))
      {
         if (msg.message == WM_LBUTTONDOWN) // Exit loop if left button pressed
         {
            x1 = LOWORD(msg.lParam);
            y1 = HIWORD(msg.lParam);
            break;
         }
      }
   }

   // Draw cursor until user presses(releases) left mouse button
   while (1)
   {
      // Check if escape is pressed
      if ((GetAsyncKeyState(VK_ESCAPE) & 0x08000))
      {
         if (drawing) // If already drawn remove old cursor
         {
            RemoveOldCursor(hdc, pLeft, pWidth, pTop, pHeight, x0, y0, cursor);
            break;
         }
      }

      if (PeekMessage(&msg, hWnd, NULL, NULL, PM_REMOVE))
      {

         if (!finished && msg.message == WM_LBUTTONUP)
         {
            xReset = 1;
         }


         else if (!finished && msg.message == WM_MOUSEWHEEL)
         {
            if ((GetAsyncKeyState(VK_SHIFT) & 0x08000))
            {
               short zDel = HIWORD(msg.wParam);    // wheel rotation

               if (zDel > 0)
                  x1++;
               else
                  x1--;

               x0 = x1; y0 = y1;
               gBlockScroll = true;

               // Run the callback procedure
               sprintf(temp, "(x,y) = (%g,%g)", plot->curXAxis()->scrnToData(x1), plot->curYAxis()->scrnToData(y1));
               SendMessage(status, SB_SETTEXT, (WPARAM)0, (LPARAM)temp);
               sprintf(temp, "%s(%g,%d)", proc, plot->curXAxis()->scrnToData(x1), xReset);

               
               if (ProcessMacroStr(itfc, false, temp) == ERR)
               {
                  RemoveOldCursor(hdc, pLeft, pWidth, pTop, pHeight, x0, y0, cursor);
                  ret = ERR;
                  goto ex;
               }
               gBlockScroll = false;

               if (xReset == 1)
               {
                  xReset = 0;
               }
            }
         }

         else if (!finished && msg.message == WM_MOUSEMOVE)
         {
            if ((GetAsyncKeyState(VK_SHIFT) == 0))
            {
               x1 = LOWORD(msg.lParam);
               y1 = HIWORD(msg.lParam);

               // Set cursor if out of bounds  	      
               if (x1 <= pLeft || x1 >= pRight || y1 <= pTop || y1 >= pBase)
               {
                  SetCursor(outofboundsCursor);
                  continue;
               }
               else
               {
                  SetCursor(trackingCursor);
               }

               if (x1 <= pLeft) x1 = pLeft + 1; // Check for out of bounds mouse movement
               if (x1 >= pRight) x1 = pRight - 1;
               if (y1 <= pTop) y1 = pTop + 1;
               if (y1 >= pBase) y1 = pBase - 1;

               // If already drawn remove old cursor
               if (drawing && !gScrollWheelEvent)
               {
                  RemoveOldCursor(hdc, pLeft, pWidth, pTop, pHeight, x0, y0, cursor);
               }
               gScrollWheelEvent = false;


               // Draw the new cursor
               drawing = true;
               if (cursor == CROSS || cursor == ROW)
               {
                  MoveToEx(hdc, pLeft, y1, 0);
                  LineTo(hdc, pLeft + pWidth, y1);
               }
               if (cursor == CROSS || cursor == COL)
               {
                  MoveToEx(hdc, x1, pTop, 0);
                  LineTo(hdc, x1, pTop + pHeight);
               }

               if ((msg.wParam & MK_LBUTTON) > 0)
               {
                  // Run the callback procedure
                  sprintf(temp, "(x,y) = (%g,%g)", plot->curXAxis()->scrnToData(x1), plot->curYAxis()->scrnToData(y1));
                  SendMessage(status, SB_SETTEXT, (WPARAM)0, (LPARAM)temp);
                  sprintf(temp, "%s(%g,%d)", proc, plot->curXAxis()->scrnToData(x1), xReset);

                  if (ProcessMacroStr(itfc, false, temp) == ERR)
                  {
                     RemoveOldCursor(hdc, pLeft, pWidth, pTop, pHeight, x0, y0, cursor);
                     ret = ERR;
                     goto ex;
                  }

                  if (xReset == 1)
                  {
                     xReset = 0;
                  }
               }
            }
         }
      }
   }
   ret = OK;

ex:

   ReleaseDC(hWnd, hdc);
   DeleteObject(rubberPen);
   gBlockWaitCursor = false;
   SetCursor(LoadCursor(NULL, IDC_WAIT));
   SendMessage(status, SB_SETTEXT, (WPARAM)1, (LPARAM)"");
   ReleaseCapture();
   return(ret);
}


short ProcessScrollEvents(Interface* itfc, Plot* plot, HCURSOR trackingCursor, HCURSOR outofboundsCursor,
                                 short cursor, HWND status, char* proc)
{
   const short ARROW = 0;
   const short CROSS = 1;
   const short ROW = 2;
   const short COL = 3;

   short ret;

   MSG msg;
   long x1, y1, x0 = -1, y0 = -1;
   bool drawing = false;
   bool finished = false;

   // Note screen plot-limits	
   long pLeft = plot->GetLeft();
   long pRight = plot->GetRight();
   long pTop = plot->GetTop();
   long pBase = plot->GetTop() + plot->GetHeight();
   long pWidth = plot->GetWidth();
   long pHeight = plot->GetHeight();

   long xReset = 0;

   extern bool gScrollWheelEvent;

   char temp[MAX_STR];

   HWND hWnd = plot->win;
   HDC hdc = GetDC(hWnd);
   HPEN rubberPen = CreatePen(PS_DOT, 0, RGB(0, 0, 0));
   SelectObject(hdc, rubberPen);
   SetROP2(hdc, R2_XORPEN);
   gBlockWaitCursor = true;
   // Force window to capture mouse events
   SetCapture(hWnd);

   int cnt = 0;

   // Wait until user presses down key
   while (1)
   {
      // Check for an ecsape press - this will abort tracking macro
      if ((GetAsyncKeyState(VK_ESCAPE) & 0x08000))
      {
         ret = OK;
         goto ex;
      }

      if (PeekMessage(&msg, hWnd, NULL, NULL, PM_REMOVE))
      {
         if (msg.message == WM_LBUTTONDOWN) // Exit loop if left button pressed
         {
            x1 = LOWORD(msg.lParam);
            y1 = HIWORD(msg.lParam);
            break;
         }
      }
   }

   // Draw cursor until user presses(releases) left mouse button
   while (1)
   {
   //   TextMessage("In main loop: %ld\n", cnt++);

      // Check if escape is pressed
      if ((GetAsyncKeyState(VK_ESCAPE) & 0x08000))
      {
         RemoveOldCursor(hdc, pLeft, pWidth, pTop, pHeight, x0, y0, cursor);
         break;
      }

      if (PeekMessage(&msg, hWnd, NULL, NULL, PM_REMOVE))
      {
       //  TextMessage("In peak message: %ld\n", cnt++);


         if (!finished && msg.message == WM_LBUTTONUP)
         {
            xReset = 1;
         }


         else if (!finished && msg.message == WM_MOUSEWHEEL)
         {
          //  if ((GetAsyncKeyState(VK_SHIFT) & 0x08000))
            {
               short zDel = HIWORD(msg.wParam);    // wheel rotation
               short  dir = (zDel < 0) ? 1 : -1;

               gBlockScroll = true;

               // Run the callback procedure
               sprintf(temp, "%s(%d)", proc, dir);
               if (ProcessMacroStr(itfc, false, temp) == ERR)
               {
                  ret = ERR;
                  goto ex;
               }
               gBlockScroll = false;

               if (xReset == 1)
               {
                  xReset = 0;
               }
            }
         }

         else if (!finished && msg.message == WM_MOUSEMOVE)
         {
        //    TextMessage("In move: %ld\n",cnt++);
       //     if ((GetAsyncKeyState(VK_SHIFT) == 0))
            {
               x1 = LOWORD(msg.lParam);
               y1 = HIWORD(msg.lParam);

               // Set cursor if out of bounds  	      
               if (x1 <= pLeft || x1 >= pRight || y1 <= pTop || y1 >= pBase)
               {
                  SetCursor(outofboundsCursor);
                  continue;
               }
               else
               {
                  SetCursor(trackingCursor);
               }

               if (x1 <= pLeft) x1 = pLeft + 1; // Check for out of bounds mouse movement
               if (x1 >= pRight) x1 = pRight - 1;
               if (y1 <= pTop) y1 = pTop + 1;
               if (y1 >= pBase) y1 = pBase - 1;

               // If already drawn remove old cursor
               if (drawing && !gScrollWheelEvent)
               {
                  RemoveOldCursor(hdc, pLeft, pWidth, pTop, pHeight, x0, y0, cursor);
               }
               gScrollWheelEvent = false;

               // Draw the new cursor
               drawing = true;
               if (cursor == CROSS || cursor == ROW)
               {
                  MoveToEx(hdc, pLeft, y1, 0);
                  LineTo(hdc, pLeft + pWidth, y1);
               }
               if (cursor == CROSS || cursor == COL)
               {
                  MoveToEx(hdc, x1, pTop, 0);
                  LineTo(hdc, x1, pTop + pHeight);
               }
            }
         }

      }
   }
   ret = OK;

ex:

   ReleaseDC(hWnd, hdc);
   DeleteObject(rubberPen);
   gBlockWaitCursor = false;
   SetCursor(LoadCursor(NULL, IDC_WAIT));
   SendMessage(status, SB_SETTEXT, (WPARAM)1, (LPARAM)"");
   ReleaseCapture();
   return(ret);
}

/*********************************************************************************************
  User will click with left mouse button and then drag the cursor, releasing when finished.
  During this operation the callback procedure proc will be called each time the mouse it
  moved.
*********************************************************************************************/

short ProcessDragEvents(Interface *itfc,Plot *plot, HCURSOR trackingCursor, HCURSOR outofboundsCursor,
                        short cursor, HWND status, char *proc)
{
   const short ARROW = 0;
   const short CROSS = 1;
   const short ROW   = 2;
   const short COL   = 3;

   short ret;

   MSG msg;
   long x1,y1,x0 = -1,y0=-1;
   bool drawing = false;
   bool finished = false;

// Note screen plot-limits	
	long pLeft = plot->GetLeft();
	long pRight = plot->GetRight();
	long pTop = plot->GetTop();
	long pBase = plot->GetTop() + plot->GetHeight();
   long pWidth = plot->GetWidth();
   long pHeight = plot->GetHeight();

   extern bool gScrollWheelEvent;

   char temp[MAX_STR];

   HWND hWnd = plot->win;
   HDC hdc = GetDC(hWnd);
   HPEN rubberPen = CreatePen(PS_DOT,0,RGB(0,0,0));
   SelectObject(hdc,rubberPen);
   SetROP2(hdc,R2_XORPEN);
   gBlockWaitCursor = true;
// Force window to capture mouse events
   SetCapture(hWnd);

// Wait until user presses down key
   while(1)
   {
   // Check for an ecsape press - this will abort tracking macro
      if(ProcessBackgroundEvents() != OK)
      {
         ret = OK;
         goto ex;
      }

	   if(PeekMessage(&msg, hWnd,NULL,NULL,PM_REMOVE))
	   {
	      if(msg.message == WM_LBUTTONDOWN) // Abort if escape pressed
	         break;
      }
   }

// Draw cursor until user presses(releases) left mouse button
   while(1)
   {
   // Check for an ecsape press - this will abort tracking macro
      if(ProcessBackgroundEvents() != OK)
      {
		   if(drawing) // If already drawn remove old cursor
		   {
            RemoveOldCursor(hdc, pLeft, pWidth, pTop, pHeight, x0, y0, cursor);   
		   }
         break;
      }

	   if(PeekMessage(&msg, hWnd,NULL,NULL,PM_REMOVE))
	   {
	      if(!finished && msg.message == WM_MOUSEMOVE && msg.wParam == MK_LBUTTON)
	      {
	      	x1 = LOWORD(msg.lParam);
			   y1 = HIWORD(msg.lParam);
			   
			// Set cursor if out of bounds  	      
			   if(x1 <= pLeft || x1 >= pRight || y1 <= pTop || y1 >= pBase)
			   {
	            SetCursor(outofboundsCursor);
	            continue;
	         }
	         else
	         {
	             SetCursor(trackingCursor);
	         }

			   if(x1 <= pLeft) x1 = pLeft+1; // Check for out of bounds mouse movement
			   if(x1 >= pRight) x1 = pRight-1;
			   if(y1 <= pTop) y1 = pTop+1;
			   if(y1 >= pBase) y1 = pBase-1;
			   
         // If already drawn remove old cursor
		      if(drawing && !gScrollWheelEvent) 
		      {
               RemoveOldCursor(hdc, pLeft, pWidth, pTop, pHeight, x0, y0, cursor);   
		      }
            gScrollWheelEvent = false;


         // Draw the new cursor
            drawing = true;
		      if(cursor == CROSS || cursor == ROW)
		      {            
		         MoveToEx(hdc,pLeft,y1,0);
		         LineTo(hdc,pLeft+pWidth,y1);
		      }
		      if(cursor == CROSS || cursor == COL)
		      {			      
		         MoveToEx(hdc,x1,pTop,0);
		         LineTo(hdc,x1,pTop+pHeight);
		      }
	         x0 = x1; y0 = y1;
	         
	       // Run the callback procedure
				sprintf(temp,"(x,y) = (%g,%g)",plot->curXAxis()->scrnToData(x1),plot->curYAxis()->scrnToData(y1));
				SendMessage(status,SB_SETTEXT, (WPARAM)0, (LPARAM) temp);
	         sprintf(temp,"%s(%g,%g)",proc,plot->curXAxis()->scrnToData(x1),plot->curYAxis()->scrnToData(y1));
	         
	         if(ProcessMacroStr(itfc,false,temp) == ERR)
	         {
	            RemoveOldCursor(hdc, pLeft, pWidth, pTop, pHeight, x0, y0, cursor); 
               ret = ERR;
               goto ex;
	         }	            	 
	      }
	
	      if(!finished && msg.message == WM_LBUTTONUP) // Record final coordinates
	      {
	      	x1 = LOWORD(msg.lParam);
			   y1 = HIWORD(msg.lParam);
			  
			 // Check for out of bounds mouse movement			   
			   if(x1 <= pLeft || x1 >= pRight || y1 <= pTop || y1 >= pBase)
			   {
			       MessageBeep(MB_OK);
	             continue;
	         }
	         	
         // Remove the old cursor
	         RemoveOldCursor(hdc, pLeft, pWidth, pTop, pHeight, x0, y0, cursor); 
			   finished = true;      
	      }

	      if(!finished && msg.message == WM_MOUSEMOVE && msg.wParam != MK_LBUTTON) // Record final coordinates
	      {
	      	x1 = LOWORD(msg.lParam);
			   y1 = HIWORD(msg.lParam);
			  
			 // Check for out of bounds mouse movement			   
			   if(x1 <= pLeft || x1 >= pRight || y1 <= pTop || y1 >= pBase)
			   {
			       MessageBeep(MB_OK);
	             continue;
	         }
	         RemoveOldCursor(hdc, pLeft, pWidth, pTop, pHeight, x0, y0, cursor); 
			   break;      
	      }
	      
	      if(finished && msg.message == WM_LBUTTONUP) // Exit
	      {
            break;	      
	      }
	   }
   }
   ret = OK;

ex:

   ReleaseDC(hWnd,hdc);
   DeleteObject(rubberPen);
   gBlockWaitCursor = false;
   SetCursor(LoadCursor(NULL,IDC_WAIT));   
	SendMessage(status,SB_SETTEXT, (WPARAM)1, (LPARAM) "");
   ReleaseCapture(); 
   return(ret);
}


short ProcessMoveEvents(Interface *itfc, Plot *plot, HCURSOR trackingCursor, HCURSOR outofboundsCursor,
                        short cursor, HWND status, char *proc)
{
   const short ARROW = 0;
   const short CROSS = 1;
   const short ROW   = 2;
   const short COL   = 3;

   short ret;

   MSG msg;
   long x1,y1,x0 = -1,y0=-1;
   bool drawing = false;
   bool finished = false;

// Note screen plot-limits	
   long pLeft = plot->GetLeft();
   long pRight = plot->GetRight();
   long pTop = plot->GetTop();
   long pBase = plot->GetTop() + plot->GetHeight();
   long pWidth = plot->GetWidth();
   long pHeight = plot->GetHeight();
   extern bool gScrollWheelEvent;

   CText temp;

   HWND hWnd = plot->win;
   HDC hdc = GetDC(hWnd);
   HPEN rubberPen = CreatePen(PS_DOT,0,RGB(0,0,0));
   SelectObject(hdc,rubberPen);
   SetROP2(hdc,R2_XORPEN);
   gBlockWaitCursor = true;

// Force window to capture mouse events
   SetCapture(hWnd);

// Draw cursor until user presses left mouse button
   while(1)
   {
      if(ProcessBackgroundEvents() != OK)
      {
		   if(drawing) // If already drawn remove old cursor
		   {
            RemoveOldCursor(hdc, pLeft, pWidth, pTop, pHeight, x0, y0, cursor);   
		   }
         break;
      }

	   if(PeekMessage(&msg, hWnd,NULL,NULL,PM_REMOVE))
	   {
	      if(!finished && msg.message == WM_MOUSEMOVE && msg.wParam != MK_LBUTTON)   // In move mode button must be up
	      {
	      	x1 = LOWORD(msg.lParam);
			   y1 = HIWORD(msg.lParam);
			   
			// Set cursor if out of bounds  	      
			   if(x1 <= pLeft || x1 >= pRight || y1 <= pTop || y1 >= pBase)
			   {
	            SetCursor(outofboundsCursor);
	            continue;
	         }
	         else
	         {
	             SetCursor(trackingCursor);
	         }

			   if(x1 <= pLeft) x1 = pLeft+1; // Check for out of bounds mouse movement
			   if(x1 >= pRight) x1 = pRight-1;
			   if(y1 <= pTop) y1 = pTop+1;
			   if(y1 >= pBase) y1 = pBase-1;
			   
         // If already drawn remove old cursor
		      if(drawing && !gScrollWheelEvent) 
		      {
               RemoveOldCursor(hdc, pLeft, pWidth, pTop, pHeight, x0, y0, cursor);   
		      }
            gScrollWheelEvent = false;

         // Draw the new cursor
            drawing = true;
		      if(cursor == CROSS || cursor == ROW)
		      {            
		         MoveToEx(hdc,pLeft,y1,0);
		         LineTo(hdc,pLeft+pWidth,y1);
		      }
		      if(cursor == CROSS || cursor == COL)
		      {			      
		         MoveToEx(hdc,x1,pTop,0);
		         LineTo(hdc,x1,pTop+pHeight);
		      }
	         x0 = x1; y0 = y1;
	         
	       // Run the callback procedure
	         temp.Format("(x,y) = (%g,%g)",plot->curXAxis()->scrnToData(x1),plot->curYAxis()->scrnToData(y1));
				SendMessage(status,SB_SETTEXT, (WPARAM)0, (LPARAM) temp.Str());
	         temp.Format("%s(%g,%g)",proc,plot->curXAxis()->scrnToData(x1),plot->curYAxis()->scrnToData(y1));
	         
	         if(ProcessMacroStr(itfc,temp.Str(),itfc->macroName.Str(),itfc->macroPath.Str()) == ERR)
	         if(ProcessMacroStr(LOCAL,temp.Str()) == ERR)
	         {
	            RemoveOldCursor(hdc, pLeft, pWidth, pTop, pHeight, x0, y0, cursor);  
               ret = ERR;
				   goto ex;
	         }	
            gBlockWaitCursor = true;
	      }
	      	         
      // User is finished so remove cursors and record final coordinates
	      if(!finished && msg.message == WM_LBUTTONDOWN) 
	      {
	      	x1 = LOWORD(msg.lParam);
			   y1 = HIWORD(msg.lParam);
			  
			 // Check for out of bounds mouse movement			   
			   if(x1 <= pLeft || x1 >= pRight || y1 <= pTop || y1 >= pBase)
			   {
			       MessageBeep(MB_OK);
	             continue;
	         }    
         // Remove old cursor
            if(!gScrollWheelEvent)
               RemoveOldCursor(hdc, pLeft, pWidth, pTop, pHeight, x0, y0, cursor);   
            gScrollWheelEvent = false;
			   finished = true;  
            continue; // Wait for mouse up
	      }

	      if(!finished && msg.message == WM_MOUSEMOVE && msg.wParam == MK_LBUTTON) // Record final coordinates
	      {
	      	x1 = LOWORD(msg.lParam);
			   y1 = HIWORD(msg.lParam);
			  
			 // Check for out of bounds mouse movement			   
			   if(x1 <= pLeft || x1 >= pRight || y1 <= pTop || y1 >= pBase)
			   {
			       MessageBeep(MB_OK);
	             continue;
	         }
            RemoveOldCursor(hdc, pLeft, pWidth, pTop, pHeight, x0, y0, cursor);   
			   break;      
	      }
	      
	      if(finished && msg.message == WM_LBUTTONUP) // && msg.hwnd == hWnd) // Exit
	      {
            break;	      
	      }
	   }
   } 
   ret = OK;

ex:

   ReleaseDC(hWnd,hdc);
   DeleteObject(rubberPen);
   gBlockWaitCursor = false;
   SetCursor(LoadCursor(NULL,IDC_WAIT));   
	SendMessage(status,SB_SETTEXT, (WPARAM)1, (LPARAM) "");
   ReleaseCapture(); 
   return(ret);
}

void RemoveOldCursor(HDC hdc, long plotLeft, long plotWidth, long plotTop, long plotHeight, long x0, long y0, short cursor)
{
   const short CROSS = 1;
   const short ROW   = 2;
   const short COL   = 3;

   if(y0 != -1)
   {
      if(cursor == CROSS || cursor == ROW) // Remove old cursor
	   {            
		   MoveToEx(hdc,plotLeft,y0,0);
		   LineTo(hdc,plotLeft+plotWidth,y0);
	   }
	   if(cursor == CROSS || cursor == COL)
	   {			      
		   MoveToEx(hdc,x0,plotTop,0);
		   LineTo(hdc,x0,plotTop+plotHeight);
	   }
   }
}
	    
/**********************************************************************************************************
* Allow the user to select a 2D coordinate. Return these coordinates.
*
* Syntax  (x,y) = getxy(["cross"],[mode])  // Default mode x,y cursor
* Syntax  x = getxy("vert[ical],[mode]")   // Vertical cursor
* Syntax  y = getxy("horiz[ontal],[mode]") // Horizontal cursor
*
***********************************************************************************************************/

int Get2DCoordinate(Interface *itfc, char args[])
{
	MSG msg ;
   short x1 = -1,y1 = -1,x0 = -1, y0 = -1;
   float x,y;
	long xmin,xmax,ymin,ymax;
   bool drawing = false;
   long plotLeft,plotTop,plotWidth,plotHeight;
   CText temp;
   static CText cursor = "cross";
   short r;
   enum {CROSS,HORIZ,VERT} cursor_mode = CROSS;
   HCURSOR outofboundsCursor;
   HCURSOR trackingCursor;
   bool button_pressed = false;
   CText mode = "index";
   
	Plot2D* cur2DPlot = Plot2D::curPlot();
// Don't bother if there is no 2D plot   
	if(!cur2DPlot)
     return(0);

// Get type of cursor
   if((r = ArgScan(itfc,args,1,"cursor, [mode]","ee","tt",&cursor,&mode)) < 0)
      return(r);

// Make sure we don't need to repaint a window first
	if(PeekMessage(&msg, NULL,NULL,NULL,PM_NOREMOVE)) 
	{
	   if(msg.message == WM_PAINT) 
	   {
	      MyUpdateWindow(msg.hwnd);
	   }
	}

// Set cursor to steer user toward 2D plot
   outofboundsCursor = TwoDCursor;
	
   if(cursor == "cross")
   {
      cursor_mode = CROSS;
      trackingCursor = LoadCursor(NULL,IDC_ARROW);      
   }
   else if(cursor == "vert")
   {
      cursor_mode = VERT;
      trackingCursor = LoadCursor(NULL,IDC_SIZENS);             
   }
   else if(cursor == "horiz")
   {
      cursor_mode = HORIZ;
      trackingCursor = LoadCursor(NULL,IDC_SIZEWE);         
   }
   else
   {
      ErrorMessage("invalid cursor type"); 
      return(ERR);
   }
   SetCursor(outofboundsCursor);
   
// Get some variables defined	
   plotLeft   = cur2DPlot->GetLeft();
   plotTop    = cur2DPlot->GetTop();
   plotWidth  = cur2DPlot->GetWidth();
   plotHeight = cur2DPlot->GetHeight();
   
	xmin = plotLeft;
	ymin = plotTop;
	xmax = xmin+plotWidth;
	ymax = ymin+plotHeight;
	   
   HWND hWnd = cur2DPlot->win;
   HDC hdc = GetDC(hWnd);
   HPEN rubberPen = CreatePen(PS_DOT,0,RGB(0,0,0));
   SelectObject(hdc,rubberPen);
   SetROP2(hdc,R2_XORPEN);

   UpdateStatusWindow(hWnd,1,"Move mouse to select point, press left button to exit");

// Force window to capture mouse events
   SetCapture(hWnd);
   gBlockWaitCursor = true;

// Draw cursor until user presses left mouse button  
   while(true)
   {
      Sleep(0); // Give time back to the system if necessary

      if(ProcessBackgroundEvents() != OK)
      {
		   if(drawing) // If already drawn remove old cursor
		   {
		      if(cursor_mode == CROSS || cursor_mode == HORIZ)
		      {
		         MoveToEx(hdc,plotLeft,y0,0);
		         LineTo(hdc,plotLeft+plotWidth,y0);
		      }
		      if(cursor_mode == CROSS || cursor_mode == VERT)
		      {		         
		         MoveToEx(hdc,x0,plotTop,0);
		         LineTo(hdc,x0,plotTop+plotHeight);
		      }
		   }
         break;
      }

	   if(PeekMessage(&msg, hWnd,0,0,PM_REMOVE))
	   {   
	      if(msg.message == WM_MOUSEMOVE && msg.wParam == 0) // Track movement
	      {
	      	x1 = LOWORD(msg.lParam);
			   y1 = HIWORD(msg.lParam);
			   
			 // Check for out of bounds mouse movement			   
			   if(x1 <= xmin || x1 >= xmax || y1 <= ymin || y1 >= ymax)
	             SetCursor(outofboundsCursor);
	         else
	             SetCursor(trackingCursor);
	             			   
			   if(x1 <= xmin) x1 = xmin+1;
			   if(x1 >= xmax) x1 = xmax-1;
			   if(y1 <= ymin) y1 = ymin+1;
			   if(y1 >= ymax) y1 = ymax-1;
			   
		      if(drawing) // If already drawn remove old cursor
		      {
		         if(cursor_mode == CROSS || cursor_mode == HORIZ)
		         {
		            MoveToEx(hdc,plotLeft,y0,0);
		            LineTo(hdc,plotLeft+plotWidth,y0);
		         }
		         if(cursor_mode == CROSS || cursor_mode == VERT)
		         {		         
		            MoveToEx(hdc,x0,plotTop,0);
		            LineTo(hdc,x0,plotTop+plotHeight);
		         }
		      }
            drawing = true;

		      if(cursor_mode == CROSS || cursor_mode == HORIZ)
		      {            
		         MoveToEx(hdc,plotLeft,y1,0);
		         LineTo(hdc,plotLeft+plotWidth,y1);
		      }
		      if(cursor_mode == CROSS || cursor_mode == VERT)
		      {			      
		         MoveToEx(hdc,x1,plotTop,0);
		         LineTo(hdc,x1,plotTop+plotHeight);
		      }
		      
	         x0 = x1; y0 = y1;
	         
	         // Run procedure
	         
	         temp.Format("(x,y) = (%g,%g)",cur2DPlot->curXAxis()->scrnToData(x1),cur2DPlot->curYAxis()->scrnToData(y1));
            UpdateStatusWindow(hWnd,0,temp.Str());
	      }
	      	         
	      if(msg.message == WM_LBUTTONDOWN) // Record final coordinates and amplitude
	      {
	      	x1 = LOWORD(msg.lParam);
			   y1 = HIWORD(msg.lParam);
			   	      
			   if(x1 <= xmin || x1 >= xmax || y1 <= ymin || y1 >= ymax) // Ignore if outofbounds
			   {
			       MessageBeep(MB_OK);
	             continue;
	         }
	      
		      if(cursor_mode == CROSS || cursor_mode == HORIZ)
		      {  	      
		         MoveToEx(hdc,plotLeft,y1,0);
		         LineTo(hdc,plotLeft+plotWidth,y1);
		      }
		      if(cursor_mode == CROSS || cursor_mode == VERT)
		      {		      
		         MoveToEx(hdc,x1,plotTop,0);
		         LineTo(hdc,x1,plotTop+plotHeight);
            }	
		      if(mode == "index")
            {
			      x = cur2DPlot->curXAxis()->scrnToData(x1);
			      y = cur2DPlot->curYAxis()->scrnToData(y1);
            }
            else
            {
			      x = cur2DPlot->curXAxis()->scrnToUser(x1);
			      y = cur2DPlot->curYAxis()->scrnToUser(y1);
            }
			   if(cursor_mode == CROSS)
			   {
               itfc->retVar[1].MakeAndSetFloat(x);   
	            itfc->retVar[2].MakeAndSetFloat(y);
               itfc->nrRetValues = 2;	            
	         }
	         else if(cursor_mode == HORIZ)
	         {
	            itfc->retVar[1].MakeAndSetFloat(y);
               itfc->nrRetValues = 1;	
	         }
	         else
	         {
	            itfc->retVar[1].MakeAndSetFloat(x);
               itfc->nrRetValues = 1;	
	         }         
			   button_pressed = true;      
	      }

         if(msg.message == WM_LBUTTONUP && button_pressed) // Record final coordinates and amplitude
           break;
	   }
   }
   gBlockWaitCursor = false;
   ReleaseDC(hWnd,hdc);
   DeleteObject(rubberPen);
   UpdateStatusWindow(hWnd,1,"");
   SetCursor(LoadCursor(NULL,IDC_WAIT));
   ReleaseCapture();            
  
   return(OK);
}

// Show or hide color scale

int ShowColorScale(Interface* itfc ,char args[])
{
   short nrArgs;
   CText show;
	HMENU hMenu;

// Check for current 2D plot
	Plot2D* cur2DPlot = Plot2D::curPlot();
   if(!cur2DPlot)
      return(OK);

// Get the current status
  if(cur2DPlot->displayColorScale())
     show = "true";
  else
     show = "false";
 
// Get the new status
   if((nrArgs = ArgScan(itfc,args,0,"true/false","e","t",&show)) < 0)
     return(nrArgs);

// Get the color-scale visibility status
   if(nrArgs == 0)
   {
      itfc->nrRetValues = 1;
      itfc->retVar[1].MakeAndSetString(show.Str());
      return(OK);
   }


// Set the color-scale visibility status
	hMenu = GetMenu(cur2DPlot->win);
	   
	if(show == "true" || show == "yes")
	{
		CheckMenuItem(hMenu,ID_COLOR_BAR,MF_CHECKED);
		cur2DPlot->setDisplayColorScale(true);
	}
	if(show == "false" || show == "no")
	{
		CheckMenuItem(hMenu,ID_COLOR_BAR,MF_UNCHECKED);
		cur2DPlot->setDisplayColorScale(false);
	}
   cur2DPlot->DisplayAll(false); 
  
	      
	itfc->nrRetValues = 0;
   return(OK);
   
}

// Callback procedure for the plot color dialog box

int CALLBACK Plot2DColorDlgProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   RECT r;
   LPDRAWITEMSTRUCT pdis;
	static bool modifyAll = false;
	
	Plot2D* cur2DPlot = Plot2D::curPlot();
	PlotWindow *pp1D = Plot1D::curPlot()->plotParent;
	PlotWindow *pp2D = cur2DPlot->plotParent;
	
	switch (message)
	{
		case(WM_INITDIALOG):
		{
			SendDlgItemMessage(hWnd,ID_ALL,BM_SETCHECK,modifyAll,0); 
         PlaceDialogOnTopInCentre(hWnd); 
         SaveDialogParent(hWnd);          			
	      return true;
		}

		case(WM_CLOSE):
		{
         MyEndDialog(hWnd,1);
      	GetClientRect(cur2DPlot->win,&r);
      	MyInvalidateRect(cur2DPlot->win,&r,false);
      	break;
		}
			    
		case(WM_COMMAND):
		{
		   switch(LOWORD (wParam))
		   {
		   	case(ID_ALL):
		   	   modifyAll = !modifyAll;
		   	   break;
		   	case(ID_DEFAULTCOLORS):
		   	{
					if(modifyAll)
					{
						for(Plot* p: pp2D->plotList())
						{
					         dynamic_cast<Plot2D*>(p)->ChooseDefaultColours();
						}  
					}
					else
               {
						if (cur2DPlot)
						{
							cur2DPlot->ChooseDefaultColours();
							//cur2DPlot->SetTraceColor();
						}
               }

					MyInvalidateRect(hWnd,NULL,false);
					MyInvalidateRect(cur2DPlot->win,NULL,false);
	            break;
	         }
	         case(ID_BLACK_N_WHITE):
	         {	         
					if(modifyAll)
					{
						for(Plot* p: pp2D->plotList())
						{
							dynamic_cast<Plot2D*>(p)->ChooseBlackNWhite();
						}
					}
					else
               {
						if (cur2DPlot)
						{
							cur2DPlot->ChooseBlackNWhite();
							//cur2DPlot->SetTraceColor(RGB(0,0,0),RGB(0,0,0),RGB(0,0,0));
						}
               }
					
					MyInvalidateRect(hWnd,NULL,false);
					MyInvalidateRect(cur2DPlot->win,NULL,false);
	            break;
	         }	             
				case(ID_AXES_COLOR):
				case(ID_BK_COLOR):
				case(ID_BORDER_COLOR):
				{
				   COLORREF col = cur2DPlot->ChoosePlotColor(hWnd,LOWORD (wParam),(LOWORD(wParam) == ID_BORDER_COLOR));
				   if(col == ABORT_COLOR_CHOICE)
                  break;
                   
					if(modifyAll)
					{
					   if(cur2DPlot->getDimension() == 1)
					   {
							for(Plot* p: pp1D->plotList())
							{
								p->SetPlotColor(col,LOWORD (wParam));
							}
					   }
					   else
					   {
							for(Plot* p: pp2D->plotList())
							{
					         p->SetPlotColor(col,LOWORD (wParam));
							}
					   }
					}
					else
					{
						if (cur2DPlot)
						{
							cur2DPlot->SetPlotColor(col,LOWORD (wParam));
						}
					}

					MyInvalidateRect(hWnd,NULL,false);
					MyInvalidateRect(cur2DPlot->win,NULL,false);
					break;
			   }
	       	case(ID_CANCEL):
               MyEndDialog(hWnd,1);
	         	MyInvalidateRect(cur2DPlot->win,NULL,false);
	         	return(true);
	    	}
	      break ;
		}
		
		case(WM_DRAWITEM):
		{
		   COLORREF color;
			HBRUSH hBrush; 
         pdis = (LPDRAWITEMSTRUCT) lParam;
			if (cur2DPlot)
			{
				cur2DPlot->GetPlotColor(pdis->CtlID,color);
				hBrush = (HBRUSH)CreateSolidBrush(color);
			}
			else
			{
				hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
			}

		   FillRect(pdis->hDC, &pdis->rcItem, hBrush);
         DeleteObject(hBrush);
         FrameRect(pdis->hDC, &pdis->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
         break;
		}
	}
	return(0);
}


short Plot2D::Draw2DImage(Interface* itfc ,char arg[])
{
   Variable colorMapVar;
   short nrArgs;
   long i,j;
   CArg carg;
   Variable vXAxis;
   Variable vYAxis;
   Variable vMat;
   char matName[MAX_STR];

// Process the passed arguments
   nrArgs = carg.Count(arg);

// Get matrix name
   if(nrArgs >= 1)
      strncpy_s(matName,MAX_STR,carg.Extract(1), _TRUNCATE);

   if(nrArgs == 1)
   {
      if((nrArgs = ArgScan(itfc,arg,1,"matrix","e","v",&vMat)) < 0)
		{
         return(nrArgs);  
		}
   }

   else if(nrArgs == 2)
   {
      if((nrArgs = ArgScan(itfc,arg,2,"matrix,colormap","ee","vv",&vMat,&colorMapVar)) < 0)
		{
         return(nrArgs);  
		}
   }

   else if(nrArgs == 3)
   {
      if((nrArgs = ArgScan(itfc,arg,3,"matrix,xrange,yrange","eee","vvv",&vMat,&vXAxis,&vYAxis)) < 0)
		{
         return(nrArgs);  
		} 
   }

   else
   {
      if((nrArgs = ArgScan(itfc,arg,4,"matrix,[[xrange,yrange],colormap]","eeee","vvvv",&vMat,&vXAxis,&vYAxis,&colorMapVar)) < 0)
		{
         return(nrArgs);  
		}
   }
	//plotParent->setBusyWithCriticalSection(true);

// Set display mode to image **********************
   setDrawMode(DISPLAY_IMAGE);
   if(vx_)   FreeMatrix2D(vx_);
   if(vy_)   FreeMatrix2D(vy_);
   vx_ = NULL;
   vy_ = NULL;

// Is a matrix is being imaged? ******************
   if(vMat.GetType() == MATRIX2D)
   {
      float **mat = VarRealMatrix(&vMat);

   // Copy matrix to the 2D plot matrix ********** 
      clearData();
		clearColorMap();
      setMatWidth(VarColSize(&vMat));
      setMatHeight(VarRowSize(&vMat));
      setMat(MakeMatrix2D(matWidth_,matHeight_));
      if(!mat_)
      {
         ErrorMessage("out of memory!");
	//		plotParent->setBusyWithCriticalSection(false);
         return(ERR);
      }
      
      for(i = 0; i < matHeight_; i++)
         for(j = 0; j < matWidth_; j++)
            this->mat_[i][j] = mat[i][j];
      
		resetDataView();

		curXAxis()->initialiseRange();
		curXAxis()->setMapping(PLOT_LINEAR_X);
		curYAxis()->initialiseRange();
		curYAxis()->setMapping(PLOT_LINEAR_Y);
      this->removeLines();
   }
   else if(vMat.GetType() == DMATRIX2D)
   {
      double **mat = vMat.GetDMatrix2D();

   // Copy matrix to the 2D plot matrix ********** 
      clearData();
		clearColorMap();
      setMatWidth(VarColSize(&vMat));
      setMatHeight(VarRowSize(&vMat));
      setMat(MakeMatrix2D(matWidth_,matHeight_));
      if(!mat_)
      {
         ErrorMessage("out of memory!");
	//		plotParent->setBusyWithCriticalSection(false);
         return(ERR);
      }
      
      for(i = 0; i < matHeight_; i++)
         for(j = 0; j < matWidth_; j++)
            mat_[i][j] = (float)mat[i][j];

		resetDataView();

		curXAxis()->initialiseRange();
		curXAxis()->setMapping(PLOT_LINEAR_X);
		curYAxis()->initialiseRange();
		curYAxis()->setMapping(PLOT_LINEAR_Y);
      this->removeLines(); 
	}
// Is a cmatrix is being imaged? ******************
   else if(VarType(&vMat) == CMATRIX2D)
   {
      complex **cmat = VarComplexMatrix(&vMat);

    // Copy matrix to the 2D display matrix ********  
      clearData();
		clearColorMap();
      setMatWidth(VarColSize(&vMat));
      setMatHeight(VarRowSize(&vMat));
      setCMat(MakeCMatrix2D(matWidth_,matHeight_));
      if(!cmat_)
      {
         ErrorMessage("out of memory!");
	//		plotParent->setBusyWithCriticalSection(false);
         return(ERR);
      }

      for(i = 0; i < matHeight_; i++)
         for(j = 0; j < matWidth_; j++)
            cmat_[i][j] = cmat[i][j];
     
		resetDataView();

		curXAxis()->initialiseRange();
		curXAxis()->setMapping(PLOT_LINEAR_X);
		curYAxis()->initialiseRange();
		curYAxis()->setMapping(PLOT_LINEAR_Y);
      this->removeLines();
   }
   else if(vMat.GetType() == MATRIX3D && vMat.GetDimZ() == 3)
   {
      float ***mat = vMat.GetMatrix3D();

   // Copy matrix to the 2D plot matrix ********** 
      clearData();
		clearColorMap();
      setMatWidth(VarColSize(&vMat));
      setMatHeight(VarRowSize(&vMat));
      setMatRGB(MakeMatrix3D(matWidth_,matHeight_,3));
      if(!matRGB_)
      {
         ErrorMessage("out of memory!");
		//	plotParent->setBusyWithCriticalSection(false);
         return(ERR);
      }
      
      for(i = 0; i < matHeight_; i++)
      {
         for(j = 0; j < matWidth_; j++)
         {
				matRGB_[0][i][j] = mat[0][i][j];
				matRGB_[1][i][j] = mat[1][i][j];
            matRGB_[2][i][j] = mat[2][i][j];
         }
      }

		resetDataView();

		curXAxis()->initialiseRange();
		curXAxis()->setMapping(PLOT_LINEAR_X);
		curYAxis()->initialiseRange();
		curYAxis()->setMapping(PLOT_LINEAR_Y);
      this->removeLines();
   }
   else
   {
      ErrorMessage("can only image 2D matrices");
	//	plotParent->setBusyWithCriticalSection(false);
      return(ERR);
   }

// Process colormap argument ***********************
   if(nrArgs == 2 || nrArgs == 4)
   {
      if(colorMapVar.GetType() != MATRIX2D || colorMapVar.GetDimX() != 3 || colorMapVar.GetDimY() <= 1)
      {
         ErrorMessage("invalid colour-scale");
	//		plotParent->setBusyWithCriticalSection(false);
         return(ERR);
      }
   // Load a new colormap
      this->clearColorMap();
      setColorMapLength(VarHeight(&colorMapVar)); 
      float **mIn = VarRealMatrix(&colorMapVar);
      setColorMap(CopyMatrix(mIn,3,colorMapLength_));
   }
   else
   {
      if(gDefaultColorMap != NULL && gDefaultColorMapLength > 0)
      {
         this->clearColorMap();
         setColorMap(CopyMatrix(gDefaultColorMap,3,gDefaultColorMapLength));
         setColorMapLength(gDefaultColorMapLength);
      }
   }


// Check for x and y axis data *******************
   if(nrArgs == 3 || nrArgs == 4)
   {
      if(VarType(&vXAxis) == MATRIX2D)
      {
         if(VarWidth(&vXAxis) == 2 && VarHeight(&vXAxis) == 1)
         {
            curXAxis()->setBase(VarRealMatrix(&vXAxis)[0][0]);
				curXAxis()->setLength(VarRealMatrix(&vXAxis)[0][1] - this->curXAxis()->base());
         }
         else
         {
            ErrorMessage("invalid x axis specifier should be [minx,max]");
		//		plotParent->setBusyWithCriticalSection(false);
            return(ERR);
         }
         this->xCalibrated_ = true;
      }

      if(VarType(&vYAxis) == MATRIX2D)
      {
         if(VarWidth(&vYAxis) == 2 && VarHeight(&vYAxis) == 1)
         {
            curYAxis()->setBase(VarRealMatrix(&vYAxis)[0][0]);
				curYAxis()->setLength(VarRealMatrix(&vYAxis)[0][1] - this->curYAxis()->base());
         }
         else
         {
            ErrorMessage("invalid y axis specifier should be [miny,may]");
		//		plotParent->setBusyWithCriticalSection(false);
            return(ERR);
         }
         this->yCalibrated_ = true;
      }
   }
   else
   {
      this->xCalibrated_ = false;
      this->yCalibrated_ = false;
   }

// Remove any offset
   RemoveCharFromString(this->statusText,'O');
   this->removeOffset();

// Remove the zoom flag
   this->resetZoomCount(); 
   RemoveCharFromString(this->statusText,'Z');

// Make sure the status flag is updated
	UpdateStatusWindow(this->win, 3, this->statusText);

// Copy the matrix name to the plot title *****
	title().setText(matName);

//	plotParent->setBusyWithCriticalSection(false);

// Display the image if desired ****************
   DisplayAll(true);

// Remove any selection rectangle **************  
   ResetSelectionRectangle();

   return(OK);
}


/************************************************************************
* Return the image matrix data
*************************************************************************/

short Plot2D::GetData(Interface *itfc, short mode)
{
   float xRange[2];
   float yRange[2];

   if(!mat_ && !cmat_ && !matRGB_)
   {
      ErrorMessage("no image data present");
      return(ERR);
   }

	if(mat_)
   {
      if(mode == 1)
      {
         xRange[0] = curXAxis()->base();
         xRange[1] = curXAxis()->base()+curXAxis()->length();
         yRange[0] = curYAxis()->base();
         yRange[1] = curYAxis()->base()+curYAxis()->length();

			itfc->retVar[1].MakeAndLoadMatrix2D(mat_, matWidth_, matHeight_);
			itfc->retVar[2].MakeMatrix2DFromVector(xRange,2,1);
			itfc->retVar[3].MakeMatrix2DFromVector(yRange,2,1);
      }
      else
      {
         long x0 = visibleLeft();
         long y0 = visibleTop();
         long width = visibleWidth();
         long height = visibleHeight();
         float totwidth = matWidth_;
         float totheight = matHeight_;
			itfc->retVar[1].MakeAndLoadMatrix2D(NULL, width, height);
         float** mIn = mat_;
         float** mOut = itfc->retVar[1].GetMatrix2D();
         for(int y = 0; y < height; y++)
            for(int x = 0; x < width; x++)
               mOut[y][x] = mIn[y+y0][x+x0];
			itfc->retVar[2].MakeMatrix2DFromVector(NULL,2,1);
			itfc->retVar[3].MakeMatrix2DFromVector(NULL,2,1);
         itfc->retVar[2].GetMatrix2D()[0][0] = x0/totwidth*(curXAxis()->length()) + curXAxis()->base();
         itfc->retVar[2].GetMatrix2D()[0][1] = (x0+width)/totwidth*(curXAxis()->length()) + curXAxis()->base();
         itfc->retVar[3].GetMatrix2D()[0][0] = y0/totheight*(curYAxis()->length()) + curYAxis()->base();
         itfc->retVar[3].GetMatrix2D()[0][1] = (y0+height)/totheight*(curYAxis()->length()) + curYAxis()->base();
      }
   }
	else if(cmat_)
   {
      if(mode == 1)
      {
         xRange[0] = curXAxis()->base();
         xRange[1] = curXAxis()->base()+curXAxis()->length();
         yRange[0] = curYAxis()->base();
         yRange[1] = curYAxis()->base()+curYAxis()->length();

			itfc->retVar[1].MakeAndLoadCMatrix2D(cmat_, matWidth_, matHeight_);
			itfc->retVar[2].MakeMatrix2DFromVector(xRange,2,1);
			itfc->retVar[3].MakeMatrix2DFromVector(yRange,2,1);
      }
      else
      {
         long x0 = visibleLeft();
         long y0 = visibleTop();
         long width = visibleWidth();
         long height = visibleHeight();
         float totwidth = matWidth_;
         float totheight = matHeight_;
			itfc->retVar[1].MakeAndLoadCMatrix2D(NULL, width, height);
         complex** mIn = cmat_;
         complex** mOut = itfc->retVar[1].GetCMatrix2D();
         for(int y = 0; y < height; y++)
            for(int x = 0; x < width; x++)
               mOut[y][x] = mIn[y+y0][x+x0];
			itfc->retVar[2].MakeMatrix2DFromVector(NULL,2,1);
			itfc->retVar[3].MakeMatrix2DFromVector(NULL,2,1);
         itfc->retVar[2].GetMatrix2D()[0][0] = x0/totwidth*(curXAxis()->length()) + curXAxis()->base();
         itfc->retVar[2].GetMatrix2D()[0][1] = (x0+width)/totwidth*(curXAxis()->length()) + curXAxis()->base();
         itfc->retVar[3].GetMatrix2D()[0][0] = y0/totheight*(curYAxis()->length()) + curYAxis()->base();
         itfc->retVar[3].GetMatrix2D()[0][1] = (y0+height)/totheight*(curYAxis()->length()) + curYAxis()->base();
      }
   }
   else if(matRGB_)
   {
      if(mode == 1)
      {
         xRange[0] = curXAxis()->base();
         xRange[1] = curXAxis()->base()+curXAxis()->length();
         yRange[0] = curYAxis()->base();
         yRange[1] = curYAxis()->base()+curYAxis()->length();

			itfc->retVar[1].MakeAndLoadMatrix3D(matRGB_, matWidth_, matHeight_,3);
			itfc->retVar[2].MakeMatrix2DFromVector(xRange,2,1);
			itfc->retVar[3].MakeMatrix2DFromVector(yRange,2,1);
      }
   }	

   itfc->nrRetValues = 3;
   itfc->defaultVar = 1;

   return(OK);

}
  
// Zoom into the 2D plot or return current zoom limits

short Plot2D::Zoom(Interface* itfc, char args[])
{
   short n;
 
   long left = visibleLeft();
   long right = left + visibleWidth() - 1;
   long bottom = visibleTop();
   long top = bottom + visibleHeight() - 1;
   
   if((n = ArgScan(itfc,args,4,"left, right, bottom, top","eeee","llll",&left,&right,&bottom,&top)) < 0)
      return(n);

// Check for errors
   if(left < 0 || bottom < 0 || right >= matWidth_ || top >= matHeight_)
   {
      ErrorMessage("zoom rectangle out of bounds");
      return(ERR);
   }
   
   if(left >= right || top <= bottom)
   {
      ErrorMessage("invalid zoom rectangle");
      return(ERR);
   }


// Update select rect parameters      
   selectRect.left   = left;
   selectRect.right  = right;
   selectRect.top    = bottom;
   selectRect.bottom = top;
   rectSelected = true;

// Zoom
	ZoomRegion();
	updateStatusWindowForZoom(win);

// Display
   this->DisplayAll(false); 

   return(OK);
}


/********************************************************
*    Functions specific to the 1D plot
********************************************************/

int Plot2DFunctions(Interface* itfc ,char args[])
{
   CText func;
   short r;

   if(!Plot2D::curPlot())
   {
      ErrorMessage("2D Plot window not defined\n");
      return(ERR);
   }

   if((r = ArgScan(itfc,args,0,"function","e","t",&func)) < 0)
      return(r); 


// Make the first plot in the current window the current plot
// if the current window doesn't contain the current plot
// This is because the following functions are usually called from  
// the gui and we expect them to apply to the window they were
// called from. (ANY EXCEPTIONS?)

   HWND hWnd = Plot2D::curPlot()->win;
   PlotWindow2D *pp = static_cast<PlotWindow2D*>(Plot2D::curPlot()->plotParent);

   if(itfc->win && (itfc->win->hWnd != pp->obj->hwndParent))
   {
      ObjectData *obj = itfc->win->widgets.findByType(IMAGEWINDOW);
      if(obj)
      {
         pp = (PlotWindow2D*)obj->data;
         if(pp)
         {
				Plot2D* curPlot2D = Plot2D::curPlot();
				PlotList& pl = pp->plotList();
				PlotListIterator plit = find(pl.begin(), pl.end(), curPlot2D);

            if(plit == pl.end()) // Current plot not found
            {
          // Set current plot to first region in plot parent
               MyInvalidateRect(Plot2D::curPlot()->win,NULL,false); // Remove old indent
					pp->makeCurrentPlot();
					pp->makeCurrentDimensionalPlot();
               MyInvalidateRect(Plot2D::curPlot()->win,NULL,false); // Draw new indent
            }
         }
      }
   }

   // Some routines below use curPlot not Plot2D::curPlot()
	Plot2D::curPlot()->makeCurrentPlot();
	HDC hdc = GetDC(hWnd);

   if(func == "axes font")
	{
		if (pp->StylePlotAxisTicksLabels())
		{
         pp->curPlot()->Invalidate();	   
      }
	}
   else if(func == "border axes")
   {
	   pp->curPlot()->axesMode = PLOT_AXES_CORNER+1;
		WinData* parent = pp->obj->winParent;
		const char* const toolbarName = pp->toolbarName();
		parent->setToolBarItemCheck(toolbarName,"corner_axes", false);
      parent->setToolBarItemCheck(toolbarName,"border_axes", true);
      parent->setToolBarItemCheck(toolbarName,"crossed_axes", false);
      pp->curPlot()->Invalidate();
   }
   else if(func == "copy all plots")
   {
      pp->CopyPlotsToClipboard(true);
      pp->CopyCurrentPlot();
   }
   else if(func == "copy plot")
   {
      pp->CopyPlotsToClipboard(false);
      pp->CopyCurrentPlot();
   }
   else if(func == "corner axes")
   {
	   pp->curPlot()->axesMode = PLOT_AXES_CORNER;
		WinData* parent = pp->obj->winParent;
		const char* const toolbarName = pp->toolbarName();
      parent->setToolBarItemCheck(toolbarName,"corner_axes", true);
      parent->setToolBarItemCheck(toolbarName,"border_axes", false);
      parent->setToolBarItemCheck(toolbarName,"crossed_axes", false);
      pp->curPlot()->Invalidate();
   }
   else if(func == "cut plot")
   {
      pp->CopyCurrentPlot();
      pp->CloseCurrentPlot(); 
      RECT r;
      GetClientRect(Plot2D::curPlot()->win,&r);
      long width = r.right-r.left+1;
      long height = r.bottom-r.top+1;
      SendMessage(Plot2D::curPlot()->win,WM_SIZE,(WPARAM)SIZE_RESTORED,(LPARAM)(width+(height<<16)));  
   }
   else if(func == "delete all plots")
   {
      CText name = "untitled";
      pp->fileNumber = rootWin->GetNextFileNumber(pp->obj,name);
      pp->fileName(name);
      pp->modified(false);
	   pp->MakeMultiPlot(1,1); 
      pp->curPlot()->initialiseMenuChecks("clear");
	   pp->curPlot()->Invalidate();  
   }
   else if(func == "display data")
   {
      pp->ClearCursors(hdc);
      gResetCursor = CrossCursor;
      SetCursor(gResetCursor);
      pp->mouseMode = SHOW_DATA;
		UpdateStatusWindow(hWnd,1,"Left button : Hold for data display");
		WinData* parent = pp->obj->winParent;
		const char* const menuName = pp->menuName();
      parent->setMenuItemCheck(menuName,"drag_plot",false);
      parent->setMenuItemCheck(menuName,"display_data",true);
      parent->setMenuItemCheck(menuName,"select_region",false);
      parent->setMenuItemCheck(menuName,"select_row",false);
		parent->setMenuItemCheck(menuName,"select_column",false);

		const char* const toolbarName = pp->toolbarName();
		parent->setToolBarItemCheck(toolbarName,"drag_plot", false);
      parent->setToolBarItemCheck(toolbarName,"display_data", true);
      parent->setToolBarItemCheck(toolbarName,"select_region", false);
      parent->setToolBarItemCheck(toolbarName,"select_row", false);
      parent->setToolBarItemCheck(toolbarName,"select_column", false);
   }
   else if(func == "drag plot")
   {
	   pp->curPlot()->HideDataCursor(hdc);
      gResetCursor = LoadCursor(NULL,IDC_SIZEALL);
      SetCursor(gResetCursor);
      pp->mouseMode = MOVE_PLOT;
      UpdateStatusWindow(hWnd,1,"Left button : Drag plot");

		WinData* parent = pp->obj->winParent;
		const char* const menuName = pp->menuName();
      parent->setMenuItemCheck(menuName,"drag_plot",true);
      parent->setMenuItemCheck(menuName,"display_data",false);
      parent->setMenuItemCheck(menuName,"select_region",false);
      parent->setMenuItemCheck(menuName,"select_row",false);
      parent->setMenuItemCheck(menuName,"select_column",false);

		const char* const toolbarName = pp->toolbarName();
      parent->setToolBarItemCheck(toolbarName,"drag_plot", true);
      parent->setToolBarItemCheck(toolbarName,"display_data", false);
      parent->setToolBarItemCheck(toolbarName,"select_region", false);
      parent->setToolBarItemCheck(toolbarName,"select_row", false);
      parent->setToolBarItemCheck(toolbarName,"select_column", false);
   }
   else if(func == "enlarge horizontal")
   {
		Plot* curPlot = pp->curPlot();
	   curPlot->HideDataCursor(hdc);  	        	   
      curPlot->ScalePlot(hWnd, LOWORD(ID_ENLARGE_HORIZ));   
      curPlot->HideSelectionRectangle(hdc);  	        
      curPlot->Invalidate();
		pp->Paint();
      SendMessageToGUI("2D Plot,Enlarge Horizontal",0); 
   }
   else if(func == "enlarge vertical")
   {
		Plot* curPlot = pp->curPlot();
	   curPlot->HideDataCursor(hdc);        	   
      curPlot->ScalePlot(hWnd, LOWORD(ID_ENLARGE_VERT));    	        
      curPlot->Invalidate();
      pp->Paint();
      SendMessageToGUI("2D Plot,Reduce Horizontal",0); 
   }
   else if(func == "full region")
   {
		Plot* curPlot = pp->curPlot();
  	   curPlot->resetZoomCount(); 
      Plot2D::curPlot()->FullRegion();
      curPlot->Invalidate();
		curPlot->updateStatusWindowForZoom(hWnd);
      SendMessageToGUI("2D Plot,DisplayAll",0); 
   }
   else if(func == "hide borders")
   {
      pp->showLabels = false;
	   pp->curPlot()->Invalidate();     	
   }
   else if(func == "image colours")
   {
      DialogBox(prospaInstance,"COLORDLG_2D",hWnd,Plot2DColorDlgProc);
   }
   else if(func == "last region")
   {
		Plot* curPlot = pp->curPlot();
		if (!curPlot->lastRegion(hWnd))
		{
         MessageDialog(prospaWin,MB_ICONINFORMATION,"Message","No regions in 2D history list");
		}
		curPlot->Invalidate();
      SendMessageToGUI("2D Plot,LastImage",0);
   }
   else if(func == "load plot")
   {
      if(Load2DPlotDialog(hWnd) != OK)
         return(OK); 
      SetCursor(LoadCursor(NULL,IDC_WAIT));
		if(pp)
		{
			if(pp->LoadAndDisplayPlotFile(hWnd, Plot2D::currFilePath, Plot2D::currFileName) == ERR)
            return(ERR);
		}
		SetCursor(LoadCursor(NULL,IDC_ARROW));
      pp->curPlot()->initialiseMenuChecks("load");
   }
   else if(func == "move up")
   {
		Plot* curPlot = pp->curPlot();
	   curPlot->HideDataCursor(hdc);	        	   
      curPlot->ShiftPlot(LOWORD(ID_SHIFT_UP));   
      curPlot->HideSelectionRectangle(hdc);  	        
      curPlot->Invalidate();
      pp->Paint();
      SendMessageToGUI("2D Plot,Move Up",0); 
   }
   else if(func == "move down")
   {
		Plot* curPlot = pp->curPlot();
	   curPlot->HideDataCursor(hdc);	        	   
      curPlot->ShiftPlot(LOWORD(ID_SHIFT_DOWN));   
      curPlot->HideSelectionRectangle(hdc);  	        
      curPlot->Invalidate();
      pp->Paint();
      SendMessageToGUI("2D Plot,Move Down",0); 
   }
   else if(func == "move left")
	{
		Plot* curPlot = pp->curPlot();
	   curPlot->HideDataCursor(hdc);	        	   
      curPlot->ShiftPlot(LOWORD(ID_SHIFT_LEFT));   
      curPlot->HideSelectionRectangle(hdc);  	        
      curPlot->Invalidate();
      pp->Paint();
      SendMessageToGUI("2D Plot,Move Left",0); 
   }
   else if(func == "move right")
   {
		Plot* curPlot = pp->curPlot();
	   curPlot->HideDataCursor(hdc);	        	   
      curPlot->ShiftPlot(LOWORD(ID_SHIFT_RIGHT));   
      curPlot->HideSelectionRectangle(hdc);  	        
      curPlot->Invalidate();
      pp->Paint();
      SendMessageToGUI("2D Plot,Move Right",0); 
   }
   else if(func == "multiplot 1*1")
   {
       pp->AdjustMultiPlotSize(1,1);  
       pp->curPlot()->Invalidate();	    	           
   }
   else if(func == "multiplot 1*2")
   {
       pp->AdjustMultiPlotSize(2,1);  
       pp->curPlot()->Invalidate();	     	           
   }
   else if(func == "multiplot 2*1")
   {
       pp->AdjustMultiPlotSize(1,2);  
       pp->curPlot()->Invalidate();	     	           
   }
   else if(func == "multiplot 2*2")
   {
       pp->AdjustMultiPlotSize(2,2);  
       pp->curPlot()->Invalidate();	    	           
   }
   else if(func == "multiplot m*n")
   {
       gMultiPlotYCells = pp->rows;
       gMultiPlotXCells = pp->cols;	   
   	 if(DialogBox(prospaInstance,"MULTIPLOTDLG",hWnd,MultiPlotDlgProc))
       {
          ReleaseDC(pp->curPlot()->win,hdc);
          return(OK);	   
		 }
       pp->AdjustMultiPlotSize(gMultiPlotXCells,gMultiPlotYCells);             
       pp->curPlot()->Invalidate();	     	           
   }	
   else if(func == "paste plot")
   {
      pp->PasteSavedPlot();
		Plot::curPlot()->makeCurrentDimensionalPlot();
	   pp->curPlot()->Invalidate(); 
   }
   else if(func == "plot title")
   {
		if (pp->StylePlotTitles())
		{
         pp->curPlot()->Invalidate();
      }
   }
   else if(func == "plot x,y labels")
   {
		if (pp->StylePlotAxisLabels())
		{
         pp->curPlot()->Invalidate();	
      }
   }
   else if(func == "print plot")
   {
      if(DialogBox(prospaInstance,"PRINTDLG",hWnd,PrintDlgProc))
         PrintDisplay(pp);
   }
   else if(func == "reduce horizontal")
   {
		Plot* curPlot = pp->curPlot();
	   curPlot->HideDataCursor(hdc);	        	   
      curPlot->ScalePlot(hWnd, LOWORD(ID_REDUCE_HORIZ));   
      curPlot->HideSelectionRectangle(hdc);  	        
      curPlot->Invalidate();
      pp->Paint();
      SendMessageToGUI("2D Plot,Reduce Horizontal",0); 
   }
   else if(func == "reduce vertical")
	{
		Plot* curPlot = pp->curPlot();
	   curPlot->HideDataCursor(hdc);	        	   
      curPlot->ScalePlot(hWnd, LOWORD(ID_REDUCE_VERT));   
      curPlot->HideSelectionRectangle(hdc);  	        
      curPlot->Invalidate();
      pp->Paint();
      SendMessageToGUI("2D Plot,Reduce Vertical",0); 
   }
   else if(func == "remove current data")
   {
      RECT r;
      pp->CloseCurrentPlot();
		Plot2D* curPlot2D = Plot2D::curPlot();
      GetClientRect(curPlot2D->win,&r);
      long width = r.right-r.left+1;
      long height = r.bottom-r.top+1;
      SendMessage(curPlot2D->win,WM_SIZE,(WPARAM)SIZE_RESTORED,(LPARAM)(width+(height<<16)));       
   }
   else if(func == "remove other subplots")
   {
      RECT r;
      pp->RemoveAllButCurrentPlot();
		Plot* curPlot = pp->curPlot();
	   GetClientRect(curPlot->win,&r);
      long width = r.right-r.left+1;
	   long height = r.bottom-r.top+1;
	   SendMessage(curPlot->win,WM_SIZE,(WPARAM)SIZE_RESTORED,(LPARAM)(width+(height<<16)));       
	}	
   else if(func == "save as image")
   {
		Plot2D* cur2DPlot = Plot2D::curPlot();
      if(!(cur2DPlot->mat()) && !(cur2DPlot->cmat()))
      {
         MessageDialog(prospaWin,MB_ICONERROR,"Error","No 2D data to save");
         return(OK);
      }  
      pp->SaveAsImage(NULL);
      return(OK);
   } 
   else if(func == "save plot")
   {
		Plot* curPlot = pp->curPlot();
		Plot2D* cur2DPlot = Plot2D::curPlot();
      if(!(cur2DPlot->mat()) && !(cur2DPlot->cmat()))
      {
         MessageDialog(prospaWin,MB_ICONERROR,"Error","No 2D plot data to save");
         return(OK);
      }	
      long version;
      if(Save2DPlotDialog(hWnd, curPlot->getFilePath(), curPlot->getFileName(), version) != OK)
         return(OK);

      long bak = plot2DSaveVersion;
      version =  fileVersionNumberToConstant(version,2);
      plot2DSaveVersion =  version;
      SetCursor(LoadCursor(NULL,IDC_WAIT));
      pp->SavePlots(curPlot->getFilePath(), curPlot->getFileName(),-1,-1);
      SetCursor(LoadCursor(NULL,IDC_ARROW));
      plot2DSaveVersion =  bak;

   }
   else if(func == "select column")
   {
      pp->ClearCursors(hdc);
      gResetCursor = LoadCursor(NULL,IDC_SIZENS);
      SetCursor(gResetCursor); 
      pp->mouseMode = SELECT_COLUMN;
      UpdateStatusWindow(hWnd,1,"Left button : Select column");

		WinData* parent = pp->obj->winParent;
		const char* const m = pp->menuName();
		const char* const tbn = pp->toolbarName();
      parent->setMenuItemCheck(m,"drag_plot",false);
      parent->setMenuItemCheck(m,"display_data",false);
      parent->setMenuItemCheck(m,"select_region",false);
      parent->setMenuItemCheck(m,"select_row",false);
      parent->setMenuItemCheck(m,"select_column",true);
      parent->setToolBarItemCheck(tbn, "drag_plot", false);
      parent->setToolBarItemCheck(tbn, "display_data", false);
      parent->setToolBarItemCheck(tbn, "select_region", false);
      parent->setToolBarItemCheck(tbn, "select_row", false);
      parent->setToolBarItemCheck(tbn, "select_column", true);
   }
   else if(func == "select region")
   {
	   pp->curPlot()->HideDataCursor(hdc);
      pp->mouseMode = SELECT_RECT;
      gResetCursor = SquareCursor;
      SetCursor(gResetCursor); 
      UpdateStatusWindow(hWnd,1,"Left button : Select a region");

		WinData* parent = pp->obj->winParent;
		const char* const m = pp->menuName();
		const char* const tbn = pp->toolbarName();
      parent->setMenuItemCheck(m,"drag_plot",false);
      parent->setMenuItemCheck(m,"display_data",false);
      parent->setMenuItemCheck(m,"select_region",true);
      parent->setMenuItemCheck(m,"select_row",false);
      parent->setMenuItemCheck(m,"select_column",false);
      parent->setToolBarItemCheck(tbn, "drag_plot", false);
      parent->setToolBarItemCheck(tbn, "display_data", false);
      parent->setToolBarItemCheck(tbn, "select_region", true);
      parent->setToolBarItemCheck(tbn, "select_row", false);
      parent->setToolBarItemCheck(tbn, "select_column", false);
   }
   else if(func == "select row")
   {
      pp->ClearCursors(hdc);
      gResetCursor = LoadCursor(NULL,IDC_SIZEWE);
      SetCursor(gResetCursor); 
      pp->mouseMode = SELECT_ROW;
      UpdateStatusWindow(hWnd,1,"Left button : Select row");

		WinData* parent = pp->obj->winParent;
		const char* const m = pp->menuName();
		const char* const tbn = pp->toolbarName();
      parent->setMenuItemCheck(m,"drag_plot",false);
      parent->setMenuItemCheck(m,"display_data",false);
      parent->setMenuItemCheck(m,"select_region",false);
      parent->setMenuItemCheck(m,"select_row",true);
      parent->setMenuItemCheck(m,"select_column",false);
      parent->setToolBarItemCheck(tbn, "drag_plot", false);
      parent->setToolBarItemCheck(tbn, "display_data", false);
      parent->setToolBarItemCheck(tbn, "select_region", false);
      parent->setToolBarItemCheck(tbn, "select_row", true);
      parent->setToolBarItemCheck(tbn, "select_column", false);
   }
   else if(func == "show all plots")
   {
		pp->ViewFullPlot();
   }
   else if(func == "show borders")
   {
      pp->showLabels = true;
	   pp->curPlot()->Invalidate();     	
   }
   else if(func == "toggle border")
   {
      pp->showLabels = !pp->showLabels;
      bool state = pp->showLabels;
      pp->obj->winParent->setMenuItemCheck(pp->menuName(), "toggle_border", state);
		pp->curPlot()->Invalidate();     	
   }
   else if(func == "toggle colorbar")
   {
		Plot* curPlot = pp->curPlot();
      Plot2D::curPlot()->setDisplayColorScale(!curPlot->displayColorScale());
      bool state = curPlot->displayColorScale();
      pp->obj->winParent->setToolBarItemCheck(pp->toolbarName(), "toggle_colorbar", state);
      pp->curPlot()->Invalidate();   
   }
   else if(func == "view one plot")
   {
      if(pp->rows*pp->cols > 1)
      {
         pp->DisplayOnePlot();
      }
   }
   else if(func == "zoom region")
   {
		Plot* curPlot = pp->curPlot();
      if(pp->mouseMode == SHOW_DATA) 
      {
         curPlot->HideDataCursor(hdc);
         pp->mouseMode = SELECT_RECT;
      }
      if(curPlot->rectSelected)
      {
   	   curPlot->ZoomRegion();
			curPlot->updateStatusWindowForZoom(hWnd);
   	   curPlot->Invalidate();
         SendMessageToGUI("2D Plot,Zoom",0);
      }
      else
      {
         MessageDialog(prospaWin,MB_ICONINFORMATION,"Message","A region needs to be selected first");
      }	         
   } 
   else
   {
      ErrorMessage("unknown function");
      ReleaseDC(Plot2D::curPlot()->win,hdc);
      return(ERR);
   }

   ReleaseDC(pp->curPlot()->win,hdc);
	itfc->nrRetValues = 0;
   return(OK);
}


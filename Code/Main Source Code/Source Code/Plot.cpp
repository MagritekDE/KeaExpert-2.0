#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)
#include <gdiplus.h> 
#include "stdafx.h"
#include "defines.h"
#include "plot.h"
#include "Inset.h"
#include "plotText.h"
#include "plotLine.h"
#include "allocate.h"
#include "defineWindows.h"
#include "drawing.h"
#include "font.h"
#include "globals.h"
#include "guiModifyObjectParameters.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include <math.h>
#include "mymath.h"
#include "process.h"
#include "PlotFile.h"
#include "PlotGrid.h"
#include "plotwindefaults.h"
#include "PlotWindow.h"
#include "print.h"
#include "prospaResource.h"
#include "plot_dialog_1d.h"
#include "rgbColors.h"
#include "string_utilities.h"
#include "utilities.h"
#include "variablesOther.h"
#include "winEnableClass.h"
#include <string>
#include <algorithm>
#include <assert.h>
#include <functional>
#include "memoryLeak.h"

// Suppress the warning about using a reference to "this" in base member initializers.
#pragma warning( disable : 4355 )

using namespace Gdiplus;
using namespace std;
// Global variables

bool Plot::zoomPoint = false;
float Plot::gZoomX = 0;
float Plot::gZoomY = 0; // Zoom coordinates

RECT toolBarRect1D; // This is not given values anywhere. I moved it from plot1DEvents here; it was never assigned.

Plot* Plot::_curPlot = 0;
bool Plot::gApplyToAll = false; // Apply font selections to all windows?
bool Plot::g_chooseBkgColor = false;
bool Plot::g_bkgColorEnabled = false;

Margins	defaultMargins1D(70,45,45,55);
Margins	defaultMargins2D(70,45,45,55);

// Constructor for Plot class
Plot::Plot(PlotWinDefaults *pd, HWND hWnd, PlotWindow *pp, short row, short col)
: margins(70,45,45,55), Scalable()
{
   win = hWnd;

	ProspaFont titleFont(pd->titleFontName, pd->titleFontColor, pd->titleFontSize,  pd->titleFontStyle);
	title_ = new TitleLabel(this, titleFont, "\0");

   validationCode = GetNextObjectValidationCode(); // Makes this object unique

// General stuff (1D and 2D)
   rowNr = row;
   colNr = col;
   ResetSelectionRectangle();
   strncpy_s(plotStr,MAX_STR,"unused",_TRUNCATE);

	regionLeft = 0;
	regionTop = 0;
	regionBottom = 0;
	regionRight = 0;
	selectRect.left = 0;
	selectRect.top = 0;
	selectRect.bottom = 0;
	selectRect.right = 0;

	xAxis_ = 0;
	yAxisR_ = 0;
	yAxisL_ = 0;
	curXAxis_ = 0;
	curYAxis_ = 0;

// 1D specific stuff
	antiAliasing = pd->antiAliasing;

   overRideAutoRange = false;   

   updatePlots_ = true;

	updatingPlots_ = false;

   displayHold = false;
	syncAxes_ = true;

	gridLocked_ = false;
   	
// Initialise plot colors
   bkColor        = pd->bkColor;
   axesColor      = pd->axesColor;
   borderColor    = pd->borderColor;

   axesMode      = pd->axesMode;
      
   filePath[0] = '\0';
   fileName[0] = '\0';
   statusText[0] ='\0';
   
   yLabelVert = true;
   
   plotParent = pp;

	strncpy_s(filePath,PLOT_STR_LEN,PlotFile1D::getCurrPlotDirectory(),_TRUNCATE);
   strncpy_s(fileName,PLOT_STR_LEN,"untitled",_TRUNCATE);

   fileVersion = 0;

   traceXBorderFactor = 0.0;
   traceYBorderFactor = 0.1;

   xCalibrated_ = false;
   yCalibrated_ = false;

   allowMakeCurrentPlot_ = true;

   zoomBkgColor = pd->zoomBkgColor;         // Zoom rectangle color
   zoomBorderColor = pd->zoomBorderColor;   // Zoom rectangle color
   zoomRectMode = pd->zoomRectMode;         // Zoom rect drawing mode

   plotBeingMoved = false; // Parameters for interactive plot movement
   plotOldX = -1;
   plotOldY = -1;
   plotNewX = -1;
   plotNewY = -1;

   rectSelected = false;   

   plotCmdScaled = false;
   plotMouseScaled = false;
   plotShifted = false;
}

// Constructor for Plot class
Plot::Plot(PlotWinDefaults *pd, HWND hWnd, PlotWindow *pp, short row, short col, Margins &marg)
: margins(marg), Scalable()
{
   win = hWnd;

//	this->setMargins(margins);

	ProspaFont titleFont(pd->titleFontName, pd->titleFontColor, pd->titleFontSize,  pd->titleFontStyle);
	title_ = new TitleLabel(this, titleFont, "\0");

   validationCode = GetNextObjectValidationCode(); // Makes this object unique

// General stuff (1D and 2D)
   rowNr = row;
   colNr = col;
   ResetSelectionRectangle();
   strncpy_s(plotStr,MAX_STR,"unused",_TRUNCATE);

	regionLeft = 0;
	regionTop = 0;
	regionBottom = 0;
	regionRight = 0;
	selectRect.left = 0;
	selectRect.top = 0;
	selectRect.bottom = 0;
	selectRect.right = 0;

	xAxis_ = 0;
	yAxisR_ = 0;
	yAxisL_ = 0;
	curXAxis_ = 0;
	curYAxis_ = 0;

// 1D specific stuff
	antiAliasing = pd->antiAliasing;

   overRideAutoRange = false;   

   updatePlots_ = true;

  updatingPlots_ = false;

   displayHold = false;
	syncAxes_ = true;

	gridLocked_ = false;
   	
// Initialise plot colors
   bkColor        = pd->bkColor;
   axesColor      = pd->axesColor;
   borderColor    = pd->borderColor;

   axesMode      = pd->axesMode;
      
   filePath[0] = '\0';
   fileName[0] = '\0';
   statusText[0] ='\0';
   
   yLabelVert = true;
   
   plotParent = pp;

	strncpy_s(filePath,PLOT_STR_LEN,PlotFile1D::getCurrPlotDirectory(),_TRUNCATE);
   strncpy_s(fileName,PLOT_STR_LEN,"untitled",_TRUNCATE);

   fileVersion = 0;

   traceXBorderFactor = 0.0;
   traceYBorderFactor = 0.1;

   xCalibrated_ = false;
   yCalibrated_ = false;

   allowMakeCurrentPlot_ = true;

   zoomBkgColor = pd->zoomBkgColor;         // Zoom rectangle color
   zoomBorderColor = pd->zoomBorderColor;   // Zoom rectangle color
   zoomRectMode = pd->zoomRectMode;         // Zoom rect drawing mode

   plotBeingMoved = false; // Parameters for interactive plot movement
   plotOldX = -1;
   plotOldY = -1;
   plotNewX = -1;
   plotNewY = -1;

   rectSelected = false;   

   plotCmdScaled = false;
   plotMouseScaled = false;
   plotShifted = false;
}

Plot::~Plot()
{
	axisList_.clear();
	delete xAxis_;
	delete yAxisL_;
	delete yAxisR_;
	delete title_;
	removeLines();
   removeTexts();
   removeInsets();
}

void Plot::removeLines()
{
   for(int i = 0; i < lines_.size(); i++)
   {
      PlotLine *p = lines_[i];
      delete p;
   }
// This is not calling the destructor - why some header problem?
  // std::for_each(lines_.begin(), lines_.end(), delete_object());
   lines_.clear();
}

void Plot::removeInsets()
{
	std::for_each(insets_.begin(), insets_.end(), delete_object());
   insets_.clear();
}

void Plot::removeTexts()
{
	std::for_each(text_.begin(), text_.end(), delete_object());
   text_.clear();
}

void Plot::Invalidate()
{
	RECT plotWinRect;
	GetClientRect(win,&plotWinRect);
	MyInvalidateRect(win,&plotWinRect,false);
  // DisplayAll(false);
}

Plot::Plot(const Plot& c)
: margins(c.margins), dimensions(c.dimensions),	Scalable(c)
{
	strncpy_s(plotStr,MAX_STR,c.plotStr,_TRUNCATE);

	title_ = c.title_->clone();
	static_cast<TitleLabel*>(title_)->setParent(this);
	xAxis_ = c.xAxis_->clone();
	yAxisL_ = c.yAxisL_->clone();
	yAxisR_ = c.yAxisR_->clone();
	xAxis_->setParent(this);
	yAxisL_->setParent(this);
	yAxisR_->setParent(this);

	axisList_.push_back(xAxis_);
	axisList_.push_back(yAxisL_);
	axisList_.push_back(yAxisR_);

	curXAxis_ = xAxis_;
	curYAxis_ = yAxisL_;
	if (c.gridLocked_)
	{
		if (c.gridLocked_ == c.yAxisL_)
		{
			gridLocked_ = yAxisL_;
		}
		else
		{
			gridLocked_ = yAxisR_;
		}
	}
   else
   {
		gridLocked_ = NULL;
   }
	syncAxes_ = c.syncAxes_;

	overRideAutoRange = c.overRideAutoRange;

	yLabelVert = c.yLabelVert;       // Is y label to be vertical

	axesMode = c.axesMode;        // Type of axes: CORNERAXES, BORDERAXES, CENTERAXES

	strncpy_s(fileName,PLOT_STR_LEN,c.fileName,_TRUNCATE);
	strncpy_s(filePath,PLOT_STR_LEN,c.filePath,_TRUNCATE);
	strncpy_s(statusText,PLOT_STR_LEN,c.statusText,_TRUNCATE);

	axesColor = c.axesColor;       
	bkColor = c.bkColor;
	plotColor = c.plotColor;
	borderColor = c.borderColor;


	rowNr = c.rowNr;  // Where the plot is in relation to others
	colNr = c.colNr;

	regionLeft = c.regionLeft; 
	regionTop = c.regionTop;
	regionBottom = c.regionBottom;
	regionRight = c.regionRight;

	win = c.win;         // Parent window

	displayHold = c.displayHold;

	selectRect = c.selectRect; // Selected rectangle (data coordinates)
	scrnSelectRect = c.scrnSelectRect;  // Selected rectangle (screen coordinates)
	rectSelected = c.rectSelected;  // Has a rectange been selected?

	plotParent = c.plotParent;
	antiAliasing = c.antiAliasing;
	validationCode = GetNextObjectValidationCode(); 

	for(Inset* inset : c.insets_)
	{
		if (!inset->isSaveable())
		{
			continue;
		}
		Inset* copied = inset->clone();
		copied->setParent(this);
		insets_.push_back(copied);
	}

   updatePlots_ = true;
   updatingPlots_ = false;

	zoomHistory = c.zoomHistory;

   fileVersion = c.fileVersion;

   traceXBorderFactor  = c.traceXBorderFactor;
   traceYBorderFactor  = c.traceYBorderFactor;

   xCalibrated_ = c.xCalibrated_;
   yCalibrated_ = c.yCalibrated_;

   allowMakeCurrentPlot_ = c.allowMakeCurrentPlot_;

	plotBeingMoved = false; // Parameters for interactive plot movement
	plotOldX = -1;
	plotOldY = -1;
	plotNewX = -1;
	plotNewY = -1;
}

void Plot::makeCurrentPlot()
{
	_curPlot = this;
}

Plot* Plot::curPlot()
{
	return _curPlot;
}

void Plot::setNoCurPlot()
{
	_curPlot = 0;
}

COLORREF Plot::ChooseAColor(HWND hWnd, COLORREF init, bool bkgColorEnabled)
{
   static COLORREF crCustColors[16];
   static CHOOSECOLOR cc;
   CWinEnable winEnable;

	g_chooseBkgColor = false;
   g_bkgColorEnabled = bkgColorEnabled;   
   cc.lStructSize = sizeof(CHOOSECOLOR);
   cc.hwndOwner = hWnd;
   cc.hInstance = NULL;
   cc.lpCustColors = crCustColors;
   cc.Flags = CC_RGBINIT | CC_FULLOPEN | CC_ENABLEHOOK | CC_ENABLETEMPLATE;
   cc.lCustData = 0;
   cc.lpfnHook = ChooseColorHookProc;
   cc.lpTemplateName = "CHOOSECOLORDLG";
   cc.rgbResult = init;
 
   winEnable.Disable(NULL);
   if(ChooseColor(&cc))
   {
      winEnable.Enable(NULL);
      if(g_bkgColorEnabled && g_chooseBkgColor)
         return(0xFFFFFFFF);
      else
         return(cc.rgbResult);
   }
   winEnable.Enable(NULL);
   return(ABORT_COLOR_CHOICE);
}

bool Plot::isUpdatePlots()
{
	return plotParent->updatePlots();
}


/*****************************************************************************************
* Select a rectangular region in a plot. Results are returned in screen
* coordinates in the plot member variable scrnSelectRect.
* By holding down the shift key the selected region will have
* dimensions in data coordinates which are powers of two. NOT ANY MORE
****************************************************************************************/

short Plot::SelectRegion(HWND hWnd, short xs0, short ys0)
{
	MSG msg ;
   short xs1,ys1;
   long xmin,xmax,ymin,ymax;
   char str[MAX_STR];
   extern bool gScrollWheelEvent;

// Get region limits
	xmin = dimensions.left();
	ymin = dimensions.top();
	xmax = dimensions.left() + dimensions.width();
	ymax = dimensions.top() + dimensions.height();

// Check for valid starting point inside plot region
   if(xs0 < xmin || xs0 > xmax || ys0 < ymin || ys0 > ymax)
      return(ABORT);
      
   gScrollWheelEvent = false;

// Initalise begin and end coordinates
   xs1 = xs0;
   ys1 = ys0;

// Force window to capture mouse events
   SetCapture(hWnd);
   rectSelected = true;

// Track the cursor as the region is selected with the mouse             	
   while(true)
   {
	   if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	   {
         if(msg.hwnd == hWnd)
         {
	         if(msg.message == WM_LBUTTONUP ||  // Region has been selected
	            msg.message == WM_LBUTTONDOWN)  // Very unlikely case - if pop-up window has interrupted process
	         {					
			      break;	// Exit while loop
	         }

	         else if(msg.message == WM_MOUSEMOVE && msg.hwnd == hWnd) // Mouse is moving so track
			   {	    
               if((msg.wParam & MK_LBUTTON) == 0) // If left button up is somehow missed.
                  break;

               xs1 = LOWORD(msg.lParam);  // Get cursor position
               ys1 = HIWORD(msg.lParam);
		   
				   if(xs1 < xmin) xs1 = xmin; // Check for out of bounds cursor location
				   if(xs1 > xmax) xs1 = xmax; // and limit to plot border.
				   if(ys1 < ymin) ys1 = ymin;
				   if(ys1 > ymax) ys1 = ymax;

               gScrollWheelEvent = false;
               scrnSelectRect.left    = xs0;
               scrnSelectRect.top     = ys0;
               scrnSelectRect.right   = xs1;
               scrnSelectRect.bottom  = ys1;
               InvalidateRect(hWnd,0,false);
               UpdateWindow(hWnd); // Make sure the zoom rectangle is drawn
			   }
         }
		}
   }

// Release mouse from window
   ReleaseCapture();
   
// Ignore an empty rectangle
   if(xs0 == xs1 || ys0 == ys1)
   {
		ResetSelectionRectangle();
      return(OK);
   }

// Record screen coordinates of rectangle (for later removal)
   scrnSelectRect.left    = xs0;
   scrnSelectRect.top     = ys0;
   scrnSelectRect.right   = xs1;
   scrnSelectRect.bottom  = ys1;

// Send message to GUI
	if(this == Plot2D::curPlot())
      SendMessageToGUI("2D Plot,DrawRectangle",0);
   else
      SendMessageToGUI("1D Plot,DrawRectangle",0);

   return(OK);
}

/*****************************************************************************************
* Select a rectangular region in a plot. Results are returned in screen
* coordinates in the plot member variable scrnSelectRect and in 
* data coordinates in selectRect
* By holding down the shift key the selected region will have
* dimensions in data coordinates which are powers of two.
****************************************************************************************/

short Plot::GetDataAtCursor(HWND hWnd, short xs0, short ys0)
{
	MSG msg ;
   short xs1,ys1;
   long xmin,xmax,ymin,ymax;
   char str[MAX_STR];
   extern bool gScrollWheelEvent;

// Get plot region limies
	xmin = dimensions.left();
	ymin = dimensions.top();
	xmax = dimensions.left() + dimensions.width();
	ymax = dimensions.top() + dimensions.height();

// Check for valid starting point inside plot region
   if(xs0 < xmin || xs0 > xmax || ys0 < ymin || ys0 > ymax)
      return(ABORT);
      
   gScrollWheelEvent = false;

// Initalise begin and end coordinates
   xs1 = xs0;
   ys1 = ys0;

// Force window to capture mouse events
   SetCapture(hWnd);

// Track the cursor as the region is selected with the mouse             	
   while(true)
   {
	   if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	   {
         if(msg.hwnd == hWnd)
         {
	         if(msg.message == WM_LBUTTONUP ||  // Data measurement has been stopped
	            msg.message == WM_LBUTTONDOWN)  // Very unlikely case - if pop-up window has interrupted process
	         {					
			      break;	// Exit while loop
	         }
				else if(msg.message == WM_RBUTTONDOWN)
				{
            // If right button is pressed toggle an offset
					if(msg.wParam & MK_LBUTTON)
					{	
						isOffset_ = !isOffset_;
						xOffsetScrn_ = LOWORD(msg.lParam);;
						yOffsetScrn_ = HIWORD(msg.lParam);
						InvalidateRect(hWnd,0,false);
						UpdateWindow(hWnd);
					}
				}
	         else if(msg.message == WM_MOUSEMOVE && msg.hwnd == hWnd) // Mouse is moving so track
			   {	    
               if((msg.wParam & MK_LBUTTON) == 0) // If left button up is somehow missed.
                  break;

               xs1 = LOWORD(msg.lParam);  // Get cursor position
               ys1 = HIWORD(msg.lParam);
		   
				   if(xs1 < xmin) xs1 = xmin; // Check for out of bounds cursor location
				   if(xs1 > xmax) xs1 = xmax; // and limit to plot border.
				   if(ys1 < ymin) ys1 = ymin;
				   if(ys1 > ymax) ys1 = ymax;

               dataCursor.x  = xs1;
               dataCursor.y  = ys1;

               InvalidateRect(hWnd,0,false);
               UpdateWindow(hWnd); // Make sure the zoom rectangle is drawn
			   }
         }
		}
   }

// Release mouse from window
   ReleaseCapture();

// Signal that the cursor is not present
   dataCursor.x = -1;
   dataCursor.y = -1;

// Update the display to remove the curstor
   InvalidateRect(hWnd,0,false);
   UpdateWindow(hWnd);

   return(OK);
}


HBRUSH Plot::GetPlotBorderBrush()
{
	HBRUSH borderBrush;

   if(gPlotMode == PRINT && gPrintMode == BLACK_AND_WHITE)
   {
	   borderBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
   }
   else
   {
      if(borderColor == 0xFFFFFFFF)  
	      borderBrush  = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	   else
	      borderBrush  = CreateSolidBrush(borderColor);
	}
	return borderBrush;
}

void Plot::HideSelectionRectangle(HDC hdc)
{
	rectSelected = false;
}


void Plot::ScalingPlotByCmd()
{
   if(!plotCmdScaled)
      return;
   EnlargePlotCentred(scaleDirection);
   plotCmdScaled = false;
}

void Plot::ScalingPlotByMouseWheel()
{
   if(!plotMouseScaled)
      return;
   ScalePlot(GetParent(this->plotParent->hWnd),scaleDirection);
   plotMouseScaled = false;
}


void Plot::MovingPlotByCmd()
{
   if(!plotShifted)
      return;

   ShiftPlot(shiftDirection);

   plotShifted = false;
}


void Plot::MovingPlotByMouse()
{

   if(!plotBeingMoved)
      return;

   long oldx = plotOldX;
   long oldy = plotOldY;
   long x = plotNewX;
   long y = plotNewY;

   Axis* curXAxis = this->curXAxis();
	Axis* curYAxis = this->curYAxis();
	Axis* otherYAxis = this->otherYAxis();
   float dx,dy;


	dx = curXAxis->scrnToData(x) - curXAxis->scrnToData(oldx);
   short mappingBack = curYAxis->mapping();
							
   if(this->axesMode != PLOT_AXIS_BOX_Y_INDEPENDENT) 
      dy = curYAxis->scrnToData(y) - curYAxis->scrnToData(oldy);
   else
   {
      int h = this->getDimensions().height();
      float rng = curYAxis->MaxIndep()-curYAxis->MinIndep();
      dy = -(y-oldy)*rng/(0.9*h);
   }

   if(curXAxis->mapping() == PLOT_LOG_X) // Log x mapping
   {
      if(dx != 0)
      {
         float xmin = log10(curXAxis->Min());
         float xmax = log10(curXAxis->Max());
         float xold = log10(curXAxis->scrnToData(oldx));
         float xnew = log10(curXAxis->scrnToData(x));
         dx = -(xnew - xold)/(xmax-xmin);
         float newMin = pow((double)10.0,(double)((xmax - xmin) * dx + xmin));
         float newMax = pow((double)10.0,(double)((xmax - xmin) * dx + xmax));    
			curXAxis->setMin(newMin);
			curXAxis->setMax(newMax); 
      }
   }
   else // Linear x mapping
   {
 		curXAxis->setMin(curXAxis->Min() - dx);
		curXAxis->setMax(curXAxis->Max() - dx);                       
   }
   if(curYAxis->mapping() == PLOT_LOG_Y)
   {
      if(this->axesMode != PLOT_AXIS_BOX_Y_INDEPENDENT) // Log y mapping
      {
         if(dy != 0)
         {
            float ymin = log10(curYAxis->Min());
            float ymax = log10(curYAxis->Max());
            float yold = log10(curYAxis->scrnToData(oldy));
            float ynew = log10(curYAxis->scrnToData(y));
            dy = -(ynew - yold)/(ymax-ymin);
            float newMin = pow((double)10.0,(double)((ymax - ymin) * dy + ymin));
            float newMax = pow((double)10.0,(double)((ymax - ymin) * dy + ymax));    
				curYAxis->setMin(newMin);
				curYAxis->setMax(newMax); 
         }
      }
      else
      {
         if(dy != 0)
         {
            int b = this->getDimensions().bottom();
            int t = this->getDimensions().top();
            float ymin = log10(curYAxis->MinIndep());
            float ymax = log10(curYAxis->MaxIndep());
            float rng = 0.9*curYAxis->MaxIndep()-curYAxis->MinIndep();
            float yold = ((b-oldy)/(float)(b-t)-0.05)*(ymax-ymin)+ymin;
            float ynew = ((b-y)/(float)(b-t)-0.05)*(ymax-ymin)+ymin;
            dy = -(ynew - yold)/(ymax-ymin);
            float newMin = pow((double)10.0,(double)((ymax - ymin) * dy + ymin));
            float newMax = pow((double)10.0,(double)((ymax - ymin) * dy + ymax));    
				curYAxis->setMinIndep(newMin);
				curYAxis->setMaxIndep(newMax); 
         }
      }
   }
   else // Linear y mapping
   {
		curYAxis->setMin(curYAxis->Min() - dy);
		curYAxis->setMax(curYAxis->Max() - dy);
		curYAxis->setMinIndep(curYAxis->MinIndep() - dy);
		curYAxis->setMaxIndep(curYAxis->MaxIndep() - dy);  
//      TextMessage("%f %f %f\n",dy,curYAxis->MinIndep(),curYAxis->MaxIndep());
   }

   curYAxis->setMapping(mappingBack);


// If left and right axes are synchronised then move the other axis as well
	if (this->syncAxes())
	{
		// Move the other axis by the same proportion as the selected axis is moving.
		// Find the proportion of dy to the current Y axis' range.
		double proportion = ((double) dy) / (curYAxis->Max() - curYAxis->Min());
		// Find the corresponding proportion of the other Y axis' range.
		float correspondingOther = proportion * (otherYAxis->Max() - otherYAxis->Min());
									
		otherYAxis->setMin(otherYAxis->Min() - correspondingOther);
		otherYAxis->setMax(otherYAxis->Max() - correspondingOther);
	}

   plotBeingMoved = false;
}

void Plot::DrawSelectionRectangle(HDC hdc)
{
   if(!rectSelected)
      return;

   RectF r;
   int x0,y0,x1,y1;
   x0 = scrnSelectRect.left;
   y0 = scrnSelectRect.top;
   x1 = scrnSelectRect.right;
   y1 = scrnSelectRect.bottom;
   if(x1 < x0) Swap(x0,x1);
   if(y1 < y0) Swap(y0,y1);

   float xd0 = curXAxis()->scrnToData(x0); 
   float xd1 = curXAxis()->scrnToData(x1); 
   float yd0 = curYAxis()->scrnToData(y0); 
   float yd1 = curYAxis()->scrnToData(y1);

   // Get order right
   if(xd1 < xd0) Swap(xd0,xd1);
   if(yd1 < yd0) Swap(yd0,yd1);

// Record data coordinates of rectangle (for later zoom)
   selectRect.left   = xd0;
   selectRect.right  = xd1;
   selectRect.top    = yd0;
   selectRect.bottom = yd1;

// Draw the rectange
   Graphics gfx(hdc); 
   Rect zoomRect(x0,y0,x1-x0,y1-y0);
   COLORREF bkg = zoomBkgColor;
   COLORREF border = zoomBorderColor;
   SolidBrush zoomBrush(Color(GetAValue(bkg), GetRValue(bkg), GetGValue(bkg), GetBValue(bkg)));
   Pen zoomPen(Color(GetAValue(border),GetRValue(border),GetGValue(border),GetBValue(border)),1.0);
   gfx.FillRectangle(&zoomBrush, zoomRect);
   gfx.DrawRectangle(&zoomPen,zoomRect);
				
// Report region width and height to user
   float w = abs(xd1-xd0);   
   float h = abs(yd1-yd0); 
   char str[50];
   sprintf(str,"Width = %2.5g, Height = %2.5g",w,h);
   UpdateStatusWindow(this->plotParent->hWnd,0,str);
}

void Plot::ResetSelectionRectangle(void)
{
   scrnSelectRect.left   = 0;
   scrnSelectRect.right  = 0;
   scrnSelectRect.top    = 0;
   scrnSelectRect.bottom = 0;
   rectSelected = false;   
}

short Plot::ZoomRegion()
{
// Copy currently selected region to zoom history list
   zoomHistory.push_back(selectRect);

// Reset region rectangle

   return(OK);
}


void Plot::SetPlotColor(COLORREF col, short dest)
{
	switch(dest)
	{
	   case(ID_PREF_BG_COLOR):
	   case(ID_BK_COLOR):
		   this->bkColor = col;
		   break;
	   case(ID_PREF_AXES_COLOR):
	   case(ID_AXES_COLOR):
		   this->axesColor = col;
		   break; 
	   case(ID_PREF_AXES_FONT_COLOR):
		   for(Axis* axis : axisList_)
		   {
			   axis->ticks().setFontColor(col);
		   }
		   break; 
	   case(ID_PREF_TITLE_COLOR):
		   this->title().setFontColor(col);
		   break;
	   case(ID_PREF_LABEL_COLOR):
		   this->curXAxis()->label().setFontColor(col);
		   break;           
	   case(ID_PREF_BORDER_COLOR):
	   case(ID_BORDER_COLOR):
		   this->borderColor = col;
		   break;
	   case(ID_PREF_GRID_COLOR):
	   case(ID_GRID_COLOR):
		   for(Axis* axis: axisList_)
		   {
			   axis->grid()->setColor(col);
		   }
		   break;
	   case(ID_PREF_FINE_GRID_COLOR):         
	   case(ID_FINE_GRID_COLOR):
		   for(Axis* axis: axisList_)
		   {
			   axis->grid()->setFineColor(col);
		   }
		   break;        
	   }
}


void Plot::GetAxisParameter(short parameter, char *value)
{
	switch(parameter)
	{
	case(ID_XTICK_SPACING):
		sprintf(value,"%g", curXAxis()->ticks().spacing());
		break;
	case(ID_YTICK_SPACING):
		sprintf(value,"%g",curYAxis()->ticks().spacing());
		break;
	case(ID_X_TICKS_PER_LABEL):
		sprintf(value,"%g",curXAxis()->ticks().perLabel());
		break;
	case(ID_Y_TICKS_PER_LABEL):
		sprintf(value,"%g",curYAxis()->ticks().perLabel());
		break; 
	case(ID_TICK_SIZE):
		sprintf(value,"%g",curXAxis()->ticks().minorLength());
		break;
	case(ID_LABEL_SIZE):
		sprintf(value,"%g",curXAxis()->ticks().majorLength());
		break;
	case(ID_MINX):
		sprintf(value,"%g",this->curXAxis()->Min());
		break;
	case(ID_MINY):
		sprintf(value,"%g",this->curYAxis()->Min());
		break;
	case(ID_MAXX):
		sprintf(value,"%g",this->curXAxis()->Max());
		break;
	case(ID_MAXY):
		sprintf(value,"%g",this->curYAxis()->Max());
		break;
	case(ID_TITLE_TXT):
		strncpy_s(value,AXIS_PARAM_LENGTH,this->title().text(),_TRUNCATE);   
		break;
	case(ID_XLABEL_TXT):
		strncpy_s(value,AXIS_PARAM_LENGTH,this->curXAxis()->label().text(),_TRUNCATE);   
		break;
	case(ID_YLABEL_TXT):
		strncpy_s(value,AXIS_PARAM_LENGTH,this->curYAxis()->label().text(),_TRUNCATE);   
		break;
	}
}


char* Plot::GetColorStr(COLORREF col)
{
	static char colStr[20];

	sprintf(colStr,"[%d,%d,%d]",(int)GetRValue(col),(int)GetGValue(col),(int)GetBValue(col));

	return(colStr);
}

char* Plot::GetAlphaColorStr(COLORREF col)
{
	static char colStr[20];

	sprintf(colStr,"[%d,%d,%d,%d]",(int)GetRValue(col),(int)GetGValue(col),(int)GetBValue(col),(int)GetAValue(col));

	return(colStr);
}

void Plot::SetAxisParameter(short parameter, char *value)
{
	float num;

	// Check for labels

	switch(parameter)
	{
	case(ID_TITLE_TXT):
		title().setText(value);
		break;
	case(ID_XLABEL_TXT):
		curXAxis()->label().setText(value);
		break;
	case(ID_YLABEL_TXT):
		curYAxis()->label().setText(value);
		break;
	}

	// Check for numeric values

	sscanf(value,"%f",&num);

	switch(parameter)
	{
	case(ID_XTICK_SPACING):
		curXAxis()->ticks().setSpacing(num);
		break;
	case(ID_YTICK_SPACING):
		curYAxis()->ticks().setSpacing(num);
		break;
	case(ID_X_TICKS_PER_LABEL):
		curXAxis()->ticks().setPerLabel(num);
		break;
	case(ID_Y_TICKS_PER_LABEL):
		curYAxis()->ticks().setPerLabel(num);
		break; 
	case(ID_TICK_SIZE):
		curXAxis()->ticks().setMinorLength(num);
		curYAxis()->ticks().setMinorLength(num);
		break;
	case(ID_LABEL_SIZE):
		curXAxis()->ticks().setMajorLength(num);
		curYAxis()->ticks().setMajorLength(num);
		break; 
	case(ID_MINX):
		curXAxis()->setMin(num);
		break;
	case(ID_MINY):
		curYAxis()->setMin(num);
		break;
	case(ID_MAXX):
		curXAxis()->setMax(num);
		break;
	case(ID_MAXY):
		curYAxis()->setMax(num);
		break;  
	}
}

void Plot::FormatAxisLabelParameters(StringPairs& state)
{
	char buf[128] = {'\0'};
	sprintf(buf,"(%hd,%hd)", this->plotParent->obj->winParent->nr,this->plotParent->obj->nr());
	state.add("parent",buf);
	state.add("size",stringifyInt(this->curXAxis()->label().fontSize()).c_str());
	state.add("font",this->curXAxis()->label().font().lfFaceName);
	state.add("color",Plot::GetColorStr(this->curXAxis()->label().fontColor()));
	state.add("style",GetFontStyleStr(this->curXAxis()->label().fontStyle()));
}

const string Plot::FormatXLabelParameters()
{
	StringPairs state;
	FormatAxisLabelParameters(state);
	state.add("text",curXAxis()->label().text());
	return FormatStates(state);
}

const string Plot::FormatYLabelParameters()
{
	StringPairs state;
	FormatAxisLabelParameters(state);
	state.add("text",curYAxis()->label().text());
	return FormatStates(state);
}

const string Plot::FormatYLabelRightParameters()
{
	StringPairs state;
	FormatAxisLabelParameters(state);
   state.add("text",yAxisRight()->label().text());
	return FormatStates(state);
}

const string Plot::FormatYLabelLeftParameters()
{
	StringPairs state;
	FormatAxisLabelParameters(state);
	state.add("text",yAxisLeft()->label().text());
	return FormatStates(state);
}




/**********************************************************************************
*                 Writes title, x and y labels on plot border                     *
**********************************************************************************/

void Plot::WritePlotLabels(HDC hdc)
{
   SaveDC(hdc);
   
   if(plotParent->showLabels)
   {
		title().draw(hdc);
		curXAxis()->label().draw(hdc);
		yAxisL_->label().draw(hdc);
		if (getDimension() == 1)
			yAxisR_->label().draw(hdc);
	}
	else
	{
	// Write title ****************************

	   HFONT title_Font = GetFont(hdc,title().font(),0,0); 
	   SelectObject(hdc,title_Font);
	   if(strlen(title().text()) > 0)
	   {
			SIZE te;    
		   GetTextExtentPoint32(hdc, title().text(), strlen(title().text()), &te);
	      short xoff = dimensions.left() + dimensions.width()/2;
	      short yoff = dimensions.top()+ te.cy;
	      WriteText(hdc,xoff,yoff,WindowLayout::CENTRE_ALIGN,WindowLayout::TOP_ALIGN,WindowLayout::HORIZONTAL,title().font(),title().fontColor(),RGB(255,255,255),title().text(),title().rect());
	   } 

	   RestoreDC(hdc,-1);
	   DeleteObject(title_Font); 
	}
 }

/**********************************************************************************
*                         Initialize the plot parameters                          *
**********************************************************************************/

short Plot::DrawBoundingRectange(HDC hdc, HPEN axesColor)
{
   SaveDC(hdc);

	HPEN dkGrey  = CreatePen(PS_SOLID,0,RGB(100,100,100));
	HPEN ltGrey  = CreatePen(PS_SOLID,0,RGB(200,200,200));
//	HPEN borderPen  = CreatePen(PS_SOLID,0,borderColor);
//	HPEN axesPen  = CreatePen(PS_SOLID,0,RGB(0,0,0));
   
// Draw rectangle around plot **************************************

   long xoff   = dimensions.left()- 1;
   long width  = dimensions.width() + 1;
   long yoff   = dimensions.top() - 1;
   long height = dimensions.height() + 1;

// Enlarge rectangle if this is the current plot *******************


	bool cPlot = (getDimension() == 1 && this == Plot1D::curPlot()) ||
		          (getDimension() == 2 && this == Plot2D::curPlot());
      
   if(gPlotMode == DISPLAY && cPlot)
   {
      SelectObject(hdc,ltGrey);   
      MoveToEx(hdc,xoff-1,yoff+height,0);
      LineTo(hdc,xoff+width+1,yoff+height);
      MoveToEx(hdc,xoff-1,yoff+height+1,0);
      LineTo(hdc,xoff+width+1,yoff+height+1);
      MoveToEx(hdc,xoff+width,yoff+height+1,0);
      LineTo(hdc,xoff+width,yoff-1);
      MoveToEx(hdc,xoff+width+1,yoff+height+1,0);
      LineTo(hdc,xoff+width+1,yoff-1);
      
      SelectObject(hdc,dkGrey);   
      MoveToEx(hdc,xoff-1,yoff-1,0);
      LineTo(hdc,xoff+width+2,yoff-1);
      MoveToEx(hdc,xoff-1,yoff,0);
      LineTo(hdc,xoff+width+1,yoff);
      MoveToEx(hdc,xoff-1,yoff,0);
      LineTo(hdc,xoff-1,yoff+height+2);
      MoveToEx(hdc,xoff,yoff,0);
      LineTo(hdc,xoff,yoff+height+1);     
   }  
   else
   {
      SelectObject(hdc,axesColor);   
	   DrawRect(hdc,xoff,yoff,xoff+width,yoff+height);
   }       

   RestoreDC(hdc,-1);
   DeleteObject(dkGrey);
   DeleteObject(ltGrey);
   
   return(OK);  
}

void Plot::setAntiAliasing(bool aa)
{
	this->antiAliasing = aa;
}

bool Plot::isAntiAliasing(void)
{
	return false;
}


/*****************************************************************************************
*                           Set up the clipping region for plotting                      *
*****************************************************************************************/

void Plot::SetClippingRegion(HDC hdc, long off)
{
   HRGN hClpRgn;

	hClpRgn = CreateRectRgn(dimensions.left()-off,dimensions.top()-off-1,
	                        dimensions.left()+dimensions.width()+off+1,
	                        dimensions.top()+dimensions.height()+off+1);
 	SelectClipRgn(hdc, hClpRgn);
 	DeleteObject(hClpRgn);  
}

void Plot::SetMappingMode(HDC hdc)
{

// Set mapping mode - don't scale since this can cause rounding errors
// However do flip y so have a normal coordinate system

   SetMapMode(hdc,MM_ISOTROPIC);        // Allow different x and y scaling
   SetWindowExtEx(hdc,dimensions.width(),dimensions.height(),NULL);         // Data range
   SetViewportExtEx(hdc,dimensions.width(),dimensions.height(),NULL);      // Window or page range in pixels
   SetViewportOrgEx(hdc, 0, 0, NULL); // Origin in window or page
   SetBkMode(hdc,TRANSPARENT);
}


short Plot::CursorOnAxes(short x, short y)
{
   short xl,xr,yt,yb;
   
   if(axesMode == PLOT_AXES_CROSS)
   {
      xl = dimensions.left();
      xr = xl + dimensions.width();
      yt = curYAxis()->dataToScrn(0)-10;
      yb = yt+20;
      
	   if(x > xl && x < xr && y > yt && y < yb) // x axis
	      return(OK);  
	      
      xl = curXAxis()->dataToScrn(0)-10;
      xr = xl + 20;
      yt = dimensions.top();
      yb = yt + dimensions.height();
      
	   if(x > xl && x < xr && y > yt && y < yb) // y axis
	      return(OK);	          
   }
   else
   {
	   xl = dimensions.left();
	   xr = xl + dimensions.width();
	   yt = dimensions.top();
	   yb = yt + dimensions.height();
	   
		// Is it the left Y axis?
	   if(x > xl-10 && x < xl + 10 && y < yb+10 && y > yt-10)
	      return(OK);

		// Is it the right Y axis?

		if(x > xr-10 && x < xr + 10 && y < yb+10 && y > yt-10)
	      return(OK);

		// Is it the X axis?
	      
	   if(y < yb+10 && y > yb-10 && x > xl-10 && x < xr-10)
	      return(OK);
   }
      
   return(ABORT);
}

short Plot::CursorInBackGround(short x, short y)
{
   short xl,xr,yt,yb;
   
   xl = dimensions.left();
   xr = xl + dimensions.width();
   yt = dimensions.top();
   yb = yt + dimensions.height();
   
   if(x > xl && x < xr && y > yt && y < yb)
      return(OK);

      
   return(ABORT);
}

short Plot::CursorInText(short x, short y)
{
   if(title().rect().PointInRect(x,y))
      return(TITLE_TEXT);
   else if(curXAxis()->label().rect().PointInRect(x,y))
      return(X_LABEL);
	else if(yAxisL_->label().rect().PointInRect(x,y))
		return Y_LABEL_L;
   else if(yAxisR_->label().rect().PointInRect(x,y))
      return Y_LABEL_R;

	else if(curXAxis()->rect().PointInRect(x,y))
      return(X_AXES_TEXT);
	else if(yAxisL_->rect().PointInRect(x,y))
		return(Y_AXES_TEXT_L);
	else if(yAxisR_->rect().PointInRect(x,y))
      return(Y_AXES_TEXT_R);      
   return(0);  
}

bool Plot::InPlotRect(long x, long y)
{
   if(x >= dimensions.left() && x <= dimensions.left()+dimensions.width() &&
      y >= dimensions.top() && y <= dimensions.top()+dimensions.height())
      return(true);
   else
      return(false);
}

/****************************************************************************
*  Wait until the mouse has been clicked somewhere in the plot and return   *
*  the coordinates in screen coordinates.                                   *
****************************************************************************/

// Draw a cross from (x1,y1) to (x2,y2) centred on (x0,y0)
void DrawCross(HDC hdc, long x0, long y0, long x1, long y1, long x2, long y2)
{
   MoveToEx(hdc,x1,y0,0);
   LineTo(hdc,x2,y0);
   MoveToEx(hdc,x0,y1,0);
   LineTo(hdc,x0,y2);
}


long Plot::GetLeft()
{
   return(dimensions.left());
}

long Plot::GetRight()
{
   return(dimensions.left() + dimensions.width());
}

long Plot::GetWidth()
{
	return(dimensions.width());
}

long Plot::GetHeight(void)
{
	return(dimensions.height());
}

long Plot::GetBottom()
{
   return(dimensions.top() + dimensions.height());
}

long Plot::GetTop()
{
   return(dimensions.top());
}



void Plot::SetLeft(long value)
{
   dimensions.SetLeft(value);
	notifyObservers();
}

void Plot::SetTop(long value)
{
   dimensions.SetTop(value);
	notifyObservers();
}

void Plot::SetWidth(long value)
{
   dimensions.SetWidth(value);
	notifyObservers();
}

void Plot::SetHeight(long value)
{
   dimensions.SetHeight(value);
	notifyObservers();
}

bool Plot::InRegion(short x, short y)
{
   if(PointVisible((long)regionLeft, (long)regionTop, (long)regionRight, (long)regionBottom, (long)x, (long)y))
      return(true);
   else
      return(false);
}
       
void Plot::ResetTextCoords()
{
   title().rect().reset();
	for_each(axisList_.begin(), axisList_.end(), mem_fun(&Axis::resetTextCoords));
}

/*****************************************************************************************
*                   Make sure a new data set will be visible when loaded                 *
*****************************************************************************************/

void Plot::ResetPlotParameters()
{
	for(Axis* axis: axisList())
	{
		axis->setAutorange(true);
	}
	gridLocked_ = 0;
}


void Plot::setPlotState(char* state)
{
	strncpy_s(plotStr,MAX_STR, state, _TRUNCATE);
}

char* Plot::getPlotState()
{
	return plotStr;
}

void Plot::setHold(bool hold)
{
	displayHold = hold;
}

bool Plot::getHold()
{
	return displayHold;
}
	
void Plot::clearLabels()
{
	title().clearText();
	for(Axis* axis: axisList_)
	{
		axis->label().clearText();
	}
}

void Plot::setFileName(char* fileName)
{
	strncpy_s(this->fileName, PLOT_STR_LEN, fileName, _TRUNCATE);
}

char* Plot::getFileName()
{
	return fileName;
}

void Plot::setFilePath(char* filePath)
{
	strncpy_s(this->filePath, PLOT_STR_LEN, filePath, _TRUNCATE);
}

char* Plot::getFilePath()
{
	return filePath;
}

long Plot::getFileVersion()
{
   switch(fileVersion)
   {
      case(PLOTFILE_VERSION_1_0):
         return(100);
      case(PLOTFILE_VERSION_1_1):
         return(110);
      case(PLOTFILE_VERSION_1_2):
         return(120);
      case(PLOTFILE_VERSION_1_4):
         return(140);
      case(PLOTFILE_VERSION_1_6):
         return(160);
      case(PLOTFILE_VERSION_2_0):
         return(200);
      case(PLOTFILE_VERSION_2_1):
         return(210);
      case(PLOTFILE_VERSION_2_2):
         return(220);
      case(PLOTFILE_VERSION_3_0):
         return(300);
      case(PLOTFILE_VERSION_3_1):
         return(310);
      case(PLOTFILE_VERSION_3_2):
         return(320);
      case(PLOTFILE_VERSION_3_3):
         return(330);
      case(PLOTFILE_VERSION_3_4):
         return(340);
      case(PLOTFILE_VERSION_3_5):
         return(350);
      case(PLOTFILE_VERSION_3_6):
         return(360);
      case(PLOTFILE_VERSION_3_7):
         return(370);
      case(PLOTFILE_VERSION_3_8):
         return(380);
      case(PLOTFILE_VERSION_3_9):
         return(390);
      case(PLOTFILE_VERSION_4_0):
         return(400);
      case(PLOTFILE_VERSION_4_1):
         return(410);
      case(PLOTFILE_VERSION_4_2):
         return(420);
      case(PLOTFILE_VERSION_4_3):
         return(430);
      case(PLOTFILE_VERSION_4_4):
         return(440);
      case(PLOTFILE_VERSION_4_5):
         return(450);
      case(PLOTFILE_VERSION_4_6):
         return(460);
      case(PLOTFILE_VERSION_4_7):
         return(470);
      case(PLOTFILE_VERSION_4_8):
         return(480);
      case(PLOTFILE_VERSION_4_9):
         return(490);
      case(PLOTFILE_VERSION_5_0):
         return(500);

   }
   return(0);
}


void Plot::setFileVersion(long version)
{
	fileVersion = version;
}

void Plot::setTitle(PlotLabel* title)
{
	if (title_)
	{
		delete title_;
	}
	title_ = title;
	((TitleLabel*)(title))->setParent(this);
}

void Plot::setTitleText(char* title)
{
	title_->setText(title);
}

void Plot::setColors(COLORREF& axes, COLORREF& bkgd, COLORREF& plot, COLORREF& border)
{
	this->axesColor = axes;
	this->bkColor = bkgd;
	this->plotColor = plot;
	this->borderColor = border;
}


// Resize plot making sure there is sufficient space around each region
void Plot::ResizePlotRegion(HWND hWnd)
{
	RECT r;
	short winWidth,winHeight;
	short th=0,sh=0;

	// Get size of screen plot window 
	GetClientRect(hWnd,&r);
	if(gPlotMode == DISPLAY || gPlotMode == CLIPBOARD)
	{
		winHeight = r.bottom-r.top-th-sh+1;
		winWidth  = r.right-r.left+1;
	}
	else if(gPlotMode == IMAGEFILE)
	{
		winHeight = r.bottom-r.top-th-sh+1;
		winWidth  = r.right-r.left+1;
		th = 0;
	} 
	else
	{
		winHeight = r.bottom-r.top+1;
		winWidth  = r.right-r.left+1;
		th = 0;
	} 

	// Divide up plot window into rows*cols regions
	// These define the limits of the plot region (incl. labels)
	regionLeft   = (colNr*winWidth/plotParent->cols)*x_scaling_factor();
	regionRight  = ((colNr+1)*winWidth/plotParent->cols)*x_scaling_factor();
	regionTop    = (th + rowNr*winHeight/plotParent->rows)*y_scaling_factor();
	regionBottom = (th + (rowNr+1)*winHeight/plotParent->rows)*y_scaling_factor();

	if(plotParent->showLabels)
	{
		short ww = regionRight-regionLeft;
		short wh = regionBottom-regionTop;

		long newWidth = displayColorScale() ?
			ww - 0.10*ww - (margins.totalHorizontalMargins()) :
		ww - (margins.totalHorizontalMargins());

		PlotDimensions dim(regionTop + margins.top(),
			regionLeft + margins.left(),
			newWidth,
			wh - (margins.totalVerticalMargins()), 0,0, x_scaling_factor(), y_scaling_factor());
	//		wh - (margins.totalVerticalMargins()), -1, -1);

		setDimensions(dim);
	}
	else
	{
		PlotDimensions dim(regionTop,
			regionLeft,
			regionRight-regionLeft,
			regionBottom-regionTop, -1, -1);
		setDimensions(dim);
	}
	for(Inset* inset: insets_)
	{
		inset->layoutHasChanged();
	}
}

void Plot::resetDataView()
{
}

void Plot::resetZoomCount()
{
	zoomHistory.clear();
}

void Plot::updateStatusWindowForZoom(HWND hWnd)
{
	if (!zoomHistory.empty())
	{
		AddCharToString(statusText, 'Z');
	}
	else
	{
		RemoveCharFromString(statusText,'Z');
	}
	UpdateStatusWindow(hWnd,3,statusText);
}
	
void Plot::setXTicks(Ticks& ticks) 
{
	curXAxis()->setTicks(ticks);
}

void Plot::setYTicks(Ticks& ticks)
{
	curYAxis()->setTicks(ticks);
}

Ticks& Plot::getXTicks()
{
	return curXAxis()->ticks();
}

Ticks& Plot::getYTicks()
{
	return curYAxis()->ticks();
}

Axis* Plot::otherYAxis(Axis* axis)
{
	if (axis == 0)
	{
		axis = curYAxis();
	}
	if (axis == yAxisR_)
		return yAxisL_;
	return yAxisR_;
}

short Plot::setAxisMapping(short mapping)
{
	switch(mapping)
	{
	case PLOT_LINEAR_X:
	case PLOT_LOG_X:
		curXAxis()->setMapping(mapping);
		break;
	case PLOT_LINEAR_Y:
	case PLOT_LOG_Y:
		curYAxis()->setMapping(mapping);
		otherYAxis()->setMapping(mapping);
		break;
	}
	return OK;
}

void Plot::setCurYAxis(Axis* axis)
{
	curYAxis_ = axis; 
}

void Plot::setCurYAxis(VerticalAxisSide side)
{
	setCurYAxis((side == LEFT_VAXIS_SIDE) ? yAxisL_ : yAxisR_);
}

void Plot::scale_(double x, double y)
{
	regionLeft = regionLeft * x + 0.5;
	regionBottom =  regionBottom * y + 0.5;
	regionTop = regionTop * y + 0.5;
	regionRight = regionRight * x + 0.5;

	for(Axis* axis: axisList())
	{
		axis->scale(x,y);
	}

	dimensions.scale(x,y);
	margins.scale(x,y);
	title().scale(x,y);
	for(Inset* inset: insets_)
	{
		inset->scale(x,y);
	}
}

void Plot::unscale_()
{	
	regionLeft = x_inverse_scaling_factor() * regionLeft + 0.5;
	regionBottom = y_inverse_scaling_factor() * regionBottom + 0.5;
	regionTop = y_inverse_scaling_factor() * regionTop + 0.5;
	regionRight = x_inverse_scaling_factor() * regionRight + 0.5;

	for(Axis* axis: axisList())
	{
		axis->unscale();
	}

	dimensions.unscale();
	margins.unscale();
	title().unscale();
	for(Inset* inset: insets_)
	{
		inset->unscale();
		inset->layoutHasChanged();
	}
}


short Plot::mapping(char dir)
{
	if (dir == 'x')
		return curXAxis_->mapping();
	return curYAxis_->mapping();
}

void Plot::InitialisePositions(HWND hWnd)
{
// Initialize position of text objects ************
	ResetTextCoords();
// Setup plot window dimensions *******************
	ResizePlotRegion(hWnd);
}

Inset* Plot::insetBeingMoved()
{	
	for(Inset* inset: insets_)
	{
		if (inset->beingMoved())
		{
			return inset;
		}
	}
	return NULL;
}

void Plot::beginToDragMovePlot(HWND hWnd, short x, short y)
{
	HDC hdc = GetDC(hWnd);               
	HideSelectionRectangle(hdc);  
	ReleaseDC(hWnd,hdc);
	for(Axis* axis: axisList_)
	{
		axis->setAutorange(false);
	}
	// See if we should be moving one of the insets, rather than the
	// plot data.
	bool choseOneToMoveAlready = false;
	for(Inset* inset: insets_)
	{
		if (choseOneToMoveAlready)
		{
			inset->beingMoved(false);
		}
		else
		{
			if (!inset->visible())
			{
				continue;
			}
			inset->beingMoved(inset->position().PointInRect(x,y));
			choseOneToMoveAlready = inset->beingMoved();
		}
	}
}

void Plot::stopShifting()
{
	for(Inset* inset: insets_)
	{
		inset->beingMoved(false);
	}
}

string* Plot::describeInsets()
{
	const int MAX_DESCRIPTION_LENGTH = 20;
	char buffer[8192] = {'\0'};
	
	// Grab all the descriptions.
	int index = 0;
	vector<InsetDescription*> descriptions;
	for(Inset* inset: insets_)
	{
		InsetDescription* d = inset->description(index);
		if (d)
		{
			descriptions.push_back(d);
		}
		index++;
	}
	// If there are none, reply that there are none
	if (descriptions.empty())
	{
		sprintf(buffer, "No insets defined.");
	}
	else
	{
		// Otherwise, format all the descriptions
		sprintf(buffer,"\n   INDEX, CONTENTS, POSITION, VISIBILITY\n   -----------------------------------\n");
		for(InsetDescription* d: descriptions)
		{
			char current[1024];
			char truncated_contents[MAX_DESCRIPTION_LENGTH+1];
			strncpy(truncated_contents, d->contents().c_str(), 21);
			truncated_contents[MAX_DESCRIPTION_LENGTH] = 0;			
			sprintf(current, "   %d, %s, %s, %s\n", d->index(), truncated_contents, d->position().c_str(), d->visibility()? "true":"false");
			strcat(buffer, current);
			delete(d);
		}
	}
	return new string(buffer);
}

int Plot::removeInset(int index)
{
	if (index >= insets_.size())
	{
		return ERR;
	}
	deque<Inset*>::iterator it = insets_.begin();
	for (int i = 0; i < index; i++)
	{
		++it;
	}
	Inset* removeme  = *it;
	insets_.erase(it,it + 1);
	delete(removeme);
	return OK;
}

Inset* Plot::findInsetByID(int n)
{
	if (n >= insets_.size())
	{
		return NULL;
	}
	return insets_[n];
}


void Plot::setXAxis(Axis* axis)
{
	// // Find this axis in the list of axes.
	Axis* victim = 0;
	for(Axis* a: axisList_)
	{
		if (a->side() == NO_VAXIS_SIDE)
		{
			victim = a;
			break;
		}
	}
	// If victim is curXAxis, then the new guy becomes curXAxis.
	if (victim)
	{
		if (victim == curXAxis_)
		{
			setCurXAxis(axis);
		}
		// Remove the victim from axisList_.
		AxisList::iterator it;
		for(it = axisList_.begin(); it != axisList_.end(); ++it)
		{
			if (*it == victim)
			{
				axisList_.erase(it);
				break;
			}
		}
	}
	// add axis to axisList_.
	this->xAxis_ = axis;
	axisList_.push_back(axis);
}

void Plot1D::setXAxis(Axis* axis)
{
	Axis* victim = 0;
	for(Axis* a: axisList_)
	{
		if (a->side() == NO_VAXIS_SIDE)
		{
			victim = a;
		}
	}
	Plot::setXAxis(axis);
	// // Find this axis in the list of axes.
	// update trace references to the x axis
	if (victim)
	{
		for(Trace* t: traceList)
		{
			if (t->xAxis() == victim)
			{
				t->setXAxis(axis);
			}
		}
		delete victim;
	}
}

void Plot::setYAxisL(Axis* axis)
{
	// // Find this axis in the list of axes.
	Axis* victim = 0;
	for(Axis* a: axisList_)
	{
		if (a->side() == LEFT_VAXIS_SIDE)
		{
			victim = a;
			break;
		}
	}
	// If victim is curYAxis, then the new guy becomes curYAxis.
	if (victim)
	{
		if (victim == curYAxis_)
		{
			setCurYAxis(axis);
		}
		// Remove the victim from axisList_.
		AxisList::iterator it;
		for(it = axisList_.begin(); it != axisList_.end(); ++it)
		{
			if (*it == victim)
			{
				axisList_.erase(it);
				break;
			}
		}
	}
	// add axis to axisList_.
	this->yAxisL_ = axis;
	axisList_.push_back(axis);
}

void Plot1D::setYAxisL(Axis* axis)
{
	Axis* victim = 0;
	for(Axis* a: axisList_)
	{
		if (a->side() == LEFT_VAXIS_SIDE)
		{
			victim = a;
		}
	}
	Plot::setYAxisL(axis);
	if (victim)
	{
		// // Find this axis in the list of axes.
		// update trace references to the Y axis
		for(Trace* t: traceList)
		{
			if (t->yAxis() == victim)
			{
				t->setYAxis(axis);
			}
		}
		delete victim;
	}
}

void Plot::setYAxisR(Axis* axis)
{
	// // Find this axis in the list of axes.
	Axis* victim = 0;
	for(Axis* a: axisList_)
	{
		if (a->side() == RIGHT_VAXIS_SIDE)
		{
			victim = a;
			break;
		}
	}
	// If victim is curYAxis, then the new guy becomes curYAxis.
	if (victim)
	{
		if (victim == curYAxis_)
		{
			setCurYAxis(axis);
		}

		// Remove the victim from axisList_.
		AxisList::iterator it;
		for(it = axisList_.begin(); it != axisList_.end(); ++it)
		{
			if (*it == victim)
			{
				axisList_.erase(it);
				break;
			}
		}
	}
	// add axis to axisList_.
	this->yAxisR_ = axis;
	axisList_.push_back(axis);
}

void Plot1D::setYAxisR(Axis* axis)
{
	Axis* victim = 0;
	for(Axis* a: axisList_)
	{
		if (a->side() == RIGHT_VAXIS_SIDE)
		{
			victim = a;
		}
	}
	Plot::setYAxisR(axis);
	if (victim)
	{
		// // Find this axis in the list of axes.
		// update trace references to the x axis
		for(Trace* t: traceList)
		{
			if (t->yAxis() == victim)	
			{
				t->setYAxis(axis);
			}
		}
		delete victim;
	}
}

int Plot::addInset(Inset* inset)
{
	// Do not add this inset if it is already there.
	int index;
	if (-1 != (index = indexOfInset(inset)))
	{
		return index;
	}
	insets_.push_back(inset);
	inset->setParent(this);
	return insets_.size();
}

int Plot::indexOfInset(const Inset* const inset) const
{
	int position = 0;
	for(Inset* inset_: insets_)
	{
		if (inset_ == inset)
		{
			return position;
		}
		position++;
	}
	return -1;
}


void Plot::setDefaultParameters()
{
	// Get the plot fonts
	ProspaFont titleFont(pwd->titleFontName,pwd->titleFontColor,pwd->titleFontSize,pwd->titleFontStyle);
	ProspaFont labelFont(pwd->labelFontName,pwd->labelFontColor,pwd->labelFontSize,pwd->labelFontStyle);
	ProspaFont ticksFont(pwd->axesFontName,pwd->ticks->fontColor(), pwd->ticks->fontSize(), pwd->ticks->fontStyle());

	SetPlotColor(pwd->borderColor,ID_BORDER_COLOR);
	SetPlotColor(pwd->bkColor,ID_BK_COLOR);
	SetPlotColor(pwd->axesColor,ID_PREF_AXES_COLOR);
	SetPlotColor(pwd->gridColor,ID_PREF_GRID_COLOR);
	SetPlotColor(pwd->fineGridColor,ID_PREF_FINE_GRID_COLOR);
   zoomBkgColor = pwd->zoomBkgColor;
   zoomBorderColor = pwd->zoomBorderColor;
   zoomRectMode = pwd->zoomRectMode;

	title().setFont(titleFont);
	for(Axis* axis: axisList())
	{
		axis->label().setFont(labelFont);
		axis->ticks().setFont(ticksFont);
	}
}

Margins::Margins()
: Scalable()
{
	rightMargin = leftMargin = topMargin = baseMargin = 0;
}

Margins::Margins(long left, long right, long top, long base)
: Scalable()
{
	leftMargin = left;
	rightMargin = right;
	topMargin = top;
	baseMargin = base;
}

Margins::Margins(const Margins& copyMe)
: Scalable(copyMe)
{
	this->rightMargin = copyMe.rightMargin;
	this->leftMargin = copyMe.leftMargin;
	this->topMargin = copyMe.topMargin;
	this->baseMargin = copyMe.baseMargin;
}

Margins::~Margins()
{
}


void Margins::scale_(double x, double y)
{
	leftMargin = leftMargin * x + 0.5;
	rightMargin = rightMargin * x + 0.5;
	topMargin = topMargin * y + 0.5;
	baseMargin = baseMargin * y + 0.5;
}

void Margins::unscale_()
{
	leftMargin = x_inverse_scaling_factor() * leftMargin + 0.5;
	rightMargin = x_inverse_scaling_factor() * rightMargin + 0.5;
	topMargin = y_inverse_scaling_factor() * topMargin + 0.5;
	baseMargin = y_inverse_scaling_factor() * baseMargin + 0.5;
}

#include "stdafx.h"
#include "plot.h"
#include "allocate.h"
#include "bitmap.h"
#include "trace.h"
#include "defineWindows.h"
#include "globals.h"
#include "guiModifyObjectParameters.h"
#include "guiWindowClass.h"
#include "guiWindowCLI.h"
#include "guiWindowsEvents.h"
#include "Inset.h"
#include "interface.h"
#include "plotLine.h"
#include "load_save_data.h"
#include "message.h"
#include "metafile.h"
#include "main.h"
#include <math.h>
#include "mymath.h"
#include "plot1dCLI.h"
#include "PlotFile.h"
#include "plotText.h"
#include "process.h"
#include "PlotGrid.h"
#include "plotwindefaults.h"
#include "PlotWindow.h"
#include "print.h"
#include "prospaResource.h"
#include "rgbColors.h"
#include "string_utilities.h"
#include "utilities.h"
#include "variablesOther.h"
#include "winEnableClass.h"
#include <deque>
#include <algorithm>
#include "memoryLeak.h"

#define TRACE_INDICATOR_SIZE 10

using std::deque;

Plot1D* Plot1D::cur1DPlot = 0;
bool Plot1D::dataCursorVisible_ = false;

char Plot1D::currFileName[PLOT_STR_LEN] = {'\0'};
char Plot1D::currFilePath[PLOT_STR_LEN] = {'\0'};

long plot1DSaveVersion = PLOTFILE_CURRENT_VERSION_1D;

Plot1D::Plot1D(PlotWinDefaults *pd, HWND hWnd, PlotWindow *pp, short row, short col)
: Plot(pd,hWnd,pp,row,col)
{
   tracePar = pd->tracePar;
   display1DComplex = SHOW_REAL + SHOW_IMAGINARY;
   xOffset_ = 0;
   xOffsetIndex_ = 0;
   isOffset_ = false;
	xold = yold = yoldr = yoldi = 0;
	symbolScale_  = 1.0;

	xAxis_ = Axis::makeAxis(HORIZONTAL_AXIS, PLOT_LINEAR_X, this);
	yAxisL_ = Axis::makeAxis(VERTICAL_AXIS, PLOT_LINEAR_Y, this);
	yAxisR_ = Axis::makeAxis(VERTICAL_AXIS, PLOT_LINEAR_Y, this, RIGHT_VAXIS_SIDE);

	curXAxis_ = xAxis_;
	curYAxis_ = yAxisL_;

	axisList().push_back(xAxis_);
	axisList().push_back(yAxisL_);
	axisList().push_back(yAxisR_);

   dataCursor.x = -1;
   dataCursor.y = -1;
   pointSelection.xScrn = -1;
   pointSelection.yScrn = -1;

   setNoCurTrace();
	legend_ = 0;
	setLegend(new PlotLegend());
	limitfunc = false;

	indicatorSize = TRACE_INDICATOR_SIZE;
	isFiltered = true;

}


Plot1D::Plot1D(PlotWinDefaults *pd, HWND hWnd, PlotWindow *pp, short row, short col, Margins &margins)
: Plot(pd,hWnd,pp,row,col,margins)
{
   tracePar = pd->tracePar;
   display1DComplex = SHOW_REAL + SHOW_IMAGINARY;
   xOffset_ = 0;
   xOffsetIndex_ = 0;
   isOffset_ = false;
	xold = yold = yoldr = yoldi = 0;
	symbolScale_  = 1.0;

	xAxis_ = Axis::makeAxis(HORIZONTAL_AXIS, PLOT_LINEAR_X, this);
	yAxisL_ = Axis::makeAxis(VERTICAL_AXIS, PLOT_LINEAR_Y, this);
	yAxisR_ = Axis::makeAxis(VERTICAL_AXIS, PLOT_LINEAR_Y, this, RIGHT_VAXIS_SIDE);

	curXAxis_ = xAxis_;
	curYAxis_ = yAxisL_;

	axisList().push_back(xAxis_);
	axisList().push_back(yAxisL_);
	axisList().push_back(yAxisR_);

   dataCursor.x = -1;
   dataCursor.y = -1;
   pointSelection.xScrn = -1;
   pointSelection.yScrn = -1;

   setNoCurTrace();
	legend_ = 0;
	setLegend(new PlotLegend());
   limitfunc = false;
	
	showLines = true;
	showText = true;
	showInsets = true;

	indicatorSize = TRACE_INDICATOR_SIZE;
	isFiltered = true;

}

Plot1D::Plot1D(PlotWinDefaults *pd)
: Plot(pd, 0,0,0,0)
{
   tracePar = pd->tracePar;
   display1DComplex = SHOW_REAL + SHOW_IMAGINARY;
   xOffset_ = 0;
   xOffsetIndex_ = 0;
   isOffset_ = false;

	xold = yold = yoldr = yoldi = 0;
	symbolScale_  = 1.0;

   dataCursor.x = -1;
   dataCursor.y = -1;
   pointSelection.xScrn = -1;
   pointSelection.yScrn = -1;

	xAxis_ = Axis::makeAxis(HORIZONTAL_AXIS, PLOT_LINEAR_X, this);
	yAxisL_ = Axis::makeAxis(VERTICAL_AXIS, PLOT_LINEAR_Y, this);
	yAxisR_ = Axis::makeAxis(VERTICAL_AXIS, PLOT_LINEAR_Y, this, RIGHT_VAXIS_SIDE);

	axisList_.push_back(xAxis_);
	axisList_.push_back(yAxisL_);
	axisList_.push_back(yAxisR_);
	curXAxis_ = 0;
	curYAxis_ = 0;
	setNoCurTrace();
	legend_ = 0;
   limitfunc = false;
	
	showLines = true;
	showText = true;
	showInsets = true;

	indicatorSize = TRACE_INDICATOR_SIZE;
	isFiltered = true;

}


Plot1D::Plot1D(PlotWinDefaults *pd, Margins &margins)
: Plot(pd, 0,0,0,0,margins)
{
   tracePar = pd->tracePar;
   display1DComplex = SHOW_REAL + SHOW_IMAGINARY;
   xOffset_ = 0;
   xOffsetIndex_ = 0;
   isOffset_ = false;

	xold = yold = yoldr = yoldi = 0;
	symbolScale_  = 1.0;

	xAxis_ = Axis::makeAxis(HORIZONTAL_AXIS, PLOT_LINEAR_X, this);
	yAxisL_ = Axis::makeAxis(VERTICAL_AXIS, PLOT_LINEAR_Y, this);
	yAxisR_ = Axis::makeAxis(VERTICAL_AXIS, PLOT_LINEAR_Y, this, RIGHT_VAXIS_SIDE);

   dataCursor.x = -1;
   dataCursor.y = -1;
   pointSelection.xScrn = -1;
   pointSelection.yScrn = -1;

	axisList_.push_back(xAxis_);
	axisList_.push_back(yAxisL_);
	axisList_.push_back(yAxisR_);
	curXAxis_ = 0;
	curYAxis_ = 0;
	setNoCurTrace();
	legend_ = 0;
   limitfunc = false;
		
	showLines = true;
	showText = true;
	showInsets = true;

	indicatorSize = TRACE_INDICATOR_SIZE;
	isFiltered = true;

}

// Copy the contents of plot 'c' into 'this'

Plot1D::Plot1D(const Plot1D& c)
: Plot(c)
{
	tracePar = c.tracePar;
	display1DComplex = c.display1DComplex;  // Display real + imaginary
	xOffset_ = c.xOffset_; // 1D X offset
	xOffsetIndex_ = c.xOffsetIndex_; // 1D X offset as an index
   isOffset_ = c.isOffset_;

	xold = c.xold;
	yold = c.yold;
	yoldr = c.yoldr;
	yoldi = c.yoldi;

   dataCursor.x = c.dataCursor.x;
   dataCursor.y = c.dataCursor.y;
   pointSelection.xScrn = c.pointSelection.xScrn;
   pointSelection.yScrn = c.pointSelection.yScrn;

	symbolScale_ = c.symbolScale_;

	Axis* origLeftYAxis = c.yAxisL_;

	TraceListConstIterator it;
	for(Trace* t : c.traceList)
	{
		Trace* temp = t->clone();	
		temp->setParent(this);
		temp->setXAxis(this->curXAxis());
		if (t->yAxis() == origLeftYAxis)
		{
			temp->setYAxis(yAxisL_);
			yAxisL_->addTrace(temp);
		}
		else
		{
			temp->setYAxis(yAxisR_);
			yAxisR_->addTrace(temp);
		}
		xAxis_->addTrace(temp);
		this->traceList.push_back(temp);
	}

	legend_ = 0;
	setLegend(new PlotLegend());
   setCurTrace();
   limitfunc = false;
		
	showLines = c.showLines;
	showText = c.showText;
	showInsets = c.showInsets;
	zoomRectMode = c.zoomRectMode;
	zoomBkgColor = c.zoomBkgColor;
	zoomBorderColor = c.zoomBorderColor;

	indicatorSize = c.indicatorSize;

	isFiltered = c.isFiltered;

}

Plot1D* Plot1D::clone() const
{
	return new Plot1D(*this);
}

Plot1D::~Plot1D()
{
	clearData();
   overRideAutoRange = false;
}

short Plot1D::getDimension()
{
	return 1;
}

/*****************************************************************************************
*                      Check to see if plot data is present                              *
*****************************************************************************************/
       
bool Plot1D::DataPresent() const
{
	if(traceList.empty())
		return false;
	return true;
}

bool Plot1D::isAntiAliasing(void)
{
	return this->antiAliasing;
}

/****************************************************
Set this plot as the current 1D plot.
****************************************************/
void Plot1D::makeCurrentDimensionalPlot()
{
	cur1DPlot = this;
}

Plot1D* Plot1D::curPlot()
{
	return cur1DPlot;
}

void Plot1D::setNoCurPlot()
{
	cur1DPlot = 0;
}

void Plot1D::DrawVerticalSelectionLine(HDC hdc)
{
	CText temp;

	if(pointSelection.xScrn == -1)
		return;

	SaveDC(hdc);

	COLORREF penCol = curTrace()->tracePar.getRealColor();
	short penWidth = curTrace()->tracePar.getTraceWidth();

	HPEN pen = CreatePen(PS_DOT, 0, penCol);
   SelectObject(hdc,pen);

	MoveToEx(hdc,pointSelection.xScrn,GetTop(),0);
	LineTo(hdc,pointSelection.xScrn,GetTop()+GetHeight());

	DeleteObject(pen);
   RestoreDC(hdc, -1);

	pointSelection.xIndex = curTrace()->xAxis()->scrnToIndex(pointSelection.xScrn);
	pointSelection.xData = curTrace()->xAxis()->scrnToData(pointSelection.xScrn);

	temp.Format("(x) = (%g)",pointSelection.xData);
	UpdateStatusWindow(win, 0, temp.Str());
}


void Plot1D::DrawDataCrossHairs(HDC hdc)
{
   char str[100];
   bool shiftDown = false;

   if(dataCursor.x == -1 && dataCursor.y == -1)
      return;

	if(isOffset_)
	{
		if(curTrace()) 
		{
			long closestIndex = currentTrace->indexOfClosestPoint(xOffsetScrn_,yOffsetScrn_);
			xOffset_ = currentTrace->X(closestIndex);
			xOffsetIndex_ = closestIndex;
		} 
		AddCharToString(this->statusText,'O');
		UpdateStatusWindow(win,3,this->statusText);
		UpdateStatusWindow(win,1,"Left button : Hold for data display; Right button : remove offset");
	}
	else
	{
      xOffset_ = 0;
      xOffsetIndex_ = 0;
		RemoveCharFromString(this->statusText,'O');
		UpdateStatusWindow(win,3,this->statusText);
		UpdateStatusWindow(win,1,"Left button : Hold for data display; Right button : apply offset");
	}  


  if(currentTrace) // Find closest data point to cursor position
  {
		long mini = currentTrace->drawCursor(hdc,dataCursor.x,dataCursor.y);

      if(display1DComplex & SHOW_MAGNITUDE)
      {
         float yMag = sqrt(curTrace()->YReal(mini)*curTrace()->YReal(mini) + curTrace()->YImag(mini)*curTrace()->YImag(mini));

			if(shiftDown)
				sprintf(str,"(idx, x, |y|) = (%ld, %ld,  %g)",mini,mini-xOffsetIndex_,yMag);
			else
				sprintf(str,"(idx, x, |y|) = (%ld, %g,  %g)",mini,curTrace()->X(mini)-xOffset_,yMag);
      }
      else
      {
		   if (!curTrace()->hasImaginary())
		   {
			   if(shiftDown)
				   sprintf(str,"(idx, x, y) = (%ld, %ld,  %g)",mini,mini-xOffsetIndex_,curTrace()->YReal(mini));
			   else
				   sprintf(str,"(idx, x, y) = (%ld, %g,  %g)",mini,curTrace()->X(mini)-xOffset_,curTrace()->YReal(mini));
		 
		   }
		   else
		   {
			   if(shiftDown)
				   sprintf(str,"(idx, x, yr, yi) = (%ld, %ld,  %g, %g)",mini,mini-xOffsetIndex_,curTrace()->YReal(mini),curTrace()->YImag(mini));
			   else
				   sprintf(str,"(idx, x, yr, yi) = (%ld, %g,  %g, %g)",mini,curTrace()->X(mini)-xOffset_,curTrace()->YReal(mini),curTrace()->YImag(mini));
		   }
      }
		UpdateStatusWindow(win,0,str);
	}
}


void Plot1D::HideDataCursor(HDC hdc)
{
   extern bool gScrollWheelEvent;
   HPEN cursorPen;

   if(gScrollWheelEvent)
   {
      gScrollWheelEvent = false;
      return;
   }
   
   SaveDC(hdc);
	SetClippingRegion(hdc,0);
	cursorPen = CreatePen(PS_DOT,0,RGB(0,0,0));
   SelectObject(hdc,cursorPen);
   SetROP2(hdc,R2_XORPEN);
     
   if(yold >= 0 && dataCursorVisible() == true)
   {
      MoveToEx(hdc,dimensions.left(),yold,0);
      LineTo(hdc,dimensions.left()+dimensions.width(),yold);
      MoveToEx(hdc,xold,dimensions.top(),0);
      LineTo(hdc,xold,dimensions.top()+dimensions.height());   
      dataCursorVisible_ = false;
		//TextMessage("Hide real cursor\n");
      yold = -1;
   }

   if(yoldr >= 0 && dataCursorVisible() == true)
   {
	   if(display1DComplex & SHOW_MAGNITUDE)
      {
         MoveToEx(hdc,dimensions.left(),yoldm,0);
         LineTo(hdc,dimensions.left()+dimensions.width(),yoldm);
      }
      else
      {
	      if(display1DComplex & SHOW_REAL)
         {
            MoveToEx(hdc,dimensions.left(),yoldr,0);
            LineTo(hdc,dimensions.left()+dimensions.width(),yoldr);
         }
	      if(display1DComplex & SHOW_IMAGINARY)
         {
            MoveToEx(hdc,dimensions.left(),yoldi,0);
            LineTo(hdc,dimensions.left()+dimensions.width(),yoldi);
         }
      }
      MoveToEx(hdc,xold,dimensions.top(),0);
      LineTo(hdc,xold,dimensions.top()+dimensions.height());     
      dataCursorVisible_ = false;
		//TextMessage("Hide complex cursor\n");

      yoldr = yoldi = yoldm = -1;
   }
      
   RestoreDC(hdc, -1);
   DeleteObject(cursorPen);
}

bool Plot1D::dataCursorVisible()
{
	return dataCursorVisible_;
}


// Shift the plot by a certain fraction vertically and/or horizontally 
void Plot1D::ShiftPlot(short dir)
{
	if (syncAxes())
	{

		for(Axis* axis : axisList())
		{
			axis->setAutorange(false);
			axis->shiftPlot(dir);
		}
	}
	else
	{
		curXAxis()->setAutorange(false);
		curXAxis()->shiftPlot(dir);
		curYAxis()->setAutorange(false);
		curYAxis()->shiftPlot(dir);
	}
}

/**********************************************************
Reverse plots have decreasing x values from left to right.
***********************************************************/
bool Plot1D::isReversePlot()
{
	if (currentTrace)
		return currentTrace->isReverseTrace();
	return false;
}

/****************************************************************************
   Check to see if the x or y data is positive and nonzero 
*****************************************************************************/
bool Plot1D::OKForLog(char dir)
{
	TraceListIterator it;

	for(Trace* t : traceList)
   {
		if (!t->greaterThanZero(dir))
			return false;
   }
	return true;
}

float* Plot1D::GetTraceIDs(int *sz)
{
	if (traceList.empty())
	{
		*sz = 0;
		return NULL;
	}

   float *traceIds = new float[traceList.size()];
   int index = 0;
	TraceListConstIterator it;
	
	for(it = traceList.begin(); it != traceList.end(); ++it)
	{
		traceIds[index++] = (*it)->getID();
	}
   *sz = index;
   return(traceIds);
}

short Plot1D::nextTraceID()
{
	short candidate = 0;
	int count = 0;
	float* existing = GetTraceIDs(&count);
	
	for (candidate = 0; true;candidate++)
	{
		int i;
	// Count up from the lowest possible trace
		for (i = 0; i < count; i++)
		// Compare the current candidate next-trace with all existing ones.
		{
			if (((int)existing[i]) == candidate)
			{
				// This candidate trace id already exists; try the next one.
				break;
			}
		}
		if (i == count)
			// Nothing matched. Return the current number.
			break;
	}
	delete[] existing;
	return candidate;
}


Trace* Plot1D::FindTraceByID(short n)
{
	for(Trace* t : traceList)
   {
		if (t->getID() == n)
			return t;
   }
	return 0;
}


// See if dat is in data list
bool Plot1D::InDataSet(Trace* dat)
{
	for(Trace* t : traceList)
	{
      if(t == dat)
         return(true);
   }
   return(false);
}

/*************************************************************************************************
   Calculates the smallest distance between a line (xs1,ys1) -> (xs2,ys2) and a point (x,y)
   and returns the square of this distance. If the point does not project onto the line segment
   the routine returns -1.
  
   Calculates d = |r2|^2 = |r-r1|^2 = r - v(r.v)/(|v|^2) where v defines the line segment
   (xs1,ys1) -> (xs2,ys2), r the line from (xs1,ys1) to (x,y), r1 the projection of r onto v 
   and r2 the desired vector.
**************************************************************************************************/

float Plot1D::PointToLineDist(float x,float y,float xs1,float ys1,float xs2,float ys2)
{
   float k; // Fraction vector r1 is of length v
   float dx1 = (x-xs1);
   float dx2 = (xs2-xs1);
   float dy1 = (y-ys1);
   float dy2 = (ys2-ys1);
   float d;
   
   k = (dx1*dx2+dy1*dy2)/(dx2*dx2+dy2*dy2);
   
   if(k < 0 || k > 1) return(ERR); // Ignore projections outside segment
   
   d = sqr(dx1-k*dx2) + sqr(dy1-k*dy2);
   
   return(d);
}

/*********************************************************************************
*                      Plot the data stored in arrays x and y                    *
*********************************************************************************/

long Plot1D::Display(HWND hWnd, HDC hdc)
{
   HPEN axes_Color;

// Save current device context ********************
   SaveDC(hdc);
	 
// Make some pens *********************
   if(gPlotMode == PRINT && gPrintMode == BLACK_AND_WHITE)
   {
	   axes_Color  = CreatePen(PS_SOLID,curXAxis()->lineWidth(),RGB(0,curXAxis()->lineWidth(),0));
   }
   else
   {
	   axes_Color  = CreatePen(PS_SOLID,curXAxis()->lineWidth(),axesColor);
		if (gPlotMode != PRINT)
		{
			InitialisePositions(hWnd);
		}
	}
   
// Draw background ********************************
	DrawPlotBackGround(hdc);

// Draw the current plot/trace indicator
	if(gPlotMode == DISPLAY)
		DrawCurrentTraceIndicator(hdc);

// If no data then go no further
   if(traceList.empty())
   {
      SelectObject(hdc,axes_Color);
		DrawBoundingRectange(hdc,axes_Color);
		
		// Need to draw the insets.
		for(Inset* inset : insets_)
		{
			inset->draw(hdc);
		}
      
		RestoreDC(hdc,-1); 
      DeleteObject(axes_Color);
      return(OK);
   }

// See if the plot has been moved in the UI *******
   MovingPlotByMouse();

// See if the plot has been moved by a command ****
   MovingPlotByCmd();

// See if the plot has been scaled by a command ***
   ScalingPlotByCmd();

// See if the plot has been scaled by a the mouse wheel ***
   ScalingPlotByMouseWheel();

   if(getOverRideAutoRange() == false)
   {
		curXAxis()->updateExtrema(curXAxis()->Min(), curXAxis()->Max());
		curYAxis()->updateExtrema(curXAxis()->Min(), curXAxis()->Max());
		otherYAxis()->updateExtrema(curXAxis()->Min(), curXAxis()->Max());
   }

	PlotDimensions origPlotDimensions;

	long xL = dimensions.left();
	long xR = xL + dimensions.width();
	long yT = dimensions.top();
	long yB = yT + dimensions.height();

   // Plot data  
   SaveDC(hdc);

// Make sure the tick spacing is up to date
   for(Axis* axis : axisList_)
	{
		if (axis->getTickLabel() == ERR)
		{
			return ERR;
		}
	}

// Draw a grid is required
	curXAxis()->grid()->draw(hdc);
	if (gridLocked())
	{
		gridLocked()->grid()->draw(hdc);
	}
	else
	{
		curYAxis()->grid()->draw(hdc);
	}

// Draw each trace
	SetClippingRegion(hdc,0);   

// If plotting log in y dependent mode this only applies to the axes
   curYAxis()->setOrigMapping(curYAxis()->mapping());
   if(curYAxis()->origMapping() == PLOT_LOG_Y && axesMode == PLOT_AXIS_BOX_Y_INDEPENDENT)
      curYAxis()->setMapping(PLOT_LINEAR_Y);

	for(Trace* t : traceList)
   {
      // Plot the data

      t->plot(hdc, xL, xR, yT, yB);		

		long szX, szY;

		if(gPlotMode == PRINT)
		{
			float printPixelWidth = GetDeviceCaps(hdc,HORZRES);
			float printMMWidth = GetDeviceCaps(hdc,HORZSIZE);
			szX = szY = printPixelWidth/printMMWidth/4;;
		}
		else if(gPlotMode == IMAGEFILE || gPlotMode == CLIPBOARD)
		{
			//szX = szY = exportScaleFactor_;
			szX = szY = gPlotSF;
		}
		else // Display
		{
			szX = szY = symbolScale_;
		}

   // Plot error bars on real data   
		if(t->tracePar.isShowErrorBars() && t->HasX() && t->HasY())
      {
			t->PlotErrorBars(hdc, szX);
      }
           
   // Plot data symbols     
      if(t->tracePar.getSymbolType() != PLOT_SYMBOL_NO_SYMBOL)
      {
			HPEN bkgPen;
			HBRUSH bkgBrush;
				
			if(gPlotMode == PRINT && gPrintMode == BLACK_AND_WHITE)
			{   
				bkgBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
				bkgPen   = CreatePen(PS_SOLID,0,RGB(255,255,255));
			}
			else
			{   
				bkgBrush = CreateSolidBrush(bkColor);
				bkgPen   = CreatePen(PS_SOLID,0,bkColor);
			}   
			int symbolSize = nint(szX * t->tracePar.getSymbolSize());
			t->PlotDataSymbols(hdc, bkgBrush, bkgPen, symbolSize, symbolSize, dimensions.left(), dimensions.top(), dimensions.right(), dimensions.bottom());

			DeleteObject(bkgBrush);
			DeleteObject(bkgPen);	
		}
   }

// Draw the data cursor cross-hairs (if any) ********
   DrawDataCrossHairs(hdc);

// Draw a vertical selection line (if any) ********
	DrawVerticalSelectionLine(hdc);

// Remove clipping limits
   RestoreDC(hdc, -1);

// Restore the mapping mode
   curYAxis()->setMapping(curYAxis()->origMapping());

// Draw the selection rectangle (if any) ************
   DrawSelectionRectangle(hdc); 

// Update the status bar with vector type and dimensions
	if(this == Plot1D::curPlot())
   {
    //  if(plotParent->inCriticalSection()) // This can cause a dead lock without the critical section check
    //  {
    //     LeaveCriticalSection(&criticalSection);
     //    plotParent->UpdateStatusBar();
    //     EnterCriticalSection(&criticalSection);
    //  }
    //  else
         plotParent->UpdateStatusBar();
   }

// Draw lines
	if(showLines)
	{
		for(PlotLine* line : lines_)
		{
			line->Draw(this,hdc,1);
		}
	}

// Draw text
	if(showText)
	{
		for(PlotText* pltTxt : text_)
		{
			pltTxt->Draw(this,hdc,1);
		}
	}

// Draw insets
	if(showInsets)
	{
		for(Inset* inset : insets_)
		{
			inset->draw(hdc);
		}
	}

// Write plot labels and axes *********************  
// Draw over the top of everything else
   if(axesMode == PLOT_AXES_CROSS || axesMode == PLOT_X_AXIS_CROSS || axesMode == PLOT_Y_AXIS_CROSS)
   {
      SaveDC(hdc);
	   if(gPlotSF == 1)
         SetClippingRegion(hdc,1);
      else
	      SetClippingRegion(hdc,gPlotSF-1);
      if(DrawAxes(hdc,axes_Color) == ERR)
         return(ERR);
      DrawBoundingRectange(hdc,axes_Color);
      RestoreDC(hdc,-1);
      WritePlotLabels(hdc);     
   }
   else
   {
      if(DrawAxes(hdc,axes_Color) == ERR)
         return(ERR);
      DrawBoundingRectange(hdc,axes_Color);
      WritePlotLabels(hdc); 
   }

// Tidy up and restore device context ***************
   RestoreDC(hdc,-1); 
   DeleteObject(axes_Color);

   return(OK);
}

HBRUSH Plot1D::GetPlotBackgroundBrush()
{
	HBRUSH bkBrush;

	if(gPlotMode == PRINT && gPrintMode == BLACK_AND_WHITE)
   {
	   bkBrush     = (HBRUSH)GetStockObject(WHITE_BRUSH);
   }
   else
   {
	   bkBrush     = CreateSolidBrush(bkColor);	   
	}

   if(bkColor == 0xFFFFFFFF)
      bkColor = GetSysColor(COLOR_BTNFACE);

	return bkBrush;
}

/**********************************************************************************
*     Draw a small colored rectange to indicate current plot/trace                 *
**********************************************************************************/

void Plot1D::DrawCurrentTraceIndicator(HDC hdc)
{
	RECT r;

	Plot1D* c1DP = Plot1D::curPlot();
	Plot* cP = Plot::curPlot();

	if (c1DP == this && cP == this)
	{
		Trace* cT = c1DP->curTrace();
		if (cT && c1DP->indicatorSize > 0)
		{
			COLORREF bkColor = cT->tracePar.getRealColor();
			SaveDC(hdc);
			HBRUSH indictorBrush = (HBRUSH)CreateSolidBrush(bkColor);
			HBRUSH borderBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);

			// Fill plot region with background color
			int offset = 6;
			SetRect(&r, regionLeft + offset, regionTop + offset, regionLeft + offset + c1DP->indicatorSize, regionTop + offset + c1DP->indicatorSize);
			FillRect(hdc, &r, indictorBrush);
			FrameRect(hdc, &r, borderBrush);
			DeleteObject(borderBrush);
			DeleteObject(indictorBrush);
			RestoreDC(hdc, -1);
		}
	}
}


/**********************************************************************************
*                    Draw the background and border for plotting                  *
**********************************************************************************/

void Plot1D::DrawPlotBackGround(HDC hdc)
{
	RECT r;

   SaveDC(hdc);
	HBRUSH bkBrush = GetPlotBackgroundBrush();
	HBRUSH borderBrush = GetPlotBorderBrush();

// Fill plot region with background color
	SetRect(&r, dimensions.left(), dimensions.top(), dimensions.left()+dimensions.width(), dimensions.top()+dimensions.height());
	FillRect(hdc,&r,bkBrush);

// Draw border
	if(plotParent->showLabels)
	{   
		SetRect(&r,regionLeft,regionTop,dimensions.left(),regionBottom+1);
		FillRect(hdc,&r,borderBrush); // Left strip
		SetRect(&r,dimensions.left()+dimensions.width(),regionTop,regionRight+1,regionBottom+1);
		FillRect(hdc,&r,borderBrush); // Right strip
		SetRect(&r,dimensions.left(),regionTop,dimensions.left()+dimensions.width(),dimensions.top());
		FillRect(hdc,&r,borderBrush); // Top strip
		SetRect(&r,dimensions.left(),dimensions.top()+dimensions.height(),dimensions.left()+dimensions.width(),regionBottom+1);
		FillRect(hdc,&r,borderBrush); // Bottom strip 
	}
	
	DeleteObject(bkBrush);
	DeleteObject(borderBrush);
	RestoreDC(hdc,-1);
}


void Plot1D::OffsetData(HDC hdc, long x, long y)
{
// Remove the offset if it is already present
   if(isOffset_)
   {
      xOffset_ = 0;
      xOffsetIndex_ = 0;
      isOffset_ = false;
      return;
   }

// Find closest data point to cursor position
   if(curTrace()) 
   {
		long closestIndex = currentTrace->indexOfClosestPoint(x,y);

// Record this as the offset
      xOffset_ = currentTrace->X(closestIndex);
      xOffsetIndex_ = closestIndex;
      isOffset_ = true;
   }   
}

      


void Plot1D::ScalePlot(HWND hWnd, short dir)
{
	for(Axis* axis : axisList())
	{
		if (axis->hasData())
		{
			axis->setAutorange(false);
			axis->saveMinMax();
		}
	}
  
	// Get the location of the mouse pointer within the client window
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(hWnd, &pt);

	RECT r;
	GetClientRect(hWnd, &r);

	// Get the dimensions of the toolbar
	WinData* win = GetWinData(hWnd);
	RECT tb;
	if(win->toolbar)
		GetClientRect(win->toolbar->hWnd, &tb);
	else
		SetRect(&tb,0,0,0,0);

	// TODO: This 7 should be derived from something, but I can't see what right now... they ensure that 
	//  the rightmost and bottommost pixels give a proportion of 1, it works for now, but they shouldn't be derived here!
	const int MAGIC_PLOTHEIGHT_FUDGE = 7;

	int windowTop = tb.bottom ? tb.bottom : plotParent->obj->yo;

	// As a proportion of plot dimensions, how far to the right...
	float relative_width = 
		((float)(pt.x - dimensions.left() - plotParent->obj->xo) / (float)dimensions.width());
	// ... or to the bottom ...

	float relative_height = 
		((float)(pt.y - dimensions.top() - windowTop - MAGIC_PLOTHEIGHT_FUDGE)  / (float)dimensions.height());
   // ... is the mouse pointer?	

	//if (isReversePlot())
   if(curXAxis()->plotDirection() == PLT_REVERSE)
	{
		relative_width = 1 - relative_width;
	}
   if(curYAxis()->plotDirection() == PLT_REVERSE)
	{
		relative_height = 1 - relative_height;
	}
	curXAxis()->zoom(dir, relative_width, relative_height);
	curYAxis()->zoom(dir, relative_width, relative_height);
	if (syncAxes())
	{
		otherYAxis()->zoom(dir, relative_width, relative_height);
	}
}  


/****************************************************************************
   Remove specified entry from the data list
*****************************************************************************/

void Plot1D::RemoveDataFromList(Trace* toRemove)
{
	// Find the one to kill
	TraceListIterator it;
	for (it = traceList.begin(); it != traceList.end(); ++it)
	{
		if (toRemove == (*it))
		{
			delete(*it);
			traceList.erase(it);
			break;
		}
	}
	return;
}


void Plot1D::EnlargePlotCentred(short dir)
{
	if (syncAxes())
	{
		for(Axis* axis : axisList())
		{
			axis->setAutorange(false);
			axis->saveMinMax();
			axis->zoomCentred(dir);
		}
	}
	else
	{
		xAxis_->setAutorange(false);
		xAxis_->saveMinMax();
		xAxis_->zoomCentred(dir);
	
		curYAxis()->setAutorange(false);
		curYAxis()->saveMinMax();
		curYAxis()->zoomCentred(dir);
	}
}  

void Plot1D::displayToEMF(HDC hdc, HWND hWnd)
{
	scale(gPlotSF);
	Display(hWnd, hdc);
	unscale();
}


/****************************************************************************
   Checks to see if the cursor is on or close to one of the currently loaded 
   data sets. If so it returns appropriate trace. Otherwise it returns null.
*****************************************************************************/
Trace* Plot1D::CursorOnDataLine(short x, short y)
{
 	Trace* candidate;
   Trace* closestTrace = NULL;
   float minDist = 1e30;
   float dist;
   long indx;

// Search through display list looking for match
   TraceListIterator it;
	for(Trace* t : traceList)
   {
		candidate = t->CursorOnDataLine(x, y, &indx, &dist);
      if(dist < minDist)
      {
         minDist = dist;
         closestTrace = candidate;
      }
   }
	return(closestTrace);
}


/*****************************************************************************************
*                    Remove all the trace data in a subplot                              *
*****************************************************************************************/
  
void Plot1D::clearData()
{
	setNoCurTrace();
	std::for_each( traceList.begin(), traceList.end(), delete_object());
	traceList.clear();
  // setCurTrace();
	lockGrid(false);
	legend_->setVisible(false);
	legend_->setVisibleLocked(false);
	for(Axis* axis : axisList())
	{
		axis->setMapping(PLOT_LINEAR_Y);
		axis->setAutorange(true);
		axis->setAutoscale(true);
      axis->grid()->setDrawGrid(false);
      axis->grid()->setDrawFineGrid(false);
	}
   curXAxis()->setMapping(PLOT_LINEAR_X);
	clearLabels();
   displayHold = false;
}

/*****************************************************************************************
*                    Modify the override autorange parameter                             *
*****************************************************************************************/
  
void Plot::setOverRideAutoRange(bool state)
{
   overRideAutoRange = state;
	if(plotParent)
      plotParent->obj->winParent->setMenuItemCheck(plotParent->menuName(),"fixed_range",state);
}

bool Plot::getOverRideAutoRange()
{
   return(overRideAutoRange);
}

/*****************************************************************************************
*                    Get axes mode as a string                                           *
*****************************************************************************************/
  
char* Plot::GetAxesTypeStr()
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
   else if(axesMode == PLOT_AXIS_BOX_Y_INDEPENDENT)
      return("box_y_independent");
   else if(axesMode == PLOT_Y_AXIS_CROSS)
      return("yaxis_cross");
   else if(axesMode == PLOT_NO_AXES)
      return("none");
   return("corner");
}

/*****************************************************************************************
*                    Set axes mode as a string                                           *
*****************************************************************************************/
  
bool Plot::SetAxesType(CText type)
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
   else if(type == "box_y_independent")
      axesMode = PLOT_AXIS_BOX_Y_INDEPENDENT;
   else if(type == "yaxis_cross")
      axesMode = PLOT_Y_AXIS_CROSS;
   else if(type == "none")
      axesMode = PLOT_NO_AXES;
   else
      return(1);
   return(0);
}

/*****************************************************************************
*          Initialise menus and toolbar button states based on mode
*****************************************************************************/

void Plot1D::initialiseMenuChecks(const char *mode)
{
   WinData *win = plotParent->obj->winParent;
   if(!strcmp(mode,"clear"))
   {
		setOverRideAutoRange(false);
		const char* const m = plotParent->menuName();
		const char* const tbn = plotParent->toolbarName();
      win->setToolBarItemCheck(tbn, "hold", false);
      win->setMenuItemCheck(m,"show_legend",false);
      win->setMenuItemCheck(m,"hold",false);
      win->setMenuItemCheck(m,"antialiasing",pwd->antiAliasing);
      win->setMenuItemCheck(m,"drag_plot",false);
      win->setMenuItemCheck(m,"display_data",true);
      win->setMenuItemCheck(m,"select_region",false);
      win->setMenuItemCheck(m,"show_border",plotParent->showLabels);
      win->setMenuItemCheck(m,"show_real",true);
      win->setMenuItemCheck(m,"show_imaginary",true);
      win->setMenuItemCheck(m,"show_magnitude",false);
      win->setToolBarItemCheck(tbn, "drag_plot", false);
      win->setToolBarItemCheck(tbn, "display_data", true);
      win->setToolBarItemCheck(tbn, "select_region", false);
      win->setToolBarItemCheck(tbn, "corner_axes", false);
      win->setToolBarItemCheck(tbn, "border_axes", true);
      win->setToolBarItemCheck(tbn, "crossed_axes", false);
      win->setToolBarItemCheck(tbn, "linear_axis", false);
      win->setToolBarItemCheck(tbn, "log_axis", false);
   }
   else if(!strcmp(mode,"load"))
   {
		setOverRideAutoRange(false);
		const char* const m = plotParent->menuName();
		const char* const tbn = plotParent->toolbarName();
      win->setToolBarItemCheck(tbn, "hold", false);
		win->setMenuItemCheck(m,"hold",false);
      win->setMenuItemCheck(m,"show_legend",showLegend());
      win->setMenuItemCheck(m,"antialiasing",isAntiAliasing());
      win->setMenuItemCheck(m,"sync_axes",syncAxes());
      win->setMenuItemCheck(m,"lock_grid",gridLocked());
      win->setMenuItemCheck(m,"show_border",plotParent->showLabels);
      win->setMenuItemCheck(m,"show_real",(display1DComplex & SHOW_REAL));
      win->setMenuItemCheck(m,"show_imaginary",(display1DComplex & SHOW_IMAGINARY));
      win->setMenuItemEnable(m,"show_real",!(display1DComplex & SHOW_MAGNITUDE));
      win->setMenuItemEnable(m,"show_imaginary",!(display1DComplex & SHOW_MAGNITUDE));
      win->setMenuItemCheck(m,"show_magnitude",(display1DComplex & SHOW_MAGNITUDE));
      win->setToolBarItemCheck(tbn, "corner_axes", (axesMode == PLOT_AXES_CORNER));
      win->setToolBarItemCheck(tbn, "border_axes", (axesMode == PLOT_AXES_BOX));
      win->setToolBarItemCheck(tbn, "crossed_axes", (axesMode == PLOT_AXES_CROSS));
      win->setToolBarItemCheck(tbn, "linear_axis", (xAxis_->mapping() == PLOT_LINEAR_X));
      win->setToolBarItemCheck(tbn, "log_axis", (xAxis_->mapping() == PLOT_LOG_X));
   }
   else if(!strcmp(mode,"selectplot"))
   {
      setOverRideAutoRange(getOverRideAutoRange());
		const char* const m = plotParent->menuName();
		const char* const tbn = plotParent->toolbarName();
		win->setToolBarItemCheck(tbn, "hold", displayHold);
      win->setMenuItemCheck(m,"hold",displayHold);
      win->setMenuItemCheck(m,"show_legend",showLegend());
      win->setMenuItemCheck(m,"antialiasing",isAntiAliasing());
      win->setMenuItemCheck(m,"show_border",plotParent->showLabels);
      win->setMenuItemCheck(m,"sync_axes",syncAxes());
      win->setMenuItemCheck(m,"lock_grid",gridLocked());
      win->setMenuItemCheck(m,"show_real",(display1DComplex & SHOW_REAL));
      win->setMenuItemCheck(m,"show_imaginary",(display1DComplex & SHOW_IMAGINARY) && !(display1DComplex & SHOW_MAGNITUDE));
      win->setMenuItemCheck(m,"show_magnitude",(display1DComplex & SHOW_MAGNITUDE) && !(display1DComplex & SHOW_MAGNITUDE));
      win->setToolBarItemCheck(tbn, "corner_axes", (axesMode == PLOT_AXES_CORNER));
      win->setToolBarItemCheck(tbn, "border_axes", (axesMode == PLOT_AXES_BOX));
      win->setToolBarItemCheck(tbn, "crossed_axes", (axesMode == PLOT_AXES_CROSS));
      win->setToolBarItemCheck(tbn, "linear_axis", (xAxis_->mapping() == PLOT_LINEAR_X));
      win->setToolBarItemCheck(tbn, "log_axis", (xAxis_->mapping() == PLOT_LOG_X));
   }
}



/*****************************************************************************
*              Display all the plots in the 1D window
*****************************************************************************/

void Plot1D::DisplayAll(bool locked)
{
   RECT pr;
   HBITMAP bkBitMap;

   PlotWindow* pp = this->plotParent;

// Don't plot if update is false
   if(!pp->updatePlots())
      return;

   if(!updatePlots_)
      return;

// Don't plot from a background thread
   if(GetCurrentThreadId() != mainThreadID)
   {
      SendMessage(pp->hWnd,WM_USER,0,0);
      return; 
   }

   PlotList& pd = pp->plotList();

   if(locked)
   {
		EnterCriticalSection(&cs1DPlot);
		pp->incCriticalSectionLevel();
	}

   GetClientRect(win,&pr);

   HDC hdc = GetDC(win); // prepare window for painting
   if(pp->useBackingStore)
   {
      HDC hdcMem = CreateCompatibleDC(hdc);
      bkBitMap = (HBITMAP)SelectObject(hdcMem,pp->bitmap);
		for(Plot* p : pd)
		{
			p->Display(win, hdcMem);
		}
      BitBlt(hdc,pr.left,pr.top,pr.right-pr.left,pr.bottom-pr.top,hdcMem,pr.left,pr.top,SRCCOPY);
      SelectObject(hdcMem,bkBitMap);
      DeleteDC(hdcMem);
   }
   else // Just draw to screen directly
   {
		for(Plot* p : pd)
		{
			p->Display(win,hdc);
      }
	}
   ReleaseDC(win,hdc);

   if(locked)
   {
		LeaveCriticalSection(&cs1DPlot);
		pp->decCriticalSectionLevel();
	}

}


Trace* Plot1D::curTrace()
{
	return currentTrace;
}

Trace* Plot1D::setCurTrace(int index)
// index defaults to zero.
{
	if ((index < traceList.size()) && this->DataPresent()) 
	{
		currentTrace = traceList[index];
		setCurXAxis(currentTrace->xAxis());
		setCurYAxis(currentTrace->yAxis());
	}
	else 
	{
		setNoCurTrace();
	}
	return currentTrace;
}

Trace* Plot1D::setCurTrace(Trace* t)
// index defaults to zero.
{
	currentTrace = t;
	if (0 != currentTrace)
	{
		setCurXAxis(t->xAxis());
		setCurYAxis(t->yAxis());
	}
	return currentTrace;
}

void Plot1D::setNoCurTrace()
{
	currentTrace = 0;
}


bool Plot1D::hasNoCurTrace()
{
	return (0 == currentTrace);
}

/**************************************
  Add trace t to this plot
**************************************/

Trace* Plot1D::appendTrace(Trace* t)
{
	if(t)
	{
      this->setNoCurTrace();
		traceList.push_back(t);	 // Add the trace to the plot trace list	
		t->setXAxis(curXAxis()); // Make sure the trace knows which axis it belongs to
		t->setYAxis(curYAxis()); 
		curXAxis()->addTrace(t); // Add the trace to the axis trace list
		curYAxis()->addTrace(t); // And update axis limits and direction
		t->setParent(this); // Set trace parent
		t->setID(nextTraceID()); // Give the trace an ID number
		this->setCurTrace(t); // Make this the current trace in this plot
		this->legend()->layoutHasChanged();
	}                        // This also sets the current plot axis ???A bit confusing??
	return t;
}


short Plot1D::removeTrace(int index)
{
	//if (index < 0 || index >= traceList.size())
	//{
	//	return ERR;
	//}
	
	TraceListIterator it;
	for (it = traceList.begin(); it != traceList.end();++it)
	{
		if((*it)->getID() == index)
		{
			delete (*it);
			traceList.erase(it);
			setCurTrace();			
         MyInvalidateRect(win,NULL,false);
			lockGrid(false);
			return(OK);
		}
	}
	this->legend()->layoutHasChanged();
	return(ERR);
}

short Plot1D::removeTrace(Trace* t)
{
	TraceListIterator it;
	for (it = traceList.begin(); it != traceList.end();++it)
	{
		if((*it) == t)
		{
			delete (*it);
			traceList.erase(it);
			setCurTrace();			
         MyInvalidateRect(win,NULL,false);
			lockGrid(false);
			return(OK);
		}
	}
	this->legend()->layoutHasChanged();
	return(ERR);
}


// Set the trace colors using predefined default

void Plot1D::SetTraceColor()
{
	TraceListIterator it;
	for(Trace* t : traceList)
	{
		t->tracePar.setRealColor(pwd->tracePar.getRealColor());
		t->tracePar.setImagColor(pwd->tracePar.getImagColor());
		t->tracePar.setRealSymbolColor(pwd->tracePar.getRealSymbolColor());
		t->tracePar.setImagSymbolColor(pwd->tracePar.getImagSymbolColor());
		t->tracePar.setBarColor(pwd->tracePar.getBarColor());
	}

	this->tracePar.setRealColor(pwd->tracePar.getRealColor());
	this->tracePar.setImagColor(pwd->tracePar.getImagColor());
	this->tracePar.setRealSymbolColor(pwd->tracePar.getRealSymbolColor());
	this->tracePar.setImagSymbolColor(pwd->tracePar.getImagSymbolColor());
	this->tracePar.setBarColor(pwd->tracePar.getBarColor());
}

// Set the trace colors using fixed values

void Plot1D::SetTraceColor(COLORREF traceCol, COLORREF symbolCol, COLORREF barCol)
{
	TraceListIterator it;
	for(Trace* t : traceList)
	{
		t->tracePar.setColors(traceCol, symbolCol, barCol);
	}
	this->tracePar.setColors(traceCol, symbolCol, barCol);
}


// Set the traces in plot to back and white mode with dashed imaginary part

void Plot1D::SetTraceColorBlackNWhite()
{
	TraceListIterator it;
	for(Trace* t : traceList)
	{
		t->tracePar.setColorsBlackNWhite();
	}
	this->tracePar.setColorsBlackNWhite();
}


void Plot1D::DrawSymbol(HDC hdc,short symbol, HBRUSH symbolBrush, HPEN symbolPen, HBRUSH bkgBrush, HPEN bkgPen, long xp, long yp, long szX, long szY)
{
   POINT p[4];

   SaveDC(hdc);

	if(symbol == PLOT_SYMBOL_SQUARE)
	{
	   SelectObject(hdc,symbolBrush);
	   SelectObject(hdc,symbolPen);	   
	   Rectangle(hdc,xp-szX,yp-szY,xp+szX+1,yp+szY+1);
	}
	
	else if(symbol == PLOT_SYMBOL_DIAMOND)
	{
	   SelectObject(hdc,symbolBrush);
	   SelectObject(hdc,symbolPen);
	   p[0].x = xp; p[0].y = yp+szY;
	   p[1].x = xp-szX; p[1].y = yp;
	   p[2].x = xp; p[2].y = yp-szY;
	   p[3].x = xp+szX; p[3].y = yp;
	   
	   Polygon(hdc,p,4);
	}
	else if(symbol == PLOT_SYMBOL_TRIANGLE)
	{
	   SelectObject(hdc,symbolBrush);
	   SelectObject(hdc,symbolPen);
	   p[0].x = xp-szX; p[0].y = yp+szY;
	   p[1].x = xp+szX; p[1].y = yp+szY;
	   p[2].x = xp; p[2].y = yp-szY;
	   
	   Polygon(hdc,p,3);
	}
	else if(symbol == PLOT_SYMBOL_INV_TRIANGLE)
	{
	   SelectObject(hdc,symbolBrush);
	   SelectObject(hdc,symbolPen);
	   p[0].x = xp-szX; p[0].y = yp-szY+1;
	   p[1].x = xp+szX; p[1].y = yp-szY+1;
	   p[2].x = xp; p[2].y = yp+szY+1;
	   
	   Polygon(hdc,p,3);
	}
	else if(symbol == PLOT_SYMBOL_CIRCLE)
	{
	   SelectObject(hdc,symbolBrush);
	   SelectObject(hdc,symbolPen);
	   Ellipse(hdc,xp-szX,yp-szY,xp+szX+1,yp+szY+1);
	   Arc(hdc,xp-szX,yp-szY,xp+szX+1,yp+szY+1,xp+1,yp+szY+1,xp+1,yp+szY+1);
	} 	      
	else if(symbol == PLOT_SYMBOL_OPEN_SQUARE)
	{
	   SelectObject(hdc,bkgBrush);
	   SelectObject(hdc,symbolPen);
	   MoveToEx(hdc,xp-szX,yp-szY,0);
	   Rectangle(hdc,xp-szX,yp-szY,xp+szX+1,yp+szY+1);
	}
	else if(symbol == PLOT_SYMBOL_OPEN_DIAMOND)
	{
	   SelectObject(hdc,bkgBrush);
	   SelectObject(hdc,symbolPen);
	   p[0].x = xp; p[0].y = yp+szY;
	   p[1].x = xp-szX; p[1].y = yp;
	   p[2].x = xp; p[2].y = yp-szY;
	   p[3].x = xp+szX; p[3].y = yp;
	   
	   Polygon(hdc,p,4);
	}	      	      
	else if(symbol == PLOT_SYMBOL_OPEN_TRIANGLE)
	{
	   SelectObject(hdc,bkgBrush);
	   SelectObject(hdc,symbolPen);
	   p[0].x = xp-szX; p[0].y = yp+szY;
	   p[1].x = xp+szX; p[1].y = yp+szY;
	   p[2].x = xp; p[2].y = yp-szY;
	   
	   Polygon(hdc,p,3);
	}
	else if(symbol == PLOT_SYMBOL_OPEN_INV_TRIANGLE)
	{
	   SelectObject(hdc,bkgBrush);
	   SelectObject(hdc,symbolPen);	   
	   p[0].x = xp-szX; p[0].y = yp-szY+1;
	   p[1].x = xp+szX; p[1].y = yp-szY+1;
	   p[2].x = xp; p[2].y = yp+szY+1;
	   
	   Polygon(hdc,p,3);
	}
	else if(symbol == PLOT_SYMBOL_CROSS)
	{
	   SelectObject(hdc,bkgBrush);
	   SelectObject(hdc,bkgPen);	      
	   MoveToEx(hdc,xp-szX,yp-szY,0);
	   Rectangle(hdc,xp-szX,yp-szY,xp+szX+1,yp+szY+1);
	
	   SelectObject(hdc,symbolPen);	      
	   MoveToEx(hdc,xp-szX,yp-szY,0);
	   LineTo(hdc,xp+szX+1,yp+szY+1);
	   MoveToEx(hdc,xp+szX,yp-szY,0);
	   LineTo(hdc,xp-szX-1,yp+szY+1);
	} 
	else if(symbol == PLOT_SYMBOL_PLUS)
	{
	   SelectObject(hdc,bkgBrush);
	   SelectObject(hdc,bkgPen);	      
	   MoveToEx(hdc,xp-szX,yp-szY,0);
	   Rectangle(hdc,xp-szX,yp-szY,xp+szX+1,yp+szY+1);
	
	   SelectObject(hdc,symbolPen);	      
	   MoveToEx(hdc,xp-szX,yp,0);
	   LineTo(hdc,xp+szX+1,yp);
	   MoveToEx(hdc,xp,yp-szY,0);
	   LineTo(hdc,xp,yp+szY+1);
	} 
	else if(symbol == PLOT_SYMBOL_OPEN_CIRCLE)
	{
	   SelectObject(hdc,bkgBrush);
	   SelectObject(hdc,symbolPen);	   
	   Ellipse(hdc,xp-szX,yp-szY,xp+szX+1,yp+szY+1);
	}  
   RestoreDC(hdc,-1);
}


void Plot1D::CloseCurrentData(char *mode)
{
   if(mode[0] == '\0' || !strcmp(mode,"current"))
   {
		if(currentTrace)
		{
         RemoveDataFromList(currentTrace);
			setCurTrace();
		   ResetZoomPoint();
		}
   }
   else if(!strcmp(mode,"all"))
   {
      clearData();
      setOverRideAutoRange(false);
		ResetZoomPoint();
   }
   else // Search for data set by name
   {
      TraceListIterator it;
		for (it = traceList.begin(); it != traceList.end(); ++it)		
      {
 	      if(!strcmp((*it)->getName(),mode))
 	      {
 	         if((*it) == currentTrace)
 	         {
					setNoCurTrace();
 	         }
				delete(*it);
				traceList.erase(it);
				setCurTrace(0);
		      return;
		   }
		}
   }
}

void Plot1D::currentTraceAsVariables(Variable** xAsVar, Variable** yAsVar)
{
	Trace* t = curTrace();
	if (!t) 
	{
		// Nothing to do if there is no current trace.
		*xAsVar = *yAsVar = NULL;
		return;
	}

	*xAsVar = t->xComponentAsVariable();
	*yAsVar = t->yComponentAsVariable();
}
	
void Plot1D::currentTraceMinMaxAsVariables(Variable** xAsVar, Variable** yAsVar)
{
	Trace* t = curTrace();
	if (!t) 
	{
		// Nothing to do if there is no current trace.
		*xAsVar = *yAsVar = NULL;
		return;
	}

	*xAsVar = t->xMinMaxAsVariable();
	*yAsVar = t->yMinMaxAsVariable();   
}

short Plot1D::ZoomRegion(void)
{
   extern float NaN;
	zoomHistory.push_back(selectRect);

// Update current region
   curXAxis()->setMin(selectRect.left);
   curXAxis()->setMax(selectRect.right);
   if(!isnan(selectRect.top))
      curYAxis()->setMin(selectRect.top);
   if(!isnan(selectRect.bottom))
	   curYAxis()->setMax(selectRect.bottom);
	ResetSelectionRectangle();
   return(OK);
}

short Plot1D::DisplayHistory(FloatRect& displayMe)
{
   curXAxis()->setMin(displayMe.left);
   curXAxis()->setMax(displayMe.right);
   curYAxis()->setMin(displayMe.top);
   curYAxis()->setMax(displayMe.bottom);
	return OK;
}

bool Plot1D::lastRegion(HWND hWnd)
{
	if (zoomHistory.size() <= 1)	
   {
      curXAxis()->setAutorange(true);
      curYAxis()->setAutorange(true);
		if (!zoomHistory.empty())
		{
			zoomHistory.pop_back();
		}
   } 
   else 
   {
		zoomHistory.pop_back();
      DisplayHistory(zoomHistory.back());
	}
	updateStatusWindowForZoom(hWnd);
	return true;
}

/*****************************************************************************************
*                                 Draw x and y axes for a plot                           *
*****************************************************************************************/

short Plot1D::DrawAxes(HDC hdc, HPEN axesColor)
{   
// Setup axes tick and label spacings *************
	for(Axis* axis : axisList_)
	{
		if (axis->getTickLabel() == ERR)
		{
			return ERR;
		}
	}
  
   SaveDC(hdc);

// Draw x-y axis **********************************
   SelectObject(hdc,axesColor); 

   if(axesMode == PLOT_X_AXIS || axesMode == PLOT_X_AXIS_BOX)
   {
		if (xAxis_->Max() > xAxis_->Min() && xAxis_->Max() < 3e38 && xAxis_->Min() > -3e38)
		{
			xAxis_->draw(hdc);
		}
   }
   else if(axesMode == PLOT_Y_AXIS || axesMode == PLOT_Y_AXIS_BOX)
   {
		if (yAxisL_->Max() > yAxisL_->Min() && yAxisL_->Max() < 3e38 && yAxisL_->Min() > -3e38)
		{
			yAxisL_->draw(hdc);
		}
		if (yAxisR_ && yAxisR_->Max() > yAxisR_->Min() && yAxisR_->Max() < 3e38 && yAxisR_->Min() > -3e38)
		{
			yAxisR_->draw(hdc);
		}
   }
   else if(axesMode == PLOT_AXIS_BOX_Y_INDEPENDENT)
   {
		if (xAxis_->Max() > xAxis_->Min() && xAxis_->Max() < 3e38 && xAxis_->Min() > -3e38)
		{
			xAxis_->draw(hdc);
		}
		if (yAxisL_->MaxIndep() > yAxisL_->MinIndep() && yAxisL_->MaxIndep() < 3e38 && yAxisL_->MinIndep() > -3e38)
		{
			yAxisL_->drawIndependent(hdc);
		}
   }
   else
   {
      if(axesMode == PLOT_X_AXIS_CROSS)
      {
		   if (xAxis_->Max() > xAxis_->Min() && xAxis_->Max() < 3e38 && xAxis_->Min() > -3e38)
		   {
			   xAxis_->draw(hdc);
		   }
      }
      else if(axesMode == PLOT_Y_AXIS_CROSS)
      {
		   if (yAxisL_->Max() > yAxisL_->Min() && yAxisL_->Max() < 3e38 && yAxisL_->Min() > -3e38)
		   {
			   yAxisL_->draw(hdc);
		   }
      }
      else if(axesMode != PLOT_NO_AXES && (plotParent->showLabels || axesMode == PLOT_AXES_CROSS))
      {
		   if (xAxis_->Max() > xAxis_->Min() && xAxis_->Max() < 3e38 && xAxis_->Min() > -3e38)
		   {
			   xAxis_->draw(hdc);
		   }
		   if (yAxisL_->Max() > yAxisL_->Min() && yAxisL_->Max() < 3e38 && yAxisL_->Min() > -3e38)
		   {
			   yAxisL_->draw(hdc);
		   }
		   if (yAxisR_ && yAxisR_->Max() > yAxisR_->Min() && yAxisR_->Max() < 3e38 && yAxisR_->Min() > -3e38)
		   {
			   yAxisR_->draw(hdc);
		   }
      } 
   }
    
// Draw x & y axis line if in center-axes mode ****

   long xoff   = dimensions.left();
   long width  = dimensions.width();
   long yoff   = dimensions.top();
   long height = dimensions.height();
   long xScrn = curXAxis()->dataToScrn(0);
   long yScrn = curYAxis()->dataToScrn(0);

   if(axesMode == PLOT_AXES_CROSS)
   {
      MoveToEx(hdc,xoff,yScrn,0);
      LineTo(hdc,xoff+width,yScrn); 
      MoveToEx(hdc,xScrn,yoff,0);
      LineTo(hdc,xScrn,yoff+height); 
   }
   else if(axesMode == PLOT_X_AXIS_CROSS)
   {
      MoveToEx(hdc,xoff,yScrn,0);
      LineTo(hdc,xoff+width,yScrn); 
   }
   else if(axesMode == PLOT_Y_AXIS_CROSS)
   {
      MoveToEx(hdc,xScrn,yoff,0);
      LineTo(hdc,xScrn,yoff+height); 
   }
   RestoreDC(hdc,-1);
   
   return(OK);  
}


void Plot1D::ResetZoomPoint()
{
   zoomPoint = false;
} 


float Plot1D::getTickStart(Axis* axis, Ticks* ticks)
{
	return (long) (axis->Min()/ticks->spacing()) * ticks->spacing();
}


bool Plot1D::moveTraceToAxis(Trace* t, VerticalAxisSide side)
{
	Axis* axis = (side == LEFT_VAXIS_SIDE) ? yAxisL_ : yAxisR_;

	if (t->yAxis() == axis)
	{
		return true;
	}

	if (!axis->canDisplay(t))
	{
		// We are trying to move a linear plot with values <= 0 into a 
		// log axis with an existing trace.
		return false;
	}
	if (!axis->hasData() && otherYAxis(axis)->hasData())
	{
		// The destination axis has no traces attached; force its mapping
		// to be the mapping of the source axis.
		axis->setMapping(t->yAxis()->mapping());
		axis->setMin(otherYAxis(axis)->Min());
		axis->setMax(otherYAxis(axis)->Max());
		axis->setGrid(t->yAxis()->grid()->clone());
		axis->grid()->setAxis(axis);
	}
	axis->addTrace(t);
	t->yAxis()->removeTrace(t);
	t->setYAxis(axis);
	this->setCurYAxis(axis);
	this->legend()->layoutHasChanged();
	return true;
}


/*************************************************************************
*         Save 1D data in subplot to a file                        *
*************************************************************************/

short Plot1D::SaveData(char* pathName, char* fileName)
{
// Convert to a 1D plot
   Interface itfc;

// Get the data
   this->GetData(&itfc, 1);
   Variable *xData = &itfc.retVar[1];
   Variable *yData = &itfc.retVar[2];

// Save the data
   return(::SaveData(NULL, pathName, fileName, xData, yData));
}	   


short Plot1D::save(FILE* fp)
{
   long version = PLOTFILE_VERSION_3_7;

   fwrite(&version,sizeof(long),1,fp);

	title().save(fp);
   
	fwrite(&axesMode,sizeof(short),1,fp);
   fwrite(&yLabelVert,sizeof(bool),1,fp);
   fwrite(&axesColor,sizeof(COLORREF),1,fp);
   fwrite(&bkColor,sizeof(COLORREF),1,fp);
   fwrite(&plotColor,sizeof(COLORREF),1,fp);
	fwrite(&borderColor,sizeof(COLORREF),1,fp);

// 3.2 additions
   if(version >= 320)
      fwrite(&display1DComplex, sizeof(short),1,fp); 

	fwrite(&antiAliasing, sizeof(bool),1,fp); 

	legend()->save(fp);

// 3.3 additions
   if(version >= 330)
   {
      // Count the number of insets to save
	   deque<Inset*> saveable_insets;
	   for(Inset* inset : this->insets_)
	   {
		   if (inset->isSaveable())
		   {				
			   saveable_insets.push_back(inset);
		   }
	   }
	   // Write the number of insets
	   int inset_count = saveable_insets.size();
	   fwrite(&inset_count, sizeof(int),1,fp);
	   // Save those insets
	   for(Inset* inset : saveable_insets)
	   {
		   inset->save(fp);
	   }
   }

// 3.4 additions
   if(version >= 340)
   {
      // Save the lines
      long size = lines_.size();
      fwrite(&size,sizeof(long),1,fp);

      for(PlotLine* line : this->lines_)
      {
         line->Save(this, fp);
      }
   }

// 3.5 additions
   if(version >= 350)
   {
      // Save the text
      long size = text_.size();
      fwrite(&size,sizeof(long),1,fp);

      for(PlotText* txt : this->text_)
      {
         txt->Save(this, fp);
      }
   }

	int axisCount;
	// How many axes to write?
	if (this->otherYAxis()->isDrawable())
		axisCount = 3;
	else
		axisCount = 2;
	fwrite(&axisCount, sizeof(int), 1, fp);

	// X axis must exist, and is the first in the file.
	xAxis_->save(fp, version);
	// Current Y axis must exist, and is the second in the file.
	curYAxis()->save(fp, version);
	// Save the "other" Y axis if it has associated traces.
	if (axisCount == 3)
	{
		otherYAxis()->save(fp,version);
	}

	short nrDataSets = traceList.size();
	fwrite(&nrDataSets, sizeof(short), 1, fp);

	for(Trace* t : traceList)
	{
		t->save(fp, version);
	}

	return OK;
}	


/******************************************
 Save additional info for V3.80

 Resaves line info with units included
*****************************************/

short Plot1D::SaveV3_8Info(FILE* fp)
{
   long version = 380;

	fwrite(&version,sizeof(long),1,fp);

	// Save the lines again
	long size = lines_.size();
	fwrite(&size,sizeof(long),1,fp);

	for(PlotLine* line : this->lines_)
	{
		line->Save3_8(this, fp);
	}

	return(OK);
}

/******************************************
 Load additional info for V3.8 

 Reloads line info with units included
*****************************************/

short Plot1D::LoadV3_8Info(FILE *fp)
{
	// Check for additional information

	long version;
   if(!fread(&version,sizeof(long),1,fp))
		return(OK);
	setFileVersion(PLOTFILE_VERSION_3_8);

	if(version == 380)
	{
	// load lines
		long nrLines;
		lines_.clear(); // Remove previous version
		fread(&nrLines, sizeof(long), 1, fp);
		for(long i = 0; i < nrLines; i++)
		{
			PlotLine* line = new PlotLine();
			line->Load3_8(this, fp);
			lines_.push_back(line);
		}
	}
	else
		TextMessage("\nWarning! Expecting extra information for plot V3.8 but see V%3.2f\n",version/100.0);

	
	return(OK);
}


void Plot1D::GetPlotColor(short mode, COLORREF &color)
{
	switch(mode)
	{
	case(ID_PREF_AXES_COLOR):
	case(ID_AXES_COLOR):
		color = this->axesColor;
		break;
	case(ID_PREF_AXES_FONT_COLOR):
		color = this->curXAxis()->ticks().fontColor();
		break;
	case(ID_PREF_GRID_COLOR):
	case(ID_GRID_COLOR):
		color = this->curXAxis()->grid()->color();
		break;
	case(ID_PREF_FINE_GRID_COLOR):         
	case(ID_FINE_GRID_COLOR):
		color = this->curXAxis()->grid()->fineColor();
		break;      
	case(ID_PREF_BG_COLOR):
	case(ID_BK_COLOR):
		color = this->bkColor;
		break;
	case(ID_PREF_BORDER_COLOR):
	case(ID_BORDER_COLOR):
		if(this->borderColor == 0xFFFFFFFF)
			color = GetSysColor(COLOR_BTNFACE);
		else
			color = this->borderColor;
		break;
	case(ID_PREF_TITLE_COLOR):
		color = this->title().fontColor();
		break;
	case(ID_PREF_LABEL_COLOR):
		color = this->curXAxis()->label().fontColor();
		break;  
	case(ID_PREF_REAL_COLOR):
	case(ID_REAL_COLOR):
		color = this->tracePar.getRealColor();
		break;
	case(ID_PREF_IMAG_COLOR):
	case(ID_IMAG_COLOR):
		color = this->tracePar.getImagColor();
		break;
	case(ID_PREF_SYMBOL_COLOR):
	case(ID_SYMBOL_COLOR):
		color = this->tracePar.getRealSymbolColor();
		break; 
	case(ID_PREF_BAR_COLOR):
	case(ID_ERROR_BAR_COLOR):
		color = this->tracePar.getBarColor();
		break;            
	}
}


void Plot2D::GetPlotColor(short mode, COLORREF &color)
{
	switch(mode)
	{
	case(ID_PREF_AXES_COLOR):
	case(ID_AXES_COLOR):
		color = this->axesColor;
		break;
	case(ID_PREF_AXES_FONT_COLOR):
		color = this->curXAxis()->ticks().fontColor();
		break;
	case(ID_PREF_GRID_COLOR):
	case(ID_GRID_COLOR):
		color = this->curXAxis()->grid()->color();
		break;
	case(ID_PREF_FINE_GRID_COLOR):         
	case(ID_FINE_GRID_COLOR):
		color = this->curXAxis()->grid()->fineColor();
		break;      
	case(ID_PREF_BG_COLOR):
	case(ID_BK_COLOR):
		color = this->bkColor;
		break;
	case(ID_PREF_BORDER_COLOR):
	case(ID_BORDER_COLOR):
		if(this->borderColor == 0xFFFFFFFF)
			color = GetSysColor(COLOR_BTNFACE);
		else
			color = this->borderColor;
		break;
	case(ID_PREF_TITLE_COLOR):
		color = this->title().fontColor();
		break;
	case(ID_PREF_LABEL_COLOR):
		color = this->curXAxis()->label().fontColor();
		break;          
	}
}


void Plot1D::scale_(double x, double y)
{
	Plot::scale_(x,y);

	for(Trace* t : traceList)
   {
      t->scale(x,y);
   }
}

void Plot1D::unscale_()
{
	Plot::unscale_();

	for(Trace* t : traceList)
   {
      t->unscale();
   }
}


void Plot1D::showLegend(bool show)
{
	legend_->setVisible(show);
	DisplayAll(false); 
}

bool Plot1D::showLegend() 
{
	return legend_->visible();
}

bool Plot1D::legendIsVisible(){
	return showLegend();
}


void Plot1D::PasteInto(Plot* destination)
{
	Plot1D* dest;
	if (!(dest = dynamic_cast<Plot1D*>(destination)))
	{
		return;
	}
	for(Trace* t : traceList)
	{
		Trace* tc = t->clone();
		dest->appendTrace(tc);
	}
}

bool Plot1D::canAddTrace(Trace* t)
{
   if(axesMode != PLOT_AXIS_BOX_Y_INDEPENDENT)
   {
	   if (!t->greaterThanZero('x'))
	   {
		   if (mapping('x') == PLOT_LOG_X)
		   {
			   return false;
		   }
	   }
	   if (!t->greaterThanZero('y'))
	   {
		   return mapping('y') != PLOT_LOG_Y;
	   }
   }
	return true;
}


short Plot1D::setAxisMapping(short mapping)
{
	// Verify that this is a legit change of mapping.
	switch(mapping)
	{
	case PLOT_LOG_X:
	   for(Trace* t : traceList)
		{
			if (!(t->greaterThanZero('x')))
			{
				ErrorMessage("Cannot display x values <= 0 in log plot.");
				return ERR;
			}
		}
		break;
	case PLOT_LOG_Y:
	   for(Trace* t : traceList)
		{
         if(this->axesMode != PLOT_AXIS_BOX_Y_INDEPENDENT)
         {
			   if (!(t->greaterThanZero('y')))
			   {
				   ErrorMessage("Cannot display y values <= 0 in log plot.");
				   return ERR;
			   }
         }
		}
		break;
	}
	return Plot::setAxisMapping(mapping);
}


COLORREF Plot1D::ChoosePlotColor(HWND hWnd, short dest, bool bkgColorEnabled)
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
   switch(dest)
   {
      case(ID_PREF_TITLE_COLOR):
         cc.rgbResult = title().fontColor();
         break;
      case(ID_PREF_LABEL_COLOR):
         cc.rgbResult = curXAxis()->label().fontColor();
         break;
      case(ID_PREF_BG_COLOR):
      case(ID_BK_COLOR):
         cc.rgbResult = bkColor;
         break; 
      case(ID_PREF_AXES_COLOR):
      case(ID_AXES_COLOR):
         cc.rgbResult = axesColor;
         break; 
      case(ID_PREF_AXES_FONT_COLOR):
			cc.rgbResult = curXAxis()->ticks().fontColor();
         break; 
      case(ID_PREF_BORDER_COLOR):
      case(ID_BORDER_COLOR):
         if(borderColor == 0xFFFFFFFF)
            cc.rgbResult = GetSysColor(COLOR_BTNFACE);
         else
            cc.rgbResult = borderColor;         
         break;
      case(ID_PREF_GRID_COLOR):
      case(ID_GRID_COLOR):
         cc.rgbResult = curXAxis()->grid()->color();
         break;
      case(ID_FINE_GRID_COLOR):
         cc.rgbResult = curXAxis()->grid()->fineColor();
         break;
      case(ID_PREF_REAL_COLOR):
      case(ID_REAL_COLOR):
         cc.rgbResult = tracePar.getRealColor();
         break;
      case(ID_PREF_IMAG_COLOR):
      case(ID_IMAG_COLOR):
         cc.rgbResult = tracePar.getImagColor();
         break;
      case(ID_PREF_SYMBOL_COLOR):
      case(ID_SYMBOL_COLOR):
         cc.rgbResult = tracePar.getRealSymbolColor();
         break;
      case(ID_PREF_BAR_COLOR):
      case(ID_ERROR_BAR_COLOR):
         cc.rgbResult = tracePar.getBarColor();
         break;         
   }
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

void Plot1D::setLegend(PlotLegend* legend) 
{
	if (legend_) delete legend_; 
	legend_ = legend;
	legend_->setParent(this); 
	legend_->setTraceList(&traceList); 
	insets_.push_front(legend_); // FIXME: push_front, because some bad code elsewhere assumes the legend is at index 0. fix that bad code.
}

void Plot1D::setDefaultParameters()
{
	Plot::setDefaultParameters();
	if(GetNrTraces() == 1)
	{
		COLORREF plotRCol = pwd->tracePar.getRealColor();
		COLORREF plotICol = pwd->tracePar.getImagColor();
		COLORREF symbRCol = pwd->tracePar.getRealSymbolColor();
		COLORREF symbICol = pwd->tracePar.getImagSymbolColor();
		short symSize = pwd->tracePar.getSymbolSize();
		short symType = pwd->tracePar.getSymbolType();
		curTrace()->tracePar.setRealColor(plotRCol);
		curTrace()->tracePar.setImagColor(plotICol);
		curTrace()->tracePar.setRealSymbolColor(symbRCol);
		curTrace()->tracePar.setImagSymbolColor(symbICol);
		//     curTrace()->tracePar.setSymbolType(symType);
		curTrace()->tracePar.setSymbolSize(symSize);
		curTrace()->tracePar.setImagColor(plotICol);
		getTracePar()->setRealColor(plotRCol);
		getTracePar()->setImagColor(plotICol);
		getTracePar()->setRealSymbolColor(symbRCol);
		getTracePar()->setImagSymbolColor(symbICol);
		getTracePar()->setSymbolSize(symSize);
		//     getTracePar()->setSymbolType(symType);
	}
}

void Plot1D::LoadAndDisplayDataFile(HWND hWnd, char *basePath, char *fileName)
{
	Interface itfc;
	Variable var1,var2;
	makeCurrentPlot();
	makeCurrentDimensionalPlot();

	if(LoadData(&itfc, basePath, fileName, &var1, &var2) == OK)
	{
		if(itfc.nrRetValues == 2) // x-y data
		{
			CText name1 = fileName;
			name1.RemoveExtension();
			name1 = name1 + "_x";
			AssignToExpression(&itfc,GLOBAL,name1.Str(),&var1,true);
			CText name2 = fileName;
			name2.RemoveExtension();
			name2 = name2 + "_y";
			AssignToExpression(&itfc,GLOBAL,name2.Str(),&var2,true);
			CText arg = name1 + "," + name2;
			PlotXY(&itfc,arg.Str());
		}
		else // y data
		{
			CText arg = fileName;
			arg.RemoveExtension();
			AssignToExpression(&itfc,GLOBAL,arg.Str(),&var1,true);
			PlotXY(&itfc,arg.Str());
		}

		SendMessageToGUI("1D Plot,LoadPlot",0);
		UpdateStatusWindow(hWnd,1,"Left button : select a region");
		statusText[0] = '\0';
		UpdateStatusWindow(hWnd,3,statusText);
		ResetZoomPoint();  
		short len = strlen(title().text());
		char *title = new char[len+20];
		sprintf(title,"Plot-1");
		makeCurrentPlot();
		makeCurrentDimensionalPlot();
		setCurTrace(); 
		SetWindowText(hWnd,title); 
		delete [] title;
		RemoveCharFromString(statusText,'V');
		UpdateStatusWindow(hWnd,3,statusText);
		plotParent->clearPlotData(plotParent->getSavedPlotList());
		MyInvalidateRect(hWnd,NULL,false);
	}
	
}

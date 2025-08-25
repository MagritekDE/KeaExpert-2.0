#define WINVER _WIN32_WINNT 
#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)
#include <gdiplus.h> 
#include "stdafx.h"
#include "plot.h"
#include "allocate.h"
#include "trace.h"
#include "defineWindows.h"
#include "drawing.h"
#include "files.h"
#include "font.h"
#include "guiWindowClass.h"
#include "guiWindowCLI.h"
#include "guiWindowsEvents.h"
#include "globals.h"
#include "guiModifyObjectParameters.h"
#include "inset.h"
#include "interface.h"
#include "plotLine.h"
#include "load_save_data.h"
#include "math.h"
#include "message.h"
#include "metafile.h"
#include "mymath.h"
#include "plot2dCLI.h"
#include "plot3dSurface.h"
#include "PlotFile.h"
#include "PlotGrid.h"
#include "plotText.h"
#include "plotwindefaults.h"
#include "PlotWindow.h"
#include "print.h"
#include "prospaResource.h"
#include "rgbColors.h" 
#include "variablesClass.h"
#include "variablesOther.h"
#include "utilities.h"
#include "winEnableClass.h"
#include <assert.h>
#include <vector>
//#include "memoryLeak.h"

using std::vector;
using namespace Gdiplus;


Plot2D* Plot2D::cur2DPlot = 0;
bool Plot2D::rowCursorVisible = false; // Cursor visibility
bool Plot2D::colCursorVisible = false;
bool Plot2D::dataCursorVisible_ = false;
long Plot2D::xold = 0;
long Plot2D::yold = 0;
long plot2DSaveVersion = PLOTFILE_CURRENT_VERSION_2D;


char Plot2D::currFileName[PLOT_STR_LEN] = {'\0'};
char Plot2D::currFilePath[PLOT_STR_LEN] = {'\0'};
//PlotWindow *inDispAllPP = 0;
//bool inDispAll = false;
//double inDispAllTime = 0;

Plot2D::Plot2D(PlotWinDefaults *pd, HWND hWnd, PlotWindow *pp, short row, short col)
: Plot(pd,hWnd,pp,row,col)
{
// Reset the x and y offsets
   xOffset_ = 0;
   yOffset_ = 0;
   isOffset_ = false;
   xOffsetIndex_ = 0;
   yOffsetIndex_ = 0;
	xVectorStep = 0;   // How many x cells to skip over during plot
	yVectorStep = 0;   // How many y cells to skip over during plot

   vx_ = (float**)0; 
   vy_ = (float**)0; 
   cmat_ = (complex**)0; 
   matRGB_ = (float***)0;
   matWidth_ = 0;
   matHeight_ = 0;
	vectorLength_ = 0;
   mat_ = (float**)0; 
   maxVal_ = minVal_ = 0;

	resetDataView();
	
	xAxis_ = Axis::makeAxis(HORIZONTAL_AXIS, PLOT_LINEAR_X, this);
	yAxisL_ = Axis::makeAxis(VERTICAL_AXIS, PLOT_LINEAR_Y, this);
	yAxisR_ = Axis::makeAxis(VERTICAL_AXIS, PLOT_LINEAR_Y, this, RIGHT_VAXIS_SIDE);

	curXAxis_ = xAxis_;
	curYAxis_ = yAxisL_;

	axisList().push_back(xAxis_);
	axisList().push_back(yAxisL_);
	axisList().push_back(yAxisR_);

   nrContourLevels = DEFAULT_CONTOUR_LEVELS;
   contourLevels = NULL;
   contourLineWidth = 1.0;
   useFixedContourColor_ = false;
   setFixedContourColor(RGB(0,0,0));

   colorScale_ = 0;  
   colorMap_ = (float**)0;
   colorMapLength_ = 0;
   displayColorScale_ = false;

   drawMode_ = DISPLAY_IMAGE;

   
   wfColor = RGB(0,0,0);
   this->setAlpha(0);
   this->setBeta(0);
   this->setGamma(0);

	dataMapping = LINEAR_MAPPING;

   limitfunc = false;

   xAxis = (float*)0;
   yAxis = (float*)0;
}


Plot2D::Plot2D(PlotWinDefaults *pd, HWND hWnd, PlotWindow *pp, short row, short col, Margins &margins)
: Plot(pd,hWnd,pp,row,col,margins)
{
// Reset the x and y offsets
   xOffset_ = 0;
   yOffset_ = 0;
   isOffset_ = false;
   xOffsetIndex_ = 0;
   yOffsetIndex_ = 0;
	xVectorStep = 0;   // How many x cells to skip over during plot
	yVectorStep = 0;   // How many y cells to skip over during plot

   vx_ = (float**)0; 
   vy_ = (float**)0; 
   cmat_ = (complex**)0; 
   matRGB_ = (float***)0;
   matWidth_ = 0;
   matHeight_ = 0;
	vectorLength_ = 0;
   mat_ = (float**)0; 
   maxVal_ = minVal_ = 0;

	resetDataView();
	
	xAxis_ = Axis::makeAxis(HORIZONTAL_AXIS, PLOT_LINEAR_X, this);
	yAxisL_ = Axis::makeAxis(VERTICAL_AXIS, PLOT_LINEAR_Y, this);
	yAxisR_ = Axis::makeAxis(VERTICAL_AXIS, PLOT_LINEAR_Y, this, RIGHT_VAXIS_SIDE);

	curXAxis_ = xAxis_;
	curYAxis_ = yAxisL_;

	axisList().push_back(xAxis_);
	axisList().push_back(yAxisL_);
	axisList().push_back(yAxisR_);

   nrContourLevels = DEFAULT_CONTOUR_LEVELS;
   contourLevels = NULL;
   useFixedContourColor_ = false;
   setFixedContourColor(RGB(0,0,0));
   contourLineWidth = 1.0;

   colorScale_ = 0;  
   colorMap_ = (float**)0;
   colorMapLength_ = 0;
   displayColorScale_ = false;

   drawMode_ = DISPLAY_IMAGE;

   
   wfColor = RGB(0,0,0);
   this->setAlpha(0);
   this->setBeta(0);
   this->setGamma(0);

   xAxis = (float*)0;
   yAxis = (float*)0;

   dataMapping = LINEAR_MAPPING;

	limitfunc = false;

}

Plot2D::Plot2D(const Plot2D& c)
: Plot(c)
{
	xVectorStep = c.xVectorStep;   // How many x cells to skip over during plot
	yVectorStep = c.yVectorStep;   // How many y cells to skip over during plot

	xOffset_ = c.xOffset_; // 2D X offset
	yOffset_ = c.yOffset_; // 2D X offset
   isOffset_ = c.isOffset_;

	xOffsetIndex_ = c.xOffsetIndex_; // 2D X offset as an index
	yOffsetIndex_ = c.yOffsetIndex_; // 2D X offset as an index
	
	setVisibleLeft(c.visibleLeft());
	setVisibleTop(c.visibleTop());
	setVisibleWidth(c.visibleWidth());
	setVisibleHeight(c.visibleHeight());

   nrContourLevels = c.nrContourLevels;
   useFixedContourColor_ = c.useFixedContourColor_;
   contourColor_ = c.contourColor_;
   contourLineWidth = c.contourLineWidth;

	vectorLength_ = c.vectorLength_;  // 2D maximum vector length
	matWidth_ = c.matWidth_;       // and its size
	matHeight_ = c.matHeight_;
	
   long  w = c.matWidth_;
   long  h = c.matHeight_;

	dataMapping = c.dataMapping;

	mat_ = vx_ = vy_ = 0;
	matRGB_ = 0;
	cmat_ = 0;

	if (c.mat_)
	{
		mat_ = MakeMatrix2D(w,h);
      for(int y = 0; y < h; y++)
         for(int x = 0; x < w; x++)
            mat_[y][x] = c.mat_[y][x];
	}
   if(c.cmat_)
   {
      cmat_ = MakeCMatrix2D(w,h);
      for(int y = 0; y < h; y++)
         for(int x = 0; x < w; x++)
            cmat_[y][x] = c.cmat_[y][x];
   }
	if (c.matRGB_)
	{
		this->matRGB_ = MakeMatrix3D(w, h, 3);
		for (int dim = 0; dim < 3; dim++)
			for (int i = 0; i < h; i++)
				for (int j = 0; j < w; j++)
					this->matRGB_[dim][i][j] = c.matRGB_[dim][i][j];
	}	
	if (c.vx_)
	{
		this->vx_ = MakeMatrix2D(w,h);
		for(int y = 0; y < h; y++)
			for(int x = 0; x < w; x++)
				this->vx_[y][x] = c.vx_[y][x];
	}
	if (c.vy_)
	{
		this->vy_ = MakeMatrix2D(w,h);
		for(int y = 0; y < h; y++)
			for(int x = 0; x < w; x++)
				this->vy_[y][x] = c.vy_[y][x];
	}
	maxVal_ = c.maxVal_;
	minVal_ = c.minVal_;    // Matrix range

   if(c.contourLevels)
   {
      contourLevels = new float[c.nrContourLevels];
      for(int x = 0; x < nrContourLevels; x++)
         this->contourLevels[x] = c.contourLevels[x];
   }
   else
      contourLevels = NULL;

	colorScale_ = c.colorScale_;
	colorMapLength_  = c.colorMapLength_;
	colorMap_ = 0;
	displayColorScale_ = c.displayColorScale_; // Whether a color scale is drawn or not
	
	if(c.colorMap_ && c.colorMapLength_)
   {
      colorMap_ = MakeMatrix2D(3,c.colorMapLength_);
      memcpy(&colorMap_[0][0],&c.colorMap_[0][0],3*c.colorMapLength_*sizeof(float));
   }
	drawMode_ = c.drawMode_;

   if(c.xAxis)
   {
      xAxis = new float[w];
      for(int x = 0; x < w; x++)
         xAxis[x] = c.xAxis[x];
   }
   else
      xAxis = 0;
   if(c.yAxis)
   {
      yAxis = new float[h];
      for(int y = 0; y < h; y++)
         yAxis[y] = c.yAxis[y];
   }
   else
      yAxis = 0;

	limitfunc = false;

}


Plot2D* Plot2D::clone() const
{
	return new Plot2D(*this);
}

Plot2D::~Plot2D()
{
	clearData();
	clearColorMap();
}

short Plot2D::getDimension()
{
	return 2;
}

/*****************************************************************************************
*                      Check to see if plot data is present                              *
*****************************************************************************************/
       
bool Plot2D::DataPresent() const
{
   if(!cmat_ && !mat_ && !matRGB_)
		return(false);
	return true;
}

HBRUSH Plot2D::GetPlotBackgroundBrush()
{
	HBRUSH bkBrush;
   if(gPlotMode == PRINT && gPrintMode == BLACK_AND_WHITE)
   {
	   bkBrush     = (HBRUSH)GetStockObject(WHITE_BRUSH);
   }
   else
   {
      if((!mat_ && !cmat_) || ((mat_ || cmat_)  && (drawMode_ & DISPLAY_CONTOURS)))	   
	      bkBrush = CreateSolidBrush(bkColor);
	   else
	      bkBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
	}
	return bkBrush;
}


/****************************************************
Set this plot as the current 2D plot.
****************************************************/
void Plot2D::makeCurrentDimensionalPlot()
{
	cur2DPlot = this;
}

Plot2D* Plot2D::curPlot()
{
	return Plot2D::cur2DPlot;
}

void Plot2D::setNoCurPlot()
{
	cur2DPlot = 0;
}


/************************************************************************************
* Display a cross-hair cursor in the 2D plot at location (x,y)
* and report the data value at data location corresponding to coord. (x,y)
************************************************************************************/

void Plot2D::DisplayData(HDC hdc, long x, long y, bool shift_down)
{
   HPEN cursorPen;
   char str[MAX_STR];
   float r;
   extern bool gScrollWheelEvent;

// Make sure there is some data to plot 
   if(!DataPresent()) return;
   
   SaveDC(hdc);

// Select pens for cursor
   cursorPen = CreatePen(PS_DOT,0,RGB(0,0,0));
   SelectObject(hdc,cursorPen);
   SetROP2(hdc,R2_XORPEN);

// Remove last cursor drawn
   if(dataCursorVisible() && !gScrollWheelEvent)
   {
	   MoveToEx(hdc,dimensions.left(),yold,0);
	   LineTo(hdc,dimensions.left()+dimensions.width(),yold);
	   MoveToEx(hdc,xold,dimensions.top(),0);
	   LineTo(hdc,xold,dimensions.top()+dimensions.height());
      if(x == -1 && y == -1)
      { 
         RestoreDC(hdc, -1);
         DeleteObject(cursorPen);
         return;
      }
   }
   gScrollWheelEvent = false;

// Draw cursor at position (x,y)
   MoveToEx(hdc,dimensions.left(),y,0);
   LineTo(hdc,dimensions.left()+dimensions.width(),y);
   MoveToEx(hdc,x,dimensions.top(),0);
   LineTo(hdc,x,dimensions.top()+dimensions.height());
   dataCursorVisible_ = true;

   xold = x;
   yold = y;

// Display indices (indexx,indexy,value)
   if(shift_down)
   {
   // Find location in matrix at screen coords (x,y)
      long xd = nint(curXAxis()->scrnToData(x));
      long yd = nint(curYAxis()->scrnToData(y));

   // Display the data in the matrix at the location (xd,yd)
      if(mat_) // Real matrix
      {   
         if(isOffset_)
         {
            long dxi = xd-xOffsetIndex_;
            long dyi = yd-yOffsetIndex_;
            r = sqrt(sqr(xd-xOffsetIndex_)+sqr(yd-yOffsetIndex_));
	         sprintf(str,"(x, y, dist, value) = (%ld, %ld, %0.3g, %0.3g)",dxi,dyi,r,mat_[yd][xd]);
         }
         else
         {
	         sprintf(str,"(x, y, value) = (%ld, %ld,  %0.3g)",xd,yd,mat_[yd][xd]);
         }
         UpdateStatusWindow(win,0,str);
      }
      else if(cmat_) // Complex matrix
      {
         if(isOffset_)
         {
            long dxi = xd-xOffsetIndex_;
            long dyi = yd-yOffsetIndex_;
            r = sqrt(sqr(xd-xOffsetIndex_)+sqr(yd-yOffsetIndex_));
	         sprintf(str,"(x, y, dist, r, i) = (%ld, %ld, %0.3g, %0.3g, %0.3g)",dxi,dyi,r,cmat()[yd][xd].r,cmat()[yd][xd].i);
         }
         else
         {
	         sprintf(str,"(x,y, r,i) = (%ld,%ld, %0.3g,%0.3g)",xd-xOffsetIndex_,yd-yOffsetIndex_,cmat_[yd][xd].r,cmat_[yd][xd].i);
         }
         UpdateStatusWindow(win,0,str);
      }
      else if(matRGB_) // RGB matrix
      {
         if(isOffset_)
         {
            long dxi = xd-xOffsetIndex_;
            long dyi = yd-yOffsetIndex_;
            r = sqrt(sqr(xd-xOffsetIndex_)+sqr(yd-yOffsetIndex_));
            sprintf(str,"(x,y,dist) : [r,g,b] = (%ld,%ld,%ld) : [%ld,%ld,%ld]",dxi,dyi,nint(r),nint(matRGB_[0][yd][xd]),
                                                                                      nint(matRGB_[1][yd][xd]),
                                                                                      nint(matRGB_[2][yd][xd]));
         }
         else
         {
            sprintf(str,"(x,y) : [r,g,b]) = (%ld,%ld) : [%ld,%ld,%ld]",xd-xOffsetIndex_,
                                                                      yd-yOffsetIndex_,
                                                                      nint(matRGB_[0][yd][xd]),
                                                                      nint(matRGB_[1][yd][xd]),
                                                                      nint(matRGB_[2][yd][xd]));
         }
         UpdateStatusWindow(win,0,str);
      }
   }
// Display user coordinates (x,y,value)
   else
   {
   // Find location in matrix at screen coords (x,y)
      long xd = nint(curXAxis()->scrnToData(x));
      long yd = nint(curYAxis()->scrnToData(y));
      float xu = curXAxis()->dataToUser(xd);
      float yu = curYAxis()->dataToUser(yd);

   // Display the data in the matrix at the location (xd,yd)
      if(mat_) // Real matrix
      {    
         if(isOffset_)
         {
            float dx = xu-xOffset_;
            float dy = yu-yOffset_;
            r = sqrt(sqr(xu-xOffset_)+sqr(yu-yOffset_));
	         sprintf(str,"(x, y, dist, value) = (%0.3g, %0.3g, %0.3g, %0.3g)",dx,dy,r,mat_[yd][xd]);
         }
         else
         {
	         sprintf(str,"(x, y, value) = (%0.3g, %0.3g, %0.3g)",xu,yu,mat_[yd][xd]);
         }
         UpdateStatusWindow(win,0,str);
      }
      else if(cmat_)// Complex matrix
      {
         if(isOffset_)
         {
            float dx = xu-xOffset_;
            float dy = yu-yOffset_;
            r = sqrt(sqr(xu-xOffset_)+sqr(yu-yOffset_));
	         sprintf(str,"(x, y, dist, r, i) = (%0.3g, %0.3g, %0.3g, %0.3g, %0.3g)",dx,dy,r,cmat_[yd][xd].r,cmat_[yd][xd].i);
         }
         else
         {
	         sprintf(str,"(x, y, r, i) = (%0.3g, %0.3g, %0.3g, %0.3g)",xu,yu,cmat_[yd][xd].r,cmat_[yd][xd].i);
         }
         UpdateStatusWindow(win,0,str);
      }
      else if(matRGB_) // RGB matrix
      {
         if(isOffset_)
         {
            float dx = xu-xOffset_;
            float dy = yu-yOffset_;
            r = sqrt(sqr(xd-xOffsetIndex_)+sqr(yd-yOffsetIndex_));
            sprintf(str,"(x,y,dist) : [r,g,b] = (%ld,%ld,%ld) : [%ld,%ld,%ld]",nint(dx),nint(dy),nint(r),nint(matRGB_[0][yd][xd]),
                                                                                      nint(matRGB_[1][yd][xd]),
                                                                                      nint(matRGB_[2][yd][xd]));
         }
         else
         {
            sprintf(str,"(x,y):[r,g,b] = (%ld,%ld) : [%ld,%ld,%ld]",nint(xu),nint(yu),
                                                              nint(matRGB_[0][yd][xd]),
                                                              nint(matRGB_[1][yd][xd]),
                                                              nint(matRGB_[2][yd][xd]));
         }
         UpdateStatusWindow(win,0,str);
      }
   }

   RestoreDC(hdc, -1);
   DeleteObject(cursorPen);
}

void Plot2D::HideDataCursor(HDC hdc)
{
   extern bool gScrollWheelEvent;
   HPEN cursorPen;

   if(gScrollWheelEvent)
   {
      gScrollWheelEvent = false;
      return;
   }   

   if(!DataPresent()) return;

   SaveDC(hdc);
	cursorPen = CreatePen(PS_DOT,0,RGB(0,0,0));
   SelectObject(hdc,cursorPen);
   SetROP2(hdc,R2_XORPEN);

   if(yold >= 0 && dataCursorVisible())
   {
      MoveToEx(hdc,dimensions.left(),yold,0);
      LineTo(hdc,dimensions.left()+dimensions.width(),yold);
      MoveToEx(hdc,xold,dimensions.top(),0);
      LineTo(hdc,xold,dimensions.top()+dimensions.height());     
      dataCursorVisible_ = false;
   }
   
   RestoreDC(hdc, -1);
   DeleteObject(cursorPen);
}

void Plot2D::HideRowCursor(HDC hdc)
{
   HPEN cursorPen;   

   if(!DataPresent()) return;

   SaveDC(hdc);
	cursorPen = CreatePen(PS_DOT,0,RGB(0,0,0));
   SelectObject(hdc,cursorPen);
   SetROP2(hdc,R2_XORPEN);

   if(yold >= 0 && rowCursorVisible == true)
   {
      MoveToEx(hdc,dimensions.left(),yold,0);
      LineTo(hdc,dimensions.left()+dimensions.width(),yold);
      rowCursorVisible = false;
   }
   
   RestoreDC(hdc, -1);
   DeleteObject(cursorPen);
}

void Plot2D::HideColumnCursor(HDC hdc)
{
   HPEN cursorPen;
   
   if(!DataPresent()) return;

   SaveDC(hdc);
	cursorPen = CreatePen(PS_DOT,0,RGB(0,0,0));
   SelectObject(hdc,cursorPen);
   SetROP2(hdc,R2_XORPEN);

   if(yold >= 0 && colCursorVisible == true)
   {
      MoveToEx(hdc,xold,dimensions.top(),0);
      LineTo(hdc,xold,dimensions.top()+dimensions.height());     
      colCursorVisible = false;
   }
   
   RestoreDC(hdc, -1);
   DeleteObject(cursorPen);
}


/****************************************************************************
   Extract the visible row at position y from the current 2D data set and 
   display in the current 1d data set, also save complete row in vector v1
*****************************************************************************/

void Plot2D::ScanRow(HWND hWnd, short x, short y, char *name)
{
   short mapping = PLOT_LINEAR_X;
	long xd,yd;
   HPEN cursorPen;
   Variable *var;
   extern bool gScrollWheelEvent;

	Plot1D* cur1DPlot = Plot1D::curPlot();

   if(!cur1DPlot) return;
  
   HDC hdc2D = GetDC(hWnd);

// Make sure there is some data to plot **
   if(!DataPresent())
      return;

// Make sure this is not a vector plot ****
   if(vx_ || vy_)
      return;

// Select pens for cursor *****************
   cursorPen = CreatePen(PS_DOT,0,RGB(0,0,0));
   SelectObject(hdc2D,cursorPen);
   SetROP2(hdc2D,R2_XORPEN);

// Remove last row cursor drawn ***********
   if(rowCursorVisible && !gScrollWheelEvent)
   {
	   MoveToEx(hdc2D,dimensions.left(),yold,0);
	   LineTo(hdc2D,dimensions.left()+dimensions.width(),yold);
   }
   gScrollWheelEvent = false;

// Draw cursor at position (x,y) ***********
   MoveToEx(hdc2D,dimensions.left(),y,0);
   LineTo(hdc2D,dimensions.left()+dimensions.width(),y);
   ReleaseDC(hWnd,hdc2D);  
   DeleteObject(cursorPen);                          				                     
   
   rowCursorVisible = true;

   xold = x;
   yold = y;

// Clear all old data in 1D plot first *****  
   bool temp = cur1DPlot->getOverRideAutoRange();
	if(cur1DPlot->displayHold == false)
	   cur1DPlot->CloseCurrentData("all");
   else
	   cur1DPlot->CloseCurrentData("row");
   cur1DPlot->setOverRideAutoRange(temp);

// Figure out data coordinate corresponding to mouse position	   
	xd = nint(curXAxis()->scrnToData(x));
	yd = nint(curYAxis()->scrnToData(y));

// Ignore command if mouse out of plot data region *******	
	if(xd < 0 || xd >= matWidth_ || yd < 0 || yd >= matHeight_)
	   return;
	   
// Now save the row in the plot display list *************
	if(mat_) // Matrix is real data
	{
	   float* xdata = MakeVector(visibleWidth_);
	   float* ydata = MakeVector(visibleWidth_);

      mapping = curXAxis()->mapping();
      if(mapping == PLOT_LINEAR_X)
      {	
         for(long i = visibleLeft_; i < visibleWidth_+visibleLeft_; i++)
         {
            if(xCalibrated_)
               xdata[i-visibleLeft_] = i*curXAxis()->length()/((float)matWidth_-1)+curXAxis()->base();
            else
               xdata[i-visibleLeft_] = i*curXAxis()->length()/(float)matWidth_+curXAxis()->base();
            ydata[i-visibleLeft_] = mat_[yd][i];
         }
      }
      else if(mapping == PLOT_LOG_X)
      {
         float right,left,result;
         for(long i = visibleLeft_; i < visibleWidth_+visibleLeft_; i++)
         {
            right = log10(curXAxis()->length() + curXAxis()->base());
            left = log10(curXAxis()->base());
            if(xCalibrated_)
               result = i*(right-left)/(float)(matWidth_-1)+left;
            else
               result = i*(right-left)/(float)matWidth_+left;
            xdata[i-visibleLeft_] = pow((float)10.0,(float)result);
            ydata[i-visibleLeft_] = mat_[yd][i];
         }
      }

		new TraceReal(xdata,ydata,visibleWidth_,name,cur1DPlot->getTracePar(),cur1DPlot,true);

	// Copy complete row to v1			
		var = AddGlobalVariable(MATRIX2D,"v1");
		var->MakeMatrix2DFromVector(&mat_[yd][0],matWidth_,1);
   }
   else if(cmat_) // Matrix is complex data
   {
	   float*   xdata = MakeVector(visibleWidth_);
	   complex* ydata = MakeCVector(visibleWidth_);

      mapping = curXAxis()->mapping();
      if(mapping == PLOT_LINEAR_X)
      {	
         for(long i = visibleLeft_; i < visibleWidth_+visibleLeft_; i++)
         {
            if(xCalibrated_)
               xdata[i-visibleLeft_] = i*curXAxis()->length()/((float)matWidth_-1)+curXAxis()->base();
            else
               xdata[i-visibleLeft_] = i*curXAxis()->length()/(float)matWidth_+curXAxis()->base();
            ydata[i-visibleLeft_] = cmat_[yd][i];
         }
      }
      else if(mapping == PLOT_LOG_X)
      {
         float right,left,result;
         for(long i = visibleLeft_; i < visibleWidth_+visibleLeft_; i++)
         {
            right = log10(curXAxis()->length() + curXAxis()->base());
            left = log10(curXAxis()->base());
            if(xCalibrated_)
               result = i*(right-left)/((float)matWidth_-1)+left;
            else
               result = i*(right-left)/(float)matWidth_+left;
            xdata[i-visibleLeft_] = pow((float)10.0,(float)result);
            ydata[i-visibleLeft_] = cmat_[yd][i];
         }
      }

		new TraceComplex(xdata,ydata,visibleWidth_,name,cur1DPlot->getTracePar(),cur1DPlot,true);

	// Copy complete row to v1	
		var = AddGlobalVariable(CMATRIX2D,"v1");
		var->MakeCMatrix2DFromCVector(&cmat_[yd][0],matWidth_,1);
   }
   else if(matRGB_) // Matrix is RGB data
   {
	   float* xrdata = MakeVector(visibleWidth_);
	   float* xgdata = MakeVector(visibleWidth_);
	   float* xbdata = MakeVector(visibleWidth_);
	   float* rData = MakeVector(visibleWidth_);
	   float* gData = MakeVector(visibleWidth_);
	   float* bData = MakeVector(visibleWidth_);
	
	   for(long i = visibleLeft_; i < visibleWidth_+visibleLeft_; i++)
	   {
	      xrdata[i-visibleLeft_] = i*curXAxis()->length()/(float)matWidth_+curXAxis()->base();
	      xgdata[i-visibleLeft_] = xrdata[i-visibleLeft_];
	      xbdata[i-visibleLeft_] = xrdata[i-visibleLeft_];
	      rData[i-visibleLeft_] = matRGB_[0][yd][i];
	      gData[i-visibleLeft_] = matRGB_[1][yd][i];
	      bData[i-visibleLeft_] = matRGB_[2][yd][i];
      }

		Trace* tempR = new TraceReal(xrdata,rData,visibleWidth_,name,cur1DPlot->getTracePar(),cur1DPlot,true);
		tempR->tracePar.setRealColor(RGB(255,0,0));

		Trace* tempG = new TraceReal(xgdata,gData,visibleWidth_,name,cur1DPlot->getTracePar(),cur1DPlot,true);
		tempG->tracePar.setRealColor(RGB(0,255,0));

		Trace* tempB = new TraceReal(xbdata,bData,visibleWidth_,name,cur1DPlot->getTracePar(),cur1DPlot,true);
		tempB->tracePar.setRealColor(RGB(0,0,255));

		cur1DPlot->setCurTrace(tempR);
      cur1DPlot->curTrace()->tracePar = tempR->tracePar;
	}

// Set mapping mode
    cur1DPlot->setAxisMapping(mapping);
    cur1DPlot->setAxisMapping(PLOT_LINEAR_Y);

// Update toolbar mapping icons
   WinData *parent = cur1DPlot->plotParent->obj->winParent;
	const char* const tbn = cur1DPlot->plotParent->toolbarName();
   if(mapping == PLOT_LOG_X)
   {
      parent->setToolBarItemCheck(tbn, "linear_axis", false);
      parent->setToolBarItemCheck(tbn, "log_axis", true);
   }
   else
   {
      parent->setToolBarItemCheck(tbn, "linear_axis", true);
      parent->setToolBarItemCheck(tbn, "log_axis", false);
   }

// Set 1D plot direction
   cur1DPlot->curXAxis()->setDirection(curXAxis()->plotDirection());
// Set 1D labels
   char* label = curXAxis()->label().text();
   cur1DPlot->curXAxis()->label().setText(label);
   cur1DPlot->curYAxis()->label().setText("Amplitude");

// Plot labels in 1D and 2D plots
   if(cur1DPlot->getOverRideAutoRange() == false)
   {
		for(Axis* axis: cur1DPlot->axisList())
		{
			axis->setAutorange(true);
		}
   }
   CText txt;
   txt.Format("row %ld of %s",yd,title().text());
	cur1DPlot->title().setText(txt.Str());
   txt.Format("row %ld",yd);
   UpdateStatusWindow(win,0,txt.Str());
}


/****************************************************************************
   Extract the visible column at position x from the current 2D data set and 
   display in the current 1d data set, also save complete column in vector v1
*****************************************************************************/

void Plot2D::ScanColumn(HWND hWnd, short x, short y, char *name)
{
	long xd,yd;
   HPEN cursorPen;
   Variable *var;
   short mapping = PLOT_LINEAR_X;
   extern bool gScrollWheelEvent;

	Plot1D* cur1DPlot = Plot1D::curPlot();

   if(!cur1DPlot) return;

   HDC hdc2D = GetDC(hWnd);

 // Make sure there is some data to plot **
   if(!DataPresent())
      return;

// Make sure this is not a vector plot ****
   if(vx_ || vy_)
      return;

// Select pens for cursor
   cursorPen = CreatePen(PS_DOT,0,RGB(0,0,0));
   SelectObject(hdc2D,cursorPen);
   SetROP2(hdc2D,R2_XORPEN);

// Remove last row cursor drawn
   if(colCursorVisible && !gScrollWheelEvent)
   {
	   MoveToEx(hdc2D,xold,dimensions.top(),0);
	   LineTo(hdc2D,xold,dimensions.top()+dimensions.height());
   }
   gScrollWheelEvent = false;

// Draw cursor at position (x,y)
	MoveToEx(hdc2D,x,dimensions.top(),0);
	LineTo(hdc2D,x,dimensions.top()+dimensions.height());
   ReleaseDC(hWnd,hdc2D);  
   DeleteObject(cursorPen);                          				                     
   
   colCursorVisible = true;   

   xold = x;
   yold = y;

// Clear all old data in 1D plot first   
   bool temp = cur1DPlot->getOverRideAutoRange();
	if(cur1DPlot->displayHold == false)
	   cur1DPlot->CloseCurrentData("all");
   else
	   cur1DPlot->CloseCurrentData("col");
   cur1DPlot->setOverRideAutoRange(temp);

// Figure out data coordinate corresponding to mouse position	   
	xd = nint(curXAxis()->scrnToData(x));
	yd = nint(curYAxis()->scrnToData(y));
	
	if(xd < 0 || xd >= matWidth_ || yd < 0 || yd >= matHeight_)
	   return;

// And plot column
	if(mat_) // Matrix is real data
   {	   
	   float* xdata = MakeVector(visibleHeight_);
	   float* ydata = MakeVector(visibleHeight_);

      mapping = curYAxis()->mapping();
      if(mapping == PLOT_LINEAR_Y)
      {
 	      for(long i = visibleTop_; i < visibleHeight_+visibleTop_; i++)
 	      {
            if(yCalibrated_)
               xdata[i-visibleTop_] = i*curYAxis()->length()/((float)matHeight_-1)+curYAxis()->base();	      	
            else
               xdata[i-visibleTop_] = i*curYAxis()->length()/(float)matHeight_+curYAxis()->base();	      	
            ydata[i-visibleTop_] = mat_[i][xd];
         }
         mapping = PLOT_LINEAR_X; 
      }
      else if(mapping == PLOT_LOG_Y)
      {
         float base,top,result;
         for(long i = visibleTop_; i < visibleHeight_+visibleTop_; i++)
         {
            top = log10(curYAxis()->length() + curYAxis()->base());
            base = log10(curYAxis()->base());
            if(yCalibrated_)
               result = i*(top-base)/((float)matHeight_-1)+base;
            else
               result = i*(top-base)/(float)matHeight_+base;
            xdata[i-visibleTop_] = pow((float)10.0,(float)result);
            ydata[i-visibleTop_] = mat_[i][xd];
         }
         mapping = PLOT_LOG_X; 

      }

		new TraceReal(xdata,ydata,visibleHeight_,name,cur1DPlot->getTracePar(),cur1DPlot,true);

	// Copy complete column to v1	
	   ydata = MakeVectorNR(0L,matHeight_-1);
    	for(long i = 0; i < matHeight_; i++)
	      ydata[i] = mat_[i][xd];		
		var = AddGlobalVariable(MATRIX2D,"v1");
		var->MakeMatrix2DFromVector(ydata,matHeight_,1);
		FreeVector(ydata);
	}
   else if(cmat_)
   {
	   float*   xdata = MakeVector(visibleHeight_);
	   complex* ydata = MakeCVector(visibleHeight_) ;

      mapping = curYAxis()->mapping();
      if(mapping == PLOT_LINEAR_Y)
      {
 	      for(long i = visibleTop_; i < visibleHeight_+visibleTop_; i++)
 	      {
            if(yCalibrated_)
               xdata[i-visibleTop_] = i*curYAxis()->length()/((float)matHeight_-1)+curYAxis()->base();	      	
            else
             xdata[i-visibleTop_] = i*curYAxis()->length()/(float)matHeight_+curYAxis()->base();	      	
            ydata[i-visibleTop_] = cmat_[i][xd];
         }
         mapping = PLOT_LINEAR_X; 
      }
      else if(mapping == PLOT_LOG_Y)
      {
         float base,top,result;
         for(long i = visibleTop_; i < visibleHeight_+visibleTop_; i++)
         {
            top = log10(curYAxis()->length() + curYAxis()->base());
            base = log10(curYAxis()->base());
            if(yCalibrated_)
               result = i*(top-base)/((float)matHeight_-1)+base;
            else
               result = i*(top-base)/(float)matHeight_+base;
            xdata[i-visibleTop_] = pow((float)10.0,(float)result);
            ydata[i-visibleTop_] = cmat_[i][xd];
         }
         mapping = PLOT_LOG_X; 
      }
   	   
		new TraceComplex(xdata,ydata,visibleHeight_,name,cur1DPlot->getTracePar(),cur1DPlot,true);

	// Copy complete column to v1	
	   complex *yc = MakeCVector(matHeight_);
    	for(long i = 0; i < matHeight_; i++)
    	{
	      yc[i] = cmat_[i][xd];
	   }		
		var = AddGlobalVariable(CMATRIX2D,"v1");
		var->MakeCMatrix2DFromCVector(yc,matHeight_,1);
		FreeCVector(yc);
   }
   else if(matRGB_) // Matrix is RGB data
   {
	   float* xrdata = MakeVector(visibleHeight_);
	   float* xgdata = MakeVector(visibleHeight_);
	   float* xbdata = MakeVector(visibleHeight_);
	   float* rData = MakeVector(visibleHeight_);
	   float* gData = MakeVector(visibleHeight_);
	   float* bData = MakeVector(visibleHeight_);
	
	   for(long i = visibleTop_; i < visibleHeight_+visibleTop_; i++)
	   {
	      xrdata[i-visibleTop_] = i*curYAxis()->length()/(float)matHeight_+curYAxis()->base();
	      xgdata[i-visibleTop_] = xrdata[i-visibleTop_];
	      xbdata[i-visibleTop_] = xrdata[i-visibleTop_];
	      rData[i-visibleTop_] = matRGB_[0][i][xd];
	      gData[i-visibleTop_] = matRGB_[1][i][xd];
	      bData[i-visibleTop_] = matRGB_[2][i][xd];
      }

		Trace* tempR = new TraceReal(xrdata,rData,visibleHeight_,name,cur1DPlot->getTracePar(),cur1DPlot,true);
		tempR->tracePar.setRealColor(RGB(255,0,0));

		Trace* tempG = new TraceReal(xgdata,gData,visibleHeight_,name,cur1DPlot->getTracePar(),cur1DPlot,true);
		tempG->tracePar.setRealColor(RGB(0,255,0));

		Trace* tempB = new TraceReal(xbdata,bData,visibleHeight_,name,cur1DPlot->getTracePar(),cur1DPlot,true);
		tempB->tracePar.setRealColor(RGB(0,0,255));

		cur1DPlot->setCurTrace(tempR);
      cur1DPlot->curTrace()->tracePar = tempR->tracePar;
   }

// Plot labels in 1D and 2D plots
   if(cur1DPlot->getOverRideAutoRange() == false)
   {
		for(Axis* axis: cur1DPlot->axisList())
		{
			axis->setAutorange(true);
		}
	}
   
// Set 1D mapping mode
   cur1DPlot->setAxisMapping(PLOT_LINEAR_Y);
   cur1DPlot->setAxisMapping(mapping);

// Update toolbar mapping icons
   WinData *parent = cur1DPlot->plotParent->obj->winParent;
	const char* const tbn = cur1DPlot->plotParent->toolbarName();
   if(mapping == PLOT_LOG_X)
   {
      parent->setToolBarItemCheck(tbn, "linear_axis", false);
      parent->setToolBarItemCheck(tbn, "log_axis", true);
   }
   else
   {
      parent->setToolBarItemCheck(tbn, "linear_axis", true);
      parent->setToolBarItemCheck(tbn, "log_axis", false);
   }

// Set 1D plot direction
   cur1DPlot->curXAxis()->setDirection(curYAxis()->plotDirection());

// Set 1D labels
   char* label = yAxisLeft()->label().text();
   cur1DPlot->curXAxis()->label().setText(label);
   cur1DPlot->curYAxis()->label().setText("Amplitude");

   CText txt;
   txt.Format("column %ld of %s",xd,Plot2D::curPlot()->title().text());
	cur1DPlot->setTitleText(txt.Str());
   txt.Format("column %ld",xd);
   UpdateStatusWindow(win,0,txt.Str());
}

void Plot2D::ShiftPlot(short dir)
{
   long shiftx,shifty;
	const short horiz = 20;
	const short vert = 20;

   shiftx = visibleWidth()/horiz;
   if(shiftx == 0) shiftx = 1;
   
   shifty = visibleHeight()/vert;
   if(shifty == 0) shifty = 1;
   
   switch(dir)
   {
      case(ID_SHIFT_LEFT):
         setVisibleLeft(visibleLeft() - shiftx);
         break;
      case(ID_SHIFT_RIGHT):
         setVisibleLeft(visibleLeft() + shiftx);
         break; 
       case(ID_SHIFT_UP):
         setVisibleTop(visibleTop() + shifty);
         break;
      case(ID_SHIFT_DOWN):
         setVisibleTop(visibleTop() - shifty);
         break; 
   }
   
   if(visibleLeft() < 0) 
	{
		setVisibleLeft(0);
	}
   if(visibleLeft()+visibleWidth() > matWidth_) 
	{
		setVisibleLeft(matWidth_-visibleWidth());
	}

   if(visibleTop() < 0) 
	{
		setVisibleTop(0);
	}
   if(visibleTop()+visibleHeight() > matHeight_) 
	{
		setVisibleTop(matHeight_-visibleHeight());
	}
	MyInvalidateRect(plotParent->hWnd,NULL,false);	                 
}


bool Plot2D::dataCursorVisible()
{
	return dataCursorVisible_;
}


void Plot2D::ClearCursors(HDC hdc, MouseMode mouseMode)
{
	switch(mouseMode)
	{
		case(SHOW_DATA):
	      HideDataCursor(hdc);	
			break;                    
		case(SELECT_RECT):
		   HideSelectionRectangle(hdc);	
			break; 
		case(SELECT_COLUMN):
			HideColumnCursor(hdc);	
			break; 
		case(SELECT_ROW):
			HideRowCursor(hdc);	
			break;
	}
}

bool Plot2D::OKForLog(char dir)
{
	return false;
}

void Plot2D::displayToEMF(HDC hdc, HWND hWnd)
{
	scale(gPlotSF);
	Display(hWnd, hdc);
   if(displayColorScale_ && plotParent->showLabels)
      DrawColorScale(hdc, plotParent->bitmap);

	unscale();
}


void Plot2D::clearData(void)
{
	// Does not clear color map.
   if(mat_)       FreeMatrix2D(mat_);
   if(cmat_)      FreeCMatrix2D(cmat_);
   if(matRGB_)    FreeMatrix3D(matRGB_);
   if(vx_)        FreeMatrix2D(vx_);
   if(vy_)        FreeMatrix2D(vy_);
   if(contourLevels) delete [] contourLevels;
   if(xAxis) delete [] xAxis;
   if(yAxis) delete [] yAxis;

   mat_  = NULL;    
   matRGB_ = NULL;
   cmat_ = NULL;    
   vx_   = NULL;    
   vy_   = NULL;
   contourLevels = NULL;

   matWidth_ = 0;
   matHeight_ = 0;

	clearLabels();
}


void Plot2D::OffsetData(HDC hdc, long x, long y)
{
// Remove the offset if it is already present
   if(isOffset_)
   {
      xOffset_ = 0;
      yOffset_ = 0;
      xOffsetIndex_ = 0;
      yOffsetIndex_ = 0;
      isOffset_ = false;
      return;
   }

// Find selected point in data and user coordinates
   if(mat_ || cmat_ || matRGB_) 
   {
      long xd = curXAxis()->scrnToData(x);
      long yd = curYAxis()->scrnToData(y);
      float xu = curXAxis()->dataToUser(xd);
      float yu = curYAxis()->dataToUser(yd);

// Record this as the offset
      xOffset_ = xu;
      yOffset_ = yu;
      xOffsetIndex_ = xd;
      yOffsetIndex_ = yd;
      isOffset_ = true;
   }   
}


/**********************************************************************************
*                    Draw the background and border for plotting                  *
**********************************************************************************/

void Plot2D::DrawPlotBackGround(HDC hdc)
{
   RECT r;
	HBRUSH bkBrush = GetPlotBackgroundBrush();
	HBRUSH borderBrush = GetPlotBorderBrush();

	SaveDC(hdc);

// Fill plot region with background color
   if(gPlotMode != PRINT)
   {
      SetRect(&r,dimensions.left(),dimensions.top(),dimensions.left()+dimensions.width(),dimensions.top()+dimensions.height());
      FillRect(hdc,&r,bkBrush);
   }
   
// Draw border
   if(plotParent->showLabels)
   {     
	   SetRect(&r,regionLeft,regionTop,dimensions.left(),regionBottom);
	   FillRect(hdc,&r,borderBrush); // Left strip
	   SetRect(&r,dimensions.left(),regionTop,regionRight,dimensions.top());
	   FillRect(hdc,&r,borderBrush); // Top strip
	   SetRect(&r,dimensions.left(),dimensions.top()+dimensions.height(),regionRight,regionBottom);
	   FillRect(hdc,&r,borderBrush); // Bottom strip 
	   
		if(displayColorScale_)
		{
		   long ww = regionRight - regionLeft;
		   long xl = dimensions.width() + dimensions.left() + ww*0.04;
		   long xr = dimensions.width()  + dimensions.left() + ww*0.10;

	      SetRect(&r,dimensions.left()+dimensions.width(),dimensions.top(),xl,dimensions.bottom()+1);
	      FillRect(hdc,&r,borderBrush); // Left of scale

	      SetRect(&r,xr,dimensions.top(),regionRight,dimensions.bottom()+1);
	      FillRect(hdc,&r,borderBrush); // Right of scale
		   
		}
		else
		{
	      SetRect(&r,dimensions.left()+dimensions.width(),dimensions.top(),regionRight,dimensions.bottom()+1);
	      FillRect(hdc,&r,borderBrush); // Right strip
      }		
	   
   }
   DeleteObject(bkBrush);
	DeleteObject(borderBrush);
   RestoreDC(hdc,-1);
}

const float horiz_enlarge = 1.1;
const float vert_enlarge  = 1.1;
const float horiz_reduce  = 0.9;
const float vert_reduce   = 0.9;

void Plot2D::mouseZoom(bool in, HWND hWnd)
{
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
	const int MAGIC_PLOTHEIGHT_FUDGE = 7;
	int windowTop = tb.bottom ? tb.bottom : plotParent->obj->yo;

	// As a proportion of plot dimensions, how far to the right...
	float relative_width = 
		((float)(pt.x - dimensions.left() - plotParent->obj->xo) / (float)dimensions.width());
	// ... or to the bottom ...

	float relative_height = 
		((float)(pt.y - dimensions.top() - windowTop - MAGIC_PLOTHEIGHT_FUDGE)  / (float)dimensions.height());
   // ... is the mouse pointer?	

	// Ensure we're inside the plot.
	if ((relative_width > 1) ||
		(relative_width < 0) ||
		(relative_height > 1) ||
		(relative_height < 0))
		return;

	float vert_factor = in ? vert_enlarge : vert_reduce;
	float horiz_factor = in ? horiz_enlarge : horiz_reduce;

   Plot *pd = dynamic_cast<Plot*>(this);
   PlotDirection pdx = pd->curXAxis()->plotDirection();
   PlotDirection pdy = pd->curYAxis()->plotDirection();

	long oldVisibleWidth = visibleWidth();
   setVisibleWidth(nint(visibleWidth()*horiz_factor));
	long visibleWidthChange = oldVisibleWidth - visibleWidth();
   if(pdx == PlotDirection::PLT_FORWARD)
      setVisibleLeft(visibleLeft() + nint(visibleWidthChange * (relative_width)));
   else
      setVisibleLeft(visibleLeft() + nint(oldVisibleWidth - visibleWidth() - visibleWidthChange * (relative_width)));
	long oldVisibleHeight = visibleHeight();
   setVisibleHeight(nint(visibleHeight()*vert_factor));
   long visibleHeightChange = oldVisibleHeight - visibleHeight();
   if(pdy == PlotDirection::PLT_FORWARD)
      setVisibleTop(visibleTop() + nint(visibleHeightChange * (1-relative_height))); 
   else
      setVisibleTop(visibleTop() + nint(oldVisibleHeight - visibleHeight() - visibleHeightChange * (1-relative_height))); 
}


void Plot2D::ScalePlot(HWND hWnd, short dir)
{  
	long visibleHCentre = visibleLeft() + visibleWidth()/2;
   long visibleVCentre = visibleTop() + visibleHeight()/2;

	switch(dir)
   {
      case(ID_REDUCE_HORIZ):
         setVisibleWidth(nint(visibleWidth()*horiz_enlarge));
         setVisibleLeft(visibleHCentre - visibleWidth()/2);
         break;
      case(ID_REDUCE_VERT):
         setVisibleHeight(nint(visibleHeight()*vert_enlarge));
         setVisibleTop(visibleVCentre - visibleHeight()/2);      
         break; 
      case(ID_REDUCE_BOTH):
			mouseZoom(false, hWnd);
         break;  
       case(ID_ENLARGE_HORIZ):
         setVisibleWidth(nint(visibleWidth()*horiz_reduce));
         setVisibleLeft(visibleHCentre - visibleWidth()/2);   
         break;
      case(ID_ENLARGE_VERT):
         setVisibleHeight(nint(visibleHeight()*vert_reduce));
         setVisibleTop(visibleVCentre - visibleHeight()/2);      
         break; 
      case(ID_ENLARGE_BOTH):
			mouseZoom(true, hWnd);
			break;      
   }
   
// Ensure that zoomed region is not too small or too large
// (make 8 points the minimum size)
   if(visibleWidth() < 8) 
	{
		setVisibleWidth(8);
	}
   if(visibleWidth() > matWidth_) 
	{
		setVisibleWidth(matWidth_);
	}

   if(visibleHeight() < 8)
	{
		setVisibleHeight(8);
	}
   if(visibleHeight() > matHeight_)
	{
		setVisibleHeight(matHeight_);
	}

   if(visibleLeft()+visibleWidth() > matWidth_) 
	{
		setVisibleLeft(matWidth_-visibleWidth());
	}
	if(visibleLeft() < 0)
	{
		setVisibleLeft(0);
	}

   if(visibleTop()+visibleHeight() > matHeight_) 
	{
		setVisibleTop(matHeight_-visibleHeight());
	}
   if(visibleTop() < 0)
	{
		setVisibleTop(0);
	}

	curXAxis()->setMin(visibleLeft());
   curXAxis()->setMax(visibleLeft()+visibleWidth());
   curYAxis()->setMin(visibleTop());
   curYAxis()->setMax(visibleTop()+visibleHeight());
   
	MyInvalidateRect(plotParent->hWnd,NULL,false);	     
}


void Plot2D::ChooseDefaultColours()
{
	for(Axis* axis: axisList_)
	{
		axis->grid()->setColor(pwd->gridColor);
		axis->grid()->setFineColor(pwd->fineGridColor);
		axis->ticks().setFontColor(pwd->ticks->fontColor());
		axis->label().setFontColor(pwd->labelFontColor);
	}
	this->axesColor       = pwd->axesColor;
	this->bkColor         = pwd->bkColor;
	this->borderColor     = pwd->borderColor;
	this->title().setFontColor(pwd->titleFontColor);   
}


/********************************************************************
  Display the current image stored in the 2D matrix attached to the 
  plot structure into window hWnd.
*********************************************************************/
long Plot2D::Display2DImageNullBMP(HWND hWnd, HDC hdc)
{
   HPEN axes_Color;

// Save current device context ********************
   SaveDC(hdc);

// Initialize position of text objects ************
   ResetTextCoords();

// Setup plot window dimensions *******************
   ResizePlotRegion(hWnd);

// Make some pens and brushes *********************
   if(gPlotMode == PRINT && gPrintMode == BLACK_AND_WHITE)
   {
	   axes_Color  = CreatePen(PS_SOLID,0,RGB(0,0,0));
   }
   else
   {
	   axes_Color  = CreatePen(PS_SOLID,0,axesColor);
	}

// Draw background ********************************
   DrawPlotBackGround(hdc);  

// Get data range if required *********************
   if((mat_ || cmat_) && !getOverRideAutoRange())
   {
	   minVal_ = 1e9;
	   maxVal_ = -1e9;

      FindMatrixRange(minVal_,maxVal_);
	   
	// In case the data is flat
	   if(maxVal_ == minVal_)
	   {
	      minVal_ = minVal_ - 1;
	      maxVal_ = maxVal_ + 1;
	   }
	}
	  

// Draw axes if desired ***************************
   if(plotParent->showLabels && (mat_ || cmat_ || matRGB_ || (vx_ && vy_)))
   {
      DrawAxes(hdc);	 
   }
   
        
// Draw bounding rectangle ************************
   DrawBoundingRectange(hdc,axes_Color);

// Draw Interpolated image if desired *************
   if((mat_ || cmat_) && (drawMode_ & DISPLAY_INTERP_CONTOURS))
      ContourPlotIntensity(hWnd, hdc);

// Draw Contours if desired ***********************
   if((mat_ || cmat_)  && (drawMode_ & DISPLAY_CONTOURS))
       ContourPlot(hWnd, hdc);

// Draw Vectors if desired ************************
   if((vx_ && vy_)  && (drawMode_ & DISPLAY_VECTORS))
      VectorPlot(hWnd, hdc);

// Draw labels ************************************
   WritePlotLabels(hdc);

	// Draw insets
	for(Inset* inset: insets_)
	{
		inset->draw(hdc);
	}

// Draw the selection rectangle (if any) **********
   DrawSelectionRectangle(hdc); 
      
// Update the status bar with matrix type and dimensions
	if(this == Plot2D::curPlot())
      plotParent->UpdateStatusBar();

// Tidy up ****************************************   
   RestoreDC(hdc,-1); 
   DeleteObject(axes_Color);

   return(OK);
}


/********************************************************************
  Display the current image stored in the 2D matrix attached to the 
  plot structure into window hWnd.
*********************************************************************/

long Plot2D::Display(HWND hWnd, HDC hdc)
{
   HPEN axes_Color;
	HBITMAP bitmap = this->plotParent->bitmap;

// Save current device context ********************
   SaveDC(hdc);

// Initialize position of text objects ************
   ResetTextCoords();

// Setup plot window dimensions *******************
   ResizePlotRegion(hWnd);

// Make some pens and brushes *********************
   if(gPlotMode == PRINT && gPrintMode == BLACK_AND_WHITE)
   {
	   axes_Color  = CreatePen(PS_SOLID,curXAxis()->lineWidth(),RGB(0,0,0));
   }
   else
   {
	   axes_Color  = CreatePen(PS_SOLID,curXAxis()->lineWidth(),axesColor);
	}

// Draw background ********************************
   DrawPlotBackGround(hdc);  

// Get data range if required *********************
   if((mat_ || cmat_) && !getOverRideAutoRange())
   {
	   minVal_ = 1e9;
	   maxVal_ = -1e9;

      FindMatrixRange(minVal_,maxVal_);
	   
	// In case the data is flat
	   if(maxVal_ == minVal_)
	   {
	      minVal_ = minVal_ - 1;
	      maxVal_ = maxVal_ + 1;
	   }
	}

	
	curXAxis()->setMin(visibleLeft());
   curXAxis()->setMax(visibleLeft()+visibleWidth());
   curYAxis()->setMin(visibleTop());
   curYAxis()->setMax(visibleTop()+visibleHeight());
   
	  
// Draw image if desired **************************
   if((mat_ || cmat_) && (drawMode_ & DISPLAY_IMAGE) && bitmap)
   {
	   FillBitMapWithImage(bitmap);	 
   }

   if(matRGB_ && (drawMode_ & DISPLAY_IMAGE) && bitmap)
   {
	   FillBitMapWithRGBImage(bitmap);	 
   }

// Draw axes if desired ***************************
   if(plotParent->showLabels && (mat_ || cmat_ || matRGB_ || (vx_ && vy_)))
   {
      DrawAxes(hdc);	 
   }

	curXAxis()->grid()->draw(hdc);
	curYAxis()->grid()->draw(hdc);
   
// Display color scale if desired *****************
   if(displayColorScale_ && plotParent->showLabels && bitmap && (gPlotMode == DISPLAY || gPlotMode == CLIPBOARD))
   {
      DrawColorScale(hdc,bitmap); 
   }
         
// Draw Interpolated image if desired *************
   if((mat_ || cmat_) && (drawMode_ & DISPLAY_INTERP_CONTOURS))
      ContourPlotIntensity(hWnd, hdc);

// Draw Contours if desired ***********************
   if((mat_ || cmat_)  && (drawMode_ & DISPLAY_CONTOURS))
       ContourPlot(hWnd, hdc);

// Draw Vectors if desired ************************
   if((vx_ && vy_)  && (drawMode_ & DISPLAY_VECTORS))
      VectorPlot(hWnd, hdc);

// Draw Interpolated image if desired *************
   if((mat_) && (drawMode_ & DISPLAY_WATERFALL))
      WaterFallPlot(hWnd, hdc);

// Draw labels ************************************
   WritePlotLabels(hdc);

// Draw lines
	for(PlotLine* line: lines_)
	{
		line->Draw(this,hdc,2);
	}

// Draw text
	for(PlotText* pltTxt : text_)
	{
		pltTxt->Draw(this,hdc,1);
	}

// Draw insets
	for(Inset* inset: insets_)
	{
		inset->draw(hdc);
	}

// Draw the selection rectangle (if any) **********
   DrawSelectionRectangle(hdc); 
      
// Draw bounding rectangle ************************
   DrawBoundingRectange(hdc,axes_Color);

// Update the status bar with matrix type and dimensions
	if(this == Plot2D::curPlot())
      plotParent->UpdateStatusBar();

// Tidy up ****************************************   
   RestoreDC(hdc,-1); 
   DeleteObject(axes_Color);

   return(OK);
}



void Plot2D::VectorPlot(HWND hWnd, HDC hdc)
{
  float pi = 3.1415927;

  SaveDC(hdc);

// Create the clipping region *********************************
  HRGN hClpRgn;

   if(plotParent->showLabels == true)
   {
	   hClpRgn = CreateRectRgn(dimensions.left()+1,dimensions.top()+1,
	                           dimensions.left()+dimensions.width(),
	                           dimensions.top()+dimensions.height());
	}
   else
   {
	   hClpRgn = CreateRectRgn(dimensions.left()+1,dimensions.top()+1,
	                           dimensions.left()+dimensions.width(),
	                           dimensions.top()+dimensions.height()-2);
   }
 	SelectClipRgn(hdc, hClpRgn);

	float maxLen = Plot2D::curPlot()->maxVal_;

// Work out scale factor so longest vector has length "vectorLength" pixels

	// FIXME: This block is immediately blown away by the assignments in the 
	//  following block.
  float sf = vectorLength_/maxLen;
  long xstep = vectorLength_/(curXAxis()->dataToScrn(1) - curXAxis()->dataToScrn(0));
  if(xstep < 1) xstep = 1;
  long ystep = vectorLength_/(curYAxis()->dataToScrn(0) - curYAxis()->dataToScrn(1));
  if(ystep < 1) ystep = 1;
  
  xstep = xVectorStep;
  ystep = yVectorStep;
  
// Draw arrows/vectors
  float ct = cos(30*pi/180);
  float st = sin(30*pi/180);
  float arrowLen = 5;
            
  for(long yd = visibleTop(); yd < visibleTop()+visibleHeight(); yd += ystep)
  {
     for(long xd = visibleLeft(); xd < visibleLeft()+visibleWidth(); xd += xstep)
     {
        long xs = curXAxis()->dataToScrn(xd); // (xs,ys) : coords of start of arrow
        long ys = curYAxis()->dataToScrn(yd);
        
        float xl = vx_[yd][xd];
        float yl = vy_[yd][xd];
        float tl = sqrt(xl*xl + yl*yl); // Length of arrow
          
        long xv = xs + hint(xl*sf); // (xv,yv) : coords of end of arrow
        long yv = ys - hint(yl*sf);

		  float level;
        if(ColorScaleType() == PLUS_MINUS_CMAP)
           level = 0.5+0.5*tl/maxLen;
        else
           level = tl/maxLen;
        COLORREF col = GetColor(SCALE, level, 10000);
      
		  HPEN vColor  = CreatePen(PS_SOLID,0,col);
        HPEN oldPen = (HPEN)SelectObject(hdc,vColor);
                
	     MoveToEx(hdc,xs,ys,NULL); // Draw shaft of arrow
	     LineTo(hdc,xv,yv);
	  
	  // Arrow head position at (x1,y1) = (xv - cos(theta-phi)*length, yv - sin(theta-phi)*length)
	  // Arrow head position at (x2,y2) = (xv - cos(theta+phi)*length, yv - sin(theta+phi)*length)
	     
	     long x1 = xv - nint((ct*xl + st*yl)*arrowLen/tl); // Arrow head coordinates.
	     long y1 = yv - nint((st*xl - ct*yl)*arrowLen/tl);
	     long x2 = xv - nint((ct*xl - st*yl)*arrowLen/tl);
	     long y2 = yv + nint((st*xl + ct*yl)*arrowLen/tl);
	     
	     MoveToEx(hdc,xv,yv,NULL); // Draw arrow head
	     LineTo(hdc,x1,y1);
	     MoveToEx(hdc,xv,yv,NULL);
	     LineTo(hdc,x2,y2);
        SelectObject(hdc,oldPen);
        DeleteObject(vColor); 
	     
     }
  }
  RestoreDC(hdc, -1);
  DeleteObject(hClpRgn);  
}

/****************************************************************
   Make contour plot of the current image using GDI+
****************************************************************/

void Plot2D::ContourPlot(HWND hWnd, HDC hdc)
{
   float base;                // Base for defining noise levels           
   float del_x,del_y;         // Number of screen pixels per data point   
   float lev;                 // Current contour level                    
   float ll,lr;               // Intensity of left corners of cell        
   float top;                 // Top of noise levels                      
   float ul,ur;               // Intensity of right corners of cell       
   float xc[4],yc[4];         // Contour coordinates for the current cell 
   long cnt;                  // Number of contours cutting cell          
   long flip = 0;             // Line counter  (0 = odd 1 == even line)   
   long i;                    // Contour counter                          
   long left_edge,right_edge; // Edge offsets for calculating spectral    
   long s_width,s_height;     // Spectral region dimensions  and offsets  
   float shift_x,shift_y;     // 1/2 data point shift for contours        
   long spec_height;
   long spec_width;           // Spectrum dimensions                      
   long sx_off,sy_off;
   long test;                 // Number of corners less than contour      
   long top_edge,bottom_edge; // region (to allow for edge plotting)      
   long width;                // Region screen dimensions and offsets  
   long height;               // width and height in pixels   
   long x_base,y_base;        // Max and min spectral roi coordinates     
   long x_top,y_top;
   long x_width,y_height;     // Width and height of spectral roi         
   register long x;           // x spectral counter                       
   register long y;           // y spectral counter                       
   float *rowE;
   float *rowO;
   float *xgrid;
   float *ygrid;
   short nrLev;
   float *levels;
   float frac;
   Pen **contPen;
   Color col;
   float lineWidth = this->contourLineWidth;
   
// Save current device context ******************************** 
   SaveDC(hdc);

// Create the clipping region *********************************
   Rect clipRect;

   if(plotParent->showLabels == true)
   {
	   clipRect.X = dimensions.left();
      clipRect.Y = dimensions.top();
      clipRect.Width = dimensions.width();
      clipRect.Height = dimensions.height();
	}
   else
   {
	   clipRect.X = dimensions.left();
      clipRect.Y = dimensions.top();
      clipRect.Width = dimensions.width();
      clipRect.Height = dimensions.height()-2;
   }
 	
// Allocate memory for rows of contour data *******************
   if((rowE = MakeVector(matWidth_*2)) == NULL)
   {
       ErrorMessage("Unable to allocate memory for contour data");
       RestoreDC(hdc,-1); 
       return;
   } 

   if((rowO = MakeVector(matWidth_*2)) == NULL)
   {
       ErrorMessage("Unable to allocate memory for contour data");
       FreeVector(rowE);
       RestoreDC(hdc,-1); 
       return;
   } 

   if((xgrid = MakeVector(matWidth_*2)) == NULL)
   {
       ErrorMessage("Unable to allocate memory for contour data");
       FreeVector(rowE);
       FreeVector(rowO);
       RestoreDC(hdc,-1); 
       return;
   }      

   if((ygrid = MakeVector(matHeight_*2)) == NULL)
   {
       ErrorMessage("Unable to allocate memory for contour data");
       FreeVector(rowE);
       FreeVector(rowO);
       FreeVector(xgrid);
       RestoreDC(hdc,-1);        
       return;
   }

// Define GDI+ graphics object
   Graphics gfx(hdc); 
   gfx.SetSmoothingMode(SmoothingMode::SmoothingModeAntiAlias);
   gfx.SetClip(clipRect);


// Determine screen & data region dimensions *******************
   width  = dimensions.width();
   height = dimensions.height();

   sx_off   = visibleLeft();
   sy_off   = visibleTop();
   s_width  = visibleWidth();
   s_height = visibleHeight();

   spec_width  = matWidth_;
   spec_height = matHeight_;

// Adjust roi to include 1 data point around edges *************
   top_edge    = 1;
   bottom_edge = 1;
   left_edge   = 1;
   right_edge  = 1;

   if(sx_off + s_width  == spec_width)  right_edge = 0;	
   if(sy_off + s_height == spec_height) top_edge = 0;
   if(sx_off == 0)                      left_edge = 0;
   if(sy_off == 0)                      bottom_edge = 0;

   x_base   = sx_off - left_edge;
   y_base   = sy_off - bottom_edge;
   x_width  = s_width + right_edge + left_edge;
   y_height = s_height + top_edge + bottom_edge;
   x_top    = x_base + x_width;
   y_top    = y_base + y_height;

// Calculate x and y coordinates of each cell ****************
   shift_x = 0.5*(float)width/(float)(s_width) + dimensions.left(); 
   shift_y = (float)height - 0.5*(float)height/(float)(s_height) + dimensions.top();

   assert(!(x_base < 0));
   assert(!(y_base < 0));
   assert(!(x_top >= matWidth_*2));
   assert(!(y_top >= matHeight_*2));

   for(x = x_base; x < x_top; x++)
      xgrid[x] = (float)(x-sx_off)*(float)width/(float)s_width + shift_x;

   for(y = y_base; y < y_top; y++)
      ygrid[y] = shift_y - (float)(y-sy_off)*(float)height/(float)s_height;
     
   del_x =  (float)width/(float)s_width;
   del_y =  (float)height/(float)s_height;

   y_top--;

// Define the contour levels and pens for drawing ************/
   if(!contourLevels) // Automatically define contour levels
   {
      if(ColorScaleType() == PLUS_MINUS_CMAP)
      {      
         nrLev = nrContourLevels*2;
         levels = new float[nrLev];
         contPen = new Pen*[nrLev];

         for(i = 0; i < nrLev; i++)
         {
          //  contPen[i] = ::new Pen(Color());
            contPen[i] = new Pen(Color());
            contPen[i]->SetWidth(lineWidth);
            contPen[i]->SetDashStyle(Gdiplus::DashStyle::DashStyleSolid);
            contPen[i]->SetLineJoin(Gdiplus::LineJoinBevel);
         }

	      for(i = 0; i < nrLev/2; i++) // -ve levels
	      {   
				if(dataMapping == LINEAR_MAPPING)
	            levels[i] = (float)(i)*(maxVal_-minVal_)/(nrLev/2-1) - maxVal_;
				else
				{
					if(maxVal_ > 0 && minVal_ > 0)
	               levels[i] = -exp(-((float)(i)*(log(maxVal_)-log(minVal_))/(nrLev/2-1) - log(maxVal_)));
					else
	               levels[i] = (float)(i)*(maxVal_-minVal_)/(nrLev/2-1) - maxVal_;
				}
            if(useFixedContourColor())
                col.SetFromCOLORREF(fixedContourColor());
            else
	             col.SetFromCOLORREF(GetColor(SCALE,(float)i/nrLev, nrLev));

		      contPen[i]->SetColor(col);
         }
	      for(i = nrLev/2; i < nrLev; i++) // +ve levels
	      {   
				if(dataMapping == LINEAR_MAPPING)
	            levels[i] = (float)(i-nrLev/2)*(maxVal_-minVal_)/(nrLev/2-1) + minVal_;
				else
				{
					if(maxVal_ > 0 && minVal_ > 0)
	               levels[i] = exp((float)(i-nrLev/2)*(log(maxVal_)-log(minVal_))/(nrLev/2-1) + log(minVal_));
					else
	               levels[i] = (float)(i-nrLev/2)*(maxVal_-minVal_)/(nrLev/2-1) + minVal_;
				}

            if(useFixedContourColor())
               col.SetFromCOLORREF(fixedContourColor());
            else
	            col.SetFromCOLORREF(GetColor(SCALE,(float)i/nrLev, nrLev));

		      contPen[i]->SetColor(col);
         }
      }
      else // Positive color scale only
      {
         nrLev = nrContourLevels;
         levels = new float[nrLev];
         contPen = new Pen*[nrLev];

         for(i = 0; i < nrLev; i++)
         {
        //    contPen[i] = ::new Pen(Color());
            contPen[i] = new Pen(Color());
            contPen[i]->SetWidth(lineWidth);
            contPen[i]->SetDashStyle(Gdiplus::DashStyle::DashStyleSolid);
            contPen[i]->SetLineJoin(Gdiplus::LineJoinBevel);
         }

	      for(i = 0; i < nrLev; i++)
	      {   
				if(dataMapping == LINEAR_MAPPING)
	            levels[i] = (float)i/nrLev*(maxVal_-minVal_) + minVal_;
				else
				{
					if(maxVal_ > 0 && minVal_ > 0)
					   levels[i] = exp((float)i/nrLev*(log(maxVal_)-log(minVal_)) + log(minVal_));
					else
	               levels[i] = (float)i/nrLev*(maxVal_-minVal_) + minVal_;
				}

            if(useFixedContourColor())
               col.SetFromCOLORREF(fixedContourColor());
            else
	            col.SetFromCOLORREF(GetColor(SCALE,(float)i/nrLev, nrLev));
		      contPen[i]->SetColor(col);
		   }
      } 
   }
   else // Contour levels are define
   {
      if(ColorScaleType() == PLUS_MINUS_CMAP)
      {      
         nrLev = nrContourLevels*2;
         levels = new float[nrLev];
         contPen = new Pen*[nrLev];

         for(i = 0; i < nrLev; i++)
         {
          //  contPen[i] = ::new Pen(Color());
            contPen[i] = new Pen(Color());
            contPen[i]->SetWidth(lineWidth);
            contPen[i]->SetDashStyle(Gdiplus::DashStyle::DashStyleSolid);
            contPen[i]->SetLineJoin(Gdiplus::LineJoinBevel);
         }

	      for(i = 0; i < nrLev/2; i++) // -ve levels
	      {   
				if(dataMapping == LINEAR_MAPPING)
               frac = (contourLevels[i]-minVal_)/(maxVal_-minVal_);
				else
				{
					if(maxVal_ > 0 && minVal_ > 0)
                  frac = exp((contourLevels[i]-log(minVal_))/(log(maxVal_)-log(minVal_)));
					else
                  frac = (contourLevels[i]-minVal_)/(maxVal_-minVal_);
				}
            if(useFixedContourColor())
               col.SetFromCOLORREF(fixedContourColor());
            else
	            col.SetFromCOLORREF(GetColor(SCALE,frac, nrLev));
            Color gdiCol;
		      contPen[i]->SetColor(col);	
         }
	      for(i = nrLev/2; i < nrLev; i++) // +ve levels
	      {   
				if(dataMapping == LINEAR_MAPPING)
               frac = (contourLevels[i]-minVal_)/(maxVal_-minVal_);
				else
				{
					if(maxVal_ > 0 && minVal_ > 0)
                  frac = exp((contourLevels[i]-log(minVal_))/(log(maxVal_)-log(minVal_)));
					else
                  frac = (contourLevels[i]-minVal_)/(maxVal_-minVal_);
				}

            if(useFixedContourColor())
                col.SetFromCOLORREF(fixedContourColor());
            else
	             col.SetFromCOLORREF((GetColor(SCALE,frac, nrLev)));
		      contPen[i]->SetColor(col);	
		   }
      }
      else // Positive color scale
      {
         nrLev = nrContourLevels;
         levels = new float[nrLev];
         contPen = new Pen*[nrLev];

         for(i = 0; i < nrLev; i++)
         {
         //   contPen[i] = ::new Pen(Color());
            contPen[i] = new Pen(Color());
            contPen[i]->SetWidth(lineWidth);
            contPen[i]->SetDashStyle(Gdiplus::DashStyle::DashStyleSolid);
            contPen[i]->SetLineJoin(Gdiplus::LineJoinBevel);
         }

	      for(i = 0; i < nrLev; i++)
	      {   
				if(dataMapping == LINEAR_MAPPING)
               frac = (contourLevels[i]-minVal_)/(maxVal_-minVal_);
				else
				{
					if(maxVal_ > 0 && minVal_ > 0)
                  frac = exp((contourLevels[i]-log(minVal_))/(log(maxVal_)-log(minVal_)));
					else
                  frac = (contourLevels[i]-minVal_)/(maxVal_-minVal_);
				}

            if(useFixedContourColor())
                col.SetFromCOLORREF(fixedContourColor());
            else
	             col.SetFromCOLORREF((GetColor(SCALE,frac, nrLev)));
		      contPen[i]->SetColor(col);	
		   }
      } 
      levels = contourLevels;
   }
 
   //for(int i = 0; i < nrLev; i++)
   //   TextMessage("%d %f\n",i,levels[i]);

   top  = levels[0];
   base = -1.0e9; 
         
// Matrix directions
   bool xFwd = (this->curXAxis()->plotDirection() == PLT_FORWARD);
   bool yFwd = (this->curYAxis()->plotDirection() == PLT_FORWARD);

// Loop over y spectral coordinates **************************************/

// Read in first row *****************/
   long ystart;
   long xm0,xm1,xm2;
   long ym0,ym1,ym2;

   (yFwd) ? (ystart = y_base) : (ystart = y_top-1);

   if(flip == 1)
   {
      if(mat_)
      {
         for(x = x_base; x < x_top; x++)
         {
            xFwd ? (xm1 = x) : (xm1 = x_top+x_base-x-1);
            rowO[x] = mat_[ystart][xm1];
         }
      }
      else
      {
         for(x = x_base; x < x_top; x++)
         {
            xFwd ? (xm1 = x) : (xm1 = x_top+x_base-x-1);
            rowO[x] = cmat_[ystart][xm1].r;
         }
      }      
   }
   else
   {
      if(mat_)
      {
         for(x = x_base; x < x_top; x++)
         {
            xFwd ? (xm1 = x) : (xm1 = x_top+x_base-x-1);
            rowE[x] = mat_[ystart][xm1];
         }
      }
      else
      {
         for(x = x_base; x < x_top; x++)
         {
            xFwd ? (xm1 = x) : (xm1 = x_top+x_base-x-1);
            rowE[x] = cmat_[ystart][xm1].r;
         }
      } 
   }

   long tot = 0;
 // Loop over y coordinates *****************************/   
   for(y = y_base; y < y_top; y++)
   {
      if(this->curYAxis()->plotDirection() == PLT_FORWARD)
      {
         ym1 = y;
         ym2 = ym1+1;
      }
      else
      {
         ym1 = y_top+y_base-y;
         ym2 = ym1-1;
      }

    // Loop over x coordinates *****************************/   
      flip = !flip;

	   if(flip == 1)
	   {
	      if(mat_)
	      {
	         for(x = x_base; x < x_top; x++)
            {
               xFwd ? (xm1 = x) : (xm1 = x_top+x_base-x-1);
		     	   rowO[x] = mat_[ym2][xm1];
            }
	      }
	      else
	      {
	         for(x = x_base; x < x_top; x++)
            {
               xFwd ? (xm1 = x) : (xm1 = x_top+x_base-x-1);
	            rowO[x] = cmat_[ym2][xm1].r;
            }
	      }      
	   }
	   else
	   {
	      if(mat_)
	      {
	         for(x = x_base; x < x_top; x++)
            {
               xFwd ? (xm1 = x) : (xm1 = x_top+x_base-x-1);
					rowE[x] = mat_[ym2][xm1];
            }
	      }
	      else
	      {
	         for(x = x_base; x < x_top; x++)
            {
               xFwd ? (xm1 = x) : (xm1 = x_top+x_base-x-1);
	            rowE[x] = cmat_[ym2][xm1].r;
            }
	      } 
	   }
 
      for(x = x_base; x < x_top-1; x++)
      {
 
      // Read in cell corner values  *********************** 
         if(flip == 1)
         {
	        ll = rowE[x];
	        lr = rowE[x+1];
	        ul = rowO[x];
	        ur = rowO[x+1];
	     }
	     else
	     {
	        ll = rowO[x];
	        lr = rowO[x+1];
	        ul = rowE[x];
	        ur = rowE[x+1];
	     }
	     
     // Check to see if all data in cell is noise *********
	     if(ll < top && lr < top && ul < top && ur < top &&
	        ll > base && lr > base && ul > base && ur > base)
	       continue;

     // Loop over each contour levels for this cell *******
	    for(i = 0; i < nrLev; i++)
	    {
	       lev = levels[i];

	       test = (ll <= lev) + (lr <= lev) + (ul <= lev) + (ur <= lev);
	       if(test == 4 ) break; 

	       if(test > 0)
	       { 
	          cnt = 0;

	          if((ll > lev) + (ul > lev) == 1)
	          {
	             xc[cnt]   = xgrid[x];
	             yc[cnt++] = ygrid[y]-del_y*(lev-ll)/(ul-ll);
	          }
	          if((ul > lev) + (ur > lev) == 1)
	          {
	             xc[cnt]   = xgrid[x]+del_x*(lev-ul)/(ur-ul);
	             yc[cnt++] = ygrid[y+1];
	          }
	          if((ur > lev) + (lr > lev) == 1)
	          {
	             xc[cnt]   = xgrid[x+1];
	             yc[cnt++] = ygrid[y]-del_y*(lev-lr)/(ur-lr);
	          }
	          if((lr > lev) + (ll > lev) == 1)
	          {
	             xc[cnt]   = xgrid[x]+del_x*(lev-ll)/(lr-ll);
	             yc[cnt++] = ygrid[y];
	          }

	          if(cnt == 2)
	          {
                 gfx.DrawLine(contPen[i],xc[0],yc[0],xc[1],yc[1]);
                 tot++;

	          }
	          else if(cnt == 4)
	          {
		         if(ul > ll)
		         {
                   gfx.DrawLine(contPen[i],xc[0],yc[0],xc[1],yc[1]);
                   gfx.DrawLine(contPen[i],xc[2],yc[2],xc[3],yc[3]); 
                   tot+=2;
		         }
		         else
		         {
                   gfx.DrawLine(contPen[i],xc[1],yc[1],xc[2],yc[2]);
                   gfx.DrawLine(contPen[i],xc[3],yc[3],xc[0],yc[0]); 
                   tot+=2;
		         } 
	          }
	       }
	    }
      }
   }

	FreeVector(rowE);
	FreeVector(rowO);
	FreeVector(xgrid);
	FreeVector(ygrid);
   RestoreDC(hdc,-1); 	
   for(int i = 0; i < nrLev; i++)
      delete contPen[i];
     // ::delete contPen[i];
   delete [] contPen;
   if(!contourLevels)
      delete [] levels;
 //  TextMessage("Number of lines = %ld\n",tot);

}

/****************************************************************
   Make contour plot of the current image using GDI
****************************************************************/

//
//void Plot2D::ContourPlot(HWND hWnd, HDC hdc)
//{
//   float base;                // Base for defining noise levels           
//   float del_x,del_y;         // Number of screen pixels per data point   
//   float lev;                 // Current contour level                    
//   float ll,lr;               // Intensity of left corners of cell        
//   float top;                 // Top of noise levels                      
//   float ul,ur;               // Intensity of right corners of cell       
//   float xc[4],yc[4];         // Contour coordinates for the current cell 
//   long cnt;                  // Number of contours cutting cell          
//   long flip = 0;             // Line counter  (0 = odd 1 == even line)   
//   long i;                    // Contour counter                          
//   long left_edge,right_edge; // Edge offsets for calculating spectral    
//   long s_width,s_height;     // Spectral region dimensions  and offsets  
//   float shift_x,shift_y;     // 1/2 data point shift for contours        
//   long spec_height;
//   long spec_width;           // Spectrum dimensions                      
//   long sx_off,sy_off;
//   long test;                 // Number of corners less than contour      
//   long top_edge,bottom_edge; // region (to allow for edge plotting)      
//   long width;                // Region screen dimensions and offsets  
//   long height;               // width and height in pixels   
//   long x_base,y_base;        // Max and min spectral roi coordinates     
//   long x_top,y_top;
//   long x_width,y_height;     // Width and height of spectral roi         
//   register long x;           // x spectral counter                       
//   register long y;           // y spectral counter                       
//   float *rowE;
//   float *rowO;
//   float *xgrid;
//   float *ygrid;
//   COLORREF col;
//   short nrLev;
//   float *levels;
//   float frac;
//   HPEN *contPen;
//   COLORREF *color;
//   short lineWidth = nint(x_scaling_factor());
//   const float zeroOff = 0;
//   
//// Save current device context ******************************** 
//   SaveDC(hdc);
//
//// Create the clipping region *********************************
//   HRGN hClpRgn;
//
//   if(plotParent->showLabels == true)
//   {
//	   hClpRgn = CreateRectRgn(dimensions.left()+1,dimensions.top()+1,
//	                           dimensions.left()+dimensions.width(),
//	                           dimensions.top()+dimensions.height());
//	}
//   else
//   {
//	   hClpRgn = CreateRectRgn(dimensions.left()+1,dimensions.top()+1,
//	                           dimensions.left()+dimensions.width(),
//	                           dimensions.top()+dimensions.height()-2);
//   }
// 	SelectClipRgn(hdc, hClpRgn);
// 	
//// Allocate memory for rows of contour data *******************
//   if((rowE = MakeVector(matWidth_*2)) == NULL)
//   {
//       ErrorMessage("Unable to allocate memory for contour data");
//       RestoreDC(hdc,-1); 
//       return;
//   } 
//
//   if((rowO = MakeVector(matWidth_*2)) == NULL)
//   {
//       ErrorMessage("Unable to allocate memory for contour data");
//       FreeVector(rowE);
//       RestoreDC(hdc,-1); 
//       return;
//   } 
//
//   if((xgrid = MakeVector(matWidth_*2)) == NULL)
//   {
//       ErrorMessage("Unable to allocate memory for contour data");
//       FreeVector(rowE);
//       FreeVector(rowO);
//       RestoreDC(hdc,-1); 
//       return;
//   }      
//
//   if((ygrid = MakeVector(matHeight_*2)) == NULL)
//   {
//       ErrorMessage("Unable to allocate memory for contour data");
//       FreeVector(rowE);
//       FreeVector(rowO);
//       FreeVector(xgrid);
//       RestoreDC(hdc,-1);        
//       return;
//   }
//
//// Determine screen & data region dimensions *******************
//   width  = dimensions.width();
//   height = dimensions.height();
//
//   sx_off   = visibleLeft();
//   sy_off   = visibleTop();
//   s_width  = visibleWidth();
//   s_height = visibleHeight();
//
//   spec_width  = matWidth_;
//   spec_height = matHeight_;
//
//// Adjust roi to include 1 data point around edges *************
//   top_edge    = 1;
//   bottom_edge = 1;
//   left_edge   = 1;
//   right_edge  = 1;
//
//   if(sx_off + s_width  == spec_width)  right_edge = 0;	
//   if(sy_off + s_height == spec_height) top_edge = 0;
//   if(sx_off == 0)                      left_edge = 0;
//   if(sy_off == 0)                      bottom_edge = 0;
//
//   x_base   = sx_off - left_edge;
//   y_base   = sy_off - bottom_edge;
//   x_width  = s_width + right_edge + left_edge;
//   y_height = s_height + top_edge + bottom_edge;
//   x_top    = x_base + x_width;
//   y_top    = y_base + y_height;
//
//// Calculate x and y coordinates of each cell ****************
//   shift_x = 0.5*(float)width/(float)(s_width) + dimensions.left(); 
//   shift_y = (float)height - 0.5*(float)height/(float)(s_height) + dimensions.top();
//
//   assert(!(x_base < 0));
//   assert(!(y_base < 0));
//   assert(!(x_top >= matWidth_*2));
//   assert(!(y_top >= matHeight_*2));
//
//   for(x = x_base; x < x_top; x++)
//      xgrid[x] = (float)(x-sx_off)*(float)width/(float)s_width + shift_x;
//
//   for(y = y_base; y < y_top; y++)
//      ygrid[y] = shift_y - (float)(y-sy_off)*(float)height/(float)s_height;
//     
//   del_x =  (float)width/(float)s_width;
//   del_y =  (float)height/(float)s_height;
//
//   y_top--;
// //  x_base++;
//
//// Determine the top and base of the 'noisy' region ************/
//
//   if(!contourLevels)
//   {
//      if(ColorScaleType() == PLUS_MINUS_CMAP)
//      {      
//         nrLev = nrContourLevels*2;
//         levels = new float[nrLev];
//         contPen = new HPEN[nrLev];
//         color = new COLORREF[nrLev];
//
//	      for(i = 0; i < nrLev/2; i++) // -ve levels
//	      {   
//				if(dataMapping == LINEAR_MAPPING)
//	            levels[i] = (float)(i)*(maxVal_-minVal_)/(nrLev/2-1) - maxVal_;
//				else
//				{
//					if(maxVal_ > 0 && minVal_ > 0)
//	               levels[i] = -exp(-((float)(i)*(log(maxVal_)-log(minVal_))/(nrLev/2-1) - log(maxVal_)));
//					else
//	               levels[i] = (float)(i)*(maxVal_-minVal_)/(nrLev/2-1) - maxVal_;
//				}
//            if(useFixedContourColor())
//                color[i] = fixedContourColor();
//            else
//	             color[i] = GetColor(SCALE,(float)i/nrLev, nrLev);
//		      contPen[i] = CreatePen(PS_SOLID,lineWidth,color[i]);
//		   }
//	      for(i = nrLev/2; i < nrLev; i++) // +ve levels
//	      {   
//				if(dataMapping == LINEAR_MAPPING)
//	            levels[i] = (float)(i-nrLev/2)*(maxVal_-minVal_)/(nrLev/2-1) + minVal_;
//				else
//				{
//					if(maxVal_ > 0 && minVal_ > 0)
//	               levels[i] = exp((float)(i-nrLev/2)*(log(maxVal_)-log(minVal_))/(nrLev/2-1) + log(minVal_));
//					else
//	               levels[i] = (float)(i-nrLev/2)*(maxVal_-minVal_)/(nrLev/2-1) + minVal_;
//				}
//
//            if(useFixedContourColor())
//                color[i] = fixedContourColor();
//            else
//	             color[i] = GetColor(SCALE,(float)i/nrLev, nrLev);
//		      contPen[i] = CreatePen(PS_SOLID,lineWidth,color[i]);
//		   }
//      }
//      else
//      {
//         nrLev = nrContourLevels;
//
//         levels = new float[nrLev];
//         contPen = new HPEN[nrLev];
//         color = new COLORREF[nrLev];
//
//	      for(i = 0; i < nrLev; i++)
//	      {   
//				if(dataMapping == LINEAR_MAPPING)
//	            levels[i] = (float)i/nrLev*(maxVal_-minVal_) + minVal_;
//				else
//				{
//					if(maxVal_ > 0 && minVal_ > 0)
//					   levels[i] = exp((float)i/nrLev*(log(maxVal_)-log(minVal_)) + log(minVal_));
//					else
//	               levels[i] = (float)i/nrLev*(maxVal_-minVal_) + minVal_;
//				}
//
//            if(useFixedContourColor())
//                color[i] = fixedContourColor();
//            else
//	             color[i] = GetColor(SCALE,(float)i/nrLev, nrLev);
//		      contPen[i] = CreatePen(PS_SOLID,lineWidth,color[i]);
//		   }
//      } 
//   }
//   else
//   {
//      if(ColorScaleType() == PLUS_MINUS_CMAP)
//      {      
//         nrLev = nrContourLevels*2;
//         contPen = new HPEN[nrLev];
//         color = new COLORREF[nrLev];
//
//	      for(i = 0; i < nrLev/2; i++) // -ve levels
//	      {   
//				if(dataMapping == LINEAR_MAPPING)
//               frac = (contourLevels[i]-minVal_)/(maxVal_-minVal_);
//				else
//				{
//					if(maxVal_ > 0 && minVal_ > 0)
//                  frac = exp((contourLevels[i]-log(minVal_))/(log(maxVal_)-log(minVal_)));
//					else
//                  frac = (contourLevels[i]-minVal_)/(maxVal_-minVal_);
//				}
//            if(useFixedContourColor())
//                color[i] = fixedContourColor();
//            else
//	             color[i] = GetColor(SCALE,frac, nrLev);
//		      contPen[i] = CreatePen(PS_SOLID,lineWidth,color[i]);
//		   }
//	      for(i = nrLev/2; i < nrLev; i++) // +ve levels
//	      {   
//				if(dataMapping == LINEAR_MAPPING)
//               frac = (contourLevels[i]-minVal_)/(maxVal_-minVal_);
//				else
//				{
//					if(maxVal_ > 0 && minVal_ > 0)
//                  frac = exp((contourLevels[i]-log(minVal_))/(log(maxVal_)-log(minVal_)));
//					else
//                  frac = (contourLevels[i]-minVal_)/(maxVal_-minVal_);
//				}
//
//            if(useFixedContourColor())
//                color[i] = fixedContourColor();
//            else
//	             color[i] = GetColor(SCALE,frac, nrLev);
//		      contPen[i] = CreatePen(PS_SOLID,lineWidth,color[i]);
//		   }
//      }
//      else
//      {
//         nrLev = nrContourLevels;
//
//         levels = new float[nrLev];
//         contPen = new HPEN[nrLev];
//         color = new COLORREF[nrLev];
//
//	      for(i = 0; i < nrLev; i++)
//	      {   
//				if(dataMapping == LINEAR_MAPPING)
//	            levels[i] = (float)i/nrLev*(maxVal_-minVal_) + minVal_;
//				else
//				{
//					if(maxVal_ > 0 && minVal_ > 0)
//					   levels[i] = exp((float)i/nrLev*(log(maxVal_)-log(minVal_)) + log(minVal_));
//					else
//	               levels[i] = (float)i/nrLev*(maxVal_-minVal_) + minVal_;
//				}
//
//            if(useFixedContourColor())
//                color[i] = fixedContourColor();
//            else
//	             color[i] = GetColor(SCALE,(float)i/nrLev, nrLev);
//		      contPen[i] = CreatePen(PS_SOLID,lineWidth,color[i]);
//		   }
//      } 
//      levels = contourLevels;
//   }
// 
//   //for(int i = 0; i < nrLev; i++)
//   //   TextMessage("%d %f\n",i,levels[i]);
//
//   top  = levels[0];
//   base = -1.0e9; 
//         
//// Matrix directions
//   bool xFwd = (this->curXAxis()->plotDirection() == PLT_FORWARD);
//   bool yFwd = (this->curYAxis()->plotDirection() == PLT_FORWARD);
//
//// Loop over y spectral coordinates **************************************/
//
//// Read in first row *****************/
//
//   long ystart;
//   long xm0,xm1,xm2;
//   long ym0,ym1,ym2;
//
//   (yFwd) ? (ystart = y_base) : (ystart = y_top-1);
//
//   if(flip == 1)
//   {
//      if(mat_)
//      {
//         for(x = x_base; x < x_top; x++)
//         {
//            xFwd ? (xm1 = x) : (xm1 = x_top+x_base-x-1);
//            rowO[x] = mat_[ystart][xm1];
//         }
//      }
//      else
//      {
//         for(x = x_base; x < x_top; x++)
//         {
//            xFwd ? (xm1 = x) : (xm1 = x_top+x_base-x-1);
//            rowO[x] = cmat_[ystart][xm1].r;
//         }
//      }      
//   }
//   else
//   {
//      if(mat_)
//      {
//         for(x = x_base; x < x_top; x++)
//         {
//            xFwd ? (xm1 = x) : (xm1 = x_top+x_base-x-1);
//            rowE[x] = mat_[ystart][xm1];
//         }
//      }
//      else
//      {
//         for(x = x_base; x < x_top; x++)
//         {
//            xFwd ? (xm1 = x) : (xm1 = x_top+x_base-x-1);
//            rowE[x] = cmat_[ystart][xm1].r;
//         }
//      } 
//   }
// 
//   for(y = y_base; y < y_top; y++)
//   {
//      if(this->curYAxis()->plotDirection() == PLT_FORWARD)
//      {
//         ym1 = y;
//         ym2 = ym1+1;
//      }
//      else
//      {
//         ym1 = y_top+y_base-y;
//         ym2 = ym1-1;
//      }
//
//    // Loop over x spectral coordinates *****************************/
//   
//      flip = !flip;
//
//	   if(flip == 1)
//	   {
//	      if(mat_)
//	      {
//	         for(x = x_base; x < x_top; x++)
//            {
//               xFwd ? (xm1 = x) : (xm1 = x_top+x_base-x-1);
//		     	   rowO[x] = mat_[ym2][xm1];
//            }
//	      }
//	      else
//	      {
//	         for(x = x_base; x < x_top; x++)
//            {
//               xFwd ? (xm1 = x) : (xm1 = x_top+x_base-x-1);
//	            rowO[x] = cmat_[ym2][xm1].r;
//            }
//	      }      
//	   }
//	   else
//	   {
//	      if(mat_)
//	      {
//	         for(x = x_base; x < x_top; x++)
//            {
//               xFwd ? (xm1 = x) : (xm1 = x_top+x_base-x-1);
//					rowE[x] = mat_[ym2][xm1];
//            }
//	      }
//	      else
//	      {
//	         for(x = x_base; x < x_top; x++)
//            {
//               xFwd ? (xm1 = x) : (xm1 = x_top+x_base-x-1);
//	            rowE[x] = cmat_[ym2][xm1].r;
//            }
//	      } 
//	   }
// 
//      for(x = x_base; x < x_top-1; x++)
//      {
// 
//      // Read in cell corner values  ***********************
//   
//         if(flip == 1)
//         {
//	        ll = rowE[x];
//	        lr = rowE[x+1];
//	        ul = rowO[x];
//	        ur = rowO[x+1];
//	     }
//	     else
//	     {
//	        ll = rowO[x];
//	        lr = rowO[x+1];
//	        ul = rowE[x];
//	        ur = rowE[x+1];
//	     }
//	     
//       // Check to see if all data in cell is noise *********
//
//	     if(ll < top && lr < top && ul < top && ur < top &&
//	        ll > base && lr > base && ul > base && ur > base)
//	       continue;
//
//       // Loop over each contour levels for this cell *******
//
//	    for(i = 0; i < nrLev; i++)
//	    {
//	       lev = levels[i];
//          SelectObject(hdc,contPen[i]);
//          col = color[i];
//
//	       test = (ll <= lev) + (lr <= lev) + (ul <= lev) + (ur <= lev);
//	       if(test == 4 ) break; 
//
//	       if(test > 0)
//	       { 
//	          cnt = 0;
//
//	          if((ll > lev) + (ul > lev) == 1)
//	          {
//	             xc[cnt]   = xgrid[x];
//	             yc[cnt++] = ygrid[y]-del_y*(lev-ll)/(ul-ll);
//	          }
//	          if((ul > lev) + (ur > lev) == 1)
//	          {
//	             xc[cnt]   = xgrid[x]+del_x*(lev-ul)/(ur-ul);
//	             yc[cnt++] = ygrid[y+1];
//	          }
//	          if((ur > lev) + (lr > lev) == 1)
//	          {
//	             xc[cnt]   = xgrid[x+1];
//	             yc[cnt++] = ygrid[y]-del_y*(lev-lr)/(ur-lr);
//	          }
//	          if((lr > lev) + (ll > lev) == 1)
//	          {
//	             xc[cnt]   = xgrid[x]+del_x*(lev-ll)/(lr-ll);
//	             yc[cnt++] = ygrid[y];
//	          }
//
//	          if(cnt == 2)
//	          {
//	             MoveToEx(hdc,xc[0],yc[0],NULL); // Not drawing the last pixel!!
//	             LineTo(hdc,xc[1],yc[1]);
//	             SetPixel(hdc,xc[1],yc[1],col);
//	          }
//	          else if(cnt == 4)
//	          {
//		         if(ul > ll)
//		         {
//	                MoveToEx(hdc,xc[0],yc[0],NULL);
//	                LineTo(hdc,xc[1],yc[1]);
//	                SetPixel(hdc,xc[1],yc[1],col);
//	                
//	                MoveToEx(hdc,xc[2],yc[2],NULL);
//	                LineTo(hdc,xc[3],yc[3]);
//	                SetPixel(hdc,xc[3],yc[3],col);
//		         }
//		         else
//		         {
//	                MoveToEx(hdc,xc[1],yc[1],NULL);
//	                LineTo(hdc,xc[2],yc[2]);
//	                SetPixel(hdc,xc[2],yc[2],col);
//	                
//	                MoveToEx(hdc,xc[3],yc[3],NULL);
//	                LineTo(hdc,xc[0],yc[0]);
//	                SetPixel(hdc,xc[0],yc[0],col);
//		         } 
//	          }
//	       }
//	    }
//      }
//   }
//
//	FreeVector(rowE);
//	FreeVector(rowO);
//	FreeVector(xgrid);
//	FreeVector(ygrid);
//   RestoreDC(hdc,-1); 	
//   for(i = 0; i < nrLev; i++)
//   {
//	   DeleteObject(contPen[i]);
//	}   
//   delete [] color;
//   delete [] contPen;
//   if(!contourLevels)
//      delete [] levels;
// 	DeleteObject(hClpRgn);     
//}



int Plot2D::SetColorMap(Variable *colorVar)
{
   MSG msg;

   if(colorVar->GetType() != MATRIX2D || colorVar->GetDimX() != 3)
   {
      ErrorMessage("invalid colour-scale");
      return(ERR);
   }

	while(plotParent->isBusy())
      PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);

	plotParent->setBusyWithCriticalSection(true);

   if(colorMap()) FreeMatrix2D(colorMap());
   setColorMapLength(colorVar->GetDimY()); 
   float **mIn = colorVar->GetMatrix2D();
   setColorMap(CopyMatrix(mIn,3,colorMapLength()));
   if(gDefaultColorMap) FreeMatrix2D(gDefaultColorMap);
   gDefaultColorMap = CopyMatrix(mIn,3,colorMapLength());
   gDefaultColorMapLength = colorMapLength();
   
	plotParent->setBusyWithCriticalSection(false);

   return(OK);
}

int Plot2D::SetContourColor(Variable *rgb)
{
   float *col = rgb->GetMatrix2D()[0];
   contourColor_ = RGB(nint(col[0]),nint(col[1]),nint(col[2]));
   return(OK);
}


// Simulates an intensity plot - not very useful

#define SWAP(a,b) {long temp=(a);(a)=(b);(b)=temp;}


void Plot2D::ContourPlotIntensity(HWND hWnd, HDC hdc)
{
   long xs,ys;   // Screen coordinates
   COLORREF col;
   float ll,lr,ul,ur;
   float li,ui,dv;
   long xdl,xdu,xsl,xsu;
   long ydl,ydu,ysl,ysu;
   long nrLev;
   float xoff,yoff;
   long leftOff,rightOff;
   long topOff,bottomOff;

// Not suitable for print mode ********************************
   if(gPlotMode == PRINT) return;
   
// Save current device context ********************************   
   SaveDC(hdc);
  
// Create the clipping region *********************************
   HRGN hClpRgn;

	hClpRgn = CreateRectRgn(dimensions.left(),dimensions.top(),
	                        dimensions.left()+dimensions.width(),
	                        dimensions.top()+dimensions.height());

 	SelectClipRgn(hdc, hClpRgn);

   if(ColorScaleType() == PLUS_MINUS_CMAP)  
      nrLev = nrContourLevels*2;
   else
      nrLev = nrContourLevels;


// Matrix directions
   bool xFwd = (this->curXAxis()->plotDirection() == PLT_FORWARD);
   bool yFwd = (this->curYAxis()->plotDirection() == PLT_FORWARD);

   if(xFwd)
   {
      xoff = 0.5*dimensions.width()/(float)visibleWidth();
      leftOff = -xoff;
      rightOff = 0;
   }
   else
   {
      xoff = -0.5*dimensions.width()/(float)visibleWidth();
      leftOff = 0;
      rightOff = -xoff;
   }
   if(yFwd)
   {
       yoff = -0.5*dimensions.height()/(float)visibleHeight();
       topOff = 0;
       bottomOff =-yoff;
   }
   else
   {
       yoff = 0.5*dimensions.height()/(float)visibleHeight();
       topOff = -yoff;
       bottomOff = 0;
   }

// Loop over y spectral coordinates *************************** 
   for(ys = dimensions.top()+topOff; ys < dimensions.top()+dimensions.height()-yoff; ys++)
   { 
      ydl = (int)(curYAxis()->scrnToData(ys));
      ydu = ydl + 1;
      ysl = nint(curYAxis()->dataToScrn((float)ydl));
      ysu = nint(curYAxis()->dataToScrn((float)ydu));
      if(ysl == ysu) // Data point smaller than pixel in y direction?
        ydu = ydl;
      if(ydl < 0 || ydu >= matHeight())
         continue;

      for(xs = dimensions.left()+leftOff; xs < dimensions.left()+dimensions.width()+rightOff; xs++)
      {
         xdl = (int)(curXAxis()->scrnToData(xs));
         xdu = xdl + 1;
         xsl = nint(curXAxis()->dataToScrn((float)xdl));
         xsu = nint(curXAxis()->dataToScrn((float)xdu));
         if(xsl == xsu) // Data point smaller than pixel in x direction?
           xdu = xdl;  
         if(xdl < 0 || xdu >= matWidth())
            continue; 

      // Read in cell corner values  ***********************   
         if(mat_)
         {
		      ll = mat_[ydl][xdl];
		      lr = mat_[ydl][xdu];
		      ul = mat_[ydu][xdl];
		      ur = mat_[ydu][xdu];
		   }
		   else
		   {
		      ll = cmat_[ydl][xdl].r;
		      lr = cmat_[ydl][xdu].r;
		      ul = cmat_[ydu][xdl].r;
		      ur = cmat_[ydu][xdu].r;
		   }
		   
      // Linear interpolation to find level at (xs,ys) ******
      // Allow for data points being smaller than pixels      
         if(xsl != xsu)
         {
            li = ((xs-xsl)/(float)(xsu-xsl))*(lr-ll) + ll;
            ui = ((xs-xsl)/(float)(xsu-xsl))*(ur-ul) + ul;
            if(li != ui)
               dv = ((ys-ysl)/(float)(ysu-ysl))*(ui-li) + li;
            else
               dv = li;
         }
         else
         {
            if(ysl != ysu)
               dv = ((ys-ysl)/(float)(ysu-ysl))*(ul-ll) + ll;
            else
               dv = ll;
         }

       // Work out the colour and plot the pixel ************
         col = GetColor(DATA,dv,nrLev);
         SetPixel(hdc,xs+xoff,ys+yoff,col);
      }
   }
 	DeleteObject(hClpRgn);   
   RestoreDC(hdc,-1); 	
}


void Plot2D::WaterFallPlot(HWND hWnd, HDC hdc)
{    
   long spec_height;
   long spec_width;           // Matrix dimensions in data points                     

   long width,height;         // Plot region dimensions in pixels 
   long left,top,bottom;
   long x,y,ys;  
   long xsi,ysi;
   long xsil,ysil;

   long xc,yc;
   float alpha = PI*this->alpha()/180.0;
   float beta = PI*this->beta()/180.0;

   float xsa,ysa;
   float xsb,ysb;
   float xsg,ysg;

   long cnt;

   Graphics gfx(hdc); 

   Pen pen((Color(GetRValue(wfColor), GetGValue(wfColor), GetBValue(wfColor))));
   Brush* brush = new SolidBrush(Color(GetRValue(bkColor), GetGValue(bkColor), GetBValue(bkColor))); 
 //  Brush* brush = ::new SolidBrush(Color(GetRValue(bkColor), GetGValue(bkColor), GetBValue(bkColor))); 
   pen.SetWidth(1.501);
   pen.SetLineJoin(LineJoinRound);
   gfx.SetSmoothingMode(SmoothingModeAntiAlias);

// Save current device context ******************************** 
   SaveDC(hdc);

// Determine screen & data region dimensions *******************
   width  = dimensions.width();
   height = dimensions.height();
   top = dimensions.top();
   bottom = top+height;
   left = dimensions.left();

   spec_width  = visibleWidth_ ;
   spec_height = visibleHeight_;
   long spec_left = visibleLeft_;
   long spec_top = visibleTop_;

   Gdiplus::Point *pnts =  new Gdiplus::Point[spec_width+2];

   float cosa = cos(alpha);
   float sina = sin(alpha);
   float cosb = cos(beta);
   float sinb = sin(beta);

   xc = left + width/2;
   yc = top + height/2;

// Create the clipping region *********************************
   HRGN hClpRgn;
	hClpRgn = CreateRectRgn(left+1,top+1,left+width,top+height);
   gfx.SetClip(hClpRgn);
   gfx.FillRectangle(brush,left,top,width,height);

   if(this->alpha() >= 90 && this->alpha() < 270)
   {
      for(y = spec_top; y < spec_height+spec_top; y++)
      {
         cnt = 0;
         ys = height-nint((y-spec_top)/((float)spec_height-1)*height)+top;
         for(x = spec_left; x < spec_width+spec_left; x++)
         {
            xsa = (x-spec_left)/((float)spec_width-1)*width+left-xc;
            ysa = (float)ys-yc;
            xsb = cosa*xsa-sina*ysa;
            ysb = sina*xsa+cosa*ysa;
            xsg = xsb;
            ysg = mat_[y][x]*sinb + ysb*cosb;
            xsi = nint(xsg)+xc;
            ysi = nint(ysg)+yc;

            if(x > spec_left)
            {
               pnts[cnt].X = xsi;
               pnts[cnt].Y = ysi;
            }
            else
            {
               pnts[cnt].X = xsi;
               pnts[cnt].Y = ysi;
               xsil = xsi;
            }
            cnt++;
         }
         pnts[cnt].X = xsi;
         pnts[cnt++].Y = bottom;
         pnts[cnt].X = xsil;
         pnts[cnt].Y = bottom;
         gfx.FillPolygon(brush,pnts,spec_width+2);
         gfx.DrawLines(&pen,pnts,spec_width);

      }
   }
   else
   {

      for(y = spec_height+spec_top-1; y >= spec_top; y--)
      {
         cnt = 0;
         ys = height-nint((y-spec_top)/((float)spec_height-1)*height)+top;
         for(x = spec_left; x < spec_width+spec_left; x++)
         {
            xsa = (x-spec_left)/((float)spec_width-1)*width+left-xc;
            ysa = (float)ys-yc;
            xsb = cosa*xsa-sina*ysa;
            ysb = sina*xsa+cosa*ysa;
            xsg = xsb;
            ysg = mat_[y][x]*sinb + ysb*cosb;
            xsi = nint(xsg)+xc;
            ysi = nint(ysg)+yc;

            if(x > spec_left)
            {
               pnts[cnt].X = xsi;
               pnts[cnt].Y = ysi;
            }
            else
            {
               pnts[cnt].X = xsi;
               pnts[cnt].Y = ysi;
               xsil = xsi;
            }
            cnt++;
         }
         pnts[cnt].X = xsi;
         pnts[cnt++].Y = bottom;
         pnts[cnt].X = xsil;
         pnts[cnt].Y = bottom;
         gfx.FillPolygon(brush,pnts,spec_width+2);
         gfx.DrawLines(&pen,pnts,spec_width);

      }
   }
     
  // ::delete brush;
   delete brush;
 	DeleteObject(hClpRgn);   
   delete [] pnts;

   RestoreDC(hdc,-1); 
}


/********************************************************************
            Draw a colour scale next to the image(s)
*********************************************************************/

void Plot2D::DrawColorScale(HDC hdc, HBITMAP hBitMap)
{
   DIBSECTION section;     // Bitmap structure
   BITMAP *bitmap;
   long bmHeight;  // Bitmap dimensions
   BYTE *pbm;              // Pointer to start of bitmap
   long cnt;               // Counter pointing to next color byte
   long nrBytes,step;
   long y;               // Matrix coordinates
   long i,j;
   short bytesperpixel;
   COLORREF col;
   HFONT oldFont;
   HFONT scaleFont;
   HPEN oldPen;
   
   SaveDC(hdc);   

// Calculate colorscale region 
   short ww = regionRight-regionLeft;
   short xl = dimensions.width() + dimensions.left() + ww*0.04;
   short xr = dimensions.width()  + dimensions.left() + ww*0.10;
   short yt = dimensions.top();
   short yb = dimensions.bottom();

// Draw border
   HPEN borderPen  = CreatePen(PS_SOLID,0,RGB(0,0,0));	
   oldPen = (HPEN)SelectObject(hdc,borderPen);   
   DrawRect(hdc,xl,yt,xr,yb);

// Get information about bitmap dimensions and address **********  
   GetObject(hBitMap,sizeof(DIBSECTION),&section);
   
   bitmap   = &section.dsBm;
   bmHeight = bitmap->bmHeight;
   step     = nrBytes = bitmap->bmWidthBytes;
   pbm      = (BYTE*)bitmap->bmBits;
   bytesperpixel = (bitmap->bmBitsPixel)/8;
	
// Loop over the bitmap determining the appropriate position in the data set
// Note that different color scales are possible
   for(j = yt+1; j < yb; j++) // Loop over bitmap rows
   {
      col = GetColor(SCALE,(yb-j-1)/(float)(yb-yt-1),(yb-yt-1));
       
      cnt = bmHeight*step - (j+1)*step + bytesperpixel*(xl+1); // Figure out byte location in bitmap

      if (cnt < 0)
      {
         SelectObject(hdc, oldPen);
         RestoreDC(hdc, -1);
         DeleteObject(borderPen);
         return;
      }

      
      for(i = xl+1; i < xr; i++) // Loop over bitmap columns
      {
         pbm[cnt++] = (BYTE)GetBValue(col);
         pbm[cnt++] = (BYTE)GetGValue(col);
         pbm[cnt++] = (BYTE)GetRValue(col);
      } 
   }

// Display scale labels
	for(Axis* axis: axisList_)
	{
		SetTextColor(hdc,axis->ticks().fontColor());
	}	
	
	SetBkMode(hdc,TRANSPARENT);
	scaleFont = GetFont(hdc, curXAxis()->ticks().font(),-3*y_scaling_factor(),0); 
	oldFont = (HFONT)SelectObject(hdc,scaleFont);
	const int LOCAL_STRING_LENGTH = 50;
	char str[LOCAL_STRING_LENGTH],out[LOCAL_STRING_LENGTH];
   char exp_str[LOCAL_STRING_LENGTH];
   bool flag = false;
	SIZE te; 
   float label;

// No data so return
   if(maxVal_-minVal_ == 0)  
      goto ex;

// Check for scale type (stored in last red entry)
   if(ColorScaleType() == NORMAL_CMAP) // Full range scale
   {
		float spacing = Axis::getLabelSpacing((double)maxVal_-(double)minVal_); 
      if(spacing <= 0)
      {
         ErrorMessage("Invalid data range in 2D image");
         goto ex;
      }

	   label = (long)(minVal_/spacing)*spacing;
   	
	   if(label < minVal_) 
	   {
	      label += spacing;
	   }

   // Draw ticks and labels
	   while(true)
	   {
	      y = yb - nint((label-minVal_)/(maxVal_-minVal_)*(yb-yt));
	      MoveToEx(hdc,xr,y,NULL);
	      LineTo(hdc,xr-3,y);
	      MoveToEx(hdc,xl,y,NULL);
	      LineTo(hdc,xl+3,y);

         GetLabel(spacing,label,str);

	      GetTextExtentPoint32(hdc, str, strlen(str), &te);
	      TextOut(hdc,xr+5,y-te.cy/2,str,strlen(str)); 
	      label += spacing;
	      if(label > maxVal_) break;
	   }

   // Draw multipler if necessary
      if(spacing > 30 || spacing < 0.1)
      {
         GetExponent(spacing,exp_str);

	      GetTextExtentPoint32(hdc, "×10", strlen("×10"), &te);
         long w = te.cx;
         long h = te.cy;

	      TextOut(hdc,(xr+xl)/2-w/2,yt-h,"×10",strlen("×10")); 

         HFONT exponentFont = GetFont(hdc, curXAxis()->ticks().font(),-5*y_scaling_factor(),0); 
         HFONT oldFont = (HFONT)SelectObject(hdc,exponentFont);

	      GetTextExtentPoint32(hdc, exp_str, strlen(exp_str), &te);
	    //  TextOut(hdc,(xr+xl)/2+w/2+1,yt-h*1.2,exp_str,strlen(exp_str)); 
	      TextOut(hdc,nint((xr+xl)/2.0+w/2.0),nint(yt-h-te.cy/4.0),exp_str,strlen(exp_str)); 
         SelectObject(hdc,oldFont);
         DeleteObject(exponentFont); 
      }
   }
   else // Plusminus scale
   {
		float maxv;
   // Find maximum absolute value
      if(fabs(maxVal_) < fabs(minVal_))
         maxv = fabs(minVal_);
      else
         maxv = fabs(maxVal_);

   // Work out label spacing and initial (central) label
		float spacing = Axis::getLabelSpacing((double)(maxv-minVal_));   
      if(spacing <= 0)
      {
         ErrorMessage("Invalid data range in 2D image");
         return;
      }
	   label = (long)(minVal_/spacing)*spacing;
	   if(label < minVal_) 
	   {
	      label += spacing;
	   }
      float startLabel = label;
      if(minVal_ == startLabel && minVal_ != 0)
         flag = 1;

   // Draw +ve ticks and labels
	   while(true)
	   {
	      y = yt + (yb-yt)/2 - nint((label-minVal_)/(maxv-minVal_)*(yb-yt)/2);
	      MoveToEx(hdc,xr,y,NULL);
	      LineTo(hdc,xr-3,y);
	      MoveToEx(hdc,xl,y,NULL);
	      LineTo(hdc,xl+3,y);

         GetLabel(spacing,label,str);
         if(flag)
         {
            sprintf(out,"±%s",str);
            flag = 0;
         }
         else
            strncpy_s(out,LOCAL_STRING_LENGTH,str,_TRUNCATE);
	      GetTextExtentPoint32(hdc, out, strlen(out), &te);
	      TextOut(hdc,xr+5,y-te.cy/2,out,strlen(out)); 
	      label += spacing;
	      if(label > maxv) break;
	   }

   // Draw -ve ticks and labels
      label = -startLabel;

      if(minVal_ == startLabel)
      {
         label -= spacing;
      }

	   while(true)
	   {
	      y = yt + (yb-yt)/2 + nint((-label-minVal_)/(maxv-minVal_)*(yb-yt)/2);
	      MoveToEx(hdc,xr,y,NULL);
	      LineTo(hdc,xr-3,y);
	      MoveToEx(hdc,xl,y,NULL);
	      LineTo(hdc,xl+3,y);

         GetLabel(spacing,label,str);
	      GetTextExtentPoint32(hdc, str, strlen(str), &te);
	      TextOut(hdc,xr+5,y-te.cy/2,str,strlen(str)); 
	      label -= spacing;
	      if(label < -maxv) break;
	   }

   // Draw multipler if necessary
      if(spacing > 30 || spacing < 0.1)
      {
         GetExponent(spacing,exp_str);

	      GetTextExtentPoint32(hdc, "×10", strlen("×10"), &te);
         long w = te.cx;
         long h = te.cy;

	      TextOut(hdc,(xr+xl)/2-w/2,yt-h,"×10",strlen("×10")); 

         HFONT exponentFont = GetFont(hdc, curXAxis()->ticks().font(),-5*y_scaling_factor(),0); 
         HFONT oldFont = (HFONT)SelectObject(hdc,exponentFont);

	      GetTextExtentPoint32(hdc, exp_str, strlen(exp_str), &te);
	      TextOut(hdc,nint((xr+xl)/2.0+w/2.0),nint(yt-h-te.cy/4.0),exp_str,strlen(exp_str)); 

	   //   TextOut(hdc,(xr+xl)/2+w/2+1,yt-h*1.2,exp_str,strlen(exp_str)); 
         SelectObject(hdc,oldFont);
         DeleteObject(exponentFont); 
      }
   }

// Tidy up
ex:

   SelectObject(hdc,oldPen);
   SelectObject(hdc,oldFont);
   RestoreDC(hdc, -1);
   DeleteObject(borderPen);
   DeleteObject(scaleFont);
}


void Plot2D::PrintColorScaleA(HDC hdc, HBITMAP hBitMap)
{
   DIBSECTION section;     // Bitmap structure
   BITMAP *bitmap;
   long bmHeight;				// Bitmap dimensions
   BYTE *pbm;              // Pointer to start of bitmap
   long cnt;               // Counter pointing to next color byte
   long nrBytes,step;
   long i,j;
   COLORREF col;
   
   if(gPrintMode == BLACK_AND_WHITE)
      return;
      
   SaveDC(hdc);

// Calculate colorscale region 

   short ww = regionRight-regionLeft;
   short xr = ww*0.10-ww*0.04+1;
   short yb = dimensions.bottom()-dimensions.top()+1;
   short yt = 0;

// Get information about bitmap dimensions and address **********
   
   GetObject(hBitMap,sizeof(DIBSECTION),&section);
   
   bitmap   = &section.dsBm;
   bmHeight = bitmap->bmHeight;
   step     = nrBytes = bitmap->bmWidthBytes;
   pbm      = (BYTE*)bitmap->bmBits;
	
// Loop over the bitmap determining the appropriate position in the data set
// Note that different color scales are possible

   for(j = 0; j < yb; j++) // Loop over bitmap rows
   {
      col = GetColor(SCALE,(yb-j)/(float)(yb-yt-1),(yb-yt-1));

      cnt = bmHeight*step - j*step; // Figure out byte location in bitmap
      
      for(i = 0; i < xr; i++) // Loop over bitmap columns
      {
         pbm[cnt++] = (BYTE)GetBValue(col);
         pbm[cnt++] = (BYTE)GetGValue(col);
         pbm[cnt++] = (BYTE)GetRValue(col);
      } 
   }
 
   RestoreDC(hdc, -1);
}

void Plot2D::PrintColorScaleB(HDC hdc)
{
   HPEN borderPen;
   HPEN oldPen;
   HFONT oldFont = 0;
   HFONT scaleFont = 0;   
   
   if(gPrintMode == BLACK_AND_WHITE)
      return;

   SaveDC(hdc);
   
// Calculate colorscale region 

   short ww = regionRight-regionLeft;
   short xl = dimensions.width() + dimensions.left() + ww*0.04;
   short xr = dimensions.width()  + dimensions.left() + ww*0.10;
   short yt = dimensions.top();
   short yb = dimensions.bottom();

// Draw border

   if(drawMode_ & DISPLAY_IMAGE)
   {
      borderPen  = CreatePen(PS_SOLID,0,RGB(0,0,0));	
   }
   else
   {
      borderPen  = CreatePen(PS_SOLID,0,RGB(255,255,255));	
	}
	
	SetBkMode(hdc,TRANSPARENT);
   oldPen = (HPEN)SelectObject(hdc,borderPen);   
   DrawRect(hdc,xl,yt,xr,yb);

// Mark maximum and minimum levels

   if(mat_ || cmat_)
   {
		SetTextColor(hdc,curXAxis()->ticks().fontColor());
		SetBkMode(hdc,TRANSPARENT);
	   scaleFont = GetFont(hdc, curXAxis()->ticks().font(),-3*y_scaling_factor(),0); 
	   oldFont = (HFONT)SelectObject(hdc,scaleFont);
	   char str[50];
	   SIZE te; 
	
	   float spacing = Axis::getLabelSpacing((double)maxVal_-(double)minVal_);
	
	   float label = (short)(minVal_/spacing)*spacing;
	   
	   if(label < minVal_) 
	   {
	      label += spacing;
	   }
	
	   for(short i = 1; ; i++)
	   {
	      long y = yb - nint((label-minVal_)/(maxVal_-minVal_)*(yb-yt));  
	      MoveToEx(hdc,xr,y,NULL);
	      LineTo(hdc,xr-3*x_scaling_factor(),y);
	      MoveToEx(hdc,xl,y,NULL);
	      LineTo(hdc,xl+3*x_scaling_factor(),y);
	
	      sprintf(str,"%3.4g",label);
	      GetTextExtentPoint32(hdc, str, strlen(str), &te);
	      TextOut(hdc,xr+5*x_scaling_factor(),y-te.cy/2,str,strlen(str)); 
	      label += spacing;
	      if(label > maxVal_) break;
	   }
   }
	
	if(oldFont)
	{
		SelectObject(hdc,oldFont);
	}
   SelectObject(hdc,oldPen);
   RestoreDC(hdc, -1);
   DeleteObject(borderPen);
	if(scaleFont)
	{
		DeleteObject(scaleFont);
	}
}

void Plot2D::FindMatrixRange(float &minVal_, float &maxVal_)
{   
   maxVal_ = -1e30;
   minVal_ = +1e30;

 // Find the maximum and minimum values in the currently visible data set
    
   for(long i = visibleLeft(); i < visibleLeft() + visibleWidth(); i++)
   {
      for(long j = visibleTop(); j < visibleTop() + visibleHeight(); j++)
      {
         if(mat_)
         {
            if(mat_[j][i] > maxVal_) maxVal_ = mat_[j][i];
            if(mat_[j][i] < minVal_) minVal_ = mat_[j][i];
         }
         else if(cmat_)
         {
            if(cmat_[j][i].r > maxVal_) maxVal_ = cmat_[j][i].r;
            if(cmat_[j][i].r < minVal_) minVal_ = cmat_[j][i].r;
         }            
      }
   }
   
 // Max and min values have a different meaning if the +/- color scale is activated.
 // In this case the max value is the largest deviation from zero and min is 0
 
   if(ColorScaleType() == PLUS_MINUS_CMAP)
   {
      if(fabs(minVal_) > fabs(maxVal_))
         maxVal_ = -minVal_;
      minVal_ = 0;
   } 
}

void Plot2D::FindFullMatrixRange(float &minVal_, float &maxVal_)
{
   long i,j;
   
   maxVal_ = -1e30;
   minVal_ = +1e30;

 // Find the maximum and minimum values in the whole data set
   
   for(i = 0; i < matWidth_; i++)
   {
      for(j = 0; j < matHeight_; j++)
      {
         if(mat_)
         {
            if(mat_[j][i] > maxVal_) maxVal_ = mat_[j][i];
            if(mat_[j][i] < minVal_) minVal_ = mat_[j][i];
         }
         else if(cmat_)
         {
            if(cmat_[j][i].r > maxVal_) maxVal_ = cmat_[j][i].r;
            if(cmat_[j][i].r < minVal_) minVal_ = cmat_[j][i].r;
         } 
      }
   }
   
 // Max and min values have a different meaning if the +/- color scale is activated.
 // In this case the max value is the largest deviation from zero and min is 0
 
   if(ColorScaleType() == PLUS_MINUS_CMAP)
   {
      if(fabs(minVal_) > fabs(maxVal_))
         maxVal_ = -minVal_;
      minVal_ = 0;
   }  
}

/*********************************************************************************
* Work out the type of colormap
* Returns: 0 == NORMAL_CMAP
*          1 == PLUS_MINUS_CMAP
**********************************************************************************/

short Plot2D::ColorScaleType()
{
   if(colorMap_)
   {
      return(nint(colorMap_[colorMapLength_-1][0]));
   }
   return(0);
}

short Plot2D::FillBitMapWithRGBImage(HBITMAP hBitMap)
{

// Check for existence of data set to display *******************  
   if(!matRGB_)
   {
      ErrorMessage("no RGB matrix to plot");
      return(ERR);
   }
     
// Get information about bitmap dimensions and address **********  
   DIBSECTION section;     // Bitmap structure
   GetObject(hBitMap,sizeof(DIBSECTION),&section);
   
   BITMAP* bitmap   = &section.dsBm;
   long bmHeight = bitmap->bmHeight;
   long step     = bitmap->bmWidthBytes;
   BYTE* pbm      = (BYTE*)bitmap->bmBits;
   short bytesperpixel = (bitmap->bmBitsPixel)/8;
		
// Loop over the bitmap determining the appropriate position in the data set
   for(long j = dimensions.top(); j < dimensions.top()+dimensions.height(); j++) // Loop over bitmap rows
   {
      long y = visibleHeight() + visibleTop() - 1 - (j-dimensions.top())*visibleHeight()/dimensions.height(); // Work out matrix y coord

      long cnt = bmHeight*step - (j+1)*step + bytesperpixel*dimensions.left(); // Figure out byte location in bitmap
      
      for(long i = dimensions.left(); i < dimensions.left()+dimensions.width(); i++) // Loop over bitmap columns
      {
         long x = (i-dimensions.left())*visibleWidth()/dimensions.width() + visibleLeft();
                 
         pbm[cnt++] = (BYTE)matRGB_[2][y][x]; // Note this is reversed.
         pbm[cnt++] = (BYTE)matRGB_[1][y][x];
         pbm[cnt++] = (BYTE)matRGB_[0][y][x];
               
      } 
   }
   return(0);
}


/*********************************************************************************
* Take the 2D data set "mat_" or "cmat_" and map the real part into hBitMap. 
**********************************************************************************/

short Plot2D::FillBitMapWithImage(HBITMAP hBitMap)
{	
// Check for existence of data set to display *******************  
   if(!mat_ && !cmat_)
   {
      ErrorMessage("no matrix to plot");
      return(ERR);
   }

// Get information about bitmap dimensions and address **********  
   DIBSECTION section;     // Bitmap structure
   GetObject(hBitMap,sizeof(DIBSECTION),&section);
   
   BITMAP* bitmap   = &section.dsBm;
   long bmHeight = bitmap->bmHeight;
   long step     = bitmap->bmWidthBytes;
   BYTE* pbm      = (BYTE*)bitmap->bmBits;
   short bytesperpixel = (bitmap->bmBitsPixel)/8;
   long nrBytes = bmHeight * step;

   Axis *hAxis = this->xAxis_;
   Axis *vAxis = this->yAxisL_;

// Loop over the bitmap determining the appropriate position in the data set
// Note that different color scales are possible
   long x,y;
   for(long j = dimensions.top(); j < dimensions.top()+dimensions.height(); j++) // Loop over bitmap rows
   {
      if(vAxis->plotDirection() == PLT_FORWARD)
         y = visibleHeight() + visibleTop() - 1 - (j-dimensions.top())*visibleHeight()/dimensions.height(); // Work out matrix y coord
      else
         y = visibleTop() + (j-dimensions.top())*visibleHeight()/dimensions.height(); // Work out matrix y coord

      long cnt = bmHeight*step - (j+1)*step + bytesperpixel*dimensions.left(); // Figure out byte location in bitmap
      
      for(long i = dimensions.left(); i < dimensions.left()+dimensions.width(); i++) // Loop over bitmap columns
      {
         
			float data;

         if(hAxis->plotDirection() == PLT_FORWARD)
            x = (i-dimensions.left())*visibleWidth()/dimensions.width() + visibleLeft();
         else
            x = (dimensions.width()+dimensions.left()-1-i)*visibleWidth()/dimensions.width() + visibleLeft();

         if(x >= matWidth_ || x < 0 || y >= matHeight_ || y < 0) // Shouldn't happen
            continue;


         if(mat_) 
			{
				if(dataMapping == LINEAR_MAPPING)
				   data = mat_[y][x];
				else // Log mapping
				{
					if(mat_[y][x] >= 1)
				      data = log(mat_[y][x]);
					else if(mat_[y][x] <= -1)
				      data = -log(-mat_[y][x]);
               else
                  data = 0;
				}
			}
         else 
			{
				data = cmat_[y][x].r;
			}

         COLORREF col = GetColor(DATA,data,10000);

         if(cnt < 0 || cnt >= nrBytes)
            break;

         
         pbm[cnt++] = (BYTE)GetBValue(col);
         pbm[cnt++] = (BYTE)GetGValue(col);
         pbm[cnt++] = (BYTE)GetRValue(col);
               
      } 
   }

   return(0);
}


/*********************************************************************************
* Take the 2D data set "mat_" or "cmat" and map the real part into hBitMap. 
**********************************************************************************/

short Plot2D::FillBitMapWithImage2(HBITMAP hBitMap)
{
// Check for existence of data set to display *******************  

   if(!mat_ && !cmat_)
   {
      ErrorMessage("no matrix to plot");
      return(ERR);
   }
      
// Get information about bitmap dimensions and address **********
   
   DIBSECTION section;     // Bitmap structure
   GetObject(hBitMap,sizeof(DIBSECTION),&section);
   
   BITMAP* bitmap   = &section.dsBm;
   long bmHeight = bitmap->bmHeight;
   long step     = bitmap->bmWidthBytes;
   BYTE* pbm      = (BYTE*)bitmap->bmBits;	
	
// Loop over the bitmap determining the appropriate position in the data set
// Note that different color scales are possible

   for(long j = visibleTop(); j < visibleTop()+visibleHeight(); j++) // Loop over bitmap rows
   {
      long cnt = bmHeight*step - (j-visibleTop()+1)*step; // Figure out byte location in bitmap
      long k = visibleTop() + visibleHeight() - 1 - (j - visibleTop());
   
		float data;   
      for(long i = visibleLeft(); i < visibleLeft()+visibleWidth(); i++) // Loop over bitmap columns
      {   
         if(mat_)
			{
				data = mat_[k][i];
			}
         else 
			{
				data = cmat_[k][i].r;
			}
         
         COLORREF col = GetColor(DATA,data,10000);
         
         pbm[cnt++] = (BYTE)GetBValue(col);
         pbm[cnt++] = (BYTE)GetGValue(col);
         pbm[cnt++] = (BYTE)GetRValue(col);
               
      } 
   }
   return(0);
}

/*********************************************************************************
* Fill bitmap with specified colour. 
**********************************************************************************/

short Plot2D::FillBitMapWithColor(HBITMAP hBitMap,COLORREF color)
{
	// Get information about bitmap dimensions and address **********
	DIBSECTION section;     // Bitmap structure
	GetObject(hBitMap,sizeof(DIBSECTION),&section);

	BITMAP* bitmap   = &section.dsBm;
	long bmHeight = bitmap->bmHeight;
	long step     = bitmap->bmWidthBytes;
	BYTE* pbm      = (BYTE*)bitmap->bmBits;

	// Loop over the bitmap determining the appropriate position in the data set
	// Note that different color scales are possible

	for(long j = 0; j < visibleHeight(); j++) // Loop over bitmap rows
	{
		long cnt = bmHeight*step - (j+1)*step; // Figure out byte location in bitmap

		for(long i = 0; i < visibleWidth(); i++) // Loop over bitmap columns
		{
			pbm[cnt++] = (BYTE)GetBValue(color);
			pbm[cnt++] = (BYTE)GetGValue(color);
			pbm[cnt++] = (BYTE)GetRValue(color);
		} 
	}
	return(0);
}


// Given a data value and a color scheme return the appropriate color value

COLORREF Plot2D::GetColor(short mode, float data, long nrLevels)
{
   COLORREF col;
   float colorPos;
   long index;

// No colormap then return black
   if(!colorMap_ || colorMapLength_ == 0)
      return(RGB(0,0,0));

// Work out a number from 0 to 1 representing position on the color scale
   if(mode == DATA)
   {
      if(ColorScaleType() == PLUS_MINUS_CMAP)
      {
		   if(dataMapping == LINEAR_MAPPING || minVal_ < 0 || maxVal_ < 0)
         {
	         if(data > minVal_)
	            colorPos = 0.5+0.5*(data-minVal_)/(maxVal_-minVal_);
	         else if(data < -minVal_)
	            colorPos = 0.5-0.5*(-data-minVal_)/(maxVal_-minVal_);
           else
              colorPos = 0.5;
         }
         else
         {
	         if(data > log(minVal_))
	            colorPos = 0.5+0.5*(data-log(minVal_))/(log(maxVal_)-log(minVal_));
	         else if(data < -log(minVal_))
	            colorPos = 0.5-0.5*(-data-log(minVal_))/(log(maxVal_)-log(minVal_));
            else
               colorPos = 0.5;
         }
      }
      else 
      {
		   if(dataMapping == LINEAR_MAPPING || minVal_ < 0 || maxVal_ < 0)
			{
				if(data < minVal_)
				{
					colorPos = 0;
				}	
				else
				{
			      colorPos = (data-minVal_)/(maxVal_-minVal_);
				}
			}
			else
			{
				if(data < log(minVal_))
				{
					colorPos = 0;
				}	
				else
				{
				   colorPos = (data-log(minVal_))/(log(maxVal_)-log(minVal_));		
				}
			}
      }

	   colorPos = ((int)(colorPos*nrLevels)/(float)nrLevels);
   }
   else
   {
	   colorPos = data;
   }
         
// For a B/W printer just make the line black
	if(gPlotMode == PRINT && gPrintMode == BLACK_AND_WHITE && (Plot2D::curPlot()->drawMode_ & DISPLAY_CONTOURS))
   {
      return(RGB((BYTE)0,(BYTE)0,(BYTE)0));
   }

// Convert color position to an RGB value
   index = (long)(colorPos*(colorMapLength_-1));
   if(index < 0) index = 0;
   if(index >= colorMapLength_-1) index = colorMapLength_-2;
   col = RGB(nsint(255*colorMap_[index][0]),nsint(255*colorMap_[index][1]),nsint(255*colorMap_[index][2]));

	return(col);	
}


void Plot2D::clearColorMap()
{
	if(colorMap_){
		FreeMatrix2D(colorMap_);
	}
	colorMap_ = NULL;
   colorMapLength_ = 0;
}

short Plot2D::ZoomRegion()
{
	zoomHistory.push_back(selectRect);
// Update current region
   if(selectRect.left < 0)
      selectRect.left = 0;
   setVisibleLeft(selectRect.left);
   if(selectRect.top < 0)
      selectRect.top = 0;
   setVisibleTop(selectRect.top);

   int width = selectRect.right-visibleLeft()+1;
// Check for width too large or small
   if(width == 0) width = 1;
   if(selectRect.left + width > matWidth())
      width = matWidth()-selectRect.left;
   setVisibleWidth(width);

   int height = selectRect.bottom-visibleTop()+1;
// Check for height too large or small
   if(height == 0) height = 1;
   if(selectRect.top + height > matHeight())
      height = matHeight()-selectRect.top;
	setVisibleHeight(height);

	ResetSelectionRectangle();
   return(OK);
}

short Plot2D::DisplayHistory(FloatRect& r)
{
	setVisibleLeft(r.left);
   setVisibleTop(r.top);
   setVisibleWidth(r.right-visibleLeft());
   setVisibleHeight(r.bottom-visibleTop());
	return OK;
}

void Plot2D::resetDataView()
{
   setVisibleLeft(0);
   setVisibleTop(0);
   setVisibleWidth(matWidth_);
   setVisibleHeight(matHeight_);
}

bool Plot2D::lastRegion(HWND hWnd)
{
	if(zoomHistory.size() <= 1)
   {
		FullRegion();
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


// Make the 2D current region (and any others connected with it) square.

void Plot2D::MakeSquareRegions(HWND hWnd)
{
   RECT r;
   short winWidth,winHeight,extra;

   GetWindowRect(hWnd,&r);
   winWidth = r.right-r.left;
   if(displayColorScale_)
   {
      extra = winWidth - (dimensions.width() + margins.totalHorizontalMargins())*plotParent->cols/0.9;
      winHeight = r.bottom-r.top;
		winWidth = (dimensions.height() + margins.totalHorizontalMargins())*plotParent->cols/0.9+extra;
   }
   else
   {
      extra = winWidth - (dimensions.width() + margins.totalHorizontalMargins())*plotParent->cols;
      winHeight = r.bottom-r.top;
      winWidth = (dimensions.height() + margins.totalHorizontalMargins())*plotParent->cols+extra;
   }
   MoveWindow(hWnd,r.left,r.top,winWidth,winHeight,true);
}


/**********************************************************************
   Draw x and y axes on the current 2D plot
**********************************************************************/

void Plot2D::DrawAxes(HDC hdc)
{
	// Setup axes tick and label spacings *************
	if (xAxis_->getTickLabel() == ERR)
		return;
	if (yAxisL_->getTickLabel() == ERR)
		return;

// Draw the axes
   if(axesMode == PLOT_X_AXIS || axesMode == PLOT_X_AXIS_BOX)
	   xAxis_->draw(hdc);

   else if(axesMode == PLOT_Y_AXIS || axesMode == PLOT_Y_AXIS_BOX)
	   yAxisL_->draw(hdc);

   else
   {
	   xAxis_->draw(hdc);
	   yAxisL_->draw(hdc);
   }
}


void Plot2D::FullRegion()
{
// Reset region rectangle

   scrnSelectRect.left   = 0;
   scrnSelectRect.top    = 0;
   scrnSelectRect.right  = 0;
   scrnSelectRect.bottom = 0;
   
// Reset display region

	resetDataView();

	if(this == Plot2D::curPlot())
      SendMessageToGUI("Plot 2D,Zoom",0);
   else
      SendMessageToGUI("Plot 1D,Zoom",0);
}


void Plot2D::ChooseBlackNWhite()
{
	for(Axis* axis: axisList_)
	{
		axis->grid()->setColor(RGB_DARK_CYAN);
		axis->grid()->setFineColor(RGB_LIGHT_CYAN);
		axis->ticks().setFontColor(axesColor);
		axis->label().setFontColor(axesColor);
	}

	this->axesColor        = RGB_BLACK;
	this->bkColor          = RGB_WHITE;
	this->borderColor      = RGB_WHITE;
   this->title().setFontColor(this->axesColor);
}

void Plot2D::setVisibleLeft(long val) {
	visibleLeft_ = val;
}
void Plot2D::setVisibleTop(long val) {
	visibleTop_ = val;
}
void Plot2D::setVisibleWidth(long val) {
	visibleWidth_ = val;
}
void Plot2D::setVisibleHeight(long val) {
	visibleHeight_ = val;
}

float Plot2D::getTickStart(Axis* axis, Ticks* ticks)
{
	if (axis == this->curXAxis())
	{
		float mx = visibleLeft() * axis->length() / (float) matWidth_ + axis->base();
		return (long)(mx/ticks->spacing()) * ticks->spacing();
	}
	else if (axis == this->yAxisLeft() || axis == this->yAxisRight())
	{
		float my = visibleTop() * axis->length() / (float) matHeight_ + axis->base();
		return (long)(my/ticks->spacing()) * ticks->spacing();
	}
	return 0;
}


short Plot2D::SelectRegion(HWND hWnd, short xs0, short ys0)
{
// Check to see if there is any data in the plot (2D)
	if (!DataPresent() && !vx_)
	{
		return ABORT;
	}
	return Plot::SelectRegion(hWnd, xs0, ys0);
}

void Plot2D::DrawSelectionRectangle(HDC hdc)
{
	// Check to see if there is any data in the plot (2D)
	if (!DataPresent())
	{
		return;
	}
	Plot::DrawSelectionRectangle(hdc);
}

void Plot2D::HideSelectionRectangle(HDC hdc)
{
	// Check to see if there is any data in the plot (2D)

   if(!DataPresent()) 
		return;

	Plot::HideSelectionRectangle(hdc);

}

void Plot2D::initialiseMenuChecks(const char *mode)
{
   WinData *win = plotParent->obj->winParent;
	const char* const m = plotParent->menuName();
	const char* const tbn = plotParent->toolbarName();
   if(!strcmp(mode,"clear"))
   {
      win->setMenuItemCheck(m,"drag_plot",false);
      win->setMenuItemCheck(m,"display_data",true);
      win->setMenuItemCheck(m,"select_region",false);
      win->setMenuItemCheck(m,"select_row",false);
      win->setMenuItemCheck(m,"select_column",false);
      win->setToolBarItemCheck(tbn, "drag_plot", false);
      win->setToolBarItemCheck(tbn, "display_data", true);
      win->setToolBarItemCheck(tbn, "select_region", false);
      win->setToolBarItemCheck(tbn, "corner_axes", false);
      win->setToolBarItemCheck(tbn, "border_axes", true);
      win->setToolBarItemCheck(tbn, "select_row", false);
      win->setToolBarItemCheck(tbn, "select_column", false);
   }
   else if(!strcmp(mode,"load"))
   {
      win->setToolBarItemCheck(tbn, "corner_axes", (axesMode == PLOT_AXES_CORNER));
      win->setToolBarItemCheck(tbn, "border_axes", (axesMode == PLOT_AXES_BOX));
   }
}

bool Plot2D::identifyReverseVertical(float& min, float& max, float length, float base)
{
	bool reverse = false;

	min = visibleTop() * length/(float)matHeight() + base;
	max = min + visibleHeight() * length/(float)matHeight();
   if(max < min)
   {
		float temp = max;
		max = min;
		min = temp;
      reverse = true;
   }
	return reverse;
}

bool Plot2D::identifyReverseHorizontal(float& min, float& max, float length, float base)
{
	bool reverse = false;

	min = visibleLeft() * length/(float)matWidth() + base;
	max = min + visibleWidth() * length/(float)matWidth();
	if(max < min)
   {
		float temp = max;
		max = min;
		min = temp;
      reverse = true;
   }
	return reverse;
}

COLORREF Plot2D::ChoosePlotColor(HWND hWnd, short dest, bool bkgColorEnabled)
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


void Plot2D::saveTicks(FILE *fp)
{
	float temp = getXTicks().majorLength();
   fwrite(&temp,sizeof(float),1,fp);
	temp = getXTicks().minorLength();
   fwrite(&temp,sizeof(float),1,fp);
	temp = getXTicks().spacing();
   fwrite(&temp,sizeof(float),1,fp);
	temp = getYTicks().spacing();
   fwrite(&temp,sizeof(float),1,fp);
	temp = getXTicks().perLabel();
   fwrite(&temp,sizeof(float),1,fp);
	temp = getYTicks().perLabel();
   fwrite(&temp,sizeof(float),1,fp);
}

/*************************************************************************
*                        Save the plot parameters                        *
*************************************************************************/

void Plot2D::save2AxesDimensions(FILE* fp)
{
	float dim = curXAxis()->base();
   fwrite(&dim,sizeof(float),1,fp);
	dim = curXAxis()->length();
   fwrite(&dim,sizeof(float),1,fp);
	dim = curYAxis()->base();
   fwrite(&dim,sizeof(float),1,fp);
	dim = curYAxis()->length();
   fwrite(&dim,sizeof(float),1,fp);
}

void Plot2D::save2AxesDirections(FILE* fp)
{

   PlotDirection direction = curXAxis()->plotDirection();
	fwrite(&direction, sizeof(PlotDirection), 1, fp);
   direction = curYAxis()->plotDirection();
	fwrite(&direction, sizeof(PlotDirection), 1, fp);
}

void Plot2D::saveLabelFontParams(FILE* fp)
{
	LOGFONT f = curXAxis()->ticks().font();
	fwrite(&f,sizeof(LOGFONT),1,fp);
	f = curXAxis()->label().font();
   fwrite(&f,sizeof(LOGFONT),1,fp);
	f = title().font();
   fwrite(&f,sizeof(LOGFONT),1,fp);

	COLORREF color = curXAxis()->ticks().fontColor();
   fwrite(&color,sizeof(COLORREF),1,fp);
	color = curXAxis()->label().fontColor();
   fwrite(&color,sizeof(COLORREF),1,fp);
	color = title().fontColor();
   fwrite(&color,sizeof(COLORREF),1,fp);
}

void Plot2D::save2DAxesPPMInfo(FILE* fp)
{
	bool ppm = curXAxis()->ppmScale();
   fwrite(&ppm,sizeof(bool),1,fp);
	ppm = curYAxis()->ppmScale();
   fwrite(&ppm,sizeof(bool),1,fp);
}

void Plot2D::save2Lines(FILE* fp)
{
   long size = lines_.size();
   fwrite(&size,sizeof(long),1,fp);

   for(PlotLine* line: this->lines_)
   {
      line->Save(this, fp);
   }
}

void Plot2D::saveDrawGrid(FILE* fp)
{  
	bool draw = curXAxis()->grid()->drawGrid();
	fwrite(&draw,sizeof(bool),1,fp);
	draw = curYAxis()->grid()->drawGrid();
   fwrite(&draw,sizeof(bool),1,fp);
	draw = curXAxis()->grid()->drawFineGrid();
   fwrite(&draw,sizeof(bool),1,fp);
	draw = curYAxis()->grid()->drawFineGrid();
   fwrite(&draw,sizeof(bool),1,fp);
}

void Plot2D::saveLabels(FILE* fp)
{
	char labelText[PLOT_STR_LEN];

	strncpy_s(labelText,PLOT_STR_LEN, title().text(), _TRUNCATE);
   fwrite(labelText,PLOT_STR_LEN,1,fp);
	strncpy_s(labelText,PLOT_STR_LEN, curXAxis()->label().text(), _TRUNCATE);
   fwrite(labelText,PLOT_STR_LEN,1,fp);
	strncpy_s(labelText,PLOT_STR_LEN,yAxisLeft()->label().text(), _TRUNCATE);
   fwrite(labelText,PLOT_STR_LEN,1,fp);
}

void Plot2D::saveGridColors(FILE* fp)
{
	COLORREF col = curXAxis()->grid()->color();
   fwrite(&col,sizeof(COLORREF),1,fp);
	col = curXAxis()->grid()->fineColor();
   fwrite(&col,sizeof(COLORREF),1,fp);
}

void Plot2D::saveDisplayedExtremes(FILE* fp)
{	
	long val = visibleLeft();
   fwrite(&val,sizeof(long),1,fp);
	val = visibleTop();
   fwrite(&val,sizeof(long),1,fp);
	val = visibleWidth();
   fwrite(&val,sizeof(long),1,fp);
	val = visibleHeight();
   fwrite(&val,sizeof(long),1,fp);
}

void Plot2D::savePlotParameters(FILE *fp)
{
   long version = PLOTFILE_VERSION_3_1;
   fwrite(&version,sizeof(long),1,fp);
	saveTicks(fp);
	saveLabelFontParams(fp);

   fwrite(&yLabelVert,sizeof(bool),1,fp);
   fwrite(&axesMode,sizeof(short),1,fp);
	
	saveDrawGrid(fp);
   saveLabels(fp);

   fwrite(&axesColor,sizeof(COLORREF),1,fp);
	fwrite(&bkColor,sizeof(COLORREF),1,fp);
   fwrite(&plotColor,sizeof(COLORREF),1,fp);
   fwrite(&borderColor,sizeof(COLORREF),1,fp);
	saveGridColors(fp);
	short scale = colorScale();
   fwrite(&scale,sizeof(short),1,fp);
	float dim = vectorLength();
   fwrite(&dim,sizeof(float),1,fp);
   fwrite(&xVectorStep,sizeof(short),1,fp);
   fwrite(&yVectorStep,sizeof(short),1,fp);
	short mode = drawMode();
   fwrite(&mode,sizeof(short),1,fp);
	long length = colorMapLength();
   fwrite(&length,sizeof(long),1,fp);
   if(colorMapLength() && colorMap())
      fwrite(&(colorMap()[0][0]),sizeof(float),3*colorMapLength(),fp);
	bool display = displayColorScale();
	fwrite(&display,sizeof(bool),1,fp);
	
	save2AxesDimensions(fp);
	saveDisplayedExtremes(fp);
}

void Plot2D::save2AxesMappings(FILE* fp)
{
	short mapping = curXAxis()->mapping();
   fwrite(&mapping,sizeof(short),1,fp);
	mapping = curYAxis()->mapping();
   fwrite(&mapping,sizeof(short),1,fp);
}


/*************************************************************************
*         Save 2D data in subplot to a file                        *
*************************************************************************/

short Plot2D::SaveData(char* pathName, char* fileName)
{
// Convert to a 1D plot
   Interface itfc;

// Get the data
   this->GetData(&itfc, 1);
   Variable *data = &itfc.retVar[1];

// Save the data
   return(::SaveData(NULL, pathName, fileName, data));
}	   


short Plot2D::save(FILE* fp)
{
   // Save parameters specific to this plot 
   saveParameters(fp);  

	// Save dimensions (width and height)
	long dim = matWidth();
   fwrite(&dim,sizeof(long),1,fp);   // Matrix width
	dim = matHeight();
   fwrite(&dim,sizeof(long),1,fp);  // Matrix height

	// Save data type (REAL/COMPLEX/VECTOR)
   if(mat()) // Real
   {
		if (ERR == SaveReal(fp))
		{
			return ERR;
		}      
   }
   else if(cmat()) // Complex
   {
		if (ERR == SaveComplex(fp))
		{
			return ERR;
		}
   }
   else if(vx() && vy()) // x-y vector data
   {
		if (ERR == SaveVec2(fp))
		{
			return ERR;
		}
   }		   
   else // No data
   {
		if (ERR == SaveNone(fp))
		{
			return ERR;
		}
	}
	return OK;
}

/*************************************************************************
*                        Save the plot parameters                        *
*************************************************************************/

short Plot2D::saveParameters(FILE *fp)
{
   long version = plot2DSaveVersion;
   long v = PLOTFILE_VERSION_3_7;
 
	fwrite(&version,sizeof(long),1,fp);
	saveTicks(fp);
	saveLabelFontParams(fp);

   fwrite(&yLabelVert,sizeof(bool),1,fp);
   fwrite(&axesMode,sizeof(short),1,fp);
	
	saveDrawGrid(fp);
   saveLabels(fp);

   fwrite(&axesColor,sizeof(COLORREF),1,fp);
	fwrite(&bkColor,sizeof(COLORREF),1,fp);
   fwrite(&plotColor,sizeof(COLORREF),1,fp);
   fwrite(&borderColor,sizeof(COLORREF),1,fp);
	saveGridColors(fp);
	short scale = colorScale();
   fwrite(&scale,sizeof(short),1,fp);
	float dim;
   fwrite(&dim,sizeof(float),1,fp);
	setVectorLength(dim);
   fwrite(&xVectorStep,sizeof(short),1,fp);
   fwrite(&yVectorStep,sizeof(short),1,fp);
	short mode = drawMode();
   fwrite(&mode,sizeof(short),1,fp);
	long length = colorMapLength();
   fwrite(&length,sizeof(long),1,fp);
   if(colorMapLength() && colorMap())
      fwrite(&(colorMap()[0][0]),sizeof(float),3*colorMapLength(),fp);
	bool display = displayColorScale();
	fwrite(&display,sizeof(bool),1,fp);

	save2AxesDimensions(fp);

   if(plot2DSaveVersion == PLOTFILE_VERSION_2_1)
      return(OK);

	saveDisplayedExtremes(fp);

   if(plot2DSaveVersion == PLOTFILE_VERSION_3_0)
      return(OK);

   save2AxesDirections(fp);

   if(plot2DSaveVersion == PLOTFILE_VERSION_3_2)
      return(OK);

   save2AxesMappings(fp);
   save2Lines(fp);

   if(plot2DSaveVersion == PLOTFILE_VERSION_3_3)
      return(OK);

   saveFixedContourColor(fp);

   if(plot2DSaveVersion == PLOTFILE_VERSION_3_4)
      return(OK);

   saveAxesInfo(fp);

   if(plot2DSaveVersion == PLOTFILE_VERSION_3_5)
      return(OK);

   saveDataMappingInfo(fp);

   if(plot2DSaveVersion == PLOTFILE_VERSION_3_6)
      return(OK);

   save2DAxesPPMInfo(fp);

	return OK;
}

void Plot2D::saveFixedContourColor(FILE *fp)
{
   bool useFixedColor = useFixedContourColor();
   short nrLevels = nrContourLevels;
   COLORREF col = fixedContourColor();
   bool useFixedLevels = (contourLevels && nrContourLevels>0);

   fwrite(&useFixedColor,sizeof(bool),1,fp);
   fwrite(&nrLevels,sizeof(short),1,fp);
   fwrite(&col,sizeof(COLORREF),1,fp);
   fwrite(&useFixedColor,sizeof(bool),1,fp);
   fwrite(&useFixedLevels,sizeof(bool),1,fp);

   if(useFixedLevels)
      fwrite(&(contourLevels[0]),sizeof(float),nrContourLevels,fp);
}

// Save axes and information as to whether axes are calibrated.
// Axes are currently unused.
void Plot2D::saveAxesInfo(FILE *fp)
{
   long width = matWidth();
   long height = matHeight();

   fwrite(&xCalibrated_,sizeof(bool),1,fp);
   if(!xAxis)
      width = 0;
   fwrite(&width,sizeof(long),1,fp);

   if(xCalibrated_ && width > 0 && xAxis)
       fwrite(&xAxis,sizeof(float),width,fp);

   fwrite(&yCalibrated_,sizeof(bool),1,fp);
   if(!yAxis)
      height = 0;
   fwrite(&height,sizeof(long),1,fp);

   if(yCalibrated_ && height > 0 && yAxis)
       fwrite(&yAxis,sizeof(float),height,fp);
}

void Plot2D::saveDataMappingInfo(FILE *fp)
{
   float minV = minVal();
   float maxV = maxVal();
	bool overRide = getOverRideAutoRange();

	fwrite(&minV,sizeof(float),1,fp);
	fwrite(&maxV,sizeof(float),1,fp);
	fwrite(&overRide,sizeof(bool),1,fp);
   fwrite(&dataMapping,sizeof(int),1,fp);
}

void Plot2D::saveMinMaxXY(FILE* fp)
{
	float val = curXAxis()->Min();
   fwrite(&val,sizeof(float),1,fp);
	val = curXAxis()->Max();
   fwrite(&val,sizeof(float),1,fp);
	val = curYAxis()->Min();
   fwrite(&val,sizeof(float),1,fp);
	val = curYAxis()->Max();
   fwrite(&val,sizeof(float),1,fp);
}

short Plot2D::SaveReal(FILE* fp)
{
   long type =  'REAL';
   fwrite(&type,sizeof(long),1,fp);          // Real file
	long nrPoints = matWidth() * matHeight();
   fwrite(mat()[0],sizeof(float),nrPoints,fp); // Write data
	return OK;
}

short Plot2D::SaveComplex(FILE* fp)
{
	long type =  'COMP';
	fwrite(&type,sizeof(long),1,fp);             // Complex file
	long nrPoints = matWidth() * matHeight();
	fwrite(cmat()[0],sizeof(complex),nrPoints,fp); // Write data
	return OK;
}

short Plot2D::SaveVec2(FILE* fp)
{
   long type =  'VEC2';
   fwrite(&type,sizeof(long),1,fp);             // 2D vector file
	long nrPoints = matWidth() * matHeight();
   fwrite(vx()[0],sizeof(float),nrPoints,fp); // Write x data
   fwrite(vy()[0],sizeof(float),nrPoints,fp); // Write y data
	return OK;
}

short Plot2D::SaveNone(FILE* fp)
{
	long type =  'NONE'; 
	fwrite(&type,sizeof(long),1,fp);    // No data
	return OK;
}

/*****************************************************************************
*              Display all the images in the 2D window
*****************************************************************************/

void Plot2D::DisplayAll(bool locked)
{
	RECT pr;
	MSG msg;
   extern double GetMsTime();

	PlotWindow *pp = plotParent;

	if(!(pp->updatePlots() && updatePlots()))
		return;

   if(!locked)
   {
      EnterCriticalSection(&cs2DPlot);

      if(!pp->isBusy())
	   {
         pp->setBusy(true);

         LeaveCriticalSection(&cs2DPlot);
	      HDC hdc = GetDC(win); // prepare window for painting
	      HDC hdcMem = CreateCompatibleDC(hdc);
	      SelectObject(hdcMem,pp->bitmap);

	      GetClientRect(win,&pr);

	      for(Plot* p: pp->plotList())
	      {
		      p->Display(win,hdcMem);
	      }

	      BitBlt(hdc,pr.left,pr.top,pr.right-pr.left,pr.bottom-pr.top,hdcMem,pr.left,pr.top,SRCCOPY);
	      DeleteDC(hdcMem);
	      ReleaseDC(win,hdc);
  
         pp->setBusyWithCriticalSection(false);
      }
      else
         LeaveCriticalSection(&cs2DPlot);
   }
   else
   {
      HDC hdc = GetDC(win); // prepare window for painting
      HDC hdcMem = CreateCompatibleDC(hdc);
      SelectObject(hdcMem,pp->bitmap);

      GetClientRect(win,&pr);

      for(Plot* p: pp->plotList())
      {
	      p->Display(win,hdcMem);
      }

      BitBlt(hdc,pr.left,pr.top,pr.right-pr.left,pr.bottom-pr.top,hdcMem,pr.left,pr.top,SRCCOPY);
      DeleteDC(hdcMem);
      ReleaseDC(win,hdc);
   }
}


void Plot2D::LoadAndDisplayDataFile(HWND hWnd, char *basePath, char *fileName)
{
	Interface itfc;
	CText varName;

	makeCurrentPlot();
	makeCurrentDimensionalPlot();
	varName = fileName;
	RemoveExtension(varName.Str());

	if(LoadData(&itfc, basePath, fileName, varName.Str(), GLOBAL) == OK)
	{ 
		setTitleText(varName.Str());
		DisplayMatrixAsImage(&itfc,varName.Str());
		SendMessageToGUI("2D Plot,LoadImage",0);
		UpdateStatusWindow(hWnd,3,statusText);
		Invalidate();
	}
}
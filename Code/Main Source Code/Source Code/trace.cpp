#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)
#include <gdiplus.h> 
#include "stdafx.h"
#include "trace.h"
#include "allocate.h"
#include "Axis.h"
#include "cArg.h"
#include "defines.h"
#include "drawing.h"
#include "evaluate.h"
#include "font.h"
#include "globals.h"
#include "guiWindowClass.h"
#include "interface.h"
#include "mymath.h"
#include "plot.h"
#include "PlotFile.h"
#include "PlotWindow.h"
#include "print.h"
#include "prospaResource.h"
#include "rgbColors.h"
#include "string_utilities.h"
#include "StringPairs.h"
#include "TracePar.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include <float.h>
#include <math.h>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include "memoryLeak.h"

using std::string;
using std::pair;
using std::vector;
using std::stringstream;

using namespace Gdiplus;

#define MIN_DIST2 100
#define TRACE_SEGMENT_AXIS_INDICATOR_LENGTH 3
// Constructors


Trace::Trace(Plot1D* parent)
{
	this->parent = parent;
   name[0] = '\0';
	path[0] = '\0';
	ID = NO_TRACE_ID_SET;
   //parent = 0;
   x = 0;
	size = 0;
	filteredX = 0;
	filteredY = 0;
   bars = 0;
	validationCode = GetNextObjectValidationCode(); // Makes this object unique

	xAxis_ = parent? parent->curXAxis() : 0;
	yAxis_ = parent? parent->curYAxis() : 0;
	minx = maxx = miny = maxy = 0;
   ignoreXRange = false;
   ignoreYRange = false;
   varList.next = NULL;
   varList.last = NULL;
}

Trace::Trace(const Trace& di)
{
	this->ID = di.ID;
	this->parent = di.parent;
	if (di.x)
		this->x = CopyArray(di.x, di.size);
	else
		this->x = 0;
	if (di.filteredX)
		this->filteredX = CopyArray(di.filteredX, di.size);
	else
		this->filteredX = 0;
	if (di.filteredY)
		this->filteredY = CopyArray(di.filteredY, di.size);
	else
		this->filteredY = 0;
	if (di.bars)
		this->bars = CopyMatrix(di.bars, di.size,2);
	else
		this->bars = 0;	
	this->size = di.size;
	this->minx = di.minx;
	this->maxx = di.maxx;
	this->miny = di.miny;
	this->maxy = di.maxy;
	strncpy_s(this->name, MAX_STR, di.name, _TRUNCATE);
	strncpy_s(this->path, MAX_STR, di.path, _TRUNCATE);
	this->tracePar = di.tracePar;
	this->parent = di.parent;
	this->xAxis_ = 0;
	this->yAxis_ = 0;
   this->ignoreXRange = di.ignoreXRange;
   this->ignoreYRange = di.ignoreYRange;
   varList.next = NULL;
   varList.last = NULL;
	validationCode = GetNextObjectValidationCode(); // Makes this object unique
}

Trace::Trace(float *x, long size, char *name, TracePar *tp, Plot1D* parent)
{
	this->parent = parent;
	ID = NO_TRACE_ID_SET;
   this->x = x;
	filteredX = 0;
	filteredY = 0;
	bars = NULL;
   this->size = size;
   strncpy_s(this->name,MAX_STR,name,_TRUNCATE);
	this->path[0] = '\0';
	this->tracePar = *tp;
	this->xAxis_ = 0;
	this->yAxis_ = 0;
   ignoreXRange = false;
   ignoreYRange = false;
   varList.next = NULL;
   varList.last = NULL;
	validationCode = GetNextObjectValidationCode(); // Makes this object unique
}

Trace::~Trace()
{
   if(x)         GlobalFree(x);
	if(filteredX) GlobalFree(filteredX);
	if(filteredY) GlobalFree(filteredY);
	if(bars)      GlobalFree(bars);
   if(xAxis_)    xAxis_->removeTrace(this);
   if(yAxis_)    yAxis_->removeTrace(this);
   if(varList.next) 
   {
      varList.RemoveAll();
      varList.next = NULL;
   }
}

/************************************************************************
Generate a string representing the state of this object's parameter-
accessible attributes.
************************************************************************/

string Trace::FormatState()
{
	StringPairs state;
	state.add("getdata","trace data (x,y vectors)");
	state.add("size", stringifyInt(this->size).c_str());

	state.add("ebararray","trace data (errorbar matrix [~,2])");
	state.add("id number", stringifyInt(this->getID()).c_str());
	state.add("name", this->getName());

   int winNr = this->parent->plotParent->obj->winParent->nr;
   int objNr = this->parent->plotParent->obj->nr();
   CText parent;
   parent.Format("(%d,%d)",winNr,objNr);
   state.add("parent",parent.Str());

	return FormatStates(state);
}

/*****************************************************************************************/
// Does this trace decrease as its index increases?
/*****************************************************************************************/
bool Trace::isReverseTrace()
{
	if (x && size > 1)
		return (x[0] > x[size-1]);
	return false;
}

/*****************************************************************************************/
// Produce a new Variable for a dimension's data. 
/*****************************************************************************************/
Variable* Trace::xComponentAsVariable()
{
	Variable* v = new Variable;

	v->SetData((char*)MakeMatrix2D(size, 1));
   if(!v->GetData()) 
   {
      ErrorMessage("unable to allocate 2D matrix memory");
      v->SetType(NULL_VARIABLE);
      v->SetDim1(0);
		v->SetDim2(0);
		v->SetDim3(0);
		v->SetDim4(0);
		return v;
   }
	float **temp = (float**)v->GetData();

	v->SetDim1(size);
   v->SetDim2(1);
   v->SetDim3(1);
   v->SetDim4(1);
   v->SetAlias(NULL);

   if(x != NULL)
   {
      long j = 0;
	   for(long y = 0; y < 1; y++)
	   {   
		   for(long x = 0; x < size; x++,j++)
		   {
		      temp[y][x] = this->x[j];
		   }
		}
   }
   v->SetType(MATRIX2D);
	return v;
}


/*****************************************************************************************/
// Produce a new Variable for a dimension's data. 
/*****************************************************************************************/
Variable* Trace::xMinMaxAsVariable()
{
	long start = FindIndexCore(this->x, size, xAxis()->Min());
	long end = FindIndexCore(this->x, size, xAxis()->Max());
	long size = end - start + 1;

	Variable* v = new Variable;
	v->MakeAndLoadMatrix2D(0,size,1);
	float** mX = v->GetMatrix2D();
	for(int i = start; i <= end; i++)
	{
		mX[0][i-start] = this->x[i];
	}
	return v;
}

/*****************************************************************************************/
// Produce a new Variable for a dimension's data. 
/*****************************************************************************************/
Variable* Trace::barsAsVariable()
{

	Variable* v = new Variable;
	v->MakeAndLoadMatrix2D(bars,size,2);
	return v;
}

/*****************************************************************************************/
// Load bars data directly from a file.
/*****************************************************************************************/
short Trace::readBars(FILE* fp)
{
	bars = MakeMatrix2D(getSize(),2);
	if(fread(&(bars[0][0]),sizeof(float),getSize()*2,fp) != getSize()*2)
		return(ERR);
	return OK;
}

/*****************************************************************************************/
// Write bars data directly to a file.
/*****************************************************************************************/
short Trace::writeBars(FILE* fp)
{
	if (fwrite(&(bars[0][0]),sizeof(float),getSize()*2,fp) != getSize()*2)
		return (ERR);
	return OK;
}


/*****************************************************************************************/
// Load x data directly from a file.
/*****************************************************************************************/
short Trace::readX(FILE* fp)
{
	if(!(x = MakeVector(size)))
	{
		ErrorMessage("Can't allocate memory for 1D x data");
		return(ERR);
	} 
   fread(x,4,size,fp);
	return OK;
}

/*****************************************************************************************/
// Load x/y data directly from a file.
/*****************************************************************************************/
short Trace::readData(FILE* fp)
{
	if (ERR != readX(fp))
	{
		return readY(fp);
	}
	return ERR;
}

/*****************************************************************************************/
// Write x/y data directly to a file.
/*****************************************************************************************/
short Trace::writeData(FILE* fp)
{
	fwrite(x,4,size,fp);    // Save X data
	return writeY(fp);
}

/*****************************************************************************************/
// Draw error bars on the plot.
/*****************************************************************************************/
void Trace::PlotErrorBars(HDC hdc, int scale)
{
	return;
}
	
/*****************************************************************************************/
// Display all traces in the plot.
/*****************************************************************************************/
void Trace::DisplayAll()
{
	parent->DisplayAll(false);
}
	
bool Trace::isUpdatePlots()
{
	return parent->isUpdatePlots();
}

/*****************************************************************************************/
// Remove this from its current axis and add it to the specified one.
/*****************************************************************************************/
bool Trace::moveTraceToAxis(VerticalAxisSide side)
{	
	return parent->moveTraceToAxis(this, side);
}

// Determine line width, taking into account presence or absence of antialiasing
short Trace::lineWidth(bool ignoreAntialias) const
{
	return tracePar.getTraceWidth();
}

HPEN Trace::makeRealPen(bool ignoreAntialias) const
{
	short width = lineWidth(ignoreAntialias);
	short realStyle = tracePar.getRealStyle();
	COLORREF realColor = tracePar.getRealColor();
	
	return isPrinting()? CreatePen(realStyle,width,RGB_BLACK) : CreatePen(realStyle,width,realColor);
}

HPEN Trace::makeImagPen(bool ignoreAntialias) const
{
	short width = lineWidth(ignoreAntialias);
	short imagStyle = tracePar.getImagStyle();
	COLORREF imagColor = tracePar.getImagColor();

	return isPrinting()? CreatePen(imagStyle,width,RGB_BLACK) : CreatePen(imagStyle,width,imagColor);
}

void Trace::drawSegmentTick(HDC hdc, float scale, POINT* point1, POINT* point2)
{
   SaveDC(hdc);
	SetBkMode(hdc,TRANSPARENT);
	 
	HPEN pen = makeRealPen(true);

	SelectObject(hdc,pen);

	if (parent->yAxisLeft() == this->yAxis())
	{
		MoveToEx(hdc, point1->x, point1->y,0);
		LineTo(hdc, point1->x + scale*TRACE_SEGMENT_AXIS_INDICATOR_LENGTH, 
			point1->y - scale*TRACE_SEGMENT_AXIS_INDICATOR_LENGTH);
	}
	else // this plot is on the right axis
	{
		MoveToEx(hdc, point2->x, point2->y,0);
		LineTo(hdc, point2->x - scale*TRACE_SEGMENT_AXIS_INDICATOR_LENGTH, 
			point2->y + scale*TRACE_SEGMENT_AXIS_INDICATOR_LENGTH);
	}
	DeleteObject(pen);
	RestoreDC(hdc, -1);
}

void Trace::drawSegment(HDC hdc, POINT* point1, POINT* point2)
{	
   SaveDC(hdc);
	SetBkMode(hdc,TRANSPARENT);
	 
	HPEN pen = makeRealPen(true);
	COLORREF symbolColor = tracePar.getRealSymbolColor();
	HBRUSH symbolBrush = isPrinting() ? (HBRUSH)GetStockObject(BLACK_BRUSH) : CreateSolidBrush(symbolColor);
	HPEN bkgPen = CreatePen(PS_SOLID,0,parent->bkColor);
	HBRUSH bkgBrush = isPrinting() ? (HBRUSH)GetStockObject(WHITE_BRUSH) : parent->GetPlotBackgroundBrush();
	HPEN symbolPen = isPrinting() ? (HPEN)CreatePen(PS_SOLID,0,RGB_BLACK) : CreatePen(PS_SOLID,0,symbolColor);

	SelectObject(hdc,pen);

	MoveToEx(hdc, point1->x, point1->y, 0);
	LineTo(hdc, point2->x, point2->y);
   
	// Draw a single symbol in the centre of the segment.
	if (tracePar.getSymbolType() != TD_NO_SYMBOL)
	{
		SelectObject(hdc,symbolPen);
		float printPixelWidth = (gPlotMode == PRINT) ? GetDeviceCaps(hdc,HORZRES) : 4;
		float printMMWidth = (gPlotMode == PRINT) ? GetDeviceCaps(hdc,HORZSIZE) : 1;
		int szX = min(max(2,ceil((float)((5* (tracePar.getTraceWidth() + 1)) / 4))), 6) * (printPixelWidth/printMMWidth/4) ;
		int szY = szX;
		parent->DrawSymbol(hdc,tracePar.getSymbolType(), symbolBrush, symbolPen, bkgBrush, 
			bkgPen, (point1->x + point2->x) / 2,(point1->y + point2->y) / 2,szX,szY);
	}

	DeleteObject(pen);
	DeleteObject(bkgBrush);
	DeleteObject(bkgPen);
	DeleteObject(symbolPen);
	DeleteObject(symbolBrush);
	RestoreDC(hdc, -1);
}

void Trace::drawName(HDC hdc, ProspaFont& font, POINT* point)
{
   SaveDC(hdc);
	SetBkMode(hdc,TRANSPARENT);
	ShortRect rect;

   WriteText(hdc,point->x,point->y,WindowLayout::LEFT_ALIGN,WindowLayout::CENTRE_ALIGN,
		WindowLayout::HORIZONTAL,font.font(),font.color(),RGB_WHITE,name,rect);

	RestoreDC(hdc, -1);
}

void Trace::plot(HDC hdc, long xL, long xR, long yT, long yB)
{
	switch(tracePar.getTraceType())
	{
		case(PLOT_TRACE_STAIRS):        
			PlotStairs(hdc,xL,xR,yT,yB);
			break;
		case(PLOT_TRACE_LINES):
      if(gPlotMode == PRINT || gPlotMode == IMAGEFILE)
			   PlotLinesPrint(hdc,xL,xR,yT,yB);
         else
			if(this->parent->IsFiltered())
			   PlotLines(hdc,xL,xR,yT,yB);
			else
				PlotAllLines(hdc, xL, xR, yT, yB);
			break;
		case(PLOT_TRACE_DOTS):
			PlotPoints(hdc,xL,xR,yT,yB);
			break;
	}
}


short Trace::save(FILE* fp, int version)
{
	if (fp == 0)
		return ERR;

   short type = hasImaginary() ? 1 : 0;

	VerticalAxisSide side = NO_VAXIS_SIDE;
	if (yAxis_ == this->parent->yAxisLeft())
		side = LEFT_VAXIS_SIDE;
	else
		side = RIGHT_VAXIS_SIDE;

  // This is only included for backward compatibility when saving old
  // file formats. It is not used by the newer version (V3.51)
	long constVer = PLOTFILE_VERSION_3_2;

   fwrite(&constVer,sizeof(long),1,fp);
	fwrite(&type, sizeof(short), 1, fp);
	fwrite(&side, sizeof(VerticalAxisSide), 1, fp);
	fwrite(&size, sizeof(long), 1, fp);
	fwrite(&minx, sizeof(float), 1, fp);
	fwrite(&maxx, sizeof(float), 1, fp);
	fwrite(&miny, sizeof(float), 1, fp);
	fwrite(&maxy, sizeof(float), 1, fp);

	tracePar.save(fp, version);
	
	if (tracePar.isErrorBarsStored())
		writeBars(fp);

	fwrite(name, sizeof(char), MAX_STR, fp);

	writeData(fp);

	return OK;
}

///////////////////////////////////////////////////////////////////////////////////////////

TraceReal::TraceReal(Plot1D* parent)
: Trace(parent)
{
	y = 0;
}

TraceReal::TraceReal(float *x, float *y, long size, char *name, TracePar* tp, Plot1D* parent, bool append)
: Trace(x,size,name,tp,parent)
{
	this->y = y;
	DiscoverXRange();
	DiscoverYRange(this->getMinX(), this->getMaxX());
   if(append)
	   parent->appendTrace(this);
}

TraceReal::TraceReal(const TraceReal& copyMe)
: Trace(copyMe)
{
	if (copyMe.y)
		y = CopyArray(copyMe.y, copyMe.size);
	else
		y = 0;
}

/*****************************************************************************************/
// Use "clone" to make a deep copy of a TraceReal.
/*****************************************************************************************/
Trace* TraceReal::clone() const
{
	return new TraceReal(*this);
}

TraceReal::~TraceReal()
{
	GlobalFree(y);
}

/*****************************************************************************************/
// Are all y-values greater than zero?
/*****************************************************************************************/
bool TraceReal::greaterThanZero(char dir)
{
	if ('x' == dir)
	{
		for (long i = 0; i < size; i++)
			if (X(i) <= 0)
				return false;
	}
	else if (y)
	{
		for (long i = 0; i < size; i++)
		{
			if (y[i] <= 0)
				return false;
		}
	}
	return true;
}

/*****************************************************************************************/
// Get the max and min X values of this trace. ???And return the array limits (not necessarily the same)
/*****************************************************************************************/
void TraceReal::DiscoverXRange(float* lhs, float* rhs)
{
   float minx = 1e30;
	float maxx = -1e30;
   for(long i = 0; i < this->size; i++)
   {
      if(isnan(this->X(i)) || isnan(this->y[i])) continue;
      if(this->X(i) > maxx) maxx = this->X(i);
      if(this->X(i) < minx) minx = this->X(i);
   }
//
//// Make sure range is never zero
//   if(minx == maxx)
//   {
//      minx = minx-1;
//      maxx = maxx+1;
//   }

// Copy to trace data structure
   this->setMinX(minx);
   this->setMaxX(maxx);

// Return those results if requested
	if (lhs)
		*lhs = x[0];
	if (rhs)
		*rhs = x[size-1];
}


/*****************************************************************************************/
// Produce a new Variable for a dimension's data. 
/*****************************************************************************************/
Variable* TraceReal::yComponentAsVariable()
{
	Variable* v = new Variable();

	v->SetData((char*)MakeMatrix2D(size, 1));
   if(!v->GetData()) 
   {
      ErrorMessage("unable to allocate 2D matrix memory");
      v->SetType(NULL_VARIABLE);
      v->SetDim1(0);
		v->SetDim2(0);
		v->SetDim3(0);
		v->SetDim4(0);
      return v;
   }
	float **temp = (float**)v->GetData();

	v->SetDim1(size);
   v->SetDim2(1);
   v->SetDim3(1);
   v->SetDim4(1);
   v->SetAlias(NULL);

   if(y != NULL)
   {
      long j = 0;
	   for(long y = 0; y < 1; y++)
	   {   
		   for(long x = 0; x < size; x++,j++)
		   {
		      temp[y][x] = this->y[j];
		   }
		}
   }
   v->SetType(MATRIX2D);
	return v;
}

/*****************************************************************************************
*        Return the full y range of the real or complex data stored in dat               *
*****************************************************************************************/
void TraceReal::DiscoverYRange(float minx,float maxx)
{
   float miny  = 1e30;
   float maxy  = -1e30;
	for(long i = 0; i < this->size; i++)
   {
      if(this->x[i] >= minx && this->x[i] <= maxx)
      {
         if(this->y[i] > maxy) maxy = this->y[i];
         if(this->y[i] < miny) miny = this->y[i];
      }
   }

// Make sure range is never zero
   //if(miny == maxy)
   //{
   //   if(miny == 0)
   //   {
   //      miny = miny-1;
   //      maxy = maxy+1;
   //   }
   //   else
   //   {
   //      miny = miny-fabs(miny)/10;
   //      maxy = maxy+fabs(maxy)/10;
   //   }
   //}

// Copy to trace data structure
   this->setMinY(miny);
   this->setMaxY(maxy);
}


/*****************************************************************************************
*                       Get range of data references in structure dat                    *
*****************************************************************************************/
void TraceReal::GetXRange()
{
   float minx_local  = 1e30;
   float maxx_local  = -1e30;

   for(long i = 0; i < size; i++)
   {
      if(isnan(x[i]) || isnan(y[i])) continue;
      if(x[i] > maxx_local) maxx_local = x[i];
      if(x[i] < minx_local) minx_local = x[i];
   }

// Make sure range is never zero
   if(minx_local == maxx_local)
   {
      minx_local = minx_local-1;
      maxx_local = maxx_local+1;
   }

// Copy to trace data structure
   this->setMinX(minx_local);
   this->setMaxX(maxx_local);
}


/*****************************************************************************************
*        Return the full y range of the real or complex data stored in dat               *
*****************************************************************************************/
void TraceReal::GetYRange(float minx,float maxx)
{
   float miny_local  = 1e30;
   float maxy_local  = -1e30;

   for(long i = 0; i < size; i++)
   {
      if(x[i] >= minx && x[i] <= maxx)
      {
         if(y[i] > maxy_local) maxy_local = y[i];
         if(y[i] < miny_local) miny_local = y[i];
      }
   }
   
// Make sure range is never zero
   if(miny_local == maxy_local)
   {
      if(miny_local == 0)
      {
         miny_local = miny_local-1;
         maxy_local = maxy_local+1;
      }
      else
      {
         miny_local = miny_local-fabs(miny_local)/10;
         maxy_local = maxy_local+fabs(maxy_local)/10;
      }
   }

// Copy to trace data structure
   this->setMinY(miny_local);
   this->setMaxY(maxy_local);
}


/*****************************************************************************************/
// Produce a new Variable for a dimension's data. 
/*****************************************************************************************/
Variable* TraceReal::yMinMaxAsVariable()
{
	long start = FindIndexCore(x, size, xAxis()->Min());
	long end = FindIndexCore(x, size, xAxis()->Max());
	long size = end - start + 1;

	Variable* v = new Variable;
	v->MakeAndLoadMatrix2D(0,size,1);
	float** mY = v->GetMatrix2D();
	for(int i = start; i <= end; i++)
	{
		mY[0][i-start] = y[i];
	}
	return v;
}

/*****************************************************************************************/
// Set the data in the existing arraus 
/*****************************************************************************************/
void TraceReal::setData(float* xData, float* yData, long sz)
{
	if (x)
		GlobalFree(x);
	if (y)
		GlobalFree(y);

	x = xData;
	y = yData;
	size = sz;
	DiscoverXRange();
	DiscoverYRange(this->getMinX(), this->getMaxX());
}

void TraceComplex::setData(float* xData, complex* yData, long sz)
{
	if (x)
		GlobalFree(x);
	if (yc)
		GlobalFree(yc);

	x = xData;
	yc = yData;
	size = sz;
	DiscoverXRange();
	DiscoverYRange(this->getMinX(), this->getMaxX());
}

/*****************************************************************************************/
// Get the name of the type of data in this trace as a string. 
/*****************************************************************************************/
string& TraceReal::typeString() const
{
	static string typeName = string("Real");
	return typeName;
}

bool Trace::isPrinting() const
{
	return (gPlotMode == PRINT && gPrintMode == BLACK_AND_WHITE);
}

/*****************************************************************************************/
// Plot the data in this trace as stairs.
/*****************************************************************************************/	
void TraceReal::PlotStairs(HDC hdc,	long xL, long xR, long yT, long yB) const
{	
   SaveDC(hdc);

	short width = lineWidth();
	HPEN realPen = makeRealPen();
   SelectObject(hdc,realPen);
   float xOff = tracePar.getXOffset();
   float yOff = tracePar.getYOffset();
   long ixOff = xAxis_->plotDimensions()->width()*xOff;
   long iyOff = yAxis_->plotDimensions()->height()*yOff;

	long xa = xAxis_->dataToScrn(x[0]);
   long ya = yAxis_->dataToScrn(y[0]);
   for(long i = 0; i < size; i++)
   {
      long xb = xAxis_->dataToScrn(x[i],ixOff);
      long yb = yAxis_->dataToScrn(y[i],iyOff);

      if(LineVisible(xL,yT,xR,yB,xa,ya,xb,yb,width))  // All or part of line is visble
      {            
	      if(i == 0)	                
            MoveToEx(hdc,(short)xb,(short)yb,0);
	      else
	      {
		      LineTo(hdc,(short)xb,(short)ya);
		      LineTo(hdc,(short)xb,(short)yb);
	      }
      }
      else // Line is not visible at all so just move to end
      {
         MoveToEx(hdc,(short)xb,(short)yb,0);
	   }
      xa = xb; ya = yb;
   }
	DeleteObject(realPen);
	RestoreDC(hdc, -1);
}

/*****************************************************************************************
 Plot the data in this trace as points.
******************************************************************************************/	
void TraceReal::PlotPoints(HDC hdc,	long xL, long xR, long yT, long yB) const
{
	COLORREF col = isPrinting()? RGB_BLACK : tracePar.getRealColor();
   float xOff = tracePar.getXOffset();
   float yOff = tracePar.getYOffset();
   long ixOff = xAxis_->plotDimensions()->width()*xOff;
   long iyOff = yAxis_->plotDimensions()->height()*yOff;

   for(long i = 0; i < size; i++)
   {
      if(isnan(x[i])) 
      {
         continue;
      }
      long xp = xAxis_->dataToScrn(x[i],ixOff);
      long yp = yAxis_->dataToScrn(y[i],iyOff);
      if(PointVisible(xL,yT,xR,yB,xp,yp))   
      {
      	 SetPixel(hdc,xp,yp,col);
      }         
   }
}

/*****************************************************************************************
 Remove redundant trace points which occurs in the same screen pixel
******************************************************************************************/	
long Trace::FilterOutRedundantPoints(const float* const x, const float* const y, long size, float* const filteredX, float* const filteredY) const
{
	long filtPos = 0;

	long currentX = x[0];
	long currentYEntry = y[0];
	long currentYExit = currentYEntry;
	long maxCurrentY = currentYEntry;
	long minCurrentY = currentYEntry;

	for(long i = 1; i < size; i++)
   {
      if(isnan(x[i]))
      {
         continue;
	   }    

      long xb = x[i];
      long yb = y[i];

		// If they refer to the same point as the previous entry, ignore.
		if ((xb == currentX) && (yb == y[i-1]))
		{
			continue;
		}

		if (xb == currentX)  // X has not changed. 
		{
			if (maxCurrentY < yb)
			{
				maxCurrentY = yb;
			}
			if (minCurrentY > yb)
			{
				minCurrentY = yb;
			}
			currentYExit = yb;
		}
		else  // X has changed. Create the lines for the old X, and make a line to the new one.
		{
			filteredX[filtPos] = currentX;
			filteredY[filtPos++] = currentYEntry;
			
			if (currentYEntry != maxCurrentY)
			{
				filteredX[filtPos] = currentX;
				filteredY[filtPos++] = maxCurrentY;
			}
			if ((currentYEntry != minCurrentY) && (maxCurrentY != minCurrentY))
			{
				filteredX[filtPos] = currentX;
				filteredY[filtPos++] = minCurrentY;
			}
			if ((currentYEntry != currentYExit) && (maxCurrentY != currentYExit) && (minCurrentY != currentYExit))
			{
				filteredX[filtPos] = currentX;
				filteredY[filtPos++] = currentYExit;
			}
			currentX = xb;
			currentYEntry = yb;
			currentYExit = yb;
			minCurrentY = yb;
			maxCurrentY = yb;		
		}
	}

	// Add any remaining unadded points.

	filteredX[filtPos] = currentX;
	filteredY[filtPos++] = currentYEntry;
			
	if (currentYEntry != maxCurrentY)
	{
		filteredX[filtPos] = currentX;
		filteredY[filtPos++] = maxCurrentY;
	}
	if ((currentYEntry != minCurrentY) && (maxCurrentY != minCurrentY))
	{
		filteredX[filtPos] = currentX;
		filteredY[filtPos++] = minCurrentY;
	}
	if ((currentYEntry != currentYExit) && (maxCurrentY != currentYExit) && (minCurrentY != currentYExit))
	{
		filteredX[filtPos] = currentX;
		filteredY[filtPos++] = currentYExit;
	}

	// Return the index of the last element of the resulting array.
	return(filtPos - 1);
}

/*****************************************************************************************
 Remove redundant trace points which occurs in the same fractional screen pixel
******************************************************************************************/	
long Trace::FilterOutRedundantPointsF(const float* const x, const float* const y, long size, float* const filteredX, float* const filteredY) const
{
	long filtPos = 0;
   int nrPntsPixel = 2; // Two points per pixel (more is slower less is ugly)
   float fac = 1.0/nrPntsPixel;
	float currentX = x[0];
	float currentYEntry = y[0];
	float currentYExit = currentYEntry;
	float maxCurrentY = currentYEntry;
	float minCurrentY = currentYEntry;

	for(long i = 1; i < size; i++)
   {
      if(isnan(x[i]))
      {
         continue;
	   }    

      float xb = x[i];
      float yb = y[i];

		// If they refer to the same point as the previous entry, ignore.
		if ((abs(xb - currentX) <= fac) && (abs(yb - y[i-1]) <= fac))
		{
			continue;
		}

		if (abs(xb - currentX) <= fac)  // X has not changed. 
		{
			if (maxCurrentY < yb)
			{
				maxCurrentY = yb;
			}
			if (minCurrentY > yb)
			{
				minCurrentY = yb;
			}
			currentYExit = yb;
		}
		else  // X has changed. Create the lines for the old X, and make a line to the new one.
		{
			filteredX[filtPos] = currentX;
			filteredY[filtPos++] = currentYEntry;
			
			if (abs(currentYEntry - maxCurrentY) > fac)
			{
				filteredX[filtPos] = currentX;
				filteredY[filtPos++] = maxCurrentY;
			}
	      if ((abs(currentYEntry - minCurrentY) > fac) && (abs(maxCurrentY - minCurrentY) > fac))
			{
				filteredX[filtPos] = currentX;
				filteredY[filtPos++] = minCurrentY;
			}
       	if ((abs(currentYEntry - currentYExit) > fac) && (abs(maxCurrentY - currentYExit) > fac) && (abs(minCurrentY - currentYExit) > fac))
			{
				filteredX[filtPos] = currentX;
				filteredY[filtPos++] = currentYExit;
			}
			currentX = xb;
			currentYEntry = yb;
			currentYExit = yb;
			minCurrentY = yb;
			maxCurrentY = yb;		
		}
	}

	// Add any remaining unadded points.

	filteredX[filtPos] = currentX;
	filteredY[filtPos++] = currentYEntry;
			
	if (abs(currentYEntry - maxCurrentY) > fac)
	{
		filteredX[filtPos] = currentX;
		filteredY[filtPos++] = maxCurrentY;
	}
	if ((abs(currentYEntry - minCurrentY) > fac) && (abs(maxCurrentY - minCurrentY) > fac))
	{
		filteredX[filtPos] = currentX;
		filteredY[filtPos++] = minCurrentY;
	}
	if ((abs(currentYEntry - currentYExit) > fac) && (abs(maxCurrentY - currentYExit) > fac) && (abs(minCurrentY - currentYExit) > fac))
	{
		filteredX[filtPos] = currentX;
		filteredY[filtPos++] = currentYExit;
	}
	// Return the index of the last element of the resulting array.
	return filtPos - 1;
}

// Convert trace points to screen coordinates
long Trace::DataToScrnCoords(const float* const x, const float* const y, long size, float* const filteredX, float* const filteredY) const
{
   float xOff = tracePar.getXOffset();
   float yOff = tracePar.getYOffset();
   long ixOff = xAxis_->plotDimensions()->width()*xOff;
   long iyOff = yAxis_->plotDimensions()->height()*yOff;

	for (int i = 0; i < size; i++)
	{
		filteredX[i] = xAxis_->dataToScrn(x[i],ixOff);
		filteredY[i] = yAxis_->dataToScrn(y[i],iyOff);
	}
	return(size - 1);
}

// Convert trace points to screen coordinates
long Trace::DataToScrnCoordsF(const float* const x, const float* const y, long size, float* const filteredX, float* const filteredY) const
{
   float xOff = tracePar.getXOffset();
   float yOff = tracePar.getYOffset();
   long ixOff = xAxis_->plotDimensions()->width()*xOff;
   long iyOff = yAxis_->plotDimensions()->height()*yOff;

	for (int i = 0; i < size; i++)
	{
		filteredX[i] = xAxis_->dataToScrnF(x[i],ixOff);
		filteredY[i] = yAxis_->dataToScrnF(y[i],iyOff);
	}
	return(size - 1);
}


/*****************************************************************************************/
// Plot the data in this trace as lines.
/*****************************************************************************************/	
void TraceReal::PlotLinesPrint(HDC hdc, long xL, long xR, long yT, long yB)
{
   SaveDC(hdc);

	short width = lineWidth();
	HPEN realPen = makeRealPen();

	SetBkMode(hdc,TRANSPARENT);
   SelectObject(hdc,realPen);

	if (!filteredX)
		filteredX = MakeVector(size);
	if (!filteredY)
		filteredY = MakeVector(size);

	//long filtPos = 0;

	long filtPos = DataToScrnCoords(x, y, size, filteredX, filteredY);
	filtPos = FilterOutRedundantPoints(filteredX, filteredY, filtPos+1, filteredX, filteredY);

	//filteredX[filtPos] = xAxis_->dataToScrn(x[0]);
	//filteredY[filtPos] = yAxis_->dataToScrn(y[0]);


	//for(long i = 1; i < size; i++)
 //  {
 //     if(isnan(x[i]))
 //     {
 //        continue;
	//   }    
	//	
 //     long xb = xAxis_->dataToScrn(x[i]);
 //     long yb = yAxis_->dataToScrn(y[i]);

	//	if ((xb != filteredX[filtPos]) || (yb != filteredY[filtPos]))
	//	{
	//		filteredX[++filtPos] = xb;
	//		filteredY[filtPos] = yb;
	//	}
	//}

	for (long i = 0; i < filtPos; i++)
	{
		if(LineVisible(xL,yT,xR,yB,filteredX[i],filteredY[i],filteredX[i+1],filteredY[i+1],width))  // All or part of line is visble
		{
			MoveToEx(hdc,(short)filteredX[i],(short)filteredY[i],0);
			LineTo(hdc,(short)filteredX[i+1],(short)filteredY[i+1]);
		}
	}
   
	DeleteObject(realPen);
	RestoreDC(hdc, -1);
}

void TraceReal::PlotLines(HDC hdc, long xL, long xR, long yT, long yB)
{
   SaveDC(hdc);

// Only draw points which appear at different points on the screen
	if (!filteredX)
		filteredX = MakeVector(size);
	if (!filteredY)
		filteredY = MakeVector(size);
   Axis *yAxis = this->yAxis();
   long height = yAxis->plotDimensions()->height();
   long base = yAxis->plotDimensions()->bottom();
	long filtPos = DataToScrnCoordsF(x, y, size, filteredX, filteredY);
	filtPos = FilterOutRedundantPointsF(filteredX, filteredY, filtPos+1, filteredX, filteredY);

// Set up line drawing parameters and pen

// Set scale factor

   Graphics gfx(hdc); 
	short width = lineWidth();
	short realStyle = tracePar.getRealStyle();
	COLORREF realColor = tracePar.getRealColor();
   Pen pen(Color(GetRValue(realColor), GetGValue(realColor), GetBValue(realColor))); 
   pen.SetWidth(width/2.0+0.501);
   pen.SetDashStyle((Gdiplus::DashStyle)realStyle);
   if(width > 1)
      pen.SetLineJoin(LineJoinBevel);

// Check for antialiasing
   if(parent->isAntiAliasing())
      gfx.SetSmoothingMode(SmoothingModeAntiAlias); 

// Allocate enough points for the maximum possible segment length
   PointF* pts = new PointF[filtPos+1];

// Add all points to an array
   int cnt = 0;
   int maxy = -1e7;
	for (long i = 0; i <= filtPos; i++)
	{
		pts[cnt].X = filteredX[i];
		pts[cnt].Y = filteredY[i];
      if(pts[cnt].Y > maxy) maxy = pts[cnt].Y;
      cnt++;
	}

// White wash the trace if desired
   if(this->tracePar.getWhiteWash())
   {
      Point *poly = new Point[filtPos+4];
      SolidBrush whiteBrush(Color(255, 255, 255, 255));
  //    SolidBrush whiteBrush(Color(255, 128, 0, 0));
      Pen whitePen(Color(255, 255, 255, 255));
	   for (long i = 0; i <= filtPos; i++)
	   {
		   poly[i].X = pts[i].X;
		   poly[i].Y = pts[i].Y;
      }
      poly[filtPos+1].X = poly[filtPos].X;
      poly[filtPos+1].Y = base;
      poly[filtPos+2].X = poly[0].X;
      poly[filtPos+2].Y = base;
      poly[filtPos+3].X = poly[0].X;
      poly[filtPos+3].Y = poly[0].Y;
      gfx.FillPolygon(&whiteBrush,poly,filtPos+4);
     // gfx.DrawPolygon(&whitePen,poly,filtPos+2);
      delete [] poly;
   }

// Draw left over lines
// Render the array
   gfx.DrawLines(&pen,pts,cnt);

   delete [] pts;

	RestoreDC(hdc, -1);
}


void TraceReal::PlotAllLines(HDC hdc, long xL, long xR, long yT, long yB)
{
	SaveDC(hdc);

	Graphics gfx(hdc);

	// Check for antialiasing
	if (parent->isAntiAliasing())
		gfx.SetSmoothingMode(SmoothingModeAntiAlias);

	short width = lineWidth();
	short realStyle = tracePar.getRealStyle();
	COLORREF realColor = tracePar.getRealColor();

	Pen pen(Color(GetRValue(realColor), GetGValue(realColor), GetBValue(realColor)));
	pen.SetWidth(width / 2.0 + 0.501);
	pen.SetDashStyle((Gdiplus::DashStyle)realStyle);
	if (width > 1)
		pen.SetLineJoin(LineJoinBevel);


	float xOff = tracePar.getXOffset();
	float yOff = tracePar.getYOffset();
	long ixOff = xAxis_->plotDimensions()->width() * xOff;
	long iyOff = yAxis_->plotDimensions()->height() * yOff;


	// Allocate enough points for the maximum possible segment length
	PointF* pts = new PointF[size];

	for (long i = 0; i < size; i++)
	{
		pts[i].X = xAxis_->dataToScrnF(x[i], ixOff);
		pts[i].Y = yAxis_->dataToScrnF(y[i], iyOff);
	}

	gfx.DrawLines(&pen, pts, size);

	delete[] pts;

	RestoreDC(hdc, -1);
}


/*****************************************************************************************/
// Given screen coordinates, returns the index of the closest x,y point.
/*****************************************************************************************/	
long TraceReal::indexOfClosestPoint(long x_coord, long y_coord)
{
   long mini = -1;
   float minr=1e30;
   float xOff = tracePar.getXOffset();
   float yOff = tracePar.getYOffset();
   long ixOff = xAxis_->plotDimensions()->width()*xOff;
   long iyOff = yAxis_->plotDimensions()->height()*yOff;

   for(long i = 0; i < size; i++)
   {
      long xd = xAxis_->dataToScrn(x[i],ixOff);
      long yd = yAxis_->dataToScrn(y[i],iyOff);
      float d2 = sqr(xd-x_coord) + sqr(yd-y_coord);
      if(d2 < minr)
      {
         minr = d2;
         mini = i;
      }
   }
	return mini;
}

/*****************************************************************************************/
// Read the y dimension directly from a file.
/*****************************************************************************************/	
short TraceReal::readY(FILE* fp)
{
	if(!(y = MakeVector(size)))
	{
	   FreeVector(x);
	   ErrorMessage("Can't allocate memory for 1D y data");
	   return(ERR);
	}
   fread(y,4,size,fp);
	return OK;
}

/*****************************************************************************************/
// Write the y dimension directly to a file.
/*****************************************************************************************/	
short TraceReal::writeY(FILE* fp)
{
	fwrite(y,4,size,fp);
	return OK;
}

/*****************************************************************************************/
// TODO: What is this?
/*****************************************************************************************/	
void TraceReal::CopyTo1DVarDlgProc(char* varXName, char* varYName)
{
	Variable *varX = AddGlobalVariable( MATRIX2D, varXName);  
	Variable *varY = AddGlobalVariable( MATRIX2D, varYName);

	varX->MakeMatrix2DFromVector(NULL, size,1);
	varY->MakeMatrix2DFromVector(NULL, size,1);
	
	for(long i = 0; i < size; i++)	
	{
	   VarRealMatrix(varX)[0][i] = x[i];
	   VarRealMatrix(varY)[0][i] = y[i];
	}
}

/*****************************************************************************************
     Draw a cursor at the specified screen coordinates
*****************************************************************************************/	
long TraceReal::drawCursor(HDC hdc, long x_coord, long y_coord)
{
   Plot1D *locParent = parent;
   extern bool gScrollWheelEvent ;
   float xOff = tracePar.getXOffset();
   float yOff = tracePar.getYOffset();
   long ixOff = xAxis_->plotDimensions()->width()*xOff;
   long iyOff = yAxis_->plotDimensions()->height()*yOff;

   SaveDC(hdc);
   
	long mini = indexOfClosestPoint(x_coord,y_coord);
	long x_scr = xAxis_->dataToScrn(X(mini),ixOff);
	long y_scr = yAxis_->dataToScrn(y[mini],iyOff);

	COLORREF penCol = this->tracePar.getRealColor();
	HPEN cursorPen = CreatePen(PS_DOT, 0, penCol);
   SelectObject(hdc,cursorPen);

   gScrollWheelEvent = false;

   MoveToEx(hdc,locParent->GetLeft(),y_scr,0);
   LineTo(hdc,locParent->GetLeft()+locParent->GetWidth(),y_scr);
   MoveToEx(hdc,x_scr,locParent->GetTop(),0);
   LineTo(hdc,x_scr,locParent->GetTop()+locParent->GetHeight());      
	Plot1D::dataCursorVisible_ = true;

   DeleteObject(cursorPen);
   RestoreDC(hdc, -1);

	return(mini);
}

/*****************************************************************************************
*                           Plot any error bars                                          *
*****************************************************************************************/
void TraceReal::PlotErrorBars(HDC hdc, int scale)
{
	if (!tracePar.isFixedErrorBars() && !bars)
		return;

	SaveDC(hdc);
	HPEN barPen = isPrinting() ? CreatePen(PS_SOLID,0,RGB_BLACK) : CreatePen(PS_SOLID,0,tracePar.getBarColor());
	SelectObject(hdc,barPen);

	float yb_up = 0;
	float yb_dn = 0;
	long ys_up, ys_dn, xs;
	
	float fixedHeight = tracePar.getBarFixedHeight();

	for (long i = 0; i < size; i++)
	{
		if (tracePar.isFixedErrorBars())
		{
         yb_up = yb_dn = fixedHeight;
		}
      else if(bars)
      {
         yb_dn = bars[0][i];
         yb_up = bars[1][i];
      } 

      xs = xAxis_->dataToScrn(x[i]);
      ys_up = yAxis_->dataToScrn(y[i] - yb_up);
      ys_dn = yAxis_->dataToScrn(y[i] + yb_dn); 
		MoveToEx(hdc,(short)xs,(short)ys_up,0);
		LineTo(hdc,(short)xs,(short)ys_dn); 
	
		MoveToEx(hdc,(short)(xs - (2 * scale)),(short)ys_up,0);
		LineTo(hdc,(short)(xs + (3 * scale)),(short)ys_up); 
	
 	   MoveToEx(hdc,(short)(xs - (2 * scale)),(short)ys_dn,0);
		LineTo(hdc,(short)(xs + (3 * scale)),(short)ys_dn);  		  		    
	}  
	RestoreDC(hdc, -1);
   DeleteObject(barPen);	
}

/*****************************************************************************************/
// Draw symbols on the plot at the data points.
/*****************************************************************************************/	
void TraceReal::PlotDataSymbols(HDC hdc, HBRUSH bkgBrush, HPEN bkgPen, long szX, long szY, long xL, long yT, long xR, long yB)
{
   SaveDC(hdc);
   float xOff = tracePar.getXOffset();
   float yOff = tracePar.getYOffset();
   long ixOff = xAxis_->plotDimensions()->width()*xOff;
   long iyOff = yAxis_->plotDimensions()->height()*yOff;

	COLORREF symbolColor = tracePar.getRealSymbolColor();
	HBRUSH symbolBrush = isPrinting() ? (HBRUSH)GetStockObject(BLACK_BRUSH) : CreateSolidBrush(symbolColor);
	HPEN symbolPen = isPrinting() ? (HPEN)CreatePen(PS_SOLID,0,RGB_BLACK) : CreatePen(PS_SOLID,0,symbolColor);
	SelectObject(hdc,symbolPen);	     
   for(long i = 0; i < size; i++)
   {
      long xp = xAxis_->dataToScrn(x[i],ixOff);
	  	long yp = yAxis_->dataToScrn(y[i],iyOff);
		if(SymbolVisible(xL,yT,xR,yB,xp,yp,szX))  // All or part of line is visble
			parent->DrawSymbol(hdc,tracePar.getSymbolType(), symbolBrush, symbolPen, bkgBrush, bkgPen, xp,yp,szX,szY);
		
   }
	DeleteObject(symbolPen);
	DeleteObject(symbolBrush);
	RestoreDC(hdc, -1);
}


Trace* TraceReal::CursorOnDataLine(short x, short y, long *mini, float *dist)
{
	Trace* candidate = 0;
	short type = tracePar.getTraceType();
   *dist = 1e30; // Initialise to an impossibly big distance
   float xOff = tracePar.getXOffset();
   float yOff = tracePar.getYOffset();
   long ixOff = xAxis_->plotDimensions()->width()*xOff;
   long iyOff = yAxis_->plotDimensions()->height()*yOff;

	if(type == PLOT_TRACE_LINES)
   {
      for(int i = 1; i < size; i++) // Search through each data set
      {
         float xd1 = (float)this->x[i-1];
         float yd1 = (float)this->y[i-1];
         float xd2 = (float)this->x[i];
         float yd2 = (float)this->y[i];

         float xs1 = (float)xAxis_->dataToScrn(xd1,ixOff); // Convert last point to screen coordinates
         float ys1 = (float)yAxis_->dataToScrn(yd1,iyOff);
         
         float xs2 = (float)xAxis_->dataToScrn(xd2,ixOff); // Convert this point to screen coordinates
         float ys2 = (float)yAxis_->dataToScrn(yd2,iyOff);
         
			float r2 = Plot1D::PointToLineDist((float)x,(float)y,xs1,ys1,xs2,ys2); // Find distance to cursor squared

         if(r2 >= 0 && r2 < *dist)
         {
            if((sqr(x-xs1)+sqr(y-ys1)) < (sqr(x-xs2)+sqr(y-ys2)))
               *mini = i-1;
            else
               *mini = i;
            *dist = r2;
            candidate = this;
         }
      } 
   }
   else if(type == PLOT_TRACE_STAIRS)
   {
      for(int i = 1; i < size; i++) // Search through each data set
      {
         float xd1 = (float)this->x[i-1];
         float yd1 = (float)this->y[i-1];
         float xd2 = (float)this->x[i];
         float yd2 = (float)this->y[i];

         float xs1 = (float)xAxis_->dataToScrn(xd1); // Convert last point to screen coordinates
         float ys1 = (float)yAxis_->dataToScrn(yd1);
         
         float xs2 = (float)xAxis_->dataToScrn(xd2); // Convert this point to screen coordinates
         float ys2 = (float)yAxis_->dataToScrn(yd2);
         
			float r1 = Plot1D::PointToLineDist((float)x,(float)y,xs1,ys1,xs2,ys1); // Find distance to horizontal line
         float r2 = Plot1D::PointToLineDist((float)x,(float)y,xs2,ys1,xs2,ys2); // Find distance to vertical line

         if(r1 >= 0 && r2 >= 0 && r1 < r2) r2 = r1;
         if(r1 >= 0 && r2 < 0) r2 = r1;
      	
         if(r2 >= 0 && r2 < *dist) // Is it a minimum
         {
            *mini = i;
            *dist = r2;
            candidate = this;
         }
      } 
   }
   else if(type == PLOT_TRACE_DOTS || type == PLOT_TRACE_NONE)
   {
      for(int i = 1; i < size; i++) // Search through each data set
      {
	      float xd1 = (float)this->x[i];
         float yd1 = (float)this->y[i];

         float xs1 = (float)xAxis_->dataToScrn(xd1); // Convert point to screen coordinates
         float ys1 = (float)yAxis_->dataToScrn(yd1);
      
         float r2 = sqr(xs1-x) + sqr(ys1-y); // Find distance to cursor squared

         if(r2 < *dist)
         {
            *mini = i;
            *dist = r2;
            candidate = this;
         }
      } 
   }
   if(*dist <= MIN_DIST2) // Must be 5 pixels or closer to line
      return(candidate);
   else
      return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////

TraceComplex::TraceComplex(Plot1D* parent)
: Trace(parent)
{
	yc = 0;
}

TraceComplex::TraceComplex(float *x, complex *yc, long size, char *name, TracePar* tp, Plot1D* parent, bool append)
: Trace(x,size,name,tp,parent)
{
	this->yc = yc;
	DiscoverXRange();	
	DiscoverYRange(this->getMinX(), this->getMaxX());
   if(append)
	   parent->appendTrace(this);
}

TraceComplex::TraceComplex(const TraceComplex& copyMe)
: Trace(copyMe)
{
	if (copyMe.yc)
		yc = CopyCArray(copyMe.yc, copyMe.size);
	else
		yc = 0;
}

/*****************************************************************************************/
// Use "clone" to make a deep copy of a TraceComplex.
/*****************************************************************************************/
Trace* TraceComplex::clone() const
{
	return new TraceComplex(*this);
}

TraceComplex::~TraceComplex()
{
	GlobalFree(yc);
}

/*****************************************************************************************/
// Are all y-values greater than zero?
/*****************************************************************************************/
bool TraceComplex::greaterThanZero(char dir)
{
	if ('x' == dir)
	{
		for (long i = 0; i < size; i++)
			if (x[i] <= 0)
				return false;
	}
	else if (yc) 
	{
		for (long i = 0; i < size; i++)
			if (yc[i].r <= 0 || yc[i].i <= 0)
				return false;
	}
	return true;
}


/*****************************************************************************************/
// Get the max and min X values of this trace.
/*****************************************************************************************/
void TraceComplex::DiscoverXRange(float* lhs, float* rhs)
{
   float minx = 1e30;
	float maxx = -1e30;  
	for(long i = 0; i < this->size; i++)
   {
      if(isnan(this->x[i]) || isnan(this->yc[i].r) || isnan(this->yc[i].i)) continue;
      if(this->x[i] > maxx) maxx = this->x[i];
      if(this->x[i] < minx) minx = this->x[i];
   }

// Make sure range is never zero
   if(minx == maxx)
   {
      minx = minx-1;
      maxx = maxx+1;
   }

// Copy to trace data structure
   this->setMinX(minx);
   this->setMaxX(maxx);

// Return those results if requested
	if (lhs)
		*lhs = x[0];
	if (rhs)
		*rhs = x[size-1];
}


/*****************************************************************************************
*        Return the full y range of the complex data stored in dat               *
*****************************************************************************************/
void TraceComplex::DiscoverYRange(float minx,float maxx)
{
   float miny  = 1e30;
   float maxy  = -1e30;

   for(long i = 0; i < this->size; i++)
   {
      if(this->x[i] >= minx && this->x[i] <= maxx)
      {
         if(this->yc[i].r > maxy) maxy = this->yc[i].r;
         if(this->yc[i].r < miny) miny = this->yc[i].r;
         if(this->yc[i].i > maxy) maxy = this->yc[i].i;
         if(this->yc[i].i < miny) miny = this->yc[i].i;	        
      }
   }
	
//// Make sure range is never zero
//   if(miny == maxy)
//   {
//      if(miny == 0)
//      {
//         miny = miny-1;
//         maxy = maxy+1;
//      }
//      else
//      {
//         miny = miny-fabs(miny)/10;
//         maxy = maxy+fabs(maxy)/10;
//      }
//   }

// Copy to trace data structure    
   this->setMinY(miny);
   this->setMaxY(maxy);
}


/*****************************************************************************************
*                       Get range of data references in structure dat                    *
*****************************************************************************************/
void TraceComplex::GetXRange()
{
   float minx_local  = 1e30;
   float maxx_local  = -1e30;
   
   for(long i = 0; i < size; i++)
   {
      if(isnan(x[i]) || isnan(yc[i].r) || isnan(yc[i].i)) continue;
      if(x[i] > maxx_local) maxx_local = x[i];
      if(x[i] < minx_local) minx_local = x[i];
   }

// Make sure range is never zero
   if(minx_local == maxx_local)
   {
      minx_local = minx_local-1;
      maxx_local = maxx_local+1;
   }

// Copy to trace data structure
   setMinX(minx_local);
   setMaxX(maxx_local);
}


/*****************************************************************************************
*        Return the full y range of the real or complex data stored in dat               *
*****************************************************************************************/
void TraceComplex::GetYRange(float minx,float maxx)
{
   float miny_local  = 1e30;
   float maxy_local  = -1e30;

   for(long i = 0; i < size; i++)
   {
      if(x[i] >= minx && x[i] <= maxx)
      {
         if(yc[i].r > maxy_local) maxy_local = yc[i].r;
         if(yc[i].r < miny_local) miny_local = yc[i].r;
         if(yc[i].i > maxy_local) maxy_local = yc[i].i;
         if(yc[i].i < miny_local) miny_local = yc[i].i;	         
      }
   }
   
// Make sure range is never zero
   if(miny_local == maxy_local)
   {
      if(miny_local == 0)
      {
         miny_local = miny_local-1;
         maxy_local = maxy_local+1;
      }
      else
      {
         miny_local = miny_local-fabs(miny_local)/10;
         maxy_local = maxy_local+fabs(maxy_local)/10;
      }
   }

// Copy to trace data structure
   setMinY(miny_local);
   setMaxY(maxy_local);
}


/*****************************************************************************************/
// Produce a new Variable for a dimension's data. 
/*****************************************************************************************/
Variable* TraceComplex::yComponentAsVariable()
{
	Variable* v = new Variable();
	v->SetData((char*)MakeCMatrix2D(size,1));
   if(!v->GetData()) 
   {
      ErrorMessage("unable to allocate 2D cmatrix memory");
      v->SetType(NULL_VARIABLE);
      v->SetDim1(0);
		v->SetDim2(0);
		v->SetDim3(0);
		v->SetDim4(0);
      return v;
   }

   complex **temp = (complex**)v->GetData();
   v->SetDim1(size);
   v->SetDim2(1);
   v->SetDim3(1);
   v->SetDim4(1);
   v->SetAlias(NULL);

   if(yc != NULL)
   {
      long j= 0;
	   for(long y = 0; y < 1; y++)
	   {
		   for(long x = 0; x < size; x++,j++)
		   {
		      temp[y][x] = yc[j];
		   }
	   }
   }
   v->SetType(CMATRIX2D);
	return v;
}


/*****************************************************************************************/
// Produce a new Variable for a dimension's data. 
/*****************************************************************************************/
Variable* TraceComplex::yMinMaxAsVariable()
{
	long start = FindIndexCore(x, size, xAxis()->Min());
	long end = FindIndexCore(x, size, xAxis()->Max());
	long size = end - start + 1;

	Variable* v = new Variable;
	v->MakeAndLoadCMatrix2D(0,size,1);
	complex** mY = v->GetCMatrix2D();
	for(int i = start; i <= end; i++)
	{
		mY[0][i-start] = yc[i];
	}
	return v;
}

/*****************************************************************************************/
// Get the name of the type of data in this trace as a string. 
/*****************************************************************************************/
string& TraceComplex::typeString() const
{
	static string typeName = string("Complex");
	return typeName;
}

/*****************************************************************************************/
// Plot the data in this trace as stairs.
/*****************************************************************************************/	

void TraceComplex::PlotStairs(HDC hdc,	long xL, long xR, long yT, long yB) const
{	
   SaveDC(hdc);

	short width = lineWidth();
   float xOff = tracePar.getXOffset();
   float yOff = tracePar.getYOffset();
   long ixOff = xAxis_->plotDimensions()->width()*xOff;
   long iyOff = yAxis_->plotDimensions()->height()*yOff;

	if (parent->display1DComplex & SHOW_REAL)
	{
	// Plot real part using a solid line
		HPEN realPen = makeRealPen();
      SelectObject(hdc,realPen);		
    
	   long xa = xAxis_->dataToScrn(x[0],ixOff);
	   long ya = yAxis_->dataToScrn(yc[0].r,iyOff);
	
	   for(long i = 0; i < size; i++)
	   {
	      long xb = xAxis_->dataToScrn(x[i],ixOff);
	      long yb = yAxis_->dataToScrn(yc[i].r,iyOff);
	
	      if(LineVisible(xL,yT,xR,yB,xa,ya,xb,yb,width))  // All or part of line is visble
	      {            
		      if(i == 0)	                
	            MoveToEx(hdc,(short)xb,(short)yb,0);
		      else
		      {
			      LineTo(hdc,(short)xb,(short)ya);
			      LineTo(hdc,(short)xb,(short)yb);
		      }
	      }
	      else // Line is not visible at all so just move to end
	      {
	         MoveToEx(hdc,(short)xb,(short)yb,0);
		   }
	      xa = xb; ya = yb;
	   }
		DeleteObject(realPen);
   }
   
   if(parent->display1DComplex & SHOW_IMAGINARY)
   {      
	// Plot imaginary part using a paler line
		HPEN imagPen = makeImagPen();
      SelectObject(hdc,imagPen);
      SetBkMode(hdc,TRANSPARENT);
    
	   long xa = xAxis_->dataToScrn(x[0]);
	   long ya = yAxis_->dataToScrn(yc[0].i,iyOff);

	   for(long i = 0; i < size; i++)
	   {
	      long xb = xAxis_->dataToScrn(x[i]);
	      long yb = yAxis_->dataToScrn(yc[i].i,iyOff);
	
	      if(LineVisible(xL,yT,xR,yB,xa,ya,xb,yb,width))  // All or part of line is visble
	      {            
		      if(i == 0)	                
	            MoveToEx(hdc,(short)xb,(short)yb,0);
		      else
		      {
			      LineTo(hdc,(short)xb,(short)ya);
			      LineTo(hdc,(short)xb,(short)yb);
		      }
	      }
	      else // Line is not visible at all so just move to end
	      {
	         MoveToEx(hdc,(short)xb,(short)yb,0);
		   }
	      xa = xb; ya = yb;
	   }
		DeleteObject(imagPen);
	} 
	RestoreDC(hdc, -1);
}


/*****************************************************************************************/
// Plot the data in this trace as points.
/*****************************************************************************************/	

void TraceComplex::PlotPoints(HDC hdc, long xL, long xR, long yT, long yB) const
{
   float xOff = tracePar.getXOffset();
   float yOff = tracePar.getYOffset();
   long ixOff = xAxis_->plotDimensions()->width()*xOff;
   long iyOff = yAxis_->plotDimensions()->height()*yOff;

	if(parent->display1DComplex & SHOW_REAL)
   {	
		COLORREF realColor = isPrinting()? RGB_BLACK : tracePar.getRealColor();
	   for(long i = 0; i < size; i++)
	   {
	      if(isnan(x[i])) 
	      {
	         continue;
	      }
	      long xp = xAxis_->dataToScrn(x[i],ixOff);
	      long yp = yAxis_->dataToScrn(yc[i].r,iyOff);
	      if(PointVisible(xL,yT,xR,yB,xp,yp))   
	      	 SetPixel(hdc,xp,yp,realColor); 
	   }
	}
   if(parent->display1DComplex & SHOW_IMAGINARY)
   {		
		COLORREF imagColor = isPrinting()? RGB_BLACK : tracePar.getImagColor();
	   for(long i = 0; i < size; i++)
	   {
	      if(isnan(x[i])) 
	      {
	         continue;
	      }
	      long xp = xAxis_->dataToScrn(x[i],ixOff);
	      long yp = yAxis_->dataToScrn(yc[i].i,iyOff);
	      if(PointVisible(xL,yT,xR,yB,xp,yp))   
	      	 SetPixel(hdc,xp,yp,imagColor);  	      	        
	   }	
	}   
}


/****************************************************************************************
   Plot the data in this trace as lines.
*****************************************************************************************/	
void TraceComplex::PlotLinesPrint(HDC hdc,	long xL, long xR, long yT, long yB)
{
   SaveDC(hdc);
	SetBkMode(hdc,TRANSPARENT);

	short width = lineWidth();
	
	if (!filteredX)
		filteredX = MakeVector(size);
	if (!filteredY)
		filteredY = MakeVector(size);

   if(parent->display1DComplex & SHOW_IMAGINARY) // Display imaginary on bottom
   {	
	// Plot imaginary part using a paler line
		HPEN imagPen = makeImagPen();
      SelectObject(hdc,imagPen);
    
		//long filtPos = 0;
		//filteredX[filtPos] = xAxis_->dataToScrn(x[0]);
		//filteredY[filtPos] = yAxis_->dataToScrn(yc[0].i);

		float* imaginary_y = new float[size];
		for(int i = 0; i < size; i++)
		{
			imaginary_y[i] = yc[i].i;
		}

		long filtPos = DataToScrnCoords(x, imaginary_y, size, filteredX, filteredY);
		filtPos = FilterOutRedundantPoints(filteredX, filteredY, filtPos+1, filteredX, filteredY);
		delete[] imaginary_y;

		//for(long i = 1; i < size; i++)
		//{
		//	if(isnan(x[i]))
		//	{
	 //        continue;
		//	}    
		//
		//	long xb = xAxis_->dataToScrn(x[i]);
		//	long yb = yAxis_->dataToScrn(yc[i].i);

		//	if ((xb != filteredX[filtPos]) || (yb != filteredY[filtPos]))
		//	{
		//		filteredX[++filtPos] = xb;
		//		filteredY[filtPos] = yb;
		//	}
		//}

		for (long i = 0; i < filtPos; i++)
		{
			if(LineVisible(xL,yT,xR,yB,filteredX[i],filteredY[i],filteredX[i+1],filteredY[i+1],width))  // All or part of line is visble
			{
				MoveToEx(hdc,(short)filteredX[i],(short)filteredY[i],0);
				LineTo(hdc,(short)filteredX[i+1],(short)filteredY[i+1]);
			}
		}
		DeleteObject(imagPen);
	}

   if(parent->display1DComplex & SHOW_REAL) // Display real on top
   {	
	// Plot real part using a solid line	
		HPEN realPen = makeRealPen();
	   SelectObject(hdc,realPen);	

		float* real_y = new float[size];
		for(int i = 0; i < size; i++)
		{
			real_y[i] = yc[i].r;
		}

		long filtPos = DataToScrnCoords(x, real_y, size, filteredX, filteredY);
		filtPos = FilterOutRedundantPoints(filteredX, filteredY, filtPos+1, filteredX, filteredY);
		delete[] real_y;

		//long filtPos = 0;
		//filteredX[filtPos] = xAxis_->dataToScrn(x[0]);
		//filteredY[filtPos] = yAxis_->dataToScrn(yc[0].r);

		//for(long i = 1; i < size; i++)
		//{
		//	if(isnan(x[i]))
		//	{
		//		continue;
		//	}    

		//	long xb = xAxis_->dataToScrn(x[i]);
		//	long yb = yAxis_->dataToScrn(yc[i].r);

		//	if ((xb != filteredX[filtPos]) || (yb != filteredY[filtPos]))
		//	{
		//		filteredX[++filtPos] = xb;
		//		filteredY[filtPos] = yb;
		//	}
		//}

		for (long i = 0; i < filtPos; i++)
		{
			if(LineVisible(xL,yT,xR,yB,filteredX[i],filteredY[i],filteredX[i+1],filteredY[i+1],width))  // All or part of line is visble
			{
				MoveToEx(hdc,(short)filteredX[i],(short)filteredY[i],0);
				LineTo(hdc,(short)filteredX[i+1],(short)filteredY[i+1]);
			}
		}
		DeleteObject(realPen);
	}
	RestoreDC(hdc, -1);
}

void TraceComplex::PlotAllLines(HDC hdc, long xL, long xR, long yT, long yB)
{
	PlotLines(hdc, xL, xR, yT, yB);
}

/*****************************************************************************************
  Plot the data in this trace as lines.
*****************************************************************************************/	

void TraceComplex::PlotLines(HDC hdc,	long xL, long xR, long yT, long yB)
{
   SaveDC(hdc);

   Axis *yAxis = this->yAxis();
   long height = yAxis->plotDimensions()->height();
   long base = yAxis->plotDimensions()->bottom();

// Set up line drawing parameters and pen
   Graphics gfx(hdc); 

// Check for antialiasing
   if(parent->isAntiAliasing())
      gfx.SetSmoothingMode(SmoothingModeAntiAlias); //SmoothingModeAntiAlias8x8); /

	COLORREF realColor = tracePar.getRealColor();
   Pen penr(Color(GetRValue(realColor), GetGValue(realColor), GetBValue(realColor))); 
   penr.SetWidth(lineWidth()/2.0+0.501);
   penr.SetDashStyle((Gdiplus::DashStyle)tracePar.getRealStyle());
   if(lineWidth() > 1)
      penr.SetLineJoin(LineJoinBevel);

	COLORREF imagColor = tracePar.getImagColor();
   Pen peni(Color(GetRValue(imagColor), GetGValue(imagColor), GetBValue(imagColor))); 
   peni.SetWidth(lineWidth()/2.0+0.501);
   peni.SetDashStyle((Gdiplus::DashStyle)tracePar.getImagStyle());
   if(lineWidth() > 1)
      peni.SetLineJoin(LineJoinBevel);

// Only draw points which appear at different points on the screen
	if (!filteredX)
		filteredX = MakeVector(size);
	if (!filteredY)
		filteredY = MakeVector(size);

// Display magnitude ***********************************

   if(parent->display1DComplex & SHOW_MAGNITUDE) 
   {	
		float* mag_y = new float[size];
		for(int i = 0; i < size; i++)
		{
			mag_y[i] = sqrt(yc[i].r*yc[i].r + yc[i].i*yc[i].i);
		}

		long filtPos = DataToScrnCoordsF(x, mag_y, size, filteredX, filteredY);
		filtPos = FilterOutRedundantPointsF(filteredX, filteredY, filtPos+1, filteredX, filteredY);
		delete[] mag_y;

	// Allocate enough points for the maximum possible segment length
		PointF* pts = new PointF[filtPos+1];

	// Add all points to an array
		int cnt = 0;
		int maxy = -1e7;
		for (long i = 0; i <= filtPos; i++)
		{
			pts[cnt].X = filteredX[i];
			pts[cnt].Y = filteredY[i];
			if(pts[cnt].Y > maxy) maxy = pts[cnt].Y;
			cnt++;
		}

	// White wash the trace if desired
		if(this->tracePar.getWhiteWash())
		{
			Point *poly = new Point[filtPos+4];
			SolidBrush whiteBrush(Color(255, 255, 255, 255));
			Pen whitePen(Color(255, 255, 255, 255));
			for (long i = 0; i <= filtPos; i++)
			{
				poly[i].X = pts[i].X;
				poly[i].Y = pts[i].Y;
			}
			poly[filtPos+1].X = poly[filtPos].X;
			poly[filtPos+1].Y = base;
			poly[filtPos+2].X = poly[0].X;
			poly[filtPos+2].Y = base;
			poly[filtPos+3].X = poly[0].X;
			poly[filtPos+3].Y = poly[0].Y;
			gfx.FillPolygon(&whiteBrush,poly,filtPos+4);
			delete [] poly;
		}

	// Draw lines
		gfx.DrawLines(&penr,pts,cnt);

		delete [] pts;
	}

// Display imaginary on bottom ***********************************
   if(parent->display1DComplex & SHOW_IMAGINARY) 
   {	
		float* imaginary_y = new float[size];
		for(int i = 0; i < size; i++)
		{
			imaginary_y[i] = yc[i].i;
		}

		long filtPos = DataToScrnCoordsF(x, imaginary_y, size, filteredX, filteredY);
		filtPos = FilterOutRedundantPointsF(filteredX, filteredY, filtPos+1, filteredX, filteredY);
		delete[] imaginary_y;

	// Allocate enough points for the maximum possible segment length
		PointF* pts = new PointF[filtPos+1];

	// Add all points to an array
		int cnt = 0;
		int maxy = -1e7;
		for (long i = 0; i <= filtPos; i++)
		{
			pts[cnt].X = filteredX[i];
			pts[cnt].Y = filteredY[i];
			if(pts[cnt].Y > maxy) maxy = pts[cnt].Y;
			cnt++;
		}

	// White wash the trace if desired
		if(this->tracePar.getWhiteWash())
		{
			Point *poly = new Point[filtPos+4];
			SolidBrush whiteBrush(Color(255, 255, 255, 255));
			Pen whitePen(Color(255, 255, 255, 255));
			for (long i = 0; i <= filtPos; i++)
			{
				poly[i].X = pts[i].X;
				poly[i].Y = pts[i].Y;
			}
			poly[filtPos+1].X = poly[filtPos].X;
			poly[filtPos+1].Y = base;
			poly[filtPos+2].X = poly[0].X;
			poly[filtPos+2].Y = base;
			poly[filtPos+3].X = poly[0].X;
			poly[filtPos+3].Y = poly[0].Y;
			gfx.FillPolygon(&whiteBrush,poly,filtPos+4);
			delete [] poly;
		}

	// Draw left over lines
		gfx.DrawLines(&peni,pts,cnt);

		delete [] pts;
	}


// Display real part on top ***********************************
   if(parent->display1DComplex & SHOW_REAL) 
   {	
		float* real_y = new float[size];
		for(int i = 0; i < size; i++)
		{
			real_y[i] = yc[i].r;
		}

		long filtPos = DataToScrnCoordsF(x, real_y, size, filteredX, filteredY);
		filtPos = FilterOutRedundantPointsF(filteredX, filteredY, filtPos+1, filteredX, filteredY);
		delete[] real_y;

	// Allocate enough points for the maximum possible segment length
		PointF* pts = new PointF[filtPos+1];

	// Add all points to an array
		int cnt = 0;
		int maxy = -1e7;
		for (long i = 0; i <= filtPos; i++)
		{
			pts[cnt].X = filteredX[i];
			pts[cnt].Y = filteredY[i];
			if(pts[cnt].Y > maxy) maxy = pts[cnt].Y;
			cnt++;
		}

	// White wash the trace if desired
		if(this->tracePar.getWhiteWash())
		{
			Point *poly = new Point[filtPos+4];
			SolidBrush whiteBrush(Color(255, 255, 255, 255));
			Pen whitePen(Color(255, 255, 255, 255));
			for (long i = 0; i <= filtPos; i++)
			{
				poly[i].X = pts[i].X;
				poly[i].Y = pts[i].Y;
			}
			poly[filtPos+1].X = poly[filtPos].X;
			poly[filtPos+1].Y = base;
			poly[filtPos+2].X = poly[0].X;
			poly[filtPos+2].Y = base;
			poly[filtPos+3].X = poly[0].X;
			poly[filtPos+3].Y = poly[0].Y;
			gfx.FillPolygon(&whiteBrush,poly,filtPos+4);
			delete [] poly;
		}

	// Draw left over lines
		gfx.DrawLines(&penr,pts,cnt);

		delete [] pts;

	}

	RestoreDC(hdc, -1);
}

/*****************************************************************************************/
// Given screen coordinates, returns the index of the closest x,y point.
/*****************************************************************************************/	

long TraceComplex::indexOfClosestPoint(long x_coord, long y_coord)
{
	long mini = -1;
   float minr=1e30;
   for(long i = 0; i < size; i++)
   {
      long xd = xAxis_->dataToScrn(x[i]);
      float d2 = sqr(xd-x_coord);
      if(d2 < minr)
      {
         minr = d2;
         mini = i;
      }
   }
	return mini;
}


/*****************************************************************************************/
// Read the y dimension directly from a file.
/*****************************************************************************************/	

short TraceComplex::readY(FILE* fp)
{
	if(!(yc = MakeCVector(size)))
	{
	   FreeVector(x);
	   ErrorMessage("Can't allocate memory for 1D y data");
	   return(ERR);
	}
   fread(yc,8,size,fp);
	return OK;
}

/*****************************************************************************************/
// Write the y dimension directly to a file.
/*****************************************************************************************/	

short TraceComplex::writeY(FILE* fp)
{
	fwrite(yc,8,size,fp);
	return OK;
}

/*****************************************************************************************/
// TODO: What is this?
/*****************************************************************************************/	

void TraceComplex::CopyTo1DVarDlgProc(char* varXName, char* varYName)
{
	Variable* varX = AddGlobalVariable( MATRIX2D, varXName);
	Variable* varY = AddGlobalVariable( CMATRIX2D, varYName);

	varX->MakeMatrix2DFromVector(NULL, size,1);
	varY->MakeCMatrix2DFromCVector(NULL, size,1);
	
	for(long i = 0; i < size; i++)	
	{
	   VarRealMatrix(varX)[0][i] = X(i);
	   VarComplexMatrix(varY)[0][i] = yc[i];
	}

}


/*****************************************************************************************/
// Draw symbols on the plot at the data points.
/*****************************************************************************************/	

void TraceComplex::PlotDataSymbols(HDC hdc,  
										  HBRUSH bkgBrush, HPEN bkgPen, long szX, long szY, long xL, long yT, long xR, long yB)
{
	COLORREF symbolColor = tracePar.getRealSymbolColor();
	COLORREF imagSymbolColor = tracePar.getImagSymbolColor();
   float xOff = tracePar.getXOffset();
   float yOff = tracePar.getYOffset();
   long ixOff = xAxis_->plotDimensions()->width()*xOff;
   long iyOff = yAxis_->plotDimensions()->height()*yOff;

	HBRUSH symbolBrush = isPrinting() ? (HBRUSH)GetStockObject(BLACK_BRUSH) : CreateSolidBrush(symbolColor);
	HPEN symbolPen = isPrinting() ? (HPEN)CreatePen(PS_SOLID,0,RGB_BLACK) : CreatePen(PS_SOLID,0,symbolColor);
	HBRUSH symbolBrushImag = isPrinting() ? (HBRUSH)GetStockObject(BLACK_BRUSH) : CreateSolidBrush(imagSymbolColor);
	HPEN symbolPenImag = isPrinting() ? CreatePen(PS_SOLID,0,RGB_BLACK) : CreatePen(PS_SOLID, 0, imagSymbolColor);
   SaveDC(hdc);
	SelectObject(hdc,symbolPen);	      
   for(long i = 0; i < size; i++)
   {
      long xp = xAxis_->dataToScrn(X(i));
		if(parent->display1DComplex & SHOW_REAL)
      {
			long yp = yAxis_->dataToScrn(yc[i].r,iyOff);
		   if(SymbolVisible(xL,yT,xR,yB,xp,yp,szX))  // All or part of line is visble
				parent->DrawSymbol(hdc,tracePar.getSymbolType(), symbolBrush, symbolPen, bkgBrush, bkgPen, xp,yp,szX,szY);
		}
      if(parent->display1DComplex & SHOW_IMAGINARY)
      {		   
   	   long yp = yAxis_->dataToScrn(yc[i].i,iyOff);
		   if(SymbolVisible(xL,yT,xR,yB,xp,yp,szX))  // All or part of line is visble
	         parent->DrawSymbol(hdc,tracePar.getSymbolType(), symbolBrushImag, symbolPenImag, bkgBrush, bkgPen, xp,yp,szX,szY);
	   }
   }
	DeleteObject(symbolPen);
	DeleteObject(symbolPenImag);
	DeleteObject(symbolBrush);
	DeleteObject(symbolBrushImag);
	RestoreDC(hdc, -1);
}


/*****************************************************************************************
     Draw a cursor at the specified screen coordinates
*****************************************************************************************/	


long TraceComplex::drawCursor(HDC hdc, long x_coord, long y_coord)
{
	Plot1D *locParent = parent;
   extern bool gScrollWheelEvent;
   float xOff = tracePar.getXOffset();
   float yOff = tracePar.getYOffset();
   long ixOff = xAxis_->plotDimensions()->width()*xOff;
   long iyOff = yAxis_->plotDimensions()->height()*yOff;

   SaveDC(hdc);
	
	long mini = this->indexOfClosestPoint(x_coord,y_coord);
   long x_scr = xAxis_->dataToScrn(X(mini),ixOff);
	long yr_scr = yAxis_->dataToScrn(yc[mini].r,iyOff);
	long yi_scr = yAxis_->dataToScrn(yc[mini].i,iyOff);
   long ym_scr = yAxis_->dataToScrn(sqrt(yc[mini].r*yc[mini].r + yc[mini].i*yc[mini].i),iyOff);
	COLORREF penRealCol = this->tracePar.getRealColor();
	COLORREF penImagCol = this->tracePar.getImagColor();

   HPEN cursorRealPen = CreatePen(PS_DOT,0, penRealCol);
   HPEN cursorImagPen = CreatePen(PS_DOT,0, penImagCol);

   gScrollWheelEvent= false;

   if(locParent->display1DComplex & SHOW_MAGNITUDE)
   {	
	   SelectObject(hdc,cursorRealPen);
      MoveToEx(hdc,locParent->GetLeft(),ym_scr,0);
      LineTo(hdc,locParent->GetLeft()+locParent->GetWidth(),ym_scr);
		MoveToEx(hdc,x_scr,locParent->GetTop(),0);
		LineTo(hdc,x_scr,locParent->GetTop()+locParent->GetHeight());
   }
   else
   {
      if(locParent->display1DComplex & SHOW_REAL)
      {	
	      SelectObject(hdc,cursorRealPen);
         MoveToEx(hdc,locParent->GetLeft(),yr_scr,0);
         LineTo(hdc,locParent->GetLeft()+locParent->GetWidth(),yr_scr);
			MoveToEx(hdc,x_scr,locParent->GetTop(),0);
		   LineTo(hdc,x_scr,locParent->GetTop()+locParent->GetHeight());
      }
   
      if(locParent->display1DComplex & SHOW_IMAGINARY)
      {	
	      SelectObject(hdc,cursorImagPen);
         MoveToEx(hdc,locParent->GetLeft(),yi_scr,0);
         LineTo(hdc,locParent->GetLeft()+locParent->GetWidth(),yi_scr);
			MoveToEx(hdc,x_scr,locParent->GetTop(),0);
	   	LineTo(hdc,x_scr,locParent->GetTop()+locParent->GetHeight());
      }
   }
   
   locParent->dataCursorVisible_ = true;

   DeleteObject(cursorRealPen);
   DeleteObject(cursorImagPen);
   RestoreDC(hdc, -1);
	return(mini);
}


Trace* TraceComplex::CursorOnDataLine(short x, short y, long *mini, float *dist)
{
	Trace* candidate = 0;
	short type = tracePar.getTraceType();
   float xOff = tracePar.getXOffset();
   float yOff = tracePar.getYOffset();
   long ixOff = xAxis_->plotDimensions()->width()*xOff;
   long iyOff = yAxis_->plotDimensions()->height()*yOff;

   *dist = 1e30; // Initialise to an impossibly big distance

   if(type == PLOT_TRACE_LINES)
   {
      for(int i = 1; i < size; i++) // Search through each data set
      {
         float xd1 = (float)this->x[i-1];
         float yd1 = (float)this->yc[i-1].r;
         float xd2 = (float)this->x[i];
         float yd2 = (float)this->yc[i].r;

         float xs1 = (float)xAxis_->dataToScrn(xd1,ixOff); // Convert last point to screen coordinates
         float ys1 = (float)yAxis_->dataToScrn(yd1,iyOff);
         
         float xs2 = (float)xAxis_->dataToScrn(xd2,ixOff); // Convert this point to screen coordinates
         float ys2 = (float)yAxis_->dataToScrn(yd2,iyOff);
         
         float r2 = Plot1D::PointToLineDist((float)x,(float)y,xs1,ys1,xs2,ys2); // Find distance to cursor squared

         if(r2 >= 0 && r2 < *dist)
         {
            if((sqr(x-xs1)+sqr(y-ys1)) < (sqr(x-xs2)+sqr(y-ys2)))
               *mini = i-1;
            else
               *mini = i;
            *dist = r2;
            candidate = this;
         }
      } 		      
   }
   else if(type == PLOT_TRACE_STAIRS)
   {
      for(int i = 1; i < size; i++) // Search through each data set
      {
         float xd1 = (float)this->x[i-1];
         float yd1 = (float)this->yc[i-1].r;
         float xd2 = (float)this->x[i];
         float yd2 = (float)this->yc[i].r;

         float xs1 = (float)xAxis_->dataToScrn(xd1); // Convert last point to screen coordinates
         float ys1 = (float)yAxis_->dataToScrn(yd1);
         
         float xs2 = (float)xAxis_->dataToScrn(xd2); // Convert this point to screen coordinates
         float ys2 = (float)yAxis_->dataToScrn(yd2);
         
         float r1 = Plot1D::PointToLineDist((float)x,(float)y,xs1,ys1,xs2,ys1); // Find distance to horizontal line
         float r2 = Plot1D::PointToLineDist((float)x,(float)y,xs2,ys1,xs2,ys2); // Find distance to vertical line
         if(r1 >= 0 && r2 >= 0 && r1 < r2) r2 = r1;
         if(r1 >= 0 && r2 < 0) r2 = r1;
      	
         if(r2 >= 0 && r2 < *dist) // Is it a minimum
         {
            *mini = i;
            *dist = r2;
            candidate = this;
         }
      } 	      
   }
   else if(type == PLOT_TRACE_DOTS || type == PLOT_TRACE_NONE)
   {
      for(int i = 1; i < size; i++) // Search through each data set
      {
	      float xd1 = (float)this->x[i];
         float yd1 = (float)this->yc[i].r;

         float xs1 = (float)xAxis_->dataToScrn(xd1); // Convert point to screen coordinates
         float ys1 = (float)yAxis_->dataToScrn(yd1);
      
         float r2 = sqr(xs1-x) + sqr(ys1-y); // Find distance to cursor squared

         if(r2 < *dist)
         {
            *mini = i;
            *dist = r2;
            candidate = this;
         }
      } 	      
   }
   if(*dist <= MIN_DIST2) // Must be xx pixels or closer to line
      return(candidate);
   else
      return 0;
}



void TraceComplex::drawSegment(HDC hdc, POINT* point1, POINT* point2)
{	
   SaveDC(hdc);
	SetBkMode(hdc,TRANSPARENT);
	 
	HPEN rpen = makeRealPen(true);
	HPEN ipen = makeImagPen(true);
	COLORREF symbolColor = tracePar.getRealSymbolColor();
	HBRUSH symbolBrush = isPrinting() ? (HBRUSH)GetStockObject(BLACK_BRUSH) : CreateSolidBrush(symbolColor);
	HPEN bkgPen = CreatePen(PS_SOLID,0,parent->bkColor);
	HBRUSH bkgBrush = isPrinting() ? (HBRUSH)GetStockObject(WHITE_BRUSH) : parent->GetPlotBackgroundBrush();
	HPEN symbolPen = isPrinting() ? (HPEN)CreatePen(PS_SOLID,0,RGB_BLACK) : CreatePen(PS_SOLID,0,symbolColor);

   if((parent->display1DComplex & SHOW_REAL) && (parent->display1DComplex & SHOW_IMAGINARY))
	{
		SelectObject(hdc,rpen);
		MoveToEx(hdc, point1->x, point1->y - 5*y_scaling_factor(), 0);
		LineTo(hdc, point2->x, point2->y - 5*y_scaling_factor());
		SelectObject(hdc,ipen);
		MoveToEx(hdc, point1->x, point1->y + 5*y_scaling_factor(), 0);
		LineTo(hdc, point2->x, point2->y + 5*y_scaling_factor());
	}

   else if((parent->display1DComplex & SHOW_REAL) && !(parent->display1DComplex & SHOW_IMAGINARY))
	{
		SelectObject(hdc,rpen);
		MoveToEx(hdc, point1->x, point1->y, 0);
		LineTo(hdc, point2->x, point2->y);
	}

   else if(!(parent->display1DComplex & SHOW_REAL) && (parent->display1DComplex & SHOW_IMAGINARY))
	{
		SelectObject(hdc,ipen);
		MoveToEx(hdc, point1->x, point1->y, 0);
		LineTo(hdc, point2->x, point2->y);
	}
   
	// Draw a single symbol in the centre of the segment.
	if (tracePar.getSymbolType() != TD_NO_SYMBOL)
	{
		SelectObject(hdc,symbolPen);	   
		int szX = min(max(2,ceil((float)((5* (tracePar.getTraceWidth() + 1)) / 4))), 6);
		int szY = szX;
		parent->DrawSymbol(hdc,tracePar.getSymbolType(), symbolBrush, symbolPen, bkgBrush, 
			bkgPen, (point1->x + point2->x) / 2,(point1->y + point2->y) / 2,szX,szY);
	}

	DeleteObject(rpen);
	DeleteObject(ipen);
	DeleteObject(bkgBrush);
	DeleteObject(bkgPen);
	DeleteObject(symbolPen);
	DeleteObject(symbolBrush);
	RestoreDC(hdc, -1);
}

void TraceComplex::drawSegmentTick(HDC hdc, float scale, POINT* point1, POINT* point2)
{
   SaveDC(hdc);
	SetBkMode(hdc,TRANSPARENT);
	 
	HPEN rpen = makeRealPen(true);
	HPEN ipen = makeImagPen(true);

	if (parent->yAxisLeft() == this->yAxis())
	{
	   if((parent->display1DComplex & SHOW_REAL) && (parent->display1DComplex & SHOW_IMAGINARY))
	   {
			SelectObject(hdc,rpen);
			MoveToEx(hdc, point1->x, point1->y - 5*y_scaling_factor(), 0);
			LineTo(hdc, point1->x + scale*TRACE_SEGMENT_AXIS_INDICATOR_LENGTH, point1->y - 5*y_scaling_factor() - scale*TRACE_SEGMENT_AXIS_INDICATOR_LENGTH);
			SelectObject(hdc,ipen);
			MoveToEx(hdc, point1->x, point1->y + 5*y_scaling_factor(), 0);
			LineTo(hdc, point1->x + scale*TRACE_SEGMENT_AXIS_INDICATOR_LENGTH, point1->y + 5*y_scaling_factor() - scale*TRACE_SEGMENT_AXIS_INDICATOR_LENGTH);
		}
	   else if((parent->display1DComplex & SHOW_REAL) && !(parent->display1DComplex & SHOW_IMAGINARY))
	   {
			SelectObject(hdc,rpen);
			MoveToEx(hdc, point1->x, point1->y, 0);
			LineTo(hdc, point1->x + scale*TRACE_SEGMENT_AXIS_INDICATOR_LENGTH, point1->y - scale*TRACE_SEGMENT_AXIS_INDICATOR_LENGTH);
		}
		else if(!(parent->display1DComplex & SHOW_REAL) && (parent->display1DComplex & SHOW_IMAGINARY))
	   {
			SelectObject(hdc,ipen);
			MoveToEx(hdc, point1->x, point1->y, 0);
			LineTo(hdc, point1->x + scale*TRACE_SEGMENT_AXIS_INDICATOR_LENGTH, point1->y - scale*TRACE_SEGMENT_AXIS_INDICATOR_LENGTH);
		}
	}
	else // this plot is on the right axis
	{
	   if((parent->display1DComplex & SHOW_REAL) && !(parent->display1DComplex & SHOW_IMAGINARY))
	   {
			SelectObject(hdc,rpen);
			MoveToEx(hdc, point2->x, point2->y, 0);
			LineTo(hdc, point2->x - scale*TRACE_SEGMENT_AXIS_INDICATOR_LENGTH, point2->y - scale*TRACE_SEGMENT_AXIS_INDICATOR_LENGTH);
		}
	   else if((parent->display1DComplex & SHOW_IMAGINARY) && !(parent->display1DComplex & SHOW_REAL))
		{
			SelectObject(hdc,ipen);
			MoveToEx(hdc, point2->x, point2->y, 0);
			LineTo(hdc, point2->x - scale*TRACE_SEGMENT_AXIS_INDICATOR_LENGTH, point2->y  + scale*TRACE_SEGMENT_AXIS_INDICATOR_LENGTH);
		
		}
		else if ((parent->display1DComplex & SHOW_IMAGINARY) && (parent->display1DComplex & SHOW_REAL))
		{
			SelectObject(hdc, rpen);
			MoveToEx(hdc, point2->x, point2->y - 5 * y_scaling_factor(), 0);
			LineTo(hdc, point2->x - scale * TRACE_SEGMENT_AXIS_INDICATOR_LENGTH, point2->y - 5 * y_scaling_factor() - scale * TRACE_SEGMENT_AXIS_INDICATOR_LENGTH);
			SelectObject(hdc, ipen);
			MoveToEx(hdc, point2->x, point2->y + 5 * y_scaling_factor(), 0);
			LineTo(hdc, point2->x - scale * TRACE_SEGMENT_AXIS_INDICATOR_LENGTH, point2->y + 5 * y_scaling_factor() - scale * TRACE_SEGMENT_AXIS_INDICATOR_LENGTH);

		}
	}
	DeleteObject(rpen);
	DeleteObject(ipen);
	RestoreDC(hdc, -1);
}

void TraceComplex::scale_(double x, double y)
{
	tracePar.setTraceWidth(x * tracePar.getTraceWidth() + 0.5);
}
void TraceComplex::unscale_() 
{
	tracePar.setTraceWidth(x_inverse_scaling_factor() * tracePar.getTraceWidth() + 0.5);
}

void TraceReal::scale_(double x, double y)
{
	tracePar.setTraceWidth(x * tracePar.getTraceWidth() + 0.5);
}
void TraceReal::unscale_() 
{
	tracePar.setTraceWidth(x_inverse_scaling_factor() * tracePar.getTraceWidth() + 0.5);
}
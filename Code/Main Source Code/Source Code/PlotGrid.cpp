#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)
#include <gdiplus.h> 
#include "stdafx.h"
#include "PlotGrid.h"
#include "Axis.h"
#include "defines.h"
#include "globals.h"
#include "Plot.h"
#include "memoryLeak.h"

using namespace Gdiplus;

PlotGrid* PlotGrid::makePlotGrid(AxisOrientation ao,  bool drawGrid, bool drawFineGrid, COLORREF color, COLORREF fineColor)
{
	switch(ao)
	{
	case HORIZONTAL_AXIS:
		return new PlotGridHorizontal(0,drawGrid,drawFineGrid,color,fineColor);
	default:
		return new PlotGridVertical(0,drawGrid,drawFineGrid,color,fineColor);
	}
}

PlotGrid::PlotGrid(Axis* axis, bool drawGrid, bool drawFineGrid, COLORREF color, COLORREF fineColor)
{
	axis_ = axis;

	drawGrid_ = drawGrid;
	drawFineGrid_ = drawFineGrid;
	color_ = color;
	fineColor_ = fineColor;	
   gridLineWidth_ = 1.0;
	
	gridPen_     = CreatePen(PS_DOT,0,color);
	fineGridPen_ = CreatePen(PS_DOT,0,fineColor);
}

PlotGrid::PlotGrid(const PlotGrid& copyMe)
{
	axis_ = copyMe.axis_;
	drawGrid_ = copyMe.drawGrid_;
	drawFineGrid_ = copyMe.drawFineGrid_;
	color_ = copyMe.color_;
	fineColor_ = copyMe.fineColor_;	
   gridLineWidth_ = copyMe.gridLineWidth_;
	
	//gridPen_     = copyMe.gridPen_;
	//fineGridPen_ = copyMe.fineGridPen_;

	gridPen_ = CreatePen(PS_DOT, 0, color_);
	fineGridPen_ = CreatePen(PS_DOT, 0, fineColor_);
}

PlotGrid::~PlotGrid()
{
	deleteAllPens();
}

short PlotGrid::save(FILE* fp)
{
	if (!fp)
		return ERR;
	fwrite(&drawGrid_, sizeof(bool), 1, fp);
	fwrite(&drawFineGrid_, sizeof(bool), 1, fp);
	fwrite(&color_, sizeof(COLORREF), 1, fp);
	fwrite(&fineColor_, sizeof(COLORREF), 1, fp);
	return OK;
}


void PlotGrid::deleteAllPens()
{	
	DeleteObject(gridPen_);
	DeleteObject(fineGridPen_);
}

void PlotGrid::setPenColors(bool blackAndWhite)
{
	deleteAllPens();
	if (blackAndWhite)
	{
		gridPen_     = CreatePen(PS_DOT,gridLineWidth_,RGB(0,0,0));
		fineGridPen_ = CreatePen(PS_DOT,gridLineWidth_,RGB(0,0,0));
	}
	else
	{
		gridPen_     = CreatePen(PS_DOT,gridLineWidth_,color());
		fineGridPen_ = CreatePen(PS_DOT,gridLineWidth_,fineColor());
	}
}
	
void PlotGrid::drawFineLine(HDC hdc, int position)
{
	if (!drawFineGrid())
		return;
//	SelectObject(hdc,fineGridPen_);  
	drawLine(hdc, position, fineColor());
}

void PlotGrid::drawThickLine(HDC hdc, int position)
{
	if (!drawGrid())
		return;
//	SelectObject(hdc,gridPen_);   
	drawLine(hdc, position,color());
}

// This is messy having to set up the pen style each time should probably combine the grid code.
void PlotGridHorizontal::drawLine(HDC hdc, int position, COLORREF col )
{
   Graphics gfx(hdc); 
   Pen pen(Color(GetRValue(col), GetGValue(col), GetBValue(col))); 
   pen.SetWidth(gridLineWidth_/2.0+0.05);
   pen.SetDashStyle((Gdiplus::DashStyle)PS_DASH);
   pen.SetLineJoin(LineJoinBevel);

   Point p1,p2;
   p1.X = position;
   p1.Y = axis_->plotDimensions()->top();
   p2.X = position;
   p2.Y = axis_->plotDimensions()->top() + axis_->plotDimensions()->height();
   gfx.DrawLine(&pen,p1,p2);

  // MoveToEx(hdc,position,axis_->plotDimensions()->top(),0);
  // LineTo(hdc,position,axis_->plotDimensions()->top() +
		//axis_->plotDimensions()->height());
}

void PlotGridVertical::drawLine(HDC hdc, int position, COLORREF col)
{	
   Graphics gfx(hdc); 
   Pen pen(Color(GetRValue(col), GetGValue(col), GetBValue(col))); 
   pen.SetWidth(gridLineWidth_/2.0+0.05);
   pen.SetDashStyle((Gdiplus::DashStyle)PS_DASH);
   pen.SetLineJoin(LineJoinBevel);

   Point p1,p2;
   p1.X = axis_->plotDimensions()->left();
   p1.Y = position;
   p2.X = axis_->plotDimensions()->left() + axis_->plotDimensions()->width();
   p2.Y = position;
   gfx.DrawLine(&pen,p1,p2);

	//MoveToEx(hdc,axis_->plotDimensions()->left(),position,0);
	//LineTo(hdc,axis_->plotDimensions()->left() + 
	//	axis_->plotDimensions()->width(),position);
}

void PlotGrid::draw(HDC hdc)
{
	SetBkMode(hdc,TRANSPARENT);
   SaveDC(hdc);

	axis_->plot()->SetClippingRegion(hdc,0);
	// Get a list of the major line starting points from the axis.
	if (drawGrid_)
	{
		PositionList* thickStartPoints = axis_->getTickPositions(MAJOR);
		if (thickStartPoints)
		{
			for(int position: *thickStartPoints)
			{
				drawThickLine(hdc, position);
			}
			thickStartPoints->clear();
			delete thickStartPoints;
		}
	}

	// Get a list of the minor line starting points from the axis.
	if (drawFineGrid_)
	{
		PositionList* thinStartPoints = axis_->getTickPositions(MINOR);
		if (thinStartPoints)
		{
			for(int position: *thinStartPoints)
			{
				drawFineLine(hdc, position);
			}
			thinStartPoints->clear();
			delete thinStartPoints;
		}
	}
	RestoreDC(hdc,-1); 
}

PlotGridVertical::PlotGridVertical(Axis* axis, bool drawGrid, bool drawFineGrid, COLORREF color, COLORREF fineColor)
: PlotGrid(axis,drawGrid, drawFineGrid,color, fineColor)
{
}
	
PlotGridVertical::PlotGridVertical(const PlotGridVertical& copyMe)
: PlotGrid(copyMe)
{
}

PlotGridVertical::~PlotGridVertical()
{
}

PlotGrid* PlotGridVertical::clone()
{
	return new PlotGridVertical(*this);
}


PlotGridHorizontal::PlotGridHorizontal(Axis* axis, bool drawGrid, bool drawFineGrid, COLORREF color, COLORREF fineColor)
: PlotGrid(axis,drawGrid, drawFineGrid,color, fineColor)
{
}
	
PlotGridHorizontal::PlotGridHorizontal(const PlotGridHorizontal& copyMe)
: PlotGrid(copyMe)
{
}

PlotGridHorizontal::~PlotGridHorizontal()
{
}

PlotGrid* PlotGridHorizontal::clone()
{
	return new PlotGridHorizontal(*this);
}

void PlotGridHorizontal::scale_(double x, double y)
{
	gridLineWidth_ = x * gridLineWidth_ + 0.5;
}
void PlotGridHorizontal::unscale_() 
{
	gridLineWidth_ = gridLineWidth_ * x_inverse_scaling_factor() + 0.5;
}

void PlotGridVertical::scale_(double x, double y)
{
	gridLineWidth_ = x * gridLineWidth_ + 0.5;
}
void PlotGridVertical::unscale_() 
{
	gridLineWidth_ = gridLineWidth_ * x_inverse_scaling_factor() + 0.5;
}
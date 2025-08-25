#include "stdafx.h"
#include "PlotLabel.h"
#include "Axis.h"
#include "defineWindows.h"
#include "drawing.h"
#include "font.h"
#include <math.h>
#include "mymath.h"

#include "Plot.h"
#include "PlotWindow.h"
#include "plot_dialog_1d.h"
#include "ShortRect.h"
#include "memoryLeak.h"

PlotLabel* PlotLabel::makePlotLabel(PlotLabelType type, ProspaFont& font, char* text)
{
	switch(type)
	{
	case TITLE_PLOT_LABEL:
		return new TitleLabel(0,font,text);
	case VERTICAL_AXIS_PLOT_LABEL:
		return new VerticalAxisLabel(0,font,text);
	case HORIZONTAL_AXIS_PLOT_LABEL:
		return new HorizontalAxisLabel(0,font,text);
	default:
		return 0;
	}
}

PlotLabel::PlotLabel(ProspaFont& font,  char *text)
:rect_(), font_(font), Scalable()
{
	strncpy_s(text_,PLOT_STR_LEN,text,_TRUNCATE);
	textLocked_ = false;
}

PlotLabel::PlotLabel(const PlotLabel& copyMe)
: font_(copyMe.font_), Scalable(copyMe)
{
	rect_ = copyMe.rect_;
	strncpy_s(text_,PLOT_STR_LEN,copyMe.text_,_TRUNCATE);
	textLocked_ = copyMe.textLocked_;
}

PlotLabel::~PlotLabel()
{
}

void PlotLabel::clearText()
{
	setText("");
	textLocked_ = false;
}

short PlotLabel::save(FILE* fp)
{
	if (!fp)
		return ERR;	
	
	LOGFONT font = font_.font();
	COLORREF col = font_.color();
	fwrite(&text_, sizeof(char), PLOT_STR_LEN, fp);
	fwrite(&font, sizeof(LOGFONT), 1, fp);
	fwrite(&col, sizeof(COLORREF), 1, fp);
	return OK;
}

	
TitleLabel::TitleLabel(Plot* parent,  ProspaFont& font, char* text)
: PlotLabel(font, text)
	{
	plot_ = parent;
}

TitleLabel::~TitleLabel()
{
}

TitleLabel::TitleLabel(const TitleLabel& copyMe)
: PlotLabel(copyMe)
{
	plot_ = copyMe.plot_;
}

PlotLabel* TitleLabel::clone()
{
	return new TitleLabel(*this);
}	

void TitleLabel::draw(HDC hdc)
{
	if (!plot_->plotParent->showLabels)
		return;

   SaveDC(hdc);

	PlotDimensions& dim = plot_->getDimensions();

	short xoff = dim.left() + dim.width()/2;
	float yShift = plot_->curYAxis()->ticks().majorLength();

	short yoff = min( dim.top() - 10, dim.top() - yShift - 5 * y_scaling_factor());

   WriteText(hdc,xoff,yoff,WindowLayout::CENTRE_ALIGN,WindowLayout::BASE_ALIGN,
   WindowLayout::HORIZONTAL,font(),fontColor(),plot_->borderColor,text(),rect());

	RestoreDC(hdc,-1);
}


VerticalAxisLabel::VerticalAxisLabel(Axis* parent, ProspaFont& font,  char *text)
: PlotLabel(font, text)
{
	axis_ = parent;
}

VerticalAxisLabel::VerticalAxisLabel(const VerticalAxisLabel& copyMe)
: PlotLabel(copyMe)
{
	axis_ = copyMe.axis_;
}

PlotLabel* VerticalAxisLabel::clone()
{
	return new VerticalAxisLabel(*this);
}	

VerticalAxisLabel::~VerticalAxisLabel()
{
}


void VerticalAxisLabel::draw(HDC hdc)
{
	if (!axis_->plot()->plotParent->showLabels)
		return;
	if (!axis_->isDrawable())
		return;
   SaveDC(hdc);

	PlotDimensions& dim = axis_->plot()->getDimensions();

	float xShift = axis_->plot()->curXAxis()->ticks().majorLength();
	float yShift = fabs(axis_->ticks().majorLength());

	WindowLayout::Orientation textOrientation = axis_->plot()->yLabelVert ? WindowLayout::VERTICAL : WindowLayout::HORIZONTAL;
	WindowLayout::Constraint textBaseline = (axis_->side() == LEFT_VAXIS_SIDE) ? WindowLayout::RIGHT_ALIGN : WindowLayout::LEFT_ALIGN;

	short xoff = 0;
	if (axis_->plot()->axesMode == PLOT_AXES_CROSS || axis_->plot()->axesMode == PLOT_Y_AXIS_CROSS)
	{
		xoff = dim.left() - (axis_->plot()->yLabelVert ? 1.5 * yShift : xShift);
	}	
	else
	{
		if (axis_->side() == LEFT_VAXIS_SIDE)
			xoff = axis_->rect().getx0() - 4 * x_scaling_factor();
		if (axis_->side() == RIGHT_VAXIS_SIDE)
			xoff = axis_->rect().getx1() + 4 * x_scaling_factor();
	}

	short yoff = dim.top() + dim.height()/2; 

	WriteText(hdc,xoff,yoff,textBaseline,WindowLayout::CENTRE_ALIGN,
			textOrientation,font(),fontColor(),axis_->plot()->borderColor,text(),rect());

	RestoreDC(hdc,-1);
}


HorizontalAxisLabel::HorizontalAxisLabel(Axis* parent,ProspaFont& font,  char *text)
: PlotLabel(font, text)
{
	axis_ = parent;
}

HorizontalAxisLabel::HorizontalAxisLabel(const HorizontalAxisLabel& copyMe)
: PlotLabel(copyMe)
{
	axis_ = copyMe.axis_;
}

PlotLabel* HorizontalAxisLabel::clone()
{
	return new HorizontalAxisLabel(*this);
}	

HorizontalAxisLabel::~HorizontalAxisLabel()
{
}

void HorizontalAxisLabel::draw(HDC hdc)
{
	if (!axis_->plot()->plotParent->showLabels)
		return;
   SaveDC(hdc);

	PlotDimensions& dim = axis_->plot()->getDimensions();
	int axesMode = axis_->plot()->axesMode;
	float yShift = fabs(axis_->plot()->curYAxis()->ticks().majorLength());
   short xoff = dim.left() + dim.width()/2;
	short yoff;

   if(axesMode == PLOT_AXES_CROSS || axesMode == PLOT_X_AXIS_CROSS)
		yoff = dim.top() + dim.height() + yShift; 
   else
		yoff = axis_->rect().gety1(); //' + 5; 
      
   WriteText(hdc,xoff,yoff,WindowLayout::CENTRE_ALIGN,WindowLayout::TOP_ALIGN,
		WindowLayout::HORIZONTAL,font(),fontColor(),axis_->plot()->borderColor,text(),rect());
   RestoreDC(hdc,-1);
}
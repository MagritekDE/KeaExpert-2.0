#include "stdafx.h"
#include "Axis.h"
#include "PlotFile.h"
#include "defineWindows.h" // this should not be necessary here.
#include <float.h>
#include "font.h"

// Limits needs these, but windows.h has already used these names!
#undef max
#undef min
#include <limits>
#include <math.h>
#include "message.h" // this should not be necessary here.
#include "mymath.h"
#include "plot.h"
#include "PlotGrid.h"
#include "PlotGridFile.h"
#include "PlotLabel.h"
#include "PlotLabelFile.h"
#include "plotwindefaults.h"
#include "print.h"
#include "prospaResource.h"
#include "TicksFile.h"
#include "Trace.h"

#include <algorithm>
#include "memoryLeak.h"


using namespace std;
using std::find;

const float Axis::horiz_enlarge = 1.1; // 10% increase
const float Axis::vert_enlarge  = 1.1;
const float Axis::horiz_reduce = 0.9; // 10% decrease
const float Axis::vert_reduce  = 0.9;
const float Axis::horiz_shift = 0.01; // 1% shift
const float Axis::vert_shift = 0.01;

//////////////////////////////////////////////////////////////////////////////
// Factory for Axes. 
//////////////////////////////////////////////////////////////////////////////
Axis* Axis::makeAxis(AxisOrientation orientation, short mapping, Plot* parent, VerticalAxisSide side) 
{
	Axis* axis = 0;

	switch (orientation)
	{
	case HORIZONTAL_AXIS:
		axis = HorizontalAxis::createAxis(mapping, parent);
		break;
	case VERTICAL_AXIS:
		axis = VerticalAxis::createAxis(mapping, parent, side);
		break;
	}
	return axis;
}

//////////////////////////////////////////////////////////////////////////////
// Constructor for an Axis. This should only be called indirectly via makeAxis.
//////////////////////////////////////////////////////////////////////////////
Axis::Axis(Plot* parent, COLORREF color, float base, float length)
: dim_(parent->getDimensions()), ticks_(*pwd->ticks)
{
	plot_ = parent;
	this->base_ = base;
	this->length_ = length;
	this->dim_ = plot_->getDimensions();
	plot_->registerObserver(this);

	mapping_ = 0;
	origMapping_ = 0;
	autoRange_ = true;
	autoScale_ = true;		
	ppmScale_ = false;		
	min_ = FLT_MAX;
	max_ = -FLT_MAX;
	translator_ = 0;
	label_ = 0;
	grid_ = 0;
   this->setDirection(PLT_FORWARD); 
   lineWidth_ = 1;

	ticks_.setSpacing(10); // Spacing in pixels between ticks
	ticks_.setPerLabel(5); // Spacing in pixels between labels
}


//////////////////////////////////////////////////////////////////////////////
// Axis destructor
//////////////////////////////////////////////////////////////////////////////
Axis::~Axis()
{
	plot_->unregisterObserver(this);
	delete grid_;
	delete translator_;
	delete label_;
	this->traceList.clear();
	deleteAllPens();
}

//////////////////////////////////////////////////////////////////////////////
// Axis copy ctor
//////////////////////////////////////////////////////////////////////////////
Axis::Axis(const Axis& copyMe)
: ticks_(copyMe.ticks_)
{
	plot_ = 0;
	grid_ = copyMe.grid_->clone(); 
	if (grid_) grid_->setAxis(this);
	mapping_ = copyMe.mapping_;
	origMapping_ = copyMe.origMapping_;
	base_ = copyMe.base_;
	length_ = copyMe.length_;
	rect_ = copyMe.rect_;
	ticks_ = copyMe.ticks_;
	autoRange_ = copyMe.autoRange_;
	autoScale_ = copyMe.autoScale_;
	ppmScale_ = copyMe.ppmScale_;		
	translator_ = copyMe.translator_->clone();
	translator_->setAxis(this);
	min_ = copyMe.min_;
	max_ = copyMe.max_;
	minIndep_ = copyMe.minIndep_;
	maxIndep_ = copyMe.maxIndep_;
	minIndepOrig_ = copyMe.minIndepOrig_;
	oldMin_ = copyMe.oldMin_;
	oldMax_ = copyMe.oldMax_;
	plotDirection_ = copyMe.plotDirection_;
	label_ = copyMe.label_->clone();
   lineWidth_ = copyMe.lineWidth_;
   setDirection(copyMe.plotDirection_);
}

//////////////////////////////////////////////////////////////////////////////
// Reset this axis.
//////////////////////////////////////////////////////////////////////////////
void Axis::clear()
{
   if(!plot_->getOverRideAutoRange())
   {
	   min_ = FLT_MAX;
	   max_ = -FLT_MAX;
   }
	autoRange_ = true;
	//label_->userHasSetText(false);
	plot_->lockGrid(false);
}

//////////////////////////////////////////////////////////////////////////////
// Receive a notification from an observable -- in this case, it's known to 
// be a Plot telling this that its dimensions have changed.
//////////////////////////////////////////////////////////////////////////////
void Axis::notify(Observable* o)
{
	dim_ = static_cast<Plot*>(o)->getDimensions();
}

//////////////////////////////////////////////////////////////////////////////
// Determine new min_ and max_ based on the visible content of the traces
// associated with this Axis. ???Why not just call calculateDataRange???
//////////////////////////////////////////////////////////////////////////////
PlotDirection Axis::updateExtrema(float minX, float maxX)
{
	return this->calculateDataRange(minX, maxX);
}

//////////////////////////////////////////////////////////////////////////////
// Clean up all pens used by this Axis.
//////////////////////////////////////////////////////////////////////////////
void Axis::deleteAllPens()
{	
	DeleteObject(axesPen_);
}

//////////////////////////////////////////////////////////////////////////////
// Check globally whether this axis is being printed to a B&W printer.
// TODO: This should be elsewhere, and the globals shouldn't be global.
//////////////////////////////////////////////////////////////////////////////
bool Axis::isPrintingBlackAndWhite()
{
	return (gPlotMode == PRINT && gPrintMode == BLACK_AND_WHITE);
}

void Axis::setGrid(PlotGrid* grid)
{
	if (grid_) 
	{
		delete grid_; 
	}
	grid_ = grid;
}

void Axis::setLabel(PlotLabel* label) 
{
	if (label_) 
	{
		delete label_; 
	}
	label_ = label;
}


//////////////////////////////////////////////////////////////////////////////
// Change the colors of all pens of this Axis.
//////////////////////////////////////////////////////////////////////////////
void Axis::setPenColors()
{
	deleteAllPens();
	if (this->isPrintingBlackAndWhite())
	{
		grid_->setPenColors(true);
	   axesPen_     = CreatePen(PS_SOLID,lineWidth_,RGB(0,0,0));		
	}
	else
	{
		grid_->setPenColors();
	   axesPen_ = CreatePen(PS_SOLID,lineWidth_,plot()->axesColor);
	}
}


//////////////////////////////////////////////////////////////////////////////
// Change the color of the labels on this axis (1,2,3...)
//////////////////////////////////////////////////////////////////////////////
void Axis::setTextColor(HDC hdc)
{
	if (this->isPrintingBlackAndWhite())
	{
		SetTextColor(hdc, RGB(0,0,0));
	}
	else
	{
		SetTextColor(hdc, ticks().fontColor());
	}
}

//////////////////////////////////////////////////////////////////////////////
// Draw this Axis to the screen.
//////////////////////////////////////////////////////////////////////////////
void Axis::draw(HDC hdc)
{
	if (mapping_ == PLOT_LINEAR_X || mapping_ == PLOT_LINEAR_Y)
		drawLinear(hdc);
	else
		drawLog(hdc);
}


//////////////////////////////////////////////////////////////////////////////
// Draw this Axis to the screen.
//////////////////////////////////////////////////////////////////////////////
void Axis::drawIndependent(HDC hdc)
{
	if (mapping_ == PLOT_LINEAR_X || mapping_ == PLOT_LINEAR_Y)
		drawIndependentLinear(hdc);
	else
		drawIndependentLog(hdc);
}
//////////////////////////////////////////////////////////////////////////////
// Draw this Axis to the screen.
//////////////////////////////////////////////////////////////////////////////
PositionList* Axis::getTickPositions(TickType tickType)
{
	if (mapping_ == PLOT_LINEAR_X || mapping_ == PLOT_LINEAR_Y)
	{
		return getLinearTickPositions(tickType);
	}
	else
	{
		return getLogTickPositions(tickType);
	}
}

//////////////////////////////////////////////////////////////////////////////
// Check whether this Axis knows any reasons not to draw it.
//////////////////////////////////////////////////////////////////////////////
bool Axis::isDrawable()
{
	return hasData();
}

//////////////////////////////////////////////////////////////////////////////
// Check whether this Axis has any associated Traces.
//////////////////////////////////////////////////////////////////////////////
bool Axis::hasData()
{
	// An axis has data if it is 1D an associated with a trace,
	return !traceList.empty() || 
	
	// or if it is 2D and is the left hand axis.
		((plot_->getDimension() == 2) && (side() == LEFT_VAXIS_SIDE));  
}

//////////////////////////////////////////////////////////////////////////////
// Makes the supplied Trace t the responsibility of this Axis.
// From now on, this Axis will consider t when figuring out its extrema and
// labels.
//////////////////////////////////////////////////////////////////////////////
void Axis::addTrace(Trace* t)
{
	if (traceList.end() != find(traceList.begin(), traceList.end(), t))
		return;
	traceList.push_back(t);
   if(!plot_->getOverRideAutoRange())
	   updateExtrema(plot_->curXAxis()->Min(), plot_->curXAxis()->Max()); //???These arguments are ignored???
}

//////////////////////////////////////////////////////////////////////////////
// Removes the supplied Trace t from the responsibility of this Axis. This 
// Axis will no longer consider t when figuring out its extrema and labels.
//////////////////////////////////////////////////////////////////////////////
void Axis::removeTrace(Trace* t)
{
	TraceListIterator it = find(traceList.begin(),traceList.end(), t);
	if (it != traceList.end())
		traceList.erase(it);
	if (traceList.empty())
	{
		clear();
	}
}

//////////////////////////////////////////////////////////////////////////////
// This function removes the supplied Trace t from the responsibility of this 
// Axis. This Axis will no longer consider t when figuring out its extrema and
// labels.
//////////////////////////////////////////////////////////////////////////////
void Axis::setParent(Plot* parent) 
{
	if (plot_) 
	{
		plot_->unregisterObserver(this); 
	}
	plot_ = parent;
	dim_ = parent->getDimensions();
	plot_->registerObserver(this);
}


//////////////////////////////////////////////////////////////////////////////
// Set this Axis to display using linear or log mapping.
//////////////////////////////////////////////////////////////////////////////
void Axis::setMapping(short mapping) 
{
	mapping_ = mapping;

	Translator* t = Translator::makeTranslator(plot()->getDimension(), mapping);
	this->setTranslator(t);
	t->setAxis(this);
}

//////////////////////////////////////////////////////////////////////////////
// Set this Axis' translator. Translators translate between data points and
// screen coordinates.
//////////////////////////////////////////////////////////////////////////////
void Axis::setTranslator(Translator* t) 
{
	if (translator_) 
	{
		delete translator_; 
	}
	translator_ = t;
}

bool Axis::canDisplay(Trace* t)
{
	// Both linear and log axes can display traces whose y values are all
	//  greater than zero.
	if (t->greaterThanZero('y'))
		return true;
	// If there is a y[i] <= 0 and this is a log mapping, the trace can't be displayed here.
	if (PLOT_LOG_Y == mapping_)
		return false;
	// Otherwise, this is a linear axis, which can display such a trace...
	return true;
}

short Axis::save(FILE* fp, int version)
{
	if (!fp)
		return ERR;

	VerticalAxisSide lside = side();
	fwrite(&lside, sizeof(VerticalAxisSide), 1, fp);
	fwrite(&min_, sizeof(float), 1, fp);
	fwrite(&max_, sizeof(float), 1, fp);

   if(version >= 360)
   {
	   fwrite(&minIndep_, sizeof(float), 1, fp);
	   fwrite(&maxIndep_, sizeof(float), 1, fp);
   }
	fwrite(&mapping_, sizeof(short), 1, fp);
	fwrite(&autoRange_, sizeof(bool), 1, fp);
   if(version >= 370)
   {
    	fwrite(&ppmScale_, sizeof(bool), 1, fp);
   }
   PlotDirection direction = this->plotDirection();
	fwrite(&direction, sizeof(PlotDirection), 1, fp);

	if (ERR == this->ticks_.save(fp))
		return ERR;
	if (ERR == this->grid_->save(fp))
		return ERR;
	if (ERR == this->label_->save(fp))
		return ERR;
	return OK;
}

void Axis::scale_(double x, double y)
{
	rect().scale(x, y);
	dim_.scale(x, y);
	label().scale(x, y);
	ticks().scale(x, y);
   grid()->scale(x, y);
   lineWidth_ = x;
}

void Axis::unscale_()
{
	rect().unscale();
	dim_.unscale();
	label().unscale();
	ticks().unscale();
   grid()->unscale();
   lineWidth_ = 1;
}

/*****************************************************************************************
*                     Figure out the best spacing between axis labels                    *
*****************************************************************************************/  
float Axis::getLabelSpacing(double width)
{
   double labelSpacing = fabs(width/5.0);
   double temp = log10((double)labelSpacing);
   if(temp < 0) temp = temp-1;
   long n = (long)(labelSpacing*pow(10.0,(double)(-(long)temp))+0.5);
   long diff = abs(n-10);
	long cl = 10;
   if(abs(n-1) < diff) diff = abs(n-1), cl = 1;
   if(abs(n-2) < diff) diff = abs(n-2), cl = 2;
   if(abs(n-5) < diff) cl = 5;
   labelSpacing = cl*pow(10.0,(double)(long)temp);
   if(labelSpacing > MAX_FLOAT)
      labelSpacing = MAX_FLOAT;
   return(labelSpacing); 
}

void Axis::undoEnlargement()
{
   setMin(oldMin_);
   setMax(oldMax_); 
}

void Axis::resetTextCoords()
{
	label_->rect().reset();
	rect_.reset();
}

//////////////////
// HORIZONTAL AXIS

HorizontalAxis::HorizontalAxis(Plot* parent, COLORREF color, float base, float length)
: Axis(parent, color, base, length)
{
	setMapping(PLOT_LINEAR_X);
}

HorizontalAxis::HorizontalAxis(const HorizontalAxis& copyMe)
: Axis(copyMe)
{
	static_cast<HorizontalAxisLabel*>(label_)->setParent(this);
}

HorizontalAxis* HorizontalAxis::clone() const
{
	return new HorizontalAxis(*this);
}

Axis* HorizontalAxis::createAxis(short mapping, Plot* parent)
{
	Translator* t = Translator::makeTranslator(parent->getDimension(), mapping);
	Axis* axis = new HorizontalAxis(parent, pwd->gridColor, 0,0);
   axis->setMapping(mapping);
	t->setAxis(axis);
	axis->setTranslator(t);
	ProspaFont font(pwd->labelFontName, pwd->labelFontColor, pwd->labelFontSize, pwd->labelFontStyle);
	HorizontalAxisLabel* l = new HorizontalAxisLabel(axis, font, "\0");
	axis->setLabel(l);

	PlotGrid* grid = new PlotGridHorizontal(axis, false, false, pwd->gridColor, pwd->fineGridColor);
	axis->setGrid(grid);
	return axis;
}

HorizontalAxis::~HorizontalAxis()
{
}

void HorizontalAxis::clear()
{
	Axis::clear();
	setMapping(PLOT_LINEAR_X);
}

void HorizontalAxis::initialiseRange()
{
	setBase(0);
	setLength(dynamic_cast<Plot2D*>(plot())->matWidth());
}

void HorizontalAxis::drawBottomTick(HDC hdc, int xScrn, float length)
{	
	if(plot()->axesMode != PLOT_AXES_CROSS && plot()->axesMode != PLOT_X_AXIS_CROSS)
	{ 
		SelectObject(hdc,axesPen_);   
		MoveToEx(hdc, xScrn, dim_.top() + dim_.height() + 1,0);
		LineTo(hdc, xScrn, nint(dim_.top() + dim_.height() + length + 1));
	}
}

void HorizontalAxis::drawTopTick(HDC hdc, int xScrn, float length)
{
	if(plot()->axesMode == PLOT_AXES_BOX ||  plot()->axesMode == PLOT_X_AXIS_BOX)
	{   
		SelectObject(hdc,axesPen_);   
		MoveToEx(hdc, xScrn, dim_.top() - 1, 0);
		LineTo(hdc, xScrn, nint(dim_.top() - length - 1));
	}
}

void HorizontalAxis::drawCross(HDC hdc, int xScrn, int yScrn, float length)
{
	if((plot()->axesMode == PLOT_AXES_CROSS || plot()->axesMode == PLOT_X_AXIS_CROSS) && yScrn > dim_.top() && yScrn < dim_.top()+dim_.height())
   {			   
		SelectObject(hdc,axesPen_);  
		MoveToEx(hdc, xScrn, nint(yScrn - length/2.0), 0);
		LineTo(hdc, xScrn, nint(yScrn + length/2.0)); 
   }	
}

void HorizontalAxis::drawLinear(HDC hdc)
{   
   bool reverse = plot()->identifyReverseHorizontal(min_, max_, length(), base());  
   SaveDC(hdc);

// Figure out best label and tick spacing
	float xTick = ticks().spacing();
	short ticksPerLabel = ticks().perLabel();
   float xLabel = ticksPerLabel*xTick;

   if(xLabel <= 0 || xTick <= 0 || xTick > xLabel)
      return;
        
// Calculate leftmost tick and label positions in data points
   float xTickStart = (int)(min_/xTick)*xTick;
   if (xTickStart < min_) xTickStart += xTick;
  
   float xLabelStart = (int)(min_/xLabel)*xLabel;
   if (xLabelStart < min_) xLabelStart += xLabel;

// Correct for rounding problems - tickstart should always be less then labelstart
   if(xTickStart > xLabelStart)
     xTickStart = xLabelStart;

// Count number of ticks "used" so far
   int tickCnt = ticksPerLabel-(int)((xLabelStart-xTickStart)/xTick+0.5); 

// Draw ticks and labels 
	Translator* peerTranslator = this->plot()->curYAxis()->translator();
   float yScrn = peerTranslator->dataToScrn(0);
    
// Work out number of ticks to plot
   int nrTicks = nint((max_-xTickStart)/xTick);

// Check for too many ticks!
   if(nrTicks > 5000)
      return;

// Set up pens, text color, and background behaviour
	setPenColors();
	SelectObject(hdc, axesPen_);
	setTextColor(hdc);
	SetBkMode(hdc,TRANSPARENT);

// Select font for labels 
	HFONT labelFont = GetFont(hdc, this->ticks().font(),0,0); 
   SelectObject(hdc,labelFont);

// Draw the axis
   for(int i = 0; i <= nrTicks; i++)
   {
      float x = xTickStart + i*xTick;

   // Convert from axes to screen coordinates
		float xScrn;
      if(plot()->getDimension() == 1)
         xScrn = dataToScrn(x);
      else
      {
         if(this->plotDirection() == PLT_REVERSE)
            xScrn = nint(dim_.right() - (x-min_)*dim_.width()/(float)(max_-min_));
         else
            xScrn = nint(dim_.left() + (x-min_)*dim_.width()/(float)(max_-min_));
      }

      if(xScrn > dim_.left()+dim_.width() || 
			xScrn < dim_.left()) // Don't plot outside limits
         continue;

      if(tickCnt == ticksPerLabel) // Draw x label and long tick
      {
         float xl;
			drawBottomTick(hdc, xScrn, ticks().majorLength());
			drawTopTick(hdc, xScrn, ticks().majorLength());
			drawCross(hdc, xScrn, yScrn, ticks().majorLength());
	   	      
         xl = nint(x/xLabel)*xLabel;

			char str[MAX_STR];
         sprintf(str,"%g",xl);
			SIZE te; 
         GetTextExtentPoint32(hdc, str, strlen(str), &te);

			short xt,yt;
		   if(plot()->axesMode == PLOT_AXES_CROSS || plot()->axesMode == PLOT_X_AXIS_CROSS)
		   {
		      xt = xScrn-te.cx/2;
				yt = yScrn+fabs(ticks().majorLength());
            TextOut(hdc,xt,yt,str,strlen(str)); 
         } 
		   else
		   {
		      xt = xScrn-te.cx/2;
				yt = dim_.top()+dim_.height()+fabs(ticks().majorLength());
            TextOut(hdc,xt,yt,str,strlen(str)); 
         } 

         rect().SetAxisRect(xt,yt,xt+te.cx,yt+te.cy);
         
         tickCnt = 0;
      }
      else // Draw minor ticks
      {
			drawBottomTick(hdc, xScrn, ticks().minorLength());
			drawTopTick(hdc, xScrn, ticks().minorLength());
			drawCross(hdc, xScrn, yScrn, ticks().minorLength());
		}
      tickCnt++;      
   }

// Tidy up  
   RestoreDC(hdc, -1);
   DeleteObject(labelFont); 
}

void HorizontalAxis::drawLog(HDC hdc)
{
   SaveDC(hdc);
   bool reverse;

   short dim = this->plot()->getDimension();

   if(dim == 1)
   {
      reverse = plot()->identifyReverseHorizontal(min_, max_, length(), base());
   }
   else
   {
      float a,b;
      reverse = plot()->identifyReverseHorizontal(a,b, length(), base());
      min_ = this->dataToUser(min_);
      max_ = this->dataToUser(max_);
   }

// Define pens

	setPenColors();
	SelectObject(hdc, axesPen_);
	setTextColor(hdc);
	SetBkMode(hdc,TRANSPARENT);
	    
// Determine the range limits

   long minPower = floor((log10(min_)));
   long maxPower = floor((log10(max_))+1);

// Determine the distance between adjacent power labels (i.e. 10, 100, 1000 ...)

	PlotDimensions& dimensions = plot()->getDimensions();

	float distbtwnlabels = (float)dimensions.width()/(log10(max_)-log10(min_));

// Select font for labels
 
   HFONT labelFont = GetFont(hdc, ticks().font(),0,0);
   HFONT exponentFont = GetFont(hdc, ticks().font(),-4*plot()->y_scaling_factor(),0); 
      
// Get screen position for y = 0 
	Translator* peerTranslator = this->plot()->curYAxis()->translator();
   float yScrn = peerTranslator->dataToScrn(0);
    

// Loop over ticks and labels

   float xa;
   if(reverse)
      xa = dim_.left() + dim_.width() + (log10(min_)-minPower)*distbtwnlabels; // Initial tick location
   else
      xa = dim_.left() -  (log10(min_)-minPower)*distbtwnlabels; // Initial tick location
   
   for(short power = minPower; power <= maxPower; power++)
	{
		 float xl;
		 if(reverse)
          xl = xa - (power-minPower)*distbtwnlabels; // Label position
       else
          xl = xa + (power-minPower)*distbtwnlabels; // Label position
       
       for(short tick = 1; tick <= 9; tick++)
       {
			 float x;
          if(reverse)
            x = xl - log10((float)tick)*distbtwnlabels;
          else
            x = xl + log10((float)tick)*distbtwnlabels;

          if(x < dimensions.left() || x > (dimensions.left()+dimensions.width())) // Ignore ticks and labels outside plot region
             continue;
             
          if(tick == 1) // Draw x label and long tick since start of decade
          {
				char str[MAX_STR];
				drawBottomTick(hdc, nint(x), ticks().majorLength());
				drawTopTick(hdc, nint(x), ticks().majorLength());
				drawCross(hdc, nint(x), nint(yScrn), ticks().majorLength());

            SelectObject(hdc,labelFont); // Write "10"
            strncpy(str,"10",MAX_STR);
				SIZE te1;
            GetTextExtentPoint32(hdc, str, strlen(str), &te1);
            short xt = nint(x-te1.cx/2.0) ;
            short yt;
            if(plot()->axesMode == PLOT_AXES_CROSS || plot()->axesMode == PLOT_X_AXIS_CROSS)
				   yt = yScrn+fabs(ticks().majorLength());
            else
               yt = nint(dimensions.top()+dimensions.height()+fabs(ticks().majorLength()));

            TextOut(hdc,xt,yt,str,strlen(str));  
            rect().SetAxisRect(xt,yt,xt+te1.cx,yt+te1.cy);

            SelectObject(hdc,exponentFont);  // Write exponent to 10           
            sprintf(str,"%hd",power);
				SIZE te2; 
            GetTextExtentPoint32(hdc, str, strlen(str), &te2);
            xt = nint(x+te1.cx/2.0);

            if(plot()->axesMode == PLOT_AXES_CROSS || plot()->axesMode == PLOT_X_AXIS_CROSS)
				   yt = yScrn+fabs(ticks().majorLength()-te2.cy/3.0);
            else
               yt = nint(dimensions.top()+dimensions.height()+fabs(ticks().majorLength())-te2.cy/3.0);

            TextOut(hdc,xt,yt,str,strlen(str));  
            if(power == maxPower) break; // Finished
         }
         else // Draw minor ticks & labels
         {
				char str[MAX_STR];
				drawBottomTick(hdc, nint(x), ticks().minorLength());
				drawTopTick(hdc, nint(x), ticks().minorLength());
			   drawCross(hdc, nint(x), nint(yScrn), ticks().minorLength());

		      if(maxPower-minPower <= 2) // If only 1 main label then label minor ticks with mantissa
		      {
               SelectObject(hdc,labelFont); // Write "10" (dummy write to get font size)
               strncpy(str,"10",MAX_STR);
					SIZE te1;
               GetTextExtentPoint32(hdc, str, strlen(str), &te1);

               SelectObject(hdc,exponentFont);  // Write mantissa (2,3,4 ... 9)           
               sprintf(str,"%hd",tick);
					SIZE te2; 
               GetTextExtentPoint32(hdc, str, strlen(str), &te2);
               short xt = nint(x-te2.cx/2.0);
               short yt;
               if(plot()->axesMode == PLOT_AXES_CROSS || plot()->axesMode == PLOT_X_AXIS_CROSS)
                  yt = nint(yScrn+te1.cy-te2.cy/2);
               else
                  yt = nint(dimensions.top()+dimensions.height()+te1.cy-te2.cy/2);

               TextOut(hdc,xt,yt,str,strlen(str));
	            rect().SetAxisRect(xt,yt,xt+te1.cx,yt+te1.cy);
            }
            else if(maxPower-minPower == 1) // If no main labels present then need to have multiplier
            {
	            SelectObject(hdc,labelFont); // Write "x10"
	            strncpy(str,"x10",MAX_STR);
					SIZE te1;
	            GetTextExtentPoint32(hdc, str, strlen(str), &te1);
	            TextOut(hdc,dimensions.left()+dimensions.width()+te1.cx,nint(dimensions.top()+dimensions.height()+te1.cy/2.0),str,strlen(str));  
					SIZE te2; 
	            SelectObject(hdc,exponentFont);  // Write exponent to 10           
	            sprintf(str,"%hd",minPower);
	            GetTextExtentPoint32(hdc, str, strlen(str), &te2);
	            short xt = nint(dimensions.left()+dimensions.width()+2*te1.cx);
               short yt;
               if(plot()->axesMode == PLOT_AXES_CROSS || plot()->axesMode == PLOT_X_AXIS_CROSS)
                  yt = nint(yScrn+te1.cy/2.0-te2.cy/3.0);
               else
	               yt = nint(dimensions.top()+dimensions.height()+te1.cy/2.0-te2.cy/3.0);
	            TextOut(hdc,xt,yt,str,strlen(str));	
	            rect().SetAxisRect(xt,yt,xt+te1.cx,yt+te1.cy);
	         }
		   }
		}
   }
   RestoreDC(hdc, -1);
   DeleteObject(exponentFont); 
   DeleteObject(labelFont); 
}


void HorizontalAxis::drawIndependentLinear(HDC hdc)
{   
   bool reverse = plot()->identifyReverseHorizontal(min_, max_, length(), base());  
   SaveDC(hdc);

// Figure out best label and tick spacing
	float xTick = ticks().spacing();
	short ticksPerLabel = ticks().perLabel();
   float xLabel = ticksPerLabel*xTick;

   if(xLabel <= 0 || xTick <= 0 || xTick > xLabel)
      return;
        
// Calculate leftmost tick and label positions in data points
   float xTickStart = (int)(min_/xTick)*xTick;
   if (xTickStart < min_) xTickStart += xTick;
  
   float xLabelStart = (int)(min_/xLabel)*xLabel;
   if (xLabelStart < min_) xLabelStart += xLabel;

// Correct for rounding problems - tickstart should always be less then labelstart
   if(xTickStart > xLabelStart)
     xTickStart = xLabelStart;

// Count number of ticks "used" so far
   int tickCnt = ticksPerLabel-(int)((xLabelStart-xTickStart)/xTick+0.5); 

// Draw ticks and labels 
	Translator* peerTranslator = this->plot()->curYAxis()->translator();
   float yScrn = peerTranslator->dataToScrn(0);
    
// Work out number of ticks to plot
   int nrTicks = nint((max_-xTickStart)/xTick);

// Check for too many ticks!
   if(nrTicks > 5000)
      return;

// Set up pens, text color, and background behaviour
	setPenColors();
	SelectObject(hdc, axesPen_);
	setTextColor(hdc);
	SetBkMode(hdc,TRANSPARENT);

// Select font for labels 
	HFONT labelFont = GetFont(hdc, this->ticks().font(),0,0); 
   SelectObject(hdc,labelFont);

// Draw the axis
   for(int i = 0; i <= nrTicks; i++)
   {
      float x = xTickStart + i*xTick;

   // Convert from axes to screen coordinates
		float xScrn;
      if(plot()->getDimension() == 1)
         xScrn = dataToScrn(x);
      else
      {
         if(this->plotDirection() == PLT_REVERSE)
            xScrn = nint(dim_.right() - (x-min_)*dim_.width()/(float)(max_-min_));
         else
            xScrn = nint(dim_.left() + (x-min_)*dim_.width()/(float)(max_-min_));
      }

      if(xScrn > dim_.left()+dim_.width() || 
			xScrn < dim_.left()) // Don't plot outside limits
         continue;

      if(tickCnt == ticksPerLabel) // Draw x label and long tick
      {
         float xl;
			drawBottomTick(hdc, xScrn, ticks().majorLength());
			drawTopTick(hdc, xScrn, ticks().majorLength());
			drawCross(hdc, xScrn, yScrn, ticks().majorLength());
	   	      
         xl = nint(x/xLabel)*xLabel;

			char str[MAX_STR];
         sprintf(str,"%g",xl);
			SIZE te; 
         GetTextExtentPoint32(hdc, str, strlen(str), &te);

			short xt,yt;
		   if(plot()->axesMode == PLOT_AXES_CROSS || plot()->axesMode == PLOT_X_AXIS_CROSS)
		   {
		      xt = xScrn-te.cx/2;
				yt = yScrn+fabs(ticks().majorLength());
            TextOut(hdc,xt,yt,str,strlen(str)); 
         } 
		   else
		   {
		      xt = xScrn-te.cx/2;
				yt = dim_.top()+dim_.height()+fabs(ticks().majorLength());
            TextOut(hdc,xt,yt,str,strlen(str)); 
         } 

         rect().SetAxisRect(xt,yt,xt+te.cx,yt+te.cy);
         
         tickCnt = 0;
      }
      else // Draw minor ticks
      {
			drawBottomTick(hdc, xScrn, ticks().minorLength());
			drawTopTick(hdc, xScrn, ticks().minorLength());
			drawCross(hdc, xScrn, yScrn, ticks().minorLength());
		}
      tickCnt++;      
   }

// Tidy up  
   RestoreDC(hdc, -1);
   DeleteObject(labelFont); 
}

void HorizontalAxis::drawIndependentLog(HDC hdc)
{
   SaveDC(hdc);
   bool reverse;

   short dim = this->plot()->getDimension();

   if(dim == 1)
   {
      reverse = plot()->identifyReverseHorizontal(min_, max_, length(), base());
   }
   else
   {
      float a,b;
      reverse = plot()->identifyReverseHorizontal(a,b, length(), base());
      min_ = this->dataToUser(min_);
      max_ = this->dataToUser(max_);
   }

// Define pens

	setPenColors();
	SelectObject(hdc, axesPen_);
	setTextColor(hdc);
	SetBkMode(hdc,TRANSPARENT);
	    
// Determine the range limits

   long minPower = floor((log10(min_)));
   long maxPower = floor((log10(max_))+1);

// Determine the distance between adjacent power labels (i.e. 10, 100, 1000 ...)

	PlotDimensions& dimensions = plot()->getDimensions();

	float distbtwnlabels = (float)dimensions.width()/(log10(max_)-log10(min_));

// Select font for labels
 
   HFONT labelFont = GetFont(hdc, ticks().font(),0,0);
   HFONT exponentFont = GetFont(hdc, ticks().font(),-4*plot()->y_scaling_factor(),0); 
      
// Get screen position for y = 0 
	Translator* peerTranslator = this->plot()->curYAxis()->translator();
   float yScrn = peerTranslator->dataToScrn(0);
    

// Loop over ticks and labels

   float xa;
   if(reverse)
      xa = dim_.left() + dim_.width() + (log10(min_)-minPower)*distbtwnlabels; // Initial tick location
   else
      xa = dim_.left() -  (log10(min_)-minPower)*distbtwnlabels; // Initial tick location
   
   for(short power = minPower; power <= maxPower; power++)
	{
		 float xl;
		 if(reverse)
          xl = xa - (power-minPower)*distbtwnlabels; // Label position
       else
          xl = xa + (power-minPower)*distbtwnlabels; // Label position
       
       for(short tick = 1; tick <= 9; tick++)
       {
			 float x;
          if(reverse)
            x = xl - log10((float)tick)*distbtwnlabels;
          else
            x = xl + log10((float)tick)*distbtwnlabels;

          if(x < dimensions.left() || x > (dimensions.left()+dimensions.width())) // Ignore ticks and labels outside plot region
             continue;
             
          if(tick == 1) // Draw x label and long tick since start of decade
          {
				char str[MAX_STR];
				drawBottomTick(hdc, nint(x), ticks().majorLength());
				drawTopTick(hdc, nint(x), ticks().majorLength());
				drawCross(hdc, nint(x), nint(yScrn), ticks().majorLength());

            SelectObject(hdc,labelFont); // Write "10"
            strncpy(str,"10",MAX_STR);
				SIZE te1;
            GetTextExtentPoint32(hdc, str, strlen(str), &te1);
            short xt = nint(x-te1.cx/2.0) ;
            short yt;
            if(plot()->axesMode == PLOT_AXES_CROSS || plot()->axesMode == PLOT_X_AXIS_CROSS)
				   yt = yScrn+fabs(ticks().majorLength());
            else
               yt = nint(dimensions.top()+dimensions.height()+fabs(ticks().majorLength()));

            TextOut(hdc,xt,yt,str,strlen(str));  
            rect().SetAxisRect(xt,yt,xt+te1.cx,yt+te1.cy);

            SelectObject(hdc,exponentFont);  // Write exponent to 10           
            sprintf(str,"%hd",power);
				SIZE te2; 
            GetTextExtentPoint32(hdc, str, strlen(str), &te2);
            xt = nint(x+te1.cx/2.0);

            if(plot()->axesMode == PLOT_AXES_CROSS || plot()->axesMode == PLOT_X_AXIS_CROSS)
				   yt = yScrn+fabs(ticks().majorLength()-te2.cy/3.0);
            else
               yt = nint(dimensions.top()+dimensions.height()+fabs(ticks().majorLength())-te2.cy/3.0);

            TextOut(hdc,xt,yt,str,strlen(str));  
            if(power == maxPower) break; // Finished
         }
         else // Draw minor ticks & labels
         {
				char str[MAX_STR];
				drawBottomTick(hdc, nint(x), ticks().minorLength());
				drawTopTick(hdc, nint(x), ticks().minorLength());
			   drawCross(hdc, nint(x), nint(yScrn), ticks().minorLength());

		      if(maxPower-minPower <= 2) // If only 1 main label then label minor ticks with mantissa
		      {
               SelectObject(hdc,labelFont); // Write "10" (dummy write to get font size)
               strncpy(str,"10",MAX_STR);
					SIZE te1;
               GetTextExtentPoint32(hdc, str, strlen(str), &te1);

               SelectObject(hdc,exponentFont);  // Write mantissa (2,3,4 ... 9)           
               sprintf(str,"%hd",tick);
					SIZE te2; 
               GetTextExtentPoint32(hdc, str, strlen(str), &te2);
               short xt = nint(x-te2.cx/2.0);
               short yt;
               if(plot()->axesMode == PLOT_AXES_CROSS || plot()->axesMode == PLOT_X_AXIS_CROSS)
                  yt = nint(yScrn+te1.cy-te2.cy/2);
               else
                  yt = nint(dimensions.top()+dimensions.height()+te1.cy-te2.cy/2);

               TextOut(hdc,xt,yt,str,strlen(str));
	            rect().SetAxisRect(xt,yt,xt+te1.cx,yt+te1.cy);
            }
            else if(maxPower-minPower == 1) // If no main labels present then need to have multiplier
            {
	            SelectObject(hdc,labelFont); // Write "x10"
	            strncpy(str,"x10",MAX_STR);
					SIZE te1;
	            GetTextExtentPoint32(hdc, str, strlen(str), &te1);
	            TextOut(hdc,dimensions.left()+dimensions.width()+te1.cx,nint(dimensions.top()+dimensions.height()+te1.cy/2.0),str,strlen(str));  
					SIZE te2; 
	            SelectObject(hdc,exponentFont);  // Write exponent to 10           
	            sprintf(str,"%hd",minPower);
	            GetTextExtentPoint32(hdc, str, strlen(str), &te2);
	            short xt = nint(dimensions.left()+dimensions.width()+2*te1.cx);
               short yt;
               if(plot()->axesMode == PLOT_AXES_CROSS || plot()->axesMode == PLOT_X_AXIS_CROSS)
                  yt = nint(yScrn+te1.cy/2.0-te2.cy/3.0);
               else
	               yt = nint(dimensions.top()+dimensions.height()+te1.cy/2.0-te2.cy/3.0);
	            TextOut(hdc,xt,yt,str,strlen(str));	
	            rect().SetAxisRect(xt,yt,xt+te1.cx,yt+te1.cy);
	         }
		   }
		}
   }
   RestoreDC(hdc, -1);
   DeleteObject(exponentFont); 
   DeleteObject(labelFont); 
}

// ???Incorrectly named - this determines the plot direction and sets the range
// ???Also minX and maxX are not used (is this just for compatibility with vertical axis?
PlotDirection HorizontalAxis::calculateDataRange(float minX, float maxX)
{
	//plotDirection_ = PLT_FORWARD; //PLT_UNKNOWN;

	if(autoRange_)
	{
		min_ = FLT_MAX;
		max_ = -FLT_MAX;
	}

	for(Trace* t : traceList)
	{
		float lhs, rhs;
      if(t->getIgnoreXRange())
         continue;
		t->DiscoverXRange(&lhs, &rhs);  // Determine and set the X range and return array limits
		if(autoRange_) // Determine min and max range for all traces connected this axis
		{
			if(t->getMinX() < min_) min_ = t->getMinX(); 
			if(t->getMaxX() > max_) max_ = t->getMaxX();  
		}
	}
	if (mapping() == PLOT_LINEAR_X && autoRange_)
   {
	   min_ -= ((max_-min_) * this->plot()->traceXBorderFactor);
	   max_ += ((max_-min_) * this->plot()->traceXBorderFactor);
   }

// Make sure range is never zero
   if(min_ == max_)
   {
      min_ = min_-1;
      max_ = max_+1;
   }
	return(plotDirection_);
}
	
void HorizontalAxis::zoom(int dir, float relative_width, float relative_height)
{
   double tempMin = numeric_limits<double>::max();
	double tempMax = -numeric_limits<double>::max();
   bool error = false;

	switch(dir)
	{
		case(ID_REDUCE_BOTH):
      case(ID_REDUCE_HORIZ): // zoom in
         if(mapping() == PLOT_LOG_X)
         {
			   // difference is the total change between the smallest and highest possible data value on the axis.
			   float difference = (1 - horiz_reduce) * (fabs(log10(Max()) - log10(Min())));
			   tempMin = pow((double)10,log10((double)(Min())) + difference * relative_width);
			   tempMax = pow((double)10,log10((double)(Max())) - difference * (1 - relative_width));
         }
         else
         {
			   float difference = (1 - horiz_reduce) * (fabs(Max() - Min()));
            tempMin = Min() + difference * relative_width;
            tempMax = Max() - difference * (1 - relative_width);
         }
			break;

		case(ID_ENLARGE_HORIZ): // zoom out
		case(ID_ENLARGE_BOTH):
         if(mapping() == PLOT_LOG_X)
         {
			   float difference = (horiz_enlarge - 1) * (fabs(log10(Max()) - log10(Min())));
			   tempMin = pow((double)10,log10((double)(Min())) - difference * relative_width);
			   tempMax = pow((double)10,log10((double)(Max())) + difference * (1 - relative_width));
         }
         else
         {
			   float difference = (horiz_enlarge - 1) * (fabs(Max() - Min()));
            tempMin = Min() - difference * relative_width;
            tempMax = Max() + difference * (1 - relative_width);
         }
			break;

		default:
         error = true;
			break;
   }

    if(error == false)
    {
       if(mapping() == PLOT_LOG_X)
       {
          if(tempMin <= FLT_MIN/10.0) tempMin = FLT_MIN/10.0;
       }
       if(tempMax >= FLT_MAX/10.0) tempMax = FLT_MAX/10.0;
       setMin((float)tempMin);
       setMax((float)tempMax);
    }
}


void HorizontalAxis::zoomCentred(int dir)
{
   double tempMin = numeric_limits<double>::max();
	double tempMax = -numeric_limits<double>::max();
	double xc;
   bool error = false;

	if(Plot::zoomPoint)
	{
		xc = Plot::gZoomX;
      if(mapping() == PLOT_LOG_X)
         xc = log10(xc);
	}
	else
	{
		xc = (Max()+Min())/2;
      if(mapping() == PLOT_LOG_X)
         xc = (log10(Max()) + log10(Min()))/2;
	}


	switch(dir)
	{
	   case(ID_REDUCE_HORIZ):
         if(mapping() == PLOT_LOG_X)
         {
            tempMin = pow((double)10.0,(double)(xc - (log10(Max()) - log10(Min()) ) * horiz_enlarge/2));
            tempMax = pow((double)10.0,(double)(xc + (log10(Max()) - log10(Min()) ) * horiz_enlarge/2));
         }
         else
         {
		      tempMin = xc - (Max() - Min()) * horiz_enlarge/2;
		      tempMax = xc + (Max() - Min()) * horiz_enlarge/2;
         }
		   break; 
	   case(ID_ENLARGE_HORIZ):
         if(mapping() == PLOT_LOG_X)
         {
            tempMin = pow((double)10.0,(double)(xc - (log10(Max()) - log10(Min()) ) * horiz_reduce/2));
            tempMax = pow((double)10.0,(double)(xc + (log10(Max()) - log10(Min()) ) * horiz_reduce/2));
         }
         else
         {
		      tempMin = xc - (Max() - Min()) * horiz_reduce/2;
		      tempMax = xc + (Max() - Min()) * horiz_reduce/2;
         }
		   break; 
	   default:
         error = true;
		   break;
	}

    if(error == false)
    {
       if(mapping() == PLOT_LOG_X)
       {
           if(tempMin <= FLT_MIN/10.0) tempMin = FLT_MIN/10.0;
       }
       if(tempMax >= FLT_MAX/10.0) tempMax = FLT_MAX/10.0;
       setMin((float)tempMin);
       setMax((float)tempMax);
    }
}

void HorizontalAxis::shiftPlot(short dir)
{
   double tempMin = numeric_limits<double>::max();
	double tempMax = -numeric_limits<double>::max();
   bool error = false;
	if(this->plotDirection() == PLT_REVERSE)
	{
		if(dir == ID_SHIFT_RIGHT)
			dir = ID_SHIFT_LEFT;
		else if(dir == ID_SHIFT_LEFT)
			dir = ID_SHIFT_RIGHT;
	}

   switch(dir)
   {
      case(ID_SHIFT_RIGHT):
         if(mapping() == PLOT_LOG_X)
         {
            tempMin = pow((double)10.0,(double)((log10(Max()) - log10(Min())) * horiz_shift + log10(Min())));
            tempMax = pow((double)10.0,(double)((log10(Max()) - log10(Min())) * horiz_shift + log10(Max())));
            setMax(tempMax);
            setMin(tempMin);
         }
         else
         {
            tempMin = (Max() - Min()) * horiz_shift + Min();
            tempMax = (Max() - Min()) * horiz_shift + Max();
            setMax(tempMax);
            setMin(tempMin);
         }
         break;
      case(ID_SHIFT_LEFT):
         if(mapping() == PLOT_LOG_X)
         {
            tempMin = pow((double)10.0,(double)(-(log10(Max()) - log10(Min())) * horiz_shift + log10(Min())));
            tempMax = pow((double)10.0,(double)(-(log10(Max()) - log10(Min())) * horiz_shift + log10(Max())));
            setMax(tempMax);
            setMin(tempMin);
         }
         else
         {
            tempMin = -(Max() - Min()) * horiz_shift + Min();
            tempMax = -(Max() - Min()) * horiz_shift + Max();
            setMax(tempMax);
            setMin(tempMin);
         }
         break; 
		default:
         error = true;
		   break;
   }	

    if(error == false)
    {
       if(mapping() == PLOT_LOG_X)
       {
            if(tempMin <= FLT_MIN/10.0) tempMin = FLT_MIN/10.0;
       }
       if(tempMax >= FLT_MAX/10.0) tempMax = FLT_MAX/10.0;
       setMin((float)tempMin);
       setMax((float)tempMax);
    }
}

PositionList* HorizontalAxis::getLinearTickPositions(TickType tickType)
{
	PositionList* list = new PositionList();
   bool reverse = plot()->identifyReverseHorizontal(min_, max_, length(), base());

// Figure out best label and tick spacing
	float xTick = ticks().spacing();
	short ticksPerLabel = ticks().perLabel();
   float xLabel = ticksPerLabel*xTick;

   if(xLabel <= 0 || xTick <= 0 || xTick > xLabel)
      return list;
        
// Calculate leftmost tick and label positions in data points
   float xTickStart = (int)(min_/xTick)*xTick;
   if (xTickStart < min_) xTickStart += xTick;
  
   float xLabelStart = (int)(min_/xLabel)*xLabel;
   if (xLabelStart < min_) xLabelStart += xLabel;

// Correct for rounding problems - tickstart should always be less then labelstart
   if(xTickStart > xLabelStart)
     xTickStart = xLabelStart;

// Count number of ticks "used" so far
   int tickCnt = ticksPerLabel-(int)((xLabelStart-xTickStart)/xTick+0.5); 
    
// Work out number of ticks to plot
   int nrTicks = nint((max_-xTickStart)/xTick);

// Check for too many ticks!
   if(nrTicks > 5000)
      return list;

// Draw the axis
   for(int i = 0; i <= nrTicks; i++)
   {
      float x = xTickStart + i*xTick;

   // Convert from axes to screen coordinates
		float xScrn;
      if(plot()->getDimension() == 1)
         xScrn = dataToScrn(x);
      else
      {
         if(this->plotDirection() == PLT_REVERSE)

        // if(reverse)
            xScrn = nint(dim_.right() - (x-min_)*dim_.width()/(float)(max_-min_));
         else
            xScrn = nint(dim_.left() + (x-min_)*dim_.width()/(float)(max_-min_));
      }

      if(xScrn > dim_.left()+dim_.width() || 
			xScrn < dim_.left()) // Don't plot outside limits
         break;

      if(!(tickCnt % ticksPerLabel) && (MAJOR == tickType)) // Draw x label and long tick
      {
			list->push_back(xScrn);
      }
		else if ((tickCnt % ticksPerLabel) && (MINOR == tickType))
		{
			list->push_back(xScrn);
		}
      tickCnt++;      
   }
	return list;
}

PositionList* HorizontalAxis::getLogTickPositions(TickType tickType)
{
	PositionList* list = new PositionList();	
	    
// Determine the range limits

   long minPower = floor((log10(min_)));
   long maxPower = floor((log10(max_))+1);

// Determine the distance between adjacent power labels (i.e. 10, 100, 1000 ...)

	PlotDimensions& dimensions = plot()->getDimensions();

	float distbtwnlabels = (float)dimensions.width()/(log10(max_)-log10(min_));

// Loop over ticks and labels

   float xa = -(log10(min_)-minPower)*distbtwnlabels; // Initial tick location
   
   for(short power = minPower; power <= maxPower; power++)
   {
		float xl = (power-minPower)*distbtwnlabels + xa + dimensions.left(); // Label position
       
		for(short tick = 1; tick <= 9; tick++)
		{
			float x = xl + log10((float)tick)*distbtwnlabels;

			if(x < dimensions.left() || x > (dimensions.left()+dimensions.width())) // Ignore ticks and labels outside plot region
				continue;
             
			if((tick == 1) && (MAJOR == tickType)) // Draw x label and long tick since start of decade
			{
				list->push_back(nint(x));
			}
			else if ((tick != 1) && (MINOR == tickType))
			{
				list->push_back(nint(x));
			}
			if(power == maxPower) break; // Finished
		}
   }
	return list;
}

// Figure out best label and tick spacing
float HorizontalAxis::tickStart()
{
   if(autoScale())
   {
		float myLabel; 
		if (base() || length()) // ie, 2D
		{
			long visibleWidth = dynamic_cast<Plot2D*>(this->plot())->visibleWidth();
			float matWidth = dynamic_cast<Plot2D*>(this->plot())->matWidth();
			float xAxisWidth = 0;
         xAxisWidth= fabs(length());
			myLabel = getLabelSpacing((double)visibleWidth*xAxisWidth/(float)matWidth);
		}
      else // 1D
		{
			myLabel = getLabelSpacing((double)Max()-(double)Min());
		}
      ticks().setPerLabel(5);
      ticks().setSpacing(myLabel/ticks().perLabel());
		// Annoying workaround to cover data*/min*/max* discrepancy between 1D/2D plots
		float tickStart = plot()->getTickStart(this, &ticks());
		return tickStart;
	}
	return 0;
}

short HorizontalAxis::getTickLabel()
{
	if (!autoScale())
	{
		return OK;
	}
	
	if (tickStart() != 0 && ticks().spacing() == 0)
	{
		MessageDialog(prospaWin,MB_ICONERROR,"Error","Too much horizontal expansion");
		undoEnlargement();
	   return(ERR);
	}
	return OK;
}



//////////////////
// VERTICAL AXIS

VerticalAxis::VerticalAxis(Plot* parent, COLORREF color, float base, float length, VerticalAxisSide side)
: Axis(parent, color, base, length)
{
	setMapping(PLOT_LINEAR_Y);
	side_ = side;
}

VerticalAxis::VerticalAxis(const VerticalAxis& copyMe)
: Axis(copyMe)
{
	static_cast<VerticalAxisLabel*>(label_)->setParent(this);
	side_ = copyMe.side_;
}

VerticalAxis* VerticalAxis::clone() const
{
	return new VerticalAxis(*this);
}

Axis* VerticalAxis::createAxis(short mapping, Plot* parent, VerticalAxisSide side)
{
	Translator* t = Translator::makeTranslator(parent->getDimension(), mapping);
	VerticalAxis* axis = new VerticalAxis(parent, pwd->gridColor, 0,0);
   axis->setMapping(mapping);
	t->setAxis(axis);
	axis->setTranslator(t);
	ProspaFont font(pwd->labelFontName, pwd->labelFontColor, pwd->labelFontSize, pwd->labelFontStyle);
	VerticalAxisLabel* l = new VerticalAxisLabel(axis, font, "\0");
	axis->setLabel(l);
	axis->side_ = side;

	PlotGrid* grid = new PlotGridVertical(axis, false, false, pwd->gridColor, pwd->fineGridColor);
	axis->setGrid(grid);

	return axis;
}


VerticalAxis::~VerticalAxis()
{
}

void VerticalAxis::clear()
{
	Axis::clear();
}

void VerticalAxis::initialiseRange()
{
	setBase(0);
	setLength(dynamic_cast<Plot2D*>(plot())->matHeight());
}


void VerticalAxis::drawLeftHandTick(HDC hdc, int yScrn, float length)
{
	if ((plot()->axesMode != PLOT_AXES_CROSS && this->side_ == LEFT_VAXIS_SIDE) ||
		((plot()->axesMode == PLOT_AXES_BOX || plot()->axesMode == PLOT_Y_AXIS_BOX) && (!plot()->otherYAxis(this) || !plot()->otherYAxis(this)->isDrawable())))
	{
		SelectObject(hdc,axesPen_);   
		MoveToEx(hdc,dim_.left() - 1, yScrn, 0);
		LineTo(hdc, nint(dim_.left() - 1 - length), yScrn);	
	}
}

void VerticalAxis::drawRightHandTick(HDC hdc, int yScrn, float length)
{
	if ((plot()->axesMode != PLOT_AXES_CROSS && this->side_ == RIGHT_VAXIS_SIDE) ||
		((plot()->axesMode == PLOT_AXES_BOX || plot()->axesMode == PLOT_Y_AXIS_BOX) && (!plot()->otherYAxis(this) || !plot()->otherYAxis(this)->isDrawable())))
	{
		SelectObject(hdc,axesPen_);   
		MoveToEx(hdc, dim_.left() + dim_.width() + 1, yScrn, 0);
		LineTo(hdc, nint(dim_.left() + dim_.width() + 1 + length), yScrn);
	}
}	

void VerticalAxis::drawCross(HDC hdc, int xPosition, int position, float length)
{
	if (plot()->axesMode == PLOT_AXES_CROSS || plot()->axesMode == PLOT_Y_AXIS_CROSS )
	{
		SelectObject(hdc,axesPen_);   
		if(xPosition > dim_.left() && xPosition < dim_.left() + dim_.width())
		{	
			MoveToEx(hdc, nint(xPosition - length/2.0), position, 0);
			LineTo(hdc, nint(xPosition + length/2.0), position); 
		}
	}
}

void VerticalAxis::drawLinear(HDC hdc)
{	
	char exponent[20];
	char mantissa[20];
	short exp_label = 0;
	bool reverse = plot()->identifyReverseVertical(min_, max_, length(), base());

	// Extract tick and label spacing from plot info structure
	float yTick = ticks().spacing();
	short ticksPerLabel = ticks().perLabel();
	float yLabel = ticksPerLabel*yTick;

	if(yLabel == 0 || yTick == 0)
		return;    

	// Calculate top most tick and label positions in data points
	float yTickStart = (int)(min_/yTick)*yTick;
	if(yTickStart < min_) yTickStart += yTick;

	float yLabelStart = (int)(min_/yLabel)*yLabel;
	if(yLabelStart < min_) yLabelStart += yLabel;

	// Correct for rounding problems - tickstart should always be less then labelstart
	if(yTickStart > yLabelStart)
		yTickStart = yLabelStart;

	// Draw ticks and labels 
	int tickCnt = ticksPerLabel-(int)((yLabelStart-yTickStart)/yTick+0.5); 

	Translator* peerTranslator = this->plot()->curXAxis()->translator();
	float xScrn = peerTranslator->dataToScrn(0);

	// Work out number of ticks to plot
	int nrTicks = nint((max_-yTickStart)/yTick);

	// Check for too many ticks!
	if(nrTicks > 5000)
		return;

	SaveDC(hdc);

	// Define pens and select them
	setPenColors();
	SelectObject(hdc, axesPen_);
	setTextColor(hdc);
	SetBkMode(hdc,TRANSPARENT);

	// Select font for labels
	HFONT labelFont = GetFont(hdc, ticks().font(),0,0); 
	SelectObject(hdc,labelFont);

	// Draw the axis
	for(int i = 0; i <= nrTicks; i++)
	{
		float y = yTickStart + i*yTick;
		// Convert from axes to screen coordinates
		float yScrn;
		if(plot()->getDimension() == 1)
			yScrn = dataToScrn(y);  
		else
		{
			if(this->plotDirection() == PLT_REVERSE)
				yScrn = nint(dim_.top() + (y-min_)*dim_.height()/(float)(max_-min_));
			else
				yScrn = nint(dim_.height() + dim_.top() - (y-min_)*dim_.height()/(float)(max_-min_));
		}

		if(yScrn < dim_.top() || yScrn > dim_.height() + dim_.top()) // Don't plot outside limits
			break;

		if(tickCnt == ticksPerLabel) // Draw long tick and label
		{
			drawLeftHandTick(hdc, yScrn, ticks().majorLength());
			drawRightHandTick(hdc, yScrn, ticks().majorLength());
			drawCross(hdc, xScrn, yScrn, ticks().majorLength());

			// Handle large or small label values
			FloatSplit(yLabel,mantissa,exponent,0);
			sscanf(exponent,"%hd",&exp_label);
	
			char str[MAX_STR];
			if(yLabel < 10 && yLabel >= 0.01)
			{
				char format[20];
				sprintf(format,"%%1.%hdf",-exp_label);
				sprintf(str,format,y);
			}
			else if(yLabel > 300 || yLabel < 0.01)
			{
				FloatSplit(y,mantissa,exponent,5);
				short ex;
				sscanf(exponent,"%hd",&ex);
				float man;
				sscanf(mantissa,"%f",&man);
				if(ex > exp_label)
				{
					for(short i = 0; i < ex-exp_label; i++)
						man *= 10.0;
				}
				else
				{
					for(short i = 0; i < exp_label-ex; i++)
						man /= 10.0;
				}
				sprintf(str,"%ld",nint(man));
			}
			else
				sprintf(str,"%ld",nint(y));
			
			short xt, yt;
			SIZE te; 
			GetTextExtentPoint32(hdc, str, strlen(str), &te);
			if(plot()->axesMode == PLOT_AXES_CROSS || plot()->axesMode == PLOT_Y_AXIS_CROSS)
			{
				xt = xScrn-te.cx-fabs(3*ticks().majorLength()/4);
				yt = yScrn-te.cy/2;
				TextOut(hdc,xt,yt,str,strlen(str)); 
			}
			else
			{
				xt = dim_.left() - fabs(5*ticks().majorLength()/4) - te.cx;
				if (side_ == RIGHT_VAXIS_SIDE)
					xt = dim_.right() + fabs(5*ticks().majorLength()/4);
				yt = yScrn-te.cy/2;
				TextOut(hdc,xt,yt,str,strlen(str));
			} 

			rect().SetAxisRect(xt,yt,xt+te.cx,yt+te.cy);

			tickCnt = 0;
		}
		else // Draw minor ticks
		{
			if(((plot()->axesMode == PLOT_AXES_CORNER || plot()->axesMode == PLOT_Y_AXIS) && this->side_ == LEFT_VAXIS_SIDE) || plot()->axesMode == PLOT_AXES_BOX ||  plot()->axesMode == PLOT_Y_AXIS_BOX)
			{      
				drawLeftHandTick(hdc, yScrn, ticks().minorLength());
			}
			if(((plot()->axesMode == PLOT_AXES_CORNER || plot()->axesMode == PLOT_Y_AXIS) && this->side_ == RIGHT_VAXIS_SIDE)  || plot()->axesMode == PLOT_AXES_BOX ||  plot()->axesMode == PLOT_Y_AXIS_BOX)
			{
				drawRightHandTick(hdc, yScrn, ticks().minorLength());
			}
			if(plot()->axesMode == PLOT_AXES_CROSS || plot()->axesMode == PLOT_Y_AXIS_CROSS)
			{ 	
				drawCross(hdc, xScrn, yScrn, ticks().minorLength());
			}
		}
		tickCnt++;      
	}

	// Draw scaling label
	if(yLabel > 300 || yLabel < 0.01)
	{
		short x1 = 0, y1 = 0;
		short sWidth,sHeight;

		HFONT vScaleFont = GetFont(hdc, ticks().font(),-3*plot()->y_scaling_factor(),0); 
		SelectObject(hdc,vScaleFont);

		char str[MAX_STR];
		strncpy(str,"×10",MAX_STR);
		SIZE te; 
		GetTextExtentPoint32(hdc, str, strlen(str), &te);
		sWidth = te.cx;
		sHeight = te.cy;

		if(plot()->axesMode == PLOT_AXES_CROSS) // Figure out position of "x10"
		{
			x1 = xScrn+fabs(ticks().majorLength());
			y1 = dim_.top()+0.5*sHeight;
		}
		else
		{
			x1 = dim_.left()-fabs(ticks().majorLength())-te.cx;
			if (side_ == RIGHT_VAXIS_SIDE)
				x1 = dim_.right() + fabs(ticks().majorLength());
			y1 = dim_.top()-1.5*sHeight;
		}

		TextOut(hdc,x1,y1,str,strlen(str)); // Draw "x10"
		HFONT exponentFont = GetFont(hdc, ticks().font(),-5*plot()->y_scaling_factor(),0); 
		SelectObject(hdc,exponentFont);
		sprintf(str,"%hd",exp_label);
		GetTextExtentPoint32(hdc, str, strlen(str), &te);
		sHeight = te.cy;
		TextOut(hdc,x1+sWidth,y1-sHeight/4,str,strlen(str)); // Draw exponent
		RestoreDC(hdc, -1);
		DeleteObject(labelFont); 
		DeleteObject(vScaleFont); 
		DeleteObject(exponentFont); 
	} 
	else
	{
		RestoreDC(hdc, -1);
		DeleteObject(labelFont);   
	}
}

void VerticalAxis::drawLog(HDC hdc)
{
   SaveDC(hdc);
   bool reverse;
     
   short dim = this->plot()->getDimension();

   if(dim == 1)
   {
      reverse = plot()->identifyReverseVertical(min_, max_, length(), base());
   }
   else
   {
      float a,b;
      reverse = plot()->identifyReverseVertical(a,b, length(), base());
      min_ = this->dataToUser(min_);
      max_ = this->dataToUser(max_);
   }

// Define pens
	setPenColors();
	SelectObject(hdc, axesPen_);
	setTextColor(hdc);
	SetBkMode(hdc,TRANSPARENT);
	
// Determine the range limits

   long minPower = floor((log10(min_)));
   long maxPower = floor((log10(max_))+1);

   float distbtwnlabels = (float)dim_.height()/(log10(max_)-log10(min_));

// Select font for labels
 
   HFONT labelFont = GetFont(hdc, ticks().font(),0,0); 
   HFONT exponentFont = GetFont(hdc, ticks().font(),-4*plot()->y_scaling_factor(),0); 

// Loop over ticks and labels

   float ya;
	if(reverse)
      ya = dim_.top()-(log10(min_)-minPower)*distbtwnlabels; // Initial tick location
   else
      ya = dim_.top()+dim_.height()+(log10(min_)-minPower)*distbtwnlabels; // Initial tick location

   for(short power = minPower; power <= maxPower; power++)
   {
		float yl;
	   if(reverse)
         yl = ya + (power-minPower)*distbtwnlabels; // Label position
      else
         yl = ya - (power-minPower)*distbtwnlabels; // Label position
                    
       for(short tick = 1; tick <= 9; tick++)
       {
			char str[MAX_STR];
			float y;
	      if(reverse)
            y = yl + log10((float)tick)*distbtwnlabels;
         else
            y = yl - log10((float)tick)*distbtwnlabels;

          if(y < dim_.top() || y > (dim_.top()+dim_.height())) // Ignore ticks and labels outside plot region
             continue;
          
         if(tick == 1) // Draw y label and long tick
         {	
				drawLeftHandTick(hdc, nint(y), ticks().majorLength());
				drawRightHandTick(hdc, nint(y), ticks().majorLength());
	
            SelectObject(hdc,labelFont); // Write "10"
            strncpy(str,"10",MAX_STR);
				SIZE te1; 
            GetTextExtentPoint32(hdc, str, strlen(str), &te1);
				int xt, yt;
				if(ticks().majorLength() > 0)
            {
					xt = nint(dim_.left()-2*te1.cx-ticks().majorLength()/2.0);
					if (side() == RIGHT_VAXIS_SIDE)
					{
						xt = nint(dim_.right()+ticks().majorLength());
					}
               yt = nint(y-te1.cy/2.0);
               TextOut(hdc,xt,yt,str,strlen(str));
            }
            else  
            {
               xt = (dim_.left()-2*te1.cx);
					if (side() == RIGHT_VAXIS_SIDE)
					{
						xt = nint(dim_.right());
					}
               yt = nint(y-te1.cy/2.0);
               TextOut(hdc,xt,yt,str,strlen(str));  
            }
            rect().SetAxisRect(xt,yt,xt+te1.cx,yt+te1.cy);
 
            SelectObject(hdc,exponentFont);  // Write exponent to 10           
            sprintf(str,"%hd",power);
				SIZE te2;
            GetTextExtentPoint32(hdc, str, strlen(str), &te2);
				if(ticks().majorLength() > 0)
            {
					int xt = nint(dim_.left()-te1.cx-ticks().majorLength()/2.0);
					if (side() == RIGHT_VAXIS_SIDE)
					{
						xt = nint(dim_.right() + te1.cx + ticks().majorLength());
					}
               int yt = nint(y-te1.cy/2.0-te2.cy/3.0);
               TextOut(hdc,xt,yt,str,strlen(str));  
            }
            else
            {

               int xt = (dim_.left()-te1.cx);
					if (side() == RIGHT_VAXIS_SIDE)
					{
						xt = dim_.right()+te1.cx;
					}
               int yt = nint(y-te1.cy/2.0-te2.cy/3.0);
               TextOut(hdc,xt,yt,str,strlen(str)); 
            } 
            rect().SetAxisRect(xt,yt,xt+te2.cx,yt+te2.cy);
             
            if(power == maxPower) break; // Finished
         }
         else // Draw minor ticks
         {
				drawLeftHandTick(hdc, y, ticks().minorLength());
				drawRightHandTick(hdc, y, ticks().minorLength());

		      if(maxPower-minPower <= 2) // If only 1 main label then label minor ticks with mantissa
		      {
               SelectObject(hdc,labelFont); // Write "10" (dummy write to get font size)
               strncpy(str,"10",MAX_STR);
					SIZE te1;
               GetTextExtentPoint32(hdc, str, strlen(str), &te1);

               SelectObject(hdc,exponentFont);  // Write mantissa (2,3,4 ... 9)           
               sprintf(str,"%hd",tick);
					SIZE te2;
               GetTextExtentPoint32(hdc, str, strlen(str), &te2);
					int xt, yt;
					if(ticks().minorLength() > 0)
	            {
						xt = nint(dim_.left()-2*te2.cx-ticks().minorLength()/2.0);
						if (side() == RIGHT_VAXIS_SIDE)
						{
							xt = nint(dim_.right()+3*ticks().minorLength()/2.0);
						}
	               yt = nint(y-te1.cy/2.0);
	               TextOut(hdc,xt,yt,str,strlen(str));
	            }
	            else  
	            {
	               xt = (dim_.left()-2*te2.cx);
						if (side() == RIGHT_VAXIS_SIDE)
						{
							xt = nint(dim_.right());
						}
	               yt = nint(y-te1.cy/2.0);
	               TextOut(hdc,xt,yt,str,strlen(str)); 
	            } 
               rect().SetAxisRect(xt,yt,xt+te1.cx,yt+te1.cy);
            }
            else if(maxPower-minPower == 1) // If no main labels present then need to have multiplier
            {
	            SelectObject(hdc,labelFont); // Write "x10"
	            strncpy(str,"x10",MAX_STR);
					SIZE te1;
	            GetTextExtentPoint32(hdc, str, strlen(str), &te1);
	            int xt = dim_.left()-te1.cx;
					if (side() == RIGHT_VAXIS_SIDE)
					{
						xt = dim_.right()+2*te1.cx;
					}
	            int yt = nint(dim_.top()-te1.cy);
	            TextOut(hdc,xt,yt,str,strlen(str));  
               rect().SetAxisRect(xt,yt,xt+te1.cx,yt+te1.cy);	 
	            SelectObject(hdc,exponentFont);  // Write exponent to 10           
	            sprintf(str,"%hd",minPower);
					SIZE te2;
	            GetTextExtentPoint32(hdc, str, strlen(str), &te2);
	            xt = dim_.left();
					if (side() == RIGHT_VAXIS_SIDE)
					{
						xt = dim_.right()+2*te1.cx;
					}
	            yt = nint(dim_.top()-te1.cy-te2.cy/3.0);
	            TextOut(hdc,xt,yt,str,strlen(str)); 
               rect().SetAxisRect(xt,yt,xt+te1.cx,yt+te1.cy);
	         }
		   }
		}
   }
   RestoreDC(hdc, -1);
   DeleteObject(exponentFont); 
   DeleteObject(labelFont); 
}


void VerticalAxis::drawIndependentLinear(HDC hdc)
{	
	char exponent[20];
	char mantissa[20];
	short exp_label = 0;
//	bool reverse = plot()->identifyReverseVertical(min_, max_, length(), base());

	// Extract tick and label spacing from plot info structure
	float yTick = ticks().spacing();
	short ticksPerLabel = ticks().perLabel();
	float yLabel = ticksPerLabel*yTick;

	if(yLabel == 0 || yTick == 0)
		return;    

	// Calculate top most tick and label positions in data points
	float yTickStart = (int)(minIndep_/yTick)*yTick;
	if(yTickStart < minIndep_) yTickStart += yTick;

	float yLabelStart = (int)(minIndep_/yLabel)*yLabel;
	if(yLabelStart < minIndep_) yLabelStart += yLabel;

	// Correct for rounding problems - tickstart should always be less then labelstart
	if(yTickStart > yLabelStart)
		yTickStart = yLabelStart;

	// Draw ticks and labels 
	int tickCnt = ticksPerLabel-(int)((yLabelStart-yTickStart)/yTick+0.5); 

	Translator* peerTranslator = this->plot()->curXAxis()->translator();
	float xScrn = peerTranslator->dataToScrn(0);

	// Work out number of ticks to plot
	int nrTicks = nint((maxIndep_-yTickStart)/yTick);

	// Check for too many ticks!
	if(nrTicks > 5000)
		return;

	SaveDC(hdc);

	// Define pens and select them
	setPenColors();
	SelectObject(hdc, axesPen_);
	setTextColor(hdc);
	SetBkMode(hdc,TRANSPARENT);

	// Select font for labels
	HFONT labelFont = GetFont(hdc, ticks().font(),0,0); 
	SelectObject(hdc,labelFont);

	// Draw the axis
	for(int i = 0; i <= nrTicks; i++)
	{
		float y = yTickStart + i*yTick;
		// Convert from axes to screen coordinates
		float yScrn;
		if(plot()->getDimension() == 1)
         yScrn = (long)(dim_.bottom() - 0.90*(y-minIndep_)*(float)dim_.height() / (float)(maxIndep_-minIndep_)+0.5 - 0.05*dim_.height());
		else
		{
			if(this->plotDirection() == PLT_REVERSE)
				yScrn = nint(dim_.top() + (y-minIndep_)*dim_.height()/(float)(maxIndep_-minIndep_));
			else
				yScrn = nint(dim_.height() + dim_.top() - (y-minIndep_)*dim_.height()/(float)(maxIndep_-minIndep_));
		}

		if(yScrn < dim_.top() || yScrn > dim_.bottom()) // Don't plot outside limits
			break;

		if(tickCnt == ticksPerLabel) // Draw long tick and label
		{
			drawLeftHandTick(hdc, yScrn, ticks().majorLength());
			drawRightHandTick(hdc, yScrn, ticks().majorLength());
			drawCross(hdc, xScrn, yScrn, ticks().majorLength());

			// Handle large or small label values
			FloatSplit(yLabel,mantissa,exponent,0);
			sscanf(exponent,"%hd",&exp_label);
	
			char str[MAX_STR];
			if(yLabel < 10 && yLabel >= 0.01)
			{
				char format[20];
				sprintf(format,"%%1.%hdf",-exp_label);
				sprintf(str,format,y);
			}
			else if(yLabel > 300 || yLabel < 0.01)
			{
				FloatSplit(y,mantissa,exponent,5);
				short ex;
				sscanf(exponent,"%hd",&ex);
				float man;
				sscanf(mantissa,"%f",&man);
				if(ex > exp_label)
				{
					for(short i = 0; i < ex-exp_label; i++)
						man *= 10.0;
				}
				else
				{
					for(short i = 0; i < exp_label-ex; i++)
						man /= 10.0;
				}
				sprintf(str,"%ld",nint(man));
			}
			else
				sprintf(str,"%ld",nint(y));
			
			short xt, yt;
			SIZE te; 
			GetTextExtentPoint32(hdc, str, strlen(str), &te);
			if(plot()->axesMode == PLOT_AXES_CROSS || plot()->axesMode == PLOT_Y_AXIS_CROSS)
			{
				xt = xScrn-te.cx-fabs(3*ticks().majorLength()/4);
				yt = yScrn-te.cy/2;
				TextOut(hdc,xt,yt,str,strlen(str)); 
			}
			else
			{
				xt = dim_.left() - fabs(5*ticks().majorLength()/4) - te.cx;
				if (side_ == RIGHT_VAXIS_SIDE)
					xt = dim_.right() + fabs(5*ticks().majorLength()/4);
				yt = yScrn-te.cy/2;
				TextOut(hdc,xt,yt,str,strlen(str));
			} 

			rect().SetAxisRect(xt,yt,xt+te.cx,yt+te.cy);

			tickCnt = 0;
		}
		else // Draw minor ticks
		{
			if(((plot()->axesMode == PLOT_AXES_CORNER || plot()->axesMode == PLOT_Y_AXIS) && this->side_ == LEFT_VAXIS_SIDE) || plot()->axesMode == PLOT_AXES_BOX ||  plot()->axesMode == PLOT_Y_AXIS_BOX || plot()->axesMode == PLOT_AXIS_BOX_Y_INDEPENDENT)
			{      
				drawLeftHandTick(hdc, yScrn, ticks().minorLength());
			}
			if(((plot()->axesMode == PLOT_AXES_CORNER || plot()->axesMode == PLOT_Y_AXIS) && this->side_ == RIGHT_VAXIS_SIDE)  || plot()->axesMode == PLOT_AXES_BOX ||  plot()->axesMode == PLOT_Y_AXIS_BOX || plot()->axesMode == PLOT_AXIS_BOX_Y_INDEPENDENT)
			{
				drawRightHandTick(hdc, yScrn, ticks().minorLength());
			}
			if(plot()->axesMode == PLOT_AXES_CROSS || plot()->axesMode == PLOT_Y_AXIS_CROSS)
			{ 	
				drawCross(hdc, xScrn, yScrn, ticks().minorLength());
			}
		}
		tickCnt++;      
	}

	// Draw scaling label
	if(yLabel > 300 || yLabel < 0.01)
	{
		short x1 = 0, y1 = 0;
		short sWidth,sHeight;

		HFONT vScaleFont = GetFont(hdc, ticks().font(),-3*plot()->y_scaling_factor(),0); 
		SelectObject(hdc,vScaleFont);

		char str[MAX_STR];
		strncpy(str,"×10",MAX_STR);
		SIZE te; 
		GetTextExtentPoint32(hdc, str, strlen(str), &te);
		sWidth = te.cx;
		sHeight = te.cy;

		if(plot()->axesMode == PLOT_AXES_CROSS) // Figure out position of "x10"
		{
			x1 = xScrn+fabs(ticks().majorLength());
			y1 = dim_.top()+0.5*sHeight;
         TextMessage("x,y = %hd,&hd\n",x1,y1);
		}
		else
		{
			x1 = dim_.left()-fabs(ticks().majorLength())-te.cx;
			if (side_ == RIGHT_VAXIS_SIDE)
				x1 = dim_.right() + fabs(ticks().majorLength());
			y1 = dim_.top()-1.5*sHeight;
		}

		TextOut(hdc,x1,y1,str,strlen(str)); // Draw "x10"
		HFONT exponentFont = GetFont(hdc, ticks().font(),-5*plot()->y_scaling_factor(),0); 
		SelectObject(hdc,exponentFont);
		sprintf(str,"%hd",exp_label);
		GetTextExtentPoint32(hdc, str, strlen(str), &te);
		sHeight = te.cy;
		TextOut(hdc,x1+sWidth,y1-sHeight/4,str,strlen(str)); // Draw exponent
		RestoreDC(hdc, -1);
		DeleteObject(labelFont); 
		DeleteObject(vScaleFont); 
		DeleteObject(exponentFont); 
	} 
	else
	{
		RestoreDC(hdc, -1);
		DeleteObject(labelFont);   
	}
}


void VerticalAxis::drawIndependentLog(HDC hdc)
{
   SaveDC(hdc);
   bool reverse;
     
   short dim = this->plot()->getDimension();

   if(dim == 1)
   {
      reverse = plot()->identifyReverseVertical(minIndep_, maxIndep_, length(), base());
   }
   else
   {
      float a,b;
      reverse = plot()->identifyReverseVertical(a,b, length(), base());
      minIndep_ = this->dataToUser(minIndep_);
      maxIndep_ = this->dataToUser(maxIndep_);
   }

// Define pens
	setPenColors();
	SelectObject(hdc, axesPen_);
	setTextColor(hdc);
	SetBkMode(hdc,TRANSPARENT);
	
// Determine the range limits
   long minPower = floor((log10(minIndep_)));
   long maxPower = floor((log10(maxIndep_))+1);

   float distbtwnlabels = (float)dim_.height()/(log10(maxIndep_)-log10(minIndep_));

// Select font for labels 
   HFONT labelFont = GetFont(hdc, ticks().font(),0,0); 
   HFONT exponentFont = GetFont(hdc, ticks().font(),-4*plot()->y_scaling_factor(),0); 

// Loop over ticks and labels
   float ya;
	if(reverse)
      ya = dim_.top()-(log10(minIndep_)-minPower)*distbtwnlabels; // Initial tick location
   else
      ya = dim_.top()+dim_.height()+0.9*(log10(minIndep_)-minPower)*distbtwnlabels - 0.05*dim_.height(); // Initial tick location

         //   yScrn = (long)(dim_.bottom() - 0.90*(y-minIndep_)*(float)dim_.height() / (float)(maxIndep_-minIndep_)+0.5 - 0.05*dim_.height());


   for(short power = minPower; power <= maxPower; power++)
   {
		float yl;
	   if(reverse)
         yl = ya + (power-minPower)*distbtwnlabels; // Label position
      else
         yl = ya - 0.9*(power-minPower)*distbtwnlabels; // Label position
                    
       for(short tick = 1; tick <= 9; tick++)
       {
			char str[MAX_STR];
			float y;
	      if(reverse)
            y = yl + log10((float)tick)*distbtwnlabels;
         else
            y = yl - 0.9*log10((float)tick)*distbtwnlabels;

          if(y < dim_.top() || y > (dim_.top()+dim_.height())) // Ignore ticks and labels outside plot region
             continue;
          
         if(tick == 1) // Draw y label and long tick
         {	
				drawLeftHandTick(hdc, nint(y), ticks().majorLength());
				drawRightHandTick(hdc, nint(y), ticks().majorLength());
	
            SelectObject(hdc,labelFont); // Write "10"
            strncpy(str,"10",MAX_STR);
				SIZE te1; 
            GetTextExtentPoint32(hdc, str, strlen(str), &te1);
				int xt, yt;
				if(ticks().majorLength() > 0)
            {
					xt = nint(dim_.left()-2*te1.cx-ticks().majorLength()/2.0);
					if (side() == RIGHT_VAXIS_SIDE)
					{
						xt = nint(dim_.right()+ticks().majorLength());
					}
               yt = nint(y-te1.cy/2.0);
               TextOut(hdc,xt,yt,str,strlen(str));
            }
            else  
            {
               xt = (dim_.left()-2*te1.cx);
					if (side() == RIGHT_VAXIS_SIDE)
					{
						xt = nint(dim_.right());
					}
               yt = nint(y-te1.cy/2.0);
               TextOut(hdc,xt,yt,str,strlen(str));  
            }
            rect().SetAxisRect(xt,yt,xt+te1.cx,yt+te1.cy);
 
            SelectObject(hdc,exponentFont);  // Write exponent to 10           
            sprintf(str,"%hd",power);
				SIZE te2;
            GetTextExtentPoint32(hdc, str, strlen(str), &te2);
				if(ticks().majorLength() > 0)
            {
					int xt = nint(dim_.left()-te1.cx-ticks().majorLength()/2.0);
					if (side() == RIGHT_VAXIS_SIDE)
					{
						xt = nint(dim_.right() + te1.cx + ticks().majorLength());
					}
               int yt = nint(y-te1.cy/2.0-te2.cy/3.0);
               TextOut(hdc,xt,yt,str,strlen(str));  
            }
            else
            {

               int xt = (dim_.left()-te1.cx);
					if (side() == RIGHT_VAXIS_SIDE)
					{
						xt = dim_.right()+te1.cx;
					}
               int yt = nint(y-te1.cy/2.0-te2.cy/3.0);
               TextOut(hdc,xt,yt,str,strlen(str)); 
            } 
            rect().SetAxisRect(xt,yt,xt+te2.cx,yt+te2.cy);
             
            if(power == maxPower) break; // Finished
         }
         else // Draw minor ticks
         {
				drawLeftHandTick(hdc, y, ticks().minorLength());
				drawRightHandTick(hdc, y, ticks().minorLength());

		      if(maxPower-minPower <= 2) // If only 1 main label then label minor ticks with mantissa
		      {
               SelectObject(hdc,labelFont); // Write "10" (dummy write to get font size)
               strncpy(str,"10",MAX_STR);
					SIZE te1;
               GetTextExtentPoint32(hdc, str, strlen(str), &te1);

               SelectObject(hdc,exponentFont);  // Write mantissa (2,3,4 ... 9)           
               sprintf(str,"%hd",tick);
					SIZE te2;
               GetTextExtentPoint32(hdc, str, strlen(str), &te2);
					int xt, yt;
					if(ticks().minorLength() > 0)
	            {
						xt = nint(dim_.left()-2*te2.cx-ticks().minorLength()/2.0);
						if (side() == RIGHT_VAXIS_SIDE)
						{
							xt = nint(dim_.right()+3*ticks().minorLength()/2.0);
						}
	               yt = nint(y-te1.cy/2.0);
	               TextOut(hdc,xt,yt,str,strlen(str));
	            }
	            else  
	            {
	               xt = (dim_.left()-2*te2.cx);
						if (side() == RIGHT_VAXIS_SIDE)
						{
							xt = nint(dim_.right());
						}
	               yt = nint(y-te1.cy/2.0);
	               TextOut(hdc,xt,yt,str,strlen(str)); 
	            } 
               rect().SetAxisRect(xt,yt,xt+te1.cx,yt+te1.cy);
            }
            else if(maxPower-minPower == 1) // If no main labels present then need to have multiplier
            {
	            SelectObject(hdc,labelFont); // Write "x10"
	            strncpy(str,"x10",MAX_STR);
					SIZE te1;
	            GetTextExtentPoint32(hdc, str, strlen(str), &te1);
	            int xt = dim_.left()-te1.cx;
					if (side() == RIGHT_VAXIS_SIDE)
					{
						xt = dim_.right()+2*te1.cx;
					}
	            int yt = nint(dim_.top()-te1.cy);
	            TextOut(hdc,xt,yt,str,strlen(str));  
               rect().SetAxisRect(xt,yt,xt+te1.cx,yt+te1.cy);	 
	            SelectObject(hdc,exponentFont);  // Write exponent to 10           
	            sprintf(str,"%hd",minPower);
					SIZE te2;
	            GetTextExtentPoint32(hdc, str, strlen(str), &te2);
	            xt = dim_.left();
					if (side() == RIGHT_VAXIS_SIDE)
					{
						xt = dim_.right()+2*te1.cx;
					}
	            yt = nint(dim_.top()-te1.cy-te2.cy/3.0);
	            TextOut(hdc,xt,yt,str,strlen(str)); 
               rect().SetAxisRect(xt,yt,xt+te1.cx,yt+te1.cy);
	         }
		   }
		}
   }
   RestoreDC(hdc, -1);
   DeleteObject(exponentFont); 
   DeleteObject(labelFont); 
}


PlotDirection VerticalAxis::calculateDataRange(float minX, float maxX)
{
	if (autoRange_)
	{
      max_ = -1e30;
		min_ = 1e30;
		for(Trace* t : traceList)
	   {
         if(t->getIgnoreYRange())
            continue;
	      t->DiscoverYRange(minX,maxX);
	      if(t->getMinY() < min_) min_ = t->getMinY(); 
	      if(t->getMaxY() > max_) max_ = t->getMaxY();   
	   }
      if(min_ == max_)
      {
         if(min_ == 0)
         {
            min_ = min_ - 1;
            max_ = max_ + 1;
         }
         else
         {
            min_ = min_ - fabs(min_)/10;
            max_ = max_ + fabs(max_)/10;
         }
      }
	}

// Add gaps at top and bottom for visibility if in linear mode
	if (mapping() == PLOT_LINEAR_Y && autoRange_)
	{
		min_ -= ((max_-min_) * this->plot()->traceYBorderFactor);
		max_ += ((max_-min_) * this->plot()->traceYBorderFactor);
	}
	return PLT_FORWARD;
}

void VerticalAxis::addTrace(Trace* t)
{
	Axis::addTrace(t);
	if (!label_->isTextLocked())
	{
		label_->setText(t->getName());
	}
}

void VerticalAxis::removeTrace(Trace* t)
{
	Axis::removeTrace(t);
	if (!traceList.empty() && !label_->isTextLocked())
	{
		label_->setText(traceList[traceList.size() - 1]->getName());
	}
   if(!plot()->getOverRideAutoRange())
      updateExtrema(plot()->curXAxis()->Min(), plot()->curXAxis()->Max());
}


void VerticalAxis::zoom(int dir, float relative_width, float relative_height)
{
   double tempMin = numeric_limits<double>::max();
	double tempMax = -numeric_limits<double>::max();
   bool error = false;
   extern int gPlotViewVersion;

   switch(dir)
   {
      case(ID_REDUCE_VERT):
		case(ID_REDUCE_BOTH):

         if(mapping() == PLOT_LOG_Y && plot()->axesMode != PLOT_AXIS_BOX_Y_INDEPENDENT)
         {
			   float difference = (1 - vert_reduce) * (fabs(log10(Max()) - log10(Min())));
            tempMin = pow((double)10,log10((double)(Min())) + difference * (1 - relative_height));
            tempMax = pow((double)10,log10((double)(Max())) - difference * relative_height);
         }
         else
         {
			   float difference = (1 - vert_reduce) * (fabs(Max() - Min()));
            if(gPlotViewVersion == 1)
            {
               tempMin = Min() + difference * (1 - relative_height);
               tempMax = Max() - difference * relative_height;
            }
            else
            {
               tempMin = vert_reduce * Min();
               tempMax = vert_reduce * Max();
            }
         }
			break;

		case(ID_ENLARGE_VERT):
		case(ID_ENLARGE_BOTH):

         if(mapping() == PLOT_LOG_Y && plot()->axesMode != PLOT_AXIS_BOX_Y_INDEPENDENT)
         {
			   float difference = (vert_enlarge - 1) * (fabs(log10(Max()) - log10(Min())));
            tempMin = pow((double)10,log10((double)(Min())) - difference * (1 - relative_height));
            tempMax = pow((double)10,log10((double)(Max())) + difference * relative_height);
         }
         else
         {
			   float difference = (vert_enlarge - 1) * (fabs(Max() - Min()));
            if(gPlotViewVersion == 1)
            {
			      tempMin = Min() - difference *(1 - relative_height);
			      tempMax = Max() + difference * relative_height;
            }
            else
            {
               tempMin = 1.0/vert_reduce * Min();
               tempMax = 1.0/vert_reduce * Max();
            }
         }
		   break;

		default:
         error = true;
			break;
   }

    if(error == false)
    {
       if(mapping() == PLOT_LOG_Y && plot()->axesMode != PLOT_AXIS_BOX_Y_INDEPENDENT)
       {
          if(tempMin <= FLT_MIN/10.0) tempMin = FLT_MIN/10.0;
       }
       if(tempMax >= FLT_MAX/10.0) tempMax = FLT_MAX/10.0;
       setMin((float)tempMin);
       setMax((float)tempMax);
    }
}

void VerticalAxis::zoomCentred(int dir)
{
   double tempMin = numeric_limits<double>::max();
	double tempMax = -numeric_limits<double>::max();
	double yc;
   bool error = false;

	if(Plot::zoomPoint)
	{
		yc = Plot::gZoomY;
      if(mapping() == PLOT_LOG_Y && plot()->axesMode != PLOT_AXIS_BOX_Y_INDEPENDENT)
         yc = log10(yc);
	}
	else
	{
		yc = (Max()+Min())/2;
      if(mapping() == PLOT_LOG_Y && plot()->axesMode != PLOT_AXIS_BOX_Y_INDEPENDENT)
         yc = (log10(Max()) + log10(Min()))/2;
	}

	switch(dir)
	{
	   case(ID_REDUCE_VERT):
         if(mapping() == PLOT_LOG_Y && plot()->axesMode != PLOT_AXIS_BOX_Y_INDEPENDENT)
         {
            tempMin = pow((double)10.0,(double)(yc - (log10(Max()) - log10(Min()) ) * vert_enlarge/2));
            tempMax = pow((double)10.0,(double)(yc + (log10(Max()) - log10(Min()) ) * vert_enlarge/2));
         }
         else
         {
		      tempMin = yc - (Max() - Min()) * vert_enlarge/2;
		      tempMax = yc + (Max() - Min()) * vert_enlarge/2;
         }
		   break; 
	   case(ID_ENLARGE_VERT):
         if(mapping() == PLOT_LOG_Y && plot()->axesMode != PLOT_AXIS_BOX_Y_INDEPENDENT)
         {
            tempMin = pow((double)10.0,(double)(yc - (log10(Max()) - log10(Min()) ) * vert_reduce/2));
            tempMax = pow((double)10.0,(double)(yc + (log10(Max()) - log10(Min()) ) * vert_reduce/2));
         }
         else
         {
		      tempMin = yc - (Max() - Min()) * vert_reduce/2;
		      tempMax = yc + (Max() - Min()) * vert_reduce/2;
         }
		   break; 
	   default:
         error = true;
		   break;
	}

    if(error == false)
    {
       if(mapping() == PLOT_LOG_Y && plot()->axesMode != PLOT_AXIS_BOX_Y_INDEPENDENT)
       {
          if(tempMin <= FLT_MIN/10.0) tempMin = FLT_MIN/10.0;
       }
       if(tempMax >= FLT_MAX/10.0) tempMax = FLT_MAX/10.0;
       setMin((float)tempMin);
       setMax((float)tempMax);
    }
}

void VerticalAxis::shiftPlot(short dir)
{
   double tempMin = numeric_limits<double>::max();
	double tempMax = -numeric_limits<double>::max();
   bool error = false;
	if(this->plotDirection() == PLT_REVERSE)
	{
		if(dir == ID_SHIFT_DOWN)
			dir = ID_SHIFT_UP;
		else if(dir == ID_SHIFT_UP)
			dir = ID_SHIFT_DOWN;
	}

   switch(dir)
   {
      case(ID_SHIFT_DOWN): 
         if(mapping() == PLOT_LOG_Y && plot()->axesMode != PLOT_AXIS_BOX_Y_INDEPENDENT)
         {
            tempMin = pow((double)10.0,(double)(-(log10(Max()) - log10(Min())) * vert_shift + log10(Min())));
            tempMax = pow((double)10.0,(double)(-(log10(Max()) - log10(Min())) * vert_shift + log10(Max())));
         }
         else
         {
            tempMin = -(Max() - Min()) * vert_shift + Min();
            tempMax = -(Max() - Min()) * vert_shift + Max();
         }
         break;
      case(ID_SHIFT_UP): 
         if(mapping() == PLOT_LOG_Y && plot()->axesMode != PLOT_AXIS_BOX_Y_INDEPENDENT)
         {
            tempMin = pow((double)10.0,(double)((log10(Max()) - log10(Min())) * vert_shift + log10(Min())));
            tempMax = pow((double)10.0,(double)((log10(Max()) - log10(Min())) * vert_shift + log10(Max())));
         }
         else
         {
            tempMin = (Max() - Min()) * vert_shift + Min();
            tempMax = (Max() - Min()) * vert_shift + Max();
         }
         break;
	   default:
         error = true;
		   break;
   }	
    if(error == false)
    {
       if(mapping() == PLOT_LOG_Y && plot()->axesMode != PLOT_AXIS_BOX_Y_INDEPENDENT)
       {
         if(tempMin <= FLT_MIN/10.0)tempMin = FLT_MIN/10.0;
       }
       if(tempMax >= FLT_MAX/10.0) tempMax = FLT_MAX/10.0;
       setMin((float)tempMin);
       setMax((float)tempMax);
    }
}

PositionList* VerticalAxis::getLinearTickPositions(TickType tickType)
{
	PositionList* list = new PositionList();

	bool reverse = plot()->identifyReverseVertical(min_, max_, length(), base());

	// get yTick
	short ticksPerLabel = ticks().perLabel();
	float yTick = ticks().spacing();	
	float yLabel = ticksPerLabel*yTick;

	if(yLabel == 0 || yTick == 0)
		return list;    
	// get yTickStart
	float yTickStart = (int)(min_/yTick)*yTick;
	// get nrTicks
	int nrTicks = nint((max_-yTickStart)/yTick);
	
	if(yTickStart < min_) yTickStart += yTick;
	float yLabelStart = (int)(min_/yLabel)*yLabel;
	if(yLabelStart < min_) yLabelStart += yLabel;

	// Correct for rounding problems - tickstart should always be less then labelstart
	if(yTickStart > yLabelStart)
		yTickStart = yLabelStart;
	// get ticksPerLabel (draw thick grid every ticksPerLabel ticks)

	int tickCnt = ticksPerLabel-(int)((yLabelStart-yTickStart)/yTick+0.5); 
	
	for(int i = 0; i <= nrTicks; i++)
	{
		float y = yTickStart + i*yTick;
		
		float yScrn;
		// Convert from axes to screen coordinates
		if(plot()->getDimension() == 1)
			yScrn = dataToScrn(y);  
		else
		{
         if(this->plotDirection() == PLT_REVERSE)

		//	if(reverse)
				yScrn = nint(dim_.top() + (y-min_)*dim_.height()/(float)(max_-min_));
			else
				yScrn = nint(dim_.height() + dim_.top() - (y-min_)*dim_.height()/(float)(max_-min_));
		}
		if(!(tickCnt % ticksPerLabel) && (MAJOR == tickType)) // Draw long tick and label
		{
			list->push_back(yScrn);
		}
		else if ((tickCnt % ticksPerLabel) && (MINOR == tickType))
		{
			list->push_back(yScrn);
		}
		tickCnt++;
	}
	return list;
}

PositionList* VerticalAxis::getLogTickPositions(TickType tickType)
{
	PositionList* list = new PositionList();

	long minPower = floor((log10(min_)));
   long maxPower = floor((log10(max_))+1);
	
	float distbtwnlabels = (float)dim_.height()/(log10(max_)-log10(min_));

	float ya = dim_.top()+dim_.height()+(log10(min_)-minPower)*distbtwnlabels; // Initial tick location

   for(short power = minPower; power <= maxPower; power++)
   {
		float yl = ya - (power-minPower)*distbtwnlabels; // Label position
                    
      for(short tick = 1; tick <= 9; tick++)
      {
			float y = yl - log10((float)tick)*distbtwnlabels;

			if(y < dim_.top() || y > (dim_.top()+dim_.height())) // Ignore ticks and labels outside plot region
				continue;
          
			if((tick == 1) && (MAJOR == tickType)) // Draw y label and long tick
         {	
				list->push_back(nint(y));
			}
			else if ((tick != 1) && (MINOR == tickType))
			{
				list->push_back(nint(y));
			}
		}
	}
	return list;
}

// Figure out best label and tick spacing
float VerticalAxis::tickStart()
{
   if(autoScale())
   {
		float myLabel; 
		if (base() || length()) // ie, 2D
		{
			long visibleHeight = dynamic_cast<Plot2D*>(this->plot())->visibleHeight();
			float matHeight = dynamic_cast<Plot2D*>(this->plot())->matHeight();
			float yAxisHeight = 0;
         yAxisHeight= fabs(length());
			myLabel = getLabelSpacing((double)visibleHeight*yAxisHeight/(float)matHeight);
		}
      else // 1D
		{
			myLabel = getLabelSpacing((double)Max()-(double)Min());
		}
      ticks().setPerLabel(5);
      ticks().setSpacing(myLabel/ticks().perLabel());
		// Annoying workaround to cover data*/min*/max* discrepancy between 1D/2D plots
		float tickStart = plot()->getTickStart(this, &ticks());
		return tickStart;
	}
	return 0;
}

// Figure out best label and tick spacing
float VerticalAxis::independentTickStart()
{
   if(autoScale())
   {
		float myLabel; 
		if (base() || length()) // ie, 2D
		{
			long visibleHeight = dynamic_cast<Plot2D*>(this->plot())->visibleHeight();
			float matHeight = dynamic_cast<Plot2D*>(this->plot())->matHeight();
			float yAxisHeight = 0;
         yAxisHeight= fabs(length());
			myLabel = getLabelSpacing((double)visibleHeight*yAxisHeight/(float)matHeight);
		}
      else // 1D
		{
			myLabel = getLabelSpacing((double)MaxIndep()-(double)MinIndep());
		}
      ticks().setPerLabel(5);
      ticks().setSpacing(myLabel/ticks().perLabel());
		// Annoying workaround to cover data*/min*/max* discrepancy between 1D/2D plots
      float tickStart =  (long) (MinIndep()/ticks().spacing()) * ticks().spacing();
		return tickStart;
	}
	return 0;
}

short VerticalAxis::getTickLabel()
{
	if (!isDrawable() || !autoScale())
	{
		return OK;
	}
   if(plot()->axesMode == PLOT_AXIS_BOX_Y_INDEPENDENT)
   {
	   if (independentTickStart() != 0 && ticks().spacing() == 0)
	   {
		   setMin(Min() - 1);
		   setMax(Max() + 1);
		   getTickLabel();
	   }
   }
   else
   {
	   if (tickStart() != 0 && ticks().spacing() == 0)
	   {
		   setMin(Min() - 1);
		   setMax(Max() + 1);
		   getTickLabel();
	   }
   }
	return OK;
}

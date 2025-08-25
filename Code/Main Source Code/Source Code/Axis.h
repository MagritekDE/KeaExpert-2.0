#ifndef MYAXIS_H
#define MYAXIS_H

#include "Scalable.h"
#include "ShortRect.h"
#include "Ticks.h"
#include "trace.h"
#include "Translator.h"

#include <vector>

class PlotGrid;
class PlotLabel;
class Translator;

typedef std::vector<int> PositionList;

// Plot directions

typedef enum {
	PLT_FORWARD,
	PLT_REVERSE,
	PLT_MIXED,
	PLT_UNKNOWN
} PlotDirection;


// 1D Plot axes scaling - don't change these, they are used in the plot files

#define PLOT_LINEAR_X 234
#define PLOT_LINEAR_Y 410
#define PLOT_LOG_X    235
#define PLOT_LOG_Y    411

typedef enum {
	HORIZONTAL_AXIS,
	VERTICAL_AXIS
} AxisOrientation;

class Axis : public Observer, public Scalable
{
public:

	// Factory method. Use this to create Axises.
	static Axis* makeAxis(AxisOrientation orientation, short mapping, Plot* parent, VerticalAxisSide side = LEFT_VAXIS_SIDE);

	virtual ~Axis();
	Axis(const Axis& copyMe);
	virtual Axis* clone() const = 0;

	// Observed things call this method to notify an Axis that something it cares about has changed.
	virtual void notify(Observable* o);

	// Getters
	float base() {return base_;}
	float length() {return length_;}
	ShortRect& rect() {return rect_;}
	const char* mapping_s() {return (mapping_ == PLOT_LINEAR_X || mapping_ == PLOT_LINEAR_Y) ? "lin" : "log";}
	short mapping() {return mapping_;}
	short origMapping() {return origMapping_;}
	void setOrigMapping(short mapping) {origMapping_ = mapping;}
	PlotGrid* grid() {return grid_;}
	bool autoRange() {return autoRange_;}
	bool ppmScale() {return ppmScale_;}
	bool autoScale() {return autoScale_;}
	Ticks& ticks() {return ticks_;}
	Translator* translator() {return translator_;}
	Plot* plot() {return plot_;}
	float Min() {return min_;}
	float Max() {return max_;}
	float MinIndepOrig() {return minIndepOrig_;}
	float MinIndep() {return minIndep_;}
	float MaxIndep() {return maxIndep_;}
   short lineWidth() {return lineWidth_;}
	PlotDimensions* plotDimensions() {return &dim_;}
	PlotDirection plotDirection() {return plotDirection_;}
	PlotLabel& label() {return *label_;}

	PositionList* getTickPositions(TickType tickType);
	
	// Setters
	void setTranslator(Translator* t);
	void setTicks(Ticks& ticks) {ticks_ = ticks;}
	void setBase(float base) {base_ = base;}
	void setDirection(PlotDirection direction) {plotDirection_ = direction;}
	void setRect(ShortRect& rect) {rect_ = rect;}
	void setMapping(short mapping);
   void setLineWidth(short lineWidth) {lineWidth_ = lineWidth;}
	void setLength(float length) {length_ = length;}
	void setGrid(PlotGrid* grid);
	void setParent(Plot* parent);
	void setAutorange(bool autorange) {autoRange_ = autorange;}
	void setPPMScale(bool ppmScale) {ppmScale_ = ppmScale;}
	void setMin(float Min) {min_ = Min;}
	void setMax(float Max) {max_ = Max;}
	void setMinIndepOrig(float Min) {minIndepOrig_ = Min;}
	void setMinIndep(float Min) {minIndep_ = Min;}
	void setMaxIndep(float Max) {maxIndep_ = Max;}
	void setLabel(PlotLabel* label);
	void setAutoscale(bool autoscale) {autoScale_ = autoscale;}
	virtual void initialiseRange() = 0;

	virtual void zoom(int direction, float relative_width, float relative_height) = 0;
	virtual void zoomCentred(int direction) = 0;
	virtual void shiftPlot(short dir) = 0;

	// Do we bother drawing this axis right now?
	bool isDrawable();
	// Draw this axis
	void draw(HDC hdc);
	void drawIndependent(HDC hdc);
	virtual VerticalAxisSide side() {return NO_VAXIS_SIDE;}

	// Proxy through to this axis' translator.
	long dataToScrn(float val, int off) {return translator_->dataToScrn(val, off);}
	float dataToScrnF(float val, int off) {return translator_->dataToScrnF(val, off);}
	long dataToScrn(float val) {return translator_->dataToScrn(val);}
	float scrnToData(long val) {return translator_->scrnToData(val);}
	float dataToUser(long val)  {return translator_->dataToUser(val);}
	long scrnToIndex(float val) {return translator_->scrnToIndex(val);}
	float scrnToUser(long val) {return translator_->scrnToUser(val);}
	long userToData(float val) {return translator_->userToData(val);}

	// Become an axis for the specified trace.
	virtual void addTrace(Trace* t);
	// Cease being an axis for the specified trace.
	virtual void removeTrace(Trace* t);
	// Update the min and max of this axis, based on the traces it has
	PlotDirection updateExtrema(float minX, float maxX);	
	virtual short getTickLabel() = 0;
	// Remove all traces and reset this axis.
	virtual void clear();
	// Determine whether the axis can currently display the supplied trace.
	bool canDisplay(Trace* t);
	static float getLabelSpacing(double width);

	void saveMinMax() {oldMin_ = Min(); oldMax_ = Max();}

	short save(FILE* fp, int version);
	bool hasData();

	void resetTextCoords();

protected:
	Axis(Plot* parent, COLORREF color, float base, float length);
	void setPenColors();
	void setTextColor(HDC hdc);
	bool isPrintingBlackAndWhite();
	void undoEnlargement();
	virtual float tickStart() = 0;

	std::deque<Trace*> traceList;
	PlotDirection plotDirection_;

	float min_;
	float max_;
	float minIndepOrig_;
	float minIndep_;
	float maxIndep_;
	bool autoRange_;
	HPEN axesPen_;
	PlotDimensions dim_;
	PlotLabel* label_;
	
	static const float horiz_enlarge;
	static const float vert_enlarge; 
	static const float horiz_reduce; 
	static const float vert_reduce;  

	static const float horiz_shift;
	static const float vert_shift;

private:
	Plot* plot_;
	PlotGrid* grid_;
	float base_;
	float length_;
	ShortRect rect_;
	short mapping_;
	short origMapping_;
	Ticks ticks_;
	Translator* translator_;
	bool autoScale_;
   bool ppmScale_;

   short lineWidth_;

	void scale_(double x, double y);
	void unscale_();

	virtual void drawLinear(HDC hdc) = 0;
	virtual void drawLog(HDC hdc) = 0;
	virtual void drawIndependentLinear(HDC hdc) = 0;
	virtual void drawIndependentLog(HDC hdc) = 0;
	virtual PlotDirection calculateDataRange(float minX, float maxX) = 0;

	virtual PositionList* getLinearTickPositions(TickType tickType) = 0;
	virtual PositionList* getLogTickPositions(TickType tickType) = 0;
	
	void deleteAllPens();

	float oldMin_;
	float oldMax_;
};


class HorizontalAxis : public Axis
{
public:
	HorizontalAxis(const HorizontalAxis& copyMe);
	HorizontalAxis* clone() const;
	~HorizontalAxis();
	void initialiseRange();
	static Axis* createAxis(short mapping, Plot* parent);

	void zoom(int direction, float relative_width, float relative_height);
	void zoomCentred(int direction);
	void shiftPlot(short dir);

protected:
	float tickStart();

private:
	void drawLinear(HDC hdc);
	void drawLog(HDC hdc);
   void drawIndependentLinear(HDC hdc);
	void drawIndependentLog(HDC hdc);
	PlotDirection calculateDataRange(float minX, float maxX);
	short getTickLabel();
	HorizontalAxis(Plot* parent, COLORREF color, float base, float length);
	void clear();

	void drawTopTick(HDC hdc, int xScrn, float position);
	void drawBottomTick(HDC hdc, int xScrn, float position);
	void drawCross(HDC hdc, int xScrn, int yScrn, float length);

	PositionList* getLinearTickPositions(TickType tickType);
	PositionList* getLogTickPositions(TickType tickType);

};

class VerticalAxis : public Axis
{
public:
	VerticalAxis(const VerticalAxis& copyMe);
	VerticalAxis* clone() const;
	~VerticalAxis();
	void initialiseRange();
	static Axis* createAxis(short mapping, Plot* parent, VerticalAxisSide side = LEFT_VAXIS_SIDE);

	void zoom(int direction, float relative_width, float relative_height);
	void zoomCentred(int direction);
	void shiftPlot(short dir);
	PositionList* getMajorTickPositions();
	PositionList* getMinorTickPositions();

	void addTrace(Trace* t);
	void removeTrace(Trace* t);

	VerticalAxisSide side() {return side_;}

protected:
	float tickStart();
   float independentTickStart();
private:
	void drawLinear(HDC hdc);
	void drawLog(HDC hdc);	
   void drawIndependentLinear(HDC hdc);
	void drawIndependentLog(HDC hdc);
	PlotDirection calculateDataRange(float minX, float maxX);
	short getTickLabel();
	VerticalAxis(Plot* parent, COLORREF color, float base, float length, VerticalAxisSide side = LEFT_VAXIS_SIDE);
	void clear();
	
	PositionList* getLinearTickPositions(TickType tickType);
	PositionList* getLogTickPositions(TickType tickType);

	void drawLeftHandTick(HDC hdc, int yScrn, float length);
	void drawRightHandTick(HDC hdc, int yScrn, float length);
	void drawCross(HDC hdc, int xScrn, int yScrn, float length);
	VerticalAxisSide side_;
};

#endif // ifndef MYAXIS_H
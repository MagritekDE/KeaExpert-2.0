/************************************************************

trace.h

Declarations for traces.

These classes represent 1D Prospa traces.

************************************************************/

#ifndef DATA_H
#define DATA_H

#include "defines.h"
#include "TracePar.h"
#include <string>
#include "Scalable.h"
#include "variablesClass.h"

#define NO_TRACE_ID_SET -1

// Information stored for each x-y data set
class Plot1D;
class Interface;
class CArg;
class ProspaFont;
class TracePar;
class Variable;
class Axis;
//class Scalable;
/*
Abstract trace class.
*/

class Trace : public Scalable
{
public:
	// Constructors/destructors
	Trace(Plot1D* parent);
	Trace(const Trace&);
	Trace(float *x,long size, char *name, TracePar* tp, Plot1D* parent);
	// Use "clone" to make a deep copy of a Trace.
	virtual Trace* clone() const = 0;
	virtual ~Trace();

	// Generate a string representing the state of this object's parameter-
	//  accessible attributes.
	std::string FormatState();
	// Does this trace decrease as its index increases?
	bool isReverseTrace();
	// Get the data at the specified index. Avoid using these outside of this
	// class hierarchy.
	virtual float YReal(long index) = 0;
	virtual float YImag(long index) {return 0;}
	float X(long index) {return x[index];}

	// State getters.
	short getID() const {return ID;}
	long getSize() const {return size;}
	float getMinX() const {return minx;}
	float getMinY() const {return miny;}
	float getMaxX() const {return maxx;}
	float getMaxY() const {return maxy;}
	const char* const getName() const {return name;}
	VerticalAxisSide getSide() const {return side_;}

	bool appearsInLegend() const {return tracePar.inLegend();}

	Plot1D* getParent() {return parent;} // Do not use this. It should be phased out.
		// Is the x dimension defined for this trace?
	bool HasX() {return (0 != x);}
	// Is the y dimension defined for this trace?
	virtual bool HasY() = 0;
	// Are error bars defined for this trace?
	bool HasBars() {return (0 != bars);}

	// Is there an imaginary component to this trace?
	virtual bool hasImaginary() {return false;}

	// State setters.
	void setSize(long size) {this->size = size;}
	void setMinX(float minX) {this->minx = minX;}
	void setMaxX(float maxX) {this->maxx = maxX;}
	void setMinY(float minY) {this->miny = minY;}
	void setMaxY(float maxY) {this->maxy = maxY;}
	void setName(char* name) {strncpy_s(this->name,MAX_STR,name,_TRUNCATE);} // Does not take ownership of pointer.
	void setBars(float** bars) {this->bars = bars;} // Takes ownership of pointer.

	void setParent(Plot1D* parent) {this->parent = parent;}
	void setID(short ID) {this->ID = ID;}

	void setXAxis(Axis* axis) {this->xAxis_ = axis;}
	void setYAxis(Axis* axis) {this->yAxis_ = axis;}
	void setAppearsInLegend(bool appears) {tracePar.setInLegend(appears);}

	void setSide(VerticalAxisSide side) {this->side_ = side;}


	// Do this trace and its peers cause a redraw of their plot when changed?
	bool isUpdatePlots();

	// Produce a new Variable for a dimension's data. 
	virtual Variable* xComponentAsVariable();
	virtual Variable* xMinMaxAsVariable();
	virtual Variable* yComponentAsVariable() = 0;
	virtual Variable* yMinMaxAsVariable() = 0;

	Variable* barsAsVariable();
	
	// Are all y-values greater than zero?
	virtual bool greaterThanZero(char dir) = 0;
	// Get the max and min X values of this trace.
	virtual void DiscoverXRange(float* lhs = 0, float* rhs = 0) = 0;
	// Set the max and min Y values of this trace.
	virtual void DiscoverYRange(float minx, float maxx) = 0;
	virtual void GetXRange() = 0;
	virtual void GetYRange(float,float) = 0;
	
	// Given screen coordinates, returns the index of the closest x,y point.
	virtual long indexOfClosestPoint(long x, long y) = 0;

	// Get the name of the type of data in this trace as a string.
	virtual std::string& typeString() const = 0;

	// Plot the data in this trace.
	void plot(HDC, long xL, long xR, long yT, long yB);
	virtual void PlotErrorBars(HDC hdc, int scale);
	virtual void PlotDataSymbols(HDC hdc, HBRUSH bkgBrush, HPEN bkgPen, 
		long szX, long szY, long xL, long yT, long xR, long yB) = 0;
	
	virtual void drawSegmentTick(HDC hdc, float scale, POINT* point1, POINT* point2);
	virtual void drawSegment(HDC hdc, POINT* point1, POINT* point2);
	void drawName(HDC hdc, ProspaFont& font, POINT* point);

	// Draw a cursor at the specified screen coordinates.
	// TODO: Move to Plot class.
	virtual long drawCursor(HDC hdc, long x, long y) = 0;

	// TODO: What is this?
	virtual void CopyTo1DVarDlgProc(char* varXName, char* varYName) = 0;

	// File stuff. TODO: Move elsewhere.
	short readData(FILE* fp);
	short writeData(FILE* fp);
	short readX(FILE* fp);
	short readBars(FILE* fp);
	short writeBars(FILE* fp);
	virtual short readY(FILE* fp) = 0;
	virtual short writeY(FILE* fp) = 0;
	
	short save(FILE* fp, int version);

	// Display all traces in the plot.
	void DisplayAll(void);
	virtual Trace* CursorOnDataLine(short x, short y, long* indx, float* dist) = 0;

	bool moveTraceToAxis(VerticalAxisSide side);

// Class variables
	long validationCode;
   TracePar tracePar;     // Trace parameters
   Variable varList;      // List of user defined variables associated with trace

	Axis* xAxis() {return xAxis_;}
	Axis* yAxis() {return yAxis_;}

   void setIgnoreXRange(bool value) {ignoreXRange = value;}     // Set whether to use this trace when working out plot x range.
	void setIgnoreYRange(bool value) {ignoreYRange = value;}     // Set whether to use this trace when working out plot y range.

   bool getIgnoreXRange(void) {return(ignoreXRange);}     // Get whether to use this trace when working out plot x range.
	bool getIgnoreYRange(void) {return(ignoreYRange);}     // Get whether to use this trace when working out plot y range.

	bool isPrinting() const;

	// Removes (x,y) points that are drawn over by other points, or by lines between other pairs of points. 
	//   Returns the size of the filtered list that has been put into filteredX, filteredY.
	long DataToScrnCoords(const float* const x, const float* const y, long size, float* const filteredX, float* const filteredY) const;
	long DataToScrnCoordsF(const float* const x, const float* const y, long size, float* const filteredX, float* const filteredY) const;
	long FilterOutRedundantPoints(const float* const x, const float* const y, long size, float* const filteredX, float* const filteredY) const;
	long FilterOutRedundantPointsF(const float* const x, const float* const y, long size, float* const filteredX, float* const filteredY) const;

protected:
	float* x;
   float **bars;          // Size of error bars above and below point y
   long  size;            // Number of data points in arrays
	float* filteredX;
	float* filteredY;
	Plot1D* parent; // Parent class
	Axis* xAxis_;
	Axis* yAxis_;


	short lineWidth(bool ignoreAntialias = false) const;
	HPEN makeRealPen(bool ignoreAntialias = false) const;
	HPEN makeImagPen(bool ignoreAntialias = false) const;

private:
	short ID;
   float minx,maxx;       // Data range for x
   float miny,maxy;       // Data range for y
   char  name[MAX_STR];   // Name of data set
   char  path[MAX_STR];   // Directory of data set
   bool ignoreXRange;
   bool ignoreYRange;

	VerticalAxisSide side_;  

	virtual void PlotStairs(HDC hdc, long xL, long xR, long yT, long yB) const = 0;
	virtual void PlotPoints(HDC hdc, long xL, long xR, long yT, long yB) const = 0;
	virtual void PlotLinesPrint(HDC hdc, long xL, long xR, long yT, long yB) = 0;
	virtual void PlotLines(HDC hdc, long xL, long xR, long yT, long yB) = 0;
	virtual void PlotAllLines(HDC hdc, long xL, long xR, long yT, long yB) = 0;

	virtual void scale_(double x, double y) = 0;
	virtual void unscale_() = 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
Real trace class.
*/

class TraceReal : public Trace
{
public:
	TraceReal(Plot1D* parent);
	TraceReal(float *x, float *y, long size, char *name, TracePar* tp, Plot1D* parent, bool append);
	TraceReal(const TraceReal& copyMe);
	Trace* clone() const;
	~TraceReal();

	// Inherited virtual functions
	bool greaterThanZero(char dir);
	void DiscoverXRange(float* lhs = 0, float* rhs = 0);
	void DiscoverYRange(float minx, float maxx);
	void GetXRange();
	void GetYRange(float,float);
	std::string& typeString() const;

	Variable* yComponentAsVariable();
	Variable* yMinMaxAsVariable();
	
	long indexOfClosestPoint(long x, long y);

	void PlotErrorBars(HDC hdc, int scale);
	void PlotDataSymbols(HDC hdc, HBRUSH bkgBrush, HPEN bkgPen, 
		long szX, long szY, long xL, long yT, long xR, long yB);

	float YReal(long index) {return y[index];}
	bool HasY() {return (0 != y);}
	short readY(FILE* fp);
	short writeY(FILE* fp);
	long drawCursor(HDC hdc, long x, long y);

	void CopyTo1DVarDlgProc(char* varXName, char* varYName);
	Trace* CursorOnDataLine(short x, short y, long* indx, float* dist);
	void setData(float* xData, float* yData, long size);

private:
	float *y;              // Pointer to y array
	void PlotStairs(HDC hdc, long xL, long xR, long yT, long yB) const;
	void PlotPoints(HDC hdc, long xL, long xR, long yT, long yB) const; 
	void PlotLines(HDC hdc, long xL, long xR, long yT, long yB);
	void PlotAllLines(HDC hdc, long xL, long xR, long yT, long yB);
	void PlotLinesPrint(HDC hdc, long xL, long xR, long yT, long yB);
	void scale_(double x, double y);
	void unscale_();
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
Complex trace class.
*/

class TraceComplex : public Trace
{
public:
	TraceComplex(Plot1D* parent);
	TraceComplex(float *x, complex *yc, long size, char *name, TracePar* tp, Plot1D* parent, bool append);
	TraceComplex(const TraceComplex& copyMe);
	Trace* clone() const;
	~TraceComplex();

	// Inherited virtual functions
	bool greaterThanZero(char dir);
	void DiscoverXRange(float* lhs = 0, float* rhs = 0);
	void DiscoverYRange(float minx, float maxx);
	void GetXRange();
	void GetYRange(float,float);
	std::string& typeString() const;

	Variable* yComponentAsVariable();
	Variable* yMinMaxAsVariable();

	long indexOfClosestPoint(long x, long y);

	void PlotDataSymbols(HDC hdc, HBRUSH bkgBrush, HPEN bkgPen, 
		long szX, long szY, long xL, long yT, long xR, long yB);

	float YReal(long index) {return yc[index].r;}
	float YImag(long index) {return yc[index].i;}
	bool HasY()	{return (0 != yc);}
	bool hasImaginary() {return true;}
	short readY(FILE* fp);
	short writeY(FILE* fp);
	long drawCursor(HDC hdc, long x, long y);
	
	void CopyTo1DVarDlgProc(char* varXName, char* varYName);
	Trace* CursorOnDataLine(short x, short y, long* indx, float* dist);

	void drawSegment(HDC hdc, POINT* point1, POINT* point2);
	void drawSegmentTick(HDC hdc, float scale, POINT* point1, POINT* point2);
	void setData(float* xData, complex* yData, long size);

private:
	complex *yc;           // Pointer to complex array
	void PlotStairs(HDC hdc, long xL, long xR, long yT, long yB) const;
	void PlotPoints(HDC hdc, long xL, long xR, long yT, long yB) const; 
	void PlotLines(HDC hdc, long xL, long xR, long yT, long yB);
	void PlotAllLines(HDC hdc, long xL, long xR, long yT, long yB);
	void PlotLinesPrint(HDC hdc, long xL, long xR, long yT, long yB);
	void scale_(double x, double y);
	void unscale_();
};


#endif // #define DATA_H
#ifndef PLOT_H
#define PLOT_H

#include "TracePar.h"
#include "Axis.h"
#include "globals.h"
#include "Observer.h"
#include "PlotDimensions.h"
#include "PlotLabel.h"
#include "Scalable.h"
#include "ShortRect.h"
#include "Translator.h"
#include <deque>
#include <vector>

class Inset;
class PlotLine;
class PlotText;
class Trace;
class Interface;
class PlotFile1D;
class PlotFile2D;
class PlotLegend;
class PlotWinDefaults;
class PlotWindow;
class StringPairs;
class Plot;
class Ticks;
class Variable;

typedef std::deque<Trace*> TraceList;
typedef std::deque<Trace*>::iterator TraceListIterator;
typedef std::deque<Trace*>::reverse_iterator TraceListReverseIterator;
typedef std::deque<Trace*>::const_iterator TraceListConstIterator;

typedef std::deque<Axis*> AxisList;
typedef std::deque<Axis*>::iterator AxisListIterator;

#define PLOT_STR_LEN 200
#define AXIS_PARAM_LENGTH 50

#define MINIMUM_TRACES_BEFORE_DISPLAYING_LEGEND 2

#define DEFAULT_CONTOUR_LEVELS 11

# define LINEAR_MAPPING 1
# define LOG_MAPPING    2

#define SHOW_REAL      1
#define SHOW_IMAGINARY 2
#define SHOW_MAGNITUDE 4

// Color-map numbers - don't change these, they are used in the plot files

#define COLORMAP_RAINBOW    650
#define COLORMAP_HOT        651
#define COLORMAP_COOL       652
#define COLORMAP_GREY       653
#define COLORMAP_BLACK      654
#define COLORMAP_PLUS_MINUS 655

// Plot axes types - don't change these, they are used in the plot files

#define PLOT_AXES_CORNER            303
#define PLOT_AXES_BOX               304
#define PLOT_AXES_CROSS             305
#define PLOT_X_AXIS                 306
#define PLOT_X_AXIS_BOX             307
#define PLOT_X_AXIS_CROSS           308
#define PLOT_Y_AXIS                 309
#define PLOT_Y_AXIS_BOX             310
#define PLOT_Y_AXIS_CROSS           311
#define PLOT_NO_AXES                312
#define PLOT_AXIS_BOX_Y_INDEPENDENT 313

#define TITLE_TEXT   1
#define X_LABEL      2
#define Y_LABEL_L    3
#define X_AXES_TEXT  4
#define Y_AXES_TEXT_L  5
#define X_AXES_TICKS   6
#define Y_AXES_TICKS   7
#define Y_LABEL_R      8
#define Y_AXES_TEXT_R  9

typedef struct
{
	int xScrn;
	int xIndex;
	float xData;
	int yScrn;
	float yData;
}
CursorSelection;

enum {SCALE,DATA};

class Margins : public Scalable
{
public:
	Margins();
	Margins(long left, long right, long top, long base);
	Margins(const Margins&);
	~Margins();

	// getters
	long right() {return rightMargin;}
	long left() {return leftMargin;}
	long top() {return topMargin;}
	long base() {return baseMargin;}

	long totalHorizontalMargins() {return rightMargin + leftMargin;}
	long totalVerticalMargins() {return topMargin + baseMargin;}

private:
	long leftMargin;
	long rightMargin;     // Fixed sized regions around plot
	long topMargin;
	long baseMargin;

	void scale_(double x, double y);
	void unscale_();
};



/********************************
To be the model component of 
plots.
********************************/
class Plot : public Observable, public Scalable
{
public:
   
	// Creating and destroying Plots.

	virtual ~Plot(void);
	Plot(PlotWinDefaults*,HWND,PlotWindow*,short,short); // Constructor
	Plot(PlotWinDefaults*,HWND,PlotWindow*,short,short,Margins&); // Constructor
	virtual Plot* clone() const = 0;

	// Getting permanent properties of the plot
	virtual short getDimension() = 0; // (1 or 2)

	// Drawing-related functions
	virtual void DisplayData(HDC hdc, long x, long y, bool shiftDown) = 0;
	virtual void HideDataCursor(HDC hdc) = 0;
	virtual void ShiftPlot(short dir) = 0;
   virtual void EnlargePlotCentred(short) = 0;
	virtual void DisplayAll(bool locked) = 0;
	virtual void showLegend(bool show) {return;}
	virtual bool showLegend() {return false;}
	virtual bool legendIsVisible() {return false;}
	void ResizePlotRegion(HWND);
	
	void  ResetSelectionRectangle(void);
	virtual short SelectRegion(HWND, short, short);
   virtual short GetDataAtCursor(HWND hWnd, short xs0, short ys0);
	virtual void HideSelectionRectangle(HDC);
	virtual short ZoomRegion(void);
	void Invalidate();	
	bool InRegion(short, short);	
	virtual bool lastRegion(HWND hWnd) = 0;

	virtual void resetDataView();
	Axis* otherYAxis(Axis* axis = 0);
   void MovingPlotByMouse(void);
   void MovingPlotByCmd(void);
   void ScalingPlotByCmd(void);
   void ScalingPlotByMouseWheel(void);
   
	/************************************************************************
	/* The user has begun to drag this plot.          
	/* @param hWnd window handle
	/* @param x the x value at which the drag begins
	/* @param y the y value at which the drag begins
	/************************************************************************/
	void beginToDragMovePlot(HWND hWnd, short x, short y);

	// Getting dynamic properties of the Plot
	virtual bool DataPresent(void) const = 0;
	virtual short GetData(Interface *itfc, short mode) = 0; 
	long  GetLeft(void);
	long  GetTop(void);
	long  GetRight(void);
	long  GetHeight(void);
	long  GetWidth(void);
	long  GetBottom(void);
	void  SetLeft(long);
	void  SetTop(long);
	void  SetHeight(long);
	void  SetWidth(long);
	virtual bool isAntiAliasing(void);
	char* getPlotState();
	void GetAxisParameter(short parameter, char *value);
	short CursorInBackGround(short,short);
	short CursorInText(short,short);
	short CursorOnAxes(short,short);     
	bool  InPlotRect(long,long);
	bool isUpdatePlots();
	bool getHold();
	char* getFileName();
	char* getFilePath();
	long getFileVersion();
	void setFileVersion(long);
   float traceXBorderFactor;
   float traceYBorderFactor;
   bool GetXCalibration() {return xCalibrated_;}
   bool GetYCalibration() {return yCalibrated_;}
   void SetXCalibration(bool calibrated) {xCalibrated_ = calibrated;}
   void SetYCalibration(bool calibrated) {yCalibrated_ = calibrated;}

	// Labels
	PlotLabel& title() {return *title_;}
		
	// Setting dynamic properties of the plot
	virtual void makeCurrentPlot();
	virtual void makeCurrentDimensionalPlot() = 0;
	virtual void OffsetData(HDC hdc, long x, long y) = 0;
	virtual void ScalePlot(HWND hWnd, short dir) = 0;
	virtual void displayToEMF(HDC hdc, HWND hWnd) = 0;
	virtual long Display(HWND, HDC) = 0;
	virtual void clearData(void) = 0;
   virtual void initialiseMenuChecks(const char *mode) = 0;
	void  SetClippingRegion(HDC,long);
	void  SetMappingMode(HDC hdc);
	virtual void setAntiAliasing(bool);
	void setHold(bool hold);
	void setPlotState(char* state);
	void clearLabels();
	void ResetPlotParameters(void);
	void SetPlotColor(COLORREF col, short dest);
	void setFileName(char* fileName);
	void setFilePath(char* filePath);
	void setTitle(PlotLabel* title);
	void setTitleText(char* title);

	void lockGrid(bool lock) {gridLocked_ = (lock ? curYAxis_ : 0);}
	Axis* gridLocked() {return gridLocked_;}
	bool gridLocked(Axis* axis) {return gridLocked_ == axis;}

	virtual short setAxisMapping(short mapping);

	Margins& getMargins() {return margins;}
	void setMargins(Margins& margins) {this->margins = margins;}
	void setXTicks(Ticks& ticks);
	void setYTicks(Ticks& ticks);
	Ticks& getXTicks();
	Ticks& getYTicks();

// Loading and saving
	virtual short save(FILE* fp) = 0;

// CLI methods  *************************
	virtual short Zoom(Interface* itfc, char args[]) = 0;
	void resetZoomCount();

	short ProcessLabelParameters(Interface *itfc, char *which, char *args);
	int ProcessAxesParameters(Interface* itfc, char args[]);
	int ProcessGridParameters(Interface* itfc, char args[]);

	void updateStatusWindowForZoom(HWND hWnd);

	PlotDimensions& getDimensions() {return dimensions;}
	virtual COLORREF ChoosePlotColor(HWND hWnd, short dest, bool bkgColorEnabled) = 0;
	virtual bool displayColorScale() { return false; }
	virtual void PasteInto(Plot* destination) {return;}


	/**
	*	Set some colours for this Plot.
	*	@param axes the new color for the axes
	*	@param bkgd the new background color for the plot
	*	@param plot the new color to plot with
	*  @param border the new border color
	*/
	void setColors(COLORREF& axes, COLORREF& bkgd, COLORREF& plot, COLORREF& border);

	/**
	* Call to indicate that the plot is no longer being shifted (by MOVE_PLOT).
	*/
	void stopShifting();

// Generic plot methods

	/////////////////
	// Data members.
	/////////////////
	long validationCode;     // Whether this is a valid plot reference or not

	// Axis-related members.
	short axesMode;        // Type of axes: CORNERAXES, BORDERAXES, CENTERAXES

	bool updatePlots() const { return updatePlots_; }
   void updatePlots(bool val) { updatePlots_ = val; }
	bool updatingPlots() const { return updatingPlots_; }
	void updatingPlots(bool val) { updatingPlots_ = val; }

	static bool gApplyToAll; // Apply font selections to all windows?

	bool yLabelVert;       // Is y label to be vertical
   
	char statusText[PLOT_STR_LEN]; // Text for status box
	COLORREF axesColor;       
	COLORREF bkColor; // Background color
	COLORREF plotColor;
	COLORREF borderColor;

	// Belong in the PlotWindow class; the plot should not care
	//  about row/col.
	short rowNr;  // Where the plot is in relation to others
	short colNr;

	long regionWidth() {return regionRight - regionLeft;}
	long regWidth() {return regionRight - regionLeft;}
	long regHeight() {return regionBottom - regionTop;}
	long regTop() {return regionTop;}
	long regLeft() {return regionLeft;}

	// View -- move to view component
	HWND  win;         // Parent window

	bool displayHold; // Allows overlapping display

// 2D specific variables

	FloatRect selectRect; // Selected rectangle (data coordinates)
	bool rectSelected;  // Has a rectange been selected?

   bool plotShifted; // Whether the plot has been shifted using the arrow keys
   int shiftDirection; // Which direction has the plot been moved
   bool plotCmdScaled; // Whether the plot has been scaled using the arrow keys
   bool plotMouseScaled; // Whether the plot has been scaled using the arrow keys
   int scaleDirection; // Has the plot been enlarged LR/UD or reduced LR/UD
	   
	static char* GetColorStr(COLORREF col); 
   static char* Plot::GetAlphaColorStr(COLORREF col);

	const std::string FormatXLabelParameters();
	const std::string FormatYLabelParameters();
	const std::string FormatYLabelRightParameters();
	const std::string FormatYLabelLeftParameters();

	static Plot* curPlot();
	static void setNoCurPlot();
	static bool zoomPoint;
	static float gZoomX,gZoomY; // Zoom coordinates

	static bool g_chooseBkgColor;
	static bool g_bkgColorEnabled;
	PlotWindow *plotParent;



	/**
	*	Destroy and unlink the current X axis for this plot. Use the supplied axis instead.
	*	@param axis the new axis to use. This Plot takes over this memory.
	*/
	virtual void setXAxis(Axis* axis);
	/**
	*	Destroy and unlink the current left Y axis for this plot. Use the supplied axis instead.
	*	@param axis the new axis to use. This Plot takes over this memory.
	*/
	virtual void setYAxisL(Axis* axis);
	/**
	*	Destroy and unlink the current right Y axis for this plot. Use the supplied axis instead.
	*	@param axis the new axis to use. This Plot takes over this memory.
	*/
	virtual void setYAxisR(Axis* axis);

	Axis* curXAxis() {return curXAxis_;}
	Axis* curYAxis() {return curYAxis_;}

	Axis* yAxisLeft() {return yAxisL_;}
	Axis* yAxisRight() {return yAxisR_;}

	VerticalAxisSide currentVerticalAxis() {return (curYAxis_ == yAxisL_) ? LEFT_VAXIS_SIDE : RIGHT_VAXIS_SIDE;}

	void setCurXAxis(Axis* axis) {curXAxis_ = axis;}
	virtual void setCurYAxis(Axis* axis);
	virtual void setCurYAxis(VerticalAxisSide side);

	void setSyncAxes(bool synch) {syncAxes_ = synch;}
	bool syncAxes() {return syncAxes_;}

	AxisList& axisList() {return axisList_;}

	// Required because of the 2D "data*" vs 1D "min*-max*"
	// thing. It should be removed altogether once I figure out how
	// to unify data* with min*/max*...
	virtual float getTickStart(Axis* axis, Ticks* ticks)  = 0; 	
	static COLORREF ChooseAColor(HWND hWnd, COLORREF init, bool bkgColorEnabled);
	virtual void GetPlotColor(short mode, COLORREF &color) = 0;
	HBRUSH GetPlotBorderBrush();
	virtual HBRUSH GetPlotBackgroundBrush() = 0;
	/**
	* Modify the override autorange parameter         
	*/
   void setOverRideAutoRange(bool state);
   bool getOverRideAutoRange();
   char *GetAxesTypeStr();
   bool SetAxesType(CText type);
	virtual bool identifyReverseHorizontal(float& min, float& max, float length, float base){return false;}
	virtual bool identifyReverseVertical(float& min, float& max, float length, float base){return false;}

	virtual short LoadV3_8Info(FILE *fp) {return(0);}
	virtual short SaveV3_8Info(FILE* fp) {return(0);}

	/**
	* Set default plot parameters.
	*/
	virtual void setDefaultParameters();
	/************************************************************************
	/* Add an Inset to this Plot.
	/* @param inset an Inset to be added to this Plot
	/* @return the index of this Inset within this Plot
	/************************************************************************/
	int addInset(Inset* inset);

	/************************************************************************
	/* Remove an Inset from this Plot.
	/* @param index the index of the Inset to be removed from this Plot.
	/* @return OK on success, ERR otherwise
	/************************************************************************/
	int removeInset(int index); 
	/************************************************************************
	/* Remove all Insets from this Plot.
	/************************************************************************/
   void removeInsets(void);
	/************************************************************************
	/* Remove an Inset from this Plot.
	/* @param removeMe the Inset to be removed from this Plot.
	/************************************************************************/
	void removeInset(Inset* removeMe); // TODO: implement
	/************************************************************************
	/* Get the Insets attached to this Plot.
	/* @return collection of the Insets attached to this Plot.
	/************************************************************************/
	std::deque<Inset*>& getInsets() {return insets_;}
	/************************************************************************
	/* Get the Inset currently being moved in this Plot.
	/* @return pointer to that Inset, or NULL if none is being moved.
	/************************************************************************/
	Inset* insetBeingMoved();
	/************************************************************************
	/* Gets a human-readable string table describing the insets in this Plot.
	/* @return table describes the insets in this Plot. The caller is responsible
	/*   for this memory.
	/************************************************************************/
	std::string* describeInsets();
	/************************************************************************
	/* Returns a pointer to the Inset with the specified index.
	/* @param n the index of the Inset to return.
	/* @return a pointer to Inset at index n, or NULL if there is no such 
	/*   Inset.
	/************************************************************************/
	Inset* findInsetByID(int n);

   void removeLines(void);
   void removeTexts(void);

	virtual short DisplayHistory(FloatRect&) = 0;

	/**
	*	Display a ".nD" file dragged and dropped onto the plot
	*/
	virtual void LoadAndDisplayDataFile(HWND hWnd, char *basePath, char *fileName) = 0;

	/**
	*	Is this plot offset at the moment?
	*	@return true if this plot is now offset, false otherwise.
	*/
	bool isOffset() {return isOffset_;}

   void removeOffset() 
   { 
      xOffset_ = 0;
      xOffsetIndex_ = 0;
      yOffset_ = 0;
      yOffsetIndex_ = 0;
      isOffset_ = false;
   }

   std::deque<PlotLine*> lines_; ///< The lines owned by this Plot.
   std::deque<PlotText*> text_; ///< The text owned by this Plot.

   bool allowMakeCurrentPlot_; // Whether the plot can be made a current plot by clicking

	COLORREF zoomBkgColor;    // Zoom rectangle color
	COLORREF zoomBorderColor; // Zoom rectangle color
   CText zoomRectMode;       // The rectangle zoom mode

   bool plotBeingMoved; // Parameters for interactive plot movement
   long plotOldX;
   long plotOldY;
   long plotNewX;
   long plotNewY;

   CursorSelection pointSelection;

protected:
		
	Plot (const Plot& copyMe);
	void InitialisePositions(HWND hWnd);
	short DrawBoundingRectange(HDC, HPEN);
	virtual void DrawSelectionRectangle(HDC);

	void  WritePlotLabels(HDC);    
	void ResetTextCoords(void);

   POINT dataCursor;

	RECT scrnSelectRect;  // Selected rectangle (screen coordinates)
	bool antiAliasing; 
	PlotDimensions dimensions;
	Margins margins;
	std::vector<FloatRect> zoomHistory; // Zoom history

	bool updatePlots_;
	bool updatingPlots_;

	Axis* xAxis_;
	Axis* yAxisR_;
	Axis* yAxisL_;

	AxisList axisList_;

	Axis* curXAxis_;
	Axis* curYAxis_;
	bool overRideAutoRange;
   
	short mapping(char dir);
	virtual void scale_(double x, double y = 0);
	virtual void unscale_();
	std::deque<Inset*> insets_; ///< The insets owned by this Plot.

	long regionLeft; 
	long regionTop;
	long regionBottom;
	long regionRight;

   bool  isOffset_;     // Are we in offset mode or not
	long  xOffsetScrn_;  // xOffset in screen coordinates
	long  yOffsetScrn_;  // yOffset in screen coordinates
	float xOffset_;      // X offset in plot units
	float yOffset_;      // Y offset in plot units
	long  xOffsetIndex_; // X offset as an array index
	long  yOffsetIndex_; // Y offset as an array index

   bool xCalibrated_; // Whether plot has calibrated axes or not
   bool yCalibrated_; // Whether plot has calibrated axes or not


private:

	void setDimensions(PlotDimensions& dimensions) {this->dimensions = dimensions; this->notifyObservers();}
	virtual void  DrawPlotBackGround(HDC) = 0;
	void SetAxisParameter(short parameter, char *value);
	void FormatAxisLabelParameters(StringPairs& state);
	/**
	*	Get the position of the supplied Inset in the list of insets.
	*	@param inset the inset to search for
	*	@return the position of the inset in the list of insets, or -1 if it is not present in
	*		the list.
	*/
	int indexOfInset(const Inset* const inset) const;

	static Plot* _curPlot;
	char  plotStr[MAX_STR];      // User definable string.
	char  fileName[PLOT_STR_LEN];  // Plot filename
	char  filePath[PLOT_STR_LEN];  // Plot file path
	long  fileVersion;             // Plot file version
	virtual bool OKForLog(char dir) = 0;
	PlotLabel* title_; 
	bool syncAxes_;
	Axis* gridLocked_;
};

extern Margins	defaultMargins1D;
extern  Margins defaultMargins2D;

class Plot1D : public Plot
{
public:
	Plot1D();
	virtual ~Plot1D();
	Plot1D(PlotWinDefaults *pd);
	Plot1D(PlotWinDefaults *pd, Margins &margins);
	Plot1D(PlotWinDefaults *pd, HWND hWnd, PlotWindow *pp, short row, short col);
	Plot1D(PlotWinDefaults *pd, HWND hWnd, PlotWindow *pp, short row, short col, Margins &margins);
	Plot1D(const Plot1D& copyMe); 
	Plot1D* clone() const;
	void clearData(void);
   void initialiseMenuChecks(const char *mode);
	Trace* CursorOnDataLine(short x, short y);
	void DisplayData(HDC hdc, long x, long y, bool shiftDown) {;}

	short GetData(Interface *itfc, short mode);
	PlotLegend* legend() {return legend_;}
	/**
	*	Set the Legend of this Plot.
	*	@param legend the new Legend for this Plot. This Plot takes over this memory.
	*/
	void setLegend(PlotLegend* legend);
	void showLegend(bool show);
	bool showLegend();
	bool legendIsVisible();
	void HideDataCursor(HDC hdc);
	void RemoveDataFromList(Trace* toRemove);
	short getDimension();
	bool isAntiAliasing(void);
	bool DataPresent(void) const;
	long DrawPlot(Interface *itfc, char arg[]);
   void DrawDataCrossHairs(HDC);
	void DrawVerticalSelectionLine(HDC);
	void makeCurrentDimensionalPlot();
	static Plot1D* curPlot();
	static void setNoCurPlot();
	static bool dataCursorVisible();
	void ShiftPlot(short dir);
	void DisplayAll(bool locked);
	void OffsetData(HDC hdc, long x, long y);
	void ScalePlot(HWND hWnd, short); 
	bool isReversePlot();
	bool OKForLog(char dir);
	COLORREF ChoosePlotColor(HWND hWnd, short dest, bool bkgColorEnabled);
	void SetTraceColor(COLORREF traceCol, COLORREF symbolCol, COLORREF barCol);
	void PasteInto(Plot* destination);
	short LoadV3_8Info(FILE *fp);
	short DisplayHistory(FloatRect&);
	short Zoom(Interface* itfc, char args[]);
	void  ResetZoomPoint(void);
	short ZoomRegion(void);
	bool lastRegion(HWND hWnd);

	void currentTraceAsVariables(Variable** xAsVar, Variable** yAsVar);
	void currentTraceMinMaxAsVariables(Variable** xAsVar, Variable** yAsVar);

	short nextTraceID();
	float* GetTraceIDs(int *sz);
   short GetNrTraces() {return(traceList.size());}
	Trace* FindTraceByID(short n);
	bool InDataSet(Trace* dat);
	bool IsFiltered() { return isFiltered; }
	void SetFiltered(bool filtered) {isFiltered = filtered; }

	long Display(HWND,HDC);	
	void SetTraceColor();

	static char currFileName[PLOT_STR_LEN];
	static char currFilePath[PLOT_STR_LEN];
	static float PointToLineDist(float x,float y,float xs1,float ys1,float xs2,float ys2);
	void EnlargePlotCentred(short);
	void UndoEnlargement(void);    
	void displayToEMF(HDC hdc, HWND hWnd);
	void DrawSymbol(HDC,short, HBRUSH, HPEN, HBRUSH, HPEN, long, long, long, long);

	static bool dataCursorVisible_;
	long xold,yold;
	long yoldr,yoldi,yoldm;

	Trace* curTrace();
	Trace* setCurTrace(int index = 0);
	Trace* setCurTrace(Trace* t);
	void setNoCurTrace();
	bool hasNoCurTrace();
	void CloseCurrentData(char* mode);
	Trace* appendTrace(Trace* t);

	short removeTrace(int index);
	short removeTrace(Trace* t);
	short display1DComplex;  // Display real + imaginary

	bool limitfunc;

	void GetPlotColor(short mode, COLORREF &color);
	
	bool moveTraceToAxis(Trace* t, VerticalAxisSide side);
	float getTickStart(Axis* axis, Ticks* ticks); 

	short save(FILE* fp);
	short SaveV3_8Info(FILE* fp);
   short SaveData(char* pathName, char* fileName);

	TracePar* getTracePar(){return &tracePar;}

	HBRUSH GetPlotBackgroundBrush();
	short setAxisMapping(short mapping);

	bool canAddTrace(Trace* t);

	/**
	*	Display a ".1D" file dragged and dropped onto the plot
	*/
	void LoadAndDisplayDataFile(HWND hWnd, char *basePath, char *fileName);

	/**
	*	Destroy and unlink the current X axis for this plot. Use the supplied axis instead.
	*	@param axis the new axis to use. This Plot takes over this memory.
	*/
	void setXAxis(Axis* axis);
	/**
	*	Destroy and unlink the current left Y axis for this plot. Use the supplied axis instead.
	*	@param axis the new axis to use. This Plot takes over this memory.
	*/
	void setYAxisL(Axis* axis);
	/**
	*	Destroy and unlink the current right Y axis for this plot. Use the supplied axis instead.
	*	@param axis the new axis to use. This Plot takes over this memory.
	*/
	void setYAxisR(Axis* axis);
	/**
	* Set default plot parameters.
	*/
	void setDefaultParameters();

	bool showLines;
	bool showText;
	bool showInsets;
	int indicatorSize;

private:
	TracePar tracePar;       // Default trace parameters
	static Plot1D *cur1DPlot; // Pointer to current plot class instance
	static PlotFile1D* plotFile; // Plot file
	static char strold[30];
	void expandPlotForAntialias();
	void SetTraceColorBlackNWhite();
	void assignSupersamplingBitmap(HWND hWnd, HDC hdc);
	TraceList traceList; // List of data to be displayed
	Trace *currentTrace; // Last trace created in this window
	short DrawAxes(HDC, HPEN);
	short symbolScale_;
	void scale_(double x, double y = 0);
	void unscale_();
	void DrawPlotBackGround(HDC);
	void DrawCurrentTraceIndicator(HDC);
	bool isFiltered;
	PlotLegend* legend_;
};

class Plot2D : public Plot
{
public:
	Plot2D();
	virtual ~Plot2D();
	Plot2D(PlotWinDefaults *pd, HWND hWnd, PlotWindow *pp, short row, short col);
	Plot2D(PlotWinDefaults *pd, HWND hWnd, PlotWindow *pp, short row, short col, Margins &margins);
	Plot2D(const Plot2D& copyMe); 
	Plot2D* clone() const;
	void DisplayData(HDC hdc, long x, long y, bool shiftDown);
	short GetData(Interface *itfc, short mode);
	long Display(HWND, HDC);
	long Display2DImageNullBMP(HWND, HDC);
	void HideDataCursor(HDC hdc);
	short getDimension();
	bool DataPresent(void) const;
   void DrawPlotBackGround(HDC);   
	void makeCurrentDimensionalPlot();
	static Plot2D* curPlot();
	static void setNoCurPlot();
	void ScanRow(HWND hWnd, short x, short y, char *name);
	void ScanColumn(HWND hWnd, short x, short y, char *name);
	void HideRowCursor(HDC);
	void HideColumnCursor(HDC);
	void ShiftPlot(short dir);
	void DisplayAll(bool locked);
	static bool dataCursorVisible();
	void OffsetData(HDC hdc, long x, long y);
	void ClearCursors(HDC hdc, MouseMode mouseMode);
	void ScalePlot(HWND hWnd, short dir);
   void EnlargePlotCentred(short dir) {;}
	void displayToEMF(HDC hdc, HWND hWnd);
	void clearData(void);
	void clearColorMap();
	void  ChooseDefaultColours();
	void DrawAxes(HDC hdc);
	void ChooseBlackNWhite();
   void initialiseMenuChecks(const char *mode);
	short DisplayHistory(FloatRect& r);
	short LoadV3_8Info(FILE *fp) {return(0);}
	short SaveV3_8Info(FILE *fp) {return(0);}
	void FullRegion(void);
	short Zoom(Interface* itfc, char args[]);
	COLORREF GetColor(short,float,long);
	void SurfacePlot3D(HDC);
	void VectorPlot(HWND,HDC);
	void ContourPlot(HWND,HDC);
	short ColorScaleType(void);
	void WaterFallPlot(HWND,HDC);
	void ContourPlotIntensity(HWND,HDC);
	void DrawColorScale(HDC,HBITMAP);
	void FindMatrixRange(float&,float&);
	void FindFullMatrixRange(float&,float&);
	void PrintColorScaleA(HDC,HBITMAP);
	void PrintColorScaleB(HDC);
	void GetPlotColor(short mode, COLORREF &color);
   int SetColorMap(Variable *cmap);
   int SetContourColor(Variable *rgb);

	short ZoomRegion(void);
	bool lastRegion(HWND hWnd);
	void resetDataView();

	static char currFileName[PLOT_STR_LEN];
	static char currFilePath[PLOT_STR_LEN];
	short xVectorStep;   // How many x cells to skip over during plot
	short yVectorStep;   // How many y cells to skip over during plot
	short FillBitMapWithColor(HBITMAP hBitMap,COLORREF color);
	short FillBitMapWithImage2(HBITMAP);
	
	long visibleLeft() const {return visibleLeft_;}
	long visibleTop() const {return visibleTop_;}
	long visibleWidth() const {return visibleWidth_;}
	long visibleHeight() const {return visibleHeight_;}

	void setVisibleLeft(long val);
	void setVisibleTop(long val);
	void setVisibleWidth(long val);
	void setVisibleHeight(long val);

	short nrContourLevels;       // Number of contours
	float* contourLevels;        // Contour levels
	float contourLineWidth;      // Contour linewidth

   COLORREF contourColor_;      // Color for fixed contour colors
   COLORREF fixedContourColor() {return contourColor_;}
   void setFixedContourColor(COLORREF col) {contourColor_ =  col;}
   bool useFixedContourColor_;
   void setUseFixedContourColor(bool use) {useFixedContourColor_ = use;}
   bool useFixedContourColor() {return useFixedContourColor_;}

	// Ignore any attempts to set the current axis to anything other than the
	//  left side for 2D plots.
	void setCurYAxis(Axis* axis) {return;}
	void setCurYAxis(VerticalAxisSide side) {return;}
	float getTickStart(Axis* axis, Ticks* ticks); 

	short save(FILE* fp);
   short SaveData(char* pathName, char* fileName);

	float** mat() {return mat_;}
	float*** matRGB() {return matRGB_;}
	complex** cmat() {return cmat_;}
	float** vx() {return vx_;}
	float** vy() {return vy_;}
	float vectorLength() {return vectorLength_;}
	long matWidth() {return matWidth_;}
	long matHeight() {return matHeight_;}

	void setMat(float** mat) {mat_ = mat;}
	void setMatRGB(float*** matRGB) {matRGB_ = matRGB;}
	void setCMat(complex** cmat) {cmat_ = cmat;}
	void setVX(float** vx) {vx_ = vx;}
	void setVY(float** vy) {vy_ = vy;}
	void setVectorLength(float length) {vectorLength_ = length;}
	void setMatWidth(long width) {matWidth_ = width;}
	void setMatHeight(long height) {matHeight_ = height;}
	
	short colorScale() {return colorScale_;}
	float** colorMap() {return colorMap_;}
	long colorMapLength() {return colorMapLength_;}

	void setColorScale(short scale) {colorScale_ = scale;}
	void setColorMap(float** map) {colorMap_ = map;}
	void setColorMapLength(long length) {colorMapLength_ = length;}

	short Draw2DImage(Interface* itfc ,char arg[]);

	COLORREF ChoosePlotColor(HWND hWnd, short dest, bool bkgColorEnabled);
	bool displayColorScale() { return displayColorScale_; }
	void setDisplayColorScale(bool display) {displayColorScale_ = display;}

	short drawMode() {return drawMode_;}
	void setDrawMode(short drawMode) {drawMode_ = drawMode;}

	float minVal() {return minVal_;}
	float maxVal() {return maxVal_;}
	void setMinVal(float minVal) {minVal_ = minVal;}
	void setMaxVal(float maxVal) {maxVal_ = maxVal;}

   float *xAxis;
   float *yAxis;
		
	short SelectRegion(HWND hWnd, short xs0, short ys0);
   short GetDataAtCursor(HWND hWnd, short xs0, short ys0) {return(0);}

	void DrawSelectionRectangle(HDC);
	void HideSelectionRectangle(HDC);

	HBRUSH GetPlotBackgroundBrush();

	bool identifyReverseVertical(float& min, float& max, float length, float base);
	bool identifyReverseHorizontal(float& min, float& max, float length, float base);

	/**
	*	Display a ".2D" file dragged and dropped onto the plot
	*/
	void LoadAndDisplayDataFile(HWND hWnd, char *basePath, char *fileName);


   float alpha() {return alpha_;}
   float beta() {return beta_;}
   float gamma() {return gamma_;}
   void setAlpha(float alpha) {alpha_ = alpha;}
   void setBeta(float beta) {beta_ = beta;}
   void setGamma(float gamma) {gamma_ = gamma;}
	COLORREF wfColor; // Waterfall color
	int dataMapping;
	bool limitfunc;

private:
	short saveParameters(FILE *fp);
	void saveTicks(FILE* fp);
	void saveGridColors(FILE* fp);
	void saveDrawGrid(FILE* fp);
	void saveLabelFontParams(FILE* fp);
	void save2AxesDimensions(FILE* fp);
   void save2AxesDirections(FILE* fp);
	void saveLabels(FILE* fp);
	void savePlotParameters(FILE *fp);
	void save2AxesMappings(FILE* fp);
   void save2Lines(FILE* fp);
   void saveFixedContourColor(FILE *fp);
   void saveAxesInfo(FILE *fp);
	void saveDisplayedExtremes(FILE* fp);
	void saveMinMaxXY(FILE* fp);
	void saveDataMappingInfo(FILE *fp);
   void save2DAxesPPMInfo(FILE* fp);


	short SaveReal(FILE* fp);
	short SaveComplex(FILE* fp);
	short SaveVec2(FILE* fp);
	short SaveNone(FILE* fp);
	
	void mouseZoom(bool in, HWND hWnd);
	static Plot2D *cur2DPlot; // Pointer to current plot class instance
	static PlotFile2D *file;
	static long xold,yold; // Last cursor location
	static bool rowCursorVisible; // Cursor visibility
	static bool colCursorVisible;
	static bool dataCursorVisible_;
	void MakeSquareRegions(HWND);
	short FillBitMapWithImage(HBITMAP);
	short FillBitMapWithRGBImage(HBITMAP);
	bool OKForLog(char dir);

	long visibleLeft_;    // region of data set currently visible
	long visibleTop_;
	long visibleWidth_;
	long visibleHeight_;  
	
	float **mat_;         // 2D data set being displayed (real)
	float ***matRGB_;     // RGB data set being displayed (real)
	complex **cmat_;      // 2D data set being displayed (complex)
	float **vx_,**vy_;     // 2D vector data
	float vectorLength_;  // 2D maximum vector length
	long matWidth_;       // and its size
	long matHeight_;
	float maxVal_,minVal_;    // Matrix range

	bool displayColorScale_; // Whether a color scale is drawn or not

	short colorScale_;    // Which colour scale to use
	float **colorMap_;    // Colormap 
	long colorMapLength_;
	short drawMode_;

   float alpha_;  // Rotation angle for waterfall plot
   float beta_;
   float gamma_;

};

////////////////
// PlotFactories
////////////////

class PlotFactory 
{
public:
	// Pure virtual functions
	virtual Plot* makePlot(PlotWinDefaults *pd, HWND hWnd, PlotWindow *pp, short row, short col) = 0;
};

class Plot1DFactory : public PlotFactory
{
public:
	Plot1D* makePlot(PlotWinDefaults *pd, HWND hWnd, PlotWindow *pp, short row, short col);
};

class Plot2DFactory : public PlotFactory
{
public:
	Plot2D* makePlot(PlotWinDefaults *pd, HWND hWnd, PlotWindow *pp, short row, short col);
};


#endif // define PLOT_H


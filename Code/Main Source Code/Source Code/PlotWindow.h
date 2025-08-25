#ifndef PLOTWINDOW_H
#define PLOTWINDOW_H

#include "ctext.h"
#include "Scalable.h"
#include "globals.h"
#include "PlotFile.h"
#include <deque>
#include <string>
#include <exception>

class PlotWinDefaults;
class Interface;
class ObjectData;
class Plot;
class Plot1D;
class Plot2D;
class PlotFactory;
class Margins;

typedef std::deque<Plot*> PlotList;
typedef std::deque<Plot*>::iterator PlotListIterator;
typedef std::deque<Plot*>::reverse_iterator PlotListReverseIterator;

struct PlotWindowException : public std::exception
{
   std::string s;
   PlotWindowException(std::string ss) : s(ss) {}
   const char* what() const throw() { return s.c_str(); }
};

class PlotWindow
{
public:

	virtual ~PlotWindow(void); // Destructor
	PlotWindow(void); // Constructor
	virtual void InitialisePlot(short nrX, short nrY, PlotWinDefaults* pwd);
	virtual short DefaultProc(Interface *itfc, char *args);
	void DrawPlotTitle(void);
	void MakeMultiPlot(short nrX, short nrY);
	void SetMargins(Margins &margins);
	virtual void DisplayAll(bool locked) = 0;
	void AdjustMultiPlotSize(short nrX, short nrY);
	short RemoveAllButOneSubPlot(short x, short y);
	virtual short SavePlots(char* pathName, char* fileName, long x, long y) = 0;
	short LoadPlots(char *pathName, char* fileName);
	virtual void ProcessScrollWheelEvents(HWND hWnd, short zDel, short fwKeys);

	short DisplayOnePlot(short index = -1);

	virtual void RemoveAllButCurrentPlot(void);
	void ViewFullPlot(void);
	virtual void UpdateStatusBar(void) = 0;
	short ViewSubPlot(short xPos, short yPos);
	virtual int dim() = 0;
	virtual Plot* curPlot();
	virtual void GetCurrentPlot(CText &whichPlot, short &x, short &y, short &index);
	virtual long fileDimensionCode() = 0;
	
	virtual short CopyPlotsToClipboard(bool multiplot) = 0;

	static short setPlotMode(CText& mode);

	bool isBusy(void);
   int getCriticalSectionLevel(void);
   bool inCriticalSection(void);
   void incCriticalSectionLevel(void);
   void decCriticalSectionLevel(void);
	PlotWindow* setBusy(bool);
	PlotWindow* setBusyWithCriticalSection(bool);
	short getCols(void);
	short getRows(void);
	PlotWindow* setCols(short);
	PlotWindow* setRows(short);
	bool isShowLabels(void);
	PlotWindow* setShowLabels(bool showLabels);
	short getPlotInsertMode();
	static void setPlotInsertMode(short mode);
	int setFileNumber(int fileNum);

	bool CopyCurrentPlot();
	bool CopyPlot(Plot *plt);
	Plot* PasteSavedPlot();
	Plot* PasteSavedPlot(Plot *plt);
	bool PastePlot(Plot *plt);
	void PasteSavedPlotInto(void);
	void PasteSavedPlotInto(Plot* plt);

	bool StylePlotTitles();
	bool StylePlotAxisLabels();
	bool StylePlotAxisTicksLabels();

	/**
	*	Display a ".pt2" file 
	*/
	virtual int LoadAndDisplayPlotFile(HWND hWnd, char *filePath, char *fileName) = 0;

	// TODO: These makePlot methods shouldn't be necessary. Further cleanup required wherever they're is invoked -- that code should probably be pulled into here.
	Plot* makePlot(PlotWinDefaults *pd, HWND hWnd, PlotWindow *pp, short row, short col);

	Plot* makeCurrentPlot(int index = 0);
	Plot* makeCurrentDimensionalPlot(int index = 0);
	virtual void PrintDisplay(HDC hdc) = 0;

	virtual void clearPlotData(PlotList& plotList);
	virtual PlotList& getSavedPlotList() = 0;
	
// Set and get useDefaultParameters_ member
   void setUseDefaultParameters(bool mode) {useDefaultParameters_ = mode;}
   bool getUseDefaultParameters() {return useDefaultParameters_;}

	virtual void ClearCurrentPlot() {return;}
	virtual char* plotDirectory() = 0;
	
	/**
	*	Save the contents of the plot window as an enhanced meta file
	*	@param inputFileName the name of the file to save to; a dialog will open if this is null
	*	@return OK if the operation succeeded
	*/
	short SaveAsImage(char* inputFileName);
	/**
	*	Reset the plot window to have a single blank subplot
	*/
   void ResetPlot(void);

	/**
	*	Get the name of the menu for this type of PlotWindow.
	*/
	virtual const char* const menuName() const = 0;

	/**
	*	Get the name of the toolbar for this type of PlotWindow.
	*/
	virtual const char* const toolbarName() const = 0;

	HWND hWnd;
	PlotList& plotList() { return plotList_; }
	void plotList(PlotList val) { plotList_ = val; }
	ObjectData *obj;

	bool showStatusBar;
	bool showToolBar;
	bool showLabels;
	bool useBackingStore;
	HMENU traceMenu;
	HMENU bkgMenu;
	HMENU axesMenu;
	HMENU titleMenu;
	HMENU labelMenu;
	HBITMAP bitmap;
	short traceMenuNr;
	short bkgMenuNr;
	short axesMenuNr;
	short titleMenuNr;
	short labelMenuNr;
	MouseMode mouseMode;
	CText fileName() const { return fileName_; }
	void fileName(CText val) { fileName_ = val; }
	int fileNumber;
	bool modified() const { return modified_; }
	void modified(bool val) { modified_ = val; }
	bool updatePlots() const { return updatePlots_; }
	bool updatingPlots() const { return updatingPlots_; }
	void updatingPlots(bool val) { updatingPlots_ = val; }
	void updatePlots(bool val) { updatePlots_ = val; }
   Plot *GetSavedPlot() {return gSavedPlot;}
   void SetSavedPlot(Plot* plt) {if(gSavedPlot) delete gSavedPlot;gSavedPlot = plt;}
	short rows;
	short cols;

	char* annotationCallback; ///< Macro callback, called on doubleclick of Annotation
	char* imageInsetCallback; ///< ImageInset callback, called on doubleclick of ImageInset

protected:
	void ScaleToPrintedPage(HDC hdcPrint, float& xScale, float& yScale);
	void FindCurrentPlot(short &curN, short &curPlotN);
	PlotFactory* plotFactory;
	PlotList plotList_;

private:
	static long codeToDimensions(long code);
	void RestoreAllPlots(void);
	void getCurrentPlotPosition(short &row, short &col, short &index);
	void ReassignCellCoordinates();
	void RemoveBlankPlots();
	virtual std::string& fileDimensionString() = 0;
	virtual PlotList& getPlotList() = 0;

	/**
	*	Save the contents of the plot window as an enhanced meta file
	*	@param inputFileName the name of the file to save to; a dialog will open if this is null
	*	@return OK if the operation succeeded
	*/
	virtual short MakeWMF(char* inputFileName) = 0;

	bool busy;
   int criticalSectionLevel;
	static Plot* gSavedPlot;
	static short plotInsertMode;
	short oldCurrent;
   bool useDefaultParameters_; // Whether default plot parameters are used when a plot is loaded
	bool updatePlots_; // Whether a plot may be updated
	bool updatingPlots_; // Whether a plot is being drawn to.
	bool modified_;
	CText fileName_;
	short oldRows_;
	short oldCols_;
};

class PlotWindow1D : public PlotWindow
{
public:
	PlotWindow1D();
	PlotWindow1D(MouseMode mouseMode, HWND hWnd, PlotWinDefaults* pwd,
										ObjectData* obj, short w, short h);
	~PlotWindow1D();

	Plot* curPlot();
	void DisplayAll(bool locked);
	short SavePlots(char* pathName, char* fileName, long x, long y);

	void ProcessScrollWheelEvents(HWND hWnd, short zDel, short fwKeys);

	void RemoveAllButCurrentPlot(void);
	void UpdateStatusBar(void);
	short CopyPlotsToClipboard(bool multiplot);

	int dim();

	void ClearCurrentPlot();
	void GetCurrentPlot(CText &whichPlot, short &x, short &y, short &index);
	void PrintDisplay(HDC hdc);
	void clearPlotData(PlotList& plotList);	
	PlotList& getSavedPlotList();

	char* plotDirectory(){return PlotFile1D::getCurrPlotDirectory();}
	const char* const menuName() const {return "plot_view_menu";}
	const char* const toolbarName() const {return "plot_toolbar";}

	int LoadAndDisplayPlotFile(HWND hWnd, char *filePath, char *fileName);

private:
	short CopyAsBitMapToClipBoard(bool multiplot, bool emptyClipBoard);
	long fileDimensionCode();
	std::string& fileDimensionString();
	PlotList& getPlotList();
	/**
	*	Save the contents of the plot window as an enhanced meta file
	*	@param inputFileName the name of the file to save to; a dialog will open if this is null
	*	@return OK if the operation succeeded
	*/
	short MakeWMF(char* inputFileName);

	static PlotList savedPlotList; // Array of plot information to save TODO - delete on exit!
	static PlotList copiedPlotList; // Array of plot information to for copy/paste ops TODO delete on exit!
	static std::string fileDimensionString_;
	static const long fileDimensionCode_ = 'PL1D';
};

class PlotWindow2D : public PlotWindow
{
public:
	PlotWindow2D();
	~PlotWindow2D();
	
	Plot* curPlot();
	PlotWindow2D(MouseMode mouseMode, HWND hWnd, PlotWinDefaults* pwd,
									ObjectData* obj, short w, short h);
	void DisplayAll(bool locked);
	short SavePlots(char* pathName, char* fileName, long x, long y);
	void InitialisePlot(short nrX, short nrY, PlotWinDefaults* pwd);
	void ProcessScrollWheelEvents(HWND hWnd, short zDel, short fwKeys);
	void Paint();
	void UpdateStatusBar(void);
	void GetCurrentPlot(CText &whichPlot, short &x, short &y, short &index);
	short CopyPlotsToClipboard(bool multiplot);
	void PrintDisplay(HDC hdc);

	void CloseCurrentPlot();
	int dim();
	void ClearCursors(HDC hdc);
	static short getPlotInsertMode();
	static void setPlotInsertMode(short mode);
	void clearPlotData(PlotList& plotList);	
	PlotList& getSavedPlotList();
	char* plotDirectory(){return PlotFile2D::getCurrPlotDirectory();}
	const char* const menuName() const {return "image_view_menu";}
	const char* const toolbarName() const {return "image_toolbar";}

	int LoadAndDisplayPlotFile(HWND hWnd, char *filePath, char *fileName);

private:
	short CopyAsBitMapToClipBoard(bool multiplot, bool emptyClipBoard);
	long fileDimensionCode();
	std::string& fileDimensionString();
	PlotList& getPlotList();
	/**
	*	Save the contents of the plot window as an enhanced meta file
	*	@param inputFileName the name of the file to save to; a dialog will open if this is null
	*	@return OK if the operation succeeded
	*/
	short MakeWMF(char* inputFileName);


	static PlotList savedPlotList; // Array of plot information to save TODO - delete on exit!
	static PlotList copiedPlotList; // Array of plot information to for copy/paste ops TODO delete on exit!
	static std::string fileDimensionString_;
	static const long fileDimensionCode_ = 'PL2D';
};



#endif // define PLOTWINDOW_H
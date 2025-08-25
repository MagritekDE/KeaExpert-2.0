#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)
#include <gdiplus.h> 
#include "stdafx.h"
#include "PlotWindow.h"
#include "allocate.h"
#include "bitmap.h"
#include "files.h"
#include "globals.h"
#include "guiModifyObjectParameters.h"
#include "guiWindowClass.h"
#include "import_export_utilities.h"
#include "metafile.h"
#include "myBoost.h"
#include "plot1dCLI.h"
#include "plot.h"
#include "plot_dialog_1d.h"
#include "PlotFile.h"
#include "plotwindefaults.h"
#include "print.h"
#include "prospaResource.h"
#include "string_utilities.h"
#include <algorithm>
#include <math.h>
#include "memoryLeak.h"

using namespace Gdiplus;
using std::find;

// Definitions of nonconst static members.
Plot* PlotWindow::gSavedPlot = NULL;
short PlotWindow::plotInsertMode = ID_NEW_PLOT;

/*******************************************************************
  Initialise the plot parent
********************************************************************/

PlotWindow::PlotWindow()
{
   showLabels = true;
   showStatusBar = true;
   showToolBar = true;
   useBackingStore = false;
   rows = 1;
   cols = 1;
   oldRows_ = -1;
   oldCols_ = -1;   
   hWnd = NULL;
   traceMenu = NULL;
   bkgMenu = NULL;
   axesMenu = NULL;
   titleMenu = NULL;
   labelMenu = NULL;
	bitmap = NULL;
   traceMenuNr = -1;
   bkgMenuNr = -1;
   axesMenuNr = -1;
   titleMenuNr = -1;
   labelMenuNr = -1;
   fileName("untitled");
   fileNumber = 1;
   modified(false);
   obj = NULL;
	busy = false;
   criticalSectionLevel = 0;
   updatePlots(true);
   updatingPlots(false);
	oldCurrent = -1;
   useDefaultParameters_ = true;
}

/*******************************************************************
  Free all memory associate with the plot parent
********************************************************************/

PlotWindow::~PlotWindow()
{
	for(Plot* p: plotList_)
	{
      ShowWindow(p->win,SW_HIDE);
	   DestroyWindow(p->win);			
   }

	clearPlotData(plotList_);

   //if(traceMenu)
   //  DeleteObject(traceMenu);
   //if(bkgMenu)
   //  DeleteObject(bkgMenu);
   //if(axesMenu)
   //  DeleteObject(axesMenu);
   //if(titleMenu)
   //  DeleteObject(titleMenu);
   //if(labelMenu)
   //  DeleteObject(labelMenu);

   cols = rows = 0;
   oldRows_ = oldCols_ = -1;

	bitmap = NULL;
   hWnd = NULL;
   traceMenu = NULL;
   bkgMenu = NULL;
   axesMenu = NULL;
   titleMenu = NULL;
   labelMenu = NULL;
}


// Search the current plot parent for the cur1/2DPlot. If it is found
// return the index. If cur1/2DPlot is also curPlot then also note this as well

void PlotWindow::FindCurrentPlot(short &curN, short &curPlotN)
{
   curN = curPlotN = -1;
   if(this->plotList_.size() == 0)
      return;
   for(short i = 0; i < this->rows*this->cols; i++)
	{
		if(Plot1D::curPlot() == this->plotList_[i] ||
			Plot2D::curPlot() == this->plotList_[i])
      {
         curN = i;
			if(Plot::curPlot() == Plot1D::curPlot() ||
            Plot::curPlot() == Plot2D::curPlot())
         {
            curPlotN = i;
         }
         break;
      }
   }
}

/*******************************************************************
  Initialise plot parent structure
********************************************************************/

void PlotWindow::InitialisePlot(short nrX, short nrY, PlotWinDefaults* pwd)
{
   cols = nrX;
   rows = nrY;

	for(short i = 0; i < rows; i++)
	{
      for(short j = 0; j < cols; j++)
	   {
	      short k = cols*i + j;			
			plotList_.push_back(plotFactory->makePlot(pwd,hWnd,this,i,j)); // Initialize plot data
		   plotList_[k]->ResizePlotRegion(hWnd); // Set plot region positions
		}
	}
}


/*******************************************************************
  Initialise plot parent structure
********************************************************************/

short PlotWindow::DefaultProc(Interface* itfc, char *args)
{
   PlotXY(itfc ,args);
   return(OK);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/

void PlotWindow::DrawPlotTitle()
{
	CText title;

	if(modified())
	{
		if(fileNumber > 0)
			title.Format("1D Plot - %s* %d",fileName().Str(),fileNumber);
		else
			title.Format("1D Plot - %s*",fileName().Str());
	}
	else
	{
		if(fileNumber > 0)
			title.Format("1D Plot - %s %d",fileName().Str(),fileNumber);
		else
			title.Format("1D Plot - %s",fileName().Str());
	}
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
short PlotWindow::DisplayOnePlot(short index)
{
	PlotList& saveList = getSavedPlotList();

	// Don't do this if already in this mode
	if(!saveList.empty() || rows*cols == 1)
		return -1;

	if (-1 == index)
	{
		for(index = 0; index < rows*cols; index++)
		{
			if(plotList_[index] == Plot::curPlot())
				break;
      }
	}

	// Save the current plots
	for(Plot* p: plotList_)
	{
		saveList.push_back(p->clone());
	}
	// Remember the old layout
	oldRows_ = rows;
	oldCols_ = cols;

	// Make a single plot and make the necessary connection to the original data

	clearPlotData(plotList_);
	rows = cols = 1;

	plotList_.push_back(saveList[index]->clone());
	ReassignCellCoordinates();
	makeCurrentPlot();
	makeCurrentDimensionalPlot();
	AddCharToString(curPlot()->statusText, 'V');
	UpdateStatusWindow(hWnd,3,curPlot()->statusText);
	
	curPlot()->Invalidate();
	oldCurrent = index;
	return index;
}

void PlotWindow::RestoreAllPlots() 
{
	PlotList& saveList = getSavedPlotList();

	if(oldRows_ > 0 && oldCols_ > 0 && !saveList.empty())
	{
		rows = oldRows_;
		cols = oldCols_;

		clearPlotData(plotList_);

		for(Plot* p: saveList)
		{
			plotList_.push_back(p->clone());
		}
		ReassignCellCoordinates();
		makeCurrentPlot(oldCurrent);
		makeCurrentDimensionalPlot(oldCurrent);

		clearPlotData(saveList);

		oldRows_ = -1;
		oldCols_ = -1;
      RemoveCharFromString(curPlot()->statusText,'V');
      UpdateStatusWindow(hWnd,3,curPlot()->statusText);
		curPlot()->Invalidate();
	}
}


// Delete all multiplots but keep the current plot in a single region

void PlotWindow::RemoveAllButCurrentPlot()
{
	Plot* pd = curPlot();
	
	PlotList::iterator keepMe = find(plotList_.begin(), plotList_.end(), pd);
	if (keepMe == plotList_.end())
	{
		// The requested plot is not in this window. Do nothing.
		return;
	}

	Plot* kept = (*keepMe)->clone();

	clearPlotData(plotList_);
	plotList_.push_back(kept);
	cols = rows = 1;
	ReassignCellCoordinates();
	kept->makeCurrentDimensionalPlot();
	kept->makeCurrentPlot();
	if(kept->getDimension() == 1)
	{
		Plot1D* plot = dynamic_cast<Plot1D*>(kept);
		if (plot->hasNoCurTrace())
			Plot1D::curPlot()->setCurTrace();
	}
	clearPlotData(getSavedPlotList());
	oldRows_ = oldCols_ = -1;
}


void PlotWindow::ViewFullPlot(void)
{
	if(oldRows_*oldCols_ > 1)
	{
		RestoreAllPlots();
	}
}


/********************************************************
*    Remove all blank subplots 
********************************************************/
void PlotWindow::RemoveBlankPlots()
{
	PlotList nonBlankPlots;

	// Compact
	PlotList& plotList = getPlotList();
	for(Plot* p: plotList)
	{
		if(p->DataPresent())
		{
			nonBlankPlots.push_back(p->clone());
		}
	}

	clearPlotData(plotList);
	if (nonBlankPlots.empty())
	{
		plotList.push_back(this->makePlot(pwd,hWnd,this,0,0));
	}
	else
	{
		plotList = nonBlankPlots;
	}
	ReassignCellCoordinates();

	this->makeCurrentPlot();
}

short PlotWindow::ViewSubPlot(short xPos, short yPos)
{
	short index = (xPos-1) + (yPos-1)*cols;

	if(index >= rows*cols || index < 0)
	{
		ErrorMessage("Invalid sub-plot selection");
		return(ERR);
	}

	if(rows*cols > 1)
	{
		DisplayOnePlot(index);
	}
	return OK;
}


bool PlotWindow::CopyCurrentPlot()
{
	if(Plot::curPlot()->DataPresent())
	{
      Plot *plt = Plot::curPlot();
		if(gSavedPlot)
			delete gSavedPlot;
		gSavedPlot = Plot::curPlot()->clone();
      return(true);
	}
   return(false);
}

bool PlotWindow::CopyPlot(Plot *plt)
{
	if(plt->DataPresent())
	{
		if(gSavedPlot)
			delete gSavedPlot;
		gSavedPlot = plt->clone();
      return(true);
	}
   return(false);
}


Plot* PlotWindow::PasteSavedPlot()
{
	if(Plot::curPlot() && gSavedPlot && gSavedPlot->DataPresent() && gSavedPlot->getDimension() == Plot::curPlot()->getDimension())
	{
		// Protect plots from multithread access	
	//	printf("Waiting for critical section PASTESAVEDPLOT 1\n");
		EnterCriticalSection(&cs1DPlot);
		Plot::curPlot()->plotParent->incCriticalSectionLevel();
	//	printf("In critical section PASTESAVEDPLOT 1\n");

		short x = Plot::curPlot()->colNr;
		short y = Plot::curPlot()->rowNr;
		HWND win = Plot::curPlot()->win;
      Plot *plt = Plot::curPlot();

		delete Plot::curPlot();
	
		Plot* p = gSavedPlot->clone();
		p->plotParent = this;
		p->win = win;
		plotList_[y * cols + x] = p;
		ReassignCellCoordinates();
		p->makeCurrentPlot();
		p->makeCurrentDimensionalPlot();

		LeaveCriticalSection(&cs1DPlot);
      p->plotParent->decCriticalSectionLevel();
	 //  printf("Left critical section PASTESAVEDPLOT 1\n");
      return(p);
	}
   return(NULL);
}

Plot* PlotWindow::PasteSavedPlot(Plot *plt)
{
	if(plt && gSavedPlot && gSavedPlot->DataPresent() && gSavedPlot->getDimension() == plt->getDimension())
	{
		// Protect plots from multithread access	
	//	printf("Waiting for critical section PASTESAVEDPLOT 2\n");
		EnterCriticalSection(&cs1DPlot);
		plt->plotParent->incCriticalSectionLevel();
	//	printf("In critical section PASTESAVEDPLOT 2\n");

		short x = plt->colNr;
		short y = plt->rowNr;
		HWND win = plt->win;

		delete plt;
	
		Plot* p = gSavedPlot->clone();
		p->plotParent = this;
		p->win = win;
		plotList_[y * cols + x] = p;
		ReassignCellCoordinates();
		p->makeCurrentPlot();
		p->makeCurrentDimensionalPlot();

		LeaveCriticalSection(&cs1DPlot);
      plt->plotParent->decCriticalSectionLevel();
	 //  printf("Left critical section PASTESAVEDPLOT 2\n");

      return(p);
	}
   return(NULL);
}

bool PlotWindow::PastePlot(Plot *plt)
{
	if(plt && gSavedPlot && gSavedPlot->DataPresent() && gSavedPlot->getDimension() == plt->getDimension())
	{
	//	printf("Waiting for critical section PASTEPLOT\n");
		EnterCriticalSection(&cs1DPlot);
		plt->plotParent->incCriticalSectionLevel();
	//	printf("In critical section PASTEPLOT\n");

		short x = plt->colNr;
		short y = plt->rowNr;
		HWND win = plt->win;

		delete plt;
	
		Plot* p = gSavedPlot->clone();
		p->plotParent = this;
		p->win = win;
		plotList_[y * cols + x] = p;
		ReassignCellCoordinates();
		p->makeCurrentPlot();
		p->makeCurrentDimensionalPlot();

		LeaveCriticalSection(&cs1DPlot);
      p->plotParent->decCriticalSectionLevel();
	//   printf("Left critical section PASTEPLOT\n");

      return(true);
	}
   return(false);
}


void PlotWindow::PasteSavedPlotInto()
{
	if(Plot::curPlot() && gSavedPlot && gSavedPlot->DataPresent() && gSavedPlot->getDimension() == Plot::curPlot()->getDimension())
	{
		gSavedPlot->PasteInto(Plot::curPlot());
		ReassignCellCoordinates();
		makeCurrentPlot();
		makeCurrentDimensionalPlot();
	}
}

void PlotWindow::PasteSavedPlotInto(Plot* plt)
{
	if (gSavedPlot && gSavedPlot->DataPresent() && gSavedPlot->getDimension() == plt->getDimension())
	{
		gSavedPlot->PasteInto(plt);
		ReassignCellCoordinates();
		makeCurrentPlot();
		makeCurrentDimensionalPlot();
	}
}


//void PlotWindow::SwapPlots()
//{
//	PlotWindow *pp = Plot::curPlot()->plotParent;
//
//	if(Plot::curPlot() && gSavedPlot && gSavedPlot->DataPresent())
//	{
//		short x = Plot::curPlot()->colNr;
//		short y = Plot::curPlot()->rowNr;
//		short cols = pp->cols;
//		HWND win = Plot::curPlot()->win;
//
//		delete Plot::curPlot();
//		gSavedPlot->clone()->makeCurrentPlot();
//		Plot::curPlot()->makeCurrentDimensionalPlot();
//		Plot::curPlot()->plotParent = pp;
//		Plot::curPlot()->win = win;
//		Plot::curPlot()->colNr = x;
//		Plot::curPlot()->rowNr = y;
//		pp->plotList[y*cols+x] = Plot::curPlot();
//	}
//}

Plot* PlotWindow::makePlot(PlotWinDefaults *pd, HWND hWnd, PlotWindow *pp, short row, short col)
{
	return plotFactory->makePlot(pd,hWnd,pp,row,col);
}

Plot* PlotWindow::curPlot()
{
	return Plot::curPlot();
}


void PlotWindow::ProcessScrollWheelEvents(HWND hWnd, short zDel, short fwKeys)
{
    extern int gPlotViewVersion;
    extern bool gScrollWheelEvent;
	 extern CText gPlot1DScaleMacro;

 //  printf("Waiting for critical section SCROLLWHEEL_EVENTS\n");
	EnterCriticalSection(&cs1DPlot);

   gScrollWheelEvent = true;
   Plot* currentPlot = curPlot();

	currentPlot->plotParent->incCriticalSectionLevel();
//	printf("In critical section SCROLLWHEEL_EVENTS\n");

	if(currentPlot->updatingPlots())
	{
		goto EXIT;
	}

	HDC hdc = GetDC(hWnd);
	currentPlot->HideSelectionRectangle(hdc);  
   ReleaseDC(hWnd,hdc);

   Plot1D* curPlot = Plot1D::curPlot();

	if (gPlot1DScaleMacro != "")
	{
		extern short ProcessMacroStr(int scope, WinData * parent, ObjectData * obj, char* data, char* arguments, char* procName, char* macroName, char* macroPath);
		currentPlot->plotParent->decCriticalSectionLevel();
		LeaveCriticalSection(&cs1DPlot);
		CText cmd;
		cmd.Format("%s(%d, %d)", gPlot1DScaleMacro.Str(), zDel, fwKeys);
		ProcessMacroStr(0, NULL, NULL, cmd.Str(), "", "", gPlot1DScaleMacro.Str(), "");
		return;
	}


   if(gPlotViewVersion == 1)
   {
      if((fwKeys & MK_SHIFT) || (GetAsyncKeyState((int)'X') & 0x08000)) // Shift left and right
      {
         if(zDel < 0)
         {
            curPlot->plotMouseScaled = true;
            curPlot->shiftDirection = ID_SHIFT_LEFT;
         }
         else
         {
            curPlot->plotMouseScaled = true;
            curPlot->shiftDirection = ID_SHIFT_RIGHT;
         }
      }
      else if(fwKeys & MK_CONTROL) // Shift up and down
      {
         if(zDel < 0)
         {
            curPlot->plotMouseScaled = true;
            curPlot->shiftDirection = ID_SHIFT_UP;
         }
         else
         {
            curPlot->plotMouseScaled = true;
            curPlot->shiftDirection = ID_SHIFT_DOWN;
         }
      }
      else // Enlarge
      {
         if(zDel < 0)
         {
            curPlot->plotMouseScaled = true;
            curPlot->scaleDirection = ID_ENLARGE_BOTH;
         }
         else
         {
            curPlot->plotMouseScaled = true;
            curPlot->scaleDirection = ID_REDUCE_BOTH;
         }
      }
   }
   else // Spinsolve Expert
   {
      if((fwKeys & MK_CONTROL) && !(fwKeys & MK_SHIFT)) // Shift up/down
      {
         if(zDel < 0)
         {
            curPlot->plotShifted = true;
            curPlot->shiftDirection = ID_SHIFT_UP;
         }
         else
         {
            curPlot->plotShifted = true;
            curPlot->shiftDirection = ID_SHIFT_DOWN;
         }
      }
      else if(!(fwKeys & MK_CONTROL) && (fwKeys & MK_SHIFT)) // Enlarge horzontally
      {
         if(zDel < 0)
         {
            curPlot->plotMouseScaled = true;
            curPlot->scaleDirection = ID_ENLARGE_HORIZ;
         }
         else
         {
            curPlot->plotMouseScaled = true;
            curPlot->scaleDirection = ID_REDUCE_HORIZ;
         }
      }
      else if((fwKeys & MK_CONTROL) && (fwKeys & MK_SHIFT)) // Shift left/right
      {
         if(zDel < 0)
         {
            curPlot->plotShifted = true;
            curPlot->shiftDirection = ID_SHIFT_LEFT;
         }
         else
         {
            curPlot->plotShifted = true;
            curPlot->shiftDirection = ID_SHIFT_RIGHT;
         }
      }
      else // Enlarge vertically
      {
         if(zDel < 0)
         {
            curPlot->plotMouseScaled = true;
            curPlot->scaleDirection = ID_ENLARGE_VERT;
         }
         else
         {
            curPlot->plotMouseScaled = true;
            curPlot->scaleDirection = ID_REDUCE_VERT;
         }
      }
   }
   curPlot->Invalidate();

EXIT:
	currentPlot->plotParent->decCriticalSectionLevel();
	LeaveCriticalSection(&cs1DPlot);
//	printf("Left critical section SCROLLWHEEL_EVENTS\n");
}

/********************************************************
*    Make nrX by nrY subplots clearing all
********************************************************/
void PlotWindow::MakeMultiPlot(short nrX, short nrY)
{
	short n,m;

	FindCurrentPlot(n,m);
	 clearPlotData(plotList_);
   if(oldRows_*oldCols_ > 1)
   {
      PlotList& saveList = getSavedPlotList();
      clearPlotData(saveList);
      oldRows_ = oldCols_ = -1;
   }
	InitialisePlot(nrX,nrY,pwd);
	Plot *pd = plotList_[0];
	RemoveCharFromString(pd->statusText,'V');
	UpdateStatusWindow(pd->win,3,pd->statusText);
   if(dim() == 1)
      dynamic_cast<Plot1D*>(pd)->setOverRideAutoRange(false);
	if(n != -1)
	{
		makeCurrentDimensionalPlot();
	}

	if(m != -1)
	{
		makeCurrentPlot();
	}
}


/********************************************************
*    Set the margins for a plot window
********************************************************/
void PlotWindow::SetMargins(Margins &margins)
{
	for(Plot* p : this->plotList())
	{
		p->setMargins(margins);
	}
}

int PlotWindow::getCriticalSectionLevel() {return (this->criticalSectionLevel);}
bool PlotWindow::inCriticalSection(void) 
{
  // return(busy == true);
   return (this->criticalSectionLevel > 0);
}
void PlotWindow::incCriticalSectionLevel(void)
{
   busy = true;
   criticalSectionLevel++;
}
void PlotWindow::decCriticalSectionLevel(void)
{
   busy = false;
   criticalSectionLevel--;
   if(criticalSectionLevel < 0)
      int a = 23;
}
bool PlotWindow::isBusy(void) {return this->busy;}
PlotWindow* PlotWindow::setBusy(bool busy) {this->busy = busy; return this;}
PlotWindow* PlotWindow::setBusyWithCriticalSection(bool busy) 
{
 //  extern bool busySet;
   EnterCriticalSection(&cs1DPlot);
   this->busy = busy; 
   LeaveCriticalSection(&cs1DPlot);
   return this;
}
short PlotWindow::getCols(void) {return this->cols;}
short PlotWindow::getRows(void) {return this->rows;}
PlotWindow* PlotWindow::setCols(short cols) {this->cols = cols; return this;}
PlotWindow* PlotWindow::setRows(short rows) {this->rows = rows; return this;}
bool PlotWindow::isShowLabels(void) {return this->showLabels;}
PlotWindow* PlotWindow::setShowLabels(bool showLabels) {this->showLabels = showLabels; return this;}


/****************************************************************************
    Identify the current plot via the row, col and array index
****************************************************************************/
void PlotWindow::getCurrentPlotPosition(short &row, short &col, short &index)
{
	Plot* p = curPlot();
	PlotListIterator plit = find(plotList_.begin(), plotList_.end(), p);
	if (plit == plotList_.end())
	{
		index = 0;
	}

	index = ((*plit)->colNr + (*plit)->rowNr * cols);
}

/****************************************************************************
   Return the current plot as "1d/2d" and then the x, y and array index
   Note that x and y are 1 based
****************************************************************************/

void PlotWindow::GetCurrentPlot(CText &whichPlot, short &x, short &y, short &index)
{
	getCurrentPlotPosition(y,x,index);
   x++;
   y++;
}

long PlotWindow::codeToDimensions(long code)
{
	if ('PL1D' == code)
	{
		return 1;
	}
	else if ('PL2D' == code)
	{
		return 2;
	}
	else
	{
		throw PlotWindowException("Invalid plot file magic number.");
	}	
}

/*******************************************************************************
*                Load a 1D plot or plots from a file into the 2D window        *
*                                                                              *
* The file can contain a number of 1D plots organised in a square array.       *
*                                                                              *
* Routine structure is:                                                        *
*                                                                              *
* Open file for binary read                                                    *
* Read file type (PR1D/PR2D)                                                   *
* Read file version number (1/2/3 ...)                                         *
* Read number of rows of plots                                                 *
* Read number of columns of plots                                              *
* Loop over rows                                                               *
*    Loop over columns                                                         *
*       Read width of matrix i                                                 *
*       Read height of matrix i                                                *
*       Read binary float data for matrix i                                    *
*    Next column                                                               *
* Next row                                                                     *
* Close file                                                                   *
*                                                                              *
* !!!!! Needs work relating to the cur1DPlot cf original                       *
*                                                                              *
*******************************************************************************/

short PlotWindow::LoadPlots(char *pathName, char *fileName)
{
   FILE *fp;            // File pointer
   long code;           // Type code 'PL1D', 'PL2D'
   short nrX,nrY;       // Plot array dimensions
   char path[MAX_PATH]; // Backup of path
   char file[MAX_PATH]; // Backup of file
   char oldDir[MAX_PATH];

// Save the current working directory as the following command changes it
   GetCurrentDirectory(MAX_PATH,oldDir);

// Make a backup of the path and filename since they are wiped **
   strncpy_s(path,MAX_PATH,pathName,_TRUNCATE);
   strncpy_s(file,MAX_PATH,fileName,_TRUNCATE);
   
// Open file as binary for reading *******************************
   SetCurrentDirectory(pathName);  
// Make sure we have the full path
   GetCurrentDirectory(MAX_PATH,path);

   if(!(fp = fopen(fileName,"rb")))
	{
      SetCurrentDirectory(oldDir);
	   ErrorMessage("Can't open file '%s'",fileName);
	   return(ERR);
	}

// Protect plots from multithread access	
//	printf("Waiting for critical section LOADPLOTS\n");
   EnterCriticalSection(&cs1DPlot);
   this->incCriticalSectionLevel();
	//printf("In critical section LOADPLOTS\n");

   SetCurrentDirectory(oldDir);

	// Make sure its a Prospa file *****************
	fread(&code,sizeof(long),1,fp);
	if(code != 'PROS')
	{
		ErrorMessage("'%s' is not a native %s file",fileName,APPLICATION_NAME);
		goto ERR_FOUND;
	}     

	// Check this has the expected dimension code
	fread(&code,sizeof(long),1,fp);
	if (code != fileDimensionCode())
	{
		ErrorMessage("'%s' is not a %s Plot file",fileName, fileDimensionString());
		goto ERR_FOUND;
	}   

	// Work out number of plot regions in file data ***
	fread(&nrY,sizeof(short),1,fp);
	if(nrY < 1 || nrY > 10)
	{
		ErrorMessage("Invalid number of plot rows");
		goto ERR_FOUND;
	}

	fread(&nrX,sizeof(short),1,fp);
	if(nrX < 1 || nrX > 10)
	{
		ErrorMessage("Invalid number of plot columns");
		goto ERR_FOUND;
	}


	// Get the number corresponding to the number of dimensions specified
	//  as a magic number in the plot file
	try 
	{
		code = PlotWindow::codeToDimensions(code);
	}
	catch(PlotWindowException& e)
	{
		ErrorMessage(e.what());
		goto ERR_FOUND;
	}


// Display plots but note desired mode first ******** 
   switch(plotInsertMode)
   {
      case(ID_NEW_PLOT): // Start from fresh plot deleting all current plots
      {   
			clearPlotData(plotList_);

			// Load legacy info up to version 3.70
			for (int i = 0; i < nrX*nrY; i++)
			{
			   Plot* plot = 0;
				PlotFile* loader = 0;
				try
				{
					loader = PlotFile::makeLoader(code,fp);
					plot = loader->load();
				}
				catch (PlotFileException& e)
				{
					if (loader)
						delete(loader);
					if (plot)
						delete(plot);
               ResetPlot();
					ErrorMessage(e.what());
		         goto ERR_FOUND;
				}
				delete loader;
				if (!plot)
				{
		         goto ERR_FOUND;
				}
				if(getUseDefaultParameters())
				{
					plot->setDefaultParameters();
			   }
				plot->setFileName(file);
				plot->setFilePath(path);
				plot->win = this->hWnd;
				plot->plotParent = this;
				plotList_.push_back(plot);
			}

			// Load any new info after version 3.70
			for(Plot* p : this->plotList())
			{	
				p->LoadV3_8Info(fp);
			}

			if (getUseDefaultParameters())
			{
				for(Plot* plot: plotList_)
				{
					plot->setDefaultParameters();
				}
			}	
			rows = nrY;
			cols = nrX;
			ReassignCellCoordinates();
			makeCurrentPlot();
			makeCurrentDimensionalPlot();			
			break;
	   }
	   case(ID_APPEND_PLOTS): // Add plots to the end - expand multiplot if necessary
	   {
			// Keep every plot up to the last non-blank existing plot.
			PlotList temp;
			bool startCopying = false;			
			int plotsCopied = 0;

			reverse_foreach(Plot* p, plotList_)
			{
				if(p->DataPresent())
				{
					startCopying = true;
				}
				if(startCopying)
				{
					temp.push_front(p->clone());
					plotsCopied++;
				}
			}

			clearPlotData(plotList_);
			plotList_ = temp;

			// Append the loaded plots after that last non-blank existing plot.
			for (int i = 0; i < nrX*nrY; i++)
			{
				PlotFile* loader = 0;
				Plot* plot = 0;
				try
		      {
					loader = PlotFile::makeLoader(code,fp);
					plot = loader->load();
				}
				catch (PlotFileException& e)
				{
					if (loader)
						delete(loader);
					if (plot)
						delete(plot);
               ResetPlot();
					ErrorMessage(e.what());
		         goto ERR_FOUND;
				}
				delete loader;
				if (!plot)
		      {
		         goto ERR_FOUND;
		      }
				plot->setFileName(file);
				plot->setFilePath(path);
         // Check to see if we want to inherit the default (pwd) plot parameters
            if(this->getUseDefaultParameters())
               plot->setDefaultParameters(); // If so set defaults
				plot->win = this->hWnd;
				plot->plotParent = this;
				plotList_.push_back(plot);	
				plot->makeCurrentPlot();
				plot->makeCurrentDimensionalPlot();
			}
			// Load any new info after version 3.70
			//for(Plot* p : this->plotList())
			//{	
			//	p->LoadV3_8Info(fp);
			//}
			// Did we already have enough rows to fit everything?
			if ((plotsCopied + nrX * nrY) > (rows * cols))
				// No -- change the no. of rows
	      {
            rows = ceill((float)(plotsCopied + nrX*nrY)/cols);
         }	
			// Fill out the rest with blank plots, if needed
			if((plotsCopied + nrX * nrY) < (rows * cols))
			{
				int blanksNeeded = (rows * cols) - plotList_.size();
            Plot *pt;
				for(int i = 0; i < blanksNeeded; i++) 
            {
               pt = makePlot(pwd,hWnd,this,0,0);
               pt->setDefaultParameters(); // If so set defaults
					plotList_.push_back(pt); 
            }
			}	         
		 // Update cell coordinates		
			ReassignCellCoordinates();
	      break;
	   }

	   case(ID_REPLACE_PLOTS): // Replace existing plot - expand multiplot if necessary
	   {
			// Find current plot (this will be the first replaced)
			Plot* findMe = curPlot();
			PlotList::iterator replaceMe = find(plotList_.begin(), plotList_.end(), findMe);
			
			// If the current plot wasn't found, do nothing.
			if (replaceMe == plotList_.end())
			{
				goto ALL_OK;
			}

			// There are k plots being inserted. Remove the next k plots.
			PlotList temp;
			PlotList::iterator it;
			// Copy all the plots up to the one before the one to replace.
			int pos = 0;
			for(it = plotList_.begin(); ((it != plotList_.end()) && (it != replaceMe)); ++it)
			{
				temp.push_back((*it)->clone());
				pos++;
			}
			
	    // Add new data	    
		   for(int i = 0; i < nrX*nrY; i++) 
		   {
				PlotFile* loader = 0;
				Plot* plot = 0;
				try
		      {
					loader = PlotFile::makeLoader(code,fp);
					plot = loader->load();
				}
				catch (PlotFileException& e)
				{
					if (loader)
						delete(loader);
					if (plot)
						delete(plot);
               ResetPlot();
					ErrorMessage(e.what());
		         goto ERR_FOUND;
				}
				delete loader;
				if (!plot)
		      {
		         goto ERR_FOUND;
		      }
         // Check to see if we want to inherit the default (pwd) plot parameters
            if(this->getUseDefaultParameters())
               plot->setDefaultParameters(); // If so set defaults
				plot->win = this->hWnd;
				plot->plotParent = this;
				plot->setFileName(file);
				plot->setFilePath(path);
				temp.push_back(plot);
				plot->makeCurrentPlot();
				plot->makeCurrentDimensionalPlot();
		   }
			// Load any new info after version 3.70
			//for(Plot* p : this->plotList())
			//{	
			//	p->LoadV3_8Info(fp);
			//}

			// Anything left unreplaced from the original plotList?
			int remainderSize = plotList_.size() - temp.size();
			if (remainderSize > 0)
			{
				for (it = plotList_.begin() + plotList_.size() - remainderSize; it != plotList_.end(); ++it)
				{
					temp.push_back((*it)->clone());
				}
			}
			clearPlotData(plotList_);
			for (it = temp.begin(); it != temp.end(); ++it)
				plotList_.push_back(*it);
         // Adjust the number of rows and columns
         if(nrX*nrY > rows*cols)
         {
            rows = nrX*nrY; 
            cols = 1;
         }
			ReassignCellCoordinates();
	      break;

	   }
	   case(ID_INSERT_AFTER_PLOTS):  // Insert plots after current plot - expand multiplot if necessary
	   {
			// Find current plot 
			Plot* findMe = curPlot();
			PlotListIterator afterMe = find(plotList_.begin(), plotList_.end(), findMe);
			
			// If the current plot wasn't found, do nothing.
			if (afterMe == plotList_.end())
			{
				goto ALL_OK;
			}

			// Copy everything from the current list, up to and including the current plot, into a new one
			PlotList temp;
			PlotListIterator it;
			for(it = plotList_.begin(); ((it != plotList_.end()) && (it != (afterMe + 1))); ++it)
			{
				temp.push_back((*it)->clone());
			}

	    // Add new data	    
		   for(int i = 0; i < nrX*nrY; i++)
		   {
				PlotFile* loader = 0;
				Plot* plot = 0;

				try
		      {
					loader = PlotFile::makeLoader(code,fp);
					plot = loader->load();
				}
				catch (PlotFileException& e)
				{
					if (loader)
						delete(loader);
					if (plot)
						delete(plot);
               ResetPlot();
					ErrorMessage(e.what());
		         goto ERR_FOUND;
				}
				delete loader;
				if (!plot)
		      {
		         goto ERR_FOUND;
		      }
				plot->win = this->hWnd;
				plot->plotParent = this;
				temp.push_back(plot);
				plot->makeCurrentPlot();
				plot->makeCurrentDimensionalPlot();
         // Check to see if we want to inherit the default (pwd) plot parameters
            if(this->getUseDefaultParameters())
               plot->setDefaultParameters(); // If so set defaults
				plot->setFileName(file);
				plot->setFilePath(path);
		   }
			// Load any new info after version 3.70
	/*		for(Plot* p : this->plotList())
			{	
				p->LoadV3_8Info(fp);
			}*/

			// Find last non-blank entry in plotList.
			PlotListIterator lastNonBlank = plotList_.end();
			for(it = afterMe + 1; it != plotList_.end(); ++it)
			{
				if ((*it)->DataPresent())
					lastNonBlank = it;
			}

		 // Restore old data after index
			if (lastNonBlank != plotList_.end())
			{
				for(it = afterMe + 1; ((it != plotList_.end()) && (it != lastNonBlank + 1)); ++it)
				{
					temp.push_back((*it)->clone());
				}	
			}

			// Ensure that we have not overgrown the number of rows.
			// Did we already have enough rows to fit everything?
			if ((temp.size()) > (rows * cols))
				// No -- change the no. of rows
	      {
            rows = ceill((float)(temp.size())/cols);
         }	
			// Fill out the rest with blank plots, if needed
			if((temp.size()) < (rows * cols))
			{
				int blanksNeeded = (rows * cols) - temp.size();
            Plot *pt;
				for(int i = 0; i < blanksNeeded; i++) 
            {
               pt = makePlot(pwd,hWnd,this,0,0);
               pt->setDefaultParameters(); // If so set defaults
					temp.push_back(pt); 
            }
			}	         

			// Copy the plots back from temp to the member plotList.
			clearPlotData(plotList_);
			for(Plot* plot: temp)
				plotList_.push_back(plot);
			ReassignCellCoordinates();
			makeCurrentPlot();
			makeCurrentDimensionalPlot();
	      break;
	   }
	   case(ID_INSERT_BEFORE_PLOTS):  // Insert plots before current plot - expand multiplot if necessary
	   {
			// Find current plot 
			Plot* findMe = curPlot();
			PlotListIterator plotPos = find(plotList_.begin(), plotList_.end(), findMe);
			
			// If the current plot wasn't found, do nothing.
			if(plotPos == plotList_.end())
			{
				goto ALL_OK;
			}

			// Copy everything from the current list, up to and excluding the current plot, into a new one
			PlotList temp;
			PlotListIterator it;
			for(it = plotList_.begin(); ((it != plotList_.end()) && (it != plotPos)); ++it)
			{
				temp.push_back((*it)->clone());
			}

	    // Add new data	    
		   for(int i = 0; i < nrX*nrY; i++)
		   {
				PlotFile* loader = 0;
				Plot* plot = 0;
				try
		      {
					loader = PlotFile::makeLoader(code,fp);
					plot = loader->load();
				}
				catch (PlotFileException& e)
				{
					if (loader)
						delete(loader);
					if (plot)
						delete(plot);
               ResetPlot();
					ErrorMessage(e.what());
		         goto ERR_FOUND;
				}
				delete loader;
				if (!plot)
		      {
		         goto ERR_FOUND;
		      }
				plot->win = this->hWnd;
				plot->plotParent = this;
				temp.push_back(plot);
				plot->makeCurrentPlot();
				plot->makeCurrentDimensionalPlot();
         // Check to see if we want to inherit the default (pwd) plot parameters
            if(this->getUseDefaultParameters())
               plot->setDefaultParameters(); // If so set defaults
				plot->setFileName(file);
				plot->setFilePath(path);
		   }
			// Load any new info after version 3.70
			//for(Plot* p : this->plotList())
			//{	
			//	p->LoadV3_8Info(fp);
			//}
			// Find last non-blank entry in plotList.
			PlotListIterator lastNonBlank = plotList_.end();
			for(it = plotPos; it != plotList_.end(); ++it)
			{
				if ((*it)->DataPresent())
					lastNonBlank = it;
			}

		 // Restore old data after index
			if (lastNonBlank != plotList_.end())
			{
				for(it = plotPos; ((it != plotList_.end()) && (it != lastNonBlank + 1)); ++it)
				{
					temp.push_back((*it)->clone());
				}	
			}

			// Ensure that we have not overgrown the number of rows.
			// Did we already have enough rows to fit everything?
			if ((temp.size()) > (rows * cols))
				// No -- change the no. of rows
	      {
            rows = ceill((float)(temp.size())/cols);
         }	
			// Fill out the rest with blank plots, if needed
			if((temp.size()) < (rows * cols))
			{
				int blanksNeeded = (rows * cols) - temp.size();
            Plot *pt;
				for(int i = 0; i < blanksNeeded; i++) 
            {
               pt = makePlot(pwd,hWnd,this,0,0);
               pt->setDefaultParameters(); // If so set defaults
					temp.push_back(pt); 
            }
			}	         

			// Copy the plots back from temp to the member plotList.
			clearPlotData(plotList_);
			for(Plot* plot: temp)
				plotList_.push_back(plot);
			ReassignCellCoordinates();
			makeCurrentPlot();
			makeCurrentDimensionalPlot();
	      break;
	   }  
	}

ALL_OK:
	LeaveCriticalSection(&cs1DPlot);
   this->decCriticalSectionLevel();
	//printf("Left critical section LOADPLOT\n");
   fclose(fp);
   return(OK);   

ERR_FOUND:
	LeaveCriticalSection(&cs1DPlot);
   this->decCriticalSectionLevel();
//	printf("Left critical section LOADPLOT\n");
   fclose(fp);
	return(ERR); 
             
}

void PlotWindow::ResetPlot()
{
   CText name = "untitled";
   fileNumber = rootWin->GetNextFileNumber(this->obj,name);
   fileName(name);
   modified(false);
	MakeMultiPlot(1,1); 
   makeCurrentPlot(0);
	makeCurrentDimensionalPlot();

   curPlot()->initialiseMenuChecks("clear");
	curPlot()->Invalidate(); 
}

short PlotWindow::getPlotInsertMode()
{
	return plotInsertMode;
}

void PlotWindow::setPlotInsertMode(short mode)
{
	plotInsertMode = mode;
}

/*************************************************************************
*    Allow the user to set the plot load mode via the macro interface    *
*************************************************************************/
short PlotWindow::setPlotMode(CText& mode)
{
   if(mode == "append")
		setPlotInsertMode(ID_APPEND_PLOTS);
   else if(mode == "insert before")
      setPlotInsertMode(ID_INSERT_BEFORE_PLOTS);
   else if(mode == "insert after" || mode == "insert")
      setPlotInsertMode(ID_INSERT_AFTER_PLOTS);
   else if(mode == "replace")
      setPlotInsertMode(ID_REPLACE_PLOTS);
   else if(mode == "new")
      setPlotInsertMode(ID_NEW_PLOT);
   else
   {
      ErrorMessage("invalid mode (append/insert/insert before/insert after/replace/new)");
      return(-1);
   }

   return(OK);
}

Plot* PlotWindow::makeCurrentDimensionalPlot(int index)
{
	if ((index < 0) || (index >= rows * cols))
	{
		return 0;
	}
	plotList_[index]->makeCurrentDimensionalPlot();
	return plotList_[index];
}

Plot* PlotWindow::makeCurrentPlot(int index)
{
	if ((index < 0) || (index >= rows * cols))
	{
		return 0;
	}
	plotList_[index]->makeCurrentPlot();
	return plotList_[index];
}


void PlotWindow::clearPlotData(PlotList& plotList)
{
	PlotList::iterator it;
	it = std::find(plotList.begin(), plotList.end(), Plot::curPlot());
	if (it != plotList.end())
	{
		Plot::setNoCurPlot();
	}
	std::for_each( plotList.begin(), plotList.end(), delete_object());
	plotList.clear();
}


void PlotWindow::ReassignCellCoordinates()
{
	// Reassign cell coordinates
	for(int i = 0; i < rows; i++)
	{
		for(int j = 0; j < cols; j++)
		{
			plotList_[i*cols+j]->rowNr = i;
			plotList_[i*cols+j]->colNr = j;
		}
	}
}

int PlotWindow::setFileNumber(int fileNum)
{
	this->fileNumber = fileNum;
	return fileNum;
}

/* Calculate scaling factors from window to printer page */
void PlotWindow::ScaleToPrintedPage(HDC hdcPrint, float& xScale, float& yScale)
{
	RECT pr;
	GetClientRect(hWnd,&pr);

   float printPixelWidth = GetDeviceCaps(hdcPrint,HORZRES);
   float printPixelHeight = GetDeviceCaps(hdcPrint,VERTRES);

   float printMMWidth = GetDeviceCaps(hdcPrint,HORZSIZE);
   float printMMHeight = GetDeviceCaps(hdcPrint,VERTSIZE);
   
   float videoPixelWidth = (pr.right-pr.left);
   float videoPixelHeight = (pr.bottom-pr.top);

   float xMMScale = printPixelWidth/(printMMWidth*videoPixelWidth);
   float yMMScale = printPixelHeight/(printMMHeight*videoPixelHeight);
   
   if(gFitToPage == true) // Fill paper
   {
      xScale = printPixelWidth/videoPixelWidth;
      yScale = printPixelHeight/videoPixelHeight;
   }
   else // Print to specific size in mm
   {
      xScale = gPrintWidth*xMMScale;
      yScale = gPrintHeight*yMMScale;
   }   
}


bool PlotWindow::StylePlotTitles()
{
   CHOOSEFONT cf;
	Plot* plot = curPlot();
   LOGFONT font = plot->title().font();      
   InitFont(TitleHookProc,"SETTITLEDIALOG", hWnd, &cf, &font, RGB(0,0,0));
   cf.rgbColors = plot->title().fontColor();
   if(ChooseFont(&cf))
   {
		ProspaFont pfont(*(cf.lpLogFont));
		plot->title().setFont(pfont);
      plot->title().setFontSize(cf.iPointSize/10);
      plot->title().setFontColor(cf.rgbColors);         
      plot->title().setFontStyle(GetFontStyle(cf.lpLogFont));
		if(Plot::gApplyToAll)
      {
			for(Plot* p: plotList_)
	      {
            p->title().setFontColor(plot->title().fontColor());
            p->title().setFont(pfont);
            p->title().setFontStyle(plot->title().fontStyle());
         }
      }
		return true;
	}
	return false;
}


bool PlotWindow::StylePlotAxisLabels()
{
	CHOOSEFONT cf;
	Plot* plot = curPlot();
	Axis* xAxis = plot->curXAxis();
	LOGFONT font = plot->curXAxis()->label().font();   
   InitFont(XYLabelHookProc,"SETXYLABELDIALOG", hWnd, &cf, &xAxis->label().font(), RGB(0,0,0));
	cf.rgbColors = xAxis->label().fontColor();
   cf.lpLogFont = &font;
   if(ChooseFont(&cf))
   {
		ProspaFont pfont(*(cf.lpLogFont));
		for(Axis* axis: plot->axisList())
		{
			axis->label().setFont(pfont);
			axis->label().setFontSize(cf.iPointSize/10);
			axis->label().setFontColor(cf.rgbColors);
			axis->label().setFontStyle(GetFontStyle(cf.lpLogFont)); 
		}

      if(Plot::gApplyToAll)
      {
			for(Plot* p: plotList_)
	      {
            p->yLabelVert = plot->yLabelVert;
				for(Axis* axis: p->axisList())
				{
					axis->label().setFontColor(xAxis->label().fontColor());
					axis->label().setFont(pfont);
					axis->label().setFontStyle(xAxis->label().fontStyle());  
				}
         }
		}
		return true;
	}
	return false;
}

bool PlotWindow::StylePlotAxisTicksLabels()
{
   CHOOSEFONT cf;
	Plot* plot = curPlot();
	Axis* xAxis = plot->curXAxis();
   
	LOGFONT font = xAxis->ticks().font();
   InitFont(FontHookProc,"AXESFONTDIALOG", hWnd, &cf, &font, RGB(0,0,0));
   cf.rgbColors = xAxis->ticks().fontColor();
   cf.lpLogFont = &xAxis->ticks().font();
   strncpy_s(fontDltitle,MAX_STR,"Set Axes Font",_TRUNCATE);
   if(ChooseFont(&cf))
   {
		ProspaFont pfont(*(cf.lpLogFont));
		for(Axis* axis: plot->axisList())
		{
			axis->ticks().setFont(pfont);
			axis->ticks().setFontSize(cf.iPointSize/10);
			axis->ticks().setFontColor(cf.rgbColors);
			axis->ticks().setFontStyle(GetFontStyle(cf.lpLogFont));
		}
      if(Plot::gApplyToAll)
      {
			for(Plot* p: plotList_)
	      {
				for(Axis* axis: p->axisList())
				{
					axis->ticks().setFont(pfont);
					axis->ticks().setFontColor(xAxis->ticks().fontColor());
					axis->ticks().setFont(xAxis->ticks().pfont());
					axis->ticks().setFontStyle(xAxis->ticks().fontStyle());  
				}
         }
      } 
		return true;
	}
	return false;
}

short PlotWindow::SaveAsImage(char* inputFileName)
{
	HDC hdc;
	static char fileName[MAX_STR] = "untitled";
	static char directory[MAX_STR];
	static short index = 1;
	char oldDir[MAX_PATH];
	MSG msg;

	// Save the current working directory
	GetCurrentDirectory(MAX_PATH,oldDir);

	// Don't understand why this needs to be here but sometimes
	// the image file is not drawn correctly without it
	Sleep(10);

	// Allow user to choose filename
	if(inputFileName == NULL)
	{
		if(directory[0] == '\0')
			strncpy_s(directory,MAX_STR,plotDirectory(), _TRUNCATE);

		short err = FileDialog(hWnd, false, directory, fileName, 
			"Save plot as image", CentreHookSaveProc, NULL, noTemplateFlag,6,&index,
			"EMF","emf",
			"BMP","bmp",
			"TIF","tif",
			"GIF","gif",
			"JPG","jpg",
			"PNG","png");

		if(err != OK)
			return(ABORT);
	}
	else
		strncpy_s(fileName,MAX_STR,inputFileName,_TRUNCATE);


	// Get image type
	char extension[MAX_STR];
	GetExtension(fileName,extension);

	if(!strcmp(extension,"emf"))
		return(MakeWMF(fileName));

	CText mode;
	if(!strcmp(extension,"jpg"))
		mode = "image/jpeg";
	else if(!strcmp(extension,"tif"))
		mode = "image/tiff";
	else if(!strcmp(extension,"png") || !strcmp(extension,"gif") || !strcmp(extension,"bmp"))
		mode.Format("image/%s",extension);
	else
	{
		ErrorMessage("invalid extension for image save");
		return(ERR);
	}
	wchar_t *wMode = CharToWChar(mode.Str());
	// Convert image name
	wchar_t *wFileName = CharToWChar(fileName);

	gPlotMode = IMAGEFILE; //Prevent curplot indent from being drawn

   while(isBusy())
      PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);

 //  setBusyWithCriticalSection(true);

	// Draw plots to bitmap - scale bitmap if required
	hdc = GetDC(hWnd);
	if(bitmap) 
		DeleteObject(bitmap);
	RECT r;
	GetClientRect(hWnd,&r);
   long newWidth;
	GenerateBitMap((r.right-r.left)*gPlotSF,(r.bottom-r.top)*gPlotSF, &bitmap, hdc, newWidth);
	HDC hdcMem = CreateCompatibleDC(hdc);
	SelectObject(hdcMem,bitmap);
	for(Plot* plot: plotList_)
	{
		plot->scale(gPlotSF);
		plot->Display(hWnd,hdcMem);
      if(plot->displayColorScale() && showLabels)
				dynamic_cast<Plot2D*>(plot)->DrawColorScale(hdcMem, bitmap);
		plot->unscale();
	}
	DeleteDC(hdcMem);
	ReleaseDC(hWnd,hdc);

	// Convert to gdi+ and then save file
	Bitmap *bmp = Bitmap::FromHBITMAP(bitmap,NULL);
	CLSID pngClsid;
	GetEncoderClsid(wMode, &pngClsid);
	Status st = bmp->Save(wFileName, &pngClsid, NULL);
	SysFreeString(wFileName);
	SysFreeString(wMode);
   delete bmp;

	// Restore the directory
	SetCurrentDirectory(oldDir);

	gPlotMode = DISPLAY;

 //  setBusyWithCriticalSection(false);

	// Make sure plot is redraw correctly
	DisplayAll(false);

	// Test for errors
	if(st != Ok)
	{
		ErrorMessage("can't save image - GDI+ status error no. %d%",(int)st);
		return(ERR);
	}

	return(OK);
}

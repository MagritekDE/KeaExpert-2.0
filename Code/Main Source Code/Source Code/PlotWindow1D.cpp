#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)
#include <gdiplus.h> 
#include "defines.h"
#include "DefineWindows.h"
#include "stdafx.h"
#include "PlotWindow.h"
#include "bitmap.h"
#include "error.h"
#include "main.h"
#include "files.h"
#include "globals.h"
#include "guiModifyObjectParameters.h"
#include "guiWindowsEvents.h"
#include "import_export_utilities.h"
#include "metafile.h"
#include "plotwindefaults.h"
#include "print.h"
#include "prospaResource.h"
#include "string_utilities.h"
#include <shellapi.h>
#include <math.h>
#include <string>
#include <algorithm>
#include "memoryLeak.h"

using std::find;
using std::string;
using namespace Gdiplus;

// Definitions of nonconst static members.
PlotList PlotWindow1D::savedPlotList; // Array of plot information to save TODO - delete on exit!
PlotList PlotWindow1D::copiedPlotList;
string PlotWindow1D::fileDimensionString_ = "1D";

PlotWindow1D::PlotWindow1D()
: PlotWindow()
{
	plotFactory = new Plot1DFactory;
}

PlotWindow1D::PlotWindow1D(MouseMode mouseMode, HWND hWnd, PlotWinDefaults* pwd,
										ObjectData* obj, short w, short h)
										: PlotWindow()
{
	plotFactory = new Plot1DFactory;
   showLabels = true;
   showStatusBar = false;
   showToolBar = false;
   useBackingStore = true;
	this->mouseMode = mouseMode;
	this->hWnd = hWnd;
	InitialisePlot(1,1,pwd);
	this->obj = obj;
	HDC hdc = GetDC(hWnd);
   long newWidth;
	GenerateBitMap(w,h,&bitmap,hdc, newWidth);
	ReleaseDC(hWnd, hdc);
	DragAcceptFiles(hWnd, true);
}

PlotWindow1D::~PlotWindow1D()
{
	for(Plot* p: plotList_)
	{
		if(Plot1D::curPlot() == p)
			Plot1D::setNoCurPlot();
	}
	if(bitmap)
		DeleteObject(bitmap);
	bitmap = NULL;
	delete plotFactory;
}

/*****************************************************************************
*              Display all the plots in the plot window
*****************************************************************************/

void PlotWindow1D::DisplayAll(bool locked)
{
	RECT pr;
	HBITMAP bkBitMap;

// Don't draw unless update mode is true          
    if(!updatePlots())
       return;

// If called from another non-GUI thread then update in GUI thread
   if(GetCurrentThreadId() != mainThreadID)
   {
      SendMessage(this->hWnd,WM_USER,0,0);
      return; 
   }

   if(!locked)
	{
	//   printf("In critical section DISPLAYALL\n");
		EnterCriticalSection(&cs1DPlot);
		incCriticalSectionLevel();
	}

	GetClientRect(hWnd,&pr);

	HDC hdc = GetDC(hWnd); // prepare window for painting
	if(this->useBackingStore)
	{
		HDC hdcMem = CreateCompatibleDC(hdc);
		bkBitMap = (HBITMAP)SelectObject(hdcMem,this->bitmap);
		for(Plot* p: plotList_)
		{
			p->Display(hWnd,hdcMem);
		}

		BitBlt(hdc,pr.left,pr.top,pr.right-pr.left,pr.bottom-pr.top,hdcMem,pr.left,pr.top,SRCCOPY);
		SelectObject(hdcMem,bkBitMap);
		DeleteDC(hdcMem);
	}
	else // Just draw to screen directly
	{
		for(Plot* p: plotList_)
		{
			p->Display(hWnd,hdc);
		}
	}
	ReleaseDC(hWnd,hdc);

	if(!locked)
	{
		LeaveCriticalSection(&cs1DPlot);
		decCriticalSectionLevel();
		// printf("Leaving critical section DISPLAYALL\n");
	}
}


/*************************************************************************
*                         Save 1D plots to a file                        *
*************************************************************************/

short PlotWindow1D::SavePlots(char* pathName, char* fileName, long x, long y)
{
	FILE *fp;
	char oldDir[MAX_PATH];
	CText newDir;

   if(gReadOnly)
   {
      MessageBox(prospaWin,"This version of Prospa is readonly, please purchase a full licensed version if you wish to save files","Read-only warning",MB_OK | MB_ICONEXCLAMATION);
      ErrorMessage("This version of Prospa is read only");
      return(ERR);
   }

	// Save the current working directory as the following command changes it
	GetCurrentDirectory(MAX_PATH,oldDir);

	// Open file
	SetCurrentDirectory(pathName);      
	GetCurrentDirectory(newDir);      
	if(!(fp = fopen(fileName,"wb")))
	{
		SetCurrentDirectory(oldDir);
		ErrorMessage("Can't save file '%s'",fileName);
		return(ERR);
	}
	SetCurrentDirectory(oldDir);

	// Write data out file parameters in binary format
	long ti = 'PROS'; fwrite(&ti,4,1,fp);   // Owner
	ti = 'PL1D'; fwrite(&ti,4,1,fp);   // File type

	// Save data in each plot region
	if(x == -1 && y == -1)
	{
		fwrite(&this->rows,sizeof(short),1,fp);   // Number of plot region rows
		fwrite(&this->cols,sizeof(short),1,fp);   // Number of plot region columns

		for(Plot* p: plotList())
		{				
			if(p->save(fp) == ERR) // Save information about plot (row,col)
			{
				fclose(fp);
				return(ERR);
			}
         p->setFileName(fileName);
         p->setFilePath(newDir.Str());
		}
		for(Plot* p: plotList())
		{				
			if(p->SaveV3_8Info(fp) == ERR) // Save information after V3.70
			{
				fclose(fp);
				return(ERR);
			}
		}
	}
	else
	{
		if(x < 0 || x >= this->cols || y < 0 || y >= this->rows)
		{
			ErrorMessage("Invalid plot region");
			fclose(fp);
			return(ERR);
		}
		short n = 1;
		fwrite(&n,sizeof(short),1,fp);   // Number of plot region rows = 1
		fwrite(&n,sizeof(short),1,fp);   // Number of plot region columns = 1

		Plot* pd =  this->plotList_[y*this->cols + x];

		if(pd->save(fp) == ERR) // Save information about plot (row,col)
		{
			fclose(fp);
			return(ERR);
		}

      pd->setFileName(fileName);
      pd->setFilePath(newDir.Str());
	}

	fclose(fp);
	return(OK);
}	   


/********************************************************
    Events relating to the mouse scroll wheel
********************************************************/

void PlotWindow1D::ProcessScrollWheelEvents(HWND hWnd, short zDel, short fwKeys)
{
	PlotWindow::ProcessScrollWheelEvents(hWnd,zDel,fwKeys);
   curPlot()->Invalidate();
}


/********************************************************
*    Remove all subplots except the current one
********************************************************/

void PlotWindow1D::RemoveAllButCurrentPlot()
{  
	PlotWindow::RemoveAllButCurrentPlot();
	if(Plot1D::curPlot()->hasNoCurTrace())
	{
		Plot1D::curPlot()->setCurTrace();
	}
}

/********************************************************
*    Update the status bar with data set dimensions
********************************************************/

void PlotWindow1D::UpdateStatusBar()
{
	char str[MAX_STR];
	Plot1D* cur1DPlot = Plot1D::curPlot();

	if(cur1DPlot && cur1DPlot->curTrace())
	{
		sprintf(str,"%s: %ld points", cur1DPlot->curTrace()->typeString().c_str(), cur1DPlot->curTrace()->getSize());
		UpdateStatusWindow(cur1DPlot->win, 2, str);
	}
}

int PlotWindow1D::dim()
{
	return 1;
}


/********************************************************
*  Clear the current plot of all data
********************************************************/
void PlotWindow1D::ClearCurrentPlot()
{  
   short index;
	// Find current plot index *************   
   for(index = 0; index < rows*cols; index++)
   {
		if(plotList_[index] == Plot1D::curPlot())
      {
         break;
	   }
	}
	if (index >= rows * cols)
		return;

	makeCurrentDimensionalPlot(index);
	Plot1D::curPlot()->clearData();
   Plot1D::curPlot()->setOverRideAutoRange(false);

// Update global data pointers ************	
	makeCurrentDimensionalPlot();
	Plot1D::curPlot()->rowNr = 0;
	Plot1D::curPlot()->colNr = 0;
	if(Plot1D::curPlot()->hasNoCurTrace())
   {
		Plot1D::curPlot()->setCurTrace();
   }
} 

Plot* PlotWindow1D::curPlot()
{
	return Plot1D::curPlot();
}

void PlotWindow1D::GetCurrentPlot(CText &whichPlot, short &x, short &y, short &index)
{
	static CText type = "1d";
	PlotWindow::GetCurrentPlot(type, x, y, index);
}

// Copy the current plot to the clip board, first in
// bitmap form - preserving antialiasing and then 
// in EMF form so it can be decomposed in objects if desired
short PlotWindow1D::CopyPlotsToClipboard(bool multiplot) 
{

// Copy the image as a bitmap to the clupboard
	gPlotMode = CLIPBOARD; //Prevent curplot indent from being drawn
   if(CopyAsBitMapToClipBoard(multiplot,true) == ERR)
   {
      gPlotMode = DISPLAY;
      return(ERR);
   }
//
//// Make an enhanced metafile and save to clipboard 
//   HDC hdc = CreateEnhMetaFile(NULL,NULL,NULL,NULL);
//	if(hdc)
//	{
//	   gPlotMode = IMAGEFILE;
//
//		if(multiplot)
//		{
//			for(Plot* p: plotList_)
//			{
//				p->displayToEMF(hdc,hWnd);
//			}
//		}
//		else
//		{
//			curPlot()->displayToEMF(hdc,hWnd);
//		}
//
//      HENHMETAFILE hemf = CloseEnhMetaFile(hdc);
//
//      // Copy metafile to clipboard 
//      if(OpenClipboard(NULL))
//      {
//       //  EmptyClipboard(); 
//         SetClipboardData(CF_ENHMETAFILE,hemf); 
//         CloseClipboard();  
//      }
//
//      DeleteEnhMetaFile(hemf);     
//   }
//   else
//   {
//	   ErrorMessage("Can't copy 1D plot to clipboard");
//	}

// Make sure 1D plot is redraw correctly
   gPlotMode = DISPLAY;
	curPlot()->DisplayAll(false);
   
   return(OK);
}



// Copy all 1D plots or the current plot to the clipboard

short PlotWindow1D::CopyAsBitMapToClipBoard(bool multiplot, bool emptyFirst)
{
   if(!this->bitmap)
   {
	   ErrorMessage("Can't copy 1D plot to clipboard");
      return(ERR);
	}

// Draw plots to bitmap - scale bitmap if required
   HDC hdc = GetDC(hWnd);
	if(bitmap) 
		DeleteObject(bitmap);
   RECT r,rn;
	GetClientRect(hWnd,&r);
   long widthSrc;
   long width = (r.right-r.left)*gPlotSF;
   long height = (r.bottom-r.top)*gPlotSF;
	GenerateBitMap(width,height, &bitmap, hdc, widthSrc, true);
	HDC hdcMem = CreateCompatibleDC(hdc);
	SelectObject(hdcMem,this->bitmap);

// Fill bitmap bit border or background
   HBRUSH hBrush;
   if(this->showLabels) 
   	hBrush = (HBRUSH)CreateSolidBrush(curPlot()->borderColor);
   else
   	hBrush = (HBRUSH)CreateSolidBrush(curPlot()->bkColor);

   SetRect(&rn,0,0,width,height);
   FillRect(hdcMem,&rn,hBrush);
   DeleteObject(hBrush);

// Copy all plots to bitmap
	for(Plot* plot: this->plotList_)
   {
      plot->scale(gPlotSF);
		plot->Display(hWnd,hdcMem);
      plot->ResizePlotRegion(hWnd);
      plot->unscale();
   }

// Copy all plots or just the current one
   Bitmap *bmp;
   if(multiplot) // All plots
   {
      bmp = Bitmap::FromHBITMAP(this->bitmap,NULL); // Convert to gdi+
   }
   else // Current plot
   {

   // Copy from subplot in bitmap to output bitmap
      HBITMAP outputBM;
      short nrRows = this->rows;
      short nrCols = this->cols;
      short row = curPlot()->rowNr;
      short col = curPlot()->colNr;
      long width = (r.right-r.left + 1)/nrCols;
      long height = (r.bottom-r.top + 1)/nrRows;
      long newWidth;
      
   // Make subplot bitmap
      GenerateBitMap(width*gPlotSF,height*gPlotSF, &outputBM, hdc, newWidth, true);

      HDC dcDst = CreateCompatibleDC(hdc);
      SelectObject(dcDst, outputBM);

      SetRect(&rn,0,0,newWidth,height);
      FillRect(dcDst,&rn,hBrush);
      DeleteObject(hBrush);

      if(!this->showLabels) // To prevent edge lines appearing
      {
         width--;
         height--;
      }
      BitBlt(dcDst,0,0,width*gPlotSF,height*gPlotSF,hdcMem, col*(r.right-r.left+1)*gPlotSF/nrCols,row*(r.bottom-r.top+1)*gPlotSF/nrRows,SRCCOPY);
      bmp = Bitmap::FromHBITMAP(outputBM,NULL); // Convert to gdi+
      DeleteObject(outputBM);
	   DeleteDC(dcDst);
   }


// Copy to clip board
   HBITMAP hBitmap = NULL;
  // Get a handle for a 32bpp DIB from gdiplus
   if(bmp->GetHBITMAP(Gdiplus::Color(255,0,0,0), &hBitmap) == Gdiplus::Ok)
   {
      DIBSECTION ds;
      GetObject( hBitmap, sizeof(ds), &ds );
      ds.dsBmih.biCompression = BI_RGB; // change compression from
      // Convert the DIB to a device dependent bitmap(i.e., DDB)
      HDC hDC = GetDC(NULL);
      HBITMAP hDDB = CreateDIBitmap( hDC, &ds.dsBmih, CBM_INIT,
      ds.dsBm.bmBits, (BITMAPINFO*)&ds.dsBmih, DIB_RGB_COLORS );
      ReleaseDC(NULL, hDC);
      // Open the clipboard
      OpenClipboard(hWnd);
      if(emptyFirst)
         EmptyClipboard();
      // Place the handle to the DDB on the clipboard
      SetClipboardData(CF_BITMAP, hDDB);
      	
      // Do not delete the hDDB handle, the clipboard owns it
      CloseClipboard();
   }


// Clean up
	if(bitmap) 
		DeleteObject(bitmap);
   delete bmp;
   DeleteObject(hBitmap);

	GetClientRect(hWnd,&r);
   long newWidth;
	GenerateBitMap((r.right-r.left),(r.bottom-r.top), &bitmap, hdc, newWidth);

	DeleteDC(hdcMem);
   ReleaseDC(hWnd,hdc);

   return(OK);
}


string& PlotWindow1D::fileDimensionString()
{
	return fileDimensionString_;
}

long PlotWindow1D::fileDimensionCode()
{
	return fileDimensionCode_;
}


void PlotWindow1D::PrintDisplay(HDC hdcPrint)
{
	float xScale, yScale;
	ScaleToPrintedPage(hdcPrint, xScale, yScale);
	for(Plot* plot: plotList_)
	{
		plot->scale(xScale, yScale);
		plot->makeCurrentPlot();
		plot->makeCurrentDimensionalPlot();
      plot->Display(hWnd,hdcPrint);
		plot->unscale();
	}	    
}


void PlotWindow1D::clearPlotData(PlotList& plotList)
{	
	PlotList::iterator it;
	it = find(plotList.begin(), plotList.end(), Plot1D::curPlot());
	if (it != plotList.end())
	{
		Plot1D::setNoCurPlot();
	}
	PlotWindow::clearPlotData(plotList);
}

PlotList& PlotWindow1D::getPlotList()
{
	return plotList_;
}

PlotList& PlotWindow1D::getSavedPlotList()
{
	return savedPlotList;
}

short PlotWindow1D::MakeWMF(char* inputFileName)
{
	HDC hdc;
	static char fileName[MAX_STR] = "untitled";
	static char directory[MAX_STR];
	static short index = 1;
	char oldDir[MAX_PATH];

	// Save the current working directory
	GetCurrentDirectory(MAX_PATH,oldDir);

	// Allow user to choose filename
	if(inputFileName == NULL)
	{
		if(directory[0] == '\0')
			strncpy_s(directory,MAX_STR,PlotFile1D::getCurrPlotDirectory(),_TRUNCATE);

		short err = FileDialog(hWnd, false, directory, fileName, 
			"Save WMF", CentreHookProc, NULL, noTemplateFlag,
			1, &index, "Enhanced Metafile", "emf");

		if(err != OK)
			return(ABORT);
	}
	// Filename has been passed to function
	else
		strncpy_s(fileName,MAX_STR,inputFileName,_TRUNCATE);

	hdc = CreateEnhMetaFile(NULL,fileName,NULL,NULL);
	if(hdc)
	{
		RECT pr;

		GetClientRect(hWnd,&pr);

		gPlotMode = IMAGEFILE;

		for(Plot* plot: plotList())
		{
			plot->scale(gPlotSF);
			plot->Display(hWnd,hdc);
			plot->unscale();
		}
		HENHMETAFILE hemf = CloseEnhMetaFile(hdc);
		DeleteEnhMetaFile(hemf);
	}
	else
	{
		ErrorMessage("Can't write to file '%s'",fileName);
	}

	gPlotMode = DISPLAY;

	// Restore the directory
	SetCurrentDirectory(oldDir);

	// Make sure 1D plot is redraw correctly
	DisplayAll(false);

	return(OK);
	
}

int PlotWindow1D::LoadAndDisplayPlotFile(HWND hWnd, char *basePath, char *fileName)
{
	makeCurrentPlot();
	makeCurrentDimensionalPlot();

	if(LoadPlots(basePath, fileName) == ERR)
      return(ERR);

	SendMessageToGUI("1D Plot,LoadPlot",0);
	UpdateStatusWindow(hWnd,1,"Left button : select a region");
	Plot1D::curPlot()->statusText[0] = '\0';
	UpdateStatusWindow(hWnd,3,Plot1D::curPlot()->statusText);
	Plot1D::curPlot()->ResetZoomPoint();  
	short len = strlen(plotList()[0]->title().text());
	char *title = new char[len+20];
	sprintf(title,"Plot-1");
	Plot1D::curPlot()->setCurTrace(); 
	Plot1D::curPlot()->makeCurrentPlot();
	SetWindowText(hWnd,title); 
	delete [] title;

	RemoveCharFromString(Plot1D::curPlot()->statusText,'V');
	UpdateStatusWindow(hWnd,3,Plot1D::curPlot()->statusText);
	Plot1D::curPlot()->plotParent->clearPlotData(Plot1D::curPlot()->plotParent->getSavedPlotList());
	MyInvalidateRect(hWnd,NULL,false);
   return(OK);
}

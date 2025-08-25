#define WINVER _WIN32_WINNT 
#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)
#include <gdiplus.h> 
#include "stdafx.h"
#include "defines.h"
#include "defineWindows.h"
#include "PlotWindow.h"
#include "allocate.h"
#include "bitmap.h"
#include "error.h"
#include "files.h"
#include "globals.h"
#include "guiModifyObjectParameters.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include "import_export_utilities.h"
#include "message.h"
#include "metafile.h"
#include <math.h>
#include "mymath.h"
#include "plot.h"
#include "plot2dCLI.h"
#include "plotwindefaults.h"
#include "print.h"
#include "process.h"
#include "prospaResource.h"
#include "string_utilities.h"
#include "guiObjectClass.h"
#include <algorithm>
#include <shellapi.h>
#include <string>
#include "memoryLeak.h"

using std::string;
using std::for_each;
using namespace Gdiplus;

// Definitions of nonconst static members.
PlotList PlotWindow2D::savedPlotList; // Array of plot information to save TODO - delete on exit!
PlotList PlotWindow2D::copiedPlotList;
string PlotWindow2D::fileDimensionString_ = "2D";

PlotWindow2D::PlotWindow2D()
: PlotWindow()
{
	plotFactory = new Plot2DFactory;
}

PlotWindow2D::PlotWindow2D(MouseMode mouseMode, HWND hWnd, PlotWinDefaults* pwd,
									ObjectData* obj, short w, short h)
									: PlotWindow()
{
	plotFactory = new Plot2DFactory;
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
   GenerateBitMap(w,h, &bitmap, hdc, newWidth);	
   ReleaseDC(hWnd,hdc);
   DragAcceptFiles(hWnd,true);
}

PlotWindow2D::~PlotWindow2D()
{
	for(Plot* p: plotList_)
	{
		if(Plot2D::curPlot() == p)
			Plot2D::setNoCurPlot();
	}
	if(bitmap)
		DeleteObject(bitmap);
	bitmap = NULL;
	delete plotFactory;
}

/*****************************************************************************
*              Display all the images in the plot window
*****************************************************************************/

void PlotWindow2D::DisplayAll(bool locked)
{
	RECT pr;
	MSG msg;

// Don't draw unless update mode is true          
    if(!updatePlots())
       return;

   if(!locked)
   {
      EnterCriticalSection(&cs2DPlot);

      if(!isBusy())
	   {
         setBusy(true);
         LeaveCriticalSection(&cs2DPlot);
     
	      HDC hdc = GetDC(this->hWnd); // prepare window for painting
	      HDC hdcMem = CreateCompatibleDC(hdc);
	      SelectObject(hdcMem,this->bitmap);

	      GetClientRect(this->hWnd,&pr);

	      for(Plot* p: plotList_)
	      {
		      p->Display(hWnd,hdcMem);
	      }
	      BitBlt(hdc,pr.left,pr.top,pr.right-pr.left,pr.bottom-pr.top,hdcMem,pr.left,pr.top,SRCCOPY);
	      DeleteDC(hdcMem);
	      ReleaseDC(this->hWnd,hdc);

         setBusyWithCriticalSection(false);

      }
      else
         LeaveCriticalSection(&cs2DPlot);
   }
   else
   {
      HDC hdc = GetDC(this->hWnd); // prepare window for painting
      HDC hdcMem = CreateCompatibleDC(hdc);
      SelectObject(hdcMem,this->bitmap);

      GetClientRect(this->hWnd,&pr);

      for(Plot* p: plotList_)
      {
	      p->Display(hWnd,hdcMem);
      }
      BitBlt(hdc,pr.left,pr.top,pr.right-pr.left,pr.bottom-pr.top,hdcMem,pr.left,pr.top,SRCCOPY);
      DeleteDC(hdcMem);
      ReleaseDC(this->hWnd,hdc);
   }

}


/********************************************************
*    Remove all subplots except the specified one
********************************************************/

short PlotWindow::RemoveAllButOneSubPlot(short x, short y)
{
	bool currentDimensionalPlotFound = false;
	bool currentPlotFound = false;

	int index = (x-1) + (y-1)*cols;

	// Check to see if the index is value
	if(index >= rows*cols || index < 0)
	{
		ErrorMessage("Invalid sub-plot selection");
		return(ERR);
	}

	// See if the current plot is in this multiplot
	for(Plot* p: plotList_)
	{
		if (curPlot() == p)
			currentDimensionalPlotFound = true;
		if (Plot::curPlot() == p)
			currentPlotFound = true;
	}

	// Remove all but the selected plot
	if (index > 0){
		// Move the one we want to the front, and remove the existing reference
		plotList_.push_front(plotList_[index]);
		plotList_[0]->rowNr = 0;
		plotList_[0]->colNr = 0;
		plotList_[index+1] = NULL;
	}
	
	for_each( plotList_.begin() + 1, plotList_.end(), delete_object());
	plotList_.resize(1);
	rows = cols = 1;     

	// Update current plots if relevant
	if(currentDimensionalPlotFound)
	{
		this->makeCurrentDimensionalPlot();
		if (curPlot()->getDimension() == 1)
		{
			if (dynamic_cast<Plot1D*>(curPlot())->hasNoCurTrace())
			{
				dynamic_cast<Plot1D*>(curPlot())->setCurTrace();
			}
		}
		curPlot()->plotParent->clearPlotData(getSavedPlotList());
	}

	if(currentPlotFound)
	{
		curPlot()->makeCurrentPlot();
	}
	return(OK);
}

/********************************************************
*    Make nrX by nrY subplots updating them with
*    the previous subplot contents
********************************************************/

void PlotWindow::AdjustMultiPlotSize(short nrX, short nrY)
{
	PlotList& plotList = getPlotList();
	int remainder = nrX * nrY - plotList.size();
   Plot *pt;

	if(remainder > 0) // need to add some blank ones.
	{
		for (int i = 0; i < remainder; i++)
		{   
         pt = makePlot(pwd,curPlot()->win,this,0,0);
         pt->setDefaultParameters();
			plotList.push_back(pt);
		}
	}
	else if(remainder < 0) // need to remove from the end.
	{
		for_each( plotList.begin() + nrX*nrY, plotList.end(), delete_object());
		plotList.resize(nrX * nrY);
	}

// Update the row and column numbers for all regions
   rows = nrY;
   cols = nrX;
	ReassignCellCoordinates();
	makeCurrentPlot();
	makeCurrentDimensionalPlot();

	RemoveCharFromString(curPlot()->statusText,'V');
	UpdateStatusWindow(curPlot()->win,3,curPlot()->statusText);
	clearPlotData(getSavedPlotList());
   obj->winParent->setMenuItemCheck(menuName(),"hold",false);
   obj->winParent->setToolBarItemCheck(toolbarName(), "hold", false);
}


/*******************************************************************
  Initialise plot parent structure
********************************************************************/
void PlotWindow2D::InitialisePlot(short nrX, short nrY, PlotWinDefaults* pwd)
{
	PlotWindow::InitialisePlot(nrX,nrY,pwd);
	if(gDefaultColorMap)
	{
		for(short i = 0; i < rows; i++)
		{
	      for(short j = 0; j < cols; j++)
			{
				short k = cols*i + j;
         	static_cast<Plot2D*>(plotList_[k])->setColorMap(CopyMatrix(gDefaultColorMap,3,gDefaultColorMapLength));
				static_cast<Plot2D*>(plotList_[k])->setColorMapLength(gDefaultColorMapLength);
			}
      }
	}	
}


/********************************************************
    Events relating to the mouse scroll wheel
********************************************************/

bool gScrollWheelEvent = 0;

void PlotWindow2D::ProcessScrollWheelEvents(HWND hWnd, short zDel, short fwKeys)
{
   extern int gPlotViewVersion;

   gScrollWheelEvent = true;
	Plot* currentPlot = curPlot();
	
	HDC hdc = GetDC(hWnd);
	currentPlot->HideSelectionRectangle(hdc);  
   ReleaseDC(hWnd,hdc);

   if(gPlotViewVersion == 1)
   {
      if((fwKeys & MK_SHIFT) || (GetAsyncKeyState((int)'X') & 0x08000)) // Shift left and right
      {
         if(zDel < 0)
         {
            currentPlot->ShiftPlot(ID_SHIFT_LEFT);
         }
         else
         {
            currentPlot->ShiftPlot(ID_SHIFT_RIGHT);
         }
      }
      else if(fwKeys & MK_CONTROL) // Shift up and down
      {
         if(zDel < 0)
         {
            currentPlot->ShiftPlot(ID_SHIFT_UP);
         }
         else
         {
            currentPlot->ShiftPlot(ID_SHIFT_DOWN);
         }
      }
      else // Enlarge
      {
         if(zDel < 0)
         {
            currentPlot->ScalePlot(hWnd,ID_ENLARGE_BOTH);
         }
         else
         {
            currentPlot->ScalePlot(hWnd,ID_REDUCE_BOTH);
         }
      }
   }
   else if(gPlotViewVersion == 2) //  Expert
   {
      if((fwKeys & MK_SHIFT) || (GetAsyncKeyState((int)'X') & 0x08000)) // Shift left and right
      {
         if(zDel < 0)
         {
            currentPlot->ShiftPlot(ID_SHIFT_LEFT);
         }
         else
         {
            currentPlot->ShiftPlot(ID_SHIFT_RIGHT);
         }
      }
      else if(fwKeys & MK_CONTROL) // Shift up and down
      {
         if(zDel < 0)
         {
            currentPlot->ShiftPlot(ID_SHIFT_UP);
         }
         else
         {
            currentPlot->ShiftPlot(ID_SHIFT_DOWN);
         }
      }
      else  // Zoom in and out
      {
         if(zDel < 0)
         {
            currentPlot->ScalePlot(hWnd,ID_ENLARGE_BOTH);
         }
         else
         {
            currentPlot->ScalePlot(hWnd,ID_REDUCE_BOTH);
         }
      }
   }
   else if(gPlotViewVersion == 3) //  Expert
   {
      float fact;
      float minVal,maxVal;

      minVal = static_cast<Plot2D*>(currentPlot)->minVal();
      maxVal = static_cast<Plot2D*>(currentPlot)->maxVal();

      float newMinVal = minVal;
      float newMaxVal = maxVal;

      if(zDel>=0)
         fact = 0.9;
      else
         fact = 1.0/0.9;


      if((fwKeys & MK_SHIFT) || (GetAsyncKeyState((int)'X') & 0x08000)) // Change contour maximum value ('x' is more the Macintosh)
      {    
         newMaxVal = fact*maxVal;
         if(newMaxVal <= minVal)
            newMaxVal = maxVal;
         static_cast<Plot2D*>(currentPlot)->setMinVal(newMinVal);
         static_cast<Plot2D*>(currentPlot)->setMaxVal(newMaxVal);
         currentPlot->DisplayAll(false);  
      }
      else if(fwKeys & MK_CONTROL) // Zoom in and out
      {
         if(zDel < 0)
         {
            currentPlot->ScalePlot(hWnd,ID_ENLARGE_BOTH);
         }
         else
         {
            currentPlot->ScalePlot(hWnd,ID_REDUCE_BOTH);
         }
			SendMessageToGUI("2D Plot,Zoom", 0);
      }
      else // Change contour base
      {
         newMinVal = fact*minVal;
         if(newMinVal >= maxVal)
            newMinVal = minVal;
         static_cast<Plot2D*>(currentPlot)->setMinVal(newMinVal);
         static_cast<Plot2D*>(currentPlot)->setMaxVal(newMaxVal);
         currentPlot->DisplayAll(false);  
      }
   }
   Paint();
// Update 3D plot if spacebar is pressed
   if(GetAsyncKeyState(VK_SPACE) & 0x08000)
   {
      ProcessMacroStr(1,NULL,NULL,"surf2dParameters:plot_data","","","surf2dParameters.mac","");
   }
}



void PlotWindow2D::Paint()
{
	PAINTSTRUCT p;
	RECT pr;
	Plot2D* cur2DPlot = Plot2D::curPlot();

	GetClientRect(cur2DPlot->win,&pr);

	// Draw each image in the 2D array onto a bitmap & then copy to screen
	HDC hdc = BeginPaint(cur2DPlot->win, &p ); 

	HDC hdcMem = CreateCompatibleDC(hdc);
	SelectObject(hdcMem,this->bitmap);
	for(Plot* p: plotList_)
	{
		p->Display(cur2DPlot->win,hdcMem);
	}
	BitBlt(hdc,pr.left,pr.top,pr.right-pr.left,pr.bottom-pr.top,hdcMem,pr.left,pr.top,SRCCOPY);
	DeleteDC(hdcMem);

	EndPaint(cur2DPlot->win, &p );	   
}   

/********************************************************
*    Update the status bar with data set dimensions
********************************************************/

void PlotWindow2D::UpdateStatusBar()
{
	char str[MAX_STR];
	Plot2D* cur2DPlot = Plot2D::curPlot();

	if(cur2DPlot)
	{
		if(cur2DPlot->mat())
		{
			sprintf(str,"Real: %ld by %ld",cur2DPlot->matWidth(),cur2DPlot->matHeight());
			UpdateStatusWindow(cur2DPlot->win,2,str);
		}
		else if(cur2DPlot->cmat())
		{
			sprintf(str,"Complex: %ld by %ld",cur2DPlot->matWidth(),cur2DPlot->matHeight());
			UpdateStatusWindow(cur2DPlot->win,2,str);
		}
		else if(cur2DPlot->matRGB())
		{
			sprintf(str,"RGB: %ld by %ld",cur2DPlot->matWidth(),cur2DPlot->matHeight());
			UpdateStatusWindow(cur2DPlot->win,2,str);
		}
	}
}


int PlotWindow2D::dim()
{
	return 2;
}

// Delete the current 2D plot
void PlotWindow2D::CloseCurrentPlot()
{
	Plot2D* cur2DPlot = Plot2D::curPlot();

	int index = 0;
	for(index = 0; index < rows*cols; index++)
	{
	   if(cur2DPlot == plotList_[index])
	      break;
	}

	if (index >= rows*cols)
		return;
	else
	{
	   cur2DPlot->clearData();    
		cur2DPlot->clearColorMap();
	}
	
}


Plot* PlotWindow2D::curPlot()
{
	return Plot2D::curPlot();
}


void PlotWindow2D::GetCurrentPlot(CText &whichPlot, short &x, short &y, short &index)
{
	static CText type = "2d";
	PlotWindow::GetCurrentPlot(type, x, y, index);
}

// Remove any cursors currently on the 2D plot
void PlotWindow2D::ClearCursors(HDC hdc)
{
	Plot2D::curPlot()->ClearCursors(hdc,mouseMode);
}

// Copy all 2D plots or the current plot to the clipboard

short PlotWindow2D::CopyPlotsToClipboard(bool multiplot)
{
	RECT pr;

	short sbh=0;
   short width,height;
   short top = 0;

	MSG msg;
	while(isBusy())
      PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);

	setBusyWithCriticalSection(true);

// Copy the image as a bitmap
	gPlotMode = CLIPBOARD; //Prevent curplot indent from being drawn
   if(CopyAsBitMapToClipBoard(multiplot,true) == ERR)
   {
      gPlotMode = DISPLAY;
      return(ERR);
   }

//// Get the plot window dimensions (without status and toolbar)
//	GetClientRect(curPlot()->win,&pr);
//
//// Work out width and height of destination image
//	height = pr.bottom-pr.top - (sbh-1) - top;
//   width = pr.right-pr.left;
//
//// Make a new bit map for drawing the EMF
//   HDC hdc = GetDC(hWnd);
//   if(bitmap) 
//      DeleteObject(bitmap);
//   long newWidth;
//   GenerateBitMap((pr.right-pr.left+1)*gPlotSF,(pr.bottom-pr.top+1)*gPlotSF, &bitmap, hdc, newWidth);
//	DeleteObject(hdc);
//
//// Make an enhanced metafile
//   hdc = CreateEnhMetaFile(NULL,NULL,NULL,NULL);
//	if(hdc)
//	{
//		HDC hdcMem = CreateCompatibleDC(hdc);
//		SelectObject(hdcMem,bitmap);
//
//	   gPlotMode = IMAGEFILE;
//
//		for(Plot* p: plotList_)
//      {
//			p->displayToEMF(hdcMem, hWnd);
//      }
//
//   // Copy from the current window to the EMF bitmap
//
//      if(multiplot) // Copy all plots
//         BitBlt(hdc,0,0,width*gPlotSF,height*gPlotSF,hdcMem,0,0,SRCCOPY);
//      else // Just copy a single subplot
//      {
//         for(short q = 0; q < rows*cols; q++)
//         {
//      	   Plot* plot = plotList_[q];
//
//            if(plot == Plot::curPlot())
//            {
//               long  j = (int)(q/cols);
//               long i = q - j*cols;
//               long x = i*width/cols;
//               long y = j*height/rows;
//               long w = width/cols;
//               long h = height/rows;
//               BitBlt(hdc,0,0,w*gPlotSF,h*gPlotSF,hdcMem,x,y,SRCCOPY);
//               break;
//            }
//         }
//      }
//
//      DeleteDC(hdcMem);
// 	   
//      HENHMETAFILE hemf = CloseEnhMetaFile(hdc);
//
//      // Push DIB in clipboard 
//      if(OpenClipboard(NULL))
//      {
//      //   EmptyClipboard(); 
//         SetClipboardData(CF_ENHMETAFILE,hemf); 
//         CloseClipboard();  
//      }
//
//      DeleteEnhMetaFile(hemf);     
//   }
//   else
//   {
//      gPlotMode = DISPLAY;
//	   ErrorMessage("Can't copy 2D plot ");
//	}

// Restore normal display
 //  gPlotMode = DISPLAY;
 //  hdc = GetDC(hWnd);
 //  if(bitmap) 
 //     DeleteObject(bitmap);
 //  GenerateBitMap((pr.right-pr.left+1),(pr.bottom-pr.top+1), &bitmap, hdc, newWidth);
	//DeleteObject(hdc);

	setBusyWithCriticalSection(false);

// Make sure 2D plot cursors are correctly drawn
	gPlotMode = DISPLAY;
	curPlot()->DisplayAll(false);


   return(OK);
}

short PlotWindow2D::CopyAsBitMapToClipBoard(bool multiplot, bool emptyFirst)
{
   HBRUSH hBrush;

   if(!this->bitmap)
   {
	   ErrorMessage("Can't copy 2D plot to clipboard");
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
	GenerateBitMap(width,height, &bitmap, hdc, widthSrc);
	HDC hdcMem = CreateCompatibleDC(hdc);
	SelectObject(hdcMem,this->bitmap);

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

      if(this->showLabels) 
   	   hBrush = (HBRUSH)CreateSolidBrush(curPlot()->borderColor);
      else
   	   hBrush = (HBRUSH)CreateSolidBrush(curPlot()->bkColor);


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

/*************************************************************************
*                         Save 2D plots to a file                        *
*************************************************************************/
      
short PlotWindow2D::SavePlots(char *pathName, char *fileName, long x, long y)
{
   FILE *fp;
   long owner,type;
   char oldDir[MAX_PATH];
   CText newDir;

   if(gReadOnly)
   {
      MessageBox(prospaWin,"This version of Prospa is readonly, please purchase a full licensed version if you wish to save files.","Read-only warning",MB_OK | MB_ICONEXCLAMATION);
      ErrorMessage("This version of Prospa is read only");
      return(ERR);
   }

// Save the current working directory as the following command changes it
   GetCurrentDirectory(MAX_PATH,oldDir);

// Open file as binary for write
   SetCurrentDirectory(pathName);
   GetCurrentDirectory(newDir);

   if(!(fp = fopen(fileName,"wb")))
	{
      SetCurrentDirectory(oldDir);
	   ErrorMessage("Can't open file '%s'",fileName);
	   return(ERR);
	}
   SetCurrentDirectory(oldDir);

// Write data out file parameters in binary format
   owner = 'PROS'; fwrite(&owner,sizeof(long),1,fp);   // File owner
   type =  'PL2D'; fwrite(&type,sizeof(long),1,fp);    // File type

	// Save data in each plot region
	if(x == -1 && y == -1)
	{
		// Save the plot region organisation (rows and columns)
		fwrite(&this->rows,sizeof(short),1,fp);   // Number of plot region rows
		fwrite(&this->cols,sizeof(short),1,fp);   // Number of plot region columns

		// Save dimension of data set, number type and the data itself
		for(Plot* p: plotList_)
		{
			p->save(fp);
         p->setFileName(fileName);
         p->setFilePath(newDir.Str());
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

string& PlotWindow2D::fileDimensionString()
{
	return fileDimensionString_;
}

long PlotWindow2D::fileDimensionCode()
{
	return fileDimensionCode_;
}

void PlotWindow2D::PrintDisplay(HDC hdcPrint)
{
   HBITMAP prBitMap = NULL,oldBitMap = NULL;

	Plot2D* plot = 0;
	float xScale, yScale;
	
	ScaleToPrintedPage(hdcPrint, xScale, yScale);

	HDC hdcMemPr = CreateCompatibleDC(hdcPrint);
// Print the images first
	for(short i = 0; i < rows; i++)
	{
	   for(short j = 0; j < cols; j++)
	   {
	      plot = dynamic_cast<Plot2D*>(plotList_[i*cols+j]);
			plot->makeCurrentDimensionalPlot();
			plot->scale(xScale, yScale);
         if(plot->mat() || plot->cmat())
         {
            if(prBitMap) DeleteObject(prBitMap);
            long newWidth;
            GenerateBitMap(plot->visibleWidth(),plot->visibleHeight(), &prBitMap, hdcMemPr, newWidth);
            if(i == 0 && j == 0)
          	   oldBitMap = (HBITMAP)SelectObject(hdcMemPr,prBitMap);
          	else
          	  (HBITMAP)SelectObject(hdcMemPr,prBitMap);

			   if(plot->drawMode() & DISPLAY_IMAGE)
			   {
				   plot->FillBitMapWithImage2(prBitMap); 
				}
				else
				{
				   plot->FillBitMapWithColor(prBitMap,RGB(255,255,255));
				} 
				
            plot->ResizePlotRegion(hWnd);		          	  
                 
            short dl = plot->GetLeft(); short dt = plot->GetTop();
            short db = plot->GetBottom(); short dr = plot->GetRight();
	         StretchBlt(hdcPrint,dl,dt,dr-dl,db-dt,
	                    hdcMemPr,0,0,plot->visibleWidth(),plot->visibleHeight(),SRCCOPY);	
				plot->unscale();

         // Print the colorscale 
				if(plot->displayColorScale() && showLabels)
				{
               plot->ResizePlotRegion(hWnd);		          	  
				
				   long ww = plot->regionWidth();
				   long xl = plot->GetWidth() + plot->GetLeft() + ww*0.04;
				   long xr = plot->GetWidth() + plot->GetLeft() + ww*0.10;
				   long yt = plot->GetTop();
				   long yb = plot->GetBottom();
		                    
               if(prBitMap) DeleteObject(prBitMap);
               long newWidth;
               GenerateBitMap(xr-xl+1,yb-yt+1, &prBitMap, hdcMemPr, newWidth);
          	   SelectObject(hdcMemPr,prBitMap);

					if(plot->displayColorScale())
					   plot->PrintColorScaleA(hdcMemPr,prBitMap); 

					plot->scale(xScale, yScale);
					   
               plot->ResizePlotRegion(hWnd);		          	  

				   ww = plot->regionWidth();		                    
               dl = plot->GetWidth() + plot->GetLeft() + ww*0.04; dt = plot->GetTop();
               db = plot->GetBottom(); dr = plot->GetWidth()  + plot->GetLeft() + ww*0.10;
		         StretchBlt(hdcPrint,dl,dt,dr-dl,db-dt,
		                    hdcMemPr,0,0,xr-xl+1,yb-yt+1,SRCCOPY);	
					plot->unscale();
					   
			   }
	      }	      
	   }
	}	
		   
 //Print the labels etc
	if (oldBitMap)
	{
		SelectObject(hdcMemPr,oldBitMap);
	}
	for(Plot* p: plotList_)
	{
		Plot2D* plot = dynamic_cast<Plot2D*>(p);
		plot->makeCurrentPlot();
		plot->makeCurrentDimensionalPlot();

		plot->scale(xScale, yScale);
      
      plot->Display2DImageNullBMP(hWnd,hdcPrint);

		if(plot->displayColorScale() && showLabels)
		   plot->PrintColorScaleB(hdcPrint); 
      plot->unscale();
	}	    
	DeleteDC(hdcMemPr);
	DeleteObject(prBitMap);
}


void PlotWindow2D::clearPlotData(PlotList& plotList)
{
	PlotList::iterator it;
	it = std::find(plotList.begin(), plotList.end(), Plot2D::curPlot());
	if (it != plotList.end())
	{
		Plot2D::setNoCurPlot();
	}
	PlotWindow::clearPlotData(plotList);
}

PlotList& PlotWindow2D::getPlotList()
{
	return plotList_;
}

PlotList& PlotWindow2D::getSavedPlotList()
{
	return savedPlotList;
}

short PlotWindow2D::MakeWMF(char* inputFileName)
{
	HDC hdc;
	RECT pr;
	short sbh=0;
	static char fileName[MAX_STR] = "untitled";
	static char directory[MAX_STR];
	short width,height;
	short top = 0;
	static short index = 1;
	char oldDir[MAX_PATH];

	// Save the current working directory
	GetCurrentDirectory(MAX_PATH,oldDir);

	// Get the EMF filename from the user
	if(inputFileName == NULL)
	{
		if(directory[0] == '\0')
			strncpy_s(directory,MAX_STR,PlotFile2D::getCurrPlotDirectory(),_TRUNCATE);
		short err = FileDialog(hWnd, false, directory, fileName, 
			"Save WMF", CentreHookProc, NULL, noTemplateFlag, 
			1, &index, "Enhanced Metafile", "emf");  
		if(err != OK)
			return(ABORT);
	}
	// Filename has been passed to function
	else
		strncpy_s(fileName,MAX_STR,inputFileName,_TRUNCATE);


	// Get the plot window dimensions (without status and toolbar)
	GetClientRect(Plot2D::curPlot()->win,&pr);

	// Work out width and height of destination image
	height = pr.bottom-pr.top - (sbh-1) - top;
	width = pr.right-pr.left;

	// Make a new bit map for drawing the EMF
	hdc = GetDC(hWnd);  
	if(bitmap) 
		DeleteObject(bitmap);
   long newWidth;
	GenerateBitMap((pr.right-pr.left+1)*gPlotSF,(pr.bottom-pr.top+1)*gPlotSF, &bitmap, hdc, newWidth);

	// Make an enhanced metafile
	hdc = CreateEnhMetaFile(NULL,fileName,NULL,NULL);
	if(hdc)
	{
		HDC hdcMem = CreateCompatibleDC(hdc);

		SelectObject(hdcMem,bitmap);

		gPlotMode = IMAGEFILE;

		for(Plot* plot: plotList_)
		{
			plot->scale(gPlotSF);
			plot->Display(hWnd,hdcMem);
			if(plot->displayColorScale() && showLabels)
				dynamic_cast<Plot2D*>(plot)->DrawColorScale(hdcMem, bitmap);
			plot->unscale();		
		}

		// Copy from the current window to the EMF bitmap
		BitBlt(hdc,0,0,width*gPlotSF,height*gPlotSF,hdcMem,0,0,SRCCOPY);
		DeleteDC(hdcMem);
		HENHMETAFILE hemf = CloseEnhMetaFile(hdc); 
		DeleteEnhMetaFile(hemf);     
	}
	else
	{
		ErrorMessage("Can't write to file '%s'",fileName);
	}

	// Restore normal display
	gPlotMode = DISPLAY;
	hdc = GetDC(hWnd);
	if(bitmap) 
		DeleteObject(bitmap);
	GenerateBitMap((pr.right-pr.left+1),(pr.bottom-pr.top+1), &bitmap, hdc, newWidth);

	// Restore the directory
	SetCurrentDirectory(oldDir); 

	// Make sure 2D plot cursors are correctly drawn
	DisplayAll(false);

	return(OK);
}

int PlotWindow2D::LoadAndDisplayPlotFile(HWND hWnd, char *filePath, char *fileName)
{
	if(Plot2D::curPlot() == 0)
	{
		makeCurrentPlot();
		makeCurrentDimensionalPlot();
	}
	if(LoadPlots(filePath, fileName) == ERR)
      return(ERR);
	SendMessageToGUI("2D Plot,LoadImage",0);
	Plot2D* plot = Plot2D::curPlot();
	UpdateStatusWindow(hWnd,3,plot->statusText);
	plot->Invalidate();
   return(OK);
}
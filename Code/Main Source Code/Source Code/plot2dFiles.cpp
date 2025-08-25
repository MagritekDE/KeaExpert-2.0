#include "stdafx.h"
#include "plot2dFiles.h"
#include <math.h>
#include "allocate.h"
#include "cli_files.h"
#include "defineWindows.h"
#include "dialog_utilities.h"
#include "files.h"
#include "globals.h"
#include "import_export_utilities.h"
#include "message.h"
#include "mymath.h"
#include "plot_dialog_1d.h"
#include "plot2dCLI.h"
#include "plot.h"
#include "plotCLI.h"
#include "PlotFile.h"
#include "plotwindefaults.h"
#include "PlotWindow.h"
#include "prospaResource.h"
#include "memoryLeak.h"

// Locally defined functions

UINT  Load2DPlotHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

/************************************************************************
*         Displays modified Open File dialog for loading 2D plot data   *
*                                                                       *
* User passes pointer for returned directory & filename                 *
* Title for window                                                      *
* Filter range (eg "txt" for *.txt files or "all" for *.* files         *
************************************************************************/

short Load2DPlotDialog(HWND hWnd)
{
   short err;
   char filePath[MAX_PATH];
   static short index = 1;
   char oldDir[MAX_PATH];

// Check to make sure there is a 2D plot
   if(!Plot2D::curPlot())
   {
	   ErrorMessage("No current 2D plot");
	   return(ERR);
	}

// Save the current working directory
   GetCurrentDirectory(MAX_PATH,oldDir);

// Get the current name and path
	char* fileName = Plot2D::curPlot()->getFileName();
	strncpy_s(filePath,MAX_PATH,PlotFile2D::getCurrPlotDirectory(),_TRUNCATE);

   if(IsPlacesBarHidden())
   {
      err = FileDialog(hWnd, true, filePath, fileName, 
                      "Open a 2D plot", Load2DPlotHookProc, "LOAD2DPLOTDLG",
                       templateFlag, 1, &index, "2D Plot Files", "pt2");
   }
   else
   {
      err = FileDialog(hWnd, true, filePath, fileName, 
                      "Open a 2D plot", Load2DPlotHookProc, "LOAD2DPLOTDLG2",
                       templateFlag, 1, &index, "2D Plot Files", "pt2");
   }

               
   if(err == OK)
   {
   // Update plot directory
      GetCurrentDirectory(MAX_PATH,PlotFile2D::getCurrPlotDirectory());

   // Update the plots name and path
      strncpy_s(Plot2D::currFilePath,PLOT_STR_LEN,filePath,_TRUNCATE);
      strncpy_s(Plot2D::currFileName,PLOT_STR_LEN,fileName,_TRUNCATE);
   }

// Restore the current directory
   SetCurrentDirectory(oldDir);

   return(err);
}

/************************************************************************
*           Hook procedure to process events from load plot dialog      *
************************************************************************/

static bool plotOriginalColors = true;

UINT  Load2DPlotHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message)
	{
		case(WM_INITDIALOG):
		{
			SendDlgItemMessage(hWnd,ID_NEW_PLOT,BM_SETCHECK,(Plot2D::curPlot()->plotParent->getPlotInsertMode() == ID_NEW_PLOT),0); 
			SendDlgItemMessage(hWnd,ID_INSERT_BEFORE_PLOTS,BM_SETCHECK,(Plot2D::curPlot()->plotParent->getPlotInsertMode() == ID_INSERT_BEFORE_PLOTS),0); 
			SendDlgItemMessage(hWnd,ID_INSERT_AFTER_PLOTS,BM_SETCHECK,(Plot2D::curPlot()->plotParent->getPlotInsertMode() == ID_INSERT_AFTER_PLOTS),0); 
			SendDlgItemMessage(hWnd,ID_APPEND_PLOTS,BM_SETCHECK,(Plot2D::curPlot()->plotParent->getPlotInsertMode() == ID_APPEND_PLOTS),0); 
			SendDlgItemMessage(hWnd,ID_REPLACE_PLOTS,BM_SETCHECK,(Plot2D::curPlot()->plotParent->getPlotInsertMode() == ID_REPLACE_PLOTS),0); 
			SendDlgItemMessage(hWnd,ID_DEFAULT_COLS,BM_SETCHECK,(plotOriginalColors),0); 
         return false;
		}

		case(WM_NOTIFY): // Process events from standard part of dialog
		{
		   LPOFNOTIFY lpon = (LPOFNOTIFY)lParam;
		   switch(lpon->hdr.code)
		   {
		      case(CDN_INITDONE): // Std dialog has finished updating so centre it in window
		      {
               PlaceDialogOnTopInCentre(hWnd,lpon->hdr.hwndFrom);            
			      break;
			   }
			   case(CDN_FILEOK): // OK button has been pressed in std dialog
			   {
               Plot2D::curPlot()->plotParent->setPlotInsertMode(ID_NEW_PLOT*IsDlgButtonChecked(hWnd,ID_NEW_PLOT) +
                                ID_INSERT_BEFORE_PLOTS*IsDlgButtonChecked(hWnd,ID_INSERT_BEFORE_PLOTS)	+
                                ID_INSERT_AFTER_PLOTS*IsDlgButtonChecked(hWnd,ID_INSERT_AFTER_PLOTS)	+
                                ID_APPEND_PLOTS*IsDlgButtonChecked(hWnd,ID_APPEND_PLOTS)  +	      
                                ID_REPLACE_PLOTS*IsDlgButtonChecked(hWnd,ID_REPLACE_PLOTS));	      
               plotOriginalColors = IsDlgButtonChecked(hWnd,ID_DEFAULT_COLS);
               Plot2D::curPlot()->plotParent->setUseDefaultParameters(plotOriginalColors);
	         	break;
			   }
			}
         break;
	   }
      case(WM_DESTROY):
      {
         DialogTidyUp(hWnd,true);
         break;
      }
   }   
   return(0);
}

/************************************************************************
*         Displays modified Save File dialog for saving plot data       *
*                                                                       *
* User passes pointer for returned directory & filename                 *
* Title for window                                                      *
* Filter range (eg "txt" for *.txt files or "all" for *.* files         *
************************************************************************/

short Save2DPlotDialog(HWND hWnd, char *filePath, char *fileName, long &version)
{
   short err;
   static short index = 1;
   char oldDir[MAX_PATH];
   
// Save the current working directory
   GetCurrentDirectory(MAX_PATH,oldDir);

    err = FileDialog(hWnd, false, filePath, fileName, 
                   "Save a 2D plot", CentreHookProc, NULL,
                    noTemplateFlag, 8, &index,
                    "2D Plot File V3.7", "pt2",
                    "2D Plot File V3.6", "pt2",
                    "2D Plot File V3.5", "pt2",
                    "2D Plot File V3.4", "pt2",
                    "2D Plot File V3.3", "pt2",
                    "2D Plot File V3.2", "pt2",
                    "2D Plot File V3.0", "pt2",
                    "2D Plot File V2.1", "pt2");
 
   switch(index)
   {
      case(1):
           version = 370; break;
      case(2):
           version = 360; break;
      case(3):
           version = 350; break;
      case(4):
           version = 340; break;
      case(5):
           version = 330; break;
      case(6):
           version = 320; break;
      case(7):
           version = 300; break;
      case(8):
           version = 210; break;
      default:
           version = 370;
   }

// Update plot directory
   if(err == OK)
		GetCurrentDirectory(MAX_PATH,PlotFile2D::getCurrPlotDirectory());

// Restore the directory
   SetCurrentDirectory(oldDir);
   
   return(err);
}
 
#include "stdafx.h"
#include "plot_load_save_1d.h"
#include <math.h>
#include "allocate.h"
#include "cli_files.h"
#include "trace.h"
#include "dialog_utilities.h"
#include "files.h"
#include "globals.h"
#include "import_export_utilities.h"
#include "mymath.h"
#include "plot.h"
#include "PlotFile.h"
#include "plotwindefaults.h"
#include "PlotWindow.h"
#include "prospaResource.h"
#include "TracePar.h"
#include "variablesOther.h"
#include "memoryLeak.h"

/************************************************************************
*                             Load and save 1D data                     *
*                                                                       *
* OpenPlotDialog ... dialog to select native plot filename.             *
* Load1DPlots ...... load 1D plot data.                                 *
*    Load1DData .... load data specific to each data set.               *
*    Load1DPlotParameters .... load parameters specific to plot.        *
*    Load1DDataParameters .... load parameters specific to data set.    *
* Load1DDataFile ... 
************************************************************************/

UINT Load1DPlotHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

static bool plotOriginalColors = true;

/************************************************************************
*         Displays modified Load File dialog for loading plot data      *
*                                                                       *
* User passes pointer for returned directory path & filename            *
************************************************************************/

short Load1DPlotDialog(HWND hWnd)
{
   short err;
   char filePath[MAX_PATH];
   char* fileName;
   static short index = 1;
   char oldDir[MAX_PATH];

// Save the current working directory
   GetCurrentDirectory(MAX_PATH,oldDir);

// Get the current name and path
	fileName = Plot1D::curPlot()->getFileName();
	strncpy_s(filePath,MAX_PATH,PlotFile1D::getCurrPlotDirectory(),_TRUNCATE);
   
   if(IsPlacesBarHidden())
   {
      err = FileDialog(hWnd, true, filePath, fileName, 
                      "Open a 1D plot", Load1DPlotHookProc, "LOAD1DPLOTDLG", 
                      templateFlag, 1, &index, "1D Plot Files", "pt1");
   }
   else
   {
      err = FileDialog(hWnd, true, filePath, fileName, 
                      "Open a 1D plot", Load1DPlotHookProc, "LOAD1DPLOTDLG2", 
                      templateFlag, 1, &index, "1D Plot Files", "pt1");
   }

   if(err == OK)
   {
   // Update plot directory
		GetCurrentDirectory(MAX_PATH,PlotFile1D::getCurrPlotDirectory());

   // Update the plots name and path
		strncpy_s(Plot1D::currFilePath,PLOT_STR_LEN,filePath,_TRUNCATE);
      strncpy_s(Plot1D::currFileName,PLOT_STR_LEN,fileName,_TRUNCATE);
   }

// Restore the current directory
   SetCurrentDirectory(oldDir);
   
   return(err);
}


// Load/Save plot globals
static LPOFNOTIFY lpon;

/************************************************************************
*           Hook procedure to process events from load plot dialog      *
************************************************************************/


UINT Load1DPlotHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message)
	{
		case(WM_INITDIALOG):
		{
			SendDlgItemMessage(hWnd,ID_NEW_PLOT,BM_SETCHECK,(Plot1D::curPlot()->plotParent->getPlotInsertMode() == ID_NEW_PLOT),0); 
			SendDlgItemMessage(hWnd,ID_INSERT_BEFORE_PLOTS,BM_SETCHECK,(Plot1D::curPlot()->plotParent->getPlotInsertMode() == ID_INSERT_BEFORE_PLOTS),0); 
			SendDlgItemMessage(hWnd,ID_INSERT_AFTER_PLOTS,BM_SETCHECK,(Plot1D::curPlot()->plotParent->getPlotInsertMode() == ID_INSERT_AFTER_PLOTS),0); 
			SendDlgItemMessage(hWnd,ID_APPEND_PLOTS,BM_SETCHECK,(Plot1D::curPlot()->plotParent->getPlotInsertMode() == ID_APPEND_PLOTS),0); 
			SendDlgItemMessage(hWnd,ID_REPLACE_PLOTS,BM_SETCHECK,(Plot1D::curPlot()->plotParent->getPlotInsertMode() == ID_REPLACE_PLOTS),0); 
			SendDlgItemMessage(hWnd,ID_DEFAULT_COLS,BM_SETCHECK,(plotOriginalColors),0); 
	      break;
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

               Plot1D::curPlot()->plotParent->setPlotInsertMode(ID_NEW_PLOT*IsDlgButtonChecked(hWnd,ID_NEW_PLOT) +
                                ID_INSERT_BEFORE_PLOTS*IsDlgButtonChecked(hWnd,ID_INSERT_BEFORE_PLOTS)	+
                                ID_INSERT_AFTER_PLOTS*IsDlgButtonChecked(hWnd,ID_INSERT_AFTER_PLOTS)	+
                                ID_APPEND_PLOTS*IsDlgButtonChecked(hWnd,ID_APPEND_PLOTS)  +	      
                                ID_REPLACE_PLOTS*IsDlgButtonChecked(hWnd,ID_REPLACE_PLOTS));

               plotOriginalColors = IsDlgButtonChecked(hWnd,ID_DEFAULT_COLS);
               Plot1D::curPlot()->plotParent->setUseDefaultParameters(plotOriginalColors);
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
*         Displays modified Save File dialog for saving 1D plot data    *
*                                                                       *
* User passes pointer for returned directory path & filename            *
************************************************************************/

short Save1DPlotDialog(HWND hWnd, char *filePath, char *fileName, long &version)
{
   short err;
   static short index = 1;
   char oldDir[MAX_PATH];
   
// Save the current working directory
   GetCurrentDirectory(MAX_PATH,oldDir);

// Get the new path and filename
   err = FileDialog(hWnd, false, filePath, fileName, 
                   "Save a 1D plot", CentreHookProc, NULL,
                    noTemplateFlag, 6, &index,
                    "1D Plot File V3.8", "pt1",
                    "1D Plot File V3.7", "pt1",
                    "1D Plot File V3.6", "pt1",
                    "1D Plot File V3.5", "pt1",
                    "1D Plot File V3.4", "pt1",
                    "1D Plot File V3.3", "pt1",
                    "1D Plot File V3.2", "pt1");

   switch(index)
   {
      case(1):
           version = 380; break;
      case(2):
           version = 370; break;
      case(3):
           version = 360; break;
      case(4):
           version = 350; break;
      case(5):
           version = 340; break;
      case(6):
           version = 330; break;
      case(7):
           version = 320; break;
      default:
           version = 380;
   }

// Update plot directory
   if(err == OK)
      GetCurrentDirectory(MAX_PATH,PlotFile1D::getCurrPlotDirectory());

// Restore the directory
   SetCurrentDirectory(oldDir);
 
   return(err);   
}
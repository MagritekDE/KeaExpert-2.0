#include "stdafx.h"
#include "plot_dialog_1d.h"
#include "htmlhelp.h"
#include "trace.h"
#include "defineWindows.h"
#include "dialog_utilities.h"
#include "globals.h"
#include "interface.h"
#include "main.h"
#include "message.h"
#include "metafile.h"
#include "plot1dCLI.h"
#include "plot.h"
#include "print.h"
#include "prospaResource.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "memoryLeak.h"

/*****************************************************************************
*                                                                            *
*  Callback procedures for the various dialog boxes used by program          *
*                                                                            *
* AxesDlgProc ........ Sets tick and label parameters.                       *
* RangeDlgProc ....... Set range of displayed data.                          *
* PlotColorDlgProc ... Set colour of various parts of plot window.           *
* TraceColorDlgProc .. Sets color and type of trace.                         *
* SymbolDlgProc ...... Sets type and colour of symbol associated with data.  *
*                                                                            *
*****************************************************************************/

// Locally defined variables

static HWND gRangeHwnd;
static HWND gColorHwnd;
static HWND gSymbolHwnd;
static HWND gImportHwnd;
static HWND gGridHwnd;
static HWND gPrintHwnd;
static HWND gTextHwnd;
static HWND gEMFHwnd;
HWND gAxesHwnd;

static short axesX   =-1000,axesY;   // Axes dialog coordinates
static short rngX    =-1000,rngY;    // Range dialog coordinates
static short pClrX   =-1000,pClrY;   // Plot color dialog coordinates
static short traceX  =-1000,traceY;  // Trace dialog coordinates
short importX =-1000,importY; // Import dialog coordinates
static short exportX =-1000,exportY; // Export dialog coordinates
static short printX  =-1000,printY;  // Print dialog coordinates
static short gridX   =-1000,gridY;   // Grid dialog coordinates
static short emfX    =-1000,emfY;    // EMF dialog coordinates
static short textX   =-1000,textY;   // Text dialog coordinates

char fontDltitle[MAX_STR] = "Set Font";

short gMultiPlotXCells = 3,gMultiPlotYCells = 3;
short gMultiEditXCells = 3,gMultiEditYCells = 3;

void SetDialogPosition(short dialog, short x, short y)
{
   switch(dialog)
   {
      case(AXES_DLG):
         axesX = x;
         axesY = y;
         break;
      case(RANGE_DLG):
         rngX = x;
         rngY = y;
         break;
      case(PLOT_COLOR_DLG):
         pClrX = x;
         pClrY = y;
         break;
       case(TRACE_DLG):
         traceX = x;
         traceY = y;
         break;
       case(IMPORT_DLG):
         importX = x;
         importY = y;
         break;
       case(EXPORT_DLG):
         exportX = x;
         exportY = y;
         break;
       case(PRINT_DLG):
         printX = x;
         printY = y;
         break; 
    }
 }
 
void
GetDialogPosition(short dialog, short &x, short &y)
{
   switch(dialog)
   {
      case(AXES_DLG):
         x = axesX;
         y = axesY;
         break;
      case(RANGE_DLG):
         x = rngX;
         y = rngY;
         break;
      case(PLOT_COLOR_DLG):
         x = pClrX;
         y = pClrY;
         break;
       case(TRACE_DLG):
         x = traceX;
         y = traceY;
         break;
       case(IMPORT_DLG):
         x = importX;
         y = importY;
         break;
       case(EXPORT_DLG):
         x = exportX;
         y = exportY;
         break;
       case(PRINT_DLG):
         x = printX;
         y = printY;
         break; 
    }
 }

// Update all dialog items in the axes window

void
UpdateAxesDialog(Plot* plot)
{
   char str[AXIS_PARAM_LENGTH];
   
   if(gAxesHwnd)
   {
		plot->GetAxisParameter(ID_LABEL_SIZE,str);			       			
		SetDlgItemText(gAxesHwnd,ID_LABEL_SIZE,str);
		
		plot->GetAxisParameter(ID_XTICK_SPACING,str);
		SetDlgItemText(gAxesHwnd,ID_XTICK_SPACING,str);

		plot->GetAxisParameter(ID_YTICK_SPACING,str);		
		SetDlgItemText(gAxesHwnd,ID_YTICK_SPACING,str);

		plot->GetAxisParameter(ID_X_TICKS_PER_LABEL,str);		
		SetDlgItemText(gAxesHwnd,ID_X_TICKS_PER_LABEL,str);
		
		plot->GetAxisParameter(ID_Y_TICKS_PER_LABEL,str);		
		SetDlgItemText(gAxesHwnd,ID_Y_TICKS_PER_LABEL,str);
		
		plot->GetAxisParameter(ID_TICK_SIZE,str);		
		SetDlgItemText(gAxesHwnd,ID_TICK_SIZE,str);
		
		if(plot->curXAxis()->mapping() == PLOT_LINEAR_X)
		{
		   SendDlgItemMessage(gAxesHwnd,ID_LINEAR_X,BM_SETCHECK,1,0);
		   SendDlgItemMessage(gAxesHwnd,ID_LOG_X,BM_SETCHECK,0,0);
		}
		else
		{
		   SendDlgItemMessage(gAxesHwnd,ID_LINEAR_X,BM_SETCHECK,0,0);
		   SendDlgItemMessage(gAxesHwnd,ID_LOG_X,BM_SETCHECK,1,0);
		}

		if(plot->curYAxis()->mapping() == PLOT_LINEAR_Y)
		{
		   SendDlgItemMessage(gAxesHwnd,ID_LINEAR_Y,BM_SETCHECK,1,0);
		   SendDlgItemMessage(gAxesHwnd,ID_LOG_Y,BM_SETCHECK,0,0);
		}
		else
		{
		   SendDlgItemMessage(gAxesHwnd,ID_LINEAR_Y,BM_SETCHECK,0,0);
		   SendDlgItemMessage(gAxesHwnd,ID_LOG_Y,BM_SETCHECK,1,0);
		}
	}
}

// Update all the items in the region window

void
UpdateRegionDialog(Plot *plot)
{
   char str[20];
 
   if(gRangeHwnd)
   {  
		plot->GetAxisParameter(ID_MINX,str);
		SetDlgItemText(gRangeHwnd,ID_MINX,str);
		plot->GetAxisParameter(ID_MAXX,str);
		SetDlgItemText(gRangeHwnd,ID_MAXX,str);
		plot->GetAxisParameter(ID_MINY,str);
		SetDlgItemText(gRangeHwnd,ID_MINY,str);
		plot->GetAxisParameter(ID_MAXY,str);
		SetDlgItemText(gRangeHwnd,ID_MAXY,str);
	}
}


/*******************************************************************************
   Callback for copy to 1D variable dialog
*******************************************************************************/

int CALLBACK CopyTo1DVarDlgProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{  
	switch (message)
	{
		case(WM_INITDIALOG):
		{ 
	      SetWindowText(GetDlgItem(hWnd,ID_X_NAME),"x");
	      SetWindowText(GetDlgItem(hWnd,ID_Y_NAME),"y");
         PlaceDialogOnTopInCentre(hWnd);
         SaveDialogParent(hWnd); 
	      break;
		}	
		case(WM_CLOSE):
		{
         MyEndDialog(hWnd,1);
      	break;
		}		
		case(WM_COMMAND):
		{
         char varXName[50];
         char varYName[50];
		   
		   switch(LOWORD(wParam))
		   {
	       	case(ID_CANCEL):
	            MyEndDialog(hWnd,0);
	         	break;
	       	case(ID_APPLY):
				   GetDlgItemText(hWnd,ID_X_NAME,varXName,50);
				   GetDlgItemText(hWnd,ID_Y_NAME,varYName,50);
				   if(varXName[0] != '\0' && varYName[0] != '\0')
				   {
						Plot1D* cur1DPlot = Plot1D::curPlot();
				      if(cur1DPlot && cur1DPlot->curTrace())
				      {
				         cur1DPlot->curTrace()->CopyTo1DVarDlgProc(varXName, varYName);					   
					   }
					   MyEndDialog(hWnd,0);
	         	   break;	
	         	}
	         	break;	         	         	
	    	}
	      break;
		}
   }
	
	return(0);
}

/*******************************************************************************
   Callback for paste to 1D variable dialog
*******************************************************************************/

int CALLBACK Paste1DDataDlgProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{  
   char cmd[MAX_STR];
   
	switch (message)
	{
		case(WM_INITDIALOG):
		{ 
	      SetWindowText(GetDlgItem(hWnd,ID_X_NAME),"x");
	      SetWindowText(GetDlgItem(hWnd,ID_Y_NAME),"y");
			SendDlgItemMessage(hWnd,ID_CLEAR_PLOT,BM_SETCHECK,false,0); 	      
         PlaceDialogOnTopInCentre(hWnd);
         SaveDialogParent(hWnd); 
         break;
		}	
		case(WM_CLOSE):
		{
         MyEndDialog(hWnd,1);
      	break;
		}		
		case(WM_COMMAND):
		{
         char varXName[50];
         char varYName[50];
         bool holdBak;
		   
		   switch(LOWORD(wParam))
		   {
	       	case(ID_CANCEL):
	            MyEndDialog(hWnd,0);
	         	return(0);
	       	case(ID_APPLY):
				   GetDlgItemText(hWnd,ID_X_NAME,varXName,50);
				   GetDlgItemText(hWnd,ID_Y_NAME,varYName,50);
				   sprintf(cmd,"%s,%s",varXName,varYName);
					Plot1D* cur1DPlot = Plot1D::curPlot();
				   holdBak = cur1DPlot->displayHold;
               if(!SendDlgItemMessage(hWnd,ID_CLEAR_PLOT,BM_GETCHECK,0,0))
               	cur1DPlot->displayHold = true;	
               else
                  cur1DPlot->ResetPlotParameters();
               Interface itfc;
				   PlotXY(&itfc,cmd);
				   cur1DPlot->displayHold = holdBak;
					MyEndDialog(hWnd,0);
	         	break;		         	         	
	    	}
	      break;
		}
   }
	
	return(0);
}

/*******************************************************************************
   Callback for about Prospa dialog
*******************************************************************************/

int CALLBACK AboutDlgProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{  
	switch (message)
	{
		case(WM_INITDIALOG): // Centre window
		{ 
         PlaceDialogOnTopInCentre(hWnd);
         break;
		}		
		case(WM_CLOSE):
		{
         RemoveFromDiagList(hWnd,false);
         EndDialog(hWnd,1);
      	break;
		}
		case(WM_COMMAND):
		{
		   switch(LOWORD(wParam))
		   {
	       	case(IDOK):
               RemoveFromDiagList(hWnd,false);
               EndDialog(hWnd,0);
	         	break;
	    	}
	      break ;
		}
   }
	return(0);
}



/*******************************************************************************
   Callback for print dialog
*******************************************************************************/

int CALLBACK PrintDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   
   RECT r;
   
	switch (message)
	{
		case(WM_INITDIALOG):
		{
		   if(gPrintMode == COLOUR)
		   {
			   SendDlgItemMessage(hWnd,ID_PRINT_COLOR,BM_SETCHECK,true,0);
			   SendDlgItemMessage(hWnd,ID_PRINT_BW,BM_SETCHECK,false,0);
			}
			else
			{
			   SendDlgItemMessage(hWnd,ID_PRINT_COLOR,BM_SETCHECK,false,0);
			   SendDlgItemMessage(hWnd,ID_PRINT_BW,BM_SETCHECK,true,0);
			}
			SetDlgItemText(hWnd,ID_PRINT_WIDTH,GetPrintParameter(ID_PRINT_WIDTH));
			SetDlgItemText(hWnd,ID_PRINT_HEIGHT,GetPrintParameter(ID_PRINT_HEIGHT));
			SetDlgItemText(hWnd,ID_LONG_TICK_LENGTH,GetPrintParameter(ID_LONG_TICK_LENGTH));
			SetDlgItemText(hWnd,ID_SHORT_TICK_LENGTH,GetPrintParameter(ID_SHORT_TICK_LENGTH));			
			SetDlgItemText(hWnd,ID_SYMBOL_SIZE,GetPrintParameter(ID_SYMBOL_SIZE));			
								
         PlaceDialogOnTopInCentre(hWnd); 
         SaveDialogParent(hWnd);         			
	      break;
		}
		case(WM_MOVE):
		{
         GetWindowRect(hWnd,&r);
         printX = r.left;
         printY = r.top;
		   break;
		}
		case(WM_CLOSE):
		{
	      MyEndDialog(hWnd,0);
      	break;
		}
		case(WM_COMMAND):
		{
		   switch(LOWORD(wParam))
		   {
	       	case(ID_CANCEL):
	            MyEndDialog(hWnd,0);
	         	break;         	
	         case(ID_APPLY): 
	         {
	            char str[25];
	            float par;
               if(SendDlgItemMessage(hWnd,ID_PRINT_COLOR,BM_GETCHECK,0,0))
                  gPrintMode = COLOUR;
               else
                  gPrintMode = BLACK_AND_WHITE;
                  
               if(SendDlgItemMessage(hWnd,ID_FIT_TO_PAGE,BM_GETCHECK,0,0))
                  gFitToPage = true;
               else
                  gFitToPage = false;
               
				   GetDlgItemText(hWnd,ID_PRINT_WIDTH,str,20);
				   sscanf(str,"%f",&par);
				   gPrintWidth = par;
				   GetDlgItemText(hWnd,ID_PRINT_HEIGHT,str,20);
				   sscanf(str,"%f",&par);		
				   gPrintHeight = par;
				   GetDlgItemText(hWnd,ID_LONG_TICK_LENGTH,str,20);
				   sscanf(str,"%f",&par);
				   gPrintLongTickLength = par;
				   GetDlgItemText(hWnd,ID_SHORT_TICK_LENGTH,str,20);
				   sscanf(str,"%f",&par);		
				   gPrintShortTickLength = par;		
				   GetDlgItemText(hWnd,ID_SYMBOL_SIZE,str,20);
				   sscanf(str,"%f",&par);		
				   gPrintSymbolSize = par;						   		   
	            MyEndDialog(hWnd,1);
	         	break;
	         }
	         case(ID_FIT_TO_PAGE):
	         {
               if(SendDlgItemMessage(hWnd,ID_FIT_TO_PAGE,BM_GETCHECK,1,0))
               {
			         EnableWindow(GetDlgItem(hWnd,ID_PRINT_WIDTH),false);
			         EnableWindow(GetDlgItem(hWnd,ID_PRINT_HEIGHT),false);
			      }
			      else
			      {
			         EnableWindow(GetDlgItem(hWnd,ID_PRINT_WIDTH),true);
			         EnableWindow(GetDlgItem(hWnd,ID_PRINT_HEIGHT),true);
			      }
			   }
	     	}
	      break;
      }
   }
   return(0);
}

/*******************************************************************************
   Callback for 1D/2D "m by n" multi-plot window selection
*******************************************************************************/

int CALLBACK MultiPlotDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   
	switch (message)
	{
		case(WM_INITDIALOG):
		{
		   char str[20];
		   sprintf(str,"%hd",gMultiPlotXCells);
	      SetDlgItemText(hWnd,MULTIPLOT_X_CELLS,str);
		   sprintf(str,"%hd",gMultiPlotYCells);
	      SetDlgItemText(hWnd,MULTIPLOT_Y_CELLS,str);
         PlaceDialogOnTopInCentre(hWnd);
         SaveDialogParent(hWnd);
	      break;
		}

		case(WM_CLOSE):
		{
	      MyEndDialog(hWnd,1);
      	break;
		}

		case(WM_COMMAND):
		{
		   switch(LOWORD(wParam))
		   {
	       	case(ID_CANCEL):
	       	{
	            MyEndDialog(hWnd,1);
	         	break; 
	         }        	
	         case(ID_APPLY): 
	         {
	            char str[25];
				    
				   GetDlgItemText(hWnd,MULTIPLOT_X_CELLS,str,20);
				   sscanf(str,"%hd",&gMultiPlotXCells);
				   GetDlgItemText(hWnd,MULTIPLOT_Y_CELLS,str,20);
				   sscanf(str,"%hd",&gMultiPlotYCells);		   
	            MyEndDialog(hWnd,0);
	         	break;
	         }
	     	}
	      break;
      }
   }
   return(0);
}


/*******************************************************************************
   Callback for editor "m by n" multi window selection
*******************************************************************************/

int CALLBACK MultiEditDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   
	switch (message)
	{
		case(WM_INITDIALOG):
		{
		   char str[20];
		   sprintf(str,"%hd",gMultiEditXCells);
	      SetDlgItemText(hWnd,MULTIEDIT_X_CELLS,str);
		   sprintf(str,"%hd",gMultiEditYCells);
	      SetDlgItemText(hWnd,MULTIEDIT_Y_CELLS,str);
         PlaceDialogOnTopInCentre(hWnd);
         SaveDialogParent(hWnd);
	      break;
		}

		case(WM_CLOSE):
		{
	      MyEndDialog(hWnd,1);
      	break;
		}

		case(WM_COMMAND):
		{
		   switch(LOWORD(wParam))
		   {
	       	case(ID_CANCEL):
	       	{
	            MyEndDialog(hWnd,1);
	         	break; 
	         }        	
	         case(ID_APPLY): 
	         {
	            char str[25];
				    
				   GetDlgItemText(hWnd,MULTIEDIT_X_CELLS,str,20);
				   sscanf(str,"%hd",&gMultiEditXCells);
				   GetDlgItemText(hWnd,MULTIEDIT_Y_CELLS,str,20);
				   sscanf(str,"%hd",&gMultiEditYCells);		   
	            MyEndDialog(hWnd,0);
	         	break;
	         }
	     	}
	      break;
      }
   }
   return(0);
}

// Initialize choosefont structure with logfont entry

void InitFont(LPCFHOOKPROC hookProc, LPCTSTR dialogTemplate, HWND hWnd, CHOOSEFONT *cf, LOGFONT *lf, COLORREF color)
{
	 cf->lStructSize    = sizeof(CHOOSEFONT);
	 cf->hwndOwner      = Plot::curPlot()->win;
	 cf->hDC            = NULL;
	 cf->lpLogFont      = lf;
	 cf->iPointSize     = 0;
	 cf->Flags          = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS | CF_ENABLEHOOK |
	                      CF_SCALABLEONLY	| CF_NOSCRIPTSEL	| CF_EFFECTS | CF_ENABLETEMPLATE;
	 cf->rgbColors      = color;
	 cf->lCustData      = 0;
	 cf->lpfnHook       = hookProc;
	 cf->lpTemplateName = dialogTemplate;
	 cf->hInstance      = prospaInstance;
	 cf->lpszStyle      = NULL;
	 cf->nFontType      = 0;      
	 cf->nSizeMin       = 0;
	 cf->nSizeMax       = 0;
}

/*******************************************************************************
   Convert the font style - italics, underline and bold states to a code
*******************************************************************************/

short GetFontStyle(LOGFONT *font)
{
   short style = (font->lfItalic!=0)*FONT_ITALIC + (font->lfUnderline!=0)*FONT_UNDERLINE + (font->lfWeight==FW_BOLD)*FONT_BOLD;
   return(style);
}
      
/*******************************************************************************
   Add code to title dialog to initialise title and extract new one
*******************************************************************************/

UINT APIENTRY TitleHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
   switch(message)
   {
      case(WM_INITDIALOG):
	   {
         PlaceDialogOnTopInCentre(hWnd); 
         SaveDialogParent(hWnd);
			SetDlgItemText(hWnd,ID_TITLE_TEXT,Plot::curPlot()->title().text());
         break;
	   }
      case(WM_COMMAND):
      {
		   switch(LOWORD(wParam))
		   {
		      case(IDOK):
		      {
					char title[PLOT_STR_LEN];
					strncpy_s(title, PLOT_STR_LEN, Plot::curPlot()->title().text(), _TRUNCATE);
					GetDlgItemText(hWnd,ID_TITLE_TEXT,title,PLOT_STR_LEN);
					Plot::curPlot()->title().userSetText(title);
		         Plot::gApplyToAll = SendDlgItemMessage(hWnd,ID_ALL,BM_GETCHECK,0,0);
               DialogTidyUp(hWnd,false);
		         break;
		      }
            case(IDCANCEL):
            {
               DialogTidyUp(hWnd,false);
		         break;
            }
		   }
         break;
      }
      case(WM_DESTROY):
      {
         DialogTidyUp(hWnd,false);
         break;
      }
   }
	return(0);
}

/*******************************************************************************
   Add code to xy dialog to initialise x-y labels and extract new ones
*******************************************************************************/

UINT APIENTRY XYLabelHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{ 
	Plot* curPlot = Plot::curPlot();
   switch(message)
   {
      case(WM_INITDIALOG):
	   {
         PlaceDialogOnTopInCentre(hWnd);
         SaveDialogParent(hWnd);
		   SetDlgItemText(hWnd,ID_XLABEL_TEXT,curPlot->curXAxis()->label().text());
		   SetDlgItemText(hWnd,ID_Y_LEFT_LABEL_TEXT,curPlot->yAxisLeft()->label().text());
		   SetDlgItemText(hWnd,ID_Y_RIGHT_LABEL_TEXT,curPlot->yAxisRight()->label().text());
		   SendDlgItemMessage(hWnd,ID_VERT_Y_LABEL,BM_SETCHECK,curPlot->yLabelVert ,0);
		   SendDlgItemMessage(hWnd,ID_HORIZ_Y_LABEL,BM_SETCHECK,!(curPlot->yLabelVert) ,0);
	      break;
      }
      case(WM_COMMAND):
      {
		   switch(LOWORD(wParam))
		   {
		      case(IDOK):
		      {
					char text[PLOT_STR_LEN];
					strncpy_s(text,PLOT_STR_LEN,curPlot->curXAxis()->label().text(), _TRUNCATE);
		         GetDlgItemText(hWnd,ID_XLABEL_TEXT,text,PLOT_STR_LEN);
					curPlot->curXAxis()->label().userSetText(text);

               strncpy_s(text,PLOT_STR_LEN,curPlot->yAxisLeft()->label().text(), _TRUNCATE);
		         GetDlgItemText(hWnd,ID_Y_LEFT_LABEL_TEXT,text,200);
					curPlot->yAxisLeft()->label().userSetText(text);

					strncpy_s(text,PLOT_STR_LEN,curPlot->yAxisRight()->label().text(), _TRUNCATE);
		         GetDlgItemText(hWnd,ID_Y_RIGHT_LABEL_TEXT,text,200);
					curPlot->yAxisRight()->label().userSetText(text);

		         curPlot->yLabelVert = SendDlgItemMessage(hWnd,ID_VERT_Y_LABEL,BM_GETCHECK,0,0);
		         Plot::gApplyToAll = SendDlgItemMessage(hWnd,ID_ALL,BM_GETCHECK,0,0);
               DialogTidyUp(hWnd,false);
		         break;
		      }
            case(IDCANCEL):
               DialogTidyUp(hWnd,false);
		         break;
		   }
         break;
      }
   }
	return(0);
}

/*******************************************************************************
   Add code to axes font dialog to initialise axes label font and extract new one
*******************************************************************************/

UINT APIENTRY FontHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
   switch(message)
   {
      case(WM_INITDIALOG):
	   {
         PlaceDialogOnTopInCentre(hWnd); 
         SaveDialogParent(hWnd);
         SetWindowText(hWnd,fontDltitle);
         break;
	   }
      case(WM_COMMAND):
      {
		   switch(LOWORD(wParam))
		   {
		      case(IDOK):
		      {
		         Plot::gApplyToAll = SendDlgItemMessage(hWnd,ID_ALL,BM_GETCHECK,0,0);
               DialogTidyUp(hWnd,false);
		         break;
		      }
            case(IDCANCEL):
            {
               DialogTidyUp(hWnd,false);
		         break;
            }
		   }
         break;
      }
      case(WM_DESTROY):
      {
         DialogTidyUp(hWnd,false);
         break;
      }
   }
	return(0);
}

// Load and save dialog positions to the parameter file

void LoadDlgParameters(FILE *fp)
{
   short err = 0;
	// Supplying field width limit of 6, since the values we are 
	// reading in here are all shorts (ie, ranging from -32768 to 32767)
   err += fscanf(fp,"axesDlgPos    = (%6hd,%6hd)\n",&axesX,&axesY);
   err += fscanf(fp,"rangeDlgPos   = (%6hd,%6hd)\n",&rngX,&rngY);
   err += fscanf(fp,"plotColDlgPos = (%6hd,%6hd)\n",&pClrX,&pClrY);
   err += fscanf(fp,"traceDlgPos   = (%6hd,%6hd)\n",&traceX,&traceY);
   err += fscanf(fp,"importDlgPos  = (%6hd,%6hd)\n",&importX,&importY);
   err += fscanf(fp,"exportDlgPos  = (%6hd,%6hd)\n",&exportX,&exportY);
   err += fscanf(fp,"printDlgPos   = (%6hd,%6hd)\n",&printX,&printY);
   err += fscanf(fp,"gridDlgPos    = (%6hd,%6hd)\n",&gridX,&gridY);
   err += fscanf(fp,"emfDlgPos     = (%6hd,%6hd)\n",&emfX,&emfY);
   err += fscanf(fp,"textDlgPos    = (%6hd,%6hd)\n",&textX,&textY);
   if(err != 20)
   {
      MessageDialog(prospaWin,MB_ICONERROR,"Error","Invalid plot parameter file (dialog position)");
      exit(0);
   }
} 

void SaveDlgParameters(HWND hWnd, FILE *fp)
{
   fprintf(fp,"axesDlgPos    = (%hd,%hd)\n",axesX,axesY);
   fprintf(fp,"rangeDlgPos   = (%hd,%hd)\n",rngX,rngY);
   fprintf(fp,"plotColDlgPos = (%hd,%hd)\n",pClrX,pClrY);
   fprintf(fp,"traceDlgPos   = (%hd,%hd)\n",traceX,traceY);
   fprintf(fp,"importDlgPos  = (%hd,%hd)\n",importX,importY);
   fprintf(fp,"exportDlgPos  = (%hd,%hd)\n",exportX,exportY);
   fprintf(fp,"printDlgPos   = (%hd,%hd)\n",printX,printY);   
   fprintf(fp,"gridDlgPos    = (%hd,%hd)\n",gridX,gridY);
   fprintf(fp,"emfDlgPos     = (%hd,%hd)\n",emfX,emfY);
   fprintf(fp,"textDlgPos    = (%hd,%hd)\n",textX,textY);   
} 



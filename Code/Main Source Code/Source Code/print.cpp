#include "stdafx.h"
#include "print.h"
#include "bitmap.h"
#include "trace.h"
#include "dialog_utilities.h"
#include "edit_class.h"
#include "font.h"
#include "globals.h"
#include "plot.h"
#include "PlotWindow.h"
#include "prospaResource.h"
#include "memoryLeak.h"

UINT APIENTRY PrintHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

float   gPrintWidth  = 100; // Print parameters
float   gPrintHeight = 100;
float   gPrintLongTickLength  = 2; 
float   gPrintShortTickLength = 1;
float   gPrintSymbolSize = 3; 
short   gTitleFontSize = 12;
bool    gFitToPage   = false;
short   gPrintMode   = BLACK_AND_WHITE;
short   gPlotMode    = DISPLAY;

HWND hdlgCancel;

/*******************************************************************
             Send plot information to the printer
*******************************************************************/

short PrintDisplay(PlotWindow *pp)
{
   static PRINTDLG prd;

   HWND hWnd = pp->hWnd;

// Initialize the PRINTDLG members 
	prd.lStructSize = sizeof(PRINTDLG); 
	prd.Flags = PD_RETURNDC | PD_ENABLEPRINTHOOK; 
	prd.hwndOwner = hWnd; 
	prd.nCopies = 1; 
   prd.lpfnPrintHook = PrintHookProc; 
	 
// Display the PRINT dialog box. 
	if(!PrintDlg(&prd))
	   return(OK); 
	
// Print window
	
   HDC hdcPrint = prd.hDC;
   static DOCINFO di = { sizeof (DOCINFO), TEXT ("") } ;

	if(StartDoc(hdcPrint,&di) > 0)
	{
	   if(StartPage(hdcPrint) > 0)
	   {
         gPlotMode = PRINT;		

			pp->PrintDisplay(hdcPrint);

         gPlotMode = DISPLAY;
			
			if(EndPage(hdcPrint) > 0)
			   EndDoc(hdcPrint);
	   }
	}
	   
	DeleteDC(hdcPrint);
   return(OK);
}

/*******************************************************************
             Send 3D plot information to the printer
*******************************************************************/

//short Print3DDisplay(HWND hWnd)
//{
//   static PRINTDLG prd;
//   HDC            hDC;
//   DOCINFO        di = { sizeof (DOCINFO), TEXT ("") };
//   HGLRC          hglrc;
//   HBITMAP prBitMap = NULL,oldBitMap;
//
//// Initialize the PRINTDLG members 
//	prd.lStructSize = sizeof(PRINTDLG); 
//	prd.Flags = PD_RETURNDC | PD_ENABLEPRINTHOOK; 
//	prd.hwndOwner = hWnd; 
//	prd.nCopies = 1; 
//   prd.lpfnPrintHook = PrintHookProc; 
//	 
//// Display the PRINT dialog box. 
//	if(!PrintDlg(&prd))
//	   return(OK); 
//	
//// Print window
////	curPlotBack = cur3DPlot;
//	
//   HDC hdcPrint = prd.hDC;
//
//
//// Call StartDoc to start the document
//   StartDoc( hDC, &di );
//
//// Prepare the printer driver to accept data
//   StartPage(hDC);
//
//// Create a rendering context using the metafile DC
// //  hglrc = wglCreateContext(hDCmetafile);
//
//// Do OpenGL rendering operations here
// //   . . .
//   wglMakeCurrent(NULL, NULL); // Get rid of rendering context
////    . . .
//   EndPage(hDC); // Finish writing to the page
//   EndDoc(hDC); // End the print job
//
//	DeleteDC(hdcPrint);
//
////	cur3DPlot = curPlotBack;
//
//   return(OK);
//}

// Print macro text - need to add some other features available from the print dialog
// see if I can get PrintDlgEx working

short PrintText(HWND hWnd)
{
   
   static PRINTDLG prd;
   TEXTMETRIC tm;
   static DOCINFO di = { sizeof (DOCINFO), TEXT ("FormFeed") } ;
   short index,nrLines;
   char *text;
   char str[MAX_PATH];
   EditRegion *editor;
   HFONT printFont;
   bool success = true;
   short titleOff;

   if(!curEditor)
   {   
      ErrorMessage("No current editor selected");
      return(ERR);
   }
     
// Initialize the PRINTDLG members 
	prd.lStructSize = sizeof(PRINTDLG); 
	prd.Flags = PD_RETURNDC | PD_ENABLEPRINTHOOK; 
	prd.hwndOwner = hWnd; 
	prd.nCopies = 1; 
   prd.lpfnPrintHook =  PrintHookProc; 
 
// Display the PRINT dialog box. 
	if(!PrintDlg(&prd))
	   return(OK); 

// Get information from current text editor
   EditParent *ep = curEditor->edParent;
   index = ep->curRegion;
   editor = ep->editData[index];
   nrLines = SendMessage(editor->edWin,EM_GETLINECOUNT,(WPARAM)0,(LPARAM)0);
   
// Print document
   HDC hdcPrint = prd.hDC;
   printFont = MakeFont(hdcPrint, "Courier New", 9, 0, 0, 0);				                      
   SelectObject(hdcPrint,printFont);
   GetTextMetrics(hdcPrint,&tm);
   int charHeight = tm.tmHeight + tm.tmExternalLeading;
   titleOff = charHeight*1.5;   
   int linesPerPage = (GetDeviceCaps(hdcPrint,VERTRES)- titleOff*2)/charHeight ;
   int totalPages = (nrLines + linesPerPage - 1)/linesPerPage;
   
	if(StartDoc(hdcPrint, &di) > 0)
	{
		for(short copies = 0 ;copies < ((WORD) prd.Flags & PD_COLLATE ? prd.nCopies : 1) ;copies++)
		{
	      for(short page = 0 ; page < totalPages ; page++)
	      {
	         for(short copies2 = 0 ;copies2 < (prd.Flags & PD_COLLATE ? 1 : prd.nCopies); copies2++)
	         {
				   if(StartPage(hdcPrint) < 0)
				   {
				      success = false ;
				      break ;
				   }

				   for(int line = 0 ; line < linesPerPage ; line++)
				   {
				      if(line == 0)
				      {
				          sprintf(str,"Page %d of %d - %s",page+1,totalPages,editor->edName);
						    TextOut(hdcPrint,0, 0 ,str,strlen(str));
						    MoveToEx(hdcPrint,0,titleOff,NULL);
						    LineTo(hdcPrint,GetDeviceCaps(hdcPrint,HORZRES),titleOff);
						}
						
	               short lineNum = linesPerPage * page + line;
	                        
	               if(lineNum == nrLines)
	                  break ;

						index = SendMessage(editor->edWin,EM_LINEINDEX,lineNum,0);
						long length = SendMessage(editor->edWin,EM_LINELENGTH,index,0);
						text = new char[length+4]; // Allow space for length number
               	*(int*)text = length; // buffer size
						SendMessage(editor->edWin,EM_GETLINE,(WPARAM)lineNum,(LPARAM)(text));
						text[length] = '\0'; // Remove \r\n	
						TextOut(hdcPrint,0, charHeight * line + titleOff*2 ,text,length);
						delete [] text;
	            }
	                   
					if(EndPage(hdcPrint) < 0)
					{
					   success = false;
					   break;
					}
	         }
	              
				if(!success)
				   break;
	      }
	         
		   if(!success)
				break;
	   }
	}
	else
	{
	   success = false;
	}
	
	if(success)
	    EndDoc(hdcPrint) ;
	

	DeleteDC(hdcPrint) ;
	DeleteObject(printFont);

   return(OK);
}

/************************************************************************
*         Place the print dialog in the centre of the window            *
************************************************************************/

UINT APIENTRY PrintHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{      
	if(message == WM_INITDIALOG) // Process events from standard part of dialog
	{
      PlaceDialogOnTopInCentre(hWnd); 
   } 

   if(message == WM_DESTROY)
   {
      DialogTidyUp(hWnd,false);
   }

   return(0);                      				                     
}


char* GetPrintParameter(short which)
{
	static char str[25];

	if(which == ID_PRINT_WIDTH)
	{
		sprintf(str,"%g",gPrintWidth);
		return(str);
	}
	else if(which == ID_PRINT_HEIGHT)
	{
		sprintf(str,"%g",gPrintHeight);
		return(str);
	}
	if(which == ID_LONG_TICK_LENGTH)
	{
		sprintf(str,"%g",gPrintLongTickLength);
		return(str);
	}
	else if(which == ID_SHORT_TICK_LENGTH)
	{
		sprintf(str,"%g",gPrintShortTickLength);
		return(str);
	}
	else if(which == ID_SYMBOL_SIZE)
	{
		sprintf(str,"%g",gPrintSymbolSize);
		return(str);
	} 
	return(OK);
}


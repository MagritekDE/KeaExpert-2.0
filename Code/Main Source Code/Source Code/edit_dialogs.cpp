#include "stdafx.h"
#include "edit_dialogs.h"
#include "defineWindows.h"
#include "dialog_utilities.h"
#include "edit_class.h"
#include "globals.h"
#include "message.h"
#include "mymath.h"
#include "prospaResource.h"
#include "memoryLeak.h"

/*****************************************************************************************
    Display a dialog which allows the user to jump to particular line in the current
    text editor.
******************************************************************************************/

int CALLBACK LineNumberDlgProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{  
	char lineNrStr[50];
   static long lineNr;
   
	switch (message)
	{
		case(WM_INITDIALOG):
		{ 		   
		   sprintf(lineNrStr,"%ld",lineNr);
	      SetWindowText(GetDlgItem(hWnd,ID_LINE_NR),lineNrStr);
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
		   long lineNr;
         float lineNrf;

		   switch(LOWORD(wParam))
		   {
	       	case(ID_CANCEL):
	            MyEndDialog(hWnd,0);
	         	break;
	       	case(ID_APPLY):
				   GetDlgItemText(hWnd,ID_LINE_NR,lineNrStr,50);
				   sscanf(lineNrStr,"%f",&lineNrf);
               lineNr = nint(lineNrf);
				   if(lineNr > 0 && curEditor && curEditor->SelectLines(lineNr,lineNr) != ERR)
               {
				      MyEndDialog(hWnd,0);
               }
               else
               {
                  MessageDialog(prospaWin,MB_ICONERROR,"Go to line","Invalid line number");
               }
	         	break;	         	         	
	    	}
	      break;
		}
   }
	
	return(0);
}
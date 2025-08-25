#include "stdafx.h"
#include "utilities.h"
#include <math.h>
#include "defineWindows.h"
#include "dialog_utilities.h"
#include "globals.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include "interface.h"
#include "mymath.h"
#include "plot.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "variablesOther.h"
#include "winEnableClass.h"
#include "guiMakeObjects.h"
#include "memoryLeak.h"

int IsKeyPressed(Interface* itfc ,char arg[]);

// Locally defined procedures

UINT APIENTRY ChooseColorHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	if(message == WM_INITDIALOG)
	{
      RECT pr,r;
      HDC hdc = GetDC(hWnd);
      GetWindowRect(prospaWin,&pr);
      short width = pr.right - pr.left;
      short height = pr.bottom - pr.top;
      GetWindowRect(hWnd,&r);
      short w = r.right - r.left;
      short h = r.bottom - r.top;
      short xoff = (width - w)/2 + pr.left;
      short yoff = (height - h)/2 + pr.top;
      CorrectWindowPositionIfInvisible(xoff, yoff, w, h);
      SetWindowPos(hWnd,HWND_NOTOPMOST,xoff,yoff,0,0,SWP_NOSIZE);
      ReleaseDC(hWnd,hdc);       
		Plot::g_chooseBkgColor = false;
		EnableWindow(GetDlgItem(hWnd,ID_BKGD_COLOR),Plot::g_bkgColorEnabled);     
	}
	else if(message == WM_COMMAND)
   {
		switch(LOWORD(wParam))
		{
		   case(ID_BKGD_COLOR):
		   {
				Plot::g_chooseBkgColor = true;
		      PostMessage(hWnd,WM_COMMAND,(WPARAM) IDOK,0);
		      break;
		   }
		}
   }	
	return(0);
}

// If the coordinates of a window fall outside the monitor space correct them so we can see it
void CorrectWindowPositionIfInvisible(short& x, short& y, short w, short h)
{
   // Make sure the window is visible
   short nrMon;
   RECT mr;
   GetMonitorRect(&mr, &nrMon);
   if (x > mr.right)
   {
      x = mr.right - 100;
   }
   if (x + w < mr.left)
   {
      x = 100;
   }
   if (y > mr.bottom)
   {
      y = mr.bottom - 100;
   }
   if (y + h < mr.top)
   {
      y = 100;
   }
   if (y < mr.top)
   {
      y = mr.top;
   }
}

// Return the monitor space as a rectangle and the number of monitors
void GetMonitorRect(RECT* r, short* num_monitors)
{
   *num_monitors = GetSystemMetrics(SM_CMONITORS);
   int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
   int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
   int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
   int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
   SetRect(r, x, y, x + w - 1, y + h - 1);
}

int IsKeyPressed(Interface* itfc ,char arg[])
{
   short nrArgs;
   CText key;
   char keyChar;
   bool state = false;

   if((nrArgs = ArgScan(itfc,arg,0,"key to check","e","t",&key)) < 0)
      return(nrArgs); 

   if(key == "shift")
      state = (GetAsyncKeyState(VK_SHIFT) & 0x08000);
   else if(key == "control")
      state = (GetAsyncKeyState(VK_CONTROL) & 0x08000);
   else if(key == "escape")
      state = (GetAsyncKeyState(VK_ESCAPE) & 0x08000);
   else if(key == "alt")
      state = (GetAsyncKeyState(VK_MENU) & 0x08000);
   else if(key == "enter")
      state = (GetAsyncKeyState(VK_RETURN) & 0x08000);
   else if(key == "up")
      state = (GetAsyncKeyState(VK_UP) & 0x08000);
   else if(key == "down")
      state = (GetAsyncKeyState(VK_DOWN) & 0x08000);
   else if(key == "left")
      state = (GetAsyncKeyState(VK_LEFT) & 0x08000);
   else if(key == "right")
      state = (GetAsyncKeyState(VK_RIGHT) & 0x08000);
   else
   {
      keyChar = key[0];
      state = (GetAsyncKeyState((int)keyChar) & 0x08000);
   }

   itfc->retVar[1].MakeAndSetFloat((int)state);
   itfc->nrRetValues = 1;
   return(OK);      
}

 
/**********************************************************************************
* Allow the user to select a color interactively
**********************************************************************************/

UINT APIENTRY SelectColorHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

int SelectColour(Interface* itfc ,char args[])
{
   Variable vColor;
   static CHOOSECOLOR cc;
   float color[4];
   short nrArgs;
   short bkgColorEnabled = 0;
   COLORREF resultCol,initCol = RGB(0,0,0);
  
   if((nrArgs = ArgScan(itfc,args,0,"default colour, [bkcolor]","ee","vd",&vColor,&bkgColorEnabled)) < 0)
      return(nrArgs); 

   if(nrArgs > 0)
   {
      if((VarType(&vColor) == MATRIX2D) && (VarWidth(&vColor) >= 3) && (VarHeight(&vColor) == 1))
      {
         float** col = VarRealMatrix(&vColor);
         BYTE red   = nint(col[0][0]); 
         BYTE green = nint(col[0][1]); 
         BYTE blue  = nint(col[0][2]); 
         initCol = RGB(red,green,blue);
      }
      else
      {
         ErrorMessage("invalid initial color");
         return(ERR);
      }
   }

   WinData* win = GetGUIWin();
   
	resultCol =  Plot::ChooseAColor(win->hWnd, initCol, bkgColorEnabled);

   if(resultCol == ABORT_COLOR_CHOICE)
   {
      itfc->retVar[1].MakeNullVar();
   }
   else if(resultCol == 0xFFFFFFFF)
   {
      color[0] = -1;
      color[1] = -1;
      color[2] = -1;
      color[3] = -1;
      itfc->retVar[1].MakeMatrix2DFromVector(color,4,1);
   }
   else
   {
      color[0] = GetRValue(resultCol);
      color[1] = GetGValue(resultCol);
      color[2] = GetBValue(resultCol);
      color[3] = 0;
      itfc->retVar[1].MakeMatrix2DFromVector(color,4,1);
   }

   itfc->nrRetValues = 1;

	WinData::SetGUIWin(win);
   return(OK);
}


UINT APIENTRY SelectColorHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	if(message == WM_INITDIALOG)
	{
      PlaceDialogOnTopInCentre(hWnd);    
	}

	return(0);
}

// Temporary Trap for InvalidateRect

BOOL MyInvalidateRect(HWND hWnd,const RECT* r,BOOL bErase)
{
   return(InvalidateRect(hWnd,r,bErase));
}

void MySetFocus(HWND hWnd)
{
   SetFocus(hWnd);
}

void MySetCursor(HCURSOR cur)
{
   //if(cur == (HCURSOR)0x10009)
   //{
   //   TextMessage("Cursor %X\n",cur);
   //}
   SetCursor(cur);
}

// Temporary Trap for UpdateWindow

BOOL MyUpdateWindow(HWND hWnd)
{
   return(UpdateWindow(hWnd));
}

// Check or uncheck a toolbar button defined by a toolbar name and button key
short SetToolBarItemCheck(WinData *win, char *toolbarName, char *butKey, bool check)
{
    ObjectData *obj = win->FindObjectByValueID(toolbarName);

    if(!obj || obj->type != TOOLBAR)
       return(ERR);

    ToolBarInfo *info = (ToolBarInfo*)obj->data;

    for(int i = 0; i < info->nrItems; i++)
    {
       if(info->item[i].key == butKey)
       {
	       SendMessage(obj->hWnd,TB_CHECKBUTTON,(WPARAM)i,(LPARAM) MAKELONG(check, 0));
          return(OK);
       }
    }
    return(ERR);
}

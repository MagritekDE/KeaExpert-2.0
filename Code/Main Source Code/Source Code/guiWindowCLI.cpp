#include "stdafx.h"
#include <shellapi.h>
#include "guiWindowCLI.h"
#include "cArg.h"
#include "control.h"
#include "debug.h"
#include "defineWindows.h"
#include "edit_class.h"
#include "edit_files.h"
#include "error.h"
#include "evaluate.h"
#include "files.h"
#include "globals.h"
#include "guiInteractiveWindowBuilder.h"
#include "guiMakeObjects.h"
#include "guiModifyObjectParameters.h"
#include "guiObjectClass.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include "interface.h"
#include "list_functions.h"
#include "main.h"
#include "message.h"
#include "mymath.h"
#include "plot1dCLI.h"
#include "plot.h"
#include "plot3dClass.h"
#include "process.h"
#include "scanstrings.h"
#include "variablesOther.h"
#include "utilities.h"
#include "memoryLeak.h"

#pragma warning (disable: 4312) // Ignore Get/SetWindowLong warnings

#define WM_UPDATEUISTATE  0x0128
#define WM_QUERYUISTATE   0x0129
//#define UIS_SET  1
//#define UIS_CLEAR  2
//#define UIS_INITIALIZE  3
//#define UISF_HIDEFOCUS  1
//#define UISF_HIDEACCEL  2
//#define UISF_ACTIVE  4

char* CheckAndGetArrayString(ObjectData* obj, char *value);
bool CheckForValidArrayString(char* value);
int ProcessTextEntry(Interface *itfc, ObjectData *obj, char **entry);
int ProcessString(ObjectData *obj, char *value, char **entry, int len);

int ListWindows(Interface* itfc ,char args[])
{
   WinData *lst,*w, *next= rootWin->next;
   CText text;
   lst = w = next;
   TextMessage("\n\n Window no.\tTitle\n");
   TextMessage(" --------------------------------");

   while(w != NULL)
   {
      lst = w;
      w = w->next;         
   }
   w = lst;
   while(w != rootWin)
   {
      w->GetWindowText(text);
      TextMessage("\n   %2hd\t\t%s",w->nr,text.Str());
      w = w->last;         
   }
   TextMessage("\n");
   itfc->nrRetValues = 0;
   return(OK);
}


int ListControls(Interface* itfc ,char args[])
{
   short nrArgs;
   short winNr;
	
	// Get arguments (window number, parameter identifier, parameter value) ***
	if((nrArgs = ArgScan(itfc,args,1,"winNr","e","d",&winNr)) < 0)
	   return(nrArgs);

// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}

// Find window class ******************************************************
   WinData *win = rootWin->FindWinByNr(winNr);
	if(!win)
	{
	   ErrorMessage("window %d undefined",winNr);
	   return(ERR);
	}

   TextMessage("\n\n Control no.\tType\n");
   TextMessage(" --------------------------------");

	for(ObjectData* obj: win->widgets.getWidgets())
   {
      TextMessage("\n   %2hd\t\t%s",obj->nr(),ObjectData::GetTypeAsString(obj->type));
   }
   TextMessage("\n");
   itfc->nrRetValues = 0;

   return(OK);
}

// Give one of the GUI windows focus

int GetObject(Interface *itfc, char args[])
{
   static short winNr, objNr;
   short nrArgs;
	WinData *win;
   ObjectData *obj;
   Variable objVar;
   itfc->nrRetValues = 0;

// Get arguments (window number, parameter identifier, parameter value) ***
	if((nrArgs = ArgScan(itfc,args,1,"winNr, [objNr]","ee","dv",&winNr,&objVar)) < 0)
	   return(nrArgs);

// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}

// Find window class ******************************************************
   win = rootWin->FindWinByNr(winNr);
	if(!win)
	{
	   //ErrorMessage("window %d undefined",winNr);
	   //return(ERR);
      itfc->retVar[1].MakeNullVar();
      itfc->nrRetValues = 1;
      return(OK);
	}

   if(nrArgs == 2)
   {
      if(objVar.GetType() == FLOAT32) // By number
      {
         objNr = nint(objVar.GetReal());
         if(!(obj = win->widgets.findByNr(objNr)))
         {
            itfc->retVar[1].MakeNullVar();
            itfc->nrRetValues = 1;
            return(OK);
            //ErrorMessage("object (%d,%d) not found",winNr,objNr);
            //return(ERR);
         }
      }
      else if(objVar.GetType() == UNQUOTED_STRING) // By name
      {
         if(!(obj = win->widgets.findByValueID(objVar.GetString())))
         {
            if(!(obj = win->widgets.findByObjectID(objVar.GetString())))
            {
               itfc->retVar[1].MakeNullVar();
               itfc->nrRetValues = 1;
               return(OK);
            }
         }
         objNr = obj->nr();
      }
		else // Neither
		{
			ErrorMessage("Only numbers and strings may be supplied.");
			return(ERR);
		}
      itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
      itfc->nrRetValues = 1;
   }
   else
   {
      itfc->retVar[1].MakeClass(WINDOW_CLASS,(void*)win);
      itfc->nrRetValues = 1;
   }

   return(OK);
}      
// Return the object which called this procedure

int GetParentObject(Interface *itfc, char *args)
{
   if(itfc->obj != NULL)
   {
      itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)itfc->obj);
      itfc->nrRetValues = 1;
      return(OK);
   }
   else
   {
      ErrorMessage("no parent object defined");
      return(ERR);
   }
}

int GetMenuName(Interface *itfc, char *args)
{
   if(itfc->obj != NULL && itfc->obj->type == MENU && itfc->obj->data)
   {
      MenuInfo *info = (MenuInfo*)itfc->obj->data;

      itfc->retVar[1].MakeAndSetString(info->name);
      itfc->nrRetValues = 1;
      return(OK);
   }
   else
   {
      ErrorMessage("no parent menu");
      return(ERR);
   }
}


// Give one of the GUI windows focus (or an object)

int SetWindowFocus(Interface *itfc, char args[])
{
   static short winNr;
   static short objNr;
   short nrArgs;
	WinData *win;
   ObjectData *obj = 0;
	itfc->nrRetValues = 0;

   win = GetWinDataClass(GetFocus());
   if(win)
      winNr = win->nr;
   else
      winNr = -1;

// Get arguments (window number, parameter identifier, parameter value) ***
	if((nrArgs = ArgScan(itfc,args,1,"window","ee","dd",&winNr,&objNr)) < 0)
	   return(nrArgs);

// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}

// Find window class ******************************************************
   win = rootWin->FindWinByNr(winNr);
	if(!win)
	{
	   ErrorMessage("window %d undefined",winNr);
	   return(ERR);
	}

   if(nrArgs == 2)
   {
      // Find control instance ***********************
      if(!(obj = win->widgets.findByNr(objNr)))
      {
         ErrorMessage("object '%ld' not found",(long)objNr);
         return(ERR);
      }     
   }  

// Update statusbar, toolbar and menu based on selected window/control
   if(win->isMainWindow)
   {
      if(obj) 
        SelectFixedControls(win,obj); 
      else
        SelectFixedControls(win); 
   }

// Set focus
   if(nrArgs == 1)
   {
      SetFocus(win->hWnd);
   }
   else
	{
		if (obj)
		{
			SetFocus(obj->hWnd);
		}
	}

   return(OK);
}      

int GetCtrlFocus(Interface* itfc ,char args[])
{
   HWND hwnd = GetFocus();

   if(!hwnd)
   {
      itfc->retVar[1].MakeAndSetFloat(-1);
      itfc->retVar[2].MakeAndSetFloat(-1);
      itfc->nrRetValues = 2;
   }

   WinData *next = rootWin->next;
   ObjectData *obj = NULL;
   WinData *win;
   for(win = next; win != NULL; win = win->next)
   {
      if(obj = win->widgets.findByWin(hwnd))
         break;
   }
   
   if(obj)
   {
      itfc->retVar[1].MakeAndSetFloat(win->nr);
      itfc->retVar[2].MakeAndSetFloat(obj->nr());
      itfc->nrRetValues = 2;
   }
   else
   {
      itfc->retVar[1].MakeAndSetFloat(-1);
      itfc->retVar[2].MakeAndSetFloat(-1);
      itfc->nrRetValues = 2;
   }

   return(OK);


}

/*************************************************************************
*          Command line interface to modify window properties
*************************************************************************/

int SetWindowParameter(Interface *itfc, char args[])
{
   CText parameter; 
   Variable value;
   short winNr;
   short nrArgs;
	WinData *win;
	RECT r;
   short x,y,w,h;
   bool correctType = true;
	extern void ResizeObjectsWhenParentResizes(WinData *win, ObjectData *objDiv, HWND hWnd, short x, short y);


// Get the screen dimenions minus the taskbar
   SystemParametersInfo(SPI_GETWORKAREA,0,&r,0); // TODO doesn't account for dual monitors
   long scrnHeight = r.bottom;
   long scrnWidth = r.right;

// Get arguments (window number, parameter identifier, parameter value) ***
	if((nrArgs = ArgScan(itfc,args,3,"window, parameter, value","eee","dtv",&winNr,&parameter,&value)) < 0)
	   return(nrArgs);

// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}
	
// Find window class ******************************************************
   win = rootWin->FindWinByNr(winNr);
	if(!win)
	{
	   ErrorMessage("window %d undefined",winNr);
	   return(ERR);
	}
	
// Get window dimenions ***************************************************	
   GetWindowRect(win->hWnd,&r);
   x = (short)r.left;
   y = (short)r.top;
   w = (short)(r.right - r.left);
   h = (short)(r.bottom - r.top);

// Parameter to lower case
   parameter.LowerCase();

// Set window property ****************************************************	

   if(parameter == "bkgcolor" || parameter == "bgcolor") // Set the window background color
   {
      if(SetColor(win->bkgColor, &value) == ERR)
         return(ERR);
      MyInvalidateRect(win->hWnd,NULL,NULL);
   }
   else if(parameter == "constrained")
   {
      if(value.GetType() == UNQUOTED_STRING)
      {           
         if(!strcmp(value.GetString(),"yes") || !strcmp(value.GetString(),"true"))
         {
            win->constrained = true;

         }
         else
         {
            win->constrained = false;
         }
      }
   }
   else if(parameter == "debug")
   {
      if(value.GetType() == UNQUOTED_STRING)
      {   
         if(!strcmp(value.GetString(),"true"))
         {
            win->debugger = true;
            gDebug.win = win;
         }
         else
         {
            win->debugger = false;
            gDebug.win = NULL;
         }
      }
   }
   else if(parameter == "dimensions")
   {
      if(value.GetType() == MATRIX2D && value.GetDimY() == 1 && value.GetDimX() == 4)
      {   
         x = nsint(value.GetMatrix2D()[0][0]);
         y = nsint(value.GetMatrix2D()[0][1]);
         w = nsint(value.GetMatrix2D()[0][2]);
         h = nsint(value.GetMatrix2D()[0][3]);
         h += titleBarHeight;
         CorrectWindowPositionIfInvisible(x, y, w, h);
         if(win->resizeable)
         {
            if(w > scrnWidth)  w = 0.95*scrnWidth;
            if(h > scrnHeight) h = 0.95*scrnHeight;
         }
         MoveWindow(win->hWnd,x,y,w,h,true);
      }
      else if(value.GetType() == LIST && value.GetDimX() == 4)
      {
         bool region;
         if(GetObjectDimensions(value.GetList()[0], &win->xSzScale, &win->xSzOffset, &region) == ERR)
         {
            ErrorMessage("invalid x position");
            return(ERR);
         }
         if(GetObjectDimensions(value.GetList()[1], &win->ySzScale, &win->ySzOffset, &region) == ERR)
         {
            ErrorMessage("invalid y position");
            return(ERR);
         }
         if(GetObjectDimensions(value.GetList()[2], &win->wSzScale, &win->wSzOffset, &region) == ERR)
         {
            ErrorMessage("invalid width");
            return(ERR);
         }
         if(GetObjectDimensions(value.GetList()[3], &win->hSzScale, &win->hSzOffset, &region) == ERR)
         {
            ErrorMessage("invalid height");
            return(ERR);
         }
         RECT rect;
         long xoff,yoff,ww,wh;

         if(win->isMainWindow)
         {
            xoff = 0;
            yoff = 0;
            ww = GetSystemMetrics(SM_CXFULLSCREEN);
            wh = GetSystemMetrics(SM_CYFULLSCREEN);
         }
         else
         {
            GetWindowRect(prospaWin,&rect);
            xoff = resizableWinBorderSize + rect.left;
            yoff = titleBarNMenuHeight + rect.top;
            ww = rect.right - rect.left - 2*resizableWinBorderSize;
            wh = rect.bottom - rect.top - (resizableWinBorderSize + titleBarNMenuHeight);
         }

         x = ww*win->xSzScale + win->xSzOffset + xoff;
         y = wh*win->ySzScale + win->ySzOffset + yoff;
         w = ww*win->wSzScale + win->wSzOffset;
         h = wh*win->hSzScale + win->hSzOffset;
       
         short nrMon;
         RECT mr;
         GetMonitorRect(&mr,&nrMon);

         if(!win->constrained)
         {
            if(x > mr.right)
            {
               x = mr.right - 100;
            }
            if(x+w < mr.left)
            {
               x = 100;
            }
            if(y > mr.bottom)
            {
               y = mr.bottom - 100;
            }
            if(y+h < mr.top)
            {
               y = 100;
            }
         }
         MoveWindow(win->hWnd,x,y,w,h,true);
      }
      else
      {
         ErrorMessage("Invalid dimension vector [x,y,w,h]");
         return(ERR);
      }
   }
	else if(parameter == "dragndropproc")
	{  
      if(value.GetType() == UNQUOTED_STRING)
      {
         win->dragNDropProc = value.GetString();
         DragAcceptFiles(win->hWnd,(win->dragNDropProc != ""));
		}
		else
		   correctType = false; 
   }
   else if(parameter == "draw")
	{
		if(value.GetType() == UNQUOTED_STRING)
		{
         CText valString = value.GetString();
			if(valString == "true" || valString == "yes" || valString == "on")  
         {
				win->drawing = true;
            SendMessage(win->hWnd, WM_SETREDRAW, true, 0);
            MyInvalidateRect(win->hWnd,NULL,1); // Make sure the window is redrawn
         }
			else 
         {
				win->drawing = false;
          //  GetWindowRect(win->hWnd,&r);
            SendMessage(win->hWnd, WM_SETREDRAW, false, 0);
          //  MyInvalidateRect(NULL,&r,1); // Make sure the background is redrawn is the window moves or resizes (why was this here??).
         }
		}    
		else
		   correctType = false; 
	}
   else if(parameter == "recalc")
	{
		extern void RecalculateObjectPositions(WinData *win);
		RecalculateObjectPositions(win);
	}
   else if(parameter == "exit_procedure" || parameter == "exitprocedure")
   {
      if(value.GetType() == UNQUOTED_STRING)
      {           
         strncpy_s(win->exitProcName,MAX_NAME,value.GetString(),_TRUNCATE); 
      }
		else
		   correctType = false; 
   }
   else if(parameter== "focus")
   {
      if(value.GetType() == FLOAT32)
      {   
         ObjectData *obj;
         long n = nint(value.GetReal());
         if(obj = win->widgets.findByNr(n))
         {
            SelectFixedControls(win, obj);
            SetFocus(obj->hWnd);
         }
         else 
	      {
	         ErrorMessage("can't find object nr '%ld'",n);
	         return(ERR);
	      }
      }
      else
      {   
         SetFocus(win->hWnd);
      }
   }
   else if(parameter == "gridspacing")
   {
      if(value.GetType() == FLOAT32)
      {   
         win->gridSpacing = nsint(value.GetReal());
      }
		else
		   correctType = false; 
   }
   else if(parameter == "hide")
   {
     	ShowWindow(win->hWnd,SW_HIDE);
      win->visible = false;
   }
   else if(parameter == "height")
   {
      if(value.GetType() == FLOAT32)
      {   
         h = nsint(value.GetReal());
         h += titleBarHeight;
         if(win->resizeable)
            if(h > scrnHeight) h = 0.95*scrnHeight;
         MoveWindow(win->hWnd,x,y,w,h,true);
      }
		else
		   correctType = false; 
   }
   else if(parameter == "icon")
   {
      if(value.GetType() == UNQUOTED_STRING)
      {   
		//	CText dir;
		//	GetCurrentDirectory(dir);
	   	HANDLE hIcon = LoadImage(NULL, value.GetString(), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
			if (hIcon)
			{
		      // Change both icons to the same icon handle.
				 SendMessage(win->hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
				 SendMessage(win->hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

				 //// This will ensure that the application icon gets changed too.
			//	 SendMessage(GetWindow(win->hWnd, GW_OWNER), WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
				// SendMessage(GetWindow(win->hWnd, GW_OWNER), WM_SETICON, ICON_BIG, (LPARAM)hIcon);
			}
		}
		else
		   correctType = false; 
   }
   else if(parameter == "keepinfront") // Specify whether window will be infront of non-gui windows
   {
      if(value.GetType() == UNQUOTED_STRING)
      {   
         if(!strcmp(value.GetString(),"true"))
         {
            if(win->hWnd != prospaWin)
            {
               win->keepInFront = true;
            }
            else
            {
               ErrorMessage("Main Prospa window cannot have \"keepInFront\" option set");
               return(ERR);
            }
         }
         else
            win->keepInFront = false;
      }
		else
		   correctType = false; 
   }

	else if(parameter == "mainwindow") // Make this Prospa window.
   {
      extern int RegisterUSBDevice(HWND hWnd);
      WinData *oldMainWin;
      if(oldMainWin = rootWin->FindWinByHWND(prospaWin)) // Remove main window tags from old one.
           oldMainWin->isMainWindow = false; // Not the main window
     // Update new Prospa window
      prospaWin = win->hWnd;
      RegisterUSBDevice(prospaWin);
      SetWindowLong(prospaWin, GWL_EXSTYLE, GetWindowLong(prospaWin, GWL_EXSTYLE) | WS_EX_APPWINDOW); // Task bar icon
      SetWindowLong(win->hWnd, GWL_HWNDPARENT, (LONG)0); // Prospa window has no parent
      win->isMainWindow = true;
      win->keepInFront = false; // Rear window now
      itfc->win = win; 
      if(oldMainWin)
         SetWindowLong(oldMainWin->hWnd, GWL_HWNDPARENT, (LONG)win->hWnd); // Give old prospa window a new paren
   }

   else if(parameter == "maximized")
   {
		if(value.GetType() == UNQUOTED_STRING)
		{
         CText valString = value.GetString();
			if(valString == "true" || valString == "yes" || valString == "on")  
         {
            ShowWindow(win->hWnd,SW_MAXIMIZE);
         }
			else 
         {
            ShowWindow(win->hWnd,SW_NORMAL);
         }
		}    
		else
		   correctType = false;
   }

   else if(parameter == "minimized")
   {
		if(value.GetType() == UNQUOTED_STRING)
		{
         CText valString = value.GetString();
			if(valString == "true" || valString == "yes" || valString == "on")  
         {
            ShowWindow(win->hWnd,SW_MINIMIZE);
         }
			else 
         {
            ShowWindow(win->hWnd,SW_NORMAL);
         }
		}    
		else
		   correctType = false;
   }
   else if(parameter == "menu") // Set menu and accelerator table based on child object
   {

      if(value.GetType() == FLOAT32 ) // Menu object number
      {   
         if(value.GetReal() == 0)
         {
            if(win->menu) // Use the current window menu
            {
            // Add the new menu to the GUI window
               SetMenu(win->hWnd,win->menu);

            //  Add accelerator table
               win->hAccelTable = win->accelTable;

            // Make sure this is the current accelerator if this is the current window
               if(currentAppWindow == win->hWnd)
               {
	               currentAccel = win->hAccelTable;
               }

            // Save the object which contributed the window menu
               win->menuObj = NULL;
            }

         }
         else // Use a child object's menu
         {

            ObjectData *obj = win->widgets.findByNr(nint(value.GetReal()));

            if(obj && obj->menu)
            {
            // Add the new menu to the GUI window
               SetMenu(win->hWnd,obj->menu);

            //  Add accelerator table
               win->hAccelTable = obj->accelTable;

            // Make sure this is the current accelerator if this is the current window
               if(currentAppWindow == win->hWnd)
               {
	               currentAccel = win->hAccelTable;
               }

            // Save the object which contributed the window menu
               win->menuObj = obj;
            }
         }
      }
		else
		   correctType = false; 
   }
   else if(parameter == "menubar") // Set menu and accelerator table for window
   {
      if(value.GetType() == MATRIX2D && value.GetDimY() == 1) // The new menu is a vector of menu numbers
      {   
         long w =  value.GetDimX(); // Number of menus
         short objNr;
         ObjectData *menuObj,*menuObjPR;
         MenuInfo *info;
         float *menuList = value.GetMatrix2D()[0]; // Menu vector

      // Make a new menu - blank for now
         HMENU menu = CreateMenu();

      // Count the total number of menu items in the supplied menus
         int cnt = 0;
         for(int i = 0; i < w; i++) // Loop over menus
         {
            objNr = nsint(menuList[i]); // Extract menu object number

            if(objNr < 0) continue;

            if(!(menuObj = win->widgets.findByNr(objNr))) // Get the menu object by widget number
            {
               DestroyMenu(menu);
               ErrorMessage("object '%ld' not found",(long)objNr);
               return(ERR);
            }
            info = (MenuInfo*)menuObj->data; // Found menu info for this menu widget

          // Search menu for pull-rights and add them to the item count
            for(int j = 0; j < info->nrItems; j++)
            {
               if(!strcmp(info->label[j],"\"Pull_right\"")) // Is it a pull-right menu
               {
                  short objNrPR;
                  if(sscanf(info->cmd[j],"%hd",&objNrPR) != 1) // Command should be a number
                  {
                     DestroyMenu(menu);
                     ErrorMessage("'%s' is not a valid menu object number",info->cmd[j]);
                     return(ERR);
                  }
                  if(!(menuObjPR = win->widgets.findByNr(objNrPR))) // Get the pull rightmenu object by menu number
                  {
                     DestroyMenu(menu);
                     ErrorMessage("object '%hd' not found",objNrPR);
                     return(ERR);
                  }
                  MenuInfo* infoPR = (MenuInfo*)menuObjPR->data;
                  cnt += infoPR->nrItems;
               }
            }

            cnt += info->nrItems; // Add the number of items in this menu
         }

       // Make a backup of the existing window accelerator table
         ACCEL *accelNew,*accelMenu = NULL;
         int j,k,nMenu = 0;

       // Allocate sufficent space for new accelerators
         accelMenu = new ACCEL[cnt];
         nMenu = 0;

      // Load all the accelerators
         for(int i = 0; i < w; i++) // Loop over menus
         {
            objNr = nsint(menuList[i]); // Extract menu object number
            if(objNr < 0) // See if its a user menu (i.e. folder menu)
            {
                AppendUserMenu(menu);
                continue;
            }
            if(!(menuObj = win->widgets.findByNr(objNr))) // Get the menu object
            {
               DestroyMenu(menu);
               delete [] accelMenu;
               ErrorMessage("object '%ld' not found",(long)objNr);
               return(ERR);
            }

            info = (MenuInfo*)menuObj->data;

          // Search menu for pull-rights
            for(int k = 0; k < info->nrItems; k++)
            {
               if(!strcmp(info->label[k],"\"Pull_right\""))
               {
                  short objNrPR;
                  sscanf(info->cmd[k],"%hd",&objNrPR);
                  menuObjPR = win->widgets.findByNr(objNrPR); // Get the pull-right menu object
                  MenuInfo* infoPR = (MenuInfo*)menuObjPR->data;
                  cnt = infoPR->nrItems;
              
                  for(int j = 0; j < cnt; j++) // Add the accelerators fro the pull-right
                  {
                     if(infoPR->accel[j].key != 0)
                        accelMenu[nMenu++] = infoPR->accel[j];
                  }

               }
            }
            cnt = info->nrItems;

          // Add the new sub-menu to the new main window menu
            AppendMenu(menu,MF_POPUP,(UINT_PTR)info->menu,info->name);

         // Add the accelerator keys from this menu 
            for(int j = 0; j < cnt; j++) // Loop over items in each menu
            {
               if(info->accel[j].key != 0)
                  accelMenu[nMenu++] = info->accel[j];
            }
         }

      // Make a new accelerator table which combines existing and new
         accelNew = new ACCEL[nMenu];

      // Add accelerator menu entries (WHY DO THIS?)
         k = 0;
         for(j = 0; j < nMenu; j++)
            accelNew[k++] = accelMenu[j];

	   // Disconnect the previous menus
			while(RemoveMenu(win->menu, 0, MF_BYPOSITION) != 0);

      // Remove old accelerator table and menu
			if(win->accelTable)
				DestroyAcceleratorTable(win->accelTable);
			if(win->menu)
				DestroyMenu(win->menu);

      // Add this new menu to the object
         win->menu = menu;
         win->accelTable = CreateAcceleratorTable((LPACCEL)accelNew,k);

         if(accelMenu)
            delete [] accelMenu;
         if(accelNew)
            delete [] accelNew;

      // Save the object numbers which contributed to the window menu
         if(win->menuList) delete win->menuList;
         win->menuList = new short[w];
         win->menuListSize = w;
         for(k = 0; k < w; k++)
            win->menuList[k] = nsint(menuList[k]);

      // Display the menu
         SetMenu(win->hWnd,win->menu);

      //  Set the  accelerator table
          win->hAccelTable = win->accelTable;

      // Make sure this is the current accelerator if this is the current window
         if(currentAppWindow == win->hWnd)
         {
	         currentAccel = win->hAccelTable;
         }
      }
		else
		   correctType = false; 
   }
   else if(parameter == "name")
   {
      if(value.GetType() == UNQUOTED_STRING)
      {
         if(win->name) delete [] win->name;
         win->name = new char[strlen(value.GetString())+1];          
         strcpy(win->name,value.GetString()); 
      }
		else
		   correctType = false; 
   }
   else if(parameter == "mergetitle")
   {
      if(value.GetType() == UNQUOTED_STRING)
      {
         CText mode = value.GetString();
 
         if(mode == "true" || mode == "on")
         {   
            win->mergeTitle = true;
         }  
         else if(mode == "false" || mode == "off")
         {   
            win->mergeTitle = false;
         }
      }
		else
		   correctType = false; 
   }
   else if(parameter == "position")
   {
      if(value.GetType() == MATRIX2D && value.GetDimY() == 1 && value.GetDimX() == 2)
      {   
		   GetWindowRect(win->hWnd,&r);
         x = nsint(value.GetMatrix2D()[0][0]);
         y = nsint(value.GetMatrix2D()[0][1]);
		   w = (short)(r.right-r.left);
		   h = (short)(r.bottom-r.top);
         if(x > scrnWidth)  x = scrnWidth/2;
         if(x < 0)
         {
            long width =  GetSystemMetrics(SM_CXFULLSCREEN);
            x = (width - w)/2;
         }
         if(y > scrnHeight) y = scrnHeight/2;
         if(y < 0)
         {
            long height =  GetSystemMetrics(SM_CYFULLSCREEN);
            y = (height - h)/2;
         }
         MoveWindow(win->hWnd,x,y,w,h,true);
      }
      else
      {
         ErrorMessage("Invalid position vector [x,y]");
         return(ERR);
      }
   }
   else if(parameter == "permanent")
   {
      if(value.GetType() == UNQUOTED_STRING)
      {   
         if(!strcmp(value.GetString(),"true"))
            win->permanent = true;
         else
            win->permanent = false;
      }
		else
		   correctType = false; 
   }

   else if(parameter == "resizable")
   {
      if(value.GetType() == UNQUOTED_STRING)
      {           
         if(!strcmp(value.GetString(),"yes") || !strcmp(value.GetString(),"true"))
         {
            win->resizeable = true;
            win->EnableResizing(true);
         }
         else
         {
            win->resizeable = false;
            win->EnableResizing(false);
         }
      }
		else
		   correctType = false; 
   }
   else if(parameter == "saveeditsessions")
   {
      if(SaveWinEditSessions(win) == IDCANCEL)
	      itfc->retVar[1].MakeAndSetString("cancel");
      else
	      itfc->retVar[1].MakeAndSetString("ok");
      itfc->nrRetValues = 1;
      return(OK);
   }
   else if(parameter == "sizelimits")
   {
      if(value.GetType() == MATRIX2D && value.GetDimY() == 1 && value.GetDimX() == 4)
      {   
         win->sizeLimits.minWidth = nsint(value.GetMatrix2D()[0][0]);
         win->sizeLimits.maxWidth = nsint(value.GetMatrix2D()[0][1]);
         win->sizeLimits.minHeight = nsint(value.GetMatrix2D()[0][2]);
         win->sizeLimits.maxHeight = nsint(value.GetMatrix2D()[0][3]);

         if(win->sizeLimits.minHeight > 0)
            win->sizeLimits.minHeight += titleBarHeight + GetSystemMetrics(SM_CYSIZEFRAME);

         if(win->sizeLimits.maxHeight > 0)
            win->sizeLimits.maxHeight += titleBarHeight  + GetSystemMetrics(SM_CYSIZEFRAME);
      }
		else
		   correctType = false; 
   }
   else if(parameter == "show_menu" || parameter == "showmenu" || parameter == "editmenu")
   {
      if(value.GetType() == UNQUOTED_STRING)
      {           
         if(!strcmp(value.GetString(),"yes") || !strcmp(value.GetString(),"true"))
            win->showMenu = true;
         else
            win->showMenu = false;
      }
		else
		   correctType = false; 
   }
   else if(parameter == "show" || parameter == "showwindow")
   {
      CText mode = "normal";
       if(!strcmp(value.GetString(),"maximized"))
          mode = "maximized";

      // Make sure tab controls which are not visible are hidden
      if(win->updateControlVisibilityFromTabs() == ERR)
         return(ERR);

     // Activate window (i.e. not in edit mode)
      win->activate();

     // Show the window - maximized if required
      if(win->keepInFront)
      {
         if(IsZoomed(win->hWnd) || mode == "maximized")
	         ShowWindow(win->hWnd,SW_SHOWMAXIMIZED); // Display the window maximised 
         else
            ShowWindow(win->hWnd,SW_SHOWNORMAL); // Display the window
         SetCurrentGUIParent();               
      }
      else
      {
         ChangeGUIParent(win->hWnd);
         if(IsZoomed(win->hWnd) || mode == "maximized")
	         ShowWindow(win->hWnd,SW_SHOWMAXIMIZED); // Display the window maximised
         else
            ShowWindow(win->hWnd,SW_SHOWNORMAL); // Display the window
      }

   // Make sure all controls accept the focus
   // (a manifest bug means that some controls do not)
      SendMessage(win->hWnd,WM_UPDATEUISTATE,UIS_CLEAR + (UISF_HIDEFOCUS * 65536),0);

    // Force it to redraw now
      UpdateWindow(win->hWnd);            

   // Make sure all control objects are assigned as local variables
      CText nr;
      nr.Format("%hd",win->nr);
	   AssignControlObjects(itfc,nr.Str());

   // Set the visibility flag to true
      win->visible = true;
   }
   else if(parameter == "showgrid") // Specify whether a grid will be drawn over the top of the window
   {
      if(value.GetType() == UNQUOTED_STRING)
      {   
         if(!strcmp(value.GetString(),"true"))
            win->showGrid = true;
         else
            win->showGrid = false;
      }
		else
		   correctType = false; 
   } 
   else if(parameter == "snaptogrid") // Specify whether an edited window will have a snap-to-grid function
   {
      if(value.GetType() == UNQUOTED_STRING)
      {   
         if(!strcmp(value.GetString(),"true"))
            win->snapToGrid = true;
         else
            win->snapToGrid = false;
      }
		else
		   correctType = false; 
   } 

   else if(parameter == "statusbox") 
   {
      if(value.GetType() == FLOAT32)
      {  
         ObjectData *obj;
         long n = nint(value.GetReal());
         if(obj = win->widgets.findByNr(n))
         {
            win->defaultStatusbox = obj;
         }
         else 
	      {
	         ErrorMessage("can't find object nr '%ld'",n);
	         return(ERR);
	      }
      }
		else
		   correctType = false; 
   } 
   else if(parameter == "toolbar") 
   {
      if(value.GetType() == FLOAT32)
      {  
         ObjectData *obj;
         long n = nint(value.GetReal());
         if(obj = win->widgets.findByNr(n))
         {
             win->defaultToolbar = obj;
         }
         else 
	      {
	         ErrorMessage("can't find object nr '%ld'",n);
	         return(ERR);
	      }
      }
		else
		   correctType = false; 
   } 

   else if(parameter == "bkgmenu")
   {
      if(value.GetType() == FLOAT32)
      {
         short objNr = value.GetReal();
         ObjectData *menuObj = win->widgets.findByNr(objNr);
         if(!menuObj)
         {
            ErrorMessage("Object %hd not found",objNr);
            return(ERR);
         }
         if(menuObj->type != MENU)
         {
            ErrorMessage("Object %hd is not a menu",objNr);
            return(ERR);
         }
         MenuInfo* info = (MenuInfo*)menuObj->data;
         if(parameter == "bkgmenu")
         {
            win->bkgMenu = info->menu;
            win->bkgMenuNr = objNr;
         }
        
      // Get the exisiting accelerator table for the plot object
         ACCEL *accelOld = NULL,*accelNew = NULL,*accelMenu = NULL;
         int j,k,nMenu = 0,nOld = 0;
      // Extract old table
         if(win->accelTable)
         {
            nOld = CopyAcceleratorTable(win->accelTable,NULL,0);
            accelOld = new ACCEL[nOld];
            CopyAcceleratorTable(win->accelTable,(LPACCEL)accelOld,nOld);
         }
      // Extract menu table
         accelMenu = new ACCEL[info->nrItems];
         for(j = 0; j < info->nrItems; j++) // Loop over items in each menu
         {
            if(info->accel[j].key != 0)
               accelMenu[nMenu++] = info->accel[j];
         }
      // Copy old table
         accelNew = new ACCEL[nOld+nMenu];
         k = 0;
			if (accelOld)
			{
				for(k = 0, j = 0; j < nOld; j++)
            accelNew[k++] = accelOld[j];
			}
      // Add menu entries
         for(j = 0; j < nMenu; j++)
            accelNew[k++] = accelMenu[j];
         if(win->accelTable)
            DestroyAcceleratorTable(win->accelTable);
      // Update the accelerator table
         win->accelTable = CreateAcceleratorTable((LPACCEL)accelNew,k);
         if(accelNew)
            delete [] accelNew;
         if(accelOld)
            delete [] accelOld;
         if(accelMenu)
            delete [] accelMenu;
      }
   }
	else if(parameter == "type")
	{
      if(value.GetType() == UNQUOTED_STRING)
      {          
		   if(!strcmp(value.GetString(),"dialog"))
		   {
		      win->type = DIALOG_WIN;
		   }
		}
		else
		   correctType = false; 
	}
   else if(parameter == "titleupdate")
   {
      if(value.GetType() == UNQUOTED_STRING)
      {
         CText mode = value.GetString();
 
         if(mode == "true" || mode == "on")
         {   
            win->titleUpdate = true;
         }  
         else if(mode == "false" || mode == "off")
         {   
            win->titleUpdate = false;
         }
      }
		else
		   correctType = false; 
   }
   else if(parameter == "title") // Set window title
   {
      if(value.GetType() == UNQUOTED_STRING)
      {
         if(win->title) delete [] win->title;
         win->title = new char[strlen(value.GetString())+1];       
         strcpy(win->title,value.GetString()); 
  //       strcat(win->title," ");  // Need this bug in next command skips last character sometimes
         SetWindowText(win->hWnd,win->title);
      }
		else
		   correctType = false; 
   }
   else if(parameter == "width")
   {
      if(value.GetType() == FLOAT32)
      {   
         w = nsint(value.GetReal());
         if(win->resizeable)
             if(w > scrnWidth) w = 0.95*scrnWidth;
         MoveWindow(win->hWnd,x,y,w,h,true);
      }
		else
		   correctType = false; 
   }
   else if(parameter == "x")
   {
      if(value.GetType() == FLOAT32)
      {   
         x = nsint(value.GetReal());
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
         MoveWindow(win->hWnd,x,y,w,h,true);
      }
		else
		   correctType = false; 
   }
   else if(parameter == "y")
   {
      if(value.GetType() == FLOAT32)
      {   
         y = nsint(value.GetReal());
         short nrMon;
         RECT mr;
         GetMonitorRect(&mr, &nrMon);
         if (y > mr.bottom)
         {
            y = mr.bottom - 100;
         }
         if (y + h < mr.top)
         {
            y = 100;
         }
         MoveWindow(win->hWnd,x,y,w,h,true);
      }
		else
		   correctType = false; 
   }
	else if(parameter == "reposition")
	{
      ResizeObjectsWhenParentResizes(win,NULL,NULL,0,0);
	}
   else 
	{
	   ErrorMessage("invalid window parameter '%s'",parameter);
	   return(ERR);
	}

   if(correctType == false)
	{
		ErrorMessage("invalid data type for parameter '%s'",parameter.Str());
		return(ERR);
	} 

	itfc->nrRetValues = 0;
	return(OK);
}

/*************************************************************************
*          Command line interface to extract window properties
*************************************************************************/

int GetWindowParameter(Interface* itfc, char args[])
{
   static CText parameter;
   static short winNr;
   short nrArgs;
	WinData *win;
	RECT r;

   Variable *ans = &itfc->retVar[1];

// Get arguments (window number, parameter identifier) *******************   
	if((nrArgs = ArgScan(itfc,args,2,"window, parameter","ee","dt",&winNr,&parameter)) < 0)
	   return(nrArgs);

// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}
	
// Find window class ******************************************************
   win = rootWin->FindWinByNr(winNr);
	if(!win)
	{
	   ErrorMessage("window %d undefined",winNr);
	   return(ERR);
	}

   GetWindowRect(win->hWnd,&r);

   parameter.LowerCase();

// Get window property ****************************************************	

   if(parameter == "bkgcolor" || parameter == "bgcolor") // Get the window background color
   {
	   BYTE *data = (BYTE*)&win->bkgColor;
      float colors[4];
      colors[0] = data[0];
      colors[1] = data[1];
      colors[2] = data[2];
      colors[3] = data[3];
      ans->MakeMatrix2DFromVector(colors,4,1);
   }
   else if(parameter == "bordersize")
   {
      int borderSize;
       if(win->resizeable)
          borderSize = GetSystemMetrics(SM_CXSIZEFRAME);
       else
          borderSize = GetSystemMetrics(SM_CXFIXEDFRAME);
#if(_WIN32_WINNT >= 0x0600)
       borderSize += GetSystemMetrics(SM_CXPADDEDBORDER);
#endif
       ans->MakeAndSetFloat(borderSize);
   }
   else if(parameter == "constrained")
   {
      if(win->constrained)
		   ans->MakeAndSetString("true");
      else
		   ans->MakeAndSetString("false");
   }
   else if(parameter == "ctrllist")
   {
      int sz;
      float* nrs = win->GetControlList(&sz);
      ans->MakeMatrix2DFromVector(nrs,sz,1);
      delete [] nrs;
   }
	else if(parameter == "dimensions")
   {
		float dim[4];
      WINDOWPLACEMENT wp;
      GetWindowPlacement(win->hWnd,&wp); // Allows for maximizing or minimizing
		dim[0] = (float)wp.rcNormalPosition.left;
		dim[1] = (float)wp.rcNormalPosition.top;
		dim[2] = (float)(wp.rcNormalPosition.right-wp.rcNormalPosition.left);
		dim[3] = (float)(wp.rcNormalPosition.bottom-wp.rcNormalPosition.top) - titleBarHeight;
		ans->MakeMatrix2DFromVector(dim,4,1);
   }
	else if(parameter == "dimensionstxt" || parameter == "dimensiontxt")
   {
       char **dim = NULL;
       CText txt;

       GetSizeExp("ww",&txt,win->xSzScale,win->xSzOffset,false);
       AppendStringToList(txt.Str(),&dim,0);
       GetSizeExp("wh",&txt,win->ySzScale,win->ySzOffset,false);
       AppendStringToList(txt.Str(),&dim,1);
       GetSizeExp("ww",&txt,win->wSzScale,win->wSzOffset,false);
       AppendStringToList(txt.Str(),&dim,2);
       GetSizeExp("wh",&txt,win->hSzScale,win->hSzOffset,false);
       AppendStringToList(txt.Str(),&dim,3);

	     ans->MakeAndSetList(dim,4);
	     FreeList(dim,4);
   }
	else if(parameter == "dragndropproc")
	{  
      ans->MakeAndSetString(win->dragNDropProc.Str());
   }
   else if(parameter == "exit_procedure" || parameter == "exitprocedure")
   {
	   ans->MakeAndSetString(win->exitProcName);
   }
   else if(parameter == "gridspacing")
      ans->MakeAndSetFloat(win->gridSpacing); 
   else if(parameter == "height")
      ans->MakeAndSetFloat((float)r.bottom-r.top - titleBarHeight);
	else if(parameter == "macroname")
	   ans->MakeAndSetString(win->macroName);
	else if(parameter == "macropath")
   {
      short len = strlen(win->macroPath);
      if(win->macroPath[len-1] == '\\')
         win->macroPath[len-1] = '\0';
	   ans->MakeAndSetString(win->macroPath);
   } 
	else if(parameter == "mainwindow")
   {
      if(win->isMainWindow)
		   ans->MakeAndSetString("true");
      else
		   ans->MakeAndSetString("false");
   }
   else if(parameter == "menubar")
   {
      int sz = win->menuListSize;
      if(sz > 0)
      {
         float* list = new float[sz]; 
         for(int k = 0; k < sz; k++)
            list[k] = win->menuList[k];
         ans->MakeMatrix2DFromVector(list,sz,1); 
         delete [] list;
      }
      else
         ans->MakeAndSetFloat(-1); 
   }
   else if(parameter == "mergetitle")
   {
       if(win->mergeTitle)
          ans->MakeAndSetString("true");
		 else
          ans->MakeAndSetString("false");
   }
   else if(parameter == "name")
   { 
      if(win->name)  // 2.2.14
      {
         char *name = new char[strlen(win->name)+1];
         strcpy(name,win->name);
         if(!win->activated)
            name[strlen(name)-1] = '\0';
         ans->MakeAndSetString(name);
         delete [] name;
      }
      else
        ans->MakeAndSetString("");
   }
   else if(parameter == "nrctrls")
   {
      int sz;
      float* nrs = win->GetControlList(&sz);
      ans->MakeAndSetFloat(sz);
      delete [] nrs;
   }
	else if(parameter == "position")
   {
		float dim[2];
		GetWindowRect(win->hWnd,&r);
		dim[0] = (float)r.left;
		dim[1] = (float)r.top;
		ans->MakeMatrix2DFromVector(dim,2,1);
   }
   else if(parameter == "permanent")
   {
      if(win->permanent)
		   ans->MakeAndSetString("true");
      else
		   ans->MakeAndSetString("false");
   }
	else if(parameter == "resizable")
   {
      if(win->resizeable)
		   ans->MakeAndSetString("true");
      else
		   ans->MakeAndSetString("false");
   }
   else if(parameter == "sizelimits")
   {
		float dim[4];
		dim[0] = (float)win->sizeLimits.minWidth;
		dim[1] = (float)win->sizeLimits.maxWidth;
		dim[2] = (float)win->sizeLimits.minHeight;
		dim[3] = (float)win->sizeLimits.maxHeight;

		ans->MakeMatrix2DFromVector(dim,4,1);
   }
   else if(parameter == "showgrid") 
   {
      if(win->showGrid)
		   ans->MakeAndSetString("true");
      else
		   ans->MakeAndSetString("false");
   } 
   else if(parameter == "showmenu") 
   {
      if(win->showMenu)
		   ans->MakeAndSetString("true");
      else
		   ans->MakeAndSetString("false");
   } 
   else if(parameter == "snaptogrid") 
   {
      if(win->snapToGrid)
		   ans->MakeAndSetString("true");
      else
		   ans->MakeAndSetString("false");
   } 
   else if(parameter == "title")
   {
		CText name;
	   GetWindowTextEx(win->hWnd,name);
      //char *name = new char[strlen(win->title)+1];
     // strcpy(name,win->title);
      if(!win->activated)
			name = name.Start(name.Size()-2);
      ans->MakeAndSetString(name.Str());
    //  delete [] name;
   }
   else if(parameter == "titlebarheight")
   {
       ans->MakeAndSetFloat(titleBarHeight);
   }
   else if(parameter == "titleupdate")
   {
       if(win->titleUpdate)
          ans->MakeAndSetString("true");
		 else
          ans->MakeAndSetString("false");
   }
   else if(parameter == "toolbar")
   {
      if(win->defaultToolbar)
         ans->MakeAndSetFloat(win->defaultToolbar->nr());
      else
         ans->MakeAndSetFloat(-1);
   }
	else if(parameter == "type")
   {
      if(win->type == DIALOG_WIN)
		   ans->MakeAndSetString("dialog");
      else
		   ans->MakeAndSetString("normal");
   }
	else if(parameter == "visible")
   {
      if(win->visible | (bool)IsIconic(win->hWnd))
		   ans->MakeAndSetString("true");
      else
		   ans->MakeAndSetString("false");
   }
	else if(parameter == "maximized")
   {
      if(IsZoomed(win->hWnd))
		   ans->MakeAndSetString("true");
      else
		   ans->MakeAndSetString("false");
   }
	else if(parameter == "minimized")
   {
      if(IsIconic(win->hWnd))
		   ans->MakeAndSetString("true");
      else
		   ans->MakeAndSetString("false");
   }
   else if(parameter == "width")
      ans->MakeAndSetFloat((float)r.right-r.left);
   else if(parameter == "winvar")
   { // Make a link from winvar (a structure) to varList (a linked list)
      Variable *s = win->winVar.GetStruct();
      Variable* var = &(win->varList);
      s->next = var->next;
      itfc->retVar[1].MakeAndSetAlias(&(win->winVar));
      itfc->nrRetValues = 1;
      return(OK);
   }
   else if(parameter == "winnr" || parameter == "nr")
      ans->MakeAndSetFloat(win->nr);   
   else if(parameter == "x")
      ans->MakeAndSetFloat((float)r.left);
   else if(parameter == "y")
      ans->MakeAndSetFloat((float)r.top);
	else
	{
	   ErrorMessage("invalid window type '%s'",parameter);
	   return(ERR);
	}

   itfc->nrRetValues = 1;
	
	return(OK);
}

// *****************
// Take the name tag for each control and convert it into an object variable 
// nameObj which can be used to access all aspects of that control.
// In addition convert it into a variable which contains the current value
// of that control. Both varibles have window scope.
// *****************

int AssignControlObjects(Interface *itfc, char args[])
{
   static short winNr;
   short nrArgs;
	Variable result;
   CText variableName;
   WinData *win;

// Get window number *******************************   
	if((nrArgs = ArgScan(itfc,args,1,"window","e","d",&winNr)) < 0)
	   return(nrArgs);

// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}
	
// Find window class ******************************************************
   win = rootWin->FindWinByNr(winNr);
	if(!win)
	{
	   ErrorMessage("window %d undefined",winNr);
	   return(ERR);
	}

//   itfc->win = win;

// Add class object of each GUI control which has a name.
	for(ObjectData* obj: win->widgets.getWidgets())
   {
		if (!obj->hasDefaultObjectName()) // Only consider controls with defined names
		{
         result.MakeClass(OBJECT_CLASS,(void*)obj);
         variableName.Format("%s",obj->getObjectName());
         AssignToExpression(itfc, WINDOW, variableName.Str(), &result, true);
		}
   }

	itfc->nrRetValues = 0;
	return(OK);
}

/*************************************************************************
*  Extract control values and store in a list equating them to
*  the control names.
*************************************************************************/

int GetControlValues(Interface *itfc, char args[])
{
   static short winNr;
   short nrArgs;
	long pos,i;
   long len;
	WinData *win;
	CText dst = "list";
   CText method = "all";
   Variable valueVar;

// Get window number *******************************   
	if((nrArgs = ArgScan(itfc,args,1,"window_number,[destination, [method, [variable]]","eeee","dttv",&winNr,&dst,&method,&valueVar)) < 0)
	   return(nrArgs);

// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}
	
// Find window class ******************************************************
   win = rootWin->FindWinByNr(winNr);
	if(!win)
	{
	   ErrorMessage("window %d undefined",winNr);
	   return(ERR);
	}

// Check the method type
   if(method != "all" && method != "list" && method != "prefix" && method != "range" && method != "visible")
   {
	   ErrorMessage("invalid method. Should be one of: all/list/prefix/range");
	   return(ERR);
	}

   if(method == "list" && valueVar.GetType() != LIST)
   {
	   ErrorMessage("parameter value should be a list");
	   return(ERR);
	}
   if(method == "prefix" && valueVar.GetType() != UNQUOTED_STRING)
   {
	   ErrorMessage("parameter value should be a string");
	   return(ERR);
	}
   if(method == "range" && valueVar.GetType() != MATRIX2D && valueVar.GetDimX() != 2 &&  valueVar.GetDimY() != 1)
   {
	   ErrorMessage("parameter value should be a 2 element row vector");
	   return(ERR);
	}

	if(dst == "list")
	{

	// Make a list with this number of entries *********
		char **list = NULL;

	// Add name and value of each GUI object to list which has a name.
		pos = 0;
		for(ObjectData* obj: win->widgets.getWidgets())
		{
       // Limit check to those parameters passed in a list
         if(method == "list")
         {
            if(obj->readOnlyOutput) // No input comes from these controls
               continue;
            char **lst = valueVar.GetList();
            int sz = valueVar.GetDimX();
            int i;
            for(i = 0; i < sz; i++)
            {
               if(!strcmp(obj->valueName,lst[i])) // Is object parameter name in list?
                  break;
            }
            if(i == sz)
               continue; // Parameter not in list
         }
         else if(method == "prefix")
         {
            if(obj->readOnlyOutput) // No input comes from these controls
               continue;
            char *prefix = valueVar.GetString();
            int len1 = strlen(prefix);
            int len2 = strlen(obj->valueName);
            if(len1 < len2)
            {
               if(strncmp(obj->valueName,prefix,len1)) // if object has different prefix ignore it
                  continue;
            }
            else
               continue;
         }
         else if(method == "range") // Just consider controls in a fixed range
         {
            int start = valueVar.GetMatrix2D()[0][0];
            int end = valueVar.GetMatrix2D()[0][1];
            if(obj->readOnlyOutput) // No input comes from these controls
               continue;
            if(obj->nr() < start || obj->nr() > end)
               continue;
         }

			if(!obj->hasDefaultValueName()) // Only consider controls with defined names
			{
				if(method == "visible") // Only extract visible controls values
				{
					if(!obj->visible)
						continue;
				}		

				switch(obj->type)
				{
					case(TEXTBOX):
					case(TEXTMENU):
					case(STATICTEXT):
					{
                  char *entry = 0;
					   ProcessTextEntry(itfc, obj, &entry);
				   	AppendStringToList(entry,&list,pos++);
						delete [] entry;
						break;
					}
               case(UPDOWN):
               {
                  UpDownInfo* info = (UpDownInfo*)obj->data;
                  CText value;
                  value.Format("%s = %g", obj->valueName, info->value);
                  AppendStringToList(value.Str(), &list, pos++);
                  break;
               }
					case(CHECKBOX):
					{
						CheckButtonInfo *info = (CheckButtonInfo*)(obj->data);
						CArg carg;
						carg.Count(info->states);
						long value = SendMessage(obj->hWnd,BM_GETCHECK,0,0);
						len = strlen(obj->valueName)+6+strlen(info->states); // Length of entry string
						char *entry = new char[len]; // Allowing for name, value, quotes, equals sign & terminator
						char *str   = new char[len]; 
						strcpy(str,carg.Extract(value+1));

					// Make sure non-numeric values are quoted
						if(str[0] == '\0')      // Check for empty entry
							AddQuotes(str);
						else if(!IsNumber(str)) // Quote a non-numeric string
							AddQuotes(str);

						sprintf(entry,"%s = %s",obj->valueName,str);
						AppendStringToList(entry,&list,pos++);
						delete [] str;
						delete [] entry;
						break;
					}
					case(RADIO_BUTTON):
					{
						RadioButtonInfo *info = (RadioButtonInfo*)(obj->data);
						CArg carg;
						carg.Count(info->states);
						for(i = 0; i < info->nrBut; i++) // Get current radiobutton state
						{
							if(SendMessage(info->hWnd[i],BM_GETCHECK,0,0))
								break;
						}
						len = strlen(carg.Extract(i+1));

						char *value = new char[len+3]; // Allow for termination and possible quotes
						strcpy(value,carg.Extract(i+1));

					// Make sure non-numeric values are quoted
						if(value[0] == '\0')      // Check for empty entry
							AddQuotes(value);
						else if(!IsNumber(value)) // Quote a non-numeric string
							AddQuotes(value);

						char *entry = new char[len+3+strlen(obj->valueName)+3]; // Allow for value, name & equals
						sprintf(entry,"%s = %s",obj->valueName,value);
						AppendStringToList(entry,&list,pos++);
						delete [] value;
						delete [] entry;

						break;
					}
					case(SLIDER):
					{
						long trackPos;
						long type = GetWindowLong(obj->hWnd,GWL_STYLE);
						if(type & TBS_VERT) 
						{   
							long trackMax = SendMessage((HWND)obj->hWnd,TBM_GETRANGEMAX,0,0);	
							trackPos = trackMax-SendMessage((HWND)obj->hWnd,TBM_GETPOS,0,0);
						}
						else
						{
							trackPos = SendMessage((HWND)obj->hWnd,TBM_GETPOS,0,0);
						}
						char *entry = new char[strlen(obj->valueName) + 20];
						sprintf(entry,"%s = %ld",obj->valueName,trackPos);
						AppendStringToList(entry,&list,pos++);
						delete [] entry;
						break;
					}
					case(COLORBOX):
					{
						BYTE *data = (BYTE*)obj->data;
						if(data)
						{
							char *entry = new char[strlen(obj->valueName) + 20];
							sprintf(entry,"%s = [%hd,%hd,%hd]",obj->valueName,(short)data[0],(short)data[1],(short)data[2]);
							AppendStringToList(entry,&list,pos++);
							delete [] entry;
						}
						break;
					}
					case(PROGRESSBAR):
					{
		            int progValue = SendMessage((HWND)obj->hWnd,PBM_GETPOS,0,0);	
						char *entry = new char[strlen(obj->valueName) + 20];
						sprintf(entry,"%s = %d",obj->valueName,progValue);
						AppendStringToList(entry,&list,pos++);
						delete [] entry;
						break;
					}
					case(LISTBOX): // Return the current selection
					{
						long index = SendMessage(obj->hWnd,LB_GETCURSEL,0,0);
						if(index != LB_ERR)
						{
							long length = SendMessage(obj->hWnd,LB_GETTEXTLEN,(WPARAM)index,0);
							char *value = new char[length+1];
							SendMessage(obj->hWnd,LB_GETTEXT,(WPARAM)index,(LPARAM)value);
							char *entry = new char[strlen(obj->valueName)+length+6];
							sprintf(entry,"%s = \"%s\"",obj->valueName,value);
							AppendStringToList(entry,&list,pos++);
							delete [] entry;
							delete [] value;
						}
						else
						{
							char *entry = new char[strlen(obj->valueName)+6];
							sprintf(entry,"%s = \"\"",obj->valueName);
							AppendStringToList(entry,&list,pos++);
							delete [] entry;
						}
						break;
					}
					default:
					{
	//	            delete [] list;
	//					ErrorMessage("unsupported object type");
	//					return(ERR);
					}
				}
			}
		}

	// Return the new list in retVar along with an error message
		itfc->retVar[1].AssignList(list,(short)pos);
		itfc->nrRetValues = 1;
   }
	else if(dst == "struct")// Process structure destination
	{
	// Make a structure *********
		Variable *var,*struc;
      itfc->retVar[1].MakeStruct();
      struc = itfc->retVar[1].GetStruct();

	// Add name and value of each GUI object to list which has a name.
		pos = 0;
		for(ObjectData* obj: win->widgets.getWidgets())
		{
       // Limit check to those parameters passed in a list
         if(method == "list")
         {
            if(obj->readOnlyOutput) // No input comes from these controls
               continue;
            char **lst = valueVar.GetList();
            int sz = valueVar.GetDimX();
            int i;
            for(i = 0; i < sz; i++)
            {
               if(!strcmp(obj->valueName,lst[i])) // Is object parameter name in list?
                  break;
            }
            if(i == sz)
               continue; // Parameter not in list
         }
         else if(method == "prefix")
         {
            if(obj->readOnlyOutput) // No input comes from these controls
               continue;
            char *prefix = valueVar.GetString();
            int len1 = strlen(prefix);
            int len2 = strlen(obj->valueName);
            if(len1 < len2)
            {
               if(strncmp(obj->valueName,prefix,len1)) // if object has different prefix ignore it
                  continue;
            }
            else
               continue;
         }

			if(!obj->hasDefaultValueName()) // Only consider controls with defined names
			{
				if(method == "visible") // Only extract visible controls values
				{
					if(!obj->visible)
						continue;
				}	

				switch(obj->type)
				{
					case(TEXTBOX):
					case(TEXTMENU):
					case(STATICTEXT):
					{
						len = GetWindowTextLength(obj->hWnd);
						char *value = new char[len+2]; // Allow for termination and possible quotes and 'd'
						GetWindowText(obj->hWnd,value,len+1);
						if(obj->dataType == FLOAT64)	
							strcat(value,"d");

						var = struc->Add(STRUCTURE,obj->valueName);
						var->MakeAndSetString(value);
						delete [] value;
						break;
					}
					case(CHECKBOX):
					{
						CheckButtonInfo *info = (CheckButtonInfo*)(obj->data);
						CArg carg;
						carg.Count(info->states);
						long value = SendMessage(obj->hWnd,BM_GETCHECK,0,0);
						len = strlen(obj->valueName)+6+strlen(info->states); // Length of entry string
						char *str   = new char[len]; 
						strcpy(str,carg.Extract(value+1));
						var = struc->Add(STRUCTURE,obj->valueName);
						var->MakeAndSetString(str);
						delete [] str;
						break;
					}
					case(RADIO_BUTTON):
					{
						RadioButtonInfo *info = (RadioButtonInfo*)(obj->data);
						CArg carg;
						carg.Count(info->states);
						for(i = 0; i < info->nrBut; i++) // Get current radiobutton state
						{
							if(SendMessage(info->hWnd[i],BM_GETCHECK,0,0))
								break;
						}
						len = strlen(carg.Extract(i+1));

						char *value = new char[len+1]; // Allow for termination and possible quotes
						strcpy(value,carg.Extract(i+1));
						var = struc->Add(STRUCTURE,obj->valueName);
						var->MakeAndSetString(value);
						delete [] value;

						break;
					}
					case(SLIDER):
					{
						long trackPos;
						long type = GetWindowLong(obj->hWnd,GWL_STYLE);
						if(type & TBS_VERT) 
						{   
							long trackMax = SendMessage((HWND)obj->hWnd,TBM_GETRANGEMAX,0,0);	
							trackPos = trackMax-SendMessage((HWND)obj->hWnd,TBM_GETPOS,0,0);
						}
						else
						{
							trackPos = SendMessage((HWND)obj->hWnd,TBM_GETPOS,0,0);
						}
						var = struc->Add(STRUCTURE,obj->valueName);
						var->MakeAndSetFloat(trackPos);
						break;
					}
					case(COLORBOX):
					{
						BYTE *data = (BYTE*)obj->data;
						if(data)
						{
							float colors[3];
							colors[0] = data[0];
							colors[1] = data[1];
							colors[2] = data[2];
							var = struc->Add(STRUCTURE,obj->valueName);
							var->MakeMatrix2DFromVector(colors,3,1);
						}
						break;
					}
					case(LISTBOX): // Return the current selection
					{
						long index = SendMessage(obj->hWnd,LB_GETCURSEL,0,0);
						if(index != LB_ERR)
						{
							long length = SendMessage(obj->hWnd,LB_GETTEXTLEN,(WPARAM)index,0);
							char *value = new char[length+1];
							SendMessage(obj->hWnd,LB_GETTEXT,(WPARAM)index,(LPARAM)value);
							var = struc->Add(STRUCTURE,obj->valueName);
							var->MakeAndSetString(value);
							delete [] value;
						}
						else
						{
							var = struc->Add(STRUCTURE,obj->valueName);
							var->MakeAndSetString("");
						}
						break;
					}
					default:
					{
					}
				}
			}
		}
		itfc->nrRetValues = 1;
	}
	return(OK);
}

/******************************************************************************************************
 Convert the text stored in the UI object 'obj' into an assignment string (i.e. varName = varValue)
 which is return in  the string entry - this should be freed with delete [ when no longer required.
 How the text is processed depends on the expected type (obj->dataType) or if none is give on
 the text value.
******************************************************************************************************/

int ProcessTextEntry(Interface *itfc, ObjectData *obj, char **entry)
{
	int i;

// Get the text from the object 'obj'
	int len = GetWindowTextLength(obj->hWnd);
	char *value = new char[len+4]; // Allow for termination and possible quotes
	GetWindowText(obj->hWnd,value,len+1);
   value[len] = '\0';
	short cnt=0;
	int dataType = obj->dataType;

// If the text values is empty then make it a string type
	if(value[0] == '\0')
		dataType = UNQUOTED_STRING;

// Process the text based on the chosen data type for this object
	switch(dataType)
	{
		case(UNQUOTED_STRING):
		case(FILENAME):
		case(PATHNAME):
		{
			if(value[len-1] == '\\')
			{
				value[len] = '\\';
				len++;
				value[len] = '\0';
			}
			if(!isStr(value)) // Add quotes if not present
				AddQuotes(value);
			*entry = new char[len+3+cnt+strlen(obj->valueName)+4]; // Allow for value, name, equals and possible double prec id
			sprintf(*entry,"%s = %s",obj->valueName,value);
			break;
		}

		case(MATRIX2D): // If the expected data is an array then process it as such
		{
			*entry = CheckAndGetArrayString(obj,value);
			if(!entry)
			{
				ErrorMessage("Invalid array entry '%s' in object '%s'",value,obj->valueName);
				delete [] value;
				return(ERR);
			}
			break;
		}

		case(INTEGER): // If the expected data is numeric them remove any spaces/tab at the beginning and end
		case(FLOAT32):
		case(FLOAT64):
		{
			RemoveEndBlanks(value);
			if(IsNumber(value)) // Is it a simple number
			{
				*entry = new char[len+3+cnt+strlen(obj->valueName)+4]; // Allow for value, name, equals and possible double prec id
				if(obj->dataType == FLOAT64)	
					sprintf(*entry,"%s = %sd",obj->valueName,value);
				else
					sprintf(*entry,"%s = %s",obj->valueName,value);
			}
			else // See if it is an expression which evaluates to some numerical result
			{
				Variable result;
				char str[MAX_STR];
				strcpy(str,value); // Necessary?

				if(Evaluate(itfc,RESPECT_ALIAS,str,&result) >= 0) 
				{
					CText entry2;
					switch(result.GetType())
					{
						case(FLOAT32):
						{
							//entry = new char[len+3+cnt+strlen(obj->valueName)+4]; 
							if(obj->dataType == FLOAT64)
								entry2.Format("%s = %.9gd",obj->valueName,result.GetReal());
							else if(obj->dataType == FLOAT32)												
								entry2.Format("%s = %.9g",obj->valueName,result.GetReal());
							else if(obj->dataType == INTEGER)
								entry2.Format("%s = %d",obj->valueName,nint(result.GetReal()));
							*entry = new char[entry2.Size()+1];
							strcpy(*entry,entry2.Str());
							break;
						}
						case(FLOAT64):
						{
							if(obj->dataType == FLOAT64)
								entry2.Format("%s = %.17gd",obj->valueName,result.GetDouble());
							else if(obj->dataType == FLOAT32)												
								entry2.Format("%s = %.9g",obj->valueName,result.GetDouble());
							else if(obj->dataType == INTEGER)
								entry2.Format("%s = %ld",obj->valueName,nhint(result.GetDouble()));
							*entry = new char[entry2.Size()+1];
							strcpy(*entry,entry2.Str());
							break;
						}
						default: // Some other type so convert to a string
						{
							ProcessString(obj, value, entry, len); 
						}
					}
				}
				else
				{
					ProcessString(obj, value, entry, len); 
				}
			}
			break;
		}
		default: // In a texbox without data type so work out the type based on the value
		{
			if(IsNumber(value)) // Convert to a number if it is single precision
			{
				*entry = new char[len+3+cnt+strlen(obj->valueName)+4]; // Allow for value, name, equals and possible double prec id
				if(obj->dataType == FLOAT64)	
					sprintf(*entry,"%s = %sd",obj->valueName,value);
				else
					sprintf(*entry,"%s = %s",obj->valueName,value);
			}
			else if(IsDouble(value))  // Convert to a number it should be double precision (ending in a 'd')
			{
				*entry = new char[len+3+cnt+strlen(obj->valueName)+4]; // Allow for value, name, equals and possible double prec id
				if(obj->dataType == FLOAT64)	
					sprintf(*entry,"%s = %s",obj->valueName,value);
				else if(obj->dataType == FLOAT32)	
				{
					value[strlen(value)-1] = '\0';
					sprintf(*entry,"%s = %s",obj->valueName,value);
				}
				else
				{
					sprintf(*entry,"%s = %sd",obj->valueName,value);
				}
			}
			else // Is a string so add quotes
			{ // and also escape embedded quotes
				ProcessString(obj, value, entry, len);     
			}
		}
	}
	delete [] value;
	return(OK);
}

/******************************************************************************************************
 Convert the text stored in string 'value' into an assignment string (i.e. valueName = "value")
 which is returned in the string 'entry'. 'value' should be freed by the calling function with
 delete [] when no longer required.
******************************************************************************************************/

int ProcessString(ObjectData *obj, char *value, char **entry, int len)
{
	CText result;

	result = obj->valueName;
	result = result + " = \"";
	result = result + value;
	result = result + "\"";

   *entry = new char[result.Size()+1];
	strcpy(*entry,result.Str());
	return(OK);
}

/**************************************************************************************
  Check that string 'value' represents a matrix with or without brackets. Return matrix
  string with brackets.
**************************************************************************************/

char* CheckAndGetArrayString(ObjectData* obj, char *value)
{
   if(CheckForValidArrayString(value))
   {
      if(value[0] == '[')
      {
         char *name = obj->valueName;
         char *out = new char[strlen(name) + 3 + strlen(value)+1];
         sprintf(out,"%s = %s",obj->valueName,value);
         return(out);
      }
      else
      {
         char *name = obj->valueName;
         char *out = new char[strlen(name) + 5 + strlen(value)+1];
         sprintf(out,"%s = [%s]",obj->valueName,value);
         return(out);
      }
   }
   else
      return(0);
}

/**************************************************************************************
  Check that string 'value' represents a matrix with or without brackets. Return true
  if OK false if not
**************************************************************************************/

bool CheckForValidArrayString(char* value)
{
   short pos = 0, pos2 = 0;
   char operand[MAX_STR];
   int len = strlen(value);
   short type;

   while(1)
   {
      if(pos >= len)
      {
         return(false);
      }
      type = GetNextOperand(pos2, value+pos, operand);
      pos += pos2;
      pos2 = 0;
      if(type == MATRIX2D)
      {
          return(true);
      }
      else if(type == FLOAT32)
      {
         if(pos < len)
         {
            type = GetNextOperand(pos2, value+pos, operand);
            pos += pos2;
            pos2 = 0;
            if(type == OPERATOR_TOKEN)
            {
               pos++;
            }
            else
            {
               return(false);
            }
         }
         else
         {
            return(true);
         }
      }
      else
      {
         return(false);
      }
   }  
   return(false);
}

/**************************************************************************************
  Search for a window number via its name

  winNr = findwin(method,value)

  method ... "name", "title" or "partialTitle" at present
  value .... the name of the window (as stored in the win->name variable)

**************************************************************************************/

int FindWindowCLI(Interface *itfc, char args[])
{
   short nrArgs;
   WinData *win = 0;
   CText method;
   CText value;
   
// Get arguments - method and value
   if((nrArgs = ArgScan(itfc,args,2,"method, value","ee","tt",&method,&value)) < 0)
     return(nrArgs);

// Loop over all GUI windows
   if(method == "title")
   {
    // Scan through current GUI windows looking for one with matching title (if any)
      win = rootWin->FindWinByTitle(value.Str());
      if(win)
         itfc->retVar[1].MakeAndSetFloat((float)win->nr);
      else
         itfc->retVar[1].MakeAndSetFloat((float)-1);
   }
   else if (method == "partialTitle")
   {
      // Scan through current GUI windows looking for one with matching title (if any)
      win = rootWin->FindWinByPartialTitle(value.Str());
      if (win)
         itfc->retVar[1].MakeAndSetFloat((float)win->nr);
      else
         itfc->retVar[1].MakeAndSetFloat((float)-1);
   }
   else if(method == "name")
   {
    // Scan through current GUI windows looking for one with matching name (if any)
      win = rootWin->FindWinByName(value.Str());
      if(win)
         itfc->retVar[1].MakeAndSetFloat((float)win->nr);
      else
         itfc->retVar[1].MakeAndSetFloat((float)-1);
   }
   else
   {
      ErrorMessage("invalid method options = [\"title\"]");
      return(ERR);
   }

   itfc->nrRetValues = 1;
   
   return(OK);
}

/*************************************************************************
   Check to see if a GUI window with the specified name exists
*************************************************************************/
 
bool FindGUIWindow(const char *name)
{
   CText txt;
   Interface itfc;
   
   txt.Format("\"name\",\"%s\"",name);
   FindWindowCLI(&itfc,txt.Str());
   if(itfc.nrRetValues == 1 && itfc.retVar[1].GetType() == FLOAT32)
   {
      int ret = itfc.retVar[1].GetReal();
      if(ret == -1)
         return(false);
      else
         return(true);
   }
   return(false);
}

/*************************************************************************
   Extract list entries from var and use these to 
   update the control values in the current dialog interface.
 
  The list entries should take the form:

  par1 = value1
  par2 = value2
  par3 = value3
  ...

  Note: entries are limited to 500 characters

*************************************************************************/


int SetControlValues(Interface *itfc, char arg[])
{
   static short winNr;
   short nrArgs,err;
	char entry[MAX_STR];
	char value[MAX_STR];
	char name[MAX_STR];
	Variable var,result;
	WinData *win;
   extern void StripTrailingZerosFromString(char* str);


// Get window number and list variable *******************************   
	if((nrArgs = ArgScan(itfc,arg,2,"window, list","ee","dv",&winNr,&var)) < 0)
	   return(nrArgs);

// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}
	
// Find window class ******************************************************
   win = rootWin->FindWinByNr(winNr);
	if(!win)
	{
	   ErrorMessage("window %d undefined",winNr);
	   return(ERR);
	}
 
// Null variable so do nothing
   if(var.GetType() == NULL_VARIABLE)
      return(OK);

// Get a pointer to the list object
	if(var.GetType() != LIST)
	{
	   ErrorMessage("variable is not a list");
	   return(ERR);
	}
	char **list = var.GetList();
	long size = var.GetDimX();


   bool processEscapesBak = itfc->processEscapes;
   itfc->processEscapes = false;

// Extract each list entry, parse it and then set the corresponding object 
	for(long i = 0; i < size; i++)
	{
		StrNCopy(entry,list[i],MAX_STR-1);
		if(ParseAssignmentString(entry,name,value) == OK)
		{
			ObjectData* obj = win->widgets.findByValueID(name);
			if(obj) 
			{
				RemoveQuotes(value);
            short sz = strlen(value);
				switch(obj->type)
				{
				   case(STATUSBOX):
				   case(TEXTMENU):
					case(TEXTBOX):
                  ReplaceEscapedQuotes(value);
                  if(obj->dataType == FLOAT64 && IsDouble(value))
                     value[sz-1] = '\0';
                  // 'd' removal
                  //if(obj->dataType == FLOAT64)
                  //{
                  //   if(IsDouble(value))
                  //      value[sz-1] = '\0';
                  //   else
                  //      StripTrailingZerosFromString(value);
                  //}
                  //else if(obj->dataType == FLOAT32)
                  //   StripTrailingZerosFromString(value);

                  if(obj->dataType == PATHNAME && (sz >= 2 && value[sz-1] == '\\' && value[sz-2] == '\\'))
                     value[sz-1] = '\0';
                  if (obj->dataType == HEX)
                  {
                     float num;
                     if(sscanf(value, "%f", &num) == 1)
                        sprintf(value, "0x%X", nint(num));
                     else
                     {
                        ErrorMessage("invalid type for hex conversion value (%s)", value);
                        return(ERR);
                     }
                  }
				      SetWindowText(obj->hWnd,value);
						break;
					//case(TEXTEDITOR):
				 //     SetWindowText(obj->hWnd,value);
					//	break;
					case(STATICTEXT):
                  sprintf(entry,"%hd, %hd, \"text\", \"%s\"",winNr, obj->nr(),value);
                  SetParameter(itfc, entry);
						break;
					case(PROGRESSBAR):
                  err = Evaluate(itfc,RESPECT_ALIAS,value,&result);
                  if(err == ERR) 
                  {
                     itfc->processEscapes = processEscapesBak;
                     return(ERR);
                  }

                  if(result.GetType() != FLOAT32)
                  {
                     itfc->processEscapes = processEscapesBak;
                     ErrorMessage("invalid type for trackbar value");
                     return(ERR);
                  }
                  SetProgressBarValue(obj,nint(result.GetReal()));
						UpdateObject(obj);  
						break;
					case(CHECKBOX):
					{
						CheckButtonInfo *info = (CheckButtonInfo*)(obj->data);
						CArg carg;
						carg.Count(info->states);
						long state = 0;
						long j;
						for(j = 1; j <= 2; j++)
						{
							if(!strcmp(value,carg.Extract(j))) 
							{
								state = j-1;
								break;
							}
						}
						if(j == 3)
						{
							ErrorMessage("invalid checkbox (obj no. = %hd) status string (%s)",obj->nr(),entry);
                     itfc->processEscapes = processEscapesBak;
							return(ERR);      
						}
						SendMessage(obj->hWnd,BM_SETCHECK,state,0);
						UpdateObject(obj);  
						break;
					}
					case(RADIO_BUTTON):
					{
                  RadioButtonInfo *info = (RadioButtonInfo*)(obj->data);
						CArg carg;
						carg.Count(info->states);
						for(long j = 1; j <= info->nrBut; j++)
						{
							if(!strcmp(value,carg.Extract(j))) 
                        SendMessage(info->hWnd[j-1],BM_SETCHECK,1,0);
                     else
                        SendMessage(info->hWnd[j-1],BM_SETCHECK,0,0);
						}
						UpdateObject(obj);  
						break;
					}
               case(COLORBOX):
               {
	               short err = Evaluate(itfc,RESPECT_ALIAS,value,&result);
	               if(err == ERR)
                  {
                     itfc->processEscapes = processEscapesBak;
                     return(ERR);
                  }
               
                  if(result.GetType() == MATRIX2D && result.GetDimX() == 3 && result.GetDimY() == 1)
                  {
                     obj->data[0]  = nint(result.GetMatrix2D()[0][0]);
                     obj->data[1]  = nint(result.GetMatrix2D()[0][1]);
                     obj->data[2]  = nint(result.GetMatrix2D()[0][2]);
                  }
               // Make sure the object is redrawn
                  MyInvalidateRect(obj->hWnd,NULL,false);
  						UpdateObject(obj);  
						break;        
               }
               case(SLIDER):
               {
                  long trackPos; 
                  short err = Evaluate(itfc,RESPECT_ALIAS,value,&result);
                  if(err == ERR) 
                  {
                     itfc->processEscapes = processEscapesBak;
                     return(ERR);
                  }

                  if(result.GetType() != FLOAT32)
                  {
                     itfc->processEscapes = processEscapesBak;
                     ErrorMessage("invalid type for trackbar value");
                     return(ERR);
                  }
                  long type = GetWindowLong(obj->hWnd,GWL_STYLE);
                  if(type & TBS_VERT) // Range is inverted wrt our requirements when vertical
                  {   
                    long trackMax = SendMessage((HWND)obj->hWnd,TBM_GETRANGEMAX,0,0);	
                    long trackMin = SendMessage((HWND)obj->hWnd,TBM_GETRANGEMIN,0,0);	
                    trackPos = trackMax+trackMin-nint(result.GetReal());
                  }
                  else
                  {
                     trackPos = nint(result.GetReal());
                  }	      
                  trackPos = SendMessage((HWND)obj->hWnd,TBM_SETPOS,true,trackPos);
  						UpdateObject(obj);  
						break; 
               }
					default:
					{
     //             itfc->processEscapes = processEscapesBak;
	//					ErrorMessage("unsupported object type");
	//					return(ERR);
					}
				}
			}
		}
		else
		{
         itfc->processEscapes = processEscapesBak;
			ErrorMessage("invalid assignment in list entry '%s'",entry);
			return(ERR);
		}
	}
   itfc->processEscapes = processEscapesBak;
	return(OK);
}

// Helper function for CheckControlValues
// Sets the inError flag while displaying a message box.
// This draws a red rectangle around the offending textbox or textmenu.

void ShowObjectError(ObjectData *obj, char *errTxt)
{
   extern void GetSubWindowRect(HWND parent,HWND child, RECT *r);
   RECT r;
   CText extendedTxt;

   SetRect(&r,obj->xo,obj->ho,obj->xo+obj->wo,obj->yo+obj->ho);
   if(obj->tabParent)
   {
      TabCtrl_SetCurSel(obj->tabParent->hWnd,obj->tabPageNr);
      ControlVisibleWithTabs(obj->winParent,obj->tabParent);
   }
   if(obj->panelParent)
   {
      RECT pRect,intersectRect;
      PanelInfo *pInfo = (PanelInfo*)obj->panelParent->data;
      GetClientRect(pInfo->hWndPanel,&pRect);
      GetSubWindowRect(pInfo->hWndPanel,obj->hWnd,&r);
      IntersectRect(&intersectRect,&r,&pRect);
      MyInvalidateRect(pInfo->hWndPanel,&intersectRect,true);
      obj->inError = true;
      MyUpdateWindow(pInfo->hWndPanel);
      if(!obj->visible)
      {
         extendedTxt = errTxt;
         extendedTxt = extendedTxt + "\r\rNote : control is currently hidden.";
         MessageDialog(prospaWin,MB_ICONERROR,"Error",extendedTxt.Str());
      }
      else
         MessageDialog(prospaWin,MB_ICONERROR,"Error",errTxt);
      
      obj->inError = false;
      InflateRect(&r,2,2);
      MyInvalidateRect(pInfo->hWndPanel,&r,true);
      MyUpdateWindow(pInfo->hWndPanel);
   }
   else
   {
      MyInvalidateRect(obj->hWnd,NULL,true);
      obj->inError = true;
      SetFocus(obj->hWnd);
      MyUpdateWindow(obj->winParent->hWnd);
      if(!obj->visible)
      {
         extendedTxt = errTxt;
         extendedTxt = extendedTxt + "\r\rNote : control is currently hidden.";
         MessageDialog(prospaWin,MB_ICONERROR,"Error",extendedTxt.Str());
      }
      else
         MessageDialog(prospaWin,MB_ICONERROR,"Error",errTxt);
      obj->inError = false;
      InflateRect(&r,2,2);
      MyInvalidateRect(obj->winParent->hWnd,&r,true);
      MyUpdateWindow(obj->winParent->hWnd);
   }

}

/***************************************************************************
*
*   Check to see if all object parameters are of the correct type and range
*
*  Syntax: error = checkvalues(window_nr)
*
****************************************************************************/

int CheckControlValues(Interface* itfc, char args[])
{
	short nrArgs;
	static short winNr;
   WinData *win = 0;
	CText value;
	float result;
	short type;
	CText typeStr = "float";
   CText errTxt;
   CText method = "all";
   Variable valueVar;

// Get window number **************
	if((nrArgs = ArgScan(itfc,args,1,"window, method, value","eee","dtv",&winNr,&method,&valueVar)) < 0)
		return(nrArgs);

   if(method != "all" && method != "prefix" && method != "list")
   {
	   ErrorMessage("invalid method. Should be one of: all/prefix/list");
	   return(ERR);
	}

   if(method == "prefix" && valueVar.GetType() != UNQUOTED_STRING)
   {
	   ErrorMessage("prefix value should be a string");
	   return(ERR);
	}
   if(method == "list" && valueVar.GetType() != LIST)
   {
	   ErrorMessage("prefix value should be a list");
	   return(ERR);
	}

// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}

// Find window instance ************************
	if(!(win = rootWin->FindWinByNr(winNr)))
	{
	   ErrorMessage("window '%ld' not found",(long)winNr);
	   return(ERR);
	}

// Loop over all controls in window ************ 
	itfc->retVar[1].MakeAndSetString("ok");

   for(ObjectData* obj: win->widgets.getWidgets())
   {

    // Limit checks to those parameters with a certain prefix
      if(method == "prefix")
      {
         if(obj->readOnlyOutput) // No input comes from these controls
            continue;
         char *prefix = valueVar.GetString();
         int len1 = strlen(prefix);
         int len2 = strlen(obj->valueName);
         if(len1 < len2)
         {
            if(strncmp(obj->valueName,prefix,len1)) // if object has different prefix ignore it
               continue;
         }
         else
            continue;
      }
    // Limit check to those parameters passed in a list
      else if(method == "list")
      {
         if(obj->readOnlyOutput) // No input comes from these controls
            continue;
         char **lst = valueVar.GetList();
         int sz = valueVar.GetDimX();
         int i;
         for(i = 0; i < sz; i++)
         {
            if(!strcmp(obj->valueName,lst[i])) // Is object parameter name in list?
               break;
         }
         if(i == sz)
            continue; // Parameter not in list
      }

   // Only do checks on Textboxes and Textmenus
	   if((obj->type == TEXTBOX || obj->type == TEXTMENU) && obj->dataType != NULL_VARIABLE)
		{
         extern bool IsString(char *str);
		// An error has occurred if there is a type mismatch or the control data
		// is out of range.
      // Get the text
			GetWindowTextEx(obj->hWnd, value);

       // See if the string represents an array of numbers with or without square brackets
         if(obj->dataType == MATRIX2D)
         {
            extern bool CheckForValidArrayString(char* value);

            if(!CheckForValidArrayString(value.Str()))
            {
               errTxt.Format("Type mismatch error in control with red border.\rExpecting array entry\r\rControl variable name is: '%s'.",obj->valueName);
               ShowObjectError(obj,errTxt.Str());
               itfc->retVar[1].MakeAndSetString("error");
               itfc->nrRetValues = 1;
               return(OK); 
            }         
         }
         else
         {
         // Work out the type of the entered data
            if(IsString(value.Str()))
            {
               if(value == "")
                 type = BLANKSTR;
               else
                 type = QUOTED_STRING;
            }
            else
            {
               short pos = 0;
               char operand[MAX_STR];
               type = GetNextOperand(pos, value.Str(), operand);
               if(type == FLOAT32 || type == FLOAT64)
               {
                  result = (float)StringToNumber(operand,type);
                  if(nint(result) == result)
                     type = INTEGER;
               }
            }
		   //	type = GetDataType(value.Str(),&result);
			   if(type == QUOTED_STRING) type = UNQUOTED_STRING;
         // Note the expected type
            switch(obj->dataType)
            {
               case(INTEGER): 
                  typeStr = "integer";
                  break;
               case(HEX):
                  typeStr = "hex";
                  break;
               case(FLOAT32):
                  typeStr = "float";
                  break;
               case(FLOAT64):
                  typeStr = "double";
                  break;
               case(FLOAT32ORBLANK):
                  typeStr = "float or blank";
                  break;
               case(FLOAT64ORBLANK):
                  typeStr = "double or blank";
                  break;
               case(FILENAME):
                  typeStr = "filename";
                  break;
               case(PATHNAME):
                  typeStr = "pathname";
                  break;
               case(UNQUOTED_STRING):
                  typeStr = "string";
                  break;
               case(MATRIX2D):
                  typeStr = "array";
                  break;
               default:
                  typeStr = "NONE";
                  break;
            }

         
         // Check for mismatches between actual type and expected type
            bool expectNumber = (obj->dataType == INTEGER || obj->dataType == HEX || obj->dataType == FLOAT32  || obj->dataType == FLOAT64 || obj->dataType == FLOAT32ORBLANK || obj->dataType == FLOAT64ORBLANK);
            if((type == UNQUOTED_STRING && expectNumber)       || 
			      (type == FILENAME && expectNumber)              ||
			      (type == PATHNAME && expectNumber)              ||
               (type == FLOAT32 && obj->dataType == INTEGER)   ||
               (type == FLOAT32 && obj->dataType == HEX)       ||
               (type == BLANKSTR &&  obj->dataType == INTEGER) ||
               (type == BLANKSTR &&  obj->dataType == FLOAT32) || 
               (type == BLANKSTR &&  obj->dataType == FLOAT64))

			   {
               errTxt.Format("Type mismatch error in control with red border.\rExpecting data of type: %s\r\rControl variable name is: '%s'.",typeStr.Str(),obj->valueName);
               ShowObjectError(obj,errTxt.Str());
               itfc->retVar[1].MakeAndSetString("error");
				   break;
			   }
            else if(type == UNQUOTED_STRING && obj->dataType == FILENAME && !IsValidFileName(value.Str()))
            {
               errTxt.Format("Type mismatch error in control with red border.\rExpecting valid filename\r\rControl variable name is: '%s'.",obj->valueName);
               ShowObjectError(obj,errTxt.Str());
               itfc->retVar[1].MakeAndSetString("error");
               break;
            }
            else if(type == UNQUOTED_STRING && obj->dataType == PATHNAME && !IsValidPathName(value.Str()))
            {
               errTxt.Format("Type mismatch error in control with red border.\rExpecting valid pathname (remove any trailing '\\' or spaces)\r\rControl variable name is: '%s'.",obj->valueName);
               ShowObjectError(obj,errTxt.Str());
               itfc->retVar[1].MakeAndSetString("error");
               break;
            }
         // Range checks
			   if(obj->dataType == FLOAT32 && obj->rangeCheck)
			   {
				   if(FloatLess(result,obj->lower) || FloatGreater(result,obj->upper))
				   {
                  errTxt.Format("Range error in control with red border.\rExpecting a number between %g and %g\r\rControl variable name is: '%s'.",obj->lower,obj->upper,obj->valueName);
                  ShowObjectError(obj,errTxt.Str());
                  itfc->retVar[1].MakeAndSetString("error");
					   break;
				   }
			   }
			   else if(obj->dataType == FLOAT64 && obj->rangeCheck)
			   {
				   if(DoubleLess(result,obj->lower) || DoubleGreater(result,obj->upper))
				   {
                  errTxt.Format("Range error in control with red border.\rExpecting a number between %g and %g\r\rControl variable name is: '%s'.",obj->lower,obj->upper,obj->valueName);
                  ShowObjectError(obj,errTxt.Str());
                  itfc->retVar[1].MakeAndSetString("error");
					   break;
				   }
			   }
            else if((obj->dataType == INTEGER || obj->dataType == HEX) && obj->rangeCheck)
			   {
				   if(nint(result) < nint(obj->lower) || nint(result) > nint(obj->upper))
				   {
                  errTxt.Format("Range error in control with red border.\rExpecting a number between %g and %g\r\rControl variable name is: '%s'.",obj->lower,obj->upper,obj->valueName);
                  ShowObjectError(obj,errTxt.Str());
                  itfc->retVar[1].MakeAndSetString("error");
					   break;
				   }
			   }
		   }   
      }
   }  

// We are returning with "ok" or "error"
   itfc->nrRetValues = 1;

   return(OK); 
}


/*************************************************************************
*          Command line interface hide but not delete a window
*************************************************************************/

int HideMyWindow(Interface* itfc ,char arg[])
{
	short nrArgs;
	Variable var;
	WinData *win;

// Get argument (window number or name) ***********************	
	if((nrArgs = ArgScan(itfc,arg,1,"window","e","v",&var)) < 0)
	   return(nrArgs);

// Process if its a window number *****************************	
	if(var.GetType() == FLOAT32)
	{
	   short winNr = nsint(var.GetReal());

		if(winNr == 0)
		{
			if((winNr = GetCurWinNr(itfc)) == ERR)
				return(ERR);
		}
   
	   win = rootWin->FindWinByNr(winNr);
		if(!win)
		{
		  ErrorMessage("window %d undefined",winNr);
		  return(ERR);
		}
	   ShowWindow(win->hWnd,SW_HIDE);
      win->visible = false;
   }
   else
   {
      ErrorMessage("Invalid window number");
      return(ERR);
   }
   

	itfc->nrRetValues = 0;  
   return(OK);
}

void RecalculateObjectPositions(WinData *win)
{
   RECT r;

   GetClientRect(win->hWnd,&r);
   long yoff = 0;
   long ww = r.right;
   long wh = r.bottom;

// If fixed objects selected then 
   if(win->fixedObjects)
   {
   // Allow for status boxes
      if(win->statusbox)
      {
         GetClientRect(win->statusbox->hWnd,&r);
         wh -= r.bottom-1;
      }

   // Allow for toolbars
      if(win->toolbar)
      {
         GetClientRect(win->toolbar->hWnd,&r);
         yoff = r.bottom+3;
         wh -= r.bottom+3;
      }
   }

// Update object positions
	for(ObjectData* obj: win->widgets.getWidgets())
   {
      obj->xo = nint(obj->xSzScale*ww + obj->xSzOffset);
      obj->yo = nint(obj->ySzScale*wh + obj->ySzOffset + yoff);
      obj->wo = nint(obj->wSzScale*ww + obj->wSzOffset);
      obj->ho = nint(obj->hSzScale*wh + obj->hSzOffset);
   }


}

/*************************************************************************
           Command line interface to show a window which was hidden
           Can either display a gui window by passing its numbers or
           a normal prospa window by passing its name.
*************************************************************************/

int ShowMyWindow(Interface* itfc ,char args[])
{
	short nrArgs;
	Variable var;
	WinData *win;
   CText mode = "normal";

// Get argument (window number or name) ***********************	
	if((nrArgs = ArgScan(itfc,args,1,"window, [mode]","ee","vt",&var,&mode)) < 0)
	   return(nrArgs);
 

// Process if its a window number *****************************	
	if(VarType(&var) == FLOAT32)
	{
	   short winNr = nsint(VarReal(&var));

		if(winNr == 0)
		{
			if((winNr = GetCurWinNr(itfc)) == ERR)
				return(ERR);
		}
   
	   win = rootWin->FindWinByNr(winNr);
		if(!win)
		{
		  ErrorMessage("window %d undefined",winNr);
		  return(ERR);
		}


   // Check for dialog
      if(win->type == DIALOG_WIN)
      {
         TextMessage("\n\n !!!Dialog should be opened with showdialog!!!\n\n");
         return(ShowDialog(itfc,args));
      }

      if(nrArgs == 2)
      {
         if(mode == "recalc")
         {
            win->fixedObjects = true;
            RecalculateObjectPositions(win);
         }
      }

      // Make sure tab controls which are not visible are hidden
      if(win->updateControlVisibilityFromTabs() == ERR)
         return(ERR);

      // Make sure panels are at the bottom
      // TODO

      win->activate();								// Activate window (i.e. not in edit mode
      if(win->keepInFront)
      {
         if(IsZoomed(win->hWnd) || mode == "maximized")
	         ShowWindow(win->hWnd,SW_SHOWMAXIMIZED); // Display the window maximised 
         else
            ShowWindow(win->hWnd,SW_SHOWNORMAL); // Display the window
         SetCurrentGUIParent();               
      }

      else
      {
         ChangeGUIParent(win->hWnd);
         if(IsZoomed(win->hWnd) || mode == "maximized")
	         ShowWindow(win->hWnd,SW_SHOWMAXIMIZED); // Display the window maximised
         else
            ShowWindow(win->hWnd,SW_SHOWNORMAL); // Display the window
      }

// Make sure all controls accept the focus
// (a manifest bug means that some controls do not)
      SendMessage(win->hWnd,WM_UPDATEUISTATE,UIS_CLEAR + (UISF_HIDEFOCUS * 65536),0);

      MyUpdateWindow(win->hWnd);             // Force it to redraw now

   // Make sure all control objects are assigned as local variables
      CText nr;
      nr.Format("%hd",winNr);
	   AssignControlObjects(itfc,nr.Str());
   }
   
// Process if its a window name *****************************   
   else if(VarType(&var) == UNQUOTED_STRING)
   {
      if(!strcmp("prospa",VarString(&var)))
      {
         ChangeGUIParent(prospaWin);
	      SetWindowPos(prospaWin,HWND_TOPMOST	,0,0,0,0,SWP_NOMOVE |SWP_NOSIZE); 
	      SetWindowPos(prospaWin,HWND_NOTOPMOST	,0,0,0,0,SWP_NOMOVE |SWP_NOSIZE); 
      }
      else
      {
		   ErrorMessage("invalid window name");
		   return(ERR);
		}         
	}

   win->visible = true;

   if(IsWindowVisible(cliWin)) 
      reportErrorToDialog = false;

	itfc->nrRetValues = 0;
	return(OK);
}

short UpdateControlVisibilityFromTabs(WinData *win)
{
	for(ObjectData* obj: win->widgets.getWidgets())
   { 
		if(obj->type != TABCTRL)
		{
			if(obj->visible && obj->isTabChild())
			{
				ObjectData* tab = obj->getTabParent();
            if(!tab || tab->type != TABCTRL)
            {
               ErrorMessage("tab parent for control %d is not a valid tab control");
               return(ERR);
            }
				int tabNumber = TabCtrl_GetCurSel(tab->hWnd);
				if(obj->tabPageNr != tabNumber)
				{
					obj->visible = false;
					obj->Show(false);
				}
			}
		}
      else // Ensure that tab controls are at the bottom and therefore visible 
      {
         if(obj->visible)
            SetWindowPos(obj->hWnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
      }
   }
   return(OK);
}

/*************************************************************************
           Command line interface to show a window which was hidden
           Can either display a gui window by passing its numbers or
           a normal prospa window by passing its name.
*************************************************************************/

int CompleteWindow(Interface* itfc ,char args[])
{
	short nrArgs;
	Variable var;
	WinData *win;
   CText mode = "normal";

// Get argument (window number or name) ***********************	
	if((nrArgs = ArgScan(itfc,args,1,"window, [mode]","ee","vt",&var,&mode)) < 0)
	   return(nrArgs);
 
// Process if its a window number *****************************	
	if(VarType(&var) == FLOAT32)
	{
	   short winNr = nsint(VarReal(&var));

		if(winNr == 0)
		{
			if((winNr = GetCurWinNr(itfc)) == ERR)
				return(ERR);
		}

	   win = rootWin->FindWinByNr(winNr);
		if(!win)
		{
		  ErrorMessage("window %d undefined",winNr);
		  return(ERR);
		}

   // Check for dialog
      if(win->type == DIALOG_WIN)
      {
         TextMessage("\n\n !!!Dialog should be opened with showdialog!!!\n\n");
         return(ShowDialog(itfc,args));
      }

      if(nrArgs == 2)
      {
         if(mode == "recalc")
         {
            win->fixedObjects = true;
            RecalculateObjectPositions(win);
         }
      }
      win->activate();                 // Activate window (i.e. not in edit mode
   }
	
	itfc->nrRetValues = 0;
	return(OK);
}

/*************************************************************************
*  Command line interface to bring a window to the top and keep it there
*************************************************************************/

int KeepWindowOnTop(Interface* itfc ,char arg[])
{
	short nrArgs;
	Variable var;
	WinData *win;

// Get argument (window number or name) ***********************
	if((nrArgs = ArgScan(itfc,arg,1,"window","e","v",&var)) < 0)
	   return(nrArgs);
 
// Process if its a window number *****************************	
	if(VarType(&var) == FLOAT32)
	{
	   short winNr = nsint(VarReal(&var));

		if(winNr == 0)
		{
			if((winNr = GetCurWinNr(itfc)) == ERR)
				return(ERR);
		}
   
	   win = rootWin->FindWinByNr(winNr);
		if(!win)
		{
		  ErrorMessage("window %d undefined",winNr);
		  return(ERR);
		}
	// Force window to be topmost
		SetWindowPos(win->hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE |SWP_NOSIZE);
   }
   else
   {
      ErrorMessage("window not found");
      return(ERR);
   }
 
	itfc->nrRetValues = 0;
	return(OK);
}

// Set the keepcurrentfocus flag
// Note that nested calls are ignored only the outer calls are noticed

bool gKeepCurrentFocus = false; // Allows window to display without getting put into background 
int gFocusCnt = 0; // Counter to keep track of multiple calls.

int KeepFocus(Interface* itfc ,char arg[])
{
	short nrArgs;
	CText response;

// Get argument (keep focus or not) ***********************
   if(gKeepCurrentFocus == true)
      response = "true";
   else
      response = "false";

	if((nrArgs = ArgScan(itfc,arg,1,"window","e","t",&response)) < 0)
	   return(nrArgs);

   if(response == "true")
   {
      gFocusCnt++;
      if(gFocusCnt == 1)
      {
         gKeepCurrentFocus = true;
      }
   }
   if(response == "false")
   {
      gFocusCnt--;
      if(gFocusCnt == 0)
      {
         gKeepCurrentFocus = false;
      }
   }

	itfc->nrRetValues = 0;
	return(OK);
}



/*************************************************************************
*       Command line interface to return the next free window number
*************************************************************************/

int NextWindowNumber(Interface* itfc, char args[])
{
   short winNr = rootWin->FindNextFreeNr();
   
   itfc->retVar[1].MakeAndSetFloat((float)winNr);
   itfc->nrRetValues = 1;
   
   return(OK);
}

/*************************************************************************
*       Set or get the current GUI window number
*************************************************************************/

int GUIWindowNumber(Interface* itfc, char args[])
{
   short nrArgs;
   short winNr;
   WinData *win;

	winNr = GetCurWinNr(itfc);
   //if(GetGUIWin())
   //   winNr = GetGUIWin()->nr;
   //else
   //   winNr = -1;

	if((nrArgs = ArgScan(itfc,args,1,"winNr","e","d",&winNr)) < 0)
	   return(nrArgs);

   win = rootWin->FindWinByNr(winNr);
   if(win)
   {
		WinData::SetGUIWin(win); // Set current GUI window
      itfc->win = win;        // Ensure window variables from this window can be accessed
   }
   else
   {
		WinData::SetGUIWin(NULL); // Set current GUI window
      itfc->win = NULL;  
   }

	itfc->nrRetValues = 0;
   return(OK);
}

/*************************************************************************
*       Command line interface to destroy a window
*************************************************************************/

int DestroyMyWindow(Interface* itfc ,char args[])
{
	short nrArgs;
	static short winNr;
	WinData *win;
   CText mode = "close";
   bool mainWin = false;

// Get argument (window number) ******************************
	if((nrArgs = ArgScan(itfc,args,1,"winNr, [mode]","ee","dt",&winNr,&mode)) < 0)
	   return(nrArgs);

// Check for current window (window number = 0) **************
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	} 

// Find window class *****************************************
	win = rootWin->FindWinByNr(winNr);
	if(!win)
	{
	   ErrorMessage("window %d undefined",winNr);
	   return(ERR);
	}

// Check for dialog
   if(win->type == DIALOG_WIN)
   {
      TextMessage("\n\n !!!Dialog should be closed with closedialog!!!\n\n");
      return(CloseDialog(itfc,""));
   }

   if(win->isMainWindow)
   {
    //  TextMessage("Destroying sub windows\n");

      WinData *w,*tmp,*next = rootWin->next;
      w = next;
      while(w != NULL)
      {
         tmp = w->next;
         if(!w->isMainWindow)
         {
            HWND hWndBak = w->hWnd; 
            delete w;
            DestroyWindow(hWndBak); 
         }
         w = tmp;          
      }
      mainWin = true;

		Plot1D::setNoCurPlot();
		Plot2D::setNoCurPlot();
      curEditor = NULL;
      cliEditWin = NULL;
      cliWin = NULL;
      plot3d = NULL;
      cur3DWin = NULL;
      itfc->obj = NULL; // Calling object has been deleted
      itfc->win = NULL; // Calling window has been deleted.
   }
   else
   {
      if(itfc->win == win)
         itfc->win = NULL;
      if(itfc->obj && itfc->obj->winParent == win)
         itfc->obj = NULL;
   }

// Free window memory and then destroy the window ************
   HWND hwnd = win->hWnd;

   if(win == gDebug.win)
   {
      gDebug.enabled = false;
      gDebug.step = true;
      gDebug.win = NULL;
   }

// This global is now nulled since the window is gone *****
   if(itfc->win == win)
      itfc->win = NULL;

   if(!win->keepInFront && !mainWin) // Reassign the parent window if not a front level gui window
   {
      HWND hWnd2 = GetNextWindow(win->hWnd,GW_HWNDNEXT); 
      delete win;
      win = NULL; 
      ChangeGUIParent(hWnd2);
   }
   else
   {
	   delete win;
      win = NULL;
   }

   DestroyWindow(hwnd);  


   if(mainWin)
      prospaWin = NULL;
   else
   {
	   BringWindowToTop(prospaWin);
   }

   RestoreCurrentWindows(hwnd);
	itfc->nrRetValues = 0;
   return(OK);
}



/****************************************************************
* Modify the title of the window specified by entry 'win'. This *
* is the windows number. If win = 0 then the current window     *
* will be modified.                                             *
****************************************************************/
  
//int  SetWindowTitle(char arg[])
//{
//   short nrArgs;
//   static char title[MAX_STR];
//   static short winNr;
//   WinData *win;
//
//// Get argument (window number and title) ********************
//   
//   if((nrArgs = ArgScan(itfc,arg,2,"win title","ee","ds",&winNr,title)) < 0)
//      return(nrArgs);
//
//// Check for current window (window number = 0) **************
//
//	if(winNr == 0 && GetGUIWin())
//      winNr = GetGUIWin()->nr;
//
//// Find window class *****************************************
//      
//   win = rootWin->FindWinByNr(winNr);
//   if(!win)
//   {
//      ErrorMessage("window %d undefined",winNr);
//      return(ERR);
//   }
//
//// Set window title ******************************************	
//
//   SetWindowText(win->hWnd,title);
//   
//   return(0);
//}


/***********************************************************************
                    Redraws the current window   

  CLI command syntax: drawwin(winNr)
  Arguments: window number (INT)
  Returns: nothing
***********************************************************************/

int DrawWindow(Interface* itfc ,char args[])
{
   short winNr;
   short r;
   WinData *win;

// Get the window number
   if((r = ArgScan(itfc,args,1,"winNr","e","d",&winNr)) < 0)
      return(r);

	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}

// From this find the WinData class instance
   win = rootWin->FindWinByNr(winNr);

// If the window exists then force it to redraw itself
	if(!(win = rootWin->FindWinByNr(winNr)))
	{
	   ErrorMessage("window '%ld' not found",(long)winNr);
	   return(ERR);
	}
   else
   {
		MyInvalidateRect(win->hWnd,NULL,true);     
      MyUpdateWindow(win->hWnd);
   }
	itfc->nrRetValues = 0;
   return(OK);
}

// Return the parent window structure given the Windows handle

WinData *GetWinData(HWND hWnd)
{
   if(hWnd)
   {
      WinInfo* wi = (WinInfo*)GetWindowLong(hWnd,GWL_USERDATA);
      if(wi && wi->winType == WINDATA)
         return((WinData*)wi->data);
   }
   return(NULL);
}

ObjectData *GetObjData(HWND hWnd)
{
   if(hWnd)
   {
      WinInfo* wi = (WinInfo*)GetWindowLong(hWnd,GWL_USERDATA);
      if(wi && wi->winType == OBJDATA)
         return((ObjectData*)wi->data);
   }
   return(NULL);
}


// Return the current window number
short GetCurWinNr(Interface *itfc)
{	
	short winNr;
	if(!itfc || !itfc->win)
	{
		if(!GetGUIWin())
		{
			ErrorMessage("no current window");
			return(ERR);
		}   
		winNr = GetGUIWin()->nr;
		//	TextMessage("GuiWinNr no itfc winNr = %d\n",winNr);

	}
	else
	{
	   winNr = itfc->win->nr;
		//			TextMessage("GuiWinNr itfc winNr = %d\n",winNr);

	}
   return(winNr);
}


/*****************************************************************
  Set or get the current gui windows

  Syntax: win = curwin()  # gets current window as a class
          curwin(winClass) # sets current window by class
          curwin(winnr) # sets current window by window number

*****************************************************************/
   
int GetOrSetCurrentWindow(Interface *itfc, char args[])
{
   int nrArgs;
   CText whichPlot,cmd;
   CText title;
	CArg carg;
   int winNr;
       
   nrArgs = carg.Count(args);

   if(GetGUIWin())
      winNr = GetGUIWin()->nr;
   else
      winNr = -1;


// Return region class
   if(nrArgs == 0) 
   {
      if(winNr != -1)
      {
         itfc->retVar[1].MakeClass(WINDOW_CLASS,(void*)GetGUIWin());
         itfc->nrRetValues = 1;
         return(OK);
      }
      else
      {
         ErrorMessage("no current window defined");
         return(ERR);
      }
   }

// Set current window
   else if(nrArgs == 1)
   {
      Variable var;
      WinData *win;
  
      if((nrArgs = ArgScan(itfc,args,1,"window","e","v",&var)) < 0)
         return(nrArgs);

      if(var.GetType() == CLASS)
      {
         ClassData *cd = (ClassData*)var.GetData();

         if(CheckClassValidity(cd,true) == ERR)
            return(ERR);

         if(cd->type == WINDOW_CLASS)
         {
            win = (WinData*)cd->data;
				WinData::SetGUIWin(win);
            itfc->win = win;        // Ensure window variables from this window can be accessed
         }
         else
         {
            ErrorMessage("argument is not a gui window");
            return(ERR);
         }
      }
      else if(var.GetType() == FLOAT32)
      {
         win = rootWin->FindWinByNr(nsint(var.GetReal()));
         if(win != NULL)
         {
				WinData::SetGUIWin(win);
            itfc->win = win;        // Ensure window variables from this window can be accessed
         }
         else
         {
            ErrorMessage("argument is not a valid gui window number");
            return(ERR);
         }
         return(OK);
      }
      else
      {
         ErrorMessage("invalid data type for window argument");
         return(ERR);
      }
      return(OK);
   }
 
   itfc->nrRetValues = 0;
	return(OK);
}

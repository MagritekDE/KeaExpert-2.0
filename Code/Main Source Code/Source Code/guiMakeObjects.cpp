#include "stdafx.h"
#pragma pack(push,default)
// Must not use 1 byte packing in USB device structure
#include <Dbt.h>
#pragma pack(pop)
#include "guiMakeObjects.h"
#include <shellapi.h>
#include "bitmap.h"
#include "cArg.h"
#include "cli_events.h"
#include "defineWindows.h"
#include "dialog_utilities.h"
#include "edit_class.h"
#include "evaluate.h"
#include "evaluate_complex.h"
#include "events_edit.h"
#include "files.h"
#include "globals.h"
#include "gridcontrol.h"
#include "guiInteractiveWindowBuilder.h"
#include "guiModifyObjectParameters.h"
#include "guiObjectClass.h"
#include "guiWindowClass.h"
#include "guiWindowCLI.h"
#include "guiWindowsEvents.h"
#include "htmlviewer.h"
#include "interface.h"
#include "macro_class.h"
#include "main.h"
#include "mymath.h"
#include "plot3dClass.h"
#include "plotwindefaults.h"
#include "PlotWindow.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "string_utilities.h"
#include "variablesOther.h"
#include "winEnableClass.h"
#include <initguid.h>
#include "utilities.h"
#include "memoryLeak.h"

DEFINE_GUID(USBIODS_GUID, 0x77f49320, 0x16ef, 0x11d2, 0xad, 0x51, 0x0, 0x60, 0x97, 0xb5, 0x14, 0xdd);


#pragma warning (disable: 4311) // Ignore pointer truncation warnings
#pragma warning (disable: 4312) // Ignore Get/SetWindowLong warnings

#define WM_UPDATEUISTATE  0x0128
#define WM_QUERYUISTATE   0x0129
//#define UIS_SET  1
//#define UIS_CLEAR  2
//#define UIS_INITIALIZE  3
//#define UISF_HIDEFOCUS  1
//#define UISF_HIDEACCEL  2
//#define UISF_ACTIVE  4

ObjectData *curCLIObject;
static char objCommand[5000]; // Needs to be more flexible
bool activeEditWin = true; // Whether to activate the edited window when a new window is made

short ProcessWindowPosition(HWND hWnd,Variable *wx, 
                            Variable *wy, Variable *ww, Variable *wh,
                            short &x,short &y, short &w, short &h,
                            ObjPos *pos);
void InitToolBar(ToolBarInfo* info, TBBUTTON *buttons);
bool MakeBitMap(long width, long height, HBITMAP *bitMap, HDC hdc);
bool StrToAccelStruct(char* str, ACCEL* accel);
CText GetMenuAccelerator(CText &menuLabel);
int IsObject(Interface* itfc ,char args[]);
int MakeControl(Interface* itfc ,char arg[]);

// List of key names used as accelerators in menus 
//  and the corresponding windows code.
typedef struct
{
	char name[10];
	short code;
}
VKeys;

VKeys Vklist[] = {
	{"ESC", VK_ESCAPE},
	{"TAB", VK_TAB},
	{"RETURN", VK_RETURN},
	{"LEFT",  VK_LEFT},
	{"RIGHT", VK_RIGHT},
	{"UP",   VK_UP},
	{"DOWN", VK_DOWN},
	{"F1",   VK_F1},
	{"F2",  VK_F2},
	{"F3",  VK_F3},
	{"F4",  VK_F4},
	{"F5",  VK_F5},
	{"F6",  VK_F6},
	{"F7",  VK_F7},
	{"F8",  VK_F8},
	{"F9",  VK_F9},
	{"F10",  VK_F10},
	{"F11",  VK_F11},
	{"F12",  VK_F12},
	{"F13",  VK_F13},
	{"F14",  VK_F14},
	{"_",    VK_SEPARATOR},
	{"-",    VK_SUBTRACT},
	{".",    VK_DECIMAL},
	{"/",    VK_DIVIDE},
	{"END",    VK_END},
	{"HOME",    VK_HOME},
	{"INSERT",    VK_INSERT},
	{"DELETE",    VK_DELETE},
	{"PAUSE",    VK_PAUSE}
};


/*********************************************************************************
                                Specify which window to edit

     Syntax:  seteditwin(nr[,mode])

     nr ... window number to edit. Negative if it is to be activated.

**********************************************************************************/

int SetEditableWindow(Interface *itfc, char arg[])
{
   short r;
   short winNr;
   WinData *win;
   CText mode = "normal";

// Initialise winNr in case user uses "getargs"
   if(editGUIWindow)
      winNr = editGUIWindow->nr;
   else
      winNr = -1;

// Get the argument   
   if((r = ArgScan(itfc,arg,1,"winNr,mode","ee","dt",&winNr,&mode)) < 0)
      return(r);

   if(mode == "fast") // no redrawing of windows
   {
      if(winNr < 0) // Activate
      {
         win = rootWin->FindWinByNr(-winNr);
         editGUIWindow = NULL;
         g_objVisibility = WS_VISIBLE;
         SendMessage(win->hWnd,WM_UPDATEUISTATE,UIS_CLEAR + (UISF_HIDEFOCUS * 65536),0); // Make sure buttons get the focus
      }
      else // Edit
      {
	      if(winNr == 0)
	      {
		      if((winNr = GetCurWinNr(itfc)) == ERR)
			      return(ERR);
	      }
         win = rootWin->FindWinByNr(winNr);
         editGUIWindow = win;
		   g_objVisibility = 0;
      }
      return(OK);
   }

// Activate window
   if(winNr < 0)
   {
      win = rootWin->FindWinByNr(-winNr);
      if(win)
      {
         win->activate();
	      itfc->nrRetValues = 0;
         return(OK);
      }
      else
      {
         ErrorMessage("window %d not found",-winNr);
         return(ERR);
      }
   }

// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}

// Get the window
   win = rootWin->FindWinByNr(winNr);

   if(!win)
   {
      ErrorMessage("window %hd not found",winNr);
      return(ERR);
   }
   else
   {
      ActivateEditedGUIWin();  // Activate currently edited window
      win->makeEditable();		// Make this window editable
      win->EnableResizing(true);
   }
   SendMessage(win->hWnd,WM_UPDATEUISTATE,UIS_CLEAR + (UISF_HIDEFOCUS * 65536),0); // Make sure buttons get the focus
	itfc->nrRetValues = 0;
   return(OK);
}

/*********************************************************************************
                                Get the currently edited window

     Syntax:  nr = geteditwin()

**********************************************************************************/

int GetEditedWindow(Interface *itfc, char args[])
{
   return(SetEditableWindow(itfc,"\"getargs\""));
}

/*********************************************************************************

     Register the Magritek USB devices with the Prospa window so it will detect
     changes.

**********************************************************************************/


extern GUID usbDSPGUID, usbFX3GUID;

int RegisterUSBDevice(HWND hWnd)
{
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;

   HDEVNOTIFY hDeviceNotify;

   NotificationFilter.dbcc_classguid = usbDSPGUID;

   hDeviceNotify = RegisterDeviceNotification(
      hWnd,                       // events recipient
      &NotificationFilter,        // type of device
      DEVICE_NOTIFY_WINDOW_HANDLE // type of recipient handle
      );

   if ( NULL == hDeviceNotify ) 
   {
      int err = GetLastError();
      TextMessage("\nDSP RegisterDeviceNotification error %ld\n",err);
      return FALSE;
   }

   NotificationFilter.dbcc_classguid = usbFX3GUID;

   hDeviceNotify = RegisterDeviceNotification(
      hWnd,                       // events recipient
      &NotificationFilter,        // type of device
      DEVICE_NOTIFY_WINDOW_HANDLE // type of recipient handle
      );

   if ( NULL == hDeviceNotify ) 
   {
      int err = GetLastError();
      TextMessage("\nFX3 RegisterDeviceNotification error %ld\n",err);
      return FALSE;
   }

   return(OK);
}


/*********************************************************************************
                                Make a gui window

     Syntax:  nr = window(title, x, y, w, h)

**********************************************************************************/


int MakeWindow(Interface* itfc ,char arg[])
{
   static short x,y,w,h;
   Variable vx,vy,vw,vh;
   short winNr;
   int r;
   static CText name;
   CText resizable= "notresizable";
   WinData *win;
   ObjPos winPos;
   CText className;
   CText mainWindow = "normalWin";

   winPos.xs = 0;
   winPos.xo = 0;
   winPos.ys = 0;
   winPos.yo = 0;
   winPos.ws = 0;
   winPos.wo = 0;
   winPos.hs = 0;
   winPos.ho = 0;

   if((r = ArgScan(itfc,arg,5,"number:title,x,y,w,h,[resize],[main]","eeeeeee","tvvvvtt",&name,&vx,&vy,&vw,&vh,&resizable,&mainWindow)) < 0)
      return(r);

// Find the next available window 
   winNr = rootWin->FindNextFreeNr();

// Convert position expressions to integers
   if(prospaWin)
   {
      if(ProcessWindowPosition(prospaWin,&vx,&vy,&vw,&vh,x,y,w,h,&winPos) == ERR)
         return(ERR);
   }
   else
   {
      x = nint(vx.GetReal());
      y = nint(vy.GetReal());
      w = nint(vw.GetReal());
      h = nint(vh.GetReal());
   }

// Centre the window on the Prospa application (or main window) if position is (-1,-1) 
   if(x == -1 && y == -1)
   {
      RECT pRect;
      long width,height;
      if(prospaWin) // Child window
      {
         GetWindowRect(prospaWin,&pRect);
         width = pRect.right - pRect.left;
         height = pRect.bottom - pRect.top;
         x = (width - w)/2 + pRect.left;
         y = (height - h)/2 + pRect.top;
         // Make sure the window is visible
         CorrectWindowPositionIfInvisible(x, y, width, height);
      }
      else // Top level window
      {
         width =  GetSystemMetrics(SM_CXFULLSCREEN);
         height =  GetSystemMetrics(SM_CYFULLSCREEN);
         x = (width - w)/2;
         y = (height - h)/2;
      }
	}

// Increase window height by title bar height and frame size
   if(resizable == "notresizable")
      h = h + titleBarHeight+GetSystemMetrics(SM_CYFIXEDFRAME);
   else
      h = h + titleBarHeight+GetSystemMetrics(SM_CYSIZEFRAME);

#if(_WIN32_WINNT >= 0x0600)
   h += GetSystemMetrics(SM_CXPADDEDBORDER);
   w += 2*GetSystemMetrics(SM_CXPADDEDBORDER);
#endif

   CorrectWindowPositionIfInvisible(x, y, w, h);

   if (mainWindow == "mainWin")
      className = "MAIN_PROSPAWIN";
   else
      className = "PROSPAWIN";

// Add a window to the gui window list
   if(resizable == "resizable")
   {
      win = rootWin->AddWin(winNr,name.Str(),x,y,w,h,true,className.Str());
      win->resizeable = true;
   }
   else if(resizable == "notresizable")
   {
      win = rootWin->AddWin(winNr,name.Str(),x,y,w,h,false,className.Str());
      win->resizeable = false;
   }
   else
   {
      ErrorMessage("invalid resizable parameter");
      return(ERR);
   }

// Add a reference to the HWND so we don't have to search for the WinData structure
   WinInfo* wi = new WinInfo;
   wi->winType = WINDATA;
   wi->data = (char*)win;

   SetWindowLong(win->hWnd,GWL_USERDATA,(LONG)wi);

   if(!prospaWin)
   {
      prospaWin = win->hWnd;
      RegisterUSBDevice(win->hWnd);
      win->isMainWindow = true;
      win->keepInFront = false;
   }

// Make sure the current GUI window is remembered
// in case this window is a dialog
   win->oldGUI = GetGUIWin();

// Created window will become new current GUI window
	WinData::SetGUIWin(win);

// Add the macroname and path to the window
   if(itfc->macro->macroName == "")
      strncpy_s(GetGUIWin()->macroName,MAX_STR,"current_text",_TRUNCATE);
   else
      strncpy_s(GetGUIWin()->macroName,MAX_STR,itfc->macro->macroName.Str(),_TRUNCATE);
   strncpy_s(GetGUIWin()->macroPath,MAX_PATH,itfc->macro->macroPath.Str(),_TRUNCATE);

   if(activeEditWin)
      ActivateEditedGUIWin();  // Activate currently edited window
   GetGUIWin()->makeEditable();	// Make this window editable


// Make sure the current macro is labelled with this window
   itfc->win = GetGUIWin();
   itfc->win->oldParent = itfc->win;

// Load all procedures from the window macro into the window
   //if(itfc->cacheProc)
   //{
   //   itfc->win->AddProcedures(itfc->win->macroPath,itfc->win->macroName);
   //   itfc->win->cacheProc = true;
   //}

// Update the window size variables
   RECT rect;
   GetClientRect(win->hWnd,&rect);

// If we are debugging then set a window flag so that all window
// procedures know this
   if(itfc->debugging)
      win->debugging = true;


 //  hAccelUser = NULL;

	win->blankStatusBar = CreateWindowEx(NULL, STATUSCLASSNAME,"",
	                         WS_CHILD,
	                         0,0,0,0,
	                         win->hWnd,NULL,prospaInstance,NULL);
   int pos[1];
   pos[0] = -1;
   SendMessage(win->blankStatusBar,SB_SETPARTS, (WPARAM)1, (LPARAM) pos);
   SendMessage(win->blankStatusBar,WM_SETFONT,(WPARAM)controlFont ,false);
      
	win->blankToolBar = CreateToolbarEx((HWND)win->hWnd,			
							      WS_CHILD  | WS_BORDER,
                           (DWORD)0,
							      (UINT)0,
							      (HINSTANCE)NULL,
							      (UINT)NULL,
                           NULL,
							      0,
							      16,16,16,16,
                           sizeof(TBBUTTON));


   win->xSzScale  = winPos.xs;
   win->xSzOffset = winPos.xo;
   win->ySzScale  = winPos.ys;
   win->ySzOffset = winPos.yo;
   win->wSzScale  = winPos.ws;
   win->wSzOffset = winPos.wo;
   win->hSzScale  = winPos.hs;
   win->hSzOffset = winPos.ho;

// Make sure the window will be informed of any USB device changes
   if(win->hWnd == prospaWin)
   {
       win->devIF = (DEV_BROADCAST_DEVICEINTERFACE *)malloc( sizeof(DEV_BROADCAST_DEVICEINTERFACE) );
       memset( win->devIF, 0, sizeof(DEV_BROADCAST_DEVICEINTERFACE) );
       win->devIF->dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
       win->devIF->dbcc_devicetype = DBT_DEVTYP_PORT;
       win->devIF->dbcc_classguid = USBIODS_GUID; // Identify our USB device
       win->_hNotifyDevNode = RegisterDeviceNotification(win->hWnd, win->devIF, DEVICE_NOTIFY_WINDOW_HANDLE);
      // if(!win->_hNotifyDevNode)
      //    TextMessage("DSP USB device notification failure! %ld\n",GetLastError());
   }

// Set window icon to match prospaWin
	HICON hIcon = 	(HICON)SendMessage(prospaWin, WM_GETICON, ICON_SMALL2, (LPARAM)0);
   SendMessage(win->hWnd, WM_SETICON, ICON_SMALL2, (LPARAM)hIcon);
	hIcon = 	(HICON)SendMessage(prospaWin, WM_GETICON, ICON_SMALL, (LPARAM)0);
   SendMessage(win->hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	hIcon = 	(HICON)SendMessage(prospaWin, WM_GETICON, ICON_BIG, (LPARAM)0);
	SendMessage(win->hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

// Make a variable link to winvar member
   win->winVar.MakeStruct();

// Return the window number and window class
   itfc->nrRetValues = 2;
   itfc->retVar[1].MakeAndSetFloat((float)winNr);
   itfc->retVar[2].MakeClass(WINDOW_CLASS,(void*)win);
   itfc->retVar[2].SetScope(WINDOW);

   return(OK);
} 

/************************************************************************
    Make a control - all parameters can be passed here
*************************************************************************/

int MakeControl(Interface* itfc ,char arg[])
{
   CText type;
   short nrArgs;
   CArg carg;
   CText parameter;
   CText value;

   TextMessage("Not operational yet\n");
   return(OK);

 /*  nrArgs = carg.Count(arg);
   if(nrArgs < 1 || nrArgs%2 == 0)
   {
      ErrorMessage("Invalid number of arguments - should be odd");
      return(ERR);
   }

   if((nrArgs = ArgScan(itfc,arg,1,"type","e","t",&type)) < 0)
      return(nrArgs);

   if(type != "textbox")
   {
      ErrorMessage("Invalid or unknown type '%s'",type.Str());
      return(ERR);
   }

   CText xExp,yExp,wExp,hExp;

   for(int i = 1; i < nrArgs; i++)
   {
      parameter = carg.Extract(i+1);
      value = carg.Extract(i+2);

      if(parameter == "x")
         xExp = parameter;
      else if(parameter == "y")
         yExp = parameter;
      else if(parameter == "w")
         wExp = parameter;
      else if(parameter == "h")
         hExp = parameter;
   }



   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,0,x,y,w,h,&pos) == ERR)
      return(ERR);

      pos.hs = 0;
      pos.ho = h;

   ObjectData *obj = new ObjectData(editGUIWindow,TEXTBOX,nr,x,y,w,h,"",objCommand,(char*)0,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);*/

 //  return(OK);
}

/************************************************************************
    Evaluate object dimensions and position and return as fixed numbers
*************************************************************************/

short ProcessWindowPosition(HWND hWnd,Variable *wx, 
                            Variable *wy, Variable *ww, Variable *wh,
                            short &x,short &y, short &w, short &h,
                            ObjPos *pos)
{
// Work out client rectangle size
   RECT rect;
   GetWindowRect(hWnd,&rect);

   short parX = (short)(rect.left)+resizableWinBorderSize;
   short parY = (short)(rect.top)+titleBarNMenuHeight+resizableWinBorderSize;
   short parW = (short)(rect.right-rect.left)-2*resizableWinBorderSize;
   short parH = (short)(rect.bottom-rect.top)-titleBarNMenuHeight-2*resizableWinBorderSize;

   pos->region = false;

// Process X position
	if (wx)
	{
		if (ERR == ProcessValuePosition(wx, &pos->xs, &pos->xo, &pos->region,"x"))
		{
			return ERR;
		}
      if(pos->xo == -1)
         x = -1;
      else
         x = parW*pos->xs + pos->xo + parX;
   }

// Process Y position 
   if(wy)
   {
		if (ERR == ProcessValuePosition(wy, &pos->ys, &pos->yo, &pos->region,"y"))
		{
			return ERR;
		}
      if(pos->yo == -1)
         y = -1;
      else
         y = parH*pos->ys + pos->yo + parY;
   }

// Process width 
   if(ww)
   {
		if (ERR == ProcessValuePosition(ww, &pos->ws, &pos->wo, &pos->region, "x"))
		{
			return ERR;
		}
      w = parW*pos->ws + pos->wo;
   }


// Process height 
   if(wh)
   {
		if (ERR == ProcessValuePosition(wh, &pos->hs, &pos->ho, &pos->region, "y"))
		{
			return ERR;
		}
      h = parH*pos->hs + pos->ho;
   }

   return(OK);
}

int MakeToolBar(Interface* itfc ,char arg[])
{
   short r,nr;
   CArg carg;
   long y=0,w=1,h=1;
   long i,j;
   CText fileName;
   HBITMAP hBitmap = NULL,hSubBitmap = NULL;
   BITMAP subBM;

   if((r = ArgScan(itfc,arg,2,"nr, bitmap, {label, command}","ee","dt",&nr, &fileName)) < 0)
      return(r);

   r = carg.Count(arg);

   objCommand[0] = '\0';

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }

   if(r%2 != 0)
   {
      ErrorMessage("invalid number of arguments");
      return(ERR);
   }

   RemoveExtension(fileName.Str());
   CText fileNameNoExt = fileName;

   long nrItems = (r-2)/2;
   CText  dir = itfc->macro->macroPath;
   CText  macroName = itfc->macro->macroName;
   RemoveExtension(macroName.Str());
   long len = dir.Size();
   if(dir[len-1] != '\\')
      dir = dir + "\\" + macroName;
   else
      dir = dir + macroName;

   if(IsDirectory(dir.Str()))
   {
      SetCurrentDirectory(dir.Str());
      fileName = fileName + ".bmp";

      if(IsFile(fileName.Str()))
      {
          hBitmap = (HBITMAP)LoadImage(NULL,fileName.Str(),IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_LOADMAP3DCOLORS);
      }
      else if(IsDirectory(fileNameNoExt.Str()))
      {
         SetCurrentDirectory(fileNameNoExt.Str());
         HDC hdc = GetDC(editGUIWindow->hWnd);
         MakeBitMap(16*nrItems,15,&hBitmap,hdc);
         ReleaseDC(editGUIWindow->hWnd,hdc);
         COLORREF *rgb = new COLORREF[16*nrItems*15];

         for(int k = 0;;k++)
         {
            fileName.Format("button%hd.bmp",k+1);
            if(IsFile(fileName.Str()))
               hSubBitmap = (HBITMAP)LoadImage(NULL,fileName.Str(),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
            else
               break;

            GetObject(hSubBitmap,sizeof(BITMAP),&subBM);
            long w = subBM.bmWidth;
            long h = subBM.bmHeight;
            if(w != 16 || h != 15)
            {
               ErrorMessage("invalid bitmap size for %s",fileName.Str());
               DeleteObject(hBitmap);
               delete [] rgb;
               return(ERR);
            }
            COLORREF *rgb2 = new COLORREF[w*h];
            GetBitmapBits(hSubBitmap,w*h*sizeof(COLORREF),(LPVOID)rgb2);

            long tw = 16*nrItems;
            for(long y = 0; y < h; y++)
               for(long x = 0; x < w; x++)
                  rgb[k*16 + x + y*tw] = rgb2[x+y*w];

            delete [] rgb2;
         }
         SetBitmapBits(hBitmap,16*nrItems*15*sizeof(COLORREF),(LPVOID)rgb);
         delete [] rgb;
         SetCurrentDirectory(".."); // Restore directory
      }
      else
         hBitmap = NULL;

      SetCurrentDirectory(".."); // Restore directory
   }
   else
   {
      ErrorMessage("toolbar folder '%s' not found",dir.Str());
      return(ERR);
   }

   ToolBarInfo* info = new ToolBarInfo;

   info->nrItems = nrItems;
   info->item = new ToolBarItem[info->nrItems];
   info->name = fileNameNoExt.Str();
   info->bitmap = hBitmap;

// Extract each toolbar item
   for(i = 0, j = 3; i < info->nrItems; i++, j+=2)
   {
      info->item[i].label = carg.Extract(j);
      info->item[i].cmd = carg.Extract(j+1);
      info->item[i].label.RemoveQuotes();
   }

   TBBUTTON* tbButtons = new TBBUTTON[info->nrItems];

	InitToolBar(info,tbButtons);

   ObjectData *obj = new ObjectData(editGUIWindow,TOOLBAR,nr,info->nrItems,y,w,h,(char*)hBitmap, objCommand,(char*)tbButtons,g_objVisibility,itfc->lineNr);
   obj->data = (char*)info;

	editGUIWindow->widgets.add(obj);
   editGUIWindow->toolbar = obj;

   RECT rect;
   GetClientRect(obj->hWnd,&rect);
  // obj->toolbarHeight = rect.bottom;

   delete [] tbButtons;

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(OK);
}


int MakeToolBarWithKeys(Interface* itfc ,char arg[])
{
   short r,nr;
   CArg carg;
   long y=0,w=1,h=1;
   long i,j;
   CText fileName;
   HBITMAP hBitmap = NULL,hSubBitmap = NULL;
   BITMAP subBM;

   if((r = ArgScan(itfc,arg,2,"nr, bitmap, {label, key, command}","ee","dt",&nr, &fileName)) < 0)
      return(r);

   r = carg.Count(arg);

   objCommand[0] = '\0';

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }

   if((r-2)%3 != 0)
   {
      ErrorMessage("invalid number of arguments");
      return(ERR);
   }

   RemoveExtension(fileName.Str());
   CText fileNameNoExt = fileName;

   long nrItems = (r-2)/3;
   CText  dir = itfc->macro->macroPath;
   CText  macroName = itfc->macro->macroName;
   RemoveExtension(macroName.Str());
   long len = dir.Size();
   if(dir[len-1] != '\\')
      dir = dir + "\\" + macroName;
   else
      dir = dir + macroName;

   if(IsDirectory(dir.Str()))
   {
      SetCurrentDirectory(dir.Str());
      fileName = fileName + ".bmp";

      if(IsFile(fileName.Str()))
      {
          hBitmap = (HBITMAP)LoadImage(NULL,fileName.Str(),IMAGE_BITMAP,0,0,LR_LOADFROMFILE|LR_LOADMAP3DCOLORS);
      }
      else if(IsDirectory(fileNameNoExt.Str()))
      {
         SetCurrentDirectory(fileNameNoExt.Str());
         HDC hdc = GetDC(editGUIWindow->hWnd);
         MakeBitMap(16*nrItems,15,&hBitmap,hdc);
         ReleaseDC(editGUIWindow->hWnd,hdc);
         COLORREF *rgb = new COLORREF[16*nrItems*15];

         for(int k = 0;;k++)
         {
            fileName.Format("button%hd.bmp",k+1);
            if(IsFile(fileName.Str()))
               hSubBitmap = (HBITMAP)LoadImage(NULL,fileName.Str(),IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
            else
               break;

            GetObject(hSubBitmap,sizeof(BITMAP),&subBM);
            long w = subBM.bmWidth;
            long h = subBM.bmHeight;
            if(w != 16 || h != 15)
            {
               ErrorMessage("invalid bitmap size for %s",fileName.Str());
               DeleteObject(hBitmap);
               delete [] rgb;
               return(ERR);
            }
            COLORREF *rgb2 = new COLORREF[w*h];
            GetBitmapBits(hSubBitmap,w*h*sizeof(COLORREF),(LPVOID)rgb2);

            long tw = 16*nrItems;
            for(long y = 0; y < h; y++)
               for(long x = 0; x < w; x++)
                  rgb[k*16 + x + y*tw] = rgb2[x+y*w];

            delete [] rgb2;
         }
         SetBitmapBits(hBitmap,16*nrItems*15*sizeof(COLORREF),(LPVOID)rgb);
         delete [] rgb;
         SetCurrentDirectory(".."); // Restore directory
      }
      else
         hBitmap = NULL;

      SetCurrentDirectory(".."); // Restore directory
   }
   else
   {
      ErrorMessage("toolbar folder '%s' not found",dir.Str());
      return(ERR);
   }

   ToolBarInfo* info = new ToolBarInfo;

   info->nrItems = nrItems;
   info->item = new ToolBarItem[info->nrItems];
   info->name = fileNameNoExt.Str();
   info->bitmap = hBitmap;

// Extract each toolbar item
   for(i = 0, j = 3; i < info->nrItems; i++, j+=3)
   {
      info->item[i].label = carg.Extract(j);
      info->item[i].key = carg.Extract(j+1);
      info->item[i].cmd = carg.Extract(j+2);
      info->item[i].label.RemoveQuotes();
      info->item[i].key.RemoveQuotes();
   }

   TBBUTTON* tbButtons = new TBBUTTON[info->nrItems];

	InitToolBar(info,tbButtons);

   ObjectData *obj = new ObjectData(editGUIWindow,TOOLBAR,nr,info->nrItems,y,w,h,(char*)hBitmap, objCommand,(char*)tbButtons,g_objVisibility,itfc->lineNr);
   obj->data = (char*)info;

	editGUIWindow->widgets.add(obj);
   editGUIWindow->toolbar = obj;

   RECT rect;
   GetClientRect(obj->hWnd,&rect);
  // obj->toolbarHeight = rect.bottom;

   delete [] tbButtons;


   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(OK);
}

bool  MakeBitMap(long width, long height, HBITMAP *bitMap, HDC hdc)
{
   BITMAPINFOHEADER  bmih; // Header info for bitmap
   BYTE *pbm;              // Pointer to color values in bitmap


// Initialize bitmap header **************************************  
   bmih.biSize          = sizeof(BITMAPINFOHEADER);
   bmih.biWidth         = width;
   bmih.biHeight        = height;
   bmih.biPlanes        = 1;
   bmih.biBitCount      = 32;
   bmih.biCompression   = BI_RGB;
   bmih.biSizeImage     = 0;
   bmih.biXPelsPerMeter = 0;
   bmih.biYPelsPerMeter = 0;
   bmih.biClrUsed       = 0;
   bmih.biClrImportant  = 0;
                
// Create bit map  ***********************************************
   *bitMap = CreateDIBSection(hdc, (BITMAPINFO*)&bmih, DIB_RGB_COLORS, (void**)&pbm, NULL, 0);
   if(!(*bitMap))
      return(false);
   
   return(true);
}

void InitToolBar(ToolBarInfo* info, TBBUTTON *buttons)
{
  
   for(short i = 0; i < info->nrItems; i++)
   {
      if(info->item[i].label == "Separator")
      {
         buttons[i].iBitmap = -1;
         buttons[i].fsState = TBSTATE_ENABLED;
         buttons[i].fsStyle  = TBSTYLE_SEP;
         buttons[i].iString = 0;
         buttons[i].idCommand = 0;
      }
      else
      {
         buttons[i].iBitmap = i;
         buttons[i].fsState = TBSTATE_ENABLED;
         buttons[i].fsStyle = TBSTYLE_BUTTON;
         buttons[i].dwData  = 0L;
         buttons[i].iString = 0;
         buttons[i].idCommand = i;
      }
   }  
}

int MakeTextEditor(Interface* itfc ,char arg[])
{
   short nr,x,y,w,h;
   short r;
   Variable wx,wy,ww,wh;
   Variable result;
   CText xStr,yStr,wStr,hStr;
   ObjPos pos;

   objCommand[0] = '\0';

   if((r = ArgScan(itfc,arg,5,"nr,x,y,w,h","eeeee","dvvvv",&nr,&wx,&wy,&ww,&wh)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }

   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

// Make the plot window and add to currently edited window

   ObjectData *obj = new ObjectData(editGUIWindow,TEXTEDITOR,nr,x,y,w,h,"","",(char*)0,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);
   SaveObjectPosition(obj,&pos);  

   EditParent *ep = new EditParent;
   ep->parent = obj;
   MakeEditWindows(false, ep, 1, 1, -1);
   curEditor->debug = false;
   obj->hWnd = curEditor->edWin;

    SendMessage(obj->hWnd,EM_SETMARGINS ,EC_LEFTMARGIN	,(LPARAM) MAKELONG(5, 0));

  // MoveWindow(obj->hWnd,x,y,w,h,false);
   DragAcceptFiles(obj->hWnd,true);

// Attach to object data
   obj->data = (char*)ep;

// Attach object data to HWND handle TODO - embed in all edit windows and remember to delete!
   WinInfo* wi = new WinInfo;
   wi->winType = OBJDATA;
   wi->data = (char*)obj;
   SetWindowLong(obj->hWnd,GWL_USERDATA,(LONG)wi);

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

  // SetFocus(obj->hWnd);
   return(OK);
}

//
//int MakeDebugStrip(Interface* itfc ,char arg[])
//{
//   static short nr,x,y,w,h;
//   ObjectData *obj;
//   Variable colorVar;
//   short r;
//   static char name[50];
//   Variable wx,wy,ww,wh;
//   ObjPos pos;
//
//   objCommand[0] = '\0';
//
//   if((r = ArgScan(itfc,arg,5,"nr,x,y,w,h","eeeee","dvvvv",&nr,&wx,&wy,&ww,&wh)) < 0)
//      return(r);
//
//   if(!editGUIWindow)
//   {
//      ErrorMessage("No window being edited");
//      return(ERR);
//   }
// 
//   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
//      return(ERR);
//
//   obj = editGUIWindow->objList.Add(editGUIWindow,DEBUGSTRIP,nr,x,y,w,h,name,objCommand,(char*)NULL,g_objVisibility,itfc->lineNr);
//   
//// Make a bit map to associate with this object
//   HDC hdc = GetDC(obj->hWnd);
//   HBITMAP hBitmap;
//   GenerateBitMap(w,h, &hBitmap, hdc);
//
//   HDC hdcMem = CreateCompatibleDC(hdc);
//   SelectObject(hdcMem,hBitmap);
//   RECT rct;
//   SetRect(&rct,0,0,w,h);
//   HBRUSH bkBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
//   FillRect(hdcMem,&rct,bkBrush);
//
//   HBRUSH symbolBrush = CreateSolidBrush(RGB(255,0,0));
//	HPEN symbolPen   = CreatePen(PS_SOLID,0,RGB(0,0,0));
//   SelectObject(hdcMem,symbolBrush);
//   SelectObject(hdcMem,symbolPen);
//   short xp = 10;
//   short yp = 10;
//   short szX = 5;
//   short szY = 5;
//   for(int n = 0; n < 17; n++)
//   {
//      yp = n*16 + 10;
//      Ellipse(hdcMem,xp-szX,yp-szY,xp+szX+1,yp+szY+1);
//      Arc(hdcMem,xp-szX,yp-szY,xp+szX+1,yp+szY+1,xp+1,yp+szY+1,xp+1,yp+szY+1);
//   }
//   DeleteDC(hdcMem);
//   DeleteObject(symbolPen);
//   DeleteObject(symbolBrush);
//   DeleteObject(bkBrush);
//
//   ReleaseDC(obj->hWnd,hdc);
//
//   DebugStripInfo *info = new DebugStripInfo;
//   info->editor = -1;
//   info->hBitmap = hBitmap;
//
//   obj->data = (char*)info;
//
//   SaveObjectPosition(obj,&pos);  
//
//   return(0);
//}


int MakeCLI(Interface* itfc ,char args[])
{
   short nr,x,y,w,h; 
   
   short r;
   Variable wx,wy,ww,wh;
   Variable result;
   CText xStr,yStr,wStr,hStr;
   ObjPos pos;
   HDC hdc;

   objCommand[0] = '\0';

   if((r = ArgScan(itfc,args,5,"nr,x,y,w,h","eeeee","dvvvv",&nr,&wx,&wy,&ww,&wh)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }

   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

// Make the plot window and add to currently edited window
   ObjectData *obj = new ObjectData(editGUIWindow,CLIWINDOW,nr,x,y,w,h,"","",(char*)0,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);
	hdc = GetDC(editGUIWindow->hWnd);
   SendMessage(obj->hWnd,EM_EXLIMITTEXT,(WPARAM)0,(LPARAM)MAX_TEXT);		                      		                      		                      
   SendMessage(obj->hWnd,WM_SETFONT,(WPARAM)cliFont,MAKELPARAM(false, 0));

   cliWin = editGUIWindow->hWnd;
   cliEditWin = obj->hWnd;
   ReleaseDC(editGUIWindow->hWnd,hdc);     
	DragAcceptFiles(obj->hWnd, true);

// Save a copy of the object position expression for later evaluation
// when the window is resized.
   SaveObjectPosition(obj,&pos);  

   SetFocus(obj->hWnd);

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(OK);
}

int MakePlotWindow(Interface* itfc ,char args[])
{
   short nr,x,y,w,h;
   short r;
   PlotWindow1D *pp;
   Variable wx,wy,ww,wh;
   Variable result;
   CText xStr,yStr,wStr,hStr;
   ObjPos pos;

   objCommand[0] = '\0';

   if((r = ArgScan(itfc,args,5,"nr,x,y,w,h","eeeee","dvvvv",&nr,&wx,&wy,&ww,&wh)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }

   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

// Make the plot window and add to currently edited window
   ObjectData *obj = new ObjectData(editGUIWindow,PLOTWINDOW,nr,x,y,w,h,"","",(char*)0,g_objVisibility,itfc->lineNr);
   editGUIWindow->widgets.add(obj);

// Save a copy of the object position expression for later evaluation
// when the window is resized.
   SaveObjectPosition(obj,&pos);  

// Initialise plotParent information
   pp = new PlotWindow1D(SHOW_DATA, obj->hWnd,pwd, obj, w, h);

  // SetTimer(obj->hWnd, 1000, 20, (TIMERPROC) NULL);     

// Attach to object data
   obj->data = (char*)pp;
	int fileNum = rootWin->GetNextFileNumber(obj,CText("untitled"));
	pp->setFileNumber(fileNum);

	if(Plot1D::curPlot())
		MyInvalidateRect(Plot1D::curPlot()->win,NULL,false); // Remove old indent

	pp->makeCurrentPlot();
	pp->makeCurrentDimensionalPlot();
   Plot1D::curPlot()->setDefaultParameters();
   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(0);
}

// Make an image (2D) window

int MakeImageWindow(Interface* itfc ,char args[])
{
   short nr,x,y,w,h;
   short r;
   PlotWindow2D *pp;
   Variable wx,wy,ww,wh;
   Variable result;
   CText xStr,yStr,wStr,hStr;
   ObjPos pos;

   objCommand[0] = '\0';

   if((r = ArgScan(itfc,args,5,"nr,x,y,w,h","eeeee","dvvvv",&nr,&wx,&wy,&ww,&wh)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }

   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

// Make the plot window and add to currently edited window

   ObjectData *obj = new ObjectData(editGUIWindow,IMAGEWINDOW,nr,x,y,w,h,"","",(char*)0,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);
   
// Save a copy of the object position expression for later evaluation
// when the window is resized.
   SaveObjectPosition(obj,&pos);  

// Initialise plotParent information
   pp = new PlotWindow2D(SHOW_DATA, obj->hWnd, pwd, obj, w, h);
	
	if(Plot2D::curPlot())
		MyInvalidateRect(Plot2D::curPlot()->win,NULL,false); // Remove old indent
	
	pp->makeCurrentPlot();
	pp->makeCurrentDimensionalPlot();
   Plot2D::curPlot()->setDefaultParameters();

// Attach to object data
   obj->data = (char*)pp;

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(0);
}

// Make a 3D window

int Make3DWindow(Interface* itfc ,char args[])
{
   short nr,x,y,w,h;
   short r;
   Variable wx,wy,ww,wh;
   Variable result;
   CText xStr,yStr,wStr,hStr;
   ObjPos pos;

   objCommand[0] = '\0';

   if((r = ArgScan(itfc,args,5,"nr,x,y,w,h","eeeee","dvvvv",&nr,&wx,&wy,&ww,&wh)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }

   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

// Make the plot window and add to currently edited window
   ObjectData *obj = new ObjectData(editGUIWindow,OPENGLWINDOW,nr,x,y,w,h,"","",(char*)0,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);
   
   SaveObjectPosition(obj,&pos);  

   cur3DWin = obj->hWnd;

   CPlot3D* plot = new CPlot3D;
   plot->parent = obj;
   obj->data = (char*)plot;
   plot3d = plot;
   DragAcceptFiles(obj->hWnd,true);

  // Initialize3DPlot(obj->hWnd);

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;
   return(0);
}

// Make a track bar or slider control
int MakeSlider(Interface* itfc ,char args[])
{
   static short nr,x,y,w,h;
   Variable wx,wy,ww,wh;
   ObjPos pos;

   short r;
   static char orient[50];
   
   objCommand[0] = '\0';

   if((r = ArgScan(itfc,args,6,"nr,x,y,w,h,orient,[objCommand]","eeeeeec","dvvvvss",&nr,&wx,&wy,&ww,&wh,orient,objCommand)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }
     
   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

   ObjectData *obj = new ObjectData(editGUIWindow,SLIDER,nr,x,y,w,h,orient,objCommand,(char*)0,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);
   SaveObjectPosition(obj,&pos);  

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(OK);
}

int MakeProgressBar(Interface* itfc ,char args[])
{
   static short nr,x,y,w,h;
   Variable wx,wy,ww,wh;
   ObjPos pos;
   short r;
   static CText direction;
   
   objCommand[0] = '\0';

   if((r = ArgScan(itfc,args,6,"nr,x,y,w,h,dir","eeeeee","dvvvvt",&nr,&wx,&wy,&ww,&wh,&direction)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }
      
   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

   ObjectData *obj = new ObjectData(editGUIWindow,PROGRESSBAR,nr,x,y,w,h,direction.Str(),objCommand,(char*)0,g_objVisibility,itfc->lineNr);
   editGUIWindow->widgets.add(obj);   	

   SaveObjectPosition(obj,&pos);  

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(0);
}


int MakeStatusBox(Interface* itfc ,char args[])
{
   CArg carg;
   CText posStr;
   short r;
   static short nr;
   StatusBoxInfo *info;
   float scale,off;
   bool region;

   objCommand[0] = '\0';

   if((r = ArgScan(itfc,args,1,"nr","e","d",&nr)) < 0)
      return(r);

   r = carg.Count(args);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }
    
   ObjectData *obj = new ObjectData(editGUIWindow,STATUSBOX,nr,0,0,0,0,NULL,objCommand,(char*)0,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);
   
   info = new StatusBoxInfo;

   info->parts = r-1;
   info->posArray = NULL;
   info->subWindow = NULL;
   info->syntax = "";

   RECT rect;
   GetClientRect(editGUIWindow->hWnd,&rect);
   long ww = rect.right-rect.left;

// These data structures will store the status bar division positions
   if(r > 1)
   {
      int* pos = new int[r-1];
      info->posArray = new char*[r-1];

      for(int i = 2; i <= r; i++)
      {
         posStr = carg.Extract(i);
         posStr.RemoveQuotes();
         int sz = posStr.Size();
         info->posArray[i-2] = new char[sz+1];
         strcpy(info->posArray[i-2],posStr.Str());
         if(GetObjectDimensions(posStr.Str(), &scale, &off, &region) == ERR)
         {
				editGUIWindow->widgets.remove(obj);
				editGUIWindow->widgets.sort();
				delete[] pos;
            delete obj;
            ErrorMessage("invalid x position");
            return(ERR);
         }
         pos[i-2] = nint(ww*scale + off);
      }

      SendMessage(obj->hWnd,SB_SETPARTS, (WPARAM)r-1, (LPARAM) pos);
      delete [] pos; //??
   }

   obj->data = (char*)info;

   editGUIWindow->statusbox = obj;
   editGUIWindow->defaultStatusbox = obj;

   GetClientRect(obj->hWnd,&rect);
   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(OK);
}


// Make a menu

int MakeMenu(Interface* itfc ,char args[])
{
   short nr,x=0,y=0,w=0,h=0;
   short r;
   CText name,accel;
   static char objCommand[100];
   MenuInfo *info;
   CArg carg;
   CText menuLabel,menuCmd;
   bool err = false;
   
   if((r = ArgScan(itfc,args,2,"nr, name, {item, command}","ee","dt",&nr,&name)) < 0)
      return(r);

   r = carg.Count(args);

   if((r-2)%2 != 0)
   {
      ErrorMessage("invalid number of arguments");
      return(ERR);
   }

   objCommand[0] = '\0';

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }

   ObjectData *obj = new ObjectData(editGUIWindow,MENU,nr,x,y,w,h,name.Str(),objCommand,(char*)0,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);
   
   info = new MenuInfo;

   info->menu = CreatePopupMenu();
   info->nrItems = (r-2)/2;
   info->name = new char[name.Size()+1];
   info->cmd = new char*[info->nrItems];
   info->label = new char*[info->nrItems];
   info->key = new char*[info->nrItems];
   strcpy(info->name,name.Str());

   info->accel = new ACCEL[info->nrItems];

// Extact each menu item
   for(int j = 0, i = 3; i <= r; i+=2, j++)
   {
      menuLabel = carg.Extract(i); // Item label
      menuCmd = carg.Extract(i+1); // Item command
      info->label[j] = new char[menuLabel.Size()+1];
      strcpy(info->label[j],menuLabel.Str());
      info->cmd[j] = new char[menuCmd.Size()+1];
      strcpy(info->cmd[j],menuCmd.Str());
      info->key[j] = NULL;
      menuLabel.RemoveQuotes();
      accel = GetMenuAccelerator(menuLabel);
      StrToAccelStruct(accel.Str(),&info->accel[j]);
      info->accel[j].cmd = j+obj->seqMenuNr*100 + MENU_BASE;
      ReplaceSpecialCharacters(menuLabel.Str(),"\\t","\t",-1);
      if(menuLabel == "Separator")
      {
         AppendMenu(info->menu, MF_SEPARATOR	, NULL, NULL);  
      }
      else if(menuLabel == "Pull_right")
      {
         Variable result;
         EvaluateComplexExpression(itfc,menuCmd.Str(),&result);
         if(result.GetType() == FLOAT32)
         {
            short menuNr = result.GetReal();
            sprintf(info->cmd[j],"%ld",nint(menuNr));
            ObjectData *popup = editGUIWindow->FindObjectByNr(menuNr);
            if(popup)
            {
               MenuInfo *popupInfo = (MenuInfo*)popup->data;
               AppendMenu(info->menu, MF_POPUP | MF_STRING, (UINT)popupInfo->menu, popupInfo->name);
            }
            else
            {
               ErrorMessage("can't find pull right menu %hd",menuNr);
               err = true;
            }
         }
      }
      else
         AppendMenu(info->menu, MF_STRING, info->accel[j].cmd, menuLabel.Str());   

   }

   if(err)
   {
		editGUIWindow->widgets.remove(obj);
		editGUIWindow->widgets.sort();
      delete obj;
      obj = NULL;
      return(ERR);
   }

   obj->data = (char*)info;

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(OK);
}

// Make a menu and include a key string so the menu can be modified externally

int MakeMenuWithKeys(Interface* itfc ,char args[])
{
   short nr,x=0,y=0,w=0,h=0;
   short r;
   CText name,accel;
   static char objCommand[100];
   MenuInfo *info;
   CArg carg;
   CText menuLabel,menuCmd,menuKey;
   bool err = false;
   
   if((r = ArgScan(itfc,args,2,"nr, name, {item, key, command}","ee","dt",&nr,&name)) < 0)
      return(r);

   r = carg.Count(args);

   if((r-2)%3 != 0)
   {
      ErrorMessage("invalid number of arguments");
      return(ERR);
   }

   objCommand[0] = '\0';

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }

   ObjectData *obj = new ObjectData(editGUIWindow,MENU,nr,x,y,w,h,name.Str(),objCommand,(char*)0,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);
   
   info = new MenuInfo;

   info->menu = CreatePopupMenu();
   info->nrItems = (r-2)/3;
   info->name = new char[name.Size()+1];
   info->cmd = new char*[info->nrItems];
   info->label = new char*[info->nrItems];
   info->key = new char*[info->nrItems];
   strcpy(info->name,name.Str());

   info->accel = new ACCEL[info->nrItems];

// Extact each menu item
   for(int j = 0, i = 3; i <= r; i+=3, j++)
   {
      menuLabel = carg.Extract(i); // Item label
      menuKey = carg.Extract(i+1); // Item key
      menuCmd = carg.Extract(i+2); // Item command
      info->label[j] = new char[menuLabel.Size()+1];
      strcpy(info->label[j],menuLabel.Str());
      info->cmd[j] = new char[menuCmd.Size()+1];
      strcpy(info->cmd[j],menuCmd.Str());
      menuKey.RemoveQuotes();
      info->key[j] = new char[menuKey.Size()+1];
      strcpy(info->key[j],menuKey.Str());
      menuLabel.RemoveQuotes();
      accel = GetMenuAccelerator(menuLabel);
      StrToAccelStruct(accel.Str(),&info->accel[j]);
      info->accel[j].cmd = j+obj->seqMenuNr*100 + MENU_BASE;
      ReplaceSpecialCharacters(menuLabel.Str(),"\\t","\t",-1);
      if(menuLabel == "Separator")
      {
         AppendMenu(info->menu, MF_SEPARATOR	, NULL, NULL);  
      }
      else if(menuLabel == "Pull_right")
      {
         Variable result;
         EvaluateComplexExpression(itfc,menuCmd.Str(),&result);
         if(result.GetType() == FLOAT32)
         {
            short menuNr = result.GetReal();
            sprintf(info->cmd[j],"%ld",nint(menuNr));
            ObjectData *popup = editGUIWindow->FindObjectByNr(menuNr);
            if(popup)
            {
               MenuInfo *popupInfo = (MenuInfo*)popup->data;
               AppendMenu(info->menu, MF_POPUP | MF_STRING, (UINT)popupInfo->menu, popupInfo->name);
            }
            else
            {
               ErrorMessage("can't find pull right menu %hd",menuNr);
               err = true;
            }
         }
      }
      else
         AppendMenu(info->menu, MF_STRING, info->accel[j].cmd, menuLabel.Str());   

   }

   if(err)
   {
		editGUIWindow->widgets.remove(obj);
		editGUIWindow->widgets.sort();
      delete obj;
      obj = NULL;
      return(ERR);
   }

   obj->data = (char*)info;

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(OK);
}


CText GetMenuAccelerator(CText &menuLabel)
{
   CText result;
   long pos = menuLabel.FindSubStr(0,"\\t");

   if(pos >= 0 && pos+2<menuLabel.Size())
      result =menuLabel.End(pos+2);
   else
      result = "";

   return(result);
}


/********************************************************
   Convert an accelerator string (such as Ctrl+Esc)
   to the corresponding windows structure.
********************************************************/

bool StrToAccelStruct(char* str, ACCEL* accel)
{
   CText code,modifier;
   accel->fVirt = 0;
   CArg carg;
   int j;
   int nrList = sizeof(Vklist)/sizeof(VKeys);

// Count number of arguments separated by "+"
   carg.Init('+');
   int n = carg.Count(str);

   if(n > 1)
   {
      for(int k = 1; k <= n; k++)
      {
         if(k < n) // Extract all the modifiers
         {
            modifier = carg.Extract(k);
            modifier.UpperCase();
            if(modifier == "CTRL")
               accel->fVirt |= FCONTROL+FVIRTKEY;
            else if(modifier == "ALT")
               accel->fVirt |= FALT+FVIRTKEY;
            else if(modifier == "SHIFT")
               accel->fVirt |= FSHIFT+FVIRTKEY;
            else
               TextMessage("\n   Error: Invalid modify string '%s' in menu string '%s'.\n",modifier.Str(),str);
         }
         else // Last argument is the key code
         {
            code = carg.Extract(k);
            if(code.Size() > 1)
            {
               code.UpperCase();
               for(j = 0; j < nrList; j++) // Search through virtual-key code list
               {
                  if(code == Vklist[j].name)
                  {
                     accel->fVirt |= FVIRTKEY;
                     accel->key = Vklist[j].code;
                  }
               }
            }
            else
               accel->key = (short)code[0];
         }
      }
   }
   else // No modifiers so just extract the key code
   {
      if(strlen(str) == 0) // No accelerator at all
      {
         accel->key = 0;
         return(false);
      }
      code = str;
      if(code.Size() > 1) // Multi-character key code
      {
         code.UpperCase();
         for(j = 0; j < nrList; j++) // Search through virtual-key code list
         {
            if(code == Vklist[j].name)
            {
               accel->fVirt |= FVIRTKEY;
               accel->key = Vklist[j].code;
            }
         }
      }
      else // Single ASCII code
         accel->key = (short)str[0];
   }

   return(true);
}

// Make a button

int MakeButton(Interface* itfc ,char args[])
{
   static short nr,x,y,w,h;
   short r;
   CText name;
   Variable wx,wy,ww,wh;
   ObjPos pos;

   objCommand[0] = '\0';

   if((r = ArgScan(itfc,args,6,"nr,x,y,w,h,name,[objCommand]","eeeeeec","dvvvvts",&nr,&wx,&wy,&ww,&wh,&name,objCommand)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }
   
   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

   PushButtonInfo* info = new PushButtonInfo;
   info->hImage = NULL;
   info->fontHeight = 0;
   info->fontName = "";
   info->italicText = FALSE;
   info->boldText = FALSE;
   info->hovering = FALSE;
   info->selected = FALSE;
   info->defaultButton = FALSE;

   ReplaceSpecialCharacters(name.Str(),"\\n","\n",-1);
   ObjectData *obj = new ObjectData(editGUIWindow,BUTTON,nr,x,y,w,h,name.Str(),objCommand,(char*)info,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);

   SaveObjectPosition(obj,&pos);  

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(0);
}


int MakeTabCtrl(Interface* itfc ,char args[])
{
   static short nr,x,y,w,h;
   short r;
   CText name;
   Variable wx,wy,ww,wh;
   ObjPos pos;

   objCommand[0] = '\0';

   if((r = ArgScan(itfc,args,5,"nr,x,y,w,h,[objCommand]","eeeeec","dvvvvs",&nr,&wx,&wy,&ww,&wh,objCommand)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }
   
   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

   ReplaceSpecialCharacters(name.Str(),"\\n","\n",-1);
   ObjectData *obj = new ObjectData(editGUIWindow,TABCTRL,nr,x,y,w,h,name.Str(),objCommand,(char*)0,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);
 
   char tabName[MAX_STR];
   TCITEM tci;
   tci.mask = TCIF_TEXT;
   tci.iImage = -1;
   tci.pszText = tabName;
   tci.cchTextMax = MAX_STR;

   TabCtrl_DeleteAllItems(obj->hWnd);
   strncpy_s(tabName,MAX_STR,"tab1",_TRUNCATE);
   TabCtrl_InsertItem(obj->hWnd, 0, &tci);
   strncpy_s(tabName,MAX_STR,"tab2",_TRUNCATE);
   TabCtrl_InsertItem(obj->hWnd, 1, &tci);
   strncpy_s(tabName,MAX_STR,"tab3",_TRUNCATE);
   TabCtrl_InsertItem(obj->hWnd, 2, &tci);

   TabInfo *info = new TabInfo;
   info->nrTabs = 3;
   info->tabLabels = new CText[3];
   info->tabLabels[0] = "tab1";
   info->tabLabels[1] = "tab2";
   info->tabLabels[2] = "tab3";
   obj->data = (char*)info;

   SaveObjectPosition(obj,&pos);  

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(0);
}


int MakeUpDown(Interface* itfc ,char arg[])
{
   static short nr,x,y,w,h;
   short r;
   static char orient[50];
   UpDownInfo* info;
   Variable wx,wy,ww,wh;
   objCommand[0] = '\0';
   ObjPos pos;

   if((r = ArgScan(itfc,arg,6,"nr,x,y,w,h,orient,[objCommand]","eeeeeec","dvvvvss",&nr,&wx,&wy,&ww,&wh,orient,objCommand)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }
   
   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

   ObjectData *obj = new ObjectData(editGUIWindow,UPDOWN,nr,x,y,w,h,orient,objCommand,(char*)0,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);
   SaveObjectPosition(obj,&pos);  

   info = new UpDownInfo;
   info->value = 0;
   info->stepSize = 1;
   info->nrSteps = 10;
   info->base = 0;

   obj->data = (char*)info;
      
   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(0);
}

int MakeDivider(Interface* itfc ,char args[])
{
   short nr,x,y,w,h;
   short r;
   Variable wx,wy,ww,wh;
   ObjPos pos;
   CText orient = "horiz";

   objCommand[0] = '\0';

   if((r = ArgScan(itfc,args,6,"nr, x, y, w, h, orient","eeeeee","dvvvvt",&nr,&wx,&wy,&ww,&wh,&orient)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }
   
   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

   ObjectData *obj = new ObjectData(editGUIWindow,DIVIDER,nr,x,y,w,h,NULL,objCommand,(char*)0,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);
 
   SaveObjectPosition(obj,&pos);  

   DividerInfo *info = new DividerInfo;

   if(orient == "horiz" || orient == "horizontal")
   {
      info->origPos = y;
      info->orientation = WindowLayout::HORIZONTAL;
   }
   else
   {
      info->origPos = x;
      info->orientation = WindowLayout::VERTICAL;
   }
   info->minPos.xo = 0;
   info->minPos.xs = 0;
   info->maxPos.xo = 0;
   info->maxPos.xs = 0;
   info->useLimits = false;

   obj->data = (char*)info;

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(0);
}



int MakeHTMLBox(Interface* itfc ,char args[])
{
   static short nr,x,y,w,h;
   short r;
   Variable wx,wy,ww,wh;
   ObjPos pos;

   objCommand[0] = '\0';

   if((r = ArgScan(itfc,args,5,"nr,x,y,w,h","eeeee","dvvvv",&nr,&wx,&wy,&ww,&wh)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }
   
   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

   ObjectData* obj = new ObjectData(editGUIWindow,HTMLBOX,nr,x,y,w,h,NULL,objCommand,(char*)0,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);
   
   if(obj->hWnd)
   {
      InitialiseHtmlViewer(obj->hWnd);
   }
 
   SaveObjectPosition(obj,&pos);  

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(0);
}

int MakeGridCtrl(Interface* itfc ,char args[])
{
   static short nr,x,y,w,h;
   short r;
   Variable wx,wy,ww,wh;
	short cols, rows;
   ObjPos pos;

   objCommand[0] = '\0';

   if((r = ArgScan(itfc,args,5,"nr,x,y,w,h,cols,rows","eeeeeee","dvvvvdd",&nr,&wx,&wy,&ww,&wh,&cols,&rows)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }
   
   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

	GridCtrlInfo* info = new GridCtrlInfo;
	info->cols = cols;
	info->rows = rows;
   ObjectData *obj = new ObjectData(editGUIWindow,GRIDCTRL,nr,x,y,w,h,NULL,objCommand,(char*)info,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);

   SaveObjectPosition(obj,&pos);  

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(0);
}


int MakeColorBox(Interface* itfc ,char args [])
{
   static short nr,x,y,w,h;
   Variable colorVar;
   short r;
   CText name;
   Variable wx,wy,ww,wh;
   ObjPos pos;

   objCommand[0] = '\0';

   if((r = ArgScan(itfc,args,6,"nr,x,y,w,h,color,[objCommand]","eeeeeec","dvvvvvs",&nr,&wx,&wy,&ww,&wh,&colorVar,objCommand)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }
   
   BYTE* data = new BYTE[4];

   if(VarType(&colorVar) == MATRIX2D)
   {
      if(VarWidth(&colorVar) == 3 && VarHeight(&colorVar) == 1)
      {
         data[0]  = nint(VarRealMatrix(&colorVar)[0][0]);
         data[1]  = nint(VarRealMatrix(&colorVar)[0][1]);
         data[2]  = nint(VarRealMatrix(&colorVar)[0][2]);
         data[3]  = 0;
      }
      else if(VarWidth(&colorVar) == 4 && VarHeight(&colorVar) == 1)
      {
         data[0]  = nint(VarRealMatrix(&colorVar)[0][0]);
         data[1]  = nint(VarRealMatrix(&colorVar)[0][1]);
         data[2]  = nint(VarRealMatrix(&colorVar)[0][2]);
         data[3]  = nint(VarRealMatrix(&colorVar)[0][3]);
      }
   }
   else
   {
		delete[] data;
      ErrorMessage("invalid color matrix");
      return(ERR);
   }

   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
	{
		delete[] data;
      return(ERR);
	}

   ObjectData *obj = new ObjectData(editGUIWindow,COLORBOX,nr,x,y,w,h,name.Str(),objCommand,(char*)data,g_objVisibility,itfc->lineNr);
   editGUIWindow->widgets.add(obj);
   SaveObjectPosition(obj,&pos);  

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(0);
}


int MakePicture(Interface* itfc ,char args[])
{
   static short nr,x,y,w,h;
   Variable colorVar;
   short r;
   static char name[50];
   Variable wx,wy,ww,wh;
   ObjPos pos;

   objCommand[0] = '\0';

   if((r = ArgScan(itfc,args,5,"nr,x,y,w,h,[objCommand]","eeeeec","dvvvvs",&nr,&wx,&wy,&ww,&wh,objCommand)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }
  
   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

   ObjectData *obj = new ObjectData(editGUIWindow,PICTURE,nr,x,y,w,h,name,objCommand,(char*)NULL,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);
   
   SaveObjectPosition(obj,&pos);  

   PictureInfo *info = new PictureInfo;
   info->bmp  = NULL;
   info->resizePixToFitFrame = true;
   info->useBlueScreen = true;
   obj->data = (char*)info;


   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(0);
}


int MakeListBox(Interface* itfc ,char args[])
{
   static short nr,x,y,w,h;
   short r;
   Variable wx,wy,ww,wh;
   ObjPos pos;

   objCommand[0] = '\0';

   if((r = ArgScan(itfc,args,5,"nr,x,y,w,h,[objCommand]","eeeeec","dvvvvs",&nr,&wx,&wy,&ww,&wh,objCommand)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }
   
   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

   ObjectData *obj = new ObjectData(editGUIWindow,LISTBOX,nr,x,y,w,h,"",objCommand,(char*)0,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);
   
   SaveObjectPosition(obj,&pos);  

	obj->data = (char*)new ListBoxInfo;
   ListBoxInfo* info = (ListBoxInfo*)obj->data;
	info->colWidth = NULL;
	info->menu = NULL;
	info->menuNr = 0;
	info->nrColumns = 1;
	info->hasIcons = false;
	info->allowMultipleSelections = false;
	info->firstLineSelected = -1;
	info->lastLineSelected = -1;

   obj->keepFocus = true;

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(0);
}

int MakeGetMessage(Interface* itfc ,char args[])
{
   short nr;
   short r;
   
   objCommand[0] = '\0';

   if((r = ArgScan(itfc,args,2,"nr,command","ec","ds",&nr,objCommand)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }

	ObjectData* obj = new ObjectData(editGUIWindow,GETMESSAGE,nr,0,0,0,0,NULL,objCommand,(char*)NULL,g_objVisibility,itfc->lineNr);
   editGUIWindow->widgets.add(obj);
   
   return(0);
}

void CALLBACK TimerProc(HWND, UINT, UINT, DWORD);

// Makes a repetitive timer

long timerID;

//int MakeTimer(char arg[])
//{
//   float time;
//   short r;
//   static char name[50];
//   
//   objCommand[0] = '\0';
//
//   if((r = ArgScan(itfc,arg,1,"time","e","f",&time)) < 0)
//      return(r);
//
//   timerID = (float)SetTimer(NULL,NULL,(UINT)(time*1000),TimerProc);
//   
//   ansVar->MakeAndSetFloat(timerID);
//   
//   return(0);
//}
//
//int StopTimer(char arg[])
//{
//   short r;
//   long id;
//   
//   if((r = ArgScan(itfc,arg,1,"timer ID","e","l",&id)) < 0)
//   return(r);
//
//   KillTimer(NULL,id);
//   return(0);
//}

void CALLBACK TimerProc(HWND hwnd, UINT mesg, UINT eventID, DWORD time)
{
   char str[50];
   sprintf(str,"Timer,%d",eventID);
   SendMessageToGUI(str,-1);
}

int MakeComboBox(Interface* itfc ,char args[])
{
   static short nr,x,y,w,h;
   Variable wx,wy,ww,wh;
   ObjPos pos;

   short r;
   
   objCommand[0] = '\0';

   if((r = ArgScan(itfc,args,5,"nr,x,y,w,h,[commands]","eeeeec","dvvvvs",&nr,&wx,&wy,&ww,&wh,objCommand)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }
    
   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

   ObjectData *obj = new ObjectData(editGUIWindow,TEXTMENU,nr,x,y,w,h,NULL,objCommand,(char*)0,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);

   pos.ho = obj->ho;
   SaveObjectPosition(obj,&pos); 

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;
   
   return(OK);
}

int MakeTextBox(Interface* itfc ,char arg[])
{
   static short nr,x,y,w,h=21;
   Variable wx,wy,ww,wh;
   int r;
   ObjPos pos;

   objCommand[0] = '\0';
   if((r = ArgScan(itfc,arg,4,"nr,x,y,w,[objCommand]","eeeec","dvvvs",&nr,&wx,&wy,&ww,objCommand)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }
         
   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,0,x,y,w,h,&pos) == ERR)
      return(ERR);

    pos.hs = 0;
    pos.ho = h;

   ObjectData *obj = new ObjectData(editGUIWindow,TEXTBOX,nr,x,y,w,h,"",objCommand,(char*)0,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);

   //obj->bgColor = RGB(255,255,255);
   //HDC hdc = GetDC(obj->hWnd);
   //SetBkColor(hdc,obj->bgColor);
   //ReleaseDC(obj->hWnd,hdc);

   SaveObjectPosition(obj,&pos); 

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(0);
}

int MakeGroupBox(Interface* itfc ,char args[])
{
   static short nr,x,y,w,h;
   Variable wx,wy,ww,wh;
   short r;
   CText label;
   ObjPos pos;

   if((r = ArgScan(itfc,args,6,"nr,label,x,y,w,h","eeeeee","dtvvvv",&nr,&label,&wx,&wy,&ww,&wh)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }

   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);
    
   ObjectData *obj = new ObjectData(editGUIWindow,GROUP_BOX,nr,x,y,w,h,label.Str(),"",(char*)0,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);
   
   SaveObjectPosition(obj,&pos); 

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(0);
}

int MakeStaticText(Interface* itfc ,char args[])
{
   static short nr,x,y,w,h;
   int r;
   CText text;
   static CText position = "left";
   Variable wx,wy,ww,wh;
   ObjPos pos;
   CArg carg;

// Add the parameters
   short nrArgs = carg.Count(args);
   if(nrArgs == 5)
   {
      if((r = ArgScan(itfc,args,5,"nr,x,y,position,text","eeeee","dvvtt",&nr,&wx,&wy,&position,&text)) < 0)
         return(r);
      ww.MakeAndSetFloat(0);
      wh.MakeAndSetFloat(0);
   }
   else if(nrArgs == 7)
   {
      if((r = ArgScan(itfc,args,7,"nr,x,y,w,h,position,text","eeeeeee","dvvvvtt",&nr,&wx,&wy,&ww,&wh,&position,&text)) < 0)
         return(r);
   }
   else
   {
      ErrorMessage("Invalid number of arguments for statictext command");
      return(ERR);
   }


// Check there is a window to add the text to
   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }
  
// Check for valid position string
   if(position != "left" && position != "right" &&
      position != "center" && position != "centre")
   {
      ErrorMessage("invalid text position");
      return(ERR);
   }

   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

// Static text information
   StaticTextInfo* info = new StaticTextInfo;
   if(nrArgs == 5)
      info->multiLine = false;
   else
      info->multiLine = true;
   info->fontHeight = 0;
   info->fontName = "";
   info->italicText = FALSE;
   info->boldText = FALSE;

// Make the static text control
   ObjectData *obj = new ObjectData(editGUIWindow,STATICTEXT,nr,x,y,w,h,text.Str(),position.Str(),(char*)info,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);

   pos.xo = pos.xo-x+obj->xo;
   pos.wo = obj->wo;
   pos.ho = obj->ho;
   pos.ws = 0;
   pos.hs = 0;

   SaveObjectPosition(obj,&pos);  

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(0);
}

//int MakeStatic2Text(char arg[])
//{
//   static short nr,x,y,w,h;
//   int r;
//   char text[MAX_STR];
//   static char position[20] = "left";
//   ObjectData *obj;
//   Variable wx,wy,ww,wh;
//   ObjPos pos;
//
//   if((r = ArgScan(itfc,arg,5,"nr,x,y,position,text","eeeee","dttss",&nr,&wx,&wy,position,text)) < 0)
//      return(r);
//
//   if(!editGUIWindow)
//   {
//      ErrorMessage("No window being edited");
//      return(ERR);
//   }
//         
//   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,0,0,x,y,w,h,&pos) == ERR)
//      return(ERR);
//
//   obj = editGUIWindow->objList.Add(editGUIWindow,STATICTEXT2,nr,x,y,w,h,text,position,(char*)0,g_objVisibility);
//   obj->data = new char[strlen(text)+1];
//   strcpy(obj->data,text);
//
//   SaveObjectPosition(obj,&pos);  
//
//   return(0);
//}

int MakeCheckBox(Interface* itfc ,char args[])
{
   static short nr,x,y,w=14,h=13;
   short r;
   CText states;
   CText init;
   CheckButtonInfo *info;
   Variable wx,wy,ww,wh;
   ObjPos pos;
   
   objCommand[0] = '\0';
     
   if((r = ArgScan(itfc,args,5,"nr,x,y,states,init,[command]","eeeeec","dvvtts",&nr,&wx,&wy,&states,&init,objCommand)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR); 
   }
   
// Allocate space for extra button info
      
   info = new CheckButtonInfo;

// Initialize info 
   info->states = new char[states.Size()+1];
	CArg carg;
	carg.Count(states.Str());
   strcpy(info->states,states.Str());
   info->init = carg.Find(init.Str());

   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,0,0,x,y,w,h,&pos) == ERR)
	{
		delete info;
      return(ERR);
	}

// Make checkbox object 
   ObjectData *obj = new ObjectData(editGUIWindow,CHECKBOX,nr,x,y,w,h,"",objCommand,(char*)info,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);
   
   pos.wo = w;
   pos.ho = h;
   pos.ws = 0;
   pos.hs = 0;

   SaveObjectPosition(obj,&pos);  

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(OK);
}


int MakeRadioButtons(Interface* itfc ,char args[])
{
   static short nr,x,y,w,h;
   static char orient[50] = "horiz";
   int r;
   RadioButtonInfo *info;
   char states[MAX_STR];
   char init[50];
   Variable wx,wy;
   ObjPos pos;
	CArg carg;

   objCommand[0] = '\0';
   
   info = new RadioButtonInfo;

   if((r = ArgScan(itfc,args,7,"nr,x,y,spacing,orient,states,init,[objCommand]","eeeeeeec","dvvdssss",
                   &nr,&wx,&wy,&info->spacing,orient,states,init,objCommand)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }
   
// Check for errors
     
   if(!strncmp(orient,"horiz",5)) info->orient = 'h';
   else if(!strncmp(orient,"vert",4)) info->orient = 'v';
   else 
   {
      ErrorMessage("invalid orientation (horiz/vert)");
      return(ERR);
   }
   info->states = new char[strlen(states)+1];
   strcpy(info->states,states);
   
   info->nrBut = carg.Count(states);
   info->init = carg.Find(init);
     
   info->hWnd = new HWND[info->nrBut];

   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,0,0,x,y,w,h,&pos) == ERR)
      return(ERR);

   ObjectData *obj = new ObjectData(editGUIWindow,RADIO_BUTTON,nr,x,y,0,0,"",objCommand,(char*)info,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj); 

   pos.ws = 0;
   pos.wo = obj->wo;
   pos.hs = 0;
   pos.ho = obj->ho;

   SaveObjectPosition(obj,&pos);  

   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->retVar[1].SetScope(WINDOW);
   itfc->nrRetValues = 1;

   return(OK);
}


int ReplaceRadioButtonObject(Interface* itfc ,ObjectData *obj, char args[])
{
   static short nr,x,y,w,h;
   CText orient = "horiz";
   int r;
   RadioButtonInfo *info;
   CText states;
   CText init;
   Variable wx,wy;
   ObjPos pos;
   CArg carg;

   objCommand[0] = '\0';
   
   info = new RadioButtonInfo;

   if((r = ArgScan(itfc,args,7,"nr,x,y,spacing,orient,states,init,[objCommand]","eeeeeeec","dvvdttts",
                   &nr,&wx,&wy,&info->spacing,&orient,&states,&init,objCommand)) < 0)
      return(r);

// Check for errors
     
   if(!strncmp(orient.Str(),"horiz",5)) info->orient = 'h';
   else if(!strncmp(orient.Str(),"vert",4)) info->orient = 'v';
   else 
   {
      ErrorMessage("invalid orientation (horiz/vert)");
      return(ERR);
   }
   info->states = new char[strlen(states.Str())+1];
   strcpy(info->states,states.Str());
   
   info->nrBut = carg.Count(states.Str());
   info->init = carg.Find(init.Str());
     
   info->hWnd = new HWND[info->nrBut];

   if(ProcessObjectPosition(obj->winParent,&wx,&wy,0,0,x,y,w,h,&pos) == ERR)
      return(ERR);

// Get some previous parameters
   HWND win = obj->winParent->hWnd;
   WinData* parent = obj->winParent;

// Delete old object entries
   obj->FreeData();

   obj->command = new char[strlen(objCommand)+1];
	strcpy(obj->command,objCommand);
   obj->xo = x;
   obj->yo = y;
   obj->wo = w;
   obj->ho = h;
   obj->selected_ = false;
   obj->cmdLineNr = itfc->lineNr;
 	obj->hwndParent = win;
   obj->winParent = parent;

   int xp = x;
   int yp = y;
   
   obj->data = (char*)info;
   
   for(int i = 0; i < info->nrBut; i++)
   {
      info->hWnd[i] = CreateWindow("button", "", WS_CHILD | BS_RADIOBUTTON | WS_VISIBLE,
   			                   xp, yp, 14, 14, 
   			                   obj->hwndParent, (HMENU)obj->nr(),
   			                   prospaInstance, NULL);
      OldButtonProc = (WNDPROC)SetWindowLong(info->hWnd[i],GWL_WNDPROC,(LONG)ButtonEventProc); 
   			                   
   	if(i == info->init-1)
   		SendMessage(info->hWnd[i],BM_SETCHECK,1,0);
	                   
       if(info->orient == 'h')
         xp += info->spacing;
      else
         yp += info->spacing;
   }
   
   obj->xo = x;
   obj->yo = y;
   if(info->orient == 'h')
   {
      obj->wo = (info->nrBut-1)*info->spacing+14;
      obj->ho = 14;
   }
   else
   {
      obj->wo = 14;
      obj->ho = (info->nrBut-1)*info->spacing+14;
   }

   pos.ws = 0;
   pos.wo = obj->wo;
   pos.hs = 0;
   pos.ho = obj->ho;

   SaveObjectPosition(obj,&pos);  

   return(OK);
}



int ReplaceUpDownObject(Interface* itfc ,ObjectData *obj, char args[])
{
   static short nr,x,y,w,h;
   static char orient[50] = "horiz";
   int r;
   long dir;
   ObjPos pos;
   Variable wx,wy,ww,wh;

   objCommand[0] = '\0';

   if((r = ArgScan(itfc,args,6,"nr,x,y,w,h,orient,[objCommand]","eeeeeec","dvvvvss",&nr,&wx,&wy,&ww,&wh,orient,objCommand)) < 0)
      return(r);

// Check for errors
   if(!strncmp(orient,"horiz",5))      dir = UDS_HORZ;
   else if(!strncmp(orient,"vert",4))  dir = 0;
   else 
   {
      ErrorMessage("invalid orientation (horiz/vert)");
      return(ERR);
   }

   if(ProcessObjectPosition(obj->winParent,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

// Get some previous parameters
   HWND win = obj->winParent->hWnd;
   WinData* parent = obj->winParent;

// Delete old object entries
   obj->FreeData();

// Initialise new object values
   obj->command = new char[strlen(objCommand)+1];
	strcpy(obj->command,objCommand);
   obj->xo = x;
   obj->yo = y;
   obj->wo = w;
   obj->ho = h;
   obj->selected_ = false;
   obj->cmdLineNr = itfc->lineNr;
 	obj->hwndParent = win;
   obj->winParent = parent;
   UpDownInfo *info = new UpDownInfo;
   info->value = 0;
   obj->data = (char*)info;
   
// Make new control
   obj->hWnd = CreateUpDownControl(WS_CHILD | WS_VISIBLE | UDS_ARROWKEYS | dir,
   			                x, y, w, h, 
   			                obj->hwndParent, (int)obj->nr(),
   			                prospaInstance, NULL, 2, 0, 1);
   OldUpDownProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)UpDownEventProc);

   SaveObjectPosition(obj,&pos);  

   return(OK);
}



int DrawObject(Interface* itfc ,char args[])
{
   short n;
   static short winNr,objNr;
   ObjectData *obj;
   WinData *win;
   
   if((n = ArgScan(itfc,args,2,"winNr, objNr","ee","dd",&winNr,&objNr)) < 0)
      return(n);

// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}

	if(!(win = rootWin->FindWinByNr(winNr)))
	{
	   ErrorMessage("window '%ld' not found",(long)winNr);
	   return(ERR);
	}

   if(!(obj = win->widgets.findByNr(objNr)))
   {
      ErrorMessage("object '%ld' not found",(long)objNr);
      return(ERR);
   }
         
	MyInvalidateRect(obj->hWnd,NULL,false);   		       
	itfc->nrRetValues = 0;
   return(OK);
}

// Check is an object exists

int IsObject(Interface* itfc ,char args[])
{
   short n;
   static short winNr,objNr;
   ObjectData *obj;
   WinData *win;
   RECT r;
   
   if((n = ArgScan(itfc,args,2,"winNr, objNr","ee","dd",&winNr,&objNr)) < 0)
      return(n);

// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}
//	TextMessage("IsObject winNr = %d\n",winNr);
   itfc->nrRetValues = 1;

	if(!(win = rootWin->FindWinByNr(winNr)))
	{
      itfc->retVar[1].MakeAndSetFloat(0);
	   return(OK);
	}

   if(!(obj = win->widgets.findByNr(objNr)))
   {
      itfc->retVar[1].MakeAndSetFloat(0);
      return(OK);
   }

   itfc->retVar[1].MakeAndSetFloat(1);

   return(OK);

}

// Remove and object

int RemoveObject(Interface* itfc ,char args[])
{
   short n;
   static short winNr,objNr;
   ObjectData *obj;
   WinData *win;
   RECT r;
   
   if((n = ArgScan(itfc,args,2,"winNr, objNr","ee","dd",&winNr,&objNr)) < 0)
      return(n);

// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}

	if(!(win = rootWin->FindWinByNr(winNr)))
	{
	   ErrorMessage("window '%ld' not found",(long)winNr);
	   return(ERR);
	}

   if(!(obj = win->widgets.findByNr(objNr)))
   {
      ErrorMessage("object '%ld' not found",(long)objNr);
      return(ERR);
   }

// Make sure window region is redrawn after deletion 
   if(obj->type == RADIO_BUTTON)
   {
      RadioButtonInfo *info = (RadioButtonInfo*)obj->data;
	   for(int i = 0; i < info->nrBut; i++)
	   {
         GetWindowRect(info->hWnd[i],&r);
         MyInvalidateRect(info->hWnd[i],&r,false);
      }
   }
   else
   {
      GetClientRect(obj->hWnd,&r);
	   MyInvalidateRect(obj->hWnd,&r,false);   		
   }

// Remove the object (V2.2.5)
	win->widgets.remove(obj);
	win->widgets.sort();
   delete obj;
	itfc->nrRetValues = 0;
   return(OK);
}
// This command should be replaced by a call to showwindow

int ShowObjects(Interface* itfc ,char args[])
{
   short n;
   static short winNr,objNr;
   WinData *win;

// Get the object or objects to make visible ***
   
   if((n = ArgScan(itfc,args,1,"winNr, objNr","ee","dd",&winNr, &objNr)) < 0)
      return(n);

// Find window *********************************

// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}

	if(!(win = rootWin->FindWinByNr(winNr)))
	{
	   ErrorMessage("window '%ld' not found",(long)winNr);
	   return(ERR);
	}

// Show all specified objects *******************
	
   if(n == 1)
   {
		for(ObjectData* obj: win->widgets.getWidgets())
		{
         obj->Show(true);
      }
   }
   else
   {
      ObjectData* obj = win->widgets.findByNr(objNr);
      if(obj)
      {
         obj->Show(true);
      }
      else
      {
         ErrorMessage("no such object number");
         return(ERR);
      }
   }
   win->activate();								// Activate window
   SetCurrentGUIParent();                 // Make sure its parent is updated to match other GUI windows
	ShowWindow(win->hWnd, SW_SHOWNORMAL);
	MyUpdateWindow(win->hWnd) ;

	itfc->nrRetValues = 0;
   return(OK);
}

/************************************************************************************
            Display a dialog window and wait for the user to press ok button

            Syntax: (r1,r2,r3, ...) = showdialog() or r1 = showdialog()

************************************************************************************/

Variable dialogVar[MAX_RETURN_VARIABLES];
short nrDialogValues;

int ShowDialog(Interface* itfc ,char args[])
{
   short n;
   static short winNr;
   WinData *win;
   CWinEnable winEnable;
   WinData *oldGUI = NULL;
   bool oldDialogDisplayed = false;

// Get the object or objects to make visible ***
   if((n = ArgScan(itfc,args,1,"winNr","e","d",&winNr)) < 0)
      return(n);

// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}

	if(!(win = rootWin->FindWinByNr(winNr)))
	{
	   ErrorMessage("window '%ld' not found",(long)winNr);
	   return(ERR);
	}

   if(win->type != DIALOG_WIN)
   {
      ErrorMessage("Window %d is not a dialog",winNr);
      return(ERR);
   }
   
// Record the previous current GUI window and macro parent
   if(GetGUIWin())
   {
      oldGUI = GetGUIWin()->oldGUI;
   }

// Remove close button from window **************
   long style = GetWindowLong(win->hWnd,GWL_STYLE);
   SetWindowLong(win->hWnd,GWL_STYLE,(~WS_SYSMENU) & style);

// Show all objects within this window **********
   for(ObjectData* obj: win->widgets.getWidgets())
   {
      if(obj->visible)
         obj->Show(true); 
   }

// Hide controls inside hidden tabs ***************
   if(win->updateControlVisibilityFromTabs() == ERR)
      return(ERR);

// Show the dialog window *************************
   oldDialogDisplayed = dialogDisplayed;
   ShowWindow(win->hWnd,SW_SHOW);
   dialogDisplayed = true;
   SetCursor(LoadCursor(NULL,IDC_ARROW));

// Make sure all controls show a focus rect.
// (a manifest bug means that some controls do not)
   SendMessage(win->hWnd,WM_UPDATEUISTATE,UIS_CLEAR + (UISF_HIDEFOCUS * 65536),0);

// Make sure all control objects are assigned as local variables
   CText nr;
   nr.Format("%hd",winNr);
   AssignControlObjects(itfc,nr.Str());

   win->activate();

// Disable all windows except dialog ***************
   winEnable.Disable(win->hWnd);

// Make sure dialog is above all others ***********
   SetWindowPos(win->hWnd,HWND_TOP,0,0,0,0,SWP_NOMOVE |SWP_NOSIZE);

// Wait for user to close dialog window ***********
   MSG msg;
	while(dialogDisplayed)	 
	{
	   if(GetMessage(&msg, 0,0,0)) // Don't use PeekMessage here since uses 100% CPU time
	   {
		   if(!TranslateAccelerator(currentAppWindow,currentAccel,&msg))
		   {
			   TranslateMessage(&msg);
			   DispatchMessage(&msg);
			}
		}

      if (GetAsyncKeyState(VK_ESCAPE) & 0x08000)
         dialogDisplayed = false;
	}
// Make sure we don't get a flash from another application
// when switch apps during dialog display
  	BringWindowToTop(prospaWin);

// Re-enable all windows (except dialog) *************
   winEnable.Enable(win->hWnd);

// Remove the dialog ********************************
   if(win->hWnd == prospaWin)
      prospaWin = NULL;
   HWND hwnd = win->hWnd;
	delete win;
   DestroyWindow(hwnd);  


// Restore previous current GUI window
// And deactivate all others if this is a dialog
   if(oldGUI) 
   {
      itfc->win = oldGUI;
		WinData::SetGUIWin(oldGUI);
      dialogDisplayed = oldDialogDisplayed;
      if(dialogDisplayed)
         winEnable.Disable(oldGUI->hWnd);
   }
   else
   {
      itfc->win = NULL;
		WinData::SetGUIWin(NULL);
   }




// Copy variables returned by closedialog ***********
   for(int i = 1; i <= nrDialogValues; i++)
   {
   	if(CopyVariable(&itfc->retVar[i],&dialogVar[i],FULL_COPY) == ERR) // Copy to output variable
         return(ERR);
      dialogVar[i].FreeData();
   }

   itfc->nrRetValues = nrDialogValues;


   return(OK);
}

/****************************************************************
 Disable all windows (gui and fixed) except win
*****************************************************************/

void DisableAllWindows(HWND win)
{
   for(WinData *w = rootWin->next; w != NULL; w = w->next)
   {
      if(w->hWnd != NULL && w->hWnd != win)
      {
         EnableWindow(w->hWnd,false);
      }               
   }

   EnableDialogs(false);
}

/****************************************************************
 Enable all windows (gui and fixed) except win
*****************************************************************/

void EnableAllWindows(HWND win)
{
   for(WinData *w = rootWin->next; w != NULL; w = w->next)
   {
      if(w->hWnd != NULL && w->hWnd != win)
      {
         EnableWindow(w->hWnd,true);
         
      }               
   }

   EnableDialogs(true);
}

/************************************************************************************
            Close the current dialog window

            Syntax: closedialg(r1, r2, r3 ...)

************************************************************************************/


int CloseDialog(Interface* itfc ,char arg[])
{
   short nrArgs = 0;
   char expression[MAX_STR];
   CArg carg;
   short e;
   Variable result;

// Evaluate each argument and store as a variable
   do
   {
      if((e = carg.GetNext(arg,expression)) == ERR)
         break;
	 	if(Evaluate(itfc,RESPECT_ALIAS,expression,&result) < 0)  // Evaluate it
		   return(ERR); 
      if(++nrArgs == MAX_RETURN_VARIABLES)
      {
         ErrorMessage("Too many return variables");
         return(ERR);
      }
	   if(CopyVariable(&dialogVar[nrArgs],&result,FULL_COPY) == ERR) // Copy to output variable
      {
         dialogDisplayed = false;
	      return(ERR);  
      }
	}
   while(e != FINISH);

// Take a note of the number of returned arguments
   nrDialogValues = nrArgs; 

// Set the global dialog display flag to false so window can exit
   dialogDisplayed = false;
   return(OK);
}

//WinData *GetLowestGUIWindow(void);

/***********************************************************************************
  Hide all gui windows non-topmost retaining the stacking order they had originally

  Routine works by finding the lowest level gui window first and then making it 
  non-top most i.e. placing it above all normal windows (prospaWin, plot1DWin etc)
************************************************************************************/

//void HideGUIandDialogWindows()
//{
//   WinData *win = &rootWin;
//
//   HWND hWnd = prospaWin;
//
//// Reset all "considered" flags
//   while((win = win->GetNextWin()) != NULL)
// 	   win->userdef = false; 
//
//// Find the lowest GUI window and make it not top most
//   while(true)
//   {
//      win = GetLowestGUIWindow();
//      if(!win) break;
// 	   ShowWindow(win->hWnd,SW_HIDE); 
//      win->userdef = true;
//   }
//
//   HideDiagWindows();
//
//   BringWindowToTop(prospaWin);
//}

void HideGUIWindows()
{
   WinData *win = rootWin;

   while((win = win->GetNextWin()) != NULL)
   {
      if(win->type != DIALOG_WIN && !win->permanent && !win->isMainWindow)   
         ShowWindow(win->hWnd,SW_HIDE);
   }
}

void ShowGUIWindows(int mode)
{
   WinData *win = rootWin;

   while((win = win->GetNextWin()) != NULL)
   {
      if(win->type != DIALOG_WIN && !win->permanent && !win->isMainWindow)   
         ShowWindow(win->hWnd,mode);
   }
}

bool AnyGUIWindowVisible()
{
   WinData *win = rootWin;

   while((win = win->GetNextWin()) != NULL)
   {
      if(win->hWnd && !win->isMainWindow && !win->permanent && (win->type != DIALOG_WIN) && IsWindowVisible(win->hWnd))
         return(true);
   }
   return(false);
}

/***********************************************************************************
  Check to see if hwnd is a Prospa window
************************************************************************************/

bool IsAProspaWindow(HWND hwnd)
{
   if(hwnd == prospaWin) return(true);
 //  if(hwnd == plot1DWin) return(true);
 //  if(hwnd == plot2DWin) return(true);
//   if(hwnd == plot3DWin) return(true);
   if(hwnd == cliWin)    return(true);
  // if(hwnd == editWin)   return(true);

   WinData *win = rootWin;
   while((win = win->GetNextWin()) != NULL)
   {
      if(win->hWnd == hwnd) return(true);
   }

   return(false);
}

/***********************************************************************************
  Make all gui windows topmost retaining the stacking order they had originally

  Routine works by finding the lowest level gui window first and then making it 
  topmost i.e. placing it above all normal windows (prospaWin, plot1DWin etc)
************************************************************************************/

//void ShowGUIandDialogWindows()
//{
//   WinData *win = &rootWin;
//
//   HWND hWnd = prospaWin;
//
//// Reset all "considered" flags
//   while((win = win->GetNextWin()) != NULL)
// 	   win->userdef = false; 
//
//// Find the lowest GUI window and make it top most
//   while(true)
//   {
//      win = GetLowestGUIWindow();
//      if(!win) break;
//      if(win->keepInFront) // Normal gui window will be infront
//         ShowWindow(win->hWnd,SW_SHOWNA);
//      win->userdef = true;
//   }
//
//   ShowDiagWindows();
//}

void ChangeGUIParent(HWND newParent)
{
   WinData *win = rootWin->GetNextWin();
   HWND parent = GetParent(newParent);


   while(win != NULL)
   {
      if(win->keepInFront && win->hWnd != parent  && win->hWnd != newParent)
      {
         SetWindowLong(win->hWnd, GWL_HWNDPARENT, (LONG)newParent);
         if(IsWindowVisible(win->hWnd))
            ShowWindow(win->hWnd,SW_SHOW);
         MyUpdateWindow(win->hWnd);
         win->parent = newParent;
      }
      win = win->GetNextWin();
   }

// Change all normal dialogs
   ChangeDialogParent(newParent);

}

//void MoveGUIWindowsToFront()
//{
//   WinData *win = &rootWin;
//
//   HWND hWnd = prospaWin;
//
//// Find the lowest GUI window and make it top most
//
//   win = GetLowestGUIWindow();
//   if(!win) return;
// 	SetWindowPos(win->hWnd,HWND_TOP,0,0,0,0,SWP_NOACTIVATE|SWP_NOMOVE |SWP_NOSIZE); 
//
//}

/***********************************************************************************
  Find the lowest GUI window based on the z-stack order
************************************************************************************/

//WinData *GetLowestGUIWindow()
//{
//   /*WinData *win = rootWin->GetNextWin();
//
//   HWND w = GetWindow(win->hWnd,GW_HWNDFIRST);
//
//   do
//   {
//      back = win;
// 	   hWnd = GetWindow(w,GW_HWNDNEXT); 
//      if(!hWnd) break;
//      win = GetWinDataClass(hWnd);
//   }
//   while(win);
//
//   win = GetWinDataClass(w);
//
//   if(win)
//   {
//      WinData *back = NULL;
//      HWND hWnd;
//
//      do
//      {
//         back = win;
// 	      hWnd = GetNextWindow(win->hWnd,GW_HWNDNEXT); 
//         if(!hWnd) break;
//         win = GetWinDataClass(hWnd);
//      }
//      while(win);
//
//      if(back)
//         return(back);
//   }*/
//   return(NULL);
//}



/********************************************************************************************
  Search for the current GUI window parent and force all GUI windows to have this parent
 (this is used when a new GUI window is formed which does not yet have a parent)
*********************************************************************************************/

void SetCurrentGUIParent()
{
   HWND parent = NULL;
   WinData *win = rootWin->GetNextWin();

// Scan through current GUI windows looking for parent (if any)
   while(win != NULL)
   {
      if(win->keepInFront && win->parent)
      {
         parent = win->parent;
         break;
      }
      win = win->GetNextWin();
   }
// Set the parent if one has been found
   if(parent)
      ChangeGUIParent(parent);
}

void UpdateCurrentGUIParent(HWND parent)
{
   WinData *win = rootWin->GetNextWin();

   while(win != NULL)
   {
      if(win->keepInFront)
      {
         win->parent = parent;
      }
      win = win->GetNextWin();
   }
}


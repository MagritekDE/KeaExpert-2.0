#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)
#include <Dbt.h>
#include <gdiplus.h> 
#include "stdafx.h"
#include "guiWindowsEvents.h"
#include "BabyGrid.h"
#include "command_other.h"
#include "button.h"
#include "debug.h"
#include "defineWindows.h"
#include "dialog_utilities.h"
#include "dividers.h"
#include "drawing.h"
#include "edit_class.h"
#include "edit_files.h"
#include "edit_utilities.h"
#include "evaluate.h"
#include "events_edit.h"
#include "files.h"
#include "globals.h"
#include "guiInteractiveWindowBuilder.h"
#include "guiMakeObjects.h"
#include "guiModifyObjectParameters.h"
#include "guiWindowClass.h"
#include "guiWindowCLI.h"
#include "hr_time.h"
#include "interface.h"
#include "listbox.h"
#include "main.h"
#include "message.h"
#include "metafile.h"
#include "mymath.h"
#include "plot3dClass.h"
#include "plot3dEvents.h"
#include "plot.h"
#include "PlotWindow.h"
#include "process.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "tab.h"
#include "thread.h"
#include "utilities.h"
#include "variablesOther.h"
#include "string_utilities.h"
#include <assert.h>
#include "mytime.h"
#include "button.h"
#include "memoryLeak.h"


using namespace Gdiplus;

#pragma warning (disable: 4312) // Ignore Get/SetWindowLong warnings

#define FREEDHEAP 0xfeeefeee 

WinData *messageSource;    // window which produced the message

void GradientFill(HDC hdc, RECT rect, LPCOLORREF color_array);
void GradientFill(HDC hdc, HRGN rgn, LPCOLORREF color_array);
void GetTabRegion(ObjectData *obj, HRGN hRgn);
bool gInMessageMode = false;
bool IsMouseInWindow(HWND hWnd);
int TextEditFunctions(Interface *itfc, char args[]);
int WhichObjectIsMouseOver(WinData*);

CText gCopyDataRetMessage;
bool gReceivedCopyDataMessage = false;
extern bool gBlockScroll;


/***********************************************************************************
      Functions related to events occurring in the gui (user defined) window        

   UserWinEventsProc ......... Main entry point for events
      ProcessLeftButtonDown .. Process left mouse click
      ProcessMouseMove ....... Process mouse movement
         SelectRegion
         MoveOrResizeObject
         ModifyCursor
      ProcessKeyDown ......... Process special keyboard characters (arrow keys)
      ProcessCharacter ....... Process raw keyboard character entry
      PreProcessCommand ...... Process control or menu command requests
         ProcessControlCommands
      ProcessContextMenu ..... Display and respond to contextual menu
         ProcessContextMenuItems
      ProcessScrollEvents .. process track bar movements
      EnterObjectInfoDlgProc . Enter information about a control.

   ButtonEventProc ........... event procedure for button
   GroupBoxEventProc ......... event procedure for groupbox
   ColorScaleEventProc ....... event procedure for colorscale
   TextMenuEventProc ......... event procedure for textmenu
   TextMenuEditEventProc ..... event procedure for textmenu edit region
   TextBoxEventProc .......... event procedure for text box
   ProgressBarEventProc ...... event procedure for progress-bar
   SliderEventProc ......... event procedure for track-bar
   StatusBarEventProc ........ event procedure for status-bar
   STextEventProc ............ event procedure for static-text
   SText2EventProc ........... event procedure for ???


   SendMessageToGUI .......... Send a message to all the GUI windows


************************************************************************************/

// Callback procedures for gui controls (handle special nonstandard events)
// (The addresses of these functions are needed when the controls are created)
LRESULT CALLBACK ButtonEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
//LRESULT CALLBACK ListBoxEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK STextEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SText2EventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK GroupBoxEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TextMenuEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TextMenuEditEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TextBoxEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PanelWinEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ProgressBarEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SliderEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK StatusBarEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ColorScaleEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EdLineProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HTMLEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK ToolBarEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK UpDownEventProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DebugStripEventProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK TabCtrlEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK PictureEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);

// Static Event processing routines
static void ProcessLeftButtonDown(LPARAM lParam,WPARAM wParam,HWND hWnd,WinData *win);
static void ProcessLeftButtonUp(LPARAM lParam,WPARAM wParam,HWND hWnd,WinData *win);
static void ProcessMouseMove(LPARAM lParam, WPARAM wParam, HWND hWnd, WinData *win);
static void ProcessContextMenu(LPARAM lParam, WPARAM wParam, HWND hWnd, WinData *win);
static bool ProcessKeyDown(LPARAM lParam, WPARAM wParam, HWND hWnd, WinData *win);
static void ProcessCharacter(LPARAM lParam, WPARAM wParam, HWND hWnd, WinData *win);
static bool ProcessScrollEvents(LPARAM lParam, WPARAM wParam, HWND hWnd, WinData *win);
static void SelectRegion(HWND hWnd, short &x0, short &y0, short &x2, short &y2);
static void MoveOrResizeObject(HWND hWnd, WinData *win, ObjectData *obj, short x, short y);
static void ModifyCursor(HWND hWnd, WinData *win, ObjectData *obj, short x, short y);
static void ProcessContextMenuItems(HWND hWnd,WinData *win, short item);
static void ProcessControlCommands(HWND, HWND, int);

// Global variables
ObjectData *currentObj; // Not sure what this is used for

static short pasteMode = GUI_IDLE; // Paste to current mouse location
static short g_click_x, g_click_y; // Last point clicked on
static RECT copyRect; // Current object copy region
static long timerID;
static long controlProcRunning = 0; // Number of procedures running when a slider is moved
static bool gWMPrintCalled = false;
void PaintTab(ObjectData *obj, LPDRAWITEMSTRUCT lpdis);
COLORREF gDesignerGridColor = RGB(200,200,200);
COLORREF gMainWindowColor = RGB(255, 255, 255);

/*****************************************************************************
*                   Event procedure for a user defined window  
* Events
*
* WM_ACTIVATE ........ window has been selected - make sure accelerator table is correct
* WM_CHAR ............ keyboard has been pressed - raw characters
* WM_CLOSE ........... a window is being closed
* WM_COMMAND ......... process control events
* WM_CONTEXTMENU ..... a contextual menu to to be displayed
* WM_DRAWITEM ........ a colorbox, picture etc is being drawn
* WM_ERASEBKGND ...... prevent flicker
* WM_GETMINMAXINFO ... intercept minimise/maximise event
* WM_HSCROLL ......... process slider horizontal events
* WM_INITMENUPOPUP ... a pop-up or drop down menu is about to be displayed
* WM_KEYDOWN ......... keyboard has been pressed (virtual key mode)
* WM_LBUTTONDBLCLK ... left button has been double clicked.
* WM_LBUTTONDOWN ..... plot selected also move, data, region selection 
* WM_LBUTTONUP ....... selection complete
* WM_MEASUREITEM ..... set the size of listbox items
* WM_MOUSEACTIVATE ... gui window selected
* WM_MOUSEMOVE ....... Move, data display region selection.
* WM_MOUSEWHEEL ...... process mouse wheel events
* WM_MOVE ............ window has been moved
* WM_MOVING .......... window is being moved
* WM_NCLBUTTONDOWN ... a title bar button has been pressed
* WM_NOTIFY .......... handle tooltips and updown control events
* WM_PAINT ........... redraw gui window
* WM_RBUTTONDOWN ..... contextual menus, data offset
* WM_SETCURSOR ....... control the cursor when moving the mouse
* WM_SIZE ............ parent resized
* WM_SIZE ............ window has been resized
* WM_SIZING .......... window is being resized
* WM_SYSCOMMAND ...... the window is being closed from the title bar
* WM_TIMER ........... a timer even has occurred
* WM_USER_LOADDATA ... load a file when prospa is already running
* WM_VSCROLL ......... process slider vertical events

*****************************************************************************/


GUID usbDSPGUID =  { 0xb35924d6, 0x3e16, 0x4a9e, { 0x97, 0x82, 0x55, 0x24, 0xa4, 0xb7, 0x9b, 0xac } };
GUID usbFX3GUID =  {0xae18aa60, 0x7f6a, 0x11d4, 0x97, 0xdd, 0x0, 0x1, 0x2, 0x29, 0xb9, 0x59};

static bool sliderMouseButtonDn = false; // This prevents the mouse from sticking in the down state when using a slider
static bool sliderMouseButtonUp = false; // This prevents the mouse from sticking in the down state when using a slider


LRESULT CALLBACK UserWinEventsProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
	WinData *win;
	ObjectData *obj;
   short x,y;
  // static bool record = false;

// Find Prospa window class instance for window event occurred in
   win = GetWinData(hWnd);
  
   //SYSTEMTIME loctime;
   //CText time;
   //short r;

   //   GetLocalTime(&loctime);

   //   time.Format("%02hd:%02hd",loctime.wMinute,loctime.wSecond,loctime.wMilliseconds);


   //if(record && win && messg == WM_ACTIVATEAPP)
   //{
   //   CText txt;
   //   txt.Format("Event AA = %X %d %hd %s\n",messg, wParam, win->nr, time.Str());
   //   OutputDebugString(txt.Str());
   //}

   //if(record && win && messg == WM_SHOWWINDOW && win->nr == 7)
   //{
   //   CText txt;
   //   txt.Format("Event SW = %X %d %hd %s\n",messg, wParam, win->nr, time.Str());
   //   OutputDebugString(txt.Str());
   //}


   //if(record && win && win->nr == 7)
   //{
   //   CText txt;
   //   IsWindowVisible(win->hWnd);
   //   txt.Format("Visible: %hd Iconic: %hd Event = %X %s\n",(short)(IsWindowVisible(win->hWnd)==TRUE),(short)(IsIconic(win->hWnd)==TRUE),messg, time.Str());
   //   OutputDebugString(txt.Str());
   //}

  //if(messg != 0x111 && messg != 0x20)
  //    TextMessage("mesg = %X\n",messg);

// Process the events ************************
	switch(messg)
	{

   // On creation
	   case(WM_CREATE): 
	   {
	      break;
      }

   // User event from external program
	  // case(WM_USER): 
	  // {
			//TextMessage("Seen event %ld %ld\n",wParam,lParam);
			//HWND w = FindWindow("MyWindowClass","win32gui test");
			//char lpszString[20] = "My string";
			//COPYDATASTRUCT cds;
			//cds.dwData = 1; // can be anything
			//cds.cbData = sizeof(CHAR) * (strlen(lpszString) + 1);
			//cds.lpData = lpszString;
			//SendMessage(w, WM_COPYDATA, (WPARAM)hWnd,(LPARAM)(LPVOID)&cds);
	  //    break;
   //   }

	// Run a macro by processing the COPYDATA event
	// data should be a simple char string - can be used
   // by an external program (e.g. Python) to run experiments using Prospa
   // Or to communicate between different Prospa apps
		case(WM_COPYDATA):
		{
			COPYDATASTRUCT* pcds = (COPYDATASTRUCT*)lParam;
			win = rootWin->FindWinByHWND(prospaWin);
         //TextMessage("Seen COPYDATA event %ld %ld\n",wParam,lParam);

			if(pcds->dwData == 1) // Command is coming from  external application e.g. Python
			{
          //  TextMessage("External application\n");

				Interface itfc;
				short r;
				itfc.win = win;
				r = ProcessMacroStr(&itfc,(char*)pcds->lpData);
            if (r == ERR)
            {
               COPYDATASTRUCT cds;
               cds.dwData = 1;
               cds.cbData = strlen("error") + 1;
               cds.lpData = "error";
               SendMessage((HWND)wParam, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)(LPVOID)&cds);
            }
            else if(itfc.nrRetValues == 1 && itfc.retVar[1].GetType() == UNQUOTED_STRING)
				{ // Send any return string back to the calling window
					char *data = itfc.retVar[1].GetString();
					COPYDATASTRUCT cds;
					cds.dwData = 1; 
					cds.cbData = strlen(data)+1;
					cds.lpData = data;
					SendMessage((HWND)wParam, WM_COPYDATA, (WPARAM)hWnd,(LPARAM)(LPVOID)&cds);
				}
			}
         if (pcds->dwData == 2) // Command is coming from Prospa (runremote)
         {
            Interface itfc;
            short r;
            itfc.win = win;
            r = ProcessMacroStr(&itfc, (char*)pcds->lpData);
            if (r == OK && itfc.nrRetValues == 1 && itfc.retVar[1].GetType() == UNQUOTED_STRING)
            { // Send any return string back to the calling window
               char* data = itfc.retVar[1].GetString();
               COPYDATASTRUCT cds;
               cds.dwData = 3;
               cds.cbData = strlen(data) + 1;
               cds.lpData = data;
               SendMessage((HWND)wParam, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)(LPVOID)&cds);
            }
            else
            {
               COPYDATASTRUCT cds;
               cds.dwData = 3;
               cds.cbData = strlen("error") + 1;
               cds.lpData = "error";
               SendMessage((HWND)wParam, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)(LPVOID)&cds);
            }
         }
         else if (pcds->dwData == 3) // Return value from running above macro (see runremote command)
         {
            gCopyDataRetMessage = (char*)pcds->lpData;
            gReceivedCopyDataMessage = true;
         }
			break;
		}

   // Ensure that all in-front windows have Prospa window as parent
   // when the Prospa application is deactivated. This prevents these
   // windows from appearing when opening desktop folders when Prospa has been 
   // hidden using the Show Desktop toggle and other subwindows are the parent.
   // Not sure why this happens but his fixes it!
	   case(WM_ACTIVATEAPP): 
	   {
         BOOL activate = (BOOL)wParam;
         if(win && prospaWin && win->keepInFront && !activate)
         {
          //  TextMessage("Parent changed to Prospa win");
            ChangeGUIParent(prospaWin);
         }
	      break;
      }

      // Detects if a Spectrometer has been plugged in or removed
       case WM_DEVICECHANGE:
       {
          if(hWnd == prospaWin) // Ignore messages to other toplevel windows
          {
             DEV_BROADCAST_DEVICEINTERFACE* db = (DEV_BROADCAST_DEVICEINTERFACE*)lParam;
             
             if(wParam == DBT_DEVICEARRIVAL)
             {
                if(db->dbcc_classguid == usbDSPGUID)
                {
                    SendMessageToGUI("USBChanged,DSP_Connected",-1); 
                }
                else if(db->dbcc_classguid == usbFX3GUID)
                {
                   char* name = db->dbcc_name;
                   char* pid = strstr(name, "PID_");
                   if (pid && !strncmp(pid, "PID_AAC1", 8)) // Spinsolve USB Product ID
                   {
                      SendMessageToGUI("USBChanged,FX3_Connected", -1);
                   }
                   else if (pid && !strncmp(pid, "PID_AAC2", 8)) // Kea USB Product ID
                   {
                      SendMessageToGUI("USBChanged,FX3_Kea_Connected", -1);
                   }
                   else
                   {
                      return(FALSE); 
                   }
                }
             }
             else if(wParam == DBT_DEVICEREMOVECOMPLETE)
             {
                if(db->dbcc_classguid == usbDSPGUID)
                {
                    SendMessageToGUI("USBChanged,DSP_Disconnected",-1); 
                }
                else if(db->dbcc_classguid == usbFX3GUID)
                {
                   char* name = db->dbcc_name;
                   char* pid = strstr(name, "PID_");
                   if (pid && !strncmp(pid, "PID_AAC1", 8)) // Spinsolve USB Product ID
                   {
                      SendMessageToGUI("USBChanged,FX3_Disconnected", -1);
                   }
                   else if (pid && !strncmp(pid, "PID_AAC2", 8)) // Kea USB Product ID
                   {
                      SendMessageToGUI("USBChanged,FX3_Kea_Disconnected", -1);
                   }
                   else
                   {
                      return(FALSE);
                   }
                }
             }
             break;
          }
          break;
       }


      case(WM_DROPFILES):
      {
         CText path,file,ext;

			int objNr = WhichObjectIsMouseOver(win);
        // if (objNr == -1)
        //    return(0);

         HDROP hDrop = (HDROP)wParam;

         int nrDropped = GetNumberOfDroppedFiles(hDrop);

         for(int i = 0; i < nrDropped; i++)
         {
            if(GetDropFileInfo(hDrop,path,file,ext,i) == OK)
            {
               SetForegroundWindow(prospaWin);
               SetFocus(hWnd);
               char proc[50];
               char spath[MAX_PATH];
               strcpy(spath,path.Str());
               ReplaceSpecialCharacters(spath,"\\","\\\\",MAX_PATH);
               sprintf(proc,"window(%d)",win->nr);
               CText cmd;
               cmd.Format("%s(\"%s\",\"%s\",\"%s\",\"%d\")",win->dragNDropProc.Str(),spath,file.Str(),ext.Str(),objNr);
               ProcessMacroStr(LOCAL,win,NULL,cmd.Str(),"",proc, win->macroName, win->macroPath);
            }
            else
               return(0);
         }
         DragFinish(hDrop);
         return(0);
      }

      case(WM_USER_LOADDATA): // Load data when Prospa already running
      { 
         LoadUserData(wParam);
         ShowWindow(prospaWin,SW_RESTORE);
         SetFocus(prospaWin);
         break;
      }

      case(WM_MOUSEACTIVATE):
      {
         if(win && !win->keepInFront)
         {
            ChangeGUIParent(win->hWnd);
         }
         break;
      }

   // Intercept maximise event
      case(WM_GETMINMAXINFO):
      {
         if(win && win->constrained)
         {
            LPMINMAXINFO mmi = (LPMINMAXINFO) lParam; // address of structure 
            long x,y,w,h;
            GetMaximisedSize(hWnd,x,y,w,h,true);
            mmi->ptMaxSize.x = w;
            mmi->ptMaxSize.y = h;
            mmi->ptMaxPosition.x = x;
            mmi->ptMaxPosition.y = y;

            return(0);
         }
         break;
      }

   // Redraw window background when damaged
		case(WM_PAINT): 
		{
		   PAINTSTRUCT p;
         HBRUSH hBrush;
         HRGN hRgn;
         HDC hdc;
         RECT r;
         if(win)
         {
            hRgn = win->CreateBgrdRegion(); // Calculate the background region
          
            if(wParam == 0)
            {
               hdc = BeginPaint(hWnd, &p ); // Paint the background
               if(win->bkgColor & 255<<24)
                  hBrush = (HBRUSH)CreateSolidBrush(gMainWindowColor);
   	       // hBrush = (HBRUSH)CreateSolidBrush(RGB(255,0,0));
               else
   	            hBrush = (HBRUSH)CreateSolidBrush(win->bkgColor);
             //  CStopWatch sw;
             //  sw.startTimer();
               
            //    COLORREF color_array[] = {RGB(255,0,0),RGB(255,0,0),RGB(0,255,0),RGB(0,255,0)};

              // GradientFill(hdc, hRgn, color_array);
            //   TextMessage("Filling\n");
               FillRgn(hdc,hRgn,hBrush);
             //  sw.stopTimer();
             //  double tm = sw.getElapsedTime();
             //  Beep(1000,0.1);
             //  TextMessage("Time elapsed = %g\r",(float)tm);
               DeleteObject(hBrush);
            }
            else 
            {
               GetClientRect(hWnd,&r);
               hdc = (HDC)wParam; // Allow for WM_PRINTCLIENT call
               if(win->bkgColor & 255<<24)
                  hBrush = (HBRUSH)CreateSolidBrush(gMainWindowColor);
               else
   	            hBrush = (HBRUSH)CreateSolidBrush(win->bkgColor);
               FillRect(hdc,&r,hBrush);
               DeleteObject(hBrush);
            }

         // Draw a grid on the window
            if(win->showGrid && !win->activated)
            {
               ExtSelectClipRgn(hdc,hRgn,RGN_AND); // Stops grid from flashing inside controls as they are moved
               HPEN plotPen = CreatePen(PS_SOLID,0,gDesignerGridColor);
               HPEN oldPen = (HPEN)SelectObject(hdc,plotPen);
               int s;
               int width = win->ww;
               int height = win->wh;
               int spacing = win->gridSpacing;
               for(s = spacing; s < width; s+= spacing)
               {
                  MoveToEx(hdc,s,0,NULL);
                  LineTo(hdc,s,height-1);
               }
               for(s = spacing; s < height; s+= spacing)
               {
                  MoveToEx(hdc,0,s,NULL);
                  LineTo(hdc,width-1,s);
               }
               SelectObject(hdc,oldPen);
               DeleteObject(plotPen);
               SelectClipRgn(hdc,NULL);
            }

            if(wParam == 0) // Allow for WM_PRINTCLIENT call
		         EndPaint(hWnd, &p );	// stop painting  

            DeleteObject(hRgn);
         }
		   return(0);	 	   
      } 

   // Draw the window to a specific device context
		case(WM_PRINTCLIENT): 
		{
			if(gWMPrintCalled)
            SendMessage(hWnd, WM_PAINT, wParam, lParam);
         break;
      }

	   case(WM_ERASEBKGND): // Prevent background being cleared before resize or repainting
	   {
	      return(0);
	   }

   // Control the cursor when moving the mouse
      case(WM_SETCURSOR): 
      {
			extern bool inEditDivider;
		//	static int cnt = 0;
         if(win && !win->activated && interactiveMode == MOVING_OVER_OBJECT) // Stops arrow cursor appearing when displaying 
            return(1);                       // other cursors and moving the mouse

         POINT pt;
         GetCursorPos(&pt);
         ScreenToClient(hWnd,&pt);
         if(win)
         {
            obj = win->FindObject(pt.x,pt.y);
            if(obj && (obj->type == PLOTWINDOW || obj->type == IMAGEWINDOW)) // || obj->type == LISTBOX)) // Let plot window handle cursor
               return(1);

            if(obj && (obj->type == TEXTEDITOR) && inEditDivider) // Don't update the cursor if in an editor divider otherwise it flickers
               return(1);

            if(obj && (obj->type == TEXTEDITOR || obj->type == TEXTBOX ||
                       obj->type == TEXTMENU || obj->type == CLIWINDOW  ||
                       obj->type == DIVIDER)) // Let native window handle cursor
				{
               return(0);
				}
         }
			// This was causing cursor flicker in text boxes when inside a panel. Why was this necessary?
       //  gResetCursor = LoadCursor(NULL,IDC_ARROW);
		//	TextMessage("Arrow %d\n",cnt++);
       //  SetCursor(gResetCursor); // Reset the cursor if mouse moves
         break;
      } 

   // Left button has been double clicked
      case(WM_LBUTTONDBLCLK): 
      {
         if(win)
         {
            if(win->modifyingCtrlNrs || win->modifyingTabNrs) // Ignore mouse movements if modifying numbers
               break;

            if(!win->activated) // Make sure window is in edit mode
            {
               char cmd[MAX_STR];
               x = LOWORD(lParam);
               y = HIWORD(lParam);
               obj = win->FindObject(x,y); 
               activeEditWin = false;
               if(obj) // Modify the object
               {
                  currentObj = obj;
                  interactiveMode = NOTHING;
                  sprintf(cmd,"enterObjectInfo(%d,%d)",win->nr,obj->nr());
                  ProcessMacroStr(1,win,obj,cmd,"","","enterObjectInfo.mac","");
   	         }
               else // Modify the title if clicked on background
               {
                  sprintf(cmd,"setGuiTitle(%d)",win->nr);
                  ProcessMacroStr(1,win,obj,cmd,"","","setGuiTitle.mac","");
               }
               activeEditWin = true;
            }
			   else if((wParam & MK_SHIFT)) // Double clicked on background with shift 
			   {                          // down, loads macro into editor
               char pathBak[MAX_PATH];
               char nameBak[MAX_PATH];
               bool alreadyLoaded = false;

              if(wParam & MK_CONTROL) // Special case for UCS macros - open the pp compiler
              {
                 char fileName[MAX_PATH];
                 char temp[MAX_PATH];
                 strcpy(temp,win->macroPath);
                 long sz = strlen(win->macroPath);
                 if(temp[sz-1] == '\\')
                    temp[sz-1] = '\0';
                 strcpy(fileName,win->macroName);
                 RemoveExtension(fileName);
                 ReplaceSpecialCharacters(temp,"\\","\\\\",sizeof(temp));
                 CText ppPath = temp;
                 CText ppName = fileName;
                 CText  fullPath = ppPath + "\\" + ppName + "\\" + ppName + "_pp.mac";
                 if(IsFile(fullPath.Str()))
                 {
                    CText cmd;
                    cmd.Format("MakeUCSPulseProgram(\"%s\",\"%s\")",ppPath.Str(),ppName.Str());
                    ProcessMacroStr(1,NULL,NULL,cmd.Str(),"","","MakeUCSPulseProgram.mac","");
                    SetCursor(LoadCursor(NULL,IDC_ARROW));
                    break;
                 }
               }

               if(curEditor)
               {
                  short nr = curEditor->regNr;
                  if(curEditor->CheckForUnsavedEdits(nr) == IDCANCEL)
                        break;
                  if(win->keepInFront && !curEditor->edParent->parent->winParent->keepInFront) // Will cause a crash if not here!
                       ChangeGUIParent(curEditor->edWin);
			         SetCursor(LoadCursor(NULL,IDC_WAIT));

               // Make a backup of current editor file
                  strncpy_s(pathBak,MAX_PATH,curEditor->edPath,_TRUNCATE);
                  strncpy_s(nameBak,MAX_PATH,curEditor->edName,_TRUNCATE);

               // Set the editor file
                  strncpy_s(curEditor->edPath,MAX_PATH,win->macroPath,_TRUNCATE);
                  strncpy_s(curEditor->edName,MAX_PATH,win->macroName,_TRUNCATE);

               // Check to see if the file is loaded already
                  if(IsEditFileAlreadyLoaded(curEditor) >= 0) // (2.2.7)
                  {
                     alreadyLoaded = true;
                     if(YesNoDialog(MB_ICONWARNING,0,"Warning","File already loaded\rDo you want to load a read-only copy?") == IDNO)
                     {
                        strncpy_s(curEditor->edPath,MAX_PATH,pathBak,_TRUNCATE);
                        strncpy_s(curEditor->edName,MAX_PATH,nameBak,_TRUNCATE);
                        break;
                     }
                  }

                 LoadEditorGivenPath(win->macroPath,win->macroName);
                 
              // Check to see if file already loaded
                  if(alreadyLoaded) // (2.2.7)
                     curEditor->readOnly = true;
                  else
                     curEditor->readOnly = false;
                  SetEditTitle(); 
                  AddFilenameToList(procLoadList,win->macroPath,win->macroName);
				      SetCursor(LoadCursor(NULL,IDC_ARROW));
               }
			   }
         }
			break;
      }

   // Make sure accelerator table is correct
      case(WM_ACTIVATE): 
      {
         if(win)
         {
            if(LOWORD(wParam) != WA_INACTIVE)
			   {
               currentAppWindow = hWnd;

               if(win->hAccelTable && win->activated)
               {
	               currentAccel = win->hAccelTable;
               }
               else
               {
                  currentAccel = hAccelUser;
               }
				   WinData::SetGUIWin(GetWinDataClass(hWnd));
   
              //  TextMessage("Window %hd activated\n",win->nr);
               DefWindowProc( hWnd, messg, wParam, lParam ); // Force gui window to get focus
               SetFocus(hWnd);
               return(0);
            }
            else
            {// Make sure rectangle is removed - unless editing
               if(win->activated && GetGUIWin())
               {
                  GetGUIWin()->objWithFocus = NULL;
                  GetGUIWin()->RemoveFocusRectangle();
               }
            }
         }
         break;
      }
   
   // Left button has been pressed
      case(WM_LBUTTONDOWN): 
      {
       //  record = true;
         if(win)
            ProcessLeftButtonDown(lParam, wParam, hWnd, win);
         break;
      }

   // Left button has been release
      case(WM_LBUTTONUP): 
      {
         if(win)
            ProcessLeftButtonUp(lParam, wParam, hWnd, win);
         break;
      }
     
   // Cursor is moving - simply allow drawing of selection rectangle and change cursor as cursor moves over controls
      case(WM_MOUSEMOVE):
      {   
         if(win &&
            interactiveMode != RESIZE && 
            !win->activated && 
            !win->modifyingCtrlNrs &&
            !win->modifyingTabNrs)
         {
            ProcessMouseMove(lParam, wParam, hWnd, win);
         }
         else // Lose mouse focus if outside object
         {
            HWND hWnd = GetFocus();
            if(!IsMouseInWindow(hWnd))
            {
               ObjectData *obj = win->widgets.findByWin(hWnd); // Find object selected
               if(obj && obj->type == SLIDER)  // Only for slider at the moment
                  SetFocus(NULL);
            }
         }
         break;
      }

    // Display a contextual menu for the GUI window  
      case(WM_CONTEXTMENU):
      {
         if(win)
            ProcessContextMenu(lParam, wParam, hWnd, win);
         break;
      }

   // Keyboard has been pressed (virtual key code)
      case(WM_KEYDOWN): 
      {
         if(win)
         {
            if(ProcessKeyDown(lParam, wParam, hWnd, win))
               return(1);   
         }
         break;
      }

   // Keyboard has been pressed (raw characters)
      case(WM_CHAR):
      { 
         if(win)
            ProcessCharacter(lParam, wParam, hWnd, win);            
         break;
      }      
    
   // Process button or menu events
		case(WM_COMMAND): 
		{
	      if(win)
         {
		      return(PreProcessCommand(lParam, wParam, hWnd, win));
         }
         break;
      }
     
   // Process track-bar movement
      case(WM_HSCROLL):
      case(WM_VSCROLL):
      {
         if(ProcessScrollEvents(lParam, wParam, hWnd, win))
            return(1);
			return(0);
      }

   // Window has been moved
		case(WM_MOVE): 
		{    
         RECT r;
		   if(win) // Resize any status region
		   {
            if(win->isMainWindow)
            {
               ResizeWindowsToParent();
  
               GetWindowRect(prospaWin,&r);
               long pw = r.right - r.left;
               long ph =  r.bottom - r.top;
               long py = r.top;
               long px = r.left;

               long ww = GetSystemMetrics(SM_CXFULLSCREEN);
               long wh = GetSystemMetrics(SM_CYFULLSCREEN);
               SystemParametersInfo(SPI_GETWORKAREA,0,&r,0);

               if(!prospaResizing && !IsIconic(prospaWin) && !IsZoomed(prospaWin) && !IsIconic(hWnd) && !IsZoomed(hWnd))
               {
                  win->wSzScale = pw/(float)ww;
                  win->hSzScale = ph/(float)wh;            
                  win->xSzScale = px/(float)ww;
                  win->ySzScale = py/(float)wh;
                  win->xSzOffset = 0;
                  win->ySzOffset = 0;
                  win->wSzOffset = 0;
                  win->hSzOffset = 0;
               }
            }
            else
            {
               RECT r;

               GetWindowRect(prospaWin,&r);
               long pw = r.right - r.left - 2*resizableWinBorderSize;
               long ph =  r.bottom - r.top - (resizableWinBorderSize + titleBarNMenuHeight);
               long py = r.top + titleBarNMenuHeight;
               long px = r.left+resizableWinBorderSize;

               GetWindowRect(hWnd,&r);

               long x = r.left-px;
               long y = r.top-py;

               if(!prospaResizing && !IsIconic(prospaWin) && !IsZoomed(prospaWin) && !IsIconic(hWnd) && !IsZoomed(hWnd))
               {
                  win->xSzScale = x/(float)pw;
                  win->ySzScale = y/(float)ph; 
               }
            }
         }
         break;
      }


   // Window has been resized
		case(WM_SIZE): 
		{    
		   if(win) // Resize any status region
		   {
            RECT r;

            if(win->isMainWindow)
            {
               if(win->toolbar)
                 SendMessage(win->toolbar->hWnd,WM_SIZE,0,0);
               if(win->statusbox)
                 SendMessage(win->statusbox->hWnd,WM_SIZE,0,0);
               SendMessage(win->blankToolBar,WM_SIZE,0,0);
               SendMessage(win->blankStatusBar,WM_SIZE,0,0);

               ResizeWindowsToParent();
               ResizeObjectsWhenParentResizes(win,NULL,NULL,0,0);
               GetWindowRect(prospaWin,&r);
               long pw = r.right - r.left;
               long ph =  r.bottom - r.top;
               long py = r.top;
               long px = r.left;

               long ww = GetSystemMetrics(SM_CXFULLSCREEN);
               long wh = GetSystemMetrics(SM_CYFULLSCREEN);

               if(!prospaResizing && !IsIconic(prospaWin) && !IsZoomed(prospaWin) && !IsIconic(hWnd) && !IsZoomed(hWnd))
               {
                  win->wSzScale = pw/(float)ww;
                  win->hSzScale = ph/(float)wh;            
                  win->xSzScale = px/(float)ww;
                  win->ySzScale = py/(float)wh;
                  win->xSzOffset = 0;
                  win->ySzOffset = 0;
                  win->wSzOffset = 0;
                  win->hSzOffset = 0;
               }
            }
            else
            {
               GetWindowRect(prospaWin,&r);
               long pw = r.right - r.left - 2*resizableWinBorderSize;
               long ph =  r.bottom - r.top - (resizableWinBorderSize + titleBarNMenuHeight);
               long py = r.top + titleBarNMenuHeight;
               long px = r.left+resizableWinBorderSize;

	            win->ww = LOWORD(lParam);
	            win->wh = HIWORD(lParam);

            // Need to do this to ensure toolbar height is correctly calculated
               if(win->toolbar)
                 SendMessage(win->toolbar->hWnd,WM_SIZE,0,0);
               SendMessage(win->blankToolBar,WM_SIZE,0,0);
               SendMessage(win->blankStatusBar,WM_SIZE,0,0);

            // Recalculate all objects sizes within windows
               ResizeObjectsWhenParentResizes(win,NULL,NULL,0,0);

               GetWindowRect(hWnd,&r);
               long ww = r.right-r.left;
               long wh = r.bottom-r.top;
               long x = r.left-px;
               long y = r.top-py;

               if(!prospaResizing && !IsIconic(prospaWin) && !IsZoomed(prospaWin) && !IsIconic(hWnd) && !IsZoomed(hWnd))
               {
                  win->wSzScale = ww/(float)pw;
                  win->hSzScale = wh/(float)ph;            
                  win->xSzScale = x/(float)pw;
                  win->ySzScale = y/(float)ph; 
               }
            }
         }
         return(0);
      }

   // User is resizing window
		case(WM_SIZING): 
		{   
		   if(win) // Give feedback to user
		   {

            // Limit resizing rectangle if required
            RECT *r = (RECT*)lParam;

            long dx = 0; //2*resizableWinBorderSize;
            long dy = 0; //titleBarHeight;
 
            if(win->sizeLimits.minWidth > 0)
            {
          //     TextMessage("Min before r->right = %ld\n",r->right);
               if(r->right-r->left-dx < win->sizeLimits.minWidth)
                  r->right = r->left + dx + win->sizeLimits.minWidth;
         //      TextMessage("Min after r->right = %ld\n",r->right);
         //      TextMessage("Min after r->left = %ld\n",r->left);
            }

            if(win->sizeLimits.maxWidth > 0)
            {
         //      TextMessage("Max before r->right = %ld\n",r->right);

               if(r->right-r->left-dx > win->sizeLimits.maxWidth)
                  r->right = r->left+ dx + win->sizeLimits.maxWidth;
        //       TextMessage("Max after r->right = %ld\n",r->right);
         //      TextMessage("Max after r->left = %ld\n",r->left);


            }

            if(win->sizeLimits.minHeight > 0)
            {
               if(r->bottom-r->top-dy < win->sizeLimits.minHeight)
                  r->bottom = r->top + dy + win->sizeLimits.minHeight;
            }

            if(win->sizeLimits.maxHeight > 0)
            {
               if(r->bottom-r->top-dy > win->sizeLimits.maxHeight)
                  r->bottom = r->top + dy + win->sizeLimits.maxHeight;
            }

            if(!win->activated)
            {
               RECT* r = (LPRECT) lParam;
               CText txt;
		         win->ww = r->right-r->left;
		         win->wh = r->bottom-r->top;
               txt.Format("w = %hd h = %hd",win->ww,win->wh);
               SetWindowText(hWnd,txt.Str());
               SetTimer(hWnd,1,2000,NULL);
            }
            else
            {
               if(win->isMainWindow)
                  ResizeWindowsToParent();
               else if(win->constrained)
                  LimitSizingRect(hWnd, wParam, lParam);
               break;
            }
         }
         break;
      }

      case(WM_CTLCOLORSTATIC):
      {
         long r = DefWindowProc(hWnd, messg, wParam, lParam );
         HDC hdc = (HDC) wParam;   // handle of display context 
         HWND hWnd = (HWND) lParam; // handle of static control 

         ObjectData *obj = win->widgets.findByWin(hWnd);
         if(obj)
         {
            if(obj->bgColor & 255<<24)
	            SetBkColor(hdc,GetSysColor(COLOR_BTNFACE));
            else
               SetBkColor(hdc,obj->bgColor);
            if(obj->fgColor & 255<<24)
	            SetTextColor(hdc,GetSysColor(COLOR_BTNFACE));
            else
               SetTextColor(hdc,obj->fgColor);
         }
         return(r);
      }
      
      case(WM_CTLCOLOREDIT):
      {
         long r = DefWindowProc(hWnd, messg, wParam, lParam );
         HDC hdc = (HDC) wParam;   // handle of display context 
         HWND hWnd = (HWND) lParam; // handle of static control 

         ObjectData *obj = win->widgets.findByWin(hWnd);
         if(obj)
         {
            if (obj->fgColor & 255 << 24)
               SetTextColor(hdc, RGB(0, 0, 0));
            else
               SetTextColor(hdc, obj->fgColor);

            if(obj->bgColor & 255<<24)
	            SetBkColor(hdc,RGB(255,255,255));
            else
            {
               SetBkColor(hdc, obj->bgColor);
               return (LRESULT)GetStockObject(GRAY_BRUSH); // Test of changing background in textboxes.
            }
         }
         return(r);
      }

      case(WM_MOVING):
      {
         if(win->isMainWindow)
            ResizeWindowsToParent();
         else if(win->constrained)
            LimitMovingRect(hWnd, wParam, lParam);
         break;
      }

   // Restore title
      case(WM_TIMER):
      {  
         if(win)
         {
            SetWindowText(hWnd,win->title);
            KillTimer(hWnd,timerID);
         }
         break;
      }

   // Close  window
		case(WM_CLOSE): 
		{
         if(win)
         {
         // Don't delete permanent windows
            if(win->permanent)
            {
               ShowWindow(win->hWnd,SW_HIDE);
               return(0);
            }
		      if((win->type == DIALOG_WIN && dialogDisplayed) || win->hWnd == prospaWin) // Exit from dialog loop if close button pressed
		      {
               dialogDisplayed = false;
               return(0);
            }
            if(!win->keepInFront) // Reassign the parent window if not a front level gui window
            {
 	            HWND hWnd2 = GetNextWindow(win->hWnd,GW_HWNDNEXT); 
               delete win;
               win = NULL; 
               ChangeGUIParent(hWnd2);
            }
            else
            {
               if(win->debugger)
               {
                  DebuggerClose();
               }
               if(win->debugging && gDebug.win)
               {
                  DebuggeeClose();
               }
               delete win;
               win = NULL;
            }
         }
         RestoreCurrentWindows(hWnd);

    //TODO??     itfc->win = NULL;
         if(hWnd != prospaWin)
         {
            DestroyWindow(hWnd);
		      BringWindowToTop(prospaWin);
         }
         else
            DestroyWindow(hWnd);

		   return(0);	
		}

      case(WM_NCLBUTTONDOWN):
      {

         if(wParam == HTCAPTION)
         {
             SelectFixedControls(win);
             if(win->isMainWindow && win->titleUpdate)
             {
                CText title;
                GetVersionTxt(title);
                title = "Prospa V" + title;
                SetWindowText(win->hWnd,title.Str());
             }
         }
         break;
      }


   // User has closed window using system menu in window
		case(WM_SYSCOMMAND):
		{
         if((wParam & 0xFFF0) == SC_RESTORE)
         {
            win->restoreFromMaximised();
         }

         if ((wParam & 0xFFF0) == SC_MAXIMIZE) // On some monitors maximising will cause window to vanish - this fixes it
         {
            if (win->hWnd != prospaWin)
            {
               DefWindowProc(hWnd, messg, wParam, lParam);
               ResizeWindowsToParent();
               return(0);
            }
         }

         if((wParam & 0xFFF0) == SC_MINIMIZE)
         {
            win->visible = false;

				ChangeGUIParent(prospaWin); // Added to ensure all windows are minimized.

            if(win->hWnd != prospaWin)
            {
               ShowWindow(win->hWnd,SW_HIDE);
               return(0);
            }
         }

		   if(wParam == SC_CLOSE)
		   {
            win->visible = false;
            if(gDebug.enabled && win->debugging)
            {
               MessageDialog(prospaWin,MB_ICONERROR,"Error","Can't close a window while\rit is being debugged.");
               return(0);
            }

		      if(dialogDisplayed && win->type == DIALOG_WIN) // Exit from dialog loop if close button pressed
		      {
               Interface itfc;
               CloseDialog(&itfc,"");
               return(0);
            }

         // Run the exit procedure
		      if(win && win->exitProcName[0] != '\0')
		      {
               Interface itfc;
		         CText proc;
            //   if(win->hWnd == prospaWin)
            //      ShowWindow(prospaWin,SW_RESTORE);
		         SetFocus(hWnd);
               proc.Format("exit_procedure(%d)",win->nr);
               itfc.procName = proc.Str();
               itfc.win = win;
               itfc.varScope = LOCAL;
		         if(ProcessMacroStr(&itfc ,win->exitProcName, win->macroName, win->macroPath) == ERR)
						return(0);
     
          // If user returns with "cancel" as the argument then don't close the window
               if(itfc.win) // Make sure the window is still present
               {
                  if(!itfc.win->permanent && itfc.nrRetValues == 1)
                  {
                     if(itfc.retVar[1].GetType() == UNQUOTED_STRING &&
                        itfc.retVar[1].GetString() &&
                        !strcmp(itfc.retVar[1].GetString(),"cancel"))
                        return(0);
                  }
               }
          // Make sure all threads are closed before closing window
               if(itfc.win && !itfc.win->isMainWindow && itfc.win->threadCnt > 0)
               {
                  MessageDialog(prospaWin,MB_ICONERROR,"Error","Can't close a window while a macro\rstarted from that window is running.");
                  return(0);
               }

		      }
         }
		   break;
		}

   // Set the size of the list box lines
      case(WM_MEASUREITEM): 
      {
         LPMEASUREITEMSTRUCT lpmis; 
         ObjectData *obj = win->widgets.findByNr(LOWORD(wParam));

         if(obj && obj->type == LISTBOX)
         {
            lpmis = (LPMEASUREITEMSTRUCT) lParam; 
 
            /* Set the height of the list box items. */ 
 
            lpmis->itemHeight = 16; 
            return TRUE; 
         }
         break;
      }
  // Draw the list box, color box and group box
      case(WM_DRAWITEM):
      {
         if(win)
         {
            LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;
            HWND hwnd = lpdis->hwndItem;
            ObjectData *obj = win->widgets.findByWin(hwnd);

          //  ObjectData *obj = win->widgets.findByNr(LOWORD(wParam));
            if(obj && obj->type == COLORBOX)
            {
               BYTE *data = (BYTE*)obj->data;
               LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT) lParam; 
               if(data[3] == 0xFF)
               {
                  HBRUSH hBrush = (HBRUSH)CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
                  FillRect(pdis->hDC, &pdis->rcItem, hBrush);
                  DeleteObject(hBrush);
               }
               else
               {
                  HBRUSH hBrush = (HBRUSH)CreateSolidBrush(RGB(data[0],data[1],data[2]));
                  FillRect(pdis->hDC, &pdis->rcItem, hBrush);
                  DeleteObject(hBrush);
               }
               FrameRect(pdis->hDC, &pdis->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
            }
            else if(obj && obj->type == LISTBOX)
            {
               return(DrawListBoxItem(obj, (LPDRAWITEMSTRUCT)lParam));
            }
            else if(obj && obj->type == GROUP_BOX)
            {
               return obj->DrawGroupBox((LPDRAWITEMSTRUCT)lParam);
            }
            else if(obj && obj->type == BUTTON && win7Mode)
            {
               return(DrawButton(obj, (LPDRAWITEMSTRUCT)lParam));
            }
            else if(obj && obj->type == PICTURE)
            {
               LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT) lParam; 
               HDC hdc = (HDC)pdis->hDC;
               RECT r = pdis->rcItem;

               if(obj->data)
               {
                  PictureInfo *info = (PictureInfo*)obj->data;
                  HBITMAP hBitmap = info->bmp;
                  BITMAP bitMap;

                  GetObject(hBitmap,sizeof(BITMAP),&bitMap);
                  long w = bitMap.bmWidth;
                  long h = bitMap.bmHeight;
   
                  HDC memDC = CreateCompatibleDC(hdc);
                  SelectObject(memDC,hBitmap);
                  StretchBlt(hdc,0,0,obj->wo,obj->ho,memDC,0,0,w,h,SRCCOPY);
                  DeleteDC(memDC);
               }
               if(obj->showFrame)
                   FrameRect(hdc, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
            }
            else if(obj && obj->type == DEBUGSTRIP)
            {
               RECT r;
               SetRect(&r,0,0,obj->wo,obj->ho);

               DebugStripInfo *info = (DebugStripInfo*)obj->data;

               HDC hdc = GetDC(obj->hWnd);

               if(obj->data)
               {
                  HBITMAP hBitmap = (HBITMAP)info->hBitmap;
                  BITMAP bitMap;
                  GetObject(hBitmap,sizeof(BITMAP),&bitMap);
                  long w = bitMap.bmWidth;
                  long h = bitMap.bmHeight;
                  HDC memDC = CreateCompatibleDC(hdc);
                  SelectObject(memDC,hBitmap);
                  StretchBlt(hdc,0,0,obj->wo,obj->ho,memDC,0,0,w,h,SRCCOPY);
                  DeleteDC(memDC);
               }
             //   FrameRect(hdc, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
                ReleaseDC(obj->hWnd,hdc);
            }
            else if(obj && obj->type == TABCTRL)
            {
               LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam; // item drawing information

               if (obj->hWnd == lpdis->hwndItem)   // is this the tab control?
               {
                  PaintTab(obj,lpdis);
                  return(true);
               }
               break;
            }

         }
      }
      case(WM_INITMENUPOPUP):
      {
			const int MENUNAME_LENGTH = 50;
         char menuName[MENUNAME_LENGTH];

         if(win)
         {

            HMENU winMenu = GetMenu(hWnd);

            if(winMenu)
            {
               short whichMenu = LOWORD(lParam);
               short nrMenus = GetMenuItemCount(winMenu); 
               CText folder;
               CText saveFolder;
               int i;

            // See if its a window with a fixed menu (not object based)
            // Only these can have the full dynamic menu list
               if(!win->menuObj) 
               {
                  if(win->menuListSize > 0 && win->menuList)
                  {
                     short nrBefore = -1,nrAfter = -1;

                     for(i = 0; i < win->menuListSize; i++)
                     {
                        if(win->menuList[i] < 0) 
                        {
                           nrBefore = i;
                           break;
                        }
                     }

                     for(i = win->menuListSize-1; i >= 0; i--)
                     {
                        if(win->menuList[i] == -1) 
                        {
                           nrAfter = win->menuListSize-i;
                           break;
                        }
                     }

                  // Is it a dynamic menu?
                     if(nrBefore >= 0 || nrAfter >= 0)
                     {
                        if(whichMenu >= nrBefore && whichMenu <= nrMenus-nrAfter)
                        {
                           UINT menuID = (whichMenu-nrBefore)*100+MAIN_UD_MENUS_START; // Each menu ID starts at 20000,20100,20200 ...            
                           HMENU winMenu = GetMenu(prospaWin);
                           GetMenuString(winMenu,whichMenu,menuName,MENUNAME_LENGTH,MF_BYPOSITION);
                              
                        // Remove all menu items first         
                           while(DeleteMenu((HMENU) wParam,0,MF_BYPOSITION) != 0){;}

                        // Get folder for this menu
                           short index = (whichMenu-nrBefore)*2+1;
                           folder = VarList(userMenusVar)[index]; 
                           Interface itfc; 
                           ReplaceVarInString(&itfc,folder); 

                           GetDirectory(saveFolder);
                           SetDirectory(folder);

                           bool found = false;
                           if(IsFile("macrohelp.mac"))
                           {
                         //     RemoveCharacter(menuName,'&');
                              strncpy_s(menuName,MENUNAME_LENGTH,"Macro Help",_TRUNCATE);
                              AppendMenu((HMENU)wParam,MF_STRING,menuID++,menuName);
                              found = true;
                           }

                           if(IsFile("addmacros.mac"))
                           {
                              strncpy_s(menuName,MENUNAME_LENGTH,"Add/Remove Macros",_TRUNCATE); 
                              AppendMenu((HMENU)wParam,MF_STRING,menuID++,menuName);
                              found = true;
                           }

                           if(found)
                              AppendMenu((HMENU)wParam,MF_SEPARATOR,0,"");
                                                
                        // Add the names of each macro in this folder
                           AddMenuMacros((HMENU)wParam,menuID);
                           SetDirectory(saveFolder);
                           return(1);
                        }
                     }
                  }
               }
               

            // Find the object this menu is attached to
               obj = win->widgets.findByMenu((HMENU)wParam);

               if(!obj || !obj->data) 
                  return(1);

               unsigned short seqNr = obj->seqMenuNr;

               MenuInfo *info = (MenuInfo*)obj->data;

               if(!info->cmd) return(1);

               for(i = 0 ; i < info->nrItems; i++)
               {
                  if(!strcmp(info->label[i],"\"user menu\"") || !strcmp(info->label[i],"\"user macro menu\""))
                  {
                     folder = info->cmd[i];
                     Interface itfc;
							itfc.win = win;
                     ReplaceVarInString(&itfc,folder);
                     folder.RemoveQuotes();
							
                     if(!IsDirectory(folder.Str()))
                       return(1);

                     UINT menuID = seqNr*100+i+OBJECT_UD_MENUS_START;            
                     GetMenuString(winMenu,whichMenu,menuName,MENUNAME_LENGTH,MF_BYPOSITION);
                        
                  // Remove all menu items first         
                     while(DeleteMenu((HMENU) wParam,i,MF_BYPOSITION) != 0){;}

                  // Get the name of the folder which contains macros to display in this menu
                     GetDirectory(saveFolder);                   
                     SetDirectory(folder);

                     if (IsFile("information.mac"))
                     {
                        strncpy_s(menuName, MENUNAME_LENGTH, "Information", _TRUNCATE);
                        AppendMenu((HMENU)wParam, MF_STRING, menuID++, menuName);
                        AppendMenu((HMENU)wParam, MF_SEPARATOR, 0, "");
                     }

                  // Add the names of each macro in this folder
                     AddMenuMacros((HMENU)wParam,menuID);

						   if(!obj->enable)
						   {
						      int nrItems = GetMenuItemCount((HMENU)wParam);
							   for(int k = 0; k < nrItems; k++)
							   {
								   //if(enable)
								   //	EnableMenuItem((HMENU)wParam,k,MF_BYPOSITION | MF_ENABLED);
								   //else
									   EnableMenuItem((HMENU)wParam,k,MF_BYPOSITION | MF_GRAYED);
							   }				
						   }
                     SetDirectory(saveFolder);
                     return(0);
                  }
                  else if(!strcmp(info->label[i],"\"user folder menu\""))
                  {
                     folder = info->cmd[i];
                     Interface itfc;
							itfc.win = win;
                     ReplaceVarInString(&itfc,folder);
                     folder.RemoveQuotes();
							
                  //   if(!IsDirectory(folder.Str()))
                  //     return(1);

                     UINT menuID = seqNr*100+i+OBJECT_UD_MENUS_START;            
                     GetMenuString(winMenu,whichMenu,menuName,MENUNAME_LENGTH,MF_BYPOSITION);
                        
                  // Remove all menu items first         
                     while(DeleteMenu((HMENU) wParam,i,MF_BYPOSITION) != 0){;}

                  // Get the name of the folder which contains macros to display in this menu
                     GetDirectory(saveFolder);                   
                     SetDirectory(folder);

                     if (IsFile("macrohelp.mac"))
                     {
                        strncpy_s(menuName, MENUNAME_LENGTH, "Macro Help", _TRUNCATE);
                        AppendMenu((HMENU)wParam, MF_STRING, menuID++, menuName);
                        AppendMenu((HMENU)wParam, MF_SEPARATOR, 0, "");
                     }
                     if (IsFile("information.mac"))
                     {
                        strncpy_s(menuName, MENUNAME_LENGTH, "Information", _TRUNCATE);
                        AppendMenu((HMENU)wParam, MF_STRING, menuID++, menuName);
                        AppendMenu((HMENU)wParam, MF_SEPARATOR, 0, "");
                     }
                  // Add the names of each macro in this folder
                     AddMenuFolders((HMENU)wParam,menuID);
                     AddMenuMacros((HMENU)wParam,menuID);

						   if(!obj->enable)
						   {
						      int nrItems = GetMenuItemCount((HMENU)wParam);
							   for(int k = 0; k < nrItems; k++)
							   {
								   //if(enable)
								   //	EnableMenuItem((HMENU)wParam,k,MF_BYPOSITION | MF_ENABLED);
								   //else
									   EnableMenuItem((HMENU)wParam,k,MF_BYPOSITION | MF_GRAYED);
							   }				
						   }
                     SetDirectory(saveFolder);
                     return(0);
                  }
                  else if(!strcmp(info->label[i],"\"windows menu\""))
                  {   
                     WinData *curwin;
                     CText name;
                     UINT cnt = WINDOW_MENU_START; 
                     
                  // Remove all existing window entries after first separator(0 refers to the number of default entries present)      
                     while(DeleteMenu((HMENU) wParam,i,MF_BYPOSITION) != 0){;}
                 
                 // Add any permanent windows
                     curwin = rootWin;
                     UINT base = cnt;
                     while((curwin = curwin->GetNextWin()) != NULL)
                     {
                        if(curwin != win && curwin->permanent)
                        {
                           if(cnt == base)
                              AppendMenu((HMENU)wParam,MF_SEPARATOR,0,""); // Add separator bar
                           GetWindowTextEx(curwin->hWnd,name);
                           AppendMenu((HMENU)wParam,MF_STRING,cnt++,name.Str());
                           if(cnt-base-1 > MAX_MENU_ITEMS)
                           {
                              TextMessage("\n   More than %d menu items\n",(int)MAX_MENU_ITEMS);
                              break;
                           }
                        }
                     }
                 // Add any non-permanent windows
                     base = cnt;
                     curwin = rootWin;
                     while((curwin = curwin->GetNextWin()) != NULL)
                     {
                        if(curwin != win && !curwin->permanent)
                        {
                           if(cnt == base)
                              AppendMenu((HMENU)wParam,MF_SEPARATOR,0,""); // Add separator bar

                           GetWindowTextEx(curwin->hWnd,name);
                           AppendMenu((HMENU)wParam,MF_STRING,cnt++,name.Str());
                           if(cnt-base-1 > MAX_MENU_ITEMS)
                           {
                              TextMessage("\n   More than %d menu items\n",(int)MAX_MENU_ITEMS);
                              break;
                           }
                        }
                     }
                     return(0);
                  }
                  else if(!strcmp(info->label[i],"\"procedure menu\""))
                  { 
                   // Remove all existing menu entries
                      while(DeleteMenu((HMENU) wParam,0,MF_BYPOSITION) != 0){;}
                   // Add procedure names
                      AddProcedureNames(win,(HMENU)wParam);    
                  }
               }
            }
         }
         return(0);
      }

      case(WM_MOUSEWHEEL): //  Mouse wheel has been rotated
      {	                  //  enlarge or reduce the plot
         short zDel;
         short fwKeys;
         extern void ProcessScrollWheelEvent(ObjectData *obj, HWND hWnd, short zDel, short fwKeys);

         if (gBlockScroll)
            return(0);

			zDel =  HIWORD(wParam);    // wheel rotation
			fwKeys = LOWORD(wParam);   // modifier keys

         if(win)
         {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hWnd,&pt);
            obj = win->FindObjectIfInside(pt.x,pt.y);

            if(obj && obj->enable)
            {
               PlotWindow *pp = (PlotWindow*)obj->data;
               if((obj->type == PLOTWINDOW) || (obj->type == IMAGEWINDOW))// Only send scroll events to plotwindow
               {
						pp->ProcessScrollWheelEvents(hWnd, zDel, fwKeys);
                //  SendMessageToGUI("2D Plot,ScrollWheel",0); 
               }
               else if(obj->type == OPENGLWINDOW)
               {
                  Process3DScrollWheelEvents(hWnd, zDel, fwKeys);
               }
               else if(obj->type == PANEL)
               {
                  ProcessScrollWheelEvent(obj,hWnd, zDel, fwKeys);
               }
            }
         }
         break;
      }

      case(WM_NOTIFY): // Used for tooltips in toolbars, tabs and updown controls
      {
         if(win)
         {    
            NMHDR *pnmh = (NMHDR*) lParam;
            ObjectData *obj;


	         obj = win->widgets.findByWin((HWND)pnmh->hwndFrom);

            if(obj && obj->type == TABCTRL) // Tab control
            {	
               if(pnmh->code == TCN_SELCHANGE)
               {
                  if(win->activated)      // Make sure new tab gets focus when pressed
                     SetFocus(obj->hWnd); // Make sure new tab gets focus when pressed
                  ControlVisibleWithTabs(win,obj); // Ensure all the right controls are visible 
                  Interface itfc;
               // Initialise the macro source object
                  itfc.macroName = win->macroName;
                  itfc.macroPath = win->macroPath;
                  itfc.win = win;
                  itfc.obj = obj;
                  itfc.objID = obj->nr();
                  itfc.varScope = LOCAL;
                  itfc.startLine = obj->cmdLineNr;
		            if(obj->command[0] != '\0')
		            {
                     if(!controlProcRunning || dialogDisplayed)
                     {
								if(gDebug.mode != "off")
									itfc.debugging = true;
                        controlProcRunning = true;
                        ProcessMacroStr(&itfc,obj->command);
                        controlProcRunning = false;
								if(gDebug.mode != "off")
								{
									gDebug.mode = "runto";
									gDebug.step = true;
									curEditor->debugLine = -1;
									SendMessage(curEditor->edWin,EM_HIDESELECTION ,true,0);
								}
                     }
		            } 
               }
            }
            else if(obj && obj->type == UPDOWN) // Updown control
            {	
               if(pnmh->code == UDN_DELTAPOS)
               {
                  NMUPDOWN *ud = (NMUPDOWN*) lParam;
                  Interface itfc;
                  char *str;

               // Initialise the macro source object
                  itfc.macroName = win->macroName;
                  itfc.macroPath = win->macroPath;
                  itfc.win = win;
                  itfc.obj = obj;
                  itfc.objID = obj->nr();
                  itfc.varScope = LOCAL;
                  itfc.startLine = obj->cmdLineNr;
	               str = obj->command;

                  itfc.procName.Format("updown(%d,%d)",win->nr,obj->nr());

                  UpDownInfo *info = (UpDownInfo*)obj->data;
                  if(ud->iDelta > 0)
                  {
                     info->value += info->stepSize;
                     if(info->value > info->base+info->nrSteps*info->stepSize)
                        info->value = info->base+info->nrSteps*info->stepSize;
                  }
                  else
                  {
                     info->value -= info->stepSize;
                     if(info->value < info->base)
                       info->value = info->base;
                  }

		            str = obj->command;
		            if(str[0] != '\0') // && macroDepth == 0)
		            {
                     if(!controlProcRunning || dialogDisplayed)
                     {
								if(gDebug.mode != "off")
								{
									itfc.debugging = true;
									 SetFocus(cliEditWin);
								}
                        controlProcRunning = true;
                        ProcessMacroStr(&itfc,str);
                        controlProcRunning = false;
								if(gDebug.mode != "off")
								{
									gDebug.mode = "runto";
									gDebug.step = true;
									curEditor->debugLine = -1;
									SendMessage(curEditor->edWin,EM_HIDESELECTION ,true,0);
								}
                     }
		            } 
                  SendMessage(obj->hWnd,WM_LBUTTONUP,0,0); // Stops autorepeat
               }
            }
            else if(obj && (obj->type == BUTTON)) // Button font control
            {
                LPNMCUSTOMDRAW custDraw = reinterpret_cast<LPNMCUSTOMDRAW>(pnmh);

                PushButtonInfo* info = (PushButtonInfo*)obj->data;
                if(info->hImage != 0 || !obj->fgColor) // Ignore the following code if we are drawing an image on the button
                   return(CDRF_DODEFAULT);

                if (custDraw->dwDrawStage == CDDS_PREPAINT)
                {
                    const int textLength = GetWindowTextLength(custDraw->hdr.hwndFrom);

                    if (textLength > 0)
                    {
                      //  TCHAR* buttonText = new TCHAR[textLength+1];
                        SIZE   dimensions = {0};
                        HFONT newFont = NULL;
                        HFONT oldFont = NULL;

                      //  GetWindowText(custDraw->hdr.hwndFrom, buttonText, textLength+1);

                        if(obj->data)
                        {
                           PushButtonInfo *info = (PushButtonInfo*)obj->data;

                           if(info->fontHeight > 0 || info->italicText || info->boldText || info->fontName.Size() > 0)
                           {
                              LOGFONT lf = {0};

                               const HFONT currentFont = (HFONT)SendMessage(custDraw->hdr.hwndFrom, WM_GETFONT, 0, 0);
                               int res = GetObject(currentFont,sizeof(LOGFONT),&lf); 

                              if(info->italicText)
                                 lf.lfItalic = TRUE;

                              if(info->boldText)
                                 lf.lfWeight = FW_BOLD;

                              if(info->fontHeight > 0)
                                 lf.lfHeight = info->fontHeight;

                              if(info->fontName.Size() > 0)
                                 strcpy(lf.lfFaceName,info->fontName.Str());

                              newFont = CreateFontIndirect(&lf);

                              oldFont = (HFONT)SelectObject(custDraw->hdc,newFont);
                           }
                        }

                        DisplayButtonText(custDraw->hdc, obj, custDraw->rc, obj->fgColor);


                        //// Need to modify this to place text inside a box - may need multiple lines
                        //// Need to write a general word wrap algorithm. Didn't Mike do this for Inserts?
                        //GetTextExtentPoint32(custDraw->hdc, buttonText, textLength, &dimensions);
                        //
                        //const int xPos = (custDraw->rc.right - dimensions.cx) / 2;
                        //const int yPos = (custDraw->rc.bottom - dimensions.cy) / 2;

                        //
                        //if(obj->bgColor & 255<<24)
                        //{
                        //   SetBkMode(custDraw->hdc, TRANSPARENT);
	                       // SetBkColor(custDraw->hdc,GetSysColor(COLOR_BTNFACE));
                        //}
                        //else
                        //   SetBkColor(custDraw->hdc,obj->bgColor);

                        //if(obj->fgColor & 255<<24)
	                       // SetTextColor(custDraw->hdc,GetSysColor(COLOR_BTNFACE));
                        //else
                        //   SetTextColor(custDraw->hdc,obj->fgColor);

                        //TextOut(custDraw->hdc, xPos, yPos, buttonText, textLength);

                        //delete [] buttonText;

                        if(newFont)
                        {
                            SelectObject(custDraw->hdc,oldFont);
                            DeleteObject(newFont);
                        }

                        return(CDRF_SKIPDEFAULT);
                    }
                }
                return(CDRF_DODEFAULT);
            }

            else if(!obj) // Toolbar tip?
            {	
              LPTOOLTIPTEXT TTtext = (LPTOOLTIPTEXT)lParam;

               if(TTtext->hdr.code == TTN_NEEDTEXT)
               {
                  short id = TTtext->hdr.idFrom;

                  obj = win->toolbar;

                  if(obj)
                  {
                     ToolBarInfo *info = (ToolBarInfo*)obj->data;

                     if(id >= 0 && id < info->nrItems)
                        TTtext->lpszText = info->item[id].label.Str(); 
                  }
               }
            }
         }
         break;
      }

	}

   return(DefWindowProc(hWnd, messg, wParam, lParam ));
}

// Paint the tab identified by 'obj'

void PaintTab(ObjectData *obj, LPDRAWITEMSTRUCT lpdis)
{
   HWND hTabCtrl = lpdis->hwndItem;
   HBRUSH hbr; 
   char label[100];
   TCITEM tci;
   HDC hdc = lpdis->hDC;
   HWND hwnd = lpdis->hwndItem;
   RECT rc = lpdis->rcItem;
   bool selected = (lpdis->itemID == TabCtrl_GetCurSel(hwnd));
   SIZE te; 

   COLORREF bkgColor = gMainWindowColor;

   memset(label, '\0', sizeof(label));
   tci.mask = TCIF_TEXT;
   tci.pszText = label;
   tci.cchTextMax = sizeof(label)-1;

   TabCtrl_GetItem(hTabCtrl, lpdis->itemID, &tci);
   int labelLen = strlen(label);

   if(!selected && GetDeviceCaps(hdc,BITSPIXEL) >= 16)
   {
      COLORREF crFrom = GetSysColor(COLOR_3DFACE);
      COLORREF col;

      int rFrom = GetRValue(crFrom);
      int gFrom = GetGValue(crFrom);
      int bFrom = GetBValue(crFrom);
      int rTo = rFrom*0.8;
      int gTo = gFrom*0.8;
      int bTo = bFrom*0.8;
      int red,green,blue;
      float fac;

      int height = lpdis->rcItem.bottom - lpdis->rcItem.top+1;

      for (int line = 0; line <= height; line++)
      {
         fac = line/(float)height;
         red = nint(rFrom - (rFrom-rTo)*fac);
         green = nint(gFrom - (gFrom-gTo)*fac);
         blue = nint(bFrom - (bFrom-bTo)*fac);
     
         col = RGB(red,green,blue);
   		
         HPEN pen = CreatePen(PS_SOLID,0,col);  
         SelectObject(hdc, pen); 
         MoveToEx(hdc,rc.left,rc.top+line,NULL);
         LineTo(hdc,rc.right,rc.top+line);
         DeleteObject(pen);
      }
   }
   else // simple solid fill
   {
      hbr = (HBRUSH)CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
      //hbr = (HBRUSH)CreateSolidBrush(RGB(0,255,0));

      FillRect(hdc, &rc, hbr);
      DeleteObject(hbr);
   }

   SetBkMode(hdc, TRANSPARENT);
   SetBkColor(hdc, bkgColor);
   SetTextColor(hdc,obj->fgColor);

   GetTextExtentPoint32(hdc, label, labelLen, &te);

   int x = rc.left+ ((rc.right-rc.left)-te.cx)/2;
   int y = rc.top + ((rc.bottom-rc.top)-te.cy)/2  + (selected ? 0 : 2);

   TextOut(hdc,x,y,label,labelLen);
}

// Return the combined rectanges of all the current tabs for the tab object obj.

void GetTabRegion(ObjectData *obj, HRGN hRgn)
{
   HWND hwnd = obj->hWnd;
   HBRUSH hbr; 
   TCITEM tci;
   short selectedTab = TabCtrl_GetCurSel(hwnd);
   SIZE te; 
   RECT r;
   HRGN hRgnRect;

   int nrTabs = TabCtrl_GetItemCount(hwnd);


   for(int i = 0; i < nrTabs; i++)
   {
      TabCtrl_GetItemRect(hwnd,i,&r);
      r.right += obj->xo;
      r.left += obj->xo;
      r.top += obj->yo;
      r.bottom += obj->yo;

      if(selectedTab == i)
      {
         r.top = obj->yo;
         r.left-=2;
         r.right-=2;
      }

		hRgnRect = CreateRectRgnIndirect(&r);
		CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
      DeleteObject(hRgnRect);
   }

 
}


/******************************************************************************
    The left mouse button has just been pressed. The possible responses to this
    action are:

    In edit mode:
        1. Select a background region including some controls
        2. Modify the control numbers
        3. Modify the tab numbers
        4. Select one or more object
        5. Deselect one or more objects
******************************************************************************/

ObjectData *objDown = NULL;
bool objSelectedOnDown = false;

void ProcessLeftButtonDown(LPARAM lParam, WPARAM wParam, HWND hWnd, WinData *win)
{
   short x = LOWORD(lParam);
   short y = HIWORD(lParam);

// Make sure that when we paste an object it goes to (g_click_x,g_click_y)
   g_click_x = x;
   g_click_y = y;
   pasteMode = GUI_CLICK; 

   if(!win->activated) // Make sure window is in edit mode
   {
	   ObjectData *obj;

	   RECT rect;
	   GetClientRect(hWnd,&rect);

      obj = win->FindObject(x,y); 

   // See if there is an object beneath the cursor
   // if not deselect any selected objects
      if(!obj)  
      {
         win->DeselectObjects();
         currentObj = NULL;
	      MyInvalidateRect(hWnd,NULL,false);         
         return;
      }

   // Are we modifying the control numbers?
      if(win->modifyingCtrlNrs)
      {
         if(obj->nr() == -1)
         {
            obj->nr(win->objectNrCnt);
		      SetWindowLong(obj->hWnd,GWL_ID,(LONG)win->objectNrCnt);
            win->objectNrCnt++;
         }
	      MyInvalidateRect(hWnd,NULL,false);   
         return;
      }

   // Are we modifying tab numbers?
      else if(win->modifyingTabNrs)
      {
         if(obj->tabNr == -1)
         {
            win->tabMode = TAB_BY_TAB_NUMBER;
            obj->tabNr = win->objectNrCnt;
            win->objectNrCnt++;
				win->widgets.sort();
         }
	      MyInvalidateRect(hWnd,NULL,false);   
         return;
      } 
      
   // Select 1 object deselect others           
      if((wParam & MK_SHIFT) == 0) // Shift up
      {
         char str[MAX_STR];
         sprintf(str,"(x,y,w,h) = (%hd,%hd,%hd,%hd)",obj->xo,obj->yo,obj->wo,obj->ho);
         SetWindowText(hWnd,str);
         SetTimer(hWnd,1,2000,NULL);

         if(obj->selected_ == false)
         {
            win->DeselectObjects(); 
            WaitButtonUp(hWnd); 
         }
         obj->selected_ = true;
         ModifyCursor(hWnd,win,obj,x,y);
         win->MoveObjectToStart(obj);
      //   obj->MoveWindowToTop(); 
         if(obj->tabParent != 0)  
            SetFocus(obj->hwndParent); // This takes the focus away from a tabparent
         MyInvalidateRect(hWnd,NULL,false);  // when selecting a tab child control
         return;
      }

   // Deselect object or select others            
      else        // Shift down
      {
         if(obj->selected_)
         {
            objDown = NULL; // Make sure a currently selected object can be deselected
         }
         else
         {
            obj->selected_ = true;  // Make sure a newly selected object is not deselected on mouse up
            objSelectedOnDown = true;
         }
         MyInvalidateRect(hWnd,NULL,false);         
         return;
      }        
   }
   //else
   //{
   //    HWND hWnd = GetFocus();
   //    if(!IsMouseInWindow(hWnd))
   //    {
   //       ObjectData *obj = win->widgets.findByWin(hWnd); // Find object selected
   //       if(obj && obj->type == SLIDER)
   //         SetFocus(NULL);
   //    }
   //}
}


void ProcessLeftButtonUp(LPARAM lParam, WPARAM wParam, HWND hWnd, WinData *win)
{
   short x = LOWORD(lParam);
   short y = HIWORD(lParam);

// Make sure that when we paste an object it goes to (g_click_x,g_click_y)
   g_click_x = x;
   g_click_y = y;
   pasteMode = GUI_CLICK; 

   if(!win->activated) // Make sure window is in edit mode
   {
	   ObjectData *obj;

	   RECT rect;
	   GetClientRect(hWnd,&rect);

      obj = win->FindObject(x,y); 

   // See if there is an object beneath the cursor
   // if not deselect any selected objects
      if(!obj)  
      {       
         return;
      }

   // Select 1 object deselect others           
      if((wParam & MK_SHIFT) == 0) // Shift up
      {
         return;
      }

   // Deselect an object but only if already selected
   // and only if it wasn't selected on mouse down
      else  // Shift down
      {
         if(obj->selected_ && objSelectedOnDown == false) //obj != objDown)
         {
             obj->selected_ = false; // Don't deselect on mouse up
         }
         objSelectedOnDown = false;
         objDown = NULL;
         MyInvalidateRect(hWnd,NULL,false);         
         return;
      }        
   }
}


/******************************************************************************
    The mouse is being moved - either modify the cursor if the mouse button is
    unpressed or move or resize the object if the left mouse button is pressed.
******************************************************************************/

void ProcessMouseMove(LPARAM lParam, WPARAM wParam, HWND hWnd, WinData *win)
{
	short x1,y1;
	ObjectData *obj;
   POINT p;

// Don't come here while in another event loop
   if(interactiveMode != NOTHING && interactiveMode != MOVING_OVER_OBJECT)
      return;

	x1 = LOWORD(lParam);
	y1 = HIWORD(lParam);
   p.x = x1;
   p.y = y1;

	obj = win->FindObject(x1,y1); 

// If the left button is down either select a region 
// or move a selected control
   if((wParam & MK_LBUTTON) > 0) 
   {    
   // If we are not over a control allow user to define 
   // a rubber rectangle to define selected objects
      if(!obj)
      {
			short x2,y2;
         MyUpdateWindow(hWnd); // Force all controls to redraw
         SelectRegion(hWnd, x1, y1, x2, y2);
         win->SelectObjects(x1,y1,x2,y2);
	      MyInvalidateRect(hWnd,NULL,false); 
         interactiveMode = NOTHING;
      }
   // Otherwise move the control
      else
      {
         win->CopyAllObjects(); // Copy first time selected for move
         MoveOrResizeObject(hWnd,win,obj,x1,y1);
         interactiveMode = NOTHING;
      }
   }
   else // Modify cursor
   {
      if(obj)
      {
         ModifyCursor(hWnd,win,obj,x1,y1);
         if(obj->selected_)
            interactiveMode = MOVING_OVER_OBJECT;
         else
            interactiveMode = NOTHING;
      }
      else
      {
	      SetCursor(LoadCursor(NULL,IDC_ARROW));
         interactiveMode = NOTHING;
      }
   }      
}

/******************************************************************************
    The coordinates (x,y) are used to determine the new position or size
    of the selected object in the editable gui window 'win'
******************************************************************************/

void MoveOrResizeObject(HWND hWnd, WinData *win, ObjectData *obj, short x, short y)
{ 

   long style = GetWindowLong(obj->hWnd,GWL_STYLE);
   short ww,wh;
	RECT rect;
   HDC hdc;
	GetClientRect(hWnd,&rect);
	ww = (short)rect.right;
	wh = (short)rect.bottom;
   StaticTextInfo* info;
   bool MultiLineStaticText = false;
   int right,top,left,base,middledown,middleacross;

   if(obj->type == PANEL)
   {
      PanelInfo *info = (PanelInfo*)obj->data;
      right = info->x+info->w+20;
      top = obj->yo;
      left = info->x;
      base = info->y+info->h;
      middledown = top+info->h/2;
      middleacross = left+info->w/2;
   }
   else
   {
      right = obj->xo+obj->wo;
      top = obj->yo;
      left = obj->xo;
      base = obj->yo + obj->ho;
      middledown = obj->yo+obj->ho/2;
      middleacross = obj->xo+obj->wo/2;
   }


// Don't move or resize a status region
   if(obj->type == STATUSBOX)
      return;

   if(obj->type == STATICTEXT)
   {
      info = (StaticTextInfo*)obj->data;
      MultiLineStaticText = info->multiLine;
   }


// Move the object don't allow resizing
   if((obj->type == STATICTEXT && !MultiLineStaticText) || obj->type == CHECKBOX || obj->type == RADIO_BUTTON )
   {
      hdc = GetDC(hWnd);
		MoveObjectsInteractively(0,win, hdc, obj, x, y, ww, wh);
      ReleaseDC(hWnd,hdc);
		MyInvalidateRect(hWnd,NULL,false);             
		return;
   }

// In resize region for horizontally resizable objects
   if(abs(x - right) < 5 && (y > top) && y < (base-5) &&
	      (obj->type == TEXTMENU ||  obj->type == TEXTBOX || obj->type == BUTTON  || obj->type == COLORBOX ||  
          obj->type == PROGRESSBAR ||  obj->type == LISTBOX || obj->type == CLIWINDOW || obj->type == PANEL ||
          obj->type == TEXTEDITOR || obj->type == PLOTWINDOW || obj->type == IMAGEWINDOW || obj->type == OPENGLWINDOW ||
			 obj->type == GRIDCTRL || MultiLineStaticText ||
	      (obj->type == SLIDER && ((style & TBS_VERT) == 0))))
	{ 
      hdc = GetDC(hWnd);
      ResizeObjectsInteractively(HORIZ_RESIZE,win, hdc, obj, x, y, ww, wh);
      ReleaseDC(hWnd,hdc);
      hdc = GetDC(obj->hwndParent);
      obj->DrawSelectRect(hdc);
      ReleaseDC(obj->hwndParent,hdc);
   }

// In resize region for vertically resizable objects
   else if(abs(y-base) < 5 && (x > left) && x < (right-5) &&
	      ((obj->type == SLIDER && (style & TBS_VERT))|| obj->type == BUTTON || obj->type == COLORBOX ||  
           obj->type == PROGRESSBAR || obj->type == LISTBOX  || obj->type == CLIWINDOW || obj->type == PANEL ||
           obj->type == PLOTWINDOW || obj->type == IMAGEWINDOW || obj->type == OPENGLWINDOW ||
			  obj->type == GRIDCTRL || MultiLineStaticText ||
           obj->type == TEXTEDITOR))
	{ 
      hdc = GetDC(hWnd);
      ResizeObjectsInteractively(VERT_RESIZE,win, hdc, obj, x, y, ww, wh);
      ReleaseDC(hWnd,hdc);
      hdc = GetDC(obj->hwndParent);
      obj->DrawSelectRect(hdc);
      ReleaseDC(obj->hwndParent,hdc);
   }
   
// In resize region for horizontally & vertically resizable objects
   else if(abs(x-right) < 5 && abs(y-base) < 5 &&
	      (obj->type != TEXTMENU &&  obj->type != TEXTBOX &&
	      obj->type != SLIDER && obj->type != GROUP_BOX && obj->type != TABCTRL))
   {
      hdc = GetDC(hWnd);
      ResizeObjectsInteractively(DIAG_RESIZE,win, hdc, obj, x, y, ww, wh);
      ReleaseDC(hWnd,hdc);
      hdc = GetDC(obj->hwndParent);
      obj->DrawSelectRect(hdc);
      ReleaseDC(obj->hwndParent,hdc);
   }

   else if((obj->type == GROUP_BOX || obj->type == TABCTRL) && (abs(x-right) < 5 && abs(y-middledown) < 5))
   {
      hdc = GetDC(hWnd);
      ResizeObjectsInteractively(HORIZ_RESIZE,win, hdc, obj, x, y, ww, wh);
      ReleaseDC(hWnd,hdc);
      hdc = GetDC(obj->hwndParent);
      obj->DrawSelectRect(hdc);
      ReleaseDC(obj->hwndParent,hdc);
   }

   else if((obj->type == GROUP_BOX || obj->type == TABCTRL) && (abs(x-middleacross) < 5 && abs(y-base) < 5))
   {
      hdc = GetDC(hWnd);
      ResizeObjectsInteractively(VERT_RESIZE,win, hdc, obj, x, y, ww, wh);
      ReleaseDC(hWnd,hdc);
      hdc = GetDC(obj->hwndParent);
      obj->DrawSelectRect(hdc);
      ReleaseDC(obj->hwndParent,hdc);
   }

   else if((obj->type == GROUP_BOX || obj->type == TABCTRL) && (abs(x - right) < 5 && abs(y-base) < 5))
   {
      hdc = GetDC(hWnd);
      ResizeObjectsInteractively(DIAG_RESIZE,win, hdc, obj, x, y, ww, wh);
      ReleaseDC(hWnd,hdc);
      hdc = GetDC(obj->hwndParent);
      obj->DrawSelectRect(hdc);
      ReleaseDC(obj->hwndParent,hdc);
   }


// Otherwise just move the object  
   else 
   {
      HDC hdc = GetDC(hWnd);
      MoveObjectsInteractively(0,win, hdc, obj, x, y, ww, wh);
      ReleaseDC(hWnd,hdc);
   }

	MyInvalidateRect(hWnd,NULL,false); 
}

/******************************************************************************
    Modify the cursor in the edited gui window hWnd as the mouse moves.
    This cursor shows which parts of objects can be used for moving and 
    which parts can be used for resizing.
******************************************************************************/

void ModifyCursor(HWND hWnd, WinData *win, ObjectData *obj, short x, short y)
{
	RECT rect;
   int right,top,left,base,middledown,middleacross;
	GetClientRect(hWnd,&rect);
   StaticTextInfo* info;
   bool MultiLineStaticText;

   if(obj->type == STATICTEXT)
   {
      info = (StaticTextInfo*)obj->data;
      MultiLineStaticText = info->multiLine;
   }
   else
      MultiLineStaticText = false;

   long style = GetWindowLong(obj->hWnd,GWL_STYLE);

   if(obj->type == PANEL)
   {
      PanelInfo *info = (PanelInfo*)obj->data;
      right = info->x+info->w+20;
      top = obj->yo;
      left = info->x;
      base = info->y+info->h;
      middledown = top+info->h/2;
      middleacross = left+info->w/2;
   }
   else
   {
      right = obj->xo+obj->wo;
      top = obj->yo;
      left = obj->xo;
      base = obj->yo + obj->ho;
      middledown = obj->yo+obj->ho/2;
      middleacross = obj->xo+obj->wo/2;
   }


   if(obj->selected_) // Only modify the cursor if the object is selected
   {
 
   // Show horizontal resizing cursor
	   if(abs(x - right) < 5 && (y > top) && y < (base-5) &&
	      (obj->type == TEXTMENU ||  obj->type == TEXTBOX ||  obj->type == BUTTON  || 
          obj->type == COLORBOX || obj->type == PROGRESSBAR ||  obj->type == LISTBOX || 
          obj->type == CLIWINDOW || obj->type == TEXTEDITOR || obj->type == PANEL ||
          obj->type == PLOTWINDOW || obj->type == IMAGEWINDOW || obj->type == OPENGLWINDOW ||
			 obj->type == GRIDCTRL || MultiLineStaticText || 
	      (obj->type == SLIDER && ((style & TBS_VERT) == 0) ) ))
	   { 
		      SetCursor(LoadCursor(NULL,IDC_SIZEWE)); 
      }

   // Show vertical resize cursor
	   else if(abs(y - base) < 5 && (x > left) && x < (right-5) &&
	      ((obj->type == SLIDER && (style & TBS_VERT) ) || obj->type == BUTTON  ||
           obj->type == COLORBOX || obj->type == PROGRESSBAR ||  obj->type == LISTBOX ||  obj->type == PANEL ||
           obj->type == CLIWINDOW || obj->type == TEXTEDITOR || obj->type == PLOTWINDOW || MultiLineStaticText || 
           obj->type == IMAGEWINDOW || obj->type == OPENGLWINDOW || obj->type == GRIDCTRL))
	   { 
		      SetCursor(LoadCursor(NULL,IDC_SIZENS)); 
      }
      
   // Show resize cursor (both directions)
	   else if(abs(x - right) < 5 && abs(y - base) < 5 &&
	      (obj->type != TEXTMENU &&  obj->type != TEXTBOX && !(obj->type == STATICTEXT && !MultiLineStaticText) &&
	      obj->type != SLIDER && obj->type != GROUP_BOX && obj->type != TABCTRL))
	   {
		      SetCursor(LoadCursor(NULL,IDC_SIZENWSE));	
      }	

      else if((obj->type == GROUP_BOX || obj->type == TABCTRL) && (abs(x-right) < 5 && abs(y-middledown) < 5))
      {
		    SetCursor(LoadCursor(NULL,IDC_SIZEWE));
      }

      else if((obj->type == GROUP_BOX || obj->type == TABCTRL) && (abs(x-middleacross) < 5 && abs(y-base) < 5))
      {
		   SetCursor(LoadCursor(NULL,IDC_SIZENS)); 
      }

      else if((obj->type == GROUP_BOX || obj->type == TABCTRL) && (abs(x-right) < 5 && abs(y-base) < 5))
      {
		   SetCursor(LoadCursor(NULL,IDC_SIZENWSE));	
      }  

      else if(obj->type == TABCTRL && abs(y-top) < 20)
      {
		   SetCursor(LoadCursor(NULL,IDC_SIZEALL));	
      }  

   // For all other positions or objects display move cursor
      else
      {
		   SetCursor(LoadCursor(NULL,IDC_SIZEALL)); 
         return;
      }
   } 
}

/******************************************************************************
    Allow user to select a rectangular region in window hWnd.
    Start coordinates are x0,y0 final coordinates are (x0,y0)->(x2,y2)
    (note that coords may be swapped to ensure that x0<x2 and y0<y2).
******************************************************************************/

void SelectRegion(HWND hWnd, short &x0, short &y0, short &x2, short &y2)
{
	MSG msg;
   RECT r;
   short x1,y1;
   short xmin,xmax,ymin,ymax;
   HPEN rubberPen;

   x2 = x0;
   y2 = y0;
   xmin = 0;
   ymin = 0;
   GetClientRect(hWnd,&r);
   xmax = (short)r.right;
   ymax = (short)r.bottom;

// Select pen for rubber rectangle        
 	HDC hdc = GetDC(hWnd);
   rubberPen = CreatePen(PS_DOT,0,RGB(0,0,0));
   SelectObject(hdc,rubberPen);
   SetROP2(hdc,R2_XORPEN);		   

// Force window to capture mouse events
   SetCapture(hWnd);
   
// Track the cursor as the region is selected with the mouse            	
   while(true)
   {
	   if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
	   {
         if(msg.hwnd == hWnd)
         {
	         if(msg.message == WM_LBUTTONUP)
	         {		
               if(x2 < x0) Swap(x0,x2);
               if(y2 < y0) Swap(y0,y2);
			      break;	// Exit while loop
	         }
	      
	         else if(msg.message == WM_MOUSEMOVE && msg.hwnd == hWnd) // Mouse is moving so track
			   {	    
               x1 = LOWORD(msg.lParam);  // Get cursor position
               y1 = HIWORD(msg.lParam);
		   
				   if(x1 <= xmin) x1 = xmin+1; // Check for out of bounds cursor location
				   if(x1 >= xmax) x1 = xmax-1; // and limit to border.
				   if(y1 <= ymin) y1 = ymin+1;
				   if(y1 >= ymax) y1 = ymax-1;
									      			   				      
				   DrawRect(hdc,x0,y0,x2,y2); // Remove old rect
				   DrawRect(hdc,x0,y0,x1,y1); // Draw new one
				
				   x2 = x1; // Record this position
				   y2 = y1; // for later erasure				
			   }
         }
		}
      Sleep(10);
   }

// Release mouse from window
   ReleaseCapture();
// Clear up
   ReleaseDC(hWnd,hdc); 
   DeleteObject(rubberPen);
}

/******************************************************************************
     Display the contextual menu for the GUI window and process the response  
******************************************************************************/

void ProcessContextMenu(LPARAM lParam, WPARAM wParam, HWND hWnd, WinData *win)
{

// Show the menu for an edited GUI window
   if(!win->activated) 
   {  
   // Get the menu to display
      HMENU hMenu;
      hMenu = LoadMenu (prospaInstance,"GUIEDITMENU");
      hMenu = GetSubMenu(hMenu,0) ;

   // Make sure we can't modify numbers unless we are displaying them
      if(win->displayObjCtrlNrs)
      {
         ModifyMenu(hMenu,ID_TOGGLE_CTRL_NUMBERS,MF_BYCOMMAND|MF_STRING,ID_TOGGLE_CTRL_NUMBERS,"Hide control numbers\tCtrl+Shift+N");
         ModifyMenu(hMenu,ID_TOGGLE_TAB_NUMBERS,MF_BYCOMMAND|MF_STRING,ID_TOGGLE_TAB_NUMBERS,"Show tab numbers\tCtrl+N");
         EnableMenuItem(hMenu,ID_TOGGLE_TAB_NUMBERS,MF_BYCOMMAND | MF_GRAYED);
         EnableMenuItem(hMenu,ID_MODIFY_TAB_NUMBERS,MF_BYCOMMAND | MF_GRAYED);
      }
      else if(win->displayObjTabNrs)
      {
         ModifyMenu(hMenu,ID_TOGGLE_TAB_NUMBERS,MF_BYCOMMAND|MF_STRING,ID_TOGGLE_TAB_NUMBERS,"Hide tab numbers\tCtrl+N");
         ModifyMenu(hMenu,ID_TOGGLE_CTRL_NUMBERS,MF_BYCOMMAND|MF_STRING,ID_TOGGLE_CTRL_NUMBERS,"Show control numbers\tCtrl+Shift+N");
         EnableMenuItem(hMenu,ID_TOGGLE_CTRL_NUMBERS,MF_BYCOMMAND | MF_GRAYED);
         EnableMenuItem(hMenu,ID_MODIFY_CTRL_NUMBERS,MF_BYCOMMAND | MF_GRAYED);
      }
      else
      {
         ModifyMenu(hMenu,ID_TOGGLE_CTRL_NUMBERS,MF_BYCOMMAND|MF_STRING,ID_TOGGLE_CTRL_NUMBERS,"Show control numbers\tCtrl+Shift+N");
         ModifyMenu(hMenu,ID_TOGGLE_TAB_NUMBERS,MF_BYCOMMAND|MF_STRING,ID_TOGGLE_TAB_NUMBERS,"Show tab numbers\tCtrl+N");
         EnableMenuItem(hMenu,ID_MODIFY_CTRL_NUMBERS,MF_BYCOMMAND | MF_GRAYED);
         EnableMenuItem(hMenu,ID_MODIFY_TAB_NUMBERS,MF_BYCOMMAND | MF_GRAYED);
      }

   // Make sure we can't manipulate the controls if we are modifying the numbers
      if(win->modifyingCtrlNrs || win->modifyingTabNrs)
      {
         EnableMenuItem(hMenu,ID_STOPEDITING,  MF_BYCOMMAND | MF_GRAYED);
         EnableMenuItem(hMenu,ID_SELECTALL,    MF_BYCOMMAND | MF_GRAYED);
         EnableMenuItem(hMenu,ID_LEFTALIGN,    MF_BYCOMMAND | MF_GRAYED);
         EnableMenuItem(hMenu,ID_RIGHTALIGN,   MF_BYCOMMAND | MF_GRAYED);
         EnableMenuItem(hMenu,ID_ALIGNTOPS,    MF_BYCOMMAND | MF_GRAYED);
         EnableMenuItem(hMenu,ID_ALIGNBASES,   MF_BYCOMMAND | MF_GRAYED);
         EnableMenuItem(hMenu,ID_COPYOBJECTS,  MF_BYCOMMAND | MF_GRAYED);
         EnableMenuItem(hMenu,ID_PASTEOBJECTS, MF_BYCOMMAND | MF_GRAYED);
         EnableMenuItem(hMenu,ID_CUTOBJECTS,   MF_BYCOMMAND | MF_GRAYED);
      }
      else // Get the copy, paste, cut menus correctly enabled
      {
         if(win->AreObjectsSelected())
         {
            EnableMenuItem(hMenu,ID_COPYOBJECTS,  MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu,ID_CUTOBJECTS,   MF_BYCOMMAND | MF_ENABLED);
         }
         else
         {
            EnableMenuItem(hMenu,ID_COPYOBJECTS,  MF_BYCOMMAND | MF_GRAYED);
            EnableMenuItem(hMenu,ID_CUTOBJECTS,   MF_BYCOMMAND | MF_GRAYED);
         }

         if(AreObjectsCopied())
         {
            EnableMenuItem(hMenu,ID_PASTEOBJECTS,   MF_BYCOMMAND | MF_ENABLED);
         }
         else
         {
            EnableMenuItem(hMenu,ID_PASTEOBJECTS,   MF_BYCOMMAND | MF_GRAYED);
         }
      }

   // While modifying control numbers disable other number options
      if(win->modifyingCtrlNrs)
      {
         ModifyMenu(hMenu,ID_MODIFY_CTRL_NUMBERS,MF_BYCOMMAND|MF_STRING,ID_MODIFY_CTRL_NUMBERS,"Stop modifying control numbers");
         EnableMenuItem(hMenu,ID_TOGGLE_CTRL_NUMBERS,MF_BYCOMMAND | MF_GRAYED);
         EnableMenuItem(hMenu,ID_TOGGLE_TAB_NUMBERS, MF_BYCOMMAND | MF_GRAYED);
         EnableMenuItem(hMenu,ID_MODIFY_TAB_NUMBERS, MF_BYCOMMAND | MF_GRAYED);
      }

   // While modifying tab numbers disable other number options
      if(win->modifyingTabNrs)
      {
         ModifyMenu(hMenu,ID_MODIFY_TAB_NUMBERS,MF_BYCOMMAND|MF_STRING,ID_MODIFY_TAB_NUMBERS,"Stop modifying tab numbers");
         EnableMenuItem(hMenu,ID_TOGGLE_TAB_NUMBERS,MF_BYCOMMAND | MF_GRAYED);
         EnableMenuItem(hMenu,ID_TOGGLE_CTRL_NUMBERS, MF_BYCOMMAND | MF_GRAYED);
         EnableMenuItem(hMenu,ID_MODIFY_CTRL_NUMBERS, MF_BYCOMMAND | MF_GRAYED);
      }

   // Display the menu and wait for users response
      short item = (short)TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, LOWORD(lParam),HIWORD(lParam), 0, hWnd, NULL);

   // Process the response
      ProcessContextMenuItems(hWnd,win,item);
      DestroyMenu(hMenu);
   }

// Show the menu for an executing GUI window (but not for a dialog window)
   else if(win->showMenu && win->type != DIALOG_WIN)
   {
      HMENU hMenu;
      hMenu = LoadMenu (prospaInstance,"GUIRUNMENU");
      hMenu = GetSubMenu(hMenu,0) ;


   // Modify menu depending on whether control numbers are being displayed or not
      if(!win->displayObjCtrlNrs)
         ModifyMenu(hMenu,ID_TOGGLENUMBERS,MF_BYCOMMAND|MF_STRING,ID_TOGGLENUMBERS,"Show control numbers\tCtrl+Shift+N");
      else
         ModifyMenu(hMenu,ID_TOGGLENUMBERS,MF_BYCOMMAND|MF_STRING,ID_TOGGLENUMBERS,"Hide control numbers\tCtrl+Shift+N");


      short item = (short)TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, LOWORD(lParam), HIWORD(lParam), 0, hWnd, NULL);

      ProcessContextMenuItems(hWnd,win,item);
      DestroyMenu(hMenu);
   }
   else if(win->bkgMenu) // Display and process background menu
   {
      POINT p;
      p.x = LOWORD(lParam);
      p.y = HIWORD(lParam);
      SetCursor(LoadCursor(NULL,IDC_ARROW));
      HMENU hMenu = win->bkgMenu;
	   short item = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, 0, hWnd, NULL) ;
	   if(item) 
         PreProcessCommand(NULL, item, hWnd, win);
	} 
}

/******************************************************************************
           Process each of the edited gui window menu items  
******************************************************************************/

void ProcessContextMenuItems(HWND hWnd, WinData *win, short item)
{
   Interface itfc;

   switch(item)
   {
      case(ID_COPYOBJECTS):
	      CopySelectedObjects(win,&copyRect); 
         pasteMode = GUI_PASTE; // Tells next paste to offset object
         break;
      case(ID_HIDE_WINDOW):
	      ShowWindow(hWnd,SW_HIDE);
         break;
      case(ID_PASTEOBJECTS):
         win->CopyAllObjects(); // For undo
         PasteSelectedObjects(win,g_click_x,g_click_y,&copyRect,pasteMode); 
		   MyInvalidateRect(hWnd,NULL,false); 
         pasteMode = GUI_PASTE;  // Tells next paste to offset object
         break;
      case(ID_CUTOBJECTS):
         win->CopyAllObjects(); // For undo
	      CutSelectedObjects(win,&copyRect);
		   MyInvalidateRect(hWnd,NULL,false); 
         pasteMode = GUI_CUT;  // Tells next paste not to offset object 
         break;
      case(ID_STOPEDITING):
			win->activate();
			break;                     
      case(ID_SELECTALL):
	      win->SelectAllObjects(true);
			MyInvalidateRect(hWnd,NULL,false); 
         break;
      case(ID_LEFTALIGN):
         win->CopyAllObjects(); // For undo
     //    macroDepth = 1; // Do this so we can abort using <esc>
         AlignObjects(&itfc,"\"left\"");
    //     macroDepth = 0;
         break;
      case(ID_RIGHTALIGN):
         win->CopyAllObjects(); // For undo
     //    macroDepth = 1;
         AlignObjects(&itfc,"\"right\"");
     //    macroDepth = 0;
         break;
      case(ID_ALIGNTOPS):
         win->CopyAllObjects(); // For undo
      //   macroDepth = 1;
         AlignObjects(&itfc,"\"top\"");
     //    macroDepth = 0;
         break;
      case(ID_ALIGNBASES):
         win->CopyAllObjects(); // For undo
     //    macroDepth = 1;
         AlignObjects(&itfc,"\"base\"");
     //    macroDepth = 0;
         break;
      case(ID_TOGGLENUMBERS):
      case(ID_TOGGLE_CTRL_NUMBERS):
		   win->displayObjCtrlNrs = !win->displayObjCtrlNrs; 
         if(win->displayObjCtrlNrs) win->displayObjTabNrs = false;
		   MyInvalidateRect(hWnd,NULL,false);   
         break;
      case(ID_TOGGLE_TAB_NUMBERS):
		   win->displayObjTabNrs = !win->displayObjTabNrs; 
         if(win->displayObjTabNrs) win->displayObjCtrlNrs = false;
		   MyInvalidateRect(hWnd,NULL,false);   
         break;
      case(ID_MODIFY_CTRL_NUMBERS):
		   win->modifyingCtrlNrs = !win->modifyingCtrlNrs; 
         if(win->modifyingCtrlNrs)
         {
            if(YesNoDialog(MB_ICONWARNING,1,"Warning","This will reset all control numbers - continue?") == IDYES)
            {
               win->modifyingTabNrs = false;
               win->ResetControlNumbers();
            }
         }
         else
         {
            win->displayObjCtrlNrs = false;
         }
		   MyInvalidateRect(hWnd,NULL,false);   
         break;
      case(ID_MODIFY_TAB_NUMBERS):
		   win->modifyingTabNrs = !win->modifyingTabNrs; 
         if(win->modifyingTabNrs)
         {
            if(YesNoDialog(MB_ICONWARNING,1,"Warning","This will reset all tab numbers - continue?") == IDYES)
            {
               win->modifyingCtrlNrs = false;
               win->ResetTabNumbers();
            }
         }
         else
         {
            win->displayObjTabNrs = false;
         }
		   MyInvalidateRect(hWnd,NULL,false);   
         break;
      case(ID_MAKEEDITABLE): // Make the current window editable
      {
      // Activate currently edited window
         ActivateEditedGUIWin();
      // Make this window editable
         win->makeEditable();	
         win->EnableResizing(true);
         currentAccel = NULL;
         SetFocus(win->hWnd);
         break;
      }
      case(ID_ACTIVEWINDOW): // Make the current window work normally
			win->activate();
         currentAccel = win->hAccelTable;
         break;

      case(ID_CLOSE_WINDOW): // Close the window
      {
	      short id = win->cancelID;
         if(id != -1) // Run commands associated with cancel button
         {
	         ObjectData *obj = win->widgets.findByNr(id);
	         if(obj)
            {
	            ProcessControlCommands(hWnd,(HWND)obj->hWnd,(int)id);
            }
	         break;
         } 
         else // Just exit window (possible calling an exit procedure)
         {
            SendMessage(hWnd,WM_SYSCOMMAND,(WPARAM)SC_CLOSE,(LPARAM)0);
         }       
	   }
   }
}

/******************************************************************************
*
* Process composite keyboard entries which relate to gui layout editing
*
* arrow keys .............. shift selected controls
* shift+arrow keys ........ resize selected controls
* ctrl+arrow keys ......... shift selected controls quickly
* shift+ctrl+arrow keys ... resize selected controls quickly
* delete .................. delete a control
* return .................. run default button commands 
*
******************************************************************************/

bool ProcessKeyDown(LPARAM lParam, WPARAM wParam, HWND hWnd, WinData *win)
{
   char  code = (CHAR)wParam;  // Virtual character code 

// Window should be edited to receive 
   if(!win->activated) 
   {  

   // Resize controls on shift+arrow keys
	   if(IsKeyDown(VK_SHIFT) && !IsKeyDown(VK_CONTROL))
	   {
		   if(code == VK_LEFT || code == VK_RIGHT || code == VK_UP || code == VK_DOWN)
		   {	
            if(win->CountSelectedObjects() > 0)
            {
               win->CopyAllObjects();
		         win->ResizeSelectedObjects(code,1);
            }
            else // Resize the window
            { 
            // Get window dimenions ***************************************************	
               RECT r;
               GetWindowRect(win->hWnd,&r);
               short x = (short)r.left;
               short y = (short)r.top;
               short w = (short)(r.right - r.left);
               short h = (short)(r.bottom - r.top);

               if(code == VK_RIGHT)
                  MoveWindow(win->hWnd,x,y,w+1,h,true);
               else if(code == VK_LEFT && w > 5)
                  MoveWindow(win->hWnd,x,y,w-1,h,true);
               else if(code == VK_UP && h > 5)
                  MoveWindow(win->hWnd,x,y,w,h-1,true);
               else if(code == VK_DOWN)
                  MoveWindow(win->hWnd,x,y,w,h+1,true);
               GetWindowRect(win->hWnd,&r);
               CText txt;
               txt.Format("w = %hd h = %hd",r.right-r.left,r.bottom-r.top);
               SetWindowText(hWnd,txt.Str());
               SetTimer(hWnd,1,2000,NULL);
            }
			   MyInvalidateRect(hWnd,NULL,false);  
			   MyUpdateWindow(hWnd);     // Make sure all objects are redrawn before next step       
		   }
	   }

   // Resize controls quickly on shift+ctrl+arrow keys
	   else if(IsKeyDown(VK_SHIFT) && IsKeyDown(VK_CONTROL))
	   {
		   if(code == VK_LEFT || code == VK_RIGHT || code == VK_UP || code == VK_DOWN)
		   {
            if(win->CountSelectedObjects() > 0)
            {
               win->CopyAllObjects();
		         win->ResizeSelectedObjects(code,5);
            }
            else // Resize the window
            { 
            // Get window dimenions ***************************************************	
               RECT r;
               GetWindowRect(win->hWnd,&r);
               short x = (short)r.left;
               short y = (short)r.top;
               short w = (short)(r.right - r.left);
               short h = (short)(r.bottom - r.top);

               if(code == VK_RIGHT)
                  MoveWindow(win->hWnd,x,y,w+5,h,true);
               else if(code == VK_LEFT && w > 5)
                  MoveWindow(win->hWnd,x,y,w-5,h,true);
               else if(code == VK_UP && y > 5)
                  MoveWindow(win->hWnd,x,y,w,h-5,true);
               else if(code == VK_DOWN)
                  MoveWindow(win->hWnd,x,y,w,h+5,true);
               GetWindowRect(win->hWnd,&r);
               CText txt;
               txt.Format("w = %hd h = %hd",r.right-r.left,r.bottom-r.top);
               SetWindowText(hWnd,txt.Str());
               SetTimer(hWnd,1,2000,NULL);
            }
			   MyInvalidateRect(hWnd,NULL,false);  
			   MyUpdateWindow(hWnd);     // Make sure all objects are redrawn before next step       
		   }
	   }

   // Move controls quickly on ctrl+arrow keys
	   else if(!IsKeyDown(VK_SHIFT) && IsKeyDown(VK_CONTROL))
	   {
		   if(code == VK_LEFT || code == VK_RIGHT || code == VK_UP || code == VK_DOWN)
		   {
            if(win->CountSelectedObjects() > 0)
            {
               win->CopyAllObjects();
		         win->MoveSelectedObjects(code,5);
            }
            else
            { 
            // Get window dimenions ***************************************************	
               RECT r;
               GetWindowRect(win->hWnd,&r);
               short x = (short)r.left;
               short y = (short)r.top;
               short w = (short)(r.right - r.left);
               short h = (short)(r.bottom - r.top);

               if(code == VK_RIGHT)
                  MoveWindow(win->hWnd,x+5,y,w,h,true);
               else if(code == VK_LEFT && x > 5)
                  MoveWindow(win->hWnd,x-5,y,w,h,true);
               else if(code == VK_UP && y > 5)
                  MoveWindow(win->hWnd,x,y-5,w,h,true);
               else if(code == VK_DOWN)
                  MoveWindow(win->hWnd,x,y+5,w,h,true);
               GetWindowRect(win->hWnd,&r);
               CText txt;
               txt.Format("x = %hd y = %hd",r.left,r.top);
               SetWindowText(hWnd,txt.Str());
               SetTimer(hWnd,1,2000,NULL);
            }

			   MyInvalidateRect(hWnd,NULL,false); 
			   MyUpdateWindow(hWnd);     // Make sure all objects are redrawn before next step       
		   }  
		}

   // Move controls on arrow keys
	   else
	   {
		   if(code == VK_LEFT || code == VK_RIGHT || code == VK_UP || code == VK_DOWN)
		   {
            if(win->CountSelectedObjects() > 0)
            {
               win->CopyAllObjects();
		         win->MoveSelectedObjects(code,1);
            }
            else
            { 
            // Get window dimenions ***************************************************	
               RECT r;
               GetWindowRect(win->hWnd,&r);
               short x = (short)r.left;
               short y = (short)r.top;
               short w = (short)(r.right - r.left);
               short h = (short)(r.bottom - r.top);

               if(code == VK_RIGHT)
                  MoveWindow(win->hWnd,x+1,y,w,h,true);
               else if(code == VK_LEFT && x > 0)
                  MoveWindow(win->hWnd,x-1,y,w,h,true);
               else if(code == VK_UP && y > 0)
                  MoveWindow(win->hWnd,x,y-1,w,h,true);
               else if(code == VK_DOWN)
                  MoveWindow(win->hWnd,x,y+1,w,h,true);
               GetWindowRect(win->hWnd,&r);
               CText txt;
               txt.Format("x = %hd y = %hd",r.left,r.top);
               SetWindowText(hWnd,txt.Str());
               SetTimer(hWnd,1,2000,NULL);
            }
			   MyInvalidateRect(hWnd,NULL,false);        
			   MyUpdateWindow(hWnd);     // Make sure all objects are redrawn before next step       
		   } 
	 //     else if(IsKeyDown(VK_SHIFT) && IsKeyDown(VK_TAB))
    //        MoveToNextWindow(hWnd);

		}

	// Delete a control	         
	   if(code == VK_DELETE)
	   {
         win->CopyAllObjects();
	      win->DeleteSelectedObjects();
			MyInvalidateRect(hWnd,NULL,false);                       
	   }
	}
   else
   {
      if(code == VK_F2)
      {

         if(AnyGUIWindowVisible())
            HideGUIWindows();
         else
            ShowGUIWindows(SW_SHOWNOACTIVATE);
      }

 // Process default button commands if return pressed while activated
 // and the mouse is not over a control
	   if(code == VK_RETURN)
	   {
	      short id = win->defaultID;
         if(id != -1)
         {
	         ObjectData *obj = win->widgets.findByNr(id);
	         if(obj)
	            ProcessControlCommands(hWnd,(HWND)obj->hWnd,(int)id);
	         return(true);
         }         
	   }

// Move to next or last window from those windows without menus
	   if(code == VK_TAB)
      {
         Interface itfc;

         if(IsKeyDown(VK_CONTROL) )
	      {
            if(IsKeyDown(VK_SHIFT))
               ShowLastWindow(&itfc,"0");
            else
               ShowNextWindow(&itfc,"0");
         }
      }

 // Process cancel button commands if escape pressed while activated
 // and the mouse is not over a control
	   if(code == VK_ESCAPE)
      {
         if(IsKeyDown(VK_SHIFT))
	      {
	         short id = win->cancelID;
            if(id != -1) // Run commands associated with cancel button
            {
	            ObjectData *obj = win->widgets.findByNr(id);
	            if(obj)
               {
	               ProcessControlCommands(hWnd,(HWND)obj->hWnd,(int)id);
               }
	            return(true);
            } 
            else // Just exit window (possible calling an exit procedure)
            {
               SendMessage(hWnd,WM_SYSCOMMAND,(WPARAM)SC_CLOSE,(LPARAM)0);
            }       
	      }
         else
         {
             gAbortMacro = true;
         }
      }
   }
   return(false);
}

/******************************************************************************
*
* Process raw character keyboard entries which relate to gui layout editing
* (i.e. keyboard short-cuts)
*
* Actions on running window
*
* ctrl+shift+E .............. make this the editable window
* shift+ctrl-N .............. show control numbers
* ctrl+V (paste) ............ paste copied objects to this window
*
* Actions on edited window
*
* ctrl+A ...  select all controls in window
* ctrl+C .... copy selected controls
* ctrl+X .....cut selected controls
* ctrl+E .... activate window
* ctrl+L .... left align selected controls
* ctrl+R .... right align selected controls
* ctrl+T .... top align selected controls
* ctrl+B .... bottom align selected controls
* ctrl+N .... show control numbers
*
*
******************************************************************************/

#define COPY	3
#define PASTE	22
#define UNDO	26
#define CUT		24
#define ENTER  13

void ProcessCharacter(LPARAM lParam, WPARAM wParam, HWND hWnd, WinData *win)
{

   char key = (char)wParam;
   bool shiftDown = IsKeyDown(VK_SHIFT); 

// If window is executing allow user make editable, paste objects or show control numbers 	      
   if(win->activated && win->showMenu) 
   {
      switch(key)
      {
         case(5): // Make this window the editable window shift-ctrl-E
	         if(shiftDown)
               ProcessContextMenuItems(hWnd,win,ID_MAKEEDITABLE);                     
		      return;                     
         case(14): // Display control numbers (shift-ctrl-N)
            if(shiftDown) // (shift-ctrl-N)
               ProcessContextMenuItems(hWnd,win,ID_TOGGLENUMBERS);                     
		      return;  

         case(PASTE): // Paste objects into this window (ctrl+V)
            if(shiftDown)
               pasteMode+=4;
            ProcessContextMenuItems(hWnd,win,ID_PASTEOBJECTS);  
		      MyInvalidateRect(hWnd,NULL,false);  
	         return;  
      }
   } 
// If window is editable allow user to do some basic things by using control keys 	      
   if(!win->activated)
   {    
      switch(key)
      {
         case(1):  // Select all controls (^A)
            ProcessContextMenuItems(hWnd,win,ID_SELECTALL);                      
	         break;
         case(COPY):  // Copy selected objects (^C)
            ProcessContextMenuItems(hWnd,win,ID_COPYOBJECTS);
	         break;
         case(UNDO):  // Undo last action (^Z)
            win->UndoObjectAction();
	         break;
         case(PASTE): // Paste objects into this window (ctrl+V)
            if(shiftDown)
               pasteMode+=4;
            ProcessContextMenuItems(hWnd,win,ID_PASTEOBJECTS); 
	         return; 
         case(CUT): // Cut selected objects (^X)
            ProcessContextMenuItems(hWnd,win,ID_CUTOBJECTS);
            break;
         case(5):  // Active window (^E)
            ProcessContextMenuItems(hWnd,win,ID_ACTIVEWINDOW);                    
			   return;                     
         case(12): // Left align selected objects (^L)
            ProcessContextMenuItems(hWnd,win,ID_LEFTALIGN); 
			   return;                     
         case(18): // Right align selected objects (^R)
            ProcessContextMenuItems(hWnd,win,ID_RIGHTALIGN); 
			   return;                     
         case(20): // Top align selected objects (^T)	
            ProcessContextMenuItems(hWnd,win,ID_ALIGNTOPS); 
			   return;                     
         case(2):  // Base align selected objects(^B)	  
            ProcessContextMenuItems(hWnd,win,ID_ALIGNBASES); 
			   return;  
         case(14):  // Show/hide control or tab numbers (^N)	
            if(shiftDown)
               ProcessContextMenuItems(hWnd,win,ID_TOGGLE_CTRL_NUMBERS); 
            else
               ProcessContextMenuItems(hWnd,win,ID_TOGGLE_TAB_NUMBERS); 
		      return;  
		}
	}      
}

/******************************************************************************
* Pre-Process a command request 
******************************************************************************/

int PreProcessCommand(LPARAM lParam, WPARAM wParam, HWND hWnd, WinData *win)
{
   if((HWND)lParam == NULL) // Its a menu or accelerator
   {
      UINT code = LOWORD(wParam);

   // Its a window
      if(code >= WINDOW_MENU_START)
      {
         char winTitle[500];
         WinData *win;
         HMENU menu = GetMenu(hWnd);
         if(menu)
         {
            GetMenuString(menu,code,winTitle,500,MF_BYCOMMAND);
            win = rootWin->FindWinByTitle(winTitle);
            if(win)
            {
               if(!win->keepInFront)
                  ChangeGUIParent(win->hWnd);
               ShowWindow(win->hWnd,SW_HIDE);
               ShowWindow(win->hWnd,SW_SHOW);
               win->visible = true;
            }
         }
         return(1);
      }
   // Its an edit procedure
      else if(code >= PROC_MENU_START)
      {
         char procName[MAX_STR];

         if(!curEditor)
            return(1);
               
         HMENU menu = GetMenu(hWnd);
         if(menu)
         {
            GetMenuString(menu,code,procName,100,MF_BYCOMMAND);
            curEditor->SelectProcedure(procName);
            curEditor->currentProc = procName; // Update proc name (2.2.6)
            SetEditTitle();
         }            
         return(1);
      }
   // User defined main window menus
      else if(code >= MAIN_UD_MENUS_START) 
      {
         HMENU menu = GetMenu(hWnd);
         char command[MAX_PATH];
					//TextMessage("User menu selected\n");

         if(menu)
         {
            GetMenuString(menu,code,command,100,MF_BYCOMMAND);
          
         // Check to see if the macro name has Help as its last part.
         // If so then design a valid macro help name (this will open a help file)  

            if(!strcmp(command,"Macro Help"))
            {
               strncpy_s(command,MAX_PATH,"MacroHelp",_TRUNCATE); 
            }

            else if(!strcmp(command,"Add/Remove Macros"))
            {
               strncpy_s(command,MAX_PATH,"AddMacros",_TRUNCATE); 
            }
        
        // Add the .mac extension back on since menu code has removed it    
            strcat(command,".mac"); 
            {
               CText path;
               CText oldPath;
               CText newPath;
               short folderIndex = ((code-MAIN_UD_MENUS_START)/100)*2+1;
               short nrBefore = -1;

               if(!win->menuObj) 
               {
                  if(win->menuListSize > 0 && win->menuList)
                  {
                     for(int i = 0; i < win->menuListSize; i++)
                     {
                        if(win->menuList[i] < 0) 
                        {
                           nrBefore = i;
                           break;
                        }
                     }
                  }
               }

               int menuSelected = nrBefore + (folderIndex - 1)/2;
               char menuName[MAX_STR];
               GetMenuString(menu,menuSelected,menuName,100,MF_BYPOSITION);
               RemoveCharacter(menuName,'&');

               path.Assign(VarList(userMenusVar)[folderIndex]); 
               Interface itfc;
               ReplaceVarInString(&itfc,path);                      
               
               GetDirectory(oldPath);
               SetDirectory(path); 
                        
               if(GetKeyState(VK_SHIFT) & 0x8000) // Edit the macro if the shift key is down
               {
                  char pathBak[MAX_PATH];
                  char nameBak[MAX_PATH];
                  bool alreadyLoaded = false;

						if(curEditor->CheckForUnsavedEdits(curEditor->regNr) == IDCANCEL)
							 return(1);

                  if(!curEditor)
                     return(1);

               // Save current editor file name
                  strncpy_s(pathBak,MAX_PATH,curEditor->edPath,_TRUNCATE); 
                  strncpy_s(nameBak,MAX_PATH,curEditor->edName,_TRUNCATE);

               // Set file path and name
                  strncpy_s(curEditor->edName,MAX_PATH,command,_TRUNCATE);
                  strncpy_s(curEditor->edPath,MAX_PATH,path.Str(),_TRUNCATE);

               // Check to see if we already have an loaded file of this name in the editor
                  if(IsEditFileAlreadyLoaded(curEditor) >= 0) // (2.2.7)
                  {
                     alreadyLoaded = true;
                     if(YesNoDialog(MB_ICONWARNING,0,"Warning","File already loaded\rDo you want to load a read-only copy?") == IDNO)
                     {
                        strncpy_s(curEditor->edPath,MAX_PATH,pathBak,_TRUNCATE); // Restore file name
                        strncpy_s(curEditor->edName,MAX_PATH,nameBak,_TRUNCATE);
                        return(1);
                     }
                  }
                  if(!curEditor->edParent->parent->winParent->keepInFront)
                     ChangeGUIParent(curEditor->edWin);
			         SetCursor(LoadCursor(NULL,IDC_WAIT));
                  LoadEditorGivenPath(path.Str(),command);  

               // Note if file already loaded
                  if(alreadyLoaded) // (2.2.7)
                     curEditor->readOnly = true; 
                  else
                     curEditor->readOnly = false; 
                  SetEditTitle(); 

                  AddFilenameToList(procLoadList,path.Str(), command);
					   SetCursor(LoadCursor(NULL,IDC_ARROW));
               }
               else // Load and run the macro
               {
                  char *text = LoadTextFileFromFolder(path.Str(), command,"");
                  AddFilenameToList(procRunList,path.Str(), command);
                  if(text)
                  {
                     ObjectData obj;
                     obj.type = MENU;
                     obj.winParent = win;
                     MenuInfo *info = new MenuInfo;
                     info->nrItems = 0;
                     info->cmd = NULL;
                     info->label = NULL;
                     info->accel = NULL;
                     info->menu = NULL;
                     info->key = NULL;
                     info->name = new char[strlen(menuName)+1];
                     strcpy(info->name,menuName);
                     obj.data = (char*)info;
                     itfc.macroPath = path;
                     itfc.macroName = command;
                     itfc.obj = &obj;
                     itfc.win = win;
                     if(itfc.win)
                        itfc.win->cacheProc = false;
                     ProcessMacroStr(&itfc,text);
                   //  delete [] info->name;
                     delete[] text; 
                  }   
               }
               GetDirectory(newPath);
               if(CompareDirectories(newPath,path))
                  SetDirectory(oldPath);      
            }
         }
      }
   // User menu command
      else if(code >= OBJECT_UD_MENUS_START)
      {
         int objNr = (code-OBJECT_UD_MENUS_START)/100;
         int menuNr = code - OBJECT_UD_MENUS_START - objNr*100;
			const int LOCAL_STR_LENGTH = 100;
         char str[LOCAL_STR_LENGTH];

 //        TextMessage("Menu selected %d %d \n",objNr,menuNr);

         ObjectData* obj = win->widgets.findBySequentialNr(objNr);

      // Check to see if non active controls have been blocked
         if(win->blockNonActiveCtrl && !obj->active)
            return(1);

        if(obj && obj->data)
         {
            MenuInfo* info = (MenuInfo*)obj->data;
            CText proc;

            int i;
				enum UserMenuMode {userMenu, userMacroMenu, userFolderMenu} userMode;
            for(i = 0 ; i <= menuNr; i++)
            {
               if(!strcmp(info->label[i],"\"user menu\""))
					{
						userMode = userMenu;
                  break;
					}
               else if(!strcmp(info->label[i],"\"user macro menu\""))
					{
						userMode = userMacroMenu;
                  break;
					}
               else if(!strcmp(info->label[i],"\"user folder menu\""))
					{
						userMode = userFolderMenu;
                  break;
					}
            }

            if(userMode == userMenu) // User defined macros menu (run the selected macro)
            {
               HMENU menu = GetMenu(hWnd);
               GetMenuString(menu,code,str,LOCAL_STR_LENGTH,MF_BYCOMMAND);
					int len = strlen(str);
					if(str[len-1] == ' ') // Remove the space if present and replace with _menu
					{                     // This is a special case where the menu option is calling
						str[len-1] = '\0'; // another macro of the same name as we would like to show in the menu
						strcat(str,"_menu");
					}
               strcat(str,".mac"); 
            }
            else if(userMode == userMacroMenu) // User defined macros menu (run the selected macro) - note shift to load option
            {
               CText path;
               CText oldPath;

               HMENU menu = GetMenu(hWnd);
               GetMenuString(menu,code,str,LOCAL_STR_LENGTH,MF_BYCOMMAND);
               if (!strcmp(str, "Macro Help"))
               {
                  strcpy(str, "MacroHelp");
               }

               path.Assign(info->cmd[i]);
               path.RemoveQuotes();
               Interface itfc;
               ReplaceVarInString(&itfc, path);
               GetDirectory(oldPath);
               SetDirectory(path);

					int len = strlen(str);
					if(str[len-1] == ' ') // Remove the space if present and replace with _menu
					{                     // This is a special case where the menu option is calling
						str[len-1] = '\0'; // another macro of the same name as we would like to show in the menu
						strcat(str,"_menu");
					}
               strcat(str,".mac"); 
					if(str[0] != '\0')
					{
						proc.Format("menu(%d,%s)",win->nr,info->name);
						controlProcRunning = true;
						ProcessMacroStr(LOCAL,win,obj,str,"",proc.Str(), win->macroName, win->macroPath);
						controlProcRunning = false;
				      return(1);
					}
            }
				else if(userMode == userFolderMenu) // User define folder menu (run the macro in the selected folder with the same name)
				{
					CText path;
               CText oldPath;

               HMENU menu = GetMenu(hWnd);
               GetMenuString(menu,code,str,LOCAL_STR_LENGTH,MF_BYCOMMAND);
               if (!strcmp(str, "Macro Help"))
               {
                  strcpy(str, "MacroHelp");
               }
					int len = strlen(str);
               path.Assign(info->cmd[i]); 
               path.RemoveQuotes();
               Interface itfc;
               ReplaceVarInString(&itfc, path);
               GetDirectory(oldPath);
               SetDirectory(path); 
					// See if its a macros
				   strcat(str,".mac"); 
					if(IsFile(str))
					{
						proc.Format("menu(%d,%s)",win->nr,str);
						controlProcRunning = true;
						ProcessMacroStr(LOCAL,win,obj,str,"",proc.Str(), win->macroName, win->macroPath);
						controlProcRunning = false;
						return(1);
					}
					// See if its a folder with the macro of the same name inside
					str[strlen(str)-4] = '\0'; // Remove extension
              // Interface itfc;
              // ReplaceVarInString(&itfc,path); Moved up 
				   path = path + "\\";
					path = path + str;
               SetDirectory(path); 
               strcat(str,".mac"); 
					if(IsFile(str))
					{
						proc.Format("menu(%d,%s)",win->nr,str);
						controlProcRunning = true;
						ProcessMacroStr(LOCAL,win,obj,str,"",proc.Str(), win->macroName, win->macroPath);
						controlProcRunning = false;
					}
					return(1);
				}
				else // Normal command
            {
               if(menuNr >= info->nrItems)
                  return(1);
		         strncpy_s(str,LOCAL_STR_LENGTH,info->cmd[menuNr],_TRUNCATE);
            }

         // Edit the macro if the shift key is down
            if((GetKeyState(VK_SHIFT) & 0x8000) &&  !(GetKeyState(VK_CONTROL) & 0x8000))
            {
               CText path;
               CText oldPath;
               CText newPath;
               char pathBak[MAX_PATH];
               char nameBak[MAX_PATH];
               bool alreadyLoaded = false;
                     
               path.Assign(info->cmd[i]); 
               path.RemoveQuotes();
               Interface itfc;
               ReplaceVarInString(&itfc,path);  

               GetDirectory(oldPath);
               SetDirectory(path); 

            // Make sure an editor is present
               if(!curEditor)
               {
                  CText cmd;
                  if(ProcessMacroStr(1,NULL,NULL,"editorWin.mac","","","editorWin.mac","") == ERR)
                      return(1);
               }


      //         if((response = curEditor->CheckForUnsavedEdits(ep->nr)) == IDCANCEL)
      //            return;

            // Save current editor file name
               strncpy_s(pathBak,MAX_PATH,curEditor->edPath,_TRUNCATE); 
               strncpy_s(nameBak,MAX_PATH,curEditor->edName,_TRUNCATE);

            // Set file path and name
               strncpy_s(curEditor->edName,MAX_PATH,str,_TRUNCATE);
               strncpy_s(curEditor->edPath,MAX_PATH,path.Str(),_TRUNCATE);

            // Check to see if we already have an loaded file of this name in the editor
               if(IsEditFileAlreadyLoaded(curEditor) >= 0) // (2.2.7)
               {
                  alreadyLoaded = true;
                  if(YesNoDialog(MB_ICONWARNING,0,"Warning","File already loaded\rDo you want to load a read-only copy?") == IDNO)
                  {
                     strncpy_s(curEditor->edPath,MAX_PATH,pathBak,_TRUNCATE); // Restore file name
                     strncpy_s(curEditor->edName,MAX_PATH,nameBak,_TRUNCATE);
                     return(1);
                  }
               }

			   // Load macro into editor
               if(!curEditor->edParent->parent->winParent->keepInFront)
                  ChangeGUIParent(curEditor->edWin);
		         SetCursor(LoadCursor(NULL,IDC_WAIT));
               LoadEditorGivenPath(path.Str(),str);  

            // Note if file already loaded
               if(alreadyLoaded) // (2.2.7)
                  curEditor->readOnly = true; 
               else
                  curEditor->readOnly = false; 
               SetEditTitle(); 

               AddFilenameToList(procLoadList,path.Str(), str);
				   SetCursor(LoadCursor(NULL,IDC_ARROW));
            }
		      else if(str[0] != '\0')
		      {
		         proc.Format("menu(%d,%s)",win->nr,info->name);
               controlProcRunning = true;
		         ProcessMacroStr(LOCAL,win,obj,str,"",proc.Str(), win->macroName, win->macroPath);
               controlProcRunning = false;
         //      SetFocus(wnd); // Restore focus - prevents preferences from coming to front
		      }
            return(1);
         }
      }

  // A normal menu command or accelerator
      else 
      {
         int objNr = (code-MENU_BASE)/100;
         int menuNr = (code-MENU_BASE) - objNr*100;
			const int LOCAL_STR_LENGTH = 100;
         char str[LOCAL_STR_LENGTH];

         ObjectData* obj = win->widgets.findBySequentialNr(objNr);

      // Check to see if non active controls have been blocked
         if(win->blockNonActiveCtrl && !obj->active)
            return(1);

         if(obj && obj->data)
         {
            MenuInfo* info = (MenuInfo*)obj->data;
            CText proc;

		      strncpy_s(str,LOCAL_STR_LENGTH,info->cmd[menuNr],_TRUNCATE);

		      if(str[0] != '\0')
		      {
               Interface itfc;

		         proc.Format("menu(%d,%s)",win->nr,info->name);

               itfc.macroName = win->macroName;
               itfc.macroPath = win->macroPath;
               itfc.procName = proc.Str();
               itfc.win = win;
          /*     if(itfc.win)
                  itfc.win->cacheProc = false; */
               itfc.varScope = LOCAL;
               itfc.startLine = obj->cmdLineNr;
					itfc.menuInfo = info;
					itfc.menuNr = menuNr;
               itfc.obj = obj;
               itfc.objID = obj->nr();

               controlProcRunning = true;
		         ProcessMacroStr(&itfc,str);
               controlProcRunning = false;
             //  SetFocus(obj->hWnd); // Restore focus - prevents preferences from coming to front
		      }
            return(1);
         }
      }
   }

   if(HIWORD(wParam) == 0) // Normal command
   {
	   ProcessControlCommands(hWnd,(HWND)lParam,(int)LOWORD(wParam));
   }
   else if(HIWORD(wParam) == CBN_DROPDOWN) // List item in combox box about to be displayed
   {                                       // Make sure the width of the list is enough to contain the longest string  
      int idComboBox = (int) LOWORD(wParam);  // identifier of combo box 
      HWND hwndComboBox = (HWND) lParam;       // handle of combo box 
      long entries = SendMessage(hwndComboBox,CB_GETCOUNT,0,0);
      long maxLength = -1;
   //   COMBOBOXINFO info = { sizeof(COMBOBOXINFO) };
   //   GetComboBoxInfo(hwndComboBox,&info);
      HFONT listFont = (HFONT)SendMessage(hwndComboBox,WM_GETFONT,NULL,NULL);
      SIZE te; 
      HDC hdc = GetDC(hwndComboBox);
      HFONT oldFont = (HFONT)SelectObject(hdc,listFont);

      if(entries > 0)
      {
         long i;
         for(i = 0; i < entries; i++)
         {
            long length = SendMessage(hwndComboBox,CB_GETLBTEXTLEN,(WPARAM)i,0);
            char *str = new char[length+1];
            SendMessage(hwndComboBox,CB_GETLBTEXT,(WPARAM)i,(LPARAM)str);
            GetTextExtentPoint32(hdc, str, strlen(str), &te);
         //   TextMessage("String = %s Length = %ld\n",str,te.cx);
            delete [] str;
            if(te.cx > maxLength)
               maxLength = te.cx;
         }
      }
      SelectObject(hdc,oldFont);
      ReleaseDC(hwndComboBox,hdc);
     // TextMessage("Max length = %ld\n",maxLength);

      SendMessage(hwndComboBox,CB_SETDROPPEDWIDTH,(WPARAM)maxLength+7,(LPARAM)0); // With margin padding

   }
   else if(HIWORD(wParam) == CBN_SELENDOK) // User selected a list item
   {
	   short index = (short)SendMessage((HWND)lParam,CB_GETCURSEL,0,0);
	   if(index != CB_ERR)
	   {
	      char data[255];
	      SendMessage((HWND)lParam,CB_GETLBTEXT,(WPARAM)index,(LPARAM)data);	
	      SetWindowText((HWND)lParam,data);	       
		   ProcessControlCommands(hWnd,(HWND)lParam,(int)LOWORD(wParam));
         return(0);
	   }
   }
   else if(HIWORD(wParam) == CBN_SELCHANGE) // User selected a list item
   {
      return(0);
   }
   else if(HIWORD(wParam) == GETMESSAGE) // Prospa has sent an event to GUI (SendMessageToGUI)
   {
	   ObjectData *obj;
	   char *str;
	   char proc[MAX_STR];
	 //  bool inCLIback = itfc->inCLI;
   	
   // Check to see if this window has a getmessage object
	   obj = win->widgets.findByType(GETMESSAGE);
	   if(obj)
	   {
         if(obj->data) delete [] obj->data;
		   obj->data = new char[strlen((char*)lParam)+1];
		   strcpy(obj->data,(char*)lParam);
   		
		   str = obj->command;
		   if(str[0] != '\0')
		   {
			   sprintf(proc,"getmessage(%d,%d)",win->nr,obj->nr());
		      ProcessMacroStr(LOCAL,win,obj,str,"",proc, win->macroName, win->macroPath);
	//		   itfc->inCLI = inCLIback;
		   }
	   }
   }
   else
   {
      if(lParam == 0) // Only process main gui window messages (ignore any control with these IDs)
      {
         switch(LOWORD(wParam))
	      {
            case(ID_HIDE_WINDOW):
            {
	            ShowWindow(hWnd,SW_HIDE);
	            break;
            }
            case(ID_SHOWHIDE_GUI):
            {
               if(AnyGUIWindowVisible())
                  HideGUIWindows();
               else
                  ShowGUIWindows(SW_SHOWNOACTIVATE);
               break;
            }
         }
      }
   }
   return(1);
}

/******************************************************************************
* Run callback macros for each control which contain one 
******************************************************************************/

void ProcessControlCommands(HWND hWnd, HWND hwndObj, int objID)
{
   char *str;
   WinData *win;
	ObjectData *obj;
   Interface itfc;
   extern bool gAbortSet;

// Get the window and object class instances for this command
   win = GetWinDataClass(hWnd);
   if(!win) return;
   obj = win->widgets.findByWin(hwndObj);
   if(!obj) return;

// Don't allow normal (nondebugger) controls to work if we are waiting  
// for the user to select the next debug option
 //  if(gDebug.enabled && win->debugging)
   if(gDebug.enabled)
      return;

// Initialise the macro source object
   itfc.macroName = win->macroName;
   itfc.macroPath = win->macroPath;
   itfc.win = win;
   itfc.obj = obj;
//   assert(objID == obj->nr());
   itfc.objID = obj->nr();
   itfc.varScope = LOCAL;
   itfc.startLine = obj->cmdLineNr;

   if(gDebug.mode != "off")
      itfc.debugging = true;

// Don't run a control macro if one is already running (unless its a dialog)
   if(controlProcRunning > 0 && !dialogDisplayed)
   {
   //   printf("Callback running ignoring - time = %lf\n",GetMsTime());
      return;
   }

// Check to see if non active controls have been blocked
   if(win->blockNonActiveCtrl && !obj->active)
   {
    // printf("Controls are blocked ignoring - time = %lf\n",GetMsTime());
      return;
   }
 
// Run the appropriate command
   switch(obj->type)
   {
      case(COLORBOX):
      case(BUTTON):
      {
			WinData::SetGUIWin(win);
		   str = win->GetObjCommand(objID);
			if(NULL == str)
			{
				break;
			}
		   if(str[0] != '\0')
		   {
            HWND wnd = GetFocus(); // Save button focus
            //if(curEditor && curEditor->debug)
            //   SetFocus(curEditor->edWin);
		   //   SetFocus(hWnd); // So escape abort can work if run button is disabled
		      itfc.procName.Format("button(%d,%d)",win->nr,obj->nr());
            itfc.startLine = obj->cmdLineNr;
            int wvc = win->validationCode;
            int ovc = obj->validationCode;
            controlProcRunning = true;
          //  printf("Starting macro from control %hd\n",obj->nr());
            short nr = obj->nr();
        //    TextMessage("Starting macro from control %hd\n",obj->nr());
		      ProcessMacroStr(&itfc,1,str);

            if (itfc.win) // Make sure window not destroyed
            {
               win = GetWinDataClass(hWnd);
               if (win)
               {
                  obj = win->widgets.findByWin(hwndObj);
                  if (obj)
                  {
                     if (win->validationCode == wvc && obj->validationCode == ovc && win->abortID == obj->nr())
                        gAbortMacro = true; // Abort if this is an abort control
                  }
               }
            }
          //  printf("Finished macro from control %hd\n",nr);

            // This version doesn't work as win or obj may be corrupted but non-zero
            //   if (win && obj && win->validationCode == wvc && obj->validationCode == ovc && win->abortID == obj->nr())
            //      gAbortMacro = true; // Abort if this is an abort control
        
            controlProcRunning = false;
      //      SetFocus(wnd); // Restore focus - prevents preferences from coming to front
		   }
		   break;
		}
      case(TEXTBOX):
      {
		   str = win->GetObjCommand(objID);
		   if(str[0] != '\0')
		   {
		      itfc.procName.Format("textbox(%d,%d)",win->nr,obj->nr());
            controlProcRunning = true;
		      ProcessMacroStr(&itfc,1,str);
            controlProcRunning = false;
		   }
		   break;
		}
		case(RADIO_BUTTON):
		{
		   RadioButtonInfo *info;
		   info = (RadioButtonInfo*)(obj->data);
         bool erase;
		   for(int i = 0; i < info->nrBut; i++)
		   {
		      if(info->hWnd[i] == hwndObj)
               erase = 0;
            else
               erase = 1;
             SendMessage(info->hWnd[i],BM_SETCHECK,!erase,0);
          // Draw the focus rect
            HDC hdc = GetDC(obj->hwndParent);
            obj->DrawFocusRect(hdc,erase,info->hWnd[i]);
            ReleaseDC(obj->hwndParent,hdc); 
		   }

       // Redraw the control
         MyUpdateWindow(obj->hwndParent);  

		   str = win->GetObjCommand(objID);
		   if(str[0] != '\0')
		   {
		      itfc.procName.Format("radiobutton(%d,%d)",win->nr,obj->nr());
            controlProcRunning = true;
		      ProcessMacroStr(&itfc,1,str);
            controlProcRunning = false;
		   }
		   break;
		}
		case(TEXTMENU):
		{
		   str = win->GetObjCommand(objID);
		   if(str[0] != '\0')
		   {
		      itfc.procName.Format("textmenu(%d,%d)",win->nr,obj->nr());
            controlProcRunning = true;
		      ProcessMacroStr(&itfc,1,str);
            controlProcRunning = false;
		   }
		   break;
		}
		case(CHECKBOX):
		{
		   str = win->GetObjCommand(objID);
		   if(str[0] != '\0')
		   {
		      itfc.procName.Format("checkbox(%d,%d)",win->nr,obj->nr());
            controlProcRunning = true;
		      ProcessMacroStr(&itfc,1,str);
            controlProcRunning = false;
		   }
		   break;
      }
      case(TOOLBAR):
		{
         ToolBarInfo *info = (ToolBarInfo*)obj->data;
         str = info->item[objID].cmd.Str();
		   if(str[0] != '\0')
		   {
		      itfc.procName.Format("toolbar(%d,%d,%d)",win->nr,obj->nr(),objID);
            controlProcRunning = true;
		      ProcessMacroStr(&itfc,1,str);
            controlProcRunning = false;
		   }
		   break;
      }
   }
   if(gDebug.mode != "off")
   {
      gDebug.mode = "runto";
      gDebug.step = true;
      curEditor->debugLine = -1;
      SendMessage(curEditor->edWin,EM_HIDESELECTION ,true,0);
   }
}


/***************************************************************************
   Event procedure for the track-bar (slider)
   Note the global variable here: sliderMouseUpAbort. 
***************************************************************************/

bool ProcessScrollEvents(LPARAM lParam, WPARAM wParam, HWND hWnd, WinData* win)
{
   char* str;
   ObjectData* obj;

   obj = win->widgets.findByWin((HWND)lParam);

   if (!obj)
      return(false);

   Interface itfc;

   // Initialise the macro source object
   itfc.macroName = win->macroName;
   itfc.macroPath = win->macroPath;
   itfc.win = win;
   itfc.obj = obj;
   itfc.objID = obj->nr();
   itfc.varScope = LOCAL;
   itfc.startLine = obj->cmdLineNr;
   str = obj->command;



   if(obj->type == SLIDER)
   {
      itfc.procName.Format("slider(%d,%d)",win->nr,obj->nr());

	   switch(LOWORD(wParam))
	   {
	      case(TB_TOP):
	      case(TB_BOTTOM):
	      case(TB_THUMBPOSITION):
	      case(TB_THUMBTRACK):
	      case(TB_PAGEUP):
	      case(TB_PAGEDOWN):
	      {
				if(str[0] != '\0')
				{
               if(!controlProcRunning || dialogDisplayed)
               {
						if(gDebug.mode != "off")
							itfc.debugging = true;
                  controlProcRunning = true;
		            ProcessMacroStr(&itfc,str);	
						SetFocus(obj->hWnd);
                  controlProcRunning = false;
						if(gDebug.mode != "off")
						{
							gDebug.mode = "runto";
							gDebug.step = true;
							curEditor->debugLine = -1;
							SendMessage(curEditor->edWin,EM_HIDESELECTION ,true,0);
						}
               }
				} 
				break;
			}
	      case(TB_LINEDOWN):
	      case(TB_LINEUP):	
	      {
				if(str[0] != '\0')
				{
               if(!controlProcRunning || dialogDisplayed)
               {
						if(gDebug.mode != "off")
							itfc.debugging = true;
                  controlProcRunning = true;
		            ProcessMacroStr(&itfc,str);
						SetFocus(obj->hWnd);
                  controlProcRunning = false;
						if(gDebug.mode != "off")
						{
							gDebug.mode = "runto";
							gDebug.step = true;
							curEditor->debugLine = -1;
							SendMessage(curEditor->edWin,EM_HIDESELECTION ,true,0);
						}
               }
				} 
				break;
			}	            				
	   }
      // Check for the case where an event has occurred but the 
      // mouse button has been released. This happens if too much
     // time passes during the processing of the event
      if (!GetAsyncKeyState(VK_LBUTTON) && sliderMouseButtonDn)
      {
         if (obj->hWnd && !sliderMouseButtonUp) // Prevents an infinite loop
         {                                     // By just sending the message once
            sliderMouseButtonUp = true;
            sliderMouseButtonDn = false;
            SendMessage(obj->hWnd, WM_LBUTTONUP, NULL, NULL);
         }
      }
	   return(true);
	}
   else if(obj->type == PANEL) // Panel
   {
      obj->ProcessPanelScrollEvent(wParam,lParam);
   }
 
   return(false);
}

/***************************************************************************
   Check is the mouse is inside a window specified by hWnd
***************************************************************************/

bool IsMouseInWindow(HWND hWnd)
{
   RECT windowRect;
   GetWindowRect(hWnd, &windowRect);
   POINT cursorPos;
   GetCursorPos(&cursorPos);
   return(PtInRect(&windowRect, cursorPos ));
}

/***************************************************************************
   Event procedure for buttons
***************************************************************************/

LRESULT CALLBACK ButtonEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   LRESULT r = 0;
   WinData *win = 0;
   ObjectData *obj = 0;
   bool inPanel = 0;

   HWND parWin = GetParent(hWnd);
   win = GetWinDataClass(parWin);
   if(!win)
   {
      HWND parWin2 = GetParent(parWin);
      win = GetWinDataClass(parWin2);
   }
   if(!win || win->inRemoval) return(1);            
   obj = win->widgets.findByWin(hWnd); // Find object selected
   if(!obj)
      return(CallWindowProc(OldButtonProc,hWnd, messg, wParam, lParam));

   switch(messg)
	{
      case(WM_DROPFILES):
      {
         CText path,file,ext;
         if(GetDropFileInfo((HDROP)wParam,path,file,ext,0) == OK)
         {
            char proc[50];
            sprintf(proc,"button(%d,%d)",win->nr,obj->nr());
            CText cmd;
            cmd.Format("%s(\"%s\",\"%s\",\"%s\")",obj->dragNDropProc.Str(),path.Str(),file.Str(),ext.Str());
            ProcessMacroStr(LOCAL,win,obj,cmd.Str(),"",proc, win->macroName, win->macroPath);
            DragFinish((HDROP)wParam);
            return(0);
         }
         return(0);
      }

      case(WM_PAINT):
      {
         if(obj)
         {
            r = CallWindowProc(OldButtonProc,hWnd, messg, wParam, lParam);
				obj->decorate(parWin);
            return(r);
         }
      }

      case(WM_SETFOCUS):
      {
         win->objWithFocus = obj;
         HWND hwndLoseFocus = (HWND) wParam;
         ObjectData *objOld = win->widgets.findByWin(hwndLoseFocus);

         if(hwndLoseFocus && !obj->IsAWindow(hwndLoseFocus))
         {
         // Remove focus rectangle from object loosing focus (checkbox, radiobutton or colorbox)
            if(objOld && (objOld->type == CHECKBOX || objOld->type == RADIO_BUTTON || objOld->type == COLORBOX || objOld->type == UPDOWN))
            {
               HDC hdc = GetDC(objOld->hwndParent);
               objOld->DrawFocusRect(hdc,1);
               ReleaseDC(objOld->hwndParent,hdc);
            }
         }
      // Focus is in a radio button but another has been selected using
      // the tab button or the mouse button so erase the current one.
         //if(obj && objOld && (obj == objOld) && (obj->type == RADIO_BUTTON))
         //{
         //   HDC hdc = GetDC(obj->hwndParent);
         //   obj->DrawFocusRect(hdc,1);
         //   ReleaseDC(obj->hwndParent,hdc);
         //}

      // Focus is now in a checkbox so draw the focus rect
         if(obj && (obj->type == CHECKBOX || obj->type == COLORBOX))
         {
            HDC hdc = GetDC(obj->hwndParent);
            obj->DrawFocusRect(hdc,0);
            ReleaseDC(obj->hwndParent,hdc);
         }
         break;
      }

      case(WM_KEYDOWN): // Keyboard has been pressed 
      {
         char  code = (CHAR)wParam;    // character code 

         if(code == VK_F2)
         {
            if(AnyGUIWindowVisible())
               HideGUIWindows();
            else
               ShowGUIWindows(SW_SHOWNOACTIVATE);
         }

         if(code == VK_RETURN && obj->type == BUTTON)  // Check for return 
         { 
            if(win->defaultID != -1)
               SendMessage(win->hWnd,WM_KEYDOWN,(WPARAM)VK_RETURN,0); // Send return to default button
            else
               SendMessage(win->hWnd,WM_COMMAND,(WPARAM)obj->nr(),(LPARAM)obj->hWnd); // Process button command
            return(0);               
         }
         if(code == VK_ESCAPE && obj->type == BUTTON) // Check for escape 
         { 
            SendMessage(win->hWnd,WM_KEYDOWN,(WPARAM)VK_ESCAPE,0); // Send escape to parent window
            return(0);               
         } 
         if(code == VK_TAB && !IsKeyDown(VK_CONTROL)) // Check for tab - if found move to the next/last tabbable object
         { 
            obj->processTab();
            return(0);
         } 
         if(code == VK_RETURN) // Execute call back with return key
         {
            if(obj->type == RADIO_BUTTON)
            { 
           // Run radiobutton macro
               char *str;
               char proc[MAX_STR];
		         str = obj->command;
		         if(str[0] != '\0')
		         {
		            sprintf(proc,"radiobutton(%d,%d)",win->nr,obj->nr());
		            ProcessMacroStr(LOCAL,win,obj,str,"",proc, win->macroName, win->macroPath);
		         }               
            }
            else if(obj->type == CHECKBOX)
            { 
           // Run checkbox macro
               char *str;
               char proc[MAX_STR];
		         str = obj->command;
		         if(str[0] != '\0')
		         {
		            sprintf(proc,"checkbox(%d,%d)",win->nr,obj->nr());
		            ProcessMacroStr(LOCAL,win,obj,str,"",proc, win->macroName, win->macroPath);
		         }               
            }
            else if(obj->type == COLORBOX)
            { 
           // Run colorbox macro
               char *str;
               char proc[MAX_STR];
		         str = obj->command;
		         if(str[0] != '\0')
		         {
		            sprintf(proc,"colorbox(%d,%d)",win->nr,obj->nr());
		            ProcessMacroStr(LOCAL,win,obj,str,"",proc, win->macroName, win->macroPath);
		         }               
            }
            return(0);
         }
         if(code == VK_SPACE)
         {
            if(obj->type == CHECKBOX) // Toggle checkbox with space bar
            {
               if(SendMessage(obj->hWnd,BM_GETCHECK,0,0))
                  SendMessage(obj->hWnd,BM_SETCHECK,0,0);
               else
                  SendMessage(obj->hWnd,BM_SETCHECK,1,0);  

               return(0);
		      }
            else
               return(0);
         }
		   if((code >= VK_LEFT && code <= VK_DOWN) && obj->type == RADIO_BUTTON) // Arrow keys on radio button moves the selection
		   {	
         // Find out which radio button is checked
	         RadioButtonInfo *info = (RadioButtonInfo*)obj->data; 
            int i,index;
	         for(i = 0; i < info->nrBut; i++)
	         {
	            if(SendMessage(info->hWnd[i],BM_GETCHECK,0,0))
                  break;
	         }
            index = i;

         // Erase the focus rect from the current radio button
            HDC hdc = GetDC(obj->hwndParent);
            obj->DrawFocusRect(hdc,1);
            ReleaseDC(obj->hwndParent,hdc);

            if(code == VK_LEFT || code == VK_UP) 
               index--;
            else if(code == VK_RIGHT || code == VK_DOWN)
               index++;

         // Check for invalid indices
           if(index < 0)
              index = info->nrBut-1;
           else if(index >= info->nrBut)
              index = 0;

        // Select the new radio button and draw the focus rect
            for(i = 0; i < info->nrBut; i++)
            {
               if(i != index)
                  SendMessage(info->hWnd[i],BM_SETCHECK,0,0);
               else
               {
                  SendMessage(info->hWnd[i],BM_SETCHECK,1,0);
                  { // Draw the focus rect
                     HDC hdc = GetDC(obj->hwndParent);
                     obj->DrawFocusRect(hdc,0);
                     ReleaseDC(obj->hwndParent,hdc);
                  }
               }
            }

            UpdateObject(obj);

         // Run radiobutton macro
            char *str;
            char proc[MAX_STR];
		      str = obj->command;
		      if(str[0] != '\0')
		      {
		         sprintf(proc,"radiobutton(%d,%d)",win->nr,obj->nr());
		         ProcessMacroStr(LOCAL,win,obj,str,"",proc, win->macroName, win->macroPath);
		      }
		   }
         break;
      } 
   // See if user wants to get control info
   // Also indicate that the button has been selected
   // this is used by ownerdraw routine to update
   // the button color.
      case(WM_LBUTTONDOWN): 
      {
         if(obj->type == BUTTON)
         {
            PushButtonInfo* info = (PushButtonInfo*)obj->data;
            if(!info->selected && win7Mode)
            {
               info->selected = true;
            }
         }
         if(wParam & MK_CONTROL) 
         {
            DisplayControlInfo(win,obj);
            return(0);
         }
         break;
      } 

   // Convert double clicks into single clicks
      case(WM_LBUTTONDBLCLK): 
      {
         if(obj->type == BUTTON)
         {
            SendMessage(hWnd, WM_LBUTTONDOWN, wParam,lParam);
            return(0);
         }
      }
 
   // The select flag is zeroed when button goes up this ensures 
   // the button is drawn non-selected
      case(WM_LBUTTONUP):
      {
         if(obj->type == BUTTON)
         {
            PushButtonInfo* info = (PushButtonInfo*)obj->data;

            if(info->selected && win7Mode)
               info->selected = false;
         }
         break;
      }

    //If moving over the button start a tracker to check for
    //leaving the button but only on the first move event.
    //Also change the button color.
    //Leaving the button will halt the tracker
      case(WM_MOUSEMOVE):
      {
         if(obj->type == BUTTON)
         {
            PushButtonInfo* info = (PushButtonInfo*)obj->data;
            if(!info->hovering && win7Mode)
            {
               TRACKMOUSEEVENT tme;
               tme.cbSize = sizeof(tme);
               tme.hwndTrack = hWnd;
               tme.dwFlags = TME_LEAVE;
               tme.dwHoverTime = HOVER_DEFAULT;
               TrackMouseEvent(&tme);
               info->hovering = true;
					InvalidateRect(hWnd,NULL,false);
            }
         }
         break;
      }

   // If leaving the button redraw the button and 
   // reset selected and hovering flags
      case(WM_MOUSELEAVE):
      {
         if(obj->type == BUTTON)
         {
            PushButtonInfo* info = (PushButtonInfo*)obj->data;
            if(info->hovering && win7Mode)
            {
               info->hovering = false;
               info->selected = false;
					InvalidateRect(hWnd,NULL,false);
            }
         }
         break;
      }
   }

   return(CallWindowProc(OldButtonProc,hWnd, messg, wParam, lParam));
}


/***************************************************************************
   Event procedure for the debugger strip
***************************************************************************/

LRESULT CALLBACK DebugStripEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   LRESULT r = 0;
   WinData *win = 0;
   ObjectData *obj = 0;
   
   HWND parWin = GetParent(hWnd);
   win = GetWinDataClass(parWin);
   if(!win || win->inRemoval) return(1);            
   obj = win->widgets.findByWin(hWnd); // Find object selected
   if(!obj)
      return(CallWindowProc(OldDebugStripProc,hWnd, messg, wParam, lParam));
      
   switch(messg)
	{
      case(WM_PAINT):
      {
         if(obj)
         {
				r = CallWindowProc(OldDebugStripProc,hWnd, messg, wParam, lParam);
            obj->decorate(parWin);
				return r;
         }
			break;
      }

   // See if user wants to get control info
      case(WM_LBUTTONDOWN): 
      {
      } 
   }

   return(CallWindowProc(OldDebugStripProc,hWnd, messg, wParam, lParam));
}


/***************************************************************************
   Event procedure for the HTML object
***************************************************************************/

LRESULT CALLBACK HTMLEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   LRESULT r = 0;
   WinData *win = 0;
   ObjectData *obj = 0;

   HWND parWin = GetParent(hWnd);
   win = GetWinDataClass(parWin);
   if(!win || win->inRemoval) return(1);            
   obj = win->widgets.findByWin(hWnd); // Find object selected
   if(!obj)
      return(CallWindowProc(OldHTMLProc,hWnd, messg, wParam, lParam));

   switch(messg)
	{
      case(WM_PAINT):
      {
         if(obj)
         {
            r = CallWindowProc(OldHTMLProc,hWnd, messg, wParam, lParam);
				obj->decorate(parWin);
				//if (!obj->selected_)
				//{
				//	HDC hdc = GetDC(obj->hwndParent);
    //           obj->DrawRect(hdc);
    //           ReleaseDC(obj->hwndParent,hdc);
    //           UpdateObject(obj->hWnd);
    //        }
            return(r);
         }
      }

      case(WM_SETFOCUS):
      {
         win->objWithFocus = obj;
         HWND hwndLoseFocus = (HWND) wParam;
         ObjectData *objOld = win->widgets.findByWin(hwndLoseFocus);

         if(hwndLoseFocus && !obj->IsAWindow(hwndLoseFocus))
         {
         // Remove focus rectangle from object loosing focus (checkbox, radiobutton or colorbox)
            if(objOld && (objOld->type == CHECKBOX || objOld->type == RADIO_BUTTON || objOld->type == COLORBOX || objOld->type == UPDOWN))
            {
               HDC hdc = GetDC(objOld->hwndParent);
               objOld->DrawFocusRect(hdc,1);
               ReleaseDC(objOld->hwndParent,hdc);
            }
         }
      }
   }

   return(CallWindowProc(OldHTMLProc,hWnd, messg, wParam, lParam));
}


LRESULT CALLBACK GridEventProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId = 0;

   HWND parWin = GetParent(hWnd);
   WinData* win = GetWinDataClass(parWin);
   if(!win || win->inRemoval) return(1);            
   ObjectData* obj = win->widgets.findByWin(hWnd); // Find object selected
   BabyGrid* g = 0;

	if (obj)
	{
		 g =  (BabyGrid*)obj->data;
		 if (g)
		//update the grid width and height variable
		{
			RECT rect;
			GetClientRect(hWnd,&rect);
			g->setGridWidth(rect.right - rect.left);
			g->setGridHeight(rect.bottom - rect.top);
		}
      else
         return DefWindowProc(hWnd, message, wParam, lParam);
	}
   else
      return DefWindowProc(hWnd, message, wParam, lParam);

   switch(message)
	{
      case(WM_PAINT):
      {
			g->handle_WM_PAINT();
			obj->decorate(parWin);
			break;
      }

		case(WM_ERASEBKGND): // Modify the tab control to have button background
			{
				return 1;
			}

		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			// Parse the menu selections:
			switch (wmId)
			{
			case 1:
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;

		case WM_SETTEXT: 
			g->handle_WM_SETTEXT((char*)lParam);
			break;

		case WM_ENABLE:
			g->handle_WM_ENABLE((BOOL)wParam);
			break;

		case WM_MOUSEMOVE:  
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);
				g->handle_WM_MOUSEMOVE(x,y);
			}
			break;

		case WM_LBUTTONUP:
			g->handle_WM_LBUTTONUP();
			break;

		case WM_LBUTTONDOWN:
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);
				g->handle_WM_LBUTTONDOWN(x,y);
			}
			break;

		case WM_GETDLGCODE:
			return g->handle_WM_GETDLGCODE(wParam);
			break;

		case WM_KEYDOWN:
			return g->handle_WM_KEYDOWN(wParam, lParam);
			break;
		case WM_HSCROLL:
			g->handle_WM_HSCROLL(wParam, lParam);
			break;
		case WM_VSCROLL:
			g->handle_WM_VSCROLL(wParam, lParam);
			break;
		case WM_SETFOCUS:
			g->handle_WM_SETFOCUS();
			break;
		case WM_KILLFOCUS:	
			g->handle_WM_KILLFOCUS();
			break;
		case WM_SETFONT:
			g->handle_WM_SETFONT((HFONT)wParam);
			break;
		case WM_SIZE:
			if (g)
			{
				g->handle_WM_SIZE((int)(HIWORD(lParam)), ((int)(LOWORD(lParam))));
			}
			break;
	
	}	
	return DefWindowProc(hWnd, message, wParam, lParam);

}


//
///***************************************************************************
//   Event procedure for the grid object
//***************************************************************************/
//
//LRESULT CALLBACK GridEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
//{
//   LRESULT r = 0;
//   WinData *win = 0;
//   ObjectData *obj = 0;
//
//   HWND parWin = GetParent(hWnd);
//   win = GetWinDataClass(parWin);
//   if(!win || win->inRemoval) return(1);            
//   obj = win->objList.FindObjByWin(hWnd); // Find object selected
//      
//   switch(messg)
//	{
//      case(WM_PAINT):
//      {
//         if(obj)
//         {
//            r = CallWindowProc(OldGridProc,hWnd, messg, wParam, lParam);
//
//				if(win->displayObjCtrlNrs)
//				{
//					HDC hdc = GetDC(obj->hwndParent);
//					obj->DrawControlNumber(hdc);
//					ReleaseDC(obj->hwndParent,hdc);
//				}
//				if(win->displayObjTabNrs)
//				{
//					HDC hdc = GetDC(obj->hwndParent);
//					obj->DrawTabNumber(hdc);
//					ReleaseDC(obj->hwndParent,hdc);
//				}
//				return(r);
//         }
//      }
//
//      //case(WM_SETFOCUS):
//      //{
//      //   win->objWithFocus = obj;
//      //   HWND hwndLoseFocus = (HWND) wParam;
//      //   ObjectData *objOld = win->objList.FindObjByWin(hwndLoseFocus);
//
//      //   if(hwndLoseFocus && !obj->IsAWindow(hwndLoseFocus))
//      //   {
//      //   // Remove focus rectangle from object loosing focus (checkbox, radiobutton or colorbox)
//      //      if(objOld && (objOld->type == CHECKBOX || objOld->type == RADIO_BUTTON))
//      //      {
//      //         HDC hdc = GetDC(objOld->hwndParent);
//      //         objOld->DrawFocusRect(hdc,1);
//      //         ReleaseDC(objOld->hwndParent,hdc);
//      //      }
//      //   }
//      //}
//      case(WM_ERASEBKGND): // Modify the tab control to have button background
//      {
//			return 1;
//		}
//
//   }
//
//   return(CallWindowProc(OldGridProc,hWnd, messg, wParam, lParam));
//}
//

/***************************************************************************
   Event procedure for the group-box
***************************************************************************/

LRESULT CALLBACK GroupBoxEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   LRESULT r = 0;
   WinData *win = 0;
   ObjectData *obj = 0;
   bool inPanel = 0;
   ObjectData *nextObj = 0;


   HWND parWin = GetParent(hWnd);
   win = GetWinDataClass(parWin);
   if(!win)
   {
      HWND parWin2 = GetParent(parWin);
      win = GetWinDataClass(parWin2);
   }

   if(!win || win->inRemoval) return(1);            
   obj = win->widgets.findByWin(hWnd); // Find object selected
   if(!obj)
      return(CallWindowProc(OldGroupBoxProc,hWnd, messg, wParam, lParam));
     
   switch(messg)
	{
      case(WM_PAINT):
      {
         if(obj)
         {
				r = CallWindowProc(OldGroupBoxProc,hWnd, messg, wParam, lParam);
				obj->decorate(parWin);
            return r;
         }
      }

   // Print the window to a specified device context (e.g. savewindow)
      case(WM_PRINT): 
		{
         DRAWITEMSTRUCT dis;
         HDC hdc = (HDC) wParam;
         dis.hDC = hdc;
         SetRect(&(dis.rcItem),0,0,obj->wo,obj->ho);
         dis.itemID = obj->nr();
         dis.itemData = 1;
         dis.hwndItem = obj->hWnd;
         wParam = (WPARAM)obj->nr();
         SendMessage(GetParent(hWnd), WM_DRAWITEM, wParam, (LPARAM) &dis);
         break;
      }

      case(WM_ERASEBKGND):
      {
         return(1);
         break;
      }

   }
  
   return(CallWindowProc(OldGroupBoxProc,hWnd, messg, wParam, lParam));
            
}



/***************************************************************************
   Event procedure for the color scale
***************************************************************************/

//LRESULT CALLBACK ColorScaleEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
//{
//   LRESULT r = 0;
//   WinData *win = 0;
//   ObjectData *obj = 0;
//   
//   HWND parWin = GetParent(hWnd);
//   win = GetWinDataClass(parWin);
//   if(!win || win->inRemoval) return(1);            
//   obj = win->objList.FindObjByWin(hWnd); // Find object selected
//
//   switch(messg)
//	{
//      case(WM_PAINT):
//      {
//         if(obj)
//         {
//
//				//FIXME: OldColorScaleProc is never defined or assigned prior to this call. Commenting it out to be safe, it's not doing anything anyways.
//            //r = OldColorScaleProc(hWnd, messg, wParam, lParam);
//
//            if(obj->selected)
//            {
//               HDC hdc = GetDC(obj->hwndParent);
//               obj->DrawSelectRect(hdc);
//            }
//
//            if(win->displayObjCtrlNrs)
//            {
//               HDC hdc = GetDC(obj->hwndParent);
//               obj->DrawControlNumber(hdc);
//               ReleaseDC(obj->hwndParent,hdc);
//            }
//
//            if(win->displayObjTabNrs)
//            {
//               HDC hdc = GetDC(obj->hwndParent);
//               obj->DrawTabNumber(hdc);
//               ReleaseDC(obj->hwndParent,hdc);
//            }
//            return(r);
//         }
//      }
//   }
//   
//   return(CallWindowProc(OldColorScaleProc,hWnd, messg, wParam, lParam));
//}

/***************************************************************************
   Event procedure for the text-menu (combo-box)
***************************************************************************/

LRESULT CALLBACK TextMenuEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   LRESULT r = 0;

   HWND parWin = GetParent(hWnd);
   WinData *win = GetWinDataClass(parWin);
   if(!win)
   {
      HWND parWin2 = GetParent(parWin);
      win = GetWinDataClass(parWin2);
   }
   if(!win || win->inRemoval) return(1);            
   ObjectData *obj = win->widgets.findByWin(hWnd); // Find object selected
   if(!obj)
      return(CallWindowProc(OldTextMenuProc,hWnd, messg, wParam, lParam));

   switch(messg)
	{
      case(WM_PAINT):
      {
         if(obj)
         {
            r = CallWindowProc(OldTextMenuProc,hWnd, messg, wParam, lParam);
				obj->decorate(parWin);
            if(obj->inError && obj->visible)
            {
               obj->DrawErrorRect(parWin);
            }
            return(r);
         }
			break;
      }
      case(WM_SETFOCUS):
      {
         win->objWithFocus = obj;
         HWND hwndLoseFocus = (HWND) wParam;
         if(hwndLoseFocus && !obj->IsAWindow(hwndLoseFocus))
         {
            ObjectData *objOld = win->widgets.findByWin(hwndLoseFocus);
            if(objOld && (objOld->type == CHECKBOX || objOld->type == RADIO_BUTTON || objOld->type == COLORBOX || objOld->type == UPDOWN))
            {
               HDC hdc = GetDC(objOld->hwndParent);
               objOld->DrawFocusRect(hdc,1);
               ReleaseDC(objOld->hwndParent,hdc);
            }
         }
         break;
      }
      case(WM_LBUTTONDOWN): 
      case(WM_LBUTTONDBLCLK): 
      {
         break;
      }
      case(WM_MOUSEWHEEL):
      {
         POINT pt;
         pt.x = 3;
         pt.y = 3;
         HWND hwndEdit = RealChildWindowFromPoint(hWnd, pt);
         HWND curFocus = GetFocus();
         if(curFocus != hwndEdit)
            return(0);
         break;
      } 
   }
   return(CallWindowProc(OldTextMenuProc,hWnd, messg, wParam, lParam));
            
}

/***************************************************************************
   Event procedure for the edit-box inside the text-menu (combo-box)
***************************************************************************/

LRESULT CALLBACK TextMenuEditEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   LRESULT r = 0;
   WinData *win = 0;
   ObjectData *obj = 0;

   HWND parWin = GetParent(GetParent(hWnd));
   win = GetWinDataClass(parWin);
   if(!win)
   {
      HWND parWin2 = GetParent(parWin);
      win = GetWinDataClass(parWin2);
   }
   if(!win || win->inRemoval) return(1);            
   obj = win->widgets.findByWin(GetParent(hWnd)); // Find object selected
   if(!obj)
      return(CallWindowProc(OldTextMenuEditProc,hWnd, messg, wParam, lParam));

   switch(messg)
	{
      case(WM_PAINT):
      {
         if(obj)
         {
            r = CallWindowProc(OldTextMenuEditProc,hWnd, messg, wParam, lParam);
				obj->decorate(parWin);
				return r;
         }
			break;
      }
      case(WM_SETFOCUS):
      {
         win->objWithFocus = obj;
         HWND hwndLoseFocus = (HWND) wParam;
         if(hwndLoseFocus && !obj->IsAWindow(hwndLoseFocus))
         {
            ObjectData *objOld = win->widgets.findByWin(hwndLoseFocus);
            if(objOld && (objOld->type == CHECKBOX || objOld->type == RADIO_BUTTON || objOld->type == COLORBOX || objOld->type == UPDOWN))
            {
               HDC hdc = GetDC(objOld->hwndParent);
               objOld->DrawFocusRect(hdc,1);
               ReleaseDC(objOld->hwndParent,hdc);
            }
         }
         break;
      }
      case(WM_MOUSEACTIVATE): // User has clicked on window - update title, menu and toolbar
      {
         if(gDebug.mode != "off") // Change current GUI window if we are not debugging
            break;
         if(!obj) return(1);
         SelectFixedControls(win, obj);
			CText txt;
         if(win && !win->keepInFront)
            ChangeGUIParent(parWin);
         SetFocus(hWnd);
         break;
      }
		case(WM_CHAR): // Ignore certain characters
		{
         char  code = (CHAR)wParam;    // character code 

         if(code == VK_TAB) // Ignore tabs
         { 
            return(0);
         } 
         if(code == VK_RETURN) // Ignore returns
         { 
            return(0);
         } 
         else if(obj->readOnly || obj->readOnlyOutput) // Ignore all characters if readonly
            return(0);
			if(obj->highLiteChanges)
			{
				obj->valueChanged = true;
				obj->fgColor = RGB(255,0,0);
				InvalidateRect(GetParent(obj->hWnd),NULL,false);
			}
			if(code == VK_BACK || (code >= 32 && code <= 127))
			{
				if(obj->acceptKeyEvents)
				{
					strcpy(obj->cb_event,"keydown");
					SendMessage(win->hWnd,WM_COMMAND,(WPARAM)obj->nr(),(LPARAM)obj->hWnd);
				}
			}
         break;
		}
      case(WM_KEYDOWN): // Keyboard has been pressed 
      {
         char  code = (CHAR)wParam;    // character code 

         if(code == VK_F2)
         {
            if(AnyGUIWindowVisible())
               HideGUIWindows();
            else
               ShowGUIWindows(SW_SHOWNOACTIVATE);
         }
         else if(code == VK_RETURN) // Check for return in button
         { 
            strcpy(obj->cb_event,"enter");
            if(win->defaultID != -1)
               SendMessage(win->hWnd,WM_KEYDOWN,(WPARAM)VK_RETURN,0);
            else
               SendMessage(win->hWnd,WM_COMMAND,(WPARAM)obj->nr(),(LPARAM)obj->hWnd);
            return(0);               
         } 
         else if(code == VK_ESCAPE) // Check for escape 
         { 
            SendMessage(win->hWnd,WM_KEYDOWN,(WPARAM)VK_ESCAPE,0); // Send escape to parent window
            return(0);               
         } 
         else if(code == VK_TAB && !IsKeyDown(VK_CONTROL)) // Check for tab - if found move to the next/last tabbable object
         { 
            obj->processTab();
            return(0);
         }
         else if(obj->readOnly || obj->readOnlyOutput) // Ignore all characters if readonly
            return(0);
			else if(code == VK_DELETE)
			{
				if(obj->acceptKeyEvents)
				{
					strcpy(obj->cb_event,"keydown");
					SendMessage(win->hWnd,WM_COMMAND,(WPARAM)obj->nr(),(LPARAM)obj->hWnd);
				}
			}
         break;
      }   
   // See if user wants to get control info
      case(WM_LBUTTONDOWN): 
      case(WM_LBUTTONDBLCLK): 
      {
         if(wParam & MK_CONTROL) 
         {
            DisplayControlInfo(win,obj);
            return(0);
         }
			break;
      } 
   }
   return(CallWindowProc(OldTextMenuEditProc,hWnd, messg, wParam, lParam));
            
}

/***************************************************************************
   Text editor functions
***************************************************************************/

int TextEditFunctions(Interface *itfc, char args[])
{
  CText func;
   short i,r;
	CText text; 

   if((r = ArgScan(itfc,args,0,"command","e","t",&func)) < 0)
      return(r);

	HWND hWnd = GetFocus();
   ObjectData *obj = GetObjData(hWnd); 
	if(!obj)
		obj = GetObjData(GetParent(hWnd));

	if(!obj)
		return(OK);

   if(func == "select all")
	{
		SendMessage(hWnd, EM_SETSEL, 0, -1) ;
	}
   else if(func == "clear all")
	{
		SendMessage(hWnd, EM_SETSEL, 0, -1) ;
		SendMessage(hWnd, EM_REPLACESEL, 1, (LPARAM)"") ;
	}
   else if(func == "copy")
	{
		DWORD start,end;
		LPTSTR  lptstrCopy; 
      HGLOBAL hglbCopy;

	// Get the text selection
		SendMessage(hWnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end) ;
		obj->GetWindowText(text);
      CText selection = text.Middle(start,end-1);
		int sz = selection.Size();
	// Prepare the global clipboard
		 if (!OpenClipboard(hWnd)) 
			  return(OK); 
		 EmptyClipboard(); 
   // Allocate a global memory buffer for the text. 
      hglbCopy = GlobalAlloc(GHND | GMEM_SHARE, sz+1); 
   // Lock the handle and copy the selected text to the buffer.  
      lptstrCopy = (LPSTR)GlobalLock(hglbCopy); 
      memcpy(lptstrCopy, selection.Str(), sz); 
		lptstrCopy[sz] = '\0';
      GlobalUnlock(hglbCopy); 

  // Place the handle on the clipboard. 
      SetClipboardData(CF_TEXT, hglbCopy); 
      CloseClipboard(); 
	}
   else if(func == "cut")
	{
		DWORD start,end;
		LPTSTR  lptstrCopy; 
      HGLOBAL hglbCopy;

	// Get the text selection and copy to clipboard
		SendMessage(hWnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end) ;
		obj->GetWindowText(text);
      CText selection = text.Middle(start,end-1);
		int sz = selection.Size();
	// Prepare the global clipboard
		 if (!OpenClipboard(hWnd)) 
			  return(OK); 
		 EmptyClipboard(); 
   // Allocate a global memory buffer for the text. 
      hglbCopy = GlobalAlloc(GHND | GMEM_SHARE, sz+1); 
   // Lock the handle and copy the selected text to the buffer.  
      lptstrCopy = (LPSTR)GlobalLock(hglbCopy); 
      memcpy(lptstrCopy, selection.Str(), sz); 
		lptstrCopy[sz] = '\0';
      GlobalUnlock(hglbCopy); 
  // Place the handle on the clipboard. 
      SetClipboardData(CF_TEXT, hglbCopy); 
      CloseClipboard(); 

	// Remove the selected text
 	   SendMessage(hWnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end) ;
		SendMessage(hWnd, EM_REPLACESEL, 1, (LPARAM)"") ;
	}
	else if(func == "paste")
	{
		DWORD start,end;
		LPTSTR  lptstrPaste; 
      HGLOBAL hglbPaste;

      if (!IsClipboardFormatAvailable(CF_TEXT)) 
			return(OK);
      if (!OpenClipboard(hWnd)) 
			return(OK);
 
      hglbPaste = GetClipboardData(CF_TEXT); 
      if (hglbPaste != NULL) 
      { 
         lptstrPaste = (LPSTR)GlobalLock(hglbPaste); 
         if (lptstrPaste != NULL) 
         { 
 				SendMessage(hWnd, EM_GETSEL, (WPARAM)&start, (LPARAM)&end) ;
				SendMessage(hWnd, EM_REPLACESEL, 1, (LPARAM)lptstrPaste) ;
				GlobalUnlock(hglbPaste); 
			 //  GlobalFree(lptstrPaste);
         } 
      } 
      CloseClipboard(); 
	}
	else if(func == "undo")
	{
		SendMessage(hWnd, EM_UNDO, (WPARAM)0, (LPARAM)0) ;
	}


// Implement PASTE, CUT and UNDO? and also check why pasting into CLI is intermittent.
	return(OK);
}

/***************************************************************************
   Event procedure for the the text-box
***************************************************************************/

LRESULT CALLBACK TextBoxEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   LRESULT r = 0;
   WinData *win = 0;
   ObjectData *obj = 0;
//	static CText curText;
   
   HWND parWin = GetParent(hWnd);
   win = GetWinDataClass(parWin);
   if(!win)
   {
      HWND parWin2 = GetParent(parWin);
      win = GetWinDataClass(parWin2);
   }
   if(!win || win->inRemoval) return(1);               
   obj = win->widgets.findByWin(hWnd); // Find object selected
   if(!obj)
      return(CallWindowProc(OldTextBoxProc,hWnd, messg, wParam, lParam));

   switch(messg)
	{
      case(WM_DROPFILES):
      {
         CText path,file,ext;
         if(GetDropFileInfo((HDROP)wParam,path,file,ext,0) == OK)
         {
            char proc[50];
            sprintf(proc,"textbox(%d,%d)",win->nr,obj->nr());
            CText cmd;
            cmd.Format("%s(\"%s\",\"%s\",\"%s\")",obj->dragNDropProc.Str(),path.Str(),file.Str(),ext.Str());
            ProcessMacroStr(LOCAL,win,obj,cmd.Str(),"",proc, win->macroName, win->macroPath);
            DragFinish((HDROP)wParam);
            return(0);
         }
         return(0);
      }
      case(WM_PAINT):
      {
         if(obj)
         {
            r = CallWindowProc(OldTextBoxProc,hWnd, messg, wParam, lParam);
				obj->decorate(parWin);
            if(obj->inError && obj->visible)
            {
               obj->DrawErrorRect(parWin);
            }
            return(r);
         }
			break;
      }
		case(WM_CHAR): // Ignore certain characters
		{
         char  code = (CHAR)wParam;    // character code 

         if(code == VK_TAB) // Ignore tabs
         { 
            return(0);
         } 
         else if(code == 0x01) // ^A selects all text
         { 
            SendMessage(hWnd,EM_SETSEL ,0,-1);
            return(0);
         } 
         else if(code == VK_RETURN) // Ignore returns
         { 
            return(0);
         }
         else if((obj->readOnly  || obj->readOnlyOutput) && code != COPY) // Ignore all characters if readonly
            return(0);
			if(obj->highLiteChanges)
			{
				obj->valueChanged = true;
				obj->fgColor = RGB(255,0,0);
				InvalidateRect(GetParent(obj->hWnd),NULL,false);
			}
			if(code == VK_BACK || (code >= 32 && code <= 127))
			{
				if(obj->acceptKeyEvents)
				{
					strcpy(obj->cb_event,"keydown");
               CallWindowProc(OldTextBoxProc, hWnd, messg, wParam, lParam); // Update UI with character before running callback
					SendMessage(win->hWnd,WM_COMMAND,(WPARAM)obj->nr(),(LPARAM)obj->hWnd);
               return(0);
				}
			}
         break;
		}
      case(WM_SETFOCUS):
      {
         win->objWithFocus = obj;
         HWND hwndLoseFocus = (HWND) wParam;
         if(hwndLoseFocus && !obj->IsAWindow(hwndLoseFocus))
         {
            ObjectData *objOld = win->widgets.findByWin(hwndLoseFocus);
            if(objOld && (objOld->type == CHECKBOX || objOld->type == RADIO_BUTTON || objOld->type == COLORBOX || objOld->type == UPDOWN))
            {
               HDC hdc = GetDC(objOld->hwndParent);
               objOld->DrawFocusRect(hdc,1);
               ReleaseDC(objOld->hwndParent,hdc);
            }
         }
         break;
      }

		//case(WM_MOUSELEAVE): // Send a command message if the contents have changed and the cursor has left the textbox
		//{
		//	if(obj->valueChanged)
		//	{
		//	   obj->valueChanged = false;
		//		if(obj->acceptKeyEvents)
		//		{
		//			strcpy(obj->cb_event,"valueChanged");
		//			SendMessage(win->hWnd,WM_COMMAND,(WPARAM)obj->nr(),(LPARAM)obj->hWnd);
		//		}
		//	}
		//	break;
		//}

		//case(WM_KILLFOCUS):
		//{
		//	if(obj->valueChanged)
		//	{
		//	   obj->valueChanged = false;
		//		if(obj->acceptKeyEvents)
		//		{
		//			strcpy(obj->cb_event,"valueChanged");
		//			SendMessage(win->hWnd,WM_COMMAND,(WPARAM)obj->nr(),(LPARAM)obj->hWnd);
		//		}
		//	}
		//		//	TextMessage("Focus lost in textbox\n");

		//	break;
		//}

      case(WM_MOUSEACTIVATE): // User has clicked on window - update title, menu and toolbar
      {
         if(gDebug.mode != "off") // Change current GUI window if we are not debugging
            break;
         if(!obj) return(1);
         SelectFixedControls(win, obj);
			CText txt;
         if(win && !win->keepInFront)
            ChangeGUIParent(parWin);
         SetFocus(hWnd);
         break;
      }
      case(WM_KEYDOWN): // Keyboard has been pressed 
      {
         char  code = (CHAR)wParam;    // character code 

         if(code == VK_F2)
         {
            if(AnyGUIWindowVisible())
               HideGUIWindows();
            else
               ShowGUIWindows(SW_SHOWNOACTIVATE);
         }
         if(code == VK_RETURN) // Check for return
         { 
            strcpy(obj->cb_event,"enter");
            if(win->defaultID != -1)
               SendMessage(win->hWnd,WM_KEYDOWN,(WPARAM)VK_RETURN,0);
            else
               SendMessage(win->hWnd,WM_COMMAND,(WPARAM)obj->nr(),(LPARAM)obj->hWnd);
            return(0);               
         }  
         if(code == VK_ESCAPE) // Check for escape - send message to parent window 
         { 
            SendMessage(win->hWnd,WM_KEYDOWN,(WPARAM)VK_ESCAPE,0); // Send escape to parent window
            return(0);               
         } 
         if(code == VK_TAB && !IsKeyDown(VK_CONTROL)) // Check for tab - if found move to the next/last tabbable object
         { 
            obj->processTab();
            return(0);
         } 
			if(code == VK_DELETE)
			{
				if(obj->acceptKeyEvents)
				{
					strcpy(obj->cb_event,"keydown");
					SendMessage(win->hWnd,WM_COMMAND,(WPARAM)obj->nr(),(LPARAM)obj->hWnd);
				}
			}
         break;
      } 
   // See if user wants to get control info
      case(WM_LBUTTONDOWN): 
      case(WM_LBUTTONDBLCLK): 
      {
         if(wParam & MK_CONTROL) 
         {
            DisplayControlInfo(win,obj);
            return(0);
         }
			break;
      } 
   }
   return(CallWindowProc(OldTextBoxProc,hWnd, messg, wParam, lParam));
            
}


/***************************************************************************
   Event procedure for the progress-bar
***************************************************************************/

LRESULT CALLBACK ProgressBarEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   LRESULT r = 0;
   bool inPanel = 0;
   WinData *win = 0;
   ObjectData *obj = 0;
   ObjectData *nextObj = 0;
   
   HWND parWin = GetParent(hWnd);
   win = GetWinDataClass(parWin);
   if(!win)
   {
      HWND parWin2 = GetParent(parWin);
      win = GetWinDataClass(parWin2);
   }
   if(!win || win->inRemoval) return(1);            
   obj = win->widgets.findByWin(hWnd); // Find object selected
   if(!obj)
      return(CallWindowProc(OldProgressBarProc,hWnd, messg, wParam, lParam));

   switch(messg)
	{
      case(WM_PAINT):
      {
         if(obj)
         {
            r = CallWindowProc(OldProgressBarProc,hWnd, messg, wParam, lParam);
				obj->decorate(parWin);
				return r;
         }
      }
   }
   return(CallWindowProc(OldProgressBarProc,hWnd, messg, wParam, lParam));
            
}


/***************************************************************************
   Event procedure for the track-bar (slider)
***************************************************************************/

LRESULT CALLBACK SliderEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   LRESULT r = 0;
   WinData *win = 0;
   ObjectData *obj = 0;
   bool inPanel = 0;
   ObjectData *nextObj = 0;


   HWND parWin = GetParent(hWnd);
   win = GetWinDataClass(parWin);
   if(!win)
   {
      HWND parWin2 = GetParent(parWin);
      win = GetWinDataClass(parWin2);
   }
   if(!win || win->inRemoval) return(1);         
   obj = win->widgets.findByWin(hWnd); // Find object selected
   if(!obj)
      return(CallWindowProc(OldSliderProc,hWnd,messg,wParam,lParam));

      
   switch(messg)
	{
      case(WM_PAINT):
      {
         if(obj)
         {
            r = CallWindowProc(OldSliderProc,hWnd,messg,wParam,lParam);
				obj->decorate(parWin);
				return r;
         }
      }
      case(WM_SETFOCUS):
      {
         win->objWithFocus = obj;
         HWND hwndLoseFocus = (HWND) wParam;
         if(hwndLoseFocus && !obj->IsAWindow(hwndLoseFocus))
         {
            ObjectData *objOld = win->widgets.findByWin(hwndLoseFocus);
            if(objOld && (objOld->type == CHECKBOX || objOld->type == RADIO_BUTTON || objOld->type == COLORBOX || objOld->type == UPDOWN))
            {
               HDC hdc = GetDC(objOld->hwndParent);
               objOld->DrawFocusRect(hdc,1);
               ReleaseDC(objOld->hwndParent,hdc);
            }
         }
         break;
      }
      case(WM_KEYDOWN): // Keyboard has been pressed 
      {
         char  code = (CHAR)wParam;    // character code 

         if(code == VK_F2)
         {
            if(AnyGUIWindowVisible())
               HideGUIWindows();
            else
               ShowGUIWindows(SW_SHOWNOACTIVATE);
         }
         if(code == VK_RETURN) // Check for return in button
         { 
            strcpy(obj->cb_event,"enter");
            if(win->defaultID != -1)
               SendMessage(win->hWnd,WM_KEYDOWN,(WPARAM)VK_RETURN,0);
            else
               SendMessage(win->hWnd,WM_COMMAND,(WPARAM)obj->nr(),(LPARAM)obj->hWnd);
            return(0);               
         } 
         if(code == VK_ESCAPE) // Check for escape 
         { 
            SendMessage(win->hWnd,WM_KEYDOWN,(WPARAM)VK_ESCAPE,0); // Send escape to parent window
            return(0);               
         } 
         if(code == VK_TAB && !IsKeyDown(VK_CONTROL)) // Check for tab - if found move to the next/last tabbable object
			{ 
				obj->processTab();
            return(0);
         } 
         break;
      }
   // See if user wants to get control info
      case(WM_LBUTTONDOWN): 
      case(WM_LBUTTONDBLCLK): 
      {
         if(wParam & MK_CONTROL) 
         {
            DisplayControlInfo(win,obj);
            return(0);
         }
         sliderMouseButtonUp = false;
         sliderMouseButtonDn = true;
			break;
      }

      // Left button has been released
     case(WM_LBUTTONUP):
     {
        sliderMouseButtonUp = true;
        sliderMouseButtonDn = false;
        break;
     }

   }
   return(CallWindowProc(OldSliderProc,hWnd,messg,wParam,lParam));
            
}

/***************************************************************************
   Event procedure for the status-bar
***************************************************************************/

LRESULT CALLBACK StatusBarEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   LRESULT r = 0;
   WinData *win = 0;
   ObjectData *obj = 0;
   
   HWND parWin = GetParent(hWnd);
   win = GetWinDataClass(parWin);
   if(!win || win->inRemoval) return(1);      
   obj = win->widgets.findByWin(hWnd); // Find object selected
   if(!obj)
       return(CallWindowProc(OldStatusBarProc,hWnd,messg,wParam,lParam));     

   switch(messg)
	{
      case(WM_PAINT):
      {
         if(obj)
         {
            r = CallWindowProc(OldStatusBarProc,hWnd,messg,wParam,lParam);
				obj->decorate(parWin);
				return r;
         }
      }
      case(WM_SIZE):
      {
         if(obj)
         {
            r = CallWindowProc(OldStatusBarProc,hWnd,messg,wParam,lParam);
            RECT rc;
		      GetWindowRect(obj->hWnd,&rc);
            ScreenToClientRect(obj->hwndParent,&rc);
		      obj->xo = (short)rc.left;
		      obj->yo = (short)rc.top; 
		      obj->wo = (short)(rc.right-rc.left);
		      obj->ho = (short)(rc.bottom-rc.top);
            return(r);
         }
      }
   }

   return(CallWindowProc(OldStatusBarProc,hWnd,messg,wParam,lParam));
}

/***************************************************************************
   Event procedure for the static-text bar
***************************************************************************/

LRESULT CALLBACK STextEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   LRESULT r = 0;
   WinData *win = 0;
   ObjectData *obj = 0;    
   
   HWND parWin = GetParent(hWnd);
   win = GetWinDataClass(parWin);
   if(!win)
   {
      HWND parWin2 = GetParent(parWin);
      win = GetWinDataClass(parWin2);
   }
   if(!win || win->inRemoval)
      return(1);   

   obj = win->widgets.findByWin(hWnd); // Find object selected
   if(!obj)
      return(CallWindowProc(OldStaticTextProc,hWnd, messg, wParam, lParam));

   switch(messg)
	{
      case(WM_PAINT):
      {
         if(obj)
         {
            r = CallWindowProc(OldStaticTextProc,hWnd, messg, wParam, lParam);
				obj->decorate(parWin);
				return r;
         }
      }

   }
   
   return(CallWindowProc(OldStaticTextProc,hWnd, messg, wParam, lParam));
            
}


/***************************************************************************
   Event procedure for the picture object
***************************************************************************/

LRESULT CALLBACK PictureEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   LRESULT r = 0;
   WinData *win = 0;
   ObjectData *obj = 0;    
   ObjectData *nextObj = 0;

   HWND parWin = GetParent(hWnd);
   win = rootWin->FindWinByHWND(parWin);
   if(!win)
   {
      HWND parWin2 = GetParent(parWin);
      win = GetWinDataClass(parWin2);
   }
   if(!win || win->inRemoval) return(1);   
   obj = win->widgets.findByWin(hWnd); // Find object selected
   if(!obj)
      return(CallWindowProc(OldPictureProc,hWnd, messg, wParam, lParam));

   switch(messg)
	{
      case(WM_PAINT):
      {
         if(obj)
         {
            r = CallWindowProc(OldPictureProc,hWnd, messg, wParam, lParam);
				obj->decorate(parWin);
				return r;
         }
      }
   // Draw the window to a specific device context
		case(WM_PRINT): 
	//	case(WM_PRINTCLIENT): 
		{
         DRAWITEMSTRUCT dis;
         HDC hdc = (HDC) wParam;
         dis.hDC = hdc;
         dis.itemID = obj->nr();
         dis.itemData = 1;
         dis.hwndItem = obj->hWnd;
         SetRect(&(dis.rcItem),0,0,obj->wo,obj->ho);
         wParam = (WPARAM)obj->nr();
         SendMessage(GetParent(hWnd), WM_DRAWITEM, wParam, (LPARAM) &dis);
         break;
      }

      case(WM_ACTIVATE):
         return(0);

      case(WM_LBUTTONDOWN): 
      case(WM_LBUTTONDBLCLK): 
      {
         if(wParam & MK_CONTROL) 
         {
            DisplayControlInfo(win,obj);
            return(0);
         }
         else
         {
		      char *str = win->GetObjCommand(obj->nr());
		      if(str[0] != '\0')
		      {
               Interface itfc;
               HWND wnd = GetFocus(); // Save button focus
		         itfc.procName.Format("picture(%d,%d)",win->nr,obj->nr());
               itfc.startLine = obj->cmdLineNr;
               controlProcRunning = true;
		         ProcessMacroStr(&itfc,1,str);
               controlProcRunning = false;
               return(0);
            }
         }
      } 
   }
   
   return(CallWindowProc(OldPictureProc,hWnd, messg, wParam, lParam));
            
}

LRESULT CALLBACK SText2EventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   LRESULT r = 0;
   WinData *win = 0;
   ObjectData *obj = 0;
   ShortRect rect;

   HWND parWin = GetParent(hWnd);
   win = GetWinDataClass(parWin);
   if(!win || win->inRemoval) return(1);   
   obj = win->widgets.findByWin(hWnd); // Find object selected
   if(!obj)
      return(CallWindowProc(OldStatic2TextProc,hWnd, messg, wParam, lParam));

   switch(messg)
	{
      case(WM_PAINT):
      {
         if(obj)
         {
            r = CallWindowProc(OldStatic2TextProc,hWnd, messg, wParam, lParam);

            if(obj->selected_)
            {
               HDC hdc = GetDC(obj->hwndParent);
               obj->DrawSelectRect(hdc);
            }
            else
            {
               HDC hdc = GetDC(obj->hwndParent);            
					WriteText(hdc,obj->xo,obj->yo,WindowLayout::LEFT_ALIGN,WindowLayout::TOP_ALIGN,WindowLayout::HORIZONTAL,Plot1D::curPlot()->curXAxis()->label().font(),RGB(0,0,0),Plot1D::curPlot()->borderColor,obj->data,rect);
             }  
            return(r);
         }
      }
   }
   
   return(CallWindowProc(OldStatic2TextProc,hWnd, messg, wParam, lParam));
            
}

/**********************************************************************
   Send a message to all the GUI windows (user interface)
***********************************************************************/

int SendGUIMessage(Interface* itfc ,char args[])
{
   short nrArgs;
   CText source;
   CText message;
   
// Get the directory      
   if((nrArgs = ArgScan(itfc,args,2,"source, message","ee","tt",&source,&message)) < 0)
      return(nrArgs); 

   message = source + "," + message;

   SendMessageToGUI(message.Str(),0);

   return(OK);
}

/**********************************************************************
   Send a message to all the GUI windows
***********************************************************************/

EXPORT short SendMessageToGUI(char* message, short winNr)
{  
   WinData *win = rootWin->next;
   WinData *bakGUI,*bakParent;


   bakGUI = GetGUIWin();
   bakParent = GetParentWin();

// Make sure the message receiver knows the source of the message
// Might be a problem if several messages are sent simultaneously
// from several sources since they may not be processed in order
// might be better to attach this to the GETMESSAGE command somehow.
   if(winNr == 0)
      messageSource = GetGUIWin();
   else
      messageSource = NULL;

   gInMessageMode = true;

	while(win != (WinData*)FREEDHEAP && win != NULL)
   {
		SetParentWin(win);
		WinData::SetGUIWin(win); // Make sure the getmessage callback knows which window it is in
		SendMessage(win->hWnd,WM_COMMAND,(WPARAM)(GETMESSAGE<<16),(LPARAM)message); 
		if(GetGUIWin() == NULL) // Check to see if command deleted window
			break;
		win = win->next;
   }

	if(win == (WinData*)0xfeeefeee)
	{
		TextMessage("bad address found\n");
	}

   if(GetGUIWin() != 0)
   {
		WinData::SetGUIWin(bakGUI); // Restore the current window
      SetParentWin(bakParent);
   }

   gInMessageMode = false;

   return(ABORT*(gAbort == true)); // Added to check for abort during SendMessage command above
}


/**********************************************************************
   Display  dialog box to enter information about a control
   (e.g. label, name ...)
***********************************************************************/

int CALLBACK EnterObjectInfoDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{   
	char label[MAX_STR];
   
	switch (message)
	{
		case(WM_INITDIALOG):
		{
         GetWindowText(currentObj->hWnd,label,100);
		   SetDlgItemText(hWnd,ID_LABEL_TXT,label);
		   SendMessage(GetDlgItem(hWnd,ID_LABEL_TXT),EM_SETSEL,(WPARAM)0,(LPARAM)-1); // Select text
         SetFocus(GetDlgItem(hWnd,ID_LABEL_TXT));

		   SetDlgItemText(hWnd,ID_NAME_TXT,currentObj->valueName);
         PlaceDialogOnTopInCentre(hWnd);
         SaveDialogParent(hWnd);
		   SetCursor(LoadCursor(NULL,IDC_ARROW));
	      return(false);
		}
		case(WM_MOVE):
		{
		   break;
		}
		case(WM_CLOSE):
		{
	      MyEndDialog(hWnd,0);
      	return(false);
		}
		case(WM_COMMAND):
		{
		   switch(LOWORD(wParam))
		   {
	       	case(ID_CANCEL):
	            MyEndDialog(hWnd,0);
	         	return(false);         	
	         case(ID_APPLY): 
	         {
				   GetDlgItemText(hWnd,ID_LABEL_TXT,label,100); // Save label
               SetWindowText(currentObj->hWnd,label);
				   GetDlgItemText(hWnd,ID_NAME_TXT,currentObj->valueName,100); // Save name
               
               if(currentObj->type == STATICTEXT)
               {
				      HDC hdc = GetDC(currentObj->hwndParent);
				      SIZE size;
			
				      SelectObject(hdc,controlFont);	      
				      GetTextExtentPoint32(hdc,label,strlen(label),&size);
				      
				      short w = (short)size.cx;
				      short h = (short)size.cy; 
				      
                  currentObj->Place(currentObj->xo, currentObj->yo, w, h,true);
                  currentObj->invalidate();
				   }
               else if(currentObj->type == PROGRESSBAR)
               {
                  long type = GetWindowLong(currentObj->hWnd,GWL_STYLE);
				      if(!strcmp(label,"vertical"))
				      {
				         type = type | PBS_VERTICAL;
				         SetWindowLong(currentObj->hWnd,GWL_STYLE,type);
				      }
				      else
				      {
				         type = type & ~PBS_VERTICAL;
				         SetWindowLong(currentObj->hWnd,GWL_STYLE,type);
				      }				      
				   }
               else if(currentObj->type == SLIDER)
               {
                  long type = GetWindowLong(currentObj->hWnd,GWL_STYLE);
                  long x,y,w,h;
				      if(!strcmp(label,"vertical"))
				      {
				         type = type & ~TBS_HORZ;
				         type = type | TBS_VERT;
				         SetWindowLong(currentObj->hWnd,GWL_STYLE,type);
                     x = currentObj->xo;
                     y = currentObj->yo;
                     w = currentObj->wo;
                     h = currentObj->ho;
                     currentObj->Place(x,y,h,w,true);
				      }
				      else
				      {
				         type = type & ~TBS_VERT;
				         type = type | TBS_HORZ;
				         SetWindowLong(currentObj->hWnd,GWL_STYLE,type);
                     x = currentObj->xo;
                     y = currentObj->yo;
                     w = currentObj->wo;
                     h = currentObj->ho;
                     currentObj->Place(x,y,h,w,true);
				      }				      
				   }	
	            MyEndDialog(hWnd,1);
	         	return(false);
	         }
	     	}
	      break;
      }
   }
   return(false);
}

/*************************************************************************
  Display information about control(win,obj) in the gui macro
  controlInfo.mac. If the window does not exist create it otherwise
  use an existing one.
**************************************************************************/

void DisplayControlInfo(WinData *win, ObjectData *obj)
{
   long n;
   char cmd[100];
   Interface itfc;

// Find any current gui window
   if(FindWindowCLI(&itfc,"\"name\",\"ControlInfo\"") == ERR)
      return;
   n = nint(itfc.retVar[1].GetReal());

   if(n == -1) // Load info into new window
   {
      sprintf(cmd,"controlInfo(%hd,%hd)",win->nr,obj->nr());
      ProcessMacroStr(1,NULL,NULL,cmd,"","","controlInfo.mac","");
   }
   else // Load info into existing window (encode winNr and objNr in one string)
   {
      sprintf(cmd,"ControlInfo,%ld",(long)(win->nr*1000+obj->nr()));
      SendMessageToGUI(cmd,-1);
   }
}

/***************************************************************************
   Event procedure for tab control
***************************************************************************/

LRESULT CALLBACK TabCtrlEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   LRESULT r = 0;
   WinData *win = 0;
   ObjectData *obj = 0;
   
   HWND parWin = GetParent(hWnd);
   win = rootWin->FindWinByHWND(parWin);
   if(!win)
   {
      HWND parWin2 = GetParent(parWin);
      win = GetWinDataClass(parWin2);
   }
   if(!win || win->inRemoval) return(1);            
   obj = win->widgets.findByWin(hWnd); // Find object selected
   if(!obj)
      return(CallWindowProc(OldTabCtrlProc,hWnd, messg, wParam, lParam));

   switch(messg)
	{
      case(WM_DROPFILES):
      {
         CText path,file,ext;
         if(GetDropFileInfo((HDROP)wParam,path,file,ext,0) == OK)
         {
            char proc[50];
            sprintf(proc,"tab(%d,%d)",win->nr,obj->nr());
            CText cmd;
            cmd.Format("%s(\"%s\",\"%s\",\"%s\")",obj->dragNDropProc.Str(),path.Str(),file.Str(),ext.Str());
            ProcessMacroStr(LOCAL,win,obj,cmd.Str(),"",proc, win->macroName, win->macroPath);
            DragFinish((HDROP)wParam);
            return(0);
         }

         return(0);
      }

      case(WM_PAINT):
      {
         if(obj)
         {
            r = CallWindowProc(OldTabCtrlProc,hWnd, messg, wParam, lParam);
				obj->decorate(parWin);
				return r;
         }
      }


      case(WM_ERASEBKGND): // Modify the tab control to have button background
      {
         HDC hdc = (HDC)wParam;
         HRGN hRgn;
         HBRUSH hBrush;
         hRgn = obj->CreateTabBkgRegion(); // Calculate the background region for tab
         
         hBrush = (HBRUSH)CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
           //  hBrush = (HBRUSH)CreateSolidBrush(RGB(255,0,0));
    
         FillRgn(hdc,hRgn,hBrush);
         DeleteObject(hBrush);
         DeleteObject(hRgn); 

     // Draw a grid on the control
         if(win->showGrid && !win->activated)
         {
            HPEN plotPen = CreatePen(PS_SOLID,0,gDesignerGridColor);
            HPEN oldPen = (HPEN)SelectObject(hdc,plotPen);
            int s;
            int width = win->ww;
            int height = win->wh;
            int spacing = win->gridSpacing;
				int x0 = obj->xo;
            for(s = (x0/spacing+1)*spacing-x0; s < width; s+= spacing)
            {
               MoveToEx(hdc,s,20,NULL);
               LineTo(hdc,s,height-1);
            }
				int y0 = obj->yo;
            for(s = 20+(y0/spacing+1)*spacing-y0; s < height; s+= spacing)
            {
               MoveToEx(hdc,0,s,NULL);
               LineTo(hdc,width-1,s);
            }
            SelectObject(hdc,oldPen);
            DeleteObject(plotPen);
         }
         return(1);
      }

      case(WM_SETFOCUS):
      {
         win->objWithFocus = obj;
         HWND hwndLoseFocus = (HWND) wParam;
         ObjectData *objOld = win->widgets.findByWin(hwndLoseFocus);

         if(hwndLoseFocus && !obj->IsAWindow(hwndLoseFocus))
         {
         // Remove focus rectangle from object loosing focus (checkbox, radiobutton or colorbox)
            if(objOld && (objOld->type == CHECKBOX || objOld->type == RADIO_BUTTON || objOld->type == COLORBOX || objOld->type == UPDOWN))
            {
               HDC hdc = GetDC(objOld->hwndParent);
               objOld->DrawFocusRect(hdc,1);
               ReleaseDC(objOld->hwndParent,hdc);
            }
         }
      // Focus is in a radio button but another has been selected using
      // the tab button or the mouse button so erase the current one.
         if(obj && objOld && (obj == objOld) && (obj->type == RADIO_BUTTON))
         {
            HDC hdc = GetDC(obj->hwndParent);
            obj->DrawFocusRect(hdc,1);
            ReleaseDC(obj->hwndParent,hdc);
         }
         break;
      }

      case(WM_MOUSEACTIVATE): // User has clicked on window - update title, menu and toolbar
      {
         if(gDebug.mode != "off") // Change current GUI window if we are not debugging
            break;
         if(!obj) return(1);
         SelectFixedControls(win, obj);
         if(win && !win->keepInFront)
            ChangeGUIParent(parWin);
         SetFocus(hWnd);
         break;
      }

      case(WM_KEYDOWN): // Keyboard has been pressed 
      {
         char  code = (CHAR)wParam;    // character code 

         if(code == VK_F2)
         {
            if(AnyGUIWindowVisible())
               HideGUIWindows();
            else
               ShowGUIWindows(SW_SHOWNOACTIVATE);
         }

         if(!win->activated)
            return(0);


      // Make sure we can tab within the other controls in the tab
         if(code == VK_TAB && !IsKeyDown(VK_SHIFT) && !IsKeyDown(VK_CONTROL)) // Check for tab - if found move to the next/last tabbable object
			{ 
				obj->processTab();
            return(0);
         } 

         if(code == VK_TAB && !IsKeyDown(VK_CONTROL)) // Check for tab - if found move to the next/last tabbable object
			{ 
				obj->processTab();
            return(0);
         } 

      // Make sure we can move accross the tab controls using the arrow keys
         if((code >= VK_LEFT && code <= VK_RIGHT) && obj->type == TABCTRL)
         {
              r = CallWindowProc(OldTabCtrlProc,hWnd, messg, wParam, lParam);
              SetFocus(hWnd);
              return(0);
         }
         break;
      } 
   }

   return(CallWindowProc(OldTabCtrlProc,hWnd, messg, wParam, lParam));
}

/***************************************************************************
   Event procedure for updown control
***************************************************************************/

LRESULT CALLBACK UpDownEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   LRESULT r = 0;
   bool inPanel = 0;
   WinData *win = 0;
   ObjectData *obj = 0;
   ObjectData *nextObj = 0;
   
   HWND parWin = GetParent(hWnd);
   win = rootWin->FindWinByHWND(parWin);
   if(!win)
   {
      HWND parWin2 = GetParent(parWin);
      win = GetWinDataClass(parWin2);
   }
   if(!win || win->inRemoval) return(1);            
   obj = win->widgets.findByWin(hWnd); // Find object selected
   if(!obj)
      return(CallWindowProc(OldUpDownProc,hWnd, messg, wParam, lParam));

   switch(messg)
	{
      case(WM_PAINT):
      {
         if(obj)
         {
            r = CallWindowProc(OldUpDownProc,hWnd, messg, wParam, lParam);
				obj->decorate(parWin);
				return r;
         }
      }

      case(WM_SETFOCUS):
      {
         HWND hwndLoseFocus = (HWND) wParam;
         ObjectData *objOld = win->widgets.findByWin(hwndLoseFocus);

         if(hwndLoseFocus && !obj->IsAWindow(hwndLoseFocus))
         {
         // Remove focus rectangle from object loosing focus (checkbox, radiobutton or colorbox)
            if(objOld && (objOld->type == CHECKBOX || objOld->type == RADIO_BUTTON || objOld->type == COLORBOX || objOld->type == UPDOWN))
            {
               HDC hdc = GetDC(objOld->hwndParent);
               objOld->DrawFocusRect(hdc,1);
               ReleaseDC(objOld->hwndParent,hdc);
            }
         }

      // Focus is in a radio button but another has been selected using
      // the tab button or the mouse button so erase the current one.
         if(obj && objOld && (obj == objOld) && (obj->type == RADIO_BUTTON))
         {
            HDC hdc = GetDC(obj->hwndParent);
            obj->DrawFocusRect(hdc,1);
            ReleaseDC(obj->hwndParent,hdc);
         }

      // Focus is now in a checkbox so draw the focus rect
         if(obj)
         {
            HDC hdc = GetDC(obj->hwndParent);
            obj->DrawFocusRect(hdc,0);
            ReleaseDC(obj->hwndParent,hdc);
         }

         break;
      }

      case(WM_KEYDOWN): // Keyboard has been pressed 
      {
         char  code = (CHAR)wParam;    // character code 

         if(code == VK_RETURN && obj->type == BUTTON)  // Check for return 
         { 
            if(win->defaultID != -1)
               SendMessage(win->hWnd,WM_KEYDOWN,(WPARAM)VK_RETURN,0); // Send return to default button
            else
               SendMessage(win->hWnd,WM_COMMAND,(WPARAM)obj->nr(),(LPARAM)obj->hWnd); // Process button command
            return(0);               
         }
         if(code == VK_ESCAPE && obj->type == BUTTON) // Check for escape 
         { 
            SendMessage(win->hWnd,WM_KEYDOWN,(WPARAM)VK_ESCAPE,0); // Send escape to parent window
            return(0);               
         } 
         if(code == VK_TAB && !IsKeyDown(VK_CONTROL)) // Check for tab - if found move to the next/last tabbable object
			{ 
				obj->processTab();
            return(0);
         } 
         if(code == VK_SPACE)
         {
         // Run radiobutton macro
            char *str;
            char proc[MAX_STR];
		      str = obj->command;
		      if(str[0] != '\0')
		      {
		         sprintf(proc,"updown(%d,%d)",win->nr,obj->nr());
		         ProcessMacroStr(LOCAL,win,obj,str,"",proc, win->macroName, win->macroPath);
		      }
            return(0);
		   }
		   if((code >= VK_LEFT && code <= VK_DOWN) && obj->type == RADIO_BUTTON) // Arrow keys on radio button moves the selection
		   {	
         // Find out which radio button is checked
	         RadioButtonInfo *info = (RadioButtonInfo*)obj->data; 
            int i,index;
	         for(i = 0; i < info->nrBut; i++)
	         {
	            if(SendMessage(info->hWnd[i],BM_GETCHECK,0,0))
                  break;
	         }
            index = i;

         // Erase the focus rect from the current radio button
            HDC hdc = GetDC(obj->hwndParent);
            obj->DrawFocusRect(hdc,1);
            ReleaseDC(obj->hwndParent,hdc);

            if(code == VK_LEFT || code == VK_UP) 
               index--;
            else if(code == VK_RIGHT || code == VK_DOWN)
               index++;

         // Check for invalid indices
           if(index < 0)
              index = info->nrBut-1;
           else if(index >= info->nrBut)
              index = 0;

        // Select the new radio button and draw the focus rect
            for(i = 0; i < info->nrBut; i++)
            {
               if(i != index)
                  SendMessage(info->hWnd[i],BM_SETCHECK,0,0);
               else
               {
                  SendMessage(info->hWnd[i],BM_SETCHECK,1,0);
                  { // Draw the focus rect
                     HDC hdc = GetDC(obj->hwndParent);
                     obj->DrawFocusRect(hdc,0);
                     ReleaseDC(obj->hwndParent,hdc);
                  }
               }
            }

            UpdateObject(obj);

         // Run radiobutton macro
            char *str;
            char proc[MAX_STR];
		      str = obj->command;
		      if(str[0] != '\0')
		      {
		         sprintf(proc,"updown(%d,%d)",win->nr,obj->nr());
		         ProcessMacroStr(LOCAL,win,obj,str,"",proc, win->macroName, win->macroPath);
		      }
		   }
         break;
      } 
   // See if user wants to get control info
      case(WM_LBUTTONDOWN): 
      case(WM_LBUTTONDBLCLK): 
      {
         if(wParam & MK_CONTROL) 
         {
            DisplayControlInfo(win,obj);
            return(0);
         }
      } 
   }

   return(CallWindowProc(OldUpDownProc,hWnd, messg, wParam, lParam));
}

WinData* GetGUIWin(void)
{
   ThreadGlobals *data = (ThreadGlobals*)TlsGetValue(dwTlsIndex);
   if(!data)
      return(NULL);
   return(data->curGUIWin);
}

CommandInfo* GetErrorInfo(void)
{
   ThreadGlobals *data = (ThreadGlobals*)TlsGetValue(dwTlsIndex);
   if(!data)
      return(NULL);
   return(&data->errInfo);
}

CommandInfo* GetCmdInfo(void)
{
   ThreadGlobals *data = (ThreadGlobals*)TlsGetValue(dwTlsIndex);
   if(!data)
      return(NULL);
   return(&data->curCmdInfo);
}

WinData* GetParentWin(void)
{
   ThreadGlobals *data = (ThreadGlobals*)TlsGetValue(dwTlsIndex);
   return(data->parentWin);
}


void SetParentWin(WinData* win)
{
   ThreadGlobals *data = (ThreadGlobals*)TlsGetValue(dwTlsIndex);
   data->parentWin = win;
   if (!TlsSetValue(dwTlsIndex, (void*)data)) 
      return; 
}


/*********************************************************************************
                 Save a window bitmap to a file or clipboard
**********************************************************************************/

void CopyWindow(WinData *win, CText &fileName, int x1clip, int x2clip, int y1clip, int y2clip);

int SaveGUIWindowImage(Interface* itfc ,char args[])
{
   short nArgs;
   short winNr;
   WinData *win;
   CText fileName = "clipboard";
   CText borderMode = "frame";
   Variable winRecVar;
   int x1clip=0,y1clip=0,x2clip=0,y2clip=0;

// Get the argument   
   if((nArgs = ArgScan(itfc,args,1,"winNr, fileName, cliprect, borderMode","eeee","dtvt",&winNr,&fileName,&winRecVar,&borderMode)) < 0)
      return(nArgs);


// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(ERR);
	}

// Get the window
   win = rootWin->FindWinByNr(winNr);

// Get window dimensions
   HWND hWnd = win->hWnd;
   RECT winRect;
   GetWindowRect(hWnd,&winRect);
   int w = winRect.right-winRect.left;
   int h = winRect.bottom-winRect.top;

// Allow for "noframe" option
   int titleHeight=0;
   int xEdge=0,yEdge=0;
   int copyWidth,copyHeight;

   if(borderMode == "noframe") // Exclude frame
   {
      if(!win->menuObj && !win->menuList  && !win->resizeable) // No menu and window not resizable
      {
         titleHeight = GetSystemMetrics(SM_CYCAPTION);
         xEdge = yEdge = fixedSizeWinBorderSize;
      }
      else if(!win->menuObj && !win->menuList  && win->resizeable) // No menu and window resizable
      {
         titleHeight = GetSystemMetrics(SM_CYCAPTION);
         xEdge = yEdge = resizableWinBorderSize;
      }
      else if((win->menuObj || win->menuList)  && !win->resizeable) // Menu and window not resizable
      {
         titleHeight = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU);
         xEdge = yEdge = fixedSizeWinBorderSize;
      }
      else if((win->menuObj || win->menuList)  && win->resizeable)// Menu and window resizable
      {
         titleHeight = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYMENU);
         xEdge = yEdge = resizableWinBorderSize;
      }

      copyWidth = w - 2*xEdge;
      copyHeight = h - titleHeight - 2*yEdge;
      yEdge += titleHeight;
   }
   else  // Include frame
   {
      xEdge = 0;
      yEdge = 0;
      copyWidth = w;
      copyHeight = h;
      titleHeight = 0;
   }

// Add in external clipping information if present
   if(nArgs >= 3)
   {
      if(winRecVar.GetType() == MATRIX2D && winRecVar.GetDimX() == 4 && winRecVar.GetDimY() == 1)
      {
         float **mat = winRecVar.GetMatrix2D();
         x1clip = nint(mat[0][0]);
         y1clip = nint(mat[0][1]);
         x2clip = nint(mat[0][2]);
         y2clip = nint(mat[0][3]);

         if(x1clip < 0)
            x1clip += copyWidth+xEdge;
         else
            x1clip += xEdge;

         if(y1clip < 0)
            y1clip += copyHeight+yEdge;
         else
            y1clip += yEdge;

         if(x2clip <= 0)
            x2clip += copyWidth+xEdge;
         else
            x2clip += xEdge;

         if(y2clip <= 0)
            y2clip += copyHeight+yEdge;
         else
            y2clip += yEdge;

         if(x1clip < 0 || x1clip > copyWidth+xEdge || x2clip < 0 || x2clip > copyWidth+xEdge ||
            y1clip < 0 || y1clip > copyHeight+yEdge || y2clip < 0 || y2clip > copyHeight+yEdge ||
            x1clip >= x2clip || y1clip >= y2clip)
         {
            ErrorMessage("invalid clipping rectange");
            return(ERR);
         }
      }
      else if(winRecVar.GetType() == NULL_VARIABLE)
      {
      // Copy to clip limits
         x1clip = xEdge;
         x2clip = copyWidth+xEdge;
         y1clip = yEdge;
         y2clip = copyHeight+yEdge;
      }
      else
      {
         ErrorMessage("invalid clipping variable");
         return(ERR);
      }
   }
   else
   {
   // Copy to clip limits
      x1clip = xEdge;
      x2clip = copyWidth+xEdge;
      y1clip = yEdge;
      y2clip = copyHeight+yEdge;
   }

   if(fileName == "clipboard") // Copy to clipboard
      CopyWindow(win,fileName,x1clip,x2clip,y1clip,y2clip);
   else // Copy to a file
   {    

   // Get image type
      char extension[MAX_STR];
      GetExtension(fileName.Str(),extension);

      if(!strcmp(extension,"emf"))
           // This is the old code - just copies from screen to file - can't write to clipboard yet
      {    // this is just a backup at the moment until I can copy Richedit, 3D and dividers
         HDC mfHDC = CreateEnhMetaFile(NULL,fileName.Str(),NULL,NULL);
   	   if(mfHDC)
         {
            HDC hdc = GetWindowDC(hWnd);
            BitBlt(mfHDC,0,0,x2clip-x1clip,y2clip-y1clip,hdc,x1clip,y1clip-2,SRCCOPY); // Not sure why I need the -2 but the window seems to be offset vertically otherwise
            HENHMETAFILE hemf = CloseEnhMetaFile(mfHDC); 
            DeleteEnhMetaFile(hemf); 
            ReleaseDC(hWnd,hdc);
         }
      }
      else
      {
         CText type;
         if(!strcmp(extension,"jpg"))
            type = "image/jpeg";
         else if(!strcmp(extension,"tif"))
            type = "image/tiff";
         else if(!strcmp(extension,"png") || !strcmp(extension,"gif") || !strcmp(extension,"bmp"))
            type.Format("image/%s",extension);
         else
         {
            ErrorMessage("invalid extension for image save");
            return(ERR);
         }
         wchar_t *wMode = CharToWChar(type.Str());
         wchar_t *wFileName = CharToWChar(fileName.Str());
         HDC hdc = GetWindowDC(hWnd);
         HDC hdcMem = CreateCompatibleDC(hdc);
         HBITMAP hBitmap = CreateCompatibleBitmap(hdc,x2clip-x1clip,y2clip-y1clip);
    	   SelectObject(hdcMem,hBitmap);     
         BitBlt(hdcMem,0,0,x2clip-x1clip,y2clip-y1clip,hdc,x1clip,y1clip,SRCCOPY);
         Bitmap *bmp = Bitmap::FromHBITMAP(hBitmap,NULL);
         CLSID pngClsid;
         GetEncoderClsid(wMode, &pngClsid);
         Status st = bmp->Save(wFileName, &pngClsid, NULL);
         SysFreeString(wFileName);
         SysFreeString(wMode);
	      DeleteDC(hdcMem);
         ReleaseDC(hWnd,hdc);
         delete bmp;
      }
   }
   
   itfc->nrRetValues = 0;
   
   return(OK);
}

/*****************************************************************************************
  Copy a window image to the clipboard or a file in EMF format. Note that the window
   can be obscured. Uses the WM_PRINT event and currently doesn't work for Richedit
   3D plot or dividers.
******************************************************************************************/

void CopyWindow(WinData *win, CText &fileName, int x1clip, int x2clip, int y1clip, int y2clip)
{
   long width_tot;
   long height_tot;
   RECT rect;
   long xEdge, yEdge;
   HBITMAP hBmp1 = NULL;
   HBITMAP hBmp2 = NULL;
   HWND hWnd = win->hWnd;
   HDC hDCMem1 = CreateCompatibleDC(NULL);
   HDC hDCMem2 = CreateCompatibleDC(NULL);

   GetWindowRect(hWnd, & rect);
   long width = rect.right - rect.left;
   long height = rect.bottom - rect.top;

   xEdge = x1clip;
   yEdge = y1clip;
   width_tot = x2clip-x1clip;
   height_tot = y2clip-y1clip;

   HDC hDC = GetDC(hWnd);
   hBmp1 = CreateCompatibleBitmap(hDC, width, height);
   hBmp2 = CreateCompatibleBitmap(hDC, width_tot, height_tot);
   ReleaseDC(hWnd, hDC);

   HGDIOBJ hOld1 = SelectObject(hDCMem1, hBmp1);

	gWMPrintCalled = 1;
   SendMessage(hWnd, WM_PRINT, (WPARAM) hDCMem1, PRF_CHILDREN | PRF_CLIENT | PRF_ERASEBKGND | PRF_NONCLIENT | PRF_OWNED);
	gWMPrintCalled = 0;

   HGDIOBJ hOld2 = SelectObject(hDCMem2, hBmp2);
   BitBlt(hDCMem2,0,0, width_tot, height_tot, hDCMem1, xEdge, yEdge, SRCCOPY);

   if(fileName != "clipboard")
   {
      HDC mfHDC = CreateEnhMetaFile(NULL,fileName.Str(),NULL,NULL);
      if(mfHDC)
      {
         BitBlt(mfHDC,0,0,width_tot,height_tot,hDCMem2,0,0,SRCCOPY); // Not sure why I need the -2 but the window seems to be offset vertically otherwise
         HENHMETAFILE hemf = CloseEnhMetaFile(mfHDC); 
         DeleteEnhMetaFile(hemf); 
      }
   }
   else
   {
      OpenClipboard(hWnd);
      EmptyClipboard(); 
      SetClipboardData(CF_BITMAP, hBmp2);
      CloseClipboard();
   }

   SelectObject(hDCMem1, hOld1);
   SelectObject(hDCMem2, hOld2);
   DeleteObject(hDCMem1);
   DeleteObject(hDCMem2);
   DeleteObject(hBmp1);
   DeleteObject(hBmp2);

}

/*********************************************************************
 If a current window has been closed make sure there is another
 to take its place (if one exists)
*********************************************************************/

void RestoreCurrentWindows(HWND wIn)
{
   ObjectData *obj;
   WinData *win,*bak = NULL;
   HWND w;
  // int cnt = 0;

// All current windows are in place so return
	if(Plot1D::curPlot() && Plot2D::curPlot() && curEditor && cliEditWin && plot3d)
      return;

// Find top Prospa window by searching all windows from Prospa
   w = prospaWin; // Star with Prospa window
   do
   {
      win = GetWinDataClass(w); // Is it a Prospa window
    //  cnt++;
      if(win)
         bak = win;

 	   w = GetWindow(w,GW_HWNDPREV);  // Search up
   }
   while(w);

   if(bak)
   {
      win = bak;
      w = win->hWnd;
   }
   else
      return;

// Find top Prospa window
   do
   {
      if(win)
      {
			if(!Plot1D::curPlot() && (obj = win->widgets.findByType(PLOTWINDOW)))
         {
            PlotWindow *pp = (PlotWindow*)obj->data;
				pp->makeCurrentPlot();
				pp->makeCurrentDimensionalPlot();
            MyInvalidateRect(win->hWnd,NULL,false);
      //      TextMessage("New 1D plot %hd\n",win->nr);
         }
			if(!Plot2D::curPlot() && (obj = win->widgets.findByType(IMAGEWINDOW)))
         {
            PlotWindow *pp = (PlotWindow*)obj->data;
				pp->makeCurrentPlot();
				pp->makeCurrentDimensionalPlot();
            MyInvalidateRect(win->hWnd,NULL,false);
     //       TextMessage("New 2D plot %hd\n",win->nr);

         }
         if(!curEditor && (obj = win->widgets.findByType(TEXTEDITOR)))
         {
            EditParent *ep = (EditParent*)obj->data;
            curEditor = ep->editData[0];
      //      TextMessage("New editor %hd\n",win->nr);

         }
         if(!cliEditWin && (obj = win->widgets.findByType(CLIWINDOW)))
         {
            cliEditWin = obj->hWnd;
      //      TextMessage("New CLI %hd\n",win->nr);
         }
         if(!plot3d && (obj = win->widgets.findByType(OPENGLWINDOW)))
         {
            plot3d = (CPlot3D*)obj->data;
            cur3DWin = obj->hWnd;
      //      TextMessage("New 3D window %hd\n",win->nr);
         }
			if(Plot1D::curPlot() && Plot2D::curPlot() && curEditor && cliEditWin && plot3d)
            break;
      }

 	   w = GetWindow(w,GW_HWNDNEXT);  // Find next lower
     // cnt++;

      if(w == wIn) // Ignore removed window
         continue;

      win = GetWinDataClass(w); // Is it a Prospa window?
   }
   while(w);
}

// Hide those controls which are not visible in the current tab page
// win is the parent window and tab is the parent tab

void ControlVisibleWithTabs(WinData *win, ObjectData *tab)
{
	short tabNumber = TabCtrl_GetCurSel(tab->hWnd);

	for(ObjectData* obj: win->widgets.getWidgets())
   {
		if(obj != tab)
		{
         ObjectData *tabParent = obj->tabParent;

			if(tabParent == tab) // Only consider objects with a tab parent - ignore others
         {
            if(obj->tabPageNr == tabNumber) // On this tab page so make visible
			   {
				   obj->visible = true;
				   obj->Show(true);
               if(obj->type == PANEL)
                   obj->UpdatePanelThumb(false);
			   }
			   else // Not on this tab page so hide
			   {
				   obj->visible = false;
				   obj->Show(false);
			   }
         }
		}
   }  
}

// Convert a special color - only one at present it the background color [255,255,255,255]

int ConvertColor(Interface* itfc ,char args[])
{
   short r;
   Variable colVar;

// Get the argument   
   if((r = ArgScan(itfc,args,1,"color","e","v",&colVar)) < 0)
      return(r);

   if(colVar.GetType() == MATRIX2D && colVar.GetDimY() == 1)
   {
      int sz = colVar.GetDimX();
      float **mat = colVar.GetMatrix2D();

      if(sz == 3)
      {
         float col[3];
         col[0]  = nint(mat[0][0]);
         col[1]  = nint(mat[0][1]);
         col[2]  = nint(mat[0][2]);
         itfc->retVar[1].MakeMatrix2DFromVector(col,3,1);
         itfc->nrRetValues = 1;
      }
      else if(sz == 4)
      {
         float col[4];
         col[0]  = nint(mat[0][0]);
         col[1]  = nint(mat[0][1]);
         col[2]  = nint(mat[0][2]);
         col[3]  = nint(mat[0][3]);
         if(col[0] == 255 && col[1] == 255 && col[2] == 255 && col[3] == 255)
         {
            col[0] = GetRValue(GetSysColor(COLOR_BTNFACE));
            col[1] = GetGValue(GetSysColor(COLOR_BTNFACE));
            col[2] = GetBValue(GetSysColor(COLOR_BTNFACE));
            itfc->retVar[1].MakeMatrix2DFromVector(col,3,1);
            itfc->nrRetValues = 1;
         }
         else
         {
            ErrorMessage("invalid color vector");
            return(ERR);
         }
      }
      else
      {
         ErrorMessage("invalid color vector");
         return(ERR);
      }
   }
   else
   {
      ErrorMessage("invalid type for vector");
      return(ERR);
   }

   return(OK);
}


// Adds rectange described by (x1,y1,x2,y2) to region hRng

HRGN AddRectToRgn(HRGN hRgn, int x1, int y1, int x2, int y2)
{
   HRGN hRgnRect;
   hRgnRect = CreateRectRgn(x1,y1,x2,y2);
   CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
   DeleteObject(hRgnRect);
   return(hRgn);
}

/***************************************************************************************
   Return window text
***************************************************************************************/

void GetWindowTextEx(HWND hWnd, CText &txt)
{
   long n = GetWindowTextLength(hWnd);
   char *title = new char[n+1];
   n = GetWindowText(hWnd,title,n+1);
   title[n] = '\0';
   txt.Assign(title);
   delete [] title;
}


// Wait for the left mouse button to be released

void WaitButtonUp(HWND hWnd)
{
	MSG msg;

	while(1)
	{
		if(PeekMessage(&msg, hWnd,WM_LBUTTONUP, WM_LBUTTONUP,PM_REMOVE))
		{         
			if(msg.message == WM_LBUTTONUP) 
				break;

			DispatchMessage(&msg);	  
			TranslateMessage(&msg);
		}
	}
}

// Wait for left button down and return coordinates
short GetButtonDown(HWND hWnd, short &x, short &y)
{
	MSG msg;

	while(1)
	{
		if(gAbortMacro)
		{
			gAbortMacro = false;
			return(1);
		}

		if(PeekMessage(&msg, hWnd, WM_LBUTTONDOWN, WM_LBUTTONDOWN,PM_REMOVE))
		{         
			if(msg.message == WM_LBUTTONDOWN) 
			{
				x = LOWORD(msg.lParam);
				y = HIWORD(msg.lParam);
				//       SetCursor(LoadCursor(NULL,IDC_ARROW));
				break;
			}

			DispatchMessage(&msg);	  
			TranslateMessage(&msg);
		}
	} 

	return(0);
}

/*************************************************************************
*       See if a keyboard key has been pressed.
*************************************************************************/

bool IsKeyDown(int key)
{
	short state = GetKeyState(key); //GetAsyncKeyState(key);
	//  TextMessage("key = %d state = %d\n",key,state);
	return(state & 0x8000);
}

// Interpolates intermediate value at N
// X0 corresponds to N = 0 and X1 to N = 255
#define IPOL(X0, X1, N) ((X0) + ((X1) - (X0)) * N / 256)

// Fills the rectangle 'rect' in graded colours
void GradientFill(HDC hdc, RECT rect, LPCOLORREF color_array)
{
   // Calculates size of single colour bands
   int xStep = (rect.right - rect.left) / 256 + 1;
   int yStep = (rect.bottom - rect.top) / 256 + 1;
   // X loop starts
   for (int iX = 0, X = rect.left; iX < 256; iX++)
   {
      // Calculates end colours of the band in Y direction
      int RGBColor[3][2] = 
      {
         {
            IPOL(GetRValue(color_array[0]), GetRValue(color_array[1]), iX),
            IPOL(GetRValue(color_array[3]), GetRValue(color_array[2]), iX)
         },
         {
            IPOL(GetGValue(color_array[0]), GetGValue(color_array[1]), iX),
            IPOL(GetGValue(color_array[3]), GetGValue(color_array[2]), iX)
         },
         {
            IPOL(GetBValue(color_array[0]), GetBValue(color_array[1]), iX),
            IPOL(GetBValue(color_array[3]), GetBValue(color_array[2]), iX)
         }
      };
      // Y loop starts
      for (int iY = 0, Y = rect.top; iY < 256; iY++)
      {
         // Calculates the colour of the rectangular band
         COLORREF Color = RGB(IPOL(RGBColor[0][0], RGBColor[0][1], iY),
         IPOL(RGBColor[1][0], RGBColor[1][1], iY),
         IPOL(RGBColor[2][0], RGBColor[2][1], iY));
         // Creates the brush to fill the rectangle
         HBRUSH hBrush = CreateSolidBrush(Color);
         // Paints the rectangular band with the brush
         RECT Rect = {X, Y, X + xStep, Y + yStep};
         FillRect(hdc, &Rect, hBrush);
         // Deletes the brush
         DeleteObject(hBrush);
         // Updates Y value of the rectangle
         Y += yStep;
         if (Y > rect.bottom)
            Y = rect.bottom;
      }
      // Updates X value of the rectangle
      X += xStep;
      if (X > rect.right)
         X = rect.right;
   }
}



// Fills the region 'rgn' in graded colours
void GradientFill(HDC hdc, HRGN rgn, LPCOLORREF color_array)
{
   // Creates memory DC
   HDC hMemDC = CreateCompatibleDC(hdc);
   if (hMemDC) // Memory DC creation successful
   {
      // Gets bounding rectangle of region
      RECT rectRgn;
      GetRgnBox(rgn, &rectRgn);
      // Left top point of applying mask
      int X = rectRgn.left, Y = rectRgn.top;
      // Size of mask
      int Width = rectRgn.right - X, Height = rectRgn.bottom - Y;
      // Creates bitmap for the mask
      HBITMAP hBitmap = CreateCompatibleBitmap(hdc, Width, Height);
      if (hBitmap) // Bitmap created successfully
      {
         // Selects bitmap in memory DC
         HBITMAP hOldBitmap = (HBITMAP) SelectObject(hMemDC, hBitmap);
         // Prepares gradient filled mask and applies to output DC
         OffsetRect(&rectRgn, -rectRgn.left, -rectRgn.top);
         GradientFill(hMemDC, rectRgn, color_array);
         BitBlt(hdc, X, Y, Width, Height, hMemDC, 0, 0, SRCINVERT);
         // Displays region in black in output DC
         FillRgn(hdc, rgn, (HBRUSH) GetStockObject(BLACK_BRUSH));
         // Applies mask to output DC again
         BitBlt(hdc, X, Y, Width, Height, hMemDC, 0, 0, SRCINVERT);
         // De-selects bitmap from memory DC
         SelectObject(hMemDC, hOldBitmap);
         // Deletes bitmap
         DeleteObject(hBitmap);
      }
      // Deletes memory DC
      DeleteDC(hMemDC);
   }
}

int WhichObjectIsMouseOver(WinData *win)
{
	POINT coord;

	GetCursorPos(&coord);

	ObjectData *obj = win->widgets.findByScreenPosition(coord);

	if(obj)
	  return(obj->nr());

	return(-1);
}
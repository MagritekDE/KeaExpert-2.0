#pragma pack(push,8) // Must not use 1 byte packing for ACCEL structure
#include <windows.h>
#pragma pack(pop)
#include <gdiplus.h> 
#include "stdafx.h"
#include "guiObjectClass.h"
#include <shellapi.h>
#include <sstream>
#include "ax.h"
#include "BabyGrid.h"
#include "bitmap.h"
#include "cArg.h"
#include "cli_events.h"
#include "ctext.h"
#include "defineWindows.h"
#include "defines.h"
#include "drawing.h"
#include "edit_class.h"
#include "events_edit.h"
#include "globals.h"
#include "gridcontrol.h"
#include "guiModifyObjectParameters.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include "htmlviewer.h"
#include "interface.h"
#include "listbox.h"
#include "main.h"
#include "mymath.h"
#include "plot1dCLI.h"
#include "plot1dEvents.h"
#include "plot2dEvents.h"
#include "plot3dClass.h"
#include "plotwindefaults.h"
#include "PlotWindow.h"
#include "prospaResource.h"
#include "richedit.h"
#include "scanstrings.h"
#include "string_utilities.h"
#include "StringPairs.h"
#include "variablesOther.h"
#include <winuser.h>
#include <string>
#include "memoryLeak.h"

using namespace Gdiplus;

using std::string;
using std::stringstream;

#pragma warning (disable: 4312) // Ignore Get/SetWindowLong warnings
#pragma warning (disable: 4311) // Ignore pointer truncation warnings


WNDPROC OldButtonProc;
WNDPROC OldCLIProc;
WNDPROC OldHTMLProc;
WNDPROC OldGridProc;
WNDPROC OldStaticTextProc;
WNDPROC OldStatic2TextProc;
WNDPROC OldGroupBoxProc;
WNDPROC OldPanelProc;
WNDPROC OldPanelScrollProc;
WNDPROC OldTextMenuProc;
WNDPROC OldTextMenuEditProc;
WNDPROC OldTextBoxProc;
WNDPROC OldProgressBarProc;
WNDPROC OldStatusBarProc;
WNDPROC OldSliderProc;
WNDPROC OldColorScaleProc;  // FIXME: Not defined anywhere
WNDPROC OldUpDownProc;
WNDPROC OldDebugStripProc;
WNDPROC OldTabCtrlProc;
WNDPROC OldPictureProc;

DWORD g_objVisibility = WS_VISIBLE;    // Whether a created object will be immediately visible or not

static const char* DEFAULT_OBJECT_STRING = "undefined";

bool win7Mode = false;

// Constructor

ObjectData::ObjectData()
{
   validationCode = GetNextObjectValidationCode();
   data = (char*)0;
   command = (char*)0;
   type = -1;
   strcpy(objName,DEFAULT_OBJECT_STRING);
   strcpy(valueName,DEFAULT_OBJECT_STRING);
   strcpy(cb_event,DEFAULT_OBJECT_STRING);
   strcpy(tag,DEFAULT_OBJECT_STRING);
	lower = 0;
	upper = 0;
   rangeCheck = false;
   active = false;
   tabNr = NO_TAB_NUMBER;
   nr(-1);
	dataType = NULL_VARIABLE;
   visible = true;
   enable = true;
   xSzScale = 0;  
   xSzOffset = 0;
   ySzScale = 0;
   ySzOffset = 0;
   wSzScale = 0;
   wSzOffset = 0;
   hSzScale = 0;
   hSzOffset = 0;
   toolbar = -1;
   statusbox = -1;
   menuList = NULL;
   menuListSize = 0;
   region.left = 0;
   region.right = 0;
   region.top = 0;
   region.bottom = 0;
   regionSized = false;
   menu = NULL;
   accelTable = NULL;
   debug = false;
   eventActive = true;
   winParent = NULL;
   toolTipHwnd = NULL;
   readOnly = false;
   readOnlyOutput = false;
   keepFocus = false;
   seqMenuNr = -1;
   hWnd = NULL;
   hwndParent = NULL;
	tabPageNr = NO_TAB_PAGE;
	tabParent = 0;
   panelParent = NULL;
   showFrame = false;
   fgColor = RGB(0,0,0);
   bgColor = RGB(255,255,255) + (255<<24);
   varList.next = NULL;
   varList.last = NULL;
	highLiteChanges = false;
	valueChanged = false;
	acceptKeyEvents = false;

}

ObjectData::ObjectData(WinData *parent, short type, short objNr, 
                      long x, long y, long w, long h, 
                      char *label, char *cmd, char *objInfo, DWORD visibility, long lineNr)
{
   validationCode = GetNextObjectValidationCode();
	POINT pt;
	this->menuList = 0;
	this->lower = 0;
	this->upper = 0;
   rangeCheck = false;
   active = false;
   tabNr = NO_TAB_NUMBER;
   dataType = NULL_VARIABLE;
   visible = (visibility==WS_VISIBLE); //true;
   enable = true;
   xSzScale = 0;  
   xSzOffset = 0;
   ySzScale = 0;
   ySzOffset = 0;
   wSzScale = 0;
   wSzOffset = 0;
   hSzScale = 0;
   hSzOffset = 0;
   toolbar = -1;
   statusbox = -1;
   menuList = NULL;
   menuListSize = 0;
   region.left = 0;
   region.right = 0;
   region.top = 0;
   region.bottom = 0;
   regionSized = false;
   menu = NULL;
   accelTable = NULL;
   debug = false;
   eventActive = true;
   winParent = NULL;
   readOnly = false;
   readOnlyOutput = false;
   keepFocus = false;
   toolTipHwnd = NULL;
   seqMenuNr = -1;
   hWnd = NULL;
   hwndParent = NULL;
	tabPageNr = NO_TAB_PAGE;
	tabParent = NULL;
   panelParent = NULL;
   showFrame = false;
   fgColor = RGB(0,0,0);
   bgColor = RGB(255,255,255) + (255<<24);
   varList.next = NULL;
   varList.last = NULL;
	highLiteChanges = false;
	valueChanged = false;
   acceptKeyEvents = false;

   this->data = (char*)0;
	this->type = type;
   strcpy(objName,DEFAULT_OBJECT_STRING);
   strcpy(valueName,DEFAULT_OBJECT_STRING);
   strcpy(cb_event,DEFAULT_OBJECT_STRING);
   strcpy(tag,DEFAULT_OBJECT_STRING);
	nr(objNr);
	this->command = new char[strlen(cmd) + 1];
	strncpy(this->command, cmd, strlen(cmd));
	this->command[strlen(cmd)] = '\0';
	this->xo = x;
   this->yo = y;
   this->wo = w;
   this->ho = h;
	this->inError = false;
	this->selected_ = false;
	this->cmdLineNr = lineNr;
   HWND win = parent->hWnd;
 	this->hwndParent = win;
   this->winParent = parent;
	
 	switch(type)
 	{
      case(MENU):
		{
         this->seqMenuNr = parent->nrMenuObjs++;
		   this->hWnd = NULL;	   
		   break; 
	   }
 	   case(GETMESSAGE):
		{
		   this->hWnd = NULL;
		   this->data = objInfo;		   
		   break; 
	   }
      case(TEXTEDITOR):
      {
         break;
      }
      case(DEBUGSTRIP):
      {
		   this->hWnd = CreateWindow("button", "", WS_CHILD | visibility | BS_OWNERDRAW,
	      			                x, y, w, h, 
	      			                this->hwndParent, (HMENU)objNr,
	      			                prospaInstance, NULL);
		   OldDebugStripProc = (WNDPROC)SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)DebugStripEventProc);
		   break; 
      }
		case(UPDOWN):
		{
	      long dir;
	 
	      if(!strncmp(label,"horizontal",5))
	        dir = UDS_HORZ;
	      else
	        dir = 0;

		   this->hWnd = CreateUpDownControl(WS_CHILD  | visibility |  dir | UDS_ARROWKEYS, //WS_BORDER
	      			                x, y, w, h, 
	      			                this->hwndParent, (int)objNr,
	      			                prospaInstance, NULL, 2, 0, 1);
		   OldUpDownProc = (WNDPROC)SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)UpDownEventProc);
		   break; 
	   }
		case(TABCTRL):
		{
         this->hWnd = CreateWindow(WC_TABCONTROL,			
							"",
							WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | visibility | TCS_OWNERDRAWFIXED |  TCS_MULTILINE,
							x,y,w,h,
							(HWND)this->hwndParent,
							(HMENU)objNr,
							(HINSTANCE)prospaInstance,
							(LPSTR)NULL);

         

      //   long style = GetWindowLong(this->hWnd,GWL_STYLE);
      //   SetWindowLong(this->hWnd,GWL_STYLE,(~CS_HREDRAW) & (~CS_VREDRAW) & style);
		   OldTabCtrlProc = (WNDPROC)SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)TabCtrlEventProc);
         break; 
	   }
      case(DIVIDER):
      {
         this->hWnd = CreateWindow("DIVIDER",			
							"",
							WS_CHILD | visibility,
							x,y,w,h,
							(HWND)this->hwndParent,
							(HMENU)objNr,
							(HINSTANCE)prospaInstance,
							(LPSTR)NULL);

		//   SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)DividerEventsProc);
         break;
      }
      case(PLOTWINDOW):
      {
         this->hWnd = CreateWindow("PLOT",			
							"1D Plot",
							WS_CHILD | visibility,
							x,y,w,h,
							(HWND)this->hwndParent,
							(HMENU)objNr,
							(HINSTANCE)prospaInstance,
							(LPSTR)NULL);

		   SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)PlotEventsProc);
         break;
      }
      case(IMAGEWINDOW):
      {
         this->hWnd = CreateWindow("PLOT",			
							"2D Plot",
							WS_CHILD | visibility,
							x,y,w,h,
							(HWND)this->hwndParent,
							(HMENU)objNr,
							(HINSTANCE)prospaInstance,
							(LPSTR)NULL);

		   SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)ImageEventsProc);
         break;
      }
      case(OPENGLWINDOW):
      {
          this->hWnd = CreateWindow("OPENGL",			
							"3D Plot",
							WS_CHILD | visibility,
							x,y,w,h,
							(HWND)this->hwndParent,
							(HMENU)objNr,
							(HINSTANCE)prospaInstance,
							(LPSTR)NULL);

         break;
      }
      case(TOOLBAR):
      {
         UINT nrBut = x;
         HBITMAP hbitmap = (HBITMAP)label;
         TBBUTTON *lpButtons = (TBBUTTON*)objInfo;
         //obj->hWnd = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, 
         //                          visibility | TBSTYLE_FLAT | WS_CHILD  | WS_BORDER | TBSTYLE_TOOLTIPS | TBSTYLE_WRAPABLE,
         //                           0, 0, 16*nrBut, 15,
         //                           obj->hwndParent, NULL, prospaInstance, NULL);
         this->hWnd = CreateToolbarEx((HWND)this->hwndParent,			
							visibility | TBSTYLE_FLAT | WS_CHILD  | WS_BORDER | TBSTYLE_TOOLTIPS | TBSTYLE_WRAPABLE,
                     (DWORD)objNr,
							(UINT)nrBut,
							(HINSTANCE)NULL,
							(UINT)hbitmap,
                     lpButtons,
							nrBut,
							16,15,16,15,
                     sizeof(TBBUTTON));

         break;
      }
      case(CLIWINDOW):
      {
         this->hWnd = CreateWindowEx (WS_EX_STATICEDGE, RICHEDIT_CLASS, NULL,
			                        WS_CHILD | visibility | WS_HSCROLL | WS_VSCROLL | // | WS_DLGFRAME	|
			                        ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
			                        x,y,w,h,
                                 this->hwndParent, NULL,
		                           prospaInstance, NULL); 

         OldCLIProc = (WNDPROC)SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)CLIEditEventsProc); 
         break;
      }
      case(HTMLBOX): // Active-X HTML viewer control
      {
         this->hWnd = CreateWindowEx(NULL,
                    "AX",
                     "{8856F961-340A-11D0-A96B-00C04FD705A2}",
                     WS_CHILD | visibility,
	      			   x, y, w, h, 
                     this->hwndParent,
                     NULL,
                     prospaInstance,
                     NULL);
		   OldHTMLProc = (WNDPROC)SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)HTMLEventProc);

         break;

      }

		case(GRIDCTRL):
		{
			int cols = ((GridCtrlInfo*)(objInfo))->cols;
			int rows = ((GridCtrlInfo*)(objInfo))->rows;
	
			delete objInfo;
			this->data = 0;
			this->hWnd = CreateWindowEx(NULL,
				"BABYGRID",
				"",
				visibility | WS_CHILD | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_HSCROLL | WS_VSCROLL , x, y, w, h, 
				this->hwndParent,
				(HMENU)objNr,
				prospaInstance,
				NULL);
			BabyGrid* g = new BabyGrid(this->hWnd, prospaInstance);
		   this->data = (char*)g;	
			OldGridProc = (WNDPROC)SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)GridEventProc);
			// Set initial grid dimensions
			g->setColCount(cols);
			g->setRowCount(rows);
			//////
			// TODO: Make these the defaults in the constructor.
			//////
			// Hide the header row by default.
			g->setHeaderRowHeight(0);
//		g->setGridHeight(200);
			// Hide the title row.
			g->setTitleHeight(0);
			// Ensure the grid is editable.
			g->setEditable(true);
			// Don't allow the grid to shrink itself.
			g->setAutoRow(false);
			// Don't let the last column grow to fill the space available.
			g->extendLastColumn(false);
			// Use any titles set, rather than numbers for rows and alpha for cols.
			g->setRowsNumbered(false);
			g->setColsNumbered(false);
			g->setZeroBased(true);
			ShowScrollBar(this->hWnd, SB_BOTH, false);
			break;
		}

		case(BUTTON):
		{
		   this->data = objInfo;
         if(win7Mode)
         {
		      this->hWnd = CreateWindow("button", label, WS_CHILD  | BS_PUSHBUTTON | visibility | BS_OWNERDRAW,
	      			                   x, y, w, h, 
	      			                   this->hwndParent, (HMENU)objNr,
	      			                   prospaInstance, NULL);
         }
         else
         {
            this->hWnd = CreateWindow("button", label, WS_CHILD  | BS_PUSHBUTTON | visibility | BS_MULTILINE,
	      			                   x, y, w, h, 
	      			                   this->hwndParent, (HMENU)objNr,
	      			                   prospaInstance, NULL);
         }

		   OldButtonProc = (WNDPROC)SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)ButtonEventProc);
		   break; 
	   }
		case(COLORBOX):
		{
		   this->data = objInfo;
		   this->hWnd = CreateWindow("button", "", WS_CHILD | visibility | BS_OWNERDRAW,
	      			                x, y, w, h, 
	      			                this->hwndParent, (HMENU)objNr,
	      			                prospaInstance, NULL);
		   OldButtonProc = (WNDPROC)SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)ButtonEventProc);
		   break; 
	   }

		case(PICTURE):
		{
		   this->hWnd = CreateWindow("button", "", WS_CHILD | visibility | BS_OWNERDRAW,
	      			                x, y, w, h, 
	      			                this->hwndParent, (HMENU)objNr,
	      			                prospaInstance, NULL);
		   OldPictureProc = (WNDPROC)SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)PictureEventProc);
		   break; 
	   }

		case(LISTBOX):
		{
         MakeListBoxObject(this, x, y, w, h, visibility);
		   break; 
	   }
      case(GROUP_BOX):
	   {
		   this->hWnd = CreateWindow("static", label, WS_CHILD  | SS_OWNERDRAW	 | visibility,
	      			                x, y, w, h, 
	      			                this->hwndParent, (HMENU)objNr,
	      			                prospaInstance, NULL); 
		   OldGroupBoxProc = (WNDPROC)SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)GroupBoxEventProc); 
		   break;
	   }	   
      case(PANEL):
	   {
		   PanelInfo *info = (PanelInfo*)objInfo;
		   this->data = (char*)info;

			int width = GetSystemMetrics(SM_CXVSCROLL);

		   this->hWnd = CreateWindowEx(WS_EX_WINDOWEDGE,"SCROLLBAR", "", WS_CHILD 	| visibility | SBS_VERT | SBS_RIGHTALIGN,
	      			                x+w-width, y, width, h, 
	      			                this->hwndParent, (HMENU)objNr,
	      			                prospaInstance, NULL); 

         info->hWndPanel = CreateWindowEx(WS_EX_STATICEDGE/*WS_EX_CLIENTEDGE*/,"static", label, WS_CHILD | visibility,
	      			                x, y, w-width, h, 
	      			                this->hwndParent, (HMENU)objNr,
	      			                prospaInstance, NULL); 


		   OldPanelScrollProc = (WNDPROC)SetWindowLong(this->hWnd ,GWL_WNDPROC,(LONG)PanelScrollEventProc); 
		   OldPanelProc = (WNDPROC)SetWindowLong(info->hWndPanel ,GWL_WNDPROC,(LONG)PanelEventProc); 

         SCROLLINFO si;
         si.cbSize = sizeof(SCROLLINFO);
         si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE | SIF_DISABLENOSCROLL;
         si.nMin = 0;
         si.nMax = 100;
         si.nPos = 0;
         si.nPage = 0;

         SetScrollInfo(this->hWnd , SB_CTL, &si, true);

		   break;
	   }
      case(TEXTMENU):
	   {
		   this->hWnd = CreateWindowEx(NULL,"combobox", "", WS_CHILD  | CBS_HASSTRINGS | CBS_DROPDOWN | CBS_AUTOHSCROLL /*| WS_HSCROLL*/ | WS_VSCROLL  | visibility, //| CBS_SORT,
	      			                x, y, w, h, 
	      			                this->hwndParent, (HMENU)objNr,
	      			                prospaInstance, NULL);
         this->ho = 21; // Sets the size for designer manipulation
         pt.x = 3; 
         pt.y = 3; 
         HWND hwndEdit = RealChildWindowFromPoint(this->hWnd, pt);  
		   OldTextMenuEditProc = (WNDPROC)SetWindowLong(hwndEdit,GWL_WNDPROC,(LONG)TextMenuEditEventProc); 
		   OldTextMenuProc = (WNDPROC)SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)TextMenuEventProc); 
		   break;
	   }   
      case(TEXTBOX):
      {
			this->hWnd = CreateWindowEx (WS_EX_CLIENTEDGE,"edit", NULL,
							WS_CHILD  | ES_LEFT | ES_AUTOHSCROLL | visibility,
							x, y, w, h, 
	                  this->hwndParent, (HMENU) objNr,
							prospaInstance, NULL);
		   OldTextBoxProc = (WNDPROC)SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)TextBoxEventProc); 
		   break;
	   }
	   case(STATICTEXT):
      {
	      long xp = x;
	      long mode = SS_LEFT;
	      HDC hdc = GetDC(win);
	      SIZE size;
	      
	      SelectObject(hdc,controlFont);
	      GetTextExtentPoint32(hdc,label,strlen(label),&size);

         if(w == 0)
            w = size.cx;
         if(h == 0)
            h = size.cy;
	
	      if(!strcmp(cmd,"left"))
	         xp = x, mode = SS_LEFT;
	      else if(!strcmp(cmd,"right"))
	         xp = x - w, mode = SS_RIGHT;
	      else if(!strcmp(cmd,"centre") || !strcmp(cmd,"center"))
	         xp = x - w/2, mode = SS_CENTER;

         this->data = objInfo;
	         
			this->hWnd = CreateWindow ("static", label,
							WS_CHILD  | mode | visibility	,
							xp, y, w, h,
	                  this->hwndParent, (HMENU) objNr,
							prospaInstance, NULL); 
		   OldStaticTextProc = (WNDPROC)SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)STextEventProc);
							
		   ReleaseDC(win,hdc);
		   this->xo = xp;
		   this->yo = y; 
		   this->wo = w;
		   this->ho = h;
		   break;
	   }
	   case(STATICTEXT2):
      {
	      long xp = x;
	      long mode = SS_LEFT;
	      HDC hdc = GetDC(win);
	      SIZE size;
	      
	      SelectObject(hdc,controlFont);
	      GetTextExtentPoint32(hdc,label,strlen(label),&size);
	      
	      long w = size.cx;
	      long h = size.cy;
	
	      if(!strcmp(cmd,"left"))
	         xp = x, mode = SS_LEFT;
	      else if(!strcmp(cmd,"right"))
	         xp = x - w, mode = SS_RIGHT;
	       else if(!strcmp(cmd,"center"))
	         xp = x - w/2, mode = SS_CENTER;
	         
			this->hWnd = CreateWindow ("static", "",
							WS_CHILD  | mode | visibility,
							xp, y, w, h,
	                  this->hwndParent, (HMENU) objNr,
							prospaInstance, NULL); 
		   OldStatic2TextProc = (WNDPROC)SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)SText2EventProc);
							
		   ReleaseDC(win,hdc);
		   this->xo = xp;
		   this->yo = y; 
		   this->wo = w;
		   this->ho = h;
		   break;
	   }	   
	   case(PROGRESSBAR):
	   {
	      long dir;
	      
	      if(!strcmp(label,"vertical"))
	         dir = PBS_VERTICAL;
	      else
	         dir = 0;
	      
	      this->hWnd =   CreateWindow(PROGRESS_CLASS, label,
	                    WS_CHILD  | /*WS_BORDER |*/ PBS_SMOOTH | dir | visibility,
	                    x, y, w, h,
	                    this->hwndParent, (HMENU)objNr,  
						     prospaInstance, NULL);

         OldProgressBarProc = (WNDPROC)SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)ProgressBarEventProc);
			break;			     					     
	   }
	   case(STATUSBOX):
	   {
	      RECT rc;
			this->hWnd = CreateWindow(STATUSCLASSNAME,"",
			                         WS_CHILD | visibility,
			                         0,0,0,0,
			                         this->hwndParent,(HMENU)objNr,prospaInstance,NULL);

		   OldStatusBarProc = (WNDPROC)SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)StatusBarEventProc);
		   GetWindowRect(this->hWnd,&rc);
         ScreenToClientRect(this->hwndParent,&rc);
		   this->xo = rc.left;
		   this->yo = rc.top; 
		   this->wo = rc.right-rc.left;
		   this->ho = rc.bottom-rc.top;
		   break;
	   } 
	   case(SLIDER):
	   { 
	      long dir;
	 
	      if(!strncmp(label,"horizontal",5))
	        dir = TBS_HORZ;
	      else
	        dir = TBS_VERT;
	     
	      this->hWnd = CreateWindow(TRACKBAR_CLASS, "",
	                 WS_CHILD  |  TBS_AUTOTICKS | TBS_NOTICKS  /*TBS_ENABLESELRANGE*/ | WS_TABSTOP | dir | visibility, 
	                 x, y, w, h,  
	                 this->hwndParent, (HMENU)objNr,  
						  prospaInstance, NULL);
	
		   OldSliderProc = (WNDPROC)SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)SliderEventProc);
		   break;
	   }
	   case(CHECKBOX):
		{
		   CheckButtonInfo *info = (CheckButtonInfo*)objInfo;
		   this->data = objInfo;
		   this->hWnd = CreateWindow("button", label, WS_CHILD  | BS_AUTOCHECKBOX | visibility,
	      			                x, y, 14,13, 
	      			                this->hwndParent, (HMENU)objNr,
	      			                prospaInstance, NULL);
	      SendMessage(this->hWnd,BM_SETCHECK,info->init-1,0);
	      this->wo = 14;
	      this->ho = 13;
		   OldButtonProc = (WNDPROC)SetWindowLong(this->hWnd,GWL_WNDPROC,(LONG)ButtonEventProc); 
	      break;
	   }
	   case(RADIO_BUTTON):
		{
		   RadioButtonInfo *info = (RadioButtonInfo*)objInfo;
		   int xp = x;
		   int yp = y;
		   
		   this->data = objInfo;
	      
		   for(int i = 0; i < info->nrBut; i++)
		   {
		      info->hWnd[i] = CreateWindow("button", label, WS_CHILD | BS_RADIOBUTTON | visibility,
	      			                   xp, yp, 14, 14, 
	      			                   this->hwndParent, (HMENU)objNr,
	      			                   prospaInstance, NULL);
		      OldButtonProc = (WNDPROC)SetWindowLong(info->hWnd[i],GWL_WNDPROC,(LONG)ButtonEventProc); 
	      			                   
	      	if(i == info->init-1)
	      		SendMessage(info->hWnd[i],BM_SETCHECK,1,0);
			                   
	          if(info->orient == 'h')
		         xp += info->spacing;
		      else
		         yp += info->spacing;
	      }
	      
	      this->xo = x;
	      this->yo = y;
	      if(info->orient == 'h')
	      {
	         this->wo = (info->nrBut-1)*info->spacing+14;
	         this->ho = 14;
	      }
	      else
	      {
	         this->wo = 14;
	         this->ho = (info->nrBut-1)*info->spacing+14;
	      }
	      break;
	   }
	}
      
   SendMessage(this->hWnd,WM_SETFONT,(WPARAM)controlFont ,false);
      
// Add a reference to the obj->hWnd so we don't have to search for the ObjectData structure

   if(this->hWnd != NULL && type != HTMLBOX) // && type != RADIO_BUTTON && type != MENU  && type != TEXTEDITOR && type != GETMESSAGE)
   {
      WinInfo* wi = new WinInfo;
      wi->winType = OBJDATA;
      wi->data = (char*)this;

      SetWindowLong(this->hWnd,GWL_USERDATA,(LONG)wi);
   }
}

// Destructor

ObjectData::~ObjectData()
{
   FreeData();
}

/*****************************************************************************
*                 Process an object class function call
*
* Plots & Images
* obj->multiplot(2,2) ... make a new multi-region plot
* obj->subplot(1,2) ..... returns particular plot region (1 based)
* obj->load("file") ..... load file into ??? NEEDS THOUGHT - ALSO ADD to plot func
* obj->save("file") ..... save all plots to a .pt1/2 file - ALSO ADD to plot func
* obj->size() ........... returns number of plot regions in each dimension
* obj->border(bool) ..... show or hide plot border
*
* General object commands
*
* obj->get(par) ......... get parameter value
* obj->par() ............ get parameter value
* obj->set(par,value) ... set parameter value
* obj->par(value) ....... set parameter value
*
*****************************************************************************/

short ProcessObjectClassReferences(Interface *itfc, ObjectData *obj, char *name, char *args)
{

// Object specific parameters -> move to plotwindow and imagewindow
   if(obj->type == PLOTWINDOW || obj->type == IMAGEWINDOW)
   {
     if(!strcmp(name,"multiplot")) // Make a muli-regioned plot e.g. obj->multiplot(2,2)
     {
        short nx,ny;
        CText mode = "discard";

         PlotWindow *pp = (PlotWindow*)obj->data;
         if(pp)
         {
            if(ArgScan(itfc,args,2,"","eee","ddt",&nx,&ny,&mode) < 0)
            {
               ErrorMessage("Invalid multiplot dimensions");
               return(ERR);
            }
				Margins *margins;
				if(obj->type == PLOTWINDOW)
					margins = &defaultMargins1D;
				else
					margins = &defaultMargins2D;

            if(mode == "discard")
               pp->MakeMultiPlot(nx,ny);  
            else
               pp->AdjustMultiPlotSize(nx,ny);

            pp->DisplayAll(false);

         // Return all the subplots
            itfc->nrRetValues = 0;
            for(int i = 0; i < pp->rows*pp->cols; i++)
            {
               Plot *pd = pp->plotList()[i];
               if(pd)
               {
                  itfc->retVar[i+1].MakeClass(PLOT_CLASS,(void*)pd);
                  itfc->nrRetValues++;
               }
               else
               {
                  ErrorMessage("Invalid subplot"); // Shouldn't happen
                  return(ERR);
               }
            }
         }
         return(OK);
	   }
      else if(!strcmp(name,"keepsubplot")) // Keep a subplot region e.g. obj->keepsubplot(1,2) or obj->keepsubplot(pr)
      {
         if(obj->data)
         {
            PlotWindow *pp = (PlotWindow*)obj->data;
            if(pp)
            {
               CArg carg;
               short rx = -1,ry = -1;
               int nrArgs = carg.Count(args);

               if(nrArgs == 1) // Plot class option
               {
                  Variable var;
                  nrArgs = ArgScan(itfc,args,1,"","e","v",&var);

                  if((nrArgs < 0) | (var.GetType() != CLASS))
                  {
                     ErrorMessage("Invalid region");
                     return(ERR);
                  }
                  ClassData *cData = (ClassData*)var.GetData();
                  if(cData)
                  {
                     Plot *pd = (Plot*)cData->data;
                     if(pd)
                     {
                        rx = pd->colNr+1;
                        ry = pd->rowNr+1;
                        if(pp->RemoveAllButOneSubPlot(rx,ry) == ERR)
                          return(ERR);
                        pp->DisplayAll(false);
                        return(OK);
                     }
                  }
                  ErrorMessage("Invalid region");
                  return(ERR);
               }
               else if(nrArgs == 2) // col,row option
               {
                  if(ArgScan(itfc,args,2,"","ee","dd",&rx,&ry) < 0)
                  {
                     ErrorMessage("Invalid region indices");
                     return(ERR);
                  }
                  if(rx < 1 || rx > pp->cols || ry < 1 || ry > pp->rows)
                  {
                     ErrorMessage("Invalid region indices");
                     return(ERR);
                  }
                  if(pp->RemoveAllButOneSubPlot(rx,ry) == ERR)
                    return(ERR);
                  pp->DisplayAll(false);
                  return(OK);
               }
            }
         }
         return(OK);
      } 
      else if(!strcmp(name,"load")) // Load a data set into a new plot
      {
			PlotWindow *pp = (PlotWindow*)obj->data;
         if(pp)
         {
            CText fileName;

            if(ArgScan(itfc,args,1,"","e","t",&fileName) < 0)
            {
               ErrorMessage("Invalid multiplot dimensions");
               return(ERR);
            }

            if(pp->dim() == 1)
				{
					pp->setPlotInsertMode(ID_NEW_PLOT);
               PlotWindow1D* pp1 = static_cast<PlotWindow1D*>(pp);
               pp1->LoadPlots(".",fileName.Str()); 
               pp1->DisplayAll(false); 
         	}
            else if(pp->dim() == 2)
				{
					pp->setPlotInsertMode(ID_NEW_PLOT);
					PlotWindow2D* pp2 = static_cast<PlotWindow2D*>(pp);
               pp2->LoadPlots(".",fileName.Str()); 
               pp2->DisplayAll(false); 
         	}
          //  MyInvalidateRect(pp->hWnd,NULL,false);
            itfc->nrRetValues = 0;
         }
         return(OK);
	   }
	   else if(!strcmp(name,"usedefaults"))
      {
			PlotWindow *pp = (PlotWindow*)obj->data;
         int r;

         CText defaults = "false";

         if(pp->getUseDefaultParameters())
            defaults = "true";

      // Get arguments from user *************
         if((r = ArgScan(itfc,args,1,"use defaults?","e","t",&defaults)) < 0)
            return(r);

         if(defaults == "true")
            pp->setUseDefaultParameters(true);
         else
            pp->setUseDefaultParameters(false);

         itfc->nrRetValues = 0;
         return(OK);
      }
      else if(!strcmp(name,"save")) // Save all subplots to 1 plot file
      {
         extern void GetExtension(char *file, char *extension);

         short err = OK;
			PlotWindow *pp = (PlotWindow*)obj->data;
         if(pp)
         {
            CText fileName;
            if(ArgScan(itfc,args,1,"","e","t",&fileName) < 0)
            {
               ErrorMessage("Invalid argument to save command");
               return(ERR);
            }

            char ext[MAX_STR];
            GetExtension(fileName.Str(),ext);

				if (!strcmp(ext,"pt1") || !strcmp(ext,"pt2"))
				{
					err = pp->SavePlots(".",fileName.Str(),-1,-1); 
				}
            else
					err = pp->SaveAsImage(fileName.Str());
            itfc->nrRetValues = 0;
         }
         return(err);
	   }
      else if(!strcmp(name,"subplot")) // Return a plot region e.g. obj->subplot(1,2)
      {
         if(obj->data)
         {
            PlotWindow *pp = (PlotWindow*)obj->data;
            if(pp)
            {
               short rx,ry;
               if(ArgScan(itfc,args,2,"","ee","dd",&rx,&ry) < 0)
               {
                  ErrorMessage("Invalid region indices");
                  return(ERR);
               }
               rx -= 1;
               ry -= 1;
               if(rx < 0 || rx >= pp->cols || ry < 0 || ry >= pp->rows)
                {
                  ErrorMessage("Invalid region indices");
                  return(ERR);
               }
               Plot *pd = pp->plotList()[rx+pp->cols*ry];
               if(pd)
               {
                  itfc->retVar[1].MakeClass(PLOT_CLASS,(void*)pd);
                  itfc->nrRetValues = 1;
                  return(OK);
               }
            }
         }
         return(ERR);
      } 
      else if(!strcmp(name,"size")) // Return the size of the subplot matrix
      {
       //  if(obj->data)
       //  {
            PlotWindow *pp = (PlotWindow*)obj->data;
            if(pp)
            {
               itfc->retVar[1].MakeAndSetFloat(pp->cols);
               itfc->retVar[2].MakeAndSetFloat(pp->rows);
               itfc->nrRetValues = 2;
               return(OK);
            }
            else // Shouldn't happen
            {
               ErrorMessage("plot object has no plotparent");
               return(ERR);
            }
         }
         //else // Shouldn't happen
         //{
         //   ErrorMessage("plot object has no data");
         //   return(ERR);
         //}
    //  }
      else if(!strcmp(name,"draw"))
      {
         if(obj->data)
         {
            PlotWindow *pp = (PlotWindow*)obj->data;
            if(pp)
            {
               CText draw;
               if(ArgScan(itfc,args,1,"true/false","e","t",&draw) == ERR)
                  return(ERR);
               
               if(draw == "true" || draw == "yes" || draw == "on")
               {
                  pp->updatePlots(true);
               }
               else if(draw == "false" || draw == "no" || draw == "off")
               {
						pp->updatingPlots(true);
                  pp->updatePlots(false);
               }
               else
               {
                  ErrorMessage("invalid argument");
                  return(ERR);
               }

               pp->DisplayAll(false);  
      
					if(draw == "true" || draw == "yes" || draw == "on")
						pp->updatingPlots(false);

               itfc->nrRetValues = 0;
               return(OK);
            }
            else // Shouldn't happen
            {
               ErrorMessage("plot object has no plotparent");
               return(ERR);
            }
         }
         else // Shouldn't happen
         {
            ErrorMessage("plot object has no data");
            return(ERR);
         }
      }
      else if(!strcmp(name,"border")) // Show or hide subplot border
      {
         if(obj->data)
         {
            PlotWindow *pp = (PlotWindow*)obj->data;
            CText mode;
            if(ArgScan(itfc,args,1,"","e","t",&mode) < 0)
            {
               ErrorMessage("Invalid border mode");
               return(ERR);
            }
            if(mode == "show" || mode == "true")
               pp->showLabels = true;
            else if(mode == "hide" || mode == "false")
               pp->showLabels = false;
            else if(mode == "toggle")
               pp->showLabels = !pp->showLabels;

				pp->obj->winParent->setMenuItemCheck(pp->menuName(),"toggle_border", pp->showLabels);

            if(pp->updatePlots())
            {
               MyInvalidateRect(pp->hWnd,NULL,false);
               MyUpdateWindow(pp->hWnd);
            }  
            itfc->nrRetValues = 0;
            return(OK);
         }
      }

		else if(!strcmp(name,"bordercolor"))
		{
         if(obj->data)
         {
				Variable colorVar;
				COLORREF col;
            PlotWindow *pp = (PlotWindow*)obj->data;

			// Get arguments from user *************
				if(ArgScan(itfc,args,1,"color","e","v",&colorVar) == ERR)
					return(ERR); 
	       
				if(ConvertAnsVar(&colorVar,"border color", colorVar.GetType(), col) == ERR)
					return(ERR);

				for(Plot* p: pp->plotList())
				{
					p->borderColor = col;
				}

			// Redraw
				MyInvalidateRect(pp->hWnd,NULL,false);
				itfc->nrRetValues = 0;

				return(OK);
			}
		}
		else if(!strcmp(name,"bkcolor") || !strcmp(name,"bkgcolor"))
		{
         if(obj->data)
         {
				Variable colorVar;
				COLORREF col;
				PlotWindow *pp = (PlotWindow*)obj->data;

			// Get arguments from user *************
				if(ArgScan(itfc,args,1,"color","e","v",&colorVar) == ERR)
					return(ERR); 
	       
				if(ConvertAnsVar(&colorVar,"background color", colorVar.GetType(),col) == ERR)
					return(ERR);

				for(Plot* p: pp->plotList())
				{
					p->bkColor = col;
				}

			// Redraw
				MyInvalidateRect(pp->hWnd,NULL,false);
				itfc->nrRetValues = 0;
				return(OK);
		   }
		}

      else if (!strcmp(name, "margins")) // Get or set the subplot margins
     {
        if (obj->data)
        {
           PlotWindow* pp = (PlotWindow*)obj->data;
           short n;

           short left, right, top, base;
           Plot* first = pp->plotList()[0];
           if (first)
           {
              Margins& originalMargins = first->getMargins();
              left = originalMargins.left();
              right = originalMargins.right();
              top = originalMargins.top();
              base = originalMargins.base();
           }

           if (!args || args[0] == '\0')
           {
              itfc->retVar[1].MakeAndSetFloat(left);
              itfc->retVar[2].MakeAndSetFloat(top);
              itfc->retVar[3].MakeAndSetFloat(right);
              itfc->retVar[4].MakeAndSetFloat(base);
              itfc->nrRetValues = 4;
              return(OK);
           }

           if ((n = ArgScan(itfc, args, 1, "left,right,top,base", "eeee", "dddd", &left, &right, &top, &base)) < 0)
              return(n);

           Margins m(left, right, top, base);

           for (Plot* p : pp->plotList())
           {
              p->setMargins(m);
           }

           if (pp->updatePlots())
           {
              MyInvalidateRect(pp->hWnd, NULL, false);
              MyUpdateWindow(pp->hWnd);
           }
           itfc->nrRetValues = 0;
           return(OK);
        }
     }
   }
	else if (obj->type == GRIDCTRL) 
	{
		BabyGrid* grid = (BabyGrid*)obj->data;
		int zerobased = grid->getZeroBased() ? 1 : 0;

		if(!strcmp(name,"set"))
		{
			// g->set(int, int, str);
			short col, row;
			Variable objVar;
			int nrArgs;

		// Get arguments (window number, parameter identifier, parameter value) ***
			if((nrArgs = ArgScan(itfc,args,3,"colNr, rowNr, val","eee","ddv",&col,&row, &objVar)) < 0)
				return(nrArgs);		  

			if (!grid->validRow(row + zerobased) || !grid->validColumn(col + zerobased))
			{
				ErrorMessage("invalid grid address (%d, %d)", col, row);
				return ERR;
			}
			if(objVar.GetType() == FLOAT32) // By number
			{
				grid->setCellData(row + zerobased, col + zerobased, objVar.GetReal());
			}
			else if (objVar.GetType() == INTEGER) // By number
			{
				grid->setCellData(row + zerobased, col + zerobased, objVar.GetLong());
			}
			else
			{
				grid->setCellData(row + zerobased, col + zerobased, objVar.GetString());
			}
			itfc->nrRetValues = 0;
			return(OK);

		}
		else if(!strcmp(name,"clear"))
		{
			// g->clear();
			grid->clear();
			itfc->nrRetValues = 0;
			return(OK);			
		}
		else if(!strcmp(name,"get"))
		{
			// g->get(var, var);
			Variable colVar, rowVar;
			int nrArgs;

			// Get arguments (window number, parameter identifier, parameter value) ***
			if((nrArgs = ArgScan(itfc,args,2,"colNr, rowNr","ee","vv",&colVar,&rowVar)) < 0)
				return(nrArgs);

			if (nrArgs != 2)
			{
				ErrorMessage("grid_get requires 2 arguments, received %d arguments", nrArgs);
				return (ERR);
			}

			// Make sure no more than one of col, var is a wild card.
			if ((colVar.GetType() != FLOAT32) && (rowVar.GetType() != FLOAT32) && 
				(colVar.GetType() != INTEGER) && (rowVar.GetType() != INTEGER))
			{
				ErrorMessage("One of col and row must be a number.");
				return (ERR);
			}

			// Handle col as wild card.
			if (colVar.GetType() == UNQUOTED_STRING)
			{
				if ('~' != colVar.GetString()[0])
				{
					ErrorMessage("Invalid string received as column argument.");
					return (ERR);
				}
				if (ERR == GridControlGetRow(grid, rowVar.GetReal(), itfc))
				{
					return ERR;
				}
				return OK;
			}

			// Handle row as wild card.
			if (rowVar.GetType() == UNQUOTED_STRING)
			{
				if ('~' != rowVar.GetString()[0])
				{
					ErrorMessage("Invalid string received as row argument.");
					return (ERR);
				}
				if (ERR == GridControlGetColumn(grid, colVar.GetReal(), itfc))
				{
					return ERR;
				}
				return OK;
			}

			// Only retrieving a single cell.

			short col = colVar.GetReal();
			short row = rowVar.GetReal();

			if (!grid->validRow(row + zerobased) || !grid->validColumn(col + zerobased))
			{
				ErrorMessage("invalid grid address (%d, %d)", col, row);
				return ERR;
			}

			char result[MAX_CELL_DATA_LEN];
			CellDataType dataType = grid->getCellData(row + zerobased, col + zerobased, result);
			switch (dataType)
			{
				case ALPHA:
				case BOOL_T:
				case BOOL_F:
					itfc->retVar[1].MakeAndSetString(result);
					itfc->nrRetValues = 1;
					break;
				case NUMERIC:
					itfc->retVar[1].MakeAndSetDouble(atof(result));
					itfc->nrRetValues = 1;
					break;
				default:
					itfc->nrRetValues = 0;
			}
			return (OK);
		}
		else if(!strcmp(name,"protect"))
		{
			// g->protect(var, var);
			Variable colVar, rowVar;
			CText protectText;
			int nrArgs;

			if((nrArgs = ArgScan(itfc,args,3,"colNr, rowNr, true/false","eee","vvt",&colVar,&rowVar,&protectText)) < 0)
				return(nrArgs);
			short col = colVar.GetReal();
			short row = rowVar.GetReal();
			bool protect = ("true" == protectText);

			// Protect the entire grid?
			if ((colVar.GetType() == UNQUOTED_STRING) && (rowVar.GetType() == UNQUOTED_STRING))
			{
				if (('~' == colVar.GetString()[0]) && ('~' == rowVar.GetString()[0]))
				{
					for (col = 0; col < grid->getCols(); col++)
					{
						if (ERR == GridControlProtectColumn(grid,col,protect,itfc))
						{
							return ERR;
						}
					}
				}
				else
				{
					ErrorMessage("invalid string received as column argument.");
					return ERR;
				}
			}

			// Protect a column?
			else if (rowVar.GetType() == UNQUOTED_STRING)
			{
				if ('~' == rowVar.GetString()[0])
				{
					if (grid->validColumn(col))
					{
						if (ERR == GridControlProtectColumn(grid,col,protect,itfc))
						{
							return ERR;
						}
					}
					else
					{
						return ERR;
					}
				}
				else
				{
					ErrorMessage("invalid string received as column argument.");
					return ERR;
				}
			}

			// Protect a row?
			else if (colVar.GetType() == UNQUOTED_STRING)
			{
				if ('~' == colVar.GetString()[0])
				{
					if (grid->validRow(row))
					{
						return GridControlProtectRow(grid,row,protect,itfc);
					}
					else
					{
						return ERR;
					}
				}
				else
				{
					ErrorMessage("invalid string received as row argument.");
					return ERR;
				}
			}

			// Protect one cell.
			else if (ERR == grid->protectCell(row + zerobased, col + zerobased, protect))
			{
				ErrorMessage("invalid grid address (%d, %d)", col, row);
				return ERR;
			}
			itfc->nrRetValues = 0;
			return(OK);
		}
		else if(!strcmp(name,"colwidth"))
		{
			// g->colwidth(int col, int width);
			short col, width;
			int nrArgs;

		// Get arguments (window number, parameter identifier, parameter value) ***
			if((nrArgs = ArgScan(itfc,args,2,"col, width","ee","dd",&col,&width)) < 0)
				return(nrArgs);

			if (ERR == grid->setColWidth(col + zerobased, width))
			{
				ErrorMessage("invalid column specified (%d)", col);
				return ERR;
			}
			itfc->nrRetValues = 0;
			return(OK);
		}
		else if(!strcmp(name,"label"))
		{
			// g->label(var collabel, var rowlabel);
			Variable colVar, rowVar;
			int nrArgs;

			if((nrArgs = ArgScan(itfc,args,2,"rowNr, label","ee","vv",&colVar,&rowVar)) < 0)
				return(nrArgs);

			// One of colVar, rowVar must be a string (the label), and the other must be a number (the row/col to set).
			if ((colVar.GetType() == UNQUOTED_STRING) && (rowVar.GetType() == FLOAT32))
			{
				// Set the label of a row.
				short row = rowVar.GetReal();
				if (!grid->validRow(row+ zerobased))
				{	
					ErrorMessage("invalid row specified (%d)", row);
					return ERR;
				}
				if (ERR == grid->setRowLabel(row + zerobased, colVar.GetString()))
				{
					ErrorMessage("invalid row specified (%d)", row);
					return ERR;
				}
			}

			else if ((rowVar.GetType() == UNQUOTED_STRING) && (colVar.GetType() == FLOAT32))
			{
				// Set the label of a column.
				short col = colVar.GetReal();
				if (!grid->validColumn(col+ zerobased))
				{
					ErrorMessage("invalid column specified (%d)", col);
					return ERR;
				}
				if (ERR == grid->setColumnLabel(col + zerobased, rowVar.GetString()))
				{
					ErrorMessage("invalid column specified (%d)", col);
					return ERR;
				}
			}

			else
			{
				// Bad format.
				ErrorMessage("One of colVar, rowVar must be a string (the label), and the other must be a number (the row/col to set)");
				return ERR;
			}

			itfc->nrRetValues = 0;
			return(OK);
		}
	}

// Generic set/get object parameters
   itfc->nrRetValues = 0;
   int r = obj->ProcessClassProc(itfc, name, args);
   return(r);
}
//
///*********************************************************************************************
//                Add an object to the top of the object linked list
//*********************************************************************************************/
//
//ObjectData* ObjectData::Add(WinData *parent, short type, short objNr, 
//                      long x, long y, long w, long h, 
//                      char *label, char *cmd, char *objInfo, DWORD visibility, long lineNr)
//{
//	POINT pt;
//
//   ObjectData *obj = new ObjectData;
//   obj->type = type;
//   if(next != (ObjectData*)0)
//      next->last = obj;
//   obj->nr = objNr;
//   obj->next = next;
//   obj->last = this;
//	next = obj;
//
//	obj->command = new char[strlen(cmd)+1];
//	strcpy(obj->command,cmd);
//   obj->xo = x;
//   obj->yo = y;
//   obj->wo = w;
//   obj->ho = h;
//   obj->selected = false;
//   obj->cmdLineNr = lineNr;
//   HWND win = parent->hWnd;
// 	obj->hwndParent = win;
//   obj->winParent = parent;
//
// 	switch(type)
// 	{
//      case(MENU):
//		{
//         obj->seqMenuNr = parent->nrMenuObjs++;
//		   obj->hWnd = NULL;	   
//		   return(obj);
//		   break; 
//	   }
// 	   case(GETMESSAGE):
//		{
//		   obj->hWnd = NULL;
//		   obj->data = objInfo;		   
//		   return(obj);
//		   break; 
//	   }
//      case(TEXTEDITOR):
//      {
//         return(obj);
//         break;
//      }
//      case(DEBUGSTRIP):
//      {
//		   obj->hWnd = CreateWindow("button", "", WS_CHILD | visibility | BS_OWNERDRAW,
//	      			                x, y, w, h, 
//	      			                obj->hwndParent, (HMENU)objNr,
//	      			                prospaInstance, NULL);
//		   OldDebugStripProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)DebugStripEventProc);
//		   break; 
//      }
//		case(UPDOWN):
//		{
//	      long dir;
//	 
//	      if(!strncmp(label,"horizontal",5))
//	        dir = UDS_HORZ;
//	      else
//	        dir = 0;
//
//		   obj->hWnd = CreateUpDownControl(WS_CHILD  | visibility |  dir | UDS_ARROWKEYS, //WS_BORDER
//	      			                x, y, w, h, 
//	      			                obj->hwndParent, (int)objNr,
//	      			                prospaInstance, NULL, 2, 0, 1);
//		   OldUpDownProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)UpDownEventProc);
//		   break; 
//	   }
//		case(TABCTRL):
//		{
//         obj->hWnd = CreateWindow(WC_TABCONTROL,			
//							"",
//							WS_CHILD |  WS_CLIPSIBLINGS | visibility | TCS_OWNERDRAWFIXED,
//							x,y,w,h,
//							(HWND)obj->hwndParent,
//							(HMENU)objNr,
//							(HINSTANCE)prospaInstance,
//							(LPSTR)NULL);
//
//		   OldTabCtrlProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)TabCtrlEventProc);
//
//         break; 
//	   }
//      case(DIVIDER):
//      {
//         obj->hWnd = CreateWindow("DIVIDER",			
//							"",
//							WS_CHILD | visibility,
//							x,y,w,h,
//							(HWND)obj->hwndParent,
//							(HMENU)objNr,
//							(HINSTANCE)prospaInstance,
//							(LPSTR)NULL);
//
//		//   SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)DividerEventsProc);
//         break;
//      }
//      case(PLOTWINDOW):
//      {
//         obj->hWnd = CreateWindow("PLOT",			
//							"1D Plot",
//							WS_CHILD | visibility,
//							x,y,w,h,
//							(HWND)obj->hwndParent,
//							(HMENU)objNr,
//							(HINSTANCE)prospaInstance,
//							(LPSTR)NULL);
//
//		   SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)PlotEventsProc);
//         break;
//      }
//      case(IMAGEWINDOW):
//      {
//         obj->hWnd = CreateWindow("PLOT",			
//							"2D Plot",
//							WS_CHILD | visibility,
//							x,y,w,h,
//							(HWND)obj->hwndParent,
//							(HMENU)objNr,
//							(HINSTANCE)prospaInstance,
//							(LPSTR)NULL);
//
//		   SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)ImageEventsProc);
//         break;
//      }
//      case(OPENGLWINDOW):
//      {
//          obj->hWnd = CreateWindow("OPENGL",			
//							"3D Plot",
//							WS_CHILD | visibility,
//							x,y,w,h,
//							(HWND)obj->hwndParent,
//							(HMENU)objNr,
//							(HINSTANCE)prospaInstance,
//							(LPSTR)NULL);
//
//         break;
//      }
//      case(TOOLBAR):
//      {
//         UINT nrBut = x;
//         HBITMAP hbitmap = (HBITMAP)label;
//         TBBUTTON *lpButtons = (TBBUTTON*)objInfo;
//         //obj->hWnd = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, 
//         //                          visibility | TBSTYLE_FLAT | WS_CHILD  | WS_BORDER | TBSTYLE_TOOLTIPS | TBSTYLE_WRAPABLE,
//         //                           0, 0, 16*nrBut, 15,
//         //                           obj->hwndParent, NULL, prospaInstance, NULL);
//         obj->hWnd = CreateToolbarEx((HWND)obj->hwndParent,			
//							visibility | TBSTYLE_FLAT | WS_CHILD  | WS_BORDER | TBSTYLE_TOOLTIPS | TBSTYLE_WRAPABLE,
//                     (DWORD)objNr,
//							(UINT)nrBut,
//							(HINSTANCE)NULL,
//							(UINT)hbitmap,
//                     lpButtons,
//							nrBut,
//							16,15,16,15,
//                     sizeof(TBBUTTON));
//
//         break;
//      }
//      case(CLIWINDOW):
//      {
//
//         obj->hWnd = CreateWindowEx (WS_EX_STATICEDGE, RICHEDIT_CLASS, NULL,
//			                        WS_CHILD | visibility | WS_HSCROLL | WS_VSCROLL | WS_DLGFRAME	|
//			                        ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
//			                        x,y,w,h,
//                                 obj->hwndParent, NULL,
//		                           prospaInstance, NULL); 
//
//         OldCLIProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)CLIEditEventsProc); 
//         break;
//      }
//      case(HTMLBOX): // Active-X HTML viewer control
//      {
//         obj->hWnd = CreateWindowEx(NULL,
//                    "AX",
//                     "{8856F961-340A-11D0-A96B-00C04FD705A2}",
//                     WS_CHILD | visibility,
//	      			   x, y, w, h, 
//                     obj->hwndParent,
//                     NULL,
//                     prospaInstance,
//                     NULL);
//		   OldHTMLProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)HTMLEventProc);
//
//         break;
//
//      }
//
//		case(GRIDCTRL):
//		{
//			int cols = ((GridCtrlInfo*)(objInfo))->cols;
//			int rows = ((GridCtrlInfo*)(objInfo))->rows;
//	
//			delete objInfo;
//			obj->data = 0;
//			obj->hWnd = CreateWindowEx(NULL,
//				"BABYGRID",
//				"",
//				visibility | WS_CHILD | ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_HSCROLL | WS_VSCROLL , x, y, w, h, 
//				obj->hwndParent,
//				(HMENU)objNr,
//				prospaInstance,
//				NULL);
//			DWORD result = GetLastError();
//			BabyGrid* g = new BabyGrid(obj->hWnd, prospaInstance);
//		   obj->data = (char*)g;	
//			OldGridProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)GridEventProc);
//			// Set initial grid dimensions
//			g->setColCount(cols);
//			g->setRowCount(rows);
//			//////
//			// TODO: Make these the defaults in the constructor.
//			//////
//			// Hide the header row by default.
//			g->setHeaderRowHeight(0);
//			// Hide the title row.
//			g->setTitleHeight(0);
//			// Ensure the grid is editable.
//			g->setEditable(true);
//			// Don't allow the grid to shrink itself.
//			g->setAutoRow(false);
//			// Don't let the last column grow to fill the space available.
//			g->extendLastColumn(false);
//			// Use any titles set, rather than numbers for rows and alpha for cols.
//			g->setRowsNumbered(false);
//			g->setColsNumbered(false);
//			g->setZeroBased(true);
//			ShowScrollBar(obj->hWnd, SB_BOTH, false);
//			break;
//		}
//
//		case(BUTTON):
//		{
//		   obj->data = objInfo;
//
//
//		   obj->hWnd = CreateWindow("button", label, WS_CHILD  | BS_PUSHBUTTON | visibility | BS_MULTILINE,
//	      			                x, y, w, h, 
//	      			                obj->hwndParent, (HMENU)objNr,
//	      			                prospaInstance, NULL);
//
//		   OldButtonProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)ButtonEventProc);
//		   break; 
//	   }
//		case(COLORBOX):
//		{
//		   obj->data = objInfo;
//		   obj->hWnd = CreateWindow("button", "", WS_CHILD | visibility | BS_OWNERDRAW,
//	      			                x, y, w, h, 
//	      			                obj->hwndParent, (HMENU)objNr,
//	      			                prospaInstance, NULL);
//
//		   OldButtonProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)ButtonEventProc);
//		   break; 
//	   }
//
//		case(PICTURE):
//		{
//		   obj->hWnd = CreateWindow("button", "", WS_CHILD | visibility | BS_OWNERDRAW,
//	      			                x, y, w, h, 
//	      			                obj->hwndParent, (HMENU)objNr,
//	      			                prospaInstance, NULL);
//
//		   OldPictureProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)PictureEventProc);
//		   break; 
//	   }
//
//		case(LISTBOX):
//		{
//         MakeListBoxObject(obj, x, y, w, h, visibility);
//		   break; 
//	   }
//      case(GROUP_BOX):
//	   {
//		   obj->hWnd = CreateWindow("static", label, WS_CHILD  | SS_OWNERDRAW	 | visibility,
//	      			                x, y, w, h, 
//	      			                obj->hwndParent, (HMENU)objNr,
//	      			                prospaInstance, NULL); 
//		   OldGroupBoxProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)GroupBoxEventProc); 
//		   break;
//	   }	   
//      case(TEXTMENU):
//	   {
//		   obj->hWnd = CreateWindowEx(NULL,"combobox", "", WS_CHILD  | CBS_HASSTRINGS | CBS_DROPDOWN | CBS_AUTOHSCROLL	| WS_VSCROLL | visibility, //| CBS_SORT,
//	      			                x, y, w, h, 
//	      			                obj->hwndParent, (HMENU)objNr,
//	      			                prospaInstance, NULL);
//         obj->ho = 21; // Sets the size for designer manipulation
//         pt.x = 3; 
//         pt.y = 3; 
//         HWND hwndEdit = RealChildWindowFromPoint(obj->hWnd, pt);  
//		   OldTextMenuEditProc = (WNDPROC)SetWindowLong(hwndEdit,GWL_WNDPROC,(LONG)TextMenuEditEventProc); 
//		   OldTextMenuProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)TextMenuEventProc); 
//		   break;
//	   }   
//      case(TEXTBOX):
//      {
//			obj->hWnd = CreateWindowEx (WS_EX_CLIENTEDGE,"edit", NULL,
//							WS_CHILD  | ES_LEFT | ES_AUTOHSCROLL | visibility, //WS_BORDER
//							x-1, y, w+2, h, // This style doesn't line up with others!!
//	                  obj->hwndParent, (HMENU) objNr,
//							prospaInstance, NULL);
//		   OldTextBoxProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)TextBoxEventProc); 
//		   break;
//	   }
//	   case(STATICTEXT):
//      {
//	      long xp;
//	      long mode = SS_LEFT;
//	      HDC hdc = GetDC(win);
//	      SIZE size;
//	      
//	      SelectObject(hdc,controlFont);
//	      GetTextExtentPoint32(hdc,label,strlen(label),&size);
//	      
//	      long w = size.cx;
//	      long h = size.cy;
//	
//	      if(!strcmp(cmd,"left"))
//	         xp = x, mode = SS_LEFT;
//	      else if(!strcmp(cmd,"right"))
//	         xp = x - w, mode = SS_RIGHT;
//	      else if(!strcmp(cmd,"centre") || !strcmp(cmd,"center"))
//	         xp = x - w/2, mode = SS_CENTER;
//	         
//			obj->hWnd = CreateWindow ("static", label,
//							WS_CHILD  | mode | visibility	,
//							xp, y, w, h,
//	                  obj->hwndParent, (HMENU) objNr,
//							prospaInstance, NULL); 
//		   OldStaticTextProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)STextEventProc);
//							
//		   ReleaseDC(win,hdc);
//		   obj->xo = xp;
//		   obj->yo = y; 
//		   obj->wo = w;
//		   obj->ho = h;
//		   break;
//	   }
//	   case(STATICTEXT2):
//      {
//	      long xp;
//	      long mode = SS_LEFT;
//	      HDC hdc = GetDC(win);
//	      SIZE size;
//	      
//	      SelectObject(hdc,controlFont);
//	      GetTextExtentPoint32(hdc,label,strlen(label),&size);
//	      
//	      long w = size.cx;
//	      long h = size.cy;
//	
//	      if(!strcmp(cmd,"left"))
//	         xp = x, mode = SS_LEFT;
//	      else if(!strcmp(cmd,"right"))
//	         xp = x - w, mode = SS_RIGHT;
//	       else if(!strcmp(cmd,"center"))
//	         xp = x - w/2, mode = SS_CENTER;
//	         
//			obj->hWnd = CreateWindow ("static", "",
//							WS_CHILD  | mode | visibility,
//							xp, y, w, h,
//	                  obj->hwndParent, (HMENU) objNr,
//							prospaInstance, NULL); 
//		   OldStatic2TextProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)SText2EventProc);
//							
//		   ReleaseDC(win,hdc);
//		   obj->xo = xp;
//		   obj->yo = y; 
//		   obj->wo = w;
//		   obj->ho = h;
//		   break;
//	   }	   
//	   case(PROGRESSBAR):
//	   {
//	      long dir;
//	      
//	      if(!strcmp(label,"vertical"))
//	         dir = PBS_VERTICAL;
//	      else
//	         dir = 0;
//	      
//	      obj->hWnd =   CreateWindow(PROGRESS_CLASS, label,
//	                    WS_CHILD  | /*WS_BORDER |*/ PBS_SMOOTH | dir | visibility,
//	                    x, y, w, h,
//	                    obj->hwndParent, (HMENU)objNr,  
//						     prospaInstance, NULL);
//
//         OldProgressBarProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)ProgressBarEventProc);
//			break;			     					     
//	   }
//	   case(STATUSBOX):
//	   {
//	      RECT rc,rp;
//			obj->hWnd = CreateWindow(STATUSCLASSNAME,"",
//			                         WS_CHILD | visibility,
//			                         0,0,0,0,
//			                         obj->hwndParent,(HMENU)objNr,prospaInstance,NULL);
//
//		   OldStatusBarProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)StatusBarEventProc);
//		   GetWindowRect(obj->hWnd,&rc);
//         ScreenToClientRect(obj->hwndParent,&rc);
//		   obj->xo = rc.left;
//		   obj->yo = rc.top; 
//		   obj->wo = rc.right-rc.left;
//		   obj->ho = rc.bottom-rc.top;
//		   break;
//	   } 
//	   case(SLIDER):
//	   { 
//	      long dir;
//	 
//	      if(!strncmp(label,"horizontal",5))
//	        dir = TBS_HORZ;
//	      else
//	        dir = TBS_VERT;
//	     
//	      obj->hWnd = CreateWindow(TRACKBAR_CLASS, "",
//	                 WS_CHILD  |  TBS_AUTOTICKS | TBS_NOTICKS  /*TBS_ENABLESELRANGE*/ | WS_TABSTOP | dir | visibility, 
//	                 x, y, w, h,  
//	                 obj->hwndParent, (HMENU)objNr,  
//						  prospaInstance, NULL);
//	
//		   OldSliderProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)SliderEventProc);
//		   break;
//	   }
//	   case(CHECKBOX):
//		{
//		   CheckButtonInfo *info = (CheckButtonInfo*)objInfo;
//		   obj->data = objInfo;
//		   obj->hWnd = CreateWindow("button", label, WS_CHILD  | BS_AUTOCHECKBOX | visibility,
//	      			                x, y, 14,13, 
//	      			                obj->hwndParent, (HMENU)objNr,
//	      			                prospaInstance, NULL);
//	      SendMessage(obj->hWnd,BM_SETCHECK,info->init-1,0);
//	      obj->wo = 14;
//	      obj->ho = 13;
//		   OldButtonProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)ButtonEventProc); 
//	      break;
//	   }
//	   case(RADIO_BUTTON):
//		{
//		   RadioButtonInfo *info = (RadioButtonInfo*)objInfo;
//		   int xp = x;
//		   int yp = y;
//		   
//		   obj->data = objInfo;
//	      
//		   for(int i = 0; i < info->nrBut; i++)
//		   {
//		      info->hWnd[i] = CreateWindow("button", label, WS_CHILD | BS_RADIOBUTTON | visibility,
//	      			                   xp, yp, 14, 14, 
//	      			                   obj->hwndParent, (HMENU)objNr,
//	      			                   prospaInstance, NULL);
//		      OldButtonProc = (WNDPROC)SetWindowLong(info->hWnd[i],GWL_WNDPROC,(LONG)ButtonEventProc); 
//	      			                   
//	      	if(i == info->init-1)
//	      		SendMessage(info->hWnd[i],BM_SETCHECK,1,0);
//			                   
//	          if(info->orient == 'h')
//		         xp += info->spacing;
//		      else
//		         yp += info->spacing;
//	      }
//	      
//	      obj->xo = x;
//	      obj->yo = y;
//	      if(info->orient == 'h')
//	      {
//	         obj->wo = (info->nrBut-1)*info->spacing+14;
//	         obj->ho = 14;
//	      }
//	      else
//	      {
//	         obj->wo = 14;
//	         obj->ho = (info->nrBut-1)*info->spacing+14;
//	      }
//	      break;
//	   }
//	}
//      
//   SendMessage(obj->hWnd,WM_SETFONT,(WPARAM)controlFont ,false);
//      
//// Add a reference to the obj->hWnd so we don't have to search for the ObjectData structure
//
//   if(type != HTMLBOX && type != RADIO_BUTTON)
//   {
//      WinInfo* wi = new WinInfo;
//      wi->winType = OBJDATA;
//      wi->data = (char*)obj;
//
//      SetWindowLong(obj->hWnd,GWL_USERDATA,(LONG)wi);
//   }
//
//   return(obj);
//}
//
//
//// Add and existing object to the window object list
//
//void ObjectData::AddObj(ObjectData* obj)
//{
//   if(next != (ObjectData*)0)
//      next->last = obj;
//   obj->next = next;
//   obj->last = this;
//	next = obj;
//}


/********************************************************************************
    Duplicate the current object returning the copy.
    Last modified 26 Nov 12 to include MENU and copy TEXTMENU menu items
********************************************************************************/

ObjectData* ObjectData::Copy(HWND newParent)
{
   short x,y,w,h;
   CText label;
   
// Make a new object
   ObjectData *obj = new ObjectData;

// Copy object specific stuff
   obj->type = type;	
   obj->nr(this->nr());	
	obj->hwndParent = newParent;
	obj->command = new char[strlen(command)+1];
	strcpy(obj->command,command);
   strcpy(obj->valueName,valueName);
   strcpy(obj->objName,objName);
   x = obj->xo = xo;
   y = obj->yo = yo;
   w = obj->wo = wo;
   h = obj->ho = ho;
   obj->xSzOffset = xSzOffset;
   obj->xSzScale = xSzScale;
   obj->ySzOffset = ySzOffset;
   obj->ySzScale = ySzScale;
   obj->wSzOffset = wSzOffset;
   obj->wSzScale = wSzScale;
   obj->hSzOffset = hSzOffset;
   obj->hSzScale = hSzScale;
   obj->selected_ = selected_;
	obj->inError = inError;
   obj->winParent = winParent;
   obj->tabNr = tabNr;
   obj->tabPageNr = tabPageNr;
   obj->tabParent = tabParent; // This changes if tab changes
   obj->panelParent = panelParent;
   strcpy(obj->tag,tag);
   obj->dataType = dataType;
   obj->rangeCheck = rangeCheck;
   obj->statusbox = statusbox;
   obj->upper = upper;
   obj->lower = lower;
   obj->fgColor = fgColor;
   obj->bgColor = bgColor;
   obj->toolTipHwnd = NULL;
   obj->menuListSize = menuListSize;
	obj->highLiteChanges = highLiteChanges;
	obj->valueChanged = false;

   if(menuListSize > 0)
   {
      obj->menuList = new short[menuListSize];
      for(int i = 0 ; i < menuListSize; i++)
         obj->menuList[i] = menuList[i];
   }

   GetWindowText(label);

// Make windows part of object
 	switch(obj->type)
 	{
		case(GRIDCTRL):
		{
			obj->data = (char*)(new BabyGrid(*((BabyGrid*)(this->data))));
			obj->hWnd = CreateWindowEx(NULL,
				"BABYGRID",
				"",
				WS_CHILD | WS_HSCROLL | WS_VSCROLL, x, y, w, h, 
				newParent,
				(HMENU)nr(),
				prospaInstance,
				NULL);			
			((BabyGrid*)(obj->data))->hWnd = obj->hWnd;


		   OldGridProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)GridEventProc);
			break;
		}

		case(BUTTON):
		{
         obj->data = (char*)new PushButtonInfo;
         PushButtonInfo* dstInfo = (PushButtonInfo*)obj->data;
         PushButtonInfo* srcInfo = (PushButtonInfo*)this->data;
         dstInfo->hImage = 0; // This needs to be fixed
			dstInfo->fontName = srcInfo->fontName;
			dstInfo->fontHeight = srcInfo->fontHeight;
			dstInfo->italicText = srcInfo->italicText;
		   obj->hWnd = CreateWindow("button", label.Str(), WS_CHILD  | BS_PUSHBUTTON,
	      			                x, y, w, h, 
	      			                newParent, (HMENU)nr(),
	      			                prospaInstance, NULL);
		   OldButtonProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)ButtonEventProc);

         long type = GetWindowLong(hWnd,GWL_STYLE);
         if(type & BS_DEFPUSHBUTTON)
            SetWindowLong(obj->hWnd,GWL_STYLE,type | BS_DEFPUSHBUTTON);

		   break; 
	   }
      case(MENU):
		{
         short size = sizeof(MenuInfo);
         obj->data = new char[size];
         MenuInfo* dstInfo = (MenuInfo*)obj->data;
         MenuInfo* srcInfo = (MenuInfo*)data;

         dstInfo->menu = CreatePopupMenu();
         dstInfo->nrItems = srcInfo->nrItems;
         dstInfo->name = new char[strlen(srcInfo->name)+1];
         strcpy(dstInfo->name,srcInfo->name);
         dstInfo->cmd = new char*[srcInfo->nrItems];
         dstInfo->label = new char*[srcInfo->nrItems];
         dstInfo->key = new char*[srcInfo->nrItems];
         dstInfo->accel = new ACCEL[srcInfo->nrItems];

         this->seqMenuNr = winParent->nrMenuObjs++;
		   this->hWnd = NULL;	

       // Copy each menu item
         for(int i = 0; i < dstInfo->nrItems; i++)
         {
            dstInfo->label[i] = 0;
            if(srcInfo->label[i])
            {
               dstInfo->label[i] = new char[strlen(srcInfo->label[i])+1];
               strcpy(dstInfo->label[i],srcInfo->label[i]);
            }

            dstInfo->cmd[i] = 0;
            if(srcInfo->cmd[i])
            {
               dstInfo->cmd[i] = new char[strlen(srcInfo->cmd[i])+1];
               strcpy(dstInfo->cmd[i],srcInfo->cmd[i]);
            }

            if(srcInfo->key[i])
            {
               dstInfo->key[i] = new char[strlen(srcInfo->key[i])+1];
               strcpy(dstInfo->key[i],srcInfo->key[i]);
            }
            else
               dstInfo->key[i] = 0;

            memcpy(&(dstInfo->accel[i]),&(srcInfo->accel[i]),sizeof(ACCEL));

            dstInfo->accel[i].cmd = i+obj->seqMenuNr*100 + MENU_BASE;

            if(!strcmp(dstInfo->label[i],"Separator"))
            {
               AppendMenu(dstInfo->menu, MF_SEPARATOR	, NULL, NULL);  
            }
            else if(!strcmp(dstInfo->label[i],"Pull_right"))
            {
               short menuNr;
               sscanf(dstInfo->cmd[i],"%hd",&menuNr);
               ObjectData *popup = winParent->FindObjectByNr(menuNr);
               if(popup)
               {
                  MenuInfo *popupInfo = (MenuInfo*)popup->data;
                  AppendMenu(dstInfo->menu, MF_POPUP | MF_STRING, (UINT)popupInfo->menu, popupInfo->name);
               }
               else
               {
                  ErrorMessage("can't find pull right menu %hd",menuNr);
               }
            }
            else
            {
               AppendMenu(dstInfo->menu, MF_STRING, dstInfo->accel[i].cmd, dstInfo->label[i]); 
            }
         }
   
		   break; 
	   }

      case(CLIWINDOW):
      {
         obj->hWnd = CreateWindowEx (WS_EX_STATICEDGE, RICHEDIT_CLASS, NULL,
			                        WS_CHILD | WS_HSCROLL | WS_VSCROLL | //WS_DLGFRAME	|
			                        ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
			                        x,y,w,h,
                                 newParent, (HMENU)nr(),
		                           prospaInstance, NULL); 

         OldCLIProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)CLIEditEventsProc); 
         SendMessage(obj->hWnd,EM_EXLIMITTEXT,(WPARAM)0,(LPARAM)MAX_TEXT);	
       	HDC hdc = GetDC(obj->hWnd);
         SendMessage(obj->hWnd,WM_SETFONT,(WPARAM)cliFont,MAKELPARAM(false, 0));
         ReleaseDC(obj->hWnd,hdc); 
         cliEditWin = obj->hWnd;
         break;
      }
      case(TEXTEDITOR):
      {
			DWORD oldVis = g_objVisibility;
         g_objVisibility = 0; // make sure new object is invisible
         EditParent *ep = new EditParent;
         ep->parent = obj;
         MakeEditWindows(false, ep, 1, 1, -1);
         ep->rows = 1;
         ep->cols = 1;
         ep->editData[0]->debug = false;
         obj->hWnd = ep->editData[0]->edWin;
         SendMessage(obj->hWnd,EM_SETMARGINS ,EC_LEFTMARGIN	,(LPARAM) MAKELONG(5, 0));
         obj->data = (char*)ep;
         MoveWindow(ep->editData[0]->edWin,x,y,w,h,true); 
         EnableWindow(ep->editData[0]->edWin,false);
         g_objVisibility = oldVis;
         break;
      } 

      case(PLOTWINDOW):
      {
         obj->hWnd = CreateWindow("PLOT",			
							"1D Plot",
							WS_CHILD,
							x,y,w,h,
							newParent,
							(HMENU)nr(),
							(HINSTANCE)prospaInstance,
							(LPSTR)NULL);

		   SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)PlotEventsProc);

      // Make parent object
         PlotWindow1D *pp = new PlotWindow1D(SHOW_DATA, obj->hWnd,pwd, obj, w, h);

		//	pp->makeCurrentPlot();
		//	pp->makeCurrentDimensionalPlot();

      // Attach to object data
         obj->data = (char*)pp;

         break;
      }

      case(IMAGEWINDOW):
      {
         obj->hWnd = CreateWindow("PLOT",			
							"2D Plot",
							WS_CHILD,
							x,y,w,h,
							newParent,
							(HMENU)nr(),
							(HINSTANCE)prospaInstance,
							(LPSTR)NULL);

		   SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)ImageEventsProc);

      // Initialise plotParent information
         PlotWindow2D *pp = new PlotWindow2D;
     //    pp->plotData = new Plot*[1*1];
         pp->showStatusBar = false;
         pp->showToolBar = false;
         pp->useBackingStore = true;
         pp->mouseMode = SHOW_DATA;
         pp->hWnd = obj->hWnd;
         pp->InitialisePlot(1,1,pwd);
         HDC hdc = GetDC(pp->hWnd);
         long newWidth;
         GenerateBitMap(w,h, &pp->bitmap, hdc, newWidth);
         ReleaseDC(pp->hWnd,hdc);
         DragAcceptFiles(pp->hWnd,true);
			pp->makeCurrentPlot();
			pp->makeCurrentDimensionalPlot();

      // Attach to object data
          obj->data = (char*)pp;

         break;
      }

		case(COLORBOX):
		{
         obj->data = new char[3];
         memcpy(obj->data,data,3);
		   obj->hWnd = CreateWindow("button", "", WS_CHILD | BS_OWNERDRAW,
	      			                x, y, w, h, 
	      			                newParent, (HMENU)nr(),
	      			                prospaInstance, NULL);
		   OldButtonProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)ButtonEventProc);
		   break; 
	   }

      case(PICTURE):
		{
		   obj->hWnd = CreateWindow("button", "", WS_CHILD  | BS_OWNERDRAW,
	      			                x, y, w, h, 
	      			                obj->hwndParent, (HMENU)nr(),
	      			                prospaInstance, NULL);
		   OldPictureProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)PictureEventProc);

		   break; 
	   }

      case(TABCTRL):
		{

         obj->hWnd = CreateWindow(WC_TABCONTROL,			
							"",
							WS_CHILD |  WS_CLIPSIBLINGS | TCS_OWNERDRAWFIXED,
							x,y,w,h,
							(HWND)newParent,
							(HMENU)nr(),
							(HINSTANCE)prospaInstance,
							(LPSTR)NULL);

         OldTabCtrlProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)TabCtrlEventProc);

         char name[MAX_STR];
         TCITEM tci;
         tci.mask = TCIF_TEXT;
         tci.iImage = -1;
         tci.pszText = name;
         tci.cchTextMax = MAX_STR;

         TabInfo *oldInfo = (TabInfo*)data;   
         TabInfo *newInfo = new TabInfo;
         newInfo->nrTabs = oldInfo->nrTabs;
         newInfo->tabLabels = new CText[oldInfo->nrTabs];


         int selected = TabCtrl_GetCurSel(this->hWnd);

         TabCtrl_DeleteAllItems(obj->hWnd);

         for(short i = 0; i < oldInfo->nrTabs; i++) // Add new one from list
         {
             strcpy(name,oldInfo->tabLabels[i].Str());
             newInfo->tabLabels[i] = name;
             TabCtrl_InsertItem(obj->hWnd, i, &tci);
         }  

         TabCtrl_SetCurSel(obj->hWnd,selected);

         obj->data = (char*)newInfo;
         
         break; 
	   }

      case(DIVIDER):
      {
         short size = sizeof(DividerInfo);
         obj->data = new char[size];
         memcpy(obj->data,data,size);

         obj->hWnd = CreateWindow("DIVIDER",			
							"",
							WS_CHILD,
							x,y,w,h,
							(HWND)newParent,
							(HMENU)nr(),
							(HINSTANCE)prospaInstance,
							(LPSTR)NULL);

         break;
      }
		case(UPDOWN):
		{
	      long dir;
	 
         short size = sizeof(UpDownInfo);
         obj->data = new char[size];
         memcpy(obj->data,data,size);

	      if(!strncmp(label.Str(),"horizontal",5))
	        dir = UDS_HORZ;
	      else
	        dir = 0;

		   obj->hWnd = CreateUpDownControl(WS_CHILD | UDS_ARROWKEYS | dir,
	      			                x, y, w, h, 
	      			                obj->hwndParent, (int)nr(),
	      			                prospaInstance, NULL, 99, 0, 50);
		   OldUpDownProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)UpDownEventProc);
		   break; 
	   }
		case(LISTBOX):
		{
			short size = sizeof(ListBoxInfo);
         obj->data = (char*)new ListBoxInfo;
         memcpy(obj->data,data,size);
         MakeListBoxObject(obj, x, y, w, h, true);
		   break; 
	   }
      case(GROUP_BOX):
	   {
		   obj->hWnd = CreateWindow("static", label.Str(), WS_CHILD  | SS_OWNERDRAW,
	      			                x, y, w, h, 
	      			                obj->hwndParent, (HMENU)nr(),
	      			                prospaInstance, NULL); 
		   OldGroupBoxProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)GroupBoxEventProc); 
		   break;
	   }	
      case(PANEL):
	   {
         short size = sizeof(PanelInfo);
         obj->data = new char[size];
         memcpy(obj->data,data,size);
         PanelInfo* info = (PanelInfo*)obj->data;

		   this->hWnd = CreateWindow("SCROLLBAR", "", WS_CHILD | SBS_VERT,
	      			                x+w-20, y, 20, h, 
	      			                this->hwndParent, (HMENU)nr(),
	      			                prospaInstance, NULL); 

         info->hWndPanel = CreateWindowEx(WS_EX_CLIENTEDGE,"static", label.Str(), WS_CHILD,
	      			                x, y, w-20, h, 
	      			                this->hwndParent, (HMENU)nr(),
	      			                prospaInstance, NULL); 


		   OldPanelScrollProc = (WNDPROC)SetWindowLong(this->hWnd ,GWL_WNDPROC,(LONG)PanelScrollEventProc); 
		   OldPanelProc = (WNDPROC)SetWindowLong(info->hWndPanel ,GWL_WNDPROC,(LONG)PanelEventProc); 

         SCROLLINFO si;
         si.cbSize = sizeof(SCROLLINFO);
         si.fMask = SIF_RANGE | SIF_POS | SIF_PAGE | SIF_DISABLENOSCROLL;
         si.nMin = 0;
         si.nMax = 100;
         si.nPos = 0;
         si.nPage = 10;
         SetScrollInfo(this->hWnd , SB_CTL, &si, true);

		   break;
	   }
      case(TEXTMENU):
	   {
         POINT pt;
		   obj->hWnd = CreateWindowEx(NULL,"combobox", "", WS_CHILD  | CBS_HASSTRINGS | CBS_DROPDOWN | CBS_AUTOHSCROLL	| WS_VSCROLL,
	      			                x, y, w, h, 
	      			                newParent, (HMENU)nr(),
	      			                prospaInstance, NULL);
         this->ho = 21; // Sets the size for designer manipulation
         pt.x = 3; 
         pt.y = 3; 
         HWND hwndEdit = RealChildWindowFromPoint(obj->hWnd, pt);  
		   OldTextMenuEditProc = (WNDPROC)SetWindowLong(hwndEdit,GWL_WNDPROC,(LONG)TextMenuEditEventProc); 
		   OldTextMenuProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)TextMenuEventProc); 

        // Copy the menu data
			char *data = NULL;
         long n = SendMessage(this->hWnd, CB_GETCOUNT, 0, 0);
         if(n > 0)
         {
			   for(long index = 0; index < n; index++)
			   {
	            int len = SendMessage(this->hWnd,CB_GETLBTEXTLEN,(WPARAM)index,(LPARAM)0);
               if(data) 
                  delete [] data;
               data = new char[len+1];
	            SendMessage(this->hWnd,CB_GETLBTEXT,(WPARAM)index,(LPARAM)data);
	            SendMessage(obj->hWnd, CB_ADDSTRING, 0, (LPARAM) data);
			   }
            if(data) 
               delete [] data;
         }

		   break;
	   }   
      case(TEXTBOX):
      {
			obj->hWnd = CreateWindowEx (WS_EX_CLIENTEDGE,"edit", NULL,
							WS_CHILD   | /*WS_BORDER |*/ ES_LEFT | ES_AUTOHSCROLL,
							x, y, w, h,
	                  newParent, (HMENU) nr(),
							prospaInstance, NULL);
		   OldTextBoxProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)TextBoxEventProc); 
		   break;
	   }
	   case(STATICTEXT):
      {
         obj->data = (char*)new StaticTextInfo;
         StaticTextInfo* dstInfo = (StaticTextInfo*)obj->data;
         StaticTextInfo* srcInfo = (StaticTextInfo*)data;
         dstInfo->fontName = srcInfo->fontName;
         dstInfo->fontHeight = srcInfo->fontHeight;
         dstInfo->boldText = srcInfo->boldText;
         dstInfo->multiLine = srcInfo->multiLine;
         dstInfo->italicText = srcInfo->italicText;

	      HDC hdc = GetDC(obj->hwndParent);
	      SIZE size;

	      SelectObject(hdc,controlFont);	      
	      GetTextExtentPoint32(hdc,label.Str(),strlen(label.Str()),&size);
	      
	      long w = size.cx;
	      long h = size.cy;

         long mode = GetWindowLong(hWnd,GWL_STYLE);
         mode = (mode & 0x000F);	
         
			obj->hWnd = CreateWindow ("static", label.Str(),
							WS_CHILD  | mode,
							x, y, w, h,
	                  newParent, (HMENU) nr(),
							prospaInstance, NULL); 
		   OldStaticTextProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)STextEventProc);
							
		   ReleaseDC(obj->hwndParent,hdc);
		   break;
	   }
	   case(PROGRESSBAR):
	   {
	      long dir;
	      
	      if(!strcmp(label.Str(),"vertical"))
	         dir = PBS_VERTICAL;
	      else
	         dir = 0;
	      
	      obj->hWnd =   CreateWindow(PROGRESS_CLASS, label.Str(),
	                    WS_CHILD  | /* WS_BORDER | */ PBS_SMOOTH | dir,
	                    x, y, w, h,
	                    newParent, (HMENU)nr(),  
						     prospaInstance, NULL);
		   OldProgressBarProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)ProgressBarEventProc);
			break;			     					     
	   }
	   case(STATUSBOX):
	   {
	      RECT rc,rp;
			obj->hWnd = CreateWindow(STATUSCLASSNAME,"",
			                         WS_CHILD,
			                         0,0,0,0,
			                         newParent,(HMENU)nr(),prospaInstance,NULL);
		   GetWindowRect(newParent,&rp);
		   GetWindowRect(obj->hWnd,&rc);
		   obj->xo = 0;
		   obj->yo = rc.top - rp.top;
		   obj->wo = rc.right-rc.left;
		   obj->ho = rc.bottom-rc.top;
		   break;
	   } 
	   case(SLIDER):
	   { 
	      long dir;
         long style = GetWindowLong(hWnd,GWL_STYLE);
	 
	      if(style & TBS_VERT)
	        dir = TBS_VERT;
	      else
	        dir = TBS_HORZ;
	     
	      obj->hWnd = CreateWindow(TRACKBAR_CLASS, "",
	                 WS_CHILD  |  TBS_AUTOTICKS  | WS_TABSTOP | dir, 
	                 x, y, w, h,  
	                 newParent, (HMENU)nr(),  
						  prospaInstance, NULL);
	
		   OldSliderProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)SliderEventProc);
		   break;
	   } 
	   case(CHECKBOX):
		{
         obj->data = (char*)new CheckButtonInfo;
		   memcpy(obj->data,data,sizeof(CheckButtonInfo));
         CheckButtonInfo *infoDst = (CheckButtonInfo*)obj->data;
         CheckButtonInfo *infoSrc = (CheckButtonInfo*)data;

		   obj->hWnd = CreateWindow("button", label.Str(), WS_CHILD  | BS_AUTOCHECKBOX,
	      			                x, y, 14,13, 
	      			                newParent, (HMENU)nr(),
	      			                prospaInstance, NULL);
	      SendMessage(obj->hWnd,BM_SETCHECK,infoDst->init-1,0);
		   OldButtonProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)ButtonEventProc); 

      // Make sure state information is copied
         infoDst->states = new char[strlen(infoSrc->states)+1];
         strcpy(infoDst->states,infoSrc->states);
 
         break;
	   }
	   case(RADIO_BUTTON):
		{
         obj->data = (char*)new RadioButtonInfo;
		   memcpy(obj->data,data,sizeof(RadioButtonInfo));
		   RadioButtonInfo *infoDst = (RadioButtonInfo*)obj->data;
		   RadioButtonInfo *infoSrc = (RadioButtonInfo*)data;
         infoDst->hWnd = new HWND[infoDst->nrBut];	

		   int xp = x;
		   int yp = y;
		   	      
		   for(int i = 0; i < infoDst->nrBut; i++)
		   {
		      infoDst->hWnd[i] = CreateWindow("button", label.Str(), WS_CHILD | BS_RADIOBUTTON,
	      			                   xp, yp, 14, 14, 
	      			                   newParent, (HMENU)nr(),
	      			                   prospaInstance, NULL);
		      OldButtonProc = (WNDPROC)SetWindowLong(infoDst->hWnd[i],GWL_WNDPROC,(LONG)ButtonEventProc); 
	      			                   
	      	if(i == infoDst->init-1)
	      		SendMessage(infoDst->hWnd[i],BM_SETCHECK,1,0);
			                   
	          if(infoDst->orient == 'h')
		         xp += infoDst->spacing;
		      else
		         yp += infoDst->spacing;
	      }

       // Make sure state information is copied
         infoDst->states = new char[strlen(infoSrc->states)+1];
         strcpy(infoDst->states,infoSrc->states);
 
	      break;
	   }	   
	} 
 
// Encode the WinData information into the Windows HWND object.
   if(obj->hWnd != NULL && type != HTMLBOX) // && type != RADIO_BUTTON && type != MENU)
   {
      WinInfo* wi = new WinInfo;
      wi->winType = OBJDATA;
      wi->data = (char*)obj;

      SetWindowLong(obj->hWnd,GWL_USERDATA,(LONG)wi);
   }

   if((type != CLIWINDOW) && (type != GRIDCTRL))
      SendMessage(obj->hWnd,WM_SETFONT,(WPARAM)controlFont ,false);

// Return copied object to calling program
      
   return(obj);
}

//
///************************************************************************
//  Search for the next object to which I can tab and return that object
//  Returns NULL object if tabbing not defined yet
//*************************************************************************/
//
//ObjectData* ObjectData::FindNextObjByTabNumber()
//{
//   short cnt,min = 32767,max = -1;
//   ObjectData *obj,*objFirst,*firstControl = 0;
//
//// Note current object tab number
//   if(this->type == TABCTRL)
//      cnt = 0;
//   else
//      cnt = this->tabNr;
//   
//// Find start of object list   
//   for(obj = this->last; obj->last != NULL; obj = obj->last){;}
//      objFirst = obj->next;
//
//   if(this->tabPageNr == -1 && this->type != TABCTRL)
//   {
//   // Find first control tab number (the one with the smallest number) 
//  	   for(obj = objFirst; obj != NULL; obj = obj->next)
//  	   {
//	      if(obj->tabNr < min && obj->tabNr > 0 && IsTabbable(obj) && obj->tabPageNr == -1)
//         {
//            min = obj->tabNr;  
//            firstControl = obj;
//         }
//      }
//
//   // Tabbing not set up yet so exit
//      if(!firstControl)
//         return(0);
//
//   // Find highest tab number (the one with the largest number)
//  	   for(obj = objFirst; obj != NULL; obj = obj->next)
//  	   {
//	      if(obj->tabNr > max && IsTabbable(obj) && obj->tabPageNr == -1)
//	         max = obj->tabNr;
//      }
//
//   // Find object with next highest tab number which is tabbable
//   // Do this by searching through the list multiple times 
//      while(true)
//      {
//	      for(obj = objFirst; obj != NULL; obj = obj->next) // Search through list for next control (by number)
//	      {
//	         if(obj->tabNr == (cnt+1) && IsTabbable(obj) && obj->tabPageNr == -1)
//   	         return(obj);
//	      }
//         cnt++;
//         if(cnt > max) // Run out of control so back to first one
//            return(firstControl);
//      }
//   }
//   else // Object is part of a tabcontrol so only tab inside the tabcontrol
//   {
//      short tabNr,tabParent;
//      if(this->type == TABCTRL)
//      {
//         tabNr = TabCtrl_GetCurSel(this->hWnd);
//         tabParent = this->nr;
//      }
//      else
//      {
//         tabNr = this->tabPageNr;
//         tabParent = this->tabParent;
//      }
//
//   // Find first control tab number in this tabControl page (the one with the smallest number) 
//  	   for(obj = objFirst; obj != NULL; obj = obj->next)
//  	   {
//	      if(obj->tabNr < min && obj->tabNr > 0 && IsTabbable(obj) && obj->tabPageNr == tabNr && obj->tabParent == tabParent)
//         {
//            min = obj->tabNr;  
//            firstControl = obj;
//         }
//      }
//
//   // Tabbing not set up yet so exit
//      if(!firstControl)
//         return(0);
//
//      if(this->type == TABCTRL)
//         return(firstControl);
//
//   // Find highest tab number (the one with the largest number)
//  	   for(obj = objFirst; obj != NULL; obj = obj->next)
//  	   {
//	      if(obj->tabNr > max && IsTabbable(obj) && obj->tabPageNr == tabNr && obj->tabParent == tabParent)
//	         max = obj->tabNr;
//      }
//
//   // Find object with next highest tab number which is tabbable
//   // Do this by searching through the list multiple times 
//      while(true)
//      {
//	      for(obj = objFirst; obj != NULL; obj = obj->next) // Search through list for next control (by number)
//	      {
//	         if(obj->tabNr == (cnt+1) && IsTabbable(obj) && obj->tabPageNr == tabNr && obj->tabParent == tabParent)
//   	         return(obj);
//	      }
//         cnt++;
//         if(cnt > max) // Run out of controls so back tab parent
//            return(this->winParent->FindObjectByNr(this->tabParent));
//      }
//
//   }
//   return(NULL);
//}

/************************************************************************
  Search for the last object to which I can tab and return that object
  Returns NULL object if tabbing not defined yet
*************************************************************************/
//
//ObjectData* ObjectData::FindLastObjByTabNumber()
//{
//   short cnt,max = -1,min = 32767;
//   ObjectData *obj,*objFirst,*lastControl = 0;
//
//// Note current object tab number
//   if(this->type == TABCTRL)
//      cnt = 0;
//   else
//      cnt = this->tabNr;
//   
//// Find start of object list
//	for(obj = this->last; obj->last != NULL; obj = obj->last){;}
//	   objFirst = obj->next;
//
//   if(this->tabPageNr == -1 && this->type != TABCTRL)
//   {
//   // Find object with smallest tab number which is tabbable
//  	   for(obj = objFirst; obj != NULL; obj = obj->next)
//  	   {
//	      if(obj->tabNr < min && obj->tabNr > 0 &&IsTabbable(obj) && obj->tabPageNr == -1)
//         {
//	         min = obj->tabNr;
//         }
//      }
//
//   // Find object with largest tab number which is tabbable
//  	   for(obj = objFirst; obj != NULL; obj = obj->next)
//  	   {
//	      if(obj->tabNr > max && IsTabbable(obj) && obj->tabPageNr == -1)
//         {
//	         max = obj->tabNr;
//            lastControl = obj;
//         }
//      }
//
//   // Tabbing not set up yet so return
//      if(!lastControl)
//         return(0);
//
//   // Find object with next lowest tab number which is tabbable
//   // Do this by searching through the list multiple times 
//      while(true)
//      {
//	      for(obj = objFirst; obj != NULL; obj = obj->next)
//	      {
//	         if(obj->tabNr == (cnt-1) && IsTabbable(obj) && obj->tabPageNr == -1)
//   	         return(obj);
//	      }
//         cnt--;
//         if(cnt < min || cnt <= 0) // Run out of control so back to last one
//            return(lastControl);
//
//      }
//   }
//   else
//   {
//      short tabNr,tabParent;
//      if(this->type == TABCTRL)
//      {
//         tabNr = TabCtrl_GetCurSel(this->hWnd);
//         tabParent = this->nr;
//      }
//      else
//      {
//         tabNr = this->tabPageNr;
//         tabParent = this->tabParent;
//      }
//
//  // Find object with smallest tab number which is tabbable
//  	   for(obj = objFirst; obj != NULL; obj = obj->next)
//  	   {
//	      if(obj->tabNr < min && obj->tabNr > 0 &&IsTabbable(obj) && obj->tabPageNr == tabNr  && obj->tabParent == tabParent)
//         {
//	         min = obj->tabNr;
//         }
//      }
//
//   // Find object with largest tab number which is tabbable
//  	   for(obj = objFirst; obj != NULL; obj = obj->next)
//  	   {
//	      if(obj->tabNr > max && IsTabbable(obj) && obj->tabPageNr == tabNr && obj->tabParent == tabParent)
//         {
//	         max = obj->tabNr;
//            lastControl = obj;
//         }
//      }
//
//   // Tabbing not set up yet so return
//      if(!lastControl)
//         return(0);
//
//      if(this->type == TABCTRL)
//         return(lastControl);
//
//   // Find object with next lowest tab number which is tabbable
//   // Do this by searching through the list multiple times 
//      while(true)
//      {
//	      for(obj = objFirst; obj != NULL; obj = obj->next)
//	      {
//	         if(obj->tabNr == (cnt-1) && IsTabbable(obj) && obj->tabPageNr == tabNr && obj->tabParent == tabParent)
//   	         return(obj);
//	      }
//         cnt--;
//         if(cnt < min || cnt <= 0) // Run out of control so back to last one
//            return(this->winParent->FindObjectByNr(this->tabParent));
//
//      }
//   }
//	return(NULL);
//}

/************************************************************************
  Places the object at the top of the Z order
  (note that this seems to be backwards!!)
*************************************************************************/

void ObjectData::MoveWindowToTop()
{
   if(type != TABCTRL & type != PANEL) // Tab and panels controls remain at the bottom
      SetWindowPos(hWnd,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE); 
   else
      SetWindowPos(hWnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE); 
}


/************************************************************************
  See if a window object is tabbable or not
*************************************************************************/

bool ObjectData::IsTabbable()
{
// Some objects are not tabbable
   if(type == STATICTEXT  || type == GROUP_BOX ||
      type == PROGRESSBAR || type == STATUSBOX ||
      type == COLORSCALE)
      return(false);

// Only tab to enabled visible objects
   if(type == RADIO_BUTTON)
   {
      RadioButtonInfo* info = (RadioButtonInfo*)data;
	   for(int i = 0; i < info->nrBut; i++)
	   {   
         if(!IsWindowEnabled(info->hWnd[i]))
           return(false);

         if(!IsWindowVisible(info->hWnd[i]))
            return(false);
      }
   }
   else // Other objects
   {
      if(!IsWindowEnabled(hWnd) || !IsWindowVisible(hWnd))
         return(false);
   }

   return(true);
}


HWND ObjectData::GetCurrentWindow()
{
   if(type == RADIO_BUTTON)
   {
	   RadioButtonInfo *info = (RadioButtonInfo*)data; 
		for(int i = 0; i < info->nrBut; i++)
		{
		   if(SendMessage(info->hWnd[i],BM_GETCHECK,1,0))
		      return(info->hWnd[i]);
		}
   }
            
   return(hWnd);
}

bool ObjectData::IsAWindow(HWND win)
{
   if(type == RADIO_BUTTON)
   {
	   RadioButtonInfo *info = (RadioButtonInfo*)data; 
	   for(int i = 0; i < info->nrBut; i++)
	   {  
         if(info->hWnd[i] == win)
            return(true);	 
      }
   }
   else
   {            
      if(hWnd == win)
         return(true);
   }
   return(false);
}

void ObjectData::EnableObject(bool enable)
{
   if(type == RADIO_BUTTON)
   {
      RadioButtonInfo* info = (RadioButtonInfo*)data;
		if(info)
		{
			for(int i = 0; i < info->nrBut; i++)
			{   
				EnableWindow(info->hWnd[i],enable);
				selected_ = false;
			}
		}
   }
   else if(type == TEXTEDITOR)
   {
      EditParent* info= (EditParent*)data;
		if(info)
		{
			for(int i = 0; i < info->rows*info->cols; i++)
			{
				EnableWindow(info->editData[i]->edWin,enable);
			}
		}
      selected_ = false;
   }
	else if(type == MENU)
	{
      MenuInfo *info = (MenuInfo*)data;
		if(info)
		{
			for(int k = 0; k < info->nrItems; k++)
			{
				if(enable)
					EnableMenuItem(info->menu,k,MF_BYPOSITION | MF_ENABLED);
				else
					EnableMenuItem(info->menu,k,MF_BYPOSITION | MF_GRAYED);
			}
		}
      selected_ = false;
   }
	else if(type == HTMLBOX || type == TABCTRL) // Don't disable these controls.
   {
      selected_ = false;
   }
	else if(type == PANEL) // Make sure the thumb is hidden when enabled if appropriate
   {
		EnableWindow(hWnd,enable);
		selected_ = false;
		if(enable)
			this->UpdatePanelThumb(true);
   }
   else // Enable/disable all others
   {
      EnableWindow(hWnd,enable);
      selected_ = false;
   }


	

}


// Display the object in the parent window

void ObjectData::Show(bool show)
{
   if(type == RADIO_BUTTON)
   {
	   RadioButtonInfo* info = (RadioButtonInfo*)data;
      
	   for(int i = 0; i < info->nrBut; i++)
	   {
         if(show)
		      ShowWindow(info->hWnd[i],SW_SHOW);
         else
		      ShowWindow(info->hWnd[i],SW_HIDE);
	   } 
   }
   else if(type == PANEL)
   {
      PanelInfo *pInfo = (PanelInfo*)data;
      if(show)
      {
         ShowWindow(hWnd,SW_SHOW);
         ShowWindow(pInfo->hWndPanel,SW_SHOW);
			this->UpdatePanelThumb(true); // Ensure the thumb is updated
      }
      else
      {
         ShowWindow(hWnd,SW_HIDE);
         ShowWindow(pInfo->hWndPanel,SW_HIDE);
      }

	   WinData *win = this->winParent;
		for(ObjectData* o: win->widgets.getWidgets())
		{
			if(o->panelParent && (o->panelParent->nr() == this->nr()))
			{
				if(show) // If window is visible then set visibility flag to true otherwise keep false
				{
					if(IsWindowVisible(o->hWnd))
					{
						o->visible = true;
					}
					else
						o->visible = false;
				}
				else // Note: do not hide the control window - panel hiding takes care of this
				{
				   o->visible = false;
				}
			}
		}
   }
   else if(type == GROUP_BOX)
   {
      if(show)
      {
         ShowWindow(hWnd,SW_SHOW);
      }
      else
      {
         EraseGroupBox();
         ShowWindow(hWnd,SW_HIDE);
      }
   }
   else if(type == CHECKBOX)
   {
      if(show)
      {
         ShowWindow(hWnd,SW_SHOW);
      }
      else
      {
         EraseCheckBox();
         ShowWindow(hWnd,SW_HIDE);
      }
   }
   else if(type == STATUSBOX)
   {     
      if(show)
      {
		   ShowWindow(hWnd,SW_SHOW);
         StatusBoxInfo* info = (StatusBoxInfo*)this->data;
         ShowWindow(this->winParent->blankStatusBar,false);
       //  if(info && info->subWindow)
       //     ShowWindow(info->subWindow,SW_SHOW);
      }
      else
      {
		   ShowWindow(hWnd,SW_HIDE);
         StatusBoxInfo* info = (StatusBoxInfo*)this->data;
         if(info && info->subWindow)
            ShowWindow(info->subWindow,SW_HIDE);
      }
   }
   else
   {
      if(show)
		   ShowWindow(hWnd,SW_SHOW);
      else
		   ShowWindow(hWnd,SW_HIDE);
   }

}

// Move the object - special code for radio buttons

void ObjectData::Move(long x, long y, long w, long h, bool redraw)
{
   if(type == RADIO_BUTTON)
   {
	   RadioButtonInfo* info = (RadioButtonInfo*)data;

	   for(int i = 0; i < info->nrBut; i++)
	   {
         if(info->orient == 'h')
            MoveWindow(info->hWnd[i],x+info->spacing*i,y,14,14,redraw);  
         else
            MoveWindow(info->hWnd[i],x,y+info->spacing*i,14,14,redraw);  
	   } 
   }
   //else if(type == TEXTBOX)
   //{
   //   MoveWindow(hWnd,x-1,y,w+2,h,redraw);
   //}
   else
   {
      MoveWindow(hWnd,x,y,w,h,redraw); 
   }
}

// Free data stored in an object list

void ObjectData::FreeData(void)
{
   if(command)
   {
      delete [] command;
      command = NULL;
   }

   validationCode = 0; // Makes sure this object can no longer be accessed

// Delete the window user data
   if(type != HTMLBOX)
   {
      WinInfo* wi = (WinInfo*)GetWindowLong(hWnd,GWL_USERDATA);
      if(wi)
      {
         delete wi;
         SetWindowLong(hWnd,GWL_USERDATA,0);
      }
   }

// Remove object window data

   if(type == BUTTON)
   {
      PushButtonInfo* info = (PushButtonInfo*)this->data;
      if(info)
      {
         
 //  CText label;
	//this->GetWindowText(label);
 //  printf("Deleting button '%s'\n",label.Str());

         if(info->hImage) // Delete any button image
         {
             //  printf("Deleting image '%s'\n",label.Str());
            if(win7Mode)
               ::delete (Bitmap*)info->hImage;
            else
               DeleteObject((HBITMAP)info->hImage);
            info->hImage = NULL;
         }
         delete info;
         info = NULL;
      }
      ShowWindow(hWnd,SW_HIDE);
	   DestroyWindow(hWnd);         	      	      
	}
   else if(type == STATICTEXT)
   {
      if(data)
		{
		   StaticTextInfo* info = (StaticTextInfo*)this->data;
			if(info)
         {
				delete info;
            info = NULL;
         }
		}
	   ShowWindow(hWnd,SW_HIDE);
	   DestroyWindow(hWnd);         	      	      
	}
	else if(type == GRIDCTRL)
	{      
		if(data)
		{
			BabyGrid* grid = (BabyGrid*)data;
         delete grid;
         grid = NULL;
		}
		ShowWindow(hWnd,SW_HIDE);
	   DestroyWindow(hWnd);     
	}
   else if(type == RADIO_BUTTON)
   {
      RadioButtonInfo* info = (RadioButtonInfo*)data;
      if(info)
      {
	      for(int i = 0; i < info->nrBut; i++)
         {
        //    SetWindowLong(info->hWnd[i],GWL_WNDPROC,(LONG)OldButtonProc);
	         ShowWindow(info->hWnd[i],SW_HIDE);
	         DestroyWindow(info->hWnd[i]);
	      }
	      if(info->states)
         {
	         delete [] info->states;
            info->states = NULL;
         }
	      if(info->hWnd) 
         {
	         delete [] info->hWnd;
            info->hWnd = NULL;
         }
         delete info;
         info = NULL;
      }
	}
   else if(type == CHECKBOX)
   {
      CheckButtonInfo* info = (CheckButtonInfo*)data;
	   if(info->states) 
      {
	      delete [] info->states;
         info->states = NULL;
      }
      delete info;	
      info = NULL;
      SetWindowLong(hWnd,GWL_WNDPROC,(LONG)OldButtonProc);
	   ShowWindow(hWnd,SW_HIDE);
	   DestroyWindow(hWnd);            	      
	}
   else if(type == STATICTEXT2)
   {
      if(data)
      {
         delete data;
         data = NULL;
      }
	   ShowWindow(hWnd,SW_HIDE);
	   DestroyWindow(hWnd);         	      	      
	}
   else if(type == SLIDER)
   {
      SetWindowLong(hWnd,GWL_WNDPROC,(LONG)OldSliderProc);
      ShowWindow(hWnd,SW_HIDE);
	   DestroyWindow(hWnd);         	      	      
	}
   else if(type == LISTBOX)
   {
		if(data)
		{
			ListBoxInfo* info = (ListBoxInfo*)data;
			if(info->colWidth)
			{
				delete [] info->colWidth;
				info->colWidth = 0;
				info->nrColumns = 0;
				if(info->menu)
				{
            //   DeleteObject(info->menu);
					info->menu = NULL;
				}
			}
			delete info;
         info = NULL;
		}
      SetWindowLong(hWnd,GWL_WNDPROC,(LONG)OldListBoxProc);
      ShowWindow(hWnd,SW_HIDE);
	   DestroyWindow(hWnd);         	      	      
	}
	else if(type == GETMESSAGE)
	{
	   if(data)   
      {
	      delete data;
         data = NULL;
      }
   }
   else if(type == COLORBOX)
   {
      if(data)
      {
         delete [] data;
         data = NULL;
      }
      SetWindowLong(hWnd,GWL_WNDPROC,(LONG)OldButtonProc);
	   ShowWindow(hWnd,SW_HIDE);
	   DestroyWindow(hWnd); 
   }
   else if(type == HTMLBOX)
   {
	   ShowWindow(hWnd,SW_HIDE);
      DestroyAX(hWnd);
      DestroyHTMLViewer(hWnd);
   }
   else if(type == TOOLBAR)
   {
      if(data)
      {
         ToolBarInfo* info = (ToolBarInfo*)data;
         if(info->item)
         {
            delete []  info->item;
            info->item = NULL;
         }
         if(info->bitmap)
         {
            DeleteObject(info->bitmap);
            info->bitmap = 0;
         }
         delete info;
         info = NULL;
      }    
	   ShowWindow(hWnd,SW_HIDE);
	   DestroyWindow(hWnd);  
   }
   else if(type == MENU)
   {
      if(data)
      {
         MenuInfo* info = (MenuInfo*)data;
         for(int i = 0; i < info->nrItems; i++)
         {
            if(info->cmd[i])    delete [] info->cmd[i];
            if(info->label[i])  delete [] info->label[i];
            if(info->key[i])    delete [] info->key[i];
         }
         if(info->cmd)  delete [] info->cmd;
         if(info->label) delete [] info->label;
         if(info->accel) delete [] info->accel;
         if(info->key) delete [] info->key;
         if(info->name)  delete [] info->name;
         if(info->menu)  DestroyMenu(info->menu);
         delete info;
         info = NULL;
      }
   }
   else if(type == STATUSBOX)
   {
      StatusBoxInfo* info = (StatusBoxInfo*)data;
      if(info)
      {
         if(info->posArray)
         {
            for(int i = 0 ; i < info->parts; i++)
               delete [] info->posArray[i];

            delete [] info->posArray;
         }
         delete info;
         info = NULL;
      }
	   ShowWindow(hWnd,SW_HIDE);
	   DestroyWindow(hWnd);
   }
   else if(type == TEXTEDITOR)
   {
       EditParent *ep = (EditParent*)data;
       if(ep)
       {
          ep->FreeEditMemory();
          delete ep;
          ep = NULL;
       }
	   ShowWindow(hWnd,SW_HIDE);
	   DestroyWindow(hWnd);  
   }
   else if(type == PLOTWINDOW)
   {
       PlotWindow *pp = (PlotWindow*)data;
       if(pp)
       {
          delete pp;
          pp = NULL;
       }
	   ShowWindow(hWnd,SW_HIDE);
	   DestroyWindow(hWnd);  
   }
   else if(type == IMAGEWINDOW)
   {
       PlotWindow *pp = (PlotWindow*)data;
       if(pp)
          delete pp;
       pp = NULL;
	   ShowWindow(hWnd,SW_HIDE);
	   DestroyWindow(hWnd);  
   }
   else if(type == OPENGLWINDOW)
   {
      CPlot3D *plot = (CPlot3D*)data;
       if(plot3d == plot)
          plot3d = NULL;
       if(hWnd == cur3DWin)
          cur3DWin = NULL;
       if(plot)
          delete plot;
       plot = NULL;
	    ShowWindow(hWnd,SW_HIDE);
	    DestroyWindow(hWnd);
   }
   else if(type == PICTURE)
   {
      PictureInfo *info = (PictureInfo*)data;
      if(info)
		{
         DeleteObject(info->bmp);
			delete info;
         info = NULL;
		}
      SetWindowLong(hWnd,GWL_WNDPROC,(LONG)OldPictureProc);
	   ShowWindow(hWnd,SW_HIDE);
	   DestroyWindow(hWnd);  
   }
   else if(type == TABCTRL)
   {
      if(data)
      {
         TabInfo *info = (TabInfo*)data;
         if(info->tabLabels)
            delete [] info->tabLabels;
         delete info;
         info = NULL;
      }
      SetWindowLong(hWnd,GWL_WNDPROC,(LONG)OldTabCtrlProc);
	   ShowWindow(hWnd,SW_HIDE);
	   DestroyWindow(hWnd);  
   }
   else if(type == DIVIDER)
   {
      if(data)
         delete data;
      data = NULL;
      if(hWnd)
      {
	      ShowWindow(hWnd,SW_HIDE);
	      DestroyWindow(hWnd);
      }
   }
   else if(type == UPDOWN)
   {
      if(data)
         delete data;
      data = NULL;
      if(hWnd)
      {
         SetWindowLong(hWnd,GWL_WNDPROC,(LONG)OldUpDownProc);
	      ShowWindow(hWnd,SW_HIDE);
	      DestroyWindow(hWnd);
      }
   }
   else if(type == CLIWINDOW)
   {
      if(hWnd)
      {
  //       HFONT font = (HFONT)SendMessage(hWnd,WM_GETFONT,(WPARAM)0,(LPARAM)0);
         if(cliEditWin == hWnd)
            cliEditWin = NULL;
	      ShowWindow(hWnd,SW_HIDE);
	      DestroyWindow(hWnd);
  //       DeleteObject(font);
      }
   }
   else if(type == TEXTMENU)
   {
      if(hWnd)
      {
         POINT pt;
         pt.x = 3; 
         pt.y = 3; 
         HWND hwndEdit = RealChildWindowFromPoint(hWnd, pt);  
		   SetWindowLong(hwndEdit,GWL_WNDPROC,(LONG)OldTextMenuEditProc); 
		   SetWindowLong(hWnd,GWL_WNDPROC,(LONG)OldTextMenuProc); 
	      ShowWindow(hWnd,SW_HIDE);
	      DestroyWindow(hWnd);
      }
   }
   else if(type == TEXTBOX)
   {
      if(hWnd)
      {
         SetWindowLong(hWnd,GWL_WNDPROC,(LONG)OldTextBoxProc); 
	      ShowWindow(hWnd,SW_HIDE);
	      DestroyWindow(hWnd);
      }
   }
   else if(type == PANEL)
   {
      PanelInfo* info = (PanelInfo*)data;
      if(info)
		{
			SetWindowLong(info->hWndPanel,GWL_WNDPROC,(LONG)OldPanelProc); 
			ShowWindow(info->hWndPanel,SW_HIDE);
			DestroyWindow(info->hWndPanel);  
			delete info;
         info = NULL;
		}
      SetWindowLong(hWnd,GWL_WNDPROC,(LONG)OldPanelScrollProc); 
	   ShowWindow(hWnd,SW_HIDE);
	   DestroyWindow(hWnd);  
   }
	else
	{
      if(hWnd)
      {
	      ShowWindow(hWnd,SW_HIDE);
	      DestroyWindow(hWnd);
      }
	}

   if(toolTipHwnd)
   {
      DestroyWindow(toolTipHwnd);
   }

   //if(menu)
   //{
   //   DestroyMenu(menu);
   //   menu = NULL;
   //}
   // Remove any submenus but don't delete them
   // Then delete the object menu
   if(menu)
   {
      int c = GetMenuItemCount(menu);
      while(RemoveMenu((HMENU) menu,0,MF_BYPOSITION) != 0){;}
      DestroyMenu(menu);
      menu = NULL;
   }

   if(menuList)
   {
      delete [] menuList;
      menuList = NULL;
   }

   if(accelTable)
   {
       DestroyAcceleratorTable(accelTable);
       accelTable = NULL;
   }

   if(varList.next)
   {
		varList.RemoveAll();
    //  varList.next->Remove();
   //   varList.next = NULL;
   }

   data = NULL;
}


/***********************************************
   Get the dimensions of a control 
***********************************************/

void ObjectData::GetSize(short &w, short &h)
{
   RECT r;
   GetWindowRect(this->hWnd,&r);
   w = r.right  - r.left;
   h = r.bottom - r.top;
      
   if(type == RADIO_BUTTON)
   {
      w = wo;
      h = ho;
   }
}


// Return the tooltip for an object
char* ObjectData::GetToolTip()
{
   if(this->toolTipHwnd)
   {
      HWND hParWin = this->hWnd;
      if(this->type == RADIO_BUTTON)
      {
	      RadioButtonInfo* info = (RadioButtonInfo*)this->data;
         hParWin = info->hWnd[0];
      }
      TOOLINFO toolinfo;
      static char toolText[MAX_STR];
      memset(&toolinfo, 0, sizeof(TOOLINFO));
      toolinfo.cbSize = sizeof(TOOLINFO);
      toolinfo.hwnd = this->hwndParent;
      toolinfo.uFlags = TTF_SUBCLASS | TTF_IDISHWND;
      toolinfo.uId = (UINT)hParWin;
      toolinfo.hinst = NULL;
      toolinfo.lpszText = toolText;
      SendMessage (toolTipHwnd, TTM_GETTEXT, 0, (LPARAM)&toolinfo );
      return(toolText);
   }
   else
   {
      return("undefined");
   }
}

/***************************************************************************************
   Return object text
***************************************************************************************/

void ObjectData::GetWindowText(CText &txt)
{
   long n = ::GetWindowTextLength(hWnd);
   char *title = new char[n+1];
   n = ::GetWindowText(hWnd,title,n+1);
   title[n] = '\0';
   txt.Assign(title);
   delete [] title;
}

void ObjectData::GetRectExp(CText *xs, CText *ys, CText *ws, CText *hs, bool quote)
{
   GetXExp(xs,quote);
   GetYExp(ys,quote);
   GetWExp(ws,quote);
   GetHExp(hs,quote);
}

void ObjectData::GetXExp(CText *xs, bool quote)
{
   float xoff;

   if(type == PANEL)
   {
       PanelInfo* info = (PanelInfo*)data;
       xoff = info->pos.xo;
   }
   else
      xoff = xSzOffset;

   if(type == STATICTEXT)
   {
      POINT p = {0,0};
      ClientToScreen(hWnd,&p);
      ScreenToClient(hwndParent,&p);
      RECT ro;
      GetWindowRect(hWnd,&ro);
      long objwidth = ro.right-ro.left;

      long style = GetWindowLong(hWnd,GWL_STYLE);
      style = style & 0x0003;
      if(style == SS_LEFT)
         xoff = xSzOffset; // Return left coord of text
      else if(style == SS_RIGHT)
         xoff = xSzOffset+objwidth; // Return right coord of text
      else if(style == SS_CENTER)
         xoff = xSzOffset+objwidth/2; // Return centre coord of text
   }

   if(xoff != 0)
   {
      if(xSzScale == 1)
         xs->Format("ww%+ld",nint(xoff));
      else if(xSzScale != 0)
         xs->Format("ww*%g%+ld",xSzScale,nint(xoff));
      else
         xs->Format("%ld",nint(xoff));
   }
   else
   {
      if(xSzScale == 1)
         xs->Format("ww");
      else if(xSzScale != 0)
         xs->Format("ww*%g",xSzScale);
      else
         xs->Format("0");
   }

   if(quote)
   {
      if(xs->Search(0,'w') != -1)
         *xs = "\"" + *xs + "\"";
   }

}

void GetSizeExp(char *dir, CText *txt, float scale, float offset,bool quote)
{   
   if(offset != 0)
   {
      if(scale == 1)
         txt->Format("%s%+ld",dir,nint(offset));
      else if(scale != 0)
         txt->Format("%s*%g%+ld",dir,scale,nint(offset));
      else
         txt->Format("%ld",nint(offset));
   }
   else
   {
      if(scale == 1)
         txt->Format("%s",dir);
      else if(scale != 0)
         txt->Format("%s*%g",dir,scale);
      else
         txt->Format("0");
   }

   if(quote)
   {
      if(txt->Search(0,'w') != -1)
         *txt = "\"" + *txt + "\"";
   }

}
void ObjectData::GetYExp(CText *ys, bool quote)
{
   int yoff;
   if(type == PANEL)
   {
       PanelInfo* info = (PanelInfo*)data;
       yoff = info->pos.yo;
   }
   else
      yoff = ySzOffset;

   if(yoff != 0)
   {
      if(ySzScale == 1)
         ys->Format("wh%+ld",nint(yoff));
      else if(ySzScale != 0)
         ys->Format("wh*%g%+ld",ySzScale,nint(yoff));
      else
         ys->Format("%ld",nint(yoff));
   }
   else
   {
      if(ySzScale == 1)
         ys->Format("wh");
      else if(ySzScale != 0)
         ys->Format("wh*%g",ySzScale);
      else
         ys->Format("0");
   }

   if(quote)
   {
      if(ys->Search(0,'w') != -1)
         *ys = "\"" + *ys + "\"";
   }
}

void ObjectData::GetWExp(CText *ws, bool quote)
{
   float woff;

   if(type == PANEL)
   {
       PanelInfo* info = (PanelInfo*)data;
       woff = info->pos.wo;
   }
   else
      woff = wSzOffset;


   if(woff != 0)
   {
      if(wSzScale == 1)
         ws->Format("ww%+ld",nint(woff));
      else if(wSzScale != 0)
         ws->Format("ww*%g%+ld",wSzScale,nint(woff));
      else
         ws->Format("%ld",nint(woff));
   }
   else
   {
      if(wSzScale == 1)
         ws->Format("ww");
      else if(xSzScale != 0)
         ws->Format("ww*%g",wSzScale);
      else
         ws->Format("0");
   }

   if(quote)
   {
      if(ws->Search(0,'w') != -1)
         *ws = "\"" + *ws + "\"";
   }
}

void ObjectData::GetHExp(CText *hs, bool quote)
{
   float hoff;

   if(type == PANEL)
   {
       PanelInfo* info = (PanelInfo*)data;
       hoff = info->pos.ho;
   }
   else
      hoff = hSzOffset;

   if(hoff != 0)
   {
      if(hSzScale == 1)
         hs->Format("wh%+ld",nint(hoff));
      else if(hSzScale != 0)
         hs->Format("wh*%g%+ld",hSzScale,nint(hoff));
      else
         hs->Format("%ld",nint(hoff));
   }
   else
   {
      if(hSzScale == 1)
         hs->Format("wh");
      else if(xSzScale != 0)
         hs->Format("wh*%g",hSzScale);
      else
         hs->Format("0");
   }

   if(quote)
   {
      if(hs->Search(0,'w') != -1)
         *hs = "\"" + *hs + "\"";
   }
}

/***********************************************
   Get the position of a control 
***********************************************/

void ObjectData::GetPosition(short &x, short &y)
{
   RECT r;
   GetWindowRect(this->hWnd,&r);
   x = r.left;
   y = r.top;
}

/***********************************************
   Draw a selection rectangle around a control 
***********************************************/

void ObjectData::DrawSelectRect(HDC hdc)
{
   HPEN rectPen = CreatePen(PS_DOT,0,RGB(0,0,0));
   HPEN oldPen = (HPEN)SelectObject(hdc,rectPen);
   SetROP2(hdc,R2_COPYPEN);

   if(type == PANEL)
   {
      PanelInfo *info = (PanelInfo*)data;
      ::DrawRect(hdc,info->x-3,info->y-3,info->x+info->w+22,info->y+info->h+2);
      SelectObject(hdc,oldPen);
      DeleteObject(rectPen);
      return;
   }
 
// Group box is special since it can't be selected in middle
// and so need special resizing pads.
   if(type == GROUP_BOX || type == TABCTRL)
   {
      ::DrawRect(hdc,xo+wo-5,yo+ho/2-4,xo+wo+3,yo+ho/2+4);
      ::DrawRect(hdc,xo+wo/2-4,yo+ho-5,xo+wo/2+4,yo+ho+3);
      ::DrawRect(hdc,xo+wo-5,yo+ho-5,xo+wo+3,yo+ho+3);
   }

// Draw main rectangle
   ::DrawRect(hdc,xo,yo,xo+wo-1,yo+ho-1);

   SelectObject(hdc,oldPen);
   DeleteObject(rectPen);
}


/***********************************************
   Draw an error rectangle around a control 
***********************************************/

void ObjectData::DrawErrorRect(HWND parentWindow)
{
   HDC hdc = GetDC(parentWindow);
   HPEN rectPen = CreatePen(PS_SOLID,0,RGB(255,0,0));
   HPEN oldPen = (HPEN)SelectObject(hdc,rectPen);
   SetROP2(hdc,R2_COPYPEN);
   RECT r;
   GetClientRect(parentWindow,&r);

// Make sure we can't draw outside the parent window
   IntersectClipRect(hdc,r.left,r.top,r.right,r.bottom);
 
// Only these controls are relevant for this error
   if(type == TEXTBOX)
   {
      ::DrawRect(hdc,xo-1,yo,xo+wo,yo+ho-1);
   }
   else if(type == TEXTMENU)
   {
      ::DrawRect(hdc,xo,yo,xo+wo-1,yo+ho-1);
   }
   SelectObject(hdc,oldPen);
   DeleteObject(rectPen);
   ReleaseDC(parentWindow,hdc);
}

/***********************************************
 Draw a black rectangle around a control 
 with a white centre
***********************************************/

void ObjectData::DrawRect(HDC hdc)
{
   HPEN rectPen = CreatePen(PS_SOLID,0,RGB(0,0,0));
   HPEN oldPen = (HPEN)SelectObject(hdc,rectPen);
   SetROP2(hdc,R2_COPYPEN);
	HBRUSH bkBrush  = CreateSolidBrush(RGB(255,255,255));	
   RECT r;

// Draw main rectangle
   SetRect(&r,xo,yo,xo+wo-1,yo+ho-1);
   FillRect(hdc,&r,bkBrush);
   ::DrawRect(hdc,xo,yo,xo+wo-1,yo+ho-1);
   SelectObject(hdc,oldPen);
   DeleteObject(rectPen);
   DeleteObject(bkBrush);
}

/**************************************************
Draw a focus rectangle or circle around a control 
***************************************************/

void DrawRBCircle(HDC hdc, short x1, short y1);
void DrawDottedControlRect(HDC hdc, short x1, short y1, short w, short h);
void DrawRBSquare(HDC hdc, short x1, short y1);

void ObjectData::DrawFocusRect(HDC hdc, bool erase, HWND win)
{
   RECT r;
// Only do this for radiobuttons and checkboxes
   if(type != RADIO_BUTTON && type != CHECKBOX && type != COLORBOX && type != UPDOWN)
      return;

// Erase rectangle/circle
   if(erase)
   {
      RECT r;
      if(!win)
         win = GetCurrentWindow();
	    GetClientRect(win,&r);
	    int w = r.right-r.left+1;
	    int h = r.bottom-r.top+1;
	    POINT p = {0,0};
	    ClientToScreen(win,&p);
	    ScreenToClient(hwndParent,&p);

      if(type == RADIO_BUTTON)
         SetRect(&r,p.x-3,p.y-2,p.x+w+1,p.y+h+2);
      else if(type == CHECKBOX)
         SetRect(&r,p.x-2,p.y-2,p.x+wo+1,p.y+ho+2);
      else if(type == COLORBOX)
         SetRect(&r,p.x-2,p.y-2,p.x+wo+2,p.y+ho+2);
      else if(type == UPDOWN)
         SetRect(&r,p.x-2,p.y-2,p.x+wo+2,p.y+ho+3);
      MyInvalidateRect(hwndParent,&r,true);
      MyUpdateWindow(hwndParent);
   }

// Draw rectangle/circle
   else
   {
      if(!win)
         win = GetCurrentWindow();
      GetClientRect(win,&r);
      int w = r.right-r.left+1;
	   int h = r.bottom-r.top+1;
      POINT p = {0,0};
      ClientToScreen(win,&p);
      ScreenToClient(hwndParent,&p);
      HPEN rectPen = CreatePen(PS_SOLID,0,RGB(0,0,0));
      HPEN oldPen = (HPEN)SelectObject(hdc,rectPen);
      if(type == RADIO_BUTTON)
          DrawRBSquare(hdc,p.x-3,p.y-2);
      else if(type == CHECKBOX)
         DrawDottedControlRect(hdc,p.x-2,p.y-2,wo+2,ho+3);
      else if(type == COLORBOX)
         DrawDottedControlRect(hdc,p.x-2,p.y-2,wo+3,ho+3);
      else if(type == UPDOWN)
         DrawDottedControlRect(hdc,p.x-2,p.y-2,wo+3,ho+4);

      SelectObject(hdc,oldPen);
      DeleteObject(rectPen);
   }
}

POINT dots[] = {{9,3},{12,3},{15,4},{17,6},{18,9},{18,12},{17,15},{15,17},{12,18},{9,18},{6,17},{4,15},{3,12},{3,9},{4,6},{6,4}};
//POINT dots[] = {{6,0},{8,0},{10,0},{12,1},{14,2},{15,4},{16,6},{16,8},{16,10},{15,12},{14,14},{12,15},{10,16},{8,16},{6,16},{4,15},
//                {2,14},{1,12},{0,10},{0,8},{0,6},{1,4},{2,2},{4,1}};


// Draw a dotted circle around a radiobutton
void DrawRBCircle(HDC hdc, short x1, short y1)
{
   short x,y;

   for(int i = 0; i < 16; i++)
   {
      x = dots[i].x + x1 - 2-3;
      y = dots[i].y + y1 - 2-2;
      SetPixel(hdc,x,y,RGB(0,0,0));
   }
}

// Draw a dotted square around the radiobutton
void DrawRBSquare(HDC hdc, short x1, short y1)
{
   short x,y,h=16;
   OSVERSIONINFO versInfo;
   versInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
   GetVersionEx(&versInfo);

// 2000 & XP version
   if(versInfo.dwMajorVersion == 5)
   {
      x1++;
      for(int i = 1; i <= h; i+=2) // top
      {
         x = i+x1;
         y = y1;
         SetPixel(hdc,x,y,RGB(0,0,0));
      }

      for(int i = 1; i <= h; i+=2) // Bottom
      {
         x = i+x1;
         y = y1+h;
         SetPixel(hdc,x,y,RGB(0,0,0));
      }

      for(int i = 1; i <= h; i+=2) // Left
      {
         x = x1;
         y = y1+i;
         SetPixel(hdc,x,y,RGB(0,0,0));
      }

      for(int i = 1; i <= h; i+=2) // Right
      {
         x = x1+h;
         y = y1+i;
         SetPixel(hdc,x,y,RGB(0,0,0));
      } 
   }
   else // Vista and Windows 7
   {
      for(int i = 1; i <= h+2; i+=2)
      {
         x = i+x1;
         y = y1;
         SetPixel(hdc,x,y,RGB(0,0,0));
      }

      for(int i = 0; i <= h; i+=2)
      {
         x = i+x1;
         y = y1+h+1;
         SetPixel(hdc,x,y,RGB(0,0,0));
      }

      for(int i = 1; i <= h; i+=2)
      {
         x = x1;
         y = y1+i;
         SetPixel(hdc,x,y,RGB(0,0,0));
      }

      for(int i = 2; i <= h; i+=2)
      {
         x = x1+h+1;
         y = y1+i;
         SetPixel(hdc,x,y,RGB(0,0,0));
      } 
   }
 
}

// Draw a dotted rectange around a control
void DrawDottedControlRect(HDC hdc, short x1, short y1, short w, short h)
{
   short x,y;
   short xoff=0,yoff=0;

   if(!(h%2)) // Even number of pixel in height
      yoff = 1;

   if(!(w%2)) // Even number of pixels in width
      xoff = 1;

   if(!(h%2)  && !(w%2)) // Both even
   {
      yoff = 0;
      xoff = 0;
   }

   if((h%2)  && (w%2)) // Both odd
   {
      yoff = 1;
      xoff = 1;
   }

   for(int i = 1; i <= w; i+=2) // Top
   {
      x = i+x1;
      y = y1;
      SetPixel(hdc,x,y,RGB(0,0,0));
   }

   for(int i = 1; i <= w-xoff; i+=2) // Bottom
   {
      x = i+x1+xoff;
      y = y1+h;
      SetPixel(hdc,x,y,RGB(0,0,0));
   }

   for(int i = 1; i <= h; i+=2) // Left
   {
      x = x1;
      y = y1+i;
      SetPixel(hdc,x,y,RGB(0,0,0));
   }

   for(int i = 1; i <= h-yoff; i+=2) // Right
   {
      x = x1+w;
      y = y1+i+yoff;
      SetPixel(hdc,x,y,RGB(0,0,0));
   }
}


/***********************************************
    Draw the object number over the control 
***********************************************/

void ObjectData::DrawControlNumber(HDC hdc)
{
 	SetTextColor(hdc,RGB(255,0,0));
   SetBkColor(hdc,RGB(255,255,255)); 

   HFONT oldFont = (HFONT)SelectObject(hdc,numberFont);

   CText numStr;
   numStr.Format("%d",nr());
   if(panelParent)
      TextOut(hdc,0,0,numStr.Str(),numStr.Size());
   else
      TextOut(hdc,xo,yo,numStr.Str(),numStr.Size());

   SelectObject(hdc,oldFont);
}

/***********************************************
    Draw the tab number over the control 
***********************************************/

void ObjectData::DrawTabNumber(HDC hdc)
{
 	SetTextColor(hdc,RGB(255,255,0));
   SetBkColor(hdc,RGB(0,0,0)); 

   HFONT oldFont = (HFONT)SelectObject(hdc,numberFont);

   CText numStr;
   numStr.Format("%d",tabNr);
   TextOut(hdc,xo,yo,numStr.Str(),numStr.Size());

   SelectObject(hdc,oldFont);
}

/*****************************************************************************
*                 Process an object class procedure
*****************************************************************************/

short ObjectData::ProcessClassProc(Interface *itfc, char *cmd, char *parameter)
{
   CText arg;
   CText param,value;
   short r,nrArgs;
	CArg carg;

   if(!parameter)
      nrArgs = 0;
   else
      nrArgs = carg.Count(parameter);

   if(!strcmp(cmd,"set")) // Set an object parameter e.g. obj->set("width", 20)
   {
      arg.Format("%d,%d,%s",this->winParent->nr,this->nr(),parameter);
      r = SetParameter(itfc, arg.Str());
   }
   else if(!strcmp(cmd,"get")) // Get an object parameter e.g. obj->get("width")
   {
      arg.Format("%d,%d,%s",this->winParent->nr,this->nr(),parameter);
      r = GetParameter(itfc, arg.Str());
   }
   else // Implicit command syntax
   {
      if(nrArgs == 0)  // Get an object parmeter e.g. obj->width()
      {
       //  arg.Format("%d,%d,\"%s\"",this->winParent->nr,this->nr(),cmd);
       //  r = GetParameter(itfc, arg.Str());
         param = cmd;
         r = GetParameterCore(itfc, this, param);
      }
      else if(nrArgs == 1) // Set an object parameter obj->width(20)
      {
         itfc->nrRetValues = 0;
        // arg.Format("%d,%d,\"%s\",%s",this->winParent->nr,this->nr(),cmd,parameter);
        // r = SetParameter(itfc, arg.Str());
         CText paramTxt = parameter;
         CText cmdTxt;
         cmdTxt.Format("\"%s\"",cmd);
         r = SetParameterCore(itfc, this, cmdTxt, paramTxt);
      }
      else
      {
         ErrorMessage("Invalid command- zero or one argument expected");
         return(ERR);
      }
   }

   return(r);
}

short ObjectData::DefaultProc(Interface *itfc, char *args)
{
   short r = OK;

   switch(this->type)
   {
      case(PLOTWINDOW):
      {
         PlotWindow *pp;
         if(this->data)
         {
            pp = (PlotWindow*)this->data;
            r = pp->DefaultProc(itfc,args);
         }
         break;
      }
   }

   return(r);
}


const char* const ObjectData::GetTypeAsString(short type)
{
	switch(type)
	{
	case(RADIO_BUTTON):
		return ("radio button");
	case(CHECKBOX):
		return ("check box");
	case(STATICTEXT):
		return ("static text");
	case(TEXTBOX):
		return ("text box");
	case(TEXTMENU):
		return ("text menu");
	case(LISTBOX):
		return ("list box");
	case(SLIDER):
		return ("slider");
	case(GETMESSAGE):
		return ("get message");
	case(PROGRESSBAR):
		return ("progress bar");
	case(STATUSBOX):
		return ("status box");	         	         	         
	case(COLORSCALE):
		return ("color scale");
	case(GROUP_BOX):
		return ("group box");
	case(BUTTON):
		return ("button");	  
	case(COLORBOX):
		return ("color box");
	case(UPDOWN):
		return ("updown control");	
	case(DIVIDER):
		return ("divider");	
	case(TEXTEDITOR):
		return ("Text editor");	
	case(CLIWINDOW):
		return ("CLI window");	
	case(PLOTWINDOW):
		return ("1D plot window");	
	case(IMAGEWINDOW):
		return ("2D plot window");
	case(OPENGLWINDOW):
		return ("3D plot window");
	case(TOOLBAR):
		return ("toolbar");
	case(MENU):
		return ("menu");
	case(TABCTRL):
		return ("tabctrl");
	case(GRIDCTRL):
		return ("grid control");
	default:
		return ("unknown");
	}
}


short ObjectData::GetTypeAsNumber(CText &type)
{
	if(type == "radio button")
      return(RADIO_BUTTON);
	else if(type == "check box")
	   return(CHECKBOX);
	else if(type == "static text")
	   return(STATICTEXT);
	else if(type == "text box")
	   return(TEXTBOX);
	else if(type == "text menu")
	  return(TEXTMENU);
	else if(type == "list box")
	   return(LISTBOX);
	else if(type == "slider")
	   return(SLIDER);
	else if(type == "get message")
	   return(GETMESSAGE);
	else if(type == "progress bar")
	   return(PROGRESSBAR);
	else if(type == "status box")	
	   return(STATUSBOX);
 	else if(type == "color scale")        	         	         
	   return(COLORSCALE);
	else if(type == "group box")
	   return(GROUP_BOX);
	else if(type == "button")	  
	   return(BUTTON);
	else if(type == "color box")
	   return(COLORBOX);
	else if(type == "updown control")	
	   return(UPDOWN);
	else if(type == "divider")	
	   return(DIVIDER);
	else if(type == "Text editor")	
	   return(TEXTEDITOR);
	else if(type == "CLI window")	
	   return(CLIWINDOW);
	else if(type == "1D plot window")	
	   return(PLOTWINDOW);
	else if(type == "2D plot window")
	   return(IMAGEWINDOW);
	else if(type == "3D plot window")
	   return(OPENGLWINDOW);
	else if(type == "toolbar")
	   return(TOOLBAR);
	else if(type == "menu")
	   return(MENU);
	else if(type == "tabctrl")
	   return(TABCTRL);
	else if(type == "grid control")
	   return(GRIDCTRL);
   else
      return(-1);
}

/************************************************************************
Generate a string representing the state of this object's parameter-
accessible attributes.
************************************************************************/

const string ObjectData::FormatState()
{
	StringPairs state;

   state.add("type",GetTypeAsString(type));
	state.add("ctrlnr",stringifyInt(nr()).c_str());
	state.add("ctrlID",objName);
	state.add("valueID",valueName);
	state.add("visible",toTrueFalse(visible));
	state.add("enabled",toTrueFalse(enable));
   CText numbers;
	numbers.Format("(%g,%g)", lower,upper);
	state.add("range",numbers.Str());
	state.add("statusbox",stringifyInt(statusbox).c_str());
	state.add("toolbar",stringifyInt(toolbar).c_str());
	state.add("tooltip",  this->GetToolTip());
	numbers.Format("(%hd,%hd,%hd,%hd)", region.left, region.right, region.top, region.bottom);
	state.add("region",numbers.Str());
	state.add("x",stringifyInt(xo).c_str());
	state.add("y",stringifyInt(yo).c_str());
	state.add("w",stringifyInt(wo).c_str());
   state.add("h", stringifyInt(ho).c_str());
   state.add("procedure", this->command);
   return FormatStates(state);
}

bool ObjectData::isTabChild()
{	
	return (tabParent != 0);
}

bool ObjectData::isTabParent()
{
	return (type == TABCTRL);
}

ObjectData* ObjectData::getTabParent()
{
	return tabParent;
}

bool ObjectData::isTabChildOf(ObjectData* parent)
{
	return ((type != TABCTRL) && (tabParent == parent));	
}

bool ObjectData::isSelected()
{
	return selected_;
}

void ObjectData::setSelected(bool selected)
{
	this->selected_ = selected;
}

bool ObjectData::hasDefaultObjectName()
{
	if (strcmp(objName, DEFAULT_OBJECT_STRING))
	{
		return false;
	}
	return true;
}

bool ObjectData::hasDefaultValueName()
{
	if (strcmp(valueName, DEFAULT_OBJECT_STRING))
	{
		return false;
	}
	return true;
}

char* ObjectData::getObjectName()
{
	return objName;
}


bool ObjectData::compareCtrlNumbers(ObjectData* o1, ObjectData* o2)
{
	return (o1->nr() < o2->nr());
}


short ObjectData::nr()
{
	return nr_;
}

void ObjectData::nr(short number)
{
   if(type == RADIO_BUTTON) // Need to move each sub-button
   {
      RadioButtonInfo* info = (RadioButtonInfo*)data; 
      if(info)
      {
	      for(int i = 0; i < info->nrBut; i++)
	      {
            SetWindowLong(info->hWnd[i],GWL_ID,number);  
         }
      }
   }
   else
   {
      SetWindowLong(hWnd,GWL_ID,number); 
   }
	nr_ = number;
}

bool ObjectData::menuNrEquals(HMENU menuNr)
{
	if (type == MENU && data)
	{
		MenuInfo *info = (MenuInfo*)data;
		return (info->menu == menuNr);
	}
	return false;
}

bool ObjectData::winEquals(HWND win)
{
	if(type == RADIO_BUTTON)
	{
		RadioButtonInfo *info = (RadioButtonInfo*)data;
		for(int i = 0; i < info->nrBut; i++)
		{  
			if(info->hWnd[i] == win)
				return true;	 
		}
	}
	else
	{            
		if(hWnd == win)
			return true;
	}
	return false;
}

void ObjectData::resetControlNumber()
{
	nr_ = -1;
	// Set the window identifier
	if(type == RADIO_BUTTON)
	{
		RadioButtonInfo* info = (RadioButtonInfo*)data;  
		for(int i = 0; i < info->nrBut; i++)
			SetWindowLong(info->hWnd[i],GWL_ID,nr_);
	}
	else
		SetWindowLong(hWnd,GWL_ID,nr_);
}

void ObjectData::resetTabNumber()
{
	tabNr = -1;
}

void ObjectData::selectIfWithinRect(FloatRect* r)
{
	if(xo >= r->left && xo <= r->right&&
		yo >= r->top && yo <= r->bottom && visible)
	{
		selected_ = true;
	}  
}


short ObjectData::Place(short x, short y, short w, short h, bool updateSz)
{   
	RECT r;
	WinData *win;

	// Update the object positions relative to the window
	win = GetWinDataClass(hwndParent);
	GetClientRect(hwndParent,&r);
	short ww = r.right-r.left;
	short wh = r.bottom-r.top;
	short offy = 0;

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
			offy = r.bottom+3;
			wh -= r.bottom+3;
		}
	}

	// Calculate original position based on window scale and offset parameters

	if(updateSz)
	{
		short oldx = nint(xSzScale*ww + xSzOffset);
		short oldy = nint(ySzScale*wh + ySzOffset + offy);
		short oldw = nint(wSzScale*ww + wSzOffset);
		short oldh = nint(hSzScale*wh + hSzOffset);

		// Update the object positions relative to the window  
		// Need to turn this off when toolbar is wrappable
		xSzOffset += (x - oldx);
		ySzOffset += (y - oldy);
		wSzOffset += (w - oldw);
		hSzOffset += (h - oldh);   
	}

	// Move and redraw object at final position - note that the radiobuttons
	// require extra work
	if(type == RADIO_BUTTON) // Need to move each sub-button
	{
		RadioButtonInfo* info = (RadioButtonInfo*)data;
		short xp,yp,wp,hp;
		xp = x;
		yp = y;
		wp = 14;
		hp = 14;

		xo = x; yo = y;  

		for(int i = 0; i < info->nrBut; i++)
		{
			MoveWindow(info->hWnd[i],xp,yp,wp,hp,true);
			if(visible)
				ShowWindow(info->hWnd[i],SW_SHOW);
			SetWindowLong(info->hWnd[i],GWL_ID,nr());

			if(info->orient == 'h')
				xp += info->spacing;
			else
				yp += info->spacing;

		}  
		SetRect(&r,xo,yo,xo+wo+1,yo+ho+1);
		MyInvalidateRect(hwndParent,&r,false);
	}
	else if(type == PANEL) 
	{
      PanelInfo* info = (PanelInfo*)this->data;
		int scrollWidth = GetSystemMetrics(SM_CXVSCROLL);

		if(this->wSzScale == 0)
		{
         info->x = x;
			x = info->x + +info->w;
		}
      else // Resizing so calc new width with scrollbar x
      {
         info->w = (w-scrollWidth);
         x = info->x+info->w;
      }

      info->y = y;
      info->h = h;

	   MoveWindow(hWnd,x,y,scrollWidth,h,true);
      MoveWindow(info->hWndPanel,info->x,info->y,info->w,info->h,true);
	   xo = x; yo = y;
	   wo = scrollWidth; ho = h;
	} 
	//else if(type == TEXTBOX) // This allows for the strange sizing of this window
	//{
	//	MoveWindow(hWnd,x-1,y,w+2,h,true);
	//	xo = x; yo = y;
	//	wo = w; ho = h;
	//} 
	else if(type == TEXTEDITOR) // This allows for the strange sizing of this window
	{
		EditParent *ep;
		EditRegion *er;
		short xs,ys,ws,hs;

		ep = (EditParent*)data;

		for(short i = 0; i < ep->rows*ep->cols; i++) 
		{	
			er = ep->editData[i];
			xs = nint(er->x*w + x);
			ys = nint(er->y*h + y);
			ws = nint(er->w*w);
			hs = nint(er->h*h);	               
			MoveWindow(ep->editData[i]->edWin,xs,ys,ws,hs,true);   		            
		}	

		xo = x; yo = y;
		wo = w; ho = h;
	} 
	else if(type == HTMLBOX)
	{
		MoveWindow(hWnd,x,y,w+1,h,true); // This forces the vertical scroll-bar thumb to
		MoveWindow(hWnd,x,y,w,h,true);   // redraw correcty if only expanding vertically
		xo = x; yo = y;
		wo = w; ho = h;
	}
	else
	{
		MoveWindow(hWnd,x,y,w,h,true);
		xo = x; yo = y;
		wo = w; ho = h;
	}
	return(OK);
}

void ObjectData::invalidate()
{
	RECT r;
   if(type == PANEL)
   {
      PanelInfo *pInfo = (PanelInfo*)this->data;
      int px = pInfo->x;
      int py = pInfo->y;
      int pw = pInfo->w;
      int ph = pInfo->h;
	   SetRect(&r,px,py,px+pw+wo+1,py+ph+1);
   }
   else
   {
	   SetRect(&r,xo,yo,xo+wo+1,yo+ho+1);
   }
	MyInvalidateRect(hwndParent,&r,false);
}

void ObjectData::ClearTabRegion()
{
  	RECT r;
   if(type == TABCTRL)
   {
      HDC hdc = GetDC(this->hwndParent);
	   SetRect(&r,xo,yo,xo+wo+1,yo+ho);
      //            hBrush = (HBRUSH)CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
   	HBRUSH hBrush = (HBRUSH)CreateSolidBrush(RGB(255,0,0));  
      FillRect(hdc,&r,hBrush);
      DeleteObject(hBrush);
      ReleaseDC(this->hWnd,hdc);
   }
}


short ObjectData::updateVisibilityWRTTabs()
{
	if(type != TABCTRL)
	{
		if(visible && isTabChild())
		{
			ObjectData* tab = getTabParent();
			if(!tab || tab->type != TABCTRL)
			{
				ErrorMessage("tab parent for control %d is not a valid tab control");
				return(ERR);
			}
			int tabNumber = TabCtrl_GetCurSel(tab->hWnd);
			if(tabPageNr != tabNumber)
			{
				visible = false;
				Show(false);
			}
		}
	}
	else // Ensure that tab controls are at the bottom and therefore visible 
	{
		if(visible)
			SetWindowPos(hWnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
	}
	return OK;
}

void ObjectData::processTab()
{
	ObjectData *nextObj = 0;

	// Tab to the next (or last) object
	if(IsKeyDown(VK_SHIFT)) 
	{
		if (winParent->tabMode == TAB_BY_CTRL_NUMBER)
			nextObj = winParent->widgets.findPrevByNumber(this);
		else
			nextObj = winParent->widgets.findPrevByTabNumber(this);
	}
	else
	{
		if (winParent->tabMode == TAB_BY_CTRL_NUMBER)
			nextObj = winParent->widgets.findNextByNumber(this);
		else
			nextObj = winParent->widgets.findNextByTabNumber(this);
	}

	// Set the focus on the next object and check for special cases
	if(nextObj)
	{
		SetFocus(nextObj->GetCurrentWindow());
		if(nextObj->type == RADIO_BUTTON)
		{ // Draw the focus rect
			HDC hdc = GetDC(nextObj->hwndParent);
			nextObj->DrawFocusRect(hdc,0);
			ReleaseDC(nextObj->hwndParent,hdc);
		}
		if(nextObj->type == TEXTBOX) 
		{// If textbox select line
			long len = SendMessage(nextObj->hWnd,EM_LINELENGTH,(WPARAM)0, (LPARAM)0);
			SendMessage(nextObj->hWnd,EM_SETSEL,(WPARAM)0, (LPARAM)len); // (Include /r)
		}
	}
}

HRGN ObjectData::CreateTabBkgRegion()
{
   extern void GetTabRegion(ObjectData *obj, HRGN hRgn);
	RECT r;
	HRGN hRgnRect,hRngPanel;
	HRGN hRgn = CreateRectRgn(0,0,1,1);
	short tabNumber = TabCtrl_GetCurSel(hWnd);

  // GetTabRegion(this,hRgn); // Returns region which includes all tab rectangles REMOVED BECAUSE OF FLICKERING ON MULTILINE TAB

	for(ObjectData *obj: winParent->widgets.getWidgets())
	{ 
		if(obj->tabParent == this && obj->tabPageNr == tabNumber)
		{
         if(obj->visible)
			{
            if(obj->type == PANEL)
            {
               hRgnRect = obj->GetPanelRegion();
				   CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
					DeleteObject(hRgnRect);
            }
				else if(obj->type == CLIWINDOW)
				{
					SetRect(&r,obj->xo+5,obj->yo+2,obj->xo+obj->wo-5,obj->yo+obj->ho-2);
					hRgnRect = CreateRectRgnIndirect(&r);
					CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
					DeleteObject(hRgnRect);
				}
				else if(obj->type == TEXTEDITOR)
				{
               if(obj->panelParent)
               {
                  PanelInfo *pInfo = (PanelInfo*)obj->panelParent->data;
                  int px = pInfo->x;
                  int py = pInfo->y;
                  int pw = pInfo->w;
                  int ph = pInfo->h;
                  int xoff = px+2;
                  int yoff = py+2;

                  SetRect(&r,px,py,px+pw,py+ph);
				    	HRGN hRngPanel = CreateRectRgnIndirect(&r);
					   SetRect(&r,obj->xo+xoff+1,obj->yo+yoff,obj->xo+obj->wo-1+xoff,obj->yo+obj->ho+yoff);
					   hRgnRect = CreateRectRgnIndirect(&r);
					   CombineRgn(hRgnRect,hRgnRect,hRngPanel,RGN_AND);
					   CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
                  DeleteObject(hRgnRect);
                  DeleteObject(hRngPanel);
               }
               else
               {
				//	   SetRect(&r,obj->xo-1,obj->yo,obj->xo+obj->wo+1,obj->yo+obj->ho);
					   SetRect(&r,obj->xo,obj->yo,obj->xo+obj->wo,obj->yo+obj->ho);
					   hRgnRect = CreateRectRgnIndirect(&r);
					   CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
                  DeleteObject(hRgnRect);
               }
				}
				else if(obj->type == TEXTBOX)
				{
               if(obj->panelParent)
               {
                  PanelInfo *pInfo = (PanelInfo*)obj->panelParent->data;
                  int px = pInfo->x;
                  int py = pInfo->y;
                  int pw = pInfo->w;
                  int ph = pInfo->h;
                  int xoff = px+2;
                  int yoff = py+2;

                  SetRect(&r,px,py,px+pw,py+ph);
				    	HRGN hRngPanel = CreateRectRgnIndirect(&r);
					//   SetRect(&r,obj->xo-1+xoff,obj->yo+yoff,obj->xo+obj->wo+1+xoff,obj->yo+obj->ho+yoff);
					   SetRect(&r,obj->xo+xoff,obj->yo+yoff,obj->xo+obj->wo+xoff,obj->yo+obj->ho+yoff);
					   hRgnRect = CreateRectRgnIndirect(&r);
					   CombineRgn(hRgnRect,hRgnRect,hRngPanel,RGN_AND);
					   CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
					   DeleteObject(hRgnRect);
                  DeleteObject(hRngPanel);
               }
               else
               {
					 //  SetRect(&r,obj->xo-1,obj->yo,obj->xo+obj->wo+1,obj->yo+obj->ho);
					   SetRect(&r,obj->xo,obj->yo,obj->xo+obj->wo,obj->yo+obj->ho);
					   hRgnRect = CreateRectRgnIndirect(&r);
					   CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
                  DeleteObject(hRgnRect);
               }
				}
				else if(obj->type == TOOLBAR)
				{
					SetRect(&r,obj->xo,obj->yo,obj->xo+obj->wo,obj->yo+obj->ho);
					hRgnRect = CreateRectRgnIndirect(&r);
					CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
					DeleteObject(hRgnRect);
				}
				else if(obj->type == RADIO_BUTTON)
				{
					RadioButtonInfo *info = (RadioButtonInfo*)obj->data;

               if(obj->panelParent)
               {
                  PanelInfo *pInfo = (PanelInfo*)obj->panelParent->data;
                  int px = pInfo->x;
                  int py = pInfo->y;
                  int pw = pInfo->w;
                  int ph = pInfo->h;
                  int xoff = px+2;
                  int yoff = py+2;

                  SetRect(&r,px,py,px+pw,py+ph);
				    	HRGN hRngPanel = CreateRectRgnIndirect(&r);
					   for(int i = 0; i < info->nrBut; i++)
					   {  
						   GetSubWindowRect(winParent->hWnd,info->hWnd[i], &r);
						   hRgnRect = CreateRectRgnIndirect(&r);
					      CombineRgn(hRgnRect,hRgnRect,hRngPanel,RGN_AND);
						   CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
						   DeleteObject(hRgnRect); 
					   }
                  DeleteObject(hRngPanel);
               }
               else
               {
					   for(int i = 0; i < info->nrBut; i++)
					   {  
						   GetSubWindowRect(winParent->hWnd,info->hWnd[i], &r);
						   hRgnRect = CreateRectRgnIndirect(&r);
						   CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
						   DeleteObject(hRgnRect); 
					   }
               }
				}
				else if(obj->type == GROUP_BOX)
				{
					hRgnRect = obj->GetGroupBoxRegion();
					CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
					DeleteObject(hRgnRect);
				}
				else
				{
               if(obj->panelParent)
               {
                  PanelInfo *pInfo = (PanelInfo*)obj->panelParent->data;
                  int px = pInfo->x;
                  int py = pInfo->y;
                  int pw = pInfo->w;
                  int ph = pInfo->h;
                  int xoff = px+2;
                  int yoff = py+2;

                  SetRect(&r,px,py,px+pw,py+ph);
				    	HRGN hRngPanel = CreateRectRgnIndirect(&r);
					   SetRect(&r,obj->xo+xoff,obj->yo+yoff,obj->xo+obj->wo+xoff,obj->yo+obj->ho+yoff);
					   hRgnRect = CreateRectRgnIndirect(&r);
					   CombineRgn(hRgnRect,hRgnRect,hRngPanel,RGN_AND);
					   CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
                  DeleteObject(hRgnRect);
                  DeleteObject(hRngPanel);
               }
               else
               {
					   SetRect(&r,obj->xo,obj->yo,obj->xo+obj->wo,obj->yo+obj->ho);
					   hRgnRect = CreateRectRgnIndirect(&r);
					   CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
                  DeleteObject(hRgnRect);
               }
				}
			}
		}
	} 

// Calculate the region inside the window rectangle which doesn't
// include the objects 
	SetRect(&r,xo,yo,xo+wo,yo+ho);
	hRgnRect = CreateRectRgnIndirect(&r);
	CombineRgn(hRgn,hRgnRect,hRgn,RGN_DIFF);
	OffsetRgn(hRgn,-xo,-yo);
	DeleteObject(hRgnRect);

	return(hRgn);
}

HRGN ObjectData::GetGroupBoxRegion()
{
	HRGN hRgn = CreateRectRgn(0,0,1,1);
	SIZE te; 
	bool islabel = true;

	char label[MAX_STR];
	HDC hdc = GetDC(hWnd);
	::GetWindowText(hWnd,label,MAX_STR);

	if(label[0] == '\0')
	{
		islabel = false;
		strncpy_s(label,MAX_STR,"Test",_TRUNCATE);
	}

	SelectObject(hdc,controlFont);
	GetTextExtentPoint32(hdc, label, strlen(label), &te);


	int top = yo+te.cy/2; // Top of rectange
	int left = xo;
	int right = xo+wo;
	int base = yo+ho;


	if(islabel)
	{
		int gap = 2;
		int off = 5;

		// Don't let text go outside groupbox width
		if(left+gap+off+te.cx >= right)
			te.cx = right - left - gap - off - gap;

		hRgn = AddRectToRgn(hRgn,left+2,top,left+off,top+1);
		hRgn = AddRectToRgn(hRgn,left+off+gap,yo,left+off+gap+te.cx,yo+te.cy);
		hRgn = AddRectToRgn(hRgn,left+off+gap+te.cx+gap,top,right-2,top+1);
	}
	else
	{
		hRgn = AddRectToRgn(hRgn,left+2,top,right-2,top+1); // Horizontal top
	}

	hRgn = AddRectToRgn(hRgn,right-3,top+1,right-1,top+2); // Top right corner
	hRgn = AddRectToRgn(hRgn,right-2,top+2,right-1,top+3);

	hRgn = AddRectToRgn(hRgn,right-1,top+2,right,base-2); // Vertical  right

	hRgn = AddRectToRgn(hRgn,right-3,base-2,right-1,base-1); // bottom right corner
	hRgn = AddRectToRgn(hRgn,right-2,base-3,right-1,base-2);

	hRgn = AddRectToRgn(hRgn,left+2,base-1,right-2,base); // Horizontal bottom

	hRgn = AddRectToRgn(hRgn,left,base-3,left+2,base-2); // bottom left corner
	hRgn = AddRectToRgn(hRgn,left+1,base-2,left+3,base-1);

	hRgn = AddRectToRgn(hRgn,left,top+2,left+1,base-2); // Vertical  left

	hRgn = AddRectToRgn(hRgn,left,top+2,left+2,top+3); // top left corner
	hRgn = AddRectToRgn(hRgn,left+1,top+1,left+3,top+2);

	ReleaseDC(hWnd,hdc);

	return(hRgn);
}

int ObjectData::DrawGroupBox(LPDRAWITEMSTRUCT pdis)
{
	RECT r = pdis->rcItem;
	SIZE te; 
	bool islabel = true;

	HDC hdc = pdis->hDC;
	COLORREF bkgColor = gMainWindowColor;

	SetBkColor(hdc, bkgColor);

	char label[MAX_STR];
	::GetWindowText(hWnd,label,MAX_STR);

	if(label[0] == '\0')
	{
		islabel = false;
		strncpy_s(label,MAX_STR,"Test",_TRUNCATE);
	}

	SelectObject(hdc,controlFont);
	GetTextExtentPoint32(hdc, label, strlen(label), &te);

	COLORREF col = GetSysColor(COLOR_BTNSHADOW);
	HPEN pen = CreatePen(PS_SOLID,0,col);  
	SelectObject(hdc, pen); 

	int off = 5;
	int gap = 2;

	int x = r.left+off+gap;
	int y = r.top;

	int top = r.top+te.cy/2;

	if(islabel) //Top side with label
	{
		if(enable)
			SetTextColor(hdc,RGB(0,70,213));
		else
			SetTextColor(hdc,GetSysColor(COLOR_GRAYTEXT));

		if(r.left+gap+off+te.cx >= r.right)
		{
			HRGN hRgn = CreateRectRgn(r.left, r.top, r.right-gap, r.bottom); 
			SelectClipRgn(hdc, hRgn); 
			TextOut(hdc,x,y,label, strlen(label));
			MoveToEx(hdc,r.left+2,top,NULL);
			LineTo(hdc,r.left+off,top);
			MoveToEx(hdc,r.left+off+gap+te.cx+gap,top,NULL);
			LineTo(hdc,r.right-2,top);
			SelectClipRgn(hdc, NULL); 
			DeleteObject(hRgn);
		}
		else
		{
			TextOut(hdc,x,y,label, strlen(label));
			MoveToEx(hdc,r.left+2,top,NULL);
			LineTo(hdc,r.left+off,top);
			MoveToEx(hdc,r.left+off+gap+te.cx+gap,top,NULL);
			LineTo(hdc,r.right-2,top);
		}
	}
	else
	{
		MoveToEx(hdc,r.left+2,top,NULL);
		LineTo(hdc,r.right-2,top); //Top side
	}

	// Top right corner
	MoveToEx(hdc,r.right-3,top+1,NULL);
	LineTo(hdc,r.right-1,top+1); 
	MoveToEx(hdc,r.right-2,top+2,NULL);
	LineTo(hdc,r.right-1,top+2); 

	// Right side
	MoveToEx(hdc,r.right-1,top+2,NULL);
	LineTo(hdc,r.right-1,r.bottom-2);

	// Bottom right corner
	MoveToEx(hdc,r.right-2,r.bottom-3,NULL);
	LineTo(hdc,r.right-1,r.bottom-3); 
	MoveToEx(hdc,r.right-3,r.bottom-2,NULL);
	LineTo(hdc,r.right-1,r.bottom-2); 

	// Bottom side
	MoveToEx(hdc,r.right-3,r.bottom-1,NULL);
	LineTo(hdc,r.left+1,r.bottom-1); 

	// Bottom left corner
	MoveToEx(hdc,r.left+1,r.bottom-3,NULL);
	LineTo(hdc,r.left+2,r.bottom-3); 
	MoveToEx(hdc,r.left+1,r.bottom-2,NULL);
	LineTo(hdc,r.left+3,r.bottom-2); 

	// Left side
	MoveToEx(hdc,r.left,r.bottom-3,NULL);
	LineTo(hdc,r.left,top+1); 

	// Top left corner
	MoveToEx(hdc,r.left+1,top+2,NULL);
	LineTo(hdc,r.left+2,top+2); 
	MoveToEx(hdc,r.left+1,top+1,NULL);
	LineTo(hdc,r.left+3,top+1); 

	DeleteObject(pen);

	return(1); //<-- Correct?
}

void ObjectData::EraseGroupBox()
{
	HRGN hRgn;
	HDC hdc = (HDC)GetDC(hWnd);
	hRgn = GetGroupBoxRegion();
	OffsetRgn(hRgn,-xo,-yo);
	HBRUSH hBrush = (HBRUSH)CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	FillRgn(hdc,hRgn,hBrush);
	DeleteObject(hBrush);
	DeleteObject(hRgn); 
	ReleaseDC(hWnd,hdc);
}

int ObjectData::DrawPanel()
{
   HDC hdc = GetDC(hwndParent);

   RECT rw;
//	GetWindowRect(hWnd,&rw);
 //  ScreenToClientRect(this->hwndParent,&rw);

   PanelInfo* info = (PanelInfo*)data;

   rw.top = info->y;
   rw.left = info->x;
   rw.right = info->x+info->w-1;
   rw.bottom = info->y+info->h-1;


   MoveToEx(hdc,rw.left,rw.top,NULL);
	LineTo(hdc,rw.right,rw.top); 
	LineTo(hdc,rw.right,rw.bottom);
	LineTo(hdc,rw.left,rw.bottom); 
	LineTo(hdc,rw.left,rw.top); 

   rw.top = info->y-2;
   rw.left = info->x-2;
   rw.right = info->x+info->w+21;
   rw.bottom = info->y+info->h+1;


   MoveToEx(hdc,rw.left,rw.top,NULL);
	LineTo(hdc,rw.right,rw.top); 
	LineTo(hdc,rw.right,rw.bottom);
	LineTo(hdc,rw.left,rw.bottom); 
	LineTo(hdc,rw.left,rw.top);


	ReleaseDC(hwndParent,hdc);
	return(1); //<-- Correct?
}

void ObjectData::EraseCheckBox()
{
   RECT rw;
	GetWindowRect(hWnd,&rw);
   ScreenToClientRect(this->hwndParent,&rw);
   InflateRect(&rw,2,2);
   MyInvalidateRect(this->hwndParent,&rw,FALSE);
}

void ObjectData::decorate(HWND parentWin)
{
   HWND win;

   if(panelParent)
      win = this->hWnd;
   else
      win = parentWin;

	if(!panelParent && selected_)
	{
		HDC hdc = GetDC(parentWin);
		DrawSelectRect(hdc);
		ReleaseDC(parentWin,hdc);
	}
	if(winParent->displayObjCtrlNrs)
	{
		HDC hdc = GetDC(win);
		DrawControlNumber(hdc);
		ReleaseDC(win,hdc);
	}
	if(winParent->displayObjTabNrs)
	{
		HDC hdc = GetDC(win);
		DrawTabNumber(hdc);
		ReleaseDC(win,hdc);
	}

}

short ObjectData::setMenuItemCheck(const char* const menuKey, bool check)
{
	if(type != MENU)
		return(ERR);

	MenuInfo *info = (MenuInfo*)data;

	for(int i = 0; i < info->nrItems; i++)
	{
		if(!strcmp(menuKey,info->key[i]))
		{
			if(check)
				CheckMenuItem(info->menu, i, MF_BYPOSITION |  MF_CHECKED);
			else
				CheckMenuItem(info->menu, i, MF_BYPOSITION | MF_UNCHECKED);
			return(OK);
		}
	}

	return(ERR);

}


short ObjectData::setMenuItemEnable(const char* const menuKey, bool enable)
{
	if(type != MENU)
		return(ERR);

	MenuInfo *info = (MenuInfo*)data;

	for(int i = 0; i < info->nrItems; i++)
	{
		if(!strcmp(menuKey,info->key[i]))
		{
			if(enable)
				EnableMenuItem(info->menu, i, MF_BYPOSITION |  MF_ENABLED);
			else
				EnableMenuItem(info->menu, i, MF_BYPOSITION | MF_DISABLED);
			return(OK);
		}
	}

	return(ERR);

}

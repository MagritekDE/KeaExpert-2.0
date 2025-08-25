#include "stdafx.h"
#include "shellapi.h"
#include "files.h"
#include "listbox.h"
#include "evaluate.h"
#include "globals.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include "guiWindowCLI.h"
#include "interface.h"
#include "list_functions.h"
#include "main.h"
#include "mymath.h"
#include "plot1dCLI.h"
#include "process.h"
#include "variablesOther.h"
#include "memoryLeak.h"

#pragma warning (disable: 4312) // Ignore Get/SetWindowLong warnings
#pragma warning (disable: 4311) // Ignore pointer truncation warnings

WNDPROC OldListBoxProc;
static LRESULT CALLBACK  ListBoxEventProc(HWND, UINT, WPARAM, LPARAM);

static bool listBoxUpdated = false; // Set when listbox contents is renewed
                                    // reset by the drawitem procedure
static int lastPosition = -1; // Last line selected in the listbox prevent 
                              // moving up to the top line


/***************************************************************************
   Make a list box object
***************************************************************************/

void MakeListBoxObject(ObjectData *obj, long x, long y, long w, long h, DWORD visibility)
{
   obj->hWnd = CreateWindowEx(WS_EX_CLIENTEDGE	,"listbox", "", WS_CHILD  | LBS_NOTIFY | WS_VSCROLL | LBS_NOINTEGRALHEIGHT | LBS_HASSTRINGS | visibility | LBS_OWNERDRAWFIXED,
	      			                x, y, w, h, 
	      			                obj->hwndParent, (HMENU)obj->nr(),
	      			                prospaInstance, NULL);
   OldListBoxProc = (WNDPROC)SetWindowLong(obj->hWnd,GWL_WNDPROC,(LONG)ListBoxEventProc);
}

/***************************************************************************
   Override routine for listbox (draws icon and text)
***************************************************************************/

int DrawListBoxItem(ObjectData *obj, LPDRAWITEMSTRUCT pdis)
{
   HBITMAP hbmpOld;
   TEXTMETRIC tm;
   long xoff; // x offset of first column (allow for possible icon)
	static long xoff_icon = 0;
	static long xoff_noicon = 0;
   COLORREF rgb[50*24]; 
   COLORREF rgb2[50*24]; 
   HDC hdcMem;
   short indx,shift;


   if(pdis->itemID == -1) 
   { 
      return(1); 
   } 

	lastPosition = pdis->itemID;

// See if there is an icon present or not
   long itemData = (long) SendMessage(pdis->hwndItem, LB_GETITEMDATA, pdis->itemID, (LPARAM) 0);
	if(itemData == -1)
		itemData = 255;
	long pic = itemData & 0x000000FF;
	long fgColor = (itemData & 0xFFFFFF00) >> 8;

// Get the number of items
	int nrItems = SendMessage(pdis->hwndItem, LB_GETCOUNT, 0,0); 

// Get other listbox information
	ListBoxInfo* info = (ListBoxInfo*)obj->data;
	HDC hdc = pdis->hDC;

   switch (pdis->itemAction) 
   { 
// Draw this row and highlight if it has been selected
	   case (ODA_DRAWENTIRE): 
      case (ODA_SELECT): 
      {
			// Draw the background and text for this row -------------------------------------------

         if(pic != 255) // If true then there an icon in first column so draw it
         {
            hdcMem = CreateCompatibleDC(pdis->hDC); 
         // Byte reverse the window background colour
            COLORREF bakcol = GetSysColor(COLOR_BTNFACE);
            bakcol = ((bakcol&0x00FF0000)>>16)  + ((bakcol&0x0000FF00)) + ((bakcol&0x000000FF)<<16);

        // Get the bitmap and replace blue with background color
            if(pic > 127)
            {
               indx = pic&0xFF;
               shift = pic>>8;
            }
            else 
            {
               indx = pic;
               shift = 0;
            }

            HBITMAP icon = iconBMPs[indx];
            BITMAP bitmap;
            GetObject(icon,sizeof(BITMAP),&bitmap);
            long w = bitmap.bmWidth;
            long h = bitmap.bmHeight;
            GetBitmapBits(icon,w*h*4,(LPVOID)rgb);
            memcpy(rgb2,rgb,w*h*4);
            for(long i = 0; i < w*h; i++)
               if(rgb[i] == 255) rgb[i] = bakcol;
            SetBitmapBits(icon,w*h*4,(LPVOID)rgb);

        // Copy the bitmap to the screen
            hbmpOld = (HBITMAP)SelectObject(hdcMem, icon); 

            BitBlt(pdis->hDC, 
                  pdis->rcItem.left+shift, pdis->rcItem.top, 
                  pdis->rcItem.right - pdis->rcItem.left, 
                  pdis->rcItem.bottom - pdis->rcItem.top, 
                  hdcMem, 0, 0, SRCCOPY); 


        // Restore the bitmap for next time
            SetBitmapBits(icon,w*h*4,(LPVOID)rgb2);

            xoff = w+shift; 
				xoff_icon = xoff;
         }
         else // No icon so no offset
			{
            xoff = 0;
				xoff_noicon = xoff;
			}

			// Getting the text for this row and measure its size.
         char tchBuffer[MAX_STR];
         SendMessage(pdis->hwndItem, LB_GETTEXT, pdis->itemID, (LPARAM) tchBuffer); 
         GetTextMetrics(pdis->hDC, &tm); 

			// Get information about the vertical scrollbar
			SCROLLBARINFO si;
			si.cbSize = sizeof(SCROLLBARINFO);
			GetScrollBarInfo(obj->hWnd, OBJID_VSCROLL, &si);
			int isScrollBar = !(si.rgstate[0] & 0x8000);
			int scrollBarWidth = (isScrollBar)*GetSystemMetrics(SM_CXVSCROLL);

         int y = (pdis->rcItem.bottom + pdis->rcItem.top - tm.tmHeight) / 2; 

	  // Draw the background for this row ----------------------------------------

			// Rectangle 'r' defines dimensions for this row
			RECT r;
			r.left   = pdis->rcItem.left+xoff; 
			r.top    = pdis->rcItem.top; 
			r.right  = pdis->rcItem.right; 
			r.bottom = pdis->rcItem.bottom;

			// First item is the title (but only if columns are defined)
			if(pdis->itemID == 0 && info->colWidth != 0)
			{
	         HBRUSH backBrush = CreateSolidBrush(RGB(245,245,245));
				RECT r = pdis->rcItem;
				r.bottom = r.bottom/3;
				int base = r.bottom;
            FillRect(pdis->hDC,&r,backBrush);
            DeleteObject(backBrush);
				r = pdis->rcItem;
				r.top = base;
	         backBrush = CreateSolidBrush(RGB(230,230,230));
            FillRect(pdis->hDC,&r,backBrush);
            DeleteObject(backBrush);
				COLORREF col = GetSysColor(COLOR_BTNSHADOW); 
				HPEN pen = CreatePen(PS_SOLID,0,col);  
				SelectObject(hdc, pen); 
				MoveToEx(hdc,pdis->rcItem.left,pdis->rcItem.bottom-1,NULL);
				LineTo(hdc,pdis->rcItem.right,pdis->rcItem.bottom-1);
				DeleteObject(pen);

				if(pdis->itemID == nrItems-1) // Last entry
				{
					RECT fullR = pdis->rcItem;
					fullR.left = fullR.left+xoff;
					fullR.top = fullR.bottom;
					fullR.bottom = obj->ho-4;
					FillRect(pdis->hDC,&fullR,(HBRUSH)GetStockObject(WHITE_BRUSH));
				}
			}
			else // Non-title rows
			{ 
				if (pdis->itemState & ODS_SELECTED) // When selected background is button interface color
				{ 
					if(pic != 255)
						r.left += 4;

					if(info->firstLineSelected != info->lastLineSelected)
					{
						if(pdis->itemID ==  info->lastLineSelected)
						{
						//	printf("Last row = %d\n",pdis->itemID);
							HBRUSH backBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
							FillRect(pdis->hDC,&r,backBrush);
						}
					}

					HBRUSH backBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
					FillRect(pdis->hDC,&r,backBrush);
					DeleteObject(backBrush);

					if(pdis->itemID == nrItems-1) // Last entry so fill the rest of the list with white
					{
						RECT fullR = r;
						fullR.top = r.bottom;
						fullR.bottom = obj->ho-4;
					   FillRect(pdis->hDC,&fullR,(HBRUSH)GetStockObject(WHITE_BRUSH));
					}
					if(pic != 255)
						r.left -= 4;
				} 
				else // Otherwise set the background to white unless this is a multiple row selection
				{
					//printf("first or nth ID = %d first = %d last = %d\n",pdis->itemID,info->firstLineSelected,info->lastLineSelected);
				   if(info->firstLineSelected != info->lastLineSelected)
					{
						if(pdis->itemID  == info->firstLineSelected)
						{
						//	printf("First row = %d\n",pdis->itemID);
							HBRUSH backBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
							FillRect(pdis->hDC,&r,backBrush);
							if(pdis->itemID == nrItems-1) 
							{
								RECT fullR = r;
								fullR.top = r.bottom;
								fullR.bottom = obj->ho-4;
								FillRect(pdis->hDC,&fullR,(HBRUSH)GetStockObject(WHITE_BRUSH));
							}							
						}
						else if((pdis->itemID > info->firstLineSelected && pdis->itemID  < info->lastLineSelected)  ||
								  (pdis->itemID < info->firstLineSelected && pdis->itemID  > info->lastLineSelected))
						{
						//	printf("nth row = %d\n",pdis->itemID);

							HBRUSH backBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
							FillRect(pdis->hDC,&r,backBrush);
						}
						else
						{
							FillRect(pdis->hDC,&r,(HBRUSH)GetStockObject(WHITE_BRUSH));
							if(pdis->itemID == nrItems-1) 
							{
								RECT fullR = r;
								fullR.top = r.bottom;
								fullR.bottom = obj->ho-4;
								FillRect(pdis->hDC,&fullR,(HBRUSH)GetStockObject(WHITE_BRUSH));
							}		
						}
					}
					else
					{
						FillRect(pdis->hDC,&r,(HBRUSH)GetStockObject(WHITE_BRUSH));
						if(pdis->itemID == nrItems-1) 
						{
							RECT fullR = r;
							fullR.top = r.bottom;
							fullR.bottom = obj->ho-4;
							FillRect(pdis->hDC,&fullR,(HBRUSH)GetStockObject(WHITE_BRUSH));
						}	
					}
				}
			}

		// Define the column positions for this row -------------------------------

			// Reset ectangle 'r' defines dimensions for this row
			r.left   = pdis->rcItem.left+xoff; // xoff accounts for icon column
			r.top    = pdis->rcItem.top; 
			r.right  = pdis->rcItem.right; 
			r.bottom = pdis->rcItem.bottom;

         SetBkMode(pdis->hDC,TRANSPARENT);
			// Make an array to store the column widths - either user defined or fixed
			// based on the number of columns
			if(pic != 255)
			   info->hasIcons = true;

			float *colWidth = 0;
			if(info->colWidth == NULL)
			{
				colWidth = new float[info->nrColumns];
				for(int k = 0; k < info->nrColumns; k++)
					colWidth[k] = 1.0/info->nrColumns;
			}
			else
			{
				colWidth = info->colWidth;
			}
	
		// Draw the text for this row ----------------------------------------------

			int len = strlen(tchBuffer);
			char* item = new char[len+1];
			int p = 0;
			int colNr = 0;
			int savedDC = SaveDC(hdc);
			int base = (r.bottom < obj->ho-4) ? r.bottom : obj->ho-4;
			int offTxt=0,offClip=0;
			int boxWidth = (r.right-r.left-2+1); //-scrollBarWidth;

			for(int k = 0; k < len; k++)
			{
				SetTextColor(pdis->hDC,fgColor);
				if(tchBuffer[k] == '|')
				{
					if(colNr > 0)
				      offTxt += (int)(boxWidth*colWidth[colNr-1]);
				   offClip += (int)(boxWidth*colWidth[colNr]);
					item[p] = '\0';
					HRGN hRgn = CreateRectRgn(r.left, r.top, r.left+offClip-1, base); 
               SelectClipRgn(hdc, hRgn); 
				   if((pic != 255) && (k == 0))
						TextOut(pdis->hDC, xoff+7+offTxt, y, item, strlen(item));
					else
						TextOut(pdis->hDC, xoff+5+offTxt, y, item, strlen(item));
					DeleteObject(hRgn);
					colNr++;
					if(colNr == info->nrColumns)
						break;
					p = 0;
				}
				else
				{
					item[p++] = tchBuffer[k];
				}
			}
			// Print last text entry
			if(colNr < info->nrColumns)
			{
				item[p] = '\0';
				if(colNr > 0)
					offTxt += (int)(boxWidth*colWidth[colNr-1]);
				offClip += (int)(boxWidth*colWidth[colNr]);
				HRGN hRgn = CreateRectRgn(r.left, r.top, r.left+offClip-1, base); 
				SelectClipRgn(hdc, hRgn); 
				TextOut(pdis->hDC, xoff+5+offTxt, y, item, strlen(item)); 
				DeleteObject(hRgn);
				delete [] item;
			}

		// Draw the dividers for this row if nrColumns > 1 ---------------------------

			if(info->nrColumns > 1)
			{
				HRGN hRgn = CreateRectRgn(r.left, r.top, r.left+r.right-1, obj->ho-4); 
				SelectClipRgn(hdc, hRgn); 
				COLORREF col;
				if(pdis->itemID == 0)
				   col = GetSysColor(COLOR_BTNSHADOW);
				else
					col = RGB(230,230,230);

				HPEN pen = CreatePen(PS_SOLID,0,col);  
				SelectObject(hdc, pen); 
            if(pic != 255)
			   {
				   MoveToEx(hdc,r.left+2,r.top,NULL);
					if(pdis->itemID == nrItems-1) // Last entry
				   	LineTo(hdc,r.left+2,obj->ho-4);
					else
					   LineTo(hdc,r.left+2,r.bottom);
			   }
				int divPos = 0;
				for(int k = 1; k < info->nrColumns; k++)
				{
					divPos += (int)(boxWidth*colWidth[k-1]);
					MoveToEx(hdc,r.left+divPos,r.top,NULL);
					if(pdis->itemID == nrItems-1) // Last entry
						LineTo(hdc,r.left+divPos,obj->ho-4);
					else
						LineTo(hdc,r.left+divPos,r.bottom);
				}
			   DeleteObject(pen);
				DeleteObject(hRgn);
			}
			RestoreDC(hdc,savedDC);
			if(info->colWidth == NULL && colWidth != NULL)
				delete [] colWidth;
         if(pic != 255)
         {
            SelectObject(hdcMem, hbmpOld); 
            DeleteDC(hdcMem); 
         }

         return(1);
      }
// Draw the focus rectangle for this row if it has focus
      case ODA_FOCUS: 
      {
			if(pdis->itemID != 0)
			{
				if(listBoxUpdated)
				{
					listBoxUpdated = false;
				}
				else
				{
					if(pic == 255)
						xoff = xoff_noicon;
					else
						xoff = xoff_icon+4;
					RECT r;
					r.left = pdis->rcItem.left+xoff; 
					r.top = pdis->rcItem.top; 
					r.right = pdis->rcItem.right; 
					r.bottom = pdis->rcItem.bottom; 
						//	TextMessage("list redraw focus %d\n",pdis->itemID);

					if(info->firstLineSelected == info->lastLineSelected)
					   DrawFocusRect(pdis->hDC, &r);
				}
			}
         return(1); 
      }
   }
   return(1); //<-- Correct?
}


/***************************************************************************
   Event procedure for listboxes

   There is the need here for a state machine to account for the fact that
   Prospa is multi-threaded and while one event is being processed another
   may occur. This means that the double click event may occur before the
   single click event is finished. I solve this by storing the events in a
   stack and servicing this when time is available.

***************************************************************************/

#define LST_EVENT_NONE            0
#define LST_EVENT_SGL_CLICK       1
#define LST_EVENT_SGL_UP          2
#define LST_EVENT_DBL_CLICK       3
#define LST_EVENT_DBL_UP          4
//#define LST_EVENT_RGT_CLICK       5
//#define LST_EVENT_RGT_UP          6

#define MAX_LIST_EVENT_STACK      4 

static short mouse_state[MAX_LIST_EVENT_STACK];
static short state_index = 0;

HCURSOR SetMyCursor(HCURSOR cursor)
{
	HCURSOR tmpCur = LoadCursor(NULL,IDC_ARROW);
	if(tmpCur == cursor)
		TextMessage("Arrow cursor\n");
	
	return(SetCursor(cursor));
}

static void ProcessListEventStack(WinData *win, ObjectData *obj);

LRESULT CALLBACK ListBoxEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   LRESULT r = 0;
   WinData *win = 0;
   ObjectData *obj = 0;
   ObjectData *nextObj = 0;
   static short state = LST_EVENT_NONE; // This is used to determine when a dbl click up event occurs
	static bool inDivider = false;
	static int colNr = -1;
	extern HCURSOR VertDivCursor;

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
      return(CallWindowProc(OldListBoxProc,hWnd, messg, wParam, lParam));

   ListBoxInfo* info = (ListBoxInfo*)obj->data;

   switch(messg)
	{
      case(WM_DROPFILES): // User has dropped a folder onto the CLI - just print out the foldername
      {
         CText path,file,ext;
         if(GetDropFileInfo((HDROP)wParam,path, file, ext,0) == OK)
         {
            DragFinish((HDROP)wParam);
         // Is it a folder?
            if(file == "")
            {
               if(SetCurrentDirectory(path.Str()))
	               strncpy_s(gCurrentDir,MAX_PATH,path.Str(),_TRUNCATE); 
	            TextMessage("\n\n  getcwd() = %s\n\n> ",path.Str());
            }
         }
         return(0);
      }

		case(WM_SETCURSOR): 
		{
			if(inDivider) // Ignore cursor updates if in divider to prevent cursor flicker
			   return(1);
			break;
		}

		case(WM_MOUSEMOVE): // As the mouse is moved show a different cursor over the column lines
		{
			ObjectData *obj = GetObjData(hWnd); 

	  // Note location of cursor in client coordinates
         int x = LOWORD(lParam);  // horizontal position of cursor 
         int y = HIWORD(lParam);  // vertical position of cursor 

			if(obj)
			{
				ListBoxInfo* info = (ListBoxInfo*)obj->data;

				if(!info->colWidth)
					return(1);

				SCROLLBARINFO si;
				si.cbSize = sizeof(SCROLLBARINFO);
				GetScrollBarInfo(obj->hWnd, OBJID_VSCROLL, &si);
				int isScrollBar = !(si.rgstate[0] & 0x8000);
				int scrollBarWidth = (isScrollBar)*GetSystemMetrics(SM_CXVSCROLL);
				RECT r;
				GetWindowRect(hWnd,&r);
				int divPos;
				int minColumns;

				if(info->hasIcons)
				{
					minColumns = 1;
					divPos = 20;
				}
				else
				{
					minColumns = 0;
					divPos = 0;
				}

				int boxWidth = (r.right-r.left+1)-scrollBarWidth-divPos-2;
				int rightLimit = (r.right-r.left+1)-scrollBarWidth;
				int leftLimit = divPos;


				if((wParam & MK_LBUTTON) && inDivider && (info->nrColumns > minColumns)) // Drag the divider
				{
					// Get the positions of all current column lines
					int* colPos = new int[info->nrColumns];
					for(int i = 0; i < info->nrColumns; i++)
					{
						colPos[i] = divPos;
						divPos += boxWidth*info->colWidth[i];
					}
					// Make sure we don't move a divider past adjacent ones
					bool canMove = false;
					if(info->nrColumns > 2)
					{
						if(colNr == 1)
						{
							if(x > leftLimit+5 && x < colPos[2]-5)
							{
								float s = 0;
								for(int i = 0; i < info->nrColumns; i++)
								{
									if(i != colNr && i!= colNr-1)
										s = s + info->colWidth[i];
								}
								info->colWidth[colNr-1] = (x-leftLimit)/(float)boxWidth;
								info->colWidth[colNr] = 1-s-info->colWidth[colNr-1];
								canMove = true;
							}
						}
						else if(colNr == info->nrColumns-1)
						{
							if(x > colPos[info->nrColumns-2]+5 && x < rightLimit-5)
							{
								float s = 0;
								for(int i = 0; i < info->nrColumns; i++)
								{
									if(i != colNr && i!= colNr-1)
										s = s + info->colWidth[i];
								}
								info->colWidth[colNr-1] = (x-colPos[colNr-1])/(float)boxWidth;
								info->colWidth[colNr] = 1-s-info->colWidth[colNr-1];
								canMove = true;
							}
						}
						else if(colNr > 0)
						{
							if(x > colPos[colNr-1]+5 &&  x < colPos[colNr+1]-5)
							{
								float s = 0;
								for(int i = 0; i < info->nrColumns; i++)
								{
									if(i != colNr && i!= colNr-1)
										s = s + info->colWidth[i];
								}
								info->colWidth[colNr-1] = (x-colPos[colNr-1])/(float)boxWidth;
								info->colWidth[colNr] = 1-s-info->colWidth[colNr-1];
								canMove = true;
							}
						}
					}
					else
					{
						if(colNr == 1)
						{
							if(x > leftLimit+5 && x < rightLimit-5)
							{
								info->colWidth[0] = (x-leftLimit)/(float)boxWidth;
								info->colWidth[1] = 1-info->colWidth[0];
								canMove = true;
							}
						}

					}
					// Move the divider position
					if(canMove)
					{
						InvalidateRect(hWnd,NULL,false);
					}
				}
				else
				{
					inDivider = false;

					if(info->nrColumns > 1)
					{
						int start = 0;
						if(info->hasIcons)
							start = 1;
						
						for(int i = 0; i < info->nrColumns; i++)
						{
							if(abs(x-divPos+2) < 4 && i >= start)
							{
								inDivider = true;
								colNr = i;
							}
						   divPos += boxWidth*info->colWidth[i];
						}
						if(inDivider)
							SetCursor(VertDivCursor);
						else
							SetCursor(LoadCursor(NULL,IDC_ARROW));
					}
				}
			}
			return(1);
		}
      case(WM_PAINT):
      {
         if(obj)
         {
            r = CallWindowProc(OldListBoxProc,hWnd, messg, wParam, lParam);
            if(obj->selected_) // Draw a selection rectangle around the complete control
            {
               HDC hdc = GetDC(obj->hwndParent);
               obj->DrawSelectRect(hdc);
               ReleaseDC(obj->hwndParent,hdc);
            }
            if(win->displayObjCtrlNrs) // Display the objects control number
            {
               HDC hdc = GetDC(obj->hwndParent);
               obj->DrawControlNumber(hdc);
               ReleaseDC(obj->hwndParent,hdc);
            }
            if(win->displayObjTabNrs) // Display the objects tab number
            {
               HDC hdc = GetDC(obj->hwndParent);
               obj->DrawTabNumber(hdc);
               ReleaseDC(obj->hwndParent,hdc);
            }
            return(r);
         }
      }
      case(WM_SETFOCUS): // Listbox has gained or lost keyboard focus
      {                  // Make sure object with previous focus has focus rect removed
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
      case(WM_LBUTTONDOWN): // Left click down
      {
			// Get mouse vertical position
         short x = LOWORD(lParam); 
         short y = HIWORD(lParam); 
			int idx;

         if(wParam & MK_CONTROL)  // User requests control info
         {
            DisplayControlInfo(win,obj);
            return(0);
         }
         else if(wParam & MK_SHIFT)  // Multiple lines selected
			{
            if(info->allowMultipleSelections)
				{
               LPARAM lParam = MAKELPARAM(x, y);
					idx = SendMessage(obj->hWnd,LB_ITEMFROMPOINT,WPARAM(0),lParam);
					if(info->firstLineSelected > 0) // Selection with shift down so extend range
					{
						info->lastLineSelected = idx;
						InvalidateRect(obj->hWnd,NULL,false);
					}
					else // First press is with shift down so initialise
					{
						info->firstLineSelected = idx;
						info->lastLineSelected = idx;
					}
				}
				else
				{
					info->firstLineSelected = -1;
					info->lastLineSelected = -1;
				}
			}
			else
			{
            if(info->allowMultipleSelections) // First selection - shift not down so intialise range
				{
               LPARAM lParam = MAKELPARAM(x, y);
					idx = SendMessage(obj->hWnd,LB_ITEMFROMPOINT,WPARAM(0),lParam);
					info->firstLineSelected = idx;
					info->lastLineSelected = idx;
					InvalidateRect(obj->hWnd,NULL,false);
				}
				else
				{
					info->firstLineSelected = -1;
					info->lastLineSelected = -1;
				}
			}

         state = LST_EVENT_SGL_CLICK;
         mouse_state[state_index] = LST_EVENT_SGL_CLICK;
         if(!inDivider)
			{
				r = CallWindowProc(OldListBoxProc,hWnd, messg, wParam, lParam);
				ProcessListEventStack(win,obj);
			}

         return(r);
      }
      case(WM_LBUTTONUP): // Left click up (single or double)
      {
         if(wParam & MK_CONTROL)  // Ignore control info request
         {
            return(0);
         }
         if(state == LST_EVENT_DBL_CLICK)
         {
            mouse_state[state_index] = LST_EVENT_DBL_UP;
        // printf("double click up\n");
         }
         else
         {
            mouse_state[state_index] = LST_EVENT_SGL_UP;
        // printf("single click up\n");
         }

         r = CallWindowProc(OldListBoxProc,hWnd, messg, wParam, lParam);
         ProcessListEventStack(win,obj);
         return(r);
      } 
      case(WM_LBUTTONDBLCLK): // Left button double click
      {
         if(wParam & MK_CONTROL) // User requests control info
         {
            DisplayControlInfo(win,obj);
            return(0);
         }
         mouse_state[state_index] = LST_EVENT_DBL_CLICK;
         state = LST_EVENT_DBL_CLICK;
     //  printf("double click down\n");
         r = CallWindowProc(OldListBoxProc,hWnd, messg, wParam, lParam);
         ProcessListEventStack(win,obj);
         return(r);
      }
      case(WM_KEYDOWN): // Keyboard has been pressed 
      {
         char  code = (CHAR)wParam;    // character code 

         if(code == VK_RETURN) // Check for return
         { 
            char *str;
            char proc[MAX_STR];

         // Get the window and object class instances for this command

		      str = win->GetObjCommand(obj->nr());
		      if(str[0] != '\0')
		      {
               strncpy_s(obj->cb_event,MAX_NAME,"enter_pressed",_TRUNCATE);
		         sprintf(proc,"listbox(%d,%d)",win->nr,obj->nr());
		         ProcessMacroStr(LOCAL,win,obj,str,"",proc, win->macroName, win->macroPath);
		      }
            return(0);               
         }  
         if(code == VK_ESCAPE) // Check for escape 
         { 
            SendMessage(win->hWnd,WM_KEYDOWN,(WPARAM)VK_ESCAPE,0); // Send escape to parent window
            return(0);               
         } 
         if(code == VK_TAB  && !IsKeyDown(VK_CONTROL)) // Check for tab - if found move to the next/last tabbable object
         { 
            obj->processTab();
            return(0);
         }
         if(code == VK_DOWN) // Check for arrow
         { 
            char *str;
            char proc[MAX_STR];

            r = CallWindowProc(OldListBoxProc,hWnd, messg, wParam, lParam);

		      str = win->GetObjCommand(obj->nr());
		      if(str[0] != '\0')
		      {
               strncpy_s(obj->cb_event,MAX_NAME,"down_arrow",_TRUNCATE);
		         sprintf(proc,"listbox(%d,%d)",win->nr,obj->nr());
		         ProcessMacroStr(LOCAL,win,obj,str,"",proc, win->macroName, win->macroPath);
               if(obj->keepFocus)
                  SetFocus(obj->hWnd); // The macro should do this if required
		      }
            return(r);
         }
         if(code == VK_UP) // Check for arrow
         { 
            char *str;
            char proc[MAX_STR];

				// Prevent moving up when the top row is a column title
				ListBoxInfo* info = (ListBoxInfo*)obj->data;
				if(info->colWidth && lastPosition == 1)
					return(1);

            r = CallWindowProc(OldListBoxProc,hWnd, messg, wParam, lParam);

		      str = win->GetObjCommand(obj->nr());
		      if(str[0] != '\0')
		      {
               strncpy_s(obj->cb_event,MAX_NAME,"up_arrow",_TRUNCATE);
		         sprintf(proc,"listbox(%d,%d)",win->nr,obj->nr());
		         ProcessMacroStr(LOCAL,win,obj,str,"",proc, win->macroName, win->macroPath);
               if(obj->keepFocus)
                  SetFocus(obj->hWnd); // The macro should do this if required
		      }
            return(r);
         }
         break;
      } 
	   case(WM_RBUTTONDOWN): // Select contextual menu
		{
			POINT p;        
         p.x = LOWORD(lParam);
         p.y = HIWORD(lParam); 

			//state = LST_EVENT_RGT_CLICK;
   //      mouse_state[state_index] = LST_EVENT_RGT_CLICK;
   //      r = CallWindowProc(OldListBoxProc,hWnd, messg, wParam, lParam);

			if(obj->data)
			{
				ListBoxInfo* info = (ListBoxInfo*)obj->data;
				if(info->menu)
				{
					HMENU hMenu;
					short item;	
					hMenu = info->menu;

               ClientToScreen(hWnd,&p);
               item = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, 0, hWnd, NULL) ;
               if(item)
                  PreProcessCommand(NULL, item, hWnd, win);
				}
			}

       //  ProcessListEventStack(win,obj);
			break;
		}

	 //  case(WM_RBUTTONUP): 
		//{
  //       mouse_state[state_index] = LST_EVENT_RGT_UP;
		//	state = LST_EVENT_RGT_UP;
  //       r = CallWindowProc(OldListBoxProc,hWnd, messg, wParam, lParam);
  //       ProcessListEventStack(win,obj);
  //       return(r);
		//}
   }
  
   return(CallWindowProc(OldListBoxProc,hWnd, messg, wParam, lParam));
            
}

// Not currently used.
void ListBoxCallBack(WinData *win, ObjectData *obj)
{
   char proc[MAX_STR];
   char *cmd = win->GetObjCommand(obj->nr());
   if(cmd[0] != '\0')
   {
	   sprintf(proc,"listbox(%d,%d)",win->nr,obj->nr());
	   ProcessMacroStr(LOCAL,win,obj,cmd, "", proc, win->macroName, win->macroPath);
   }
}


/***************************************************************************
   Process any button list events which have been added to stack.
   Do not allow more than 4 events to accumulate since this
   corresponds to 1 event for each part of a double click
***************************************************************************/

void ProcessListEventStack(WinData *win, ObjectData *obj)
{
	char *str = win->GetObjCommand(obj->nr());
   char proc[MAX_STR];
   short index;

// Only accept valid states
   if(state_index > 0 && mouse_state[state_index] != mouse_state[state_index-1]+1)
      return;

// Increment the state stack
   if(state_index < MAX_LIST_EVENT_STACK-1)
      state_index++;

// Do nothing if there is a macro running already
   //if(macroDepth != 0)
   //{
   //   return;
   //}

// Work through all the stack events
   for(index = 0; index < state_index; index++)
   {
      switch(mouse_state[index])
      {
         case(LST_EVENT_SGL_CLICK):
            strncpy_s(obj->cb_event,MAX_NAME,"single_click_down",_TRUNCATE);
            break;
         case(LST_EVENT_SGL_UP):
            strncpy_s(obj->cb_event,MAX_NAME,"single_click_up",_TRUNCATE);
            break;
         case(LST_EVENT_DBL_CLICK):
            strncpy_s(obj->cb_event,MAX_NAME,"double_click_down",_TRUNCATE);
            break;
         case(LST_EVENT_DBL_UP):
            strncpy_s(obj->cb_event,MAX_NAME,"double_click_up",_TRUNCATE);
            break;  
         //case(LST_EVENT_RGT_CLICK):
         //   strncpy_s(obj->cb_event,MAX_NAME,"right_click_down",_TRUNCATE);
         //   break;
      }

		if(str[0] != '\0')
		{
         extern bool gKeepCurrentFocus;
		   sprintf(proc,"listbox(%d,%d)",win->nr,obj->nr());
      //   bool oldState = gKeepCurrentFocus;
       //  gKeepCurrentFocus = true;
		   ProcessMacroStr(LOCAL,win,obj,str,"",proc, win->macroName, win->macroPath);
       //  gKeepCurrentFocus = oldState;
         if(obj->keepFocus)
            SetFocus(obj->hWnd); // The macro should do this if required
		}
   }
   state_index = 0;
}


/*******************************************************************************
* Modify the parameters for the list box
*
* Command syntax is setpar(win,obj,parameter,value)
*
* parameter         value
*  "list"        1D matrix or list
*  "text"        string
*  "index"       number >= 1
*  "zindex"      number >= 0
*
*******************************************************************************/


int SetListBoxParameter(ObjectData *obj, CText &parameter, Variable *value)
{
   char str[50];
   short type = value->GetType();

   if(parameter == "list") // Modify the contents of the listbox
   {
	   if(type == MATRIX2D) // Apply the contents of a 1D matrix to a menu
	   {
	      short rows = value->GetDimY();
	      short cols = value->GetDimX();
         float **mat = value->GetMatrix2D();

			short n;
	      do
	      {
            n = SendMessage(obj->hWnd, LB_DELETESTRING, 0, 0);
         }
         while(n > 0);
	               
	      if(rows == 1)
	      {
	         for(short i = 0; i < cols; i++)
	         {
	            if(mat[0][i] == nint(mat[0][i]))
	               sprintf(str,"%ld",nint(mat[0][i]));
	            else
	               sprintf(str,"%g",mat[0][i]);

               SendMessage(obj->hWnd,LB_ADDSTRING, 0, (LPARAM) str);
               SendMessage(obj->hWnd, LB_SETITEMDATA, (LPARAM)i, (WPARAM)255);
            }
         }
         else
         {
            ErrorMessage("Can only apply a 1D matrix to a menu");
            return(ERR);
         }
       }
       else if(type == LIST)	// Apply the contents of a text list
       {
			 bool enable = obj->enable;
          if(!enable)
             EnableWindow(obj->hWnd,true); // Can't update list correctly if it is disabled

			 short n;
	       do // Delete current menu
	       {
             n = SendMessage(obj->hWnd, LB_DELETESTRING, 0, 0);
          }
          while(n > 0);
            
          for(short i = 0; i < value->GetDimX(); i++) // Add new one from list
          {
             SendMessage(obj->hWnd, LB_ADDSTRING, 0, (LPARAM) value->GetList()[i]);
             SendMessage(obj->hWnd, LB_SETITEMDATA, (LPARAM)i, (WPARAM)255); // No icon
          } 
          listBoxUpdated = true; // Need to add this because focus rectangle has been
                                 // drawn already even though listbox hasn't got focus yet
          if(!enable)
             EnableWindow(obj->hWnd,false);
          SetFocus(obj->hWnd);  
          return(OK);

       }            
	}
   else if(parameter == "multilineselection") // Are multiline spections allowed
	{
		if(type == UNQUOTED_STRING)
		{
			ListBoxInfo* info = (ListBoxInfo*)obj->data;
			CText result = value->GetString();
			if(result == "true")
				info->allowMultipleSelections = true;
			else
				info->allowMultipleSelections = false;
			info->firstLineSelected = -1;
			info->lastLineSelected = -1;
			InvalidateRect(obj->hWnd,NULL,false);
		}
      else
      {
         ErrorMessage("invalid data type for multilineselection parameter");
         return(ERR);
      } 
	}
   else if(parameter == "topindex")
   {
      if(type == FLOAT32)
      {
         int pos = nint(value->GetReal())-1;
         SendMessage(obj->hWnd,LB_SETTOPINDEX,pos,0);
      }
      else
      {
         ErrorMessage("invalid data type for scrollto parameter");
         return(ERR);
      }   
   }
   else if(parameter == "topzindex")
   {
      if(type == FLOAT32)
      {
         int pos = nint(value->GetReal());
         SendMessage(obj->hWnd,LB_SETTOPINDEX,pos,0);
      }
      else
      {
         ErrorMessage("invalid data type for topzindex parameter");
         return(ERR);
      }   
   }
	else if(parameter == "selection")
	{
		if(type == MATRIX2D)
		{
			if(value->GetDimY() == 1)
			{
				ListBoxInfo* info = (ListBoxInfo*)obj->data;
				float **selection = value->GetMatrix2D();
				info->firstLineSelected = selection[0][0];
				info->lastLineSelected = selection[0][1];
				MyInvalidateRect(obj->hWnd,NULL,true);
			}
			else
			{
				ErrorMessage("Selection argument should be a 2 element vector");
				return(ERR);
			}
		}
		else
		{
			ErrorMessage("Selection argument should be a 2 element vector");
			return(ERR);
		}
	}



   else if(parameter == "color" || parameter == "fgcolor") // Either one or all items
   {
		int nrRows = SendMessage(obj->hWnd,LB_GETCOUNT,0,0);
		if(type == MATRIX2D) // Apply the contents of a 1D matrix to a menu
	   {
	      short width = value->GetDimX();
	      short height = value->GetDimY();
			if(height == 1 && width == 4)
			{
				float **mat = value->GetMatrix2D();
				int index = mat[0][0];
				if(index < 0 || index >= nrRows)
				{
					ErrorMessage("invalid list index");
					return(ERR);
				} 
				long r = nint(mat[0][1]);
				long g = nint(mat[0][2]);
				long b = nint(mat[0][3]);
				long oldData = (long) SendMessage(obj->hWnd, LB_GETITEMDATA, index, (LPARAM) 0);
			   int value = (r<<24) + (g<<16) + (b<<8) + (oldData&0xFF);
            SendMessage(obj->hWnd, LB_SETITEMDATA, index, value);
				MyInvalidateRect(obj->hWnd,NULL,true);
			}
			else if(height == nrRows && width == 4) // WORK ON THIS THURSDAY
			{
				float **mat = value->GetMatrix2D();
				int index = mat[0][0];
				if(index < 0 || index >= nrRows)
				{
					ErrorMessage("invalid list index");
					return(ERR);
				} 
				long r = nint(mat[0][1]);
				long g = nint(mat[0][2]);
				long b = nint(mat[0][3]);
				long oldData = (long) SendMessage(obj->hWnd, LB_GETITEMDATA, index, (LPARAM) 0);
			   int value = (r<<24) + (g<<16) + (b<<8) + (oldData&0xFF);
            SendMessage(obj->hWnd, LB_SETITEMDATA, index, value);
				MyInvalidateRect(obj->hWnd,NULL,true);
			}
			else
			{
				ErrorMessage("invalid dimension - should be 4 column vector (idx, r, g, b)");
				return(ERR);
			} 
		}
      else
      {
         ErrorMessage("invalid data type - should be 4 column vector (idx, r, g, b)");
         return(ERR);
      } 
      
   }
   else if(parameter == "bgcolor" || parameter == "bkgcolor")
   {
      if(SetColor(obj->bgColor, value) == ERR)
         return(ERR);
   }
   else if(parameter == "icons") // Modify the listbox icons
   {
	   if(type == MATRIX2D) // Apply the contents of a 1D matrix to a menu
	   {
	      short width = value->GetDimX();
	      short height = value->GetDimY();
	      float **mat = value->GetMatrix2D();
	               
	      if(height == 1)
	      {
         // Check for correct number of entries
            if(SendMessage(obj->hWnd,LB_GETCOUNT,0,0) != width)
            {
               ErrorMessage("list box has different number of entries from icon list");
               return(ERR);
            }

         // Add icon indices to user data item
	         for(short i = 0; i < width; i++)
	         {
					long index;
	            if(mat[0][i] == nint(mat[0][i]))
               {
                  index = nint(mat[0][i]);
               }
	            else
               {
                  ErrorMessage("icon index should be an integer");
                  return(ERR);
               }

               if(index != -1 && ((index&0xFF) < -1 || (index&0xFF) > 25))
               {
                  ErrorMessage("invalid icon index (-1 to 25)");
                  return(ERR);
               }
               SendMessage(obj->hWnd, LB_SETITEMDATA, i, index);
 
            }
            MyInvalidateRect(obj->hWnd,NULL,true); // Make sure list is redrawn
         }
         else
         {
            ErrorMessage("Can only use a 1D matrix to extract icon indices");
            return(ERR);
         }
       }
       else if(type == LIST)	// Apply the contents of a text list
       {
			 char iconNames[][22] = {"folder","prospa_file","button","colorbox","checkbox","groupbox","listbox","progressbar",
											 "radiobutton","slider","statictext","statusbox","textbox","textmenu","htmlbox","updown",
											 "plot1d","plot2d","plot3d","cli","editor","book","page","","","","divider","picture","tab","file",
											 "link","grid","blank","full_ucs","full_ucs1","full_ucs2","full_ucs3","full_ucs4","full_ucs5","center_ucs","spinsolve"};

       // Check for correct number of entries
          if(SendMessage(obj->hWnd,LB_GETCOUNT,0,0) != value->GetDimX())
          {
             ErrorMessage("list box has different number of entries from icon list");
             return(ERR);
          }

       // Add icon indices to user data item
          for(short i = 0; i < value->GetDimX(); i++) 
          {
             char *txt = value->GetList()[i];
			    long oldData = (long) SendMessage(obj->hWnd, LB_GETITEMDATA, i, (LPARAM) 0);
				 
				 int sz = sizeof(iconNames);
				 int j;
				 for(j = 0; j < sz; j++)
				 {
					 if(!strcmp(iconNames[j],txt))
					 {
						long newData = oldData&0xFFFFFF00 + j;
                  SendMessage(obj->hWnd, LB_SETITEMDATA, i, newData);
						break;
					 }
				 }
				 if(j == sz)
				 {
					  long newData = oldData&0xFFFFFF00 + 32;
					  SendMessage(obj->hWnd, LB_SETITEMDATA, i, newData);
				 }

          } 
          MyInvalidateRect(obj->hWnd,NULL,true); // Make sure list is redrawn
       }            
	}
	else if(parameter == "text") // Select the list entry by text
	{
	   short index;
	   if(type == UNQUOTED_STRING)
	   {
         index = SendMessage(obj->hWnd, LB_FINDSTRINGEXACT, -1, (LPARAM)value->GetString());
         index = SendMessage(obj->hWnd, LB_SETCURSEL, index, 0);        
         if(index == LB_ERR)
         {
           ErrorMessage("invalid text value");
           return(ERR);
         } 
      }
      else if(type == FLOAT32)
      {
	      sprintf(str,"%g",value->GetReal());      
         index = SendMessage(obj->hWnd, LB_FINDSTRINGEXACT, -1, (LPARAM)str);
         index = SendMessage(obj->hWnd, LB_SETCURSEL, index, 0);        
         if(index == LB_ERR)
         {
           ErrorMessage("invalid text value");
           return(ERR);
         } 
      }	      
	}
	else if(parameter == "index") // Select the one referenced list entry by index
	{
	   if(type == FLOAT32)
	   {
	      short index;
         index = nsint(value->GetReal())-1;
         index = SendMessage(obj->hWnd, LB_SETCURSEL, index, 0);  
         if(index == LB_ERR)
         {
           ErrorMessage("invalid index value");
           return(ERR);
         } 
			ListBoxInfo* info = (ListBoxInfo*)obj->data;
			info->firstLineSelected = index;
			info->lastLineSelected = index;
	   } 
      else
      {
         ErrorMessage("invalid data type for 'index' parameter");
         return(ERR);
      }    
	}
	else if(parameter == "zindex") // Select the zero referenced list entry by index
	{
	   if(type == FLOAT32)
	   {
	      short index;
         index = nsint(value->GetReal());
         index = SendMessage(obj->hWnd, LB_SETCURSEL, index, 0);        
         if(index == LB_ERR)
         {
           ErrorMessage("invalid zindex value");
           return(ERR);
         }  
			ListBoxInfo* info = (ListBoxInfo*)obj->data;
			info->firstLineSelected = index;
			info->lastLineSelected = index;
	   } 
      else
      {
         ErrorMessage("invalid data type for 'index' parameter");
         return(ERR);
      }    
	}	
	else if(parameter == "nrcolumns") // Specify the number of columns
	{
		ListBoxInfo* info = (ListBoxInfo*)obj->data;
		int nrCols = nsint(value->GetReal());
	   if(nrCols < 1 || nrCols > 100)
      {
         ErrorMessage("invalid number of columns (1-100)");
         return(ERR);
      } 
		info->colWidth = NULL;
		info->nrColumns = nrCols;
	}
	else if(parameter == "colwidth") // Specify the width of each column
	{
	   if(type == MATRIX2D) // Apply the contents of a 1D matrix to a menu
	   {
			ListBoxInfo* info = (ListBoxInfo*)obj->data;
	      short width = value->GetDimX();
	      short height = value->GetDimY();
			if(height == 1 && width == info->nrColumns)
			{
				float **mat = value->GetMatrix2D();
				info->colWidth = new float[width];
				for(int i = 0; i < width; i++)
				{
					info->colWidth[i] = mat[0][i];
				}
			}
			else
			{
				ErrorMessage("value should be a 1D matrix with %d entries",info->nrColumns);
				return(ERR);
			} 
		}
		else
      {
         ErrorMessage("value should be a 1D matrix");
         return(ERR);
      } 
	}
	else if(parameter == "menu") // Specify the contextual menus
	{
	   WinData *win = rootWin->FindWinByHWND(obj->hwndParent);

      if(value->GetType() == FLOAT32)
      {
         short objNr = value->GetReal();
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
			ListBoxInfo* listInfo = (ListBoxInfo*)obj->data;
         if(parameter == "menu")
         {
            listInfo->menu = info->menu;
            listInfo->menuNr = objNr;
         }

      // Get the exisiting accelerator table for the plot object
         ACCEL *accelOld = NULL,*accelNew = NULL,*accelMenu = NULL;
         int j,k,nMenu = 0,nOld = 0;
      // Extract old table
         if(obj->accelTable)
         {
            nOld = CopyAcceleratorTable(obj->accelTable,NULL,0);
            accelOld = new ACCEL[nOld];
            CopyAcceleratorTable(obj->accelTable,(LPACCEL)accelOld,nOld);
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
         if(obj->accelTable)
            DestroyAcceleratorTable(obj->accelTable);
      // Update the accelerator table
         obj->accelTable = CreateAcceleratorTable((LPACCEL)accelNew,k);
         if(accelNew)
            delete [] accelNew;
         if(accelOld)
            delete [] accelOld;
         if(accelMenu)
            delete [] accelMenu;
      }
      else
      {
         ErrorMessage("invalid parameter type");
         return(ERR);
      }
	}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}

 	return(OK);
}


/*******************************************************************************
* Extract the parameters for a listbox 
*
* Command syntax is value = getpar(win,obj,parameter)
*
* parameter  
*  "text" .... return the selected text unchanged
*  "value" ... evaluate the selected text first before returning
*  "index" ... extract the current text (1 based) string as a list
*  "zindex" .. extract the current text (0 based) string as a list
*
*******************************************************************************/
//TODO check number returnedvalues
int GetListBoxParameter(Interface *itfc, ObjectData *obj, CText &parameter, Variable *ans)
{
   HWND hWnd;
   long length;
   short index;
   
   hWnd = obj->hWnd;
   
   if(parameter == "text") // Return text verbatim
   {
      index = SendMessage(obj->hWnd,LB_GETCURSEL,0,0);
      if(index != LB_ERR)
      {
         length = SendMessage(obj->hWnd,LB_GETTEXTLEN,(WPARAM)index,0);
         char *data = new char[length+1];
         SendMessage(obj->hWnd,LB_GETTEXT,(WPARAM)index,(LPARAM)data);
		   ans->MakeAndSetString(data);
         delete [] data;
      }
      else
      {
		   ans->MakeNullVar();
      }
	}
   else if(parameter == "topindex")
   {
      int pos = SendMessage(obj->hWnd,LB_GETTOPINDEX,0,0);
      ans->MakeAndSetFloat(pos+1);
   }
   else if(parameter == "topzindex")
   {
      int pos = SendMessage(obj->hWnd,LB_GETTOPINDEX,0,0);
      ans->MakeAndSetFloat(pos);
   }
   else if(parameter == "color" || parameter == "fgcolor" )
   {
	   BYTE *data = (BYTE*)&obj->fgColor;
      float colors[3];
      colors[0] = data[0];
      colors[1] = data[1];
      colors[2] = data[2];
      ans->MakeMatrix2DFromVector(colors,3,1);
   }
   else if(parameter == "bkgcolor" || parameter == "bgcolor" )
   {
	   BYTE *data = (BYTE*)&obj->bgColor;
      float colors[3];
      colors[0] = data[0];
      colors[1] = data[1];
      colors[2] = data[2];
      ans->MakeMatrix2DFromVector(colors,3,1);
   }
   else if(parameter == "value") // Evaluate after selecting
   {
      index = SendMessage(obj->hWnd,LB_GETCURSEL,0,0);
      if(index != LB_ERR)
      {
         length = SendMessage(obj->hWnd,LB_GETTEXTLEN,(WPARAM)index,0);
         char *data = new char[length+1];
         SendMessage(obj->hWnd,LB_GETTEXT,(WPARAM)index,(LPARAM)data);
		   ans->MakeAndSetString(data);

	      if(Evaluate(itfc,RESPECT_ALIAS,data,ans) == -1)
         {
            delete [] data;
	         return(ERR);  
         }
         delete [] data;
      }
      else
      {
		   ans->MakeNullVar();
      }
	}	
	else if(parameter == "index") // Get 1 based index of selected text
	{
      index = SendMessage(obj->hWnd,LB_GETCURSEL,0,0);
		ans->MakeAndSetFloat((float)index+1);   
	}
	else if(parameter == "zindex") // Get 0 based index of selected text
	{
      index = SendMessage(obj->hWnd,LB_GETCURSEL,0,0);
		ans->MakeAndSetFloat((float)index);   
	}	
	else if(parameter == "selection") // Return the selection range - keep order
	{
		ListBoxInfo* info = (ListBoxInfo*)obj->data;
		float *selection = new float[2];
		selection[0] = info->firstLineSelected;
		selection[1] = info->lastLineSelected;
		//if(info->firstLineSelected < info->lastLineSelected)
		//{
		//	selection[0] = info->firstLineSelected;
		//	selection[1] = info->lastLineSelected;
		//}
		//else
		//{
		//	selection[1] = info->firstLineSelected;
		//	selection[0] = info->lastLineSelected;
		//}
      ans->MakeMatrix2DFromVector(selection,2,1);
		delete [] selection; 
	}
   else if(parameter == "multilineselection") // Are multiline spections allowed
	{
		ListBoxInfo* info = (ListBoxInfo*)obj->data;
		if(info->allowMultipleSelections)
			ans->MakeAndSetString("true");
		else
			ans->MakeAndSetString("false");
	}
   else if(parameter == "list") // return string as a list
   {
      long entries = SendMessage(obj->hWnd,LB_GETCOUNT,0,0);
      if(entries > 0)
      {
         char **list = NULL; 
			long i;
         for(i = 0; i < entries; i++)
         {
            length = SendMessage(obj->hWnd,LB_GETTEXTLEN,(WPARAM)i,0);
            char *data = new char[length+1];
            SendMessage(obj->hWnd,LB_GETTEXT,(WPARAM)i,(LPARAM)data);
            AppendStringToList(data,&list,i);
            delete [] data;
         }
	      ans->AssignList(list,i);
      }
      else
      {
		   ans->MakeNullVar();
      }
	}	
   else if(parameter == "icons") // Get the current listbox icons
   {
   // Get the number of entries
      long entries = SendMessage(obj->hWnd,LB_GETCOUNT,0,0);

      if(entries > 0)
      {
         char **list = NULL; 
         CText itemTxt;
			long i;
         for(i = 0; i < entries; i++)
         {
            int itemData = SendMessage(obj->hWnd,LB_GETITEMDATA,(WPARAM)i,(LPARAM)0L);

            switch(itemData)
            {
               case(0):
                  itemTxt = "folder";
                  break;
               case(1):
                  itemTxt = "prospa_file";
                  break;
               case(2):
                  itemTxt = "button";
                  break;
               case(3):
                  itemTxt = "colorbox";
                  break;
               case(4):
                  itemTxt = "checkbox";
                  break;
               case(5):
                  itemTxt = "groupbox";
                  break;
               case(6):
                  itemTxt = "listbox";
                  break;
               case(7):
                  itemTxt = "progressbar";
                  break;
               case(8):
                  itemTxt = "radiobutton";
                  break;
               case(9):
                  itemTxt = "slider";
                  break;
               case(10):
                  itemTxt = "statictext";
                  break;
               case(11):
                  itemTxt = "statusbox";
                  break;
               case(12):
                  itemTxt = "textbox";
                  break;
               case(13):
                  itemTxt = "textmenu";
                  break;
               case(14):
                  itemTxt = "htmlbox";
                  break;
               case(15):
                  itemTxt = "updown";
                  break;
               case(16):
                  itemTxt = "plot1d";
                  break;
               case(17):
                  itemTxt = "plot2d";
                  break;
               case(18):
                  itemTxt = "plot3d";
                  break;
               case(19):
                  itemTxt = "cli";
                  break;
               case(20):
                  itemTxt = "editor";
                  break;
               case(21):
                  itemTxt = "book";
                  break;
               case(22):
                  itemTxt = "page";
                  break;
               case(23):
                  itemTxt = "cli";
                  break;
               case(24):
                  itemTxt = "editor";
                  break;
               case(25):
                  itemTxt = "book";
                  break;
               case(26):
                  itemTxt = "divider";
                  break;
               case(27):
                  itemTxt = "picture";
                  break;
               case(28):
                  itemTxt = "tab";
                  break;
               case(29):
                  itemTxt = "file";
                  break;
               case(30):
                  itemTxt = "link";
                  break;
               case(31):
                  itemTxt = "grid";
                  break;
               case(32):
                  itemTxt = "blank";
                  break;
               case(33):
                  itemTxt = "full_ucs";
                  break;
					case(34):
                  itemTxt = "full_ucs1";
                  break;
               case(35):
                  itemTxt = "full_ucs2";
                  break;
               case(36):
                  itemTxt = "full_ucs3";
                  break;
               case(37):
                  itemTxt = "full_ucs4";
                  break;
               case(38):
                  itemTxt = "full_ucs5";
                  break;
               case(39):
                  itemTxt = "center_ucs";
                  break;
					case(40):
                  itemTxt = "spinsolve";
                  break;
               default:
                  itemTxt = "blank";
                  break;
            }
            AppendStringToList(itemTxt.Str(),&list,i);
         }
	      ans->AssignList(list,i);
      }
      else
      {
		   ans->MakeNullVar();
      }   
	}
	else
	{
	   ErrorMessage("invalid parameter requested");
	   return(ERR);
	}	

   itfc->nrRetValues = 1;
      
 	return(OK);
}
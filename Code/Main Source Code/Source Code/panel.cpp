#include "stdafx.h"
#include "allocate.h"
#include "button.h"
#include "defines.h"
#include "edit_class.h"
#include "events_edit.h"
#include "error.h"
#include "globals.h"
#include "interface.h"
#include "mymath.h"
#include "guiModifyObjectParameters.h"
#include "guiInteractiveWindowBuilder.h"
#include "guiMakeObjects.h"
#include "scanstrings.h"
#include "variablesClass.h"
#include "guiWindowClass.h"
#include "panel.h"
#include "memoryLeak.h"

/******************************************************************
* GUI interface commands
*
* MakePanel
*
* Panel class member-function
*
* SetPanelParent
* UpdatePanelFromScrollBar
* UpdatePanelStruct
* ProcessPanelScrollEvent
*
*******************************************************************/

void ProcessScrollWheelEvent(HWND hWnd, short zDel, short fwKeys);

// Define a new panel

int MakePanel(Interface* itfc ,char args[])
{
   static short nr,x,y,w,h;
   Variable wx,wy,ww,wh;
   short r;
   CText label;
   ObjPos pos;

   if((r = ArgScan(itfc,args,5,"nr,x,y,w,h","eeeee","dvvvv",&nr,&wx,&wy,&ww,&wh)) < 0)
      return(r);

   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }

   if(ProcessObjectPosition(editGUIWindow,&wx,&wy,&ww,&wh,x,y,w,h,&pos) == ERR)
      return(ERR);

// Panel information
   PanelInfo* info = new PanelInfo;
   info->x = x;
   info->y = y;
   info->w = w-20;
   info->h = h;
   info->nrChildren = 0;
   info->hWndPanel = NULL;
   info->maxY = -10000;
   info->minY = 10000;
   info->childInfo = NULL;
   info->yOffset = 0;
   int sz = sizeof(ObjPos);
   memcpy(&info->pos,&pos,sz);

   ObjectData *obj = new ObjectData(editGUIWindow,PANEL,nr,x,y,w,h,"","",(char*)info,g_objVisibility,itfc->lineNr);
	editGUIWindow->widgets.add(obj);

   SaveObjectPosition(obj,&pos); 
   itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
   itfc->nrRetValues = 1;

   return(0);
}

// Make a control part of panel which has control number "nr"
// If nr = -1 then revert to having the main window as parent

short ObjectData::SetPanelParent(short nr)
{
   if(nr == -1)
   {
      if(this->panelParent)
      {
         PanelInfo* pInfo = (PanelInfo*)this->panelParent->data;
         // Restore y position
         WinData *win = this->winParent;
       
         for(int i = 0; i < pInfo->nrChildren; i++)
         {
            if(pInfo->childInfo[0][i] == this->nr())
            {
                this->yo = pInfo->childInfo[1][i];
                this->ySzOffset = yo;
                this->Move(xo,yo,wo,ho, false);
            }
         }

         if(this->type == RADIO_BUTTON)
         {
            RadioButtonInfo* rbInfo = (RadioButtonInfo*)this->data; 
            if(rbInfo)
            {
               for(int i = 0; i < rbInfo->nrBut; i++)
               {
                  SetParent(rbInfo->hWnd[i],this->winParent->hWnd);
               }
            }
         }
         else if(this->type == TEXTEDITOR)
         {
            EditParent* ep = (EditParent*)this->data;
            for(short i = 0; i < ep->rows*ep->cols; i++) 
            {	
              SetParent(ep->editData[i]->edWin,this->winParent->hWnd);  		            
            }	
            SetParent(this->hWnd,this->winParent->hWnd);  		            
         }
         else
            SetParent(this->hWnd,this->winParent->hWnd);
         this->panelParent = NULL;
      }
   }
   else
   {
      ObjectData *panelCtrl = this->winParent->FindObjectByNr(nr);
      if(!panelCtrl || panelCtrl->type != PANEL)
      {
         ErrorMessage("invalid panel parent number");
         return(ERR);
      }
      this->panelParent = panelCtrl;
      PanelInfo *pInfo = (PanelInfo*)panelCtrl->data;
      if(this->type == RADIO_BUTTON)
      {
         RadioButtonInfo* rbInfo = (RadioButtonInfo*)this->data; 
         if(rbInfo)
         {
            for(int i = 0; i < rbInfo->nrBut; i++)
            {
               SetParent(rbInfo->hWnd[i],pInfo->hWndPanel);
            }
         }
      }
      else if(this->type == TEXTEDITOR)
      {
         EditParent* ep = (EditParent*)this->data;
         for(short i = 0; i < ep->rows*ep->cols; i++) 
         {	
           SetParent(ep->editData[i]->edWin,pInfo->hWndPanel);  		            
         }	
         SetParent(this->hWnd,pInfo->hWndPanel);  		            
      }

      else if(this->type == TABCTRL)
      {
         ErrorMessage("tab control can't be part of a panel");
         return(ERR);
      }
      else if(this->type == STATUSBOX)
      {
         ErrorMessage("status box can't be part of a panel");
         return(ERR);
      }
      else
         SetParent(this->hWnd,pInfo->hWndPanel);

   // This fixes checkbox selection in panel
      // but causes a crash when exiting??
   //   hwndParent = pInfo->hWndPanel;
   }

   return(OK);
}


// Scroll all panel children as the scrollbar is adjusted
// Could also use ScrollWindowEx??

short ObjectData::UpdatePanelFromScrollBar(long offset)
{
	static long oldOffset = -1;

   if(oldOffset != offset && offset >= 0) // Ensure scroll bar is not redraw when hidden
      SetScrollPos(hWnd,SB_CTL,offset,true);
	oldOffset = offset;

   PanelInfo* pInfo = (PanelInfo*)data;

   if(pInfo == NULL | pInfo->childInfo == 0 | pInfo->nrChildren == 0)
      return(ERR);

// Record control offset
   pInfo->yOffset = offset;

   WinData *win = this->winParent;

// Offset each control inside the panel
   for(ObjectData* o: win->widgets.getWidgets())
   {
      if(/*o->visible && */o->panelParent && (o->panelParent->nr() == this->nr()))
      {
         for(int i = 0; i < pInfo->nrChildren; i++)
         {
            if(pInfo->childInfo[0][i] == o->nr())
            {
                o->yo = pInfo->childInfo[1][i] - offset;
                o->ySzOffset = o->yo;
                o->Move(o->xo,o->yo,o->wo,o->ho, false);
            }
         }
      }
   }

// Redraw the panel
   MyInvalidateRect(pInfo->hWndPanel,NULL,false);
   MyUpdateWindow(pInfo->hWndPanel);

   return(OK);
}

// Reposition the scrollbar thumb based on panel size

short ObjectData::UpdatePanelThumb(bool recalcObjPos)
{
   SCROLLINFO si;
   PanelInfo* pInfo = (PanelInfo*)data;

   int childH = pInfo->maxY-pInfo->minY;
   int panelH = pInfo->h;

   if(childH < 0) return(OK);

// Move scroll thumb to new position
   si.fMask =  SIF_RANGE | SIF_PAGE | SIF_DISABLENOSCROLL;
   si.nMin = 0;
   si.nMax = childH; 
   si.nPage = panelH;

   SetScrollInfo(this->hWnd, SB_CTL, &si, true);

   if(recalcObjPos)
   {
      int base = pInfo->yOffset + panelH;
      if(base > childH)
      {
         if(childH-panelH < 0)
            UpdatePanelFromScrollBar(0);
         else
            UpdatePanelFromScrollBar(childH-panelH);
      }
   }

   return(OK);
}

// Initialise the panel data structure

short ObjectData::UpdatePanelStruct(short mode)
{
   PanelInfo *pInfo = (PanelInfo*)this->data;
   WinData *win = this->winParent;

   // Count how many objects belong to this panel
   int cnt = 0;
   for(ObjectData* o: win->widgets.getWidgets())
   {
      if(o->panelParent && (o->panelParent->nr() == this->nr()) && (o->visible || mode == 0))
		   cnt++;
   }
   // Save the control numbers and y coordinates of each panel object
   // and work out the y range
   long **childInfo = MakeIMatrix2D(cnt,2);
   int i = 0;
   int minY = 1e6;
   int maxY = -1e6;
   long yt,yb;

   for(ObjectData* o: win->widgets.getWidgets())
   {
      if(o->panelParent && (o->panelParent->nr() == this->nr()) && (o->visible || mode == 0))
      {
         childInfo[0][i] = o->nr();
         yt = o->yo+pInfo->yOffset;
         yb = yt+o->ho;
         childInfo[1][i++] = yt;
         if(yb > maxY) maxY = yb;
         if(yt < minY) minY = yt;
      }
   }
 
   if(pInfo->childInfo) 
      FreeIMatrix2D(pInfo->childInfo);
   pInfo->nrChildren = cnt;
   pInfo->childInfo = childInfo;
   pInfo->maxY = maxY+10; // + (minY-pInfo->y);
   pInfo->minY = 0; //pInfo->y;

   UpdatePanelThumb(true);

   return(OK);
}

// Process panel scroll bar events

short ObjectData::ProcessPanelScrollEvent(WPARAM wParam,LPARAM lParam)
{
   switch(LOWORD(wParam))
   {
      case(SB_THUMBPOSITION):
      case(SB_THUMBTRACK):
      {
         SCROLLINFO si;
         si.cbSize = sizeof(SCROLLINFO);
         si.fMask = SIF_TRACKPOS;
         GetScrollInfo(this->hWnd,SB_CTL, &si);
         int pos = si.nTrackPos;
         SetScrollPos(this->hWnd,SB_CTL,pos,true);
         this->UpdatePanelFromScrollBar(pos);
		   return(0);
	   }	

      case(SB_LINEUP):
      {
         SCROLLINFO si;
         si.cbSize = sizeof(SCROLLINFO);
         si.fMask = SIF_ALL;
         GetScrollInfo(this->hWnd,SB_CTL, &si);
         int pos = si.nPos;
         if(pos > 0)
         {
            SetScrollPos(this->hWnd,SB_CTL,pos-1,true);
            this->UpdatePanelFromScrollBar(pos-1);
         }
         break;
      }
      case(SB_LINEDOWN):
      {
         SCROLLINFO si;
         si.cbSize = sizeof(SCROLLINFO);
         si.fMask = SIF_ALL;
         GetScrollInfo(this->hWnd,SB_CTL, &si);
         int pos = si.nPos;
         if((pos+1) < (si.nMax-(int)si.nPage+2))
         {
            SetScrollPos(this->hWnd,SB_CTL,pos+1,true);
            this->UpdatePanelFromScrollBar(pos+1);
         }
         break;
      }
      case(SB_PAGEUP):
      {
         SCROLLINFO si;
         si.cbSize = sizeof(SCROLLINFO);
         si.fMask = SIF_ALL;
         GetScrollInfo(this->hWnd,SB_CTL, &si);
         int pos = si.nPos;
         if((pos-(int)si.nPage) > 0)
         {
            SetScrollPos(this->hWnd,SB_CTL,pos-si.nPage,true);
            this->UpdatePanelFromScrollBar(pos-si.nPage);
         }
         else
         {
            SetScrollPos(this->hWnd,SB_CTL,0,true);
            this->UpdatePanelFromScrollBar(0);
         }
         break;
      }
      case(SB_PAGEDOWN):
      {
         SCROLLINFO si;
         si.cbSize = sizeof(SCROLLINFO);
         si.fMask = SIF_ALL;
         GetScrollInfo(this->hWnd,SB_CTL, &si);
         int pos = si.nPos;
         if((pos+(int)si.nPage) < (si.nMax-(int)si.nPage+2))
         {
            SetScrollPos(this->hWnd,SB_CTL,pos+si.nPage,true);
            this->UpdatePanelFromScrollBar(pos+si.nPage);
         }
         else
         {
            SetScrollPos(this->hWnd,SB_CTL,si.nMax-si.nPage+2,true);
            this->UpdatePanelFromScrollBar(si.nMax-si.nPage+2);
         }
         break;
      }
   }

   return(OK);
}


void ProcessScrollWheelEvent(ObjectData *obj, HWND hWnd, short zDel, short fwKeys)
{
      float zDelta = zDel/20.0;    // wheel rotation
      SCROLLINFO si;
      si.cbSize = sizeof(SCROLLINFO);
      si.fMask = SIF_ALL;
      GetScrollInfo(obj->hWnd,SB_CTL, &si);
      int pos = si.nTrackPos;
      int off = nint(pos-zDelta);
      if(off < 0)
         off = 0;
      if(off > si.nMax-si.nPage+2)
         off = si.nMax-si.nPage+2;

      SetScrollPos(obj->hWnd,SB_CTL,off,true);
      obj->UpdatePanelFromScrollBar(off);
}

/***************************************************************************
   Event procedure for the panel scrollbar
***************************************************************************/

LRESULT CALLBACK PanelScrollEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   LRESULT r = 0;
   static bool resizeFlag = 0;
      
   HWND parWin = GetParent(hWnd);
   WinData* win  = GetWinDataClass(parWin);
   if(!win || win->inRemoval) return(1);            
   ObjectData* obj = win->widgets.findByWin(hWnd); // Find object selected
   if(!obj)
      return(CallWindowProc(OldPanelScrollProc,hWnd, messg, wParam, lParam));


   switch(messg)
   {
      case(WM_PAINT):
      {
         if(obj)
         {
			   r = CallWindowProc(OldPanelScrollProc,hWnd, messg, wParam, lParam);
			   obj->decorate(parWin);
	        // obj->DrawPanel();
            return r;
         }
         break;
      }


      //case(WM_ERASEBKGND): // Prevent background being cleared before resize or repainting
      //{
      //   return(0);
      //} // Makes no difference

 
   }

     
   return(CallWindowProc(OldPanelScrollProc,hWnd, messg, wParam, lParam));
            
}
HRGN CreatePanelBackgroundRegion(WinData *win, HWND hWnd);


/***************************************************************************
   Event procedure for the panel
***************************************************************************/

LRESULT CALLBACK PanelEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   WinData *win = 0;
   ObjectData *obj = 0;
   int r;

   HWND parWin = GetParent(hWnd);
   win = GetWinDataClass(parWin);
   if(!win || win->inRemoval) return(1);  

   switch(messg)
   {
		case(WM_PAINT): 
		{
		   PAINTSTRUCT p;
         HBRUSH hBrush;
         HRGN hRgn;
         HDC hdc;
         RECT r;
         if(win)
         {
            hRgn = CreatePanelBackgroundRegion(win,hWnd); // Calculate the background region
          
               hdc = BeginPaint(hWnd, &p ); // Paint the background
               if(win->bkgColor & 255<<24)
                 hBrush = (HBRUSH)CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
   	          // hBrush = (HBRUSH)CreateSolidBrush(RGB(0,128,0));
               else
   	            hBrush = (HBRUSH)CreateSolidBrush(win->bkgColor);
               FillRgn(hdc,hRgn,hBrush);
               DeleteObject(hBrush);
      
		      EndPaint(hWnd, &p );	// stop painting  

            DeleteObject(hRgn);
         }
		   return(0);	 	   
      } 

      case(WM_COMMAND): 
		{
         HWND hwndCtl = (HWND)lParam;      // handle of control 
         obj = win->widgets.findByWin(hwndCtl); // Find object selected
         if(obj)
         {
            r = SendMessage(parWin,WM_COMMAND,wParam,lParam);
            return(r);
         }
         break;
      }

      case(WM_ERASEBKGND): // Prevent background being cleared before resize or repainting
      {
         return(0);
      }


      case(WM_CTLCOLORSTATIC): // Make sure statictext controls in the panel are painted in correct colour
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
            if(obj->bgColor & 255<<24)
	            SetBkColor(hdc,RGB(255,255,255));
            else
               SetBkColor(hdc,obj->bgColor);
            if(obj->fgColor & 255<<24)
	            SetTextColor(hdc,RGB(0,0,0));
            else
               SetTextColor(hdc,obj->fgColor);
         }
         return(r);
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
              // return(DrawListBoxItem((LPDRAWITEMSTRUCT)lParam));
            }
            else if(obj && obj->type == GROUP_BOX)
            {
               return obj->DrawGroupBox((LPDRAWITEMSTRUCT)lParam);
            }
            else if(obj && obj->type == BUTTON)
            {
               return(DrawButton(obj, (LPDRAWITEMSTRUCT)lParam));
            }
            else if(obj && obj->type == PANEL)
            {
             //  return obj->DrawPanel();
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
                //  PaintTab(obj,lpdis);
                  return(true);
               }
               break;
            }

         }
      }

   }

   return(CallWindowProc(OldPanelProc,hWnd, messg, wParam, lParam));  
}

HRGN CreatePanelBackgroundRegion(WinData *win, HWND hWnd)
{
	RECT r;
	HRGN hRgnRect;
	HRGN hRgn = CreateRectRgn(0,0,1,1);
   ObjectData *o;

	for(ObjectData *obj: win->widgets.getWidgets())
	{ 
      if(obj->panelParent)
      {
         PanelInfo *pInfo = (PanelInfo*)obj->panelParent->data;

         if(pInfo)
         {
		      if(obj->panelParent && obj->visible && pInfo->hWndPanel == hWnd)
		      {
		       //  if(obj->type == TEXTBOX)
			      //{
				     // SetRect(&r,obj->xo-1,obj->yo,obj->xo+obj->wo+1,obj->yo+obj->ho);
				     // hRgnRect = CreateRectRgnIndirect(&r);
				     // CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
				     // DeleteObject(hRgnRect);
			      //}
			       if(obj->type == RADIO_BUTTON)
			      {
				      RadioButtonInfo *info = (RadioButtonInfo*)obj->data;
				      for(int i = 0; i < info->nrBut; i++)
				      {  
					      GetSubWindowRect(hWnd,info->hWnd[i], &r);
					      hRgnRect = CreateRectRgnIndirect(&r);
					      CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
					      DeleteObject(hRgnRect); 
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
	GetClientRect(hWnd,&r);
	hRgnRect = CreateRectRgnIndirect(&r);
	CombineRgn(hRgn,hRgnRect,hRgn,RGN_DIFF);
	DeleteObject(hRgnRect);

	return(hRgn);
	
}
// Define a region which encloses the panel object
HRGN ObjectData::GetPanelRegion()
{
   extern HRGN AddRectToRgn(HRGN hRgn, int x1, int y1, int x2, int y2);

   PanelInfo *pInfo = (PanelInfo*)this->data;
   int px = pInfo->x;
   int py = pInfo->y;
   int pw = pInfo->w;
   int ph = pInfo->h;

	HRGN hRgn = CreateRectRgn(0,0,1,1);
   hRgn = AddRectToRgn(hRgn,px,py,px+pw+wo,py+ph);
   //hRgn = AddRectToRgn(hRgn,px,py,px+pw,py+del);
   //hRgn = AddRectToRgn(hRgn,px,py+ph-del,px+pw,py+ph);
   //hRgn = AddRectToRgn(hRgn,px,py,px+del,py+ph);
   //hRgn = AddRectToRgn(hRgn,px+pw-del,py,px+pw,py+ph);
   //hRgn = AddRectToRgn(hRgn,xo,yo,xo+wo,yo+ho);
	
	return(hRgn);
}
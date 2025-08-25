#include "stdafx.h"
#include "dividers.h"
#include "defineWindows.h"
#include "edit_class.h"
#include "events_edit.h"
#include "globals.h"
#include "guiInteractiveWindowBuilder.h"
#include "guiModifyObjectParameters.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include "htmlviewer.h"
#include "mymath.h"
#include "PlotWindow.h"
#include "WidgetRegistry.h"
#include "memoryLeak.h"

void TrackDivider(WinData *win, ObjectData *obj,HWND hWnd, bool horiz);
void MoveDivider(WinData *win, HWND hwnd, short &x, short &y, short &oldx, short &oldy, short dividerWidth, bool horiz);
void ResizeObjectsWhenParentResizes(WinData *win, ObjectData *obj, HWND hWnd, short x, short y);
void GetObjSizeParameters(WinData *win, ObjectData *objCur, RECT region, short ww, short wh, short offy);
void WindowCoorindateConversion(HWND srcWin, HWND destWin, short xIn, short yIn, short &xOut, short &yOut);

LRESULT CALLBACK DividerEventsProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{  
   HBRUSH hBrush;
   LRESULT r = 0;
   WinData *win = 0;
   ObjectData *obj = 0;

// Find object receiving events
 //  HWND parWin = GetParent(hWnd);
 //  WinData* win = rootWin->FindWinByHWND(parWin);
 //  if(!win || win->inRemoval) return(1);    
	//ObjectData *obj = win->widgets.findByWin(hWnd); 
 //  if(!obj) return(1);

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
      return(DefWindowProc( hWnd, messg, wParam, lParam ));
	
	switch(messg)
	{	
      case(WM_PAINT):
      {
		   PAINTSTRUCT p;
         RECT r;
         hBrush = (HBRUSH)CreateSolidBrush(GetSysColor(COLOR_3DFACE));
         HPEN pen1 = CreatePen(PS_SOLID,0,GetSysColor(COLOR_3DHIGHLIGHT));  
         HPEN pen2 = CreatePen(PS_SOLID,0,GetSysColor((COLOR_3DSHADOW)));  
         HPEN pen3 = CreatePen(PS_SOLID,0,RGB(0,0,0));  
  
	      HDC hdc = BeginPaint(hWnd, &p ); // Paint the background
   	
         GetClientRect(hWnd,&r);
         FillRect(hdc,&r,hBrush);
	
			// Draw the divider. If >= 5 in width draw 'drag grips' otherwise keep it simple
         if(r.bottom > r.right) // Vertical
         {
				int width = r.right - r.left;

				if(width >= 5 && obj->eventActive)
				{
					int sz = 1;
					SelectObject(hdc, pen1); 
					MoveToEx(hdc,(r.right+r.left)/2-sz,r.top,NULL);
					LineTo(hdc,(r.right+r.left)/2-sz,r.bottom);
					SelectObject(hdc, pen2); 
					MoveToEx(hdc,(r.right+r.left)/2+sz+1,r.top,NULL);
					LineTo(hdc,(r.right+r.left)/2+sz+1,r.bottom);
					for(int k = -20; k < 20; k+=2)
					{
						MoveToEx(hdc,(r.right+r.left)/2,(r.top+r.bottom)/2+k,NULL);
						LineTo(hdc,(r.right+r.left)/2,(r.top+r.bottom)/2+k-1);	
					}
				}
				else
				{
					SelectObject(hdc, pen1); 
					MoveToEx(hdc,(r.right+r.left)/2-1,r.top,NULL);
					LineTo(hdc,(r.right+r.left)/2-1,r.bottom);
					SelectObject(hdc, pen2); 
					MoveToEx(hdc,(r.right+r.left)/2+1,r.top,NULL);
					LineTo(hdc,(r.right+r.left)/2+1,r.bottom);
				}
         }
         else // Horizontal
         {
				int height = r.bottom - r.top;
				if(height >= 5 && obj->eventActive)
				{
					int sz = 1;
					SelectObject(hdc, pen1);
					MoveToEx(hdc,r.right,(r.top+r.bottom)/2-sz,NULL);
					LineTo(hdc,r.left,(r.top+r.bottom)/2-sz);
					//MoveToEx(hdc,r.left,r.top-sz,NULL); // Left End
		 //        LineTo(hdc,r.left,r.bottom+sz);
					SelectObject(hdc, pen2);
					MoveToEx(hdc,r.right,(r.top+r.bottom)/2+sz+1,NULL);
					LineTo(hdc,r.left,(r.top+r.bottom)/2+sz+1);
					//MoveToEx(hdc,r.right-1,r.top-sz,NULL); // Right End
		 //        LineTo(hdc,r.right-1,r.bottom+sz);
					SelectObject(hdc, pen2);
					for(int k = -20; k < 20; k+=2)
					{
						MoveToEx(hdc,(r.right+r.left)/2+k,(r.top+r.bottom)/2,NULL);
						LineTo(hdc,(r.right+r.left)/2+k-1,(r.top+r.bottom)/2);	
					}
				}
				else
				{
					SelectObject(hdc, pen1);
					MoveToEx(hdc,r.right,(r.top+r.bottom)/2-1,NULL);
					LineTo(hdc,r.left,(r.top+r.bottom)/2-1);
					SelectObject(hdc, pen2);
					MoveToEx(hdc,r.right,(r.top+r.bottom)/2+1,NULL);
					LineTo(hdc,r.left,(r.top+r.bottom)/2+1);
				}

         }
           
		   EndPaint(hWnd, &p );	// stop painting  
         DeleteObject(hBrush);
         DeleteObject(pen1);
         DeleteObject(pen2);
         DeleteObject(pen3);

         obj->decorate(parWin);
         return(0);
      }

      case(WM_ERASEBKGND): // Prevent background being cleared before resize
	   {
	      return(1);
	   }

      case(WM_LBUTTONDBLCLK):
		{
         break;
		}

      case(WM_MOUSEMOVE): // Mouse has been moved
      case(WM_LBUTTONDOWN): // Left button is down
      { 
         if(!obj->eventActive)
            break;

			long x,y;

	  // Note location of cursor in client coordinates
         x = LOWORD(lParam);  // horizontal position of cursor 
         y = HIWORD(lParam);  // vertical position of cursor 

         if(obj->wo > obj->ho)
            SetCursor(HorizDivCursor);
         else
            SetCursor(VertDivCursor);

      // Track the cursor
         if(wParam == MK_LBUTTON)
         {
            POINT p;
            p.x = x;
            p.y = y;
            ClientToScreen(hWnd,&p);   
            ScreenToClient(obj->hwndParent,&p); 
            x = p.x;
            y = p.y;
            TrackDivider(win, obj, hWnd, (obj->wo > obj->ho));
         }
         break;
      }
	}
	
	return(DefWindowProc( hWnd, messg, wParam, lParam ));
}	

void TrackDivider(WinData *win, ObjectData *obj, HWND hWnd, bool horiz)
{
   MSG msg;
   short x = -1,y = -1;
   short oldx = -1;
   short oldy = -1;
   DividerInfo *info;
 	RECT wR,cR; 
   short tbh = 0;
   short sbh = 0;

// Check for gui toolbar and status bar
   if(win->toolbar)
   {
      GetClientRect(win->toolbar->hWnd,&cR);
      tbh = cR.bottom-cR.top;
   }
   if(win->statusbox)
   {
      GetClientRect(win->statusbox->hWnd,&cR);
      sbh = cR.bottom-cR.top;
   }

// Convert mouse screen coordinates to gui window client coordinates 
   GetSubWindowRect(win->hWnd,hWnd, &cR);
   GetClientRect(win->hWnd,&wR);

   info = (DividerInfo*)obj->data;

// Left mouse button must be down  
   if(!IsKeyDown(VK_LBUTTON))
      return;

	int leftLim,bottomLim;

// Check for left or bottom limits (limit > 10000 is a 'backwards' limit from windows edge)
// e.g. 10200 means can't get closer than 200 pixels to left or bottom edge
	if(horiz)
	{
		if(info->maxPos.xo > 10000)
			leftLim = wR.bottom-(info->maxPos.xo-10000);
		else
			leftLim = info->maxPos.xo;

	}
	else
	{
		if(info->maxPos.xo > 10000)
			leftLim = wR.right-(info->maxPos.xo-10000);
		else
			leftLim = info->maxPos.xo;
	}

// Capture mouse for editwin so we can move beyond edges of window    
   SetCapture(hWnd);
  
// Track movement of mouse as long as leftbutton is down
   while(true)
   {
	   if(PeekMessage(&msg, NULL, NULL , NULL, PM_REMOVE))
	   {
	      if(IsKeyDown(VK_TAB) && IsKeyDown(VK_LMENU)) // Exit on Alt-Tab
	         break;
 
		//	if(!editActivated) // Exit if another application gets focus
		//	   break;	
			       
	      if(msg.hwnd == hWnd && msg.message == WM_LBUTTONUP) // Finished moving divider?
			   break;
			   	      
         if(msg.hwnd == hWnd && msg.message == WM_MOUSEMOVE) // Mouse is moving so track
			{	
			   if(msg.wParam & MK_LBUTTON)
			   {
			      x = LOWORD(msg.lParam);
			      y = HIWORD(msg.lParam);
               RECT rect;
               GetClientRect(win->hWnd,&rect);
               long aw = rect.right;
               long ah = rect.bottom;

           // Make sure cursor is not outside divider limits
               if(info->useLimits) // Limit tracking range
               {
                  long max,min;
                  if(horiz)
                  {
                     max = info->maxPos.xs*ah+leftLim;
                     min = info->minPos.xs*ah+info->minPos.xo;
                     if(y+obj->yo > max)
                        y = max-obj->yo;
                     else if(y+obj->yo < min)
                        y = min-obj->yo;
                  }
                  else
                  {
                     max = info->maxPos.xs*aw+leftLim;
                     min = info->minPos.xs*aw+info->minPos.xo;
                     if(x+obj->xo > max)
                        x = max-obj->xo;
                     else if(x+obj->xo < min)
                        x = min-obj->xo;
                  }
               }

               short xw,yw;
               WindowCoorindateConversion(obj->hWnd, win->hWnd, x, y, xw, yw);

               short xn = xw;
               short yn = yw;

               short max = -30000;
               short min = 30000;

           // Make sure cursor is not outside other divider limits
	            for(ObjectData* ctrl : win->widgets.getWidgets())
               {
                  if(obj == ctrl)
                     continue;

                  if(ctrl->type == DIVIDER && ctrl->panelParent == NULL) // Look for divider not in a panel
                  {
                     DividerInfo *ctrlInfo;
                     ctrlInfo = (DividerInfo*)ctrl->data;
                     if(ctrlInfo->orientation == WindowLayout::VERTICAL && !horiz)
                     {
                        if((ctrlInfo->origPos > info->origPos) && ((xw+ctrl->wo/2+DIVIDER_OFFSET-ctrl->xo) > 0))
                        {
                           x = ctrl->xo-ctrl->wo/2-DIVIDER_OFFSET;
                           if(x < min) 
                           {
                              min = x;
                              xn = x;
                           }

                        }
                        else if((info->origPos > ctrlInfo->origPos) && ((ctrl->xo+1.5*ctrl->wo+DIVIDER_OFFSET-xw) > 0))
                        {
                           x = ctrl->xo+1.5*ctrl->wo+DIVIDER_OFFSET; 
                           if(x > max)
                           {
                              max = x;
                              xn = x;
                           }
                        }
                     }
                     else if(ctrlInfo->orientation == WindowLayout::HORIZONTAL && horiz)
                     {
                        if((ctrlInfo->origPos > info->origPos) && ((yw+ctrl->ho/2+DIVIDER_OFFSET-ctrl->yo) > 0))
                        {
                           y = ctrl->yo-ctrl->ho/2-DIVIDER_OFFSET;
                           if(y < min) 
                           {
                              min = y;
                              yn = y;
                           }

                        }
                        else if((info->origPos > ctrlInfo->origPos) && ((ctrl->yo+1.5*ctrl->ho+DIVIDER_OFFSET-yw) > 0))
                        {
                           y = ctrl->yo+1.5*ctrl->ho+DIVIDER_OFFSET;
                           if(y > max)
                           {
                              max = y;
                              yn = y;
                           }
                        }
                     }
                  }
               }
					
				// Is it a vertical divider? *************************
					if(!horiz)
					{
						short leftLimit = DIVIDER_OFFSET;
						short rightLimit = wR.right-DIVIDER_OFFSET-obj->wo;  
                 //         TextMessage("x,y (%d,%d) leftLim, rightLim (%d,%d)\n",xn,yn, leftLimit,rightLimit);  
					  // Limit the movement of the cursor       
						if(xn < leftLimit)
							xn = leftLimit;
						if(xn > rightLimit)
							xn = rightLimit;   
					}
					else
					{
						short topLimit = DIVIDER_OFFSET+tbh;
						short bottomLimit = wR.bottom-DIVIDER_OFFSET-sbh;   
                 // TextMessage("x,y (%d,%d) topLim, botLim (%d,%d)\n",xn,yn, topLimit,bottomLimit);  
             
					  // Limit the movement of the cursor       
						if(yn < topLimit)
							yn = topLimit;
						if(yn > bottomLimit)
							yn = bottomLimit; 
					}

               WindowCoorindateConversion(win->hWnd, obj->hWnd, xn, yn, x, y);

					// Comment these lines for real time updates
			      MoveDivider(win, msg.hwnd,x,y,oldx,oldy,obj->wo, horiz); 

               WindowCoorindateConversion(win->hWnd, obj->hWnd, x, y, x, y);

					// Comment these lines for late updates
				//	ResizeObjectsWhenParentResizes(win, obj, msg.hwnd, x, y);

             //  RedrawWindow(win->hWnd,NULL,NULL,RDW_ERASE | RDW_UPDATENOW | RDW_INVALIDATE);

			   }
			   else
	            break; // Finished moving divider
			}
		}
   }
   
// Update the sub editor windows with chosen sizes
   if(x != -1 && y != -1)
   {
      ResizeObjectsWhenParentResizes(win, obj, msg.hwnd, x, y);
   }
				
// Release and restore cursor
   ReleaseCapture();

// Redraw parent window to clean up any divider mess
   MyInvalidateRect(win->hWnd,NULL,false);

 //  cursor_mode = NORMAL_CURSOR;
}

/***************************************************************************
  Convert coordinates for point (xIn,yIn) in srcWin to (xOut,yOut) in
  destWin.
****************************************************************************/ 
  
void WindowCoorindateConversion(HWND srcWin, HWND destWin, short xIn, short yIn, short &xOut, short &yOut)
{
   POINT p;
   p.x = xIn;
   p.y = yIn;
   ClientToScreen(srcWin,&p);   
   ScreenToClient(destWin,&p); 
   xOut = p.x;
   yOut = p.y; 
}

/***************************************************************************
  Draw a line representing the new position of the window divider. This 
  line tracks the mouse cursor position.
****************************************************************************/ 
   
void MoveDivider(WinData *win, HWND hwnd, short &x, short &y, short &oldx, short &oldy, short dividerWidth, bool horiz)
{
 	RECT wR,cR; 
   short tbh = 0;
   short sbh = 0;

// Check for gui toolbar and status bar
   if(win->toolbar)
   {
      GetClientRect(win->toolbar->hWnd,&cR);
      tbh = cR.bottom-cR.top;
   }
   if(win->statusbox)
   {
      GetClientRect(win->statusbox->hWnd,&cR);
      sbh = cR.bottom-cR.top;
   }

// Convert mouse screen coordinates to gui window client coordinates 
   GetSubWindowRect(win->hWnd,hwnd, &cR);
   GetClientRect(win->hWnd,&wR);

   WindowCoorindateConversion(hwnd, win->hWnd, x, y, x, y);

// Choose the drawing pen for the movable divider
   HDC hdc = GetDC(win->hWnd);
   SetROP2(hdc,R2_XORPEN);
   HPEN pen = CreatePen(PS_SOLID,4,RGB(128,128,128));  
   HPEN oldpen = (HPEN)SelectObject(hdc, pen); 
             
// Is it a vertical divider? *************************
	if(!horiz)
	{
      if(oldx != -1)
      {  // Erase the old divding line             
         MoveToEx(hdc,oldx+1,cR.top,NULL);
         LineTo(hdc,oldx+1,cR.bottom);
      }   

      short leftLimit = DIVIDER_OFFSET;
      short rightLimit = wR.right-DIVIDER_OFFSET-dividerWidth;  
                            
     // Limit the movement of the cursor       
      if(x < leftLimit)
         x = leftLimit;
      if(x > rightLimit)
         x = rightLimit;   

     // Draw the new divider position                                      
      MoveToEx(hdc,x+1,cR.top,NULL);
      LineTo(hdc,x+1,cR.bottom);
      oldx = x;
   }
   else
   {
      if(oldy != -1)
      {  // Erase the old divding line             
         MoveToEx(hdc,cR.left,oldy+1,NULL);
         LineTo(hdc,cR.right,oldy+1);
      }   

      short topLimit = DIVIDER_OFFSET+tbh;
      short bottomLimit = wR.bottom-DIVIDER_OFFSET-sbh;  
                            
     // Limit the movement of the cursor       
      if(y < topLimit)
         y = topLimit;
      if(y > bottomLimit)
         y = bottomLimit;  

     // Draw the new divider position                                      
      MoveToEx(hdc,cR.left,y+1,NULL);
      LineTo(hdc,cR.right,y+1);
      oldy = y;
   }

// Tidy up
   SelectObject(hdc, oldpen); 
   ReleaseDC(win->hWnd,hdc);
   DeleteObject(pen);                                    
}

// Resize and position all the objects based on new divider position or if window has resized

void ResizeObjectsWhenParentResizes(WinData *win, ObjectData *objDiv, HWND hWnd, short x, short y)
{
   RECT r;
   long ww,wh,offy=0;


   if(hWnd)
   {
      POINT p;
      p.x = x;
      p.y = y;
      ClientToScreen(hWnd,&p);   
      ScreenToClient(win->hWnd,&p); 
      x = p.x;
      y = p.y;
   }

   GetClientRect(win->hWnd,&r);
   ww = r.right-r.left;
   wh = r.bottom-r.top;

   if(win->fixedObjects)
   {
   // Allow for status boxes
      if(win->statusbox)
      {
         GetClientRect(win->statusbox->hWnd,&r);
      //   offy = r.bottom+1;
         wh -= r.bottom-1;
      }

   // Allow for toolbars
      if(win->toolbar)
      {
         GetClientRect(win->toolbar->hWnd,&r);
         offy += r.bottom+3;
         wh -= r.bottom+3;
      }
   }

// Work out new divider position
   if(objDiv)
   {
      if(objDiv->wo < objDiv->ho)
      {
         if(objDiv->xSzScale > 0)
            objDiv->xSzScale = (x-objDiv->wo/2)/(float)ww;
      }
      else
      {
         if(objDiv->ySzScale > 0)
            objDiv->ySzScale = (y-offy-objDiv->ho/2)/(float)wh;
      }
   }

	for(ObjectData* obj : win->widgets.getWidgets())
   {
      if(obj->regionSized) // Region sized
      {
         GetObjSizeParameters(win,obj,obj->region,ww,wh,offy);
      }
      else // Fixed or window based
      {
         obj->xo = nint(obj->xSzScale*ww + obj->xSzOffset);
         obj->yo = nint(obj->ySzScale*wh + obj->ySzOffset + offy);
         obj->wo = nint(obj->wSzScale*ww + obj->wSzOffset);
         obj->ho = nint(obj->hSzScale*wh + obj->hSzOffset);
      }

      if(obj->type == DIVIDER) // Check for limits on divider position
      {
         DividerInfo *info = (DividerInfo*)obj->data;  

         if(info->useLimits) // Limit divider position 
         {
            long max,min;
            if(info->orientation ==  WindowLayout::HORIZONTAL)
            {
               max = info->maxPos.xs*wh+info->maxPos.xo;
               min = info->minPos.xs*wh+info->minPos.xo;
               if(obj->yo > max)
                  obj->yo = max;
               else if(obj->yo < min)
                  obj->yo = min;
            }
            else
            {
               max = info->maxPos.xs*ww+info->maxPos.xo;
               min = info->minPos.xs*ww+info->minPos.xo;
               if(obj->xo > max)
                  obj->xo = max;
               else if(obj->xo < min)
                  obj->xo = min;
            }
         }
         obj->Place(obj->xo,obj->yo,obj->wo,obj->ho,false);

      }
      if(obj->type == TEXTEDITOR)
      {		   
         EditParent *ep;
         EditRegion *er;
         short x,y,w,h;

         ep = (EditParent*)obj->data;

         for(short i = 0; i < ep->rows*ep->cols; i++) 
         {	
            er = ep->editData[i];
            x = nint(er->x*obj->wo + obj->xo);
            y = nint(er->y*obj->ho + obj->yo);
            w = nint(er->w*obj->wo);
            h = nint(er->h*obj->ho);	               
            MoveWindow(ep->editData[i]->edWin,x,y,w,h,true);   		            
         }	
      }
      else if(obj->type == IMAGEWINDOW)
      {
         PlotWindow *pp = (PlotWindow*)obj->data;

         if(!pp->isBusy())
         {
             obj->Place(obj->xo,obj->yo,obj->wo,obj->ho,false);
         }
      }
      else
      {
          obj->Place(obj->xo,obj->yo,obj->wo,obj->ho,false);
      }

      // Resize status box parts
      if(obj->type == STATUSBOX) 
      {
			float scale,off;
         bool region;
         long yoff = 0;
         extern short titleBarNMenuHeight,titleBarHeight;

         if(obj->winParent->menuObj)
            yoff = titleBarNMenuHeight;
         else
            yoff = titleBarHeight;

         if(obj->data)
         {
            StatusBoxInfo* info = (StatusBoxInfo*)obj->data;
            int* statusParts = new int[info->parts];
            for(int k = 0; k < info->parts; k++)
            {
               if(GetObjectDimensions(info->posArray[k], &scale, &off, &region) == ERR)
               {
						delete[] statusParts;
                  ErrorMessage("invalid x position");
                  return;
               }
               statusParts[k] = nint(ww*scale + off);
            }
            SendMessage(obj->hWnd,SB_SETPARTS, (WPARAM)info->parts, (LPARAM) statusParts);

            GetWindowRect(obj->hWnd,&r);
            DWORD syntaxSize =  ((r.bottom-r.top-4)<<16) + (statusParts[0]-5);
            SendMessage(info->subWindow,WM_SIZE,SIZE_RESTORED,(LPARAM)syntaxSize);
            delete [] statusParts;
            RECT r,rp;
            GetWindowRect(obj->hWnd,&r);
            GetWindowRect(obj->hwndParent,&rp);
            obj->xo = 0;
            obj->yo = r.top-rp.top-yoff;
            obj->wo = r.right-r.left+1;
            obj->ho = r.bottom-r.top+1;
         }
      }
      else if(obj->type == HTMLBOX) // Resize html viewer
      {
         ResizeHTMLViewer(obj->hWnd,obj->wo,obj->ho);
      }
      else if(obj->type == TOOLBAR) 
      {
         RECT r;
         GetClientRect(obj->hWnd,&r);
         obj->xo = 0;
         obj->yo = 0;
         obj->wo = r.right-r.left+1;
         obj->ho = r.bottom-r.top+3;
      }
      else if(obj->type == PANEL) // Resize panel scroll bar thumb
      {
         obj->UpdatePanelThumb(true);
      }
   }

  // MyInvalidateRect(win->hWnd,NULL,true);

}

void  GetObjSizeParameters(WinData *win, ObjectData *objCur, RECT region, short ww, short wh, short yoff)
{
   short oxl=0,oxr=0;
   short oyt=0,oyb=0;
   short rxl=0,rxr=0;
   short ryt=0,ryb=0;
   short rw,rh;
   ObjectData *obj;

// Region edge is a gui window boundary
   if(region.left == -1)
   {
      rxl = 0;
   }
   if(region.right == -2)
   {
      rxr = ww;
   }
   if(region.top == -3)
   {
      ryt = 0;
   }
   if(region.bottom == -4)
   {
      ryb = wh;
   }

// Region edge is a defined by normal object
   if(region.left == -5)
   {
      rxl = ww*objCur->xSzScale + objCur->xSzOffset;
   }
   if(region.right == -5)
   {
      rxr = ww*(objCur->xSzScale+objCur->wSzScale) + (objCur->xSzOffset+objCur->wSzOffset);
   }
   if(region.top == -5)
   {
      ryt = wh*objCur->ySzScale + objCur->ySzOffset;
   }
   if(region.bottom == -5)
   {
      ryb = wh*(objCur->ySzScale+objCur->hSzScale) + (objCur->ySzOffset+objCur->hSzOffset);
   }

// Region edge is a divider work out edge position
   if(region.left > 0)
   {
      obj = win->widgets.findByNr(region.left);
      if(obj)
      {
         DividerInfo *info = (DividerInfo*)obj->data;  
         int max = info->maxPos.xs*ww+info->maxPos.xo+obj->wSzOffset;
         int min = info->minPos.xs*ww+info->minPos.xo-obj->wSzOffset;
         rxl = nint(ww*(obj->xSzScale+obj->wSzScale) + obj->xSzOffset+obj->wSzOffset);
         if(info->useLimits && info->orientation == WindowLayout::VERTICAL)
         {
            if(rxl < min) rxl = min;
            if(rxl > max) rxl = max;
         }
      }
   }
   if(region.right > 0)
   {
      obj = win->widgets.findByNr(region.right);
      if(obj)
      {
         DividerInfo *info = (DividerInfo*)obj->data;  
         int max = info->maxPos.xs*ww+info->maxPos.xo;
         int min = info->minPos.xs*ww+info->minPos.xo;
         rxr = nint(ww*(obj->xSzScale) + (obj->xSzOffset));
         if(info->useLimits && info->orientation == WindowLayout::VERTICAL)
         {
            if(rxr < min) rxr = min;
            if(rxr > max) rxr = max;
         }
      }
   }
   if(region.top > 0)
   {
      obj = win->widgets.findByNr(region.top);
      if(obj)
      {
         DividerInfo *info = (DividerInfo*)obj->data;  
         int max = info->maxPos.xs*wh+info->maxPos.xo+obj->hSzOffset;
         int min = info->minPos.xs*wh+info->minPos.xo-obj->hSzOffset;
         ryt = nint(wh*(obj->ySzScale+obj->hSzScale) + obj->ySzOffset+obj->hSzOffset);
         if(info->useLimits && info->orientation == WindowLayout::HORIZONTAL)
         {
            if(ryt < min) ryt = min;
            if(ryt > max) ryt = max;
         }
      }
   }
   if(region.bottom > 0)
   {
      obj = win->widgets.findByNr(region.bottom);
      if(obj)
      {
         DividerInfo *info = (DividerInfo*)obj->data;  
         int max = info->maxPos.xs*wh+info->maxPos.xo;
         int min = info->minPos.xs*wh+info->minPos.xo;
         ryb = nint(wh*(obj->ySzScale) + (obj->ySzOffset));
         if(info->useLimits && info->orientation == WindowLayout::HORIZONTAL)
         {
            if(ryb < min) ryb = min;
            if(ryb > max) ryb = max;
         }
      }
   }

// Calc region dimensions
   rw = rxr-rxl;
   rh = ryb-ryt;

// Region edge is a divider - work out object position inside region
   if(region.left != -5)
   {
      oxl = nint(rw*objCur->xSzScale + objCur->xSzOffset);
   }
   if(region.right != -5)
   {
      oxr = nint(rw*(objCur->xSzScale+objCur->wSzScale) + (objCur->xSzOffset+objCur->wSzOffset));
   }
   if(region.top != -5)
   {
      oyt = nint(rh*objCur->ySzScale + objCur->ySzOffset);
   }
   if(region.bottom != -5)
   {
      oyb = nint(rh*(objCur->ySzScale+objCur->hSzScale) + (objCur->ySzOffset+objCur->hSzOffset));
   }

   objCur->xo = rxl+oxl;
   objCur->yo = ryt+oyt+yoff;
   if(region.left == -5 && region.right == -5)
      objCur->wo = rw;
   else
      objCur->wo = oxr - oxl;
   if(region.top == -5 && region.bottom == -5)
      objCur->ho = rh;
   else
      objCur->ho = oyb - oyt;

}

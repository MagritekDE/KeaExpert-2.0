#define WINVER _WIN32_WINNT 
#include "stdafx.h"
#include "plot2dEvents.h"
#include <shellapi.h>
#include "bitmap.h"
#include "defineWindows.h"
#include "files.h"
#include "globals.h"
#include "guiMakeObjects.h"
#include "guiModifyObjectParameters.h"
#include "guiWindowClass.h"
#include "guiWindowCLI.h"
#include "guiWindowsEvents.h"
#include "Inset.h"
#include "interface.h"
#include "load_save_data.h"
#include "plot2dCLI.h"
#include "PlotWindow.h"
#include "preferences.h"
#include "process.h"
#include "string_utilities.h"
#include "variablesOther.h"
#include "Windowsx.h"
#include "memoryLeak.h"

#pragma warning (disable: 4996) // Ignore deprecated library functions

/*****************************************************************************
*            Routines relating to the 2D user defined plot window
*
* Plot2DFunctions ................. plot2dfunc command arguments to access image functions: 
*
*   axes font
*   axes range
*   border axes
*   copy to clipboard
*   corner axes
*   display data
*   drag plot
*   enlarge horizontal
*   enlarge vertical
*   full region
*   hide borders
*   image colours
*   last region
*   load file
*   make square
*   mapping and grid
*   move down
*   move left
*   move right
*   move up
*   multiplot 1*1
*   multiplot 1*2
*   multiplot 2*1
*   multiplot 2*2
*   multiplot m*n
*   plot colours
*   plot title
*   plot x,y labels
*   print plot
*   reduce horizontal
*   reduce vertical
*   remove current data
*   remove other subplots	
*   save file
*   select column
*   select region
*   select row
*   show all plots
*   show borders
*   ticks and labels
*   toggle border
*   toggle colorbar
*   view one plot
*   zoom region
*
*****************************************************************************/

//bool inResize = false;
//bool inRepaint = false;
//PlotWindow2D *inRepaintPP = 0;
//PlotWindow2D *inResizePP = 0;
//double inRepaintTime = 0;
// Locally define functions
void  LoadAndDisplay3DDataFile(HWND hWnd, Plot *pd, char *filePath, char *fileName);
bool busySet = false;

/*****************************************************************************
*                   Event procedure for 1D plot window
* Events
*
* WM_MOUSEACTIVATE ... plot selected
* WM_PAINT ........... redraw plot
* WM_ERASEBKGND ...... prevent flicker
* WM_SIZE ............ parent resized
* WM_LBUTTONDOWN ..... plot selected also move, data, region selection 
* WM_LBUTTONUP ....... selection complete
* WM_RBUTTONDOWN ..... contextual menus, data offset
* WM_MOUSEMOVE ....... Move, data display region selection.
*
*****************************************************************************/

LRESULT CALLBACK ImageEventsProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{  
   static short oldx = -1;       // Last cursor location 
   static short oldy = -1; 
   static TRACKMOUSEEVENT tme;


// Find object receiving events
   HWND parWin = GetParent(hWnd);
   WinData* win = rootWin->FindWinByHWND(parWin);
   if(!win)
   {
      HWND parWin2 = GetParent(parWin);
      win = GetWinDataClass(parWin2);
   }
   if(!win || win->inRemoval) return(1);    
   ObjectData *obj = GetObjData(hWnd); 
   if(!obj || !obj->data) return(1);
   PlotWindow2D *pp = (PlotWindow2D*)obj->data;

	switch(messg)
   {
      case(WM_DROPFILES): // User dropped a file onto the window
      {
         CText path,file,ext;
         if(GetDropFileInfo((HDROP)wParam ,path, file, ext, 0) == OK)
         {
            DragFinish((HDROP)wParam);
            SetForegroundWindow(prospaWin);
            SetFocus(hWnd);

         // Is it a folder?
            if(file == "")
            {
               PlotFile2D::setCurrPlotDirectory(path.Str());
	            TextMessage("\n\n  pathnames(\"plot2d\") = %s\n\n> ",path.Str());
               return(0);
            }

            else if(ext == "pt2")
            {
					if (pp)
					{
						if(pp->LoadAndDisplayPlotFile(hWnd, path.Str(), file.Str()) == ERR)
                     return(0);
					}
            }
			   else
			   {
				   int i;
				   POINT pt;
				   GetCursorPos(&pt);
				   ScreenToClient(hWnd,&pt);

				   PlotList& pd = pp->plotList();

				   for(i = 0; i < pp->rows*pp->cols; i++)
				   {
					   if(pd[i]->InRegion(pt.x,pt.y))
					      break;
				   }

               if(ext == "2d")
				   {
						if (pd[i])
						{
							pd[i]->LoadAndDisplayDataFile(hWnd, path.Str(), file.Str());
						}
				   }
               else if(ext == "3d")
				   {
					   LoadAndDisplay3DDataFile(hWnd, pd[i], path.Str(), file.Str());
			      }
				   else if(ext == "jpg" || ext == "png" || ext == "bmp")
				   {
	               if(pd[i])
	               {
                     char curDir[MAX_PATH];
                     Interface itfc;
                     itfc.nrRetValues = -1;
                   //  if(pd[i]->allowMakeCurrentPlot_)
                     {
						      pd[i]->makeCurrentPlot();
						      pd[i]->makeCurrentDimensionalPlot();
                     }
		               CText args;
                     args.Format("\"%s\"",file.Str());
                     GetCurrentDirectory(MAX_PATH,curDir);
                     SetCurrentDirectory(path.Str());
						   pd[i]->setFileName(file.Str());
						   pd[i]->setFilePath(path.Str());
						   pd[i]->setTitleText(file.Str());
					      LoadPictureTo3DMatrix(&itfc, args.Str());
                     SetCurrentDirectory(curDir);
                  }
			      }
			   }
         }
         return(0);
      }

      case(WM_MOUSEACTIVATE): // User has clicked on window - update title, menu and toolbar
      {
         SelectFixedControls(win, obj);
			CText txt;
			if(win->titleUpdate)
         {
			   txt.Format("2D Plot (%hd)",win->nr);
			   SetWindowText(win->hWnd,txt.Str());	
         }
			//else
			//   txt.Format("2D Plot");

         if(win && !win->keepInFront)
            ChangeGUIParent(parWin);
         SetFocus(hWnd);
         break;
      }

		case(WM_PAINT):  // Redraw plot when damaged TODO - no direct drawing?
		{
         PAINTSTRUCT p;
	      RECT pr;
         HDC hdc;
         MSG msg;
            extern double GetMsTime();

	      GetClientRect(hWnd,&pr);

        // Draw each image in the 2D array onto a bitmap & then copy to screen
         if(wParam == 0)
            hdc = BeginPaint(hWnd, &p ); 
         else 
            hdc = (HDC)wParam; // Allow for WM_PRINTCLIENT call
       
         while(pp->inCriticalSection())
            PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);

         if(TryEnterCriticalSection(&cs2DPlot))
         {
            pp->incCriticalSectionLevel();

            HDC hdcMem = CreateCompatibleDC(hdc);
            SelectObject(hdcMem,pp->bitmap);
			   for(Plot* p: pp->plotList())
            {
               p->Display(hWnd,hdcMem);
            }
            BitBlt(hdc,pr.left,pr.top,pr.right-pr.left,pr.bottom-pr.top,hdcMem,pr.left,pr.top,SRCCOPY);
            DeleteDC(hdcMem);

            LeaveCriticalSection(&cs2DPlot);
            pp->decCriticalSectionLevel();
         }

			if(Plot2D::dataCursorVisible())
            Plot2D::curPlot()->DisplayData(hdc, -1,-1, false);
    
         if(wParam == 0) // Allow for WM_PRINTCLIENT call
            EndPaint(hWnd, &p );	

         if(obj->selected_)
         {
            HDC hdc = GetDC(obj->hwndParent);
            obj->DrawSelectRect(hdc);
            ReleaseDC(obj->hwndParent,hdc);
         }
         if(win->displayObjCtrlNrs)
         {
            HDC hdc = GetDC(obj->hwndParent);
            obj->DrawControlNumber(hdc);
            ReleaseDC(obj->hwndParent,hdc);
         }
         if(win->displayObjTabNrs)
         {
            HDC hdc = GetDC(obj->hwndParent);
            obj->DrawTabNumber(hdc);
            ReleaseDC(obj->hwndParent,hdc);
         }

         return(0);   	   
      }

   // Draw the window to a specific device context
		case(WM_PRINTCLIENT): 
		{
         SendMessage(hWnd, WM_PAINT, wParam, lParam);
         break;
      }

	   case(WM_ERASEBKGND): // Prevent background being cleared before resize
	   {
	      return(1);
	   }

		case(WM_SIZE): // Parent window has been resized so resize plot
		{
		   RECT r;
         MSG msg;

         while(pp->inCriticalSection())
            PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);

         if(TryEnterCriticalSection(&cs2DPlot))
         {
		   	pp->incCriticalSectionLevel();

          // Update size of each 2D subwindow
	    	   for(Plot* p: pp->plotList())
			   {
				   p->ResizePlotRegion(hWnd); 
			   }
			   GetClientRect(hWnd,&r);
			   if(pp->bitmap) 
				   DeleteObject(pp->bitmap);
			   HDC hdc = GetDC(hWnd);
            long newWidth;
			   GenerateBitMap(r.right-r.left+1,r.bottom-r.top+1, &pp->bitmap, hdc, newWidth);
			   ReleaseDC(hWnd,hdc);
          
	     // Redraw window now all elements have been resized         
			   MyInvalidateRect(hWnd,NULL,false);

		      LeaveCriticalSection(&cs2DPlot);
            pp->decCriticalSectionLevel();
         }


		   break;
		}

   // Check for show/hide gui windows
      case(WM_KEYDOWN):
      {
         unsigned char key = (unsigned char)wParam;

         if(key == VK_F2)
         {
            if(AnyGUIWindowVisible())
               HideGUIWindows();
            else
               ShowGUIWindows(SW_SHOWNOACTIVATE);
         }

         break;
      }

   // User had clicked with left mouse button. Actions:
   // 1. Select a new plot
   // 2. Select a region
   // 3. In move mode so remove region rectangle

		case(WM_LBUTTONDOWN): 
		{
		   short x,y;
        
	  // Note location of mouse click
         x = LOWORD(lParam);
         y = HIWORD(lParam);
              
     // Find out which plot region has been selected.
     // If its a new region then set window title,
     // hide old selection rectangle, set current 2D plot,
     // redraw window and then return.

			for(Plot* p: pp->plotList())
			{
            if(p && p->InRegion(x,y))
            { // Don't change things unnecessarily
					Plot2D* plot = Plot2D::curPlot();

               if(!plot || (plot != p) || Plot::curPlot() != p) 
               {
						if((!plot || !Plot::curPlot())) // && p->allowMakeCurrentPlot_)
						{
							p->makeCurrentPlot();
							p->makeCurrentDimensionalPlot();
						}

	               plot->HideSelectionRectangle(NULL);
						p->makeCurrentDimensionalPlot();
                  UpdateStatusWindow(hWnd,3,p->statusText);	
                  MyInvalidateRect(Plot::curPlot()->win,NULL,false); // Remove old indent
                  MyUpdateWindow(Plot::curPlot()->win);
                  MyInvalidateRect(hWnd,NULL,false); // Add new indent
                  MyUpdateWindow(hWnd);
		            p->makeCurrentPlot();

                  HWND focusWin = GetFocus();
                  SendMessageToGUI("2D Plot,SelectImage",0);
                  SetFocus(focusWin);
               }

            // Update the status window text
               ShowWindow(parWin,SW_SHOW); 
				//	plot->updateStatusWindowForZoom(hWnd);
             //  UpdateStatusWindow(hWnd,3,plot->statusText);	
            }
         }

      // Update preferences window TODO why?
         if(prefsWin) 
            MyInvalidateRect(prefsWin,NULL,false);

      // Move plot mode selected - select start point
         if(pp->mouseMode == MOVE_PLOT)
         {            
	         Plot2D::curPlot()->beginToDragMovePlot(hWnd,x,y);  
            oldx = x;
            oldy = y;      
		   }
		   
     // If the plot region is not new then
     // select a rectangle if in SELECT_RECT mode or the control key is pressed
         if((pp->mouseMode == SELECT_RECT) | (wParam & MK_CONTROL))
         {
            MouseMode curMode = pp->mouseMode;
            pp->mouseMode = SELECT_RECT;
            SetCursor(SquareCursor);     

	         Plot2D::curPlot()->SelectRegion(hWnd,x,y);

            if(wParam & MK_CONTROL) // Zoom as well if control key is pressed
            {
               Interface itfc;
               itfc.win = win;
               Plot2DFunctions(&itfc,"\"zoom region\"");
	         }
            pp->mouseMode = curMode;
	      }		   	      
		   break;
		}


   // When leaving the 2D plot hide the data cursor
      case(WM_MOUSELEAVE):
      {
         if(Plot2D::curPlot())
         {
            HDC hdc = GetDC(hWnd);
            Plot2D::curPlot()->HideDataCursor(hdc);
            ReleaseDC(hWnd,hdc);
         }
         break;
      }

   // User is moving the mouse. Actions:
   // 1. Set the cursor based on the mode ...
   // 2. Left button down & MOVE_PLOT: Move the data
   // 3. In move mode so remove region rectangle

      case(WM_MOUSEMOVE): 
      { 
			long x,y;
         tme.cbSize = sizeof(TRACKMOUSEEVENT);
         tme.hwndTrack = hWnd;
         tme.dwFlags = TME_LEAVE;
         tme.dwHoverTime = HOVER_DEFAULT;
         TrackMouseEvent(&tme);

	  // Note location of cursor	
         x = GET_X_LPARAM(lParam);  // horizontal position of cursor 
         y = GET_Y_LPARAM(lParam);  // vertical position of cursor 

     // Set the cursor
         if(Plot2D::curPlot() && Plot2D::curPlot()->InPlotRect(x,y))
         {
             if(pp->mouseMode == MOVE_PLOT)
               SetCursor(LoadCursor(NULL,IDC_SIZEALL));
            else if(pp->mouseMode == SHOW_DATA)
               SetCursor(CrossCursor);
            else if(pp->mouseMode == SELECT_RECT)
               SetCursor(SquareCursor);               
            else if(pp->mouseMode == SELECT_COLUMN)
               SetCursor(LoadCursor(NULL,IDC_SIZENS));
            else if(pp->mouseMode == SELECT_ROW)
               SetCursor(LoadCursor(NULL,IDC_SIZEWE));
         }
         else
            SetCursor(LoadCursor(NULL,IDC_ARROW));  
                       
     // Left mouse button is down while cursor is moving (drag or pan mode)        
         if(wParam & MK_LBUTTON)
         {
        //    if(macroDepth == 0) // Don't do anything here if running a macro
            {
               if(Plot2D::curPlot() && (Plot2D::curPlot()->DataPresent() || (Plot2D::curPlot()->vx() && Plot2D::curPlot()->vy())))
               {
 		            HDC hdc = GetDC(hWnd);

	               if(pp->mouseMode == MOVE_PLOT) // Move data
	               {
	                  if(oldx != -1)
							{
								Plot2D* plot = Plot2D::curPlot();
								if (plot->insetBeingMoved())
								{	
									float dx = x - oldx;
									float dy = y - oldy;
									plot->insetBeingMoved()->shift(dx, dy);
									plot->Invalidate();
								}				
								else
								{
									long dx = plot->curXAxis()->scrnToData(x) - plot->curXAxis()->scrnToData(oldx);
									long dy = plot->curYAxis()->scrnToData(y) - plot->curYAxis()->scrnToData(oldy);
									plot->setVisibleLeft(plot->visibleLeft() - dx);
									plot->setVisibleTop(plot->visibleTop() - dy);
   	                  
									if(plot->visibleLeft() < 0) 
								      plot->setVisibleLeft(0);
									if(plot->visibleLeft()+plot->visibleWidth() > plot->matWidth())
								      plot->setVisibleLeft(plot->matWidth()-plot->visibleWidth());
   							
								   if(plot->visibleTop() < 0) 
								      plot->setVisibleTop(0);
								   if(plot->visibleTop()+plot->visibleHeight() > plot->matHeight())
								      plot->setVisibleTop(plot->matHeight()-plot->visibleHeight());
   	                  
						         Plot2D::curPlot()->Invalidate();
							      pp->Paint();
                           SendMessageToGUI("2D Plot,Move",0); 
								  // Update 3D plot if spacebar is pressed
									if(GetAsyncKeyState(VK_SPACE) & 0x08000)
									{
	                           ProcessMacroStr(1,NULL,NULL,"surf2dParameters:plot_data","","","surf2dParameters.mac","");
		                     }
								}
	                  }  
	                  oldx = x;
	                  oldy = y;
			         }
			         else if(pp->mouseMode == SHOW_DATA) // Display cross-hair cursor and data
		            {
			            if(Plot2D::curPlot()->InPlotRect(x,y)) 
			               Plot2D::curPlot()->DisplayData(hdc, x,y, (wParam & MK_SHIFT));
			            else
			               Plot2D::curPlot()->HideDataCursor(hdc);
		            }
		            else if(pp->mouseMode == SELECT_ROW) // Display horizontal cursor and row
		            {
			            if(Plot2D::curPlot()->InPlotRect(x,y)) 
							{
				            Plot2D::curPlot()->ScanRow(hWnd,x,y,"row");
								Plot1D::curPlot()->Invalidate();  
							}
				         else
			               Plot2D::curPlot()->HideRowCursor(hdc);
				      }
		            else if(pp->mouseMode == SELECT_COLUMN) // Display vertical cursor and row
		            {
			            if(Plot2D::curPlot()->InPlotRect(x,y)) 
							{
				            Plot2D::curPlot()->ScanColumn(hWnd,x,y,"col");
								Plot1D::curPlot()->Invalidate();  
							}
				         else
			               Plot2D::curPlot()->HideColumnCursor(hdc);
				      }
                  ReleaseDC(hWnd,hdc);
				   }
		      }		            
         }
         break;
      } 

   // User has released the left button:
   //   1. Hide cursors
   //   2. Record last row or column position
      case(WM_LBUTTONUP): 
      { 
         if(Plot2D::curPlot())
         {
            short x = LOWORD(lParam);  // horizontal position of cursor 
            short y = HIWORD(lParam);  // vertical position of cursor 

         // Display last horizontal row and tag it
		      if(pp->mouseMode == SELECT_ROW) 
		      {
               if(Plot2D::curPlot()->InPlotRect(x,y)) 
				      Plot2D::curPlot()->ScanRow(hWnd,x,y,"last");
               SendMessageToGUI("1D Plot,SelectRow",0);
			   }
         // Display vertical column
		      else if(pp->mouseMode == SELECT_COLUMN) 
		      {
			      if(Plot2D::curPlot()->InPlotRect(x,y)) 
				      Plot2D::curPlot()->ScanColumn(hWnd,x,y,"last");
               SendMessageToGUI("1D Plot,SelectCol",0);
			   }
         // Remove cursors
		      HDC hdc = GetDC(hWnd);      
            pp->ClearCursors(hdc);
	         ReleaseDC(hWnd,hdc);
            break;
         }
      }

   // User has pressed the right button. 
   //   1. First set current region
   // Then display a user defined contextual menu:
   //   1. Axes menu
   //   2. Title menu
   //   3. Labels menu
   //   4. Axes font menu
   //   5. Background (image) menu
      case(WM_RBUTTONDOWN): 
      {
         if(Plot2D::curPlot())
         {
		      POINT p;
            short whichTxt;

         // Get mouse coordinates
            p.x = LOWORD(lParam);
            p.y = HIWORD(lParam);
      	      
         // Find out which plot region the cursor is in // TODO lacks functionality?
				for(Plot* plot: pp->plotList())
				{
		         if(plot->InRegion(p.x,p.y)) // && plot->allowMakeCurrentPlot_)
               {
						plot->makeCurrentPlot();
						plot->makeCurrentDimensionalPlot();
                  break;
               }
            }
      
         // If left button is pressed and data cursor is visible a right click applies an offset
            if(wParam & MK_LBUTTON)
            {
	   	      if(pp->mouseMode == SHOW_DATA) 
		         {		         
			         if(Plot2D::curPlot()->InPlotRect(p.x,p.y)) 
                  {
		               HDC hdc = GetDC(hWnd);
	                  Plot2D::curPlot()->OffsetData(hdc,p.x,p.y);
                     ReleaseDC(hWnd,hdc);
                  }	

	               if(Plot2D::curPlot()->isOffset())
                  {
                     AddCharToString(Plot2D::curPlot()->statusText,'O');
                     UpdateStatusWindow(hWnd,3,Plot2D::curPlot()->statusText);	
                     UpdateStatusWindow(hWnd,1,"Left button : Hold for data display; Right button : remove offset");	
                  }
					   else
                  {
                     RemoveCharFromString(Plot2D::curPlot()->statusText,'O');
                     UpdateStatusWindow(hWnd,3,Plot2D::curPlot()->statusText);	
                     UpdateStatusWindow(hWnd,1,"Left button : Hold for data display; Right button : apply offset");	
                  }
		            break;
		         }
		      }

      // Check for clicking over certain regions (i.e. data, axes, text)  
      // and display appropriate contextual menu via WM_COMMAND

            HMENU hMenu;
            short item;

         // Axes contextual menu
            if(Plot2D::curPlot()->CursorOnAxes(p.x,p.y) == OK) 
            {
               if(!pp->axesMenu)
                  break;
               SetCursor(LoadCursor(NULL,IDC_ARROW));
               hMenu = pp->axesMenu;            
               ClientToScreen(hWnd,&p);
               item = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, 0, hWnd, NULL) ;
               if(item)
                  PreProcessCommand(NULL, item, hWnd, win);
            }

            else if((whichTxt = Plot2D::curPlot()->CursorInText(p.x,p.y)) != 0) // See if text has been selected
            {
               switch(whichTxt)
               {
               // Title contextual menu
                  case(TITLE_TEXT):
                  {
                     if(!pp->titleMenu)
                        break;
                     SetCursor(LoadCursor(NULL,IDC_ARROW));
                     hMenu = pp->titleMenu;            
                     ClientToScreen(hWnd,&p);
                     item = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, 0, hWnd, NULL);
                     if(item)
                        PreProcessCommand(NULL, item, hWnd, win);
                     break;
                  }
               // X/Y label contextual menu
                  case(X_LABEL):
                  case(Y_LABEL_L):
						case(Y_LABEL_R):
                  {
                     if(!pp->labelMenu)
                        break;
                     SetCursor(LoadCursor(NULL,IDC_ARROW));
                     hMenu = pp->labelMenu;            
                     ClientToScreen(hWnd,&p);
                     item = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, 0, hWnd, NULL);
                     if(item)
                        PreProcessCommand(NULL, item, hWnd, win);
                     break;
                  }
               // X/Y axes contextual menu
                  case(X_AXES_TEXT):
                  case(Y_AXES_TEXT_L):
						case(Y_AXES_TEXT_R):
                  {
                     HMENU hMenu;
                     if(!pp->axesMenu)
                        break;
                     SetCursor(LoadCursor(NULL,IDC_ARROW));
                     hMenu = pp->axesMenu;            
                     ClientToScreen(hWnd,&p);
                     item = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, 0, hWnd, NULL) ;
                     if(item)
                        PreProcessCommand(NULL, item, hWnd, win);
                  }
               }
	         }
         // Image contextual menu	      
            else if(Plot2D::curPlot()->InPlotRect(p.x,p.y) == true) 
            {
               if(!pp->bkgMenu)
                  break;

               SetCursor(LoadCursor(NULL,IDC_ARROW));
               hMenu = pp->bkgMenu;
	            ClientToScreen(hWnd,&p);
	            item = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, 0, hWnd, NULL) ;
               if(item)
                  PreProcessCommand(NULL, item, hWnd, win);
	         }     		                      
            break;
         }
      }
   }
   return(DefWindowProc( hWnd, messg, wParam, lParam ));
}
	

// Display a ".3D" file (first plane only)

void LoadAndDisplay3DDataFile(HWND hWnd, Plot *pd,  char *basePath, char *fileName)
{
   Interface itfc;
   CText varName;

	if(pd)
	{
		pd->makeCurrentPlot();
		pd->makeCurrentDimensionalPlot();
		varName = fileName;
		RemoveExtension(varName.Str());

		if(LoadData(&itfc, basePath, fileName, varName.Str(), GLOBAL) == OK)
		{ 
			varName.Format("%s[~,~,0]",varName.Str());
			Plot2D::curPlot()->setTitleText(varName.Str());
			DisplayMatrixAsImage(&itfc,varName.Str());
			SendMessageToGUI("2D Plot,LoadImage",0);
			UpdateStatusWindow(hWnd,3,Plot2D::curPlot()->statusText);
			Plot2D::curPlot()->Invalidate();
		}
	}
}


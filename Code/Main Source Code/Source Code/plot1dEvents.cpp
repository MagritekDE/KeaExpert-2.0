#define WINVER _WIN32_WINNT 
#include "stdafx.h"
#include "plot1dEvents.h"
#include <math.h>
#include "bitmap.h"
#include "command_other.h"
#include "defineWindows.h"
#include "files.h"
#include "guiMakeObjects.h"
#include "guiModifyObjectParameters.h"
#include "guiWindowClass.h"
#include "guiWindowCLI.h"
#include "guiWindowsEvents.h"
#include "interface.h"
#include "Inset.h"
#include "main.h"
#include "message.h"
#include "plot1dCLI.h"
#include "plot_dialog_1d.h"
#include "plot_load_save_1d.h"
#include "PlotWindow.h"
#include "print.h"
#include "process.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "string_utilities.h"
#include <algorithm>
#include "memoryLeak.h"

using std::find;

#pragma warning (disable: 4996) // Ignore deprecated library functions

/*****************************************************************************
*            Routines relating to the 1D user defined plot window
*
* Plot1DFunctions ................. plot1dfunc command arguments to access plot functions: 
*
*   axes font
*   border axes
*   corner axes
*   crossed axes
*   display data
*   drag plot
*   enlarge horizontal
*   enlarge vertical
*   full region
*   hide borders
*   last region
*   linear x scale
*   log x scale
*   move down
*   move left
*   move right
*   move up
*   multiplot 1*1
*   multiplot 1*2
*   multiplot 2*1
*   multiplot 2*2
*   multiplot m*n
*   overlapping plots
*   plot title
*   plot x,y labels
*   print plot
*   reduce horizontal
*   reduce vertical
*   remove all data
*   remove current data
*   remove other subplots
*   select region
*   show all plots
*   show borders
*   toggle border
*   toggle imaginary
*   toggle real
*   view one plot
*   zoom region
*
*****************************************************************************/

// Locally define functions
LRESULT CALLBACK PlotEventsProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);

// Locally define functions

Plot* namedCurrentPlot(CText& which);

// User defined plot interaction flags (this allows the user to override the
// normal plotGUI interactions and define their own).

CText gPlot1DScaleMacro = "";
CText gPlot1DShiftMacro = "";
CText gPlot1DDblClickMacro = "";

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


LRESULT CALLBACK PlotEventsProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
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
   PlotWindow1D *pp = (PlotWindow1D*)obj->data;

//	if(messg != 0x111 && messg != 0x20)
 //     TextMessage("mesg = %X\n",messg);

	switch(messg)
	{	

      //case WM_TIMER: 
      //{
      //   if(pp->plotChanged)
      //   {       
      //       TextMessage("Changed count = %d\n",++chCnt);

      //      pp->DisplayAll(false);
      //   }
      //   break;
      //}

      case(WM_USER): 
      {
         pp->DisplayAll(true);
         break;
      }

      case(WM_DROPFILES):
      {
         CText path,file,ext;

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
               cmd.Format("%s(\"%s\",\"%s\",\"%s\")",win->dragNDropProc.Str(),spath,file.Str(),ext.Str());
               ProcessMacroStr(0,win,NULL,cmd.Str(),"",proc, win->macroName, win->macroPath);
            }
            else
               return(0);
         }
         DragFinish(hDrop);
         return(0);
      }

      case(WM_MOUSEACTIVATE): // User has clicked on window - update title, menu and toolbar
      {
			CText txt;
			if(win->titleUpdate)
         {
			   txt.Format("1D Plot (%hd)",win->nr);
			   SetWindowText(win->hWnd,txt.Str());	
         }

         SelectFixedControls(win, obj);
         if(win && !win->keepInFront)
            ChangeGUIParent(GetParent(hWnd));
         SetFocus(hWnd);
         break;
      }

		case(WM_PAINT): // Redraw plot when damaged
		{
	      PAINTSTRUCT p;
		   RECT pr;
         HDC hdc;
         MSG msg;
			static int cnt = 0;

		   GetClientRect(hWnd,&pr);
	     // Draw each plot in the 1D array 
         if(wParam == 0)
            hdc = BeginPaint(hWnd, &p ); 
         else 
            hdc = (HDC)wParam; // Allow for WM_PRINTCLIENT call
 
			//printf("Waiting for critical section 1\n");

         while(pp->inCriticalSection())
           PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);

         if(TryEnterCriticalSection(&cs1DPlot))
         {
            pp->incCriticalSectionLevel();
			//	printf("In critical section plot PAINT\n");

			 // Write to bitmap before displaying
            if(pp->updatePlots() && (pp->plotList().size() > 0) && pp->plotList()[0]->updatePlots())
            {
				   if(pp->useBackingStore)
					{
						HDC hdcMem = CreateCompatibleDC(hdc);
						SelectObject(hdcMem,pp->bitmap);
					//	TextMessage("Repainting 1D plot %d\n",cnt++);
						for(Plot* p : pp->plotList())
						{
							p->Display(hWnd,hdcMem);
						}
						BitBlt(hdc,pr.left,pr.top,pr.right-pr.left,pr.bottom-pr.top,hdcMem,pr.left,pr.top,SRCCOPY);
						DeleteDC(hdcMem);
					}
				   else // Just draw to screen directly
				   {
					   for(Plot* p : pp->plotList())
					   {
						   p->Display(hWnd,hdc);
					   }
				   }
            }

            LeaveCriticalSection(&cs1DPlot);
            pp->decCriticalSectionLevel();
         //   printf("Left critical section PAINT\n");
         }

         if(wParam == 0) // Allow for WM_PRINTCLIENT call
	         EndPaint(hWnd, &p );	// stop painting  

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
// What happens if the last plot in the window is not the current plot
         // but the current plot is in this window?

         pp->DrawPlotTitle();

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
         MSG msg;

         while(pp->inCriticalSection())
            PeekMessage(&msg,NULL,NULL,NULL,PM_REMOVE);

         if(TryEnterCriticalSection(&cs1DPlot))
         {
		   //   printf("In critical section WM_SIZE\n");
		   	pp->incCriticalSectionLevel();

			// Update size of each 1D subwindow
				for(Plot* p : pp->plotList())
				{
					 p->ResizePlotRegion(hWnd); // Update plot size parameters
				}

			// Regenerate the backing store bitmap
				if(pp->useBackingStore)
				{   
					RECT r;
					GetClientRect(hWnd,&r);
					if(pp->bitmap) 
						DeleteObject(pp->bitmap);
					HDC hdc = GetDC(hWnd);
               long newWidth;
					GenerateBitMap(r.right-r.left,r.bottom-r.top, &pp->bitmap, hdc, newWidth);
					ReleaseDC(hWnd,hdc);
				}

		  // Redraw window now all elements have been resized
				MyInvalidateRect(hWnd,NULL,false);

		      LeaveCriticalSection(&cs1DPlot);
            pp->decCriticalSectionLevel();
		   //   printf("Leaving critical section WM_SIZE\n");
         }
         
		   break;
		}

   // Check for show.hide gui windows
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
   // 3. Select a new data trace
   // 4. In move mode so remove region rectangle

		case(WM_LBUTTONDOWN) :
		{
		   short x,y;
         bool newPlot = false;

	  // Record position of cursor when mouse clicked
         x = LOWORD(lParam);
         y = HIWORD(lParam);

     // Find out which plot region has been selected.
     // If its a new region then set window title,
     // hide old selection rectangle, set current 1D plot,
     // redraw window and then return.
			for(Plot* p : pp->plotList())
			{
            if(p && p->InRegion(x,y))
            {  
					if(!(Plot1D::curPlot()) || (Plot1D::curPlot() != p) || (Plot::curPlot() != p))
               {
						if(!(Plot1D::curPlot()) || !(Plot::curPlot()))
						{
							p->makeCurrentPlot();
							p->makeCurrentDimensionalPlot();
						}
						Plot1D::curPlot()->setPlotState("newregion");
	               Plot1D::curPlot()->HideSelectionRectangle(NULL);  // Hide old selection rect.  
                  MyInvalidateRect(Plot1D::curPlot()->win, NULL, false); // Remove old indent
						p->makeCurrentDimensionalPlot();
                  UpdateStatusWindow(hWnd,3,p->statusText);
                  MyUpdateWindow(Plot1D::curPlot()->win);
                  MyInvalidateRect(hWnd,NULL,false); // Add new indent
                  MyUpdateWindow(hWnd);
						p->makeCurrentPlot(); // Make this the current window
                  newPlot = true;
               }

            // Update the status window text
					Plot1D* cur1DPlot = Plot1D::curPlot();
					cur1DPlot->updateStatusWindowForZoom(hWnd);

            // Update menu checks
               cur1DPlot->initialiseMenuChecks("selectplot");

            // Set current trace (either the selected one or the first one) 
               Trace* oldTrace = cur1DPlot->curTrace();
               Trace* newTrace;
					if(!(newTrace = cur1DPlot->CursorOnDataLine(x,y)))
               {
                  if(cur1DPlot->hasNoCurTrace())
							cur1DPlot->setCurTrace();
               }
               else
               {
                  cur1DPlot->setCurTrace(newTrace);
               }

            // Update the status bar with the new trace information if new trace
					if(cur1DPlot->curTrace())
               {
                  if(cur1DPlot->curTrace() != oldTrace)
                    pp->UpdateStatusBar();
					}

            // This is now the current plot 
					cur1DPlot->makeCurrentPlot();

            // If this is a new plot or a new trace then tell all macros about this change
					if(newPlot)
						SendMessageToGUI("1D Plot,SelectPlot",0); 
               if(cur1DPlot->curTrace() && cur1DPlot->curTrace() != oldTrace)
               {
                  if(cur1DPlot->showLegend()) // Make sure the legend is updated if new trace selected.
                     cur1DPlot->Invalidate();
                  SendMessageToGUI("1D Plot,SelectTrace",0); 
               }
            }
         }


     // If the plot region is not new then ...
     // Select a rectangle if in SELECT_RECT mode or the control key is pressed
         if((pp->mouseMode == SELECT_RECT) | (wParam & MK_CONTROL))
         {
             MouseMode curMode = pp->mouseMode;
             pp->mouseMode = SELECT_RECT;
             SetCursor(SquareCursor);     
	          Plot1D::curPlot()->SelectRegion(hWnd,x,y);

            if(wParam & MK_CONTROL) // Zoom as well if control key is pressed
            {
               Interface itfc;
               itfc.win = win;
               Plot1DFunctions(&itfc,"\"zoom region\"");
	         }
            pp->mouseMode = curMode;
         }
         // To be implemented - add a userdefine option
         //if(pp->mouseMode == USER_DEFINED)
         //{
         //   Interface itfc;
         //   itfc.win = win;
         //   Plot1DFunctions(&itfc,"\"zoom region\"");
         //}

     // Show data values if in SHOW_DATA mode
	      else if(pp->mouseMode == SHOW_DATA && !(wParam & MK_CONTROL))
	      {
             MouseMode curMode = pp->mouseMode;
             pp->mouseMode = SHOW_DATA;
             SetCursor(CrossCursor);     

            Plot1D::curPlot()->GetDataAtCursor(hWnd, x, y);

             pp->mouseMode = curMode;

	   //     	Trace *di;
    //        extern bool gScrollWheelEvent;
    //        gScrollWheelEvent = false;

    //     // Check for change in current data trace
				Plot1D* cur1DPlot = Plot1D::curPlot();
		  //    if(di = cur1DPlot->CursorOnDataLine(x,y))
		  //    {
				//	cur1DPlot->setCurTrace(di);
    //        }  
            if(cur1DPlot->isOffset())
               UpdateStatusWindow(hWnd,1,"Left button : Hold for data display; Right button : remove offset");
            else
               UpdateStatusWindow(hWnd,1,"Left button : Hold for data display; Right button : apply offset");       
         }
         
      // Allow user to drag the plot if in MOVE_PLOT mode      
         else if(pp->mouseMode == MOVE_PLOT)
         {
				Plot1D::curPlot()->beginToDragMovePlot(hWnd,x,y);
				oldx = x;
				oldy = y;
			}
		   break;
		}

   // When double clicking on a trace display the trace parameters macro
      case(WM_LBUTTONDBLCLK):
      { 
			long x,y;

	  // Note location of cursor in client coordinates
         x = LOWORD(lParam);  // horizontal position of cursor 
         y = HIWORD(lParam);  // vertical position of cursor 

	      Trace *di;
			Plot1D* cur1DPlot = Plot1D::curPlot();
			
			if(cur1DPlot->limitfunc)
			   break;

         if (di = cur1DPlot->CursorOnDataLine(x, y))
         {
            Interface itfc;
            cur1DPlot->setCurTrace(di);

            if (gPlot1DDblClickMacro != "")
            {
               float xd = cur1DPlot->curXAxis()->scrnToData(x);
               float yd = cur1DPlot->curYAxis()->scrnToData(y);
               cur1DPlot->setCurTrace(di);
               CText cmd;
               cmd.Format("%s(%f, %f)", gPlot1DDblClickMacro.Str(), xd, yd);
             //  ProcessMacroStr(0, NULL, NULL, cmd.Str(), "", "", gPlot1DDblClickMacro.Str(), "");
               ProcessMacroStr(0, NULL, NULL, cmd.Str(), "", "", "", "");
            }
            else
            {
               if (!FindGUIWindow("ModifyTraceParameters"))
                  CheckForMacroProcedure(&itfc, "ModifyTraceParameters.mac", "");
            }
         }
         break;
      }

     
   // User is moving the mouse. Actions:
   // 1. Set the cursor based on the mode ...
   // 2. Left button down & MOVE_PLOT: Move the data
   // 3. Left button down & SHOW_DATA: Show the data under cursor
   // 4. In move mode so remove region rectangle

      case(WM_MOUSELEAVE):
      {
         Plot1D* curPlot = Plot1D::curPlot();
         if(curPlot)
         {
            HDC hdc = GetDC(hWnd);
            Plot1D::curPlot()->HideDataCursor(hdc);
            ReleaseDC(hWnd,hdc);
         }
         else
         {
          //  printf("curPlot not defined! (MOUSE_LEAVE)\n");
         }
         break;
      }

      case(WM_MOUSEMOVE): 
      { 
			long x,y;
         tme.cbSize = sizeof(TRACKMOUSEEVENT);
         tme.hwndTrack = hWnd;
         tme.dwFlags = TME_LEAVE;
         tme.dwHoverTime = HOVER_DEFAULT;
         TrackMouseEvent(&tme);

	  // Note location of cursor in client coordinates
         x = LOWORD(lParam);  // horizontal position of cursor 
         y = HIWORD(lParam);  // vertical position of cursor 

     // Set the cursor based on the mode   
			Plot1D* cur1DPlot = Plot1D::curPlot();
         if(cur1DPlot && cur1DPlot->InPlotRect(x,y))
         {
             if(pp->mouseMode == MOVE_PLOT)
               SetCursor(LoadCursor(NULL,IDC_SIZEALL));
            else if(pp->mouseMode == SHOW_DATA)
               SetCursor(CrossCursor);
            else if(pp->mouseMode == SELECT_RECT)
               SetCursor(SquareCursor);     
         }
         else
         {
            SetCursor(LoadCursor(NULL,IDC_ARROW));
         }
            
     // Left mouse button is down while cursor is moving
         if(wParam & MK_LBUTTON)
         {
            if(cur1DPlot)
            {
               if(pp->mouseMode == MOVE_PLOT) // Move trace data if in move mode
               {
                  if(oldx != -1)
                  {
							if (cur1DPlot->insetBeingMoved())
							{	
								float dx = x - oldx;
								float dy = y - oldy;
								cur1DPlot->insetBeingMoved()->shift(dx, dy);
								cur1DPlot->Invalidate();
							}							
							else // We are moving the traces
							{
                        if (gPlot1DShiftMacro != "")
                        {
                           float xd = cur1DPlot->curXAxis()->scrnToData(x);
                           float oldxd = cur1DPlot->curXAxis()->scrnToData(oldx);
                           float yd = cur1DPlot->curYAxis()->scrnToData(y);
                           float oldyd = cur1DPlot->curYAxis()->scrnToData(oldy);
                           CText cmd;
                           cmd.Format("%s(%f, %f, %f, %f)", gPlot1DShiftMacro.Str(), xd, yd, oldxd, oldyd);
                           ProcessMacroStr(0, NULL, NULL, cmd.Str(), "", "", gPlot1DShiftMacro.Str(), "");
                        }
                        else
                        {
                           cur1DPlot->plotBeingMoved = true;
                           cur1DPlot->plotOldX = oldx;
                           cur1DPlot->plotOldY = oldy;
                           cur1DPlot->plotNewX = x;
                           cur1DPlot->plotNewY = y;
                           cur1DPlot->Invalidate();
                        }
							}
                  }  
                  oldx = x;
						oldy = y;
		         }
            }
	      }

         break;
      }

   // User has released the left button:
   //   1. Hide cursors
   //   2. Update status window text
   //   3. Reset old cursor position
      case(WM_LBUTTONUP): 
      {	
			Plot1D* cur1DPlot = 0; 
         if(cur1DPlot = Plot1D::curPlot())
         {
		      HDC hdc = GetDC(hWnd);      
	         cur1DPlot->HideDataCursor(hdc);
	         if(pp->mouseMode == SHOW_DATA)
               UpdateStatusWindow(hWnd,1,"Left button : press for data display");
            else if(pp->mouseMode == SELECT_RECT)
               UpdateStatusWindow(hWnd,1,"Left button : press to select rectangle");
            
	         ReleaseDC(hWnd,hdc);
	         oldx = -1;
				oldy = -1;
				cur1DPlot->rectSelected = false;
				cur1DPlot->stopShifting();
            break;
         }
      }

   // User has pressed the right button. 
   //   1. First set current region
   // If left button pressed:
   //   2. Apply offset to data coordinate
   // Otherwise display a user defined contextual menu:
   //   1. Axes menu
   //   2. Title menu
   //   3. Labels menu
   //   4. Axes font menu
   //   5. Trace menu
   //   6. Background menu
      case(WM_RBUTTONDOWN): // Select contextual menu
		{
			Plot1D* cur1DPlot = 0;
         if(cur1DPlot = Plot1D::curPlot())
         {
		      POINT p;
            short whichTxt;

				if(cur1DPlot->limitfunc)
					break;
           
         // Get mouse coordinates
            p.x = LOWORD(lParam);
            p.y = HIWORD(lParam); 

         // Set the current region // TODO - lacks some functionality here w.r.t WM_LBUTTONDOWN
				for(Plot* plot : pp->plotList())
			   {
               if(plot->InRegion(p.x,p.y))
               {
						plot->makeCurrentPlot();
						plot->makeCurrentDimensionalPlot();
                  break;
               }
            }

         // Update the current trace
            Trace *di;
            if(di = cur1DPlot->CursorOnDataLine(p.x,p.y))
				   cur1DPlot->setCurTrace(di);

      //   // If left button is pressed and data cursor is visible a right click applies an offset
      //      if(wParam & MK_LBUTTON)
      //      {
	   	 //     if(pp->mouseMode == SHOW_DATA) 
		    //     {		 
	     //          if(cur1DPlot->InPlotRect(p.x,p.y))
      //            {
		    //           HDC hdc = GetDC(hWnd);
	    //              cur1DPlot->OffsetData(hdc,p.x,p.y);
      //               ReleaseDC(hWnd,hdc);
      //            }
      //   
	     //          if(cur1DPlot->isOffset())
      //            {
      //               AddCharToString(cur1DPlot->statusText,'O');
      //               UpdateStatusWindow(hWnd,3,cur1DPlot->statusText);
      //               UpdateStatusWindow(hWnd,1,"Left button : Hold for data display; Right button : remove offset");
      //            }
      //            else
      //            {
      //               RemoveCharFromString(cur1DPlot->statusText,'O');
      //               UpdateStatusWindow(hWnd,3,cur1DPlot->statusText);
      //               UpdateStatusWindow(hWnd,1,"Left button : Hold for data display; Right button : apply offset");
      //            }  
						//TextMessage("Right button down event\n");
		    //        break;
		    //     }
		    //  }
     		   
       // Check for clicking over certain regions (i.e. data, axes, text)  
       // and display appropriate contextual menu via WM_COMMAND

            HMENU hMenu;
            short item;

         // Axes contextual menu
            if(cur1DPlot->CursorOnAxes(p.x,p.y) == OK) 
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
            else if((whichTxt = cur1DPlot->CursorInText(p.x,p.y)) != 0) // See if text has been selected
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

         // Trace contextual menu
            else if(cur1DPlot->CursorOnDataLine(p.x,p.y)) // Modify trace or symbols
            {
               if(!pp->traceMenu)
                  break;
               SetCursor(LoadCursor(NULL,IDC_ARROW));
               hMenu = pp->traceMenu;            
               ClientToScreen(hWnd,&p);
               item = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, 0, hWnd, NULL) ;
               if(item)
                  PreProcessCommand(NULL, item, hWnd, win);
	         }
   	  
	     // Background contextual 
            else if(cur1DPlot->CursorInBackGround(p.x,p.y) == OK) 
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


/********************************************************
*    Functions specific to the 1D plot which are called
*    from the GUI
********************************************************/

int Plot1DFunctions(Interface* itfc ,char args[])
{
   CText func;
   short r;

	Plot1D* cur1DPlot = 0;
   if(!(cur1DPlot = Plot1D::curPlot()))
   {
      ErrorMessage("1D Plot window not defined\n");
      return(ERR);
   }

   if((r = ArgScan(itfc,args,0,"function name","e","t",&func)) < 0)
      return(r); 

// Make the first plot in the current window the current plot
// if the current window doesn't contain the current plot
// This is because the following functions are usually called from  
// the gui and we expect them to apply to the window they were
// called from. (ANY EXCEPTIONS?)
 
   HWND hWnd = cur1DPlot->win;
   PlotWindow *pp = cur1DPlot->plotParent;

   if(itfc->win && (itfc->win->hWnd != pp->obj->hwndParent))
   {
      ObjectData *obj = itfc->win->widgets.findByType(PLOTWINDOW);
      if(obj)
      {
         pp = (PlotWindow1D*)obj->data;
         if(pp)
         {
				PlotList& pl = pp->plotList();
				PlotListIterator plit = find(pl.begin(), pl.end(), cur1DPlot);

				if(plit == pl.end()) // Current plot not found
            {
          // Set current plot to first region in plot parent
               MyInvalidateRect(cur1DPlot->win,NULL,false); // Remove old indent
					pp->makeCurrentPlot();
					pp->makeCurrentDimensionalPlot();
					cur1DPlot = Plot1D::curPlot();
               MyInvalidateRect(cur1DPlot->win,NULL,false); // Draw new indent
           // Set current trace (either the first or the selected one) 
               if(cur1DPlot->curTrace() && !cur1DPlot->InDataSet(cur1DPlot->curTrace()))
               {
						if(cur1DPlot->hasNoCurTrace())
                  {
							cur1DPlot->setCurTrace();
                  }
               }
            }
         }
      }
   }

// Some routines below use curPlot not cur1DPlot
	Plot1D::curPlot()->makeCurrentPlot();

// Recalculate after finding correct plot window
   hWnd = Plot1D::curPlot()->win;
	HDC hdc = GetDC(hWnd);
   pp = static_cast<PlotWindow1D*>(Plot1D::curPlot()->plotParent);

   if(func == "axes font")
   {
		if (pp->StylePlotAxisTicksLabels())
		{
         pp->curPlot()->Invalidate();	   
      }
	}
   else if(func == "border axes")
   {
	   pp->curPlot()->axesMode = PLOT_AXES_CORNER+1;
		WinData* parent = pp->obj->winParent;
		const char* const tbn = pp->toolbarName();
      parent->setToolBarItemCheck(tbn, "corner_axes", false);
      parent->setToolBarItemCheck(tbn, "border_axes", true);
      parent->setToolBarItemCheck(tbn, "crossed_axes", false);
      pp->curPlot()->Invalidate();
   }
   else if(func == "copy all plots")
   {
      pp->CopyPlotsToClipboard(true);
      pp->CopyCurrentPlot();
   }
   else if(func == "copy plot")
   {
      pp->CopyPlotsToClipboard(false);
      pp->CopyCurrentPlot();
   }
   else if(func == "corner axes")
   {
		pp->curPlot()->axesMode = PLOT_AXES_CORNER;
		WinData* parent = pp->obj->winParent;
		const char* const tbn = pp->toolbarName();
      parent->setToolBarItemCheck(tbn, "corner_axes", true);
      parent->setToolBarItemCheck(tbn, "border_axes", false);
      parent->setToolBarItemCheck(tbn, "crossed_axes", false);
      pp->curPlot()->Invalidate();
   }
   else if(func == "crossed axes")
   {
		pp->curPlot()->axesMode = PLOT_AXES_CORNER+2;
		WinData* parent = pp->obj->winParent;
		const char* const tbn = pp->toolbarName();
      parent->setToolBarItemCheck(tbn, "corner_axes", false);
      parent->setToolBarItemCheck(tbn, "border_axes", false);
      parent->setToolBarItemCheck(tbn, "crossed_axes", true);
      pp->curPlot()->Invalidate();
   }
   else if(func == "cut plot")
   {
      pp->CopyCurrentPlot();
      pp->ClearCurrentPlot();  
	   pp->curPlot()->Invalidate();  
   }
   else if(func == "delete all plots")
   {
      CText name = "untitled";
      pp->fileNumber = rootWin->GetNextFileNumber(pp->obj,name);
      pp->fileName(name);
      pp->modified(false);
	   pp->MakeMultiPlot(1,1);
      pp->curPlot()->initialiseMenuChecks("clear");
	   pp->curPlot()->Invalidate();  
   }
   else if(func == "display data")
   {
	   pp->curPlot()->HideSelectionRectangle(hdc);
      gResetCursor = CrossCursor;
      SetCursor(gResetCursor);
      pp->mouseMode = SHOW_DATA;
		UpdateStatusWindow(hWnd,1,"Left button : Hold for data display");
		
		WinData* parent = pp->obj->winParent;
		const char* const m = pp->menuName();
		const char* const tbn = pp->toolbarName();
      parent->setMenuItemCheck(m,"drag_plot",false);
      parent->setMenuItemCheck(m,"display_data",true);
      parent->setMenuItemCheck(m,"select_region",false);
      parent->setToolBarItemCheck(tbn,"drag_plot", false);
      parent->setToolBarItemCheck(tbn,"display_data", true);
      parent->setToolBarItemCheck(tbn,"select_region", false);
   }
   else if(func == "drag plot")
   {
	   Plot1D::curPlot()->HideDataCursor(hdc);
      gResetCursor = LoadCursor(NULL,IDC_SIZEALL);
      SetCursor(gResetCursor);
      pp->mouseMode = MOVE_PLOT;

		WinData* parent = pp->obj->winParent;
		const char* const m = pp->menuName();
		const char* const tbn = pp->toolbarName();
      parent->setMenuItemCheck(m,"drag_plot",true);
      parent->setMenuItemCheck(m,"display_data",false);
      parent->setMenuItemCheck(m,"select_region",false);
      parent->setToolBarItemCheck(tbn, "drag_plot", true);
      parent->setToolBarItemCheck(tbn, "display_data", false);
      parent->setToolBarItemCheck(tbn, "select_region", false);
   }
   else if(func == "enlarge horizontal")
   {
		Plot1D* curPlot = Plot1D::curPlot();
      curPlot->HideSelectionRectangle(hdc);  
      curPlot->plotCmdScaled = true;
      curPlot->scaleDirection = ID_ENLARGE_HORIZ;
      curPlot->Invalidate();
   }
   else if(func == "enlarge vertical")
	{
		Plot1D* curPlot = Plot1D::curPlot();
      curPlot->HideSelectionRectangle(hdc);  
      curPlot->plotCmdScaled = true;
      curPlot->scaleDirection = ID_ENLARGE_VERT;
      curPlot->Invalidate();
   }
   else if(func == "full region")
   {
		dynamic_cast<Plot1D*>(pp->curPlot())->setOverRideAutoRange(false);
      pp->curPlot()->curXAxis()->setAutorange(true);
      pp->curPlot()->curYAxis()->setAutorange(true); 
		if (pp->curPlot()->syncAxes())
		{
			pp->curPlot()->otherYAxis()->setAutorange(true);	
		}		
      pp->curPlot()->resetZoomCount(); 
      pp->curPlot()->HideSelectionRectangle(hdc);      
      Plot1D::curPlot()->ResetZoomPoint();
		pp->curPlot()->updateStatusWindowForZoom(hWnd);
      pp->curPlot()->Invalidate();	
   }
   else if(func == "hide borders")
   {
      pp->showLabels = false;
	   pp->curPlot()->Invalidate();     	
   }
   else if(func == "last region")
   {
		if (!pp->curPlot()->lastRegion(hWnd))
		{
			MessageDialog(prospaWin,MB_ICONINFORMATION,"Message","No regions in 1D history list");
      }	
		pp->curPlot()->Invalidate();
   }
   else if(func == "linear x scale")
   {
		pp->curPlot()->setAxisMapping(PLOT_LINEAR_X);
		for(Axis* axis : pp->curPlot()->axisList())
		{
			axis->setAutorange(true);
		}
		WinData* parent = pp->obj->winParent;
		const char* const tbn = pp->toolbarName();
      parent->setToolBarItemCheck(tbn, "linear_axis", true);
      parent->setToolBarItemCheck(tbn, "log_axis", false);
      pp->curPlot()->Invalidate();
   }
   else if(func == "load plot")
   {
       if(Load1DPlotDialog(hWnd) != OK)
          return(OK);
          
       SetCursor(LoadCursor(NULL,IDC_WAIT));
		 if(pp->LoadPlots(Plot1D::currFilePath, Plot1D::currFileName) == OK)
       {
          SendMessageToGUI("1D Plot,LoadPlot",0);
          UpdateStatusWindow(hWnd,1,"Left button : select a region");
          pp->curPlot()->statusText[0] = '\0';
          UpdateStatusWindow(hWnd,3,Plot1D::curPlot()->statusText);
          Plot1D::curPlot()->ResetZoomPoint();  
          Plot1D::curPlot()->setCurTrace();
			 pp->curPlot()->makeCurrentPlot();
          CText name;
          name = pp->curPlot()->getFileName();
          name.RemoveExtension(); 
          pp->fileNumber = rootWin->GetNextFileNumber(pp->obj,name);
          pp->fileName(name);
          pp->modified(false);
          RemoveCharFromString(pp->curPlot()->statusText,'V');
          UpdateStatusWindow(hWnd,3,pp->curPlot()->statusText);
			 pp->clearPlotData(pp->getSavedPlotList());
          pp->curPlot()->initialiseMenuChecks("load");
       }
       MyInvalidateRect(hWnd,NULL,false);
       SetCursor(LoadCursor(NULL,IDC_ARROW));
   }
   else if(func == "log x scale")
   {
		Plot1D* curPlot = Plot1D::curPlot();
	   if(curPlot->OKForLog('x'))
	   {
		   curPlot->setAxisMapping(PLOT_LOG_X);
   		for(Axis* axis : curPlot->axisList())
			{
				axis->setAutorange(true);
			}
			WinData* parent = pp->obj->winParent;
			const char* const tbn = pp->toolbarName();
         parent->setToolBarItemCheck(tbn, "linear_axis", false);
         parent->setToolBarItemCheck(tbn, "log_axis", true);
		}
		else
		{
		   MessageDialog(prospaWin,MB_ICONERROR,"Unable to complete command","The current data set has zero or negative x values");
		}
      curPlot->Invalidate();
   }
   else if(func == "move up")
   {
		Plot1D* curPlot = Plot1D::curPlot();
      curPlot->plotShifted = true;
      curPlot->shiftDirection = ID_SHIFT_UP;
      curPlot->Invalidate();
   }
   else if(func == "move down")
	{
		Plot1D* curPlot = Plot1D::curPlot();
      curPlot->plotShifted = true;
      curPlot->shiftDirection = ID_SHIFT_DOWN;
      curPlot->Invalidate();
   }
   else if(func == "move left")
	{
		Plot1D* curPlot = Plot1D::curPlot();
      curPlot->plotShifted = true;
      curPlot->shiftDirection = ID_SHIFT_RIGHT;
      curPlot->Invalidate();
   }
   else if(func == "move right")
	{
		Plot1D* curPlot = Plot1D::curPlot();
      curPlot->plotShifted = true;
      curPlot->shiftDirection = ID_SHIFT_LEFT;
      curPlot->Invalidate();
   }
   else if(func == "multiplot 1*1")
   {
       pp->AdjustMultiPlotSize(1,1);  
	    MyInvalidateRect(pp->hWnd,0,false);
   }
   else if(func == "multiplot 1*2")
   {
       pp->AdjustMultiPlotSize(2,1);  
	    MyInvalidateRect(pp->hWnd,0,false);
   }
   else if(func == "multiplot 2*1")
   {
       pp->AdjustMultiPlotSize(1,2);  
	    MyInvalidateRect(pp->hWnd,0,false);

   }
   else if(func == "multiplot 2*2")
   {
       pp->AdjustMultiPlotSize(2,2);  
	    MyInvalidateRect(pp->hWnd,0,false);
   }
   else if(func == "multiplot m*n")
   {
       gMultiPlotYCells = pp->rows;
       gMultiPlotXCells = pp->cols;	   
   	 if(DialogBox(prospaInstance,"MULTIPLOTDLG",hWnd,MultiPlotDlgProc))
       {
          ReleaseDC(pp->curPlot()->win,hdc);
          return(OK);	  
       }
       pp->AdjustMultiPlotSize(gMultiPlotXCells,gMultiPlotYCells);             
       pp->curPlot()->Invalidate();	     	           
   }	
   else if(func == "overlapping plots")
   {
		Plot1D* curPlot = Plot1D::curPlot();
      long ID = itfc->objID;
		HWND hWnd = itfc->obj ? itfc->obj->hWnd : curPlot->plotParent->hWnd;

		WinData* parent = pp->obj->winParent;
		const char* const m = pp->menuName();
		const char* const tbn = pp->toolbarName();
      if(curPlot->displayHold)
      {
         curPlot->displayHold = false;
         parent->setMenuItemCheck(m,"hold",false);
         parent->setToolBarItemCheck(tbn, "hold", false);
      }
      else
      {
         curPlot->displayHold = true;
         parent->setMenuItemCheck(m,"hold",true);
         parent->setToolBarItemCheck(tbn, "hold", true);
      }
      curPlot->Invalidate();	  
   } 	
	else if(func == "paste into plot")
	{
		pp->PasteSavedPlotInto();
		pp->curPlot()->makeCurrentDimensionalPlot();
	   pp->curPlot()->Invalidate(); 
   }
   else if(func == "paste plot")
   {
      pp->PasteSavedPlot();
		Plot::curPlot()->makeCurrentDimensionalPlot();
	   pp->curPlot()->Invalidate(); 
   }
   else if(func == "plot title")
   {
		if (pp->StylePlotTitles())
		{
         pp->curPlot()->Invalidate();
      }
   }
   else if(func == "plot x,y labels")
   {
		if (pp->StylePlotAxisLabels())
		{
         pp->curPlot()->Invalidate();	
      }
   }
   else if(func == "print plot")
   {
      if(DialogBox(prospaInstance,"PRINTDLG",hWnd,PrintDlgProc))
         PrintDisplay(pp);
      HWND w = cur1DPlot->plotParent->obj->winParent->hWnd;
      if(IsWindowEnabled(w) ==FALSE)
         EnableWindow(w,TRUE);
   }
   else if(func == "reduce horizontal")
   {
		Plot1D* curPlot = Plot1D::curPlot();
      curPlot->HideSelectionRectangle(hdc);  	        
      curPlot->EnlargePlotCentred(ID_REDUCE_HORIZ);
      curPlot->Invalidate();
   }
   else if(func == "reduce vertical")
	{
		Plot1D* curPlot = Plot1D::curPlot();
      curPlot->HideSelectionRectangle(hdc);  	        
      curPlot->EnlargePlotCentred(ID_REDUCE_VERT);
      curPlot->Invalidate();
   }
   else if(func == "remove all data")
   {
      RECT r;
      Plot1D::curPlot()->plotParent->ClearCurrentPlot();
      GetClientRect(pp->curPlot()->win,&r);
      long width = r.right-r.left+1;
      long height = r.bottom-r.top+1;
      SendMessage(pp->curPlot()->win,WM_SIZE,(WPARAM)SIZE_RESTORED,(LPARAM)(width+(height<<16)));       
   }
   else if(func == "remove current data")
	{
		Plot1D* curPlot = Plot1D::curPlot();
		curPlot->CloseCurrentData("current");
      curPlot->Invalidate();
   }
   else if(func == "remove other subplots")
   {
      RECT r;
      pp->RemoveAllButCurrentPlot();
	   GetClientRect(pp->curPlot()->win,&r);
      long width = r.right-r.left+1;
	   long height = r.bottom-r.top+1;
	   SendMessage(pp->curPlot()->win,WM_SIZE,(WPARAM)SIZE_RESTORED,(LPARAM)(width+(height<<16)));       
	}	
   else if(func == "save as image")
	{
		Plot1D* curPlot = Plot1D::curPlot();
      if(curPlot->hasNoCurTrace())
      {
         MessageDialog(prospaWin,MB_ICONERROR,"Error","No 1D plot data to save");
         return(OK);
      }  
      curPlot->plotParent->SaveAsImage(NULL);
      return(OK);
   } 
   else if(func == "save plot")
	{
		Plot1D* curPlot = Plot1D::curPlot();
      if(curPlot->hasNoCurTrace())
      {
         MessageDialog(prospaWin,MB_ICONERROR,"Error","No 1D plot data to save");
         return(OK);
      }
      long version;
      if(Save1DPlotDialog(hWnd, pp->curPlot()->getFilePath(), pp->curPlot()->getFileName(), version) != OK)
         return(OK);
      long bak = plot1DSaveVersion;
      version =  fileVersionNumberToConstant(version,1);
      plot1DSaveVersion =  version;
      SetCursor(LoadCursor(NULL,IDC_WAIT));
		PlotWindow1D* pp1 = static_cast<PlotWindow1D*>(curPlot->plotParent);
      pp1->SavePlots(curPlot->getFilePath(), curPlot->getFileName(),-1,-1);     
      SetCursor(LoadCursor(NULL,IDC_ARROW));
      plot1DSaveVersion =  bak;
		pp1->modified(false);
      return(OK);
   }
   else if(func == "select region")
   {
	   pp->curPlot()->HideDataCursor(hdc);
      pp->mouseMode = SELECT_RECT;
      gResetCursor = SquareCursor;
      SetCursor(gResetCursor); 
      UpdateStatusWindow(hWnd,1,"Left button : select a region");

		WinData* parent = pp->obj->winParent;
		const char* const m = pp->menuName();
		const char* const tbn = pp->toolbarName();
      parent->setMenuItemCheck(m,"drag_plot",false);
      parent->setMenuItemCheck(m,"display_data",false);
      parent->setMenuItemCheck(m,"select_region",true);
      parent->setToolBarItemCheck(tbn, "drag_plot", false);
      parent->setToolBarItemCheck(tbn, "display_data", false);
      parent->setToolBarItemCheck(tbn, "select_region", true);
   }
   else if(func == "show all plots")
   {
		pp->ViewFullPlot();
   }
   else if(func == "show borders")
   {
      pp->showLabels = true;
      pp->obj->winParent->setMenuItemCheck(pp->menuName(),"show_border",true);

	   pp->curPlot()->Invalidate();     	
   }
   /*else if(func == "swap plots")
   {
      pp->SwapPlots();
	   pp->curPlot()->Invalidate(); 
   } */
	else if (func == "toggle antialiasing")
	{
      Plot1D* curPlot = Plot1D::curPlot();
		pp->curPlot()->setAntiAliasing(!pp->curPlot()->isAntiAliasing());
      pp->obj->winParent->setMenuItemCheck(pp->menuName(),"antialiasing",curPlot->isAntiAliasing());
	   pp->curPlot()->Invalidate();     	
	}
   else if(func == "toggle border")
   {
      pp->showLabels = !pp->showLabels;
      pp->obj->winParent->setMenuItemCheck(pp->menuName(),"show_border",pp->showLabels);
	   pp->curPlot()->Invalidate();     	
   }
   else if(func == "toggle fixed range")
   {
      pp->curPlot()->setOverRideAutoRange(!pp->curPlot()->getOverRideAutoRange());
      pp->obj->winParent->setMenuItemCheck(pp->menuName(),"fixed_range",pp->curPlot()->getOverRideAutoRange());
	   pp->curPlot()->Invalidate();     	
   }
   else if(func == "toggle imaginary")
   {
		Plot1D* curPlot = Plot1D::curPlot();
      if(curPlot->display1DComplex & SHOW_MAGNITUDE)
         pp->obj->winParent->setMenuItemEnable(pp->menuName(),"show_imaginary",true);

      if(curPlot->display1DComplex & SHOW_IMAGINARY)
	      curPlot->display1DComplex = curPlot->display1DComplex & ~SHOW_IMAGINARY;
	   else
	      curPlot->display1DComplex = curPlot->display1DComplex | SHOW_IMAGINARY;

      pp->obj->winParent->setMenuItemCheck(pp->menuName(),"show_imaginary",(curPlot->display1DComplex & SHOW_IMAGINARY));
      pp->obj->winParent->setMenuItemCheck(pp->menuName(),"show_real",(curPlot->display1DComplex & SHOW_REAL));

      if(curPlot->display1DComplex & SHOW_MAGNITUDE)
         pp->obj->winParent->setMenuItemEnable(pp->menuName(),"show_imaginary",false);

      pp->curPlot()->Invalidate();	
   }
   else if(func == "toggle real")
	{
		Plot1D* curPlot = Plot1D::curPlot();
      if(curPlot->display1DComplex & SHOW_MAGNITUDE)
         pp->obj->winParent->setMenuItemEnable(pp->menuName(),"show_real",true);

		if(curPlot->display1DComplex & SHOW_REAL)
	      curPlot->display1DComplex = curPlot->display1DComplex & ~SHOW_REAL;
	   else
	      curPlot->display1DComplex = curPlot->display1DComplex | SHOW_REAL;

      pp->obj->winParent->setMenuItemCheck(pp->menuName(),"show_imaginary",(curPlot->display1DComplex & SHOW_IMAGINARY));
      pp->obj->winParent->setMenuItemCheck(pp->menuName(),"show_real",(curPlot->display1DComplex & SHOW_REAL));

      if(curPlot->display1DComplex & SHOW_MAGNITUDE)
         pp->obj->winParent->setMenuItemEnable(pp->menuName(),"show_real",false);

      pp->curPlot()->Invalidate();	   			   		   
   }
   else if(func == "toggle magnitude")
	{
		Plot1D* curPlot = Plot1D::curPlot();
		if(curPlot->display1DComplex & SHOW_MAGNITUDE)
	      curPlot->display1DComplex = curPlot->display1DComplex & ~SHOW_MAGNITUDE;
	   else
	      curPlot->display1DComplex = curPlot->display1DComplex | SHOW_MAGNITUDE;

      pp->obj->winParent->setMenuItemCheck(pp->menuName(),"show_magnitude",(curPlot->display1DComplex & SHOW_MAGNITUDE));
      if((curPlot->display1DComplex & SHOW_MAGNITUDE))
      {
         pp->obj->winParent->setMenuItemEnable(pp->menuName(),"show_real",false);
         pp->obj->winParent->setMenuItemEnable(pp->menuName(),"show_imaginary",false);
      }
      else
      {
         pp->obj->winParent->setMenuItemEnable(pp->menuName(),"show_real",true);
         pp->obj->winParent->setMenuItemEnable(pp->menuName(),"show_imaginary",true);
      }

      pp->curPlot()->Invalidate();	   			   		   
   }
   else if(func == "view one plot")
   {
      if(pp->rows*pp->cols > 1)
      {
         pp->DisplayOnePlot();
      }
   }
   else if(func == "zoom region")
   {
		Plot* curPlot = pp->curPlot();
      if(pp->mouseMode == SHOW_DATA) 
      {
         curPlot->HideDataCursor(hdc);
         pp->mouseMode = SELECT_RECT;
      }
      curPlot->curXAxis()->setAutorange(false);              
      curPlot->curYAxis()->setAutorange(false);              
      if(curPlot->rectSelected)
      {
         curPlot->ZoomRegion();
			curPlot->updateStatusWindowForZoom(hWnd);
         curPlot->Invalidate();
      }   
      else
      {
         MessageDialog(prospaWin,MB_ICONINFORMATION,"Message","A region needs to be selected first");
      }	         
   }
   else
   {
      ErrorMessage("unknown function");
      ReleaseDC(Plot1D::curPlot()->win,hdc);
      return(ERR);
   }

	if (Plot1D::curPlot())
	{
		ReleaseDC(Plot1D::curPlot()->win,hdc);
	}

	itfc->nrRetValues = 0;
   return(OK);
}

/************************************************************************
      Display a single 1D or 2D subplot removing all other plots
************************************************************************/

int KeepSubPlot(Interface* itfc ,char args[])
{
   CText whichPlot;
   short xPos,yPos,index;
   short nrArgs;
   RECT r;

// Initialize whichPlot in case user want to determine current plot
   Plot::curPlot()->plotParent->GetCurrentPlot(whichPlot,xPos,yPos,index);

// Extract parameters  *************        
   if((nrArgs = ArgScan(itfc,args,1,"1d/2d, [xPos, yPos]","eee","tdd",&whichPlot,&xPos,&yPos)) < 0)
      return(nrArgs);  

	Plot* p = namedCurrentPlot(whichPlot);
	if (!p)
	{
		return ERR;
	}
	if(p->plotParent->RemoveAllButOneSubPlot(xPos,yPos) == ERR)
		return(ERR);
	GetClientRect(p->win,&r);
	long width = r.right-r.left+1;
	long height = r.bottom-r.top+1;
	SendMessage(p->win,WM_SIZE,(WPARAM)SIZE_RESTORED,(LPARAM)(width+(height<<16))); 
   itfc->nrRetValues = 0;
   return(OK);
}
		
Plot* namedCurrentPlot(CText& which)
{
	Plot* p = 0;
	which.LowerCase();
	if (which == "1d")
	{
		if(!(p = Plot1D::curPlot()))
		{
			ErrorMessage("1d plot not defined");
			return(0);
		}
	}
	else if (which == "2d")
	{
		if(!(p = Plot2D::curPlot()))
		{
			ErrorMessage("2d plot not defined");
			return(0);
		}
	}
	else
	{
		ErrorMessage("invalid plot reference (1d/2d)");
		return(0);
	}
	return p;
}

/********************************************************
*    Display all subplots
********************************************************/

int ViewFullPlot(Interface* itfc ,char args[])
{
   CText whichPlot;
   short xPos,yPos,index;
   short nrArgs;

// Initialize whichPlot in case user want to determine current plot
   Plot::curPlot()->plotParent->GetCurrentPlot(whichPlot,xPos,yPos,index);

// Extract parameters  *************        
   if((nrArgs = ArgScan(itfc,args,1,"1d/2d","e","t",&whichPlot)) < 0)
      return(nrArgs);  

	Plot* p = namedCurrentPlot(whichPlot);
	if (!p)
	{
		return ERR;
	}
	p->plotParent->ViewFullPlot();
	p->Invalidate();
   return(OK);
}


/********************************************************
*    Display one subplot
********************************************************/

int ViewSubPlot(Interface* itfc ,char args[])
{
   CText whichPlot;
   short xPos,yPos,index;
   short nrArgs;

   // Initialize whichPlot in case user want to determine current plot
   Plot::curPlot()->plotParent->GetCurrentPlot(whichPlot,xPos,yPos,index);

// Extract parameters  *************        
   if((nrArgs = ArgScan(itfc,args,1,"1d/2d, [xPos, yPos]","eee","tdd",&whichPlot,&xPos,&yPos)) < 0)
      return(nrArgs);  

	Plot* p = namedCurrentPlot(whichPlot);
	if (!p)
	{
		return ERR;
	}
	if (p->plotParent->ViewSubPlot(xPos,yPos) == ERR)
		return(ERR);
	p->Invalidate();

   return(OK);
}

/********************************************************
*  Make sure the selection rectangle is not
*  redraw when the plot is next displayed.
********************************************************/
   
int RemoveSelectionRect(Interface* itfc ,char args[])
{
	if(!Plot::curPlot())
   {
      ErrorMessage("no current plot");
      return(ERR);
   }

	Plot* p = Plot::curPlot();   

	HDC hdc = GetDC(p->win);               
   p->HideSelectionRectangle(hdc);
   ReleaseDC(p->win,hdc);
   return(OK);
}

//
// This allows the double click, move, scroll-wheel actions in 1D plots to be overridden
// Just pass a macro to use instead. An empty string will revert to the defaul action
//
int DefinePlotCallback(Interface* itfc, char args[])
{
   CText funcToReplace, callBack;
   short nrArgs;

   if ((nrArgs = ArgScan(itfc, args, 1, "function_to_replace, callback_macro", "ee", "tt", &funcToReplace, &callBack)) < 0)
      return(nrArgs);

   if (nrArgs == 1)
   {
      if (funcToReplace == "plot1DScale")
      {
         itfc->retVar[1].MakeAndSetString(gPlot1DScaleMacro.Str());
         itfc->nrRetValues = 1;

      }
      else if (funcToReplace == "plot1DMove")
      {
         itfc->retVar[1].MakeAndSetString(gPlot1DShiftMacro.Str());
         itfc->nrRetValues = 1;
      }
      else if (funcToReplace == "plot1DDblClick")
      {
         itfc->retVar[1].MakeAndSetString(gPlot1DDblClickMacro.Str());
         itfc->nrRetValues = 1;
      }
      else
      {
         ErrorMessage("Unknown function to replace name (should be plot1DScale or plot1DMove)");
         return(ERR);
      }
   }
   else
   {
      if (funcToReplace == "plot1DScale")
      {
         gPlot1DScaleMacro = callBack;
      }
      else if (funcToReplace == "plot1DMove")
      {
         gPlot1DShiftMacro = callBack;
      }
      else if (funcToReplace == "plot1DDblClick")
      {
         gPlot1DDblClickMacro = callBack;
      }
      else
      {
         ErrorMessage("Unknown  function to replace name (should be plot1DScale or plot1DMove)");
         return(ERR);
      }
      itfc->nrRetValues = 0;
   }

   return(OK);
}

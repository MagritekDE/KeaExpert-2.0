#include "stdafx.h"
#include "guiInteractiveWindowBuilder.h"
#include "BabyGrid.h"
#include "cArg.h"
#include "command_other.h"
#include "defineWindows.h"
#include "drawing.h"
#include "edit_class.h"
#include "files.h"
#include "globals.h"
#include "guiObjectClass.h"
#include "guiWindowClass.h"
#include "guiWindowCLI.h"
#include "guiWindowsEvents.h"
#include "interface.h"
#include "main.h"
#include "message.h"
#include "mymath.h"
#include "plot.h"
#include "process.h"
#include "scanstrings.h"
#include "string_utilities.h"
#include "tab.h"
#include "variablesOther.h"
#include <deque>
#include "memoryLeak.h"

using std::deque;

short interactiveMode = NOTHING;
WinData *editGUIWindow;  // Window currently being edited

long SaveObjectDefinitions(Interface *itfc, WinData* saveWin, FILE *fp, CText &mode1,  CText &mode2);
bool SaveAdditionalObjectParameters(WinData *saveWin, FILE *fp, CText &mode);
bool SaveAdditionalObjectParameters(WinData *saveWin, FILE *fp, CText &mode, ObjectData *obj);
bool FindTabCtrl(ObjectData *obj, ObjectData **tab, short &nr);
void SaveWindowDef(FILE *fp);
bool SaveAdditionalWindowParameters(WinData *saveWin, FILE *fp, CText &mode);
Variable* GetControlVariable(WinData *win, ObjectData *obj);

/************************************************************************************
            Select an object interactively and return to the user the
            parent window number and the object number.

            Syntax: (winNr, objNr) = selectobj()

            Revision history:

            8/2/07 ... updated syntax and call help.
************************************************************************************/

int SelectObjInteractively(Interface* itfc ,char arg[])
{
   MSG msg;
	HDC hdc;
   short x,y;
   WinData *win;
   ObjectData *obj;
   HWND parent;

// Allow the user to interactively select the object ***********

   while(1)
   {
      if(PeekMessage(&msg, NULL,WM_MOUSEFIRST,WM_MOUSELAST,PM_REMOVE))
      {         
         SetCursor(LoadCursor(NULL,IDC_ARROW));
	      if(msg.message == WM_LBUTTONDOWN) 
	      {
	         hdc = GetDC(msg.hwnd);
            x = LOWORD(msg.lParam);
            y = HIWORD(msg.lParam);
	         break;
	      }	
         DispatchMessage(&msg);	  
         TranslateMessage(&msg);
      }
      if(GetAsyncKeyState(VK_ESCAPE) & 0x08000)
      {
         itfc->nrRetValues = 3;
         itfc->retVar[1].MakeAndSetFloat(-1);   
         itfc->retVar[2].MakeAndSetFloat(-1); 
         itfc->retVar[3].MakeNullVar(); 	
         return(OK);
      }
   }
   
// Find window selected (edit menus have a window embedded in a window so need to parent of parent)
   win = GetWinDataClass(msg.hwnd);
   if(!win)
   {
      parent = GetParent(msg.hwnd);
      win = GetWinDataClass(parent);
      if(!win)
      { 
         parent = GetParent(parent);
         win = GetWinDataClass(parent);   
      }
              
      if(win)
      {
         POINT p;
         p.x = x; p.y = y; 
         ClientToScreen(msg.hwnd,&p);  
         ScreenToClient(parent,&p);  
         x = (short)p.x; y = (short)p.y;  
      }
   }
 
 // Take a note of the number of returned arguments
   itfc->nrRetValues = 3;

// No window found      
   if(!win) 
   {
	   itfc->retVar[1].MakeAndSetFloat(-1);   
	   itfc->retVar[2].MakeAndSetFloat(-1); 
      itfc->retVar[3].MakeNullVar(); 	
      return(OK); 
   }       
   
// Find the object (if any)
   obj = win->FindObject(x,y);

// Return object and window number
   if(obj)
   {      
	   itfc->retVar[1].MakeAndSetFloat(win->nr);   
	   itfc->retVar[2].MakeAndSetFloat(obj->nr()); 
      itfc->retVar[3].MakeClass(OBJECT_CLASS,(void*)obj);
	}
// Just return window number
	else 
	{
	   itfc->retVar[1].MakeAndSetFloat(win->nr);   
	   itfc->retVar[2].MakeAndSetFloat(-1); 	
	   itfc->retVar[3].MakeNullVar(); 	
	} 
	return(OK);    
}

/************************************************************************************
            Allow the user to position and size an object interactively            
            The argument to this function is a creation command for
            an object e.g. button(n,..). The steps in doing this are:
            1. Select gui window to add control to
            2. Drag dotted rectangle representing object to desired position
            3. Release left mouse button. Object is then displayed at this position
************************************************************************************/

int MakeObjectInteractively(Interface* itfc ,char args[])
{
   static char command[MAX_STR];
   short r;
   WinData *win,*designerWin;
   short objID;
      
// Extract command (should be quoted) ****************
   if((r = ArgScan(itfc,args,1,"command","c","s",command)) < 0)
      return(r);

   gKeepCurrentFocus = true;

   designerWin = GetGUIWin();
   
// Allow user to select window to add object to ******
   MSG msg;
	HDC hdc;
   short x,y;
   SetCursor(CtrlCursor);
   interactiveMode = MAKE;


 // Wait for key to return to up state if still down
// For example following a double click
   if(IsKeyDown(VK_LBUTTON))
   {
      while(1)
      {
         if(PeekMessage(&msg, NULL, WM_LBUTTONUP, WM_LBUTTONUP,PM_REMOVE))
         {         
	         if(msg.message == WM_LBUTTONUP) 
	         {
	            break;
	         }	      
            DispatchMessage(&msg);	  
            TranslateMessage(&msg);
         }
      }
   }

// Allow user to select new control position
// Ignore all events except left button down
// Abort by pressing escape
   while(1)
   {
      if(gAbortMacro) // May need a special flag to use this
      {
         interactiveMode = NOTHING;
         gAbortMacro = false;
         return(0);
      }

      if(PeekMessage(&msg, NULL, WM_LBUTTONDOWN, WM_LBUTTONDBLCLK,PM_REMOVE))
      {  
	      if(msg.message == WM_LBUTTONDBLCLK) // Prevent double click from doing anything
	         return(0);

	      if(msg.message == WM_LBUTTONDOWN) 
	      {
	         hdc = GetDC(msg.hwnd);
            x = LOWORD(msg.lParam);
            y = HIWORD(msg.lParam);
	         break;
	      }
         DispatchMessage(&msg);	  
         TranslateMessage(&msg);
      }
   } 
   interactiveMode = NOTHING;
   gBlockWaitCursor = false;

// Set window for editing, exit if not a GUI ***********
   win = GetWinDataClass(msg.hwnd);
   if(!win) 
   {
		WinData::SetGUIWin(designerWin);
      return(0); 
   }

// Ignore if designer is clicked on *******************  
   if(win == designerWin)
   {
      return(0);
   }
         
// Stop editing current window ************************
   if(win != editGUIWindow)
   {
      ActivateEditedGUIWin();
   }
 
// Find handle for new object *************************
   objID = win->FindNextFreeObjNr();

// Make this window editable
   win->makeEditable();
   win->EnableResizing(true);

// Get size of object rectange ************************
   RECT rect;
   GetClientRect(msg.hwnd,&rect);
   short ww = (short)rect.right;
   short wh = (short)rect.bottom;


// Add object and get a handle to it plus its size *****   
   DWORD oldVis = g_objVisibility;
   g_objVisibility = 0; // make sure new object is invisible
   if(ProcessMacroStr(itfc,(char*)command) == ERR)
   {
      g_objVisibility = oldVis;
		WinData::SetGUIWin(designerWin);
      return(0);
   }
   g_objVisibility = oldVis;
   ClassData *cls = (ClassData*)itfc->retVar[1].GetClass();
   ObjectData *obj = (ObjectData*)cls->data;
   obj->EnableObject(false);
	obj->selected_ = true;

// Special case which doesn't require manual placement *
   if(obj->type == STATUSBOX)
   {
	   obj->selected_ = true;
	   obj->nr(objID);
	   ShowWindow(obj->hWnd,SW_SHOW);      
	   MyInvalidateRect(win->hWnd,NULL,false);   
	   WinData::SetGUIWin(designerWin);
      return(0);
   }
       
// Snap to grid if desired
   if(win->snapToGrid)
   {
      x = win->gridSpacing*nint(x/(float)win->gridSpacing);
      y = win->gridSpacing*nint(y/(float)win->gridSpacing);
   }

// Move the outline of object to the required position interactively  
   obj->xo = x;
   obj->yo = y;
   obj->xSzOffset = x;
   obj->ySzOffset = y;
   MoveObjectsInteractively(1,win, hdc, obj, x, y, ww, wh);

// Move to its final position - note that the radiobuttons
// require extra work
   obj->nr(objID);

// Set the window identifier
   if(obj->type == RADIO_BUTTON)
   {
      RadioButtonInfo* info = (RadioButtonInfo*)obj->data;  
	   for(int i = 0; i < info->nrBut; i++)
         SetWindowLong(info->hWnd[i],GWL_ID,obj->nr());
	}
	else
	   SetWindowLong(obj->hWnd,GWL_ID,obj->nr());

   obj->selected_ = true;

// See if object is over a tab control 
	ObjectData *tab = NULL;
	short nr;

	if(FindTabCtrl(obj,&tab,nr))
	{
		obj->tabPageNr = nr;
	   obj->tabParent = tab;
	}

// Make the object visible
   obj->Show(true);
   obj->visible = true;

// Redraw the whole window just in case
   MyInvalidateRect(win->hWnd,NULL,false);
// Ensure that focus is in main window not control (CLI in particular)
// otherwise can't move control or delete it
   SetFocus(win->hWnd);
// Restore the designer as current gui window
   WinData::SetGUIWin(designerWin);
   return(0);
}

/****************************************************************************
         Interactively move (a) control(s) both horizontally and vertically
         If the shift key is help down before or during the move the motion
         will be restricted to vertical or horizonal.

         mode .... whether to initially draw/undraw rectange around object
         win ..... gui window info
         hdc ..... device context
         obj ..... object under cursor
         x,y ..... initial position of mouse
         ww,wh ... width and height of selected object

         Still to do - restrict all objects to be within window
*****************************************************************************/

int MoveObjectsInteractively(short mode, WinData *win, HDC hdc, ObjectData *obj, short &x, short &y, short ww, short wh)
{
   short xo,yo; // Initial object position
   short wo,ho;
   short dx,dy; // Distance between mouse and object
   short delx = 0,dely = 0;
   short origx,origy; // Original mouse position
   MSG msg;
   HWND hwnd = win->hWnd;
   char str[50];
   class MyObjData
   {
	public:
      ObjectData *obj;
      short x,y,w,h;
      short ox,oy;
      short orx,ory;
      short nx,ny;
      bool show;
   };
   
	typedef deque<MyObjData*> RectanglesList;
   
// Initial mouse position
   origx = x;
   origy = y;

// See if any of the selected objects are tab controls
// If so select all connected controls

	WidgetList* tabControls = win->widgets.getAllOfType(TABCTRL);
	for(ObjectData* o: *tabControls)
	{
		if (o->isSelected())
		{
			for(ObjectData* amIConnected: win->widgets.getWidgets())
			{
				if (amIConnected->isTabChildOf(o))
				{
					amIConnected->setSelected(true);
				}
			}
		}
	}
	// Delete tabControls, but not the controls it's referring to.
	if (tabControls)
	{
		delete tabControls;
	}
 
// Get the selected objects and	
// Extract their rectangles
	RectanglesList rectangles;
	for(ObjectData* o: win->widgets.getWidgets())
	{
		if (o->isSelected())
		{
			MyObjData* objData = new MyObjData();

         if(o->type == PANEL)
         {
            objData->obj  = o;
            PanelInfo* info = (PanelInfo*)o->data;
            objData->x  = info->x-3;
            objData->w  = info->w+20+2+4;
            objData->y  = info->y-3;
            objData->h  = info->h+2+4;
            objData->nx  = info->x-3;
            objData->ny  = info->y-3;
            objData->ox  = -1;
            objData->oy  = -1;
            objData->orx  = info->x-3;
            objData->ory  = info->y-3;
            objData->show = true;
         }
         else   

         {
            objData->obj  = o;
            objData->x  = o->xo;
            objData->w  = o->wo;
            objData->y  = o->yo;
            objData->h  = o->ho;
            objData->nx  = o->xo;
            objData->ny  = o->yo;
            objData->ox  = -1;
            objData->oy  = -1;
            objData->orx  = o->xo;
            objData->ory  = o->yo;
            objData->show = true;
         }

			if (o->isTabChild())// Don't show selection rectangle for objects in a selected tab control
			{
				ObjectData* tabParent = o->getTabParent();
				if (tabParent->isSelected())
				{
               objData->show = false;
				}
         }
			rectangles.push_back(objData);
		}
	}
         
// Current object position and dimensions
   if(obj->type == PANEL)
   {
      PanelInfo* info = (PanelInfo*)obj->data;
      xo = info->x;
      yo = info->y;
      wo = info->w+20;
      ho = info->h;
   }
   else
   {
      xo = obj->xo;
      yo = obj->yo;
      wo = obj->wo;
      ho = obj->ho;
   }
   dx = x-xo;  // Distance between mouse and object
   dy = y-yo;

// Initial object position
   short origox = xo;
   short origoy = yo;

// Move the outline of object to the required position interactively
   HPEN rubberPen = CreatePen(PS_DOT,0,RGB(0,0,0));
   HPEN oldPen = (HPEN)SelectObject(hdc,rubberPen);
   SetROP2(hdc,R2_XORPEN);

// Draw or undraw outline around each object
	for(MyObjData* objData: rectangles)
   {
      if(mode && objData->show)
   	   DrawRect(hdc,objData->x,objData->y,objData->x+objData->w-1,objData->y+objData->h-1);
      objData->ox = objData->x;
      objData->oy = objData->y;
   }

   interactiveMode = MOVE;

   SetCapture(hwnd); 
   while(1)
   {
   //   if(PeekMessage(&msg, NULL ,WM_MOUSEFIRST,WM_MOUSELAST,PM_REMOVE))
      if(PeekMessage(&msg, NULL ,NULL,NULL,PM_REMOVE))
      {         
	      if(msg.message == WM_MOUSEMOVE && msg.hwnd != hwnd)
	      {
				for(MyObjData* objData: rectangles)
		      {
               if(objData->ox > 0 && objData->oy > 0 && objData->ox < ww && objData->oy < wh)
               {		
                  if(objData->show)
		               DrawRect(hdc,objData->ox,objData->oy,objData->ox+objData->w-1,objData->oy+objData->h-1);
		            objData->ox = -1;
		            objData->oy = -1;
		         }
		      }	
            SetCursor(LoadCursor(NULL,IDC_NO));
         }
	       
	      if(msg.message == WM_MOUSEMOVE && msg.hwnd == hwnd) 
	      {
            SetCursor(LoadCursor(NULL,IDC_SIZEALL));
            x = LOWORD(msg.lParam); // Current mouse position
            y = HIWORD(msg.lParam);

            if(msg.wParam & MK_SHIFT)
            {
               if(abs(x-origx) > abs(y-origy)) // Move in x direction only
               {
                  if(x-dx < 0 || x-dx+wo >= ww) // Check for out of bounds
                  {
                     SetCursor(LoadCursor(NULL,IDC_NO));
                     delx = 0;
                  }
                  else
                  {
                     delx = x-dx-xo;
                  }
                  dely = 0;
                  xo += delx; // New object position
                  yo = origoy;
               }
               else if(abs(x-origx) < abs(y-origy))
               {
                  if(y-dy < 0 || y-dy+ho >= wh) // Check for out of bounds
                  {
                     SetCursor(LoadCursor(NULL,IDC_NO));
                     dely = 0;
                  }
                  else
                  {
                     dely = y-dy-yo;
                  }
                  delx = 0;
                  yo += dely; // New object position
                  xo = origox;
               }
            }
            else
            {
               if(x-dx < 0 || x-dx+wo >= ww) // Check for out of bounds
               {          
                  SetCursor(LoadCursor(NULL,IDC_NO));
                  delx = 0;
               }
               else
                  delx = x-dx-xo;  // Amount to shift object by to match mouse movement

               if(y-dy < 0 || y-dy+ho >= wh) // Check for out of bounds
               {
                  SetCursor(LoadCursor(NULL,IDC_NO));
                  dely = 0;
               }
               else
                  dely = y-dy-yo;

               xo += delx; // New object position
               yo += dely; 
            }
           

         // Remove old rectangle
				for(MyObjData* objData: rectangles)
			   {	
               if(objData->show)
			         DrawRect(hdc,objData->ox,objData->oy,objData->ox+objData->w-1,objData->oy+objData->h-1);
			   }

         // Draw new rectangle
			   for(MyObjData* objData: rectangles)
			   {
               if(msg.wParam & MK_SHIFT)
               {
                  if(abs(x-origx) > abs(y-origy)) // Move in x direction only
                  {
			            objData->nx += delx; // Shift object rect
			            objData->ny = objData->ory; // don't shift y 
                  }
                  else if(abs(x-origx) < abs(y-origy))
                  {
 			            objData->ny += dely; // Shift object rect
			            objData->nx = objData->orx; // don't shift y 
                  }
               }
               else
               {
			         objData->nx += delx; // Shift object rect
			         objData->ny += dely; 
               }

               sprintf(str,"x = %hd y = %hd",objData->x,objData->y);
               SetWindowText(obj->hwndParent,str);
               if(win->snapToGrid)
               {
                  objData->x = win->gridSpacing*nint(objData->nx/(float)win->gridSpacing);
                  objData->y = win->gridSpacing*nint(objData->ny/(float)win->gridSpacing);
               }
               else
               {
                  objData->x = objData->nx;
                  objData->y = objData->ny;
               }
               if(objData->show)
		            DrawRect(hdc,objData->x,objData->y,objData->x+objData->w-1,objData->y+objData->h-1);

               objData->ox = objData->x;
               objData->oy = objData->y;
			   }
	      }
	      if(msg.message == WM_LBUTTONUP && msg.hwnd == hwnd) 
	      {	      
            SetCursor(LoadCursor(NULL,IDC_ARROW));	      
            x = xo;     
            y = yo; 
            break;
	      }
	      if(msg.message == WM_LBUTTONUP && msg.hwnd != hwnd) 
	      {	      
            SetCursor(LoadCursor(NULL,IDC_ARROW));	 
            x = xo;     
            y = yo;
            break;
	      }	      		      	      
         DispatchMessage(&msg);	  
         TranslateMessage(&msg);
      }
   }

// Place the objects

	for(MyObjData* objData: rectangles)
   {
      if(objData->obj->type == PANEL)
      {
         objData->obj->xSzOffset += (objData->x-objData->orx);
         objData->obj->ySzOffset += (objData->y-objData->ory);
         objData->obj->Place(objData->x+3+objData->w-20-2-4,objData->y+3,20,objData->h-2-4,false);
      }
      else
      {
         objData->obj->xSzOffset += (objData->x-objData->orx);
         objData->obj->ySzOffset += (objData->y-objData->ory);
         objData->obj->Place(objData->x,objData->y,objData->w,objData->h,false);
      }
   }

   SetTimer(obj->hwndParent,1,2000,NULL); // Update window title
   ReleaseCapture();
   SelectObject(hdc,oldPen);
   DeleteObject(rubberPen);
   interactiveMode = NOTHING;
   return(0);
}


/****************************************************************************

         Interactively resize (a) control(s) both horizontally and vertically
         If the shift key is help down before or during the move the motion
         will be restricted to vertical or horizonal.

         mode .... which direction to resize the object
         win ..... gui window info
         hdc ..... device context
         obj ..... object under cursor
         x,y ..... initial position of mouse
         ww,wh ... width and height of selected object

         Still to do - restrict all objects to be within window

*****************************************************************************/

int ResizeObjectsInteractively(short mode, WinData *win, HDC hdc, ObjectData *obj, short x, short y, short ww, short wh)
{
   short xo,yo;
   short wo,ho;
   short orig_wo,orig_ho;
   float fracw,frach;
   MSG msg;
   HWND hwnd = win->hWnd;
   RECT r;
   char str[50];

   class MyObjData
   {
	public:    
		ObjectData *obj;
      short x,y,w,h;
      short ox,oy;
      short orig_w,orig_h;
   };
	
	typedef deque<MyObjData*> RectanglesList;

// Object position and dimensions
   if(obj->type == PANEL)
   {
      PanelInfo* info = (PanelInfo*)obj->data;
      xo = info->x;
      yo = info->y;
      wo = info->w+20;
      ho = info->h;
   }
   else
   {
      xo = obj->xo;
      yo = obj->yo;
      wo = obj->wo;
      ho = obj->ho;
   }
   orig_wo = wo;
   orig_ho = ho;

// Get theselected objects and extract their rectangles
	RectanglesList rectangles;
	for(ObjectData* o: win->widgets.getWidgets())
	{
		if (o->isSelected())
		{
			MyObjData* objData = new MyObjData();

         if(o->type == PANEL)
         {
            objData->obj  = o;
            PanelInfo* info = (PanelInfo*)o->data;
            objData->x  = info->x-3;
            objData->w  = info->w+20+2+4;
            objData->y  = info->y-3;
            objData->h  = info->h+2+4;
            objData->ox  = -1;
            objData->oy  = -1;
            objData->orig_w  = info->w+20+2+4;
            objData->orig_h  = info->h+2+4;
         }
         else
         {
            objData->obj = o;
            objData->x  = o->xo;
            objData->w  = o->wo;
            objData->y  = o->yo;
            objData->h  = o->ho;
            objData->orig_w  = o->wo;
            objData->orig_h  = o->ho;
            objData->ox  = -1;
            objData->oy  = -1;
         }
			rectangles.push_back(objData);
		}
	}

   GetClientRect(hwnd,&r);

// Move the outline of object to the required position interactively
   HPEN rubberPen = CreatePen(PS_DOT,0,RGB(0,0,0));
   HPEN oldPen = (HPEN)SelectObject(hdc,rubberPen);
   SetROP2(hdc,R2_XORPEN);

// Delete dotted lines around all controls
	for(MyObjData* objData: rectangles)
   {
      DrawRect(hdc,objData->x,objData->y,objData->x+objData->w-1,objData->y+objData->h-1);
      objData->ox = objData->x;
      objData->oy = objData->y;
   }

   interactiveMode = RESIZE;
   SetCapture(hwnd);

   while(1)
   {
      if(PeekMessage(&msg, hwnd ,WM_MOUSEFIRST, WM_MOUSELAST, PM_REMOVE))
      {   
	   // Moving mouse so draw a rubber rectangle 	       
	      if(msg.message == WM_MOUSEMOVE) 
	      {
            x = LOWORD(msg.lParam);
            y = HIWORD(msg.lParam);

         // Check for cursor position outside window
            if(x > r.right || x < xo || y > r.bottom || y < yo)
               SetCursor(LoadCursor(NULL,IDC_NO));
            else
            {
               if(mode == DIAG_RESIZE)
                  SetCursor(LoadCursor(NULL,IDC_SIZENWSE));
               else if(mode == VERT_RESIZE)
                  SetCursor(LoadCursor(NULL,IDC_SIZENS));
               else if(mode == HORIZ_RESIZE)
                  SetCursor(LoadCursor(NULL,IDC_SIZEWE));
            }

         // New control width and height
            if(mode == DIAG_RESIZE)
            {
               wo = x-xo;
               ho = y-yo;
            }
            else if(mode == VERT_RESIZE)
            {
               ho = y-yo;
            }
            else if(mode == HORIZ_RESIZE)
            {
               wo = x-xo;
            }


         // Limit width and height to current window size or 5 pixels
            if(xo+wo > ww)
              wo = ww-xo;
            if(yo+ho > wh)
              ho = wh-yo; 

            if(wo < 5)
               wo = 5;
            if(ho < 5)
               ho = 5;

         // Record change in size
            fracw = (float)wo/orig_wo;
            frach = (float)ho/orig_ho;

         // Remove old rectangle
				for(MyObjData* objData: rectangles)
			   {				   
		         DrawRect(hdc,objData->x,objData->y,objData->ox,objData->oy);
			   }

         // Draw the new rectangle(s)
            sprintf(str,"w = %hd h = %hd",wo,ho);
            SetWindowText(obj->hwndParent,str);
				for(MyObjData* objData: rectangles)
		      {
           // Calculate the new width and height of the ith control
               objData->w = nsint(fracw*objData->orig_w);
               objData->h = nsint(frach*objData->orig_h);
           // Limit width and height to a minimum of 5 pixels
               if(objData->w < 5)
                  objData->w = 5;
               if(objData->h < 5)
                  objData->h = 5;
            // Draw the outline of the modified control
	            DrawRect(hdc,objData->x,objData->y,objData->x+objData->w-1,objData->y+objData->h-1);
            // Record these drawing coordinates
               objData->ox = objData->x+objData->w-1;
               objData->oy = objData->y+objData->h-1;
		      }
            
	      }
      // Mouse button released so update all controls and exit
	      if(msg.message == WM_LBUTTONUP) 
	      {
            SetCursor(LoadCursor(NULL,IDC_ARROW));	

	/*		   for(MyObjData* objData: rectangles)
			   {		
               objData->obj->wSzOffset += (objData->w-objData->orig_w);
               objData->obj->hSzOffset += (objData->h-objData->orig_h);
               objData->obj->Place(objData->x,objData->y,objData->w,objData->h,false);
			   }*/

	         for(MyObjData* objData: rectangles)
            {
               if(objData->obj->type == PANEL)
               {
                  PanelInfo* info = (PanelInfo*)objData->obj->data;
                  objData->obj->hSzOffset += (objData->h-objData->orig_h);
                  objData->obj->Place(objData->x+3+objData->w-20-2-4,objData->y+3,objData->w-objData->orig_w+20,objData->h-2-4,false); 
               }
               else
               {
                  objData->obj->wSzOffset += (objData->w-objData->orig_w);
                  objData->obj->hSzOffset += (objData->h-objData->orig_h);
                  objData->obj->Place(objData->x,objData->y,objData->w,objData->h,false);
               }
            }
            break;
	      }

         DispatchMessage(&msg);	  
         TranslateMessage(&msg);
      }
   }
   SetTimer(obj->hwndParent,1,1000,NULL); // Update gui window label after 1 s
   ReleaseCapture();
   SelectObject(hdc,oldPen);
   DeleteObject(rubberPen);
   interactiveMode = NOTHING; 
   return(OK);
}


/*********************************************************************
             Activate the current GUI window which is being edited
**********************************************************************/

void ActivateEditedGUIWin()
{
   WinData *w;
   for(w = (rootWin->next); w != NULL; w = w->next)
   {
      if(w->activated == false) 
         w->activate();
	}
}

/*********************************************************************
             Activate or deactivate the current GUI window

  Syntax: activate("true"/"false")

  Note: Activate means make runnable
        Deactive means make editable

**********************************************************************/

int ActivateWindow(Interface* itfc ,char arg[])
{
   short n;
   WinData *win;
   static char activate[50];
   WinData *designerWin;
   POINT p;

// Assume this window has been started from the designer macro ****
   designerWin = GetGUIWin();

// Get user choice (activate or not) *******************************
   if((n = ArgScan(itfc,arg,1,"true/false","e","s",activate)) < 0)
      return(n);

// If a GUI window exists which is being edited then activate it ****
   if(!strcmp(activate,"true"))
   {
   // Search for the window which is being edited
      win = rootWin->next;
      WinData *w;
      for(w = win; w != NULL; w = w->next)
      {
         if(w == designerWin || w->activated == true) 
            continue;
         else
            break;
      }
      if(!w)
      {
         MessageDialog(prospaWin,MB_ICONINFORMATION,"Information","No window to run");
         return(0); // No window to edit
      }
      w->activate();
   }

// Interactively find a window to edit (deactivate) ******************	   
   else if(!strcmp(activate,"false"))
   {
   // Search for the window which is already being edited and activate it
      ActivateEditedGUIWin();

   // First check to see if there is a window to edit!
      WinData *w;
      for(w = rootWin->next; w != NULL; w = w->next)
      {
         if(w == designerWin) 
            continue;
         else
            break;
      }
      if(!w)
      {
         MessageDialog(prospaWin,MB_ICONINFORMATION,"Information","No window to edit");
         return(0); // No window to edit
      }
   // Allow the user to select the window to edit
	   MSG msg;
	   SetCursor(LoadCursor(NULL,IDC_ARROW));
      SetCapture(designerWin->hWnd);
	   while(1)
	   {
         if(gAbortMacro)
         {
            ReleaseCapture();
            gAbortMacro = false;
            return(0);
         }
	      if(PeekMessage(&msg, NULL,WM_MOUSEFIRST,WM_MOUSELAST,PM_REMOVE))
	      { 
            p.x = (short)LOWORD(msg.lParam);
            p.y = (short)HIWORD(msg.lParam);
            ClientToScreen(designerWin->hWnd,&p);

		      if(msg.message == WM_LBUTTONDOWN) 
		      {
		         win = rootWin->FindWinByPosition(p);
	            if(win) break;
		      }
		      else if(msg.message == WM_MOUSEMOVE) 
		      {
		         win = rootWin->FindWinByPosition(p);
	            if(win)
	            	SetCursor(LoadCursor(NULL,IDC_ARROW));
	            else
	            	SetCursor(LoadCursor(NULL,IDC_NO));
		      }		      	      
	      }
	   } 
      ReleaseCapture();
   // Make sure the window is not the designer and then deactivate it   
	   if(win != designerWin)
	   {   
         win->makeEditable();
         win->EnableResizing(true);
		}
	} 
   else
   {
      ErrorMessage("invalid option");
      return(ERR); // No window to edit
   }

   WinData::SetGUIWin(designerWin);
	itfc->nrRetValues = 0;
   return(OK);
}

/*********************************************************************
             Save the current gui window layout to a file
**********************************************************************/

int SaveWindowLayout(Interface* itfc ,char args[])
{
   FILE *fp;
   long len = 0;
	long pos = 0;
   static CText file;
   CText fileNoExt;
   short nrArgs;
   bool modify = false;
   char *text = 0;
   CText path;
   CText mode1 = "window with objects";
   CText mode2 = "definition and setpar separated";

// Is there a window to save?
   if(!editGUIWindow)
   {
      ErrorMessage("No window being edited");
      return(ERR);
   }

// Note macro-path
   GetCurrentDirectory(path);

// Get filename for window layout  
   if((nrArgs = ArgScan(itfc,args,1,"file, mode","eee","ttt",&file,&mode1,&mode2)) < 0)
      return(nrArgs);

// Check to see if the file exists first
   if((fp = fopen(file.Str(),"rb")) != NULL)
   {
      fclose(fp);
      modify = true;
      
   // If it does then read in the old file completely
   // and replace the window definition section with the follow
   // macro code.
      text = LoadTextFileFromFolder(path.Str(),file.Str(),"");
      len = strlen(text);
   
   // Search for the window definition procedure
      if(mode1 == "window with objects")
      {
         if(!FindSubStr(text,"procedure(windowdefinition)",pos))
         {
            delete [] text;
            ErrorMessage("Can't find window definition procedure");
            return(ERR);
         }
      }
      else
      {
         if(!FindSubStr(text,"procedure(definecontrols,n)",pos))
         {
            delete [] text;
            ErrorMessage("Can't find control definition procedure");
            return(ERR);
         }
      }
       
   // Write out code before window definition procedure      
	   if((fp = fopen(file.Str(),"wb")) == NULL)
	   {
         delete [] text;
	      ErrorMessage("File can't be opened");
	      return(ERR);
	   }

      char *subtext = new char[pos+1];
      ExtractSubStr(text,subtext,0,pos); 	   
      fwrite(subtext,1L,(long)pos,fp);
      delete [] subtext;
   }
   else // New layout file.
   {
	// Open a file to save the layout in	
	   if((fp = fopen(file.Str(),"wb")) == NULL)
	   {
	      ErrorMessage("File can't be opened");
	      return(ERR);
	   }
	   fileNoExt = file;
      fileNoExt.RemoveExtension();
      if(mode1 == "window with objects")
      {
	      fprintf(fp,"procedure(%s)\r\n\r\n",fileNoExt.Str());
	      fprintf(fp,"   n = :windowdefinition()\r\n");
	      fprintf(fp,"   showwindow(n)\r\n\r\n");
	      fprintf(fp,"endproc()\r\n\r\n");
      }
      else
      {
	      fprintf(fp,"procedure(%s)\r\n\r\n",fileNoExt.Str());
         SaveWindowDef(fp);
         fprintf(fp,"   :definecontrols(n)\r\n");
	      fprintf(fp,"   showwindow(n)\r\n\r\n");
	      fprintf(fp,"endproc()\r\n\r\n");
      }
	}

   if(mode1 == "window with objects")
   {
   // Save procedure definition
      fprintf(fp,"procedure(windowdefinition)\r\n\r\n");
	   fprintf(fp,"   # Automatically generated window definition procedure.\r\n   # Any code added manually will be removed if layout modified interactively\r\n");
      SaveWindowDef(fp);
      fprintf(fp,"\r\n      # Define all controls with basic parameters\r\n");

   // Save window variables definition
      Variable *var = &(editGUIWindow->varList);
      short type;
      short cnt = 0;
      
      while(var)
      {
         var = var->GetNext(type);
         if(!var) break;
         if(cnt == 0)
         {
            fprintf(fp,"      windowvar(%s",var->GetName());
            cnt++;
         }
         else
         {
            fprintf(fp,",%s",var->GetName());
            cnt++;
         }
      }
      if(cnt > 0)
        fprintf(fp,")\r\n");
   }
   else
   {
       fprintf(fp,"procedure(definecontrols,n)\r\n\r\n");
   }
  
// Save the object info to a file
   long nrObjs = SaveObjectDefinitions(itfc, editGUIWindow, fp, mode1, mode2);

// Save any additional object parameters using setpar() commands
   bool foundAny;

   if(mode2 == "definition and setpar separated")
      foundAny = SaveAdditionalObjectParameters(editGUIWindow, fp, mode1);
   
// Save any window parameters
   foundAny = SaveAdditionalWindowParameters(editGUIWindow, fp, mode1);

// End of window procedure
   if(mode1 == "window with objects")
   {
      if(foundAny)
         fprintf(fp,"\r\n\r\nendproc(n)");
      else
         fprintf(fp,"\r\nendproc(n)");
   }
   else
   {
      if(foundAny)
         fprintf(fp,"\r\n\r\nendproc(%ld)",nrObjs);
      else
         fprintf(fp,"\r\nendproc(%ld)",nrObjs);
   }

// If this is an existing file then
// search for the end of the window definition procedure
// and write the rest of the text to the file
   if(modify == true)
   {
      if(!FindSubStr(text,"endproc(",pos))
      {
         delete [] text;
         ErrorMessage("Can't find end of window definition procedure");
         return(ERR);
      } 
      if(!FindSubStr(text,"\r\n",pos))
      {
         fprintf(fp,"\r\n");
      }
      else
      {
         char *subtext = new char[len-pos+1];
         ExtractSubStr(text,subtext,pos,len);
         fwrite(subtext,1L,(long)(len-pos),fp);
         delete [] subtext;
      }
      delete [] text;
   }
      
// Close the file
   fclose(fp);

// Update the window macroname and macropath
   strcpy(editGUIWindow->macroName,file.Str());
   strcpy(editGUIWindow->macroPath,path.Str());

	itfc->nrRetValues = 0;
   return(OK);
}

/*********************************************************************
           Save the window definition to the file 'fp'
**********************************************************************/

void SaveWindowDef(FILE *fp)
{
   // Save the window definition
   RECT r;
   GetWindowRect(editGUIWindow->hWnd,&r);

   short wo = (short)(r.right-r.left);

   short ho;
   if(editGUIWindow->resizeable)
      ho = (short)(r.bottom-r.top) - titleBarHeight - GetSystemMetrics(SM_CYSIZEFRAME);
   else
      ho = (short)(r.bottom-r.top) - titleBarHeight - GetSystemMetrics(SM_CYFIXEDFRAME);

   CText title = editGUIWindow->title;
   title = title.Start(title.Size()-2);
   if(editGUIWindow->resizeable)
      fprintf(fp,"   n = window(\"%s\", %hd, %hd, %hd, %hd, \"resizable\")\r\n",title.Str(), -1, -1, wo, ho);
   else
      fprintf(fp,"   n = window(\"%s\", %hd, %hd, %hd, %hd)\r\n",title.Str(), -1, -1, wo, ho);
}

/*********************************************************************
           Save to file 'fp' text for defining the object 'obj'
**********************************************************************/

long SaveObjectDefinitions(Interface *itfc, WinData* saveWin, FILE *fp, CText &mode1, CText &mode2)
{
   short nrCmd;
   CText label;
   char cmd[MAX_STR];
   char prefix[MAX_STR];
   char ctrlNamePrefix[MAX_STR];


   saveWin->widgets.sort();
	WidgetList& orderedWidgets = saveWin->widgets.getWidgetsInCtrlNrOrder();

	for(ObjectData* obj: orderedWidgets)
	{
      short nr = obj->nr();
      short xo,yo,wo,ho;

      if(obj->type == PANEL)
      {
         PanelInfo* info = (PanelInfo*)obj->data;
         xo = info->x;
         yo = info->y;
         wo = info->w+20;
         ho = info->h;
      }
      else
      {
         xo = obj->xo;
         yo = obj->yo;
         wo = obj->wo;
         ho = obj->ho;
      }

		if(mode1 == "window with objects")
			strcpy(prefix,"      ");
		else
			strcpy(prefix,"   ");

	// Add the object name if it exists
		if(strcmp(obj->objName,"undefined"))
		{
			strcpy(ctrlNamePrefix,obj->objName);
			strcat(ctrlNamePrefix," = ");
		}
		else
	// Search for a variable which connects to this object
		{
         Variable *var = GetControlVariable(saveWin, obj);
			if(var)
			{
				strcpy(ctrlNamePrefix, var->GetName());
				strcat(ctrlNamePrefix," = ");
			}
			else
			   strcpy(ctrlNamePrefix,"");
		}

      CText xTxt,yTxt,wTxt,hTxt;

      obj->GetRectExp(&xTxt,&yTxt,&wTxt,&hTxt,true);

      GetWindowTextEx(obj->hWnd,label);
      char* txt = new char[2*strlen(label.Str())+1];
      strcpy(txt,label.Str());
      ReplaceSpecialCharacters(txt,"\r","\\r",-1);
      label = txt;

		CArg cmdLst(';');
      if(obj->command[0] != '\0')
      {
         nrCmd = cmdLst.Count(obj->command);
         char *cmd = cmdLst.Extract(nrCmd); 
         if(cmd[0] == '\0') // If last argument is "" then ignore it (2.2.5)
           nrCmd--;
      }
      else
         nrCmd = 0;

      switch(obj->type)
      {
         case(BUTTON):
         {
            if(nrCmd == 0)
               fprintf(fp,"%s%sbutton(%hd, %s, %s, %s, %s, \"%s\")\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str(), label.Str());
            else
            {
               fprintf(fp,"%s%sbutton(%hd, %s, %s, %s, %s, \"%s\",\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str(), label.Str());
            
               for(short i = 1; i <= nrCmd; i++)
               {
                  strcpy(cmd,cmdLst.Extract(i));
                  if(i == nrCmd)
                     fprintf(fp,"%s   %s;)\r\n",prefix, cmd);
                  else
                     fprintf(fp,"%s   %s;\r\n",prefix, cmd);
               } 
            }              
            break;
         }
         case(COLORBOX):
         {
            BYTE *data = (BYTE*)obj->data;
            if(data[3] == 0xFF)
            {
               if(nrCmd == 0)
                  fprintf(fp,"%s%scolorbox(%hd, %s, %s, %s, %s, [%d,%d,%d,%d])\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str(), (int)data[0],(int)data[1],(int)data[2],(int)data[3]);
               else
               {
                  fprintf(fp,"%s%scolorbox(%hd, %s, %s, %s, %s, [%d,%d,%d,%d],\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str(), (int)data[0],(int)data[1],(int)data[2],(int)data[3]);
            
                  for(short i = 1; i <= nrCmd; i++)
                  {
                     strcpy(cmd,cmdLst.Extract(i));
                     if(i == nrCmd)
                        fprintf(fp,"%s   %s;)\r\n",prefix, cmd);
                     else
                        fprintf(fp,"%s   %s;\r\n",prefix, cmd);
                  } 
               }
            }
            else
            {
               if(nrCmd == 0)
                  fprintf(fp,"%s%scolorbox(%hd, %s, %s, %s, %s, [%d,%d,%d])\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str(), (int)data[0],(int)data[1],(int)data[2]);
               else
               {
                  fprintf(fp,"%s%scolorbox(%hd, %s, %s, %s, %s, [%d,%d,%d],\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str(), (int)data[0],(int)data[1],(int)data[2]);
            
                  for(short i = 1; i <= nrCmd; i++)
                  {
                     strcpy(cmd,cmdLst.Extract(i));
                     if(i == nrCmd)
                        fprintf(fp,"%s   %s;)\r\n",prefix, cmd);
                     else
                        fprintf(fp,"%s   %s;\r\n",prefix, cmd);
                  } 
               }
            }
            break;
         }
         case(PANEL):
         {
            PanelInfo *info = (PanelInfo*)obj->data;

            fprintf(fp,"%s%spanel(%hd, %s, %s, %s, %s)\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
              
            break;
         }
         case(LISTBOX):
         {
            if(nrCmd == 0)
               fprintf(fp,"%s%slistbox(%hd, %s, %s, %s, %s)\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
            else
            {
               fprintf(fp,"%s%slistbox(%hd, %s, %s, %s, %s,\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
            
               for(short i = 1; i <= nrCmd; i++)
               {
                  strcpy(cmd,cmdLst.Extract(i));
                  if(i == nrCmd)
                     fprintf(fp,"%s   %s;)\r\n",prefix, cmd);
                  else
                     fprintf(fp,"%s   %s;\r\n",prefix, cmd);
               } 
            }              
            break;
         }
         case(TEXTMENU):
         {
            if(nrCmd == 0)
               fprintf(fp,"%s%stextmenu(%hd, %s, %s, %s, 200)\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str());
            else
            {
               fprintf(fp,"%s%stextmenu(%hd, %s, %s, %s, 200,\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str());
            
               for(short i = 1; i <= nrCmd; i++)
               {
                  strcpy(cmd,cmdLst.Extract(i));
                  if(i == nrCmd)
                     fprintf(fp,"%s   %s;)\r\n",prefix, cmd);
                  else
                     fprintf(fp,"%s   %s;\r\n",prefix, cmd);
               } 
            }   
            
            break;
         }         
         case(STATICTEXT):
         {
            SIZE size;
            HDC hdc = GetDC(obj->hWnd);
		      SelectObject(hdc,controlFont);
		      GetTextExtentPoint32(hdc,label.Str(),strlen(label.Str()),&size);
         	StaticTextInfo* info = (StaticTextInfo*)obj->data;

            long type = GetWindowLong(obj->hWnd,GWL_STYLE);
            type = (type & 0x000F);

            if(info->multiLine)
            {
               if(type == SS_LEFT)
                  fprintf(fp,"%s%sstatictext(%hd, %s, %s, %s, %s, \"%s\", \"%s\")\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str(), "left", label.Str());
               else if(type == SS_RIGHT)
                  fprintf(fp,"%s%sstatictext(%hd, %s, %s, %s, %s, \"%s\", \"%s\")\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str(), "right", label.Str());
               else if(type == SS_CENTER)
                  fprintf(fp,"%s%sstatictext(%hd, %s, %s, %s, %s, \"%s\", \"%s\")\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str(), "center", label.Str());
            }
            else
            {
               if(type == SS_LEFT)
                  fprintf(fp,"%s%sstatictext(%hd, %s, %s, \"%s\", \"%s\")\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), "left", label.Str());
               else if(type == SS_RIGHT)
                  fprintf(fp,"%s%sstatictext(%hd, %s, %s, \"%s\", \"%s\")\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), "right", label.Str());
               else if(type == SS_CENTER)
                  fprintf(fp,"%s%sstatictext(%hd, %s, %s, \"%s\", \"%s\")\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), "center", label.Str());
            }

            break;
         }      
         case(TEXTBOX):
         {
            if(nrCmd == 0)
               fprintf(fp,"%s%stextbox(%hd, %s, %s, %s)\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str());
            else
            {
               fprintf(fp,"%s%stextbox(%hd, %s, %s, %s,\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str());
            
               for(short i = 1; i <= nrCmd; i++)
               {
                  strcpy(cmd,cmdLst.Extract(i));
                  if(i == nrCmd)
                     fprintf(fp,"%s  %s;)\r\n",prefix, cmd);
                  else
                     fprintf(fp,"%s  %s;\r\n",prefix, cmd);
               } 
            }   
            break;            
         } 
         case(GETMESSAGE):
         {
            if(nrCmd == 0)
               fprintf(fp,"%s%sgetmessage(%hd)\r\n",prefix, ctrlNamePrefix, nr);
            else
            {
               fprintf(fp,"%s%sgetmessage(%hd,\r\n",prefix, ctrlNamePrefix, nr);
            
               for(short i = 1; i <= nrCmd; i++)
               {
                  strcpy(cmd,cmdLst.Extract(i));
                  if(i == nrCmd)
                     fprintf(fp,"%s  %s;)\r\n",prefix, cmd);
                  else
                     fprintf(fp,"%s  %s;\r\n",prefix, cmd);
               } 
            }   
            break;            
         } 
         case(GROUP_BOX):
         {
            fprintf(fp,"%s%sgroupbox(%hd, \"%s\", %s, %s, %s, %s)\r\n",prefix, ctrlNamePrefix, nr, label.Str(), xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
            break;
         } 
         case(PROGRESSBAR):
         {
            long type = GetWindowLong(obj->hWnd,GWL_STYLE);
            if(type & PBS_VERTICAL)
               fprintf(fp,"%s%sprogressbar(%hd, %s, %s, %s, %s, \"vertical\")\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
            else
               fprintf(fp,"%s%sprogressbar(%hd, %s, %s, %s, %s, \"horizontal\")\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
            break;
         }
			case(STATUSBOX):
			{
				StatusBoxInfo* info = (StatusBoxInfo*)obj->data;
				if(!info)
					fprintf(fp,"%s%sstatusbox(%hd)\r\n",prefix, ctrlNamePrefix, nr);
				else
				{
					if(info->parts > 0)
					{
						int i;
						fprintf(fp,"%s%sstatusbox(%hd,",prefix, ctrlNamePrefix, nr);
						for(i = 0; i < info->parts-1; i++)
							fprintf(fp,"\"%s\",",info->posArray[i]);
						fprintf(fp,"\"%s\")\r\n",info->posArray[i]);
					}
					else
					{
						fprintf(fp,"%s%sstatusbox(%hd)\r\n",prefix, ctrlNamePrefix, nr);
					}
				}

				break;
			}   
         case(SLIDER):
         {
            long type = GetWindowLong(obj->hWnd,GWL_STYLE);
            if(nrCmd == 0)
            {
               if(type & TBS_VERT)         
                  fprintf(fp,"%s%sslider(%hd, %s, %s, %s, %s, \"vertical\")\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
               else
                  fprintf(fp,"%s%sslider(%hd, %s, %s, %s, %s, \"horizontal\")\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
            }
            else
            {
               if(type & TBS_VERT)         
                  fprintf(fp,"%s%sslider(%hd, %s, %s, %s, %s, \"vertical\",\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
               else
                  fprintf(fp,"%s%sslider(%hd, %s, %s, %s, %s, \"horizontal\",\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
               
               for(short i = 1; i <= nrCmd; i++)
               {
                  strcpy(cmd,cmdLst.Extract(i));
                  if(i == nrCmd)
                     fprintf(fp,"%s     %s;)\r\n",prefix, cmd);
                  else
                     fprintf(fp,"%s     %s;\r\n",prefix, cmd);
               }            
            }
            break;
         }
         case(RADIO_BUTTON):
         {
            char init[50];
            RadioButtonInfo *info = (RadioButtonInfo*)obj->data;
            short spacing = info->spacing;
				CArg carg;
				carg.Count(info->states);
            strcpy(init,carg.Extract(info->init));
            if(nrCmd == 0)
            {
               if(info->orient == 'h')
                  fprintf(fp,"%s%sradiobuttons(%hd, %s, %s, %hd, \"horizontal\", \"%s\", \"%s\")\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), spacing, info->states,init);
               else
                  fprintf(fp,"%s%sradiobuttons(%hd, %s, %s, %hd, \"vertical\", \"%s\", \"%s\")\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), spacing, info->states,init);
            }
            else
            {
               if(info->orient == 'h')
                  fprintf(fp,"%s%sradiobuttons(%hd, %s, %s, %hd, \"horizontal\", \"%s\", \"%s\",\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), spacing, info->states,init);
               else
                  fprintf(fp,"%s%sradiobuttons(%hd, %s, %s, %hd, \"vertical\", \"%s\", \"%s\",\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), spacing, info->states,init);
               
               for(short i = 1; i <= nrCmd; i++)
               {
                  strcpy(cmd,cmdLst.Extract(i));
                  if(i == nrCmd)
                     fprintf(fp,"%s     %s;)\r\n",prefix, cmd);
                  else
                     fprintf(fp,"%s     %s;\r\n",prefix, cmd);
               }            
            }         
            break;
         }   
			case(GRIDCTRL):
			{
				BabyGrid* grid = (BabyGrid*)obj->data;
				short cols = grid->getCols();
				short rows = grid->getRows();
				if(nrCmd == 0)
				{
					fprintf(fp,"%s%sgridctrl(%hd, %s, %s, %s, %s, %hd, %hd)\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str(), cols, rows);
				}
				else
				{
					fprintf(fp,"%s%sgridctrl(%hd, %s, %s, %s, %s, %hd, %hd,\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str(), cols, rows);
         
					for(short i = 1; i <= nrCmd; i++)
					{
                  strcpy(cmd,cmdLst.Extract(i));
						if(i == nrCmd)
							fprintf(fp,"%s   %s;)\r\n",prefix, cmd);
                  else
	                  fprintf(fp,"%s   %s;\r\n",prefix, cmd);
		         } 
			   }   
				break;
			}
         case(CHECKBOX):
         {
            CheckButtonInfo *info = (CheckButtonInfo*)obj->data;
				CArg carg;
				carg.Count(info->states);
            char init[50];
            strcpy(init,carg.Extract(info->init));
         
            if(nrCmd == 0)
               fprintf(fp,"%s%scheckbox(%hd, %s, %s, \"%s\", \"%s\")\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), info->states, init);
            else
            {
               fprintf(fp,"%s%scheckbox(%hd, %s, %s, \"%s\", \"%s\",\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), info->states, init);
            
               for(short i = 1; i <= nrCmd; i++)
               {
                  strcpy(cmd,cmdLst.Extract(i));
                  if(i == nrCmd)
                     fprintf(fp,"%s     %s;)\r\n",prefix, cmd);
                  else
                     fprintf(fp,"%s     %s;\r\n",prefix, cmd);
               } 
            }     
            break;
         } 
         case(HTMLBOX):
         {
            fprintf(fp,"%s%shtmlbox(%hd, %s, %s, %s, %s)\r\n",prefix, ctrlNamePrefix, nr,xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
            break;
         }
         case(MENU):
         {
            MenuInfo *info = (MenuInfo*)obj->data;

            if(info)
            {
               if(info->nrItems > 1)
               {
                  fprintf(fp,"%s%smenu(%hd, \"%s\", %s, %s,\r\n",prefix, ctrlNamePrefix, nr, info->name, info->label[0], info->cmd[0]);

                  for(int i = 1; i < info->nrItems; i++)
                  {
                     if(i == info->nrItems-1)
                        fprintf(fp,"%s          %s, %s)\r\n",prefix, info->label[i], info->cmd[i]);
                     else
                        fprintf(fp,"%s          %s, %s,\r\n",prefix, info->label[i], info->cmd[i]);
                  }
               }
               else
                  fprintf(fp,"%s%smenu(%hd, \"%s\", %s, %s)\r\n",prefix, ctrlNamePrefix, nr, info->name, info->label[0], info->cmd[0]);

            }

            break;
         }
         case(UPDOWN):
         {
            long type = GetWindowLong(obj->hWnd,GWL_STYLE);
            if(nrCmd == 0)
            {
               if(type & UDS_HORZ)         
                  fprintf(fp,"%s%supdown(%hd, %hd, %hd, %hd, %hd, \"horizontal\")\r\n",prefix, ctrlNamePrefix, nr, xo, yo, wo, ho);
               else
                  fprintf(fp,"%s%supdown(%hd, %hd, %hd, %hd, %hd, \"vertical\")\r\n",prefix, ctrlNamePrefix, nr, xo, yo, wo, ho);
            }
            else
            {
               if(type & UDS_HORZ)         
                  fprintf(fp,"%s%supdown(%hd, %hd, %hd, %hd, %hd, \"horizontal\",\r\n",prefix, ctrlNamePrefix, nr, xo, yo, wo, ho);
               else
                  fprintf(fp,"%s%supdown(%hd, %hd, %hd, %hd, %hd, \"vertical\",\r\n",prefix, ctrlNamePrefix, nr, xo, yo, wo, ho);
               
               for(short i = 1; i <= nrCmd; i++)
               {
                  strcpy(cmd,cmdLst.Extract(i));
                  if(i == nrCmd)
                     fprintf(fp,"%s     %s;)\r\n",prefix, cmd);
                  else
                     fprintf(fp,"%s     %s;\r\n",prefix, cmd);
               }            
            }
            break;
         }
         case(PLOTWINDOW):
         {
            fprintf(fp,"%s%splot1d(%hd, %s, %s, %s, %s)\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
            break;
         }
         case(IMAGEWINDOW):
         {
            fprintf(fp,"%s%splot2d(%hd, %s, %s, %s, %s)\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
            break;
         }
         case(TEXTEDITOR):
         {
            fprintf(fp,"%s%seditor(%hd, %s, %s, %s, %s)\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
            break;
         }
         case(CLIWINDOW):
         {
            fprintf(fp,"%s%scli(%hd, %s, %s, %s, %s)\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
            break;
         }
         case(OPENGLWINDOW):
         {
            fprintf(fp,"%s%splot3d(%hd, %s, %s, %s, %s)\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
            break;
         }
         case(DIVIDER):
         {
            DividerInfo *info = (DividerInfo*)obj->data;

            if(info)
            {
					if(info->orientation == WindowLayout::HORIZONTAL)
                  fprintf(fp,"%s%sdivider(%hd, %s, %s, %s, %s, \"horizontal\")\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
               else
                  fprintf(fp,"%s%sdivider(%hd, %s, %s, %s, %s, \"vertical\")\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
            }
            break;
         }
         case(PICTURE):
         {
            fprintf(fp,"%s%spicture(%hd, %s, %s, %s, %s)\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
            break;
         }
         case(TABCTRL):
         {
            if(nrCmd == 0)
               fprintf(fp,"%s%stab(%hd, %s, %s, %s, %s)\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
            else
            {
               fprintf(fp,"%s%stab(%hd, %s, %s, %s, %s,\r\n",prefix, ctrlNamePrefix, nr, xTxt.Str(), yTxt.Str(), wTxt.Str(), hTxt.Str());
 
               for(short i = 1; i <= nrCmd; i++)
               {
                  strcpy(cmd,cmdLst.Extract(i));
                  if(i == nrCmd)
                     fprintf(fp,"%s   %s;)\r\n",prefix, cmd);
                  else
                     fprintf(fp,"%s   %s;\r\n",prefix, cmd);
               } 
            }

            break;
         }
         case(TOOLBAR):
         {
            ToolBarInfo *info = (ToolBarInfo*)obj->data;

            if(info)
            {
               if(info->nrItems > 1)
               {
                  fprintf(fp,"%s%stoolbar(%hd, \"%s\", \"%s\", %s,\r\n",prefix, ctrlNamePrefix, nr, info->name.Str(), info->item[0].label.Str(), info->item[0].cmd.Str());

                  for(int i = 1; i < info->nrItems; i++)
                  {
                     if(i == info->nrItems-1)
                        fprintf(fp,"%s          \"%s\", %s)\r\n",prefix, info->item[i].label.Str(), info->item[i].cmd.Str());
                     else
                        fprintf(fp,"%s          \"%s\", %s,\r\n",prefix, info->item[i].label.Str(), info->item[i].cmd.Str());
                  }
               }
               else
                  fprintf(fp,"%s%stoolbar(%hd, \"%s\", \"%s\", %s)\r\n",prefix, ctrlNamePrefix, nr, info->name.Str(), info->item[0].label.Str(), info->item[0].cmd.Str());
            }

            break;
         }
      }
// Uncomment to save additional object data with the object definition
      if(mode2 == "definition and setpar combined")
         SaveAdditionalObjectParameters(editGUIWindow, fp, mode1, obj);

   }
   long nrObjs = orderedWidgets.size();
   return(nrObjs);
} 


Variable* GetControlVariable(WinData *win, ObjectData *obj)
{
   
// Is it in the window variable list? *************
   if(win)
   {
		Variable *next = (win->varList).next;

		for(Variable *var = next; var != NULL; var = var->next)
		{
			if(var->GetType() == CLASS)
			{
				ClassData *cData = (ClassData*)var->GetData();

				if(CheckClassValidity(cData,false) == ERR)
					continue; 

				ObjectData *objList = (ObjectData*)cData->data;
				if(objList == obj)
				{
					return(var);
				}
			}
		}
	}

   return(NULL);
}



/*********************************************************************
    Save to file 'fp' text for defining addition parameters
    for the object 'obj' e.g. setpar(~,~,"name","myobj")
**********************************************************************/

bool SaveAdditionalObjectParameters(WinData *saveWin, FILE *fp, CText &mode)
{
   char prefix[10];

   if(mode == "window with objects")
      strcpy(prefix,"      ");
   else
      strcpy(prefix,"   ");

   bool foundAny = false;

	WidgetList& orderedWidgets = saveWin->widgets.getWidgetsInCtrlNrOrder();

	fprintf(fp,"\r\n     # Set other control parameters");

// Go through the object list in numerical order

	for(ObjectData* obj: orderedWidgets)
   {
	// Check to see if any additional parameters have been defined
      bool found =	strcmp(obj->valueName,"undefined")        || (obj->dataType != NULL_VARIABLE)       ||
							obj->rangeCheck                           || obj->type == TEXTMENU                  || 
							(editGUIWindow->panicID == obj->nr())     || (editGUIWindow->abortID == obj->nr())  ||
							(editGUIWindow->defaultID == obj->nr())   || (editGUIWindow->cancelID == obj->nr()) || 
							(obj->active)                             || (obj->tabNr != -1)                     ||
							(obj->type == TABCTRL)                    || (strcmp(obj->tag,"undefined"))         || 
							(strcmp(obj->objName,"undefined"))        || (obj->tabParent != 0)                  || 
							(obj->panelParent != 0)                   || (obj->type == UPDOWN)                  ||
							(obj->statusbox > 0)                      ||  obj->menuList                         ||
                     (obj->toolTipHwnd)                        ||  (obj->fgColor != RGB(0,0,0))          || 
                     (obj->bgColor != (RGB(255,255,255) + (255<<24)) ||
                     (obj->region.bottom != 0 || obj->region.top != 0 || obj->region.left != 0 || obj->region.right != 0));

		if(found)
		{
			foundAny = true;
			bool started = false;
      
			fprintf(fp,"\r\n%ssetpar(n,%hd,",prefix,obj->nr());

			if(strcmp(obj->objName,"undefined"))
			{
				fprintf(fp,"\"objID\",\"%s\"",obj->objName);
				started = true;
			}

			if(strcmp(obj->valueName,"undefined"))
			{
				if(started) fprintf(fp,",\r\n%s            ",prefix);
				fprintf(fp,"\"valueID\",\"%s\"",obj->valueName);
				started = true;
			}

			if(obj->fgColor != RGB(0,0,0))
			{
				if(started) fprintf(fp,",\r\n%s            ",prefix);
				fprintf(fp,"\"fgcolor\",%s",Plot::GetColorStr(obj->fgColor));
				started = true;
			}

			if(obj->bgColor != (RGB(255,255,255) + (255<<24)))
			{
				if(started) fprintf(fp,",\r\n%s            ",prefix);
				fprintf(fp,"\"bgcolor\",%s",Plot::GetColorStr(obj->bgColor));
				started = true;
			}

			if(strcmp(obj->tag,"undefined"))
			{
				if(started) fprintf(fp,",\r\n%s            ",prefix);
				fprintf(fp,"\"tag\",\"%s\"",obj->tag);
				started = true;
			}

			if(obj->dataType != NULL_VARIABLE)
			{
				if(started) fprintf(fp,",\r\n%s            ",prefix);

				if(obj->dataType == INTEGER)
				   fprintf(fp,"\"type\",\"integer\"");
            else if (obj->dataType == HEX)
               fprintf(fp, "\"type\",\"hex\"");
				else if(obj->dataType == FLOAT32)
				   fprintf(fp,"\"type\",\"float\"");
				else if(obj->dataType == FLOAT64)
				   fprintf(fp,"\"type\",\"double\"");
				else if(obj->dataType == MATRIX2D)
				   fprintf(fp,"\"type\",\"array\"");
				else if(obj->dataType == UNQUOTED_STRING)
				   fprintf(fp,"\"type\",\"string\"");
				else if(obj->dataType == FLOAT32ORBLANK)
               fprintf(fp,"\"type\",\"floatorblank\"");
				else if(obj->dataType == FLOAT64ORBLANK)
               fprintf(fp,"\"type\",\"doubleorblank\""); 
			   else if(obj->dataType == FILENAME)
               fprintf(fp,"\"type\",\"filename\"");   
			   else if(obj->dataType == PATHNAME)
               fprintf(fp,"\"type\",\"pathname\"");             
				else
				   fprintf(fp,"\"type\",\"???\"");
				started = true;
			}

         if(obj->toolTipHwnd)
			{
            char txt[MAX_STR];
				if(started) fprintf(fp,",\r\n%s            ",prefix);
            strcpy(txt,obj->GetToolTip());
            ReplaceSpecialCharacters(txt,"\r","\\r",-1);
				fprintf(fp,"\"tooltip\",\"%s\"",txt);
				started = true;
			}

			if(obj->rangeCheck)
			{
				if(started) fprintf(fp,",\r\n%s            ",prefix);
				fprintf(fp,"\"range\",[%g,%g]",obj->lower,obj->upper);
				started = true;
			}

			if(obj->statusbox > 0)
			{
				if(started) fprintf(fp,",\r\n%s            ",prefix);
            fprintf(fp,"\"statusbox\",%hd",obj->statusbox);
				started = true;
			}

			if(editGUIWindow->panicID == obj->nr())
			{
				if(started) fprintf(fp,",\r\n%s            ",prefix);
				fprintf(fp,"\"mode\",\"panic\"");
				started = true;
			}

			if(editGUIWindow->abortID == obj->nr())
			{
				if(started) fprintf(fp,",\r\n%s            ",prefix);
				fprintf(fp,"\"mode\",\"abort\"");
				started = true;
			}

			if(editGUIWindow->defaultID == obj->nr())
			{
				if(started) fprintf(fp,",\r\n%s            ",prefix);
				fprintf(fp,"\"mode\",\"default\"");
				started = true;
			}

			if(editGUIWindow->cancelID == obj->nr())
			{
				if(started) fprintf(fp,",\r\n%s            ",prefix);
				fprintf(fp,"\"mode\",\"cancel\"");
				started = true;
			}

			if(obj->active)
			{
				if(started) fprintf(fp,",\r\n%s            ",prefix);
				fprintf(fp,"\"active\",\"true\"");
				started = true;
			}

			if(obj->tabNr != -1)
			{
				if(started) fprintf(fp,",\r\n%s            ",prefix);
				fprintf(fp,"\"tab_number\",%d",obj->tabNr);
				started = true;
			}

			if(obj->tabParent != 0)
			{
				if(started) fprintf(fp,",\r\n%s            ",prefix);
				fprintf(fp,"\"tabparent\",[%hd,%hd]",obj->tabParent->nr(),obj->tabPageNr);
				started = true;
			}

         if(obj->panelParent != 0)
			{
				if(started) fprintf(fp,",\r\n%s            ",prefix);
				fprintf(fp,"\"panelParent\",%hd",obj->panelParent->nr());
				started = true;
			}

			if(obj->menuList)
			{
				if(started) fprintf(fp,",\r\n%s            ",prefix);
            int sz = obj->menuListSize;
            fprintf(fp,"\"menubar\",[");
            for(int i = 0; i < sz-1; i++)
				   fprintf(fp,"%hd,",obj->menuList[i]);
				fprintf(fp,"%hd]",obj->menuList[sz-1]);
				started = true;
			}

         if(obj->region.bottom != 0 || obj->region.top != 0 || obj->region.left != 0 || obj->region.right != 0)
			{
				if(started) fprintf(fp,",\r\n%s            ",prefix);
				fprintf(fp,"\"region\",[%hd,%hd,%hd,%hd]",obj->region.left,obj->region.right,obj->region.top,obj->region.bottom);
				started = true;
			}

         if (obj->type == UPDOWN)
         {
            if (started) fprintf(fp, ",\r\n%s            ", prefix);
            fprintf(fp, "\"base\",%g,", ((UpDownInfo*)obj->data)->base);
            fprintf(fp, "\"value\",%g,", ((UpDownInfo*)obj->data)->value);
            fprintf(fp, "\"stepSize\",%g,", ((UpDownInfo*)obj->data)->stepSize);
            fprintf(fp, "\"nrSteps\",%ld", ((UpDownInfo*)obj->data)->nrSteps);
            started = true;
         }

			if(obj->type == TABCTRL)
         {
				if(started) fprintf(fp,",\r\n%s            ",prefix);
				fprintf(fp,"\"inittabs\",[");

				char *data = NULL;
            long n = TabCtrl_GetItemCount(obj->hWnd);
            if(n == 0)
               fprintf(fp,"\"\"",data);
            else
            {
               char str[100];
               TCITEM tci;
               tci.mask = TCIF_TEXT;
               tci.iImage = -1;
               tci.pszText = str;
               tci.cchTextMax = 100;
				   for(long index = 0; index < n; index++)
				   {
		            TabCtrl_GetItem(obj->hWnd,index,&tci);
					   if(index == 0)
				         fprintf(fp,"\"%s\"",tci.pszText);
					   else
				         fprintf(fp,",\"%s\"",tci.pszText);
				   }
            }
				fprintf(fp,"]");
			}


			if(obj->type == TEXTMENU)
			{
				if(started) fprintf(fp,",\r\n%s            ",prefix);
				fprintf(fp,"\"menu\",[");

				char *data = NULL;
            long n = SendMessage(obj->hWnd, CB_GETCOUNT, 0, 0);
            if(n == 0)
               fprintf(fp,"\"\"",data);
            else
            {
				   for(long index = 0; index < n; index++)
				   {
		            int len = SendMessage(obj->hWnd,CB_GETLBTEXTLEN,(WPARAM)index,(LPARAM)0);
                  if(data) 
                     delete [] data;
                  data = new char[len+1];
		            SendMessage(obj->hWnd,CB_GETLBTEXT,(WPARAM)index,(LPARAM)data);
					   if(index == 0)
				         fprintf(fp,"\"%s\"",data);
					   else
				         fprintf(fp,",\"%s\"",data);
				   }
               if(data) 
                  delete [] data;
            }
				fprintf(fp,"]");
			}
			fprintf(fp,")");
		}
   }
   return(foundAny);
}


/*********************************************************************
    Save to file 'fp' text for defining addition parameters
    for the object 'obj' e.g. setpar(~,~,"name","myobj")
**********************************************************************/

bool SaveAdditionalObjectParameters(WinData *saveWin, FILE *fp, CText &mode, ObjectData *obj)
{
   char prefix[10];

   if(mode == "window with objects")
      strcpy(prefix,"      ");
   else
      strcpy(prefix,"   ");



// Check to see if any additional parameters have been defined
   bool found =	strcmp(obj->valueName,"undefined")        || (obj->dataType != NULL_VARIABLE)       ||
						obj->rangeCheck                           || obj->type == TEXTMENU                  || 
						(editGUIWindow->panicID == obj->nr())     || (editGUIWindow->abortID == obj->nr())  ||
						(editGUIWindow->defaultID == obj->nr())   || (editGUIWindow->cancelID == obj->nr()) || 
						(obj->active)                             || (obj->tabNr != -1)                     ||
						(obj->type == TABCTRL)                    || (strcmp(obj->tag,"undefined"))         || 
						(strcmp(obj->objName,"undefined"))        || (obj->tabParent != 0)                  || 
					   (obj->panelParent != 0)                   || (obj->type == UPDOWN)                  ||
						(obj->statusbox > 0)                      ||  obj->menuList                         ||
                  (obj->toolTipHwnd)                        ||  (obj->fgColor != RGB(0,0,0))          || 
                  (obj->bgColor != (RGB(255,255,255) + (255<<24)) ||
                  (obj->region.bottom != 0 || obj->region.top != 0 || obj->region.left != 0 || obj->region.right != 0));

	if(found)
	{
		bool started = false;
      
		fprintf(fp,"%ssetpar(n,%hd,",prefix,obj->nr());

		if(strcmp(obj->objName,"undefined"))
		{
			fprintf(fp,"\"objID\",\"%s\"",obj->objName);
			started = true;
		}

		if(strcmp(obj->valueName,"undefined"))
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
			fprintf(fp,"\"valueID\",\"%s\"",obj->valueName);
			started = true;
		}

		if(obj->fgColor != RGB(0,0,0))
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
			fprintf(fp,"\"fgcolor\",%s",Plot::GetColorStr(obj->fgColor));
			started = true;
		}

		if(obj->bgColor != (RGB(255,255,255) + (255<<24)))
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
			fprintf(fp,"\"bgcolor\",%s",Plot::GetColorStr(obj->bgColor));
			started = true;
		}

		if(strcmp(obj->tag,"undefined"))
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
			fprintf(fp,"\"tag\",\"%s\"",obj->tag);
			started = true;
		}

		if(obj->dataType != NULL_VARIABLE)
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);

			if(obj->dataType == INTEGER)
				fprintf(fp,"\"type\",\"integer\"");
         else if (obj->dataType == HEX)
            fprintf(fp, "\"type\",\"hex\"");
			else if(obj->dataType == FLOAT32)
				fprintf(fp,"\"type\",\"float\"");
			else if(obj->dataType == FLOAT64)
				fprintf(fp,"\"type\",\"double\"");
			else if(obj->dataType == UNQUOTED_STRING)
				fprintf(fp,"\"type\",\"string\"");
			else if(obj->dataType == FLOAT32ORBLANK)
            fprintf(fp,"\"type\",\"floatorblank\"");
			else if(obj->dataType == FLOAT64ORBLANK)
            fprintf(fp,"\"type\",\"doubleorblank\"");  
			else if(obj->dataType == FILENAME)
            fprintf(fp,"\"type\",\"filename\"");   
			else if(obj->dataType == PATHNAME)
            fprintf(fp,"\"type\",\"pathname\""); 
			else
				fprintf(fp,"\"type\",\"???\"");
			started = true;
		}

      if(obj->toolTipHwnd)
		{
         char txt[MAX_STR];
			if(started) fprintf(fp,",\r\n%s            ",prefix);
         strcpy(txt,obj->GetToolTip());
         ReplaceSpecialCharacters(txt,"\r","\\r",-1);
			fprintf(fp,"\"tooltip\",\"%s\"",txt);
			started = true;
		}

		if(obj->rangeCheck)
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
			fprintf(fp,"\"range\",[%g,%g]",obj->lower,obj->upper);
			started = true;
		}

		if(obj->statusbox > 0)
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
         fprintf(fp,"\"statusbox\",%hd",obj->statusbox);
			started = true;
		}

		if(editGUIWindow->panicID == obj->nr())
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
			fprintf(fp,"\"mode\",\"panic\"");
			started = true;
		}

		if(editGUIWindow->abortID == obj->nr())
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
			fprintf(fp,"\"mode\",\"abort\"");
			started = true;
		}

		if(editGUIWindow->defaultID == obj->nr())
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
			fprintf(fp,"\"mode\",\"default\"");
			started = true;
		}

		if(editGUIWindow->cancelID == obj->nr())
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
			fprintf(fp,"\"mode\",\"cancel\"");
			started = true;
		}

		if(obj->active)
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
			fprintf(fp,"\"active\",\"true\"");
			started = true;
		}

		if(obj->tabNr != -1)
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
			fprintf(fp,"\"tab_number\",%d",obj->tabNr);
			started = true;
		}

		if(obj->tabParent != 0)
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
			fprintf(fp,"\"tabparent\",[%hd,%hd]",obj->tabParent->nr(),obj->tabPageNr);
			started = true;
		}


      if(obj->panelParent != 0)
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
			fprintf(fp,"\"panelParent\",%hd",obj->panelParent->nr());
			started = true;
		}

		if(obj->menuList)
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
         int sz = obj->menuListSize;
         fprintf(fp,"\"menubar\",[");
         for(int i = 0; i < sz-1; i++)
				fprintf(fp,"%hd,",obj->menuList[i]);
			fprintf(fp,"%hd]",obj->menuList[sz-1]);
			started = true;
		}

      if(obj->region.bottom != 0 || obj->region.top != 0 || obj->region.left != 0 || obj->region.right != 0)
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
			fprintf(fp,"\"region\",[%hd,%hd,%hd,%hd]",obj->region.left,obj->region.right,obj->region.top,obj->region.bottom);
			started = true;
		}

      if (obj->type == UPDOWN)
      {
         if (started) fprintf(fp, ",\r\n%s            ", prefix);
         fprintf(fp, "\"base\",%g,", ((UpDownInfo*)obj->data)->base);
         fprintf(fp, "\"value\",%g,", ((UpDownInfo*)obj->data)->value);
         fprintf(fp, "\"stepSize\",%g,", ((UpDownInfo*)obj->data)->stepSize);
         fprintf(fp, "\"nrSteps\",%ld", ((UpDownInfo*)obj->data)->nrSteps);
         started = true;
      }

      if(obj->type == TABCTRL)
      {
			if(started) fprintf(fp,",\r\n%s            ",prefix);
			fprintf(fp,"\"inittabs\",[");

			char *data = NULL;
         long n = TabCtrl_GetItemCount(obj->hWnd);
         if(n == 0)
            fprintf(fp,"\"\"",data);
         else
         {
            char str[100];
            TCITEM tci;
            tci.mask = TCIF_TEXT;
            tci.iImage = -1;
            tci.pszText = str;
            tci.cchTextMax = 100;
				for(long index = 0; index < n; index++)
				{
		         TabCtrl_GetItem(obj->hWnd,index,&tci);
					if(index == 0)
				      fprintf(fp,"\"%s\"",tci.pszText);
					else
				      fprintf(fp,",\"%s\"",tci.pszText);
				}
         }
			fprintf(fp,"]");
		}


		if(obj->type == TEXTMENU)
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
			fprintf(fp,"\"menu\",[");

			char *data = NULL;
         long n = SendMessage(obj->hWnd, CB_GETCOUNT, 0, 0);
         if(n == 0)
            fprintf(fp,"\"\"",data);
         else
         {
				for(long index = 0; index < n; index++)
				{
		         int len = SendMessage(obj->hWnd,CB_GETLBTEXTLEN,(WPARAM)index,(LPARAM)0);
               if(data) 
                  delete [] data;
               data = new char[len+1];
		         SendMessage(obj->hWnd,CB_GETLBTEXT,(WPARAM)index,(LPARAM)data);
					if(index == 0)
				      fprintf(fp,"\"%s\"",data);
					else
				      fprintf(fp,",\"%s\"",data);
				}
            if(data) 
               delete [] data;
         }
			fprintf(fp,"]");
		}
		fprintf(fp,")");
	}

  if(found)
     fprintf(fp,"\r\n");

  return(found);
}

/*********************************************************************
    Save to file 'fp' text for defining addition parameters
    for the window 'saveWin' e.g. setwindowpar(~,"name","mypar")
**********************************************************************/

bool SaveAdditionalWindowParameters(WinData *saveWin, FILE *fp, CText &mode)
{
   char prefix[10];

   if(mode == "window with objects")
      strcpy(prefix,"      ");
   else
      strcpy(prefix,"   ");

   bool foundAny = false;

	WidgetList& orderedWidgets = saveWin->widgets.getWidgetsInCtrlNrOrder();

	fprintf(fp,"\r\n\r\n     # Set other window parameters");

// Check to see if any additional parameters have been defined
   bool found = (saveWin->bkgMenu || !saveWin->showMenu);

	if(found)
	{
		foundAny = true;
		bool started = false;
      
		fprintf(fp,"\r\n%ssetwindowpar(n,",prefix);


		if(saveWin->bkgMenu)
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
			fprintf(fp,"\"bkgmenu\",%hd",saveWin->bkgMenuNr);
			started = true;
		}

		if(!saveWin->showMenu)
		{
			if(started) fprintf(fp,",\r\n%s            ",prefix);
			fprintf(fp,"\"showmenu\",\"false\"");
			started = true;
		}

		fprintf(fp,")");

   }
   return(foundAny);
}

/*********************************************************************************
    Make all selected objects the same width, height or both
*********************************************************************************/

int ResizeObjects(Interface* itfc ,char arg[])
{
   ObjectData *obj;
   static CText resizeMode;
   short r;
   WinData *designerWin;

   designerWin = GetGUIWin();

// Extract alignment mode (left, right, top, base, horiz, vert)
   if((r = ArgScan(itfc,arg,1,"resize mode","e","t",&resizeMode)) < 0)
    return(r);
    
    
// Check for the presence of an edited window
   if(!editGUIWindow)
   {
      MessageDialog(prospaWin,MB_ICONINFORMATION,"Information","No window to edit");
      return(0); // No window to edit
   }
 
// Check for objects to resize
	if(editGUIWindow->widgets.empty())
   {
      MessageDialog(prospaWin,MB_ICONINFORMATION,"Information","No objects to resize");
      return(0); // No window to edit
   }

// Select object
   short x,y;

   SetCursor(CtrlCursor);
   
// Wait for button down
   if(GetButtonDown(editGUIWindow->hWnd,x,y))
      return(0);

// Wait for button up
   WaitButtonUp(editGUIWindow->hWnd);

// Find object selected   
   obj = editGUIWindow->FindObject(x,y); 
   
   if(obj)
   {   
	// Align selected objects
	   if(resizeMode == "width")
	      editGUIWindow->ResizeSelectedObjects(obj->wo,-1);
	   else if(resizeMode == "height")
	      editGUIWindow->ResizeSelectedObjects((short)-1,(short)obj->ho);
	   else if(resizeMode == "both")
	      editGUIWindow->ResizeSelectedObjects(obj->wo,obj->ho);
   }

// Restore designer as current GUI window
   WinData::SetGUIWin(designerWin);
   SetCursor(LoadCursor(NULL,IDC_ARROW));

   return(OK);
}


/***************************************************************************************
   Select an object for editing
****************************************************************************************/

int EditObject(Interface* itfc ,char args[])
{
   ObjectData *obj;
   short winNr,objNr;
   WinData *win;
   short r;
   Variable selObj;

// Check what objects are currently selected
   if(editGUIWindow != (WinData*)0)
   {
      editGUIWindow->GetSelectedObjects(selObj);
      winNr = editGUIWindow->nr;
   }
   else
   {
      selObj.MakeNullVar();
      winNr = -1;
   }

// Extract alignment mode (left, right, top, base, horiz, vert)
   if((r = ArgScan(itfc,args,2,"winNr, objNr","ee","dv",&winNr,&selObj)) < 0)
    return(r);

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

// Select the specified object(s)

   if(selObj.GetType() == FLOAT32)
   {
      objNr = nint(selObj.GetReal());

	   if(!(obj = win->widgets.findByNr(objNr)))
      {
         ErrorMessage("object (%d,%d) not found",winNr,objNr);
         return(ERR);
      }

   // Make this object part of the edited window and redraw it
      editGUIWindow = obj->winParent;
      editGUIWindow->DeselectObjects();
      obj->selected_ = true;
   }
   else if(selObj.GetType() == MATRIX2D && selObj.GetDimY() == 1)
   {
      editGUIWindow->DeselectObjects();

      int nrObj = selObj.GetDimX();
      for(int i = 0; i < nrObj; i++)
      {
         objNr = selObj.GetMatrix2D()[0][i];
	      if(!(obj = win->widgets.findByNr(objNr)))
         {
            ErrorMessage("object (%d,%d) not found",winNr,objNr);
            return(ERR);
         }
         obj->selected_ = true;
      }
   }

   MyInvalidateRect(editGUIWindow->hWnd,NULL,0);

   itfc->nrRetValues = 0;

   return(OK);
}

/***************************************************************************************
   Attach selected GUI objects to one side of the parent window
   by passing a mode string (left, right, top , bottom) 
****************************************************************************************/

int AttachObjects(Interface* itfc ,char args[])
{
   CText attachMode;
   short r;
   WinData *designerWin;

   designerWin = GetGUIWin();

// Extract alignment mode (left, right, top, base, horiz, vert)
   if((r = ArgScan(itfc,args,1,"attach mode","e","t",&attachMode)) < 0)
    return(r);
    
// Check for the presence of an edited window
   if(!editGUIWindow)
   {
      MessageDialog(prospaWin,MB_ICONINFORMATION,"Information","No window to edit");
      return(0); // No window to edit
   }
 
// Check for objects to align
	if(editGUIWindow->widgets.empty())
   {
      MessageDialog(prospaWin,MB_ICONINFORMATION,"Information","No objects to attach");
      return(0); // No window to edit
   }
  
// Align selected objects
   if(attachMode == "left")
      editGUIWindow->AttachSelectedObjects(WindowLayout::LEFT_ATTACH);
   else if(attachMode == "right")
      editGUIWindow->AttachSelectedObjects(WindowLayout::RIGHT_ATTACH);
   else if(attachMode == "top")
      editGUIWindow->AttachSelectedObjects(WindowLayout::TOP_ATTACH);
   else if(attachMode == "base")
      editGUIWindow->AttachSelectedObjects(WindowLayout::BASE_ATTACH);


// Restore designer as current GUI window
   WinData::SetGUIWin(designerWin);
  
   return(OK);
}
/***************************************************************************************
   Align GUI objects by passing a mode string (left, right ...) 
   and then clicking on a reference object
****************************************************************************************/

int AlignObjects(Interface* itfc ,char args[])
{
   ObjectData *obj;
   CText alignMode;
   short r;
   WinData *designerWin;

   designerWin = GetGUIWin();

// Extract alignment mode (left, right, top, base, horiz, vert)
   if((r = ArgScan(itfc,args,1,"align mode","e","t",&alignMode)) < 0)
    return(r);
    
// Check for the presence of an edited window
   if(!editGUIWindow)
   {
      MessageDialog(prospaWin,MB_ICONINFORMATION,"Information","No window to edit");
      return(0); // No window to edit
   }
 
// Check for objects to align
   if(editGUIWindow->widgets.empty())
   {
      MessageDialog(prospaWin,MB_ICONINFORMATION,"Information","No objects to align");
      return(0); // No window to edit
   }

// Select object
   short x,y;

   SetCursor(CtrlCursor);
   
// Wait for button down
   if(GetButtonDown(editGUIWindow->hWnd,x,y))
      return(0);

// Wait for button up
   WaitButtonUp(editGUIWindow->hWnd);

// Find object selected
   obj = editGUIWindow->FindObject(x,y);
   
// Align selected objects
   if(obj)
   {  
	   if(alignMode == "left")
			editGUIWindow->AlignSelectedObjects(obj,WindowLayout::LEFT_ALIGN);
	   else if(alignMode == "right")
	      editGUIWindow->AlignSelectedObjects(obj,WindowLayout::RIGHT_ALIGN);
	   else if(alignMode == "top")
	      editGUIWindow->AlignSelectedObjects(obj,WindowLayout::TOP_ALIGN);
	   else if(alignMode == "base")
	      editGUIWindow->AlignSelectedObjects(obj,WindowLayout::BASE_ALIGN);
	   else if(alignMode == "horiz")
	      editGUIWindow->AlignSelectedObjects(obj,WindowLayout::HORIZ_ALIGN);
	   else if(alignMode == "vert")
	      editGUIWindow->AlignSelectedObjects(obj,WindowLayout::VERT_ALIGN);	      
   }


// Restore designer as current GUI window
   WinData::SetGUIWin(designerWin);
  
   return(OK);
}

/***************************************************************************************
   Distribute GUI objects by passing a mode string (left, right ...) 
   and then clicking on a reference object.
****************************************************************************************/

int DistributeObjects(Interface* itfc ,char args[])
{
   CText distMode;
   short r;

// Extract alignment mode (left, right, top, base, horiz, vert)
   if((r = ArgScan(itfc,args,1,"distribute mode","e","t",&distMode)) < 0)
    return(r);
    
  
// Check for the presence of an edited window
   if(!editGUIWindow)
   {
      MessageDialog(prospaWin,MB_ICONINFORMATION,"Information","No window to edit");
      return(0); // No window to edit
   }
 
// Check for objects to distribute
	if(editGUIWindow->widgets.empty())
   {
      MessageDialog(prospaWin,MB_ICONINFORMATION,"Information","No objects to distribute");
      return(0); // No window to edit
   }

// Distribute selected objects 
   if(!strncmp(distMode.Str(),"horiz",5))
      editGUIWindow->DistributeSelectedObjects(WindowLayout::HORIZ_DISTRIBUTE);
   else if(!strncmp(distMode.Str(),"vert",4))
      editGUIWindow->DistributeSelectedObjects(WindowLayout::VERT_DISTRIBUTE);
   else if(!strncmp(distMode.Str(),"winvert",7))
      editGUIWindow->DistributeSelectedObjects(WindowLayout::WINDOW_VERT_DISTRIBUTE);
   else if(!strncmp(distMode.Str(),"winhoriz",8))
      editGUIWindow->DistributeSelectedObjects(WindowLayout::WINDOW_HORIZ_DISTRIBUTE);
   else if(!strncmp(distMode.Str(),"objhoriz",7))
      editGUIWindow->DistributeInsideObject(WindowLayout::HORIZ_DISTRIBUTE);
   else if(!strncmp(distMode.Str(),"objvert",8))
      editGUIWindow->DistributeInsideObject(WindowLayout::VERT_DISTRIBUTE);      
    else if(!strncmp(distMode.Str(),"equalvert",9))
      editGUIWindow->EvenlySpaceSelectedObjects(WindowLayout::EQUAL_VERT_DISTRIBUTE);
   else if(!strncmp(distMode.Str(),"equalhoriz",10))
      editGUIWindow->EvenlySpaceSelectedObjects(WindowLayout::EQUAL_HORIZ_DISTRIBUTE); 

// Make sure edited window has focus again after distribution
   SetFocus(editGUIWindow->hWnd);
	itfc->nrRetValues = 0;
   return(OK);
}

/***************************************************************************************
*                      Copy the selected objects to a linked list                      *
***************************************************************************************/

void CutSelectedObjects(WinData *win, RECT *r)
{
   CopySelectedObjects(win,r);
   win->DeleteSelectedObjects();
}

WidgetRegistry copyPasteList;     // Copy paste array for objects

void getEnclosingRectangle(WidgetList& list, RECT* r, HWND hWnd)
{
	short x1 = 10000;
	short y1 = x1;
	short x2 = -10000;
	short y2 = x2;
	for(ObjectData* obj: list)
	{
		// Figure out this object's contribution to the 
		// enclosing rectangle.
      if(obj->xo < x1) x1 = obj->xo;
      if(obj->xo + obj->wo > x2) x2 = obj->xo + obj->wo;
      if(obj->yo < y1) y1 = obj->yo;
      if(obj->yo + obj->ho > y2) y2 = obj->yo + obj->ho;
	}
   r->left = x1;
   r->right = x2;
   r->top = y1;
   r->bottom = y2;
}

/***************************************************************************************
*   Copy the selected objects to a linked list and return the enclosing rectangle                     *
***************************************************************************************/

void CopySelectedObjects(WinData *win, RECT *r)
{
// Don't bother if there are no objects
	if(win->widgets.empty())
      return;

	copyPasteList.destroyAllWidgets();
	WidgetList selectedList;
	for(ObjectData* obj: win->widgets.getWidgets())
	{
		if (obj->isSelected())
		{
			selectedList.push_back(obj);
			copyPasteList.add(obj->Copy(win->hWnd));
		}
	}
	// And work out the enclosing rectangle
	getEnclosingRectangle(selectedList, r, win->hWnd);
}

/***************************************************************************************
*                      Paste objects in copyPasteArray to current window  
***************************************************************************************/

void PasteSelectedObjects(WinData *win, short x, short y, RECT *r, short mode)
{
   short objID;
   short dx,dy;
   
// Stop editing current window 
   if(win != editGUIWindow)
   {
      ActivateEditedGUIWin();
   }
   
// Make this window the currently edited window
   win->makeEditable();
   win->EnableResizing(true);

// Work out where to place objects
   if(mode == GUI_CLICK)
   {
      dx = x - (short)r->left;
      dy = y - (short)r->top;
   }
   else if(mode == GUI_PASTE)
   {
      dx = 10;
      dy = 10;
   }
   else
   {
      dx = 0;
      dy = 0;
   }
	ObjectData *tab = NULL;
	short nr;

// Copy each object onto new window
	for(ObjectData* obj: copyPasteList.getWidgets())
	{
      objID = win->FindNextFreeObjNr();
      ObjectData* copy = obj->Copy(win->hWnd);
      copy->nr(objID);
      SetWindowLong(copy->hWnd,GWL_ID,(LONG)objID); // make sure the window also has this id
      win->widgets.add(copy);
      copy->Place(copy->xo+dx,copy->yo+dy,copy->wo,copy->ho,true);
      copy->MoveWindowToTop(); // Makes sure tabs stay at the bottom!
      obj->xo = copy->xo;
      obj->yo = copy->yo;
      obj->xSzOffset = copy->xSzOffset;
      obj->ySzOffset = copy->ySzOffset;
      copy->EnableObject(false);
      copy->selected_ = true;
      copy->visible = true;
      copy->Show(true);   
		copy->winParent = win;
		if(FindTabCtrl(copy,&tab,nr))
		{
			copy->tabPageNr = nr;
			copy->tabParent = tab;
		}
   } 

	WidgetList selectedList;
	for(ObjectData* obj: win->widgets.getWidgets())
	{
		if (obj->isSelected())
		{
			selectedList.push_back(obj);
		}
	}
	// And work out the enclosing rectangle
	getEnclosingRectangle(selectedList, r, win->hWnd);
}


/***********************************************************************
   Check to see if any objects are currently copied   
***********************************************************************/

bool AreObjectsCopied()
{
   return(!copyPasteList.empty());
}

/***********************************************************************
   See if there is a tab control under object. If so return the 
	tab page number.
***********************************************************************/

bool FindTabCtrl(ObjectData *obj, ObjectData **tab, short &nr)
{
	if(obj->type == TABCTRL)
		return(false);

   *tab = obj->winParent->FindTabObject(obj->xo,obj->yo);

	if(!(*tab))
		return(false);

	nr = TabCtrl_GetCurSel((*tab)->hWnd);

	return(true);
}

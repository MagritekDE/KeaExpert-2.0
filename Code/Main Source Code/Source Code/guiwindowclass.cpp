/***************************************************************************************
  Routines for manipulating the window list. This is a linked list of windows   
  Each window contains a linked list of objects which can appear in the window  
***************************************************************************************/

#include "stdafx.h"
#include "cArg.h"
#include "debug.h"
#include "defineWindows.h"
#include "defines.h"
#include "edit_class.h"
#include "events_edit.h"
#include "edit_files.h"
#include "files.h"
#include "globals.h"
#include "guiInteractiveWindowBuilder.h"
#include "guiModifyObjectParameters.h"
#include "guiObjectClass.h"
#include "guiWindowClass.h"
#include "guiWindowCLI.h"
#include "guiWindowsEvents.h"
#include "htmlviewer.h"
#include "interface.h"
#include "main.h"
#include "mymath.h"
#include "plot.h"
#include "PlotWindow.h"
#include "scanstrings.h"
#include "string_utilities.h"
#include "StringPairs.h"
#include "thread.h"
#include "variablesOther.h"
#include "WidgetRegistry.h"
#include <algorithm>
#include <string>
#include <vector>
#include <utility>
#include "memoryLeak.h"

using std::string;
using std::find;
using std::pair;
using std::vector;

#pragma warning (disable: 4312) // Ignore Get/SetWindowLong warnings

bool dialogDisplayed = false; // Whether the window is a dialog or not

WinData *rootWin;        // Base window for window list
char title[MAX_STR]; // Window title (backup for resizing and moving labels)


/***************************************************************************************
   Constructor

   Revision history
   8/2/07 ... added reset for name parameter, modified name parameter to title
***************************************************************************************/

WinData::WinData()
{
   next = (WinData*)0;
   last = (WinData*)0;

   validationCode = GetNextObjectValidationCode();
   cancelID = -1;
   defaultID = -1;
   panicID = -1;
   abortID = -1;
   activeCtrl = false;
   title = new char[1];
   strcpy(title,"");
   name = new char[1];
   strcpy(name,"");
   varList.next = NULL;
   varList.last = NULL;
   type = NORMAL_WIN;
   inRemoval = false;
   hWnd = NULL;
   exitProcName[0] = '\0';
   displayObjCtrlNrs = false;
   displayObjTabNrs = false;
   modifyingCtrlNrs  = false;
   modifyingTabNrs  = false;
   activated = true;
   showMenu = true;
   tabMode = TAB_BY_CTRL_NUMBER;
   parent = NULL;
   objWithFocus = NULL;
   hAccelTable = NULL;
   keepInFront = true;
   fixedObjects = false;
   constrained = false;
   permanent = false;
   xSzScale = 0;  
   xSzOffset = 0;
   ySzScale = 0;
   ySzOffset = 0;
   wSzScale = 0;
   wSzOffset = 0;
   hSzScale = 0;
   hSzOffset = 0;
   toolbar = NULL;
   statusbox = NULL;
   isMainWindow = false;
   nrMenuObjs = 0;
   menuList = NULL;
   menuListSize = 0;
   menu = NULL;
   blankStatusBar = NULL;
   blankToolBar = NULL;
   bkgMenu = NULL;
   bkgMenuNr = -1;
   defaultToolbar = NULL;
   defaultStatusbox = NULL;
   debugger = false;
   debugging = false;
   threadCnt = 0;
   gridSpacing = 10;
   showGrid = false;
   snapToGrid = false;
   sizeLimits.minWidth = -1;
   sizeLimits.maxWidth = -1;
   sizeLimits.minHeight = -1;
   sizeLimits.maxHeight = -1;
   cacheProc = false;
   blockNonActiveCtrl = false;
   titleUpdate = true;
	mergeTitle = false; 
   menuObj = NULL;
   accelTable = NULL;
   rect = NULL;
   bkgColor = RGB(255,255,255) + (255<<24);
	alive(true);
   visible = false;
   drawing = true;
	currentEditor = NULL;
	devIF = NULL;
}



/***************************************************************************************
   Destructor
***************************************************************************************/

WinData::~WinData()
{
   inRemoval = true;
   validationCode = 0; // Makes sure this window can no longer be accessed
 //  winVar.SetData(0);
 //  winVar.SetType(NULL); // Disconnect winvar from varList
	winVar.FreeData(); 
   Destroy();
}

/***************************************************************************************
     Copy the all the object in win into an array called undoArray     
***************************************************************************************/

void WinData::CopyAllObjects()
{
	undoArray.destroyAllWidgets();

// No objects so do nothing
	if(widgets.empty())
      return;
      
// Copy these objects into array
	for(ObjectData* obj: widgets.getWidgets())
   {
      undoArray.add(obj->Copy(hWnd));
	}    

// Hook up the children to their tab parents.
	for(ObjectData* obj: widgets.getWidgets())
	{// For each of the tab children 
		if (obj->isTabChild())
		{
			int childNr = obj->nr();
			int parentNr = obj->getTabParent()->nr();

			undoArray.findByNr(childNr)->tabParent = 
				undoArray.findByNr(parentNr);
		}
	}
  //    TextMessage("copying %hd objects\n",undoObjCnt);
}


/***************************************************************************************
*                      Paste objects in undoArray to current window                      *
***************************************************************************************/

void WinData::UndoObjectAction()
{  
	if(undoArray.empty())
      return;

// Remove all objects in the window
   widgets.destroyAllWidgets();

// Copy each object onto window
	ObjectData* selObj = NULL;
	for(ObjectData* obj: undoArray.getWidgets())
   {
      ObjectData *copy = obj->Copy(hWnd);
      widgets.add(copy); 
      if(obj->selected_)
         selObj = copy;
      copy->Show(true); 
   }  

// Hook up the children to their tab parents.
	for(ObjectData* obj: undoArray.getWidgets())
	{// For each of the tab children 
		if (obj->isTabChild())
		{
			int childNr = obj->nr();
			int parentNr = obj->getTabParent()->nr();

			widgets.findByNr(childNr)->tabParent = 
				widgets.findByNr(parentNr);
		}
	}
   undoArray.destroyAllWidgets();
   this->updateControlVisibilityFromTabs();
   this->makeEditable();

// Make the selected window the top window
   if(selObj)
   {
      selObj->selected_ = true;
      selObj->MoveWindowToTop();
   }

   MyInvalidateRect(this->hWnd,NULL,false);  
}


/***********************************************************************
   Check to see if any objects are selected in the specified GUI window   
***********************************************************************/

bool WinData::AreObjectsSelected()
{
	return widgets.findSelected() ? true : false;
}

/***************************************************************************************
   Return name of window
***************************************************************************************/

char* WinData::GetTitle()
{
   return(title);
}

/***************************************************************************************
   Return window title
***************************************************************************************/

void WinData::GetWindowText(CText &txt)
{
   long n = ::GetWindowTextLength(hWnd);
   char *title = new char[n+1];
   n = ::GetWindowText(hWnd,title,n+1);
   title[n] = '\0';
   txt.Assign(title);
	delete[] title;
}

/***************************************************************************************
   Return command from object identified by ID
***************************************************************************************/

char* WinData::GetObjCommand(int ID)
{
	char *command = this->widgets.getCommand(ID);
	return(command);
}

/***************************************************************************************
   Add a window to the top of the window linked list
***************************************************************************************/

WinData* WinData::AddWin(int winNr, char *winName,int x, int y, int w, int h, bool resize, char* className)
{
   WinData *win = new WinData;
   char *s = new char[strlen(winName)+1];
   strncpy_s(s,strlen(winName)+1,winName,_TRUNCATE);
	delete [] win->title;
   win->title = s;
   if(next != (WinData*)0)
      next->last = win;
   win->next = next;
   win->last = this;
	next = win;
   win->nr = winNr;
   win->wx = x;
   win->wy = y;
   win->ww = w;
   win->wh = h;
   win->hAccelTable = NULL;
   
   if(resize)
   {
	   win->hWnd = CreateWindow(className, winName,
                          WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MAXIMIZEBOX| WS_MINIMIZEBOX	,
                          x, y, w, h,
                          prospaWin, NULL, prospaInstance, NULL);
   }
   else
   {
	   win->hWnd = CreateWindow(className, winName,
                          WS_OVERLAPPED | WS_SYSMENU,
                          x, y, w, h,
                          prospaWin, NULL, prospaInstance, NULL);
   }
                       

   return(win);
}


/***************************************************************************************
   Search for a window by name
***************************************************************************************/

WinData* WinData::FindWinByName(char *winName)
{
   for(WinData *w = next; w != NULL; w = w->next)
   {
      if(w->name && !strcmp(winName,w->name))
         return(w);
   }
   return((WinData*)0);
}

/***************************************************************************************
   Search for a window by title
***************************************************************************************/

WinData* WinData::FindWinByTitle(char *winTitle)
{
   CText title;
   for(WinData *w = next; w != NULL; w = w->next)
   {
      w->GetWindowText(title);
      if(title == winTitle)
         return(w);
   }
   return((WinData*)0);
}

/***************************************************************************************
   Search for a window by title
***************************************************************************************/

WinData* WinData::FindWinByPartialTitle(char* winTitle)
{
   CText title;
   int len = strlen(winTitle);
   for (WinData* w = next; w != NULL; w = w->next)
   {
      w->GetWindowText(title);
      if (title.Size() >= len)
      {
         if (!strncmp(title.Str(), winTitle, len))
            return(w);
      }
   }
   return((WinData*)0);
}

/***************************************************************************************
   Search for a window given screen coordinates (p)
***************************************************************************************/

WinData* WinData::FindWinByPosition(POINT p)
{
   HWND hWnd;
   
   if(hWnd = WindowFromPoint(p))
      return(FindWinByHWND(hWnd));

   return((WinData*)0);
}

/***************************************************************************************
   Search for a window given its unique number
***************************************************************************************/

WinData* WinData::FindWinByNr(short winNr)
{
   for(WinData *w = next; w != NULL; w = w->next)
   {
      if(w->nr == winNr)
         return(w);
   }
   return((WinData*)0);
}

/***************************************************************************************
  Get the next window in the gui linked list
***************************************************************************************/

WinData* WinData::GetNextWin()
{
   //for(WinData *w = next; w != NULL; w = w->next)
   //{
   //   return(w);
   //}
   //return((WinData*)0);
   return(next);
}


/***************************************************************
  Returns the next number for a duplicate file name
  by searching through all objects of the same type
****************************************************************/

int WinData::GetNextFileNumber(ObjectData *curObj, CText &name)
{
   int numbers[100],i;
   int cnt = 1;
   bool found = false;

   for(int i = 1; i < 100; i++)
      numbers[i] = 0;

   switch(curObj->type)
   {
      case(PLOTWINDOW):
      {
         WinData *w = this->next;
         while(w != NULL)
         {
				for(ObjectData* obj: widgets.getWidgets())
				{
               if(obj->type == curObj->type)
               {
                  PlotWindow *pp = (PlotWindow*)obj->data;

                  if(pp->fileName() == name)
                  {
                     if(obj == curObj)
                        return(pp->fileNumber);

                     numbers[cnt++] = pp->fileNumber;
                     if(obj != curObj)
                         found = true;
                  }
               }
            }
            w = w->next;
         }
      }
   }

   if(!found)
   {
      return(-1);
   }

   for(i = 1; i < 100; i++)
   {
      if(numbers[i] == 0)
         break;
   }

   return(i);
}

/***************************************************************************************
   Get the previous window in the gui linked list
***************************************************************************************/

WinData* WinData::GetPreviousWin()
{
   //for(WinData *w = last; w != NULL; w = w->last)
   //{
   //   return(w);
   //}
   //return((WinData*)0);
   return(last);
}

/***************************************************************************************
  Get the last window in the gui linked list
***************************************************************************************/

WinData* WinData::GetLastWin()
{
   WinData *w = this;
   while(w->next != NULL)
   {
      w = w->next;
   }
   return(w);
}

/***************************************************************************************
  Search for a window given its windows handle
***************************************************************************************/

WinData* WinData::FindWinByHWND(HWND win)
{
   for(WinData *w = next; w != NULL; w = w->next)
   {
      if(w->hWnd == win)
         return(w);
   }
   return((WinData*)0);
}


/***************************************************************************************
   Search for the next unused window number
***************************************************************************************/

short WinData::FindNextFreeNr()
{
   short nr = 1;
   WinData *w;
   
   while(1)
   {
	   for(w = next; w != NULL; w = w->next)
	   {
	      if(w->nr == nr)
	         break;
	   }
	   if(w == NULL)
	      return(nr);
	   else
	      nr++;
	}
}

/***************************************************************************************
  Returns the next unused object number in an object list               
***************************************************************************************/

short WinData::FindNextFreeObjNr()
{
   short objNr;
   
   for(objNr = 1;;objNr++)
   {
		if (!widgets.findByNr(objNr))
		{
			break;
		}
   }  
   return(objNr); 
}

/***************************************************************************************
  Search for a procedure taken from path\macro:name 
  If a macro extension is not passed then try .mac and .pex
  Note that path and macro are modified and so should be long enough
  to hold a complete pathname.
  NOTE the arguments strings are modified and should be MAX_PATH c strings
***************************************************************************************/

Variable* WinData::GetProcedure(char *path, char *macro, char *name)
{
   ProcedureInfo *procInfo;
   Variable *var;
   char fullNameMac[MAX_PATH];
   char fullNamePex[MAX_PATH];
   char curPath[MAX_PATH];

// Assume its a macro with extension .mac
   strncpy(fullNameMac,macro,MAX_PATH-1);
   AddExtension(fullNameMac,".mac");
   strncpy(fullNamePex,macro,MAX_PATH-1);
   AddExtension(fullNamePex,".pex");

   GetCurrentDirectory(MAX_PATH,curPath);

// First search for a complete match - same path, same macro and same procedure

	for(var = procList.next; var != NULL; var = var->next)
	{
      procInfo = (ProcedureInfo*)var->GetString(); 

      if(name[0] == '\0') // Search for path\macro
      {
         if(!stricmp(var->GetName(),macro))
         {
            if(path[0] != '\0') // Search for path\macro
            {
               if(ComparePathNames(procInfo->macroPath,path) &&  (!stricmp(procInfo->macroName,fullNameMac)))
               {
                  strncpy(macro,fullNameMac,MAX_PATH-1);
                  return(var);
               }

               if(ComparePathNames(procInfo->macroPath,path) &&  (!stricmp(procInfo->macroName,fullNamePex)))
               {
                  strncpy(macro,fullNamePex,MAX_PATH-1);
                  return(var);
               }
            }
            else // Search for curpath\macro
            {
               if(ComparePathNames(procInfo->macroPath,curPath) &&  (!stricmp(procInfo->macroName,fullNameMac)))
               {
                  strncpy(macro,fullNameMac,MAX_PATH-1);
                  strncpy(path,procInfo->macroPath,MAX_PATH-1);
	               return(var);
               }
               if(ComparePathNames(procInfo->macroPath,curPath) &&  (!stricmp(procInfo->macroName,fullNamePex)))
               {
                  strncpy(macro,fullNamePex,MAX_PATH-1);
                  strncpy(path,procInfo->macroPath,MAX_PATH-1);
	               return(var);
               }
            }
         }
      }
      
      else if(!stricmp(var->GetName(),name))
	   {
         if(path[0] != '\0') // Search for path\macro:name
         {
            if(ComparePathNames(procInfo->macroPath,path) &&  (!stricmp(procInfo->macroName,fullNameMac)))
            {
               strncpy(macro,fullNameMac,MAX_PATH-1);
               return(var);
            }

            if(ComparePathNames(procInfo->macroPath,path) &&  (!stricmp(procInfo->macroName,fullNamePex)))
            {
               strncpy(macro,fullNamePex,MAX_PATH-1);
               return(var);
            }
         }
         else // Search for curPath\macro:name
         {
            if(ComparePathNames(procInfo->macroPath,curPath) &&  (!stricmp(procInfo->macroName,fullNameMac)))
            {
               strncpy(macro,fullNameMac,MAX_PATH-1);
               strncpy(path,procInfo->macroPath,MAX_PATH-1);
	            return(var);
            }
            if(ComparePathNames(procInfo->macroPath,curPath) &&  (!stricmp(procInfo->macroName,fullNamePex)))
            {
               strncpy(macro,fullNamePex,MAX_PATH-1);
               strncpy(path,procInfo->macroPath,MAX_PATH-1);
	            return(var);
            }
         }
	   }
	}

// Can't find macro in cache - perhaps it is in current directory
  if(path[0] != '\0') // Search for path\macro:name
  {
     if(IsFile(fullNameMac))
        return(NULL);
  }

// Return the first match found ignoring the current path
   for(var = procList.next; var != NULL; var = var->next)
	{
      procInfo = (ProcedureInfo*)var->GetString(); 

      if(name[0] == '\0')
      {
         if(!stricmp(var->GetName(),macro))
         {
            if(!stricmp(procInfo->macroName,fullNameMac))
            {
               strncpy(macro,fullNameMac,MAX_PATH-1);
               strncpy(path,procInfo->macroPath,MAX_PATH-1);
	            return(var);
            }
            if(!stricmp(procInfo->macroName,fullNamePex))
            {
               strncpy(macro,fullNamePex,MAX_PATH-1);
               strncpy(path,procInfo->macroPath,MAX_PATH-1);
	            return(var);
            } 
         }
      }
      
      else if(!stricmp(var->GetName(),name))
	   {
         if(!stricmp(procInfo->macroName,fullNameMac))
         {
            strncpy(macro,fullNameMac,MAX_PATH-1);
            strncpy(path,procInfo->macroPath,MAX_PATH-1);
	         return(var);
         }
         if(!stricmp(procInfo->macroName,fullNamePex))
         {
            strncpy(macro,fullNamePex,MAX_PATH-1);
            strncpy(path,procInfo->macroPath,MAX_PATH-1);
	         return(var);
         }  
	   }
	}

   return(NULL);
}


/***************************************************************************************
  Finds the first object in the object list which includes the coordinates (x,y)  
***************************************************************************************/

ObjectData* WinData::FindObject(short x, short y)
{
	for(ObjectData *obj: widgets.getWidgets())
   {
      if(obj->visible && !obj->panelParent)
      {
         if(obj->type == GROUP_BOX)
         {
	         if((x >= obj->xo-2 && x <= obj->xo+2 &&
	             y >= obj->yo-2 && y <= obj->yo+obj->ho+2) ||
	            (x >= obj->xo+obj->wo-5 && x <= obj->xo+obj->wo+2 &&
	             y >= obj->yo-2 && y <= obj->yo+obj->ho+2) ||
	            (x >= obj->xo-2 && x <= obj->xo+obj->wo+2 &&
	             y >= obj->yo-2 && y <= obj->yo+10) ||
	            (x >= obj->xo-2 && x <= obj->xo+obj->wo+2 &&
	             y >= obj->yo+obj->ho-2 && y <= obj->yo+obj->ho+2))	          
	         {
	            return(obj);
	         }
	      }
         else if(obj->type == TABCTRL)
         {
	         if((x >= obj->xo-2 && x <= obj->xo+2 &&
	             y >= obj->yo-2 && y <= obj->yo+obj->ho+2) ||
	            (x >= obj->xo+obj->wo-5 && x <= obj->xo+obj->wo+2 &&
	             y >= obj->yo-2 && y <= obj->yo+obj->ho+2) ||
	            (x >= obj->xo-2 && x <= obj->xo+obj->wo+2 &&
	             y >= obj->yo-2 && y <= obj->yo+20) ||
	            (x >= obj->xo-2 && x <= obj->xo+obj->wo+2 &&
	             y >= obj->yo+obj->ho-2 && y <= obj->yo+obj->ho+2))	          
	         {
	            return(obj);
	         }
	      }
         else if(obj->type == PANEL)
         {
            PanelInfo* info = (PanelInfo*)obj->data;
            int left = info->x;
            int right = info->x+info->w+20;
            int top = info->y;
            int base = info->y+info->h;

	         if((x >= left-2 && x <= left+2 && y >= top-2 && y <= base+2) ||
	            (x >= right-20 && x <= right+2 && y >= top-2 && y <= base+2) ||
	            (x >= left-2 && x <= right+2 && y >= top-2 && y <= top+2) ||
	            (x >= left-2 && x <= right+2 && y >= base-2 && y <= base+2))	          
	         {
	            return(obj);
	         }
	      }
	      else
         {
	         if(x >= obj->xo && x <= obj->xo+obj->wo &&
	            y >= obj->yo && y <= obj->yo+obj->ho)
	         {
	            return(obj);
	         }
	      }
      }
   }  
   return(NULL); 
}

ObjectData* WinData::FindObjectIfInside(short x, short y)
{
	for(ObjectData *obj: widgets.getWidgets())
   {
      if(obj->visible)
      {
         if(obj->type == PANEL)
         {
            PanelInfo* info = (PanelInfo*)obj->data;
            int left = info->x;
            int right = info->x+info->w+20;
            int top = info->y;
            int base = info->y+info->h;

	         if(x >= left && x <= right &&
	            y >= top && y <= base)	          
	         {
	            return(obj);
	         }
	      }
         else if(obj->type == TABCTRL || obj->type == GROUP_BOX)
            continue;
	      else
         {
	         if(x >= obj->xo && x <= obj->xo+obj->wo &&
	            y >= obj->yo && y <= obj->yo+obj->ho)
	         {
	            return(obj);
	         }
	      }
      }
   }  
   return(NULL); 
}

ObjectData* WinData::FindTabObject(short x, short y)
{
   for(ObjectData *obj: widgets.getWidgets())
   {
      if(obj->visible)
      {
         if(obj->type == TABCTRL)
         {
	         if(x >= obj->xo && x <= obj->xo+obj->wo &&
	            y >= obj->yo && y <= obj->yo+obj->ho)
	         {
	            return(obj);
	         }
	      }
      }
   }  
   return(NULL); 
}


/***************************************************************************************
  Finds the first object in the object list which includes the coordinates (x,y)  
***************************************************************************************/

ObjectData* WinData::InDivider(short x, short y)
{
	for(ObjectData *obj: widgets.getWidgets())
   {
      if(obj->visible && obj->type == DIVIDER)
      {
         if(x >= obj->xo && x <= obj->xo+obj->wo &&
            y >= obj->yo && y <= obj->yo+obj->ho)
         {
            return(obj);
         }
      }
   }  
   return(NULL); 
}

/***************************************************************************************
  Returns the first object with a name member = "txt     
***************************************************************************************/

ObjectData* WinData::FindObjectByValueID(const char* const txt)
{
	return widgets.findByValueID(txt);
}

/***************************************************************************************
  Returns the first object with a name member = "txt     
***************************************************************************************/

ObjectData* WinData::FindObjectByObjectID(const char* const txt)
{
	return widgets.findByObjectID(txt);
}

/***************************************************************************************
  Returns the first object with number = nr     
***************************************************************************************/

ObjectData* WinData::FindObjectByNr(short nr)
{
	return widgets.findByNr(nr);
}

/***************************************************************************************
  Returns the first object with type = type    
***************************************************************************************/

ObjectData* WinData::FindObjectByType(short type)
{
	return widgets.findByType(type);
}

/***************************************************************************************
  Return a vector of the selected objects using object number    
***************************************************************************************/

void WinData::GetSelectedObjects(Variable& selObjVec)
{
   int cnt = 0;

	for(ObjectData* obj: widgets.getWidgets())
   {
      if(obj->isSelected()) 
         cnt++;
   }
   if(cnt == 0)
   {
      selObjVec.MakeNullVar();
      return;
   }

   selObjVec.MakeAndLoadMatrix2D(NULL,cnt,1);

   float **vec = selObjVec.GetMatrix2D();

   cnt = 0;
	for(ObjectData* obj: widgets.getWidgets())
   {
      if(obj->isSelected()) 
         vec[0][cnt++] = obj->nr();
   }
}

/***************************************************************************************
  Move srcObj to the beginning of the window object list                         
***************************************************************************************/

void WinData::MoveObjectToStart(ObjectData *srcObj)
{	
	WidgetList list = widgets.getWidgets();
	WidgetListIterator wit = find(list.begin(), list.end(), srcObj);
	if (wit != list.end())
	{
		list.erase(wit);
		list.push_front(srcObj);
	}
}

/***************************************************************************************
  Returns the status region from the object list                         
***************************************************************************************/

ObjectData* WinData::FindStatusBox()
{
	return widgets.findByType(STATUSBOX);
}

/***************************************************************************************
  Enables or disables all objects in a window which have enable flag = true                         
***************************************************************************************/

void WinData::EnableObjects(bool enable)
{
	for(ObjectData* obj: widgets.getWidgets())
   {
      if(obj->enable) 
         obj->EnableObject(enable);

      if(obj->type == HTMLBOX)
         EnableHtmlViewer(obj->hWnd, enable);
   }  
}

void WinData::EnableResizing(bool enable)
{
   long style = GetWindowLong(hWnd,GWL_STYLE);

   if(enable)
 	   SetWindowLong(hWnd,GWL_STYLE,WS_SIZEBOX | style);
   else
 	   SetWindowLong(hWnd,GWL_STYLE,(~WS_SIZEBOX) & style);


   // The following is supposed to toggle the appearance of the status bar
   // resize region between the edit and run modes but windows just ignores
   // it.
   //ObjectData *obj = FindStatusBox();
   //long ans;
   //if(obj)
   //{
   //   style = GetWindowLong(obj->hWnd,GWL_STYLE);
   //   if(enable)
  	//      ans = SetWindowLong(obj->hWnd,GWL_STYLE, style | SBARS_SIZEGRIP); 
   //   else
  	//      ans = SetWindowLong(obj->hWnd,GWL_STYLE, style & (~SBARS_SIZEGRIP)); 
   //}
}


float* WinData::GetControlList(int *sz)
{
   float *ctrlNrs;
   int index = 0;

// Allocate space for numbers
   ctrlNrs = new float[widgets.size()];

// Get the control numbers
   index = 0;
	for(ObjectData* obj: widgets.getWidgets())
   {
      ctrlNrs[index++] = obj->nr();
   }
   *sz = index;
   return(ctrlNrs);
}


/***************************************************************************************
   Enables or disables all objects in a window                          
***************************************************************************************/

void WinData::SelectAllObjects(bool select)
{
	widgets.selectAll(select);
}

/***************************************************************************************
   Set all the object numbers to -1                                                
***************************************************************************************/

void WinData::ResetControlNumbers()
{
	widgets.resetControlNumbers();
   objectNrCnt = 1;
}

/***************************************************************************************
   Set all the objects tab numbers to -1                                                *
***************************************************************************************/

void WinData::ResetTabNumbers()
{
	widgets.resetTabNumbers();
   objectNrCnt = 1;
}


/***************************************************************************************
   Selects objects inside a specified rectange                    
***************************************************************************************/

void WinData::SelectObjects(short x1, short y1, short x2, short y2)
{
	widgets.selectAllWithinRect(x1,y1,x2,y2);
}


/***************************************************************************************
   Enables or disables all objects in a window                
***************************************************************************************/

void WinData::DeselectObjects(void)
{
	widgets.selectAll(false);
}


/***************************************************************************************
  Deletes all selected object in the current gui window               
***************************************************************************************/

void WinData::DeleteSelectedObjects()
{
	bool deletedSomethingThisTime = false;
	do
	{
		deletedSomethingThisTime = false;
 		for(ObjectData *obj: widgets.getWidgets())
		{
			if(obj->selected_)
			{
				delete obj;
				widgets.remove(obj);
				deletedSomethingThisTime = true;
				break;
		   }  
		}  
	} while (deletedSomethingThisTime);

	widgets.sort();
}

void WinData::AlignSelectedObjects(ObjectData *alignObj, short dir)
{
   short xa,ya,wa,ha;
   
   xa = alignObj->xo;
   ya = alignObj->yo;
   wa = alignObj->wo;
   ha = alignObj->ho;

   for(ObjectData *obj: widgets.getWidgets())
   {
      if(obj->selected_)
      {
         switch(dir)
         {
				case(WindowLayout::LEFT_ALIGN):
            {
               obj->xo = xa;
               obj->xSzScale = alignObj->xSzScale;
               obj->xSzOffset = alignObj->xSzOffset;
               break;
            }
				case(WindowLayout::RIGHT_ALIGN):
            {
               obj->xo = xa + wa - obj->wo;
               obj->xSzScale = alignObj->xSzScale;
               obj->xSzOffset = alignObj->xSzOffset+wa-obj->wo;
               break;
            }
				case(WindowLayout::TOP_ALIGN):
            {
               obj->yo = ya;
               obj->ySzScale = alignObj->ySzScale;
               obj->ySzOffset = alignObj->ySzOffset;
               break;
            }
				case(WindowLayout::BASE_ALIGN):
            {
               obj->yo = ya + ha - obj->ho;
               obj->ySzScale = alignObj->ySzScale;
               obj->ySzOffset = alignObj->ySzOffset+ha-obj->ho;
               break;
            } 
				case(WindowLayout::HORIZ_ALIGN):
            {
               obj->xo = xa + wa/2 - obj->wo/2;
               obj->xSzScale = alignObj->xSzScale;
               obj->xSzOffset = alignObj->xSzOffset+wa/2-obj->wo/2;
               break;
            } 
				case(WindowLayout::VERT_ALIGN):
            {
               obj->yo = ya + ha/2 - obj->ho/2;
               obj->ySzScale = alignObj->ySzScale;
               obj->ySzOffset = alignObj->ySzOffset+ha/2-obj->ho/2;
               break;
            }                                                 
         } 
         obj->Place(obj->xo,obj->yo,obj->wo,obj->ho,false);
      }  
   }  
}

// Attach the selected objects to one side of the window

void WinData::AttachSelectedObjects(short dir)
{
   RECT r;
   GetClientRect(hWnd,&r);

   long width = r.right;
   long height = r.bottom;

	for(ObjectData *obj: widgets.getWidgets())
   {
      if(obj->selected_)
      {
         switch(dir)
         {
            case(WindowLayout::LEFT_ATTACH):
            {
               obj->xSzScale = 0.0;
               obj->xSzOffset = obj->xo;
               break;
            }
            case(WindowLayout::TOP_ATTACH):
            {
               obj->ySzScale = 0.0;
               obj->ySzOffset = obj->yo;
               break;
            }
            case(WindowLayout::RIGHT_ATTACH):
            {
               obj->xSzScale = 1.0;
               obj->xSzOffset = obj->xo-width;
               break;
            }
            case(WindowLayout::BASE_ATTACH):
            {
               obj->ySzScale = 1.0;
               obj->ySzOffset = obj->yo-height;
               break;
            }                                     
         } 
         obj->Place(obj->xo,obj->yo,obj->wo,obj->ho,false);
      }  
   }  
}

void WinData::DistributeInsideObject(short dir)
{
   ObjectData *movObj = 0;  
   ObjectData *enclosingObj; 
   short xe,ye,we,he;
   RECT r;
   short totObjWidth = 0;   // Total width of all objects in window
   short totObjHeight = 0;  // Total height of all objects in window
   short objCnt = 0;
   short xSpacing,ySpacing; // Spacing between distributed objects
   short xpos,ypos;         // Position of left or top edge of each window
   WinData *designerWin;

// Don't bother if there are no objects
	if(widgets.empty())
   {
 //     SetGUIWin(designerWin); //<-- check this
      return;
   }

// Select object to distribute controls in
   short x,y;

   designerWin = GetGUIWin();
   SetCursor(CtrlCursor);

// Wait for button down
   if(GetButtonDown(editGUIWindow->hWnd,x,y))
      return;

// Wait for button up
   WaitButtonUp(editGUIWindow->hWnd);

// Find object selected
   if(!(enclosingObj = editGUIWindow->FindObject(x,y))) 
   {
		WinData::SetGUIWin(designerWin);
      return;
   }
      
// Corner coordinates and dimensions of enclosing object 
   xe = enclosingObj->xo;
   ye = enclosingObj->yo;
   we = enclosingObj->wo;
   he = enclosingObj->ho;

// Find the total width of the enclosed objects
	for(ObjectData* obj: widgets.getWidgets())
   {
      if(obj->selected_ && obj != enclosingObj)
      {
         objCnt++;
         totObjWidth  += obj->wo;
         totObjHeight += obj->ho;
         obj->flag = 0;
      }  
   }

// Work out the spacing between the objects   
   xSpacing = (we - totObjWidth)/(objCnt+1);
   ySpacing = (he - totObjHeight)/(objCnt+1);
   xpos = xSpacing + xe;
   ypos = ySpacing + ye;  
      
	if(dir == WindowLayout::HORIZ_DISTRIBUTE)
	{
	   for(short i = 1; i <= objCnt; i++)
	   {		
		// Find leftmost - unshifted object		
		   short minX = 1e4;
			for(ObjectData* obj: widgets.getWidgets())
			{
		      if(obj->selected_ && obj->flag == 0 && obj != enclosingObj)
		      {
		         if(obj->xo < minX) 
		         {
		            minX = obj->xo;
		            movObj = obj;
		         }
		      }  
		   } 
			if(movObj)
			{
				movObj->flag = 1;
				// Shift this object
				movObj->xo = xpos;
				xpos += movObj->wo + xSpacing;
	
				movObj->Place(movObj->xo,movObj->yo,movObj->wo,movObj->ho,true);
				GetClientRect(movObj->hwndParent,&r);
				MyInvalidateRect(movObj->hwndParent,&r,false);         
			}
		}
	}
	else
	{
	   for(short i = 1; i <= objCnt; i++)
	   {		
		// Find topmost - unshifted object		
		   short minY = 1e4;
			for(ObjectData* obj: widgets.getWidgets())
		   {
		      if(obj->selected_ && obj->flag == 0 && obj != enclosingObj)
		      {
		         if(obj->yo < minY) 
		         {
		            minY = obj->yo;
		            movObj = obj;
		         }
		      }  
		   }
			if(movObj)
			{
				movObj->flag = 1;
		   
				// Shift this object
				movObj->yo = ypos;
				ypos += movObj->ho + ySpacing;
	
				movObj->Place(movObj->xo,movObj->yo,movObj->wo,movObj->ho,true);
			   GetClientRect(movObj->hwndParent,&r);
		      MyInvalidateRect(movObj->hwndParent,&r,false);         
			}
		}
   }
// Restore designer as current GUI window
	WinData::SetGUIWin(designerWin);
}


/***************************************************************************************
   Distrubute selected window objects in one of two ways:
  
   1. So that there is an even space between each but the end objects don't move
   2. So that there is an even space between each object and the window edges.
**************************************************************************************/

void WinData::DistributeSelectedObjects(short mode)
{
   ObjectData *movObj = NULL;   
   RECT r;
   short leftX = 1e4,rightX = -1e4;
   short topY = 1e4,baseY = -1e4;
   short totObjWidth = 0;   // Total width of all objects in window
   short totObjHeight = 0;  // Total height of all objects in window
   short objCnt = 0;
   short xSpacing,ySpacing; // Spacing between distributed objects
   short xpos,ypos;         // Position of left or top edge of each window
   
// Don't bother if there are no objects
	if(widgets.empty())
      return;

// Find x or y limits for region bounded by selected objects **********   

	for(ObjectData* obj: widgets.getWidgets())
   {
      if(obj->selected_)
      {
         objCnt++;
         if(obj->xo < leftX) leftX = obj->xo;
         if(obj->yo < topY)  topY  = obj->yo;
         if(obj->xo + obj->wo > rightX) rightX = obj->xo + obj->wo;
         if(obj->yo + obj->ho > baseY)  baseY  = obj->yo + obj->ho;
         totObjWidth  += obj->wo;
         totObjHeight += obj->ho;
         obj->flag = 0;
      }  
   }  
   
   if(objCnt <= 1)
      return;

   short wwMod = ww-2*resizableWinBorderSize;

// Work out spacing required to distribute objects evenly **************   
   if(mode == WindowLayout::WINDOW_HORIZ_DISTRIBUTE || mode == WindowLayout::WINDOW_VERT_DISTRIBUTE)
   { 
      xSpacing = (wwMod - totObjWidth)/(objCnt+1);
      ySpacing = (wh - totObjHeight)/(objCnt+1);  // What is the correct number here?
      xpos = xSpacing + resizableWinBorderSize;
      ypos = ySpacing;          
   }
   else if(mode == WindowLayout::HORIZ_DISTRIBUTE || mode == WindowLayout::VERT_DISTRIBUTE)
   {   
      xSpacing = (rightX-leftX - totObjWidth)/(objCnt-1);
      ySpacing = (baseY-topY - totObjHeight)/(objCnt-1);
      xpos = leftX;
      ypos = topY;      
   }
	else	// Should never happen; this is to ensure xpos and ypos are initialized.
			// Arbitrarily making this the same as the HORIZ_DISTRIBUTE and VERT_DISTRIBUTE.
	{ 
      xSpacing = (rightX-leftX - totObjWidth)/(objCnt-1);
      ySpacing = (baseY-topY - totObjHeight)/(objCnt-1);
		xpos = leftX;
		ypos = topY;
	}
     
// Distribute objects evenly in the bounded region *********************

   switch(mode)
   {
 	   case(WindowLayout::WINDOW_HORIZ_DISTRIBUTE):     
	   case(WindowLayout::HORIZ_DISTRIBUTE):
	   {   
		   for(short i = 1; i <= objCnt; i++)
		   {			
			// Find leftmost - unshifted object			
			   short minX = 1e4;
			   for(ObjectData* obj: widgets.getWidgets())
			   {
			      if(obj->selected_ && obj->flag == 0)
			      {
			         if(obj->xo < minX) 
			         {
			            minX = obj->xo;
			            movObj = obj;
			         }
			      }  
			   } 
            if(!movObj) return;
			   movObj->flag = 1;	

	      // Shift this object	   
	         movObj->xo = xpos;
	         xpos += movObj->wo + xSpacing;
	
	         movObj->Place(movObj->xo,movObj->yo,movObj->wo,movObj->ho,true);
	         GetClientRect(movObj->hwndParent,&r);
	         MyInvalidateRect(movObj->hwndParent,&r,false);         
			}
			break;
	   }
 
 	   case(WindowLayout::WINDOW_VERT_DISTRIBUTE):  
	   case(WindowLayout::VERT_DISTRIBUTE):
	   {   
		   for(short i = 1; i <= objCnt; i++)
		   {
			// Find topmost - unshifted object			
			   short minY = 1e4;
				for(ObjectData* obj: widgets.getWidgets())
			   {
			      if(obj->selected_ && obj->flag == 0)
			      {
			         if(obj->yo < minY) 
			         {
			            minY = obj->yo;
			            movObj = obj;
			         }
			      }  
			   } 
            if(!movObj) return;
			   movObj->flag = 1;
			   
	      // Shift this object
	         movObj->yo = ypos;
	         ypos += movObj->ho + ySpacing;
	
	         movObj->Place(movObj->xo,movObj->yo,movObj->wo,movObj->ho,true);
	         GetClientRect(movObj->hwndParent,&r);
	         MyInvalidateRect(movObj->hwndParent,&r,false);         
			}
			break;
	   }
	}
}

/***************************************************************************************
   Distrubute selected window objects so that there is an even space 
   between each, equal to the space between leftmost or topmost two
**************************************************************************************/

void WinData::EvenlySpaceSelectedObjects(short mode)
{
   ObjectData *movObj = NULL;
   RECT r;
   short objCnt = 0;
   
// Don't bother if there are no objects
	if(widgets.empty())
      return;

// Search for left or top most control **********
   if(mode == WindowLayout::EQUAL_HORIZ_DISTRIBUTE)
   {
   // Search for left-most window and reset flag
		short xPos = 1e4;
		for(ObjectData* obj: widgets.getWidgets())
		{
		   if(obj->isSelected())
		   {
		      objCnt++;
		      if(obj->xo < xPos) 
		      {
		         xPos = obj->xo;
		         movObj = obj;
		      }
		   }  
         obj->flag = 0;		 
		} 
      if(!movObj) return;
		xPos = movObj->xo + movObj->wo;
		
	// Find space between left most object and it nearest neighbour
      short xSpacing = 1e4;
		for(ObjectData* obj: widgets.getWidgets())
		{
		   if(obj->selected_ && obj != movObj)
		   {
		      if(obj->xo - xPos < xSpacing) 
		      {
		         xSpacing = obj->xo - xPos;
		      }
		   }  
		}
      if(!movObj) return;
		xPos = movObj->xo;

   // Space all controls this distance apart horizontally
	   for(short i = 1; i <= objCnt; i++)
	   {
		// Find leftmost - unshifted object
		   short minX = 1e4;
			for(ObjectData* obj: widgets.getWidgets())
		   {
		      if(obj->selected_ && obj->flag == 0)
		      {
		         if(obj->xo < minX) 
		         {
		            minX = obj->xo;
		            movObj = obj;
		         }
		      }  
		   } 
         if(!movObj) return;
		   movObj->flag = 1;
		   
      // Shift this object
         movObj->xo = xPos;
         movObj->Place(movObj->xo,movObj->yo,movObj->wo,movObj->ho,true);
         GetClientRect(movObj->hwndParent,&r);
         MyInvalidateRect(movObj->hwndParent,&r,false);
         xPos += movObj->wo + xSpacing;                  
		}
   }
   else if(mode == WindowLayout::EQUAL_VERT_DISTRIBUTE)
   {
   // Search for top-most window and reset flag
		short yPos = 1e4;
		for(ObjectData* obj: widgets.getWidgets())
		{
		   if(obj->selected_)
		   {
		      objCnt++;
		      if(obj->yo < yPos) 
		      {
		         yPos = obj->yo;
		         movObj = obj;
		      }
		   }  
         obj->flag = 0;		   
		} 
      if(!movObj) return;
		yPos = movObj->yo + movObj->ho;
		
	// Find space between top most object and it nearest neighbour
      short ySpacing = 1e4;
		for(ObjectData* obj: widgets.getWidgets())
		{
		   if(obj->selected_ && obj != movObj)
		   {
		      if(obj->yo - yPos < ySpacing) 
		      {
		         ySpacing = obj->yo - yPos;
		      }
		   }
		}
      if(!movObj) return;
		yPos = movObj->yo;

   // Space all controls this distance apart horizontally
	   for(short i = 1; i <= objCnt; i++)
	   {		
		// Find leftmost - unshifted object
		   short minY = 1e4;
			for(ObjectData* obj: widgets.getWidgets())
		   {
		      if(obj->isSelected() && obj->flag == 0)
		      {
		         if(obj->yo < minY) 
		         {
		            minY = obj->yo;
		            movObj = obj;
		         }
		      }  
		   } 
         if(!movObj) return;
		   movObj->flag = 1;
		   
      // Shift this object
         movObj->yo = yPos;
         movObj->Place(movObj->xo,movObj->yo,movObj->wo,movObj->ho,true);
         GetClientRect(movObj->hwndParent,&r);
         MyInvalidateRect(movObj->hwndParent,&r,false); 
         yPos += movObj->ho + ySpacing;                 
		}
   }   
}

/***************************************************************************************
  Enables or disables all objects in a window                            
***************************************************************************************/

void WinData::MoveSelectedObjects(char dir, short step)
{
   char str[MAX_STR];
   
// See if any of the selected objects are tab controls
// If so select all connected controls
	for(ObjectData *objSel: widgets.getWidgets())
   {
      if(objSel->isSelected() && objSel->type == TABCTRL)
      {
         for(ObjectData* temp: widgets.getWidgets())
         {
            if(temp->type != TABCTRL && temp->tabParent == objSel)
            {
               temp->selected_ = true;
            }  
         }
      }  
   } 

   short cnt = CountSelectedObjects();

	for(ObjectData* obj: widgets.getWidgets())
   {
      if(obj->isSelected() && obj->type != STATUSBOX)
      {
         switch(dir)
         {
            case(VK_LEFT):
               obj->xo -= step; break;
            case(VK_RIGHT):
               obj->xo += step; break;               
            case(VK_UP):
               obj->yo -= step; break;
            case(VK_DOWN):
               obj->yo += step; break;
         } 
         if(cnt == 1)
         {
            sprintf(str,"x = %hd y = %hd",obj->xo,obj->yo);
            SetWindowText(hWnd,str);
            SetTimer(hWnd,1,2000,NULL);
         }
         obj->Place(obj->xo,obj->yo,obj->wo,obj->ho,true);
      }  
   }  
}

/***************************************************************************************
  Remove focus rectangle from any radiobutton or checkbox  controls          
***************************************************************************************/

void WinData::RemoveFocusRectangle()
{
	for(ObjectData *obj: widgets.getWidgets())
   {
      if((obj->type == CHECKBOX || obj->type == RADIO_BUTTON || obj->type == COLORBOX) && obj->visible)
      {
         HDC hdc = GetDC(obj->hwndParent);
         obj->DrawFocusRect(hdc,1);
         ReleaseDC(obj->hwndParent,hdc);
      }  
   }  
}

/***************************************************************************************
  Enables or disables all objects in a window                   
***************************************************************************************/

void WinData::MoveSelectedObjects(short dx, short dy)
{
   for(ObjectData *obj: widgets.getWidgets())
   {
      if(obj->selected_ && obj->type != STATUSBOX)
      {
         obj->xo += dx;
         obj->yo += dy;
         obj->Place(obj->xo,obj->yo,obj->wo,obj->ho,true);
      }  
   }  
}

/***************************************************************************************
   Give an object a new size                                   
***************************************************************************************/

void WinData::ResizeSelectedObjects(short wo, short ho)
{
   StaticTextInfo* info;
   bool MultiLineStaticText = false;

	for(ObjectData *obj: widgets.getWidgets())
   {
      if(obj->type == STATICTEXT)
      {
         info = (StaticTextInfo*)obj->data;
         MultiLineStaticText = info->multiLine;
      }

      if(obj->selected_ && obj->type != STATUSBOX && obj->type != CHECKBOX &&
                          obj->type != RADIO_BUTTON && !(obj->type == STATICTEXT && !MultiLineStaticText))
      {
			char tbDir = 'h';
         if(obj->type == SLIDER)
         {
            if(GetWindowLong(obj->hWnd,GWL_STYLE) & TBS_VERT)
               tbDir = 'v';
         }

         if(wo != -1)
         {
            if(!(obj->type == SLIDER && tbDir == 'v'))
               obj->wo = wo;
         }
         if(ho != -1)
         {
            if(!(obj->type == SLIDER && tbDir == 'h') && 
                 obj->type != TEXTBOX && obj->type != TEXTMENU)
               obj->ho = ho;
         }
         obj->wSzOffset = obj->wo;
         obj->hSzOffset = obj->ho;
         obj->Place(obj->xo,obj->yo,obj->wo,obj->ho,true);
      }  
   }  
}

/***************************************************************************************
   Count the number of selected objects in window                         
***************************************************************************************/

short WinData::CountObjects()
{
	return widgets.size();
}

// Make sure 
void WinData::ValidateObjRects()
{
	
   RECT r;

   for(ObjectData *obj: widgets.getWidgets())
   { 
      if(obj->type != GROUP_BOX && obj->type != TEXTEDITOR  &&
      /*   obj->type != PLOTWINDOW && obj->type != IMAGEWINDOW && */
         obj->type != CLIWINDOW && obj->type != DIVIDER &&
         /*obj->type != OPENGLWINDOW &&*/ obj->visible)
      {
         if(obj->type == RADIO_BUTTON)
         {
            RadioButtonInfo *info = (RadioButtonInfo*)obj->data;
		      for(int i = 0; i < info->nrBut; i++)
		      {
               GetWindowRect(info->hWnd[i],&r);
               ValidateRect(info->hWnd[i],&r);
            }
         }
         else if(obj->type == STATUSBOX)
          {
            SetRect(&r,obj->xo,obj->yo,obj->xo+obj->wo,obj->yo+obj->ho);
            ValidateRect(this->hWnd,&r);
         }
         else
         {
            SetRect(&r,obj->xo,obj->yo,obj->xo+obj->wo,obj->yo+obj->ho);
            ValidateRect(this->hWnd,&r);
         }
      }
   } 
}

void WinData::InvalidateObjRects()
{
   RECT r;
   for(ObjectData *obj: widgets.getWidgets())
   {
      if(obj->type != GROUP_BOX)
      {
         SetRect(&r,obj->xo,obj->yo,obj->xo+obj->wo,obj->yo+obj->ho);
         MyInvalidateRect(this->hWnd,&r,false);
      }
   } 
}

/***************************************************************************************
   Count the number of selected objects in window                         
***************************************************************************************/

short WinData::CountSelectedObjects()
{
	return widgets.countSelectedObjects();
}

/***************************************************************************************
    Resize an object using the arrow keys                          
***************************************************************************************/

void WinData::ResizeSelectedObjects(char dir, short step)
{
   long cnt;
   char str[50];
   StaticTextInfo* info;
   bool MultiLineStaticText = false;


   cnt = CountSelectedObjects();
	for(ObjectData* obj: widgets.getWidgets())
   {
      if(obj->type == STATICTEXT)
      {
         info = (StaticTextInfo*)obj->data;
         MultiLineStaticText = info->multiLine;
      }

   // Ignore these controls
      if(obj->selected_ && obj->type != STATUSBOX &&
                          !(obj->type == STATICTEXT && !MultiLineStaticText)  &&
                          obj->type != CHECKBOX     &&
                          obj->type != RADIO_BUTTON)
      {
			char tbDir = 'h';
         if(obj->type == SLIDER)
         {
            if(GetWindowLong(obj->hWnd,GWL_STYLE) & TBS_VERT)
               tbDir = 'v';
         }

      // Ignore other controls depending on direction or resize
	      switch(dir)
	      {
	         case(VK_LEFT):
	            if(obj->type == SLIDER && tbDir == 'v')
               {
	               continue;
               }
	            break;
	         case(VK_RIGHT):
               if(obj->type == SLIDER && tbDir == 'v')
               {
	               continue;
               }
	            break;               
	         case(VK_DOWN):
               if(obj->type == TEXTBOX || obj->type == TEXTMENU || (obj->type == SLIDER && tbDir == 'h'))
               {
	               continue;
               }
	            break;
	         case(VK_UP):
	            if(obj->type == TEXTBOX || obj->type == TEXTMENU || (obj->type == SLIDER && tbDir == 'h'))
               {
	               continue;
               }
	            break;
	      } 

      // Resize the others
	      switch(dir)
	      {
	         case(VK_LEFT):
	            if(obj->wo >= 10)
	               obj->wo -= step;
	            break;
	         case(VK_RIGHT):
	            obj->wo += step;
               break;               
	         case(VK_DOWN):
	            obj->ho += step; 
               break;
	         case(VK_UP):
	            if(obj->ho >= 10)
	               obj->ho -= step;
               break;
	      } 

         if(cnt == 1)
         {
            sprintf(str,"w = %hd h = %hd",obj->wo,obj->ho);
            SetWindowText(hWnd,str);
            SetTimer(hWnd,1,2000,NULL);
         }
	      obj->Place(obj->xo,obj->yo,obj->wo,obj->ho,true);
      }  
   }  
}

/***************************************************************************************
   Add all procedure in path\name to the current window                          
***************************************************************************************/

void WinData::AddProcedures(char *path, char* name)
{
   char fileName[MAX_PATH];
   long startLine = 0;
   long lineNr = 0;

// Don't cache if window loaded from text
   if(!strcmp(name,"current_text")) 
	   return;

   strncpy_s(fileName,MAX_PATH,name,_TRUNCATE);

// Read in the text 
   char *text = LoadTextFileFromFolder(path, fileName,".mac");
   if(!text) return;

   char *procedure;
   char procName[MAX_STR];
   long i = 0;
   long len = strlen(text);
   Variable* procVar;
   short type;

// Make sure we have an filename extension and are lower case
   AddExtension(fileName,".mac");
//   ToLowerCase(fileName);

// Search for all procedures in text
   while(FindProcedures(text, i, &procedure, procName, len, lineNr, startLine))
   {
      // If this procedure is already in the macro list remove it
      procVar = GetProcedure(path,name,procName);
      if(procVar) // Remove previous cached procedure
		{
		//	TextMessage("Removing 1 %s:%s\n",fileName,procName);
         procVar->Remove();
			delete procVar;
		}
      procVar = procList.Add(PROCEDURE,procName);
    	procVar->MakeAndSetProcedure(procedure,procName,fileName,path,startLine);
	  // TextMessage("Adding 1 %s:%s\n",fileName,procName);
      delete [] procedure;
   }
   delete [] text;
}

/***************************************************************************************
   Update the local window title and also update actual window title.                          
***************************************************************************************/

void WinData::SetTitle(char *newTitle)
{
   if(title) delete title;
   title = new char[strlen(newTitle)+1];
   strcpy(title,newTitle);
   SetWindowText(this->hWnd,newTitle);
}

/***************************************************************************************
   Remove a window in the list by name                          
***************************************************************************************/

void WinData::RemoveWin(char *winName)
{
   for(WinData *w = next; w != NULL; w = w->next)
   {
      if(!strcmp(winName,w->title))
      {
         delete w;
         return;
      }
   }
}

/***************************************************************************************
   Remove all memory allocated to a window and then remove
   it from the window list                         
***************************************************************************************/

void WinData::Destroy()
{
   WinData *w;
   
   w = this;

    //  printf("Entering WinData destroy\n");

   if(GetGUIWin() == this)
		WinData::SetGUIWin(NULL); 
   if(editGUIWindow == this)
      editGUIWindow = NULL; 

// destroyAllWidgets (called below) indirectly causes callback SyntaxEventsProc to fire.
// Unfortunately that callback assumes that all the widgets controlled by the window are 
// alive. But, because the destructor for each of those objects has already been called
// by the time the callback fires, we need to check that this window is still alive. 
// The real fix is to prevent the callback from firing in the first place...
	this->alive(false);
   //printf("Entering widgets destruction\n");

// Remove objects in object list
	widgets.destroyAllWidgets();
   //printf("All widgets destroyed\n");
// Make sure that ansVar doesn't point to deleted data
 //  if(this != &rootWin && ansVar->GetAlias() && ansVar->GetAlias()->GetScope() == WINDOW) 
 //     ansVar->MakeAndSetFloat(0);

// Remove window variables   
   varList.RemoveAll();
   //printf("All variables destroyed\n");
  
// Remove all window procedures
   procList.RemoveAll();
   //printf("Procedures destroyed\n");

// Delete undo array
	undoArray.destroyAllWidgets();
   //printf("Undo  destroyed\n");

// Delete accelerator tables
   if(hAccelTable)
       DestroyAcceleratorTable(hAccelTable);

   if(accelTable)
       DestroyAcceleratorTable(accelTable);
   //printf("Acc tables destroyed\n");

// Destroy menu
  //   HMENU winMenu = GetMenu(hWnd);
     if(menu) // was winMenu
         DestroyMenu(menu);

// Destroy menu list
     if(menuList)
        delete [] menuList;
   //printf("Menus destroyed\n");

// Destroy blank toolbar and status bar
     if(blankToolBar)
        DestroyWindow(blankToolBar);
     if(blankStatusBar)
        DestroyWindow(blankStatusBar);
   //printf("Toolbar destroyed\n");

// Remove window name
   if(w->title)
      delete[] w->title;
   if(w->name)
      delete[] w->name;

// Don't delete if isolated window or first in list
   if(w->last == NULL)
      return;
   //printf("Window name destroyed\n");

// Remove window from window list     
   if(w->next == (WinData*)0) // Last in list
   {
   	w->last->next = (WinData*)0;
   }
   else
   {
  		w->last->next = w->next;
   	w->next->last = w->last;
   }

// Delete the window info data
   WinInfo* wi = (WinInfo*)GetWindowLong(hWnd,GWL_USERDATA);
   if(wi)
   {
      delete wi;
      SetWindowLong(hWnd,GWL_USERDATA,0);
   }
   //printf("Window info destroyed\n");

// Delete the USB device notifcation info
	if(this->devIF)
	{
		UnregisterDeviceNotification(this->_hNotifyDevNode);
		free(this->devIF);
	}
   
// Remove MS window object ****
//   DestroyWindow(w->hWnd);
}


/***************************************************************************************
    Update object size & position based on new parent window width and height                          
***************************************************************************************/

void WinData::UpdateGUIObjectPositions(int ww, int wh)
{
   RECT r;
   int offy = 0;

   if(fixedObjects)
   {
   // Allow for status boxes
      if(statusbox)
      {
         GetClientRect(statusbox->hWnd,&r);
         wh -= r.bottom-1;
      }

   // Allow for toolbars
      if(toolbar)
      {
         GetClientRect(toolbar->hWnd,&r);
         offy = r.bottom+3;
         wh -= r.bottom+3;
      }
   }

	for(ObjectData* obj: widgets.getWidgets())
   {
      obj->xo = nint(obj->xSzScale*ww + obj->xSzOffset);
      obj->yo = nint(obj->ySzScale*wh + obj->ySzOffset + offy);
      obj->wo = nint(obj->wSzScale*ww + obj->wSzOffset);
      obj->ho = nint(obj->hSzScale*wh + obj->hSzOffset);

   // Make sure static text is correctly positioned based
   // on its justification mode
      if(obj->type == STATICTEXT)
      {
         POINT p = {0,0};
         ClientToScreen(obj->hWnd,&p);
         ScreenToClient(obj->hwndParent,&p);
         RECT ro;
         GetWindowRect(obj->hWnd,&ro);
         long objwidth = ro.right-ro.left;
         long style = GetWindowLong(obj->hWnd,GWL_STYLE);
         style = style & 0x0003;
			long x = obj->xo;
         if(style == SS_LEFT)
	         x = obj->xo; // Return left coord of text
         else if(style == SS_RIGHT)
	         x = obj->xo-objwidth; // Return right coord of text
         else if(style == SS_CENTER)
	         x = obj->xo-objwidth/2; // Return centre coord of text

          float back = obj->xSzOffset;
	       obj->Place(x,obj->yo,obj->wo,obj->ho,true);
          obj->xSzOffset = back;
       }
      else if(obj->type == TEXTEDITOR)
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
       else
         obj->Place(obj->xo,obj->yo,obj->wo,obj->ho,true);
 
   // Resize status box parts
      if(obj->type == STATUSBOX) 
      {
         float scale,off;
         bool region;
         if(obj->data)
         {
            StatusBoxInfo* info = (StatusBoxInfo*)obj->data;
            info->colorize = true;
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
            delete [] statusParts;
         }
      }
      else if(obj->type == HTMLBOX) // Resize html viewer
      {
         ResizeHTMLViewer(obj->hWnd,obj->wo,obj->ho);
      }
   }

}

/*****************************************************************************
*                 Process a window class procedure
*****************************************************************************/

short WinData::ProcessClassProc(Interface *itfc, char *cmd, char *parameter)
{
   CText arg;
   short r,nrArgs;
	CArg carg;

   if(!parameter)
      nrArgs = 0;
   else
      nrArgs = carg.Count(parameter);

   if(!strcmp(cmd,"set"))  // Set a window parameter e.g. win->set("width",200)
   {
      CText param,value;
      arg.Format("%d,%s",this->nr,parameter);
      r = SetWindowParameter(itfc, arg.Str());
   }
   else if(!strcmp(cmd,"get")) // Get a window parameter e.g. win->get("width")
   {
      CText param,value;
      if(ArgScan(itfc,parameter,1,"","e","t",&param)< 0)
      {
         ErrorMessage("invalid parameter");
         return(ERR);
      }
      arg.Format("%d,\"%s\"",this->nr,param.Str());
      r = GetWindowParameter(itfc, arg.Str());
   }
   else if(!strcmp(cmd,"obj")) // Get a window object e.g. win->obj(2)
   {
      short nr;
      if(ArgScan(itfc,parameter,1,"","e","d",&nr) < 0)
         return(ERR);
      ObjectData *obj = this->FindObjectByNr(nr);
      itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
      itfc->nrRetValues = 1;
      r = OK;
   }
   else // Implicit command syntax
   {
      if(nrArgs == 0)  // Get an window parmeter e.g. win->width()
      {
         arg.Format("%d,\"%s\"",this->nr,cmd);
         r = GetWindowParameter(itfc, arg.Str());
      }
      else if(nrArgs == 1) // Set an window parameter win->width(20)
      {
         itfc->nrRetValues = 0;
         arg.Format("%d,\"%s\",%s",this->nr,cmd,parameter);
         r = SetWindowParameter(itfc, arg.Str());
      }
      else
      {
         ErrorMessage("Invalid command - zero or one argument expected");
         return(ERR);
      }
   }

   return(r);
}

/************************************************************************
Generate a string representing the state of this object's parameter-
accessible attributes.
************************************************************************/
string WinData::FormatState()
{
	char str[128] = {(char)'/0'};
	RECT r;
	GetWindowRect(hWnd,&r);
	StringPairs state;

	state.add("bkcolor", Plot::GetColorStr(bkgColor));
	state.add("bkgmenu", (this->bkgMenu) ? stringifyInt(this->bkgMenuNr) : "default");
   state.add("bordersize", (this->resizeable) ? (stringifyFloat(GetSystemMetrics(SM_CXSIZEFRAME)).c_str()) : (stringifyFloat(GetSystemMetrics(SM_CXFIXEDFRAME)).c_str()));
	state.add("constrained", toTrueFalse(constrained));
   state.add("ctrllist","List of control numbers");
	sprintf(str,"(%hd,%hd,%hd,%hd)", r.left,r.top,r.right-r.left,r.bottom-r.top-titleBarHeight);
	state.add("dimensions (x,y,w,h)", str);
	state.add("dragndropproc", dragNDropProc.Str());
	state.add("exitprocedure",exitProcName);
	state.add("focus",toTrueFalse(GetFocus() == hWnd));
	state.add("gridspacing", stringifyFloat(gridSpacing).c_str());
	sprintf(str,"%hd", r.bottom-r.top-titleBarHeight);
   state.add("height",str);
	state.add("keepinfront",toTrueFalse(keepInFront));
	state.add("macroname",macroName);
	state.add("macropath",macroPath);
	state.add("mainwindow",toTrueFalse(hWnd == prospaWin));
	state.add("menubar","Array of menu object numbers");
   state.add("mergetitle", toTrueFalse(mergeTitle));
	state.add("name", name);
	state.add("nrctrls", stringifyInt(CountObjects()).c_str());
	state.add("permanent", toTrueFalse(permanent));
	sprintf(str,"(%hd,%hd)", r.left,r.top);
	state.add("position (x,y)", str);
	state.add("resizable", toTrueFalse(resizeable));
	state.add("showgrid", toTrueFalse(showGrid));
	state.add("showmenu",toTrueFalse(showMenu));
	sprintf(str,"(%hd,%hd,%hd,%hd)", sizeLimits.minWidth, sizeLimits.maxWidth, sizeLimits.minHeight, sizeLimits.maxHeight);
	state.add("sizelimits (x,y,w,h)", str);
	state.add("snaptogrid", toTrueFalse(snapToGrid));
	state.add("statusbox",stringifyInt((defaultStatusbox) ? defaultStatusbox->nr() : -1).c_str());
	state.add("title",title);
	state.add("titlebarheight", stringifyFloat(titleBarHeight).c_str());
	state.add("titleupdate", toTrueFalse(titleUpdate));
	state.add("toolbar",stringifyInt((defaultToolbar) ? defaultToolbar->nr() : -1).c_str());
	state.add("type", (type == NORMAL_WIN) ? "normal": "dialog");
	state.add("visible", toTrueFalse(IsWindowVisible(hWnd)));
	sprintf(str,"%hd", r.right-r.left);
   state.add("width",str);
	state.add("winnr",stringifyInt(nr).c_str());
   state.add("winvar","Print window variables");
	sprintf(str,"%hd", r.left);
   state.add("x",str);
	sprintf(str,"%hd", r.top);
   state.add("y",str);

	return FormatStates(state);
}

void WinData::activate()
{
	// If window is edited remove "*" from window title
	if(!activated)
	{
		long length = strlen(title);
		if(title[length-1] == '*')
			title[length-1] = '\0';
		SetWindowText(hWnd,title);
	}

	activated = true;
	EnableObjects(true);
	if(!resizeable)
		EnableResizing(false);
	displayObjCtrlNrs = false;
	displayObjTabNrs = false;
	modifyingCtrlNrs = false;
	modifyingTabNrs = false;
	MyInvalidateRect(hWnd,NULL,false); 
	editGUIWindow = NULL;
	MyUpdateWindow(hWnd);
}

void WinData::makeEditable()
{
	char *name;

	// If the window is running add "*" to filename
	if(activated)
	{
		long length = strlen(title)+1;
		name = new char[length + 10];
		strcpy(name,title);
		strcat(name,"*");
		SetWindowText(hWnd,name);
		delete []title;
		title = new char[strlen(name)+1];
		strcpy(title,name);
		delete [] name;
	}

	activated = false;
	DeselectObjects();   
	EnableObjects(false); 
	editGUIWindow = this;

	MyInvalidateRect(hWnd,NULL,false); 
	MyUpdateWindow(hWnd);

}

short WinData::updateControlVisibilityFromTabs()
{
	return widgets.updateControlVisibilityFromTabs();
}


void WinData::restoreFromMaximised()
{
	if(isMainWindow || !constrained)
		return;

	RECT pr;

	// Get the actual size of the prospa window without menu bar and borders
	GetWindowRect(prospaWin,&pr);
	long width = pr.right - pr.left;
	long height = pr.bottom - pr.top;
	long xoff = resizableWinBorderSize + pr.left;
	long yoff = titleBarNMenuHeight + pr.top;
	long ww = width - 2*resizableWinBorderSize;
	long wh = height - (resizableWinBorderSize + titleBarNMenuHeight);

	// This updates the normal (unzoomed) window position
	if(!IsIconic(prospaWin) && IsZoomed(hWnd))
	{
		WINDOWPLACEMENT wpl;
		wpl.length = sizeof(WINDOWPLACEMENT);
		long x = nint(ww*xSzScale + xSzOffset + xoff);
		long y = nint(wh*ySzScale + ySzOffset + yoff);
		long w = nint(ww*wSzScale + wSzOffset);
		long h = nint(wh*hSzScale + hSzOffset);

		GetWindowPlacement(hWnd,&wpl);

		wpl.rcNormalPosition.left = x;
		wpl.rcNormalPosition.top  = y;
		wpl.rcNormalPosition.right = x+w-1;
		wpl.rcNormalPosition.bottom = y+h-1;
		wpl.showCmd = SW_SHOWNORMAL; // Restore to normal size
		wpl.flags = 0;
		wpl.ptMaxPosition.x = -10000; // Make sure flash is not visible

		SetWindowPlacement(hWnd,&wpl); 
	}
	
}

void WinData::SetGUIWin(WinData* newGUI)
{
	ThreadGlobals *data = (ThreadGlobals*)TlsGetValue(dwTlsIndex);
	data->curGUIWin = newGUI;
	if (!TlsSetValue(dwTlsIndex, (void*)data)) 
		return; 
}

short WinData::setMenuItemCheck(const char* const menuName, const char* const menuKey, bool check)
{
	ObjectData *obj = FindObjectByValueID(menuName);
	if (!obj)
		return ERR;
	return obj->setMenuItemCheck(menuKey,check);	
}

short WinData::setMenuItemEnable(const char* const menuName, const char* const menuKey, bool enable)
{
	ObjectData *obj = FindObjectByValueID(menuName);

	if (!obj)
		return ERR;
	return obj->setMenuItemEnable(menuKey,enable);	
}

short WinData::setToolBarItemCheck(const char* const toolbarName, const char* const butKey, bool check)
{
	ObjectData *obj = FindObjectByValueID(toolbarName);

	if(!obj || obj->type != TOOLBAR)
		return(ERR);

	ToolBarInfo *info = (ToolBarInfo*)obj->data;

	for(int i = 0; i < info->nrItems; i++)
	{
		if(info->item[i].key == butKey)
		{
			SendMessage(obj->hWnd,TB_CHECKBUTTON,(WPARAM)i,(LPARAM) MAKELONG(check, 0));
			return(OK);
		}
	}
	return(ERR);
}

HRGN WinData::CreateBgrdRegion()
{
	RECT r;
	HRGN hRgnRect;
	HRGN hRgn = CreateRectRgn(0,0,1,1);

	for(ObjectData *obj: widgets.getWidgets())
	{ 
		if(obj->visible & obj->hwndParent == this->hWnd & !obj->panelParent)
		{
			//if(obj->type == CLIWINDOW)
			//{
			//	SetRect(&r,obj->xo+2,obj->yo+2,obj->xo+obj->wo-2,obj->yo+obj->ho-2);
			//	hRgnRect = CreateRectRgnIndirect(&r);
			//	CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
			//	DeleteObject(hRgnRect);
			//}
			//if(obj->type == TEXTEDITOR)
			//{
			//	EditParent *ep;
			//	EditRegion *er;
			//	short x,y,w,h;

			//	ep = (EditParent*)obj->data;

			//	for(short i = 0; i < ep->rows*ep->cols; i++) 
			//	{	
			//		er = ep->editData[i];
			//		x = nint(er->x*obj->wo + obj->xo);
			//		y = nint(er->y*obj->ho + obj->yo);
			//		w = nint(er->w*obj->wo);
			//		h = nint(er->h*obj->ho);	               
			//		SetRect(&r,x+2,y+2,x+w-2,y+h-2);
			//		hRgnRect = CreateRectRgnIndirect(&r);
			//		CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
			//		DeleteObject(hRgnRect); 
			//	}
			//}
			//if(obj->type == TEXTBOX)
			//{
			//	SetRect(&r,obj->xo-1,obj->yo,obj->xo+obj->wo+1,obj->yo+obj->ho);
			//	hRgnRect = CreateRectRgnIndirect(&r);
			//	CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
			//	DeleteObject(hRgnRect);
			//}
			if(obj->type == TOOLBAR)
			{
				SetRect(&r,obj->xo,obj->yo,obj->xo+obj->wo,obj->yo+obj->ho);
				hRgnRect = CreateRectRgnIndirect(&r);
				CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
				DeleteObject(hRgnRect);
			}
			else if(obj->type == RADIO_BUTTON)
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
		   else if(obj->type == PANEL)
			{
            PanelInfo* pInfo = (PanelInfo*)obj->data;
				SetRect(&r,pInfo->x,pInfo->y,pInfo->x+pInfo->w,pInfo->y+pInfo->h);
				hRgnRect = CreateRectRgnIndirect(&r);
				CombineRgn(hRgn,hRgn,hRgnRect,RGN_OR);
				DeleteObject(hRgnRect);
				SetRect(&r,obj->xo,obj->yo,obj->xo+obj->wo,obj->yo+obj->ho);
				hRgnRect = CreateRectRgnIndirect(&r);
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

	// Calculate the region inside the window rectangle which doesn't
	// include the objects 
	GetClientRect(hWnd,&r);
	hRgnRect = CreateRectRgnIndirect(&r);
	CombineRgn(hRgn,hRgnRect,hRgn,RGN_DIFF);
	DeleteObject(hRgnRect);

	return(hRgn);
	
}

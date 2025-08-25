
#pragma warning (disable: 4312) // Ignore Get/SetWindowLong warnings
#pragma warning (disable: 4311) // Ignore pointer truncation warnings

/*******************************************************
  Routines to handle the placement of internal popups
  and dialogs on the screen and also to handle their
  visibility when changing application focus.
*******************************************************/

#include "dialog_utilities.h"
#include "stdafx.h"
#include "defineWindows.h"
#include "utilities.h"
#include "memoryLeak.h"

#define LIST_SIZE 50
struct
{
   HWND key;
   HWND win;
   HWND parent;
   bool wasEnabled;
}
dialog_list[LIST_SIZE];

/*******************************************************
  Make sure all the entries in the list are NULL
*******************************************************/

void InitialiseDialogList()
{
   for(int i = 0; i < LIST_SIZE; i++)
   { 
      dialog_list[i].key = NULL;
      dialog_list[i].win = NULL;
      dialog_list[i].parent = NULL;
      dialog_list[i].wasEnabled = false;
   }
}

/*******************************************************
  Add a dialog window to the list
*******************************************************/

void AddToDiagList(HWND win)
{
   for(int i = 0; i < LIST_SIZE; i++)
   {
      if(dialog_list[i].win == NULL)
      {
         dialog_list[i].win = win;
         dialog_list[i].key = win;
     //    TextMessage("Added %d\n",i);
         break;
      }
   }

}

HWND GetOrigWindow(HWND key)
{
   for(int i = 0; i < LIST_SIZE; i++)
   {
      if(dialog_list[i].key == key)
      {
         return(dialog_list[i].win);
      }
   }
   return(NULL);
}

void SaveDialogParent(HWND hWnd)
{
   for(int i = 0; i < LIST_SIZE; i++)
   {
      if(dialog_list[i].win == hWnd)
      {
         dialog_list[i].parent = (HWND)GetWindowLong(hWnd, GWL_HWNDPARENT);
      }
   }
}

bool RestoreDialogParent(HWND hWnd)
{
   for(int i = 0; i < LIST_SIZE; i++)
   {
      if(dialog_list[i].win == hWnd)
      {
         SetWindowLong(hWnd, GWL_HWNDPARENT,( long)dialog_list[i].parent);
         return(true);
      }
   }
   return(false);
}

// Enable or disable dialogs
void EnableDialogs(bool enable)
{
   for(int i = 0; i < LIST_SIZE; i++)
   {
      if(dialog_list[i].win)
      {
         if(enable) // Enable the window if enabled previously
         {
            if(dialog_list[i].wasEnabled)
               EnableWindow(dialog_list[i].win,true);
         }
         else if(!enable) // Disable and note current status
         {
            if(IsWindowEnabled(dialog_list[i].win))
            {
               dialog_list[i].wasEnabled = true;
               EnableWindow(dialog_list[i].win,false);
            }
            else
               dialog_list[i].wasEnabled = false;
         }
      }
   }
}


/*******************************************************
  Add a dialog window to the list but this time save
  a key window as well for accessing it later.
*******************************************************/

void AddToDiagList(HWND key, HWND win)
{
   for(int i = 0; i < LIST_SIZE; i++)
   {
      if(dialog_list[i].win == NULL)
      {
         dialog_list[i].key = key;
         dialog_list[i].win = win;
      //   TextMessage("Added %d\n",i);
         break;
      }
   }

}

/*******************************************************
  Remove a dialog window from the list - use a key
  window to find it if necessary.
*******************************************************/

void RemoveFromDiagList(HWND win, bool useKey)
{
   if(useKey)
   {
      for(int i = 0; i < LIST_SIZE; i++)
      {
         if(dialog_list[i].key == win)
         {
            dialog_list[i].key = NULL;
            dialog_list[i].win = NULL;
      //      TextMessage("Removed %d\n",i);
            break;
         }
      }
   }
   else
   {
      for(int i = 0; i < LIST_SIZE; i++)
      {
         if(dialog_list[i].win == win)
         {
            dialog_list[i].key = NULL;
            dialog_list[i].win = NULL;
     //       TextMessage("Removed %d\n",i);
            break;
         }
      }
   }
}

/*******************************************************
 Hide all the dialog windows when switching application
*******************************************************/

void HideDiagWindows()
{
   for(int i = 0; i < LIST_SIZE; i++)
   {
      if(dialog_list[i].win)
      {
 	      ShowWindow(dialog_list[i].win,SW_HIDE);
      }
   }
}

/*******************************************************
 Show all the dialog windows when switching back to Prospa
*******************************************************/

void ShowDiagWindows()
{
   for(int i = 0; i < LIST_SIZE; i++)
   {
      if(dialog_list[i].win)
      {
         ShowWindow(dialog_list[i].win,SW_SHOW);
      }
   }
}

void ChangeDialogParent(HWND newParent)
{
   for(int i = 0; i < LIST_SIZE; i++)
   {
      if(dialog_list[i].win && dialog_list[i].win == dialog_list[i].key)
      {
 	      SetWindowLong(dialog_list[i].win, GWL_HWNDPARENT, (LONG)newParent);
         ShowWindow(dialog_list[i].win,SW_SHOW);
      }
   }
}

/*******************************************************
 When a dialog exits remove the window from the list
*******************************************************/

BOOL MyEndDialog(HWND hWnd,int value)
{
   RestoreDialogParent(hWnd);
   RemoveFromDiagList(hWnd,false);
   return(EndDialog(hWnd,value));
}

/*******************************************************
 Place a dialog in the centre of the window and make
 it topmost. Also store the window handle in list.
*******************************************************/

void PlaceDialogOnTopInCentre(HWND hWnd)
{
   RECT pr,r;
   GetWindowRect(prospaWin,&pr);
   short width = pr.right - pr.left;
   short height = pr.bottom - pr.top;
   GetWindowRect(hWnd,&r);
   short w = r.right - r.left;
   short h = r.bottom - r.top;
   short xoff = (width - w)/2 + pr.left;
   short yoff = (height - h)/2 + pr.top;
   CorrectWindowPositionIfInvisible(xoff, yoff, w, h);
   MoveWindow(hWnd,xoff,yoff,w,h,true);
   AddToDiagList(hWnd);
}

/*******************************************************
 Place a dialog in the centre of the window and make
 it topmost. Also store the window handle in list.
*******************************************************/

void PlaceDialogAtPosition(HWND hWnd, short xoff, short yoff)
{
   RECT r;
   HDC hdc = GetDC(hWnd);
   GetWindowRect(hWnd,&r);
   short w = r.right-r.left;
   short h = r.bottom-r.top;
   CorrectWindowPositionIfInvisible(xoff, yoff, w, h);
   MoveWindow(hWnd,xoff,yoff,w,h,true);
   ReleaseDC(hWnd,hdc); 
   AddToDiagList(hWnd);
}


/*******************************************************
 Place a modified dialog in the centre of the window
  and make it topmost. Also store the window handle and
  key in the list.
*******************************************************/

void PlaceDialogOnTopInCentre(HWND hWnd, HWND subWin)
{
   RECT pr,r;
   GetWindowRect(prospaWin,&pr);
   short width = pr.right - pr.left;
   short height = pr.bottom - pr.top;
   GetWindowRect(subWin,&r);
   short w = r.right - r.left;
   short h = r.bottom - r.top;
   short xoff = (width - w)/2 + pr.left;
   short yoff = (height - h)/2 + pr.top;
   CorrectWindowPositionIfInvisible(xoff, yoff, w, h);
   MoveWindow(subWin,xoff,yoff,w,h,false);
   AddToDiagList(hWnd,subWin);
}

/*******************************************************
 When a pop-up exits remove the window from the list
*******************************************************/

void DialogTidyUp(HWND hWnd, bool subWin)
{
   if(!subWin)
      RestoreDialogParent(hWnd);
   RemoveFromDiagList(hWnd,subWin);
}


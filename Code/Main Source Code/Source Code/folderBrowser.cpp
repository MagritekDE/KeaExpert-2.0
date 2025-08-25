#include "stdafx.h"
#include "folderBrowser.h"
#include <math.h>
#include <Shobjidl.h>
#include <Shlobj.h>
#include <shlguid.h>
#include <shellapi.h>
#include "cArg.h"
#include "defineWindows.h"
#include "evaluate.h"
#include "files.h"
#include "globals.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include "interface.h"
#include "main.h"
#include "message.h"
#include "mymath.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "winEnableClass.h"
#include "utilities.h"
#include "memoryLeak.h"

#define RELATIVE_PATH 10 
#define ABSOLUTE_PATH 11 
#define MAXTEXTLEN 255

// Folder browser - modified to have short cut buttons on the left side.
// Has a problem that it displays twice when jumping to a different folder.

static LRESULT CALLBACK MyProc(HWND hwnd,UINT uMsg,WPARAM wParam, LPARAM lParam);
HTREEITEM GetItemByName(HWND hWnd, HTREEITEM hItem, LPCTSTR szItemName);
ITEMIDLIST *PathToPidl(char *path);
INT CALLBACK BrowseCallbackProc(HWND hwnd, 
                                UINT uMsg,
                                LPARAM lParam, 
                                LPARAM lpData); 

void FreePIDL(ITEMIDLIST *pidl);
static TCHAR curDir[MAX_PATH];

static WNDPROC OldProc;
static HWND hwndTree;
static HWND hwndpar;
static bool firstTime = true;
static HWND hwndBut[8];
static char labels[5][10] = {"Desktop","Computer","Prospa","Macros","Data"};
static HWND pathModeRel;
static HWND pathModeAbs;
static CText allowRelativePath = "false";
static CText pathType = "relative";
static bool withButtons = true;

int SelectFolder(Interface *itfc, char args[])
{
   short nrArgs;
   CText title = "Select folder";
   CText top = "";
   CText current = "";
   WinData *win;

   allowRelativePath = "false";
   pathType = "relative";

   char result[MAX_PATH];
   CWinEnable winEnable;

   if((nrArgs = ArgScan(itfc,args,0,"cur_folder,title,top_folder,allow_relative_path","eeee","tttt",&current,&title,&top,&allowRelativePath)) < 0)
      return(nrArgs); 

   if(strlen(current.Str()) > 0)
      strncpy_s(curDir,MAX_PATH,current.Str(),_TRUNCATE);
   else
     curDir[0] = '\0';

   win = GetGUIWin(); // Record current window

   TCHAR dname[MAX_PATH];
   IMalloc *imalloc; SHGetMalloc(&imalloc);
   BROWSEINFO bi; 
   ZeroMemory(&bi,sizeof(bi));
   
   if(top != "")
      bi.pidlRoot = PathToPidl(top.Str());
   bi.hwndOwner = prospaWin;
   bi.pszDisplayName = dname;
   bi.lpszTitle = title.Str();
   bi.lpfn = BrowseCallbackProc;
   withButtons = true;
 
   bi.ulFlags = BIF_USENEWUI;

// Disable all other windows ******************
   winEnable.Disable(NULL);
   ITEMIDLIST *pidl = SHBrowseForFolder(&bi);
// Enable all other windows *******************
   if(dialogDisplayed)
      EnableWindow(win->hWnd,true); // Just enable the dialog 
   else
      winEnable.Enable(NULL); // Enable all windows

   if(SHGetPathFromIDList(pidl,result))
      itfc->retVar[1].MakeAndSetString(result);
   else
      itfc->retVar[1].MakeAndSetString("cancel");

   itfc->retVar[2].MakeAndSetString(pathType.Str());

   itfc->nrRetValues = 2;

   imalloc->Free(pidl);
   imalloc->Release();
   FreePIDL((ITEMIDLIST*)bi.pidlRoot);

	WinData::SetGUIWin(win); // Restore current window

   return(OK);
}


int SelectFolderNoButtons(Interface *itfc, char args[])
{
   short nrArgs;
   CText title = "Select folder";
   CText top = "";
   CText current = "";
   WinData *win;

   allowRelativePath = "false";
   pathType = "relative";

   char result[MAX_PATH];
   CWinEnable winEnable;

   if((nrArgs = ArgScan(itfc,args,0,"cur_folder,title,top_folder,allow_relative_path","eeee","tttt",&current,&title,&top,&allowRelativePath)) < 0)
      return(nrArgs); 

   if(strlen(current.Str()) > 0)
      strncpy_s(curDir,MAX_PATH,current.Str(),_TRUNCATE);
   else
     curDir[0] = '\0';

   win = GetGUIWin(); // Record current window

   TCHAR dname[MAX_PATH];
   IMalloc *imalloc; SHGetMalloc(&imalloc);
   BROWSEINFO bi; 
   ZeroMemory(&bi,sizeof(bi));
   
   if(top != "")
      bi.pidlRoot = PathToPidl(top.Str());
   bi.hwndOwner = prospaWin;
   bi.pszDisplayName = dname;
   bi.lpszTitle = title.Str();
   bi.lpfn = BrowseCallbackProc;
   withButtons = false;

 
   bi.ulFlags = BIF_USENEWUI;

// Disable all other windows ******************
   winEnable.Disable(NULL);
   ITEMIDLIST *pidl = SHBrowseForFolder(&bi);
// Enable all other windows *******************
   winEnable.Enable(NULL);

   if(SHGetPathFromIDList(pidl,result))
      itfc->retVar[1].MakeAndSetString(result);
   else
      itfc->retVar[1].MakeAndSetString("cancel");

   itfc->retVar[2].MakeAndSetString(pathType.Str());

   itfc->nrRetValues = 2;

   imalloc->Free(pidl);
   imalloc->Release();
   FreePIDL((ITEMIDLIST*)bi.pidlRoot);

	WinData::SetGUIWin(win); // Restore current window

   return(OK);
}

ITEMIDLIST *PathToPidl(char *path) 
{
   LPITEMIDLIST  pidl = 0;
   LPSHELLFOLDER pDesktopFolder=0;
   OLECHAR       olePath[MAX_PATH];
   ULONG         chEaten;
   ULONG         dwAttributes;

   //
   // Get a pointer to the Desktop's IShellFolder 
   // interface.
   //
   if (SUCCEEDED(SHGetDesktopFolder(&pDesktopFolder)))
   {
       //
       // IShellFolder::ParseDisplayName requires 
       // the file name be in Unicode.
       //
       MultiByteToWideChar(
                   CP_ACP, 
                   MB_PRECOMPOSED, 
                   path, 
                   -1,
                   olePath, 
                   MAX_PATH);

       //
       // Convert the path to an ITEMIDLIST.
       //
		 HRESULT hr = pDesktopFolder->ParseDisplayName(
                NULL, 
                NULL, 
                olePath, 
                &chEaten, 
                &pidl, 
                &dwAttributes);

       if (FAILED(hr))
       {
           // Handle error.
           goto ert;
       }
   }
ert:
    //release the desktop folder object
    if (pDesktopFolder) pDesktopFolder->Release();

    //return the result
    return pidl;
}


//
//  Free a pidl
//
void FreePIDL(ITEMIDLIST *pidl) {
    LPMALLOC m_imalloc;
    SHGetMalloc(&m_imalloc);
    m_imalloc->Free(pidl);
}

INT CALLBACK BrowseCallbackProc(HWND hwnd, 
                                UINT uMsg,
                                LPARAM lParam, 
                                LPARAM lpData) 
{
   switch(uMsg) 
   {
      case BFFM_INITIALIZED: 
	   {
			RECT rect;
			GetWindowRect(hwnd,&rect);

         firstTime = true;
         
			POINT pt;
			pt.x = rect.right;
			pt.y = rect.top;			
			ScreenToClient(hwnd,&pt);
         // These coordinates set the folder-button dimensions
         int x = 10, y = 58;
         int w = 80, h = 32;
         int dely = 44; // Y spacing
         int k;

         if(withButtons)
         {
            for(k = 0; k < 5; k++)
            {
               hwndBut[k] = CreateWindow("button", labels[k], WS_CHILD|BS_PUSHBUTTON|WS_VISIBLE|WS_TABSTOP,
	      			                      x, y, w,h, 
	      			                      hwnd, (HMENU)(WM_USER+k),
	      			                      prospaInstance, NULL);
              SendMessage(hwndBut[k],WM_SETFONT,(WPARAM)controlFont,MAKELPARAM(false, 0));
               y = y + dely;
            }
         }

         if(allowRelativePath == "true")
         {

             y = 10; // Set the vertical position of the path radio buttons
    
             pathModeRel = CreateWindow("button", "", WS_CHILD | BS_RADIOBUTTON | WS_VISIBLE,
	      			                                  210, y, 14, 14, 
	      			                                  hwnd, (HMENU)(WM_USER+RELATIVE_PATH),
	      			                                  prospaInstance, NULL);
             SendMessage(pathModeRel,BM_SETCHECK,1,0);

             pathModeAbs = CreateWindow("button", "", WS_CHILD | BS_RADIOBUTTON | WS_VISIBLE,
	      			                                  210, y+20, 14, 14, 
	      			                                  hwnd, (HMENU)(WM_USER+ABSOLUTE_PATH),
	      			                                  prospaInstance, NULL);
             SendMessage(pathModeAbs,BM_SETCHECK,0,0);
         
             HWND txt1 = CreateWindow("static", "Relative path", WS_CHILD | WS_VISIBLE,
	      			                                  225, y, 100,16, 
	      			                                  hwnd, (HMENU)ID_STATIC,
	      			                                  prospaInstance, NULL);
             SendMessage(txt1,WM_SETFONT,(WPARAM)controlFont,MAKELPARAM(false, 0));
        
             HWND txt2 = CreateWindow("static", "Absolute path", WS_CHILD | WS_VISIBLE,
	      			                                  225, y+20, 100,16, 
	      			                                  hwnd, (HMENU)ID_STATIC,
	      			                                  prospaInstance, NULL);
            SendMessage(txt2,WM_SETFONT,(WPARAM)controlFont,MAKELPARAM(false, 0)); 
         }


			HWND par = ::FindWindowEx(hwnd,NULL,"SHBrowseForFolder ShellNameSpace Control",NULL);
			hwndTree = ::FindWindowEx(par,NULL,"SysTreeView32",NULL);

			OldProc = (WNDPROC) SetWindowLong(hwnd,DWL_DLGPROC, (LONG) MyProc); 

			break;	
		}
   }
   return 0;
}


LRESULT CALLBACK MyProc(HWND hwnd,UINT uMsg,WPARAM wParam, LPARAM lParam)
{
   char str[MAX_PATH];
   Interface itfc;

	switch(uMsg)
   {
      case(WM_PAINT):
      {
         if(firstTime) // Without this the scroll bar is not draw initially
         {
            RECT r;
            GetWindowRect(hwnd,&r);
            SetWindowPos(hwnd,HWND_BOTTOM,r.left,r.top,r.right-r.left,r.bottom-r.top-1,SWP_NOZORDER |	SWP_SHOWWINDOW);
            SetWindowPos(hwnd,HWND_BOTTOM,r.left,r.top,r.right-r.left,r.bottom-r.top,SWP_NOZORDER | SWP_SHOWWINDOW);
            firstTime = false;
            SendMessage(hwnd, BFFM_SETSELECTION, (WPARAM) TRUE, (LPARAM)curDir); 
            TreeView_Expand(hwndTree, TreeView_GetSelection(hwndTree),TVE_EXPAND);
            TreeView_EnsureVisible(hwndTree, TreeView_GetSelection(hwndTree)); 
            SetFocus(hwndTree);
            SetTimer(hwnd,0,50,NULL); // Wait 50 ms and then try again
            return(1);
         }
         break;
      }

      case(WM_TIMER): // Necessary otherwise selected folder is not brought into view
      {
         KillTimer(hwnd,0);
         TreeView_SelectSetFirstVisible(hwndTree, TreeView_GetSelection(hwndTree)); 

         return(0);
      }


	   case(WM_COMMAND):
		{	
         int butNr = LOWORD(wParam)-WM_USER;

         if(butNr == RELATIVE_PATH)
         {
            SendDlgItemMessage(hwnd,WM_USER+RELATIVE_PATH,BM_SETCHECK,true,0);
            SendDlgItemMessage(hwnd,WM_USER+ABSOLUTE_PATH,BM_SETCHECK,false,0);
            pathType = "relative";
            break;

         }
         if(butNr == ABSOLUTE_PATH)
         {
            SendDlgItemMessage(hwnd,WM_USER+RELATIVE_PATH,BM_SETCHECK,false,0);
            SendDlgItemMessage(hwnd,WM_USER+ABSOLUTE_PATH,BM_SETCHECK,true,0);
            pathType  = "absolute";
            break;
         }

			HWND par = ::FindWindowEx(hwnd,NULL,"SHBrowseForFolder ShellNameSpace Control",NULL);
			HWND htree = ::FindWindowEx(par,NULL,"SysTreeView32",NULL);
//			HWND hedit = ::FindWindowEx(hwnd,NULL,"Edit",NULL);

         CText loc = labels[butNr];
         CText path = "";

         if(loc == "Prospa")
         {
            path = applicationHomeDir;
         }
         else if(loc == "Macros")
         {
            path = userMacroVar->GetString();
            if((ReplaceVarInString(&itfc,path)) == ERR)
               return(ERR);
         }
         else if(loc == "Data")
         {
            path = userWorkingVar->GetString();
            if((ReplaceVarInString(&itfc,path)) == ERR)
               return(ERR);
         }
         else if(loc == "Desktop")
         {
            ExpandEnvironmentStrings("%USERPROFILE%", str, MAX_PATH);
            strcat(str,"\\desktop");
            path = str;
         }
         else if(loc == "Computer")
         {
				HTREEITEM hMyComputerItem;
            hMyComputerItem = GetItemByName(htree,NULL,"Computer");
            if(!hMyComputerItem)
               hMyComputerItem = GetItemByName(htree,NULL,"My Computer");
            TreeView_SelectSetFirstVisible(hwndTree, hMyComputerItem); 
         }


      // Make sure the chosen location is selected
         if(path != "")
         {
            SendMessage(hwnd,BFFM_SETSELECTION,(WPARAM)TRUE,(LPARAM)path.Str());	
            SetTimer(hwnd,0,50,NULL);
            TreeView_Expand(hwndTree, TreeView_GetSelection(hwndTree),TVE_EXPAND);
            SetFocus(hwndTree);
         }

         break;
      }


      case(WM_SIZING):
      {
         RECT *r;
         r = (RECT*)lParam;
         if(r->bottom-r->top < 385)
            r->bottom = r->top + 385;
            
          break;
      }
      case(WM_SIZE):
      {
         RECT r,rp,pr;
 
         if(firstTime)
         {
         // Centre the window
            GetWindowRect(prospaWin,&pr);
            short width = pr.right - pr.left;
            short height = pr.bottom - pr.top;
            GetWindowRect(hwnd,&r);
            short w = r.right - r.left;
            short h = r.bottom - r.top;
            short xoff = (width - w)/2 + pr.left;
            short yoff = (height - h)/2 + pr.top;
            CorrectWindowPositionIfInvisible(xoff, yoff, w, h);
            MoveWindow(hwnd,xoff,yoff,w,h,true);
         }

         LRESULT rs = ::CallWindowProc(OldProc, hwnd, uMsg, wParam, lParam); ;

         if(allowRelativePath == "true")
         {
	         HWND label = ::FindWindowEx(hwnd,NULL,"Static",NULL);
            GetClientRect(label,&r);
            SetWindowPos(label,HWND_TOP,0,0,185,r.bottom-r.top,SWP_NOMOVE);
         }

         GetClientRect(hwnd,&rp);
		   HWND par = ::FindWindowEx(hwnd,NULL,"SHBrowseForFolder ShellNameSpace Control",NULL);
//		   HWND htree = ::FindWindowEx(par,NULL,"SysTreeView32",NULL);
         if(withButtons)
             SetWindowPos(par,HWND_BOTTOM,105,55,rp.right-rp.left-120,rp.bottom-rp.top-135,SWP_SHOWWINDOW);
         else
             SetWindowPos(par,HWND_BOTTOM,10,55,rp.right-rp.left-20,rp.bottom-rp.top-135,SWP_SHOWWINDOW);

         return(rs);
	   }

   }


	return ::CallWindowProc(OldProc, hwnd, uMsg, wParam, lParam); 
}


HTREEITEM GetItemByName(HWND hWnd, HTREEITEM hItem,
                        LPCTSTR szItemName)
{
    // If hItem is NULL, start search from root item.
    if (hItem == NULL)
        hItem = (HTREEITEM)SendMessage(hWnd, TVM_GETNEXTITEM,
                                       TVGN_ROOT, 0);
    while (hItem != NULL)
    {
        char szBuffer[MAXTEXTLEN+1];
        TV_ITEM item;

        item.hItem = hItem;
        item.mask = TVIF_TEXT | TVIF_CHILDREN;
        item.pszText = szBuffer;
        item.cchTextMax = MAXTEXTLEN;
        SendMessage(hWnd, TVM_GETITEM, 0, (LPARAM)&item);

        // Did we find it?
        if (lstrcmp(szBuffer, szItemName) == 0)
            return hItem;

      //   Check whether we have child items.
        if (item.cChildren)
        {
            // Recursively traverse child items.
            HTREEITEM hItemFound = NULL, hItemChild;

            hItemChild = (HTREEITEM)SendMessage(hWnd, TVM_GETNEXTITEM,
                                                TVGN_CHILD, (LPARAM)hItem);
            if(hItemChild)
               hItemFound = GetItemByName(hWnd, hItemChild, szItemName);

            // Did we find it?
            if (hItemFound != NULL)
                return hItemFound;
        }

        // Go to next sibling item.
        hItem = (HTREEITEM)SendMessage(hWnd, TVM_GETNEXTITEM,
                                       TVGN_NEXT, (LPARAM)hItem);
    }

    // Not found.
    return NULL;
}
		
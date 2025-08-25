#include "stdafx.h"
#include "preferences.h"
#include "cArg.h"
#include "defineWindows.h"
#include "dialog_utilities.h"
#include "edit_utilities.h"
#include "files.h"
#include "font.h"
#include "globals.h"
#include "import_export_utilities.h"
#include "interface.h"
#include "load_save_data.h"
#include "main.h"
#include "message.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "tab.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "dlgs.h"
#include "memoryLeak.h"

#pragma warning (disable: 4311) // Ignore pointer truncation warnings

HWND hDlg = (HWND) NULL;
HWND hTabWnd;
HWND hStatusWnd;
char TabName[][40] = 
{
   "Menus",
   "Macros",
   "DLLs",
 //  "Plots",
   "Folders"
};

BOOL CALLBACK MenuSelectDialog(HWND hdwnd, UINT message,WPARAM wParam, LPARAM lParam);
BOOL CALLBACK MacroSearchPathDialog(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DLLSearchPathDialog(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
//BOOL CALLBACK ColorDialog(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK FolderDialog(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void InitStatus(HWND hwnd);
char* GetRelativePath(char *path1, char *path2, char *varName);
UINT  MenuFolderHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
int GetMenuFolderName(CText &result);
short button_checked = ID_APP_DIR;
short GetRelPath(char* relPath);

void UpdatePlotParameters(HWND hWnd);
void UpdatePlotDefaults(HWND hWnd);

// Externally accessible
HWND prefsWin = NULL;

int CALLBACK PreferencesDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   NMHDR *nmptr;
   int tabnumber = 0;
   TCITEM tci;
   RECT winRect;
   static HFONT hFont;
   HDC hdc;
   
   switch(message)
   {
      case (WM_INITDIALOG):
      
         GetClientRect(hWnd,&winRect);
         hTabWnd = CreateWindow
                   (
                      WC_TABCONTROL,
                      "", 
                      WS_VISIBLE | WS_TABSTOP | WS_CHILD, 
                      0, 5, winRect.right, winRect.bottom-5,
                      hWnd,
                      NULL,
                      prospaInstance,
                      NULL
                   );
   
         hdc = GetDC(hTabWnd);
         hFont = MakeFont(hdc, "MS Sans Serif", 8, 0, 0, 0);
         SendMessage(hTabWnd,WM_SETFONT,(WPARAM)hFont,MAKELPARAM(false,0));
         ReleaseDC(hTabWnd,hdc);
         
         tci.mask = TCIF_TEXT;
         tci.iImage = -1;
   
         tci.pszText = TabName[0];
         TabCtrl_InsertItem(hTabWnd, 0, &tci);
   
         tci.pszText = TabName[1];
         TabCtrl_InsertItem(hTabWnd, 1, &tci);
   
         tci.pszText = TabName[2];
         TabCtrl_InsertItem(hTabWnd, 2, &tci);
         
         tci.pszText = TabName[3];
         TabCtrl_InsertItem(hTabWnd, 3, &tci);  

         //tci.pszText = TabName[4];
         //TabCtrl_InsertItem(hTabWnd, 4, &tci); 
   
         hDlg = CreateDialog(prospaInstance, "PrefMenuTab", hTabWnd,(DLGPROC) MenuSelectDialog);
         
         PlaceDialogOnTopInCentre(hWnd);
         SaveDialogParent(hWnd);

         prefsWin = hWnd;

         break;
              
    case (WM_NOTIFY):
     
        nmptr = (LPNMHDR) lParam;
        if(nmptr->code == TCN_SELCHANGE)
        {
           if(hDlg) DestroyWindow(hDlg);
           tabnumber = TabCtrl_GetCurSel((HWND)nmptr->hwndFrom);
           switch(tabnumber) 
           {
                case 0: 
                  hDlg = CreateDialog(prospaInstance, "PrefMenuTab",
                                      hTabWnd, (DLGPROC) MenuSelectDialog);
                  break;
                case 1: 
                  hDlg = CreateDialog(prospaInstance, "PrefPathTab",
                                      hTabWnd, (DLGPROC) MacroSearchPathDialog);
                  break;
                case 2: 
                  hDlg = CreateDialog(prospaInstance, "PrefDLLTab",
                                      hTabWnd, (DLGPROC) DLLSearchPathDialog);
                  break;                   
                //case 3: 
                //  hDlg = CreateDialog(prospaInstance, "PrefColorTab",
                //                      hTabWnd, (DLGPROC) ColorDialog);
                //  break; 
                case 3: 
                  hDlg = CreateDialog(prospaInstance, "PrefFolderTab",
                                      hTabWnd, (DLGPROC) FolderDialog);
                  break; 
                default:
                   hDlg = NULL;
            }
            
         }
         break;
         
      case(WM_CLOSE): // Destroy dialog and redraw plot window
      {
         prefsWin = NULL;
         if(hDlg) DestroyWindow(hDlg);
         DeleteObject(hFont);
         MyEndDialog(hWnd,1);
         return(false);
      }
      
      case(WM_COMMAND):
      {
         switch(LOWORD(wParam))
         {
            case(ID_CANCEL): // Cancel button presses - remove window
            {
               if(hDlg) DestroyWindow(hDlg);
               DeleteObject(hFont);
               SetWindowLong(hWnd, GWL_HWNDPARENT, (LONG)prospaWin);
               MyEndDialog(hWnd,1);
               return(true);
            }
         }
      }
   }

   return(false);
}


// Prospa menu selection dialog function.
BOOL CALLBACK MenuSelectDialog(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  char relPath[MAX_PATH];
  static HWND win;
  
  switch(message) 
  {
     case(WM_INITDIALOG):
     {
         win = GetDlgItem(hWnd,ID_MENU_NAMES);
         for(int i = 0; i < VarWidth(userMenusVar); i+=2)
         {
            AppendLine(win,VarList(userMenusVar)[i]);    
         }
         SelectChar(win,0,0);
         SendMessage(win,EM_SCROLLCARET,0,0);                

         win = GetDlgItem(hWnd,ID_MENU_FOLDERS);         
         for(int i = 1; i < VarWidth(userMenusVar); i+=2)
         {   
            AppendLine(win,VarList(userMenusVar)[i]);    
         }
         SelectChar(win,0,0);
         SendMessage(win,EM_SCROLLCARET,0,0);                
             
         return(1);
      }
                      
      case(WM_COMMAND):
      {
         switch(LOWORD(wParam))
         {
            case(ID_MENU_NAMES):
            {
               win = GetDlgItem(hWnd,ID_MENU_NAMES);                    
               break; 
            }  
             
            case(ID_MENU_FOLDERS):
            {
               win = GetDlgItem(hWnd,ID_MENU_FOLDERS);         
               break; 
            }
                
            case(ID_ADD_MENU_NAME):
            {
               win = GetDlgItem(hWnd,ID_MENU_NAMES);
               strncpy_s(relPath,MAX_PATH,"&New menu name",_TRUNCATE);
               AppendLine(win,relPath);
               SelectPartCurrentLine(win,1,-1);
               SetFocus(win);                     
               break;
            }

            case(ID_ADD_MENU_FOLDER):
            {
               win = GetDlgItem(hWnd,ID_MENU_FOLDERS);
               if(GetRelPath(relPath) == OK)
               {            
                  AppendLine(win,relPath);    
                  SelectChar(win,CountLinesInEditor(win)-1,0);
                  SendMessage(win,EM_SCROLLCARET,0,0);                
                  SetFocus(win);                     
               }
               break;
            }
                  
            case(ID_REMOVE_MENU_NAME):
            {      
               win = GetDlgItem(hWnd,ID_MENU_NAMES);
               long n = SelectCurrentLine(win);
               SendMessage(win,EM_REPLACESEL,(WPARAM)TRUE, (LPARAM)"");
               long lines = CountLinesInEditor(win);
               if(n < lines-1)
                   SelectChar(win,n,0);
               else if(n > 0)
                   SelectChar(win,n-1,0);
               SendMessage(win,EM_SCROLLCARET,0,0);                
               SetFocus(win);
               break; 
            }

            case(ID_REMOVE_MENU_FOLDER):
            {            
               win = GetDlgItem(hWnd,ID_MENU_FOLDERS);         
               long n = SelectCurrentLine(win);
               SendMessage(win,EM_REPLACESEL,(WPARAM)TRUE, (LPARAM)"");
               long lines = CountLinesInEditor(win);
               if(n < lines-1)
                   SelectChar(win,n,0);
               else if(n > 0)
                   SelectChar(win,n-1,0);
               SendMessage(win,EM_SCROLLCARET,0,0);                
               SetFocus(win);
               break; 
            }
            
            case(ID_MENU_UP):
            {
               char path[MAX_PATH];
               long n = GetCurrentLine(win,path);
               if(n > 0)
               {
                  InsertLine(win,path,n-1);
                  RemoveLine(win,n+1);
                  SelectChar(win,n-1,0);
                  SendMessage(win,EM_SCROLLCARET,0,0);                
                  SetFocus(win);
               }
               break; 
            }
            
            case(ID_MENU_DOWN):
            {
               char path[MAX_PATH];
               long n = GetCurrentLine(win,path);
               long sz = CountLinesInEditor(win);
               if(n < sz-2)
               {
                  RemoveLine(win,n);
                  InsertLine(win,path,n+1);
               }
               else if(n == sz-2)
               {
                  RemoveLine(win,n);
                  AppendLine(win,path);
               }
               SelectChar(win,n+1,0);
               SendMessage(win,EM_SCROLLCARET,0,0);
               SetFocus(win);
               break; 
            }   
               
            case(ID_SAVE_MENUS):
            {
               char curFolder[MAX_PATH];
               char *txt;

               int namesLines = CountLinesInEditor(GetDlgItem(hWnd,ID_MENU_NAMES));
               int folderLines = CountLinesInEditor(GetDlgItem(hWnd,ID_MENU_FOLDERS));
               
               if(namesLines == folderLines)
               {                                
                  GetCurrentDirectory(MAX_PATH,curFolder);
                  if(!SetCurrentDirectory(userPrefDir))
                  {
                     MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to move into preferences directory.\rIs <prospahome>\\preferences missing or write protected?");
                     return(1);
                  }
                  
                  FILE *fp = fopen("userMenus.lst","w");
                  if(fp)
                  {
                     userMenusVar->MakeList(namesLines*2);
                     HWND winNames = GetDlgItem(hWnd,ID_MENU_NAMES);
                     HWND winFolder = GetDlgItem(hWnd,ID_MENU_FOLDERS);
                     for(int i = 0, j = 0; i < namesLines; i++)
                     {
                        txt = GetLineByNumber(winNames,i);
                        if(txt[0] != '\0')   
                        {
                           userMenusVar->ReplaceListItem(txt,j++);
                           fprintf(fp,"%s",txt);
                           delete [] txt;
                           txt = GetLineByNumber(winFolder,i);
                           userMenusVar->ReplaceListItem(txt,j++);
                           fprintf(fp,",%s\n",txt);
                           delete [] txt;
                        }
                        else
                        { 
                            MessageDialog(prospaWin,MB_ICONERROR,"Error","List is empty"); 
                            fclose(fp);
                            SetCurrentDirectory(curFolder);
                            return(1); 
                        }
                     }
                     fclose(fp);                     
                  }
                  else
                  {
                     MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to save menu preferences.\rIs file 'userMenus.lst' write protected?");
                     return(1);
                  }                  

                  SetCurrentDirectory(curFolder);
                  strncpy_s(gCurrentDir,MAX_PATH,curFolder,_TRUNCATE);
               
                  UpdateMainMenu(prospaWin);
                  MessageDialog(hWnd,MB_ICONINFORMATION,"Information","Menu list saved");
               }
               else
               {
                  MessageDialog(hWnd,MB_ICONERROR,"Error","Lists should be same size"); 
                  SetCurrentDirectory(curFolder);
               }
               break;
            } 
         }
         return(1); 
      }        
  } 
  return(0);
}


// Prospa macro search path dialog function.
BOOL CALLBACK MacroSearchPathDialog(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   char relPath[MAX_PATH]; 

   HWND win = GetDlgItem(hWnd,ID_SEARCH_PATH);
     
   switch(message) 
   {
      case (WM_INITDIALOG):
      {
      // Update the search path list
         Variable *pathList;        
         pathList = globalVariable.Get("macrosearchpath");
         for(int i = 0; i < VarWidth(pathList); i++)
         {
            AppendLine(win,VarList(pathList)[i]);
         }
         SelectChar(win,0,0);
         SendMessage(win,EM_SCROLLCARET,0,0);
      // Update the cache procedure checkbox status
		//	SendDlgItemMessage(hWnd,ID_CACHE_PROC,BM_SETCHECK,cacheProcedures,0);          
         return(1);
      }
     
      case (WM_LBUTTONDOWN):
      {
        short x = LOWORD(lParam);  // horizontal position of cursor 
        short y = HIWORD(lParam);  // vertical position of cursor 

        if(hWnd == GetDlgItem(hWnd,ID_SEARCH_PATH))
        {
           short c = SendDlgItemMessage(hWnd,ID_SEARCH_PATH,EM_CHARFROMPOS,0, MAKELPARAM(x, y));
           short l = SendDlgItemMessage(hWnd,ID_SEARCH_PATH,EM_LINEFROMCHAR,(WPARAM)c, 0);
           c = SendDlgItemMessage(hWnd,ID_SEARCH_PATH,EM_LINEINDEX,(WPARAM)l, 0);
           l = SendDlgItemMessage(hWnd,ID_SEARCH_PATH,EM_LINELENGTH,(WPARAM)c, 0);
           SendDlgItemMessage(hWnd,ID_SEARCH_PATH,EM_SETSEL,(WPARAM)c, (LPARAM)c+l);
        }
        break;
     }

     case(WM_COMMAND):
     {
     
         switch(LOWORD(wParam))
         {
            case(ID_CACHE_PROC): // Extract the current procedure cache checkbox selection
            {
	     //        cacheProcedures = SendDlgItemMessage(hWnd,ID_CACHE_PROC,BM_GETCHECK,0,0);
                break;
            }

            case(ID_ADD_PATH): // User wants to select a new path
            {
               if(GetRelPath(relPath) == OK) // Display dialog to select path
               {
                  AppendLine(win,relPath);    
                  SelectChar(win,CountLinesInEditor(win)-1,0);
                  SendMessage(win,EM_SCROLLCARET,0,0);
                  SetFocus(win);
               }
               break;
            }

            case(ID_REMOVE_PATH): // Remove current path
            {            
               long n = SelectCurrentLine(win);
               SendMessage(win,EM_REPLACESEL,(WPARAM)TRUE, (LPARAM)"");
               long lines = CountLinesInEditor(win);
               if(n < lines-1)
                   SelectChar(win,n,0);
                else if(n > 0)
                    SelectChar(win,n-1,0);
                SendMessage(win,EM_SCROLLCARET,0,0);
               SetFocus(win);
               break; 
            }
            
            
            case(ID_PATH_UP): // Move the selected path up the page
            {
               char path[MAX_PATH];
               long n = GetCurrentLine(win,path);
               if(n > 0)
               {
                   InsertLine(win,path,n-1);
                   RemoveLine(win,n+1);
                   SelectChar(win,n-1,0);
                   SendMessage(win,EM_SCROLLCARET,0,0);
                  SetFocus(win);
               }
               break; 
            }
            
            case(ID_PATH_DOWN): // Move the selected path down the page
            {
               char path[MAX_PATH];
               long n = GetCurrentLine(win,path);
               long sz = CountLinesInEditor(win);
               if(n < sz-2)
               {
                   RemoveLine(win,n);
                   InsertLine(win,path,n+1);
               }
               else if(n == sz-2)
               {
                   RemoveLine(win,n);
                   AppendLine(win,path);
               }
                SelectChar(win,n+1,0);
                SendMessage(win,EM_SCROLLCARET,0,0); 
               SetFocus(win);
               break; 
            }

            case(ID_SAVE_PATH): // Save the path to a file
            {
               char curFolder[MAX_PATH];
               char *txt;

               GetCurrentDirectory(MAX_PATH,curFolder);
               if(!SetCurrentDirectory(userPrefDir))
               {
                  MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to move into preferences directory.\rIs <prospahome>\\preferences missing or write protected?");
                  return(1);
               }
                  
               FILE *fp = fopen("macroSearchPath.lst","w");
               if(fp)
               {
                  long lines = CountLinesInEditor(win);
                  macroPathVar->MakeList(lines); 
                                
                  for(int i = 0; i < lines; i++)
                  {
                     txt = GetLineByNumber(win,i);
                     if(txt[0] != '\0')
                     {
                        macroPathVar->ReplaceListItem(txt,i);
                        fprintf(fp,"%s\n",txt);
                     }
                     delete [] txt;
                  }
                  fclose(fp);                  
               }
               else
               {
                  MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to save macro search path.\rIs file 'macroSearchPath.lst' write protected?");
                  return(1);
               }               

               SetCurrentDirectory(curFolder);
               strncpy_s(gCurrentDir,MAX_PATH,curFolder,_TRUNCATE);
               MessageDialog(hWnd,MB_ICONINFORMATION,"Information","Search path saved");
               break;
            }
         }
         return(1);
      }
  }
  return(0);
}

// Prospa DLL search path dialog function.
BOOL CALLBACK DLLSearchPathDialog(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   char relPath[MAX_PATH];
  
   HWND win = GetDlgItem(hWnd,ID_SEARCH_PATH);
     
   switch(message) 
   {
      case (WM_INITDIALOG):
      {
         Variable *pathList;
        
         pathList = globalVariable.Get("dllsearchpath");
         for(int i = 0; i < VarWidth(pathList); i++)
         {
            AppendLine(win,VarList(pathList)[i]);
         }
         SelectChar(win,0,0);
         SendMessage(win,EM_SCROLLCARET,0,0);
         
        return(1);
      }
     
      case (WM_LBUTTONDOWN):
      {
        short x = LOWORD(lParam);  // horizontal position of cursor 
        short y = HIWORD(lParam);  // vertical position of cursor 

        if(hWnd == GetDlgItem(hWnd,ID_SEARCH_PATH))
        {
           short c = SendDlgItemMessage(hWnd,ID_SEARCH_PATH,EM_CHARFROMPOS,0, MAKELPARAM(x, y));
           short l = SendDlgItemMessage(hWnd,ID_SEARCH_PATH,EM_LINEFROMCHAR,(WPARAM)c, 0);
           c = SendDlgItemMessage(hWnd,ID_SEARCH_PATH,EM_LINEINDEX,(WPARAM)l, 0);
           l = SendDlgItemMessage(hWnd,ID_SEARCH_PATH,EM_LINELENGTH,(WPARAM)c, 0);
           SendDlgItemMessage(hWnd,ID_SEARCH_PATH,EM_SETSEL,(WPARAM)c, (LPARAM)c+l);
        }
        break;
     }

     case(WM_COMMAND):
     {
     
         switch(LOWORD(wParam))
         {
            case(ID_ADD_PATH):
             {
               if(GetRelPath(relPath) == OK)
               {            
                  AppendLine(win,relPath);    
                  SelectChar(win,CountLinesInEditor(win)-1,0);
                  SendMessage(win,EM_SCROLLCARET,0,0);
                  SetFocus(win);
               }
               break;
             }

            case(ID_REMOVE_PATH):
            {
               long n = SelectCurrentLine(win);
               SendMessage(win,EM_REPLACESEL,(WPARAM)TRUE, (LPARAM)"");
               long lines = CountLinesInEditor(win);
               if(n < lines-1)
                   SelectChar(win,n,0);
                else if(n > 0)
                    SelectChar(win,n-1,0);
                SendMessage(win,EM_SCROLLCARET,0,0);
               SetFocus(win);
               break; 
            }
            
            case(ID_PATH_UP):
            {
               char path[MAX_PATH];
               long n = GetCurrentLine(win,path);
               if(n > 0)
               {
                  InsertLine(win,path,n-1);
                  RemoveLine(win,n+1);
                  SelectChar(win,n-1,0);
                  SendMessage(win,EM_SCROLLCARET,0,0);
                  SetFocus(win);
               }
               break; 
            }

            case(ID_PATH_DOWN):
            {
               char path[MAX_PATH];
               long n = GetCurrentLine(win,path);
               long sz = CountLinesInEditor(win);
               if(n < sz-2)
               {
                  RemoveLine(win,n);
                  InsertLine(win,path,n+1);
               }
               else if(n == sz-2)
               {
                  RemoveLine(win,n);
                  AppendLine(win,path);
               }
               SelectChar(win,n+1,0);
               SendMessage(win,EM_SCROLLCARET,0,0); 
               SetFocus(win);
               break; 
            }
            
            case(ID_SAVE_PATH):
            {
               char curFolder[MAX_PATH];
               char *txt;
                  
               GetCurrentDirectory(MAX_PATH,curFolder);
               if(!SetCurrentDirectory(userPrefDir))
               {
                  MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to move into preferences directory.\rIs <prospahome>\\preferences missing or write protected?");
                  return(1);
               }
                  
               FILE *fp = fopen("dllSearchPath.lst","w");
               if(fp)
               {
                  long lines = CountLinesInEditor(win);
                  dllPathVar->MakeList(lines);
                              
                  for(int i = 0; i < lines; i++)
                  {
                     txt = GetLineByNumber(win,i);
                     if(txt[0] != '\0')   
                     {
                        dllPathVar->ReplaceListItem(txt,i);
                        fprintf(fp,"%s\n",txt);
                     }
                     delete [] txt;
                  }
                  fclose(fp);                  
               }
               else
               {
                  MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to save dll search path.\rIs file 'dllSearchPath.lst' write protected?");
                  return(1);
               }               

               SetCurrentDirectory(curFolder);  
               strncpy_s(gCurrentDir,MAX_PATH,curFolder,_TRUNCATE);  
              
               MessageDialog(hWnd,MB_ICONINFORMATION,"Information","Search path successfully saved");
               break;
            }
         }
         return(1);
      }
  }
  return(0);
}

short GetRelPath(char* relPath)
{
   char *path;
   CText name;

   GetMenuFolderName(name);

   if(name != "cancel")
   {
      if(button_checked == ID_APP_DIR)
      { 
         if(path = GetRelativePath(applicationHomeDir,name.Str(),"appdir"))
            strncpy_s(relPath,MAX_PATH,path,_TRUNCATE);
         else
         {
            MessageDialog(prospaWin,MB_ICONERROR,"Error","Can't find relative path");
            return(ERR);
         }

      }
      else if(button_checked == ID_PREFDIR)
      {
         if(path = GetRelativePath(userPrefDir,name.Str(),"prefdir"))
            strncpy_s(relPath,MAX_PATH,path,_TRUNCATE);
         else
         {
            MessageDialog(prospaWin,MB_ICONERROR,"Error","Can't find relative path");
            return(ERR);
         }
      }         
      else
         strncpy_s(relPath,MAX_PATH,name.Str(),_TRUNCATE);
      return(OK);
   }
   return(ERR);
}
   
 
/***********************************************************************
* Returns a string representing path2 in terms of path1
* Note path1 and path2 must be at less than MAX_PATH-1 characters in length
************************************************************************/

char* GetRelativePath(char *path1, char *path2, char *varName)
{
   long i,end,cnt;
   static char relpath[MAX_PATH];
	char p1[MAX_PATH] = {'\0'};
   char p2[MAX_PATH] = {'\0'};
   end  = 0;

// Paths may be modified so make copies
   strncpy_s(p1,MAX_PATH,path1,_TRUNCATE);
   strncpy_s(p2,MAX_PATH,path2,_TRUNCATE);

   long len1 = strlen(p1);
   long len2 = strlen(p2);
   
// Add \\ to end of path1 if not present
   if(p1[len1-1] != '\\')
   {
      p1[len1] = '\\';
      p1[++len1] = '\0';
   }
   
// Add \\ to end of path2 if not present
   if(p2[len2-1] != '\\')
   {
      p2[len2] = '\\';
      p2[++len2] = '\0';
   }

// Find shorter path
   long minlen = (len1 < len2) ? len1 : len2;
             
// Find common base folder (convert all path characters to lower case when comparing)
   for(end = -1,i = 0; i < minlen; i++)
   {
      if(p1[i] == '\\') end = i;
      if(tolower(p1[i]) != tolower(p2[i])) break;
   }
   
// If there is no common base folder return NULL string
   if(end == -1)
   {
      return(NULL);
   }
   
// Now see how many levels we have to move up in path1 to get to common path
   for(cnt = 0, i = end+1; i < len1; i++)
   {
      if(p1[i] == '\\') cnt++;
   }

// Append this to path1 (which is now a variable reference)
   sprintf(relpath,"$%s$\\",varName);
   
   for(i = 0; i < cnt; i++)
      strcat(relpath,"..\\");
   
// And then append rest of path2
   strcat(relpath,&p2[end+1]);
   return(relpath);
}

/************************************************************************
*             Return "path" expressed in terms of "variable"            *
************************************************************************/
//TODO needs work to convert to CTEXT
int RelativePath(Interface* itfc ,char args[])
{
   short n;
   Variable var;
   char path1[MAX_PATH];
   char path2[MAX_PATH];   
   char *relPath;
   int i;
   CText delimiters = "$$";
      
   if((n = ArgScan(itfc,args,2,"path, variable [,delimiter]","eee","svt",path2,&var,&delimiters)) < 0)
      return(n);
   
   if(VarType(&var) == UNQUOTED_STRING) 
   {  
		CArg carg(',');
		carg.Count(args);
      strncpy_s(path1,MAX_PATH,VarString(&var),_TRUNCATE);
      relPath = GetRelativePath(path1, path2, carg.Extract(2));
      if(relPath == NULL)
      {
         ErrorMessage("No relative path possible");
         return(ERR);
      }

      int len = strlen(relPath);
		short cnt = 0;

      for(i = 0; i < len; i++)
      {
         if(relPath[i] == '$')
         {
            if(cnt == 0)
            {
               cnt++;
               relPath[i] = delimiters[0];
            }
            else if(cnt == 1)
            {
               cnt++;
               relPath[i] = delimiters[1];
               break;
            }
         }
      }

      if(relPath[len-1] == '\\') // Remove trailing backslash
         relPath[len-1] = '\0';

      if(delimiters == "none")
      {      
         relPath = &relPath[i+1];
	      itfc->retVar[1].MakeAndSetString(relPath);
	      itfc->nrRetValues = 1;
         return(OK);
      }

	  itfc->retVar[1].MakeAndSetString(relPath);
	  itfc->nrRetValues = 1;
   }
   else
   {
      ErrorMessage("Invalid path variable");
      return(ERR);
   }
   
   return(OK);
}      
/************************************************************************
*                       Select a folder name                            *
************************************************************************/

int GetMenuFolderName(CText &result)
{
   short err;
   long flag = (OFN_EXPLORER | OFN_HIDEREADONLY | OFN_ENABLEHOOK | OFN_NONETWORKBUTTON | OFN_NOVALIDATE);
   char fileName[MAX_STR];
   char folder[MAX_PATH];
   static short index = 0;
   char oldDir[MAX_PATH];

// Save the current working directory as the following command changes it
   GetCurrentDirectory(MAX_PATH,oldDir);

   strncpy_s(fileName,MAX_STR,"Move into desired folder then press open",_TRUNCATE);
   strncpy_s(folder,MAX_PATH,applicationHomeDir,_TRUNCATE);

   if(IsPlacesBarHidden())
   {
      err = FileDialog(prospaWin, true, folder, fileName, 
                               "Select Folder", MenuFolderHookProc, "SETMENUFOLDER",
                               flag | templateFlag, 0, &index);
   }
   else
   {
      err = FileDialog(prospaWin, true, folder, fileName, 
                               "Select Folder", MenuFolderHookProc, "SETMENUFOLDER2",
                               flag | templateFlag, 0, &index);
   }

   if(err == ABORT)
      result = "cancel";
   else
      result = folder;
      
// Restore the directory
   SetCurrentDirectory(oldDir);
                                   
   return(0);
}

// Hook procedure to process events from set folder dialog (attached to standard load dialog)

UINT MenuFolderHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
   switch (message)
   {
      case(WM_INITDIALOG): // Hide a few controls and change OK label
      { 
         HWND hdlg = GetParent(hWnd);
         CommDlg_OpenSave_HideControl(hdlg,edt1);
   
         CommDlg_OpenSave_HideControl(hdlg,stc3);
         CommDlg_OpenSave_HideControl(hdlg,cmb1);
   //      CommDlg_OpenSave_HideControl(hdlg,stc2);
         CommDlg_OpenSave_SetControlText(hdlg,IDOK,"Select folder");
         CommDlg_OpenSave_SetControlText(hdlg, stc2, "Relative to:");

         SendDlgItemMessage(hWnd,ID_APP_DIR,BM_SETCHECK,(button_checked == ID_APP_DIR),0); 
         SendDlgItemMessage(hWnd,ID_PREFDIR,BM_SETCHECK,(button_checked == ID_PREFDIR),0); 
         SendDlgItemMessage(hWnd,ID_ABSOLUTE,BM_SETCHECK,(button_checked == ID_ABSOLUTE),0); 
                          
         break;
      }

      case(WM_NOTIFY): // Process events from standard part of dialog
      {
         LPOFNOTIFY lpon = (LPOFNOTIFY)lParam;
         
         switch(lpon->hdr.code)
         {
            case(CDN_INITDONE): // Std dialog has finished updating so centre it in window
            {
               PlaceDialogOnTopInCentre(hWnd,lpon->hdr.hwndFrom);
               break;
            }   
            case(CDN_FILEOK): // OK button has been pressed in std dialog
            {
               if(IsDlgButtonChecked(hWnd,ID_APP_DIR))
                  button_checked = ID_APP_DIR;
               else if(IsDlgButtonChecked(hWnd,ID_PREFDIR))
                  button_checked = ID_PREFDIR;
               else if(IsDlgButtonChecked(hWnd,ID_ABSOLUTE))
                  button_checked = ID_ABSOLUTE;                  
               break;   
             //  return(1);
            }
         }
         break;
      }
      
      case(WM_DESTROY):
      {
         DialogTidyUp(hWnd,true);
         break;
      }

   }   
   return(0);
}

// Prospa folder selection dialog function.
BOOL CALLBACK FolderDialog(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   char dir[MAX_PATH];

   HWND win = GetDlgItem(hWnd,ID_DISPLAY_WK_DIR);
     
   switch(message) 
   {
      case (WM_INITDIALOG):
      {    
          SetWindowText(win,VarString(userWorkingVar));
          return(1);
      }
     
     case(WM_COMMAND):
     {     
        switch(LOWORD(wParam))
        {
            case(ID_SELECT_WK_DIR):
                strncpy_s(dir,MAX_PATH,gCurrentDir,_TRUNCATE);
                GetWindowText(win,gCurrentDir,MAX_PATH);
                Interface itfc;
                GetFolderName(&itfc,"");
                SetFocus(hWnd);
                char* response = itfc.retVar[1].GetString();
                if(strcmp(response,"cancel"))
                {
                   SetWindowText(win,response);
                   userWorkingVar->MakeAndSetString(response);
                   SetCurrentDirectory(dir);
                   strncpy_s(gCurrentDir,MAX_PATH,dir,_TRUNCATE);
                }
                break;
 
        }
        return(1);  
     } 

     case(WM_DRAWITEM):
     {
        //COLORREF color;
        //LPDRAWITEMSTRUCT pdis = (LPDRAWITEMSTRUCT) lParam; 
        //GetPlotColor(curPlot,pdis->CtlID,color);
        //FillRect(pdis->hDC, &pdis->rcItem, (HBRUSH)CreateSolidBrush(color));
        //FrameRect(pdis->hDC, &pdis->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
     }
  }
  return(0);
}

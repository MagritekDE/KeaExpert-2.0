#include "stdafx.h"
#include "cli_events.h"

//#include "C:\Program Files\Visual Leak Detector\include\vld.h"
#include "debug.h"
#include "defineWindows.h"
#include "edit_class.h"
#include "edit_utilities.h"
#include "files.h"
#include "globals.h"
#include "guiMakeObjects.h"
#include "guiModifyObjectParameters.h"
#include "guiWindowClass.h"
#include "guiWindowCLI.h"
#include "guiWindowsEvents.h"
#include "interface.h"
#include "main.h"
#include "message.h"
#include "process.h"
#include "scanstrings.h"
#include "string_utilities.h"
#include "syntax_viewer.h"
#include "variablesOther.h"
#include "CArg.h"
#include <assert.h>
#include <richedit.h>
#include "memoryLeak.h"


// Globally defined variables
long cliHistory = 0; // Number of commands entered
FILE* printFile;
bool printToString = false;
CText printString;
extern bool gIsConsole;

/**********************************************************************************
*            Routines relating to the Command Line Interface (CLI)
*
* CLIFunctions ................. clifunc command arguments to access cli functions: 
*
*   copy
*   command help
*   cut
*   paste
*   undo
*
***********************************************************************************/


/*****************************************************************************
*                   Event procedure for CLI window
* Events
*
* WM_MOUSEACTIVATE ... cli selected
* WM_PASTE ........... redraw plot
* WM_ERASEBKGND ...... prevent flicker TODO
* WM_LBUTTONDOWN ..... plot selected also move, data, region selection 
* WM_CHAR ............ processed key information
* WM_KEYDOWN ......... raw key information
*****************************************************************************/


LRESULT CALLBACK CLIEditEventsProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   unsigned char key = wParam; 
	long length;
   long pos;   
   static char *curLine = NULL; // Last line into which user entered data manually

// Find window and object receiving events
   HWND parWin = GetParent(hWnd);
   WinData* win = rootWin->FindWinByHWND(parWin);
   if(!win)
   {
      HWND parWin2 = GetParent(parWin);
      win = GetWinDataClass(parWin2);
   }
   ObjectData *obj = NULL;
   if(win)
   {
      obj = GetObjData(hWnd); 
   }

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

      case(WM_PAINT):
      {
         CallWindowProc(OldCLIProc,hWnd,messg,wParam,lParam);

         if(obj && obj->selected_)
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

      case(WM_MOUSEACTIVATE): // User has clicked on window - update title, menu and toolbar
      {
         if(gDebug.mode != "off") // Change current GUI window if we are not debugging
            break;
         cliWin = parWin;
         cliEditWin = hWnd;
         if(!obj) return(1);
         SelectFixedControls(win, obj);
			CText txt;
			if(win->titleUpdate)
         {
			   txt.Format("CLI (%hd)",win->nr);
			   SetWindowText(win->hWnd,txt.Str());	
         }
			//else
			//   txt.Format("CLI");

         if(win && !win->keepInFront)
            ChangeGUIParent(parWin);
         SetFocus(hWnd);
         break;
      }

// Modify paste command to only paste single lines into the CLI
// But not reliable the first part works but I can't undo. The second
// only works sometimes - don't know why. Close/Open and empty in middle make no difference.
      case(WM_PASTE):
      {
		//	            LPSTR txt;

	 //     if(OpenClipboard(hWnd)) //Paste text from the clipboard
	 //     { 
  //       // Get paste buffer and replace with first line of data   	   
	 //        HGLOBAL hglb,hglb2;

	 //        if((hglb = GetClipboardData(CF_TEXT)) != NULL) 
	 //        {
	 //           LPSTR inTxt = (LPSTR)GlobalLock(hglb); 
  //             txt = new char[strlen(inTxt)+1];
  //             strcpy(txt,inTxt);
	 //           ReplaceSpecialCharacters(txt,"\r\n","\n",-1);
	 //           ReplaceSpecialCharacters(txt,"\n",";",-1);
	 //           GlobalUnlock(hglb);  
		//			//TextMessage("%s",txt); // Print modified text
		//		 //  messageSent = 0; // Ensures return and > not printed
		//			//delete [] txt;
  //    	   }
	 //     }
		////	CloseClipboard(); 

  // //      return(0);
	 //       HGLOBAL hglbCopy;
		//	  LPSTR lptstrCopy;
		////	if (!OpenClipboard(hWnd)) 
		////		 return(0); 
		//	 //EmptyClipboard(); 
		//// Allocate a global memory buffer for the text.
		//	 int sz = strlen(txt);
		//	hglbCopy = GlobalAlloc(GHND | GMEM_SHARE, sz+1); 
		//// Lock the handle and copy the selected text to the buffer.  
		//	lptstrCopy = (LPSTR)GlobalLock(hglbCopy); 
		//	memcpy(lptstrCopy, txt, sz); 
		//	lptstrCopy[sz] = '\0';
		//	GlobalUnlock(hglbCopy); 

	 // // Place the handle on the clipboard. 
		//	SetClipboardData(CF_TEXT, hglbCopy); 
		//	CloseClipboard(); 
		//	delete [] txt;
			break;
      }

      case(WM_LBUTTONDOWN):
      {
         if(gDebug.enabled == true) // Change current GUI window if we are not debugging
            break;
         int r;
         if (gUsingWine) // In Wine the double call messes up selections
         {
            r = CallWindowProc(OldCLIProc, hWnd, messg, wParam, lParam);    
         }
         else
         {
            r = CallWindowProc(OldCLIProc, hWnd, messg, wParam, lParam);  // Call twice to ensure insertion point
            CallWindowProc(OldCLIProc, hWnd, messg, wParam, lParam);      // is correct after changing windows
         }
         GetEditSelection(hWnd,pos,pos);	
         UpDateCLISyntax(win,hWnd,pos,pos);
         return(r);
      }

      case(WM_CHAR):
      {
      // Get number of lines in CLI
		   long line = SendMessage(hWnd,EM_GETLINECOUNT,0,0);
	   // Get character index for start of this line
		   long lineStart = SendMessage(hWnd,EM_LINEINDEX,line-1,0);
		   long start,end;
	   // Get position of insertion point
	      GetEditSelection(hWnd,start,end);
		// Get length of this line
		   length = SendMessage(hWnd,EM_LINELENGTH,lineStart,0);
      // Ignore copy commands (since we don't want the selection to be modified)
		   if(key == (char)COPY) 
         {
            break;
         }
      // Check for invalid start or end position for selection
         if((start < lineStart + 2) || (end < lineStart + 2))
         {

         // Cutting when the edit selection is outside the current line is not allowed
		      if(key == (char)CUT) 
            {
               cliHistory = 0;
         	   return(0);
            }

            pos = lineStart + length;


         // Make sure the selection region doesn't include text from previous lines
            if(end > lineStart + 2)
               SetEditSelection(hWnd,lineStart+2,end);
            else 
               SetEditSelection(hWnd,pos,pos);

         // Changing the edit selection breaks the paste call - so do it again
		      if(key == (char)PASTE) 
            {
           	   SendMessage(hWnd, WM_PASTE,(WPARAM)0, (LPARAM)0);
               cliHistory = 0;
		         break;
            }
         }
             
		// Escape detected, so abort command 		 
		   if(key == (char)VK_ESCAPE) 
		   {
            ReplaceEditText(hWnd,lineStart+2,lineStart+length,"");
            cliHistory = 0;
		      return(0);
		   }

         if(key == (char)PASTE) 
         {
           	SendMessage(hWnd, WM_PASTE,(WPARAM)0, (LPARAM)0);
            return(0);
         }
		   
		// Display character and then apply syntax colouring to line
         CallWindowProc(OldCLIProc,hWnd,messg,wParam,lParam);		   
	 //     UpdateLineColor(hWnd,GetCurrentLineNumber(hWnd)); 
      // Update syntax reporting in the status window
         GetEditSelection(hWnd,start,end);
         UpDateCLISyntax(win,hWnd,start,end);	       	      
         return(0);
      } 

      case(WM_KEYDOWN):
      {
		   long line = SendMessage(hWnd,EM_GETLINECOUNT,0,0);
		   long lineStart = SendMessage(hWnd,EM_LINEINDEX,line-1,0);
		   long selStart,selEnd;
		   GetEditSelection(hWnd,selStart,selEnd);	   
			length = SendMessage(hWnd,EM_LINELENGTH,lineStart,0);

         switch(wParam)
         {
            case('V'):
            {
               if(IsKeyDown(VK_CONTROL))
               {    
                  return(0); // Don't let the system paste - we've just done it
               }
               break;
            }

         // Ensure that Home and Shift-Home don't include prompt
            case(VK_HOME):
            {
               if(!IsKeyDown(VK_SHIFT))
               {
                  SetEditSelection(hWnd,lineStart+2,lineStart+2);	
                  return(0);
               }
            }

         // Show or hide gui windows
            case(VK_F2):
            {
               if(AnyGUIWindowVisible())
                  HideGUIWindows();
               else
                  ShowGUIWindows(SW_SHOWNOACTIVATE);
               break;
            }

          // Restrict deletes to the current line ******
            case(VK_BACK):
            {
               if(selStart < lineStart + 2 && selEnd <= lineStart)
               { // Selection is before insertion point so move to end
                  pos = lineStart + length;
                  SetEditSelection(hWnd,pos,pos);	
                  cliHistory = 0;
                  return(0);
               }
               if(selStart <= lineStart + 2 && (selEnd == lineStart+1 || selEnd == lineStart+2))
               { // Selection is before and ends at the prompt so set to start
                  SetEditSelection(hWnd,lineStart+2,lineStart+2);	
                  cliHistory = 0;
                  return(0);
               }
               if(selStart < lineStart + 2 && selEnd > lineStart + 2)
               { // Selection is before and ends after the prompt so remove bit before prompt
		   	      SetEditSelection(hWnd,lineStart+2,selEnd);
               }
               cliHistory = 0;
               break;
            }
            case(VK_DELETE):
            {
               if(selStart < lineStart+2 && selEnd < lineStart+2)
               {
                  pos = lineStart + length;
                  SetEditSelection(hWnd,pos,pos);			         
               }
               if(selStart < lineStart+2 && selEnd > lineStart+2)
               {
		   	      SetEditSelection(hWnd,lineStart+2,selEnd);
               }
               if(selStart == lineStart+1 && selEnd > lineStart+2)
               {
		   	      SetEditSelection(hWnd,lineStart+2,selEnd);
               }
               cliHistory = 0;
		         break;
            }
	 
         // Process entered text (i.e. text ending in <enter>) ****************
		      case(VK_RETURN): 
		      {	    
               char *command,*buf;
		         cliHistory = 0; // Reset previous command counter
               buf = new char[length+5]; // Allow for null character and initial 4 byte length
		         *(int*)buf = length; // buffer size
		      // Extract last line (i.e. the active one) ignoring line-feed
		         length = SendMessage(hWnd,EM_GETLINE,line-1,(LPARAM)buf);
		         long nChar = SendMessage(hWnd,WM_GETTEXTLENGTH,0,0);
		         SetEditSelection(hWnd,nChar,nChar);
		         ColorSelectedText(hWnd,RGB(0,0,0));
		         buf[length] = '\0';
		      // Ignore prompt '> '
               command = buf+2;
            // Place insertion point at the end of the line
               line = SendMessage(hWnd,EM_GETLINECOUNT,0,0);
               lineStart = SendMessage(hWnd,EM_LINEINDEX,line-1,0);
		   	   long end = SendMessage(hWnd,EM_LINELENGTH,lineStart,0) + lineStart;
			      SetEditSelection(hWnd,end,end);

		      // Process the command string extracted from the CLI
             //  if(macroDepth == 0 && !dialogDisplayed)

               if(obj && obj->debug)
               {
                  gDebug.cmd = command;
                  gDebug.step = true;
                  gDebug.mode = "printExpression";
               }
               else
               {
                  Interface *itfc = new Interface;
                  itfc->varScope = GLOBAL;
                  itfc->name = "CLI";
                  itfc->inCLI = true;
                  itfc->win = win;
              /*    if(itfc->win) // Make sure caching is turned off
                     itfc->win->cacheProc = false; */
 		            ProcessMacroStr(itfc,1,command);
                  delete itfc;
               }
               //{ // Code to test for memory leaks
               //   VLDEnable();
               //   Interface *itfc = new Interface;
               //   itfc->varScope = GLOBAL;
               //   itfc->name = "CLI";
               //   itfc->inCLI = true;
               //   itfc->win = win;
 		            //ProcessMacroStr(itfc,1,command);
               //   delete itfc;
               //   procRunList.RemoveAll();
               //   procLoadList.RemoveAll();
               //   VLDDisable();
               //   exit(0);
               //}
		         delete [] buf; 
		         return(0);		      
		      }
		 
		   // Check for left arrow pressed	   
            case(VK_LEFT):
            {
         	   if(selEnd == lineStart + 2) // Ignore if at start of line
			   	   return(0);
			      if(selEnd < lineStart + 2) // Reset if out-of-bounds
			      {
			   	   long pos = lineStart + length;
			   	   SetEditSelection(hWnd,pos,pos);	
 
		            return(0);
			      }
			   	GetEditSelection(hWnd,pos,pos);	
               UpDateCLISyntax(win,hWnd,pos,pos);	
			      break;	
            }
			   
		   // Check for right arrow pressed				   		   	
			   case(VK_RIGHT):
            {
			      if(selEnd < lineStart + 2) // Reset if out-of-bounds
			      {
			   	   pos = lineStart + length;
			   	   SetEditSelection(hWnd,pos,pos);	 
		            return(0);
			      }
               GetEditSelection(hWnd,pos,pos);	
               UpDateCLISyntax(win,hWnd,pos,pos);
			      break;
            }
			   
         // Up arrow, so display previous command				   			
			   case(VK_UP):
            {
               char *backLine = NULL;
             // Reset if cursor is out-of-bounds
			      if(selEnd < lineStart + 2)  
			      {
			   	   pos = lineStart + length;
			   	   SetEditSelection(hWnd,pos,pos);	
		            return(0);
			      }
            // Extract the current line as a reference
               if(cliHistory == 0 || !curLine)
               {
                  if(curLine) delete [] curLine;
                  curLine = GetLineByNumber(hWnd,line-1);
               }  			
            // Go back one line
               long cnt = 0;
               while(true)
               {
                  cliHistory++; 
                  cnt++;

               // Prevent going beyond start of CLI by looping to line above current line
                  if(line-cliHistory <= 0) 
                     cliHistory = 1; 

               // Check we don't just have one line
                  if(line-cliHistory-1 < 0)
                  {
                     if(backLine) delete [] backLine;
                     return(0);
                  }

               // Get the line specified by cliHistory
                  if(backLine) delete [] backLine;
                  assert(line-cliHistory-1 >= 0);
                  backLine = GetLineByNumber(hWnd,line-cliHistory-1);

               // Matching line not found
                  if(cnt > line)
                  {
                     MessageBeep(MB_ICONASTERISK);
                     if(backLine) delete [] backLine;
                     return(0);
                  }

               // Not a command line so ignore
		            if(backLine[0] != '>' && cliHistory <= line) 
                     continue;

               // No command on this line just a prompt so ignore
                  if(!strcmp(backLine,"> ") && cliHistory <= line) 
                     continue;

               // Compare backline with current line if current line includes part of a command
                  if(strlen(curLine) > 2)
                  {
                     if(strncmp(backLine,curLine,strlen(curLine)))
                        continue; // No match so ignore
                  }

                  break;
               }

            // Replace current line with backline
		   	   length = SendMessage(hWnd,EM_LINELENGTH,lineStart,0);
		   	   ReplaceEditText(hWnd,lineStart,lineStart+length,backLine);
	        // Tidy up
               if(backLine) delete [] backLine;
				   return(0);	
            }
	   
         // Down arrow so display previous command			   			
			   case(VK_DOWN):			   			
            {
               char *backLine = NULL;
             // Reset if cursor is out-of-bounds
			      if(selEnd < lineStart + 2) 
			      {
			   	   pos = lineStart + length;
			   	   SetEditSelection(hWnd,pos,pos);	
		            return(0);
			      }
            // Extract the current line as a reference
               if(cliHistory == 0 || !curLine)
               {
                  if(curLine) delete [] curLine;
                  curLine = GetLineByNumber(hWnd,line-1);
               }  			
            // Go back one line
               long cnt = 0;
               while(true)
               {
                  cliHistory--; 
                  cnt++;

               // Prevent going beyond end of CLI by looping to top line
                  if(cliHistory < 1)
                     cliHistory = line-1; 

                // Check we don't just have one line
                  if(line-cliHistory-1 < 0)
                  {
                     if(backLine) delete [] backLine;
                     return(0);
                  }

               // Get the line specified by cliHistory
                  if(backLine) delete [] backLine;
                  assert(line-cliHistory-1 >= 0);
                  backLine = GetLineByNumber(hWnd,line-cliHistory-1);

              // Matching line not found
                  if(cnt > line)
                  {
                     MessageBeep(MB_ICONASTERISK);
                     if(backLine) delete [] backLine;
                     return(0);
                  }
               // Not a command line so ignore
		            if(backLine[0] != '>' && cliHistory > 0) 
                     continue;

               // No command on this line just a prompt so ignore
                  if(!strcmp(backLine,"> ") && cliHistory > 0)
                     continue;

               // Compare backline with current line if current line includes part of a command
                  if(strlen(curLine) > 2)
                  {
                     if(strncmp(backLine,curLine,strlen(curLine)))
                        continue; // No match so ignore
                  }
                  break;
               }
            // Replace current line with backline
		   	   length = SendMessage(hWnd,EM_LINELENGTH,lineStart,0);
		   	   ReplaceEditText(hWnd,lineStart,lineStart+length,backLine);
	        // Tidy up
               if(backLine) delete [] backLine;
               return(0);
            }
   		   
		      // User has asked for help  
            case(VK_F1): 
            {
               Interface itfc;
               GetCommandHelp(&itfc,hWnd);
               break;
            }

            default:
            {
               cliHistory = 0;
               break;
            }
         }
       
       // Ignore all control keys except copy/cut/paste/undo/redo  
         if(IsKeyDown(VK_CONTROL))
         {
            if(wParam == 'X' && ((selStart < lineStart + 2) && (selEnd < lineStart + 2)))
            {
		         return(0); // Invalid selection range for cut
            }

            if(wParam == 'X' && ((selStart < lineStart + 2) && (selEnd > lineStart + 2)))
            {
               SetEditSelection(hWnd,lineStart+2,selEnd);
               break;
            }

            if(wParam != 'C' && wParam != 'X' && wParam != 'V' && wParam != 'Z' && wParam != 'Y')
               return(0);

            if(wParam == 'V' && ((selStart < lineStart + 2) && (selEnd < lineStart + 2)))
            {
		         return(0); // Invalid selection range for paste
            }

            if(wParam == 'V' && ((selStart < lineStart + 2) && (selEnd > lineStart + 2)))
            {
               SetEditSelection(hWnd,lineStart+2,selEnd);
               break;
            }
         }  
         break;  
		}	
	
	}
	
	return(CallWindowProc(OldCLIProc,hWnd,messg,wParam,lParam));
}	

/******************************************************************************
  Send text to CLI or print file
******************************************************************************/

void SendTextToCLI(char *message)
{
   RECT r;
   
	if(printFile) // Send text to a file, replacing \n with \r\n
	{
      long len = strlen(message);
      for(long i = 0; i < len; i++)
      {
         if(message[i] == '\n')
            fputc('\r',printFile);
         fputc(message[i],printFile);
      }
	}
   else if (printToString)
   {
      printString.Concat(message);
   }
   else if(cliEditWin) // Send text to CLI window
   {
	   
   // Get character index of beginning of last line
	//   long lineStart = SendMessage(cliEditWin,EM_LINEINDEX,-1,0);
	      long lastLine = SendMessage(cliEditWin,EM_GETLINECOUNT,0,0);
	      long lineStart = SendMessage(cliEditWin,EM_LINEINDEX,lastLine-1,0);
	// Get character index of line end
		long endLine = SendMessage(cliEditWin,EM_LINELENGTH,lineStart,0) + lineStart; 
	// Get number of characters in editor
	   long length = SendMessage(cliEditWin,WM_GETTEXTLENGTH,0,0);
	
	// If last character of message a return then replace last line with new text     
		if(message[strlen(message)-1] == '\r')
		{
			message[strlen(message)-1] = '\0';
         SendMessage(cliEditWin,EM_GETFIRSTVISIBLELINE ,0,0); // Find first visible line
	      SendMessage(cliEditWin, WM_SETREDRAW, false, 0);		
		   SetEditSelection(cliEditWin,lineStart+2,endLine+2);
		//	CutEditSelection(cliEditWin); // Not good as it copies to clipboard
		   ReplaceEditText(cliEditWin,lineStart+2,endLine+2,message);	
	      SendMessage(cliEditWin, WM_SETREDRAW, true, 0);
	      DWORD coord = SendMessage(cliEditWin,EM_POSFROMCHAR,(WPARAM)lineStart,0L);
	      GetClientRect(cliEditWin,&r);
	      r.top = HIWORD(coord);
	      r.bottom = r.top + 16; // Line height is 16
	      MyInvalidateRect(cliEditWin,&r,true); // Need to just select this line
	  //    MyInvalidateRect(cliEditWin,NULL,true); // Need to just select this line
	      MyUpdateWindow(cliEditWin);
	      ShowCaret(cliEditWin);	      	      		   	   
		}
	// Otherwise append the message to the end of the CLI text
		else
		{
	      long lastLine = SendMessage(cliEditWin,EM_GETLINECOUNT,0,0);
	      lineStart = SendMessage(cliEditWin,EM_LINEINDEX,lastLine-1,0);
	   	long endTxt = SendMessage(cliEditWin,EM_LINELENGTH,lineStart,0) + lineStart;

	   // If total text (old + new) is smaller than maximum just add it to end
	      if((endTxt + strlen(message)) < MAX_TEXT)
	      {
		      ReplaceEditText(cliEditWin,endTxt,endTxt,message);	      
			}
		// Otherwise give a warning - delete the old text and then add the new
			else 
			{
			   MessageDialog(prospaWin,MB_ICONWARNING,"Warning","Text buffer overflow\rPress OK then escape to abort");
		      ReplaceEditText(cliEditWin,0L,length+1,message);	      
	      } 
	      length = SendMessage(cliEditWin,WM_GETTEXTLENGTH,0,0);
		   SetEditSelection(cliEditWin,length,length);	// Place caret at end of line
      
		// Make sure the printed text is visible by scrolling the text into view
         short curLine = SendMessage(cliEditWin,EM_EXLINEFROMCHAR ,0,(LPARAM)length); // Find current line
         short first = SendMessage(cliEditWin,EM_GETFIRSTVISIBLELINE ,0,0); // Find first visible line
         RECT r;
         SendMessage(cliEditWin,EM_GETRECT,0,(LPARAM)&r);   // Get size of visible edit window in pixels
         short last = (r.bottom-r.top)/14 + first; // Work out last visible line (assume font is 14 point high)
         if(curLine > last) // Scroll if below bottom of window
         {
            short linestoscroll = curLine-last;  // Work out lines to scroll			      	
            SendMessage(cliEditWin,EM_LINESCROLL,0,linestoscroll);  // Scroll lines	 
         }
      }
	   // Make sure the text is visible
//		   MyUpdateWindow(cliEditWin);	          
      SendMessage(cliEditWin,EM_HIDESELECTION ,false,0);
		   	 			
		messageSent = true;

      // Also print to Console if this is activated
      if (gIsConsole)
         printf("%s", message);
	}
   else
   {
      // Also print to Console if this is activated and no CLI is present
      if (gIsConsole)
         printf("%s", message);
   }
 


}

/******************************************************************************
  Send text to a print file after running this command
  options: overwrite (write a new file - the default)
           append (append to existing file)
******************************************************************************/

int PrintToFile(Interface* itfc, char args[])
{
   short r;
   CText fileName, option = "overwrite";

   // Get filename
   if ((r = ArgScan(itfc, args, 1, "file name[, option]", "ee", "tt", &fileName, &option)) < 0)
      return(r);

   // Open file
   if (option == "overwrite")
      printFile = fopen(fileName.Str(), "wb");
   else if (option == "append")
   {
      printFile = fopen(fileName.Str(), "ab");
      if (!printFile)
         printFile = fopen(fileName.Str(), "wb");
   }
   else
   {
      ErrorMessage("Valid options: (overwrite/append)");
      return(ERR);
   }

   if (!printFile)
   {
      ErrorMessage("can't write to 'printto' file '%s'", fileName.Str());
      return(ERR);
   }
   itfc->nrRetValues = 0;
   return(OK);
}


/******************************************************************************
  Send print text to a string after running this command
******************************************************************************/

int PrintToString(Interface* itfc, char args[])
{
   short r;

   // Check to see if we are printing to a file
   if (printFile)
   {
      ErrorMessage("printtostring call inside printtofile");
      fclose(printFile);
      printFile = NULL;
      return(ERR);
   }


   // Initialize local string variable and indicate we are printing to a string
   printToString = true;
   printString = "";
   itfc->nrRetValues = 0;
   return(OK);
}

/******************************************************************************
  Stop sending text to a print file and instead send it to the CLI
******************************************************************************/

int ClosePrintFile(Interface* itfc, char args[])
{
   if (printFile)
   {
      fclose(printFile);
      printFile = NULL;
   }
   else if (printToString)
   {
      printToString = false;
      itfc->nrRetValues = 1;
      itfc->retVar[1].MakeAndSetString(printString.Str());
   }
   else
   {
      ErrorMessage("'printto' file not open");
      return(ERR);
   }   
   return(OK);
}



int SetOrGetCurrentCLI(Interface *itfc, char args[])
{
   int nrArgs;
   ObjectData *obj;
 	CArg carg;
      
   if(!cliEditWin)
	{
		if(itfc->inCLI)
		{
			ErrorMessage("No CLI defined"); // Shouldn't happen!
			itfc->nrRetValues = 0;
			return(ERR);
		}
		else
		{
			itfc->retVar[1].MakeNullVar();
			itfc->nrRetValues = 1;
			return(OK);
		}
	}
      
   nrArgs = carg.Count(args);

   obj = GetObjData(cliEditWin); 

   if(nrArgs == 0)
	{
		itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)obj);
      itfc->nrRetValues = 1;
      return(OK);
	}
	else
	{
		ErrorMessage("Unsupported option");
		return(ERR);
	}
}

/********************************************************
*    Functions specific to the CLI interface
********************************************************/

int CLIFunctions(Interface* itfc ,char args[])
{
   CText func;
   short r;

   if((r = ArgScan(itfc,args,0,"command","e","t",&func)) < 0)
      return(r);

   if(!cliEditWin)
      return(OK);

   if(func == "paste")
   {
		SendMessage(cliEditWin,WM_PASTE,(WPARAM)0,(LPARAM)0);
   }
   else if(func == "copy")
   {
		SendMessage(cliEditWin,WM_COPY,(WPARAM)0,(LPARAM)0);
   }
   else if(func == "cut")
   {
		SendMessage(cliEditWin,WM_CUT,(WPARAM)0,(LPARAM)0);
   }
   else if(func == "undo")
   {
		SendMessage(cliEditWin,WM_UNDO,(WPARAM)0,(LPARAM)0);
   }
   else if(func == "command help")
   {
		GetCommandHelp(itfc,cliEditWin);
   }
   return(OK);
}

/***************************************************************************  
Send a message (with standard arguments) to the CLI from an internal function.  
Replace certain characters first so that formating is correct
e.g. <lf> will move insertion point to start of a new line
***************************************************************************/

EXPORT void TextMessage(const char *const text,...)
{
	va_list ap;
	char *output; 
	char replace[2];
	va_start(ap,text);

   output = vssprintf(text,ap);
	replace[0] = (char)0xB1; replace[1] = '\0';
	ReplaceSpecialCharacters(output,"+-",replace,-1);
	SendTextToCLI(output); 
	va_end(ap);
	delete [] output;
}

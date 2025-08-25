#include "stdafx.h"
#include "events_edit.h"
#include <vector>
#include "carg.h"
#include "cli_files.h"
#include "command_other.h"
#include "debug.h"
#include "defineWindows.h"
#include "dialog_utilities.h"
#include "dividers.h"
#include "edit_class.h"
#include "edit_dialogs.h"
#include "edit_files.h"
#include "edit_utilities.h"
#include "files.h"
#include "font.h"
#include "globals.h"
#include "guiObjectClass.h"
#include "guiMakeObjects.h"
#include "guiModifyObjectParameters.h"
#include "guiWindowClass.h"
#include "guiWindowCLI.h"
#include "guiWindowsEvents.h"
#include "import_export_utilities.h"
#include "interface.h"
#include "main.h"
#include "message.h"
#include "mytime.h"
#include "plot1dCLI.h"
#include "plot2dCLI.h"
#include "plot.h"
#include "PlotFile.h"
#include "print.h"
#include "process.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "string_utilities.h"
#include "syntax_viewer.h"
#include "variablesOther.h"
#include "WidgetRegistry.h"
#include <RICHEDIT.h>
#include "memoryLeak.h"

using std::vector;

#pragma warning (disable: 4311) // Ignore pointer truncation warnings
#pragma warning (disable: 4996) // Ignore deprecated library functions

int SetOrGetCurrentEditor(Interface *itfc, char args[]);

bool gUsingWine = false;

/***********************************************************************
   Event procedures for the edit window and associated functions.
   
   The edit window can contain a matrix of independent sub-windows each
   of which can contain text. Each subwindow is represented by an instance
   of the EditRegion class. These are packaged together in an array called
   ep->editData.
   
   The edit window features syntax coloring, find and replace, procedure
   lookups and comprehensive undo. It was necessary to inhibit a number 
   of the standard richedit control feature to allow this to work correctly.
   
   Routines relating to the Text Editor
 
   EditFunctions ....... editfunc command arguments to access edit functions: 
 
      block comment
      block uncomment
      close all macros
      close macro
      command help
      copy
      cut
      decrement font size
      find and replace
      find and replace
      find down
      find up
      go back
      go forward
      go to line
      go to procedure
      increment font size
      indent text
      multiedit 1*1
      multiedit 1*2
      multiedit 2*1
      multiedit 2*2
      multiedit 3*1
      open macro
      paste
      print macro
      run text
      save and run text
      save macro
      save macro as
      select all
      show fault
      toggle path
      undo
      unindent text

***********************************************************************/

#define COPY	3
#define PASTE	22
#define UNDO	26
#define CUT		24
#define ENTER  13

#define NORMAL_CURSOR     0 // Normal edit cursor mode
#define HORIZONTAL_CURSOR 1 // Horizontal divider cursor mode
#define VERTICAL_CURSOR   2 // Vertical divider cursor mode

#define DIVIDER_MOVE  0 // User is moving the subedit divider
#define DIVIDER_CLICK 1 // User has clicked on the subedit divider

#define STATUS_LINE_NR_WIDTH 50 // Width of second part of edit window status bar

#define MAX_MENU_MACS 200 // Maximum number of macros in pop-up menus (needs 3 extra)

// Locally define and used functions
static void  ProcessCLIMenuCommands(HWND, HDC, WPARAM, LPARAM);
static void  RemoveAllButCurrentText(EditParent*);

void  FreeEditMemory(EditParent*);
static void  TrackEditDivider(ObjectData *obj, EditParent *ep, HWND hWnd);
static void  MoveDivider(ObjectData *obj, EditParent *ep, HWND hwnd, short x, short y, short &oldx, short &oldy);
static void  ResizeSubEditors(ObjectData *obj, EditParent *ep, HWND hwnd, short x, short y);
static short SubEditDividerEvents(ObjectData *obj, EditParent *ep, HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, short mode);
static BOOL  CALLBACK FindReplaceProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void ResizeEditSubwindows(EditParent *ep);
void ShowError(EditParent *ep, HWND hWnd);
short LoadEditDialog(HWND, char*, char*);
short FindEmptyEditor(EditRegion *curRegion);

// Locally defined functions but with global access
void  SetEditTitle();


static bool showFullName     = false;     // Display full pathname in editor title?
static bool mouseLeftClicked = false;     // Flag to prevent drag-drop mode from working
static bool editActivated    = false;     // Flag to report when another window has taken focus
static short cursor_mode = NORMAL_CURSOR; // Controls which cursor is being displayed
static short lastEditor = -1;             // Last editor index before window changed
static HWND findReplaceHwnd;              // Find and replace window

EditRegion** origEditArray;
EditRegion* oldEditor;
EditRegion* curEditor; // Currently selected editor region

int EditorFunctions(Interface* itfc ,char args[]);

bool inEditDivider = false; // Used to prevent cursor flickering when hovering over editor divider


/***********************************************************************
   Callback procedure for the  edit windows                          
   A lot of code here is used to disable features in the standard
   richedit control which would interfer with the normal operation
   of the editor. In addition, to achieve syntax colouring it was
   necessary to inhibit the normal undo mechanism. (Without this undos
   try and undo the colouring!).
   
   This handles the following messages:
   
   1. Keyboard entries.
   2. Mouse movement and mouse clicking in client and nonclient regions.
   3. Setting of correct cursor as mouse moves around the window.
   
   Events:
  
     WM_MOUSEACTIVATE ... cli selected
     WM_SETFOCUS ........ user has clicked on one editor
     WM_NCMOUSEMOVE ..... divider has been moved
     WM_NCLBUTTONDOWN ... divider has been selected
     WM_LBUTTONDBLCLK ... word has been selected
     WM_LBUTTONDOWN ..... processes
     WM_LBUTTONUP
     WM_MOUSEMOVE
     WM_RBUTTONDOWN
     WM_ERASEBKGND ...... prevent flicker TODO
     WM_CHAR ............ processed key information
     WM_KEYDOWN ......... raw key information

***********************************************************************/

LRESULT CALLBACK EditEventsProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{
   long r;
   long startSel,endSel;
   static long curClickLineNr;
   static double clickTimer;
   static short clickCounter = 0;
   const double tripleClickTime = 500.0;
   ObjectData* obj = NULL;
   short curEdNr = -1;
   EditParent* ep = NULL;

// Find window and object receiving events
   HWND parWin = GetParent(hWnd);
   WinData* win = rootWin->FindWinByHWND(parWin);
   if(!win)
   {
      HWND parWin2 = GetParent(parWin);
      win = GetWinDataClass(parWin2);
   }
   if(win)
   {
      obj = win->widgets.findCurEditor(hWnd,&ep,&curEdNr); // TODO - embed WinInfo in edit sub windows
   }

	switch(messg)
	{

      case(WM_DROPFILES): // User has clicked on window - update title, menu and toolbar
      {
         CText path,file,ext;
         if(GetDropFileInfo((HDROP)wParam, path, file, ext, 0) == OK)
         {
            DragFinish((HDROP)wParam);
      
         // Is it a folder?
            if(file == "")
            {
               PlotFile::setCurrMacroDirectory(path.Str());
               TextMessage("\n\n  pathnames(\"macrodata\") = %s\n\n> ",path.Str());
               return(0);
            }

            if(ep && (ext == "mac") || (ext == "txt") || (ext == "m") ||
               (ext == "pex") || (ext == "par") || (ext == "lst") || (ext == "asm"))
            {
               curEditor = ep->editData[curEdNr];
			   	win->currentEditor = curEditor;

            // Check to see if current text needs saving 
               if(curEditor->CheckForUnsavedEdits(ep->curRegion) == IDCANCEL)
                  return(0);
               LoadEditorGivenPath(path.Str(), file.Str());   
               AddFilenameToList(procLoadList,curEditor->edPath, curEditor->edName);

              SetForegroundWindow(prospaWin);
              SetFocus(hWnd);
            }
         }
         return(0);
      }

      case(WM_PAINT):
      {
         CallWindowProc(fnOldEdit,hWnd,messg,wParam,lParam);
		   obj->decorate(parWin);
         if(ep)
         {
            for(int i = 0; i < ep->rows*ep->cols; i++)
            {
               if(ep->editData[i] && ep->editData[i]->debug)
               {
                  DrawDebugBreakPointStrip(ep->editData[i]);
               }
            }
         }

         return(0);
      }

   // Draw the window to a specific device context
		case(WM_PRINTCLIENT): 
		{
         SendMessage(hWnd, WM_PAINT, wParam, lParam);
         break;
      }

      case(WM_MOUSEACTIVATE): // User has clicked on window - update title, menu and toolbar
      {
         if(gDebug.mode != "off") // Change current GUI window if we are not debugging
            break;
         if(obj)
         {
            SelectFixedControls(win, obj);
   //         SetWindowText(win->hWnd,"Prospa - Editor");
            if(win && !win->keepInFront)
               ChangeGUIParent(GetParent(hWnd));
            SetFocus(hWnd);
         }
         break;
      }



// Detect when the mouse moves into edit dividing bar
	   case(WM_NCMOUSEMOVE):
	   {
         if(obj && ep)
         {
	      //   if(macroDepth == 0 && ( ep->rows > 1 || ep->cols > 1))
	         if(ep->rows > 1 || ep->cols > 1)
            {
               SubEditDividerEvents(obj, ep, hWnd, messg, wParam, lParam, DIVIDER_MOVE);
            }
         }
         break;
      } 

// Detect when the users clicks on the edit dividing bar
	   case(WM_NCLBUTTONDOWN):
	   {
         if(obj && ep)
         {
	       //  if(macroDepth == 0 && (ep->rows > 1 || ep->cols > 1))
	         if(ep->rows > 1 || ep->cols > 1)
            {
                SubEditDividerEvents(obj, ep, hWnd, messg, wParam, lParam, DIVIDER_CLICK);
            }
         }
         break;
      }      
               	
// User has typed a character (comes before WM_CHAR) ************************************************
      case(WM_KEYDOWN):
      {
         unsigned char key = (unsigned char)wParam;

			//if(key < 0) // Ignore keys > 128
			//	 return(0); 

	  // Ignore all keystrokes when mouse button is held down 
         if(IsKeyDown(VK_LBUTTON))
            return(0); 
     // Ignore delete backwards
         if(key == VK_BACK && IsKeyDown(VK_CONTROL))
            return(0);        
     // Ignore insert button
         else if(key == VK_INSERT)
         {    
            return(0);
         }

      // Show or hide gui windows
         else if(key == VK_F2)
         {
            if(AnyGUIWindowVisible())
               HideGUIWindows();
            else
               ShowGUIWindows(SW_SHOWNOACTIVATE);

         }

     // User has pressed backspace - save deleted text to undo buffer       
         else if(key == VK_BACK)
         {
            if(curEditor->readOnly) 
               return(0);
	         GetEditSelection(curEditor->edWin,startSel,endSel);
            if(startSel != endSel) // A selection of characters are being backspaced over
            {
               curEditor->CopySelectionToUndo(CUT_TEXT,0);
		         curEditor->initCharInsertionPnt = startSel;
		      }	               
            else // One character is being backspaced over
            {
               SetEditSelection(curEditor->edWin,startSel-1,endSel);           
               curEditor->CopySelectionToUndo(CUT_TEXT,0);
		         curEditor->initCharInsertionPnt = startSel-1;	               
            }            
         }              
      // User has asked for help so search for help for the selected word      
         else if(key == VK_F1) 
         {
            Interface itfc;
            GetCommandHelp(&itfc,curEditor->edWin);
            break;
         }             
           
      // Process any control key inputs   
         if(IsKeyDown(VK_CONTROL))
         {    
         // Intercept cut command     
            if(key == 'X')
            {
               if(!curEditor->readOnly) // (2.2.6)
               {
                  curEditor->CopySelectionToUndo(CUT_TEXT,0);
                  curEditor->Cut(); 
               }
               return(0); // Don't let the system cut - we've just done it            
            } 
         // Intercept paste command           
            else if(key == 'V')
            {
               if(!curEditor->readOnly) // (2.2.6)
               {
                  curEditor->CopySelectionToUndo(PASTE_TEXT,0);
                  curEditor->Paste();
               }
               return(0); // Don't let the system paste - we've just done it
            } 
         // Intercept undo command                           
            else if(key == 'Z')
            { 
               if(!curEditor->readOnly) // (2.2.6)
               {
                  curEditor->CopyUndoToSelection();
                  if(curEditor->edModified && curEditor->undoIndex == 0) // All undos done!
                  {
                     curEditor->edModified = false;   
                     SetEditTitle();
  	               }  
               }
               return(0); // Don't let the system undo - we've just done it
            }
        // Ignore changes to line spacing, justification, super script or tabs
            else if(key == '2' || key == '5' || key == 'R' || key == 'E' ||
                  key == VK_OEM_PLUS || key == VK_TAB)
               return(0);
         }
        
       // Ignore subscript, capitalization and bulleting 
         if(IsKeyDown(VK_CONTROL | VK_SHIFT))
         {
            if(key == VK_OEM_PLUS  || key == 'A' || key == 'L')
              return(0);
         }
                                                                                                                                
      // Intercept delete command (note not detected by WM_CHAR)          
         if(key == VK_DELETE) 
         {
            if(curEditor->readOnly) // (2.2.6)
               return(0);
	         GetEditSelection(curEditor->edWin,startSel,endSel);        
            if(startSel != endSel) // A selection of characters are being deleted
            {         
               curEditor->CopySelectionToUndo(CUT_TEXT,0); 
               SendMessage(curEditor->edWin,EM_REPLACESEL,true,(LPARAM)(LPCTSTR)"");
               if(curEditor->edParent->showSyntaxColoring)
                  UpdateLineColor(curEditor,curEditor->edWin,GetCurrentLineNumber(curEditor->edWin));
 		         curEditor->initCharInsertionPnt = startSel;	      
               if(!curEditor->edModified)
               {
                  curEditor->edModified = true;
                  //SendMessageToGUI("Editor,Modified",-1); 
                  SetEditTitle(); 
               }
               return(0);             
            }
            else // One forward character is being deleted - need to change selection range
            {
               SetEditSelection(curEditor->edWin,startSel,endSel+1);
               curEditor->CopySelectionToUndo(CUT_TEXT,0);               
 		         curEditor->initCharInsertionPnt = startSel;	           
               if(!curEditor->edModified)
               {
                  curEditor->edModified = true;
                  //SendMessageToGUI("Editor,Modified",-1); 
                  SetEditTitle(); 
               }  
            }                      
         } 

         // Get the current text selection  
         GetEditSelection(curEditor->edWin,startSel,endSel);       
         // Enter is processed before other characters so record characters to
         // to be replaced by enter (if any) or insertion point for undo later.
         if(key == ENTER)
         {
            if(curEditor->readOnly) // (2.2.6)
               return(0);

            if(startSel != endSel)
            {
               curEditor->CopySelectionToUndo(REPLACE_TEXT_BY_CHAR,0);
               curEditor->initCharInsertionPnt = startSel+1;
            }	               
            else
            {
               curEditor->CopySelectionToUndo(ENTER_RETURN,0); 
            }
         } 

       // Process those key strokes not ignored   
         r = CallWindowProc(fnOldEdit,hWnd,messg,wParam,lParam);
   
       // Recolor line after text deleted (not necessary for other keys as this is done in WM_CHAR)
         if(key == VK_DELETE)
         {
            if(curEditor->edParent->showSyntaxColoring)
               UpdateLineColor(curEditor, curEditor->edWin,GetCurrentLineNumber(curEditor->edWin));
         }
 
       // When changing cursor position with arrow keys record new insertion point for undo   
         if(key == VK_LEFT || key == VK_RIGHT || key == VK_UP || key == VK_DOWN)
		   {
	         GetEditSelection(curEditor->edWin,startSel,endSel);		   
		      curEditor->initCharInsertionPnt = startSel;	
            if(curEditor->edParent->showSyntaxDescription)
               UpDateEditSyntax(win, curEditor,startSel,endSel);
	      }
                  
         return(r);
      }

	   case(WM_ERASEBKGND): // Prevent background being cleared before resize
	   {
	      return(1);
	   }
      
// User has typed a character (comes after WM_KEYDOWN)  **********************************************   	
      case(WM_CHAR): 
      { 
         unsigned char key = (char)wParam;

      // Escape if the copy key is pressed - this prevents subsequent commands from
      // moving the selected text 
         if(key == COPY)
            break;

      // Ignore all keystrokes when mouse button is held down      
         if(IsKeyDown(VK_LBUTTON))
            return(0);
                  
      // Note any changes to text by updating title with an asterisk               
         if(key == PASTE || key == CUT || !iscntrl(key) ||
            key == VK_RETURN || key == VK_TAB || key == VK_BACK ) // note any changes
         {

      // If read only ignore all editing keys
            if(curEditor->readOnly) // (2.2.6)
               return(0);

            if(!curEditor->edModified)
            {
               curEditor->edModified = true;
               //SendMessageToGUI("Editor,Modified",-1); 
               SetEditTitle(); 
            }
         }
   
      // Get the current text selection  
	      GetEditSelection(curEditor->edWin,startSel,endSel);       
 		// MS does something weird when you double click a space delimited word - it adds a space when
 		// you replace it. This next line stops this from happening
         SetEditSelection(curEditor->edWin,startSel,endSel);    
      // As each key is pressed update the replace text buffer and insertion point (ENTER already processed) 
         if(key != UNDO && key != PASTE && key != COPY && key != CUT && key != ENTER)
         {
            if(key != VK_BACK)
            {
               if(startSel != endSel)
               {
                  curEditor->CopySelectionToUndo(REPLACE_TEXT_BY_CHAR,0);
		            curEditor->initCharInsertionPnt = startSel+1;
		         }	               
               else
                  curEditor->CopySelectionToUndo(ENTER_TEXT,0);
            }            
         }           
      // Process key strokes (add to editor etc)   
         r = CallWindowProc(fnOldEdit,hWnd,messg,wParam,lParam);  

      // Update syntax coloring for this line
        if(curEditor->edParent->showSyntaxColoring)
            UpdateLineColor(curEditor, curEditor->edWin,GetCurrentLineNumber(curEditor->edWin));
      // Update syntax reporting
         if(curEditor->edParent->showSyntaxDescription)
            UpDateEditSyntax(win, curEditor,startSel,endSel);
         return(r);
      }  
           
// Double clicked on text - goto procedure if control key help down *******************************     
	   case(WM_LBUTTONDBLCLK): 
	   {
       //   if(gNumMacrosRunning > 0) // Change current GUI window if macro is not running
       //     break;
         if(curEditor->debug) // Ignore double click in debug strip
         {
            int xPos = LOWORD(lParam);  // horizontal position of cursor 
            int yPos = HIWORD(lParam);  // vertical position of cursor 
            RECT r;
            GetClientRect(curEditor->edWin,&r);
            int width = 25;
            int height = r.bottom;

            int lineHeight = curEditor->GetLineHeight();
            if(lineHeight == 0)
               return(0);

            if(xPos > 0 && xPos < width && yPos > 0 && yPos < height)
               return(0);
         }

	      if(wParam & MK_CONTROL) // Go to procedure
	      {
            Interface itfc;
		      HDC hdc = GetDC(hWnd);
			   SetCursor(LoadCursor(NULL,IDC_WAIT));
				if(wParam & MK_SHIFT) // Go to procedure
					EditorFunctions(&itfc,"\"go to procedure use new editor\"");
				else
					EditorFunctions(&itfc,"\"go to procedure\"");
				wParam = ID_GOTO_PROCEDURE; // ??
         // Determine which procedure the cursor is in and display it
		      long startSel,endSel;
	         GetEditSelection(curEditor->edWin,startSel,endSel);	
            curEditor->currentProc = ""; 
            curEditor->FindCurrentProcedure(endSel,curEditor->currentProc);
            SetEditTitle();
				SetCursor(LoadCursor(NULL,IDC_ARROW));
            ReleaseDC(hWnd,hdc);	
 		   } 
         else // Select words
         {
	         long startWord,endWord;      
      	   char *name;
	    	   bool classCmd=false;
				short userClassCmd=0;

	         GetEditSelection(curEditor->edWin,startWord,endWord);
            name = ExpandToFullWord(curEditor->edWin,BEFORE_OPERAND,AFTER_OPERAND,startWord,endWord,classCmd,userClassCmd);
				if(classCmd)
				{
					extern void ExtractClassProcName(char *objectName, char *procName);
					char procName[MAX_STR];
					ExtractClassProcName(name,procName);
					startWord = endWord-strlen(procName)+1;
				}
            SetEditSelection(curEditor->edWin,startWord,endWord+1);
            delete [] name;
            clickCounter++;
            return(0);
         }
	      break;
	   } 
   
// Select window focus by clicking on text ********************************************************            
		case(WM_LBUTTONDOWN):
		{
         if(gDebug.enabled && !win->debugger) // Change current GUI window if we are not debugging
            break;

         if(ep && curEdNr >= 0)
         {     
			   curEditor = ep->editData[curEdNr];  // Make this the current editor
				win->currentEditor = curEditor;
            ep->curRegion = curEdNr;
			   SetFocus(curEditor->edWin);         // Give it window focus		   
		      SetEditTitle();                     // Set the window title
		      lastEditor = curEdNr;               // Record the window number in case it changes		         
		   }	

         if(curEditor->debug)
         {
            int xPos = LOWORD(lParam);  // horizontal position of cursor 
            int yPos = HIWORD(lParam);  // vertical position of cursor 
            if(AddDebugBreakPoint(xPos, yPos, curEditor))
               return(0);
         }

     //  Update counter/timer for triple click detection
         clickCounter++;
         if(clickCounter != 1)
         {
            double elapsedTime = GetMsTime()- clickTimer;
            if(elapsedTime < tripleClickTime)
            {
               if(clickCounter == 3)
               {
                  long n = GetCurrentLineNumber(curEditor->edWin);
                  if(n == curClickLineNr)
                  {
                     SelectLine(curEditor->edWin, n);
                     clickCounter = 0;
                     return(0);
                  }
                  else
                  {
                     clickCounter = 1;
                  }
               }
            }
            else
            {
               clickCounter = 1;
            }
         }

      // Process this event so we can update the current location
         if (gUsingWine) // In Wine the double call messes up selections
         {
            r = CallWindowProc(fnOldEdit, hWnd, messg, wParam, lParam);
         }
         else
         {
            SendMessage(hWnd,EM_HIDESELECTION ,true,0);
            CallWindowProc(fnOldEdit, hWnd, messg, wParam, lParam);  // Call twice to ensure insertion point
            r = CallWindowProc(fnOldEdit, hWnd, messg, wParam, lParam);      // is correct after changing windows
            SendMessage(hWnd,EM_HIDESELECTION ,false,0);
         }

		   mouseLeftClicked = true;
		   long startSel,endSel;
         CText procName;
	      GetEditSelection(curEditor->edWin,startSel,endSel);	

      // Display the syntax (if any) for the selected text 
         UpDateEditSyntax(win, curEditor, startSel,endSel);

      // Determine which procedure the cursor is in and display it (2.2.6)
         curEditor->currentProc = ""; 
         curEditor->FindCurrentProcedure(endSel,curEditor->currentProc);
         SetEditTitle();

         if(clickCounter == 1)
         {
            curClickLineNr = GetCurrentLineNumber(curEditor->edWin);
            clickTimer = GetMsTime();
         }
		   return(r);
         break;		   
		}	

// When user releases left button record new insertion point for undo also reset dragdrop flag ****
		case(WM_LBUTTONUP): 
		{
        // if(wParam & MK_CONTROL) // Zoom as well if control key is pressed
        // {
		      //long startSel,endSel;
	       //  GetEditSelection(curEditor->edWin,startSel,endSel);
        //    r = CallWindowProc(fnOldEdit,hWnd,messg,wParam,lParam);
        //    UpDateEditSyntax(win, curEditor, startSel,endSel, true);
        //    return(r);
        // }
         if(curEditor)
         {
	         GetEditSelection(curEditor->edWin,startSel,endSel);		
		      curEditor->initCharInsertionPnt = startSel;	
		      mouseLeftClicked = false;	
         }
         break;  
	   }

      
  // Stop drag and drop since it causes editor to crash with undo limit set to zero ****************** 
  // Drag drop occurs when user clicks and drags mouse over selected text 
      case(WM_MOUSEMOVE):
      { 
         cursor_mode = NORMAL_CURSOR;
         inEditDivider = false;
	
         if(wParam & MK_LBUTTON)	
         {
            if(curEditor)
            {
	            GetEditSelection(curEditor->edWin,startSel,endSel);               
              
               if(mouseLeftClicked == true) // See if we are in dragdrop moce
               {
                  mouseLeftClicked = false;
  		            POINTL p;
  		            p.x = LOWORD(lParam);
  		            p.y = HIWORD(lParam);
  		            SendMessage(curEditor->edWin,EM_CHARFROMPOS ,(WPARAM)0,(LPARAM)&p); // Are we in it?	
               } 
            }
         }   
         break;
      } 

 // Display the contextual menu for the editor  *******************************************************             		
		case(WM_RBUTTONDOWN): 
		{
         if(ep && curEdNr >= 0)
         {     
			   curEditor = ep->editData[curEdNr];  // Make this the current editor
				win->currentEditor = curEditor;
            ep->curRegion = curEdNr;
			   SetFocus(curEditor->edWin);  // Give it window focus		   
		      SetEditTitle();       // Set the window title
		      lastEditor = curEdNr;              // Record the window number in case it changes		         
		   }		
		
		   POINT p;
         p.x = LOWORD(lParam);
         p.y = HIWORD(lParam);

         if(!ep->showContextualMenu)
            break;

      // Load pop-up menu
         HMENU hMenu1 = LoadMenu (prospaInstance, "TEXTREGIONMENU");

      // Load the previously executed text files
         HMENU hMenu2 = CreatePopupMenu();
         {
            TextList *list;
		      short cnt = 1;
            char path[MAX_PATH];
            char filename[MAX_PATH];

		      AppendMenu(hMenu2,MF_STRING,cnt++,"Reset list");
		      AppendMenu(hMenu2,MF_SEPARATOR	,cnt++,"");
		      
	         list = procRunList;
	         
            if(showFullName)
            {
	            while(1)
	            {
                  if(cnt >= MAX_MENU_MACS) break;
			         list = list->GetNextText();
			         if(!list) break;
			         AppendMenu(hMenu2,MF_STRING,cnt++,list->text);
               }
            }
            else
            {
	            while(1)
	            {
                  if(cnt >= MAX_MENU_MACS) break;
			         list = list->GetNextText();
			         if(!list) break;
                  ExtractFileNames(list->text,path,filename);
			         AppendMenu(hMenu2,MF_STRING,cnt++,filename);
               }
            }
         }

      // Load the previously run text files
         HMENU hMenu3 = CreatePopupMenu();
         {
            TextList *list;
		      short cnt = MAX_MENU_MACS;
            char path[MAX_PATH];
            char filename[MAX_PATH];

		      AppendMenu(hMenu3,MF_STRING,cnt++,"Reset list");
		      AppendMenu(hMenu3,MF_SEPARATOR	,cnt++,"");
		      
	         list = procLoadList;
	         
            if(showFullName)
            {
	            while(1)
	            {
                  if(cnt >= 2*MAX_MENU_MACS) break;
			         list = list->GetNextText();
			         if(!list) break;
			         AppendMenu(hMenu3,MF_STRING,cnt++,list->text);
               }
            }
            else
            {
	            while(1)
	            {
                  if(cnt >= 2*MAX_MENU_MACS) break;
			         list = list->GetNextText();
			         if(!list) break;
                  ExtractFileNames(list->text,path,filename);
			         AppendMenu(hMenu3,MF_STRING,cnt++,filename);
               }
            }
         }
         HMENU hMenu = GetSubMenu(hMenu1,0) ;
         InsertMenu(hMenu, 0, (UINT) MF_POPUP | MF_BYPOSITION,(UINT)hMenu2 ,"Previously run macros");
         InsertMenu(hMenu, 0, (UINT) MF_POPUP | MF_BYPOSITION,(UINT)hMenu3 ,"Previously edited macros");
         ClientToScreen(hWnd,&p);
         short item = TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_RETURNCMD, p.x, p.y, 0, hWnd, NULL) ;
 
      // Keep this edit region and remove all others? ****
         if(item >= ID_KEEP_THIS_REGION)
         { 
            Interface itfc;
            EditorFunctions(&itfc,"\"multiedit 1*1\"");
         }
         else
         {
		   // Load previous run macro? **************************
		      if(item >= 3 && item < MAX_MENU_MACS) 
            {
               bool alreadyLoaded = false;

            // Make sure the current text is saved
               if(curEditor->CheckForUnsavedEdits(curEditor->regNr) == IDCANCEL)
                   break;
            
               TextList* txt = procRunList->GetNthText(item-3);
               if(txt)
               {
                  char pathBak[MAX_PATH];
                  char nameBak[MAX_PATH];
                  strcpy(pathBak,curEditor->edPath); // save current file name
                  strcpy(nameBak,curEditor->edName);

                  ExtractFileNames(txt->text,curEditor->edPath,curEditor->edName);

                   if(IsEditFileAlreadyLoaded(curEditor) >= 0) // (2.2.7)
                   {
                        alreadyLoaded = true;
                        if(YesNoDialog(MB_ICONWARNING,0,"Warning","File already loaded\rDo you want to load a read-only copy?") == IDNO)
                        {
                           strcpy(curEditor->edPath,pathBak); // Restore file name
                           strcpy(curEditor->edName,nameBak);
                           break;
                        }
                   }

		         // Load macro into editor
			         SetCursor(LoadCursor(NULL,IDC_WAIT));
                  LoadEditorGivenPath(curEditor->edPath,curEditor->edName);
               // Save pathname to linked list for use previously edited pop-up menu
                  AddFilenameToList(procLoadList,curEditor->edPath, curEditor->edName);
			         SetCursor(LoadCursor(NULL,IDC_ARROW));  

               // Note if file already loaded
                  if(alreadyLoaded) // (2.2.7)
                     curEditor->readOnly = true; 
                  else
                     curEditor->readOnly = false; 
                  SetEditTitle();  
               }
            }

           // Load previous edited macro? **************************
		      if(item >= MAX_MENU_MACS+2 && item < 2*MAX_MENU_MACS) 
            {
             // Make sure the current text is saved
               if(curEditor->CheckForUnsavedEdits(curEditor->regNr) == IDCANCEL)
                   break;  

               TextList* txt = procLoadList->GetNthText(item-MAX_MENU_MACS-2);
               if(txt)
               {
                  bool alreadyLoaded = false;
                  char pathBak[MAX_PATH];
                  char nameBak[MAX_PATH];
                  strcpy(pathBak,curEditor->edPath); // save current file name
                  strcpy(nameBak,curEditor->edName);

                  ExtractFileNames(txt->text,curEditor->edPath,curEditor->edName);

              // Check to see if file already loaded
                  if(IsEditFileAlreadyLoaded(curEditor) >= 0)
                  {
                     alreadyLoaded = true;
                     if(YesNoDialog(MB_ICONWARNING,0,"Warning","File already loaded\rDo you want to load a read-only copy?") == IDNO)
                     {
                        strcpy(curEditor->edPath,pathBak); // Restore file name
                        strcpy(curEditor->edName,nameBak);
                        break;
                     }
                  }

		         // Load macro into editor
			         SetCursor(LoadCursor(NULL,IDC_WAIT));
                  LoadEditorGivenPath(curEditor->edPath,curEditor->edName);  

              // Note if file already loaded
                  if(alreadyLoaded)
                     curEditor->readOnly = true; 
                  else
                     curEditor->readOnly = false; 
                  SetEditTitle();  
			         SetCursor(LoadCursor(NULL,IDC_ARROW));
               }
            }

          // Reset the popup macro lists? ********************
            else if(item == MAX_MENU_MACS)
            {
               procLoadList->RemoveAll();
            }
            else if(item == 1)
            {
               procRunList->RemoveAll();
            }
         }	         
         DestroyMenu(hMenu);  
         DestroyMenu(hMenu1);  
         DestroyMenu(hMenu2);  
         DestroyMenu(hMenu3);  
         return(0);       		               
      }	
	}
		
	return(CallWindowProc(fnOldEdit,hWnd,messg,wParam,lParam));
}


/***********************************************************************
   Scans through the current text editor looking for procedure names    
   adding them to the procedure menu.
***********************************************************************/

void AddProcedureNames(WinData *win, HMENU menu)
{
   char *line;
   long pos;
   char *command;
   char *name; 
   short i;
   UINT cnt = PROC_MENU_START;

  // See if this window has a current editor
	if(win->currentEditor)
		curEditor = win->currentEditor;
	
  // Make sure a current editor is defined
	if(!curEditor)
		return;

   short n = CountLinesInEditor(curEditor->edWin);
 
 // Search each line in current text editor for procedure names     
   for(short i = 0; i < n; i++)
   {
      line = GetLineByNumber(curEditor->edWin,i);
      command = new char[strlen(line)+1];
      name = new char[strlen(line)+1]; 
            
      pos = 0;
      GetNextWord(pos, line, command, 100);
      if(!strcmp(command,"procedure"))
      {
         GetNextWord(pos, line, name, 100);
         if(curEditor->currentProc == name) // Modified to add check on current procedure name (2.2.6)
            AppendMenu(menu,MF_STRING | MF_CHECKED,cnt++,name); 
         else
            AppendMenu(menu,MF_STRING | MF_UNCHECKED,cnt++,name); 
         if(cnt-PROC_MENU_START-1 > MAX_MENU_ITEMS)
         {
            delete [] command;
            delete [] name;        
            delete [] line;
            TextMessage("\n   More than %d menu items\n",(int)MAX_MENU_ITEMS);
            break;
         }
      }   
      delete [] command;
      delete [] name;        
      delete [] line;
   }
}

/***********************************************************************
   Given a window handle finds the corresponding EditRegion instance
***********************************************************************/

short FindEditor(WinData *win, HWND hWnd, EditParent** ep)
{
   short i;

	WidgetList* editors = win->widgets.getAllOfType(TEXTEDITOR);
	for(ObjectData* obj: *editors)
   {
      if(obj->type == TEXTEDITOR)
      {
         *ep = (EditParent*)obj->data;
         if(*ep)
         {
            for(i = 0; i < (*ep)->rows*(*ep)->cols; i++)
            {
               if((*ep)->editData[i]->edWin == hWnd)
					{
						delete(editors);
                  return(i);
					}
            }
         }
      }
   }

	delete(editors);
	return(-1);
}



/**********************************************************************
  Check to see if the file loaded into edit window curNr has been 
  previously loaded
***********************************************************************/

short IsEditFileAlreadyLoaded(EditRegion *curRegion)
{

   if(!curEditor)
      return(-1);

   EditParent *ep = curEditor->edParent;

// Loop over all edit regions  
	for(int edNr = 0; edNr < ep->cols*ep->rows; edNr++) 
	{
      if(ep->editData[edNr] == curRegion) continue;

      if(!stricmp(ep->editData[edNr]->edName,curRegion->edName) &&
         !stricmp(ep->editData[edNr]->edPath,curRegion->edPath) &&
         !ep->editData[edNr]->readOnly)
         return(edNr);        
	}

	return(-1);
}

/**********************************************************************
  Check to see if the file loaded into edit window curNr has been 
  previously loaded
***********************************************************************/

short IsEditFileAlreadyLoaded(EditRegion *curRegion, char *filePath, char *fileName)
{

   if(!curEditor)
      return(-1);

   EditParent *ep = curEditor->edParent;
   int szPath = strlen(filePath);

// Loop over all edit regions  
	for(int edNr = 0; edNr < ep->cols*ep->rows; edNr++) 
	{
    //  if(ep->editData[edNr] == curRegion) continue;

      if(szPath > 0)
      {
         if(!stricmp(ep->editData[edNr]->edName,fileName) &&
            !stricmp(ep->editData[edNr]->edPath,filePath) &&
            !ep->editData[edNr]->readOnly)
            return(edNr); 
      }
      else
      {
         if(!stricmp(ep->editData[edNr]->edName,fileName) &&
            !ep->editData[edNr]->readOnly)
            return(edNr); 
      }
	}

	return(-1);
}

/**********************************************************************
  Find the next free editor
***********************************************************************/

short FindEmptyEditor(EditRegion *curRegion)
{
   EditParent *ep = curEditor->edParent;

   if(!curEditor)
      return(-1);

// Loop over all edit regions  
	for(int edNr = 0; edNr < ep->cols*ep->rows; edNr++) 
	{
      if(ep->editData[edNr] == curRegion)
			continue;

      if(ep->editData[edNr]->edName[0] == '\0' || !strcmp(ep->editData[edNr]->edName,"untitled"))
         return(edNr);    
	}

	return(-1);
}


/**********************************************************************
  Make a specified number of sub-edit windows (nrx in x directions and
  nry in y direction).
  save = true means we keep the original edit text if possible transferring
  it from the old to the new window layout.
***********************************************************************/


void MakeEditWindows(bool save, EditParent *ep, short nry, short nrx, short saveThisOne)
{
   long length = 0;
   char **text = 0;
   bool *modified = 0;
   bool *readOnly = 0;
   long *startPos = 0;
   long *endPos = 0;
   long *startLine = 0;
   char **title = 0;
   char **path = 0;
   short *syntaxStyle = 0;
   short nrSaved = 0;
   vector<EditJumpType> **jumpArrays = 0;
   vector<UndoType> **undoArrays = 0;
   long *jumpIndices = 0;
   long *undoIndices = 0;
   long *insertionPnts = 0;

   EditRegion** er = 0;
   
// Save contents of old edit window (nrx*nry subwindows)
   if(save)
   {   
   // First prompt the user to see if they wish to save any of 
   // the edited regions which cannot be transferred to the new layout  
 
      if(saveThisOne >= 0) // Save if going from larger to smaller and insertion point is in a region being deleted
      {
 		   for(int i = 0; i < ep->cols*ep->rows; i++) 
		   {  
            if(i == saveThisOne)
               continue;
            int response = ep->editData[i]->CheckForUnsavedEdits(i);
            if(response == IDYES)
            { 
               if(ep->editData[i]->SaveEditContents() == ERR)
                  return;
            }
            else if(response == ID_CANCEL)
               return;
		   }
      }
      else // Save if going from larger to smaller and insertion point is in a region being kept
      {
 		   for(int i = nrx*nry; i < ep->cols*ep->rows; i++) 
		   {  
            int response = ep->editData[i]->CheckForUnsavedEdits(i);
            if(response == IDYES)
            { 
               if(ep->editData[i]->SaveEditContents() == ERR)
                  return;
            }
            else if(response == ID_CANCEL)
               return;
		   }
      }
	// Allocate space for edit info	  
      startPos = new long[nrx*nry];
      endPos = new long[nrx*nry];      
	   text = new char*[nrx*nry];
	   title = new char*[nrx*nry];
	   path = new char*[nrx*nry];
	   modified = new bool[nrx*nry];
	   readOnly = new bool[nrx*nry];
	   jumpArrays = new vector<EditJumpType>*[nrx*nry];
	   undoArrays = new vector<UndoType>*[nrx*nry];	  
      jumpIndices = new long[nrx*nry];
      undoIndices = new long[nrx*nry]; 
      insertionPnts = new long[nrx*nry]; 
      syntaxStyle = new short[nrx*nry];
      startLine = new long[nrx*nry];
      
   // Fill in that info 

      if(saveThisOne >= 0)
      {
		   length = SendMessage(ep->editData[saveThisOne]->edWin,WM_GETTEXTLENGTH,0,0);
	      text[0] = new char[length+1];
	      title[0] = new char[strlen(ep->editData[saveThisOne]->edName)+1];
	      path[0] = new char[strlen(ep->editData[saveThisOne]->edPath)+1];
	      strcpy(title[0],ep->editData[saveThisOne]->edName);
	      strcpy(path[0],ep->editData[saveThisOne]->edPath);
	      modified[0] = ep->editData[saveThisOne]->edModified;
	      readOnly[0] = ep->editData[saveThisOne]->readOnly;
	      SendMessage(ep->editData[saveThisOne]->edWin,WM_GETTEXT,(WPARAM)length+1,(LPARAM)(text[0]));	
         GetEditSelection(ep->editData[saveThisOne]->edWin,startPos[0],endPos[0]);
	      jumpArrays[0]  = ep->editData[saveThisOne]->jumpArray;
	      jumpIndices[0] = ep->editData[saveThisOne]->jumpIndex;
	      undoArrays[0]  = ep->editData[saveThisOne]->undoArray;
	      undoIndices[0] = ep->editData[saveThisOne]->undoIndex;	
	      ep->editData[saveThisOne]->jumpArray = NULL; 
	      ep->editData[saveThisOne]->undoArray = NULL;   
	      insertionPnts[0] = ep->editData[saveThisOne]->initCharInsertionPnt;      
         syntaxStyle[0] = ep->editData[saveThisOne]->syntaxColoringStyle;
         startLine[0] = SendMessage(ep->editData[saveThisOne]->edWin,EM_GETFIRSTVISIBLELINE,0,0);
         nrSaved = 1;
      }
      else
      {
			int i = 0;
		   for(i = 0; i < nrx*nry; i++) 
		   {
		      if(i == ep->cols*ep->rows)
		         break;
		      length = SendMessage(ep->editData[i]->edWin,WM_GETTEXTLENGTH,0,0);
	         text[i] = new char[length+1];
	         title[i] = new char[strlen(ep->editData[i]->edName)+1];
	         path[i] = new char[strlen(ep->editData[i]->edPath)+1];
	         strcpy(title[i],ep->editData[i]->edName);
	         strcpy(path[i],ep->editData[i]->edPath);
	         modified[i] = ep->editData[i]->edModified;
	         readOnly[i] = ep->editData[i]->readOnly;
	         SendMessage(ep->editData[i]->edWin,WM_GETTEXT,(WPARAM)length+1,(LPARAM)(text[i]));	
            GetEditSelection(ep->editData[i]->edWin,startPos[i],endPos[i]);
	         jumpArrays[i]  = ep->editData[i]->jumpArray;
	         jumpIndices[i] = ep->editData[i]->jumpIndex;
	         undoArrays[i]  = ep->editData[i]->undoArray;
	         undoIndices[i] = ep->editData[i]->undoIndex;	
	         ep->editData[i]->jumpArray = NULL; 
	         ep->editData[i]->undoArray = NULL;   
	         insertionPnts[i] = ep->editData[i]->initCharInsertionPnt;      
            syntaxStyle[i] = ep->editData[i]->syntaxColoringStyle;
            startLine[i] = SendMessage(ep->editData[i]->edWin,EM_GETFIRSTVISIBLELINE,0,0);
		   }
         nrSaved = i;
      }
	}

// Delete all old edit regions
	ep->FreeEditMemory();

// Make new edit regions (set undo limit to zero since we will do undos ourselves)
	er = new EditRegion*[nrx*nry];

	for(int i = 0; i < nry; i++) 
	{
	   for(int j = 0; j < nrx; j++) 
	   {
	      short n = i*nrx + j;	      
	      er[n] = new EditRegion(ep,i,j,nrx,nry);
	      er[n]->x = j/(float)nrx;
	      er[n]->y = i/(float)nry;
	      er[n]->w = 1.0/nrx;
	      er[n]->h = 1.0/nry;	      
	   }
	}  

   ep->cols = nrx;
   ep->rows = nry;
   ep->editData = er;
   curEditor = er[0];
   ep->curWnd = er[0]->edWin;
   ep->curRegion = 0;
   ep->parent->hWnd = er[0]->edWin;

// Copy text back to regions
   if(save)
   {
      if(saveThisOne >= 0)
      {
			SetCursor(LoadCursor(NULL,IDC_WAIT));
      // Syntax colouring style
         ep->editData[0]->syntaxColoringStyle = syntaxStyle[0];
		// Display the text in the editor
	      ep->editData[0]->DisplayColoredText(text[0],false);
	   // Update names
         strcpy(ep->editData[0]->edName,title[0]);
         strcpy(ep->editData[0]->edPath,path[0]);
      // Update modified flag
	      ep->editData[0]->edModified = modified[0];
	   // Restore selection points
	   	ep->editData[0]->SelectEditPosition(startPos[0],endPos[0],startLine[0]); 
		// Restore cursor
			SetCursor(LoadCursor(NULL,IDC_ARROW));	
	  	// Restore jump arrays
	  	   ep->editData[0]->jumpArray = jumpArrays[0];
	  	   ep->editData[0]->jumpIndex = jumpIndices[0];
      // Restore undo arrays
	  	   ep->editData[0]->undoArray = undoArrays[0];
	  	   ep->editData[0]->undoIndex = undoIndices[0];
      // Restore insertion point
         ep->editData[0]->initCharInsertionPnt = insertionPnts[0];
      // Restore readonly status
         ep->editData[0]->readOnly = readOnly[0];
         SetFocus(ep->editData[0]->edWin);
	      delete [] text[0];	   
	      delete [] title[0];	   
	      delete [] path[0];
      }
      else
      {
		   for(int i = 0; i < nrSaved; i++) 
		   {
			   SetCursor(LoadCursor(NULL,IDC_WAIT));
         // Syntax colouring style
            ep->editData[i]->syntaxColoringStyle = syntaxStyle[i];
		   // Display the text in the editor
	         ep->editData[i]->DisplayColoredText(text[i],false);
	      // Update names
            strcpy(ep->editData[i]->edName,title[i]);
            strcpy(ep->editData[i]->edPath,path[i]);
         // Update modified flag
	         ep->editData[i]->edModified = modified[i];
	      // Restore selection points
	   	   ep->editData[i]->SelectEditPosition(startPos[i],endPos[i],startLine[i]); 
		   // Restore cursor
			   SetCursor(LoadCursor(NULL,IDC_ARROW));	
	  	   // Restore jump arrays
	  	      ep->editData[i]->jumpArray = jumpArrays[i];
	  	      ep->editData[i]->jumpIndex = jumpIndices[i];
         // Restore undo arrays
	  	      ep->editData[i]->undoArray = undoArrays[i];
	  	      ep->editData[i]->undoIndex = undoIndices[i];
         // Restore insertion point
            ep->editData[i]->initCharInsertionPnt = insertionPnts[i];
         // Restore readonly status
            ep->editData[i]->readOnly = readOnly[i];
            SetFocus(ep->editData[i]->edWin);
	         delete [] text[i];	   
	         delete [] title[i];	   
	         delete [] path[i];	   
		   }
      }
	   delete [] text;
	   delete [] path;
	   delete [] title;
      delete [] syntaxStyle;
	   delete [] modified;
	  	delete [] readOnly;	  		  		  		  	 	  	  
	  	delete [] startPos;	 
	  	delete [] endPos;	
	  	delete [] jumpArrays;	 
	  	delete [] jumpIndices;	 
	  	delete [] undoArrays;	 
	  	delete [] undoIndices;	 
	  	delete [] insertionPnts;	  		  		  		  	 	  	   		  		  		  	 	  	  
   }
 
 // Set focus and make sure window is redrawn


	SetEditTitle();	 

   MyInvalidateRect(ep->parent->hwndParent,NULL,false);   

}

/**********************************************************************
  Set the edit window title to reflect the contents of subedit window n.
  (1 based).
***********************************************************************/

void SetEditTitle()
{
   CText temp;
   CText path;
   

   if(!curEditor)
      return;

	bool update = curEditor->edParent->parent->winParent->titleUpdate;
   if(!update && (curEditor->labelCtrlNr == -1))
      return;

   bool mergeTitle = curEditor->edParent->parent->winParent->mergeTitle;

   short n = curEditor->regNr+1;
   short w = curEditor->edParent->parent->winParent->nr;
	short nrSubplots = curEditor->edParent->rows*curEditor->edParent->cols;

   path.Assign(curEditor->edPath);
   if(path.Size() > 0 && path[path.Size()-1] != '\\')
      path = path + "\\";

	CText txt;
	if(update & !mergeTitle)
	{
		if(nrSubplots > 1)
			txt.Format("Editor (%hd:%hd)",w,n);
		else
			txt.Format("Editor (%hd)",w);
	}
   else if(update & mergeTitle)
	{
	   txt = curEditor->edParent->parent->winParent->title;
	}
	else
	{
		if(nrSubplots > 1)
			txt.Format("Editor (%hd)",n);
		else
			txt.Format("Editor");
	}

   if(curEditor->currentProc.Size() > 0)
   {
	   if(curEditor->labelCtrlNr == -1)
		{
			if(curEditor->edModified)
			{
				if(showFullName)	   
					temp.Format("%s    %s%s:%s()*",txt.Str(),path.Str(),curEditor->edName,curEditor->currentProc.Str());
				else
					temp.Format("%s    %s:%s()*",txt.Str(),curEditor->edName,curEditor->currentProc.Str());
			}
			else if(curEditor->readOnly) 
			{
				if(showFullName)	   
					temp.Format("%s    %s%s:%s() ***Read-Only***",txt.Str(),path.Str(),curEditor->edName,curEditor->currentProc.Str());
				else
					temp.Format("%s    %s:%s() ***Read-Only***",txt.Str(),curEditor->edName,curEditor->currentProc.Str());
			}
			else
			{
				if(showFullName)	   
					temp.Format("%s    %s%s:%s()",txt.Str(),path.Str(),curEditor->edName,curEditor->currentProc.Str());
				else
					temp.Format("%s    %s:%s()",txt.Str(),curEditor->edName,curEditor->currentProc.Str());
			}
		}
		else
		{
			if(curEditor->edModified)
			{
				temp.Format("%s*",curEditor->edName);
			}
			else if(curEditor->readOnly)
			{				
				temp.Format("%s  ***Read-Only***",curEditor->edName);
			}
			else
			{
				temp.Format("%s",curEditor->edName);
			}
		}
   }
   else
   {
		if(curEditor->labelCtrlNr == -1)
		{
			if(curEditor->edModified)
			{
				if(showFullName)	   
					temp.Format("%s    %s%s*",txt.Str(),path.Str(),curEditor->edName);
				else
					temp.Format("%s    %s*",txt.Str(),curEditor->edName);
			}
			else if(curEditor->readOnly)
			{
				if(showFullName)	   
					temp.Format("%s    %s%s  ***Read-Only***",txt.Str(),path.Str(),curEditor->edName);
				else
					temp.Format("%s    %s  ***Read-Only***",txt.Str(),curEditor->edName);
			}
			else
			{
				if(showFullName)	   
					temp.Format("%s    %s%s",txt.Str(),path.Str(),curEditor->edName);
				else
					temp.Format("%s    %s",txt.Str(),curEditor->edName);
			}
		}
		else
		{
			if(curEditor->edModified)
			{
				temp.Format("%s*",curEditor->edName);
			}
			else if(curEditor->readOnly)
			{				
				temp.Format("%s  ***Read-Only***",curEditor->edName);
			}
			else
			{
				temp.Format("%s",curEditor->edName);
			}
		}
   }

	if(curEditor->labelCtrlNr == -1)
	{
      SetWindowText(curEditor->edParent->parent->hwndParent,temp.Str());
	}
	else
	{
		int objNr = curEditor->labelCtrlNr;
		ObjectData *ctrl = curEditor->edParent->parent->winParent->FindObjectByNr(objNr);
      HDC hdc = GetDC(ctrl->hWnd);
      SIZE sz;
      GetTextExtentPoint32(hdc, temp.Str(), temp.Size(), &sz);
      ctrl->wo = sz.cx;
      ctrl->wSzScale = 0;
      ctrl->wSzOffset = ctrl->wo;
      ctrl->Move(ctrl->xo, ctrl->yo, ctrl->wo, ctrl->ho);
		SetWindowText(ctrl->hWnd,temp.Str());
	}
}

/**********************************************************************
  Remove all editors except the current one, saving any unsaved edits
  it the user want to.
***********************************************************************/

void RemoveAllButCurrentText(EditParent *ep)
{
   long length;
   char *text;
   bool modified;
   char *title;
   char *path;
   long startSel,endSel;
   vector<EditJumpType> *jumpArray;  
   vector<UndoType> *undoArray; 
   long jumpIndex;
   long undoIndex;
   short edNr;
   int response;
   long startLine;

   if(!curEditor)
      return;

// Prompt to save editwindows which have been modified (except current)
	for(edNr = 0; edNr < ep->cols*ep->rows; edNr++) 
	{
	   if(edNr+1 == ep->curRegion) // Ignore current editor
	      continue;	      

      response = ep->editData[edNr]->CheckForUnsavedEdits(edNr);
      if(response == IDYES)
      { 
         if(ep->editData[edNr]->SaveEditContents() == ERR)
            return;
      }
      else if(response == ID_CANCEL)
         return;
	}

	SetCursor(LoadCursor(NULL,IDC_WAIT));
		     
// Save current text-editor parameters
	length = SendMessage(curEditor->edWin,WM_GETTEXTLENGTH,0,0);
	text = new char[length+1];
	title = new char[strlen(curEditor->edName)+1];
	path = new char[strlen(curEditor->edPath)+1];
	strcpy(title,curEditor->edName);
	strcpy(path,curEditor->edPath);
	modified = curEditor->edModified;
	SendMessage(curEditor->edWin,WM_GETTEXT,(WPARAM)length+1,(LPARAM)(text));	
	GetEditSelection(curEditor->edWin,startSel,endSel);
	undoArray = curEditor->undoArray;
	curEditor->undoArray = NULL;
	jumpArray = curEditor->jumpArray;
	curEditor->jumpArray = NULL;
	jumpIndex = curEditor->jumpIndex;
	undoIndex = curEditor->undoIndex;	
   startLine = SendMessage(curEditor->edWin,EM_GETFIRSTVISIBLELINE,0,0);

// Delete all edit regions  
	FreeEditMemory(ep);
	
// Make a new single-edit region
   ep->rows = 1;
   ep->cols = 1;    
	ep->editData = new EditRegion*[1];
   ep->editData[0] = new EditRegion(ep,0,0,1,1); 	      	
	ep->editData[0]->x = 0;
	ep->editData[0]->y = 0;
	ep->editData[0]->w = 1.0;
	ep->editData[0]->h = 1.0;	      
	SendMessage(ep->editData[0]->edWin,EM_SETUNDOLIMIT,(WPARAM)0,(LPARAM)(0));
	   	
// Copy text back into region
	curEditor = ep->editData[0];
	curEditor->DisplayColoredText(text,false);
	strcpy(curEditor->edName,title);
	strcpy(curEditor->edPath,path);
	curEditor->edModified = modified; 
	curEditor->SelectEditPosition(startSel,endSel,startLine);
	curEditor->undoArray = undoArray;
	curEditor->jumpArray = jumpArray;	
	curEditor->undoIndex = undoIndex;
	curEditor->jumpIndex = jumpIndex;	

// Free up memory used			
	delete [] text;	   
	delete [] title;	   
	delete [] path;	   
 
 // Set focus and make sure window and title are redrawn
   ep->curRegion = 1;
	SetFocus(curEditor->edWin);	
	SetEditTitle();   
   MyInvalidateRect(ep->parent->hwndParent,NULL,false);   
   
	SetCursor(LoadCursor(NULL,IDC_ARROW));   	    	      	       
}	



/************************************************************************
  Displays standard Open File dialog               

  User passes pointer for returned directory & filename
  Title for window 
  Filter range (eg "txt" for *.txt files or "all" for *.* files 
************************************************************************/

short LoadEditDialog(HWND hWnd, char *pathName, char *fileName)
{
   short err;
   static short index = 1;
   char oldDir[MAX_PATH];

// Save the current working directory
   GetCurrentDirectory(MAX_PATH,oldDir);
   
   if(!strcmp(fileName,"untitled"))
      strcpy(pathName,PlotFile::getCurrMacroDirectory());
   
   err = FileDialog(hWnd, true, pathName, fileName, 
                    "Open a text file", CentreHookProc, NULL, noTemplateFlag, 6, &index,
                    "Script Files","mac",
                    "Text Files","txt",
                    "List Files","lst",
                    "Parameter Files","par",
                    "Prospa executable files","pex",
                    "All Files","*");
      
   if(err == OK)
      GetCurrentDirectory(MAX_PATH,PlotFile::getCurrMacroDirectory());

// Restore the directory
   SetCurrentDirectory(oldDir);

   return(err);
}



/************************************************************************
   Displays standard Save File dialog  
 
  User passes pointer for returned directory & filename
  Title for window 
  Filter range (eg "txt" for *.txt files or "all" for *.* files 
************************************************************************/

short SaveEditDialog(HWND hWnd, char *pathName, char *fileName)
{
   short err;
   static short index = 1;
   char oldDir[MAX_PATH];

// Save the current working directory as the following command changes it
   GetCurrentDirectory(MAX_PATH,oldDir);

   err = FileDialog(hWnd, false, pathName, fileName, 
                    "Save a text file", CentreHookProc, NULL, noTemplateFlag, 5, &index,
                    "Script Files","mac",
                    "Text Files","txt",
                    "List Files","lst",
                    "Parameter Files","par",
                    "All Files","*");
 
// Make sure returned path has standard form
   if(err == OK)
      GetCurrentDirectory(MAX_PATH,PlotFile::getCurrMacroDirectory());  

// Restore the directory
   SetCurrentDirectory(oldDir);
   return(err);
}


/**************************************************************************
  Update the editor title, showing modification and readwrite status
***************************************************************************/

int UpdateEditorParentTitle(Interface* itfc ,char args[])
{
    SetEditTitle();
    return(OK);
}

/**************************************************************************
 Load text from a file and display it in the current sub-editor window
 Apply syntax coloring to the text as it is loaded.
***************************************************************************/

int LoadEditor(Interface* itfc ,char args[])
{
   short nrArgs;
   CText fileName;
   char filePath[MAX_PATH];
   char procName[MAX_PATH];
   CText extension;
   char *text;
   bool alreadyLoaded = false;
   char pathBak[MAX_PATH];
   char nameBak[MAX_PATH];

// Get filename from user  
   if((nrArgs = ArgScan(itfc,args,1,"filename","e","t",&fileName)) < 0)
     return(nrArgs); 

// Check for valid editor
   if(!curEditor)
   {
      ErrorMessage("No editor defined");
      return(ERR);
   }
   
   EditParent *ep = curEditor->edParent;

// See if the current text need saving
   if(curEditor->CheckForUnsavedEdits(ep->curRegion) == IDCANCEL)
   {
      ErrorMessage("Command aborted");
      return(ERR);
   }

// Save current editor file name
   strcpy(pathBak,curEditor->edPath); 
   strcpy(nameBak,curEditor->edName);

// Load text from file
   filePath[0] = '\0';
   ExtractProcedureName(fileName.Str(),procName);
   if((text = LoadTextFile(itfc,filePath, fileName.Str(), procName, ".mac")) == NULL)
   {   
      ErrorMessage("Can't open file '%s'",fileName.Str());
      return(ERR);
   }

		// See if the file is readonly
	if(IsReadOnly(filePath,fileName.Str()))
		curEditor->readOnly = true;
	else
		curEditor->readOnly = false;

// Update editor file path and name
   strcpy(curEditor->edName,fileName.Str());
   strcpy(curEditor->edPath,filePath);

// Check to see if we already have an loaded file of this name in the editor
   if(IsEditFileAlreadyLoaded(curEditor) >= 0) // (2.2.7)
   {
      alreadyLoaded = true;
      if(YesNoDialog(MB_ICONWARNING,0,"Warning","File already loaded\rDo you want to load a read-only copy?") == IDNO)
      {
         strcpy(curEditor->edPath,pathBak); // Restore file name
         strcpy(curEditor->edName,nameBak);
	      delete [] text;     
         return(0);
      }
   }
// Set the syntyax colouring mode
   GetExtension(fileName,extension);
   if(extension == "asm")
      curEditor->syntaxColoringStyle = ASM_STYLE;
   else if (extension == "par")
      curEditor->syntaxColoringStyle = PAR_STYLE;
   else if(extension == "mac" || extension == "pex")
      curEditor->syntaxColoringStyle = MACRO_STYLE;
   else
      curEditor->syntaxColoringStyle = NO_STYLE;

// Load macro into editor
//   TextMessage("parent window %X\n",GetParent(curEditor->edWin));
   if(!curEditor->edParent->parent->winParent->keepInFront)
      ChangeGUIParent(GetParent(curEditor->edWin));
	SetCursor(LoadCursor(NULL,IDC_WAIT));
   curEditor->CopyTextToEditor(text);  
	delete [] text;     
	SetCursor(LoadCursor(NULL,IDC_ARROW));
	SetScrollPos(curEditor->edWin,SB_HORZ,0,true);
  // MyUpdateWindow(curEditor->edWin);
// Make sure the scroll bar it full to the left

// Note if file already loaded
   if(alreadyLoaded) // (2.2.7)
      curEditor->readOnly = true; 
 //  else
 //     curEditor->readOnly = false; 
   SetEditTitle();  

   return(0);
}



/***************************************************************************
  Load text from a file and display it in the current sub-editor window
  Apply syntax coloring to the text as it is loaded.
  (This option only looks in the specified path)
  This function assumes the file being overwritten has been saved if desired
****************************************************************************/

int LoadEditorGivenPath(char filePath[], char fileName[])
{
   char *text;
   char *extension;

   if(!curEditor)
   {   
      ErrorMessage("No current editor defined or selected");
      return(ERR);
   }
         
// Load text from file - don't change current directory
   if((text = LoadTextFileFromFolder(filePath, fileName,"")) == NULL)
   {   
      ErrorMessage("Can't open file '%s'",fileName);
      return(ERR);
   }

	// See if the file is readonly
	if(IsReadOnly(filePath,fileName))
		curEditor->readOnly = true;
	else
		curEditor->readOnly = false;
   
// Save file path and name
   strcpy(curEditor->edName,fileName);
   strcpy(curEditor->edPath,filePath);

// Set the syntax colouring mode
   extension = GetExtension(fileName);
   if(!strcmp(extension,"asm"))
      curEditor->syntaxColoringStyle = ASM_STYLE;
   else if (!strcmp(extension, "par"))
      curEditor->syntaxColoringStyle = PAR_STYLE;
   else if(!strcmp(extension,"mac") || !strcmp(extension,"pex"))
      curEditor->syntaxColoringStyle = MACRO_STYLE;
   else
      curEditor->syntaxColoringStyle = NO_STYLE;

// Display the text in the editor and update title
   curEditor->CopyTextToEditor(text);  

// Make sure the scroll bar it full to the left
	SetScrollPos(curEditor->edWin,SB_HORZ,0,true);

// Free memory      	  
   delete [] text;

// Make a breakpoint strip if in debug mode
   if(curEditor->debug)
   {
      UpdateDebugBreakPointList(curEditor);
   }

   return(0);
}

// Clear data from CLI, Editor or Plot

int Clear(Interface* itfc, char args[])
{
   short nrArgs;
   static CText what = "plot";
   
   if((nrArgs = ArgScan(itfc,args,1,"clear what?","e","t",&what)) < 0)
    return(nrArgs); 
    
   HWND focus = GetFocus();
   
   if(what == "cli")
   {
      long length;
      length = SendMessage(cliEditWin,WM_GETTEXTLENGTH,0,0) + 1;
      SetEditSelection(cliEditWin,0L,length);
      SendMessage(cliEditWin,WM_CLEAR,(WPARAM)0,(LPARAM)0);
      TextMessage("");
   }
   else if(what == "edit")
   {
      if(!curEditor)
      {   
         ErrorMessage("No current editor selected");
         return(ERR);
      }
      long length;
      length = SendMessage(curEditor->edWin,WM_GETTEXTLENGTH,0,0) + 1;
      SetEditSelection(curEditor->edWin,0L,length);
      SendMessage(curEditor->edWin,WM_CLEAR,(WPARAM)0,(LPARAM)0);
      SetEditSelection(curEditor->edWin,0L,0L);
   }
   else if(what == "plot")
   {
		if(Plot::curPlot() == Plot2D::curPlot())
      {
         Clear2D(itfc,"");
      }    
		else if(Plot::curPlot() == Plot1D::curPlot())
      {
         Clear1D(itfc,"");
      }         
   }
   else
   {
      ErrorMessage("invalid option (cli/edit/plot)");
      return(ERR);
   }
   SetFocus(focus);  
   if(itfc->macro && itfc->name == "CLI") // Make sure focus is restored to CLI
      currentAppWindow = cliWin;         // if this command was run from the CLI
   itfc->nrRetValues = 0;
   return(OK);
}

/****************************************************************************
  Handle events which will resize the sub-edit windows by dragging a 
  divider bar.
*****************************************************************************/


short SubEditDividerEvents(ObjectData *obj, EditParent *ep, HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam, short mode)
{	  
	RECT clientRect; 
	RECT winRect;
	 
   short x = LOWORD(lParam);  // horizontal position of cursor 
   short y = HIWORD(lParam);  // vertical position of cursor
   
// Convert mouse screen coordinates to gui window client coordinates 
	POINT p1,p2;  
   p1.x = obj->xo;
   p1.y = obj->yo;
   ClientToScreen(obj->hwndParent,&p1); 
   p2.x = obj->xo+obj->wo;
   p2.y = obj->yo+obj->ho;
   ClientToScreen(obj->hwndParent,&p2); 

// Get status bar height 
//   GetWindowRect(editStatusWnd,&sr);
//   short sh = sr.bottom-sr.top-1;   
// Get edit window client dimensions
   GetWindowRect(obj->hwndParent,&clientRect);	
//   clientRect.bottom -= sh; 

// If we are in a border then maybe we are over a divider and so set the cursor
// If the left mouse button is then pressed set the resizing flag.
   if((int)wParam == HTBORDER)
   {
      bool vertical = false;
      bool horizontal = false;

      if(x > p1.x+1 && x < p2.x-2 && y > p1.y+1 && y < p2.y-1)

   // Only do something if we are in the window dividers - not the edges
    //  if(x > clientRect.left+border_hgt && x < clientRect.right-border_hgt && y > clientRect.top+titlebar_hgt && y < clientRect.bottom-border_hgt)
   //   if(x > clientRect.left && x < clientRect.right && y > clientRect.top && y < clientRect.bottom)
      {      
      // Work out which divider we are on - vertical or horizontal

         GetWindowRect(hWnd,&winRect); 
         if(x < winRect.left+3 || x > winRect.right-3)
            vertical = true;  
         if(y < winRect.top+3 || y > winRect.bottom-3)  
            horizontal = true;  
         
         if(!vertical || !horizontal)  
         { 
				inEditDivider = true;
            if(vertical)
            {
               cursor_mode = VERTICAL_CURSOR;
               SetCursor(VertDivCursor); 
            } 
            else
            {
               SetCursor(HorizDivCursor); 
               cursor_mode = HORIZONTAL_CURSOR;
            }                                                                                                        
            if(mode == DIVIDER_CLICK)
            {
               TrackEditDivider(obj,ep,hWnd);
					inEditDivider = false;
            }

            return(1);
         }
      }
      else
      {
			inEditDivider = false;
         cursor_mode = NORMAL_CURSOR; 
         SetCursor(LoadCursor(NULL,IDC_ARROW)); 
      }              
   }
   else
   {           
      cursor_mode = NORMAL_CURSOR; 
      SetCursor(LoadCursor(NULL,IDC_ARROW)); 
		inEditDivider = false;
   } 
   return(0);
}


/****************************************************************************
  Track the movement of the divider window in a PeekMessage event loop.
  Note that itis possible to break out of this loop when:
  1. The user released the left mouse button.
  2. Another application is activated.
  3. The Alt-Tab key is pressed allowing the user to switch apps.
*****************************************************************************/

static short leftLimit,rightLimit;
static short topLimit,bottomLimit;
static short originalX = -1;  // Position of cursor when first clicked
static short originalY = -1;
static short oldx = -1;       // Last cursor location 
static short oldy = -1;  
 
void TrackEditDivider(ObjectData *obj, EditParent *ep, HWND hWnd)
{
   MSG msg;
   short x = -1,y = -1;
   oldx = -1;
   oldy = -1;
   originalX = -1;
   originalY = -1;

// Left mouse button must be down  
   if(!IsKeyDown(VK_LBUTTON))
      return;
 
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
			      MoveDivider(obj, ep, msg.hwnd,x,y,oldx,oldy); 
			   }
			   else
	            break; // Finished moving divider
			}
		}
   }
   
// Update the sub editor windows with chosen sizes
   if(x != -1 && y != -1)
   {
      ResizeSubEditors(obj,ep,msg.hwnd,x,y);
   }
				

// Release and restore cursor
   ReleaseCapture();
   cursor_mode = NORMAL_CURSOR;
}


/***************************************************************************
  Draw a line representing the new position of the window divider. This 
  line tracks the mouse cursor position.
****************************************************************************/ 
   
void MoveDivider(ObjectData *obj, EditParent *ep, HWND hwnd, short x, short y, short &oldx, short &oldy)
{
   short divider;
 	RECT winRect; 
// Get the edit window client dimensions

// Convert mouse screen coordinates to gui window client coordinates 
	POINT p1,p2;  
   p1.x = obj->xo;
   p1.y = obj->yo;
   p2.x = obj->xo+obj->wo;
   p2.y = obj->yo+obj->ho;

// Convert client mouse coordinates to editWin coordinates
   POINT p;
   p.x = x;
   p.y = y;
   ClientToScreen(hwnd,&p);   
   ScreenToClient(obj->hwndParent,&p); 
   x = p.x;
   y = p.y; 

// Choose the drawing pen for the movable divider
   HDC hdc = GetDC(ep->parent->hwndParent);
   SetROP2(hdc,R2_NOTXORPEN);
   HPEN pen = CreatePen(PS_SOLID,4,RGB(128,128,128));  
   HPEN oldpen = (HPEN)SelectObject(hdc, pen); 
             
// Is it a vertical divider? *************************
	if(cursor_mode == VERTICAL_CURSOR)
	{
      if(oldx != -1)
      {  // Erase the old divding line             
         MoveToEx(hdc,oldx+1,p1.y+2,NULL);
         LineTo(hdc,oldx+1,p2.y-4);
      }   
      else
      {
         originalX = x;
         // First find out which divider we are close too  
         for(divider = 1; divider < ep->cols*ep->rows; divider++)
         {
            GetSubWindowRect(obj->hwndParent,ep->editData[divider]->edWin,&winRect);
            if(abs(winRect.left-originalX) < 6)
               break;
         }
         // Then resize the windows to the left and right
         if(divider < ep->cols*ep->rows)
         {
            GetSubWindowRect(obj->hwndParent,ep->editData[divider-1]->edWin,&winRect);
            leftLimit = winRect.left+DIVIDER_OFFSET;
            GetSubWindowRect(obj->hwndParent,ep->editData[divider]->edWin,&winRect);
            rightLimit = winRect.right-DIVIDER_OFFSET;  
         }                            
      }
     // Limit the movement of the cursor       
      if(x < leftLimit)
         x = leftLimit;
      if(x > rightLimit)
         x = rightLimit;   

     // Draw the new divider position                                      
      MoveToEx(hdc,x+1,p1.y+2,NULL);
      LineTo(hdc,x+1,p2.y-4);
      oldx = x;
   }
   
// Is it a horizontal divider? ***************************        	
	else if(cursor_mode == HORIZONTAL_CURSOR)
	{
      if(oldy != -1)
      { // Erase the old dividing line                               
         MoveToEx(hdc,p1.x+2,oldy+1,NULL);
         LineTo(hdc,p2.x-2,oldy+1);
      }
      else
      {
         originalY = y; 
         // First find out which divider we are close too                
         for(divider = ep->cols; divider < ep->cols*ep->rows; divider++)
         {
            GetSubWindowRect(obj->hwndParent,ep->editData[divider]->edWin,&winRect);
            if(abs(winRect.top-originalY) < 6)
               break;
         }
         // Then resize the windows to the top and bottom
         if(divider < ep->cols*ep->rows)
         {
            GetSubWindowRect(obj->hwndParent,ep->editData[divider-1]->edWin,&winRect);
            topLimit = winRect.top+DIVIDER_OFFSET;
            GetSubWindowRect(obj->hwndParent,ep->editData[divider]->edWin,&winRect);
            bottomLimit = winRect.bottom-DIVIDER_OFFSET;  
         }                 
      } 
     // Limit the movement of the cursor 
      if(y < topLimit)
         y = topLimit;
      if(y > bottomLimit)
         y = bottomLimit;
     // Draw the new divider position                            
      MoveToEx(hdc,p1.x+2,y+1,NULL);
      LineTo(hdc,p2.x-2,y+1);
      oldy = y;
   }
   
// Tidy up
   SelectObject(hdc, oldpen); 
   ReleaseDC(ep->parent->hwndParent,hdc);
   DeleteObject(pen);                                    
}

// Return child window rectangle in coordinates relative to parent 
void GetSubWindowRect(HWND parent,HWND child, RECT *r)
{
   GetWindowRect(child,r);
	POINT p1,p2;  
   p1.x = r->left;
   p1.y = r->top;
   ScreenToClient(parent,&p1); 
   p2.x = r->right;
   p2.y = r->bottom;
   ScreenToClient(parent,&p2); 
   SetRect(r,p1.x,p1.y,p2.x,p2.y);
}
  

/***************************************************************************
  The left mouse button has been released so resize each of the subedit 
  windows based on the new position of the divider (xdiv or ydiv)
****************************************************************************/ 
 
void ResizeSubEditors(ObjectData *obj, EditParent *ep, HWND hwnd, short xdiv, short ydiv)
{
   EditRegion *ed;
   long x,y,w,h;
   short divider;
 	RECT winRect;


// Convert divider coordinates to gui win coordinates   
   POINT p;
   p.x = xdiv;
   p.y = ydiv;
   ClientToScreen(hwnd,&p);   
   ScreenToClient(ep->parent->hwndParent,&p); 
   x = p.x;
   y = p.y;

// Hide window changes from the user
	SendMessage(hwnd, WM_SETREDRAW, false, 0); 
	      
// Finished moving the vertical divider  so resize sub-windows
   if(cursor_mode == VERTICAL_CURSOR)
   {
     // Limit the movement of the new divider    
      if(p.x < leftLimit)
         p.x = leftLimit;
      if(p.x > rightLimit)
         p.x = rightLimit; 
                  	      
   // Which divider is this?
      
      for(divider = 1; divider < ep->cols*ep->rows; divider++)
      {
         GetSubWindowRect(obj->hwndParent,ep->editData[divider]->edWin,&winRect);
         if(abs(winRect.left-originalX) < 6)
         {
            for(int i = 0; i < ep->cols*ep->rows; i++)
            {
               ed = ep->editData[i];
               if(i == divider-1)
               {
                  GetSubWindowRect(obj->hwndParent,ed->edWin,&winRect);
                  x = winRect.left;  
                  y = winRect.top;    
                  w = p.x + 1 - winRect.left;   
                  h = winRect.bottom - winRect.top;             
		            MoveWindow(ed->edWin,x,y,w,h,false);
		            ed->x = (x-obj->xo)/(float)obj->wo;
		            ed->y = (y-obj->yo)/(float)obj->ho;
		            ed->w = w/(float)obj->wo;
		            ed->h = h/(float)obj->ho;
 		         }
               if(i == divider)
               {
                  GetSubWindowRect(obj->hwndParent,ed->edWin,&winRect);
                  x = p.x + 1;   
                  y = winRect.top; 
                  w = winRect.right - p.x - 1;  
                  h = winRect.bottom - winRect.top;                    
		            MoveWindow(ed->edWin,x,y,w,h,false);
		            ed->x = (x-obj->xo)/(float)obj->wo;
		            ed->y = (y-obj->yo)/(float)obj->ho;
		            ed->w = w/(float)obj->wo;
		            ed->h = h/(float)obj->ho;		            
 		         } 		      
 		      }
 		   }
 	   }	 		   	            
   }
// Finished moving the horizontal divider  so resize sub-windows      
   else if(cursor_mode == HORIZONTAL_CURSOR)
   {
     // Limit the movement of the new divider       
      if(p.y < topLimit)
         p.y = topLimit;
      if(p.y > bottomLimit)
         p.y = bottomLimit;
                  	      
   // Which divider is this?
      
      for(divider = ep->cols; divider < ep->rows*ep->cols; divider++)
      {
         GetSubWindowRect(obj->hwndParent,ep->editData[divider]->edWin,&winRect);
         if(abs(winRect.top - originalY) < 6)
         {
            for(int i = 0; i < ep->rows*ep->cols; i++)
            {
               ed = ep->editData[i];            
               if(i == divider-ep->cols)
               {
                  GetSubWindowRect(obj->hwndParent,ed->edWin,&winRect);
                  x = winRect.left;
                  y = winRect.top;
                  w = winRect.right - winRect.left;
                  h = p.y + 1 - winRect.top;                       
		            MoveWindow(ed->edWin,x,y,w,h,false);
		            ed->x = (x-obj->xo)/(float)obj->wo;
		            ed->y = (y-obj->yo)/(float)obj->ho;
		            ed->w = w/(float)obj->wo;
		            ed->h = h/(float)obj->ho;			            
 		         }
               if(i == divider)
               {
                  GetSubWindowRect(obj->hwndParent,ed->edWin,&winRect);
                  x = winRect.left;
                  y = p.y + 1;
                  w = winRect.right - winRect.left;
                  h = winRect.bottom - p.y - 1;                       
		            MoveWindow(ed->edWin,x,y,w,h,false);
		            ed->x = (x-obj->xo)/(float)obj->wo;
		            ed->y = (y-obj->yo)/(float)obj->ho;
		            ed->w = w/(float)obj->wo;
		            ed->h = h/(float)obj->ho;			            
 		         } 		      
 		      }
 		   }
 	   }
   } 
	SendMessage(hwnd, WM_SETREDRAW, true, 0);                     		            
 	MyInvalidateRect(ep->parent->hwndParent,NULL,false);    
}

/***************************************************************************
  Find and replace dialog callback procedure
****************************************************************************/ 

static long frx=-1,fry=-1;



BOOL CALLBACK FindReplaceProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   static CText findTxt; 
   CText replaceTxt;
   long startSel,endSel;
   long wrapPos;
   HWND focWin;
      
   if(!curEditor)
      return(0);

   switch(message) 
   {
   // Add any selected text to the "find" text control
   // Place the window in the centre of the screen or previous position
      case(WM_INITDIALOG):
      {
         char *seltxt = curEditor->GetSelection(startSel, endSel);

         if(seltxt[0] != '\0')
	         SetWindowText(GetDlgItem(hWnd,ID_FIND_TEXT),seltxt);
         else
	         SetWindowText(GetDlgItem(hWnd,ID_FIND_TEXT),findTxt.Str());

         SendDlgItemMessage(hWnd,ID_FIND_TEXT,EM_SETSEL,0,-1);
	      SetFocus(GetDlgItem(hWnd,ID_FIND_TEXT));

	      delete [] seltxt;
	      
	      if(frx == -1) // Put in centre of screen
	      {
            PlaceDialogOnTopInCentre(hWnd);
            SaveDialogParent(hWnd);
         }	
         else // Place in old position
         {
            PlaceDialogAtPosition(hWnd,frx,fry);
//          SetWindowPos(hWnd,HWND_TOPMOST,frx,fry,0,0,SWP_NOSIZE);
            AddToDiagList(hWnd);
            SaveDialogParent(hWnd);
         }
         break;
      }

      case(WM_ACTIVATE): // Make sure current window is correct
      {      
         if(LOWORD(wParam) != WA_INACTIVE) 
         { 
            currentAppWindow = hWnd;
            isModelessDialog = true;
         }
         else
         {
            isModelessDialog = false; 
         }                   
         break;
      }
            
   // Process the button commands               
      case(WM_COMMAND):
      {
         if(HIWORD(wParam) == GETMESSAGE) // Prospa requested a new find word (2.2.6)
         {
            char *seltxt = curEditor->GetSelection(startSel, endSel);

            if(seltxt[0] != '\0') // Load the new find word and give window focus
            {
	            SetWindowText(GetDlgItem(hWnd,ID_FIND_TEXT),seltxt);
            }
            SendDlgItemMessage(hWnd,ID_FIND_TEXT,EM_SETSEL,0,-1);
            SetFocus(GetDlgItem(hWnd,ID_FIND_TEXT));
         }
         else
         {
            switch(LOWORD(wParam))
            {
               case(ID_CANCEL):
                  DestroyWindow(hWnd);   
                  break; 
               case(ID_FIND_TEXT_BUTTON):
               {
                  focWin = GetFocus();
                  wrapPos = curEditor->initCharInsertionPnt;
	               GetWindowTextEx(GetDlgItem(hWnd,ID_FIND_TEXT),findTxt);            
                  if(findTxt[0] != '\0')  
                  {         
                     bool ignoreCase = SendDlgItemMessage(hWnd,ID_FINDREPLACE_CASE,BM_GETCHECK,0,0);
                     short wrap = SendDlgItemMessage(hWnd,ID_FINDREPLACE_WRAP,BM_GETCHECK,0,0)*2-1; // -1 or 1
                     if(SendDlgItemMessage(hWnd,ID_FINDREPLACE_SEARCHUP,BM_GETCHECK,0,0) == BST_CHECKED)
                        curEditor->FindTextAndSelectUp(hWnd,findTxt.Str(),ignoreCase, wrapPos*wrap);                
                     else
                        curEditor->FindTextAndSelect(hWnd,findTxt.Str(), ignoreCase, wrapPos*wrap);                      
                  }
	               SetFocus(focWin);               
                  break;    
               } 
               case(ID_REPLACE_TEXT_BUTTON):
               {      
                  focWin = GetFocus();            
                  wrapPos = curEditor->initCharInsertionPnt;                 
	               GetWindowTextEx(GetDlgItem(hWnd,ID_FIND_TEXT),findTxt);            
	               GetWindowTextEx(GetDlgItem(hWnd,ID_REPLACE_TEXT),replaceTxt);            
                  if(findTxt[0] != '\0')
                  {
                     bool ignoreCase = SendDlgItemMessage(hWnd,ID_FINDREPLACE_CASE,BM_GETCHECK,0,0); 
                     short wrap = SendDlgItemMessage(hWnd,ID_FINDREPLACE_WRAP,BM_GETCHECK,0,0)*2-1; // -1 or 1                               
                     if(SendDlgItemMessage(hWnd,ID_FINDREPLACE_SEARCHUP,BM_GETCHECK,0,0) == BST_CHECKED)
                        curEditor->ReplaceAndFindTextUp(hWnd,replaceTxt.Str(), findTxt.Str(), ignoreCase, wrapPos*wrap);  
                     else
                        curEditor->ReplaceAndFindText(hWnd,replaceTxt.Str(), findTxt.Str(), ignoreCase, wrapPos*wrap); 
                     SetEditTitle();   
                  } 
	               SetFocus(focWin);                              
                  break;    
               }                                                         
               default:
                  break;
            }
         }
         return(1); 
      }
    // Process the close window command  
      case(WM_DESTROY):
      {
         DialogTidyUp(hWnd,false);
         findReplaceHwnd = NULL;
         return(1);
      } 

    // Record the window position if it gets moved  
      case(WM_MOVE):
      {
         frx = LOWORD(lParam);
         fry = HIWORD(lParam);
         return(1);
      }                   
   } 
  return(0);
}

int SetOrGetCurrentEditor(Interface *itfc, char args[])
{
   int nrArgs;
   ObjectData *edParent;
 	CArg carg;

	if(!curEditor)
	{
   	if(itfc->inCLI)
		{
			ErrorMessage("No editor defined");
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

   if(nrArgs == 0)
	{
      edParent = curEditor->edParent->parent;
      itfc->retVar[1].MakeClass(OBJECT_CLASS,(void*)edParent);
      itfc->nrRetValues = 1;
      return(OK);
	}
	else
	{
		ErrorMessage("Unsupported option");
		return(ERR);
	}
}


int EditorFunctions(Interface* itfc ,char args[])
{
   CText func;
   short i,r;

   if((r = ArgScan(itfc,args,0,"command","e","t",&func)) < 0)
      return(r);

   if(!curEditor)
   {   
      ErrorMessage("No current editor selected");
      return(ERR);
   }

// Make the first editor in the current window the current editor
// if the current window doesn't contain the current editor
// This is because the following functions are usually called from  
// the gui and we expect them to apply to the window they were
// called from. (ANY EXCEPTIONS?)
 

   HWND hWnd = curEditor->edWin;
   EditParent *ep = curEditor->edParent;
   WinData *parent = ep->parent->winParent;

   if(itfc->win && (itfc->win->hWnd != parent->hWnd))
   {
      ObjectData *obj = itfc->win->widgets.findByType(TEXTEDITOR);
      if(obj)
      {
         ep = (EditParent*)obj->data;
         if(ep)
         {
            int nrReg = ep->cols*ep->rows;
            for(i = 0 ; i < nrReg; i++)
            {
               if(ep->editData[i] == curEditor)
                  break;
            }

            if(i == nrReg) // Current plot not found
            {
          // Set current plot to first region in edit parent
               curEditor = ep->editData[0];
            }
         }
      }
   }


   if(func == "open macro")
   {
      char pathBak[MAX_PATH];
      char nameBak[MAX_PATH];
      bool alreadyLoaded = false;

      strcpy(pathBak,curEditor->edPath);
      strcpy(nameBak,curEditor->edName);

    // Check to see if current text needs saving 
      if(curEditor->CheckForUnsavedEdits(ep->curRegion) == IDCANCEL)
         return(OK);

    // Get the filename for the new text  
      if(LoadEditDialog(hWnd, curEditor->edPath, curEditor->edName) != OK)
         return(OK);

      if(IsEditFileAlreadyLoaded(curEditor) >= 0) 
      {
         alreadyLoaded = true;
         if(YesNoDialog(MB_ICONWARNING,0,"Warning","File already loaded\rDo you want to load a read-only copy?") == IDNO)
         {
            strcpy(curEditor->edPath,pathBak); // Restore file name
            strcpy(curEditor->edName,nameBak);
            return(OK);
         }
      }

	// Load macro into editor
	   SetCursor(LoadCursor(NULL,IDC_WAIT));
      LoadEditorGivenPath(curEditor->edPath,curEditor->edName);     
		SetCursor(LoadCursor(NULL,IDC_ARROW));
   // Save pathname to linked list for use in pop-up menu
      AddFilenameToList(procLoadList,curEditor->edPath, curEditor->edName);
      curEditor->currentProc = ""; // Reset procname (2.2.6)
   // Check to see if file already loaded
      if(alreadyLoaded)
         curEditor->readOnly = true;
    //  else
    //     curEditor->readOnly = false;
      SetEditTitle(); 
   }

   else if(func == "save macro as")
   {
      short edNr;
      char pathBak[MAX_PATH];
      char nameBak[MAX_PATH];

      if(curEditor->readOnly) // (2.2.6)
         return(OK);

      strcpy(pathBak,curEditor->edPath);
      strcpy(nameBak,curEditor->edName);

   // Get new name for macro
      if(SaveEditDialog(hWnd, curEditor->edPath, curEditor->edName) != OK)
      {
         itfc->retVar[1].MakeAndSetString("cancel");
         itfc->nrRetValues = 1;
         return(OK);
      }

   // See if the macro is already loaded
      if((edNr = IsEditFileAlreadyLoaded(curEditor)) >= 0) // (2.2.7)
      {
         if(YesNoDialog(MB_ICONWARNING,0,"Warning","File already loaded with this name.\rDo you want overwrite this\rand make the loaded file read-only?") == IDNO)
         {
            strcpy(curEditor->edPath,pathBak); // Restore file name
            strcpy(curEditor->edName,nameBak);
            return(OK);
         }
         ep->editData[edNr]->readOnly = true;
       //  SetEditTitle(edNr); 
      }

  // Save with this name
      curEditor->SaveEditContents();
   // Save pathname to linked list for use in pop-up menu
      AddFilenameToList(procLoadList,curEditor->edPath, curEditor->edName);
      itfc->retVar[1].MakeAndSetString("ok");
      itfc->nrRetValues = 1;
      return(OK);
   }

   else if(func == "save macro")
   {
      if(curEditor->readOnly) // (2.2.6)
         return(OK);
      if (curEditor->SaveEditContents() == ERR)
      {
         itfc->retVar[1].MakeAndSetString("cancel");
         itfc->nrRetValues = 1;
         return(OK);
      }
      else  // Save pathname to linked list for use in pop-up menu
      {
         AddFilenameToList(procLoadList, curEditor->edPath, curEditor->edName);
         itfc->retVar[1].MakeAndSetString("ok");
         itfc->nrRetValues = 1;
         return(OK);
      }
   }
   else if(func == "save open sessions")
   {
      if(SaveWinEditSessions(parent) == IDCANCEL)
         itfc->retVar[1].MakeAndSetString("cancel");
      else
         itfc->retVar[1].MakeAndSetString("ok");
      itfc->nrRetValues = 1;
   }
   else if(func == "print macro")
   {
      PrintText(hWnd);
   }
   else if(func == "show fault")
   {
      ShowError(ep,hWnd);
   }
   else if(func == "close macro")
   {
      int response = IDNO;
      
      if(curEditor->edModified)
         response = QueryDialog(MB_ICONWARNING, "Warning","Do you want to save the current macro?");
      
      if(response == IDYES) // Save the data
      {
          if(curEditor->SaveEditContents() == ERR)
             return(OK);
      }

      if(response == IDNO || response == IDYES) // Clear the editor
      {         
         long length = SendMessage(curEditor->edWin,WM_GETTEXTLENGTH,0,0) + 1;
         SetEditSelection(curEditor->edWin,0,length);            
         SendMessage(curEditor->edWin,WM_CLEAR,(WPARAM)0,(LPARAM)0);
         SetColor(curEditor->edWin,RGB(0,0,0));            
         curEditor->edModified = false;
         curEditor->ResetEditorUndoArray();	 
         curEditor->ResetEditorJumpArray();	 
         curEditor->syntaxColoringStyle = MACRO_STYLE;
         curEditor->edName[0] = '\0';
         curEditor->edPath[0] = '\0';
         strcpy(curEditor->edName,"untitled");
         curEditor->currentProc = ""; // Clear proc name (2.2.6)
         curEditor->readOnly = false; // (2.2.6)
         SetEditTitle();         	            
      }
   }
   else if(func == "close all macros")
   {
    // Check to see if current text needs saving 
 		for(int i = 0; i < ep->cols*ep->rows; i++) 
		{  
         short response = ep->editData[i]->CheckForUnsavedEdits(i);
         if(response == IDYES)
         { 
            if(ep->editData[i]->SaveEditContents() == ERR)
               return(OK);
         }
         if(response == ID_CANCEL)
            return(OK);
		}

      MakeEditWindows(false,ep,1,1,-1);	
      ResizeEditSubwindows(ep);
   }
   else if(func == "select all")
   {
      long length;
      length = SendMessage(curEditor->edWin,WM_GETTEXTLENGTH,0,0) + 1;
      SetEditSelection(curEditor->edWin,0L,length);            
      SetFocus(curEditor->edWin);         
   }
   else if(func == "indent text")
   {
      IndentText(0);
   }
   else if(func == "unindent text")
   {
      UnIndentText(0);
   }
   else if (func == "sort selection")
   {
      curEditor->SortSelection();
   }
   else if (func == "sort procedures")
   {
       curEditor->SortProcedures();
   }
   else if(func == "copy")
   {
      SendMessage(hWnd,WM_COPY,(WPARAM)0,(LPARAM)0);
   }
   else if(func == "paste")
   {
      curEditor->CopySelectionToUndo(PASTE_TEXT,0);
      curEditor->Paste();
      if(!curEditor->edModified)
      {
         curEditor->edModified = true;
         //SendMessageToGUI("Editor,Modified",-1); 
         SetEditTitle(); 
      }
      SetFocus(curEditor->edWin);
   }
   else if(func == "cut")
   {
      curEditor->CopySelectionToUndo(CUT_TEXT,0);
      curEditor->Cut();
      if(!curEditor->edModified)
      {
         curEditor->edModified = true;
         //SendMessageToGUI("Editor,Modified",-1); 
         SetEditTitle(); 
      }
   }
   else if(func == "undo")
   {
      curEditor->CopyUndoToSelection();
      if(curEditor->edModified && curEditor->undoIndex == 0) // All undos done!
      {
         curEditor->edModified = false;
         SendMessageToGUI("Editor,Unmodified",-1); 
         SetEditTitle(); 
      }  
   }
   else if(func == "increment font size")
   {
      editFontSize++;
      if(editFontSize > 18) editFontSize = 18; 
      DeleteObject(editFont);
      HDC hdc = GetDC(hWnd);
      editFont = MakeFont(hdc, "Courier New", editFontSize, 0, 0, 0); 
      ReleaseDC(hWnd,hdc);
      MakeEditWindows(true,curEditor->edParent,curEditor->edParent->rows,curEditor->edParent->cols,-1); 
      ResizeEditSubwindows(curEditor->edParent);
   }
   else if(func == "decrement font size")
   {
      editFontSize--;
      if(editFontSize < 6) editFontSize = 6; 
      DeleteObject(editFont);
      HDC hdc = GetDC(hWnd);
      editFont = MakeFont(hdc, "Courier New", editFontSize, 0, 0, 0); 
      ReleaseDC(hWnd,hdc);
      MakeEditWindows(true,curEditor->edParent,curEditor->edParent->rows,curEditor->edParent->cols,-1);
      ResizeEditSubwindows(curEditor->edParent);
   }
   else if(func == "toggle full path")
   {
   //   HMENU hMenu = GetMenu(hWnd);

		if(showFullName)
		{
	//	   CheckMenuItem(hMenu,ID_SHOW_EDIT_PATH,MF_UNCHECKED);
		   showFullName = false;
		}
		else
		{
	//	   CheckMenuItem(hMenu,ID_SHOW_EDIT_PATH,MF_CHECKED);
		   showFullName = true;
		} 
		SetEditTitle();
   }
   else if(func == "multiedit 1*1")
   {
      MakeEditWindows(true,ep,1,1,curEditor->regNr);	
      ResizeEditSubwindows(ep);
   }
   else if(func == "multiedit 2*1")
   {
      MakeEditWindows(true,ep,1,2,-1);	
      ResizeEditSubwindows(ep);
   }
   else if(func == "multiedit 1*2")
   {
      MakeEditWindows(true,ep,2,1,-1);	
      ResizeEditSubwindows(ep);
   }
   else if(func == "multiedit 3*1")
   {
      MakeEditWindows(true,ep,1,3,-1);	
      ResizeEditSubwindows(ep);
   }
   else if(func == "multiedit 2*2")
   {
      MakeEditWindows(true,ep,2,2,-1);	
      ResizeEditSubwindows(ep);
   }
   else if(func == "multiedit m*n")
   {
      extern short gMultiEditXCells,gMultiEditYCells;
      extern int CALLBACK MultiEditDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

       gMultiEditYCells = ep->rows;
       gMultiEditXCells = ep->cols;	   
   	 if(DialogBox(prospaInstance,"MULTIEDITDLG",hWnd,MultiEditDlgProc))
       {
          return(OK);	  
       }
      MakeEditWindows(true,ep,gMultiEditYCells,gMultiEditXCells,-1);	
      ResizeEditSubwindows(ep);     	           
   }
   else if(func == "command help")
   {
       GetCommandHelp(itfc,hWnd);
   }
   else if(func == "run text")
   {
      char *text = GetText(hWnd);
      itfc->inCLI = false;
      if(itfc->win) // Make sure caching is turned off 
          itfc->win->cacheProc = false;
      ProcessMacroStr(GLOBAL,NULL,NULL,text,"","","current_text","");
      ChangeGUIParent(hWnd);
      delete [] text;
   }
   else if(func == "save and run text")
   {
      if(!strcmp(curEditor->edName,"untitled"))
      {
         if (SaveEditDialog(hWnd, curEditor->edPath, curEditor->edName) != OK)
         {
            itfc->retVar[1].MakeAndSetString("cancel");
            itfc->nrRetValues = 1;
            return(OK);
         }
      }

      char *text = GetText(hWnd);

      if(!curEditor->readOnly) // (2.2.6)
      {
         if(SaveTextFile(curEditor->edPath,curEditor->edName,text) == ERR) 
             return(OK);
      }
      curEditor->edModified = false;
      SetEditTitle();         	            
      curEditor->ResetEditorUndoArray();	 
      curEditor->ResetEditorJumpArray();	

   // Save the filename 
      AddFilenameToList(procRunList,curEditor->edPath, curEditor->edName);
             
    // Run macro
      itfc->inCLI = false;
	//   SetCurrentDirectory(curEditor->edPath); 
      itfc->varScope = GLOBAL;
      itfc->debugging = false;
      itfc->startLine = 0;
      itfc->nrProcArgs = 0;
      if(itfc->win) // Make sure caching is turned off 
          itfc->win->cacheProc = false;
      ProcessMacroStr(itfc,text,curEditor->edName,curEditor->edPath);
      ChangeGUIParent(hWnd);
      delete [] text;
      itfc->retVar[1].MakeAndSetString("ok");
      itfc->nrRetValues = 1;
      return(OK);
   }
   else if(func == "debug text")
   {
      char *text = GetText(hWnd);
      itfc->inCLI = false;
      itfc->varScope = GLOBAL;
      itfc->debugging = true;
      gDebug.mode = "stepinto";
      gDebug.step = false;
      itfc->startLine = 0;
      itfc->nrProcArgs = 0;
      ProcessMacroStr(itfc,text,"","");
      if(itfc->debugging)
      {
         SendMessage(curEditor->edWin,EM_HIDESELECTION ,true,0);
         itfc->debugging = false;
         SendMessageToGUI("Debug,Finished",0);
      }
      ChangeGUIParent(hWnd);
      delete [] text;
   }
   else if(func == "debug file")
   {
		extern bool gDebugBlockWaitCursor;
    // Check to see if current text needs saving 
      if(curEditor->CheckForUnsavedEdits(ep->curRegion) == IDCANCEL)
         return(OK);

      char *text = GetText(hWnd);

      //if(!curEditor->readOnly) // (2.2.6)
      //{
      //   if(SaveTextFile(curEditor->edPath,curEditor->edName,text) == ERR) 
      //       return(OK);
      //}
      curEditor->edModified = false;
      SetEditTitle();         	            
      curEditor->ResetEditorUndoArray();	 
      curEditor->ResetEditorJumpArray();	

   // Save the filename 
      AddFilenameToList(procRunList,curEditor->edPath, curEditor->edName);
             
      itfc->inCLI = false;
      itfc->varScope = GLOBAL;
      itfc->debugging = true;
      gDebug.mode = "stepinto";
      gDebug.step = false;
      itfc->startLine = 0;
      itfc->nrProcArgs = 0;
      gKeepCurrentFocus = true;
		gDebugBlockWaitCursor = true;
      ProcessMacroStr(itfc,text,curEditor->edName,curEditor->edPath);
      gKeepCurrentFocus = false;
      if(curEditor)
      {
         SendMessage(curEditor->edWin,EM_HIDESELECTION ,true,0);
         curEditor->debugLine = -1;
         DrawDebugBreakPointStrip(curEditor);
      }
      itfc->debugging = false;
		gDebugBlockWaitCursor = false;
    //  SendMessageToGUI("Debug,Finished",0);
    //  gDebug.mode = "off";
      ChangeGUIParent(hWnd);
      delete [] text;
   }
   else if(func == "go to procedure" || func == "go to procedure use new editor")
   {
	   extern CText *gClassMacro, *gClassName;
   	extern int gNrClassDeclarations;
      long startWord,endWord;
      char macroName[MAX_PATH];
      char procName[MAX_PATH];	  
      char editMacro[MAX_PATH];    
      char *name;
		bool classCmd=false;
		short userClassCmd=0;
      GetEditSelection(curEditor->edWin,startWord,endWord);	      
      name = ExpandToFullWord(curEditor->edWin,BEFORE_PROCEDURE,AFTER_PROCEDURE,startWord,endWord,classCmd,userClassCmd);
      strcpy(macroName,name);
		if(!userClassCmd)
		{
			ExtractProcedureName(macroName,procName);
			strcpy(editMacro,curEditor->edName);
			RemoveExtension(editMacro);
		}
		else
		{
			if(userClassCmd == 1) // Ignore class member variables
			{
				delete [] name;
				return(OK);
			}

			ExtractClassProcName(macroName,procName);
			if(!strcmp(macroName,"self"))
				macroName[0] = '\0';
			else
			{
				int n = gNrClassDeclarations;
				for(int i = 0; i < n; i++)
				{
					if(gClassName[i] == macroName)
					{
						strcpy(macroName,gClassMacro[i].Str());
						break;
					}
				}
			}

		}
		delete [] name;	

    // Look in current editor
      if(macroName[0] == '\0' || !strcmp(macroName,editMacro))
      {
         curEditor->SelectProcedure(procName);
      }
   // Load new macro and show procedure
      else
      {
         char extension[MAX_PATH];
         int err = 0;
         bool hasExtension = false;
         strncpy(extension,GetExtension(macroName),MAX_PATH);
         if(strlen(extension) == 0)
             strcat(macroName,".mac");
         else
            hasExtension = true;
         short reg = IsEditFileAlreadyLoaded(curEditor,"",macroName);
         if(reg < 0) // New file so load it
         {
				if(func == "go to procedure use new editor")
				   reg = FindEmptyEditor(curEditor);
				if(reg < 0)
				{
               if(!(err = curEditor->LoadAndSelectProcedure(macroName,procName, false)))
                 AddFilenameToList(procLoadList,curEditor->edPath, curEditor->edName);
				}
				else
				{
					 EditParent *ep = curEditor->edParent;
                EditRegion *ed = ep->editData[reg];
                if(!(err = ed->LoadAndSelectProcedure(macroName,procName, false)))
                  AddFilenameToList(procLoadList,curEditor->edPath, curEditor->edName);
				}
         }
         else // Already loaded
         {
            EditParent *ep = curEditor->edParent;
            EditRegion *ed = ep->editData[reg];
            ed->SelectProcedure(procName);
         }
         // Try the pex file if no extension supplied
         if(!hasExtension && err)
         {
            err = 0;
            strncpy(extension,GetExtension(macroName),MAX_PATH);
            if(strlen(extension) == 0)
                strcat(macroName,".pex");
            else
            {
               RemoveExtension(macroName);
               strcat(macroName,".pex");
            }

            short reg = IsEditFileAlreadyLoaded(curEditor,"",macroName);
            if(reg < 0) // New file so load it
            {
               if(!(err = curEditor->LoadAndSelectProcedure(macroName,procName, true)))
                  AddFilenameToList(procLoadList,curEditor->edPath, curEditor->edName);
            }
            else // Already loaded
            {
               EditParent *ep = curEditor->edParent;
               EditRegion *ed = ep->editData[reg];
               ed->SelectProcedure(procName);
            }
         }

      // Make a breakpoint strip if in debug mode
         if(!err && curEditor->debug)
         {
            UpdateDebugBreakPointList(curEditor);
         }
      }   
   }
   else if(func == "go forward")
   {
      if(curEditor->CheckForUnsavedEdits(ep->curRegion) != IDCANCEL)
      {
         curEditor->SelectNextProcedure();
         if(curEditor->debug)
         {
            UpdateDebugBreakPointList(curEditor);
         }
      }
   }
   else if(func == "go back")
   {
      if(curEditor->CheckForUnsavedEdits(ep->curRegion) != IDCANCEL)
      {
	      curEditor->SelectLastProcedure();
         if(curEditor->debug)
         {
            UpdateDebugBreakPointList(curEditor);
         }
      }
   }
   else if(func == "go to line")
   {
      DialogBox(prospaInstance,"LINENRDLG",hWnd,LineNumberDlgProc);
      SetFocus(curEditor->edWin);        
   }
   else if(func == "block comment")
   {
      BlockComment(0);
   }
   else if(func == "block uncomment")
   { 
      BlockUncomment(0);	
   }
   else if(func == "find down")
   { 
      long startWord,endWord;      
   	char *name;
		bool classCmd=false;
		short userClassCmd=0;

      GetEditSelection(curEditor->edWin,startWord,endWord);
      name = ExpandToFullWord(curEditor->edWin,BEFORE_OPERAND,AFTER_OPERAND,startWord,endWord,classCmd,userClassCmd);
      endWord++;
      if(curEditor->FindNextWord(name,endWord,1) == OK) // Next word found so select it
      {
         SetEditSelection(curEditor->edWin,endWord,endWord+strlen(name));
         SetFocus(curEditor->edWin);
      }
      else //Can't find next word
      {
         MessageBeep(MB_OK);
      }
      delete [] name;
   }
   else if(func == "find up")
   { 
      long startWord,endWord;      
   	char *name;
		bool classCmd=false;
		short userClassCmd=false;

      GetEditSelection(curEditor->edWin,startWord,endWord);
      name = ExpandToFullWord(curEditor->edWin,BEFORE_OPERAND,AFTER_OPERAND,startWord,endWord,classCmd,userClassCmd);
      
      if(curEditor->FindLastWord(name,endWord,1) == OK) // Last word found so select it
      {
         SetEditSelection(curEditor->edWin,endWord,endWord+strlen(name));	      
         SetFocus(curEditor->edWin);
      }
      else //Can't find next word
      {
         MessageBeep(MB_OK);
      }
      delete [] name;
   }  
   else if(func == "find and replace")
   { 
      if(!findReplaceHwnd)
         findReplaceHwnd = CreateDialog(prospaInstance,"FINDREPLACEDLG",hWnd,FindReplaceProc);
      else
		{
			PostMessage(findReplaceHwnd, WM_CLOSE, 0, 0);
         findReplaceHwnd = CreateDialog(prospaInstance,"FINDREPLACEDLG",hWnd,FindReplaceProc);
		}

   }
   return(OK);
}


void ShowError(EditParent *ep, HWND hWnd)
{
   char *text = NULL;

   if(!curEditor)
      return;

   if(gErrorInfo.lineNr >= 0) // Is there an error?
   {
      if(curEditor->CheckForUnsavedEdits(ep->curRegion) == IDCANCEL)
         return;
               
      if(gErrorInfo.macro == "current_text") // Was the macro run from the current editor?
      {
         text = GetText(curEditor->edWin);
      }	
      else
      {
         char pathBak[MAX_PATH];
         char nameBak[MAX_PATH];
         bool alreadyLoaded = false;

      // Make a backup of current editor file
         strcpy(pathBak,curEditor->edPath);
         strcpy(nameBak,curEditor->edName);

      // Save file name and path

         short reg = IsEditFileAlreadyLoaded(curEditor,gErrorInfo.path.Str(),gErrorInfo.macro.Str());
         if(reg >= 0)
         {
            EditParent *ep = curEditor->edParent;
            curEditor = ep->editData[reg];
				curEditor->SelectEditorLine(gErrorInfo.lineNr);	   	   
            SetFocus(curEditor->edWin);
            return;
         }
         else
         {
            strcpy(curEditor->edName,gErrorInfo.macro.Str()); // Update the editor name/path
            strcpy(curEditor->edPath,gErrorInfo.path.Str());

            text = LoadTextFileFromFolder(curEditor->edPath, curEditor->edName,".mac"); // Load macro into editor
            if(text)
            {
            // Display wait cursor
               SetCursor(LoadCursor(NULL,IDC_WAIT));
            // Set syntax colouring mode
               curEditor->syntaxColoringStyle = MACRO_STYLE;
            // Display the text in the editor
               curEditor->CopyTextToEditor(text);                               
            // Restore cursor
	            SetCursor(LoadCursor(NULL,IDC_ARROW));
            // Check to see if file already loaded
               if(alreadyLoaded) // (2.2.7)
                  curEditor->readOnly = true;
               else
                  curEditor->readOnly = false;
               SetEditTitle(); // Update title
            }
         }
      }

      // Search macro for procedure where error occurred and then add error line number to this
      if(text)
      {
      // Simplify file line endings (since editor only uses \r)
         ReplaceSpecialCharacters(text,"\r\n","\r",-1);
         
         //if(macroErrorProc[0] != '\0') // Error procedure is defined so line number is relative to procedure start
         //{
         //   long charPos = FindProcedurePosition(text,macroErrorProc);
         //   if(charPos == -1)
         //   {
         //      ErrorMessage("can't find error line");
         //      return;
         //   }
         //   long lineNr = SendMessage(curEditor->edWin,EM_LINEFROMCHAR,(WPARAM) charPos, (LPARAM)0);	
         //   curEditor->SelectEditorLine(lineNr+macroErrorLine);	  
         //   SetFocus(curEditor->edWin);			         
         //} 
         //else // No procedure defined so line number must be absolute
         {
				curEditor->SelectEditorLine(gErrorInfo.lineNr);	   	   
            SetFocus(curEditor->edWin);	
         }	
         delete [] text;
      }	
   }
}

void BlockComment(short mode)
{
   long index;
   long startSel,endSel;
   long startLine,endLine;
  
   if(!curEditor || curEditor->readOnly) // (2.2.6)
      return;

   SendMessage(curEditor->edWin, WM_SETREDRAW, false, 0);
   GetEditSelection(curEditor->edWin,startSel,endSel);
   startLine = SendMessage(curEditor->edWin,EM_LINEFROMCHAR,(WPARAM)startSel,(LPARAM)0);
   endLine = SendMessage(curEditor->edWin,EM_LINEFROMCHAR,(WPARAM)endSel-1,(LPARAM)0);

   if(mode == 0)
      curEditor->CopySelectionToUndo(COMMENT_TEXT,0);         
   for(short i = startLine; i <= endLine; i++)
   {
      index = SendMessage(curEditor->edWin,EM_LINEINDEX,(WPARAM)i,(LPARAM)0);
      SetEditSelection(curEditor->edWin,index,index);
      SendMessage(curEditor->edWin,EM_REPLACESEL,true,(LPARAM)(LPCTSTR)"#");
   }  
   startSel = SendMessage(curEditor->edWin,EM_LINEINDEX,(WPARAM)startLine,(LPARAM)0);            
   endSel = SendMessage(curEditor->edWin,EM_LINEINDEX,(WPARAM)endLine+1,(LPARAM)0);
   SetEditSelection(curEditor->edWin,startSel,endSel);         
   curEditor->ContextColorLines(startLine,endLine);            
  
   SendMessage(curEditor->edWin, WM_SETREDRAW, true, 0);
   MyInvalidateRect(curEditor->edWin,0,true);
   if(!curEditor->edModified)
   {
      curEditor->edModified = true;
      //SendMessageToGUI("Editor,Modified",-1); 
      SetEditTitle(); 
   }
}

void BlockUncomment(short mode)
{
   long index;
   long startSel,endSel;
   long startLine,endLine;

   if(!curEditor || curEditor->readOnly) // (2.2.6)
      return;

   SendMessage(curEditor->edWin, WM_SETREDRAW, false, 0);
   GetEditSelection(curEditor->edWin,startSel,endSel);         
   startLine = SendMessage(curEditor->edWin,EM_LINEFROMCHAR,(WPARAM)startSel,(LPARAM)0);
   endLine   = SendMessage(curEditor->edWin,EM_LINEFROMCHAR,(WPARAM)endSel-1,(LPARAM)0);

   if(mode == 0)			
      curEditor->CopySelectionToUndo(UNCOMMENT_TEXT,0); 			
	for(short i = startLine; i <= endLine; i++)
	{
		index = SendMessage(curEditor->edWin,EM_LINEINDEX,(WPARAM)i,(LPARAM)0);
      if(curEditor->GetCharacter(index) == '#')
      {   
         SetEditSelection(curEditor->edWin,index,index+1);              
		   SendMessage(curEditor->edWin,EM_REPLACESEL,true,(LPARAM)(LPCTSTR)"");
		}
	}  
   startSel = SendMessage(curEditor->edWin,EM_LINEINDEX,(WPARAM)startLine,(LPARAM)0);            			
	endSel = SendMessage(curEditor->edWin,EM_LINEINDEX,(WPARAM)endLine+1,(LPARAM)0);
   SetEditSelection(curEditor->edWin,startSel,endSel);			
   curEditor->ContextColorLines(startLine,endLine);   				

	SendMessage(curEditor->edWin, WM_SETREDRAW, true, 0);
	MyInvalidateRect(curEditor->edWin,0,true);
   if(!curEditor->edModified)
   {
      curEditor->edModified = true;
      //SendMessageToGUI("Editor,Modified",-1); 
      SetEditTitle(); 
   } 
}

void IndentText(short mode)
{
   long startSel,endSel;
   long startLine,endLine;
  
   if(!curEditor || curEditor->readOnly) // (2.2.6)
      return;

   SendMessage(curEditor->edWin, WM_SETREDRAW, false, 0);
   GetEditSelection(curEditor->edWin,startSel,endSel);
   startLine = SendMessage(curEditor->edWin,EM_LINEFROMCHAR,(WPARAM)startSel,(LPARAM)0);
   endLine = SendMessage(curEditor->edWin,EM_LINEFROMCHAR,(WPARAM)endSel-1,(LPARAM)0);
   if(startSel == endSel || startLine == endLine) // One line selected
   {
      if(mode == 0) // Don't undo when called from undo!         
         curEditor->CopySelectionToUndo(TAB_INSERT,0);           
      SendMessage(curEditor->edWin,EM_REPLACESEL,true,(LPARAM)(LPCTSTR)"   ");
      curEditor->ContextColorLines(startLine,startLine);                     
   }
   else
   {
      if(mode == 0) // Don't undo when called from undo!
         curEditor->CopySelectionToUndo(INDENT_TEXT,0);         
      for(short i = startLine; i <= endLine; i++)
      {
         long index = SendMessage(curEditor->edWin,EM_LINEINDEX,(WPARAM)i,(LPARAM)0);
         SetEditSelection(curEditor->edWin,index,index);                   
         SendMessage(curEditor->edWin,EM_REPLACESEL,true,(LPARAM)(LPCTSTR)"   ");
      }  
      startSel = SendMessage(curEditor->edWin,EM_LINEINDEX,(WPARAM)startLine,(LPARAM)0);            
      endSel = SendMessage(curEditor->edWin,EM_LINEINDEX,(WPARAM)endLine+1,(LPARAM)0);
      SetEditSelection(curEditor->edWin,startSel,endSel);                              
   }
   SendMessage(curEditor->edWin, WM_SETREDRAW, true, 0);
   MyInvalidateRect(curEditor->edWin,0,true);
   if(!curEditor->edModified)
   {
      curEditor->edModified = true;
      //SendMessageToGUI("Editor,Modified",-1); 
      SetEditTitle(); 
   }
}

void UnIndentText(short mode)
{
   long index;
   long startSel,endSel;
   long startLine,endLine;
   short j;

   if(!curEditor || curEditor->readOnly) // (2.2.6)
      return;

   SendMessage(curEditor->edWin, WM_SETREDRAW, false, 0);
   GetEditSelection(curEditor->edWin,startSel,endSel);
   startLine = SendMessage(curEditor->edWin,EM_LINEFROMCHAR,(WPARAM)startSel,(LPARAM)0);
   endLine   = SendMessage(curEditor->edWin,EM_LINEFROMCHAR,(WPARAM)endSel-1,(LPARAM)0);

   if(startSel == endSel || startLine == endLine) // One line selected
   {     
	   index = SendMessage(curEditor->edWin,EM_LINEINDEX,(WPARAM)startLine,(LPARAM)0);
      char* line = GetLineByPosition(curEditor->edWin, startSel);
	   for(j = 0; j < 3; j++)
	   {
		   if(line[j] != ' ') 
			   break;
	   }
	   if(j < 3)
	   {
         delete [] line;	
         SendMessage(curEditor->edWin, WM_SETREDRAW, true, 0);
	      return; // Insufficient spaces to unindent
	   }

      if(mode == 0)	// Don't undo when called from undo!		
         curEditor->CopySelectionToUndo(UNINDENT_TEXT,0);              
		   
      delete [] line;
      SetEditSelection(curEditor->edWin,index,index+3);                                          
	   SendMessage(curEditor->edWin,EM_REPLACESEL,true,(LPARAM)(LPCTSTR)"");
	   if(startSel-3 >= index)
         SetEditSelection(curEditor->edWin,startSel-3,startSel-3);                                          				
	   else
         SetEditSelection(curEditor->edWin,index,index);                                          								
   }
   else // Multiple lines selected
   {
      if(mode == 0)	// Don't undo when called from undo!		
         curEditor->CopySelectionToUndo(UNINDENT_TEXT,0); 
    
    // Do the unindent - remove 3 spaces - or less if fewer available    				
	   for(short i = startLine; i <= endLine; i++)
	   {
		   index = SendMessage(curEditor->edWin,EM_LINEINDEX,(WPARAM)i,(LPARAM)0);
		   char* line = GetLineByPosition(curEditor->edWin, index);
		   for(j = 0; j < 3; j++)
		   {
			   if(line[j] != ' ') 
				   break;
		   }
         SetEditSelection(curEditor->edWin,index,index+j);                                          													
	      SendMessage(curEditor->edWin,EM_REPLACESEL,true,(LPARAM)(LPCTSTR)"");
	   }				 
      startSel = SendMessage(curEditor->edWin,EM_LINEINDEX,(WPARAM)startLine,(LPARAM)0);            			
	   endSel = SendMessage(curEditor->edWin,EM_LINEINDEX,(WPARAM)endLine+1,(LPARAM)0);
      SetEditSelection(curEditor->edWin,startSel,endSel);				
   }
   SendMessage(curEditor->edWin, WM_SETREDRAW, true, 0);
   MyInvalidateRect(curEditor->edWin,0,true);
   if(!curEditor->edModified)
   {
      curEditor->edModified = true;
      //SendMessageToGUI("Editor,Modified",-1); 
      SetEditTitle(); 
   }
}

void ResizeEditSubwindows(EditParent *ep)
{
   RECT r;
   GetClientRect(ep->parent->hwndParent,&r);
   long width = r.right-r.left+1;
   long height = r.bottom-r.top+1;
   SendMessage(ep->parent->hwndParent,WM_SIZE,(WPARAM)SIZE_RESTORED,(LPARAM)(width+(height<<16)));       
}

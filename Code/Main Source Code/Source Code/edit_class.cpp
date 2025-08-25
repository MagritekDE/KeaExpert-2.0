#include "stdafx.h"
#include "edit_class.h"
#include <shellapi.h>
#include <vector>
#include <algorithm>
#include "cli_files.h"
#include "defineWindows.h"
#include "edit_files.h"
#include "edit_utilities.h"
#include "events_edit.h"
#include "files.h"
#include "globals.h"
#include "interface.h"
#include "main.h"
#include "message.h"
#include "ole2.h"
#include "PlotFile.h"
#include "scanstrings.h"
#include "string_utilities.h"
#include "list_functions.h"
#include "memoryLeak.h"

using std::vector;
using std::string;

#pragma warning (disable: 4311) // Ignore pointer truncation warnings
#pragma warning (disable: 4312) // Ignore Get/SetWindowLong warnings
#pragma warning (disable: 4996) // Ignore deprecated library functions

WNDPROC fnOldEdit;
//EditRegion **ep->editData;
Focus_Method gFocusForRendering = CURRENTEDITOR;

// Constructor for edit region - makes window and initializes title

EditRegion::EditRegion(EditParent *ep, short y, short x, short nrX, short nrY)
{
   RECT r;
   HWND hWnd;

	strncpy_s(edName,MAX_PATH,"untitled",_TRUNCATE);
   strncpy_s(edPath,MAX_PATH,PlotFile::getCurrMacroDirectory(),_TRUNCATE);

   edModified = false;

   readOnly = false;

   hWnd = ep->parent->hwndParent;

   HDC hdc = GetDC(hWnd);
   GetClientRect(hWnd,&r);

   short regX,regY;
   short regW,regH;
   short winWidth  = r.right-r.left;
   short winHeight = r.bottom-r.top;

   syntaxColoringStyle = MACRO_STYLE;

   regX = x*winWidth/nrX;
   regY = y*winHeight/nrY;
	regW = winWidth/nrX;
   regH = winHeight/nrY;

   regNr = x + y*nrX;

   if(ep->wordWrap)
   {
      edWin = CreateWindowEx (WS_EX_STATICEDGE, RICHEDIT_CLASS, NULL,
		                         WS_CHILD | g_objVisibility |  WS_VSCROLL |
		                         ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 
		                         regX,regY,regW,regH,
                               hWnd, NULL,
	                            prospaInstance, NULL);
   }
   else
   {
      edWin = CreateWindowEx (WS_EX_STATICEDGE, RICHEDIT_CLASS, NULL,
		                         WS_CHILD | g_objVisibility | WS_HSCROLL | WS_VSCROLL | 
		                         ES_LEFT | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL,
		                         regX,regY,regW,regH,
                               hWnd, NULL,
	                            prospaInstance, NULL);
   }
  

   ep->curWnd = edWin;
   ep->curRegion = regNr;
   edParent = ep;

// Check to see which version of editor we have (may not work with richedit 1.0?)
	SendMessage(edWin,EM_SETTYPOGRAPHYOPTIONS,TO_SIMPLELINEBREAK,TO_SIMPLELINEBREAK);
	DWORD result = SendMessage(edWin,EM_GETTYPOGRAPHYOPTIONS,0,0);
	if(result == TO_SIMPLELINEBREAK)
		richEditVersion = 3;
	else
		richEditVersion = 2;

// Set the font	   
   SendMessage(edWin,WM_SETFONT,(WPARAM)editFont,MAKELPARAM(false, 0));
   
// Set the default event procedure
   fnOldEdit = (WNDPROC)SetWindowLong(edWin,GWL_WNDPROC,(LONG)EditEventsProc); 

// Turn off auto undo (we will do it ourselves)
	SendMessage(edWin,EM_SETUNDOLIMIT,(WPARAM)0,(LPARAM)(0));
	      
// Limit text input to 16 Meg (should be big enough!)
	SendMessage(edWin,EM_EXLIMITTEXT,(WPARAM)0,(LPARAM)MAX_TEXT);

// Prevent drag and drop from OLE operations
   RevokeDragDrop(edWin);

// But allow using event commands
   DragAcceptFiles(edWin,true);

   lineHeight = 16; // Needs to be more flexible (what if font changes?)
   SetColor(edWin,RGB(0,0,0));
   jumpArray = new vector<EditJumpType>;
   undoArray = new vector<UndoType>;
   undoIndex = 0;
   jumpIndex = -1;   
   initCharInsertionPnt = -1;
   debug = false;
   debugLine = -1;
	labelCtrlNr = -1;
   ReleaseDC(hWnd,hdc);     
}

EditRegion::~EditRegion()
{
	ResetEditorUndoArray();	 
	ResetEditorJumpArray();	
   delete jumpArray;
   delete undoArray;
   jumpArray = NULL;
   undoArray = NULL;
   undoIndex = 0;
   jumpIndex = -1;
}

// Select the specified line in the edit region (if it exists)

short EditRegion::SelectLines(long start, long end)
{
   long lineStart,lineEnd,length;
   
   start--;  // Zero based line number
   end--;
   
   lineStart = SendMessage(edWin,EM_LINEINDEX,(WPARAM)(long)start,(LPARAM)0);
   lineEnd = SendMessage(edWin,EM_LINEINDEX,(WPARAM)(long)end,(LPARAM)0);
   length = SendMessage(edWin,EM_LINELENGTH,(long)lineEnd,0);
   lineEnd += length;
   
   if(lineStart == -1 || lineEnd == -1)
      return(ERR);

   SetFocus(edWin);
	SetEditSelection(edWin,lineStart,lineEnd);
   SendMessage(edWin,EM_HIDESELECTION ,false,0);

   return(OK);
}


long EditRegion::CountLines()
{
   return(SendMessage(edWin,EM_GETLINECOUNT ,(WPARAM)0,(LPARAM)0));
}

// Search for the specified command number and return the line it appears on

short EditRegion::FindCommand(long cmdNr)
{
   char *text,*cleanText;
   long length;
   short lineNr;

// Extract the current editor text
   
   length = SendMessage(edWin,WM_GETTEXTLENGTH,0,0) + 1;
   text = new char[length];
   SendMessage(edWin,WM_GETTEXT,(WPARAM)length,(LPARAM)(text));

// Remove comments and EOL markers from text

   if(RemoveCommentsFromText(text,&cleanText) == ERR)
      return(ERR);
   ReplaceEOLinText(cleanText);
   lineNr = ::CountLines(cleanText,cmdNr);
   delete [] text;   
   return(lineNr);
}

// Search for the specified string embedded in current editor window.
// Returns the line number where a match is found.
// If the string cannot be found then it is shortened until it is.

short EditRegion::FindString(char *str)
{
   if(!str)
		return(ERR);

   long len = strlen(str);

// Extract the current editor text
   
   long length = SendMessage(edWin,WM_GETTEXTLENGTH,0,0) + 1;
   char* text = new char[length];
   SendMessage(edWin,WM_GETTEXT,(WPARAM)length,(LPARAM)(text));

// Remove \r\n and replace with \r
   ReplaceSpecialCharacters(text,"\r\n","\n",length);

// Search for string or substring  
   char *ptr = NULL;
   do
   {
      ptr = strstr(text,str);
      if(ptr == NULL)
      {
         len = strlen(str);
         str[len-1] = '\0'; // Reduce length of string if not found and try again
      }
      if(len-1 == 0) // Can't find so exit
      {
         delete [] text;   
         return(ERR);
      }
   }
   while(ptr == NULL);
   
   long index = ptr - text;
   long lineNr = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)index,(LPARAM)0);

   delete [] text;   

   return(lineNr+1);
}
 
/****************************************************************************************
  Return the current selection
*****************************************************************************************/
   
char* EditRegion::GetSelection(long &startSel, long &endSel)
{
   char* name;
   
   GetEditSelection(edWin,startSel,endSel);
	if(endSel > startSel)
	{
	   name = new char[endSel-startSel+2];
	   SendMessage(edWin,EM_GETSELTEXT,(WPARAM)0,(LPARAM)name);
	}
	else
	{
	   name = new char[1];	
	   name[0] = '\0';
	}
	return(name);
}

/****************************************************************************************
  Start searching for word in text editor from position 'pos'. Return start of word in pos.
  Return OK if found ERR is not found
  If mode == 1 then ???
*****************************************************************************************/

short EditRegion::FindNextWord(char *word, long &pos, short mode)
{
   long length;
   char *ptr;
   char *text;
   long startWord,endWord;
   char *name;

// Null word or space  
   if(word[0] == '\0' || !strcmp(word," "))
      return(ERR);
      
// Extract the current editor text
   length = SendMessage(edWin,WM_GETTEXTLENGTH,0,0) + 1;
   text = new char[length];
   SendMessage(edWin,WM_GETTEXT,(WPARAM)length,(LPARAM)(text));
   ReplaceSpecialCharacters(text,"\r\n","\n",length);

// Search for string or substring  
   for(;;)
   { 
      ptr = strstr(text+pos,word);
      if(ptr == NULL)
      {
         delete [] text;
         return(ERR);
      }
      if(mode == 1) // Why do we do this?
      {      
         name = ExpandToFullWord2(text,(long)(ptr-text),BEFORE_OPERAND,AFTER_OPERAND,startWord,endWord);
         if(!strcmp(name,word))
         {
            delete [] name;
            break;
         }
         delete [] name;
         if(endWord == pos)// Not getting anywhere so abort
         {
            delete [] text;
            return(ERR);
         }
         pos = endWord;
      }
      else
      {
         if((long)(ptr-text+strlen(word)) == pos) // Not getting anywhere so abort
         {
            delete [] text;
            return(ERR);
         }         
         pos = (long)(ptr-text+strlen(word));
         break;
      }
   }

// Return start of new word   
   pos = (long)(ptr-text);
   delete [] text;   

   return(OK);
}



/*********************************************************************
  Return the name of the procedure at character location pos
********************************************************************/

short EditRegion::FindCurrentProcedure(long pos, CText &procName)
{
   long length;
   long offset1,offset2;
   char *text;
   int k;

// Extract the current editor text
   length = SendMessage(edWin,WM_GETTEXTLENGTH,0,0) + 1;
   text = new char[length];
   SendMessage(edWin,WM_GETTEXT,(WPARAM)length,(LPARAM)(text));
   ReplaceSpecialCharacters(text,"\r\n","\n",length);

// Search procedure boundaries  
   offset1 = FindReverseSubStr(text, "procedure(", pos, true,true);
   offset2 = FindReverseSubStr(text, "endproc", pos, true,true);

// See if we are outside a procedure
   if(offset1 == -1)
   {
      delete [] text;
      return(ERR);
   }

// Make sure this is a valid endproc - must be preceded by a space, semicolon or return
   if(offset1 > 0 && offset2 > offset1 && strchr(" ;\n",text[offset2-1]))
   {
      delete [] text;
      return(ERR);
   }

 // Make sure this is a valid procedure - must be preceded by a space, semicolon or return
   if(offset1 > 0 && !strchr(" ;\n",text[offset1-1]))
   {
      delete [] text;
      return(ERR);
   }

// Return start of procedure   
   pos = offset1;

// Search forward for start of argument list
   for(k = pos; k < length; k++)
   {
      if(text[k] == '(')
      {
         pos = k+1;
         break;
      }
   }
   if(k == length)
   {
      delete [] text;
      return(ERR);
   }

// Skip white space
   for(k = pos; k < length; k++)
   {
      if(text[k] != ' ' && text[k] != '\t')
         break;
   }

   if(k == length)
   {
      delete [] text;
      return(ERR);
   }

   pos = k;

// Extract procedure name
   procName.Reset();
   for(k = pos; k < length; k++)
   {
      if(text[k] == ',' || text[k] == ')' || text[k] == ' ' || text[k] == '\t')
         break;
   }
   if(k == length)
   {

      return(ERR);
   }

   procName.Assign(text+pos,k-pos);
   delete [] text;

   return(OK); 
}

/****************************************************************************************
  Start searching backwards for word in text editor from position 'pos'. Return start of word in pos.
  Return OK if found ERR is not found
*****************************************************************************************/

short EditRegion::FindLastWord(char *word, long &pos, short mode)
{
   long length;
   long offset;
   long lastoffset=-1;
   char *text;
   long startWord,endWord;
   char *name;
   
// Extract the current editor text
   length = SendMessage(edWin,WM_GETTEXTLENGTH,0,0) + 1;
   text = new char[length];
   SendMessage(edWin,WM_GETTEXT,(WPARAM)length,(LPARAM)(text));
   ReplaceSpecialCharacters(text,"\r\n","\n",length);

// Search for string or substring  
   for(;;)
   { 
      offset = FindReverseSubStr(text, word, pos, false,false);
      if(offset == -1)
      {
         delete [] text;
         return(ERR);
      }
      // Stop infinite loop
      if(lastoffset != -1 && lastoffset == offset)
      {
         delete [] text;
         return(ERR);
      }
      lastoffset = offset;
      if(mode == 1)
      {        
         name = ExpandToFullWord2(text,offset,BEFORE_OPERAND,AFTER_OPERAND,startWord,endWord);
         if(!strcmp(name,word))
         {
            delete [] name;
            break;
         }
         delete [] name;
         pos = startWord;
      }
      else
      {
         pos = offset;
         break;
      }
   }

// Return start of new word   
   pos = offset;
   delete [] text;   

   return(OK);
}

   
/****************************************************************************************
  The user has selected  "jump to last procedure". So jump back to the last procedure
  selected.
*****************************************************************************************/

void EditRegion::SelectLastProcedure()
{
   char name[MAX_PATH];
   char path[MAX_PATH];
   long size = (*jumpArray).size();
   Interface itfc;
   jumpIndex--;

   if(jumpIndex >= 0 && jumpIndex < size)
   {
      long lineNumber = (*jumpArray)[jumpIndex].lineNr;
      strncpy_s(name,MAX_PATH,(*jumpArray)[jumpIndex].macroName,_TRUNCATE);
      strncpy_s(path,MAX_PATH,(*jumpArray)[jumpIndex].macroPath,_TRUNCATE);
      if(!strcmp(name,edName) || !strcmp(name,""))
         SelectEditorLine(lineNumber);   
      else
      {
         SetCursor(LoadCursor(NULL,IDC_WAIT));
	      char *text = LoadTextFile(&itfc,path, name, "",".mac");
      // Is there such a file?
         if(!text)
         {
            text = LoadTextFile(&itfc,path, name, "",".pex");
            if(!text)
            {
	            SetCursor(LoadCursor(NULL,IDC_ARROW));
               CText txt;
               txt.Format("Can't find macro '%s'",name);
               MessageDialog(edWin,MB_ICONERROR,"Error",txt.Str());
               return;
            }
         }
      // Change the focus to speed up rendering
         SetEditorFocus();
      // Display the text in the editor
	      DisplayColoredText(text,true);
      // Restore focus
         RestoreEditorFocus();
      // Reset modified flag
	      edModified = false;                              
      // Reset undo and jump arrays
	      ResetEditorUndoArray();	 
         SelectEditorLine(lineNumber);   
         strncpy_s(edName,MAX_PATH,name,_TRUNCATE);
         strncpy_s(edPath,MAX_PATH,path,_TRUNCATE);
         SetEditTitle();   
         delete [] text;
	      SetCursor(LoadCursor(NULL,IDC_ARROW));
      }
   }
   else
     jumpIndex = 0;
}

/****************************************************************************************
  The user has selected  "jump to next procedure". So jump forward to the next procedure
  in the jump list.
*****************************************************************************************/


void EditRegion::SelectNextProcedure()
{
   char name[MAX_PATH];
   char path[MAX_PATH];
   Interface itfc;
   short size = (*jumpArray).size();
   jumpIndex++;
   if(jumpIndex >= 0 && jumpIndex < size)
   {
      long lineNumber = (*jumpArray)[jumpIndex].lineNr;
      strncpy_s(name,MAX_PATH,(*jumpArray)[jumpIndex].macroName,_TRUNCATE);
      strncpy_s(path,MAX_PATH,(*jumpArray)[jumpIndex].macroPath,_TRUNCATE);
      if(!strcmp(name,edName) || !strcmp(name,""))
         SelectEditorLine(lineNumber);   
      else
      {
	      char *text = LoadTextFile(&itfc,path, name, "",".mac");
      // Is there such a file?
         if(!text)
         {
            text = LoadTextFile(&itfc,path, name, "",".pex");
            if(!text)
            {
               char str[MAX_STR];
               sprintf(str,"Can't find macro '%s'",name);
               MessageDialog(edWin,MB_ICONERROR,"Error",str);
               return;
            }
         }
      // Change the focus to speed up rendering
         SetEditorFocus();
      // Display the text in the editor
	      DisplayColoredText(text,true);
      // Restore focus
         RestoreEditorFocus();
      // Reset modified flag
	      edModified = false;                              
      // Reset undo and jump arrays
	      ResetEditorUndoArray();	 
         SelectEditorLine(lineNumber);   
         strncpy_s(edName,MAX_PATH,name,_TRUNCATE);
         strncpy_s(edPath,MAX_PATH,path,_TRUNCATE);
         SetEditTitle();   
         delete [] text;
      }   
   }
   else
     jumpIndex = size-1;
}

/****************************************************************************************
  The user has selected a procedure name from the procedures menu. So jump to that part of
  the text in the text editor, highlighting the first line of the procedure. Also record
  the line number and current macro name so we can come back here if we jump somewhere else.
*****************************************************************************************/

void EditRegion::SelectProcedure(char *procName)
{
   char *name; 
   char *command;      
   char *line;
   long pos; 
   bool found = false;
   
   long n = CountLinesInEditor(edWin);
   
// Search each line in current text editor for matching procedure name     
   for(long i = 0; i < n; i++)
   {
      line = GetLineByNumber(edWin,i);
      command = new char[strlen(line)+1];
      name = new char[strlen(line)+1];      
      pos = 0;
      GetNextWord(pos, line, command, 100);
      if(!strcmp(command,"procedure"))
      {
         GetNextWord(pos, line, name, 100);
         if(!strcmp(name,procName)) // Centre start of procedure in edit window
         {
            found = true;
			// Where we are before the jump
            long curLine = GetCurrentLineNumber(edWin);
			// Jump to the new line with the procedure definition
            SelectEditorLine(i);
			// Record location in jump array
            RecordJumpHistory(curLine,edPath,edName,i,edPath,edName);
				break;
	      }                            
      }   
      delete [] command;
      delete [] name;            
      delete [] line;
   }

   if(!found)
   {
      if(procName[0] == '\0') // No procedure in file
      {
         long curLine = GetCurrentLineNumber(edWin);
         SelectEditorLine(0);
         RecordJumpHistory(curLine,edPath,edName,0,edPath,edName);
      }
      else
      {
         char str[MAX_STR];
         sprintf(str,"Can't find procedure '%s'",procName);
         MessageDialog(edWin,MB_ICONERROR,"Error",str);
         return;
      }
   }
}



short EditRegion::LoadAndSelectProcedure(char *macroName, char *procName, bool showErrors)
{
   char *name; 
   char *command;      
   char *line;
   char oldMacro[MAX_PATH];
   char newMacro[MAX_PATH];
   char newMacroBak[MAX_PATH]; 
   char newPath[MAX_PATH]; 
   char oldPath[MAX_PATH]; 
   char str[MAX_STR];
   char *text = NULL;

// Where we are before the jump
   long curLine = GetCurrentLineNumber(edWin);
   strncpy_s(oldMacro,MAX_PATH,edName,_TRUNCATE);
   strncpy_s(oldPath,MAX_PATH,edPath,_TRUNCATE);
   strncpy_s(newPath,MAX_PATH,edPath,_TRUNCATE);
   RemoveExtension(oldMacro);
// Record new macro name
   if(macroName[0] == '\0')
      strncpy_s(newMacro,MAX_PATH,oldMacro,_TRUNCATE);
   else
      strncpy_s(newMacro,MAX_PATH,macroName,_TRUNCATE); 

   strncpy_s(newMacroBak,MAX_PATH,newMacro,_TRUNCATE);
   RemoveExtension(newMacroBak);

// Load the new text
   Interface itfc;
	text = LoadTextFile(&itfc,newPath, newMacro, "",".mac");
// Is there such a file?
   if(!text)
   {
      if(!showErrors)
         return(1);
      sprintf(str,"Can't find macro '%s.mac' or '%s.pex'",newMacroBak,newMacroBak);
      MessageDialog(edWin,MB_ICONERROR,"Error",str);
      return(1);
   }
// Check to see if the procedure exists in text
   if(procName[0] != '\0')
   {
      if(FindProcedurePosition(text,procName) == ERR)
      {
         delete [] text;
         if(!showErrors)
            return(1);
         sprintf(str,"Can't find procedure '%s'",procName);
         MessageDialog(edWin,MB_ICONERROR,"Error",str);
         return(1);
      }
   }
// Check to see if current text needs saving 
   if(CheckForUnsavedEdits(regNr) == IDCANCEL)
   {
      delete [] text;
      return(0);
   }
// Change the focus to speed up rendering
   SetEditorFocus();
// Display the text in the editor
   DisplayColoredText(text,true);
// Restore focus
   RestoreEditorFocus();
// Reset modified flag
	edModified = false;                              
// Reset undo and jump arrays
	ResetEditorUndoArray();	
// Search each line in current text editor for matching procedure name     
// recording old and new cursor location 
   if(procName[0] != '\0')
   {
      short n = CountLinesInEditor(edWin);
      for(long i = 0; i < n; i++)
      {
         line = GetLineByNumber(edWin,i);
         command = new char[strlen(line)+1];
         name = new char[strlen(line)+1];      
         long pos = 0;
         GetNextWord(pos, line, command, 100);
         if(!strcmp(command,"procedure"))
         {
            GetNextWord(pos, line, name, 100);
            if(!strcmp(name,procName)) // Centre start of procedure in edit window
            {
			   // Jump to the new line with the procedure definition
               SelectEditorLine(i);
			   // Record location in jump array
               RecordJumpHistory(curLine,oldPath,oldMacro,i,newPath,newMacro);
				   break;
	         }                            
         }   
         delete [] command;
         delete [] name;            
         delete [] line;
      }
   }
   else // No procedure is specified, just a macro so start is line 0
   {
      RecordJumpHistory(curLine,oldPath,oldMacro,0,newPath,newMacro);
   }
// Update edit window title
   strncpy_s(edName,MAX_PATH,newMacro,_TRUNCATE);
   strncpy_s(edPath,MAX_PATH,newPath,_TRUNCATE);
   SetEditTitle();   
   delete [] text;
   return(0);
}

/*************************************************************************************
  Record the current line and current macro as well as the new line and new macro
  when moving to another location by ctrl-clicking on a macro or procedure name
*************************************************************************************/

void EditRegion::RecordJumpHistory(long oldLine, char* oldPath, char *oldMacro, 
                                   long newLine, char *newPath, char* newMacro)
{
   EditJumpType jump;	
   long size = (*jumpArray).size();
   if(size > 0)
   {
	   if(jumpIndex == size-1)
      {
		   (*jumpArray)[size-1].lineNr = oldLine; // Overwrite old end
         strncpy_s((*jumpArray)[size-1].macroName,MAX_PATH,oldMacro,_TRUNCATE);
         strncpy_s((*jumpArray)[size-1].macroPath,MAX_PATH,oldPath,_TRUNCATE);
      }
	   else
	   {
		   jump.lineNr = oldLine;
         strncpy_s(jump.macroName,MAX_PATH,oldMacro,_TRUNCATE);
         strncpy_s(jump.macroPath,MAX_PATH,oldPath,_TRUNCATE);
         (*jumpArray).resize(jumpIndex+1); // Start again from this node			   
         (*jumpArray).push_back(jump);   // Record old line number
         jumpIndex++;	
	   }

      jump.lineNr = newLine; 
      strncpy_s(jump.macroName,MAX_PATH,newMacro,_TRUNCATE);
      strncpy_s(jump.macroPath,MAX_PATH,newPath,_TRUNCATE); // What do we use here??
      (*jumpArray).push_back(jump);  // Record the new line number and macro        
      jumpIndex++;	
   }
   else
   {
	   jump.lineNr = oldLine;
      strncpy_s(jump.macroName,MAX_PATH,oldMacro,_TRUNCATE);
      strncpy_s(jump.macroPath,MAX_PATH,oldPath,_TRUNCATE);
      (*jumpArray).resize(jumpIndex+1); // Start again from this node			   
      (*jumpArray).push_back(jump);   // Record old line number
      jump.lineNr = newLine; 
      strncpy_s(jump.macroName,MAX_PATH,newMacro,_TRUNCATE);
      strncpy_s(jump.macroPath,MAX_PATH,newPath,_TRUNCATE);
      (*jumpArray).push_back(jump);   // Record the new line number and macro        
      jumpIndex+=2;
   }
}

/****************************************************************************************
  Get the first and last editor lines which are currently visible
*****************************************************************************************/

//void EditRegion::GetFirstAndLast(short &first, short &last)
//{
//   RECT r;	       
// 
//// Scroll so this line is in centre of screen
//   first = SendMessage(edWin,EM_GETFIRSTVISIBLELINE ,0,0); // Find first visible line
//
//}

/****************************************************************************************
  Select line in editor and make it visible
*****************************************************************************************/

void EditRegion::SelectEditorLine(long line)
{
   RECT r;	       
// Hide all drawing
	SendMessage(edWin, WM_SETREDRAW, false, 0);
   SendMessage(edWin, WM_HSCROLL, SB_LEFT, NULL);  // Scroll line to left of window	   
// Select the line
   SelectLines(line+1,line+1);  
// Scroll so this line is in centre of screen
   short first = SendMessage(edWin,EM_GETFIRSTVISIBLELINE ,0,0); // Find first visible line
   SendMessage(edWin,EM_GETRECT,0,(LPARAM)&r);   // Get size of edit window in pixels			      
   short linestoscroll = line-first-(r.bottom-r.top)/(lineHeight*2);  // Work out lines to scroll	
   SendMessage(edWin, EM_LINESCROLL, 0, linestoscroll);
// Make sure the window is redrawn
   ShowWindow(edWin, SW_SHOWMAXIMIZED);
   ShowWindow(edWin, SW_NORMAL);
   SendMessage(edWin, WM_SETREDRAW, true, 0);

   MyInvalidateRect(edWin,0,true);
}


/****************************************************************************************
  Select a point in the editor and make it visible
*****************************************************************************************/

void EditRegion::SelectEditPosition(long startSel, long endSel, long firstVisibleLine)
{
   RECT r;	       
// Hide all drawing
	SendMessage(edWin, WM_SETREDRAW, false, 0);
	SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)startSel, (LPARAM)0); 

	long startVisPos = SendMessage(edWin,EM_LINEINDEX,(WPARAM)firstVisibleLine, (LPARAM)0); 

   SendMessage(edWin,EM_GETRECT,0,(LPARAM)&r);   // Get size of edit window in pixels			      
   short lastVisibleLine = firstVisibleLine+(r.bottom-r.top)/(lineHeight*2);  // Find last visible line

	long lastVisPos = SendMessage(edWin,EM_LINEINDEX,(WPARAM)lastVisibleLine, (LPARAM)0); 

   SendMessage(edWin,EM_LINESCROLL,0,firstVisibleLine);  // Scroll to previous first visible line

   if(startSel < startVisPos || startSel > lastVisPos)
	   SetEditSelection(edWin,startVisPos,startVisPos);
   else
	   SetEditSelection(edWin,startSel,endSel);

   SendMessage(edWin,WM_HSCROLL,SB_LEFT,NULL);  // Scroll line to left of window	      

// Make sure the window is redrawn
	SendMessage(edWin, WM_SETREDRAW, true, 0);
	MyInvalidateRect(edWin,0,true);
}		


/****************************************************************************************
  Select line in editor and make it visible
*****************************************************************************************/

void EditRegion::ScrollToEditorLine(long line)
{
   RECT r;	       
// Hide all drawing
	SendMessage(edWin, WM_SETREDRAW, false, 0);
// Place insertion point at line start
   long lineStart = SendMessage(edWin,EM_LINEINDEX,(WPARAM)(long)line,(LPARAM)0);
	SetFocus(edWin);
	SetEditSelection(edWin,lineStart,lineStart);
// Scroll so this line is in centre of screen
   short first = SendMessage(edWin,EM_GETFIRSTVISIBLELINE ,0,0); // Find first visible line
   SendMessage(edWin,EM_GETRECT,0,(LPARAM)&r);   // Get size of edit window in pixels			      
   short linestoscroll = line-first;  // Work out lines to scroll			      	
   SendMessage(edWin,EM_LINESCROLL,0,linestoscroll);  // Scroll line to middle of window	      	   	   
   SendMessage(edWin,WM_HSCROLL,SB_LEFT,NULL);  // Scroll line to left of window	      	   	   
// Make sure the window is redrawn
	SendMessage(edWin, WM_SETREDRAW, true, 0);
	MyInvalidateRect(edWin,0,true);
}

/****************************************************************************************
  User has done something in the editor which can be undone - so save information
  necessary for the undo step (range of selection characters selected etc) in the 
  undo buffer array 'undoArray' incrementing the array index 'undoIndex'.
*****************************************************************************************/
    
void EditRegion::CopySelectionToUndo(short type, long value)
{
   char *text;
   long startSel,endSel;
   UndoType undoData;
   long len = 0;
   long size = (*undoArray).size();
   
   switch(type)
   {	   
      case(TAB_INSERT):
      {
         GetEditSelection(edWin,startSel,endSel);
         text = new char[endSel-startSel+1];	 
         SendMessage(edWin,EM_GETSELTEXT,(WPARAM)0, (LPARAM)text);        
         undoData.startPos = startSel;
         undoData.endPos = startSel+3;
         undoData.type = type;  
         undoData.text = text;	
         initCharInsertionPnt = undoData.startPos;
         (*undoArray).push_back(undoData);         
         undoIndex++;
         break;
      }  
              
   // User has typed a character after moving the insertion point - if first time then record new insertion point
   // otherwise just update the position of the current character
	   case(ENTER_TEXT): 
	   {
	      GetEditSelection(edWin,startSel,endSel);
         if(size > 0 && ((*undoArray)[undoIndex-1].type == ENTER_TEXT) &&  // nth character entered
           (initCharInsertionPnt == (*undoArray)[undoIndex-1].startPos))
	      {
            (*undoArray)[undoIndex-1].endPos = endSel;	// Point just before we moved it again (on same line)   
         }
         else  if(size > 0 && ((*undoArray)[undoIndex-1].type == TAB_INSERT) &&  // First character after tab
           (initCharInsertionPnt == (*undoArray)[undoIndex-1].startPos))
	      {
            initCharInsertionPnt = startSel;
            undoData.startPos = startSel;
            undoData.endPos = endSel;	// Point just before we moved it again (on same line)   
            undoData.type = type;  
            undoData.text = NULL;	
            (*undoArray).push_back(undoData);         
            undoIndex++; 
         }
         else // 1st character entered after insertion point reset
         {	 
            undoData.startPos = initCharInsertionPnt; // Point where user last moved insertion point
            undoData.endPos = endSel;	// Point just before we moved it again (on same line)   
            undoData.type = type;  
            undoData.text = NULL;	
            (*undoArray).push_back(undoData);         
            undoIndex++;
         }                  
         break;   
	   }

   // User has type a return
	   case(ENTER_RETURN): 
	   {
	      GetEditSelection(edWin,startSel,endSel);
         initCharInsertionPnt = startSel+1;
         undoData.startPos = startSel;
         undoData.endPos = endSel;	// Point just before we moved it again (on same line)   
         undoData.type = type;  
         undoData.text = NULL;	
         (*undoArray).push_back(undoData);         
         undoIndex++; 
         break;   
	   }
	   
	// User has typed a character when text has been selected - the text will be replaced with character  
	   case(REPLACE_TEXT_BY_CHAR):
	   {
	      GetEditSelection(edWin,startSel,endSel);
         text = new char[endSel-startSel+1];	   
         SendMessage(edWin,EM_GETSELTEXT,(WPARAM)0, (LPARAM)text);
         undoData.startPos = startSel;
         undoData.endPos = endSel; 
         undoData.type = type;  
         undoData.text = text;
         (*undoArray).push_back(undoData);
         undoIndex++;
         break;	
	   }
	   
	   case(REPLACE_TEXT):
	   {
	      GetEditSelection(edWin,startSel,endSel);
         
         text = new char[endSel-startSel+1];	   
         SendMessage(edWin,EM_GETSELTEXT,(WPARAM)0, (LPARAM)text);
         undoData.startPos = startSel;
         undoData.endPos = startSel+value; 
         undoData.type = type;  
         undoData.text = text;
         (*undoArray).push_back(undoData);
         undoIndex++;
         break;	
	   }	   
	
	// User has deleted some text, with cut, delete or backspace keys. Record deleted text and its range   
      case(CUT_TEXT):
      {
	      GetEditSelection(edWin,startSel,endSel);
         text = new char[endSel-startSel+1];	 
         SendMessage(edWin,EM_GETSELTEXT,(WPARAM)0, (LPARAM)text);        
         undoData.startPos = startSel;
         undoData.endPos = endSel; 
         undoData.type = type;  
         undoData.text = text;
         (*undoArray).push_back(undoData);
         undoIndex++;
         break;	
      }
 
  // User has indented or unindented text - record lines selected for this process
      case(INDENT_TEXT):
      case(UNINDENT_TEXT):
      {
	      GetEditSelection(edWin,startSel,endSel);
         if(startSel == endSel) // Single tab inserted
         {
		      undoData.startPos = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)startSel,(LPARAM)0);	
		      undoData.endPos = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)endSel,(LPARAM)0);         
         }
         else // Multiple lines to be tabbed
         {
		      undoData.startPos = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)startSel,(LPARAM)0);	
		      undoData.endPos = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)endSel-1,(LPARAM)0);         
         }
         undoData.type = type;  
         undoData.text = NULL;
         (*undoArray).push_back(undoData);
         undoIndex++;
         break;	
      }

  // User has commented or uncommented text - record lines selected for this process
      case(COMMENT_TEXT):
      case(UNCOMMENT_TEXT):
      {
	      GetEditSelection(edWin,startSel,endSel);
		   undoData.startPos = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)startSel,(LPARAM)0);	
		   undoData.endPos = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)endSel-1,(LPARAM)0);         
         undoData.type = type;  
         undoData.text = NULL;
         (*undoArray).push_back(undoData);
         undoIndex++;
         break;	
      }
         
   // User has pasted some text. Save text pasted over and the length of the new text (note that global
   // the clipboard buffer stores line ends with \r\n but the editor just with \r)    
      case(PASTE_TEXT):
      {
	      if(OpenClipboard(edWin)) //Paste text from the clipboard
	      { 
	         GetEditSelection(edWin,startSel,endSel);
            text = new char[endSel-startSel+1];	   
            SendMessage(edWin,EM_GETSELTEXT,(WPARAM)0, (LPARAM)text);
            undoData.text = text; // Save any original text (it might be selected)
           
         // Get paste buffer and determine its length (without \n)   	   
	         HGLOBAL hglb;
            LPSTR lpstr;	         
	         if((hglb = GetClipboardData(CF_TEXT)) != NULL) 
	         {
	            lpstr = (LPSTR)GlobalLock(hglb); 
	            ReplaceSpecialCharacters(lpstr,"\r\n","\r",-1);
	            len = strlen(lpstr);
	            GlobalUnlock(hglb);             	         
	         }
	      // Record extent of replaced text
            undoData.startPos = startSel;
            undoData.endPos = startSel + len; 
            undoData.type = type; 
            (*undoArray).push_back(undoData);
            undoIndex++;	      
	         CloseClipboard(); 
	      } 
	      break;
	   } 
	} 
}

/****************************************************************************************
  User has selected undo so check in the undo buffer array 'undoArray' for things to undo.
  Just undo the operation at the end of the array and then delete the last entry.
*****************************************************************************************/

void EditRegion::CopyUndoToSelection()
{      
   undoIndex--;
   long size = (*undoArray).size();
   
   if(undoIndex >=0 && undoIndex < size && size > 0)
   {
      switch((*undoArray)[undoIndex].type)
      {
         case(TAB_INSERT):
         {
            long startSel  = (*undoArray)[undoIndex].startPos;
            long endSel = (*undoArray)[undoIndex].endPos;
	         SetEditSelection(edWin,startSel,endSel);           
            SendMessage(edWin,EM_REPLACESEL,(WPARAM)0, (LPARAM)(*undoArray)[undoIndex].text);
 	         SetFocus(edWin);
 	    //     SetFocus(editWin);
		      long line = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)startSel,(LPARAM)0);	
            ContextColorLines(line,line);
	         SetFocus(edWin);         
            delete [] (*undoArray)[undoIndex].text;
            (*undoArray).resize(undoIndex); 
            break;       
         } 

     // Undo an enter text operation by deleting the text 
         case(ENTER_RETURN):
         case(ENTER_TEXT):
	      {   
            long start = (*undoArray)[undoIndex].startPos;
            long end = (*undoArray)[undoIndex].endPos; 
            SetEditSelection(edWin,start,end+1);  
	         SendMessage(edWin,EM_REPLACESEL,(WPARAM)0, (LPARAM)""); 
            initCharInsertionPnt = start;
            (*undoArray).resize(undoIndex); 
            break;          	
	      } 

         
    // Undo an indent operation by calling unindent
         case(INDENT_TEXT):   
         {
            long firstLine = (*undoArray)[undoIndex].startPos;
            long lastLine = (*undoArray)[undoIndex].endPos;
			   long startSel = SendMessage(edWin,EM_LINEINDEX,(WPARAM)firstLine,(LPARAM)0); 			 
			   long endSel = SendMessage(edWin,EM_LINEINDEX,(WPARAM)lastLine+1,(LPARAM)0); 			       
	         SetEditSelection(edWin,startSel,endSel);  
            UnIndentText(1);     
 	         SetFocus(GetParent(edWin));
            ContextColorLines(firstLine,lastLine);
	         SetFocus(edWin);         
            delete [] (*undoArray)[undoIndex].text;
            (*undoArray).resize(undoIndex); 
            break;       
         } 
         
     // Undo an unindent operation by calling indent
         case(UNINDENT_TEXT):   
         {
            long firstLine = (*undoArray)[undoIndex].startPos;
            long lastLine = (*undoArray)[undoIndex].endPos;
				long startSel = 0;
				long endSel = 0;
            if(firstLine == lastLine)
            {
			      startSel = SendMessage(edWin,EM_LINEINDEX,(WPARAM)firstLine,(LPARAM)0); 
			      endSel = SendMessage(edWin,EM_LINEINDEX,(WPARAM)lastLine,(LPARAM)0); 			       
            }
            else
            {
 			      startSel = SendMessage(edWin,EM_LINEINDEX,(WPARAM)firstLine,(LPARAM)0); 
			      endSel = SendMessage(edWin,EM_LINEINDEX,(WPARAM)lastLine+1,(LPARAM)0); 			       
            }
	         SetEditSelection(edWin,startSel,endSel);           
            IndentText(1);      
 	         SetFocus(GetParent(edWin));
            ContextColorLines(firstLine,lastLine);
	         SetFocus(edWin);         
            delete [] (*undoArray)[undoIndex].text;
            (*undoArray).resize(undoIndex);   
            break;      
         }    
 
     // Undo an comment operation by calling uncomment
         case(COMMENT_TEXT):   
         {
            long firstLine = (*undoArray)[undoIndex].startPos;
            long lastLine = (*undoArray)[undoIndex].endPos;
			   long startSel = SendMessage(edWin,EM_LINEINDEX,(WPARAM)firstLine,(LPARAM)0); 			 
			   long endSel = SendMessage(edWin,EM_LINEINDEX,(WPARAM)lastLine+1,(LPARAM)0); 			       
	         SetEditSelection(edWin,startSel,endSel);
            BlockUncomment(1); 
 	         SetFocus(GetParent(edWin));
            ContextColorLines(firstLine,lastLine);
	         SetFocus(edWin);         
            delete [] (*undoArray)[undoIndex].text;
            (*undoArray).resize(undoIndex); 
            break;       
         } 
         
     // Undo an uncomment operation by calling comment
         case(UNCOMMENT_TEXT):   
         {
            long firstLine = (*undoArray)[undoIndex].startPos;
            long lastLine = (*undoArray)[undoIndex].endPos;
			   long startSel = SendMessage(edWin,EM_LINEINDEX,(WPARAM)firstLine,(LPARAM)0); 
			   long endSel = SendMessage(edWin,EM_LINEINDEX,(WPARAM)lastLine+1,(LPARAM)0); 			       
	         SetEditSelection(edWin,startSel,endSel);           
            BlockComment(1); 
 	         SetFocus(GetParent(edWin));
            ContextColorLines(firstLine,lastLine);
	         SetFocus(edWin);         
            delete [] (*undoArray)[undoIndex].text;
            (*undoArray).resize(undoIndex);   
            break;      
         }  
                  
    // Undo a cut operation by pasting the deleted text   
         case(CUT_TEXT):
         {
				long startSel1,endSel1;
				long startSel2,endSel2; 
	         SetEditSelection(edWin,(*undoArray)[undoIndex].startPos,(*undoArray)[undoIndex].startPos);           	           
	         GetEditSelection(edWin,startSel1,endSel1);
            SendMessage(edWin,EM_REPLACESEL,(WPARAM)0, (LPARAM)(*undoArray)[undoIndex].text);
	         GetEditSelection(edWin,startSel2,endSel2);
            initCharInsertionPnt = startSel2;			      
	         SetFocus(edWin);
		      long firstLine = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)startSel1,(LPARAM)0);	
		      long lastLine = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)endSel2,(LPARAM)0);	
            ContextColorLines(firstLine,lastLine);
	         SetFocus(edWin);         
            delete [] (*undoArray)[undoIndex].text;
            (*undoArray).resize(undoIndex);
            break;
         }      
                           	       
   // User is undoing a replace text operation. If text which was replaced included a trialing /r then we need to
   // delete the /r on the current line because the replace operation didn't delete it (MS bug?) hence the +2 option.
   // otherwise just replace the typed character (+1 option).
            
         case(REPLACE_TEXT_BY_CHAR):
	      {  
	         char *text = (*undoArray)[undoIndex].text;
				long startSel1,endSel1;
				long startSel2,endSel2; 
	         if(text[strlen(text)-1] == '\r')
	         {
               SetEditSelection(edWin,(*undoArray)[undoIndex].startPos, (*undoArray)[undoIndex].startPos+2);	         
            }
            else
            {
               SetEditSelection(edWin,(*undoArray)[undoIndex].startPos, (*undoArray)[undoIndex].startPos+1);	                     
            }
	         GetEditSelection(edWin,startSel1,endSel1);	      
            SendMessage(edWin,EM_REPLACESEL,(WPARAM)0, (LPARAM)(*undoArray)[undoIndex].text);
	         GetEditSelection(edWin,startSel2,endSel2);
            initCharInsertionPnt = startSel2;		      
	         SetFocus(GetParent(edWin));
		      long firstLine = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)startSel1,(LPARAM)0);	
		      long lastLine = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)endSel2,(LPARAM)0);	
            ContextColorLines(firstLine,lastLine);
	         SetFocus(edWin);         
            delete [] (*undoArray)[undoIndex].text;
            (*undoArray).resize(undoIndex);
            break;
         }  
 
         case(REPLACE_TEXT):
	      {  
            long startSel = (*undoArray)[undoIndex].startPos;
            long endSel = (*undoArray)[undoIndex].endPos;
            SetEditSelection(edWin, startSel, endSel);
            SendMessage(edWin,EM_REPLACESEL,(WPARAM)0, (LPARAM)(*undoArray)[undoIndex].text);
            initCharInsertionPnt = startSel;		      
	         SetFocus(GetParent(edWin));
		      long firstLine = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)startSel,(LPARAM)0);	
		      long lastLine = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)endSel,(LPARAM)0);	
            ContextColorLines(firstLine,lastLine);
	         SetFocus(edWin);         
            delete [] (*undoArray)[undoIndex].text;
            (*undoArray).resize(undoIndex);
            break;
         }  
                
     // Undo a paste operation by replacing the pasted text with the original text  
         case(PASTE_TEXT):
         {
				long startSel1,endSel1;
				long startSel2,endSel2; 
            long start = (*undoArray)[undoIndex].startPos;
            long end = (*undoArray)[undoIndex].endPos;  
            char *text = (*undoArray)[undoIndex].text; 
            
            SetEditSelection(edWin,start,end);  
	         GetEditSelection(edWin,startSel1,endSel1);
            SendMessage(edWin,EM_REPLACESEL,(WPARAM)0, (LPARAM)text);
	         GetEditSelection(edWin,startSel2,endSel2);
            initCharInsertionPnt = startSel2;			      
	         SetFocus(GetParent(edWin));
		      long firstLine = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)startSel1,(LPARAM)0);	
		      long lastLine = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)endSel2,(LPARAM)0);		      
            ContextColorLines(firstLine,lastLine);
	         SetFocus(edWin); 
            delete [] (*undoArray)[undoIndex].text;
            (*undoArray).resize(undoIndex);
            break;
         }
  	   }                    
   }
   else
   {
      undoIndex = 0;   // No more undo to do    
   }
}

/****************************************************************************************
  Paste text from the clipboard to the current editor using syntax colouring
  Last modified 11/1/05
*****************************************************************************************/

void EditRegion::Paste()
{
   if(OpenClipboard(edWin)) 
   { 
	   HGLOBAL hglb;
      LPSTR lpstr;
      long startSel1,endSel1;
      bool found = false;
      
	// Get current selection
	   GetEditSelection(edWin,startSel1,endSel1);
      	         
	   if((hglb = GetClipboardData(CF_TEXT)) != NULL) 
	   {
		// Get clipboard text and use it to replace current selection
		// (replace any cr-lf with cr first)
	      lpstr = (LPSTR)GlobalLock(hglb); 
   //      ReplaceSpecialCharacters(lpstr,"\r\r","\r");	// Fixes pastes from htmlhelp
   //      ReplaceSpecialCharacters(lpstr,"\r\n\r\n","\r");	// Fixes pastes from iexplorer
	      ReplaceSpecialCharacters(lpstr,"\r\n","\r",-1);	// Fixes pastes from other documents.
      // Replace invalid characters with blanks
         for(long i = 0; i < strlen(lpstr); i++)
         {
            if(lpstr[i] < 0)
            {
               lpstr[i] = '!';
               found = true;
            }
         }
         if(found)
         {
            MessageDialog(prospaWin,MB_ICONWARNING,"Warning","One or more invalid characters have\r been pasted and replaced with '!'");
         }

		   SendMessage(edWin,EM_REPLACESEL,true,(LPARAM)(LPCTSTR)lpstr);
	      GlobalUnlock(hglb); 

		// Find the end of the pasted text
			long startSel2,endSel2;  
	      GetEditSelection(edWin,startSel2,endSel2);		   	
         initCharInsertionPnt = startSel2;		   	

      // Get range of lines in the newly pasted text
			long firstLine = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)startSel1,(LPARAM)0);	
			long lastLine = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)endSel2,(LPARAM)0);	
		   
		//	Syntax highlight this new text
			ContextColorLines(firstLine,lastLine);
			SetFocus(GetParent(edWin));   
	   }
      CloseClipboard(); 
   } 	
}  

/****************************************************************************************
  Copy selected text to the global clipboard, replacing any /r with /r/n in the process.
  Also delete the text from the editor.
  Last modified 11/1/05
*****************************************************************************************/

void EditRegion::Cut()
{
    LPTSTR  lptstrCopy; 
    HGLOBAL hglbCopy;
    char *text,*temp;

   if(OpenClipboard(edWin)) //Paste text from the clipboard
   { 
      long startSel,endSel;
   
   // Get rid of any old clipboard material    
      EmptyClipboard(); 

   // Get the selected text   
	   GetEditSelection(edWin,startSel,endSel);       
      long size = endSel-startSel+1;
      temp = new char[size];	   
      SendMessage(edWin,EM_GETSELTEXT,(WPARAM)0, (LPARAM)temp);
   
   // Cut the selected text      
		SendMessage(edWin,EM_REPLACESEL,true,(LPARAM)(LPCTSTR)"");	
		
   // Replace \r with \r\n
      long retCnt = 0;
      for(long i = 0; i < size-1; i++)
         if(temp[i] == '\r') retCnt++;      
      text = new char[size + retCnt];
      strncpy_s(text,size+retCnt,temp,_TRUNCATE);   
      delete [] temp;            
	   ReplaceSpecialCharacters(text,"\r","\r\n",size+retCnt);	
	   
   // Allocate a global memory object for the text. 
      hglbCopy = GlobalAlloc(GHND | GMEM_SHARE, size + retCnt); 
 
   // Lock the handle and copy the selected (modified) text to the buffer.  
      lptstrCopy = (LPSTR)GlobalLock(hglbCopy); 
      memcpy(lptstrCopy, text, size + retCnt); 
      delete [] text;
      GlobalUnlock(hglbCopy); 

  // Place the handle on the clipboard. 
      SetClipboardData(CF_TEXT, hglbCopy); 

      CloseClipboard(); 
   } 	
}  


/****************************************************************************************
   Add a character to the editor and color the line its on.
   Make sure any selected text is replaced by the character
*****************************************************************************************/

void EditRegion::Enter(char c)
{
   long startSel,endSel;
   char text[2];

   GetEditSelection(edWin,startSel,endSel);
   SetEditSelection(edWin,startSel,endSel);  
  	
 	text[0] = c;
 	text[1] = '\0';
   SendMessage(edWin,EM_REPLACESEL,true,(LPARAM)(LPCTSTR)text);	
   if(this->edParent->showSyntaxColoring)
      UpdateLineColor(this,edWin,GetCurrentLineNumber(edWin)); 
}

void EditRegion::ResetEditorUndoArray()
{
   if(undoArray)
   {
      for(int i = 0; i < (*undoArray).size(); i++)
         delete []  (*undoArray)[i].text;
     
      (*undoArray).clear();	
      undoIndex = 0;
   }
} 

void EditRegion::ResetEditorJumpArray()
{
   if(jumpArray)
   {
      (*jumpArray).clear();
      jumpIndex = -1;
   }
}

/************************************************************
* Display text in editor using context coloring
************************************************************/

void EditRegion::DisplayColoredText(char *text, bool redraw)
{
// Stop all drawing in editor
	SendMessage(edWin, WM_SETREDRAW, false, 0);
// Select all text   
   SetEditSelection(edWin,0,-1);     
// Replace with new text
   SendMessage(edWin,EM_REPLACESEL,true,(LPARAM)(LPCTSTR)text);
// Set all text to black
//   COMMENT NEXT TWO STATEMENTS OUT TO COLOUR ON THE FLY - SEE WM_PAINT
   SetColor(edWin,RGB(0,0,0));
// Context color it
// Check if we want to color the text
   if(edParent->showSyntaxColoring)
      ContextColorAllLines(true,redraw);
   if(redraw)
     SetEditSelection(edWin,1,1);  	    	
// Make sure the window is redrawn
   SendMessage(edWin, WM_SETREDRAW, true, 0);
	if(redraw)
	   MyInvalidateRect(edWin,0,true);
}

/************************************************************
* Display text in editor using context coloring
************************************************************/

void EditRegion::AppendText(char* text)
{
   // Stop all drawing in editor
 //  SendMessage(edWin, WM_SETREDRAW, false, 0);
   // Get current insertion point   
   long startSel, endSel;
   SendMessage(edWin, EM_GETSEL, (WPARAM)&startSel, (LPARAM)&endSel);
   // Replace with new text
   SendMessage(edWin, EM_REPLACESEL, true, (LPARAM)(LPCTSTR)text);
   // Set all text to black
  // SetColor(edWin, RGB(0, 0, 0));
   // Context color it
   // Check if we want to color the text
  // if (edParent->showSyntaxColoring)
  //    ContextColorAllLines(true, false);

   int len = SendMessage(edWin, WM_GETTEXTLENGTH, 0, 0);
   // Select the last character so we append more text
   SetEditSelection(edWin, len-1, len-1);
}


/************************************************************
* Apply context coloring to all lines in the editor
************************************************************/

void EditRegion::ContextColorAllLines(bool resetView, bool redraw)
{   

	long startSel,endSel;
   long lines = SendMessage(edWin,EM_GETLINECOUNT,0,0);
  
// Hide all drawing
   SendMessage(edWin, WM_SETREDRAW, false, 0);
// Get current selection
	GetEditSelection(edWin,startSel,endSel);
// Color each line
   for(int i = 0; i < lines; i++)
      UpdateLineColorCore(this,edWin,i);
// Reset select to start if desired
   if(resetView)
     SetEditSelection(edWin,1,1);  	    	
   else
     SetEditSelection(edWin,startSel,endSel);     
// Make sure the window is redrawn
	SendMessage(edWin, WM_SETREDRAW, true, 0);
	if(redraw)
	   MyInvalidateRect(edWin,0,true);
}

/************************************************************
* Context color those lines currently selected
************************************************************/

void EditRegion::ContextColorSelectedLines()
{
   long startSel,endSel;
	GetEditSelection(edWin,startSel,endSel);   
	long firstLine = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)startSel,(LPARAM)0);	
	long lastLine = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)endSel,(LPARAM)0);	
   ContextColorLines(firstLine,lastLine);
}

/************************************************************
* Apply context coloring a range of lines in the display
************************************************************/

void EditRegion::ContextColorLines(long startLine, long endLine)
{   
	long startSel,endSel;
   long lines = SendMessage(edWin,EM_GETLINECOUNT,0,0);
   
   if(startLine > endLine || startLine < 0 || endLine >= lines)
      return;

// Hide all drawing
	SendMessage(edWin, WM_SETREDRAW, false, 0);      
// Get selection range  
	GetEditSelection(edWin,startSel,endSel);
// Color each line	
   for(int i = startLine; i <= endLine; i++)
   {
      SelectLine(edWin,i);
      ColorSelectedText(edWin,RGB(0,0,0));   
      UpdateLineColorCore(this,edWin,i);
   }

// Restore selection
   SetEditSelection(edWin,startSel,endSel);     
// Make sure the window is redrawn
	SendMessage(edWin, WM_SETREDRAW, true, 0);
	MyInvalidateRect(edWin,0,true);	
}

	

short GetNextWord2(short &pos, char *text, char *command, long endLine);






/**************************************
* Set the color for the current line
**************************************/

void EditRegion::SetLineColor(long lineNr, COLORREF color)
{
   long startSel,endSel;

// Hide all drawing
	SendMessage(edWin, WM_SETREDRAW, false, 0);   
// Get selection range  
   GetEditSelection(edWin,startSel,endSel);
// Color line	
   SelectLine(edWin,lineNr);
   ColorSelectedText(edWin,color);
// Restore selection
	SetEditSelection(edWin,startSel,endSel);	
// Show all drawing
	SendMessage(edWin, WM_SETREDRAW, true, 0);   			
}

// Returns the height of a single line in an edit window
short EditRegion::GetLineHeight()
{
	short char1 = SendMessage(edWin,EM_LINEINDEX,(WPARAM)1,0);
	short char2 = SendMessage(edWin,EM_LINEINDEX,(WPARAM)2,0);
	if(richEditVersion == 2)
	{
      DWORD pos1 = SendMessage(edWin,EM_POSFROMCHAR,(WPARAM)char1,0); 
      DWORD pos2 = SendMessage(edWin,EM_POSFROMCHAR,(WPARAM)char2,0);    
      return(HIWORD(pos2)-HIWORD(pos1));
   }
	else
	{
		POINTL p1,p2;
      SendMessage(edWin,EM_POSFROMCHAR,(WPARAM)&p1,(LPARAM)char1); 
      SendMessage(edWin,EM_POSFROMCHAR,(WPARAM)&p2,(LPARAM)char2);    
      return(p2.y-p1.y);
   }
}

long EditRegion::GetLineYPosition(long line)
{
   long yp;
   long firstchar = SendMessage(edWin, EM_LINEINDEX, line, 0 );
   if(richEditVersion == 2)
   {
      DWORD pos;
      pos = SendMessage(edWin, EM_POSFROMCHAR, (WPARAM)firstchar, (LPARAM)0);
      yp = HIWORD(pos);
   }
   else
   {
      POINTL pnt;
      SendMessage(edWin, EM_POSFROMCHAR, (WPARAM)&pnt, (LPARAM)firstchar);
      yp = pnt.y;
   }
   return(yp);
}

char EditRegion::GetCharacter(long pos)
{
   long startSel;
   long endSel;
   char text[2];
 
// Record the current selection  
	GetEditSelection(edWin,startSel,endSel);
// Get the selected character 
 	SetEditSelection(edWin,pos,pos+1);	
 	SendMessage(edWin,EM_GETSELTEXT,(WPARAM)0,(LPARAM) (LPSTR) text);	

// Reset selection
 	SetEditSelection(edWin,startSel,endSel);	
 	
 	return(text[0]);
}

// Some PCs are very slow at rendering colored text. This command fixes this
// by moving the focus to the parent window. Can be switched with editor command
// SetEditorFocus("current/parent")
//
void EditRegion::SetEditorFocus()
{
   if(gFocusForRendering == CURRENTEDITOR)
      SetFocus(edWin);
   else
      SetFocus(edParent->parent->hwndParent); 
}

void EditRegion::RestoreEditorFocus()
{
   if(gFocusForRendering == CURRENTEDITOR)
	   SetFocus(edWin);
}

// Skip characters in 'skip' found from position 'pos' in string 'text'
// Return the new position.
void SkipCharacters(char *text, long sz1, long &pos, char *skip)
{
	long i,j;
	bool found;
	long sz2 = strlen(skip);
	for(i = pos; i < sz1; i++)
	{
		found = false;
		for(j = 0; j < sz2; j++)
		{
			if(text[i] == skip[j])
			{
				found = true;
				break;
			}
		}
		if(!found)
		   break;
	}
	pos = i;
}

// Return all characters found from position 'pos' in 'text' until one character in
// 'delimiters' is found
bool ExtractDelimitedText(char *text, long sz, long &pos, char *delimiters, char *result)
{
	long i;
	long sz2 = strlen(delimiters);
	bool found = false;
	long cnt = 0;

	for(i = pos; i < sz; i++)
	{
		for(long j = 0; j < sz2; j++)
		{
			if(text[i] == delimiters[j])
			{
			   pos = i;
				result[cnt] = '\0';
				return(true);
			}
		}
	   result[cnt++] = text[i];
	}
	return(false);
}

bool CheckForCharacters(char *text, long &pos, char *match)
{
	long sz2 = strlen(match);

	for(long i = 0; i < sz2; i++)
	{
		if(text[pos+i] != match[i])
		{
			return(false);
		}
	}
	return(true);
}


// Copy text to editor and update edit title and reset modified flag and edit arrays

void EditRegion::CopyTextToEditor(char *text)
{   
// Note the visiblity of the parent control
    bool vis = this->edParent->parent->visible;	
// Optimise rendering speed
   SetEditorFocus();
// Change the focus to speed up rendering
	DisplayColoredText(text,true);
// Restore focus
   RestoreEditorFocus();
// Reset modified flag
	edModified = false;
// Reset procedure name (2.2.6)
   currentProc = ""; 
// Set window title     
  	SetEditTitle();                                 
// Reset undo and jump arrays
	ResetEditorUndoArray();	 
	ResetEditorJumpArray();
// Make sure the initial invisibility is restored
// Since this function always leave the object editorvisible
   if(!vis)
   {
      this->edParent->parent->visible = false;
      this->edParent->parent->Show(false); 
   }
}   

// Check to see if the editor text needs saving
// If user selects YES then text will be saved
// If user selects NO or CANCEL then this response is returned and 
      
short EditRegion::CheckForUnsavedEdits(short n)
{
   short response = IDNO;
   
   if(edModified)
   {
      response = QueryDialog(MB_ICONWARNING, "Warning","Do you want to save the text in Edit-%d named: '%s?'",n+1,edName);
   
      if(response == IDYES)
      {
         if(SaveEditContents() == ERR)
            return(IDCANCEL);
      }
   }
   return(response);
}

// Save the contents of the editor using the predefined filename
short EditRegion::SaveEditContents()
{
   if(!strcmp(edName,"untitled"))
   {
	   if(SaveEditDialog(edWin, edPath, edName) != OK)
          return(ERR);
   }
         
   char *text = GetText(edWin);
   if(SaveTextFile(edPath, edName, text) == ERR)
      return(ERR);

   edModified = false;
	SetEditTitle();         
   SetFocus(edWin);
	ResetEditorUndoArray();	 
//	ResetEditorJumpArray();	         
   delete [] text;
   return(OK);
}


/********************************************************************
 Sort the procedures alphabetically in the current text file
 ignoring the first. Procedure comments are also be part of the sort.
*********************************************************************/

typedef struct // Stores information for each procedure
{
    string name;
    string content;
} procInfo;


void EditRegion::SortProcedures()
{
    // Grab the text from the editor
    int len = GetWindowTextLength(curEditor->edWin);
    char* txt = new char[len + 1];
    GetWindowText(curEditor->edWin, txt, len);

    char* procedure;
    char procName[MAX_STR];
    long i = 0;
    long startLine = 0;
    long lineNr = 0;
   // Variable* procVar;
    vector<procInfo> procedures;
    procInfo info;

    // Find all the procedures in txt - include procedure comments
    while (FindProcedures(txt, i, &procedure, procName, len, lineNr, startLine, true))
    {
        info.name = ToLowerCase(procName);
        info.content = procedure;

        procedures.push_back(info);
        delete[] procedure;
    }

    // Alphabetically sort procedures but don't move the first
    auto procNameCmp = [](const procInfo& x, const procInfo& y) {return(x.name < y.name);};

    std::sort(procedures.begin() + 1, procedures.end(), procNameCmp);

    // Hide drawing
    SendMessage(curEditor->edWin, WM_SETREDRAW, false, 0);

    // Make a backup of the current text for undo
    len = SendMessage(curEditor->edWin, WM_GETTEXTLENGTH, 0, 0) + 1;
    SetEditSelection(curEditor->edWin, 0L, len);
    curEditor->CopySelectionToUndo(REPLACE_TEXT, len);

    // Delete the text
    SendMessage(curEditor->edWin, EM_REPLACESEL, true, (LPARAM)(LPCTSTR)"");

    // Append the sorted procedures
    for (procInfo p : procedures)
    {
       AppendText((char*)p.content.c_str());
    }

    // Recolor all lines and redraw text
    SetColor(curEditor->edWin, RGB(0, 0, 0));
    curEditor->ContextColorAllLines(true, true);

    // Indicate the file has been modified
    curEditor->edModified = true;
    SetEditTitle();

    // Delete the copy of the editor text
    delete[] txt;
}


/*************************************************************
 Sort the selected text alphabetically line by line
***************************************************************/

void EditRegion::SortSelection()
{
   long startSel, endSel;

   SendMessage(curEditor->edWin, WM_SETREDRAW, false, 0);

   char* seltxt = GetSelection(startSel, endSel);

   curEditor->CopySelectionToUndo(REPLACE_TEXT, strlen(seltxt));

   delete[] seltxt;

   long startLine = SendMessage(curEditor->edWin, EM_LINEFROMCHAR, (WPARAM)startSel, (LPARAM)0);
   long endLine = SendMessage(curEditor->edWin, EM_LINEFROMCHAR, (WPARAM)endSel - 1, (LPARAM)0);

   int nrLines = endLine - startLine + 1;
   char** lines = new char* [nrLines];

   for (short i = startLine; i <= endLine; i++)
   {
      lines[i - startLine] = GetLineByNumber(curEditor->edWin, i);
   }
   SortList(lines, nrLines);

   for (short i = startLine; i <= endLine; i++)
   {
      CHARRANGE range;
      long start = SendMessage(curEditor->edWin, EM_LINEINDEX, (WPARAM)i, (LPARAM)0);
      long len = SendMessage(curEditor->edWin, EM_LINELENGTH, (WPARAM)start, (LPARAM)0);
      range.cpMin = start; range.cpMax = start + len;
      SendMessage(curEditor->edWin, EM_EXSETSEL, (WPARAM)0, (LPARAM)&range);
      SendMessage(edWin, EM_REPLACESEL, (WPARAM)false, (LPARAM)lines[i-startLine]);
      delete[] lines[i- startLine];
   }
   delete[] lines;
   curEditor->ContextColorLines(startLine, endLine);
   SendMessage(curEditor->edWin, WM_SETREDRAW, true, 0);
   MyInvalidateRect(curEditor->edWin, 0, true);
   if (!curEditor->edModified)
   {
      curEditor->edModified = true;
      SetEditTitle();
   }
}




/*************************************************************
 Search for the specified text in the current text editor
 starting from the current selection.
 Don't wrap at the bottom if wrapPos is negative
 If found select the text and return OK, otherwise return ERR
***************************************************************/

short EditRegion::FindTextAndSelect(HWND hWnd, char *find, bool ignoreCase, long wrapPos)
{
   long length;
   char *text;
   long startSel,endSel;
   long charStart,charEnd;
   
// Copy edit text into a buffer for scanning
   length = SendMessage(edWin,WM_GETTEXTLENGTH,0,0) + 1;
   text = new char[length];   
   SendMessage(edWin,WM_GETTEXT,(WPARAM)length,(LPARAM)text);

// Remove \r\n and replace with \r
   ReplaceSpecialCharacters(text,"\r\n","\n",length);

// Start searching from end of current selection
   GetEditSelection(edWin,startSel,endSel);
// Scan for text
   charStart = FindSubStr(text,find,endSel,ignoreCase);  

 // If 'find' string found select it   
   if(charStart > 0) 
   {
      delete [] text;   
      charEnd = charStart+strlen(find);
	   SetFocus(edWin);
      SetEditSelection(edWin,charStart,charEnd);   
      if((startSel < wrapPos) && (charStart >= wrapPos)) // Wrapped
      {
         MessageDialog(hWnd,MB_ICONINFORMATION,"Find and Replace","Find has returned to the started of the search");   
      }
      return(OK);
   }

// If the wrap position is not zero start from the beginning again   
   if(wrapPos >= 0)
   {
   // Scan for text
      startSel = 0;
      charStart = FindSubStr(text,find,startSel,ignoreCase);  
      
   // If 'find' string found select it   
      if(charStart >= 0) 
      {
         delete [] text;        
         charEnd = charStart+strlen(find);
	      SetFocus(edWin);
         SetEditSelection(edWin,charStart,charEnd); 
         if(startSel < wrapPos && charStart >= wrapPos) // Wrapped
         {
            MessageDialog(hWnd,MB_ICONINFORMATION,"Find and Replace","Find has returned to the started of the search");   
         }
         return(OK);
      }
   }
     
// Not found   
   delete [] text; 
   MessageBeep(MB_OK);
   return(ERR);
}

/*************************************************************
 Search for the specified text in the current text editor
 starting from the current selection searching upwards.
 Don't wrap at the bottom if wrap = false
 If found select the text and return OK, otherwise return ERR
***************************************************************/

short EditRegion::FindTextAndSelectUp(HWND hWnd, char *find, bool ignoreCase, long wrapPos)
{
   long length;
   char *text;
   long startSel,endSel;
   long charStart,charEnd;
   
// Copy edit text into a buffer for scanning
   length = SendMessage(edWin,WM_GETTEXTLENGTH,0,0) + 1;
   text = new char[length];   
   SendMessage(edWin,WM_GETTEXT,(WPARAM)length,(LPARAM)text);

// Remove \r\n and replace with \r
   ReplaceSpecialCharacters(text,"\r\n","\n",length);

// Start searching backwards from start of current selection
	GetEditSelection(edWin,startSel,endSel);
   
// Scan for text
   charStart = FindReverseSubStr(text, find, startSel, ignoreCase, false);

// If 'find' string found select it   
   if(charStart >= 0) 
   {
      delete [] text;      
      charEnd = charStart+strlen(find);
	   SetFocus(edWin);
      SetEditSelection(edWin,charStart,charEnd);  
      if(startSel > wrapPos && charStart <= wrapPos) // Wrapped
      {
         MessageDialog(hWnd,MB_ICONINFORMATION,"Find and Replace","Find has returned to the started of the search");   
      }       
      return(ERR);
   }
   
// If the wrap position is not zero start from the beginning again   
   if(wrapPos >= 0)
   {
   // Scan for text from end
      startSel = strlen(text);
      charStart = FindReverseSubStr(text,find,startSel,ignoreCase, false);  
      
   // If 'find' string found select it   
      if(charStart > 0) 
      {
         delete [] text;        
         charEnd = charStart+strlen(find);
	      SetFocus(edWin);
         SetEditSelection(edWin,charStart,charEnd); 
         if(startSel > wrapPos && charStart <= wrapPos) // Wrapped
         {
            MessageDialog(hWnd,MB_ICONINFORMATION,"Find and Replace","Find has returned to the started of the search");   
         }
         return(OK);
      }
   }
     
// Not found   
   delete [] text; 
   MessageBeep(MB_OK);   
   return(OK);
}

/*************************************************************
 Search for the specified text in the current text editor
 starting from the current selection.
 Don't wrap at the bottom if wrap = false
 If found select the text and return OK, otherwise return ERR
***************************************************************/

short EditRegion::ReplaceAndFindText(HWND hwnd, char *replace, char *find, bool ignoreCase, long wrapPos)
{
   long length;
   char *text;
   long startSel,endSel;
   long charStart,charEnd;

   if(readOnly)
      return(OK);
          
// Start searching from end of current selection
// but only after the text has been replaced
   char *seltxt = GetSelection(startSel, endSel);  
   if((!ignoreCase && !strcmp(seltxt,find)) || (ignoreCase && !stricmp(seltxt,find)))
   {
      CopySelectionToUndo(REPLACE_TEXT,strlen(replace)); 
      SendMessage(edWin,EM_REPLACESEL,(WPARAM)false,(LPARAM)replace); 
      ContextColorSelectedLines();      
   } 
   delete [] seltxt; 
   endSel = startSel + strlen(replace);
   
// Copy edit text into a buffer for scanning
   length = SendMessage(edWin,WM_GETTEXTLENGTH,0,0) + 1;
   text = new char[length];   
   SendMessage(edWin,WM_GETTEXT,(WPARAM)length,(LPARAM)text);

// Remove \r\n and replace with \r
   ReplaceSpecialCharacters(text,"\r\n","\n",length);
  
// Scan for text
   charStart = FindSubStr(text, find, endSel,ignoreCase);  

 // If 'find' string found select it   
   if(charStart > 0) 
   {
      delete [] text;     
      charEnd = charStart+strlen(find);
	   SetFocus(edWin);
	   edModified = true;  
      SetEditSelection(edWin,charStart,charEnd); 
      if(startSel < wrapPos && charStart >= wrapPos) // Wrapped
      {
         MessageDialog(hwnd,MB_ICONINFORMATION,"Find and Replace","Find has returned to the started of the search");   
      }        
      return(OK);
   }
   
// If the wrap position is not zero start from the beginning again   
   if(wrapPos >= 0)
   {
   // Scan for text
      startSel = 0;
      charStart = FindSubStr(text,find,startSel,ignoreCase);  
      
   // If 'find' string found select it   
      if(charStart >= 0) 
      {
         delete [] text;        
         charEnd = charStart+strlen(find);
	      SetFocus(edWin);
         SetEditSelection(edWin,charStart,charEnd); 
         if(startSel < wrapPos && charStart >= wrapPos) // Wrapped
         {
            MessageDialog(hwnd,MB_ICONINFORMATION,"Find and Replace","Replace has returned to the started of the search");   
         }
         return(OK);
      }
   }
     
// Not found   
   delete [] text; 
   MessageDialog(hwnd,MB_ICONINFORMATION,"Find and Replace","The specified text was not found");   
   return(ERR);
}

/*************************************************************
 Search for the specified text in the current text editor
 starting from the current selection.
 Don't wrap at the bottom if wrap = false
 If found select the text and return OK, otherwise return ERR
***************************************************************/

short EditRegion::ReplaceAndFindTextUp(HWND hwnd, char *replace, char *find, bool ignoreCase, long wrapPos)
{
   long length;
   char *text;
   long startSel,endSel;
   long charStart,charEnd;
      
   if(readOnly)
      return(OK);

// Start searching from start of current selection
// but only after the text has been replaced
   char *seltxt = GetSelection(startSel, endSel);
   if((!ignoreCase && !strcmp(seltxt,find)) || (ignoreCase && !stricmp(seltxt,find)))
   {
      CopySelectionToUndo(REPLACE_TEXT,strlen(replace));    
      SendMessage(edWin,EM_REPLACESEL,(WPARAM)false,(LPARAM)replace); 
      ContextColorSelectedLines();
   } 
   delete [] seltxt; 
   
// Copy edit text into a buffer for scanning
   length = SendMessage(edWin,WM_GETTEXTLENGTH,0,0) + 1;
   text = new char[length];   
   SendMessage(edWin,WM_GETTEXT,(WPARAM)length,(LPARAM)text);

// Remove \r\n and replace with \r
   ReplaceSpecialCharacters(text,"\r\n","\n",length);
  
// Scan for text
   charStart = FindReverseSubStr(text, find, startSel, ignoreCase, false);

// If 'find' string found select it   
   if(charStart >= 0) 
   {
      delete [] text;   
      charEnd = charStart+strlen(find);
	   SetFocus(edWin);
      edModified = true;   
      SetEditSelection(edWin,charStart,charEnd);  
      if(startSel > wrapPos && charStart <= wrapPos) // Wrapped
      {
         MessageDialog(hwnd,MB_ICONINFORMATION,"Find and Replace","Find has returned to the started of the search");   
      }             
      return(OK);
   }
   
// If the wrap position is not zero start from the beginning again   
   if(wrapPos >= 0)
   {
   // Scan for text from end
      startSel = strlen(text);
      charStart = FindReverseSubStr(text,find,startSel,ignoreCase, false);  
      
   // If 'find' string found select it   
      if(charStart > 0) 
      {
         delete [] text;        
         charEnd = charStart+strlen(find);
	      SetFocus(edWin);
         SetEditSelection(edWin,charStart,charEnd); 
         if(startSel > wrapPos && charStart <= wrapPos) // Wrapped
         {
            MessageDialog(hwnd,MB_ICONINFORMATION,"Find and Replace","Find has returned to the started of the search");   
         } 
         return(OK);
      }
   }
     
// Not found   
   delete [] text; 
   MessageDialog(hwnd,MB_ICONINFORMATION,"Find and Replace","The specified text was not found");  
   return(ERR);
}

/****************************************************************************************
  Find the key word in 'text' surrounding the point 'pos'. 
  The delimiter string lists those characters used to delimit words.
  Also returns the extend of the word.
  Note the returned string must be freed with delete [] at some stage.
*****************************************************************************************/

char* EditRegion::ExpandToFullWord2(char* text, long pos, char* leftDelimiter, char* rightDelimiter, long &wordStart, long &wordEnd)
{
   char *name;
   long i,j;
   long szLine = strlen(text);
   long szLeftDel  = strlen(leftDelimiter);
   long szRightDel  = strlen(rightDelimiter);
      
// Find start of word
   for(i = pos; i >= 0; i--)
   {
      for(j = 0; j < szLeftDel; j++)
      {
         if(text[i] == leftDelimiter[j])
            break;
      }
      if(j < szLeftDel) break;
   }
   wordStart = i+1;

// Find end of word
   for(i = pos; i < szLine; i++)
   {
      for(j = 0; j < szRightDel; j++)
      {
         if(text[i] == rightDelimiter[j])
            break;
      }
      if(j < szRightDel) break;
   }
   wordEnd = i-1;

// User has selected a delimiter
   if(wordEnd <= wordStart)
      wordEnd = wordStart;

// Extract word from line
   name = new char[wordEnd-wordStart+2];
   strncpy(name,&text[wordStart],wordEnd-wordStart+1);
   name[wordEnd-wordStart+1] = '\0';
   return(name);
}

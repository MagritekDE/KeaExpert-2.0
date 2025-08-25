#include "stdafx.h"
#include "syntax_viewer.h"
#include <stdio.h>
#include "classFunctions.h"
#include "command_other.h"
#include "dll.h"
#include "edit_utilities.h"
#include "events_edit.h"
#include "guiWindowClass.h"
#include "interface.h"
#include "main.h"
#include "cli_events.h"
#include "edit_class.h"
#include "globals.h"
#include "string_utilities.h"
#include "process.h"
#include "variablesOther.h"
#include "memoryLeak.h"

// Handle the drawing of command syntax in the CLI and editor windows

LRESULT CALLBACK SyntaxEventsProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam)
{  
   HDC hdc;
   PAINTSTRUCT ps;
   HBRUSH hBrush;
   StatusBoxInfo* info= NULL;
   
// Find associate parent object
   HWND objWin = GetParent(hWnd);
   WinData* win = rootWin->FindWinByHWND(GetParent(objWin));
	if (win && !win->isAlive())
	{
		return (DefWindowProc( hWnd, messg, wParam, lParam ));
	}
   if(win)
   {
      ObjectData* obj = win->widgets.findByWin(objWin);
      if(obj)
          info = (StatusBoxInfo*)obj->data;
   }

	switch(messg)
	{	
	   case(WM_PAINT): // Set up edit regions
	   {		
	      hdc = BeginPaint(hWnd,&ps);
         RECT r;
         SIZE te;
         char out[2];
         out[1] = '\0';
         GetClientRect(hWnd,&r);
         hBrush = (HBRUSH)CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
         FillRect(hdc, &r, hBrush); // Draw square of solid colour
         SelectObject(hdc,controlFont);
         SetBkMode(hdc,TRANSPARENT);
         if(info)
         {
				short x = 5;
            for(int i = 0; i < strlen(info->syntax.Str()); i++)
            {
					char c = info->syntax.Str()[i];
               if(info->colorize && (c == '-' || (!islower(c) && isalpha(c))))
               {
                  if(c == '-') c = '_';
                  SetTextColor(hdc,RGB(80,80,80));
                  out[0] = tolower(c);
               }               
               else
               {
                  SetTextColor(hdc,RGB(0,0,0));
                  out[0] = c;
               }

               TextOut(hdc,x,1,out,1);
	            GetTextExtentPoint32(hdc,out,1, &te);
               x += te.cx; 
            }
         }
	      EndPaint(hWnd,&ps); 
         DeleteObject(hBrush);
         return(0);
	   }
	   
	   case(WM_SIZE): // Edit window is being resized so resize syntax window
	   {	
		   short w  = LOWORD(lParam);
		   short h = HIWORD(lParam);
         MoveWindow(hWnd,1,3,w,h,true);
		   break;
		}	   	   
	}
	
	return(DefWindowProc( hWnd, messg, wParam, lParam ));
}	

void ProcessAndDisplaySyntax(HWND syntaxWnd, char* selectedStr, StatusBoxInfo* statusInfo, bool classCmd, EditRegion *edReg, bool showProcs);

void UpDateCLISyntax(WinData *win, HWND edWin, long startSel, long endSel, bool showProcs)
{
   char *name = 0;
   char lineNr[20];
	HWND syntaxWnd = 0;

	// Check that this editor has a status box and then get it
   if(win->statusbox)
   {
      ObjectData* obj = win->statusbox;
      StatusBoxInfo* info = (StatusBoxInfo*)obj->data;

      if(info)
      {
		// Update the line number field
         syntaxWnd = info->subWindow;
         sprintf(lineNr,"%ld",GetCurrentLineNumber(edWin)+1);
         SendMessage(GetParent(syntaxWnd), SB_SETTEXT, (WPARAM)1, (LPARAM) lineNr);

         if(startSel == 0) // Why?
            return;
       
      // Extract the whole delimited word around the editor insertion point
         startSel--; // Why?
         endSel--;    	
	   	bool classCmd=false;
	   	short userClassCmd=0;
 	      name = ExpandToFullWord(edWin,BEFORE_PROCEDURE,AFTER_PROCEDURE,startSel,endSel,classCmd,userClassCmd);

		// Is Ctrl+left button pressed then show syntax
			if(((GetAsyncKeyState(VK_CONTROL) & 0x08000) && GetAsyncKeyState(VK_LBUTTON) & 0x08000)) 
				showProcs = true;

			ProcessAndDisplaySyntax(syntaxWnd,name,info,classCmd,0,showProcs);

			if(name)
            delete [] name;
      }
   }
}


void UpDateEditSyntax(WinData *win, EditRegion *edReg, long startSel, long endSel, bool showProcs)
{
   char *name = 0;
   char lineNr[20];
	HWND syntaxWnd = 0;

	// Check that this editor has a status box and then get it
   HWND edWin = edReg->edWin;
   if(win->statusbox)
   {
      ObjectData* obj = win->statusbox;
      StatusBoxInfo* info = (StatusBoxInfo*)obj->data;

      if(info)
      {
		// Update the line number field
         syntaxWnd = info->subWindow;
         sprintf(lineNr,"%ld",GetCurrentLineNumber(edWin)+1);
         SendMessage(GetParent(syntaxWnd), SB_SETTEXT, (WPARAM)1, (LPARAM) lineNr);

         if(startSel == 0) // Why?
            return;
       
      // Extract the whole delimited word around the editor insertion point
         startSel--; // Why?
         endSel--;    	
	   	bool classCmd=false;
	   	bool funcCmd=false;
		   short userClassCmd=0;
 	      name = ExpandToFullWord(edWin,BEFORE_PROCEDURE,AFTER_PROCEDURE,startSel,endSel,classCmd,userClassCmd);

		// Is Ctrl+left button pressed then show syntax
			if(((GetAsyncKeyState(VK_CONTROL) & 0x08000) && GetAsyncKeyState(VK_LBUTTON) & 0x08000)) 
				showProcs = true;

			ProcessAndDisplaySyntax(syntaxWnd,name,info,classCmd, edReg,showProcs);

			if(name)
            delete [] name;
      }
   }
}

extern void ExtractClassProcName(char *objectName, char *procName);
extern void ExtractProcedureName(char *objectName, char *procName);

void ProcessAndDisplaySyntax(HWND syntaxWnd, char* selectedStr, StatusBoxInfo* statusInfo, bool classCmd, EditRegion *edReg, bool showProcs)
{
	char objectStr[MAX_STR];
   char procName[MAX_STR];
	CText cname = selectedStr;
	char *syntax = 0;
	char calledMacro[MAX_STR];
	char calledPath[MAX_STR];
	char calledProcedure[MAX_STR];
	char fullMacroName[MAX_STR];
	HWND edWin = 0;	
	char cwd[MAX_PATH];

	GetCurrentDirectory(MAX_PATH,cwd);

	if(edReg)
		edWin = edReg->edWin;

	strncpy(objectStr,selectedStr,MAX_STR);

// Internal or external class command? ie. className->procedure
   if(strstr(selectedStr,"->") && showProcs)
	{
		ExtractClassProcName(objectStr,procName);

		if(procName[0] == '\0')
			return;

	// Allow for self->procName possibility
		if(!strcmp(objectStr,"self")) 
			strncpy(fullMacroName,edReg->edName,MAX_STR);
		else
			strncpy(fullMacroName,objectStr,MAX_STR);

	// Convert class to macro name if possible
		extern CText *gClassName;
		extern CText *gClassMacro;
		extern int gNrClassDeclarations;
		int n = gNrClassDeclarations;
		for(int i = 0; i < n; i++) 
		{
			if(!strcmp(gClassName[i].Str(),objectStr))
			{
				strncpy(fullMacroName,gClassMacro[i].Str(),MAX_STR);
				break;
			}
		}
	// Get the procedure syntax (syntax variable has form - macroName:procedureName)
		strncat(fullMacroName,":",MAX_STR);
		strncat(fullMacroName,procName,MAX_STR);
		GetFunctionSyntax((edReg ? edReg->edPath : cwd),(edReg ? edReg->edName: ""),fullMacroName,&syntax,calledPath,calledMacro,calledProcedure);

	// Check for internal classes
		if(!syntax)
			GetClassCommandSyntax(procName, &syntax); 
	}

// Check for external macro procedures
	else if(strstr(selectedStr,":") && showProcs)
	{
		ExtractProcedureName(objectStr,procName);

		if(procName[0] == '\0')
			return;

		// Get the procedure syntax (syntax variable has form - macroName:procedureName)					
		GetFunctionSyntax((edReg ? edReg->edPath : cwd),(edReg ? edReg->edName: ""),selectedStr,&syntax,calledPath,calledMacro,calledProcedure);
	}	

// Check for internal normal or DLL commands (syntax is static string here doesn't need deleteing
	else 
	{
		syntax = GetCommandSyntax(objectStr);
		if(!syntax) // Is it a DLL command
		{
			syntax = GetDLLCommandSyntax(objectStr);
		}

		if(!syntax && showProcs) // Is it an external macro (no procedure name)?
		{
			// Get the procedure syntax (syntax variable has form - macroName)					
			GetFunctionSyntax((edReg ? edReg->edPath : cwd),(edReg ? edReg->edName: ""),selectedStr,&syntax,calledPath,calledMacro,calledProcedure);
		}
	}

		
// Display the syntax
	if(syntax)
	{
		// Update the syntax window in the status line
		statusInfo->syntax = syntax;
		statusInfo->colorize = true;
		MyInvalidateRect(syntaxWnd,NULL,false);
		if(syntax) 
			delete [] syntax;
		return;
	}
}
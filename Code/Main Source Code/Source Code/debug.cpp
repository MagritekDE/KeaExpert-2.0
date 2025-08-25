#include "stdafx.h"
#include "command.h"
#include "debug.h"
#include "bitmap.h"
#include "control.h"
#include "defineWindows.h"
#include "edit_class.h"
#include "events_edit.h"
#include "globals.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include "interface.h"
#include "process.h"
#include "scanstrings.h"
#include "variablesOther.h"
#include "memoryLeak.h"

#pragma warning (disable: 4996) // Ignore deprecated library functions

DebugInfo gDebug;
Variable bpVarList;   // List of break point variables
Variable *bpVar;      // Pointer to current break point list


void DebugInitialise()
{
   gDebug.enabled = false;
   gDebug.step = false;
   gDebug.mode = "off";
   gDebug.message = "off";
   gDebug.var = "local";
   gDebug.cmd = "";
   gDebug.win = NULL;
}


void DebuggerClose()
{
   gDebug.mode = "off";
   gDebug.enabled = false;
   gDebug.step = true;
   gDebug.win = NULL;
   bpVarList.RemoveAll();
   bpVar = NULL;
}

void DebuggeeClose()
{
   gDebug.mode = "off";
   gDebug.enabled = false;
   gDebug.step = true;
   if(gDebug.win)
   {
      if(curEditor)
      {
         curEditor->debugLine = -1;
         DrawDebugBreakPointStrip(curEditor);
         SendMessage(curEditor->edWin,EM_HIDESELECTION ,true,0);
      }
      SendMessageToGUI("debug,stopped",gDebug.win->nr); 
      gDebug.message = "stopped";
   }
}


int DebugInterface(Interface* itfc, char args[])
{
   CText mode,state = "false";
   short n;

   if((n = ArgScan(itfc,args,1,"mode","ee","tt",&mode,&state)) < 0)
      return(n);

   if(mode == "variables")
   {
      gDebug.var = state;
   }
   else
   {
      gDebug.mode = mode;

      if(state == "true")
         gDebug.step = true;
      else
         gDebug.step = false;
   }

   return(OK);
}


short DebugProcess(Interface *itfc, Command *cmd)
{
   static short macroDepth;

   if(!curEditor || !curEditor->debug)
     return(OK);

  if(!bpVar)
     return(OK);

   BreakPointInfo* bpInfo = (BreakPointInfo*)bpVar->GetData();

   if(!bpInfo)
      return(OK);

// Ignore internal procedure commands if we are stepping over a procedure
  if(macroDepth != 0 && itfc->macroDepth != macroDepth && gDebug.mode == "stepover")
  {
     return(OK);
  }

// Ignore commands in the same macro if we are stepping out of a procedure
  if(macroDepth != 0 && itfc->macroDepth != macroDepth-1 && gDebug.mode == "stepout")
  {
     return(OK);
  }

// We have finished stepping over a procedure so revert to stepinto
  if(itfc->macroDepth == macroDepth && gDebug.mode == "stepover")
  {
     gDebug.mode = "stepinto";
     macroDepth = 0;
  }

// We have finished stepping out of a procedure so revert to stepinto
  if(itfc->macroDepth == macroDepth-1 && gDebug.mode == "stepout")
  {
     gDebug.mode = "stepinto";
     macroDepth = 0;
  }

// Run to a particular break point print the variables and wait for next instruction
   if(gDebug.mode == "runto")
   {
      int sz = bpInfo->size;
    //  TextMessage("%s %s %d\n", itfc->macroName.Str(), bpInfo->macroName.Str(), itfc->lineNr);

      //for (int i = 0; i < sz; i++)
      //{
      //   TextMessage("%d %d \n", i, (int)bpInfo->list[i].selected);
      //}
      if(!stricmp(itfc->macroName.Str(),bpInfo->macroName.Str()) && bpInfo->list[itfc->lineNr].selected)
      {
       //  TextMessage("Found break on line %d\n", itfc->lineNr);
         if(stricmp(bpInfo->macroName.Str(),curEditor->edName))
         {
            LoadEditorGivenPath(bpInfo->macroPath.Str(),bpInfo->macroName.Str());
            UpdateDebugBreakPointList(curEditor);
         }
         SendMessageToGUI("debug,breakpoint",gDebug.win->nr);
         gDebug.message = "breakpoint";
         curEditor->debugLine = itfc->lineNr;
         curEditor->SelectEditorLine(itfc->lineNr);

         if(gDebug.var != "" )
         {
            PrintString(itfc,gDebug.var.Str());
            TextMessage("\n>> ");
         }

         gDebug.enabled = true;
         gDebug.step = false;
         WinData *gui = GetGUIWin();
         while(gDebug.step == false && !gAbortMacro)
         {
            ProcessBackgroundEvents();
         }
         if(curEditor)
         {
            curEditor->debugLine = -1;
            DrawDebugBreakPointStrip(curEditor);
         }
         if(gDebug.win) // Debugger might have closed
         {
            SendMessageToGUI("debug,running",gDebug.win->nr); 
            gDebug.message = "running";
            ProcessBackgroundEvents();
				WinData::SetGUIWin(gui);
            gDebug.enabled = false;
            gDebug.step = false;
         }
      }
      else
      {
         if(gDebug.message == "waiting")
         {
            SendMessageToGUI("debug,running",gDebug.win->nr); 
            gDebug.message = "running";
            ProcessBackgroundEvents();
         }
      }
   }

// Step to the next command print locals and wait for next instruction
   else if(gDebug.mode == "stepinto")
   {
      SendMessageToGUI("debug,breakpoint",gDebug.win->nr); 
      curEditor->debugLine = itfc->lineNr;
      curEditor->SelectEditorLine(itfc->lineNr);
      if(gDebug.var != "")
      {
         PrintString(itfc,gDebug.var.Str());
         TextMessage("\n>> ");
      }

      gDebug.enabled = true;
      gDebug.step = false;
      WinData *gui = GetGUIWin();
      while(gDebug.step == false && !gAbortMacro)
      {
         ProcessBackgroundEvents();
      }
      if(curEditor)
      {
         curEditor->debugLine = -1;
         DrawDebugBreakPointStrip(curEditor);
      }
      if(gDebug.win) // Debugger might have closed
      {
         SendMessageToGUI("debug,running",gDebug.win->nr); 
         gDebug.message = "running";
         ProcessBackgroundEvents();
			WinData::SetGUIWin(gui);
         gDebug.enabled = false;
         gDebug.step = false;
      }
      if(gDebug.mode == "stepover" && cmd->getType() == 1)
      {
         if(!strcmp(cmd->getCmdName(),"endproc") || !strcmp(cmd->getCmdName(),"return"))
         {
            gDebug.mode = "stepout";
         }
      }
   }

// The next instruction is to "step over" a procedure 
   if(gDebug.mode == "stepover")
   {
      macroDepth = itfc->macroDepth;
   }

// The next instruction is to "step out" of procedure
   if(gDebug.mode == "stepout")
   {
      macroDepth = itfc->macroDepth;
      if(curEditor)
      {
         curEditor->debugLine = -1;
         DrawDebugBreakPointStrip(curEditor);
      }
   }

// The next instruction is to print out an expression
   if(gDebug.mode == "printExpression")
   {
      // Stay in this mode until a different instruction is received
      while(gDebug.mode == "printExpression")
      {
         itfc->inCLI = true;
         gDebug.mode = "off";
         PrintString(itfc,gDebug.cmd.Str());
         TextMessage("\n>> ");
         gDebug.mode = "printExpression";
         itfc->inCLI = false;
         gAbortMacro = false;
         SetFocus(cliEditWin);
         gDebug.enabled = true;
         gDebug.step = false;
         WinData *gui = GetGUIWin();
         while(gDebug.step == false || gAbortMacro)
         {
            ProcessBackgroundEvents();
				gShowWaitCursor = false;
         }
         if(curEditor)
         {
            curEditor->debugLine = -1;
            DrawDebugBreakPointStrip(curEditor);
         }
         if(gDebug.win) // Debugger might have closed
         {
            SendMessageToGUI("debug,running",gDebug.win->nr);
            gDebug.message = "running";
            ProcessBackgroundEvents();
          
				WinData::SetGUIWin(gui);
            gDebug.enabled = false;
            gDebug.step = false;
         }
      }
   }

// Turn off the debugging
   if(gDebug.mode == "off")
   {
      if(curEditor)
      {
          curEditor->debugLine = -1;
          DrawDebugBreakPointStrip(curEditor);

          SendMessage(curEditor->edWin,EM_HIDESELECTION ,true,0);
      }
      if(gDebug.win) // Debugger might have closed
      {
         SendMessageToGUI("debug,stopped",gDebug.win->nr); 
         gDebug.message = "stopped";
         itfc->debugging = false;
         macroDepth = 0;
      }
   }

   return(OK);
}

/*******************************************************
 When loading a new text file into the debug editor
 either load a previously cached list or make a new one
********************************************************/


// Need to add the new breakpoint list to the cache. Only
// the cache has the full list, ed->bpList is just a pointer
// to the current one.
void  UpdateDebugBreakPointList(EditRegion *ed)
{
   Variable *var;

   var = FindDebugBreakPointList(ed->edPath,ed->edName);

   if(var)
   {
      bpVar = var;
   }
   else // New list
   {
      long nrLines = ed->CountLines();

      if(nrLines > 0)
      {
         BPList *list = new BPList[nrLines];
         for(int i = 0; i < nrLines; i++)
            list[i].selected = false;
         BreakPointInfo *bpInfo = new BreakPointInfo;
         bpInfo->list = list;
         bpInfo->size = nrLines;
         bpInfo->macroName = ed->edName;
         bpInfo->macroPath = ed->edPath;
         bpVar = bpVarList.Add(BREAKPOINTINFO,ed->edName);
         bpVar->AssignBreakPointList((char*)bpInfo,nrLines);
      }
   }
} 


void  UpdateDebugBreakPointList(char *path, char *macro)
{
   Variable *var;

   var = FindDebugBreakPointList(path, macro);

   if(var)
   {
      bpVar = var;
   }
   else // New list
   {
      bpVar = NULL;
   }
} 

// Search in the editor region structure for an existing break point list
// for the specified macro

Variable* FindDebugBreakPointList(char *path, char *macro)
{
   BreakPointInfo *bpInfo;
   Variable *var;

	for(var = bpVarList.next; var != NULL; var = var->next)
	{
	   if(!stricmp(var->GetName(),macro))
	   {
         bpInfo = (BreakPointInfo*)var->GetString(); 
         if(ComparePathNames(bpInfo->macroPath.Str(),path) &&
            ComparePathNames(bpInfo->macroName.Str(),macro))
	         return(var);
	   }
	}

   return(NULL);
}

// Compare path names ignoring trailing \ and capitalisation

bool ComparePathNames(char *path1, char* path2)
{
   int size1 = strlen(path1);
   int size2 = strlen(path2);

   if(path1[size1-1] == '\\')
      size1--;
   if(path2[size2-1] == '\\')
      size2--;

   if(size1 != size2)
      return(false);

   for(int i = 0; i < size1; i++)
   {
      if(tolower(path1[i]) != tolower(path2[i]))
         return(false);
   }

   return(true);
}

void DrawDebugBreakPointStrip(EditRegion *er)
{
   RECT r;
   HDC hdc;
   POINT pnts[8] = {-5,-2,0,-2,0,-5,5,0,0,5,0,2,-5,2,-5,-2};

   hdc = GetDC(er->edWin);
   GetClientRect(er->edWin,&r);
   int height = r.bottom;
   int width = 25;

   int lineHeight = er->GetLineHeight();
   if(lineHeight == 0)
      return;

   if(!bpVar)
      return;

   BreakPointInfo* bpInfo = (BreakPointInfo*)bpVar->GetData();

// Make sure break point list matches editor
   if(!ComparePathNames(bpInfo->macroName.Str(),curEditor->edName) ||
      !ComparePathNames(bpInfo->macroPath.Str(),curEditor->edPath))
   {
       LoadEditorGivenPath(bpInfo->macroPath.Str(),bpInfo->macroName.Str());
       UpdateDebugBreakPointList(curEditor);
   }

	int nrLines = (height/lineHeight) + 1; 
   int firstLine = SendMessage(er->edWin, EM_GETFIRSTVISIBLELINE, 0, 0 );
   int totLines = SendMessage(er->edWin, EM_GETLINECOUNT, 0, 0 );

   HBITMAP hBitmap;
   long newWidth;
   GenerateBitMap(width,height, &hBitmap, hdc, newWidth);

   HDC hdcMem = CreateCompatibleDC(hdc);
   SelectObject(hdcMem,hBitmap);
   SetRect(&r,0,0,width,height);
 //  HBRUSH bkBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
   HBRUSH bkBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
   FillRect(hdcMem,&r,bkBrush);

   MoveToEx(hdcMem,24,0,NULL);
   LineTo(hdcMem,24,height);

   int xp = 12;
   int sz = 7;
   int yp;
   HBRUSH symbolBrush = CreateSolidBrush(RGB(128,0,0));
   HBRUSH arrowBrush = CreateSolidBrush(RGB(255,255,0));
   HPEN symbolPen   = CreatePen(PS_SOLID,0,RGB(255,255,255));
   HPEN arrowPen   = CreatePen(PS_SOLID,0,RGB(128,0,0));
 //  HPEN symbolPen   = CreatePen(PS_SOLID,0,GetSysColor(COLOR_BTNFACE));
   SelectObject(hdcMem,symbolBrush);


   for(int i = firstLine; i < firstLine+nrLines, i < totLines; i++)
   {
      if(bpInfo->list[i].selected)
      {
         SelectObject(hdcMem,symbolPen);
         SelectObject(hdcMem,symbolBrush);
         SendMessage(er->edWin, EM_LINEINDEX,  i, 0 );
         yp = curEditor->GetLineYPosition(i) + lineHeight/2;
         Ellipse(hdcMem,xp-sz,yp-sz,xp+sz+1,yp+sz+1);
      }

      if(er->debugLine == i)
      {
         SelectObject(hdcMem,arrowPen);
         SelectObject(hdcMem,arrowBrush);
         SendMessage(er->edWin, EM_LINEINDEX,  i, 0 );
         yp = curEditor->GetLineYPosition(i) + lineHeight/2;
         MoveToEx(hdcMem,xp,yp, NULL);
         for(int j = 0; j < 8; j++)
         {
            pnts[j].x += xp;
            pnts[j].y += yp;
         }
         Polygon(hdcMem,pnts,8);

         //LineTo(hdcMem,xp+5,yp);
         //LineTo(hdcMem,xp,yp+5);
         //MoveToEx(hdcMem,xp+5,yp, NULL);
         //LineTo(hdcMem,xp,yp-5);
      }
   }

   BitBlt(hdc,0,0,width,height,hdcMem,0,0,SRCCOPY);

   DeleteObject(symbolPen);
   DeleteObject(arrowPen);
   DeleteObject(symbolBrush);
   DeleteObject(arrowBrush);
//   DeleteObject(bkBrush);
   ReleaseDC(er->edWin,hdc);
   DeleteObject(hBitmap);
}

short AddDebugBreakPoint(int xPos, int yPos, EditRegion *er)
{
   RECT r;
   GetClientRect(er->edWin,&r);
   int width = 25;
   int height = r.bottom;
   Variable *var;

// Find the break-point list for this editor
   var = FindDebugBreakPointList(er->edPath,er->edName);
   if(!var)
      return(0);

   int lineHeight = er->GetLineHeight();
   if(lineHeight == 0)
      return(0);

   BreakPointInfo* bpInfo = (BreakPointInfo*)var->GetData();

   if(xPos > 0 && xPos < width && yPos > 0 && yPos < height)
   {
      HDC hdc = GetDC(er->edWin);
      int firstLine = SendMessage(er->edWin, EM_GETFIRSTVISIBLELINE, 0, 0 );
      SendMessage(er->edWin, EM_GETLINECOUNT, 0, 0 );
      int lypos = er->GetLineYPosition(firstLine);
      int h = er->GetLineHeight();
      if(h > 0)
      {
         int selLine = (yPos-lypos)/h + firstLine;
         bool selected = bpInfo->list[selLine].selected;
         bpInfo->list[selLine].selected = !selected;
      }
      ReleaseDC(er->edWin,hdc);
      r.right = 25;
      MyInvalidateRect(er->edWin,&r,FALSE);
      return(1);
   }
   return(0);
}

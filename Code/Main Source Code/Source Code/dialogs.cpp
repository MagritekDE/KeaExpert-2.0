#include "stdafx.h"
#include "dialogs.h"
#include "defineWindows.h"
#include "files.h"
#include "globals.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include "import_export_utilities.h"
#include "interface.h"
#include "main.h"
#include "scanstrings.h"
#include "memoryLeak.h"

/*************************************************************
   Allow the user to select a filename from a dialog.
   Passed parameters are the window prompt and default extension
   The filename is returned in the global answer variable
**************************************************************/

int GetFileName(Interface* itfc ,char args[])
{
   short r,err;
   char fileName[MAX_PATH] = "";
   static short index;
   static CText type = "Open";
   static CText title = "Open";
   static CText range = "mac";
   static CText rangeName = "Macro files";
   WinData* oldGUI;

// Get variable name, window title and file range to display
   if((r = ArgScan(itfc,args,4,"type,windowTitle,rangeTitle,range,[default]","eeeee","tttts",&type,&title,&rangeName,&range,fileName)) < 0)
      return(r);

// Display Load or Save prompt
   type.LowerCase();
   
   oldGUI = GetGUIWin();

   if(type == "save")
   {
      err = FileDialog2(prospaWin, false, gCurrentDir, fileName, title.Str(), CentreHookProc, NULL,
                       noTemplateFlag, &index, rangeName.Str(), range.Str());   

   }
   else if(type == "open" || type == "load")
   {
		 //  TextMessage("\n\n  (getfilename) dir = %s\n",gCurrentDir);

      err = FileDialog2(prospaWin, true, gCurrentDir, fileName, title.Str(), CentreHookProc, NULL,
                       noTemplateFlag, &index, rangeName.Str(), range.Str());  
 
   }
   else
   {
      ErrorMessage("invalid dialog type");
      return(ERR);
   } 


	WinData::SetGUIWin(oldGUI); 

// Return filename, or "cancel" if cancel button pressed
   if(err == OK)
   {
      GetCurrentDirectory(MAX_PATH,gCurrentDir);
      itfc->retVar[1].MakeAndSetString(fileName);
   }
   else
      itfc->retVar[1].MakeAndSetString("cancel");

   itfc->nrRetValues = 1;

   return(OK);
}
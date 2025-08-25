#include "stdafx.h"
#include <assert.h>
#include <shlwapi.h>
#include <dlgs.h>
#include "cli_files.h"
#include "cArg.h"
#include "trace.h"
#include "defineWindows.h"
#include "dialog_utilities.h"
#include "edit_class.h"
#include "evaluate.h"
#include "events_edit.h"
#include "files.h"
#include "globals.h"
#include "interface.h"
#include "list_functions.h"
#include "load_save_data.h"
#include "main.h"
#include "metafile.h"
#include "mymath.h"
#include "plot.h"
#include "plot3dClass.h"
#include "PlotFile.h"
#include "PlotWindow.h"
#include "scanstrings.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "guiWindowClass.h"
#include "memoryLeak.h"

#pragma warning (disable: 4996) // Ignore deprecated library functions

char gPlot3DDirectory[300];
char gImport3DDataDirectory[300];
char gExport3DDataDirectory[300];
void GetCommonFileDialogExtension(LPOPENFILENAME ofn, char *ext, int maxlen);
extern int IsApplicationOpen(Interface *itfc, char arg[]);

/****************************************************************************
   Set default paths for different file types
   Note: paths should not have double backslashes i.e. c:dir1\\dir2
****************************************************************************/

int SetPathNames(Interface *itfc, char args[])
{
   short nrArgs;
   CText name;
   CText path;
   short varType;
   Variable *var;

   Variable *ans = &itfc->retVar[1];
   
// Get filename from user *************
   if((nrArgs = ArgScan(itfc,args,0,"folder_name, path","ec","tt",&name,&path)) < 0)
     return(nrArgs);  


   if(var = GetVariable(itfc, ALL_VAR, path.Str(),varType))
   {
      if(varType != UNQUOTED_STRING)
      {
         ErrorMessage("Only string variables are valid");
         return(ERR);
      }
      path = var->GetString();
   }

// List current directories if no arguments
   if(nrArgs == 0)
   {
      TextMessage("\n\n#### Prospa directories ###\n\n");
      TextMessage("   plot1d:    '%s'\n",PlotFile1D::getCurrPlotDirectory());
      TextMessage("   plot2d:    '%s'\n",PlotFile2D::getCurrPlotDirectory());
      TextMessage("   data:      '%s'\n",PlotFile::getCurrDataDirectory());
      TextMessage("   import1d:  '%s'\n",PlotFile1D::getCurrImportDataDirectory());
      TextMessage("   export1d:  '%s'\n",PlotFile1D::getCurrExportDataDirectory());
      TextMessage("   import2d:  '%s'\n",PlotFile2D::getCurrImportDataDirectory());
      TextMessage("   export2d:  '%s'\n",PlotFile2D::getCurrExportDataDirectory());
      TextMessage("   import3d:  '%s'\n",gImport3DDataDirectory);       
      TextMessage("   export3d:  '%s'\n",gExport3DDataDirectory);
      TextMessage("   macrodata: '%s'\n",PlotFile::getCurrMacroDirectory());
      TextMessage("   clipath:   '%s'\n\n",gCurrentDir);
   }


// If only folder_name passed then return current value
   else if(nrArgs == 1)
   {
      if(name == "plot1d")
			ans->MakeAndSetString(PlotFile1D::getCurrPlotDirectory());
         
      else if(name == "plot2d")
         ans->MakeAndSetString(PlotFile2D::getCurrPlotDirectory());

      else if(name == "data")
			ans->MakeAndSetString(PlotFile::getCurrDataDirectory());
         
      else if(name == "import1d")
         ans->MakeAndSetString(PlotFile1D::getCurrImportDataDirectory());
         
      else if(name == "export1d")
         ans->MakeAndSetString(PlotFile1D::getCurrExportDataDirectory());

      else if(name == "import2d")
			ans->MakeAndSetString(PlotFile2D::getCurrImportDataDirectory());
         
      else if(name == "export2d")
			ans->MakeAndSetString(PlotFile2D::getCurrExportDataDirectory());

      else if(name == "import3d")
         ans->MakeAndSetString(gImport3DDataDirectory);
         
      else if(name == "export3d")
         ans->MakeAndSetString(gExport3DDataDirectory);

      else if(name == "macrodata")
         ans->MakeAndSetString(PlotFile::getCurrMacroDirectory());

      else if(name == "clipath")
         ans->MakeAndSetString(gCurrentDir);

      return(OK);
   }

 // If 2 arguments returned then update folder names

   else if(nrArgs == 2)
   {

      if(name == "plot1d")
      {
         PlotFile1D::setCurrPlotDirectory(path.Str());
      }
         
      else if(name == "plot2d")
      {
		   PlotFile2D::setCurrPlotDirectory(path.Str());
      }
      
      else if(name == "data")
      {
		   PlotFile::setCurrDataDirectory(path.Str());
      }
         
      else if(name == "import1d")
      {
		   PlotFile1D::setCurrImportDataDirectory(path.Str());
      }
         
      else if(name == "export1d")
      {
         PlotFile1D::setCurrExportDataDirectory(path.Str());
      }

      else if(name == "import2d")
      {
         PlotFile2D::setCurrImportDataDirectory(path.Str());
      }
         
      else if(name == "export2d")
      {
         PlotFile2D::setCurrExportDataDirectory(path.Str());
      }

      else if(name == "import3d")
      {
         strncpy(gImport3DDataDirectory,path.Str(),MAX_PATH-1);
         gImport3DDataDirectory[MAX_PATH-1] = '\0';
      }
         
      else if(name == "export3d")
      {
         strncpy(gExport3DDataDirectory,path.Str(),MAX_PATH-1);
         gExport3DDataDirectory[MAX_PATH-1] = '\0';
      }

      else if(name == "macrodata")
      {
		   PlotFile::setCurrMacroDirectory(path.Str());      
	   }

      else if(name == "clipath")
      {
         strncpy(gCurrentDir,path.Str(),MAX_PATH-1);
         gCurrentDir[MAX_PATH-1] = '\0';
      }

      else if(name == "workdir")
         userWorkingVar->MakeAndSetString(path.Str()); 

      else if(name == "macrodir")
         userMacroVar->MakeAndSetString(path.Str()); 
   }
   return(OK);
}


/************************************************************************
* Replacement for "fgets" which can handle Mac or PC files by ending    *
* a line with either the \n or \r characters.                           *
************************************************************************/

void 
GetLineFromFile(char *line,short size, FILE *fp )
{
   long i = 0;
   char c;
   
   do
   {
      c = (char)fgetc(fp);
    
      if(c == '\r' || c == '\n' || c == EOF)
      {
         line[i] = '\0';
         break;
      }
      line[i++] = c;
   }
   while(i < size);
}


/************************************************************************
*         Place the standard dialog in the centre of the window         *
************************************************************************/

UINT CentreHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	switch (message)
	{
      case(WM_NOTIFY): // Process events from standard part of dialog
	   {
	      LPOFNOTIFY lpon = (LPOFNOTIFY)lParam;
	      if(lpon->hdr.code == CDN_INITDONE) // Std dialog has finished updating so centre it in window
         {
            PlaceDialogOnTopInCentre(hWnd,lpon->hdr.hwndFrom);   
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



/************************************************************************
*         Place the standard dialog in the centre of the window         *
*          Also replace filename as the extensions are modified.        *
************************************************************************/

#define MAX_EXTENSION 10

UINT CentreHookSaveProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
   char fileName[MAX_PATH];

	switch (message)
	{
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
			   case(CDN_FILEOK): // OK button has been pressed in the std dialog
			   {
               char ext[MAX_EXTENSION];
               GetCommonFileDialogExtension(lpon->lpOFN,ext,MAX_EXTENSION);
			      SendMessage(lpon->hdr.hwndFrom,CDM_GETSPEC ,(WPARAM)MAX_STR,(LPARAM) (LPTSTR)fileName);
               RemoveExtension(fileName);
			      strcat(fileName,ext);
            }
            case(CDN_TYPECHANGE):
            {
               char newExt[MAX_EXTENSION];
               GetCommonFileDialogExtension(lpon->lpOFN,newExt,MAX_EXTENSION);
			      SendMessage(lpon->hdr.hwndFrom,CDM_GETSPEC ,(WPARAM)MAX_STR,(LPARAM) (LPTSTR)fileName);
               RemoveExtension(fileName);
               strcat(fileName,newExt);
			      SendMessage(lpon->hdr.hwndFrom,CDM_SETCONTROLTEXT,(WPARAM)edt1,(LPARAM)fileName);
               break;
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


void GetCommonFileDialogExtension(LPOPENFILENAME ofn, char *newExt, int maxLen)
{
   // Replace the filename extension with the selected one
   short index = (ofn->nFilterIndex-1)*2+1;

   int i=0,j=1,k = 0;
   while(1)
   {
      if(ofn->lpstrFilter[i++] == '\0')
      {
         if(j++ == index)
            break;
      }
   }
   for(k = i+1; ; k++)
   {
      assert(k-i-1 < maxLen);
      newExt[k-i-1] = ofn->lpstrFilter[k];
      if(ofn->lpstrFilter[k] == '\0')
         break;
   }
   newExt[k-i-1] = '\0';

}

/************************************************************************
                   Load prospa plot, data or text data   

   Last modified: 23 August 2007 ... modified for CText

************************************************************************/

int Load(Interface* itfc ,char args[])
{
   short nrArgs;
   long type,owner;
   static CText fileName;
   CText filetype = "useextension";
   CText display = "nodisplay";
   CText extension;
   FILE *fp;
   bool useext = false;

   Variable *ans = &itfc->retVar[1];
   Variable *ans2 = &itfc->retVar[2];

// Get filename from user *************  
   if((nrArgs = ArgScan(itfc,args,1,"filename, [option, [display]]","eee","ttt",&fileName,&filetype,&display)) < 0)
     return(nrArgs);  

// Check to see if we want to use the extension *****
   if(filetype == "useextension")
      useext = true;

// Get extension **********************
   GetExtension(fileName,extension);
		 
// Process a plot file *******************
   if((extension == "pt1" && useext) || filetype == "pt1")
   { 

	// Open file **************************	      
	   if(!(fp = fopen(fileName.Str(),"rb")))
		{
		   ErrorMessage("can't open file '%s'",fileName);
		   return(ERR);
		}
		         
	// Read file owner *********************	
	   fread(&owner,sizeof(long),1,fp);
	
	// Read file type **********************	
	   fread(&type,sizeof(long),1,fp);
	   fclose(fp);
	   
	// Load data ***************************	   
	   if(owner != 'PROS')
	   {
	      ErrorMessage("'%s' is not a '%s' file",fileName.Str(),AppName);
		   return(ERR);
	   }  
	      
	   if(type == 'PL1D')
	   {
			if(Plot1D::curPlot()->plotParent->LoadPlots(gCurrentDir, fileName.Str()) == ERR)
            return(ERR);
         Plot1D::curPlot()->initialiseMenuChecks("load");
 			if(Plot1D::curPlot()->win)
				MyInvalidateRect(Plot1D::curPlot()->win,NULL,false);
	   } 
	   else
	   {
		   ErrorMessage("'%s' is not a native '%s' 1D plot file",fileName.Str(),AppName);
		   return(ERR);   
	   }
   }
   else if((extension == "pt2" && useext) || filetype == "pt2")
   { 
   
	// Open file **************************	      
	   if(!(fp = fopen(fileName.Str(),"rb")))
		{
		   ErrorMessage("can't open file '%s'",fileName.Str());
		   return(ERR);
		} 
		        
	// Read file owner *********************	
	   fread(&owner,sizeof(long),1,fp);
	
	// Read file type **********************
	   fread(&type,sizeof(long),1,fp);
	   fclose(fp);
	   
	// Load data ***************************	   
	   if(owner != 'PROS')
	   {
	      ErrorMessage("'%s' is not a '%s' file",fileName.Str(),AppName);
		   return(ERR);
	   }  
	      
	   if(type == 'PL2D')
	   {
		   Plot2D::curPlot()->plotParent->LoadPlots(gCurrentDir, fileName.Str());
		   Plot2D::curPlot()->DisplayAll(false); 
	   }   
	   else
	   {
		   ErrorMessage("'%s' is not a native '%s' 2D plot file",fileName.Str(),AppName);
		   return(ERR);   
	   }
   } 
   else if((extension == "1d" && useext) || filetype == "1d")
   {
      if(LoadData(itfc,gCurrentDir, fileName.Str(), ans, ans2) == ERR)
         return(ERR);
		if(ans2->GetType() != NULL_VARIABLE)
		{
         itfc->nrRetValues = 2;
			return(OK);
		}
   }
   else if((extension == "2d" && useext) || filetype == "2d")
   {
      if(LoadData(itfc,gCurrentDir, fileName.Str(), ans, ans2) == ERR)
         return(ERR);
   } 
   else if((extension == "3d" && useext) || filetype == "3d")
   {
      if(LoadData(itfc,gCurrentDir, fileName.Str(), ans, ans2) == ERR)
         return(ERR);
   }
   else if((extension == "4d" && useext) || filetype == "4d")
   {
      if(LoadData(itfc,gCurrentDir, fileName.Str(), ans, ans2) == ERR)
         return(ERR);
   }

// Assume its a list file - either load into a variable or send to texteditor
   else if((((extension == "lst") || (extension == "par") || (filetype == "list")) && useext) 
             || filetype == "list" || filetype == "truedoubles")
   {
      if(display == "display")
      {
         if(!curEditor)
         {   
            ErrorMessage("No current editor selected");
            return(ERR);
         }
       // Check to see if current text needs saving 
         if(curEditor->CheckForUnsavedEdits(curEditor->edParent->curRegion) == IDCANCEL)
            return(OK);

         char *text;
         if((text = LoadTextFileFromFolder("", fileName.Str(),"")) == NULL)
         {   
            ErrorMessage("Can't open file '%s'",fileName.Str());
            return(ERR);
         }
         curEditor->CopyTextToEditor(text);
			
         strncpy_s(curEditor->edPath,MAX_PATH,gCurrentDir,_TRUNCATE);
         strncpy_s(curEditor->edName,MAX_PATH,fileName.Str(),_TRUNCATE);
         SetEditTitle();
         delete [] text; 
      }
      else if(display == "nodisplay")
      {
         if(filetype == "truedoubles")
         {
            if(LoadListDataWithTrueDoubles("", fileName.Str(), ans) == ERR)
               return(ERR);
         }
         else
         {
            if(LoadListData("", fileName.Str(), ans) == ERR)
               return(ERR);
         }
      }
      else
      {    
         ErrorMessage("invalid display option '%s'",display);
         return(ERR);
      }
   } 
 
// Assume its a text file - either load into a variable or send to texteditor
   else if(useext || filetype == "text")
   {	
      char *text;           
   // Load text from file
      if((text = LoadTextFileFromFolder("", fileName.Str(),"")) == NULL)
      {   
         ErrorMessage("Can't open file '%s'",fileName.Str());
         return(ERR);
      }
   // Display or assign text
      if(display == "display")
      {
         if(!curEditor)
         {   
            ErrorMessage("No current editor selected");
            return(ERR);
         }
         // Check to see if current text needs saving 
         if(curEditor->CheckForUnsavedEdits(curEditor->edParent->curRegion) == IDCANCEL)
            return(OK);

         curEditor->CopyTextToEditor(text);
         strncpy_s(curEditor->edPath,MAX_PATH,gCurrentDir,_TRUNCATE);
         strncpy_s(curEditor->edName,MAX_PATH,fileName.Str(),_TRUNCATE);
         SetEditTitle();
      }
      else if(display == "nodisplay")
      {
         if(itfc->processEscapes)
            ReplaceEscapedCharacters(text);
         itfc->retVar[1].MakeAndSetString(text);
      }
      else
      {  
         delete [] text;   
         ErrorMessage("invalid option '%s'",display);
         return(ERR);
      }
      delete [] text;     		
   }

   itfc->nrRetValues = 1;

   return(OK);
}


/************************************************************************
*                 Save plot, data or text in native format              *
* Syntax: save(filename,[data])                                         *
*                                                                       *
*   Last modified: 14 September 2007 ... modified for CText             *
************************************************************************/


int Save(Interface* itfc ,char args[])
{
   short nrArgs;
   CText fileName;
   CText extension;
   short type,err = ERR;
   CArg carg;

   Variable *ans = &itfc->retVar[1];

// List parameters if none passed ************************************************************************     
   nrArgs = carg.Count((char*)args);

   switch(nrArgs)
   {
      case(0):
	   {
	      TextMessage("\n   arguments: filename [variable]\n");
         err = OK;
         break;
	   }
	   case(1):
      {
      // Get filename
	      if(Evaluate(itfc,RESPECT_ALIAS,carg.Extract(1),ans) < 0) 
	        return(ERR);
	      fileName.Assign(ans->GetString());
         
		// Get extension
		   GetExtension(fileName,extension);
		
		// Process a plot file		
		   if(extension == "pt1")
		   { 
            if(!Plot1D::curPlot())
            {
	            ErrorMessage("No current plot defined");
	            return(ERR);
	         }
				PlotWindow1D* pp = static_cast<PlotWindow1D*>(Plot1D::curPlot()->plotParent);
		      err = pp->SavePlots(gCurrentDir, fileName.Str(), -1, -1);
		   }
		   else if(extension == "pt2")
		   {
		      if (!Plot2D::curPlot())
				{
	            ErrorMessage("No current plot defined");
	            return(ERR);
				}
				PlotWindow2D* pp = static_cast<PlotWindow2D*>(Plot2D::curPlot()->plotParent);
		      err = pp->SavePlots(gCurrentDir, fileName.Str(), -1, -1);
		   }  
		   else // Text file (any other extension ok)
		   {
		      if(curEditor)
		      {
		         CText title;
		         long length; 

		         strncpy_s(curEditor->edName,MAX_PATH,fileName.Str(),_TRUNCATE);
		         GetCurrentDirectory(MAX_PATH,curEditor->edPath);
					length = SendMessage(curEditor->edWin,WM_GETTEXTLENGTH,0,0) + 1;
               CText text(length);
					SendMessage(curEditor->edWin,WM_GETTEXT,(WPARAM)length,(LPARAM)(text.Str()));
					if(SaveTextFile(curEditor->edPath,curEditor->edName,text.Str()) == ERR)
					   return(ERR);
				//	title.Format("Edit-%hd    %s",ep->nr,curEditor->edName);
					curEditor->edModified = false;
			//		SetWindowText(editWin,title.Str());   
					SetFocus(curEditor->edWin);
		      }
            err = OK;
		   }
		   break;
      }
	   case(2): // Save list, 1D  or 2D or 3D data
	   {

	   // Extract filename
	      if(Evaluate(itfc,RESPECT_ALIAS,carg.Extract(1),ans) < 0) 
	        return(ERR);
	      fileName.Assign(ans->GetString());
      // Get extension
		   GetExtension(fileName,extension);
	   // Extract variable 
	      if((type = Evaluate(itfc,RESPECT_ALIAS,carg.Extract(2),ans)) < 0)
            return(ERR);
 
	      switch(type)
	      {
	         case(LIST):
	         {
			      if(extension != "lst" && extension != "par" && extension != "mac")
	            {
	               ErrorMessage("invalid extension for this data");
	            }
	            else
	            {
	               err = SaveListData(cliWin, gCurrentDir, fileName.Str(), ans);
	            }
	            break;
	         }
            case(STRUCTURE):
            {
               if (extension != "lst" && extension != "par" && extension != "mac")
               {
                  ErrorMessage("invalid extension for this data");
               }
               else
               {
                  err = SaveStructDataAsList(cliWin, gCurrentDir, fileName.Str(), ans);
               }
               break;
            }
            case(UNQUOTED_STRING):
            {
               char *str = ans->GetString();
            // 1D image functions

               if(!strcmp(str,"1d") && Plot1D::curPlot())
               {
                  if(extension == "png" || extension == "jpg" || extension == "bmp" ||
                     extension == "gif" || extension == "emf" || extension == "tif")
                     err = Plot1D::curPlot()->plotParent->SaveAsImage(fileName.Str());
                  else
                     ErrorMessage("invalid extension for 1D image file");
               }

            // 1D legacy code compatibility
               else if(!strcmp(str,"emf1d") && Plot1D::curPlot())
               {
                  if(extension == "emf")
                     err = Plot1D::curPlot()->plotParent->SaveAsImage(fileName.Str());
                  else
                     ErrorMessage("invalid extension for 1D EMF file");
               }
  
            // 2D functions
               else if(!strcmp(str,"2d") && Plot2D::curPlot())
               {
                  if(extension == "png" || extension == "jpg" || extension == "bmp" ||
                     extension == "gif" || extension == "emf" || extension == "tif")
                     err = Plot2D::curPlot()->plotParent->SaveAsImage(fileName.Str());
                  else
                     ErrorMessage("invalid extension for 2D image file");
               }
            // 2D legacy code compatibility
               else if(!strcmp(str,"emf2d") && Plot2D::curPlot())
               {
                  if(extension == "emf")
                     err = Plot2D::curPlot()->plotParent->SaveAsImage(fileName.Str());
                  else
                     ErrorMessage("invalid extension for 2D EMF file");
               }
            // 3D functions
               else if(!strcmp(str,"3d") && cur3DWin && plot3d)
               {
                  if(extension == "png" || extension == "jpg" || extension == "bmp" ||
                     extension == "gif" || extension == "emf" || extension == "tif")
                     err = plot3d->SaveAsImage(fileName.Str());
                  else
                     ErrorMessage("invalid extension for 3D image file");
               }
            // 3D legacy code compatibility
               else if(!strcmp(str,"emf3d"))
               {
                  if(extension == "emf"&& cur3DWin && plot3d)
                     err = plot3d->SaveAsImage(fileName.Str());
                  else
                     ErrorMessage("invalid extension for EMF file");
               }
               else 
               {
                  err = SaveTextFile(gCurrentDir, fileName.Str(), str);
               }
               break;
            }
	         case(MATRIX2D):
	         case(DMATRIX2D):
	         case(CMATRIX2D):
		      {
		         short rows = ans->GetDimY();
		         short cols = ans->GetDimX();
		         if((rows == 1 || cols == 1) && extension == "1d")
	               err = SaveData(cliWin,gCurrentDir, fileName.Str(), ans);
	            else if((rows > 1 || cols > 1) && extension == "2d")
	               err = SaveData(cliWin,gCurrentDir, fileName.Str(), ans);
	            else
	               ErrorMessage("invalid extension for this data");
               break;
	         }
	         case(MATRIX3D):
	         case(CMATRIX3D):
		      {
		         if(extension == "3d")
	               err = SaveData(cliWin,gCurrentDir, fileName.Str(), ans);
	            else
	               ErrorMessage("invalid extension for this data");
               break;
	         }
	         case(MATRIX4D):
	         case(CMATRIX4D):
		      {
		         if(extension == "4d")
	               err = SaveData(cliWin,gCurrentDir, fileName.Str(), ans);
	            else
	               ErrorMessage("invalid extension for this data");
               break;
	         }
	         default:         
		         ErrorMessage("invalid data for save");
		   }
         break;
	   }

      case(3):
      {
      // Get filename
	      if(Evaluate(itfc,RESPECT_ALIAS,carg.Extract(1),ans) < 0) 
	          return(ERR);
	      fileName.Assign(ans->GetString());
      // Get extension
		   GetExtension(fileName,extension);
		
		// Process a plot file		
		   if(extension == "pt1")
		   { 
				PlotWindow1D* pp = static_cast<PlotWindow1D*>(Plot1D::curPlot()->plotParent);
	         if(Evaluate(itfc,RESPECT_ALIAS,carg.Extract(2),ans) < 0) 
	            return(ERR);
            long x = nint(ans->GetReal());
	         if(Evaluate(itfc,RESPECT_ALIAS,carg.Extract(3),ans) < 0) 
	            return(ERR);
            long y = nint(ans->GetReal());
            if(!Plot1D::curPlot())
            {
	            ErrorMessage("No current plot defined");
	            return(ERR);
	         }
		      if(pp->SavePlots(gCurrentDir, fileName.Str(), x, y) == ERR)
               return(ERR);
         }
			else if(extension == "1d")
			{
				Variable ansx,ansy;
	         if(Evaluate(itfc,RESPECT_ALIAS,carg.Extract(2),&ansx) < 0) 
	            return(ERR);
				if(ansx.GetType() == MATRIX2D && ansx.GetDimY() == 1)
				{
					if(Evaluate(itfc,RESPECT_ALIAS,carg.Extract(3),&ansy) < 0) 
						return(ERR);
					if(ansy.GetType() == MATRIX2D || ansy.GetType() == CMATRIX2D && ansy.GetDimY() == 1)
					{
					    err = SaveData(cliWin,gCurrentDir, fileName.Str(), &ansx, &ansy);
				       break;
					}
				}
				else
            {
	            ErrorMessage("invalid (x,y) data types for save command");
	            return(ERR);
	         }
			}
			else if(extension == "par" && !strcmp(carg.Extract(3),"\"truedoubles\""))
	      {
	      // Extract variable 
	         if((type = Evaluate(itfc,RESPECT_ALIAS,carg.Extract(2),ans)) < 0)
               return(ERR);
	         err = SaveListDataTrueDoubles(cliWin, gCurrentDir, fileName.Str(), ans);
	      }
         err = OK;
         break;
      }
  //    case(4):
  //    {
  //       Variable matVar,xAxisVar,yAxisVar;
  //       ArgScan(itfc,args,4,"file,matrix,xAxis,yAxis","eeee","tvvv",&fileName,&matVar,&xAxisVar,&yAxisVar);
  //    
  //    // Get extension
		//   GetExtension(fileName,extension);
		//
		//// Process an image file		
		//   if(extension == "2d")
		//   { 
		//	   err = Save2DData(gCurrentDir, fileName.Str(),&matVar,&xAxisVar,&yAxisVar);
  //       }
  //       else
  //       {
  //          ErrorMessage("invalid extension");
  //          return(ERR);
  //       }
  //    }
      default:
      {
         ErrorMessage("invalid number of arguments");
         return(ERR);
      }

   }

   itfc->nrRetValues = 0;

   return(err);
}

/************************************************************************
         Extract data from a plot (1D or 2D) and return to user       

 Syntax: (x,y) = getplotdata("1d")  or  y = getplotdata("1d")
             m = getplotdata("2d")  or (m,x,y) = getplotdata("2d")

************************************************************************/

int ExtractDataFromPlot(Interface *itfc, char args[])
{
   short nrArgs;
   static CText whichPlot;
   CText mode = "all";

// Get plot to extract data from  *******************
   if((nrArgs = ArgScan(itfc,args,1,"1d/2d","ee","tt",&whichPlot,&mode)) < 0)
     return(nrArgs);  

// Extract 1D data and return to user ***************
   if(whichPlot == "1d")
   {
      if(Plot1D::curPlot() && Plot1D::curPlot()->curTrace())
      {
			Variable* xAsVar;
			Variable* yAsVar;
         if(mode == "all")
         {
				xAsVar = Plot1D::curPlot()->curTrace()->xComponentAsVariable();
				yAsVar = Plot1D::curPlot()->curTrace()->yComponentAsVariable();
         }
         else
         {
				xAsVar = Plot1D::curPlot()->curTrace()->xMinMaxAsVariable();
				yAsVar = Plot1D::curPlot()->curTrace()->yMinMaxAsVariable();
         }
			itfc->retVar[1].FullCopy(xAsVar);
			itfc->retVar[2].FullCopy(yAsVar);
			delete xAsVar;
			delete yAsVar;
	   }
	   else
	   {
			itfc->retVar[1].MakeNullVar();
		   itfc->retVar[2].MakeNullVar();
      }
      itfc->nrRetValues = 2;
      itfc->defaultVar = 2;
   }
// Extract 2D data and return to user ***************
   else if(whichPlot == "2d")
   {
		Plot2D* cur2DPlot;
      if(cur2DPlot = Plot2D::curPlot())
      {
         float *xRange = new float[2];
         float *yRange = new float[2];
	
		   if(cur2DPlot->mat())
         {
            if(mode == "all")
            {
					xRange[0] = cur2DPlot->curXAxis()->base();
					xRange[1] = cur2DPlot->curXAxis()->base()+cur2DPlot->curXAxis()->length();
               yRange[0] = cur2DPlot->curYAxis()->base();
					yRange[1] = cur2DPlot->curYAxis()->base()+cur2DPlot->curYAxis()->length();

			      itfc->retVar[1].MakeAndLoadMatrix2D(cur2DPlot->mat(), cur2DPlot->matWidth(), cur2DPlot->matHeight());
			      itfc->retVar[2].MakeMatrix2DFromVector(xRange,2,1);
			      itfc->retVar[3].MakeMatrix2DFromVector(yRange,2,1);
            }
            else
            {
               long x0 = cur2DPlot->visibleLeft();
               long y0 = cur2DPlot->visibleTop();
               long width = cur2DPlot->visibleWidth();
               long height = cur2DPlot->visibleHeight();
               float totwidth = (float)cur2DPlot->matWidth();
               float totheight = (float)cur2DPlot->matHeight();
			      itfc->retVar[1].MakeAndLoadMatrix2D(NULL, width, height);
               float** mIn = cur2DPlot->mat();
               float** mOut = itfc->retVar[1].GetMatrix2D();
               for(int y = 0; y < height; y++)
                  for(int x = 0; x < width; x++)
                     mOut[y][x] = mIn[y+y0][x+x0];
			      itfc->retVar[2].MakeMatrix2DFromVector(NULL,2,1);
			      itfc->retVar[3].MakeMatrix2DFromVector(NULL,2,1);
               itfc->retVar[2].GetMatrix2D()[0][0] = x0/totwidth*(cur2DPlot->curXAxis()->length()) + cur2DPlot->curXAxis()->base();
               itfc->retVar[2].GetMatrix2D()[0][1] = (x0+width)/totwidth*(cur2DPlot->curXAxis()->length()) + cur2DPlot->curXAxis()->base();
					itfc->retVar[3].GetMatrix2D()[0][0] = y0/totheight*(cur2DPlot->curYAxis()->length()) + cur2DPlot->curYAxis()->base();
					itfc->retVar[3].GetMatrix2D()[0][1] = (y0+height)/totheight*(cur2DPlot->curYAxis()->length()) + cur2DPlot->curYAxis()->base();
            }
         }
			else if(cur2DPlot->cmat())
         {
            if(mode == "all")
            {
               xRange[0] = cur2DPlot->curXAxis()->base();
					xRange[1] = cur2DPlot->curXAxis()->base()+cur2DPlot->curXAxis()->length();
               yRange[0] = cur2DPlot->curYAxis()->base();
					yRange[1] = cur2DPlot->curYAxis()->base()+cur2DPlot->curYAxis()->length();

			      itfc->retVar[1].MakeAndLoadCMatrix2D(cur2DPlot->cmat(), cur2DPlot->matWidth(), cur2DPlot->matHeight());
			      itfc->retVar[2].MakeMatrix2DFromVector(xRange,2,1);
			      itfc->retVar[3].MakeMatrix2DFromVector(yRange,2,1);
            }
            else
            {
               long x0 = cur2DPlot->visibleLeft();
               long y0 = cur2DPlot->visibleTop();
               long width = cur2DPlot->visibleWidth();
               long height = cur2DPlot->visibleHeight();
               float totwidth = (float)cur2DPlot->matWidth();
               float totheight = (float)cur2DPlot->matHeight();
			      itfc->retVar[1].MakeAndLoadCMatrix2D(NULL, width, height);
               complex** mIn = cur2DPlot->cmat();
               complex** mOut = itfc->retVar[1].GetCMatrix2D();
               for(int y = 0; y < height; y++)
                  for(int x = 0; x < width; x++)
                     mOut[y][x] = mIn[y+y0][x+x0];
			      itfc->retVar[2].MakeMatrix2DFromVector(NULL,2,1);
			      itfc->retVar[3].MakeMatrix2DFromVector(NULL,2,1);
               itfc->retVar[2].GetMatrix2D()[0][0] = x0/totwidth*(cur2DPlot->curXAxis()->length()) + cur2DPlot->curXAxis()->base();
					itfc->retVar[2].GetMatrix2D()[0][1] = (x0+width)/totwidth*(cur2DPlot->curXAxis()->length()) + cur2DPlot->curXAxis()->base();
					itfc->retVar[3].GetMatrix2D()[0][0] = y0/totheight*(cur2DPlot->curYAxis()->length()) + cur2DPlot->curYAxis()->base();
               itfc->retVar[3].GetMatrix2D()[0][1] = (y0+height)/totheight*(cur2DPlot->curYAxis()->length()) + cur2DPlot->curYAxis()->base();
            }
         }
         else if(cur2DPlot->matRGB())
         {
            if(mode == "all")
            {
               xRange[0] = cur2DPlot->curXAxis()->base();
               xRange[1] = cur2DPlot->curXAxis()->base()+cur2DPlot->curXAxis()->length();
               yRange[0] = cur2DPlot->curYAxis()->base();
					yRange[1] = cur2DPlot->curYAxis()->base()+cur2DPlot->curYAxis()->length();

			      itfc->retVar[1].MakeAndLoadMatrix3D(cur2DPlot->matRGB(), cur2DPlot->matWidth(), cur2DPlot->matHeight(),3);
			      itfc->retVar[2].MakeMatrix2DFromVector(xRange,2,1);
			      itfc->retVar[3].MakeMatrix2DFromVector(yRange,2,1);
            }
         }
	      else
	      {
			   itfc->retVar[1].MakeNullVar();
		      itfc->retVar[2].MakeNullVar();
		      itfc->retVar[3].MakeNullVar();
	      }	
         delete [] xRange;
         delete [] yRange;
		}  
	   else
	   {
		   itfc->retVar[1].MakeNullVar();
	      itfc->retVar[2].MakeNullVar();
	      itfc->retVar[3].MakeNullVar();
	   }	
      itfc->nrRetValues = 3;
      itfc->defaultVar = 1;
   }
   else
   {
      ErrorMessage("Invalid plot source");
      return(ERR);
   }
   return(OK);
}


/*****************************************************************
 Set up a multiple plot for 1D and 2D

 Syntax: multiplot("1d/2d",cols,rows)
     or: (cols,rows) = multiplot("1d/2d","getargs")
*****************************************************************/

int Multiplot(Interface* itfc ,char args[])
{
   short nrArgs;
   CText whichPlot,cmd;
   short nrX,nrY;
	CArg carg;

// Determine default values
	if(Plot::curPlot() && Plot1D::curPlot() && Plot2D::curPlot())
   {
		if(Plot::curPlot() == Plot1D::curPlot())
      {
         whichPlot = "1d";
			nrX = Plot1D::curPlot()->plotParent->getCols();
			nrY = Plot1D::curPlot()->plotParent->getRows();
      }
      else
      {
         whichPlot = "2d";
			nrX = Plot2D::curPlot()->plotParent->getCols();
         nrY = Plot2D::curPlot()->plotParent->getRows();
      }
   }

   nrArgs = carg.Count(args);

   if(nrArgs == 2)
   {
		int n;
      if((n = ArgScan(itfc,args,2,"1d/2d, getargs","ee","tt",&whichPlot,&cmd)) < 0)
         return(n);

      if(cmd == "getargs")
      {
         if(whichPlot == "1d")
         {
				if(!Plot1D::curPlot())
            {
               ErrorMessage("1D plot not defined");
               return(ERR);
            }
				itfc->retVar[1].MakeAndSetFloat(Plot1D::curPlot()->plotParent->getCols());
            itfc->retVar[2].MakeAndSetFloat(Plot1D::curPlot()->plotParent->getRows());
            itfc->nrRetValues = 2;
            return(OK);
         }
         else if(whichPlot == "2d")
         {
				if(!Plot2D::curPlot())
            {
               ErrorMessage("2D plot not defined");
               return(ERR);
            }
            itfc->retVar[1].MakeAndSetFloat(Plot2D::curPlot()->plotParent->getCols());
            itfc->retVar[2].MakeAndSetFloat(Plot2D::curPlot()->plotParent->getRows());
            itfc->nrRetValues = 2;
            return(OK);
         }
      }
      else
      {
         ErrorMessage("invalid command");
         return(ERR);
      }
   }

// Get plot to extract data from  *************
   if((nrArgs = ArgScan(itfc,args,3,"1d/2d, nrX, nrY","eee","tdd",&whichPlot,&nrX,&nrY)) < 0)
     return(nrArgs);  

   if(whichPlot == "1d")
   {
      if(!Plot1D::curPlot())
      {
         ErrorMessage("1D plot not defined");
         return(ERR);
      }
      Plot1D::curPlot()->plotParent->MakeMultiPlot(nrX,nrY);   
      Plot1D::curPlot()->DisplayAll(false); 
	}
	else if(whichPlot == "2d")
   {
      if(!Plot2D::curPlot())
      {
         ErrorMessage("2D plot not defined");
         return(ERR);
      }
	   Plot2D::curPlot()->plotParent->MakeMultiPlot(nrX,nrY);        
	   Plot2D::curPlot()->DisplayAll(false); 
	}
	else
	{
	   ErrorMessage("invalid plot name");
	   return(ERR);
	}
   itfc->nrRetValues = 0;
	return(OK);
}

	
/*****************************************************************
  Set or get the current 1D or 2D plot

  Syntax: region = curplot()  # gets current plot region (either 1D or 2D)
          region = curplot("1d/2d") # gets current 1D or 2D plot region
          curplot(region) # set plot by region
          region = curplot("1d/2d",x,y) # set plot by region numbers

*****************************************************************/
   
int SetOrGetCurrentPlot(Interface *itfc, char args[])
{
   short nrArgs;
   CText whichPlot,cmd;
   short xPos,yPos;
   CText title;
	CArg carg;
       
   nrArgs = carg.Count(args);

// Return region class
   if(nrArgs == 0) 
   {
      if(Plot::curPlot())
      {
         itfc->retVar[1].MakeClass(PLOT_CLASS,(void*)Plot::curPlot());
         itfc->nrRetValues = 1;
         return(OK);
      }
      else
      {
         ErrorMessage("no current plot defined");
         return(ERR);
      }
   }

// Set current plot
   else if(nrArgs == 1)
   {
      Variable region;
  
      if((nrArgs = ArgScan(itfc,args,1,"region","e","v",&region)) < 0)
         return(nrArgs);

      if(region.GetType() == CLASS)
      {
         ClassData *cd = (ClassData*)region.GetData();

         if(CheckClassValidity(cd,true) == ERR)
            return(ERR);

         if(cd->type == PLOT_CLASS)
         {
            Plot *pd = (Plot*)cd->data;
				if(pd && ((pd->getDimension() == 1) || (pd->getDimension() == 2)))
            {
               MyInvalidateRect(Plot1D::curPlot()->win, NULL, false); // Remove old indent
					pd->makeCurrentDimensionalPlot();
					pd->makeCurrentPlot();
               pd->DisplayAll(false);
				}
            else
            {
               ErrorMessage("invalid plot dimension");
               return(ERR);
            }
         }
         else
         {
            ErrorMessage("argument is not a plot region");
            return(ERR);
         }
      }
      else if(region.GetType() == UNQUOTED_STRING)
      {
         char *reg = region.GetString();

         if(!Plot::curPlot())
         {
				itfc->retVar[1].MakeNullVar();
				itfc->nrRetValues = 1;
				return(OK);
         }
         if(!stricmp(reg,"1d"))
         {
            if(Plot1D::curPlot())
				   Plot1D::curPlot()->makeCurrentPlot();
            else
            {
					itfc->retVar[1].MakeNullVar();
					itfc->nrRetValues = 1;
					return(OK);
            }
         }
         else if(!stricmp(reg,"2d"))
         {
            if(Plot2D::curPlot())
				   Plot2D::curPlot()->makeCurrentPlot();
            else
            {
					itfc->retVar[1].MakeNullVar();
					itfc->nrRetValues = 1;
					return(OK);
            }
         }
         else if(!stricmp(reg,"3d"))
         {
				WinData *win = rootWin->FindWinByHWND(cur3DWin);

				HWND temp = GetParent(cur3DWin);
				win = rootWin->FindWinByHWND(temp);
		
            if(win)
				{
					ObjectData *obj = win->widgets.findByWin(cur3DWin);
					itfc->retVar[1].MakeClass(OBJECT_CLASS,obj);
					itfc->nrRetValues = 1;
					return(OK);
				}
            else
            {
					itfc->retVar[1].MakeNullVar();
					itfc->nrRetValues = 1;
					return(OK);
            }
         }

         if(!Plot::curPlot()) 
         {
				itfc->retVar[1].MakeNullVar();
				itfc->nrRetValues = 1;
				return(OK);
         }

         itfc->retVar[1].MakeClass(PLOT_CLASS,(void*)Plot::curPlot());
         itfc->nrRetValues = 1;
         return(OK);
      }
      else
      {
         ErrorMessage("invalid data type for region");
         return(ERR);
      }
      return(OK);
   }
 
   else if(nrArgs == 3)
   {

   // Extract parameters  *************        
      if((nrArgs = ArgScan(itfc,args,1,"1d/2d, [xPos, yPos]","eee","tdd",&whichPlot,&xPos,&yPos)) < 0)
         return(nrArgs);  

      if(whichPlot == "1d")
      {
         if(Plot1D::curPlot())
         {
            PlotWindow *pp = Plot1D::curPlot()->plotParent;
            short index = pp->getCols()*(yPos-1) + xPos-1;
            if(index >= pp->getCols()*pp->getRows() || index < 0)
            {
               ErrorMessage("invalid 1D plot reference");
               return(ERR);
            }
				pp->makeCurrentPlot(index);
				pp->makeCurrentDimensionalPlot(index);

				if(itfc->inCLI)
					MyInvalidateRect(pp->hWnd,NULL,false); 

				if(!Plot1D::curPlot()->curTrace())
					Plot1D::curPlot()->setCurTrace();

            if(Plot1D::curPlot()->curTrace()) // Update title if new region 
            {
               title.Format("Plot-%hd",index+1); //,Plot1D::curPlot()->title);
               SetWindowText(Plot1D::curPlot()->win,title.Str()); 
            }
     
				if(itfc->inCLI)
					Plot1D::curPlot()->DisplayAll(false);  // Update the plot info in the status bar

            itfc->retVar[1].MakeClass(PLOT_CLASS,(void*)Plot1D::curPlot());
            itfc->nrRetValues = 1;
            return(OK);
         }
         else
         {
            ErrorMessage("no current 1D plot defined");
            return(ERR);
         }
      }
      else if(whichPlot == "2d") 
      {
         if(Plot2D::curPlot())
         {
            PlotWindow *pp = Plot2D::curPlot()->plotParent;

            EnterCriticalSection(&cs2DPlot);
            if(!pp->isBusy())
	         {
		         pp->setBusy(true);
		         LeaveCriticalSection(&cs2DPlot);

				   short index = pp->getCols()*(yPos-1) + xPos-1;
               if(index >= pp->getCols()*pp->getRows() || index < 0)
               {
                  pp->setBusyWithCriticalSection(false);
                  ErrorMessage("invalid 2D plot reference");
                  return(ERR);
               }	

				   pp->makeCurrentPlot(index);
				   pp->makeCurrentDimensionalPlot(index);
               Plot2D::curPlot()->DisplayAll(false); 
               itfc->retVar[1].MakeClass(PLOT_CLASS,(void*)Plot2D::curPlot());
               itfc->nrRetValues = 1;
               pp->setBusyWithCriticalSection(false);
            }
            else
		         LeaveCriticalSection(&cs2DPlot);

            return(OK);
         }
         else
         {
            ErrorMessage("no current 2D plot defined");
            return(ERR);
         }
      }
      else
      {
         ErrorMessage("invalid plot name");
         return(ERR);
      }
   } 
   itfc->nrRetValues = 0;
	return(OK);
}

/********************************************************************************
   Check to see if an application is already running by checking a string against
	all the titles of windows currently open. Could be fooled if an explorer 
	window has the same name. Should really check the executable name using
	https://stackoverflow.com/questions/2397578/how-to-get-the-executable-name-of-a-window
********************************************************************************/

int IsApplicationOpen(Interface *itfc, char arg[])
{
	short nrArgs;
	CText appName = "Prospa";
	HWND curWin;
	BOOL CALLBACK enumWindowsProc(__in  HWND hWnd, __in  LPARAM lParam);

// Get the directory name to search for
   if((nrArgs = ArgScan(itfc,arg,1,"application name","e","t",&appName)) < 0)
      return(nrArgs);

	SetLastError(0);

// Is called for all windows returning 1 in lasterror if it is found
	EnumWindows(enumWindowsProc,(LPARAM)appName.Str());

   itfc->retVar[1].MakeAndSetFloat(GetLastError());
   itfc->nrRetValues = 1;
	return(OK);
}

BOOL CALLBACK enumWindowsProc(__in  HWND hWnd, __in  LPARAM lParam)
{
	char name[MAX_PATH];

	char *refName = (char*)lParam;
	int refSize = strlen(refName);

	int n = GetWindowText(hWnd, name, 100);
	//TextMessage("%s\n",name);
	if(n >= refSize)
	{
		if(!strncmp(name,refName,refSize))
		{
			 if(hWnd != prospaWin)
			 {
				 GetClassName(hWnd, name, MAX_PATH); 
           //  TextMessage("\nClass name: %s\n", name);
				 if(!strcmp(name,"MAIN_PROSPAWIN") || strstr(name,"Spinsolve.exe") || strstr(name,"Spinsolve-AllUsers.exe") || 
                strstr(name,"Spinsolve Test.exe") || strstr(name,"Spinsolve Test-AllUsers.exe") || strstr(name, "Spinsolve Release Candidate-AllUsers.exe")) // Ignore for Explorer windows
				 {
					// TextMessage("%s\n",name);
					 SetLastError(1);
					 return(false);
				 }
			 }
		}
	}
   SetLastError(0);
   return(true);
}

/********************************************************************************
   Check to see if the specified file exists. File name can be explicit or
   include a path. Wild cards (* and ?) can be included in path
********************************************************************************/

int DoesFileExist(Interface *itfc, char arg[])
{
	short nrArgs;
   WIN32_FIND_DATA findData;
   HANDLE h;
   CText refFile;


// Get the directory name to search for
   if((nrArgs = ArgScan(itfc,arg,1,"file name","e","t",&refFile)) < 0)
      return(nrArgs);

// Find the first file in this directory
   h = FindFirstFile(refFile.Str(),&findData);
   
   if(h == INVALID_HANDLE_VALUE)
   {
      itfc->nrRetValues = 1;
		itfc->retVar[1].MakeAndSetFloat(0.0);
      return(OK);
   }

// Search for files which match refFile
   do
   {
	   if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	   {
         FindClose(h);
         itfc->retVar[1].MakeAndSetFloat(1);
         itfc->nrRetValues = 1;
         return(OK);
	   }
   }
   while(FindNextFile(h,&findData));
   FindClose(h);

   itfc->retVar[1].MakeAndSetFloat(0);
   itfc->nrRetValues = 1;
	return(OK);  
}

/********************************************************************************
   Check to see if the specified directory exists. 
   Directory name can include Wild cards (* and ?).
********************************************************************************/

int DoesDirectoryExist(Interface *itfc, char arg[])
{
	short nrArgs;
   WIN32_FIND_DATA findData;
   HANDLE h;
   CText refDir;

// Get the directory name to search for
   if((nrArgs = ArgScan(itfc,arg,1,"directory name","e","t",&refDir)) < 0)
      return(nrArgs);

// Remove trailing '\'
   int sz = refDir.Size();
   if(refDir[sz-1] == '\\')
   {
      if(sz == 3 && refDir[1] == ':') // Treat drive references specially
         refDir.Append('*');
      else
         refDir = refDir.Start(sz-2); // Otherwise remove backslash
   }



// Find the first file in this directory
   h = FindFirstFile(refDir.Str(),&findData);
   
   if(h == INVALID_HANDLE_VALUE)
   {
      itfc->nrRetValues = 1;
		itfc->retVar[1].MakeAndSetFloat(0.0);
      return(OK);
   }

// Search for files which match refFile
   do
   {
	   if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	   {
         FindClose(h);
         itfc->retVar[1].MakeAndSetFloat(1);
         itfc->nrRetValues = 1;
         return(OK);
	   }
   }
   while(FindNextFile(h,&findData));
   FindClose(h);

   itfc->retVar[1].MakeAndSetFloat(0);
   itfc->nrRetValues = 1;
	return(OK);  
}

/********************************************************************************
    Remove a filenames extension and return base name to user
********************************************************************************/

int RemoveFileExtension(Interface* itfc ,char args[])
{
  short r;
  Variable var;
  CText name;
  char **list;

  if((r = ArgScan(itfc,args,1,"filename","e","v",&var)) < 0)
     return(r); 

  if(var.GetType() == UNQUOTED_STRING)
  {
     name = var.GetString();
     RemoveExtension(name.Str());
	 itfc->retVar[1].MakeAndSetString(name.Str()); 
  }
  else if(var.GetType() == LIST)
  {
     list = var.GetList();
     short sz = var.GetDimX();
     for(short k = 0; k < sz; k++)
     {
        name = list[k];
        RemoveExtension(name.Str());
        ReplaceStringInList(name.Str(), &list, sz, k);
     }
     itfc->retVar[1].MakeAndSetList(list,sz); 
  }
  itfc->nrRetValues = 1;
  return(0);
}

/********************************************************************************
    Extract the  filename's extension 
********************************************************************************/

int GetFileExtension(Interface *itfc, char arg[])
{
  short r;
  CText name;
  CText extension;

  if((r = ArgScan(itfc,arg,1,"filename","e","t",&name)) < 0)
     return(r); 

  extension.Assign(name.Str());
  GetExtension(name.Str(),extension.Str());

  itfc->retVar[1].MakeAndSetString(extension.Str()); 
  itfc->nrRetValues = 1;
  return(0);
}


// Takes a complex directory name and returns it in its simplest form
//TODO update to CTEXT
int SimplifyDirectory(Interface* itfc ,char args[])
{
	short nrArgs;
	char directory[MAX_PATH];
	char bakPath[MAX_PATH];


	// Extract parameters  *************        
	if((nrArgs = ArgScan(itfc,args,1,"directory","e","s",directory)) < 0)
		return(nrArgs);  

	// Save current folder
	GetCurrentDirectory(MAX_PATH,bakPath);

	// Set to new directory
	if(SetCurrentDirectory(directory) == 0)
	{
		SetCurrentDirectory(bakPath);
		ErrorMessage("invalid directory");
		return(ERR);
	}

	// Get if back in simplified form
	GetCurrentDirectory(MAX_PATH,directory);

	// Return to user
	itfc->retVar[1].MakeAndSetString(directory);
	itfc->nrRetValues = 1;

	// Restore original directory
	SetCurrentDirectory(bakPath);

	return(OK);
}
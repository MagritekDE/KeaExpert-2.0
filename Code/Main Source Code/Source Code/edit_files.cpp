#include "stdafx.h"
#include <commdlg.h>
#include "edit_files.h"
#include "cache.h"
#include "command.h"
#include "command_other.h"
#include "defines.h"
#include "defineWindows.h"
#include "edit_class.h"
#include "edit_utilities.h"
#include "evaluate.h"
#include "events_edit.h"
#include "files.h"
#include "globals.h"
#include "guiObjectClass.h"
#include "guiWindowClass.h"
#include "interface.h"
#include "main.h"
#include "message.h"
#include "process.h"
#include "scanstrings.h"
#include "variablesOther.h"
#include "WidgetRegistry.h"
#include "list_functions.h"
#include "string_utilities.h"
#include "memoryLeak.h"

#pragma warning (disable: 4996) // Ignore deprecated library functions



short GoToLineEnd(long &pos, char *text, long len);
bool SearchForProcedure(char *text, char *funcName);
int GetProcedureNames(Interface* itfc ,char args[]);
short GetArgList(long pos,char *text, char *argStr);
short GoToLineStart(long& pos, char* text, long len);
short FindProcedureComment(char* text, char* procName, long& lineNr, bool verbose);

TextList *procLoadList;
TextList *procRunList;

// Set up the text linked lists to hold to previously loaded and run macros
void SetupTextLists()
{
	procLoadList = new TextList();
	procRunList = new TextList();
}

/*********************************************************************
  Load a text file from the specified folder. If path is empty just
  read from current folder. Return the text to the calling function.
  If the first open attempt fails the function adds the extension to 
  the filename and tries again. The returned name will have the 
  extension added in this case.
  If the file is a macro (*.mac) this name will be added to the
  editor file list.

  Note that the text pointer should be freed using delete [] when
  no longer required.

  Also note that the current directory is not modified by this function.

  Arguments
   path ........ (in) path name
   name ........ (in/out) filename/filename with extension
   extension ... (in) filename extension

  Returned
   text ........ (out) file in text format [DELETE]

**********************************************************************/

char* LoadTextFileFromFolder(char* path, char *name, char* extension)
{
   FILE *fp;
   char fileName[MAX_PATH];   
   char oldPath[MAX_PATH];

// Win 98 fix
//	ToLowerCase(name);  
   strncpy_s(fileName,MAX_PATH,name,_TRUNCATE);
   
// Save the original path
	GetCurrentDirectory(MAX_PATH,oldPath);

// Get into the correct folder
   if(path[0] != '\0')
	   SetCurrentDirectory(path);

// Open the file
   if((fp = fopen(fileName,"rb")) == (FILE*)0) 
   {
      strcat(fileName,extension);
      if((fp = fopen(fileName,"rb")) == (FILE*)0) 
      {
         SetCurrentDirectory(oldPath);	
         return(NULL); // Shouldn't happen unless pathname is wrong
      }
      strncpy_s(name,MAX_PATH,fileName,_TRUNCATE); // Fix up return filename
	}

// Find file length and then load it into 'text' buffer  
   unsigned long length = GetFileLength(fp);

// Allocate memory for the text  
   char *text = new char[length+1];
   if(!text)
   {
      fclose(fp);
      SetCurrentDirectory(oldPath);	
      return(NULL);
   }

// Read in the text from thefile
	fread(text,1,length,fp);
   text[length] = '\0';
	fclose(fp);

// Restore original folder	
   SetCurrentDirectory(oldPath);	
   
	return(text);
}

/**************************************************************************

  Load macro procedure in "name:procname" and return it as a text pointer.

  If name is empty then it takes the procedure from the current texteditor.
  Otherwise it searches the path list defined in variable 'macroPathVar' 
  until it finds "name:procname" either without or with extension added to
  name.

  The file directory path is returned on exiting the routine.

  If the macro file or procedure is not found the routine returns NULL.

  Note that if the supplied path variable if not empty then this path
  (and subdirectories) will be searched before the current directory
  and user defined search path.

  Note that the text pointer should be freed using delete [] when
  no longer required.

  Also note that the current directory is not modified by this function.

  Arguments
   path ........ (in/out) path to start search from/path file found in
   name ........ (in/out) filename/filename with extension
   procname .... (in) procedure name
   extension ... (in) filename extension

  Returned
   text ........ (out) file in text format [DELETE]

***************************************************************************/

char* LoadTextFile(Interface *itfc, char* path, char *name, char *procName, char* extension)
{
   long length;
   char coreName[MAX_PATH];
   char fileName[MAX_PATH];
   CText bakPath;
   FILE *fp = 0;

//	ToLowerCase(name);  // Win 98 fix
   strncpy_s(fileName,MAX_PATH,name,_TRUNCATE); // Save filename

// See if the macro is being run from the text editor
   if(itfc && itfc->macro && name[0] == '\0' && procName[0] != '\0')
   {
      if(itfc->name == "current_text")
      {
         char *text;
         long length;

         if(!curEditor)
            return(NULL);
         
         length = SendMessage(curEditor->edWin,WM_GETTEXTLENGTH,0,0) + 1;
         text = new char[length];
         SendMessage(curEditor->edWin,WM_GETTEXT,(WPARAM)length,(LPARAM)(text));
         if(FindProcedure(text,procName) == ERR)
           return((char*)0);
         strncpy_s(name,MAX_PATH,"current_text",_TRUNCATE);
         return(text);
      }     
      strncpy_s(name,MAX_PATH,itfc->name.Str(),_TRUNCATE);
      strncpy_s(fileName,MAX_PATH,name,_TRUNCATE); // Save filename
   }

// Search the cache - if present load the full file (maybe should return procedure if possible?)
   //strcpy(coreName, fileName);
   //RemoveExtension(coreName);
   //Variable *cacheVar = GetProcedure(path, coreName, procName);
   //if (cacheVar)
   //{
   //   GetDirectory(bakPath);
   //   SetCurrentDirectory(path);
   //   fp = fopen(fileName, "rb");
   //   SetDirectory(bakPath);
   //}

   // Otherwise search for fileName using the defined user search path
   if (!fp)
   {
      if ((fp = FindFolder(itfc, path, fileName, extension)) == NULL)
         return(NULL);
   }


// Find file length      
   length = GetFileLength(fp);

// Load file into 'text' buffer   
   char *text = new char[length+1];
   if(!text)
   {
      fclose(fp);
      return(NULL);
   }
	fread(text,1,length,fp);
   text[length] = '\0';
	fclose(fp);

// If a procedure is required scan for it and replace text with procedure
   if(procName[0] != '\0')
   {
      if(FindProcedure(text,procName) == ERR)
         return(NULL);
   }

// Make sure correct filename and pathname is returned
   strcpy(name,fileName);
   strcat(path,"\\");

	return(text);
}

char* LoadTextFile(Interface *itfc, char* path, char *name, char *procName, char* extension, long &lineNr)
{
   long length;
   char fileName[MAX_PATH];
   FILE *fp;

//	ToLowerCase(name);  // Win 98 fix
   strncpy_s(fileName,MAX_PATH,name,_TRUNCATE); // Save filename

// See if the macro is being run from the text editor
   if(itfc && itfc->macro && name[0] == '\0' && procName[0] != '\0')
   {
      if(itfc->name == "current_text")
      {
         char *text;
         long length;
         
         if(!curEditor)
            return(NULL);

         length = SendMessage(curEditor->edWin,WM_GETTEXTLENGTH,0,0) + 1;
         text = new char[length];
         SendMessage(curEditor->edWin,WM_GETTEXT,(WPARAM)length,(LPARAM)(text));
         if(FindProcedure(text,procName) == ERR)
           return((char*)0);
         strncpy_s(name,MAX_PATH,"current_text",_TRUNCATE);
         return(text);
      }     
      strncpy_s(name,MAX_PATH,itfc->name.Str(),_TRUNCATE);
      strncpy_s(fileName,MAX_PATH,name,_TRUNCATE); // Save filename
   }
 
// Search for fileName using the defined user search path
   if((fp = FindFolder(itfc,path,fileName,extension)) == NULL)
      return(NULL);

// Find file length      
   length = GetFileLength(fp);

// Load file into 'text' buffer   
   char *text = new char[length+1];
   if(!text)
   {
      fclose(fp);
      return(NULL);
   }
	fread(text,1,length,fp);
   text[length] = '\0';
	fclose(fp);

// If a procedure is required scan for it and replace text with procedure
   if(procName[0] != '\0')
   {
      if(FindProcedure(text,procName,lineNr) == ERR)
         return(NULL);
   }
   else
   {
      lineNr = 0;
   }

// Make sure correct filename and pathname is returned
   strncpy_s(name,MAX_PATH,fileName,_TRUNCATE);
   strcat(path,"\\");

	return(text);
}

/*****************************************************************************
       Does the procedure specified by the arguments exist? 
******************************************************************************/

bool IsProcedure(char* path, char *name, char *procName)
{
   long length;
   CText bak;
   FILE *fp;
   char fullNameMac[MAX_PATH];

// Assume its a macro with extension .mac
   strcpy(fullNameMac,name);
   AddExtension(fullNameMac,".mac");

   GetCurrentDirectory(bak);
   SetCurrentDirectory(path);

// Find file length 
   if(!(fp = fopen(fullNameMac,"rb")))
       return(false);

   length = GetFileLength(fp);

// Load file into 'text' buffer   
   char *text = new char[length+1];
   if(!text)
   {
      fclose(fp);
      return(false);
   }
	fread(text,1,length,fp);
   text[length] = '\0';
	fclose(fp);

// If a procedure is required scan for it and replace text with procedure
   if(procName[0] != '\0')
   {
      if(!SearchForProcedure(text,procName) )
      {
         delete [] text;
         SetCurrentDirectory(bak);
         return(false);
      }
   }


// Make sure correct filename and pathname is returned
   delete [] text;

   SetCurrentDirectory(bak);
	return(true);
}

/*****************************************************************************
       Search for a procedure
******************************************************************************/

bool SearchForProcedure(char *text, char *funcName)
{
   char command[MAX_STR];
   long len = strlen(text);
   long start,end,i;
   long lineNr = 0;
   
   i = 0;
   for(;;)
   {
      GetNextWord(i,text,command,MAX_STR-1);
      if(i >= len)
      {
         return(false);
      }
      if(!strcmp(command,"procedure"))
      {
         start = i-strlen("procedure");

         GetNextWord(i,text,command,MAX_STR-1);
         if(!strcmp(command,funcName))
         {
            break;
         }
	      if(i == len)
	      {
	         return(false);
	      }         
      }
      GoToLineEnd(i,text,len);
      lineNr++;
   }

   GoToLineEnd(i,text,len);
      
   for(;;)
   {
      GetNextWord(i,text,command,MAX_STR-1);
      if(!strcmp(command,"endproc"))
      {
         GoToLineEnd(i,text,len);
         end = i;
         break;
      }
      if(i >= len)
      {
         return(false);
      } 
      GoToLineEnd(i,text,len);
   }      
      
   for(i = start; i < end; i++)
   {
      text[i-start] = text[i];
   }
   text[i-start] = '\0';
   
   return(true);
}


/*********************************************************************
  Searches for filename using the following search path:

  1. Search the current directory.

  2. Then if curPath is not empty then search this path and then its
     subdirectories.

  3. Finally search the user supplied search paths.

  In all cases the filename is checked with and without extension.

  If the macro file or procedure is not found the routine returns NULL.

  Note that the file pointer should be closed when no longer required.

  Also note that the current directory is not modified by this function.

  Arguments
   curPath ........ (in/out) path to search first/path file found in.
   fileName ....... (in) file-name to search for.
   extension ...... (in) filename extension in case one not supplied.

  Returned
   fp ............. (out) file pointer of opened file [FCLOSE]

   Revisions:

   1. 7/2/07 changed search order to place current directory first.
***************************************************************************/


FILE* FindFolder(Interface *itfc, char *curPath, char *fileName, char *extension)
{
   FILE *fp;
   CText fileNameBk;
   CText filePath;
   CText bakPath;
   long len;
        
   fileNameBk.Assign(fileName);

// Save the current directory
   GetDirectory(bakPath);
 
// Loop over entries in the searchpath variable

   for(int i = -2; i < VarWidth(macroPathVar); i++)
   {
      if(i == -2) // First check current directory
      {
         filePath.Assign(".");
      }
      else if(i == -1) // Next check parent macro folder
      {
         if(curPath[0] == '\0') continue;
         filePath.Assign(curPath);
      }
      //else if(i == -1) // Next check parent macro subfolders
      //{
      //   if(curPath[0] == '\0') continue;
      //   filePath.Assign(curPath);
      //   filePath.Concat("*");
      //}
      else // Then check user defined list
      {
         filePath.Assign(VarList(macroPathVar)[i]);
 		   ReplaceVarInString(itfc,filePath);
      }
      len = filePath.Size();
 		      
      if(i >= 0 && filePath == ".") // Ignore extra current folder searches
         continue;

      if(filePath.Str()[len-1] == '*') // Do a recursive search through all subfolders
      {
         filePath.Str()[len-1] = '\0';

	      if(filePath[0] != '\0' && !SetDirectory(filePath))
	      {
	         SetDirectory(bakPath);	
            ErrorMessage("invalid directory '%s' in search path", filePath);
            return(NULL);
	      }

	      if(RecursiveSearch(fileName,extension,filePath) == FOUND)
	      {
		      SetDirectory(filePath);
		      if((fp = fopen(fileName,"rb")) != (FILE*)0)
		      {
               GetCurrentDirectory(MAX_PATH,curPath);
	            SetDirectory(bakPath);	
		         return(fp);
		      }
	      }
	   }
	   else // Just look in the current folder
	   {   
	      if(!SetDirectory(filePath))
	      {
            GetCurrentDirectory(MAX_PATH,curPath);
	         SetDirectory(bakPath);
            ErrorMessage("invalid directory '%s' in search path", filePath);
            return(NULL);
	      }
	      	   
		   if((fp = fopen(fileName,"rb")) != (FILE*)0) // Try raw filename first
		   {
            GetCurrentDirectory(MAX_PATH,curPath);
	         SetDirectory(bakPath);	
		      return(fp);
		   }
		   else
		   {		   
		      strcat(fileName,extension); // Add extension
		      if((fp = fopen(fileName,"rb")) != (FILE*)0) // Try with extension
            {
               GetCurrentDirectory(MAX_PATH,curPath);
	            SetDirectory(bakPath);	
		         return(fp);		
            }
		      strncpy_s(fileName,MAX_PATH,fileNameBk.Str(),_TRUNCATE);  // Restore if extension didn't work    	      	   
		   }
		}
		SetDirectory(bakPath);	         
	}

	
   SetDirectory(bakPath);	
	return(NULL); // Can't find it so give up	
}


/***********************************************************************
   Scans through the specified macro file and returns a list of
   procedure names and the argument lists for those procedures

   Input: macro_directory, macro_name
   Output: procedure_list, argument_list
***********************************************************************/

int GetProcedureNames(Interface* itfc ,char args[])
{
   char macroName[MAX_STR];
   CText macroPath;
   short nrArgs;
   
// Select the DLL to ignore
   if((nrArgs = ArgScan(itfc,args,2,"Macro to check","ee","ts",&macroPath,macroName)) < 0)
      return(nrArgs);

// Load the macro into a text file
   char *text = LoadTextFileFromFolder(macroPath.Str(), macroName,".mac");
   if(!text)
   {
      ErrorMessage("Macro not found");
      return(ERR);
   }

   long pos = 0;
   char *procName; 
   char *argStr; 
   char command[MAX_STR]; 
   char **procList = NULL;
   char **argList = NULL;
   long nrProcs = 0;
   long len = strlen(text);

   procName = new char[len+1];
   argStr = new char[len+1];

// Search each line in current text editor for procedure names     
   while(GetNextWord(pos, text, command, MAX_STR) != ERR)
   {      
      if(pos >= len)
         break;
      if(!strcmp(command,"procedure"))
      {
         GetNextWord(pos, text, procName, MAX_STR);
	      AppendStringToList(procName,&procList,nrProcs);
         GetArgList(pos,text,argStr);
	      AppendStringToList(argStr,&argList,nrProcs);
         nrProcs++;      
      }  
      GoToLineEnd(pos,text,len);
   }

// Return result
   if(nrProcs == 0)
   {
      itfc->retVar[1].MakeNullVar();
      itfc->retVar[2].MakeNullVar();
      itfc->nrRetValues = 2;
   }
   else
   {
      itfc->retVar[1].AssignList(procList,nrProcs);
      itfc->retVar[2].AssignList(argList,nrProcs);
      itfc->nrRetValues = 2;
   }

   delete [] text;
   delete [] procName;
   delete [] argStr;

   return(OK);
}

/*********************************************************************
  See if a procedure is cached or otherwise exists in the filesystem.
  The correct macroPath, file name (with or without extension) and 
  procedure name must be passed.
*********************************************************************/

int IsProcedureAvailable(Interface* itfc ,char args[])
{
   char macroName[MAX_PATH];
   char procName[MAX_PATH];
	char macroPath[MAX_PATH];
   short nrArgs;
	Variable *procVar = 0;
	int found = 0;
   
// Get the path, macro and procedure name
   if((nrArgs = ArgScan(itfc,args,3,"path, macro, procedure","eee","sss",macroPath,macroName,procName)) < 0)
      return(nrArgs);

// Search for the procedure

// See if it is cached
   if(gCachedProc.next)
	{
       if(procVar = GetProcedure(macroPath,macroName,procName))		 
		    found = 1;
	}
   if(!procVar && itfc->win && itfc->win->cacheProc)
	{
       if(procVar = itfc->win->GetProcedure(macroPath,macroName,procName))		 
		    found = 1;
	}

	if(!procVar) // No, it has not been cached, so check the file path
	{
		if(IsProcedure(macroPath,macroName,procName))
		    found = 1;
	}

	itfc->retVar[1].MakeAndSetFloat(found);
   itfc->nrRetValues = 1;

   return(OK);
}

/*********************************************************************
  Find the first instance of the passed macro name in the Prospa
  search path. Return the path and the full name
*********************************************************************/

int FindMacro(Interface *itfc, char args[])
{
   char macroName[MAX_PATH];
   char procName[MAX_PATH];
	char macroPath[MAX_PATH];
   short nrArgs;
	FILE *fp;
   
// Get macro name
   if((nrArgs = ArgScan(itfc,args,1,"macro","e","s",macroName)) < 0)
      return(nrArgs);

	GetCurrentDirectory(MAX_PATH,macroPath);

	if((fp = FindFolder(itfc,macroPath,macroName,".mac")) == NULL)
   {
		if((fp = FindFolder(itfc,macroPath,macroName,".pex")) == NULL)
		{
			itfc->retVar[1].MakeNullVar();
			itfc->retVar[2].MakeNullVar();
			itfc->nrRetValues = 2;
			return(OK);
		}
	}

	fclose(fp);

	itfc->retVar[1].MakeAndSetString(macroPath);
	itfc->retVar[2].MakeAndSetString(macroName);
	itfc->nrRetValues = 2;

	return(OK);

}


/*********************************************************************
  Copy argument list from inStr to outStr. Arguments span indicies
  start to end in inStr. Will ignore comments and \r \n characters.
*********************************************************************/

void CopyArguments(char *outStr, char* inStr, int start, int end)
{
   int j = 0;
   bool inComment = false;
   bool inString = false;

   for(int i = start; i < start+end; i++)
   {
      if(!inComment && inStr[i] == '#')
      {
         inComment = true;
         continue;
      }

      if(inComment)
      {
         if(inStr[i] != '\n')
            continue;
         inComment = false;
         continue;
      }

      if(!inString && inStr[i] == '"')
      {
         inString = true;
         outStr[j++] = inStr[i];
         continue;
      }
      else if(inString && inStr[i] == '"')
         inString = false;

      if(!inString && inStr[i] == ' ')
         continue;

      if(!inString && inStr[i] == ',')
      {
         outStr[j++] = ',';
         outStr[j++] = ' ';
         continue;
      }
      if(inStr[i] == '\r' || inStr[i] == '\n')
         continue;

      outStr[j++] = inStr[i];
   }
   outStr[j] = '\0';
}

/*********************************************************************

*********************************************************************/

short GetArgList(long pos, char *text, char *argStr)
{
   int start = -1;
   int sz = strlen(text);
   for(size_t i = pos; i < sz; i++)
   {
      if(start == -1)
      {
         if(text[i] == ')')
         {
            start = i;
            CopyArguments(argStr,text,start,i-start);
            return(OK);
         }            
         if(text[i] != ' ' && text[i] != ',')
         {
            start = i;
            continue;
         }
         continue;
      }

      if(text[i] == ')')
      {
         CopyArguments(argStr,text,start,i-start);
         return(OK);
      }
   }
   argStr[0] = '\0';
   return(ERR);
}


/*********************************************************************

 Search text for the procedure funcName. Replace text with this
 procedure. Returns OK if found ERR if not found

  Arguments
   text ........ (in/out) text to search/procedure text.
   funcName .... (in) procedure to search for.

  Returned
   OK/ERR

*********************************************************************/


short FindProcedure(char *text, char *funcName)
{
   char command[MAX_STR];
   long len = strlen(text);
   long start,end,i;
   long lineNr = 0;
   
   i = 0;
   for(;;)
   {
      GetNextWord(i,text,command,MAX_STR-1);
      if(i >= len)
      {
         ErrorMessage("Procedure '%s' not found",funcName);
         return(ERR);
      }
      if(!strcmp(command,"procedure"))
      {
         start = i-strlen("procedure");

         GetNextWord(i,text,command,MAX_STR-1);
         if(!strcmp(command,funcName))
         {
            break;
         }
	      if(i == len)
	      {
            ErrorMessage("Procedure '%s' not found",funcName);
	         return(ERR);
	      }         
      }
      GoToLineEnd(i,text,len);
      lineNr++;
   }

   GoToLineEnd(i,text,len);
      
   for(;;)
   {
      GetNextWord(i,text,command,MAX_STR-1);
      if(!strcmp(command,"endproc"))
      {
         GoToLineEnd(i,text,len);
         end = i;
         break;
      }
      if(i >= len)
      {
         ErrorMessage("Missing 'endproc' command");
         return(ERR);
      } 
      GoToLineEnd(i,text,len);
   }      
      
   for(i = start; i < end; i++)
   {
      text[i-start] = text[i];
   }
   text[i-start] = '\0';
   
   return(OK);
}

short FindProcedure(char *text, char *funcName, long &lineNr, bool verbose)
{
   char command[MAX_STR];
   long len = strlen(text);
   long start,end,i;
   lineNr = 0;
   
   i = 0;
   for(;;)
   {
      GetNextWord(i,text,command,MAX_STR-1);
      if(i >= len)
      {
         if(verbose)
            ErrorMessage("Procedure '%s' not found",funcName);
         return(ERR);
      }
      if(!strcmp(command,"procedure"))
      {
         start = i-strlen("procedure");

         GetNextWord(i,text,command,MAX_STR-1);
         if(!StrCmpIgnoreCase(command,funcName))
         {
            break;
         }
	      if(i == len)
	      {
            if(verbose)
               ErrorMessage("Procedure '%s' not found",funcName);
	         return(ERR);
	      }         
      }
      GoToLineEnd(i,text,len);
      lineNr++;
   }

   GoToLineEnd(i,text,len);
      
   for(;;)
   {
      GetNextWord(i,text,command,MAX_STR-1);
      if(!strcmp(command,"endproc"))
      {
         GoToLineEnd(i,text,len);
         end = i;
         break;
      }
      if(i >= len)
      {
         if(verbose)
            ErrorMessage("Missing 'endproc' command");
         return(ERR);
      } 
      GoToLineEnd(i,text,len);
   }      
      
   for(i = start; i < end; i++)
   {
      text[i-start] = text[i];
   }
   text[i-start] = '\0';
   
   return(OK);
}

// Return the comment for procedure name
short FindProcedureComment(char* text, char* procName, long& lineNr, bool verbose)
{
   char command[MAX_STR];
   long len = strlen(text);
   long start, end, i, j;
   lineNr = 0;

   i = 0;
   for (;;)
   {
      GetNextWord(i, text, command, MAX_STR - 1);
      if (i >= len)
      {
         if (verbose)
            ErrorMessage("Procedure '%s' not found", procName);
         return(ERR);
      }
      if (!strcmp(command, "procedure"))
      {
         start = i - strlen("procedure");

         GetNextWord(i, text, command, MAX_STR - 1);
         if (!StrCmpIgnoreCase(command, procName))
         {
            break;
         }
         if (i == len)
         {
            if (verbose)
               ErrorMessage("Procedure '%s' not found", procName);
            return(ERR);
         }
      }
      GoToLineEnd(i, text, len);
      lineNr++;
   }

   GoToLineStart(i, text, len);
   i-=2;
   if(i >= 0)
      GoToLineStart(i, text, len);

   while (i > 0 && text[i] == ' ' || text[i] == '\r')
   {
      i-=2;
      GoToLineStart(i, text, len);
   }

   j = i;
   GoToLineEnd(j, text, len);
   j -= 2;
   end = j;

   while (i > 0 && text[i] == '#')
   {
      i-=2;
      GoToLineStart(i, text, len);
   }

   start = i;

   for (i = start; i < end; i++)
   {
      text[i - start] = text[i];
   }
   text[i - start] = '\0';

   return(OK);
}

/*********************************************************************************

 Search 'text' for a procedure starting from index i. Return the procedure
 text in 'procedure'. The calling program can call this function multiple times
 until all procedures have been extracted from the text string. Note that 
 the 'procedure' string should be deleted when no longer required.

  Arguments
   text ......... (in/out) text to search/procedure text.
   i ............ (in/out) starting point in text.
   procedure .... (out) procedure found. [DELETE]
   procName ..... (out) name of procedure
   len .......... (in) text length
   lineNr ....... returns last line number in text of procedure
   startLine .... returns start line number in text of procedure
   includeComment ... includes any procedure comment in the returned string

  Returned
     0 .......... finished searching
     1 .......... procedure found
 
**********************************************************************************/


short FindProcedures(char *text, long &i, char **procedure, char *procName, long len, long &lineNr, long &startLine, bool includeComment)
{
   char command[MAX_STR];

   long start=-1,end=-1,startComment=-1;

   // If comment is to be included then start from just after the last endproc or the beginning of the file
   if (includeComment)
   {
       startComment = i;
   }

       // Search for the next procedure
   for (;;)
   {
      GetNextWord(i, text, command, MAX_STR - 1);
      if (i >= len)
      {
         return(0);
      }
      if (!strcmp(command, "procedure"))
      {
         start = i - strlen("procedure");
         GetNextWord(i, text, procName, MAX_STR - 1);
         break;
      }
      GoToLineEnd(i, text, len);
      lineNr++;
   }

   GoToLineEnd(i, text, len);
   lineNr++;

   startLine = lineNr;

// Search for the end of the procedure
   if (start >= 0)
   {
      for (;;)
      {
         GetNextWord(i, text, command, MAX_STR - 1);
         if (!strcmp(command, "endproc"))
         {
               GoToLineEnd(i, text, len);
               lineNr++;
               end = i;
               break;
         }
         if (i >= len)
         {
               ErrorMessage("Missing 'endproc' command");
               return(0);
         }
         GoToLineEnd(i, text, len);
         lineNr++;
      }
   }
   else
   {
      ErrorMessage("Missing 'endproc' command");
      return(0);
   }
      
// Return procedure text
   if (includeComment)
   {
      long size = end - startComment;
      *procedure = new char[size + 1];
      StrNCopy(*procedure, text + startComment, size);
   }
   else
   {
      long size = end - start;
      *procedure = new char[size + 1];
      StrNCopy(*procedure, text + start, size);
   }
  
   startLine--;
   return(1);
}


/*********************************************************************
  Return character position in 'text' where procedure 'funcName' begins 
  Return ERR if not found.
********************************************************************/

long FindProcedurePosition(char *text, char *funcName)
{
   char command[MAX_STR];
   long len = strlen(text);
   long i;
   
   i = 0;
   for(;;)
   {
      GetNextWord(i,text,command,MAX_STR-1);
      if(i >= len)
      {
         return(ERR);
      }
      if(!strcmp(command,"procedure"))
      {
         GetNextWord(i,text,command,MAX_STR-1);
         if(!strcmp(command,funcName))
         {
            return(i);
         }
	      if(i == len)
	      {
	         return(ERR);
	      }         
      }
      GoToLineEnd(i,text,len);
   }
   return(ERR);
}

/*********************************************************************
 Finds the end-of-line characters \r\n in 'text'.
 'pos' is the starting text index and len is the text len.
 On exit 'pos' points to the \n character.
 Returns ERR if \r\n sequence found.
********************************************************************/

short GoToLineEnd(long &pos, char *text, long len)
{ 
   while(text[pos] != '\r')
   {
      (pos)++;
      if(pos >= len) return(ERR);
   }
   
   pos++;
   if(text[pos] == '\n') 
   {
      (pos)++;
      if(pos >= len) return(ERR);
   }      
   
   return(OK);
}

// Search for the start of the previous line in text starting from pos

short GoToLineStart(long& pos, char* text, long len)
{
   while (text[pos] != '\n') // Search for end of previous line
   {
      (pos)--;
      if (pos <= 0) return(ERR);
   }

   pos++;


   return(OK);
}

short GetNextWord(long &pos, char *text, char *command, long maxLen)
{ 
   long len = strlen(text);
   long i;
      
// Ignore blanks
   while(text[pos] == ' ') pos++;

// Ignore bracket
   if(text[pos] == '(') pos++;

// Ignore blanks
   while(text[pos] == ' ') pos++;
      
// Search for end of next word ***********************************
   for(i = pos; i < len; i++)
   {
      if(i-pos >= maxLen)
      {
         command[i-pos] = '\0';
         pos = i;
         return(ERR);
      }   
         
      command[i-pos] = text[i];
      if((text[i] == ' ') || (text[i] == '\r') ||
         (text[i] == '(') || (text[i] == ';')  ||
         (text[i] == ')') || (text[i] == ','))
       break;
   }
   command[i-pos] = '\0';
   pos = i;
   return(OK);
}


short GetNextWord(long &pos, char *text, char *command, long maxLen, long &lineNr)
{ 
   long len = strlen(text);
   long i;
      
// Ignore blanks
   while(text[pos] == ' ') pos++;

// Ignore bracket
   if(text[pos] == '(') pos++;

// Ignore blanks
   while(text[pos] == ' ') pos++;
      
// Search for end of next word ***********************************
   for(i = pos; i < len; i++)
   {
      if(i-pos >= maxLen)
      {
         command[i-pos] = '\0';
         pos = i;
         return(ERR);
      } 

      if(text[i] == '\n')
         lineNr++;
       if(text[i] == '\r')
         lineNr++;        
      command[i-pos] = text[i];
      if((text[i] == ' ') || (text[i] == '\r') ||
         (text[i] == '(') || (text[i] == ';')  ||
         (text[i] == ')') || (text[i] == ','))
       break;
   }
   command[i-pos] = '\0';
   pos = i;
   return(OK);
}



/**********************************************************************
*  Save text to "path\filename" and return success (OK) or error (ERR)
***********************************************************************/

short SaveTextFile(char* path, char *file, char *text)
{
   FILE *fp;
   char fullFileName[MAX_PATH];
   
   sprintf(fullFileName,"%s\\%s",path,file);

   if((fp = fopen(fullFileName,"wb")) == (FILE*)0)
   {
      ErrorMessage("can't save file '%s'",fullFileName);
      return(ERR);
   }
   
   fwrite(text,1,strlen(text),fp);
   fclose(fp);
   return(OK);
}


/**********************************************************************
  Check to see if any edit sessions need saving and if they do ask the user
  to confirm save. After saving (or not) return to the caller the last 
  user response: IDNO/IDYES/IDCANCEL
***********************************************************************/

short SaveWinEditSessions(WinData *win)
{
   int response = IDNO;
   int edNr;
   EditRegion *curEdBack;
   EditParent *ep;

   if(!curEditor)
      return(response);
   
   curEdBack = curEditor;

	WidgetList* textEditors = win->widgets.getAllOfType(TEXTEDITOR);
	for(ObjectData* te: *textEditors)
	{
		if (!te->data)
			continue;
		
		ep = (EditParent*)te->data;
		for(edNr = 0; edNr < ep->rows*ep->cols; edNr++) 
		{
			response = ep->editData[edNr]->CheckForUnsavedEdits(edNr);

			if(response == IDCANCEL)
			{
				curEditor = curEdBack;	
				delete(textEditors);
				return(response);  
			}          
		}
	}

// Restore original current editor and return	
   curEditor = curEdBack;	
	delete(textEditors);
	return(response);
}

/**********************************************************************
  Check to see if any edit sessions need saving and if they do ask the user
  to confirm save. After saving (or not) return to the caller the last 
  user response: IDNO/IDYES/IDCANCEL
***********************************************************************/

short SaveAllEditSessions(void)
{
   int response = IDNO;
   EditRegion *curEdBack;
   WinData *nextWin;
      
   if(!curEditor)
      return(response);

   curEdBack = curEditor;

// Loop over all windows searching for edit regions  
   nextWin = rootWin->next;
   for(WinData *win = nextWin; win != NULL; win = win->next)
   {
      response = SaveWinEditSessions(win);
      if(response == IDCANCEL)
         break;
   }

// Restore original current editor and return	
   curEditor = curEdBack;	
	return(response);
}

// Save the listed of edited files to 'editHistory.lst'

void SaveEditList()
{
   FILE *fp;
   CText currentwd;

// Save current directory *****************
   GetDirectory(currentwd);

// Move to preferences folder *************
   if(!SetCurrentDirectory(userPrefDir))
   {
      SetDirectory(currentwd);
      MessageDialog(prospaWin,MB_ICONERROR,"Error","Unable to move into '%s'",userPrefDir); 
      return;
   }

// Open a file for write ******
   if(!(fp = fopen("editHistory.lst","w")))
   {
      SetDirectory(currentwd);

      MessageDialog(prospaWin,MB_ICONERROR,"Error","Can't save edit history list\rIs <user_home>/preferences/editHistory.mac write protected?");
      return;
   } 

   TextList* list = procLoadList;
   
   while(1)
   {
      list = list->GetNextText();
      if(!list) break;
      fprintf(fp,"%s\n",list->text);
   }

   fclose(fp);

// Restore original directory *************
   SetDirectory(currentwd);

}

// Load the listed of edited files from 'editHistory.lst'

void LoadEditList()
{
   FILE *fp;
   CText currentwd;
   CText fullName;
   char c;

// Save current directory *****************
   GetDirectory(currentwd);

// Move to preferences folder *************
   if(!SetCurrentDirectory(userPrefDir))
   {
      SetDirectory(currentwd);
      return;
   }

// Open a file for write ******
   if(!(fp = fopen("editHistory.lst","r")))
   {
      SetDirectory(currentwd);
      return;
   } 

   for(;;)
   {
      fullName.Reset();

      for(;;)
      {
         if((c = fgetc(fp)) == EOF)
         {
            fclose(fp);
            return;
         }

         if(c == '\n')
            break;

         fullName.Append(c);
      }
         
      if(fullName[0] != '\0')
      {
         if(IsFile(fullName.Str()))
            procLoadList->AddText(fullName.Str());
      }
   }
	if (fp)
	{
		fclose(fp);
	}

}



//
//void LoadEditList()
//{
//   FILE *fp;
//   CText currentwd;
//   char fullName[MAX_PATH];
//   char c;
//   long len;
//
//// Save current directory *****************
//   GetDirectory(currentwd);
//
//// Move to preferences folder *************
//   if(!SetCurrentDirectory(userPrefDir))
//   {
//      SetDirectory(currentwd);
//      return;
//   }
//
//// Open a file for write ******
//   if(!(fp = fopen("editHistory.lst","r")))
//   {
//      SetDirectory(currentwd);
//      return;
//   } 
//
//   for(;;)
//   {
//      if(!fgets(fullName,MAX_PATH,fp))
//         break;
//
//      len = strlen(fullName);
//
//      if(fullName[len-1] == '\n')
//         fullName[len-1] = '\0';
//         
//      if(fullName[0] != '\0')
//         procLoadList.AddText(fullName);
//   }
//
//   fclose(fp);
//
//}


/********************************************************************
* Return the current text selection to the user
*********************************************************************/

int GetTextSelection(Interface* itfc, char[])
{
   long startSel,endSel;
   char *text;
   
   if(!curEditor)
   {
      ErrorMessage("no current editor");
      return(ERR);
   }
       
	GetEditSelection(curEditor->edWin,startSel,endSel);  
   text = new char[endSel-startSel+1];	   
   SendMessage(curEditor->edWin,EM_GETSELTEXT,(WPARAM)0, (LPARAM)text);
   if(strlen(text) > 50)
   {
      ErrorMessage("text selection truncated to 50 characters");
      text[50] = '\0';
   }
      
   itfc->retVar[1].MakeAndSetString(text);
   itfc->nrRetValues = 1;
   delete [] text;
   
   return(OK);
}

/*********************************************************************************
* Returns the size of the file (in bytes) as pointed to by fp.                   *
*********************************************************************************/

long GetFileLength(FILE *fp)
{
	int currentPos, fileLength;

	currentPos = ftell(fp);
	fseek(fp, 0, SEEK_END);
	fileLength = ftell(fp);
	fseek(fp, currentPos, SEEK_SET);
	return(fileLength);
}



/*********************************************************************************
           Save full filename to linked list for use in pop-up menu
*********************************************************************************/
 
#define MAX_EDIT_HISTORY 10 // Maximum number of edit files in list

void AddFilenameToList(TextList* list, char* path, char* name)
{
   char fullName[MAX_PATH];
   strncpy_s(fullName,MAX_PATH,path,_TRUNCATE);
   if(path[strlen(fullName)-1] != '\\')
      strcat(fullName,"\\");
	strcat(fullName,name); 
	if(!list->FindIText(fullName)) 
   {
      long n = list->Count();
      if(n == MAX_EDIT_HISTORY)
         list->RemoveFirst(); // Remove entry on top of list
      list->AddText(fullName);
   }
}


/*********************************************************************************
           Linked list routines for text files
*********************************************************************************/
 
TextList::TextList()
{
   next = (TextList*)0;
   last = (TextList*)0;
   text = (char*)0;
}

//TextList::~TextList()
//{
//	RemoveAll();
//}

/*********************************************************************************
           Add a new item to the linked list
*********************************************************************************/
 
void TextList::AddText(char *str)
{
   TextList *list = new TextList;

   list->text = new char[strlen(str)+1];
   strncpy_s(list->text,strlen(str)+1,str,_TRUNCATE);
      
   if(next != (TextList*)0)
      next->last = list;
   list->next = next;
   list->last = this;
	next = list;
}


/*********************************************************************************
           Search for a specific item of text
*********************************************************************************/

TextList* TextList::FindText(char *str)
{
   for(TextList *list = next; list != NULL; list = list->next)
   { 
      if(!strcmp(list->text,str))
         return(list);
   }
   return(NULL);
}


/*********************************************************************************
          Search for a specific item of text (case independent)
*********************************************************************************/

TextList* TextList::FindIText(char *str)
{
   for(TextList *list = next; list != NULL; list = list->next)
   { 
      if(!stricmp(list->text,str))
         return(list);
   }
   return(NULL);
}


/*********************************************************************************
          Get the next item in the linked list
*********************************************************************************/

TextList* TextList::GetNextText()
{
   TextList *list = next;
   if(list != NULL)
   {
      return(list);
   }
   return(NULL);
}


/*********************************************************************************
          Get the nth item in the linked list
*********************************************************************************/

TextList* TextList::GetNthText(long n)
{
   long cnt = 0;
   for(TextList *list = next; list != NULL; list = list->next)
   { 
      if(cnt++ == n)
         return(list);
   }
   return(NULL);
}


/*********************************************************************************
          Count the number of entries in the text list
*********************************************************************************/

long  TextList::Count()
{
   long cnt = 0;
   for(TextList *list = next; list != NULL; list = list->next, cnt++);
   return(cnt);
}


/*********************************************************************************
          Remove an item from the linked list
*********************************************************************************/

void TextList::Remove()
{
   delete[] text;
   
   if(next == (TextList*)0) // Last in list
   {
   	last->next = (TextList*)0;
   }
   else
   {
  		last->next = next;
   	next->last = last;
   }
}

/*********************************************************************************
     Remove first item added to the linked list (which is at the end of the list)
*********************************************************************************/

void TextList::RemoveFirst()
{  
   for(TextList *list = this; list != NULL; list = list->next)
   {
      if(list->next == NULL)
      {
         delete [] list->text;
         list->last->next = NULL;
         delete list;
         return;
      }
   }
}


/*********************************************************************************
           Remove all items from list
*********************************************************************************/

void TextList::RemoveAll()
{
   TextList *list = next;
   while(list != (TextList*)0)
   {
      TextList *t = list;
      list = list->next;
      delete[] t->text;
      delete t; 
   }
   next = (TextList*)0;
} 


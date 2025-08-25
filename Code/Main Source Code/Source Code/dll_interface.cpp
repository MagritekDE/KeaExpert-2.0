#include "stdafx.h"
#include "dll.h"
#include "defineWindows.h"
#include "evaluate.h"
#include "globals.h"
#include "interface.h"
#include "main.h"
#include "message.h"
#include "scanstrings.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "control.h"
#include "files.h"
#include "edit_files.h"
#include "string_utilities.h"
#include "list_functions.h"
#include "cArg.h"
#include "memoryLeak.h"

#pragma warning (disable: 4996) // Ignore deprecated library functions

#ifdef _DEBUG 
   char DLLFILES[] = "*Debug.DLL";
#else
   char DLLFILES[] = "*Run.DLL";
#endif

//typedef short( *MYFUNC) (char*,char*,DLLParameters*);
//typedef void( *MYFUNC2) (void);
//typedef bool( *MYFUNC3) (char*,char*);
//
//typedef struct
//{
//	HINSTANCE Instance;          // Instance handle for this DLL
//	CText Name;                  // Name of the DLL
//	MYFUNC AddCommands;          // Addres of AddCommands function for this DLL
//	MYFUNC2 ListCommands;        // Address of ListCommands function for this DLL
//	MYFUNC3 GetCommandSyntax;    // Address of GetCommandSyntax for this DLL
//}
//DLLINFO;
//
DLLINFO *DLLInfo; // Array of currently loaded DLLs

short nrDLLs = 0; // Total number of DLLs loaded
short CountFilesWithIgnore(char name[], char ignoreFile[]);
short CountFilesWithIgnores(char name[], char **ignoreFiles, int nrFilesToIgnore);
int IgnoreDLLs(Interface* itfc, char args[]);


/**************************  User callable functions *************************************/


/*******************************************************************
 Load all DLLs currently present in the current DLL search path
 Store them in the DLLInfo array
*******************************************************************/

int LoadDLLs(Interface* itfc ,char args[])
{
   WIN32_FIND_DATA findData;
   char backUpPath[MAX_PATH];
   CText filePath;
   int i,j;
   bool fileFound;
   
   if(nrDLLs != 0)
   {
      UnloadDLLs(itfc,"");
   }
   nrDLLs = 0;

// Measure the length of the DLL filename suffix (e.g. Debug.dll)
   int suffixSz = strlen(DLLFILES);

// Save the current directory
   GetCurrentDirectory(MAX_PATH,backUpPath);
 
// Loop over entries in the dll search-path variable counting number of DLLs
   for(i = 0; i < VarWidth(dllPathVar); i++)
   {
      filePath.Assign(VarList(dllPathVar)[i]);
 	   ReplaceVarInString(itfc,filePath);
	
	   if(SetCurrentDirectory(filePath.Str()))
	   {
	      nrDLLs += CountFiles(DLLFILES);
	      SetCurrentDirectory(backUpPath); // Return to previous path
	   }
	}
	
	if(nrDLLs == 0)
	{
	   SetCurrentDirectory(backUpPath);
	   return(OK);
	}
	   
// Allocate space for DLL commands
	DLLInfo = new DLLINFO[nrDLLs];
	
// Load all libraries
   for(j = 0, i = 0; i < VarWidth(dllPathVar); i++)
   {
      filePath.Assign(VarList(dllPathVar)[i]);
 		ReplaceVarInString(itfc,filePath);
	
	   if(!SetCurrentDirectory(filePath.Str()))
	      continue;  
	
      HANDLE hdl = FindFirstFile(DLLFILES,&findData);
      if(hdl == INVALID_HANDLE_VALUE)
        continue;
      
      fileFound = true;
      CText fullName;
	   while(fileFound)
	   {
         CText fullPath;
         fullPath = filePath.Str();
         fullPath = fullPath + findData.cFileName;
         DLLInfo[j].Instance = LoadLibrary(fullPath.Str());
       //  DLLInfo[j].Instance = LoadLibrary(findData.cFileName);
         fullName = findData.cFileName;
          DLLInfo[j].Name = fullName.Start(fullName.Size() - suffixSz);
	       DLLInfo[j].AddCommands = (MYFUNC)GetProcAddress(DLLInfo[j].Instance,"AddCommands");
	       if(!DLLInfo[j].AddCommands)
	       {
              MessageDialog(prospaWin,MB_ICONWARNING,"Fatal DLL Error","'%s' has no 'AddCommand' procedure",findData.cFileName);
              return(ERR);
          }        
		    DLLInfo[j].ListCommands = (MYFUNC2)GetProcAddress(DLLInfo[j].Instance,"ListCommands");
		    DLLInfo[j++].GetCommandSyntax = (MYFUNC3)GetProcAddress(DLLInfo[j].Instance,"GetCommandSyntax");
	       fileFound = FindNextFile(hdl,&findData);
	   }
      FindClose(hdl);
	   
	   SetCurrentDirectory(backUpPath);
	}
	itfc->nrRetValues = 0;
   return(OK);
}


/*******************************************************************
  Load all DLLs except those in the ignore list
*******************************************************************/

int IgnoreDLLs(Interface* itfc ,char args[])
{
   WIN32_FIND_DATA findData;
   char backUpPath[MAX_PATH];
   CText filePath;
   Variable ignoredDLLsVar;
   char **ignoredDlls;
   int i,j;
   bool fileFound;
   short nrArgs;
   short nrToIgnore;
   CText fullName;


   // Select the DLL to ignore
   if ((nrArgs = ArgScan(itfc, args, 1, "DLL list to ignore", "e", "v", &ignoredDLLsVar)) < 0)
      return(nrArgs);

   if(ignoredDLLsVar.GetType() != LIST)
   {
      ErrorMessage("DLLs to ignore should be a list");
      return(ERR);
   }

   ignoredDlls = ignoredDLLsVar.GetList();
   nrToIgnore = ignoredDLLsVar.GetDimX();

// Unload all DLLs
   if(nrDLLs != 0)
   {
      UnloadDLLs(itfc,"");
   }

// Measure the length of the DLL filename suffix (e.g. Debug.dll)
   int suffixSz = strlen(DLLFILES);

// Save the current directory
   GetCurrentDirectory(MAX_PATH,backUpPath);
 
// Loop over entries in the dll search-path variable counting number of DLLs
   for(i = 0; i < VarWidth(dllPathVar); i++)
   {
      filePath.Assign(VarList(dllPathVar)[i]);
 		ReplaceVarInString(itfc,filePath);
	
	   if(SetCurrentDirectory(filePath.Str()))
	   {
         nrDLLs += CountFilesWithIgnores(DLLFILES, ignoredDlls, nrToIgnore);
         SetCurrentDirectory(backUpPath); // Return to previous path
	   }
	}
	
	if(nrDLLs == 0)
	{
	   SetCurrentDirectory(backUpPath);
	   return(OK);
	}
	  

// Allocate space for DLL commands
	DLLInfo = new DLLINFO[nrDLLs];
	
// Load all libraries
	for(j = 0, i = 0; i < VarWidth(dllPathVar); i++)
	{
      filePath.Assign(VarList(dllPathVar)[i]);
 		ReplaceVarInString(itfc,filePath);
	
	   if(!SetCurrentDirectory(filePath.Str()))
	      continue;  
	
      HANDLE hdl = FindFirstFile(DLLFILES,&findData);
      if(hdl == INVALID_HANDLE_VALUE)
         continue;
      
      fileFound = true;

      while (fileFound)
		{
         int k;
         for (k = 0; k < nrToIgnore; k++)
         {
            if (!stricmp(findData.cFileName, ignoredDlls[k]))
               break;
         }

         if(k == nrToIgnore)
         {
				DLLInfo[j].Instance = LoadLibrary(findData.cFileName);
				DLLInfo[j].AddCommands = (MYFUNC)GetProcAddress(DLLInfo[j].Instance, "AddCommands");
				if (!DLLInfo[j].AddCommands)
				{
					MessageDialog(prospaWin, MB_ICONWARNING, "Fatal DLL Error", "'%s' has no 'AddCommand' procedure", findData.cFileName);
					return(ERR);
				}
            fullName = findData.cFileName;
            DLLInfo[j].Name = fullName.Start(fullName.Size() - suffixSz);
				DLLInfo[j].ListCommands = (MYFUNC2)GetProcAddress(DLLInfo[j].Instance, "ListCommands");
				DLLInfo[j++].GetCommandSyntax = (MYFUNC3)GetProcAddress(DLLInfo[j].Instance, "GetCommandSyntax");
			}
			fileFound = FindNextFile(hdl, &findData);
		}
      FindClose(hdl);	   
	   SetCurrentDirectory(backUpPath);
	}
	itfc->nrRetValues = 0;
   return(OK);
}

int IgnoreDLL(Interface* itfc, char args[])
{
   WIN32_FIND_DATA findData;
   char backUpPath[MAX_PATH];
   CText filePath, ignoreDLL;
   int i, j;
   bool fileFound;
   short nrArgs;
   CText fullName;
   CArg cargs;

   // Select the DLL to ignore
   if ((nrArgs = ArgScan(itfc, args, 1, "DLL to ignore", "e", "t", &ignoreDLL)) < 0)
      return(nrArgs);

   if (nrDLLs != 0)
   {
      UnloadDLLs(itfc, "");
   }

   // Measure the length of the DLL filename suffix (e.g. Debug.dll)
   int suffixSz = strlen(DLLFILES);

   // Save the current directory
   GetCurrentDirectory(MAX_PATH, backUpPath);

   // Loop over entries in the dll search-path variable counting number of DLLs
   for (i = 0; i < VarWidth(dllPathVar); i++)
   {
      filePath.Assign(VarList(dllPathVar)[i]);
      ReplaceVarInString(itfc, filePath);

      if (SetCurrentDirectory(filePath.Str()))
      {
         nrDLLs += CountFilesWithIgnore(DLLFILES, ignoreDLL.Str());
         SetCurrentDirectory(backUpPath); // Return to previous path
      }
   }

   if (nrDLLs == 0)
   {
      SetCurrentDirectory(backUpPath);
      return(OK);
   }

   // Allocate space for DLL commands
   DLLInfo = new DLLINFO[nrDLLs];

   // Load all libraries
   for (j = 0, i = 0; i < VarWidth(dllPathVar); i++)
   {
      filePath.Assign(VarList(dllPathVar)[i]);
      ReplaceVarInString(itfc, filePath);

      if (!SetCurrentDirectory(filePath.Str()))
         continue;

      HANDLE hdl = FindFirstFile(DLLFILES, &findData);
      if (hdl == INVALID_HANDLE_VALUE)
         continue;

      fileFound = true;

      while (fileFound)
      {
         if (stricmp(findData.cFileName, ignoreDLL.Str()))
         {
            DLLInfo[j].Instance = LoadLibrary(findData.cFileName);
            DLLInfo[j].AddCommands = (MYFUNC)GetProcAddress(DLLInfo[j].Instance, "AddCommands");
            if (!DLLInfo[j].AddCommands)
            {
               MessageDialog(prospaWin, MB_ICONWARNING, "Fatal DLL Error", "'%s' has no 'AddCommand' procedure", findData.cFileName);
               return(ERR);
            }
            fullName = findData.cFileName;
            DLLInfo[j].Name = fullName.Start(fullName.Size() - suffixSz);
            DLLInfo[j].ListCommands = (MYFUNC2)GetProcAddress(DLLInfo[j].Instance, "ListCommands");
            DLLInfo[j++].GetCommandSyntax = (MYFUNC3)GetProcAddress(DLLInfo[j].Instance, "GetCommandSyntax");
         }
         fileFound = FindNextFile(hdl, &findData);
      }
      FindClose(hdl);
      SetCurrentDirectory(backUpPath);
   }
   itfc->nrRetValues = 0;
   return(OK);
}

/*******************************************************************
  Reload the DLLs putting one DLL at the front
*******************************************************************/

int UseDLL(Interface* itfc, char args[])
{
   WIN32_FIND_DATA findData;
   char backUpPath[MAX_PATH];
   CText filePath,firstDLL;
   int i,j;
   bool fileFound;
   short nrArgs;
   bool firstFound;
   CText fullName;

// Load the DLL to load first
   if((nrArgs = ArgScan(itfc,args,1,"DLL to load first","e","t",&firstDLL)) < 0)
      return(nrArgs);

// Unload existing DLLS
   if(nrDLLs != 0)
   {
      UnloadDLLs(itfc,"");
   }

// Measure the length of the DLL filename suffix (e.g. Debug.dll)
   int suffixSz = strlen(DLLFILES);

// Save the current directory
   GetCurrentDirectory(MAX_PATH,backUpPath);
 
// Loop over entries in the dll search-path variable counting number of DLLs
   for(i = 0; i < VarWidth(dllPathVar); i++)
   {
      filePath.Assign(VarList(dllPathVar)[i]);
 		ReplaceVarInString(itfc,filePath);
	
	   if(SetCurrentDirectory(filePath.Str()))
	   {
	      nrDLLs += CountFiles(DLLFILES);
	      SetCurrentDirectory(backUpPath); // Return to previous path
	   }
	}
	
	if(nrDLLs == 0)
	{
	   SetCurrentDirectory(backUpPath);
	   return(OK);
	}
	   
// Allocate space for DLL commands
	DLLInfo = new DLLINFO[nrDLLs];

// Load first DLL
      firstFound = false;
      for(i = 0; i < VarWidth(dllPathVar); i++)
      {
         filePath.Assign(VarList(dllPathVar)[i]);
 		   ReplaceVarInString(itfc,filePath);
	
	      if(!SetCurrentDirectory(filePath.Str()))
	         continue;  
	
         HANDLE hdl = FindFirstFile(DLLFILES,&findData);
         if(hdl == INVALID_HANDLE_VALUE)
            continue;
      
         fileFound = true;
         while(fileFound)
	      {
		      if(!stricmp(findData.cFileName,firstDLL.Str()))
		      {
			      DLLInfo[0].Instance = LoadLibrary(findData.cFileName);
			      DLLInfo[0].AddCommands = (MYFUNC)GetProcAddress(DLLInfo[0].Instance, "AddCommands");
               fullName = findData.cFileName;
               DLLInfo[0].Name = fullName.Start(fullName.Size() - suffixSz);
			      if (!DLLInfo[0].AddCommands)
			      {
				      MessageDialog(prospaWin, MB_ICONWARNING, "Fatal DLL Error", "'%s' has no 'AddCommand' procedure", findData.cFileName);
				      return(ERR);
			      }
			      DLLInfo[0].ListCommands = (MYFUNC2)GetProcAddress(DLLInfo[0].Instance, "ListCommands");
			      DLLInfo[0].GetCommandSyntax = (MYFUNC3)GetProcAddress(DLLInfo[0].Instance, "GetCommandSyntax");
               firstFound = true;
               break;
            }
 
	         fileFound = FindNextFile(hdl,&findData);
	      }
         FindClose(hdl);
	   
	      SetCurrentDirectory(backUpPath);

         if(firstFound)
            break;
   }

   if(!firstFound)
   {
      delete [] DLLInfo;
      DLLInfo = 0;
      LoadDLLs(itfc,""); // Reload the DLLs
      ErrorMessage("DLL %s not found",firstDLL.Str());
      return(ERR);
   }

// Load all other DLLs
   for(j = 1, i = 0; i < VarWidth(dllPathVar); i++)
   {
      filePath.Assign(VarList(dllPathVar)[i]);
 	   ReplaceVarInString(itfc,filePath);
	
	   if(!SetCurrentDirectory(filePath.Str()))
	      continue;  
	
      HANDLE hdl = FindFirstFile(DLLFILES,&findData);
      if(hdl == INVALID_HANDLE_VALUE)
         continue;
      
      fileFound = true;
      while(fileFound)
      {
         if(stricmp(findData.cFileName,firstDLL.Str())) // Ignore first DLL already loaded
         {
	         DLLInfo[j].Instance = LoadLibrary(findData.cFileName);
	         DLLInfo[j].AddCommands = (MYFUNC)GetProcAddress(DLLInfo[j].Instance, "AddCommands");
	         if (!DLLInfo[j].AddCommands)
	         {
		         MessageDialog(prospaWin, MB_ICONWARNING, "Fatal DLL Error", "'%s' has no 'AddCommand' procedure", findData.cFileName);
		         return(ERR);
	         }
            fullName = findData.cFileName;
            DLLInfo[j].Name = fullName.Start(fullName.Size() - suffixSz);
	         DLLInfo[j].ListCommands = (MYFUNC2)GetProcAddress(DLLInfo[j].Instance, "ListCommands");
	         DLLInfo[j++].GetCommandSyntax = (MYFUNC3)GetProcAddress(DLLInfo[j].Instance, "GetCommandSyntax");
         }

	      fileFound = FindNextFile(hdl,&findData);
      }
      FindClose(hdl);
	   
      SetCurrentDirectory(backUpPath);
   }     
   return(OK);
}

/*******************************************************************
   Unload all currently loaded DLLs and associated free memory
*******************************************************************/

int UnloadDLLs(Interface* itfc, char args[])
{
   long i;
   bool s;

   if (!DLLInfo)
   {
      nrDLLs = 0;
      return(OK);
   }

   for(i = 0; i < nrDLLs; i++)
   {
      if(DLLInfo[i].Instance)
         s = FreeLibrary(DLLInfo[i].Instance);
   }
   delete [] DLLInfo;
   DLLInfo = NULL;
   nrDLLs = 0;

   return(OK);
}


/*******************************************************************
List information about the DLLs loaded

Syntax: listdlls([which])

Possible arguments are:
'all' ...... list all DLL functions (default)
'names' .... Just list the names of the DLLs
library .... List all the functions of the specified library

Note that the command is not case sensitive
*********************************************************************/

int ListDLLCommands(Interface* itfc, char args[])
{
   CText which = "*";
   CText start = "*";
   short nrArgs;
   static char syntax[MAX_STR];
   static char result[MAX_STR];

   // Get arguments if any
   if ((nrArgs = ArgScan(itfc, args, 0, "library/names/all", "e", "t", &which)) < 0)
      return(nrArgs);

   which.LowerCase(); // Convert name to lower case 

   if (nrArgs == 0 || which == "all") // List all the DLLs
   {
      for (int i = 0; i < nrDLLs; i++)
      {
         if (DLLInfo[i].ListCommands)
            DLLInfo[i].ListCommands();
      }
      itfc->nrRetValues = 0;
   }
   else
   {
      if (which == "names") // Just give the names
      {
         char **cList = NULL;
         int cnt = 0;
         for (int i = 0; i < nrDLLs; i++)
         {
            AppendStringToList(DLLInfo[i].Name.Str(), &cList, cnt++);
         }
         itfc->retVar[1].AssignList(cList, cnt);
         itfc->nrRetValues = 1;
      }
      else
      {
         int found = 0;
         for (int i = 0; i < nrDLLs; i++) // List the functions for a named DLL
         {
            CText name = DLLInfo[i].Name;
            name.LowerCase(); // Convert DLL name to lower case
            if (name.Str() == which)
            {
               found = 1;
               if (DLLInfo[i].ListCommands)
               {
                  DLLInfo[i].ListCommands();
               }
            }
            itfc->nrRetValues = 0;
         }
         if (found == 0)
         {
            TextMessage("\n\n   No DLL with the name '%s' found.\n", which.Str());
            itfc->nrRetValues = 0;
         }
      }
   }
   return(OK);
}

/**************************  Helper functions *************************************/

// Make sure the DLL parameter class is up to date
void UpdateDLLParameters(DLLParameters *dllPar, Interface *itfc)
{
   dllPar->nrRetValues = itfc->nrRetValues;
   dllPar->retVar = itfc->retVar;
   dllPar->Interface = (void*)itfc;
}


// Count the number of files of a certain name (e.g. *.DLL) in current folder.
short CountFiles(char name[])
{
   HANDLE hdl;
   WIN32_FIND_DATA findData;
   short cnt = 0;
   bool found;
   
   hdl =  FindFirstFile(name,&findData);

   if(hdl == INVALID_HANDLE_VALUE) return(0);

   found = true;
   while(found)
   {
      cnt++;
      found = FindNextFile(hdl,&findData);
   }
   FindClose(hdl);
   
   return(cnt);     
}


// Count the number of files of a certain name (e.g. *.DLL) in current folder ignoring file
short CountFilesWithIgnore(char name[], char ignoreFile[])
{
   HANDLE hdl;
   WIN32_FIND_DATA findData;
   short cnt = 0;
   bool found;
   
   hdl =  FindFirstFile(name,&findData);

   if(hdl == INVALID_HANDLE_VALUE) return(0);

   found = true;
   while(found)
   {
      if(stricmp(findData.cFileName,ignoreFile))
         cnt++;
      found = FindNextFile(hdl,&findData);
   }
   FindClose(hdl);
   
   return(cnt);     
}


// Count the number of files of a certain name (e.g. *.DLL) in current folder ignoring file
short CountFilesWithIgnores(char name[], char **ignoreFiles, int nrFilesToIgnore)
{
   HANDLE hdl;
   WIN32_FIND_DATA findData;
   short cnt = 0;
   bool found;
   int i;

   hdl = FindFirstFile(name, &findData);

   if (hdl == INVALID_HANDLE_VALUE) return(0);

   found = true;
   while (found)
   {
      for (i = 0; i < nrFilesToIgnore; i++)
      {
         if (!stricmp(findData.cFileName, ignoreFiles[i]))
            break;
      }
      if(i == nrFilesToIgnore)
         cnt++;
      found = FindNextFile(hdl, &findData);
   }
   FindClose(hdl);

   return(cnt);
}

// Search for additional commands in the DLL folder
// Returns OK if command found and run
// Returns RETURN_FROM_DLL is command not found
// Otherwise the command has been found but there was an error

short ProcessContinue(Interface *itfc,char *command, char *parameters)
{
	DLLParameters dllPar;
	short r = RETURN_FROM_DLL;
	UpdateDLLParameters(&dllPar,itfc); 
	for(int i = 0; i < nrDLLs; i++) 
	{
		if(DLLInfo[i].AddCommands)
		{
			r = DLLInfo[i].AddCommands(command,parameters,&dllPar);
			if(r == OK || r != RETURN_FROM_DLL) 
				break; // Command found or error
		}
	}
	itfc->nrRetValues = dllPar.nrRetValues;
	return(r);
}


// Get the syntax for a command (there may be more than 1)
char* GetDLLCommandSyntax(char cmd[])
{
   char cmdSyntax[500];
	CText accumSyntax;
   char *finalSyntax;

   accumSyntax = "DLL COMMAND:   ";

   for(int i = 0; i < nrDLLs; i++) 
   {
      if(DLLInfo[i].GetCommandSyntax)
      {
         if(DLLInfo[i].GetCommandSyntax(cmd,cmdSyntax))
         {
            accumSyntax.Concat(cmdSyntax);
				finalSyntax = new char[accumSyntax.Size()+1];
				strcpy(finalSyntax,accumSyntax.Str());
            return(finalSyntax);
         }
      }
   }
   return(NULL);
}

// Find the syntax for the function defined by the string cmd. Other parameters
// path ... path of the current editor
// currentMacro ... name of macro in current editor
// syntax .... returned syntax - should be deleted when out of scope
// calledPath ... where the macro was actually found
// calledProcedure ... the name of the procedure extracted from cmd
//
int GetFunctionSyntax(char *path, char* currentMacro, char *cmd, char** syntax, char* calledPath, char* calledMacro, char* calledProcedure)
{
   int sz = strlen(cmd);

   if(sz >= MAX_STR)
   {
      *syntax = new char[1];
      (*syntax)[0] = '\0';
      return(OK);
   }

   char macro[MAX_PATH]; 
   char procedure[MAX_PATH];

   char *delm = strchr(cmd,':');

   if(delm)
   {
		if(delm-cmd >= MAX_PATH)
		{
			*syntax = new char[1];
			(*syntax)[0] = '\0';
			return(OK);
		}

      strncpy(macro,cmd,delm-cmd);
      macro[delm-cmd] = '\0';

      strncpy(procedure,&cmd[delm-cmd+1],(sz-(int)(delm-cmd)));
      procedure[(sz-(int)(delm-cmd))] = '\0';

      if(macro[0] == '\0')
         strcpy(macro,currentMacro);
   }
   else
   {
		//if(userClassCmd != 0)
		//{
		//	ExtractClassProcName(cmd,procedure);

		////	strcpy(procedure,cmd);
		//	strcpy(macro,cmd);
		//}
		//else
		//{
			strcpy(procedure,cmd);
			strcpy(macro,cmd);
		//}
   }

// Get the procedure text
   char curDir[MAX_PATH];
   Interface itfc;
   FILE *fp;
   extern short GetArgList(long pos, char *text, char *argStr);

// First find the file
   strcpy(curDir,path);
   if(!(fp = FindFolder(&itfc,curDir,macro,".mac")))
   {
		if(!(fp = FindFolder(&itfc,curDir,macro,".pex")))
			return(ERR);
	}
   fclose(fp);

// Load the file text
   char* text = LoadTextFileFromFolder(curDir, macro,".mac");
   long lineNr = 0;
   if(text)
   {
      int pos = 0;
      *syntax = new char[strlen(text)+1];
    // Find the procedure
      if(FindProcedure(text,procedure,lineNr,false) == ERR)
      {
         strcpy(calledPath,curDir);
         strcpy(calledMacro,macro);
         strcpy(calledProcedure,procedure);
         *syntax[0] = '\0';
         delete [] text;
         return(OK);
      }
		// Make the syntax string
      char *argStr = new char[strlen(text)+1];
      char *startChar = strchr(text+pos,'(');
      pos = startChar-text+strlen(procedure)+1;
      GetArgList(pos, text, argStr);
      strcpy(*syntax,"procedure:   ");
      strcat(*syntax,procedure);
      strcat(*syntax,"(");
      strcat(*syntax,argStr);
      strcat(*syntax,")");
      strcpy(calledPath,curDir);
      strcpy(calledMacro,macro);
      strcpy(calledProcedure,procedure);

      delete [] argStr;
      delete [] text;
   }
	return(OK);
}

// Checks to see if a DLL command exists
bool IsADLLCommand(char *cmd)
{
   char *syntax = GetDLLCommandSyntax(cmd);
	if(!syntax)
      return(false);
	delete [] syntax;
   return(true);
}

// Return the name of the help file which includes a description of DLL command 'cmd'
// Returns true on success false on failure.

bool GetDLLHelpFolderName(Interface *itfc, char *cmd, CText &name)
{
   static char syntax[MAX_STR];
   DLLParameters dllPar;
   UpdateDLLParameters(&dllPar,itfc); 

   for(int i = 0; i < nrDLLs; i++) 
   {
      if(DLLInfo[i].GetCommandSyntax)
      {
         if(DLLInfo[i].GetCommandSyntax(cmd,syntax))
         {
            if(DLLInfo[i].AddCommands("helpfolder", "", &dllPar) == OK)
            {
               if(dllPar.nrRetValues == 1 && dllPar.retVar[1].GetType() == UNQUOTED_STRING)
                  name = dllPar.retVar[1].GetString();
               else
                  return(false);
            }
            else
            {
               return(false);
            }
            return(true);
         }
      }
   }
   return(false);
}


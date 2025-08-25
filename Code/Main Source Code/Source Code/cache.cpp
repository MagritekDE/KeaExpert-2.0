#include "stdafx.h"
#include "globals.h"
#include "cache.h"
#include "guiWindowClass.h"
#include "scanstrings.h"
#include "error.h"
#include "files.h"
#include "edit_files.h"
#include "debug.h"
#include "interface.h"
#include "variablesOther.h"
#include "memoryLeak.h"

Variable gCachedProc; // The global procedure cache

/************************************************************************************
   Add a macro to a cache

   cacemacro(macro_name, mode)

   mode: "window" add to window cache
         "local"  add to macro cache
         "global"  add to global cache
*************************************************************************************/

int CacheMacro(Interface* itfc, char args[])
{
	CText fileNameTxt;
	char filePath[MAX_PATH] = "";
	char fileName[MAX_PATH] = "";
	CText mode = "window";
	short nrArgs;
	FILE *fp;
	extern Variable gCachedProc;
	extern void AddProcedures(char *path, char* name);
	extern void AddProcedures(Interface* itfc, char *path, char* name);

	// Get filename from user  
	if((nrArgs = ArgScan(itfc,args,1,"filename, mode","ee","tt",&fileNameTxt,&mode)) < 0)
		return(nrArgs); 

   strncpy(fileName,fileNameTxt.Str(),MAX_PATH);

	itfc->nrRetValues = 0;

	// Search for fileName using the defined user search path
	if((fp = FindFolder(itfc,filePath,fileName,".mac")) == NULL)
   {
      ErrorMessage("Can't find macro '%s'",fileName);
		return(ERR);
   }
	fclose(fp);

	if(mode == "global")
	{
		AddProcedures(filePath,fileName);
		return(OK);
	}

	if(mode == "window")
	{
		if(itfc->win) 
		{
			itfc->win->AddProcedures(filePath,fileName);
			return(OK);
		}
		else
		{
			mode = "local";
		}
	}
	
	if(mode == "local")
	{
		if(itfc->baseMacro)
			AddProcedures(itfc,filePath,fileName);
		else
		{
			ErrorMessage("a parent macro has not been defined");
			return(ERR);
		}
	}
	else
	{
		ErrorMessage("invalid mode '%s'",mode);
		return(ERR);
	}

	return(OK);
}

/************************************************************************************
	Checks of a file is cached or not
*************************************************************************************/

int IsFileCached(Interface* itfc, char args[])
{
	CText path="";
	CText name;
	CText mode = "window";
	short nrArgs;
	bool found = false;

	// Get file information from user  
	if ((nrArgs = ArgScan(itfc, args, 3, "macroname, macropath, mode", "eee", "ttt", &name, &path, &mode)) < 0)
		return(ERR);

	if (mode == "window")
	{
		if (itfc->win)
		{
			Variable* procList = &(itfc->win->procList);

			for (Variable* var = procList->next; var != NULL; var = var->next)
			{
				ProcedureInfo* procInfo = (ProcedureInfo*)var->GetData();

				if (!stricmp(procInfo->macroName, name.Str()) && !stricmp(procInfo->macroPath, path.Str()))
				{
					itfc->retVar[1].MakeAndSetFloat(1);
					itfc->nrRetValues = 1;
					return(OK);
				}
			}
		}
	}
	else if (mode == "global")
	{
		if (gCachedProc.next)
		{
			for (Variable* var = gCachedProc.next; var != NULL; var = var->next)
			{
				ProcedureInfo* procInfo = (ProcedureInfo*)var->GetData();

				if (!stricmp(procInfo->macroName, name.Str()) && !stricmp(procInfo->macroPath, path.Str()))
				{
					itfc->retVar[1].MakeAndSetFloat(1);
					itfc->nrRetValues = 1;
					return(OK);
				}
			}
		}
	}

	itfc->retVar[1].MakeAndSetFloat(0);
	itfc->nrRetValues = 1;
	return(OK);
}

/************************************************************************************
   Toggles the caching of procedures in macro calls
*************************************************************************************/

int CacheProcedures(Interface* itfc, char args[])
{
   short n;
   CText cache;
   
   if(itfc->cacheProc)
      cache.Assign("true");
   else
      cache.Assign("false");
   
   if((n = ArgScan(itfc,args,1,"yes/no","e","t",&cache)) < 0)
      return(n);
   
	if(cache == "true" || cache == "yes")
	{
      itfc->cacheProc = true;
      if(itfc->win)
         itfc->win->cacheProc = true;
	}
	else if(cache == "false" || cache == "no")
	{
      itfc->cacheProc = false;
      if(itfc->win)
         itfc->win->cacheProc = false;
   }
	else
	{
	   ErrorMessage("invalid argument (true/false)");
	   return(ERR);
	}
	itfc->nrRetValues = 0;
   return(OK);
}


// Remove a macro from the cache using its path and filename. Returns 1 if found 0 otherwise

int RemoveCachedMacro(Interface *itfc,  char args[])
{

   CText path;
   CText name;
	CText mode = "window";
	CText verbose = "false";
   short nrArgs;
   bool found = false;

	// Get filename from user  
	if((nrArgs = ArgScan(itfc,args,2,"macropath, macroname, mode, verbose","eeee","tttt",&path,&name,&mode,&verbose)) < 0)
		return(ERR); 

 //  TextMessage("path = %s name = %s\n",path.Str(),name.Str());

	if(mode == "window")
	{
		if(itfc->win)
		{
		//   TextMessage("Window found nr = %hd\n",itfc->win->nr);

			Variable *procList = &(itfc->win->procList);

			for(Variable *var = procList->next; var != NULL; var = var->next)
			{
				 ProcedureInfo *procInfo = (ProcedureInfo*)var->GetData();

				if(!stricmp(procInfo->macroName,name.Str()) && !stricmp(procInfo->macroPath,path.Str()))
				{
			     // TextMessage("Found match in window cache - removing\n");
					Variable *last = var->last;
					Variable *next = var->next;
					delete var;
					last->next = next;
					var = last;
					found = true;
				}
			}
		}
	}
	else if(mode == "global")
	{
		if(gCachedProc.next)
		{
			for(Variable *var = gCachedProc.next; var != NULL; var = var->next)
			{
				 ProcedureInfo *procInfo = (ProcedureInfo*)var->GetData();

				if(!stricmp(procInfo->macroName,name.Str()) && !stricmp(procInfo->macroPath,path.Str()))
				{
			    // TextMessage("Found match in global cache - removing\n");

					Variable *last = var->last;
					Variable *next = var->next;
					delete var;
					last->next = next;
					var = last;
					found = true;
				}
			}
		}
	}

	if(verbose == "true")
	{
		if(found)
		{
			itfc->retVar[1].MakeAndSetFloat(1.0);
			itfc->nrRetValues = 1;
		}
		else
		{
			itfc->retVar[1].MakeAndSetFloat(0.0);
			itfc->nrRetValues = 1;
		}
   }
	else
	{
		itfc->nrRetValues = 0;
	}

   return(OK);
}

// Remove all the cached macros by mode

int RemoveCachedMacros(Interface *itfc,  char args[])
{
	CText mode = "window";
	CText verbose = "false";
   short nrArgs;

	// Get filename from user  
	if((nrArgs = ArgScan(itfc,args,1,"mode","e","t",&mode)) < 0)
		return(ERR); 

	if(mode == "window")
	{
		if(itfc->win)
		{
			Variable *procList = &(itfc->win->procList);

			for(Variable *var = procList->next; var != NULL; var = var->next)
			{
				ProcedureInfo *procInfo = (ProcedureInfo*)var->GetData();
				Variable *last = var->last;
				Variable *next = var->next;
				delete var;
				last->next = next;
				var = last;
			}
		}
	}
	else if(mode == "global")
	{
		if(gCachedProc.next)
		{
			for(Variable *var = gCachedProc.next; var != NULL; var = var->next)
			{
				ProcedureInfo *procInfo = (ProcedureInfo*)var->GetData();
				Variable *last = var->last;
				Variable *next = var->next;
				delete var;
				last->next = next;
				var = last;
			}
		}
	}

	itfc->nrRetValues = 0;
   return(OK);
}

/***************************************************************************************
  Search for a procedure in the global cache. Use path and macro and proc name
  If a macro extension is not passed then try .mac and .pex
  Note that path and macro are modified and so should be long enough
  to hold a complete pathname.
  NOTE the arguments strings are modified and should be MAX_PATH c strings
***************************************************************************************/

Variable* GetProcedure(char *path, char *macro, char *name)
{
   ProcedureInfo *procInfo;
   Variable *var;
   char fullNameMac[MAX_PATH];
   char fullNamePex[MAX_PATH];
   char curPath[MAX_PATH];

	EnterCriticalSection(&csCache);

// Assume its a macro with extension .mac
   strcpy(fullNameMac,macro);
   AddExtension(fullNameMac,".mac");
   strcpy(fullNamePex,macro);
   AddExtension(fullNamePex,".pex");

   GetCurrentDirectory(MAX_PATH,curPath);

	//TextMessage("Searching for cached macro:procedure %s:%s\n",macro,name);

// First search for a complete match - same path, same macro and same procedure

	for(var = gCachedProc.next; var != NULL; var = var->next)
	{
      procInfo = (ProcedureInfo*)var->GetString(); 

      if(name[0] == '\0') // Search for path\macro
      {
         if(!stricmp(var->GetName(),macro))
         {
            if(path[0] != '\0') // Search for path\macro
            {
               if(ComparePathNames(procInfo->macroPath,path) &&  (!stricmp(procInfo->macroName,fullNameMac)))
               {
                  strcpy(macro,fullNameMac);
						LeaveCriticalSection(&csCache);
                  return(var);
               }

               if(ComparePathNames(procInfo->macroPath,path) &&  (!stricmp(procInfo->macroName,fullNamePex)))
               {
                  strcpy(macro,fullNamePex);
						LeaveCriticalSection(&csCache);
                  return(var);
               }
            }
            else // Search for curpath\macro
            {
               if(ComparePathNames(procInfo->macroPath,curPath) &&  (!stricmp(procInfo->macroName,fullNameMac)))
               {
                  strcpy(macro,fullNameMac);
                  strcpy(path,procInfo->macroPath);
						LeaveCriticalSection(&csCache);
	               return(var);
               }
               if(ComparePathNames(procInfo->macroPath,curPath) &&  (!stricmp(procInfo->macroName,fullNamePex)))
               {
                  strcpy(macro,fullNamePex);
                  strcpy(path,procInfo->macroPath);
						LeaveCriticalSection(&csCache);
	               return(var);
               }
            }
         }
      }
      
      else if(!stricmp(var->GetName(),name))
	   {
         if(path[0] != '\0') // Search for path\macro:name
         {
            if(ComparePathNames(procInfo->macroPath,path) &&  (!stricmp(procInfo->macroName,fullNameMac)))
            {
               strcpy(macro,fullNameMac);
				   LeaveCriticalSection(&csCache);
               return(var);
            }

            if(ComparePathNames(procInfo->macroPath,path) &&  (!stricmp(procInfo->macroName,fullNamePex)))
            {
               strcpy(macro,fullNamePex);
					LeaveCriticalSection(&csCache);
               return(var);
            }
         }
         else // Search for curPath\macro:name
         {
            if(ComparePathNames(procInfo->macroPath,curPath) &&  (!stricmp(procInfo->macroName,fullNameMac)))
            {
               strcpy(macro,fullNameMac);
               strcpy(path,procInfo->macroPath);
					LeaveCriticalSection(&csCache);
	            return(var);
            }
            if(ComparePathNames(procInfo->macroPath,curPath) &&  (!stricmp(procInfo->macroName,fullNamePex)))
            {
               strcpy(macro,fullNamePex);
               strcpy(path,procInfo->macroPath);
					LeaveCriticalSection(&csCache);
	            return(var);
            }
         }
	   }
	}

// Check to see if the file is in the current directory - if so don't use cache
  if(path[0] != '\0') 
  {
     if(IsFile(fullNameMac))
	  {
		  LeaveCriticalSection(&csCache);
        return(NULL);
	  }
  }

// Return the first match found ignoring the current path
   for(var = gCachedProc.next; var != NULL; var = var->next)
	{
      procInfo = (ProcedureInfo*)var->GetString(); 

      if(name[0] == '\0')
      {
         if(!stricmp(var->GetName(),macro))
         {
            if(!stricmp(procInfo->macroName,fullNameMac))
            {
               strcpy(macro,fullNameMac);
               strcpy(path,procInfo->macroPath);
		         LeaveCriticalSection(&csCache);
	            return(var);
            }
            if(!stricmp(procInfo->macroName,fullNamePex))
            {
               strcpy(macro,fullNamePex);
               strcpy(path,procInfo->macroPath);
					LeaveCriticalSection(&csCache);
	            return(var);
            } 
         }
      }
      
      else if(!stricmp(var->GetName(),name))
	   {
         if(!stricmp(procInfo->macroName,fullNameMac))
         {
            strcpy(macro,fullNameMac);
            strcpy(path,procInfo->macroPath);
				LeaveCriticalSection(&csCache);
	         return(var);
         }
         if(!stricmp(procInfo->macroName,fullNamePex))
         {
            strcpy(macro,fullNamePex);
            strcpy(path,procInfo->macroPath);
				LeaveCriticalSection(&csCache);
	         return(var);
         }  
	   }
	}

   LeaveCriticalSection(&csCache);

   return(NULL);
}



void AddProcedures(char *path, char* name)
{
	char fileName[MAX_PATH];
	long startLine = 0;
	long lineNr = 0;

	strcpy(fileName,name);

	// Read in the text 
	char *text = LoadTextFileFromFolder(path, fileName,".mac");
	if(!text) return;

	char *procedure;
	char procName[MAX_STR];
	long i = 0;
	long len = strlen(text);
	Variable* procVar;

	// Make sure we have an filename extension and are lower case
	AddExtension(fileName,".mac");
	//  ToLowerCase(fileName);

	// Search for all procedures in text
	while(FindProcedures(text, i, &procedure, procName, len, lineNr, startLine))
	{
      // If this procedure is already in the macro list remove it
      procVar = GetProcedure(path,name,procName);
      if(procVar) // Remove previous cached procedure
         procVar->Remove();
		delete procVar;
	//	TextMessage("Removing and adding %s to cache (%s)\n", procName, path);
		procVar = gCachedProc.AddToStart(PROCEDURE, procName);
		//procVar = gCachedProc.Add(PROCEDURE, procName);
		procVar->MakeAndSetProcedure(procedure,procName,fileName,path,startLine);
		delete [] procedure;
	}
	delete [] text;
}

void ListGlobalsCachedProcs(CText macroFilter)
{
   CText path = "";
	CText macro = "";

	TextMessage("\n\n   ### Global macro cache ###\n\n");
	Variable *procList = &gCachedProc;
	int cnt = 0;

	for(Variable *var = procList->next; var != NULL; var = var->next)
	{
		ProcedureInfo *procInfo = (ProcedureInfo*)var->GetData();

		if(macroFilter == "" | macroFilter == procInfo->macroName)
		{
			if(procInfo->macroName != macro || procInfo->macroPath != path)
				TextMessage("   Macro = '%s' (Index = %d, Path = '%s')\n",procInfo->macroName,cnt,procInfo->macroPath);

			macro = procInfo->macroName;
			path = procInfo->macroPath;

			TextMessage("            Procedure = '%s'\n",procInfo->procName);
		}
		cnt++;
	}
	TextMessage("\n");
}
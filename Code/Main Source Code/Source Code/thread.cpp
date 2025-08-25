#include "stdafx.h"
#include "thread.h"
#include <time.h>
#include <vector>
#include "cArg.h"
#include "cache.h"
#include "edit_class.h"
#include "edit_files.h"
#include "edit_utilities.h"
#include "evaluate.h"
#include "files.h"
#include "globals.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include "interface.h"
#include "macro_class.h"
#include "process.h"
#include "scanstrings.h"
#include "variablesOther.h"
#include "memoryLeak.h"

using std::vector;

DWORD WINAPI  RunMacroThread(void *par);
void AddToThreadList(DWORD id);
void RemoveFromThreadList(short err, DWORD id);
bool IsThreadRunning(DWORD id);
int WaitforThread(Interface* itfc ,char arg[]);
int CleanUpThread(Interface* itfc ,char arg[]);

typedef struct
{
   long id;
   HANDLE hdl;
}
THREADINFO;

vector<long> threadAbortIDs;	// Externally visible
vector<HANDLE> threadHandles;	// Externally visible
bool threadAbortPresent = false;	// 
DWORD dwTlsIndex; // Externally visible; Thread local storage for current gui window
vector<DWORD> threadList;  
vector <THREADINFO> threadInfo; 

typedef struct
{
   Interface* itfc;
   CText cmd;
   WinData *guiwin;
}
ThreadArg;

HANDLE thGlo = 0;

/******************************************************************
          Run a procedure in a new thread
			 Return the thread id which can be used for stopping
			 the thread or checking its status
*******************************************************************/

int Thread(Interface* itfc ,char args[])
{
   short nrArgs;
   CText argument;
   CText command;
   Variable result;
   CArg carg;
   long startNr=0;
   DWORD exitCode = 0;
   char *text = NULL;
   char macroPath[MAX_PATH];
   char macroName[MAX_PATH];
   char macroN[MAX_PATH];
   char procName[MAX_PATH];
   Variable *procVar = NULL;
   DWORD id = 0;
   HANDLE th = NULL;
   extern short CheckForMacroProcedure(Interface *itfc, char *command, char *arg);


   macroPath[0] = '\0';
   macroName[0] = '\0';
   procName[0] = '\0';

   nrArgs = carg.Count(args);
	if(nrArgs == 0)
	{
		ErrorMessage("No arguments passed, expecting at least 1");
		return(ERR);
	}
   command = carg.Extract(1);
   if(Evaluate(itfc,RESPECT_ALIAS,command.Str(),&result) == ERR)
      return(ERR);

   if(result.GetType() == UNQUOTED_STRING)
   {
      command = result.GetString();
   }
   else
   {
      ErrorMessage("Invalid command name");
      return(ERR);
   }

   if(itfc->debugging)
   {
      command.RemoveQuotes();
      CText argTxt;
      short r;

      if(nrArgs > 1)
         argTxt = carg.Extract(2);

      for(int i = 3; i <= nrArgs; i++)
      {
         argTxt = argTxt + "," + carg.Extract(i);
      }

      r = CheckForMacroProcedure(itfc, command.Str(), argTxt.Str());

      return(r);
   }
     
// If the current procedure has been originally called from a window object
// then it will be cached already. Alternatively it may be cached in the 
// calling macro if the macro has accessed this function before.
   if(itfc->macro)
   {
   // Determine the macroname and the procedurename
      if(command[0] == ':')
      {
	      strncpy_s(procName,MAX_PATH,command.Str()+1,_TRUNCATE);
	      strncpy_s(macroName,MAX_PATH,itfc->macro->macroName.Str(),_TRUNCATE);
         strncpy_s(macroPath,MAX_PATH,itfc->macro->macroPath.Str(),_TRUNCATE);
      }
      else // macro:proc or macro - path is not known
      {
         strncpy_s(macroName,MAX_PATH,command.Str(),_TRUNCATE);
         ExtractProcedureName(macroName,procName);
         macroPath[0] = '\0';
      }

		if(gCachedProc.next) // Check if this procedure is in the global cache
		{
			strncpy_s(macroN,MAX_PATH,macroName,_TRUNCATE);
         procVar = GetProcedure(macroPath,macroN,procName);
		}

      if(!procVar) // Otherwise check the window and then the macro cache
		{
			if(itfc->win && itfc->win->cacheProc)
			{
				strncpy_s(macroN,MAX_PATH,macroName,_TRUNCATE);
			//   AddExtension(macroN,".mac");
			//   ToLowerCase(macroN);
				procVar = itfc->win->GetProcedure(macroPath,macroN,procName);

				if(procVar)
				 ;//   TextMessage("window cache loaded: %s\n",command);
				else // Not found so try base macro
				{
					strncpy_s(macroN,MAX_PATH,macroName,_TRUNCATE);
				//   AddExtension(macroN,".mac");
			 //     ToLowerCase(macroN);
					procVar = itfc->baseMacro->GetProcedure(macroPath,macroN,procName);
					if(procVar)
					{
					 ;//   TextMessage("macro cache loaded: %s:%s\n",macroN,procName);
					}
					else // Not found so restore path
						strncpy_s(macroPath,MAX_PATH,itfc->macro->macroPath.Str(),_TRUNCATE); 
				}

			}
			else if(itfc->cacheProc && itfc->baseMacro) // See if the procedure has already be cached in the current macro
			{
				strncpy_s(macroN,MAX_PATH,macroName,_TRUNCATE);
			 //  AddExtension(macroN,".mac");
			//   ToLowerCase(macroN);
				procVar = itfc->baseMacro->GetProcedure(macroPath,macroN,procName);
				if(procVar)
				{
				 ;//   TextMessage("macro cache loaded: %s:%s\n",macroN,procName);
				}
				else // Not found so restore path
					strncpy_s(macroPath,MAX_PATH,itfc->macro->macroPath.Str(),_TRUNCATE);  
			}
			else
			  strncpy_s(macroPath,MAX_PATH,itfc->macro->macroPath.Str(),_TRUNCATE);  
		}
	}

   if(procVar) // Yes it has been cached
   {
      text = procVar->GetProcedureText(startNr);
   }
   else // No, must be a new one
   { 
	   if(command[0] == ':') // Command has form ':proc' - so search current macro
	   {    
	   // Load parent macro (either from file or text editor)   	         
	      if(macroPath[0] == '\0' && !strcmp(macroName,"current_text"))
         {
            if(!curEditor)
            {   
               ErrorMessage("No current editor selected");
               return(ERR);
            }
	         text = GetText(curEditor->edWin);
         }
	      else     
         {
	         text = LoadTextFileFromFolder(macroPath, macroName,".mac");
         }
			if(text && FindProcedure(text,procName,startNr) == ERR)
			{
	         delete[] text;		   
			   return(ERR); 
			} 
	   }
	   else // Command has form 'macro' or 'macro:proc' - search for macro in normal path
	   {
	      strncpy_s(macroName,MAX_PATH,command.Str(),_TRUNCATE);
	      ExtractProcedureName(macroName,procName); // Extract macro filename and procedure name (if any)
	      text = LoadTextFile(itfc,macroPath, macroName, procName,".mac",startNr);
	   }

   // Save the macro particulars to the procedure run list menu
      if(text)
         AddFilenameToList(procRunList,macroPath, macroName);

	}
   
// Process the macro text ******************************************
   if(text) 
   {
   // Add extension if necessary
      strncpy_s(macroN,MAX_PATH,macroName,_TRUNCATE);
    //  AddExtension(macroN,".mac");

	   ThreadArg *targ = new ThreadArg;

      targ->itfc = new Interface();

   // Run the macro
      targ->itfc->macroName = macroN;
      targ->itfc->macroPath = macroPath;
      targ->itfc->procName = procName;
      targ->itfc->baseMacro = itfc->baseMacro;
		targ->itfc->startLine = startNr;

      // Evaluate and store any argument list in global "arg" variable
      for(int i = 2; i <= nrArgs; i++)
      {
         argument = carg.Extract(i);

         if(Evaluate(itfc,RESPECT_ALIAS,argument.Str(),&result) < 0) 
         {
            delete targ->itfc;
				delete targ;
            return(ERR); 
         }
         if(CopyVariable(targ->itfc->argVar[i-1].var,&result,RESPECT_ALIAS) == ERR)
         {
            delete targ->itfc;
				delete targ;
            return(ERR); 
         }
      }
      targ->itfc->nrProcArgs = nrArgs-1;
      targ->itfc->name = itfc->name;
      targ->itfc->obj = itfc->obj;
      targ->itfc->objID = itfc->objID;
      targ->itfc->procName = itfc->procName;
      targ->itfc->win = itfc->win;
      targ->itfc->baseMacro = itfc->baseMacro;
//	targ->itfc->macro = itfc->macro;
      targ->guiwin = itfc->win;
      targ->cmd = text;

      th = CreateThread(NULL,0,RunMacroThread,(PVOID)targ,0,&id);
    //  TextMessage("Thread ID %X for %s\n",id, targ->itfc->name);

      THREADINFO info;
      info.hdl = th;
      info.id = id;
      threadInfo.push_back(info);

// Make a note of this thread in a global space
      AddToThreadList(id);

      SetThreadPriority(th,THREAD_PRIORITY_NORMAL);

      GetExitCodeThread(th,&exitCode);

   // Save the macro to the procedure list if not found before        
      if(itfc && itfc->macro)
      {
         // Save to window cache if procedure called from a gui window
         if(itfc->win && itfc->win->cacheProc && !procVar && command[0] != '\0')
         {
            if(command[0] != ':') // Strip off colon if its a local procedure
            {
               strncpy_s(macroName,MAX_PATH,command.Str(),_TRUNCATE);
	            ExtractProcedureName(macroName,procName); // Extract macro filename and procedure name (if any)
            }
            char macroN[MAX_STR];
            strncpy_s(macroN,MAX_PATH,macroName,_TRUNCATE);
          //  AddExtension(macroN,".mac");
         //   ToLowerCase(macroN);
            procVar = itfc->win->procList.Add(PROCEDURE,procName);
	         procVar->MakeAndSetProcedure(text,procName,macroN,macroPath,startNr);
            delete[] text;
	      }
      
      // Save to macro cache if macro not called from a gui window
         else if(itfc->cacheProc && !itfc->win && !procVar && command[0] != '\0')
         {
            if(command[0] != ':') // Strip off colon if its a local procedure
            {
               strncpy_s(macroName,MAX_PATH,command.Str(),_TRUNCATE);
	            ExtractProcedureName(macroName,procName); // Extract macro filename and procedure name (if any)
            }
            char macroN[MAX_STR];
            strncpy_s(macroN,MAX_STR,macroName,_TRUNCATE);
          //  AddExtension(macroN,".mac");
         //   ToLowerCase(macroN);
            procVar = itfc->baseMacro->procList.Add(PROCEDURE,procName);
            char pathm[MAX_PATH];
            strncpy_s(pathm,MAX_PATH,macroPath,_TRUNCATE);
            int len = strlen(pathm);
            if(pathm[len-1] == '\\') // Need this because some directories have backslashs at the end
               pathm[len-1] = '\0';
	         procVar->MakeAndSetProcedure(text,procName,macroN,pathm,startNr);
            delete[] text;
	      } 
      }

   // Free up text if not cached
      if(!procVar)
      {
         delete[] text;
      }
   }
	else
	{
		ErrorMessage("Could not find procedure '%s'",command.Str());
		return(ERR);
	}

// Return thread id 
   itfc->retVar[1].MakeAndSetFloat((float)id);
   itfc->nrRetValues = 1;

   return(OK);
}

/******************************************************************
        Run the macro called by the thread command
*******************************************************************/

DWORD WINAPI  RunMacroThread(void *par)
{
   extern double GetMsTime();

// Intialise thread argument
   ThreadArg* args = (ThreadArg*)par;

// Intialise thread local storage
   ThreadGlobals* data = new ThreadGlobals;
   data->curGUIWin = args->guiwin;
   WinData *win = args->itfc->win;
   data->parentWin = win;
	data->errInfo.errorFound = false;
	data->errInfo.blocked = false;
   if(!TlsSetValue(dwTlsIndex, (void*)data)) 
      return(ERR);

// Initialise random number generator.
   unsigned int seed = (int)GetMsTime();
   srand(seed);

// Run the thread procedure
   if(win) win->threadCnt++;
   short err = ProcessMacroStr(args->itfc, args->cmd.Str());
   if(win) win->threadCnt--;

// Copy any error found in this thread to the global error record
   if(data->errInfo.errorFound)
	{
		if(!gErrorInfo.blocked)
		{
			gErrorInfo.blocked = true;
			gErrorInfo = data->errInfo;
			gErrorInfo.blocked = false;
		}
	}

// Thread has finished so remove from thread list
   RemoveFromThreadList(err,GetCurrentThreadId());

// Clean up thread local storage and arguments
	delete args->itfc;
	delete args;
	delete data;

   return(OK);
}

/******************************************************************
          Wait for thread to finish and then remove
*******************************************************************/

int WaitforThread(Interface* itfc ,char arg[])
{
   short n;
   long threadID;
   HANDLE threadHandle = 0;
   int i;
   
   if((n = ArgScan(itfc,arg,1,"info","e","l",&threadID)) < 0)
      return(n);

   int sz = threadInfo.size();

   for(i = 0; i < sz; i++)
   {
      if(threadInfo[i].id == threadID)
      {
         threadHandle = threadInfo[i].hdl;
         break;
      }
   }

	//MSG lpMsg;

	// Begin the message loop ***************************************
	//while(GetMessage(&lpMsg,NULL,0,0))    
	//{
	//	MainWindowRedraw();
	//	if(!IsThreadRunning(threadID))
	//		break;
	//	TranslateMessage(&lpMsg);
	//	DispatchMessage(&lpMsg);
	//}

	while(IsThreadRunning(threadID))
	{   
      MainWindowRedraw();
	}

   if(threadHandle)
   {
      CloseHandle(threadHandle);
      threadInfo.erase(threadInfo.begin()+i);    
   }

   itfc->nrRetValues = 0;

   return(OK);
}


/******************************************************************
     Delete any outstanding resources associated with a thread
*******************************************************************/

int CleanUpThread(Interface* itfc ,char arg[])
{
   short n;
   long threadID;
   HANDLE threadHandle = 0;
   int i;
   
   if((n = ArgScan(itfc,arg,1,"info","e","l",&threadID)) < 0)
      return(n);

   int sz = threadInfo.size();

   for(i = 0; i < sz; i++)
   {
      if(threadInfo[i].id == threadID)
      {
         threadHandle = threadInfo[i].hdl;
         break;
      }
   }

   if(threadHandle)
   {
      CloseHandle(threadHandle);
      threadInfo.erase(threadInfo.begin()+i);  
      itfc->retVar[1].MakeAndSetFloat(1);
      itfc->nrRetValues = 1;
   }
   else
   {
      itfc->retVar[1].MakeAndSetFloat(0);
      itfc->nrRetValues = 1;
   }

   return(OK);
}
   

/******************************************************************
          Set a flag to abort the specified thread id
			 Return 1 if thread is running and flag has been set
			 Return 0 if thread is not running
*******************************************************************/

int AbortThread(Interface* itfc ,char args[])
{
   short n;
   CText message;
   Variable info;
   long threadID;
   
   if((n = ArgScan(itfc,args,1,"info","e","l",&threadID)) < 0)
      return(n);

   if(IsThreadRunning(threadID)) // Thread is running - set reset flag and add thread id to list
   {
  //     TextMessage("thread active - abort flag set\n");
      threadAbortPresent = true;
		long sz = threadAbortIDs.size();
		int k;
		for(k = 0; k < sz; k++)
		{
			if(threadAbortIDs[k] == threadID)
		      break;
	   }
		if(k == sz)
		{
         threadAbortIDs.push_back(threadID);
		}

	// Wait for the thread to exit
		while(IsThreadRunning(threadID))
		{    
         MainWindowRedraw();
			Sleep(10);
		}

      itfc->retVar[1].MakeAndSetFloat(1.0);
      itfc->nrRetValues = 1;
   }
   else // Thread is not running 
   {
//        TextMessage("thread flag found but nonactive - abort flag reset\n");

      itfc->retVar[1].MakeAndSetFloat(0.0);
      itfc->nrRetValues = 1;
   }
   return(OK);
}


/******************************************************************
          Check on the status of a thread by id
			 Return 1 if running
			 Return 0 if not running
*******************************************************************/

int ThreadStatus(Interface *itfc, char args[])
{
   short n;
   Variable info;
   long threadID;

   if((n = ArgScan(itfc,args,0,"info","e","l",&threadID)) < 0)
      return(n);

   if(n == 0)
   {
        int sz = threadList.size();

        if(sz > 0)
           TextMessage("\n\n");

         for(int i = 0; i < sz; i++)
         {
            TextMessage("Thread %ld running\n",threadList[i]);
         }

        if(sz > 0)
           TextMessage("\n");

         itfc->nrRetValues = 0;
         return(OK);
   }

   itfc->retVar[1].MakeAndSetFloat((float)IsThreadRunning(threadID));
   itfc->nrRetValues = 1;
   return(OK);
}


// Add the current thread id to the thread list

void AddToThreadList(DWORD id)
{
   threadList.push_back(id);
}

// Remove the current thread id from the thread list

void RemoveFromThreadList(short err, DWORD id)
{
   EnterCriticalSection(&csThread);

   int sz = threadList.size();

   for(int i = 0; i < sz; i++)
   {
      if(threadList[i] == id)
      {
         threadList.erase(threadList.begin()+i);
         break;
      }
   }

   sz =  threadList.size();
   LeaveCriticalSection(&csThread);

   if(gAbortMacro)
   {
      if(sz == 0)
      {
         gAbortMacro = false;
   //      TextMessage("Escape message sent\n");
         SendMessageToGUI("Macro,Escape",0);
      }
   }
   else if(err == ABORT) // Send a message for macro to process abort
   {
      if(sz == 0)
      {
         gAbortMacro = false;
    //     TextMessage("Abort message sent\n");
         SendMessageToGUI("Macro,Abort",0);
      }
   }
  // TextMessage("Number of threads = %d\n",sz);

}

// See if a thread is still current

bool IsThreadRunning(DWORD id)
{
   EnterCriticalSection(&csThread);
   int sz = threadList.size();

   for(int i = 0; i < sz; i++)
   {
      if(threadList[i] == id)
      {
         LeaveCriticalSection(&csThread);
         return(true);
      }
   }
   LeaveCriticalSection(&csThread);
   return(false);
}

// See how many threads are running

int CountThreads()
{
   int sz = threadList.size();
 //  TextMessage("Count threads = %d\n",sz);
   return(sz);
}

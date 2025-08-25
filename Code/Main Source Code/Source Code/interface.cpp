#include "stdafx.h"
#include "interface.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "memoryLeak.h"

Interface::Interface() 
{
   name = "";
   macroName = "";
   macroPath = "";
   procName = "";
   obj = NULL;
   win = NULL;
   objID = -1;
   nrRetValues = 0;
   defaultVar = 1;
   nrProcArgs = 0;
   argVar = new ArgVar[MAX_RETURN_VARIABLES+1];
   retVar = new Variable[MAX_RETURN_VARIABLES+1];
   for(int i = 0; i < MAX_RETURN_VARIABLES; i++)
   {
      argVar[i].var = new Variable;
      argVar[i].var->SetSystem(true);
      retVar[i].SetSystem(true);
   }
   namedArgumentMode = ArgMode::NONE_NAMED;
   allocMode = ALLOC_ARG + ALLOC_RET;
   cacheProc = false;
   macro = NULL;
   macroDepth = 0;
   varScope = LOCAL;
   inCLI = false;
   startLine = 0;
   lineNr = 0;
   debugging = false;
   ptr = NULL;
	menuInfo = NULL;
	menuNr = -1;
   processEscapes = true;
	processExpressions = true;
   baseMacro = NULL;
}

Interface::Interface(short mode) 
{
   name = "";
   macroName = "";
   macroPath = "";
   procName = "";
   obj = NULL;
   win = NULL;
   objID = -1;
   nrRetValues = 0;
   defaultVar = 1;
   nrProcArgs = 0;
   argVar = NULL;
   retVar = NULL;
   namedArgumentMode = ArgMode::NONE_NAMED;
   startLine = 0;
   cacheProc = false;
   
   if(mode & ALLOC_ARG)
   {
      argVar = new ArgVar[MAX_RETURN_VARIABLES];
   }
   if(mode & ALLOC_RET)
   {
      retVar = new Variable[MAX_RETURN_VARIABLES];
   }
 
   allocMode = mode;
   macro = NULL;
   varScope = LOCAL;
	menuInfo = NULL;
	menuNr = -1;
   processEscapes = true; 
   processExpressions = true;
   baseMacro = NULL;
}

// Destructor
Interface::~Interface() 
{
   if(allocMode & ALLOC_ARG)
   {
      if(argVar)
      {
         for(int i = 0; i < MAX_RETURN_VARIABLES; i++)
            delete argVar[i].var;
			delete [] argVar;
      }
   }

   if(allocMode & ALLOC_RET)
   {
      if(retVar)
         delete [] retVar;
   }

   int sz = macroStack.size();
   for (int i = 0; i < sz; i++)
   {
      delete macroStack[i];
   }

   macroStack.clear();
}
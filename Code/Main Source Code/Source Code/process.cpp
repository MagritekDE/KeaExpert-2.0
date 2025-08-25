#include "stdafx.h"
#include "process.h"
#include "array.h"
#include "cArg.h"
#include "command.h"
#include "command_other.h"
#include "control.h"
#include "debug.h"
#include "edit_class.h"
#include "edit_files.h"
#include "evaluate.h"
#include "evaluate_complex.h"
#include "globals.h"
#include "guiMakeObjects.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include "interface.h"
#include "macro_class.h"
#include "main.h"
#include <time.h>
#include "scanstrings.h"
#include "string_utilities.h"
#include "structure.h"
#include "thread.h"
#include "variablesOther.h"
#include "memoryLeak.h"

extern HANDLE ghMutex; 
 
CRITICAL_SECTION cs1DPlot;
CRITICAL_SECTION cs2DPlot;
CRITICAL_SECTION csVariable;
CRITICAL_SECTION csAssignVariable;
CRITICAL_SECTION csCache;
CRITICAL_SECTION csThread;

/**********************************************************************
*                 Functions to process macro commands                 *
**********************************************************************/

// Function prototypes
ObjectData* FindActiveObject(HWND hWnd);

// Externally accessible File variables
bool messageSent; // Flag to see if message sent to CLI during macro

// Storage for last error found by any thread
// Used by editor and lasterror command
// Externally accessible
CommandInfo gErrorInfo; 

// Macro abort thread
bool gAbortMacro     = false;         // When true macro should be aborted
bool gBlockWaitCursor = false;        // When false a cursor is shown when macro is running
bool gDebugBlockWaitCursor = false;        // When false a cursor is shown when macro is running
bool gCheckForEvents = false;         // When true should check for window events
bool gShowWaitCursor = false;         // When true application should show a wait cursor.
short gAbortSleepCnt = 0;             // Counter for monitoring different events during macro
bool errorDetected = false;
HCURSOR gResetCursor = LoadCursor(NULL,IDC_ARROW);             // Reset to arrow cursor after running a macro
short errorLevel = 0;
short gNumMacrosRunning = 0;
bool gAbort = false;
bool gShutDown = false;
DWORD currentThread;
bool gErrorFound = false;
int gShowErrorMode = 0;

int ShowErrors(Interface* itfc, char arg[]);
int PrintProcedureStack(Interface* itfc, char arg[]);



/*****************************************************************
*       Take a string and try and process it as a macro           
*                                                               
* Each string is broken into statements by searching for semi-  
* colons or carriage-return line-feeds. Commands are then extracted 
* and processed. Two types of commands are distinguished; expressions
* and assignments
*
* expression: token operand token operand ...
*    token     : function(arg1,arg2,...)
*                variable
*                number
*                cnumber
*                string
*                real_array
*                complex_array
*                list
*
*    function  : alpha + alpha/numeric + alpha/numeric (Max 15 characters)
*        arg       : token
*
*    variable   : alpha + alpha/numeric + alpha/numeric (Max 15 characters)
*    number     : numeric
*    cnumber    : numeric + [numeric i]
*    string     : "alpha/numeric + $expression$ + ... "
*    real_array : [variable/number, variable/number ...], [start:step:end]
*                 [variable/number, ...;variable/number;...] 
*    comp_array : {variable/cnumber, variable/cnumber ...}, {start:step:end}
                  [variable/cnumber, ...;variable/cnumber;...] 
*    list       : [string,string,string, ...]
*
* operand   : +-/*%!&|><
*
* 
* Assignments:
*    variable = expression
* Or
*    (variable1, variable2 ...) = expression
*                                                                 
*                                                                
*****************************************************************/

// High level interfaces to macro processing
short ProcessMacroStr(int scope, WinData* parent, ObjectData *obj, char *data, char *arguments, char* procName, char* macroName, char *macroPath)
{
   Interface itfc;
   short r;

   itfc.macroName = macroName;
   itfc.macroPath = macroPath;
   itfc.procName = procName;
   itfc.win = parent;
   itfc.varScope = scope;
   itfc.obj = obj;

   r = ProcessMacroStr(&itfc, data);


   return(r);
}


short ProcessMacroStr(int scope, char *expression, ...)
{
   va_list ap;
   Interface itfc;
   short r;

// Start of variable argument list
   va_start(ap,expression);

   char *data = vssprintf(expression,ap);

   itfc.varScope = scope;
   r = ProcessMacroStr(&itfc, 0, data);

   va_end(ap);
   delete [] data;

   return(r);
}

// Low level interfaces to macro processing
short ProcessMacroStr(Interface *itfc, char *data)
{
   return(ProcessMacroStr(itfc, false, data));
}

short ProcessMacroStr(Interface *itfc, char *data, char *macroName, char *macroPath)
{
   itfc->macroName = macroName;
   itfc->macroPath = macroPath;
   return(ProcessMacroStr(itfc, false, data));
}

EXPORT short ProcessMacroStr(Interface *itfc, bool test, char *data)
{
	long cnt;
	Macro macro;
	Macro *lastMacro;
	short err = 0;
	short varScope;
   bool inCliBack = itfc->inCLI;

   
// Catch any macro trying to run several threads
//   assert(!(test && macroDepth != 0 && !dialogDisplayed));

	varScope = itfc->varScope;

   if(itfc->macroDepth == 0)
   {      
      gAbort = false;
      gShowWaitCursor = false;
      gCheckForEvents = false;
      if(gAbortMacro && !CountThreads())
         gAbortMacro = false;
      gBlockWaitCursor = false;
      gAbortSleepCnt = 0;
 //     messageSent = false;
      errorDetected = false;
      itfc->baseMacro = &macro;
      itfc->macroStack.clear();
      if(itfc->win)
          SetParentWin(itfc->win); // Need to do this for SendMessageToGUI
      gNumMacrosRunning++;
      gKeepCurrentFocus = false;
      gFocusCnt = 0;
      gErrorFound = false;
   }
 
   itfc->macroDepth++;
   lastMacro = itfc->macro;
   itfc->macro = &macro;
// Make sure previous error command is copied over
	if(lastMacro)
	   itfc->macro->errorCommands = lastMacro->errorCommands;
   itfc->nrRetValues = 0;


   macro.macroPath = itfc->macroPath;
   macro.macroName = itfc->macroName;
   macro.procName  = itfc->procName;
   macro.startLine = itfc->startLine;
   if(lastMacro)
      macro.inTryBlock = lastMacro->inTryBlock;
   else
      macro.inTryBlock = false;

// Ignore null strings
   if(data[0] == '\0')
      goto ex;
         
// Save source of macro (path, file, procedure, object)
 //  macro.src = macroSrc;

   if(itfc->macroDepth > 100)
   {
      ErrorMessage("Macro nesting too deep");
      goto ex;
   }
      
	try
	{
	// Extract macro commands from data string
		macro.addRawMacro(data);           // Save raw data in macro      
		macro.removeComments();            // Makes a copy of the macro without comments      
		macro.removeEOL();                 // Remove \r\n and replace with EOL markers      
		cnt = macro.CountMacroCommands();  // Count number of commands in 'cleaned' macro
      if(cnt == 0)
        goto ex;
	   macro.buildArray(cnt);             // Build an array of commands of this length
	   macro.extractCommands();           // Copy command from data to command array
	   macro.ProcedureCheck();            // check procedure-endproc pairs in macro 	
	   macro.ForNextCheck();              // check for "for and next" in macro 		
	   macro.WhileCheck();                // check for "while and endwhile" in macro 
      macro.IfEndifCheck();              // check for "if, elseif, else and endif"
	   macro.TryCatchCheck();             // check for "try, catch, endtry"
      macro.InitStack();                 // Initialize control stack
   	   
	// Define macro variables
		Command *cmd;
      short cmdNr = 0;
      
   // Extract each macro command and then process it
		while(cmdNr < macro.nrCommands) 
		{
		   macro.jump = false;
       // Get command
			cmd = macro.getCmd(cmdNr);

		//	printf("%s\n",cmd->getCmdStr());

      // Note current line number
		//	itfc->lineNr = cmd->getLineNr() + macro.startLine;
			itfc->lineNr = cmd->getLineNr();

      // Record current command information for error recording
			GetCmdInfo()->command = cmd->getCmdStr();
			GetCmdInfo()->path = macro.macroPath;
			GetCmdInfo()->macro = macro.macroName;
			GetCmdInfo()->procedure = macro.procName;
			GetCmdInfo()->lineNr = itfc->lineNr;
			GetCmdInfo()->errorFound = false;
			GetCmdInfo()->blocked = false;
		
      // Check for debug mode
         if(itfc->debugging)
         {
            if(DebugProcess(itfc,cmd) == ERR)
            {
               gAbort = true;	
               err = ABORT;
               goto ex;
            }
         }

		// Process assignment e.g. a = 23 or (a,b) = func()
			if(cmd->getType() == ASSIGNMENT) 
			{	
            err = ProcessAssignment(itfc,cmd->getCmdLeft(),cmd->getCmdRight());
            if (err == ERR && ((gShowErrorMode == 2) || (gShowErrorMode == 1 && !gErrorFound)))
            {
               gErrorFound = true;
               TextMessage("\n   Error found with command %s = %s\n", cmd->getCmdLeft(), cmd->getCmdRight());
               PrintProcedureStack(itfc, "");
            }

            if((err == ERR || err == THROW) && abortOnError == true)
            {

               if(!gAbort) // Not an abort command
               {	
                  if(errorDetected == false) 
                  {
                     errorLevel = itfc->macroDepth;
                     errorDetected = true; // Note than an error has been found
                  }
                  if(macro.tryNr[cmdNr] == -1 && !macro.inTryBlock)
                  {
                     if(err == THROW)
                     {
                       // if(gErrorInfo.description != "")
                       //    TextMessage("Exception thrown: %s\n",gErrorInfo.description.Str());
                      //  err = ABORT;
                        err = ERR;
                        break;
                     }
                     else
                        macro.ReportMacroError(cmd->getCmdStr(), cmdNr, cmd->getLineNr(), errorLevel,itfc->macroDepth);
                  }
                  else if(macro.tryNr[cmdNr] != -1 && macro.inTryBlock)
                  {
			            cmdNr = macro.tryNr[cmdNr];
                     continue;
                  }	
	            }
               goto ex;
            }
            else if(err == ABORT || err == RETURN_FROM_MACRO) // Abort or return from macro
               break;
		   }	
      // Process isolated command e.g. plot(a)
			else if(cmd->getType() == FUNCTION) 
			{
            err = RunCommand(itfc,cmd->getCmdName(),cmd->getCmdArg(),3); // Run the command 
            if(err == CMD_NOT_FOUND)
               err = ERR;
            if (err == ERR && ((gShowErrorMode == 2) || (gShowErrorMode == 1 && !gErrorFound)))
            {
               gErrorFound = true;
               if(cmd->getCmdRight() == NULL)
                  TextMessage("\n   Error found with command %s\n", cmd->getCmdLeft(), cmd->getCmdRight());
               else
                  TextMessage("\n   Error found with command %s(%s)\n", cmd->getCmdLeft(), cmd->getCmdRight());
               PrintProcedureStack(itfc, "");
            }
            if((err == ERR || err == THROW) && abortOnError == true) // Exit macro on error
            {
               if(!gAbort)
               {	
                  if(errorDetected == false) 
                  {
                     errorLevel = itfc->macroDepth;
                     errorDetected = true;
                  }
                  if(macro.tryNr[cmdNr] == -1 && !macro.inTryBlock) 
                  {
                     if(err == THROW)
                     {
                       // if(gErrorInfo.description != "")
                       //    TextMessage("Exception thrown: %s\n",gErrorInfo.description.Str());
                       // err = ABORT;
                        err = ERR; // This allows multiple levels of throw but doesn't abort.
                        break;
                     } 
                     else
                        macro.ReportMacroError(cmd->getCmdStr(), cmdNr, cmd->getLineNr(), errorLevel,itfc->macroDepth);
                  }
                  else if(macro.tryNr[cmdNr] != -1 && macro.inTryBlock)
                  {
			            cmdNr = macro.tryNr[cmdNr];
                     continue;
                  }
	            }
               break;
            }
            else if(err == ABORT) // Abort from macro
               break;

      // Print out result is there are returned values e.g. cmd: sin(23)
            if(inCliBack && itfc->nrRetValues >= 1)
            {
               static short infinCheck; // Prevent inifinite looping
               CText txt;
					if(cmd->getCmdArg()[0] != '\0')
                  txt.Format("%s(%s)",cmd->getCmdName(),cmd->getCmdArg());
					else
                  txt.Format("%s",cmd->getCmdName());
               if(!infinCheck)
               {
                  infinCheck++;
                  PrintRetVar(itfc,txt.Str());
               }
               infinCheck = 0;
            }

            if(err == RETURN_FROM_MACRO) // Return from macro
               break;
			}
      // It must be an expression  e.g. 2*3+sin(23)
         else if(inCliBack) // If in CLI evaluate and print
         {
            err = PrintString(itfc,cmd->getCmdLeft());

            if((err == ERR || err == THROW) && abortOnError == true)
            {
               if(!gAbort)
               {	
                  if(errorDetected == false) 
                  {
                     errorLevel = itfc->macroDepth;
                     errorDetected = true;
                  }
                  if(macro.tryNr[cmdNr] == -1 && !macro.inTryBlock)
                  {
                     if(err == THROW)
                     {
                      //  if(gErrorInfo.description != "")
                      //     TextMessage("Exception thrown: %s\n",gErrorInfo.description.Str());
                      //  err = ERR;
                        err = ABORT;
                        break;
                     } 
                     else
                        macro.ReportMacroError(cmd->getCmdStr(), cmdNr, cmd->getLineNr(), errorLevel,itfc->macroDepth);
                  }
                  else if(macro.tryNr[cmdNr] != -1 && macro.inTryBlock)
                  {
			            cmdNr = macro.tryNr[cmdNr];
                     continue;
                  }	   
	            }
               goto ex;
            }
            else if(err == ABORT || err == RETURN_FROM_MACRO) // Abort or return from macro
               break;

      //      Variable result;
      //      Evaluate(itfc,FULL_COPY,cmd->getCmdLeft(),&result);
         }
      // Otherwise just evaluate it e.g. obj->func(n)
         else 
         {
            Variable result;
            err = Evaluate(itfc,FULL_COPY,cmd->getCmdLeft(),&result);
            if((err == ERR || err == THROW) && abortOnError == true) // Exit macro on error
            {
               if(!gAbort)
               {	
                  if(errorDetected == false) 
                  {
                     errorLevel = itfc->macroDepth;
                     errorDetected = true;
                  }
                  if(macro.tryNr[cmdNr] == -1 && !macro.inTryBlock) 
                     macro.ReportMacroError(cmd->getCmdStr(), cmdNr, cmd->getLineNr(), errorLevel,itfc->macroDepth);
                  else if(macro.tryNr[cmdNr] != -1 && macro.inTryBlock)
                  {
			            cmdNr = macro.tryNr[cmdNr];
                     continue;
                  }
	            }
               break;
            }
            else if(err == ABORT) // Abort from macro
               break;
         }

      // Goto next command
			if(macro.jump)
			   cmdNr = macro.jumpNr[cmdNr];
			else
			   cmdNr = macro.nextNr[cmdNr];

      // Don't call ProcessBackGroundEvents if this is the last command
      // (in this way single CLI events won't have the window focus changed
      // by accident).
         if(cmdNr < macro.nrCommands)
         {
         // Make sure background tasks are processed
            if(ProcessBackgroundEvents() != OK)
            {
            //   macro.ReportMacroAbort(cmd->getCmdStr(), cmdNr, cmd->getLineNr());
               gAbort = true;	
			      err = ABORT;
			      goto ex;
            }
         }
		}
	}

 // Error messages
	catch(const char* errStr)
	{
	//   gAbort = true;
      ErrorMessage(errStr);
	   err = -1;
   }

 // Update the macro stack
ex:   int sz = itfc->macroStack.size();
   if (sz > 0)
   {
    //  TextMessage("Deleting macro stack entry: '%s:%s' %d\n", itfc->macroStack[sz - 1]->macroName.Str(), itfc->macroStack[sz - 1]->procName.Str(), itfc->macroStack[sz - 1]->lineNr);
      delete itfc->macroStack[sz - 1];
      itfc->macroStack.pop_back();
   }
   itfc->macroDepth--;
   itfc->inCLI = inCliBack;

// Come here if the macro stack is exiting
   if(itfc->macroDepth == 0)
   {
// Run the error procedure when exiting this macro
		if(err == -1 && macro.errorCommands.Size() > 0)
		{
			CText cmds = macro.errorCommands;
	      if(lastMacro)
	        lastMacro->errorCommands = "";
			macro.errorCommands = "";
			itfc->macro->errorCommands = "";
			ProcessMacroStr(itfc,cmds.Str());
		}
      gErrorFound = false;


      if(CountThreads() == 0)
      {
			if(gAbortMacro)
			{
				gAbortMacro = false;
				SendMessageToGUI("Macro,Escape",0);
				
			}
			else if(err == ABORT) // Send a message for macro to process abort
			{
				gAbortMacro = false;
				SendMessageToGUI("Macro,Abort",0);
			}
		}

	// If in main thread then copy any error found to the global error record
		if(GetCurrentThreadId() == mainThreadID)
		{
         CommandInfo *errInfo = GetErrorInfo();
			if(errInfo->errorFound)
			{
				if(!gErrorInfo.blocked)
				{
					gErrorInfo.blocked = true;
					gErrorInfo = *errInfo;
					gErrorInfo.blocked = false;
				}
				errInfo->errorFound = false;
			}
		}

 	   gAbort = false;          // Reset abort flag
      itfc->baseMacro = NULL;
      gShowWaitCursor = false;
      SetCursor(gResetCursor);
 	   if(messageSent || macro.nrCommands == 0 || itfc->inCLI)
 	   {
			if(GetCurrentThreadId() == mainThreadID)
			{
 				TextMessage("\n"); // Need to do this separately 
 				TextMessage("> "); // to make sure we scroll to start of line        
 				messageSent = false;
			}
 	   }
      // Ensure that the GUI window with focus is the current window
      // (user may have clicked on another window while macro was running)
      {
         HWND hWnd = GetFocus();
         WinData *win;
         if(win = GetWinDataClass(hWnd))
         {
            if(win != GetGUIWin())
            {
                WinData::SetGUIWin(win);  
            }
         }
         gKeepCurrentFocus = false; // HTML window must have loaded by now so reset this flag
      }

      gNumMacrosRunning--;

      if(gNumMacrosRunning == 0 && gDebug.mode != "off" && gDebug.win && !gDebug.enabled)
      {
         gDebug.enabled = true;
         SendMessageToGUI("debug,waiting",gDebug.win->nr); 
         gDebug.message = "waiting";
         gDebug.enabled = false;
      }
 	}

// Restore current macro
   macroArgs = 0;

// Make sure previous error command takes on new one
	if(lastMacro)
	   lastMacro->errorCommands = itfc->macro->errorCommands;
   itfc->macro = lastMacro;
 	if(err == RETURN_FROM_MACRO) err = 0;
   itfc->varScope = varScope;

 	return(err);
}


/****************************************************************
  Process a command of the form variable = expression           
  Where 'variableStr' is the variable and 'expressionStr' 
  is the expression.  
*****************************************************************/

short ProcessAssignment(Interface *itfc, char *variableStr,char *expressionStr)
{
   short ret;
   CArg arg;
   short err;
   Variable result;

// Evaluate expression and store result in result
   if((err = Evaluate(itfc, RESPECT_ALIAS, expressionStr, &result)) < 0)
       return(err);

// Check to see if variableStr contains a list of variables e.g. (a,b) = func(n)
   if(variableStr[0] == '(')
   {
		int argNr = 0;
      short i = 0;

// This variable gets modified sometimes so make a local copy
      short nrRetValues = itfc->nrRetValues;  
      char* subVar = new char[strlen(variableStr)+1];
      char* argN = new char[strlen(variableStr)+1]; 


   	if(ExtractSubExpression(variableStr,subVar,i,'(',')') < 0)
      {
         delete [] subVar;
         delete [] argN;
	      return(ERR);
      }

      do
      {
         err = arg.GetNext(subVar,argN);		   
         if(argN[0] == '\0')
         {
            delete [] subVar;
            delete [] argN;
            ErrorMessage("null variable in returned parameter list");
            return(ERR);
         }  
         if(++argNr > nrRetValues)
         {
            delete [] subVar;
            delete [] argN;
            ErrorMessage("more variables specified than returned");
            return(ERR);
         }  
         if(itfc->varScope == LOCAL && itfc->retVar[argNr].GetScope() == WINDOW)
            ret = AssignToExpression(itfc, WINDOW, argN, &itfc->retVar[argNr],false);
         else
            ret = AssignToExpression(itfc,itfc->varScope, argN, &itfc->retVar[argNr],false);

         if(ret == ERR)
         {
            delete [] subVar;
            delete [] argN;
            ErrorMessage("Invalid expression in returned parameter list");
            return(ERR);
         } 

         itfc->retVar[argNr].FreeData();
      }
      while(err != FINISH && ret == OK);
	   
      delete [] subVar;
      delete [] argN;
	}
// Otherwise just a simple single assignment e.g. a = func(n)
	else 
	{	 
      if(itfc->nrRetValues > 1) // Multiple returned so assign default to result and release the rest
      {                  
         result.Assign(&itfc->retVar[itfc->defaultVar]);
         itfc->retVar[itfc->defaultVar].NullData();
         for(int i = 1; i <= itfc->nrRetValues; i++)
            itfc->retVar[i].FreeData();     
      }
      else if(itfc->nrRetValues == 0) // No variables returned so signal an error
      {
		   ErrorMessage("no variables returned by this command");
		   return(ERR);
		}

   //   ret = AssignToExpression(itfc, itfc->varScope, variableStr, &result, true);
  //   Perform the assignment
      if(itfc->varScope == LOCAL && result.GetScope() == WINDOW)
         ret = AssignToExpression(itfc, WINDOW, variableStr, &result, true);
      else
         ret = AssignToExpression(itfc, itfc->varScope, variableStr, &result, true);
    //  itfc->retVar[1].FreeData();

   }
   itfc->nrRetValues = 0;
   itfc->defaultVar = 1;

   return(ret);
}

/***********************************************************
   Concatenate str and argNr and place in dst
   Fast conversion for argNr < 10
***********************************************************/

void ConcatStrAndInt(char *dst, char *str, int argNr)
{
   if(argNr < 10)
   {
		long i = 0;
      for(i = 0; str[i] != '\0'; i++)
         dst[i] = str[i];
      dst[i++] = (char)((int)'0' + argNr);
      dst[i] = '\0';
   }
   else
      sprintf(dst,"%s%d",str,argNr);
}



/********************************************************************************
* Assign rhs to lhs when rhs includes a structure operator after the first operand
*
* lhs = rhs 
*
* The lhs is just a string and may be of the following forms
*
* varName1->varName2-> ... = rhs
* funcName1()->varName1-> ... = rhs
*
* where varNamex can be a class or a structure.
*
*********************************************************************************/

int AssignToStructureMember(Interface *itfc, char* lhs, Variable *rhsVar)
{
   short type;
	short nrArgs;
   extern short GetNextOperandCTB(short &pos, char str[], CText &operand);

   short i = 0;
   Variable *var = 0;
   Variable *parent = 0;
   CText operand;
   unsigned char op = '\0';
   short result;
   short iStart;
   bool firstTime = true;
   extern bool IsValidVariableName(char *str);

// Search through the LHS looking for the final variable in the lhs->xxx sequence 
   while(1)
   {
      iStart = i;

      // The first element can be a function which returns a class or a class or a structure
      // subsequent elements can only be classes or structures
      if(firstTime) 
      {
         if((type = GetNextOperandCTB(i,lhs,operand)) == ERR)
            return(ERR);  

         if(lhs[i] == '(') // Is it a function which returns a class or a structure?
         {
            // Extract the argument
            int sz = strlen(lhs);
            char* arg = new char[sz+1];
            int j;
            for(j = i+1; j < sz; j++)
            {
               if(lhs[j] == ')')
                  break;
               arg[j-i-1] = lhs[j];
            }
            arg[j-i-1] = '\0';

            // Evaluate the function
            if(RunCommand(itfc,operand.Str(),arg,3) == ERR)
            {
               delete [] arg;
               return(ERR);
            }
            delete [] arg;

            // Check the return value
            if(itfc->nrRetValues == 1)
            {
               var = &(itfc->retVar[1]);
               if((var->GetType() != CLASS) && (var->GetType() != STRUCTURE) && (var->GetType() != STRUCTURE_ARRAY) )
               {
                  ErrorMessage("Expecting '%s' to return a class, structure or structure array",operand.Str());
                  return(ERR);
               }
            }
            i = j+1;
            result = GetNextOperator(i,lhs,op);
         }
         else // Either a class or a structure
         {
            var = GetVariable(itfc,ALL_VAR,operand.Str(),type);
            result = GetNextOperator(i,lhs,op);
         }
         
         if(!var) // If variable does not exist then make it
         {
            if(IsValidVariableName(operand.Str()))
            {
               var = AddVariable(itfc,itfc->varScope,NULL_VARIABLE,operand.Str());
            }
            else
            {
               ErrorMessage("Invalid variable name '%s'",operand.Str());
               return(ERR);
            }
         }
         firstTime = false;
      }

      if(op == '&') // Reached the end
         break;

    // Now evaluate the structure or class reference
      if(op == 134) // LHS -> RHS
      {
         // If LHS is a class then evaluate the class procedure/member
         if(var->GetType() == CLASS) 
         {
            ClassData *cData = (ClassData*)var->GetData();

            if(CheckClassValidity(cData,true) == ERR)
               return(ERR);

               GetNextOperandCTB(i,lhs,operand);

               if(ProcessClassProcedure(itfc, cData, operand.Str(), "") == ERR)
                  return(ERR);

           // Extract the result of this evaluation
            if(itfc->nrRetValues == 1)
            {
               var = &(itfc->retVar[1]);
               if(var->GetType() == CLASS) 
               {
                  result = GetNextOperator(i,lhs,op);
                  continue;
               }
               else if((var->GetType() != STRUCTURE) && (var->GetType() != STRUCTURE_ARRAY))
               {
                  ErrorMessage("Expecting '%s' to return a class or structure",operand.Str());
                  return(ERR);
               }
            }
            result = GetNextOperator(i,lhs,op);
         }
         // If the LHS is not a structure then its a fault
         else if((var->GetType() != STRUCTURE) && (var->GetType() != STRUCTURE_ARRAY))
         {
				ErrorMessage("Variable '%s' should be a structure",var->GetName());
            return(ERR);
         }

         // If the RHS is a structure then find the member
         if(var->GetType() == STRUCTURE)
         {
            GetNextOperandCTB(i,lhs,operand); // Get the member
 
         // Get the structure
            Variable *strucData = var->GetStruct();
         // Find the member
            Variable *member;
            for(member = strucData->next; member != NULL; member = member->next)
            {
               if(!strcmp(member->GetName(),operand.Str())) // Old member found
               {
                  break;
               }
            }
            if(!member) // New member variable
            {
               if(IsValidVariableName(operand.Str()))
               {
                  var = strucData->Add(NULL_VARIABLE,operand.Str());
               }
               else
               {
                  ErrorMessage("Invalid variable name '%s'",operand.Str());
                  return(ERR);
               }
            }
            else
				{
               var = member;
				}
         }
			if(lhs[i] == '[') // Is it an array?
			{
            short r = AssignArray(itfc, lhs, i, operand.Str(), var, var->GetType(), rhsVar);
				return(OK);
			}
         result = GetNextOperator(i,lhs,op);
         if(result == -1)
         {
            ErrorMessage("Invalid variable type '%s'",operand.Str());
            return(ERR);
         }
      }
      else
		{
         var = GetVariable(itfc,ALL_VAR,operand.Str(),type);
			if(!var)
			{ 
            ErrorMessage("Variable '%s' not found ",operand.Str());
            return(ERR);
         }
		}
   }
   // Make the assignment LHS = RHS
   if(var)
   {
      if(CopyVariable(var,rhsVar,FULL_COPY) == ERR)
         return(ERR);
   }

   return(OK);
}

//int AssignToStructureMember(Interface *itfc, char* lhs, Variable *rhsVar)
//{
//   short type;
//	short nrArgs;
//   extern short GetNextOperandCTB(short &pos, char str[], CText &operand);
//
//   short i = 0;
//   Variable *var = 0;
//   Variable *parent = 0;
//   CText operand;
//   unsigned char op;
//   bool firstTime = 1;
//   short result;
//   short iStart;
//   extern bool IsValidVariableName(char *str);
//
//// Search through the LHS looking for the final variable in the lhs->xxx sequence 
//   while(1)
//   {
//      iStart = i;
//
//      if(firstTime) // 
//      {
//         if((type = GetNextOperandCTB(i,lhs,operand)) == ERR)
//            return(ERR);  
//
//         if(lhs[i] == '(')
//         {
//            int sz = strlen(lhs);
//            char* arg = new char[sz+1];
//
//            int j;
//            for(j = i+1; j < sz; j++)
//            {
//               if(lhs[j] == ')')
//                  break;
//               arg[j-i-1] = lhs[j];
//            }
//            arg[j-i-1] = '\0';
//            if(RunCommand(itfc,operand.Str(),arg,3) == ERR)
//            {
//               delete [] arg;
//               return(ERR);
//            }
//            delete [] arg;
//
//            if(itfc->nrRetValues == 1)
//            {
//               var = &(itfc->retVar[1]);
//               if(var->GetType() != CLASS)
//               {
//                  ErrorMessage("Expecting '%s' to return a class",operand.Str());
//                  return(ERR);
//               }
//            }
//            i = j+1;
//            result = GetNextOperator(i,lhs,op);
//         }
//         else
//         {
//            result = GetNextOperator(i,lhs,op);
//            var = GetVariable(itfc,ALL_VAR,operand.Str(),type);
//         }
//         
//         if(!var) // If variable does not exist then make it
//         {
//            if(IsValidVariableName(operand.Str()))
//            {
//               var = AddVariable(itfc,itfc->varScope,NULL_VARIABLE,operand.Str());
//            }
//            else
//            {
//               ErrorMessage("Invalid variable name '%s'",operand.Str());
//               return(ERR);
//            }
//         }
//         firstTime = 0;
//      }
//
//      if(op == '&') // Reached the end
//         break;
//
//      if(op == 134) // -> operator
//      {
//         if(var->GetType() == CLASS)
//         {
//            ClassData *cData = (ClassData*)var->GetData();
//
//            if(CheckClassValidity(cData,true) == ERR)
//               return(ERR);
//
//               GetNextOperandCTB(i,lhs,operand);
//
//               if(ProcessClassProcedure(itfc, cData, operand.Str(), "") == ERR)
//                  return(ERR);
//
//            if(itfc->nrRetValues == 1)
//            {
//               var = &(itfc->retVar[1]);
//               if(var->GetType() != STRUCTURE)
//               {
//                  result = GetNextOperator(i,lhs,op);
//                  continue;
//                //  ErrorMessage("Expecting '%s' to return or be a structure",operand.Str());
//                //  return(ERR);
//               }
//            }
//            result = GetNextOperator(i,lhs,op);
//         }
//         else if(var->GetType() != STRUCTURE)
//         {
//            var->RemoveAll();
//            var->MakeStruct();
//         }
//
//         if(var->GetType() == STRUCTURE)
//         {
//            GetNextOperandCTB(i,lhs,operand);
// 
//      // Get the structure
//            Variable *strucData = var->GetStruct();
//         // Find the member
//            Variable *member;
//            for(member = strucData->next; member != NULL; member = member->next)
//            {
//               if(!strcmp(member->GetName(),operand.Str())) // Old member found
//               {
//                  break;
//               }
//            }
//            if(!member) // New variable
//            {
//               if(IsValidVariableName(operand.Str()))
//               {
//                  var = strucData->Add(NULL_VARIABLE,operand.Str());
//               }
//               else
//               {
//                  ErrorMessage("Invalid variable name '%s'",operand.Str());
//                  return(ERR);
//               }
//            }
//            else
//               var = member;
//         // Is there another operator?
//            if(var->GetType() != STRUCTURE_ARRAY)
//               result = GetNextOperator(i,lhs,op);
//         }
//      }
//      else
//         var = GetVariable(itfc,ALL_VAR,operand.Str(),type);
//
//   }
//   if(var)
//   {
//      if(CopyVariable(var,rhsVar,FULL_COPY) == ERR)
//         return(ERR);
//   }
//
//   return(OK);
//}


short AssignToExpression(Interface *itfc, short scope, char *varName, Variable *expVar, bool aliasIt)
{
   short type;
   CText operand;
	short i;

// Check to see that 'varStr' is  a valid variable name ***********  
   if((!strcmp(varName,"i")) || (!strcmp(varName,"j")))
   {
      ErrorMessage("Can't use 'i' or 'j' as variable names");
      return(ERR);
   }
   else if(!strcmp(varName,"global") || !strcmp(varName,"glo"))
   {
      ErrorMessage("Can't use 'global' as a variable name");
      return(ERR);
   }
   else if(!strcmp(varName,"local") || !strcmp(varName,"loc"))
   {
      ErrorMessage("Can't use 'local' as a variable name");
      return(ERR);
   }
   else if(!strcmp(varName,"hidden") || !strcmp(varName,"hid"))
   {
      ErrorMessage("Can't use 'hidden' as a variable name");
      return(ERR);
   }
   else if(!strcmp(varName,"winvar"))
   {
      ErrorMessage("Can't use 'winvar' as a variable name");
      return(ERR);
   }
      
// Extract variable name from 'varStr' - i then points to next character after variable  
   if(precedence[varName[0]] != 0)
   {
      ErrorMessage("'%s' is not a valid variable name",varName);
      return(ERR);
   }

   i = 0;				   
   if((type = GetNextOperandCT(i,varName,operand)) == ERR)
      return(ERR);  

   if(type == STRUCTMEMBER)
   {
      if(AssignToStructureMember(itfc, varName,expVar) == ERR)
    //  if(AssignStructureMember(itfc, varName,expVar) == ERR)
        return(ERR);
   }

// Is varName a valid variable name e.g. 12a = expStr *****   
   else if(type != UNQUOTED_STRING) // Invalid name
   {
      ErrorMessage("'%s' is not a valid variable name",varName);
      return(ERR);
   }

// Is varStr an array? e.g. v[23] = expStr ***********
   else if(varName[i] == '[') // Modified V2.2.4
   {
      Variable srcVar;
     
     // Make a local copy of expVar 
      srcVar.Assign(expVar);
      srcVar.SetPermanent(false);
      srcVar.next = NULL;
      srcVar.last = NULL;

      expVar->NullData();
      expVar->SetAlias(NULL);
      
      if(srcVar.GetType() == NULL_VARIABLE) // 2.2.9
      {
         ErrorMessage("can't assign a null variable to a matrix element");
         return(ERR);
      }

      short r = AssignArray(itfc, varName, i, operand.Str(), &srcVar);

      return(r);
   }
   
// Is varStr a procedure? e.g. v(a) = expStr **********
   else if(varName[i] == '(') // It's a procedure call!
   {
      if(AssignToStructureMember(itfc, varName,expVar) == ERR)

     // ErrorMessage("can't assign to a procedure");
      return(ERR);
   }
   
// If none of the above then varName is just an ordinary variable e.g. v = expStr   
   else 
   {   
      if(strlen(varName) != i)
      {
         ErrorMessage("'%s' is not a valid variable name",varName);
         return(ERR);
      }

      Variable *var = AddVariable(itfc,scope,expVar->GetType(),varName);

      if(var->GetScope() == WINDOW || var->GetScope() == GLOBAL)
         EnterCriticalSection(&csAssignVariable);
      //   EnterCriticalSection(&cs1DPlot);

      if(var->GetReadOnly())
      {
         ErrorMessage("variable '%s' is read only",varName);
         goto err;
      }

   // Avoid circular aliases
      if(expVar->GetAlias() == var)
      {
         ErrorMessage("variable '%s' is aliased to itself",varName);
         goto err;

      }
 
   // Make sure we can copy ansVar to var (maybe it is an alias)                 
      if(var->GetAlias() && var->GetAliasCnt() > 0)
      {
         ErrorMessage("variable '%s' is aliased to another variable and cannot be deleted",varName);
         goto err;

      }

   // Free any data in var first
		int scope = var->GetScope();
      if(var->FreeData() == ERR)
      {
         goto err;
      }
		var->SetScope(scope);
         
   // Let 'var' absorb the data in expVar
      var->Assign(expVar);
      expVar->SetData(NULL);
 
     if(var->GetScope() == WINDOW || var->GetScope() == GLOBAL)
      //  LeaveCriticalSection(&cs1DPlot);
        LeaveCriticalSection(&csAssignVariable);
	  
     return(OK);

err:

     if(var->GetScope() == WINDOW || var->GetScope() == GLOBAL)
        // LeaveCriticalSection(&cs1DPlot);
         LeaveCriticalSection(&csAssignVariable);

     return(ERR);
    
	}
   return(OK);


}

/********************************************************************************
  Assign variable or expression to global, window or local variable

  Syntax: assign(variable, expression, scope)

  variable ..... name of variable (may be expression)
  expression ... expression to assign
  scope ........ global, local or window

*********************************************************************************/
   
bool IsValidVariableName(char *str);

int AssignSpecialVariable(Interface* itfc ,char args[])
{
   short n;
   CText varExpression;
   CText scope;
   Variable var;
	itfc->nrRetValues = 0;
 
// Extract variable name, variable value and variable scope
	if((n = ArgScan(itfc,args,2,"variable, expression, [scope]","eee","tvt",&varExpression,&var,&scope)) < 0)
	   return(n);

// Check to see if the variable name is valid
   //if(!IsValidVariableName(varExpression.Str()))
   //{
   //   ErrorMessage("invalid variable name '%s'",varExpression.Str());
   //   return(ERR);
   //}

// Scope has been specified
   if(n == 3)
   {
      if(scope == "global")
	      return(AssignToExpression(itfc,GLOBAL,varExpression.Str(),&var,true));
      else if(scope == "window")
      {
         if(GetGUIWin() == NULL)
         {
            ErrorMessage("no GUI window present");
            return(ERR);
         }
	      return(AssignToExpression(itfc,WINDOW,varExpression.Str(),&var,true));
      }
      else if(scope == "local")
	      return(AssignToExpression(itfc,LOCAL,varExpression.Str(),&var,true));
      else
      {
         ErrorMessage("unrecognized scope '%s'",scope);
         return(ERR);
      }
	}
// Just use current scope
	else
	{
	   return(AssignToExpression(itfc,itfc->varScope,varExpression.Str(),&var,true));
	}
}



/********************************************************************************
  Assign variable or expression to global, window or local variable

  Syntax: assign(variable, expression, scope)

  variable ..... name of variable (may be expression)
  expression ... expression to assign
  scope ........ global, local or window

*********************************************************************************/
   

int AssignWithLock(Interface* itfc ,char args[])
{
   short n;
   CText varExpression;
   CText scope;
   Variable var;
	short r;
	itfc->nrRetValues = 0;
 
	//DWORD  dwWaitResult = WaitForSingleObject( ghMutex, INFINITE); 
	EnterCriticalSection(&csAssignVariable);

// Extract variable name, variable value and variable scope
	if((n = ArgScan(itfc,args,2,"variable, expression, [scope]","eee","tvt",&varExpression,&var,&scope)) < 0)
	{
		LeaveCriticalSection(&csAssignVariable);
		//ReleaseMutex(ghMutex);
	   return(n);
	}

// Scope has been specified
   if(n == 3)
   {
      if(scope == "global")
		{
	      r = AssignToExpression(itfc,GLOBAL,varExpression.Str(),&var,true);
		   LeaveCriticalSection(&csAssignVariable);
		 //  ReleaseMutex(ghMutex);
			return(r);
	   }
      else if(scope == "window")
      {
         if(GetGUIWin() == NULL)
         {
				LeaveCriticalSection(&csAssignVariable);
            ErrorMessage("no GUI window present");
            return(ERR);
         }
	      r = AssignToExpression(itfc,WINDOW,varExpression.Str(),&var,true);
		   LeaveCriticalSection(&csAssignVariable);
		 //  ReleaseMutex(ghMutex);
			return(r);
      }
      else if(scope == "local")
		{
	      r = AssignToExpression(itfc,LOCAL,varExpression.Str(),&var,true);
		   LeaveCriticalSection(&csAssignVariable);
		 //  ReleaseMutex(ghMutex);
			return(r);
      }
      else
      {
			LeaveCriticalSection(&csAssignVariable);
         ErrorMessage("unrecognized scope '%s'",scope);
         return(ERR);
      }
	}
// Just use current scope
	else
	{
	   r = AssignToExpression(itfc,itfc->varScope,varExpression.Str(),&var,true);
		LeaveCriticalSection(&csAssignVariable);
		//ReleaseMutex(ghMutex);
		return(r);
	}
	// ReleaseMutex(ghMutex);
	LeaveCriticalSection(&csAssignVariable);

}


int OnError(Interface* itfc ,char args[])
{
	CText cmd;
	int n;

	if((n = ArgScan(itfc,args,1,"commands","e","t",&cmd)) < 0)
	   return(n);

	itfc->macro->errorCommands = cmd;
	itfc->nrRetValues = 0;
	return(OK);
}


// Return the location of the current macro
int GetMacroPath(Interface* itfc ,char args[])
{
   CText path;
	CText macroName;
	int n;
	FILE *fp;

	if((n = ArgScan(itfc,args,0,"macro","e","t",&macroName)) < 0)
	   return(n);

	if(n == 0)
	{
		short len = itfc->macro->macroPath.Size();
		path = itfc->macro->macroPath;
		if(path.Str()[len-1] == '\\')
			path.Str()[len-1] = '\0';
		itfc->retVar[1].MakeAndSetString(path.Str());
		itfc->nrRetValues = 1;
	}
	else
	{
		char filePath[MAX_PATH] = "";
		char fileName[MAX_PATH] = "";
		strcpy(fileName,macroName.Str());
		if(fp = FindFolder(itfc,filePath,fileName,".mac"))
		{
			fclose(fp);
			itfc->retVar[1].MakeAndSetString(filePath);
			itfc->nrRetValues = 1;
		}
		else
		{
			itfc->retVar[1].MakeNullVar();
			itfc->nrRetValues = 1;
		}
	}
   return(OK);
}

// Return the name of the current macro
int GetMacroName(Interface* itfc ,char args[])
{
   itfc->retVar[1].MakeAndSetString(itfc->macro->macroName.Str());
   itfc->nrRetValues = 1;
   return(OK);
}


bool IsValidVariableName(char *str)
{
   short i = 0;	
   CText operand;
   short type;
   short len = strlen(str);

   if(precedence[str[0]] != 0)
      return(false);

   if((type = GetNextOperandCT(i,str,operand)) == ERR)
      return(ERR);  

   if(type != UNQUOTED_STRING || i != len) // Invalid name
   {
      return(false);
   }

   for(i = 0; i < len; i++)
   {
      if(str[i] == ':') return(false);
   }

   return(true);
}

/*********************************************************************************
   Extract the nth argument from the output of a command or procedure
**********************************************************************************/
//TODO needs work to remove ansVar
int GetReturnValue(Interface* itfc, char args[])
{
   CText expression;
   short n;
   short r;
   Variable result;

   if((r = ArgScan(itfc,args,2,"Command/procedure, argument (1-based)","ce","td",&expression,&n)) < 0)
      return(r);

   r = Evaluate(itfc,RESPECT_ALIAS,expression.Str(),&result);
   if(r == ERR) return(ERR);

   if(n < 1 || n > itfc->nrRetValues)
   {
      ErrorMessage("no variables returned with index '%hd'",n);
      return(ERR);
   }

   if(itfc->nrRetValues > 0 && n == 1)
   {
       itfc->nrRetValues = 1;
   }
   else if(itfc->nrRetValues > 0 && n > 1)
   {
      if(CopyVariable(&itfc->retVar[1],&itfc->retVar[n],FULL_COPY) == ERR)
          return(ERR);
       itfc->nrRetValues = 1;
   }
   else
   {
      ErrorMessage("No variables returned by function '%s'",expression);
      return(ERR);
   }
   return(OK);
}


/*********************************************************************************
   Return the error information as a structure
**********************************************************************************/

int GetLastError(Interface* itfc ,char args[])
{
   Variable *struc,*memberVar;

	itfc->retVar[1].MakeStruct();
	struc = itfc->retVar[1].GetStruct();
    
	memberVar = struc->Add(STRUCTURE,"linenr");
	memberVar->MakeAndSetFloat(gErrorInfo.lineNr+1);
	memberVar = struc->Add(STRUCTURE,"procedure");
	memberVar->MakeAndSetString(gErrorInfo.procedure.Str());
	memberVar = struc->Add(STRUCTURE,"macro");
	memberVar->MakeAndSetString(gErrorInfo.macro.Str());
	memberVar = struc->Add(STRUCTURE,"path");
	memberVar->MakeAndSetString(gErrorInfo.path.Str());
	memberVar = struc->Add(STRUCTURE,"line");
	memberVar->MakeAndSetString(gErrorInfo.command.Str());
   memberVar = struc->Add(STRUCTURE,"type");
	memberVar->MakeAndSetString(gErrorInfo.type.Str());
	memberVar = struc->Add(STRUCTURE,"description");
	memberVar->MakeAndSetString(gErrorInfo.description.Str());

   itfc->nrRetValues = 1;
   return(OK);
}


/*************************************************************************************
   Sit in an endless loop checking for aborts, wait cursor
   display or window events which occur during macros
   The variables gShowWaitCursor and gCheckForEvents and gAbortMacro are reset at
   the start of each macro.
**************************************************************************************/

DWORD WINAPI AbortMacro(PVOID par)
{
   long last = -1;
   short r;

// Wait until escape key pressed or macro aborted
   while(1)
   {

      if(!dialogDisplayed)
      {
         r = GetAsyncKeyState(VK_ESCAPE);

      // If user has clicked on the escape button stop the macro
      // but only if the GUI window is NOT visible
         if(last == 0 && (r & 0x08000) && !(GetAsyncKeyState(VK_SHIFT) & 0x08000) && !gAbortMacro)
         {
            if(!GetGUIWin() && IsAProspaWindow(GetForegroundWindow())) // Only abort if esc pressed while prospa is in front
            {
               gAbortMacro = true; 
            }
         }

         if(++gAbortSleepCnt == 50) // Show cursor after 500 ms
         {
             gShowWaitCursor = true;
         }

         if(gAbortSleepCnt%5 == 0) // Only check for button abort every 50 ms
         {
             gCheckForEvents = true;
         }
         last = r;
      }

      Sleep(10); // Give Prospa some time to work

      if(gShutDown) // Program is exiting
         break;


   }
   return(0);
}

bool keyBreakLoop = false;

int KeyPressed(Interface* itfc ,char args[])
{
   CText key,action,cmd;
   short nrArgs;

// Get filename from user *************  
   if((nrArgs = ArgScan(itfc,args,1,"key, action, [cmd]]","eee","ttt",&key,&action,&cmd)) < 0)
     return(nrArgs);

// Used when this command is run as a separate thread
// to stop checking for escape pressed
   if(key == "stopchecking")
   {
      keyBreakLoop = true;
      itfc->retVar[1].MakeAndSetString("stopchecking");
      itfc->nrRetValues = 1;
      return(OK);
   }

   keyBreakLoop = false;

   while(1)
   {
      if(keyBreakLoop)
      {
         itfc->retVar[1].MakeAndSetString("stopped");
         itfc->nrRetValues = 1;
         return(OK);
      }
      if((GetAsyncKeyState(VK_ESCAPE) & 0x08000) && !(GetAsyncKeyState(VK_SHIFT) & 0x08000))
      {
         itfc->retVar[1].MakeAndSetString("keypressed");
         itfc->nrRetValues = 1;
         return(OK);
      }
      Sleep(10); // Give Prospa some time to work
   }

   return(OK);
}

/*************************************************************************************
   Check to see if the user is aborting the command or if the windows need redrawing
**************************************************************************************/

EXPORT short ProcessBackgroundEvents()
{
   if(threadAbortPresent)
   {
		int k;
		long id = GetCurrentThreadId();
      long sz = threadAbortIDs.size();
  //     TextMessage("Checing for abort %ld\n",sz);
		for(k = 0; k < sz; k++)
		{
			if(threadAbortIDs[k] == id)
		      break;
	   }

		if(k < sz)
		{

   //    TextMessage("Aborting thread %ld ...\n",id);

         threadAbortIDs.erase(threadAbortIDs.begin()+k);

         sz = threadAbortIDs.size();
			if(sz == 0)
			{
				threadAbortPresent = false;
			}
	
         gAbort = true;	
         return(ABORT);
      }
   }

   if(gCheckForEvents)
   {
      if(gAbortMacro) // Escape key has been pressed - return panic
         return(2);

      if(gShowWaitCursor) 
      {
         if(!gBlockWaitCursor && !gDebugBlockWaitCursor)
         {
            SetCursor(LoadCursor(NULL,IDC_WAIT));
         }
      }

      short r = MainWindowRedraw();
      if(r != 0)
         return(r); // Return panic (2) or abort (1)
   
      gCheckForEvents = false;
   }

   return(MACRO_RUNNING); // Return 0 - all ok
}


/*************************************************************************************
      While a macro is running periodically check to see if some window events 
      need processing. This includes seeing if the windows need moving, resizing
      opening or closing, if the Stop/Panic button has been pressed or if the escape
      key has been pressed.

      Return 0 if all ok, 1 if abort button pressed, 2 if panic button pressed
**************************************************************************************/

short MainWindowRedraw()
{
   MSG msg;
   HWND hWnd;
   WinData *win = NULL;
   ObjectData *obj = NULL;
   int message;
   extern bool gInMessageMode;

// Make sure the GUI window has the focus if it exists
// otherwise can't press escape key to abort or press active tabs.
  if(GetGUIWin()) 
  {
    if(gDebug.mode == "off" && !gInMessageMode && !gKeepCurrentFocus) // don't put html window into background if loading
    {
       SetFocus(GetGUIWin()->hWnd);
    }
  }

// Check for queued messeages
	while(PeekMessage(&msg, NULL,NULL,NULL,PM_REMOVE))
   {
      if(!msg.hwnd)
         continue;

      hWnd = msg.hwnd;
      message = msg.message;

      if(gDebug.mode != "off") // We are in debug mode so need to access many windows
      {
         obj = FindActiveObject(hWnd);
         if(obj)
            win = obj->winParent;
         else
         {
            if(gDebug.win)
               win = gDebug.win;
            else
               win = GetGUIWin();
         }
      }
      else // Not in debug mode so only allow access to current gui-window
      {
         win = GetGUIWin();
         if(win)
            obj = win->widgets.findByWin(hWnd);
      }

   // Process GUI window actions if one of the controls has a special mode
      if(win)
      {
      // CLI window and text box in debugger have a special mode and all actions are allowed
         if(obj && obj->debug && (obj->type == CLIWINDOW || obj->type == TEXTBOX))
         {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
         }

    //   Only left button clicks are allowed for active text editors, dividers and buttons
         if(message == WM_LBUTTONDOWN || message == WM_LBUTTONDBLCLK || message == WM_LBUTTONUP)
         {
            // Abort button
            if(obj && obj->nr() == win->abortID && message != WM_LBUTTONUP)
            { 
               DispatchMessage(&msg);	
               return(1); // Abort
            }	
            // Panic button
            if(obj && obj->nr() == win->panicID && message != WM_LBUTTONUP)
            { 
               DispatchMessage(&msg);
               gAbortMacro = true;
               return(2); // Panic
            }
            // Active controls
            if(obj && obj->active == true)
            { 
               if(obj->type == TEXTEDITOR && message != WM_LBUTTONUP) // Allow user to add break points or jump to subroutines
               {
                  DispatchMessage(&msg);
                  continue;
               }

               if(obj->type == DIVIDER && message != WM_LBUTTONUP)
               {
                  DispatchMessage(&msg);
                  continue;
               }

					if(obj->type == PLOTWINDOW && (message == WM_LBUTTONDOWN || message == WM_LBUTTONUP)) // Just allow selection
               {
                  DispatchMessage(&msg);
                  continue;
               }

            // Run control macro for button
               if((obj->type == BUTTON || obj->type == TABCTRL || obj->type == PANEL)  && message != WM_LBUTTONUP)
               {
                  DispatchMessage(&msg);

                  char *str;
                  char proc[MAX_STR];
	               str = obj->command;
	               if(str[0] != '\0')
	               {
	                  sprintf(proc,"control(%d,%d)",win->nr,obj->nr());
                     if(curEditor && curEditor->debug)
                        SetFocus(curEditor->edWin);

                     if(gDebug.enabled)
                     {
                        gDebug.enabled = false;
	                     ProcessMacroStr(LOCAL,win,obj,str,"",proc, win->macroName, win->macroPath);
                        gDebug.enabled = true;
                     }
                     else
	                     ProcessMacroStr(LOCAL,win,obj,str,"",proc, win->macroName, win->macroPath);

                    if(curEditor && curEditor->debug)
                        SetFocus(curEditor->edWin);
	               }

               }
               continue;
            }
            continue;
         }
         else if(message == WM_RBUTTONDOWN) 
         {
            if(obj && obj->active && obj->debug)
            { 
               if(obj->type == TEXTEDITOR) // Allow using to load previous files
               {
                  DispatchMessage(&msg);
                  continue;
               }
            }
         }
         else if(message == WM_NCLBUTTONDOWN) // Allow menu access in debugger
         {
            if(win->debugger && hWnd == win->hWnd)
            { 
               if(msg.wParam != HTSYSMENU)
                  DispatchMessage(&msg);
               continue;
            }
         }
      // If cursor is moved over an active button make sure an arrow is shown
         else if(message == WM_MOUSEMOVE) 
         {
            bool gBlockWaitCursorBak;
            obj = win->widgets.findByWin(msg.hwnd);
            if(obj)
            {
               if(((obj->nr() == win->abortID || obj->nr() == win->panicID || obj->active)) ||
                  win->debugger) 
               {
                  gBlockWaitCursorBak = gBlockWaitCursor;
                  gBlockWaitCursor = true;
                  DispatchMessage(&msg);
                  gBlockWaitCursor = gBlockWaitCursorBak;
               }
               else
               {
                //  gBlockWaitCursor = false;
               //   SetCursor(LoadCursor(NULL,IDC_WAIT));
               }
            }
            continue;
         } 
//         else
//        DispatchMessage(&msg);

      }


   // If user has pressed the escape button stop the macro
   // but only if the GUI window is visible and we are not debugging
      if(GetGUIWin() && message == WM_KEYDOWN) 
      {  
         if(msg.wParam == VK_ESCAPE && gDebug.mode == "off")
         { 
            DispatchMessage(&msg);	
            gAbortMacro = true;
            return(2); // Panic
         }	
      }

       if(message == WM_MOUSEMOVE) 
       {
         gBlockWaitCursor = false;
         SetCursor(LoadCursor(NULL,IDC_WAIT));
         continue;
       } 

       if(message == WM_MOUSEACTIVATE) 
          continue;

       if(message == WM_RBUTTONDOWN ||
          message == WM_LBUTTONDOWN ||
          message == WM_MOUSEMOVE   ||
          message == WM_LBUTTONUP   ||
          message == WM_SETFOCUS)
       {
          continue;
       }
   

   // Allow user to show/hide all gui windows with F2
      if(message == WM_KEYDOWN)
      {
         if(msg.wParam == VK_F2)
         {
            if(AnyGUIWindowVisible())
               HideGUIWindows();
            else
               ShowGUIWindows(SW_SHOWNOACTIVATE);
         }
         continue;
      }

   // Don't allow menus to be displayed
      if(message == WM_NCLBUTTONDOWN)
      {
         if(msg.wParam == HTMENU)
         {
            continue;
         }
      }

   // If moving over the titlebar show arrow
      if(message == WM_NCMOUSEMOVE || message == WM_NCLBUTTONDOWN)
      {
         if(msg.wParam == HTCAPTION)
            SetCursor(LoadCursor(NULL,IDC_ARROW));
      }

   // Process other messages (e.g. resize, repaint, window move etc) 
      DispatchMessage(&msg);
      continue;
	} 

   return(0);
}

/***************************************************************
 Find the object associated with a window handle.  
 The parent window should contain an active or an abort control
 If debugging only return controls from windows
 being debugged or from the debugger itself.
****************************************************************/

ObjectData* FindActiveObject(HWND hWnd)
{
   for(WinData *w = rootWin->next; w != NULL; w = w->next)
   {
      if(w->abortID != -1 || w->panicID != -1 || w->activeCtrl)
      {
         if(gDebug.mode == "off" || (w->debugging || w->debugger))
         {
				ObjectData* obj = w->widgets.findByWin(hWnd);
				if (obj)
					return obj;
         }
      }
   }
   return((ObjectData*)0);
}

/***************************************************************
       A more compact way of importing (caching) macros
****************************************************************/

char ImportMacroStr[] =
	"procedure(import, macro, directory, mode);\
;\
   if(nrArgs == 1);\
      directory = getcwd();\
	   mode = \"global\";\
	elseif(nrArgs == 2);\
		mode = \"global\";\
	endif;\
   bak = getcwd();\
   cd(directory,\"false\");\
   rmcachedmacro(directory,macro,mode);\
   cachemacro(macro,mode);\
   cacheproc(\"true\");\
   cd(bak,\"false\");\
;\
endproc()";


EXPORT int UpdateProspaArgumentVariables(Interface *itfc, char *args);

int ImportMacro(Interface *itfc, char args[])
{
   // Save the current procedure info
   CText oldMacroName  = itfc->macroName;
   CText oldMacroPath  = itfc->macroPath;
   CText oldProcName   = itfc->procName;
   long oldStartLine   = itfc->startLine;
   int oldParentLineNr = itfc->parentLineNr;

   // Save the macro stack info
   MacroStackInfo* info = new MacroStackInfo;
   info->macroPath = oldMacroPath;
   info->macroName = oldMacroName;
   info->procName  = oldProcName;
   info->lineNr    = itfc->parentLineNr;
   itfc->macroStack.push_back(info);

   // Call the import procedure
	int nrArgs = UpdateProspaArgumentVariables(itfc, args);
   short r = ProcessMacroStr(itfc, false, ImportMacroStr);

   // Restore the procedure info
   itfc->macroName    = oldMacroName;
   itfc->macroPath    = oldMacroPath;
   itfc->procName     = oldProcName;
   itfc->startLine    = oldStartLine;
   itfc->parentLineNr = oldParentLineNr;

   return(r);
}

// Set the error display level
// 0 - don't show errors
// 1 - display the first error
// 2 - display all errors (might be confusing)

int ShowErrors(Interface* itfc, char args[])
{
   short nrArgs;
   short level;

   level = gShowErrorMode;

   if ((nrArgs = ArgScan(itfc, args, 1, "error level", "e", "d", &level)) < 0)
      return(nrArgs);

   if (level >= 0 && level <= 2)
      gShowErrorMode = level;

   itfc->nrRetValues = 0;
   return(OK);
}
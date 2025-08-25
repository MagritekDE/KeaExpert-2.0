#include "stdafx.h"
#include "control.h"
#include <float.h>
#include <math.h>
#include "cache.h"
#include "cArg.h"
#include "cli_events.h"
#include "command_other.h"
#include "edit_files.h"
#include "edit_class.h"
#include "evaluate.h"
#include "files.h"
#include "font.h"
#include "globals.h"
#include "guiModifyObjectParameters.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include "Inset.h"
#include "interface.h"
#include "listbox.h"
#include "list_functions.h"
#include "macro_class.h"
#include "main.h"
#include "mymath.h"
#include "plot.h"
#include "PlotGrid.h"
#include "PlotWindow.h"
#include "process.h"
#include "scanstrings.h"
#include "string_utilities.h"
#include "variablesOther.h"
#include "defineWindows.h"
#include "memoryLeak.h"

short MaxArrayElementLength(float *m, long N);
short MaxArrayElementLength(double *m, long N);
void MaxCArrayElementLength(complex *m, long N, short &maxLenReal, short &maxLenImag);
int RemoveCachedMacro(Interface *itfc,  char args[]);
int ListCachedProcedures(Interface *itfc,  char args[]);
void ListObjectParameters(Interface *itfc, ObjectData *obj);
void ListPlotRegionParameters(Plot *plot);
void ListWindowParameters(Interface *itfc, WinData *win);

void AddProcedures(Interface *itfc, char *path, char* name);

bool gInTryBlock = false;
bool abortOnError = true;	// Externally accessible

/**************************************************************************
*    If statement - syntax: if(expression)  ... endif                     *
*                                                                         *
* If |expression| >= 0.5 then result is considered true otherwise false   *
**************************************************************************/

int IfStatement(Interface* itfc, char statement[])
{
   bool result;
   Variable ans;
   
/* Extract value of statement  *********************/
	short type = Evaluate(itfc,RESPECT_ALIAS,statement,&ans);

   if (type == FLOAT32)
      result = (abs(nint(ans.GetReal())) >= 1);
   else if (type == FLOAT64)
      result = (abs(nhint(ans.GetDouble())) >= 1);
   else
   {
      ErrorMessage("result of if statement must be a single or double precision number");
      return(ERR);
   }
    
/* Check for if-endif structure **********************/
     
   if(result == false)
   {
      itfc->macro->jump = true;
      return(0); /* Goto line number after matching else or endif */
   }
   else
   {
      itfc->macro->jump = false;
      return(0); /* Goto next instruction */
   }
}

/**************************************************************************
* if ... elseif(expression)  ... endif                                    *
*                                                                         *
* If |expression| >= 0.5 then result is considered true otherwise false   *
**************************************************************************/

int ElseIf(Interface* itfc, char statement[])
{
   short type;
   bool result;
   Variable resultVar;
   
// Extract value of statement  *********************
   type = Evaluate(itfc, RESPECT_ALIAS,statement,&resultVar);
   if (type == FLOAT32)
      result = (abs(nint(resultVar.GetReal())) >= 1);
   else if(type == FLOAT64)
      result = (abs(nhint(resultVar.GetDouble())) >= 1);
   else
   {
      ErrorMessage("result of elseif statement must be a single or double precision number");
      return(ERR);
   }
   
// Check for if-endif structure *********************
   itfc->nrRetValues = 0;

   if(result == false)
   {
      itfc->macro->jump = true;
      return(0); // Goto line number after matching else or endif 
   }
   else
   {
      itfc->macro->jump = false;
      return(0); // Goto next instruction
   }
}

/******************************************************************************
*      The "while" statement                                                  *
*                                                                             *
*      Syntax is while(expression = true)                                     *
*                  ...                                                        *
*                endwhile                                                     *
*                                                                             *
*  If |expression| >= 0.5 then result is considered true otherwise false      *
*                                                                             *
******************************************************************************/ 

int WhileStatement(Interface* itfc, char args[])
{
   short type;
   bool result;
   Variable var;

// Extract expression  *********************
   type = Evaluate(itfc,RESPECT_ALIAS,args,&var);
   if (type == FLOAT32)
      result = (abs(nint(var.GetReal())) >= 1);
   else if(type == FLOAT64)
      result = (abs(nhint(var.GetDouble())) >= 1);
   else
   {
      ErrorMessage("result of while statement must  must be a single or double precision number");
      return(ERR);
   }

// Check for result of expression ************     
   if(result == false)
   {
      itfc->macro->jump = true;
      return(OK); // Goto line number after matching endwhile
   }
   else
   {
      itfc->macro->jump = false;
      return(OK); // Goto next instruction
   }
}

int ExitWhileLoop(Interface* itfc, char args[])
{
   itfc->macro->jump = true;
   return(OK);
}

int EndWhileLoop(Interface* itfc, char args[])
{
   itfc->macro->jump = true;
   return(OK);
}

int TryForAnError(Interface* itfc, char args[])
{
   itfc->macro->inTryBlock = true;
   gInTryBlock = true;
 //  TextMessage("Try thread ID %lX\n", GetCurrentThreadId());
   return(OK);
}

int EndTry(Interface* itfc, char args[])
{
   itfc->macro->inTryBlock = false;
   gInTryBlock = false;
 //  TextMessage("End try thread ID %lX\n", GetCurrentThreadId());
   return(OK);
}

int CatchError(Interface* itfc, char args[])
{
	extern bool errorDetected;
   itfc->macro->inTryBlock = false;
//   TextMessage("Catch thread ID %lX\n", GetCurrentThreadId());
   gInTryBlock = false;
	errorDetected = false;
   return(OK);
}

int ThrowException(Interface* itfc, char args[])
{
   short n;
   CText message;

   if ((n = ArgScan(itfc, args, 1, "message", "e", "t", &message)) < 0)
      return(n);

   if (message != "")
   {
      *GetErrorInfo() = *GetCmdInfo();
      GetErrorInfo()->lastError = message;
      GetErrorInfo()->type = "Throw";
      GetErrorInfo()->description = message;
      GetErrorInfo()->errorFound = true;
      gErrorInfo.blocked = true;
      gErrorInfo = *GetErrorInfo();
      gErrorInfo.blocked = false;
   }

   return(THROW);
}

     
/******************************************************************************
*      The "for" loop                                                         *
*                                                                             *
*      Syntax is for(var = loopStart,loopEnd)                                 *
*                                                                             *
*      Example:  for(z = 0 to 100 step 0.5)  .... next(z                      *
*                                                                             *
******************************************************************************/ 

int ForLoop(Interface *itfc, char str[])
{
   char  initStr[MAX_STR];
   char initVar[MAX_STR];
   char  initExp[MAX_STR];
   char  finalExp[MAX_STR];
   char  stepStr[MAX_STR];
   float loopStart,loopEnd;
   long  steps;
   short type;
   short i,j,k,posTo=0,found;
   float stepSize = 1;
   short len;
   Variable result;

   len = strlen(str);
   
// Search for 'to' in argument and extract initial condition 'initStr' *******
   j = 0;
   for(i = 0; i < len-3; i++) // j points to end of initial condition
   {
      if(str[i] == ' ' && str[i+1] == 't' && str[i+2] == 'o' && str[i+3] == ' ')
      {
         initStr[j] = '\0';
         posTo = i+1;
         break;
      }
      initStr[j++] = str[i];
   }
   if(i == len-3)
   {
      ErrorMessage("missing 'to' in for statement");
      return(ERR);
   }

// Extract inital variable and expression from initStr **********************  
  found = 0;
  short initlen = strlen(initStr);
  for(i = 0; i < initlen; i++)
  {
     if(found == 0)
     {
        if(initStr[i] == ' ') // Skip initial white space
           continue;
        found = 1; // Found start of variable
     }

     else if(found == 1 && (initStr[i] == ' ' || initStr[i] == '=')) // Found end of variable
     {
        initVar[i] = '\0';

        if(IsWhiteSpaceString(initVar))
        {
           ErrorMessage("Loop variable not defined");
           return(ERR);
        }
		  if((!strcmp(initVar,"i")) || (!strcmp(initVar,"j")))
		  {
		     ErrorMessage("Can't use i or j as variable names");
		     return(ERR);
		  }

        k = 0;
        found = 0;
        for(j = i; j < initlen; j++)
        {
           if(found == 0 && initStr[j] == ' ') // Skip over white space after variable
              continue;
              
           else if(found == 0 && initStr[j] == '=') // Found equals
           {
              found = 1;
              continue;
           }
           
           if(found)
              initExp[k++] = initStr[j];
        }
        if(found == 0)
        {
           ErrorMessage("missing '=' in for statement");
           return(ERR);
        }
        initExp[k] = '\0';
        if(IsWhiteSpaceString(initExp))
        { 
           ErrorMessage("Inital value for loop variable not defined");
           return(ERR);
        }
        break;
     }
     initVar[i] = initStr[i];
  }
  if(found == 0)
  { 
     ErrorMessage("Inital value for loop variable not defined");
     return(ERR);
  }
           
// Extract final expression (after 'to' and before 'step') *******************************  
   k = 0;
   found = 0;

   for(i = posTo+2; i < len; i++)
   {
      if(i < len-5 && str[i] == ' ' && str[i+1] == 's' && str[i+2] == 't' && str[i+3] == 'e' && str[i+4] == 'p' && str[i+5] == ' ')
	   {
	      found = 1;
	      break;
	   }
      finalExp[k++] = str[i];
   }
   finalExp[k] = '\0';
   if(IsWhiteSpaceString(finalExp))
   {
      ErrorMessage("Loop limit value for loop variable not defined");
      return(ERR);
   }

// Extract step size *****************************************************
   if(found == 1)
   {  
	   for(j = i+5; j < len; j++) // Extract step size
	      stepStr[j-i-5] = str[j];
	   stepStr[j-i-5] = '\0';

      if(IsWhiteSpaceString(stepStr))
      {
         ErrorMessage("Step size for loop variable not defined");
         return(ERR);
      }
      type = Evaluate(itfc,RESPECT_ALIAS,stepStr,&result); // Evaluate it

      if(type != FLOAT32)
      {
         ErrorMessage("for-loop step size must a real scalar");
         return(ERR);
      }
      stepSize = result.GetReal();
      if(stepSize == 0)
      {
         ErrorMessage("for-loop step size cannot be zero");
         return(ERR);
      }
   }   

// Initialize loop counter **************************************
   type = Evaluate(itfc,RESPECT_ALIAS,initExp,&result);
   if(type != FLOAT32 )
   {
     ErrorMessage("Loop variable must be real");
      return(ERR);
   }
   Variable *var = AddVariable(itfc,itfc->varScope,FLOAT32,initVar);
   var->MakeAndSetFloat(result.GetReal());
   loopStart = result.GetReal();

// Extract loop limit *******************************************
   type = Evaluate(itfc,RESPECT_ALIAS,finalExp,&result);
   

   if(type == FLOAT32)
      loopEnd = result.GetReal();	
	else if(type == FLOAT64)
      loopEnd = (float)result.GetDouble();	
	else
   {
	  ErrorMessage("loop limit should be a real number");
	  return(ERR);
   }

// Work out the number of steps to take **************************
   if(stepSize > 0 && loopEnd < loopStart)
      steps = 0;

   else if(stepSize < 0 && loopEnd > loopStart)
      steps = 0;

   else 
   {
      steps = (long)(((double)loopEnd - (double)loopStart)/(double)stepSize) + 1;
   }

   if(steps == 0) // Loop limit is less than start so jump out of loop
   {
      itfc->macro->jump = true;
      return(OK);
   }
   else // Loop again
   {
      itfc->macro->jump = false;
   }

// Check for step size too small
   float v1 = loopStart + stepSize*steps;
   float v2 = loopStart + stepSize*(steps-1);
   if(v1 == v2)
   {
      ErrorMessage("for loop will never exit! (Loop increment below float resolution)");
      return(ERR);
   }

// Push loop variable, the loop step size and the number of steps onto stack *
   itfc->macro->PushStack(var,loopStart,stepSize,steps,steps);

   itfc->nrRetValues = 0;
   return(OK);
}

/******************************************************************************
*     When a "next" statement is found increment variable & check for limit   *
******************************************************************************/ 

// stackPos[pos]     : variable 
// stackPos[pos-1]   : step size
// stackPos[pos-2]   : nr. of steps

int NextLoop(Interface* itfc ,char name[])
{
   Variable *var;
   long pos;
   double stepSize;
   double stepsLeft;
   double loopStart;
   double steps;
   
   pos = itfc->macro->stackPos;
   
   var         = itfc->macro->stack[pos].var;
   loopStart   = itfc->macro->stack[pos].loopStart;   
   stepSize    = itfc->macro->stack[pos].stepSize;   
   stepsLeft   = itfc->macro->stack[pos].stepsLeft;
   steps       = itfc->macro->stack[pos].steps;
   

// Decrement steps left **********************************
   stepsLeft--;
   itfc->macro->stack[pos].stepsLeft = (float)stepsLeft;

// Increment loop variable *******************************
   var->MakeAndSetFloat((float)(loopStart + stepSize*(steps-stepsLeft)));

// Check to see if all steps taken ***********************
   if(stepsLeft <= 0) 
   {
      itfc->macro->PopStack();
      itfc->macro->jump = false; // Jump out of loop
   }
   else // Loop again
   {
      itfc->macro->jump = true;
   }

   return(0);
}

int ExitForLoop(Interface* itfc, char args[])
{
   itfc->macro->PopStack();
   itfc->macro->jump = true;
   return(OK);
}

/*********************************************************************************
* Load a macro from a disk file and return it as a string to the calling program *
* Throws an exception if memory can't be allocated or the file can't be found    *
* Note that memory for the data string is allocated in this routine.             *
*                                                                                *
* Written by C Eccles 11 June 1998.                                              *
*********************************************************************************/

char*
LoadFileMacro(char* fileName)

{
	FILE *fp;
	long size;	
	static char *data;

	if((fp = fopen(fileName,"rb")) == (FILE*)0)
	{
	   ErrorMessage("LoadFileMacro -> can't open file");
	   return(0);
	}
	   
	size = GetFileLength(fp);
	if((data = new char[size+1]) == (char*)0)
	{
	   ErrorMessage("LoadFileMacro -> out of memory");
	   return(0);
	}
	   
	fread(data,1,size,fp);
	fclose(fp);
	data[size] = '\0';

	return(data);
}


int PrintStringLocal(Interface* itfc, char*,int,char);
short PrintOneItem(Variable *var, char *name, char pmode, short pos, bool inCLI, short offset);
short PrintVariable(Interface *itfc, char *name, short scope, char pmode, short pos);

/*********************************************************************************
         Evaluates and prints the argument passed in 'arg'.                      
*********************************************************************************/

int PrintString(Interface* itfc, char args[])
{
   return(PrintStringLocal(itfc,args,-1,'n'));
}



/*******************************************************************************
  As above but this adds a scope flag. Scope can be -1 which means
  the print command uses normal scope precedence to decide which
  variable to print or it can be explicit (LOCAL, WINDOW or GLOBAL). 
  The latter is used by the calls:
        pr("local") pr("global") pr("winvar") & pr("hidden)
********************************************************************************/

int PrintStringLocal(Interface* itfc, char args[], int scope, char pmode)
{
   Variable *var;
   short type;
   CText txt;
   CText argumentN;
   CArg carg;
   short nrArgs;
   short result = OK;

   nrArgs = carg.Count(args);

// Loop over all arguments, each of which will be printed
   for(int arg = 1; arg <= nrArgs; arg++)
   {   
      argumentN = carg.Extract(arg);
	   
  // Visible global variables
	   if(argumentN == "glo" || argumentN == "global")
	   {
	      var = &globalVariable;
	      TextMessage("\n\n###### Global Variables ######\n");

      // Add all global variable names to a list
         char **list = NULL;
         int cnt = 0;
	      while(var)
	      {
	         var = var->GetNext(type);
	         if(!var) break;
            if(var->GetVisible() == false) continue;
            AppendStringToList(var->GetName(),&list,cnt++);
	      }

      // Sort the list alphabetically
         SortList(list,cnt);

      // Step through the list printing out results
         for(int i = 0; i < cnt; i++)
         {
            if(PrintStringLocal(itfc,list[i],GLOBAL,'v') == ERR)
            {
               result = ERR;
               break;
            }
         }

         FreeList(list,cnt);
         TextMessage("\n\n###### End global variables ######\n");
         continue;
	   }

  // Hidden global variables
	   if(argumentN == "hidden" || argumentN == "hid")
	   {
	      var = &globalVariable;
	      TextMessage("\n\n###### Hidden Variables ######\n");

      // Add all hidden global variable names to a list
         char **list = NULL;
         int cnt = 0;
	      while(var)
	      {
	         var = var->GetNext(type);
	         if(!var) break;
            if(var->GetVisible() == true) continue;
            AppendStringToList(var->GetName(),&list,cnt++);
	      }

      // Sort the list alphabetically
         SortList(list,cnt);

      // Step through the list printing out results
         for(int i = 0; i < cnt; i++)
         {
            if(PrintStringLocal(itfc,list[i],GLOBAL,'v') == ERR)
            {
               result = ERR;
               break;
            }
         }

         FreeList(list,cnt);
         TextMessage("\n\n###### End hidden variables ######\n");
         continue;
	   }

  // Window variables
	   if(argumentN == "winvar")
	   {
	      if(itfc && itfc->win)
//	      if(GetGUIWin())
	      {
	//	      var = &(GetGUIWin()->varList);
		      var = &(itfc->win->varList);
			   TextMessage("\n\n###### Window Variables ######\n");
       //     if(var && itfc->win && (itfc->win == GetGUIWin()))
            if(var)
		      {
            // Add all window variable names to a list
               char **list = NULL;
               int cnt = 0;
	            while(var)
	            {
	               var = var->GetNext(type);
	               if(!var) break;
                  AppendStringToList(var->GetName(),&list,cnt++);
	            }

            // Sort the list alphabetically
               SortList(list,cnt);

            // Step through the list printing out results
               for(int i = 0; i < cnt; i++)
               {
                  if(PrintStringLocal(itfc,list[i],WINDOW,'v') == ERR)
                  {
                     result = ERR;
                     break;
                  }
               }
               FreeList(list,cnt);
		      }
            TextMessage("\n\n###### End window variables ######\n");
		   }
         continue;
	   }
	
   // Local variables
	   if(argumentN == "loc" || argumentN == "local")
	   {      
	      if(itfc->macro)
	      {
			   TextMessage("\n\n###### Local Variables ######\n");
		      var = &(itfc->macro->varList);
		      if(var)
		      {
           // Add all local variable names to a list
               char **list = NULL;
               int cnt = 0;
	            while(var)
	            {
	               var = var->GetNext(type);
	               if(!var) break;
                  AppendStringToList(var->GetName(),&list,cnt++);
	            }

            // Sort the list alphabetically
               SortList(list,cnt);

            // Step through the list printing out results
               for(int i = 0; i < cnt; i++)
               {
                  if(PrintStringLocal(itfc,list[i],LOCAL,'v') == ERR)
                  {
                     result = ERR;
                     break;
                  }
               }
               FreeList(list,cnt);
		      }
            TextMessage("\n\n###### End local variables ######\n");
		   }
         continue;
		}

      else
      {
          result = PrintVariable(itfc, argumentN.Str(), scope, pmode, arg);
      }
   
   }

   itfc->nrRetValues = 0;
   return(result);
}

short PrintVariable(Interface *itfc, char *name, short scope, char pmode, short arg)
{
   Variable result,*var;

   if(scope == -1) // No scope defined
   {
   // List specific variables 
      int err = Evaluate(itfc, RESPECT_ALIAS, name, &result);
      if(err < 0)
         return(err);
	
   // Check to see if print argument returns multiple variables
   // If it does then call this routine iteratively until all
   // have been listed.	  
      if(itfc->nrRetValues > 1)
      {
         short n;
         TextMessage("\n\n");
         n = itfc->nrRetValues; // Gets changed in evaluate
         pmode = 'r';
         for(short i = 1; i <= n; i++)
         {
            if(PrintOneItem(&itfc->retVar[i],"ans",pmode,arg,itfc->inCLI,0) == ERR)
               break;
         }
         TextMessage("\n");      
         pmode = 'n';      
         return(OK);
      }
      else if(itfc->nrRetValues == 1)
      {
         if(PrintOneItem(&result,name,pmode,arg,itfc->inCLI,0) == ERR)
            return(ERR);
      }
   }
   else // If scope is defined
   {
      short type;

      if(scope == LOCAL)
      {
         if(itfc->macro)
         {
            if((var = itfc->macro->varList.Get(ALL_VAR,name,type)) != NULL)
            {
               if(PrintOneItem(var,name,pmode,arg,itfc->inCLI,0) == ERR)
                  return(ERR);
               return(OK);
            }         
         }
      }
      else if(scope == WINDOW)
      {
    //     if(GetGUIWin() && itfc->win)
         if(itfc->win)
         {
    //        if(itfc->win == GetGUIWin())
            {
               if((var = itfc->win->varList.Get(ALL_VAR,name,type)) != NULL)
               {
                  if(PrintOneItem(var,name,pmode,arg,itfc->inCLI,0) == ERR)
                     return(ERR);
                  return(OK);
               }
            }
         }
      }
      else if(scope == GLOBAL)
      {
         if((var = globalVariable.Get(ALL_VAR,name,type)) != NULL)
         {
            if(PrintOneItem(var,name,pmode,arg,itfc->inCLI,0) == ERR)
               return(ERR);
            return(OK);
         }
      }
   }
   return(OK);
}

/***********************************************************
   Print the contents of the interface returned variables
***********************************************************/

short PrintRetVar(Interface *itfc, char *name)
{
   Variable result;

   if(itfc->nrRetValues > 1)
   {
      TextMessage("\n\n");
      for(short i = 1; i <= itfc->nrRetValues; i++)
      {
         if(PrintOneItem(&itfc->retVar[i],name,'r',0,itfc->inCLI,0) == ERR)
            break;
      }
      TextMessage("\n");      
      return(OK);
   }
   else if(itfc->nrRetValues == 1)
   {
      TextMessage("\n");
      if(PrintOneItem(&itfc->retVar[1],name,'n',0,itfc->inCLI,0) == ERR)
         return(ERR);
   }

   return(OK);
}

/*******************************************************************************
  This prints the contents of var using the label passed in "name"
  pmode is the prefix mode which controls formating at the start of the output.
  pos specifies which variable is being printed (1,2,3 ...) again for formating
  purposes.
********************************************************************************/

short PrintOneItem(Variable *var, char *name, char pmode, short pos, bool inCLI, short offset)
{
   char prefix[MAX_STR];
   char suffix[MAX_STR];
   char start[MAX_STR];
   char str[MAX_STR];
   
// Check for errors
   long len = strlen(name);
   if(len > MAX_STR-10)
   {
      ErrorMessage("string to long to print");
      return(ERR);
   }

// Select prefix and suffix
   if(pmode == 'n')
   {
      if(inCLI)
      {
         if(pos == 1)
            strcpy(start,"\n");
         else
            strcpy(start,"");

         sprintf(prefix,"%s\n   %s = ",start,name);
         strcpy(suffix,"\n");
      }
      else
      {
         if(!messageSent && pos == 1)
         {
            if(len >= 2 && name[len-2] != '\\' && name[len-1] != 'r')
               strcpy(start,"");
            else
               strcpy(start,"\n");
         }
         else
            strcpy(start,"");

         if(!isStr(name))
         {
            sprintf(prefix,"%s\n   %s = ",start,name);
            strcpy(suffix,"\n");
         }
         else
         {
            strncpy_s(prefix,MAX_STR,start,_TRUNCATE);  
            strncpy_s(suffix,MAX_STR,"",_TRUNCATE);  
         }
      }
   }
   else if(pmode == 'v')
   {
      sprintf(prefix,"\n   %s = ",name);
      strcpy(suffix,"");  
   }
   else if(pmode == 'q')
   {
      char format[100];
      sprintf(format,"\n   %%%hds%%s = ",offset);
      sprintf(prefix,format,"",name);
      strcpy(suffix,"");
      pmode = 'v';
   }
   else
   {
      strcpy(prefix,"   ");
      strcpy(suffix,"");  
   }


// Print contents of var *************************   
   switch(var->GetType())
   {
      case(NULL_VARIABLE):
      {         
         if(var->GetAlias() && pmode == 'v')
            TextMessage("%snull - (alias to '%s' a %s%s)",prefix,var->GetAlias()->GetName(),
                               EnumToString(VARIABLE_SOURCE,var->GetAlias()->GetScope()),suffix);
         else
            TextMessage("%snull",prefix,suffix);
         
         break;
      }
      case(INTEGER):
      {
         if(var->GetAlias() && pmode == 'v')
            TextMessage("%s%d - (alias to '%s' a %s%s)",prefix,VarInteger(var),var->GetAlias()->GetName(),
                                 EnumToString(VARIABLE_SOURCE,var->GetAlias()->GetScope()),suffix);
         else
            TextMessage("%s%d%s",prefix,VarInteger(var),suffix);
         break;
      }
      case(FLOAT32):
      {         
         if(var->GetAlias() && pmode == 'v')
            TextMessage("%s%.9g - (alias to '%s' a %s%s)",prefix,VarReal(var),var->GetAlias()->GetName(),
                               EnumToString(VARIABLE_SOURCE,var->GetAlias()->GetScope()),suffix);
         else
         {
				TextMessage("%s%.9g%s",prefix,VarReal(var),suffix);
         }
         
         break;
      }
      case(FLOAT64):
      {         
         if(var->GetAlias() && pmode == 'v')
            TextMessage("%s%.17g - (alias to '%s' a %s%s)",prefix,var->GetDouble(),var->GetAlias()->GetName(),
                               EnumToString(VARIABLE_SOURCE,var->GetAlias()->GetScope()),suffix);
         else
         {
            TextMessage("%s%.17g%s",prefix,var->GetDouble(),suffix);
         }
         
         break;
      }
      case(LIST):
      {
         char *locstr;
         long xsize = VarWidth(var);
      
         if(pmode == 'n')
         {     
	         TextMessage("\n\n   %s = {\n",name);
	         for(short i = 0; i < xsize; i++)
	         {
	            locstr = VarList(var)[i];
	            if(FormatTextMessage("       %s\n",locstr) == ERR)
	               return(ERR);
	         }
            TextMessage("   }");

	      }
	      else
         {
            if(var->GetAlias() && pmode == 'v')
               TextMessage("%s(list with %ld elements) - (alias to '%s' a %s)",prefix,xsize,var->GetAlias()->GetName(),
                           EnumToString(VARIABLE_SOURCE,var->GetAlias()->GetScope()));
            else
               TextMessage("%s(list with %ld elements)",prefix,xsize);
	         break;
            
         }
         break;
      }   
      case(LIST2D):
      {
			List2DData* list = (List2DData*)VarList2D(var);
         char *locstr;
         long ysize = VarHeight(var);
      
         if(pmode == 'n')
         {     
	         TextMessage("\n\n   %s =\n",name);
	         for(short j = 0; j < ysize; j++)
				{
					for(short i = 0; i < list->rowSz[j]; i++)
					{
						locstr = list->strings[j][i];
					   if(i == 0)
						{
							if(list->rowSz[j] > 1)
							{
							   if(FormatTextMessage("       \"%s\",",locstr) == ERR)
								   return(ERR);
							}
							else
							{
							   if(FormatTextMessage("       \"%s\"",locstr) == ERR)
								   return(ERR);
							}
						}
						else
						{
							if(i < list->rowSz[j]-1)
							{
								if(FormatTextMessage("   \"%s\",",locstr) == ERR)
									return(ERR);
							}
							else
							{
								if(FormatTextMessage("   \"%s\"",locstr) == ERR)
									return(ERR);
							}
						}
					}
					TextMessage("\n");

				}

	      }
	      else
         {
            if(var->GetAlias() && pmode == 'v')
               TextMessage("%s(2D jagged list with ?? by %ld elements) - (alias to '%s' a %s)",prefix,ysize,var->GetAlias()->GetName(),
                           EnumToString(VARIABLE_SOURCE,var->GetAlias()->GetScope()));
            else
               TextMessage("%s(2D jagged list with ?? by %ld elements)",prefix,ysize);
	         break;
            
         }
         break;
      } 
      case(STRUCTURE):
      {
         Variable *struc,*svar;

         struc = var->GetStruct();
			if(!struc)
			{
				ErrorMessage("Null structure to print");
				return(ERR);
			}
         svar = struc->next;

         char format[100];

         if(pmode == 'n')
         {  
            sprintf(format,"\n\n   %%%hds%%s = {",offset);

            TextMessage(format,"",name);

            int i = 0;
            while(svar != NULL)
            {
               if(PrintOneItem(svar,svar->GetName(),'q',i++,inCLI,7+offset) == ERR)
                  break;  
               svar = svar->next;
            }
            sprintf(format,"\n   %%%hds}\n",offset);
            TextMessage(format,"");
         }
         else
         {
            TextMessage("%s(structure)",prefix);
         }
         break;
      }
      case(STRUCTURE_ARRAY):
      {
         Variable *struc;
         Variable* strucArray = (Variable*)var->GetData();
         int size = var->GetDimX();

         if(pmode == 'n')
         {
            for(int i = 0; i < size; i++)
            {
               struc = strucArray[i].GetStruct();
               {
                  Variable *svar;

                  svar = struc->next;

                  char format[100];
     
                  sprintf(format,"\n\n   %%%hds%%s[%d] = {",offset,i);

                  TextMessage(format,"",name);

                  int i = 0;
                  while(svar != NULL)
                  {
                     if(PrintOneItem(svar,svar->GetName(),'q',i++,inCLI,7+offset) == ERR)
                        break;  
                     svar = svar->next;
                  }
                  sprintf(format,"\n   %%%hds}\n",offset);
                  TextMessage(format,"");
               }
            }
         }
         else
         {
            TextMessage("%s(structure array)",prefix);
         }
         break;
      }
      case(CLASS):
      {
         Interface itfc;

         ClassData *cData = (ClassData*)var->GetData();

         if(CheckClassValidity(cData,false) == ERR)
			{
            TextMessage("%sreference object has been deleted!%s",prefix,suffix);
            break;
			}

         if(pmode == 'n')
         {
            switch(cData->type)
            {
               case(WINDOW_CLASS):
               {
                  WinData *win = (WinData*)cData->data;
                  TextMessage("%s",prefix);
                  ListWindowParameters(&itfc,win);
                  TextMessage("%s",suffix);
                  break;
               }
               case(OBJECT_CLASS):
               {
                  ObjectData *obj = (ObjectData*)cData->data;
                  TextMessage("%s",prefix);
                  ListObjectParameters(&itfc,obj);
                  TextMessage("%s",suffix);
                  break;
               }
               case(PLOT_CLASS):
               {
                  Plot* plt = (Plot*)cData->data;
                  ListPlotRegionParameters(plt);
                  break;
               }
               case(XLABEL_CLASS):
               {
                  Plot* plt = (Plot*)cData->data;
						TextMessage(formatNoArgHeader().c_str());
						TextMessage(plt->FormatXLabelParameters().c_str());
                  break;
               }
               case(YLABEL_CLASS):
               {
                  Plot* plt = (Plot*)cData->data;
						TextMessage(formatNoArgHeader().c_str());
						TextMessage(plt->FormatYLabelParameters().c_str());
						break;
               }
               case(YLABEL_LEFT_CLASS):
               {
                  Plot* plt = (Plot*)cData->data;
						TextMessage(formatNoArgHeader().c_str());
						TextMessage(plt->FormatYLabelLeftParameters().c_str());
						break;
               }
               case(YLABEL_RIGHT_CLASS):
               {
                  Plot* plt = (Plot*)cData->data;
						TextMessage(formatNoArgHeader().c_str());
						TextMessage(plt->FormatYLabelRightParameters().c_str());
						break;
               }
               case(TITLE_CLASS):
               {
                  Plot* plt = (Plot*)cData->data;
						PlotLabel& title = plt->title();
                  TextMessage("\n\n   PARAMETER     VALUE\n\n");
                  TextMessage("   parent .... (%hd,%hd)\n",plt->plotParent->obj->winParent->nr,plt->plotParent->obj->nr());
						TextMessage("   color ..... %s\n",Plot::GetColorStr(title.fontColor()));
                  TextMessage("   font ...... '%s'\n",title.font().lfFaceName);
                  TextMessage("   size ...... %hd\n",title.fontSize());
                  TextMessage("   style ..... %s\n",GetFontStyleStr(title.fontStyle()));
                  TextMessage("   text ...... '%s'\n",title.text());
                  break;
               }
               case(GRID_CLASS):
               {
                  Plot* plt = (Plot*)cData->data;
                  TextMessage("\n\n   PARAMETER     VALUE\n\n");
                  TextMessage("   parent ...... (%hd,%hd)\n",plt->plotParent->obj->winParent->nr,plt->plotParent->obj->nr());
                  TextMessage("   color ....... %s\n",Plot::GetColorStr(plt->curXAxis()->grid()->color()));
                  TextMessage("   finecolor ... %s\n",Plot::GetColorStr(plt->curXAxis()->grid()->fineColor()));
                  TextMessage("   finexgrid ... %s\n",(plt->curXAxis()->grid()->drawFineGrid()) ? "true" : "false");
                  TextMessage("   fineygrid ... %s\n",(plt->curYAxis()->grid()->drawFineGrid()) ? "true" : "false");
						TextMessage("   xgrid ....... %s\n",(plt->curXAxis()->grid()->drawGrid()) ? "true" : "false");
                  TextMessage("   ygrid ....... %s\n",(plt->curYAxis()->grid()->drawGrid()) ? "true" : "false");

                  break;
               }
               case(AXES_CLASS):
               {
                  Plot* plt = (Plot*)cData->data;
                  TextMessage("\n\n   PARAMETER     VALUE\n\n");
                  TextMessage("   parent ........ (%hd,%hd)\n",plt->plotParent->obj->winParent->nr,plt->plotParent->obj->nr());
                  TextMessage("   autoscale ..... %s\n",(plt->curXAxis()->autoScale())?"true":"false");
                  TextMessage("   axescolor ..... %s\n",Plot::GetColorStr(plt->axesColor));
						TextMessage("   fontcolor ..... %s\n",Plot::GetColorStr(plt->curXAxis()->ticks().fontColor()));
                  TextMessage("   fontname ...... '%s'\n",plt->title().font().lfFaceName);
						TextMessage("   fontsize ...... %hd\n",plt->curXAxis()->ticks().fontSize());
                  TextMessage("   fontstyle ..... '%s'\n",GetFontStyleStr(plt->title().fontStyle()));
						TextMessage("   minaxisvalue .. %g\n",plt->curYAxis()->MinIndep());
						TextMessage("   maxaxisvalue .. %g\n",plt->curYAxis()->MaxIndep());
						TextMessage("   linewidth ..... %hd\n",plt->curXAxis()->lineWidth());
						TextMessage("   lgticksize .... %g\n",plt->getXTicks().majorLength());
						TextMessage("   smticksize .... %g\n",plt->getXTicks().minorLength());
                  TextMessage("   type .......... \"%s\"\n",plt->GetAxesTypeStr());
                  TextMessage("   xdirection .... '%s'\n",(plt->curXAxis()->plotDirection() == PLT_FORWARD) ? "forward":"reversed");
						TextMessage("   xmapping ...... %s\n",plt->curXAxis()->mapping_s());
						TextMessage("   xppmscale ..... %s\n",(plt->curXAxis()->ppmScale())?"true":"false");
                  TextMessage("   xrange ........ %g,%g\n",plt->curXAxis()->Min(),plt->curXAxis()->Max());
                  TextMessage("   xtickspacing .. %g\n",plt->getXTicks().spacing());
                  TextMessage("   xticksperlabel  %g\n",plt->getXTicks().perLabel());
                  TextMessage("   ydirection .... '%s'\n",(plt->curYAxis()->plotDirection() == PLT_FORWARD) ? "forward":"reversed");
                  TextMessage("   ymapping ...... %s\n",plt->curYAxis()->mapping_s());
						TextMessage("   yppmscale ..... %s\n",(plt->curYAxis()->ppmScale())?"true":"false");
                  TextMessage("   yrange ........ %g,%g\n",plt->curYAxis()->Min(),plt->curYAxis()->Max());
                  TextMessage("   ytickspacing .. %g\n",plt->getYTicks().spacing());
                  TextMessage("   yticksperlabel  %g\n",plt->getYTicks().perLabel());

                  break;
               }
               case(TRACE_CLASS):
               {
						Trace *di = (Trace*)cData->data;
                  TracePar *tp = &(di->tracePar);
						TextMessage(formatNoArgHeader().c_str());
						TextMessage(di->FormatState().c_str());
						TextMessage(tp->FormatState().c_str());
                  break;
               }
					case(INSET_CLASS):
					{
						Inset* inset = (Inset*)cData->data;
						TextMessage(formatNoArgHeader().c_str());
						TextMessage(inset->FormatState().c_str());
						break;
					}
            }
         }
         else
         {
            switch(cData->type)
            {
               case(WINDOW_CLASS):
               {
                  TextMessage("%s(window)",prefix);
                  break;
               }
               case(OBJECT_CLASS):
               {
                  TextMessage("%s(object)",prefix);
                  break;
               }
               case(CLASS_ITEM):
               {
                  TextMessage("%s(class)",prefix);
                  break;
               }
               case(PLOT_CLASS):
               {
                  TextMessage("%s(plot)",prefix);
                  break;
               }
               case(XLABEL_CLASS):
               case(YLABEL_CLASS):
               case(YLABEL_LEFT_CLASS):
               case(YLABEL_RIGHT_CLASS):
               case(TITLE_CLASS):
               {
                  TextMessage("%s(plot label)",prefix);
                  break;
               }
               case(TRACE_CLASS):
               {
                  TextMessage("%s(plot trace)",prefix);
                  break;
               }
               case(AXES_CLASS):
               {
                  TextMessage("%s(plot axes)",prefix);
                  break;
               }
               case(GRID_CLASS):
               {
                  TextMessage("%s(plot grid)",prefix);
                  break;
               }
               case(USER_CLASS):
               {
                  TextMessage("%s(plot user)",prefix);
                  break;
               }
               case(INSET_CLASS):
               {
                  TextMessage("%s(plot user)",prefix);
                  break;
               }
					default:
               {
                  TextMessage("%s(unknown)",prefix);
                  break;
               }
            }
         }
         break;
      }
      case(COMPLEX):
      {
         complex temp = VarComplex(var);
         if(temp.i > 0)
            TextMessage("%s%g+%gi",prefix,temp.r,fabs(temp.i));
         else
            TextMessage("%s%g-%gi",prefix,temp.r,fabs(temp.i)); 

         if(var->GetAlias() && pmode == 'v')
            TextMessage(" - (alias to '%s' a %s%s)",var->GetAlias()->GetName(),
                        EnumToString(VARIABLE_SOURCE,var->GetAlias()->GetScope()),suffix);
         else
            TextMessage("%s",suffix);
       
	      break;
      }
      case(CHARACTER):
      {      
         RemoveQuotes(name);
         if(pmode == 'n')
            TextMessage("%s%s",prefix,VarString(var));
         else
            TextMessage("%s\"%s\"",prefix,VarString(var));
            
         if(var->GetAlias() && pmode == 'v')
            TextMessage(" - (alias to '%s' a %s%s)",var->GetAlias()->GetName(),
                        EnumToString(VARIABLE_SOURCE,var->GetAlias()->GetScope()),suffix);
         else
            TextMessage("%s",suffix);
            
         break;
      }

      case(UNQUOTED_STRING): 
      case(QUOTED_STRING):
      {
         char *locstr = VarString(var);
         if(pmode == 'n')
         {
            if(inCLI)
            {
               if(pos == 1)
               {
                  if(FormatTextMessage("\n\n   %s",locstr) == ERR)
                     return(ERR);
               }
               else
               {
                  if(FormatTextMessage("\n   %s",locstr) == ERR)
                     return(ERR);
               }
            }
            else
            {
               if(FormatTextMessage("%s%s",prefix,locstr) == ERR)
                  return(ERR);
            }
	         if(var->GetAlias() && pmode == 'v')
	            TextMessage(" - (alias to '%s' a %s%s)",var->GetAlias()->GetName(),
	                        EnumToString(VARIABLE_SOURCE,var->GetAlias()->GetScope()),suffix);
	         else
	            TextMessage("%s",suffix);            
         }   
         else
         {
            if(FormatTextMessage("%s\"%s\"",prefix,locstr) == ERR)
               return(ERR);
               
	         if(var->GetAlias() && pmode == 'v')
	            TextMessage(" - (alias to '%s' a %s%s)",var->GetAlias()->GetName(),
	                        EnumToString(VARIABLE_SOURCE,var->GetAlias()->GetScope()),suffix);
	         else
	            TextMessage("%s",suffix);                 
         }
         break;  
      }


      case(MATRIX2D):
      { 
         long xsize = VarColSize(var);
         long ysize = VarRowSize(var);
         float **mat = VarRealMatrix(var); 
         char format[50];
         char format2[50];
      
         if(pmode == 'n')
         {
	         sprintf(str,"\n\n   %s = \n\n",name);
	
	         TextMessage(str);
	
        // Print out the matrix
            
				short maxlen = MaxArrayElementLength(&mat[0][0],xsize*ysize);
            sprintf(format,"%%%hdg",maxlen+3);
            sprintf(format2,"%%%hds",maxlen+3);
            bool kcf = gKeepCurrentFocus;
            gKeepCurrentFocus = true;
	         for(long y = 0; y < ysize; y++)
	         {
	            for(long x = 0; x < xsize; x++)
	            {
	               sprintf(str,format,mat[y][x]);

                  if(ProcessBackgroundEvents() != OK)
	               {
	               	y = ysize;
	                  break;
	               }
	               TextMessage(str);
	            }
	            TextMessage("\n");
	         }
            gKeepCurrentFocus = kcf;
         }
         else
         {
            if(var->GetAlias())
               TextMessage("%s(matrix with %ld*%ld elements) - alias to '%s' (%s)",prefix,xsize,ysize,var->GetAlias()->GetName(),EnumToString(VARIABLE_SOURCE,var->GetAlias()->GetScope()));
            else
               TextMessage("%s(matrix with %ld*%ld elements)",prefix,xsize,ysize);
         }         
         break; 
      }
     case(DMATRIX2D):
      { 
         long xsize = VarColSize(var);
         long ysize = VarRowSize(var);
         double **mat = var->GetDMatrix2D(); 
         char format[50];
         char format2[50];
      
         if(pmode == 'n')
         {
	         sprintf(str,"\n\n   %s = \n\n",name);
	
	         TextMessage(str);
	
        // Print out the matrix
            short maxlen = MaxArrayElementLength(&mat[0][0],xsize*ysize);
            sprintf(format,"%%%hdg",maxlen+3);
            sprintf(format2,"%%%hds",maxlen+3);
            bool kcf = gKeepCurrentFocus;
            gKeepCurrentFocus = true;
	         for(long y = 0; y < ysize; y++)
	         {
	            for(long x = 0; x < xsize; x++)
	            {
	               sprintf(str,format,mat[y][x]);

                  if(ProcessBackgroundEvents() != OK)
	               {
	               	y = ysize;
	                  break;
	               }
	               TextMessage(str);
	            }
	            TextMessage("\n");
	         }
            gKeepCurrentFocus = kcf;
         }
         else
         {
            if(var->GetAlias())
               TextMessage("%s(dmatrix with %ld*%ld elements) - alias to '%s' (%s)",prefix,xsize,ysize,var->GetAlias()->GetName(),EnumToString(VARIABLE_SOURCE,var->GetAlias()->GetScope()));
            else
               TextMessage("%s(dmatrix with %ld*%ld elements)",prefix,xsize,ysize);
         }         
         break; 
      }
      case(CMATRIX2D):
      { 
         char format1[50],format2[50];
         long xsize = VarColSize(var);
         long ysize = VarRowSize(var);
         complex **cmat = VarComplexMatrix(var);   
      
         if(pmode == 'n')
         {
	         sprintf(str,"\n\n   %s = \n\n",name);
	
	         TextMessage(str);
				short maxLenReal,maxLenImag;
            MaxCArrayElementLength(&cmat[0][0],xsize*ysize,maxLenReal,maxLenImag);
            sprintf(format1,"%%%hdg + %%%hdgi",maxLenReal+3,maxLenImag);
            sprintf(format2,"%%%hdg - %%%hdgi",maxLenReal+3,maxLenImag);
            bool kcf = gKeepCurrentFocus;
            gKeepCurrentFocus = true;
	         for(long y = 0; y < ysize; y++)
	         {
	            for(long x = 0; x < xsize; x++)
	            {
	               if(cmat[y][x].i >= 0)
	                  sprintf(str,format1,cmat[y][x].r,fabs(cmat[y][x].i));
	               else
	                  sprintf(str,format2,cmat[y][x].r,fabs(cmat[y][x].i));
                  if(ProcessBackgroundEvents() != OK)
	               {
	               	y = ysize;
	                  break;
	               }	
	               TextMessage(str);
	            }
	            TextMessage("\n");
	         }
            gKeepCurrentFocus = kcf;
         }
         else
         {
            if(var->GetAlias())
               TextMessage("%s(complex matrix with %ld*%ld elements) - alias to '%s' (%s)",prefix,xsize,ysize,var->GetAlias()->GetName(),EnumToString(VARIABLE_SOURCE,var->GetAlias()->GetScope()));
            else
               TextMessage("%s(complex matrix with %ld*%ld elements)",prefix,xsize,ysize);
         }
         break; 
      }
      case(MATRIX3D):
      { 
         long xsize = VarColSize(var);
         long ysize = VarRowSize(var);
         long zsize = VarTierSize(var);
         float ***mat = VarReal3DMatrix(var);         
         char format[50];
         char format2[50];

         if(pmode == 'n')
         {
	         sprintf(str,"\n\n   %s = \n\n",name);
	
	         TextMessage(str);

            short maxlen = MaxArrayElementLength(&mat[0][0][0],xsize*ysize*zsize);
            sprintf(format,"%%%hdg",maxlen+3);
            sprintf(format2,"%%%hds",maxlen+3);
            bool kcf = gKeepCurrentFocus;
            gKeepCurrentFocus = true;
	         for(long z = 0; z < zsize; z++)
	         {
	            TextMessage("\n   Plane %ld:\n\n",z);
	         	
		         for(long y = 0; y < ysize; y++)
		         {
		            TextMessage("   ");
		            for(long x = 0; x < xsize; x++)
		            {
                     //if(_isnan(mat[z][y][x]))
	                    // sprintf(str,format2,"NaN");
                     //else if(!_finite(mat[z][y][x]))
	                    // sprintf(str,format2,"Inf");
                     //else
		                  sprintf(str,format,mat[z][y][x]);

                     if(ProcessBackgroundEvents() != OK)
		               {
		               	y = ysize;
		               	z = zsize;
		                  break;
		               }
		               TextMessage(str);
		            }
		            TextMessage("\n");
		         }
		      }
            gKeepCurrentFocus = kcf;
         }
         else
         {
            if(var->GetAlias())
               TextMessage("%s(matrix with %ld*%ld*%ld elements - alias to '%s' (%s))",prefix,xsize,ysize,zsize,var->GetAlias()->GetName(),EnumToString(VARIABLE_SOURCE,var->GetAlias()->GetScope()));
            else
               TextMessage("%s(matrix with %ld*%ld*%ld elements)",prefix,xsize,ysize,zsize);
         }
         break; 
      } 
      case(CMATRIX3D):
      { 
         char format1[50],format2[50];
         long xsize = VarColSize(var);
         long ysize = VarRowSize(var);
         long zsize = VarTierSize(var);
         complex ***cmat = VarComplex3DMatrix(var);   
      
         if(pmode == 'n')
         {
	         sprintf(str,"\n\n   %s = \n\n",name);
	
	         TextMessage(str);

				short maxLenReal,maxLenImag;
            MaxCArrayElementLength(&cmat[0][0][0],xsize*ysize*zsize,maxLenReal,maxLenImag);
            sprintf(format1,"%%%hdg + %%%hdgi   ",maxLenReal,maxLenImag);
            sprintf(format2,"%%%hdg - %%%hdgi   ",maxLenReal,maxLenImag);
            bool kcf = gKeepCurrentFocus;
            gKeepCurrentFocus = true;
	         for(long z = 0; z < zsize; z++)
	         {
	            TextMessage("\n   Plane %ld:\n\n",z);

		         for(long y = 0; y < ysize; y++)
		         {
		            TextMessage("   ");
		            for(long x = 0; x < xsize; x++)
		            {
		               if(cmat[z][y][x].i >= 0)
		                  sprintf(str,format1,cmat[z][y][x].r,fabs(cmat[z][y][x].i));
		               else
		                  sprintf(str,format2,cmat[z][y][x].r,fabs(cmat[z][y][x].i));
                     if(ProcessBackgroundEvents() != OK)
		               {
		               	y = ysize;
		               	z = zsize;
		                  break;
		               }
		               TextMessage(str);
		            }
		            TextMessage("\n");
		         }
		      }
            gKeepCurrentFocus = kcf;
         }
         else
         {
            if(var->GetAlias())
               TextMessage("%s(complex matrix with %ld*%ld*%ld elements - alias to '%s' (%s))",prefix,xsize,ysize,zsize,var->GetAlias()->GetName(),EnumToString(VARIABLE_SOURCE,var->GetAlias()->GetScope()));
            else
               TextMessage("%s(complex matrix with %ld*%ld*%ld elements)",prefix,xsize,ysize,zsize);
         }
         break; 
      }  
      case(MATRIX4D):
      { 
         long xsize = VarWidth(var);
         long ysize = VarHeight(var);
         long zsize = VarDepth(var);
         long qsize = VarHyper(var);
         float ****mat = VarReal4DMatrix(var);   
         char format[50];
         char format2[50];

         if(pmode == 'n')
         {
	         sprintf(str,"\n\n   %s = \n\n",name);
	
	         TextMessage(str);

            short maxlen = MaxArrayElementLength(&mat[0][0][0][0],xsize*ysize*zsize*qsize);
            sprintf(format,"%%%hdg",maxlen+3);
            sprintf(format2,"%%%hds",maxlen+3);
            bool kcf = gKeepCurrentFocus;
            gKeepCurrentFocus = true;
	         for(long q = 0; q < qsize; q++)
            {
	            TextMessage("\n   Hyperplane %ld:\n\n",q);

	            for(long z = 0; z < zsize; z++)
	            {
	               TextMessage("\n      Plane %ld:\n\n",z);
   	         	
		            for(long y = 0; y < ysize; y++)
		            {
		               TextMessage("      ");
		               for(long x = 0; x < xsize; x++)
		               {
   /*                     if(_isnan(mat[q][z][y][x]))
	                        sprintf(str,format2,"NaN");
                        else if(!_finite(mat[q][z][y][x]))
	                        sprintf(str,format2,"Inf");
                        else*/
		                     sprintf(str,format,mat[q][z][y][x]);

                        if(ProcessBackgroundEvents() != OK)
		                  {
		               	   y = ysize;
		               	   z = zsize;
		               	   q = qsize;
		                     break;
		                  }
		                  TextMessage(str);
		               }
		               TextMessage("\n");
		            }
		         }
            }
            gKeepCurrentFocus = kcf;
         }
         else
         {
            if(var->GetAlias())
               TextMessage("%s(matrix with %ld*%ld*%ld*%ld elements - alias to '%s' (%s))",prefix,xsize,ysize,zsize,qsize,var->GetAlias()->GetName(),EnumToString(VARIABLE_SOURCE,var->GetAlias()->GetScope()));
            else
               TextMessage("%s(matrix with %ld*%ld*%ld*%ld elements)",prefix,xsize,ysize,zsize,qsize);
         }
         break; 
      } 
      case(CMATRIX4D):
      { 
         char format1[50],format2[50];
         long xsize = VarWidth(var);
         long ysize = VarHeight(var);
         long zsize = VarDepth(var);
         long qsize = VarHyper(var);
         complex ****cmat = VarComplex4DMatrix(var);   
      
         if(pmode == 'n')
         {
	         sprintf(str,"\n\n   %s = \n\n",name);
	
	         TextMessage(str);
				short maxLenReal,maxLenImag;
            MaxCArrayElementLength(&cmat[0][0][0][0],xsize*ysize*zsize*qsize,maxLenReal,maxLenImag);
            sprintf(format1,"%%%hdg + %%%hdgi   ",maxLenReal,maxLenImag);
            sprintf(format2,"%%%hdg - %%%hdgi   ",maxLenReal,maxLenImag);
            bool kcf = gKeepCurrentFocus;
            gKeepCurrentFocus = true;
	         for(long q = 0; q < qsize; q++)
            {
	            TextMessage("\n   Hyperplane %ld:\n\n",q);

	            for(long z = 0; z < zsize; z++)
	            {
	               TextMessage("\n      Plane %ld:\n\n",z);

		            for(long y = 0; y < ysize; y++)
		            {
		               TextMessage("      ");
		               for(long x = 0; x < xsize; x++)
		               {
		                  if(cmat[q][z][y][x].i >= 0)
		                     sprintf(str,format1,cmat[q][z][y][x].r,fabs(cmat[q][z][y][x].i));
		                  else
		                     sprintf(str,format2,cmat[q][z][y][x].r,fabs(cmat[q][z][y][x].i));
                        if(ProcessBackgroundEvents() != OK)
		                  {
		               	   y = ysize;
		               	   z = zsize;
		               	   q = qsize;
		                     break;
		                  }
		                  TextMessage(str);
		               }
		               TextMessage("\n");
		            }
		         }
            }
            gKeepCurrentFocus = kcf;
         }
         else
         {
            if(var->GetAlias())
               TextMessage("%s(complex matrix with %ld*%ld*%ld*%ld elements - alias to '%s' (%s))",prefix,xsize,ysize,zsize,qsize,var->GetAlias()->GetName(),EnumToString(VARIABLE_SOURCE,var->GetAlias()->GetScope()));
            else
               TextMessage("%s(complex matrix with %ld*%ld*%ld*%ld elements)",prefix,xsize,ysize,zsize,qsize);
         }
         break; 
      }         
   }

   return(0);  
}

short MaxArrayElementLength(float *m, long N)
{
   short maxlen = -1;
   short len;
   char str[50];

   for(long i = 0; i < N; i++)
   {
	   sprintf(str,"%g",m[i]);
      len = strlen(str);
      if(len > maxlen) maxlen = len;
   }
   return(maxlen);
}

short MaxArrayElementLength(double *m, long N)
{
   short maxlen = -1;
   short len;
   char str[50];

   for(long i = 0; i < N; i++)
   {
	   sprintf(str,"%Lg",m[i]);
      len = strlen(str);
      if(len > maxlen) maxlen = len;
   }
   return(maxlen);
}

void MaxCArrayElementLength(complex *m, long N, short &maxLenReal, short &maxLenImag)
{
   maxLenReal = -1;
   maxLenImag = -1;
   short len;
   char str[50];

   for(long i = 0; i < N; i++)
   {
	   sprintf(str,"%g",m[i].r);
      len = strlen(str);
      if(len > maxLenReal) maxLenReal = len;
	   sprintf(str,"%g",m[i].i);
      len = strlen(str);
      if(len > maxLenImag) maxLenImag = len;
   }
}



/****************************************************************
        List all parameters for window obj to the CLI
****************************************************************/

void ListWindowParameters(Interface *itfc, WinData *win)
{
	TextMessage(formatNoArgHeader().c_str());
	TextMessage(win->FormatState().c_str()); 
}

/****************************************************************
        List all parameters for control obj to the CLI
****************************************************************/

void ListObjectParameters(Interface *itfc, ObjectData *obj)
{
   Variable result;
   CText type;
   CText txt,title;

   TextMessage("\n\n   Generic control parameters\n");
	TextMessage(formatNoArgHeader().c_str());
	TextMessage(obj->FormatState().c_str());

   TextMessage("\n\n   Specific control parameters\n");
   TextMessage("\n   PARAMETER     VALUE\n");


   switch(obj->type)
   {	
      case(BUTTON):
      {
         txt = "mode";
         GetButtonParameter(obj,txt,&result);
         obj->GetWindowText(title);
         TextMessage("\n   Mode ................... \"%s\"",result.GetString());
         TextMessage("\n   Label .................. \"%s\"",title.Str());
         break;
      }
      case(SLIDER):
      {
         txt = "value";
         GetSliderParameter(obj,txt,&result);
         TextMessage("\n   Value .................. %g",result.GetReal());
         txt = "range";
         GetSliderParameter(obj,txt,&result);
         float **m = result.GetMatrix2D();
         TextMessage("\n   Range .................. [%g,%g]",m[0][0],m[0][1]);
         txt = "orientation";
         GetSliderParameter(obj,txt,&result);
         TextMessage("\n   Orientation ............ \"%s\"",result.GetString());
         break;
      }
      case(TEXTBOX):
      {
         txt = "text";
         GetTextBoxParameter(itfc,obj,txt,&result);
         TextMessage("\n   Value .................. \"%s\"",result.GetString());
         break;
      }
      case(TEXTMENU):
      {
         GetWindowTextEx(obj->hWnd,txt);
         TextMessage("\n   text ................... \"%s\"",txt.Str());
         TextMessage("\n   index .................. %d",(int)SendMessage(obj->hWnd,CB_GETCURSEL,0,0)+1);
         TextMessage("\n   zindex ................. %d",(int)SendMessage(obj->hWnd,CB_GETCURSEL,0,0));
         break;
      }
      case(TEXTEDITOR):
      {
         EditParent *ep = (EditParent*)obj->data;
         TextMessage("\n   current ................. make this the current editor");
         TextMessage("\n   filename ................ \"%s\"", (ep->editData[ep->curRegion])->edName);
         TextMessage("\n   getcurline .............. get the current line number");
         TextMessage("\n   getcursorpos ............ get the current cursor position");
         TextMessage("\n   gettopline .............. get the top visible line number");
         TextMessage("\n   labelobj ................ %d", (ep->editData[ep->curRegion])->labelCtrlNr);
         TextMessage("\n   modified ................ \"%s\"", (ep->editData[ep->curRegion])->edModified ? "true" : "false");
         TextMessage("\n   pathname ................ \"%s\"", (ep->editData[ep->curRegion])->edPath);
         TextMessage("\n   readonlytext ............ \"%s\"", (ep->editData[ep->curRegion])->readOnly ? "true" : "false");
         TextMessage("\n   scrolltoline ............ scroll to the specified line");
         TextMessage("\n   setcursorpos ............ set the position of the insertion point");
         TextMessage("\n   showcontextualmenu ...... \"%s\"",ep->showContextualMenu ? "true" : "false");
         TextMessage("\n   showsyntaxcoloring ...... \"%s\"",ep->showSyntaxColoring ? "true" : "false");
         TextMessage("\n   showsyntaxdescription ... \"%s\"", ep->showSyntaxDescription ? "true" : "false");
         TextMessage("\n   text .................... set/get current text");
         TextMessage("\n   wordwrap ................ \"%s\"",ep->wordWrap ? "true" : "false");
         break;
      }
      case(PLOTWINDOW):
      {
         PlotWindow *pp = (PlotWindow*)obj->data;
         TextMessage("\n   axesmenu .... %hd",pp->axesMenuNr);
         TextMessage("\n   border ...... cmd : show/hide/toggle border");
         TextMessage("\n   bkgmenu ..... %hd",pp->bkgMenuNr);
         TextMessage("\n   labelmenu ... %hd",pp->labelMenuNr);
         TextMessage("\n   load ........ cmd : load plot(s)");
         TextMessage("\n   margins ..... cmd : set or return margins");
         TextMessage("\n   multiplot ... cmd : make a new plot");
         TextMessage("\n   parent ...... window: %hd",pp->obj->winParent->nr);
         TextMessage("\n   save ........ cmd : save all plots");
         TextMessage("\n   size ........ (%hd,%hd)",pp->cols,pp->rows);
         TextMessage("\n   showborder .. \"%s\"",pp->showLabels ? "true" : "false");
         TextMessage("\n   subplot ..... cmd: select a sub-plot");
         TextMessage("\n   titlemenu ... %hd",pp->titleMenuNr);
         TextMessage("\n   tracemenu ... %hd",pp->traceMenuNr);
	      break;	
      }
      case(IMAGEWINDOW):
      {
         PlotWindow *pp = (PlotWindow*)obj->data;

         TextMessage("\n   axesmenu .... %hd",pp->axesMenuNr);
         TextMessage("\n   border ...... cmd : show/hide/toggle border");
         TextMessage("\n   bkgmenu ..... %hd",pp->bkgMenuNr);
         TextMessage("\n   labelmenu ... %hd",pp->labelMenuNr);
         TextMessage("\n   load ........ cmd : load plot(s)");
         TextMessage("\n   margins ..... cmd : set or return margins");
         TextMessage("\n   multiplot ... cmd : make a new plot");
         TextMessage("\n   parent ...... window: %hd",pp->obj->winParent->nr);
         TextMessage("\n   save ........ cmd : save all plots");
         TextMessage("\n   size ........ (%hd,%hd)",pp->cols,pp->rows);
         TextMessage("\n   showborder .. \"%s\"",pp->showLabels ? "true" : "false");
         TextMessage("\n   subplot ..... cmd: select a sub-plot");
         TextMessage("\n   titlemenu ... %hd",pp->titleMenuNr);
	      break;	
      }
      case(TABCTRL):
      {
         Variable result;
         CText txt;
         extern int GetTabParameter(ObjectData *obj, CText &parameter, Variable *ans);
         txt = "tabname";
         GetTabParameter(obj, txt, &result);
         TextMessage("\n   tabname ....... \"%s\"",result.GetString());
         txt = "tablist";
         GetTabParameter(obj, txt, &result);
         TextMessage("\n   tablist ....... ");
         TextMessage("[\"%s\",",result.GetList()[0]);
         int dim = result.GetDimX();
         if(dim > 1)
         {
            for(int i = 1; i < result.GetDimX()-2; i++)
               TextMessage("\"%s\",",result.GetList()[i]);
            TextMessage("\"%s\"]",result.GetList()[result.GetDimX()-1]);
         }
         else
            TextMessage("]");

         TextMessage("\n   currenttab .... %d",(int)TabCtrl_GetCurSel(obj->hWnd));
         TextMessage("\n   zindex ........ %d",(int)TabCtrl_GetCurSel(obj->hWnd));
         TextMessage("\n   index ......... %d",(int)TabCtrl_GetCurSel(obj->hWnd)+1);
	      break;	
      }
      case(LISTBOX):
      {
         txt = "text";
         GetListBoxParameter(itfc,obj,txt,&result);
         TextMessage("\n   text ........ \"%s\"",(char*)result.GetString());
         TextMessage("\n   index ....... %d",(int)SendMessage(obj->hWnd,LB_GETCURSEL,0,0)+1);
         TextMessage("\n   zindex ...... %d",(int)SendMessage(obj->hWnd,LB_GETCURSEL,0,0));
         TextMessage("\n   topindex .... %d",(int)SendMessage(obj->hWnd,LB_GETTOPINDEX,0,0)+1);
         TextMessage("\n   topzindex ... %d", (int)SendMessage(obj->hWnd, LB_GETTOPINDEX, 0, 0));
         txt = "selection";
         GetListBoxParameter(itfc, obj, txt, &result);
         float** selection = result.GetMatrix2D();
         TextMessage("\n   selection ... %d->%d", nint(selection[0][0]),nint(selection[0][1]));
         break;
      }
   }
}

void ListPlotRegionParameters(Plot* plt)
{
    Plot1D *pt1d = dynamic_cast<Plot1D*>(plt);

   if(plt->getDimension() == 1)
   {
      TextMessage("\n #### 1D plot-region object ####\n");
      TextMessage("\n   PARAMETER           VALUE\n\n");
		TextMessage("   antialiasing1d..... \"%s\"\n",plt->isAntiAliasing() ? "true" : "false");
      TextMessage("   autorange ......... \"%s\"\n",plt->getOverRideAutoRange() ? "false" : "true");
      TextMessage("   axes .............. object : defines the plot axes\n");
      TextMessage("   bordercolor ....... %s\n",Plot::GetColorStr(plt->borderColor));
      TextMessage("   bkgcolor .......... %s\n",Plot::GetColorStr(plt->bkColor));
      TextMessage("   clear ............. cmd : clear all trace data\n");
      TextMessage("   dim ............... \"%s\"\n",(plt->getDimension() == 1) ? "1d" : "2d");
      TextMessage("   draw .............. \"%s\"\n",plt->plotParent->updatePlots() ? "true" : "false");
		TextMessage("   filename .......... \"%s\"\n",plt->getFileName());
      TextMessage("   filepath .......... \"%s\"\n",plt->getFilePath());
      if(plt->getFileVersion() == 0)
         TextMessage("   fileversion ....... file not loaded\n");
      else
       TextMessage("   fileversion ....... %1.1f\n",plt->getFileVersion()/100.0);
      TextMessage("   fullregion ........ cmd : show all data\n");
      TextMessage("   getdata ........... cmd : extract (x,y) data\n");
      TextMessage("   grid .............. object : defines the plot grids\n");
      TextMessage("   hold .............. \"%s\"\n",(plt->displayHold) ? "true" : "false");
      TextMessage("   load .............. cmd : load a pt1 file into plot region\n");
      TextMessage("   margins ....... ... cmd : set or return margins for plot region\n");
      TextMessage("   parent ............ control: %hd\n",plt->plotParent->obj->nr());
      TextMessage("   plot .............  cmd : display (x,y) data\n");
      TextMessage("   position .......... (%hd,%hd)\n",plt->colNr+1,plt->rowNr+1);
      TextMessage("   border ............ \"%s\"\n",(plt->plotParent->isShowLabels()) ? "true" : "false");
      TextMessage("   save .............. cmd : save plot region contents to a pt1 file\n");
      TextMessage("   trace ............. object : defines a trace \n");
      TextMessage("   tracelist ......... cmd : return array of trace id numbers\n");
      TextMessage("   tracepref ......... cmd : set or get trace drawing preferences\n");
      TextMessage("   rmtrace ........... cmd : remove trace by id\n");
      TextMessage("   filtertrace ....... cmd : filter trace data or not: \"%s\"\n", (pt1d->IsFiltered()) ? "true" : "false");
      TextMessage("   showimag .......... cmd : show or hide imaginary data\n");
      TextMessage("   showreal .......... cmd : show or hide real data\n");
      TextMessage("   showlegend ........ cmd : show or hide the trace legend\n");
      TextMessage("   curtrace .......... object : returns trace object\n");
      TextMessage("   currentaxis ....... cmd : which is the current vertical axis?\n");
      TextMessage("   syncaxes .......... cmd : align vertical axes\n");
      TextMessage("   lockgrid .......... cmd : lock grid to axis\n");
      TextMessage("   tracexborder ...... cmd : specifies\n");
      TextMessage("   traceyborder ...... cmd : specifies\n");
      TextMessage("   pasteinfo ......... cmd : add a trace to the plot from clipboard\n");
      TextMessage("   title ............. object : defines the plot title\n");
      TextMessage("   xlabel ............ object : defines the plot x-label\n");
      TextMessage("   ylabel ............ object : defines the plot y-label\n");
      TextMessage("   zoom .............. cmd : set viewing limits\n");
      TextMessage("   zoomrectmode ...... \"%s\"\n",(plt->zoomRectMode));
      TextMessage("   zoombkgcolor ...... %s\n",Plot::GetAlphaColorStr(plt->zoomBkgColor));
      TextMessage("   zoombordercolor ... %s\n",Plot::GetAlphaColorStr(plt->zoomBorderColor));
   }
   else
   {
      TextMessage("\n #### 2D plot-region object ####\n");
      TextMessage("\n   PARAMETER           VALUE\n\n");
      TextMessage("   autorange ......... \"%s\"\n",plt->getOverRideAutoRange() ? "false" : "true");
      TextMessage("   axes .............. object\n");
      TextMessage("   bordercolor ....... %s\n",Plot::GetColorStr(plt->borderColor));
      TextMessage("   bkgcolor .......... %s\n",Plot::GetColorStr(plt->bkColor));
      TextMessage("   contour ........... cmd : display image data\n");
      TextMessage("   contourlinewidth .. %g\n", static_cast<Plot2D*>(plt)->contourLineWidth);
      TextMessage("   dim ............... \"%s\"\n",(plt->getDimension() == 1) ? "1d" : "2d");
		TextMessage("   filename........... \"%s\"\n",plt->getFileName());
		TextMessage("   filepath........... \"%s\"\n",plt->getFilePath());
      if(plt->getFileVersion() == 0)
         TextMessage("   fileversion ....... file not loaded\n");
      else
         TextMessage("   fileversion ....... %1.1f\n",plt->getFileVersion()/100.0);
      TextMessage("   getdata ........... cmd : extract image data\n");
      TextMessage("   grid .............. object\n");
      TextMessage("   imagerange......... [%g,%g]\n",static_cast<Plot2D*>(plt)->minVal(),
																		static_cast<Plot2D*>(plt)->maxVal());
      TextMessage("   load .............. cmd : load a pt2 file into plot region\n");
      TextMessage("   margins ....... ... cmd : set or return margins for plot region\n");
      TextMessage("   parent ............ control: %hd\n",plt->plotParent->obj->nr());
      TextMessage("   position .......... (%hd,%hd)\n",plt->colNr+1,plt->rowNr+1);
      TextMessage("   border ............ \"%s\"\n",(plt->plotParent->isShowLabels()) ? "true" : "false");
      TextMessage("   save .............. cmd : save plot region contents to a pt2 file\n");
      TextMessage("   title ............. object\n");
      TextMessage("   xlabel ............ object\n");
      TextMessage("   ylabel ............ object\n");
      TextMessage("   zoom .............. cmd : set viewing limits\n");
      TextMessage("   zoomrectmode ...... \"%s\"\n",(plt->zoomRectMode));
      TextMessage("   zoombkgcolor ...... %s\n",Plot::GetAlphaColorStr(plt->zoomBkgColor));
      TextMessage("   zoombordercolor ... %s\n",Plot::GetAlphaColorStr(plt->zoomBorderColor));
   }
}


/************************************************************************************
   User has asked to abort a macro, also send a message to the CLI
*************************************************************************************/

int Abort(Interface* itfc ,char args[])
{
   short n;
   CText message;
   
   if((n = ArgScan(itfc,args,1,"message","e","t",&message)) < 0)
      return(n);

// If aborting and there is no main window then exit Prospa
   if(!prospaWin) 
      exit(0);

   if(message.Size() > 0)
      TextMessage("Abort: %s\n",message.Str());

   return(ABORT);
}


int MakeErrorString(Interface* itfc ,char args[])
{
	short r;
	float number,error;
	short res = 1;
	float mantissaNum,mantissaErr;
	short exponentNum,exponentErr;
	char format[100];
	char out[100];
	float factor;
	short digits;
	char mantissaStr[100],exponentStr[100];
	short modeSwitch = 5;
	char destination[100];

	strcpy(destination,"plot");

	if((r = ArgScan(itfc,args,2,"number, error, [resolution], [mode switch], [destination]","eeeee","ffdds",&number,&error,&res,&modeSwitch,destination)) < 0)
		return(r); 

	// Make sure the number of significant digits in the error is at least 1 and not too many
	if(res < 1 || res > 7)
	{
		ErrorMessage("invalid number of error resolution");
		return(ERR);
	}

	// Make sure power for switching between print modes is realistic
	if(modeSwitch < 0)
	{
		ErrorMessage("invalid mode switch power");
		return(ERR);
	}

	// Separate out the mantissa and exponent for each number
	FloatSplit(error,mantissaStr,exponentStr,7);
	sscanf(mantissaStr,"%f",&mantissaErr);
	sscanf(exponentStr,"%hd",&exponentErr);

	FloatSplit(number,mantissaStr,exponentStr,7);
	sscanf(mantissaStr,"%f",&mantissaNum);
	sscanf(exponentStr,"%hd",&exponentNum);

	// Make sure the number and error are rounded correctly
	factor = pow(10.0,(res+exponentNum-exponentErr-1));
	number = nint(mantissaNum*factor)/factor*pow(10.0,exponentNum);
	factor = pow(10.0,res-1);
	error = nint(mantissaErr*factor)/factor*pow(10.0,exponentErr);

	// Print out for small numbers (0.0001 to 999999)
	if(exponentNum < modeSwitch && exponentNum > -modeSwitch)
	{
		digits = res-1-exponentErr;
		if(!strcmp(destination,"plot"))
		{
			if(digits >= 0) // Error is less than number
				sprintf(format,"(%%1.%hdf ± %%1.%hdf)",digits,digits);
			else // Error is bigger than number
				sprintf(format,"(%%1.0f ± %%1.0f)");
			sprintf(out,format,number,error);
		}
		else
		{
			if(digits >= 0) // Error is less than number
				sprintf(format,"(%%1.%hdf +- %%1.%hdf)",digits,digits);
			else // Error is bigger than number
				sprintf(format,"(%%1.0f +- %%1.0f)");
			sprintf(out,format,number,error);
		}
		itfc->retVar[1].MakeAndSetString(out);
	}
	else
	{
		FloatSplit(error,mantissaStr,exponentStr,7);
		sscanf(mantissaStr,"%f",&mantissaErr);
		sscanf(exponentStr,"%hd",&exponentErr);

		FloatSplit(number,mantissaStr,exponentStr,7);
		sscanf(mantissaStr,"%f",&mantissaNum);
		sscanf(exponentStr,"%hd",&exponentNum);

		if(!strcmp(destination,"plot"))
		{
			if(exponentNum >= exponentErr) // Error is smaller than number
			{
				mantissaErr *= pow(10.0,exponentErr - exponentNum);
				digits = exponentNum-exponentErr+res-1;
				sprintf(format,"(%%1.%hdf ± %%1.%hdf) × 10^(%hd)",digits,digits,exponentNum);
			}
			else // Error is bigger than number
			{
				digits = res-1;
				mantissaNum /= pow(10.0,exponentErr - exponentNum);
				sprintf(format,"(%%1.%hdf ± %%1.%hdf) × 10^(%hd)",digits,digits,exponentErr);
			}
		}
		else
		{
			if(exponentNum >= exponentErr) // Error is smaller than number
			{
				mantissaErr *= pow(10.0,exponentErr - exponentNum);
				digits = exponentNum-exponentErr+res-1;
				sprintf(format,"(%%1.%hdf +- %%1.%hdf) × 1e%hd",digits,digits,exponentNum);
			}
			else // Error is bigger than number
			{
				digits = res-1;
				mantissaNum /= pow(10.0,exponentErr - exponentNum);
				sprintf(format,"(%%1.%hdf +- %%1.%hdf) × 1e%hd",digits,digits,exponentErr);
			}
		}

		sprintf(out,format,mantissaNum,mantissaErr);
		itfc->retVar[1].MakeAndSetString(out);
	}
	itfc->nrRetValues = 1;

	return(OK);
}


/************************************************************************************
   Controls the abortOnError flag. If this is set then when an error is detected the
   current macro will abort. If it is false the macro will continue processing
   and the error will be ignored and not reported.
*************************************************************************************/

int AbortOnError(Interface* itfc ,char arg[])
{
   short n;
   CText abort;

   if((n = ArgScan(itfc,arg,1,"true/false","e","t",&abort)) < 0)
      return(n);
   
	if(abort == "true")
	{
	   abortOnError = true;
	}
	else if(abort == "false")
	{
	   abortOnError = false;
	}
	else
	{
	   ErrorMessage("invalid argument (true/false)");
	   return(ERR);
	}	
   itfc->nrRetValues = 0;
   return(OK);
}


/***************************************************************************  
 Send a message (with standard arguments) to the CLI from a Prospa command  
          Replace certain characters first so that formating is correct
          e.g. \n will move insertion point to start of a new line
               \r will move insertion point to start of current line
***************************************************************************/


short FormatTextMessage(char *text, ...)
{
   va_list ap;
   char *output;
   char replace[2];
  
   va_start(ap,text);

// Replace arguments with values
   output = vssprintf(text,ap);


// Replace special characters or pairs of characters
 //  ReplaceSpecialCharacters(output,"\\n","\n");   // Replace \n with <cr><lf> (new line)
 //  ReplaceSpecialCharacters(output,"\\r","\r");   // Replace \r with <cr> (move to start of line)
 //  ReplaceSpecialCharacters(output,"\\t","\t");   // Replace \t with <tab> (tab space)
   replace[0] = (char)0xB1; replace[1] = '\0';
   ReplaceSpecialCharacters(output,"+-",replace,-1); // Replace +- with +- symbol

// Send to CLI
   SendTextToCLI(output); 
 
   va_end(ap);

   delete [] output;
   return(OK);
}

/*************************************************************************
*       Extract variable name and value from str if it has the form
        variable = value which is not quoted
        Note that str is modified as spaces are removed.
*************************************************************************/

short ParseAssignmentString(char *str, char *name, char *value)
{
   bool inString = false;
   RemoveUnquotedCharacter(str,' '); // Remove white space outside quotes
   int sz = strlen(str);
   long pos = -1;
   for(int i = 0; i < sz; i++)
   {
      if(!inString && str[i] == '"')
      {
         inString = true;
         continue;
      }

      if(inString && str[i] == '"')
      {
         inString = false;
         continue;
      }

      if(inString && str[i] == '=')
      {
         continue;
      }

      if(!inString && i > 0 && i >= sz-2 && str[i] == '=' && str[i-1] != '!' && str[i-1] != '=') // Look for assignment but ignore != or ==
      {
         pos = i;
         break;
      }

      if(!inString && i > 0 && i < sz-2 && str[i] == '=' && str[i-1] != '!' && str[i-1] != '=' && str[i+1] != '=') // Look for assignment but ignore != or ==
      {
         pos = i;
         break;
      }
   }

	if(pos == -1) // Not an assignment so just return the str in name
   {
      strcpy(name,str);
      strcpy(value,"");
      return(ERR);
   }
   else // An assignment exists
   {
	   LeftStr(str,pos-1,name);
	   RightStr(str,pos+1,value);
   }
	return(OK);
}


void SimpleErrorMessage(char *text)
{
	ErrorMessage(text);
}

void AddProcedures(Interface *itfc, char *path, char* name)
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
      procVar = itfc->baseMacro->GetProcedure(path,name,procName);
      if(procVar) // Remove previous cached procedure
		{
         procVar->Remove();
			delete procVar;
		}
		procVar = itfc->baseMacro->procList.Add(PROCEDURE,procName);
		procVar->MakeAndSetProcedure(procedure,procName,fileName,path,startLine);
		delete [] procedure;
	}
	delete [] text;
}

// List all macros in the window, local or global caches

int ListCachedProcedures(Interface *itfc,  char args[])
{
   CText mode = "global";
   short nrArgs;
	CText path = "";
	CText macro = "";
	CText macroFilter = "";
	extern Variable gCachedProc;

	// Get filename from user  
	if((nrArgs = ArgScan(itfc,args,0,"mode, [macro]","ee","tt",&mode,&macroFilter)) < 0)
		return(ERR); 

   if(mode == "window")
   {
      TextMessage("\n\n   ### Window macro cache ###\n\n");
      Variable *procList = &(itfc->win->procList);

	   for(Variable *var = procList->next; var != NULL; var = var->next)
	   {
         ProcedureInfo *procInfo = (ProcedureInfo*)var->GetData();

			if(macroFilter == "" | macroFilter == procInfo->macroName)
			{
				if(procInfo->macroName != macro)
					TextMessage("   Macro = '%s' (Path = '%s')\n",procInfo->macroName,procInfo->macroPath);

				macro = procInfo->macroName;
				path = procInfo->macroPath;

				TextMessage("            Procedure = '%s'\n",procInfo->procName);
			}
      }
      TextMessage("\n");
   }
   else if(mode == "local")
   {
      TextMessage("\n\n   ### Local macro cache ###\n\n");
      Variable *procList = &(itfc->baseMacro->procList);

	   for(Variable *var = procList->next; var != NULL; var = var->next)
	   {
         ProcedureInfo *procInfo = (ProcedureInfo*)var->GetData();

			if(macroFilter == "" | macroFilter == procInfo->macroName)
			{
				if(procInfo->macroName != macro)
					TextMessage("   Macro = '%s' (Path = '%s')\n",procInfo->macroName,procInfo->macroPath);

				macro = procInfo->macroName;
				path = procInfo->macroPath;

				TextMessage("            Procedure = '%s'\n",procInfo->procName);
			}
		}
      TextMessage("\n");
   }
   else if(mode == "global")
   {
		ListGlobalsCachedProcs(macroFilter);
   }
	itfc->nrRetValues = 0;
   return(OK);
}

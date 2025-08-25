#include "stdafx.h"
#include "scanstrings.h"
#include "cArg.h"
#include "command_other.h"
#include "evaluate.h"
#include "globals.h"
#include "interface.h"
#include "list_functions.h"
#include "mymath.h"
#include "string_utilities.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include <assert.h>
#include "memoryLeak.h"
#define COMMA ','
#define SPACE ' '

class DLLParameters
{
public:
   short nrRetVar;
   Variable *retVar;
   Interface *itfc;
};

// Externally accessible
char promptArg[MAX_STR];

bool IsVarString(char *s);
void ListArguments(char*, char*, char*);
int ProcessEscapeCharacters(Interface* itfc, char args[]);
int CompareStringsCore(CText &src, CText &ref);
char* ExtractString(char str[],int nr); 
int CountStrings(char[]);
void RemoveTrailingBlanks(char*);
bool ishex(char c);
bool IsADigit(char c);
bool SingleArgument(char *arg);
char* GetStrSub(char*, long, long);
void StrNCat(char *A, char *B, short N);
void AddEscapedQuotes(char* out, char* in);
void ReplaceEscapedQuotes(char str[]);
EXPORT int UpdateProspaArgumentVariables(Interface *itfc, char *args);
EXPORT int UpdateProspaReturnVariables(DLLParameters *par, Interface *itfc);
int TrimString(Interface *itfc, char arg[]);

//bool processEscapes = true; // Process all escape character as they are read into Prospa
                            // either via the keyboard, clipboard or a file

short ArgScan(void *par, char argStr[], short minNr, char prompt[], char dataType[], char argType[], ...)
{
   short nrArgsRequired;
   va_list ap;
   short cnt = 0;
   bool err = false;
   long len;
   char *argument;
   CArg carg;
   CText *txt;
   Variable result;
   Interface *itfc  = (Interface*)par;
   if(argStr[0] == '\0' && minNr == 0)
      return(0);

   int oldNrRetValues = itfc->nrRetValues;

// Start of variable argument list
   va_start(ap,argType);

// Minimum number of parameters which should have been passed
   nrArgsRequired = strlen(dataType);

// Allocate space for argument string
   len = strlen(argStr)+1;
   if(len < 200) len = 200;
   argument = new char[len];

   if(nrArgsRequired > 0)
   {  
      if(argStr[0] == '\0' || carg.GetNext(argStr,argument) != ERR)
      {
      // Check to see if user wants to return defaults
	      if(!strcmp(argStr,"\"getargs\"") || argStr[0] == '\0')
	      {
	         Variable *var;
   	      
	         for(short i = 1; i <= nrArgsRequired; i++)
	         {
               var = &itfc->retVar[i];
   	      
	            switch(argType[i-1])
	            {
	               case('d'):
                     var->MakeAndSetFloat((float)*(va_arg(ap,short*)));
	                  break;
	               case('f'):
                     var->MakeAndSetFloat((float)*(va_arg(ap,float*)));
	                  break;
	               case('l'):
                     var->MakeAndSetFloat((float)*(va_arg(ap,long*)));
	                  break;
	               case('s'):
                     var->MakeAndSetString(va_arg(ap,char*));
	                  break;
                  case('t'):
                     txt = va_arg(ap,CText*);
                     var->MakeAndSetString(txt->Str());
                     break;
                  case('v'):
                     if(CopyVariable(var,(Variable*)(va_arg(ap,Variable*)),FULL_COPY) == ERR)
                        return(ERR);
                     break;
	            }
	         }	
            itfc->nrRetValues = nrArgsRequired; 
	         delete[] argument;
	         return(-2);
	      }
         
	   // Extract data from arguments *********************/
	      for(short i = 0; i < nrArgsRequired; i++)
         {
	         err = false;

            if(i > 0)
               carg.GetNext(argStr,argument);

	         if(argument[0] == '\0')
	            goto ex;
   	
	         if(dataType[i] == 'e')  // An expression may have been passed
	         {
               short type = Evaluate(itfc,RESPECT_ALIAS,argument,&result); // Evaluate expression
          //     nrReturnedValues = 1; // Only ansVar is used here
               if(type == ERR)
               {
	               delete[] argument;
                  return(ERR);
               }

               switch(argType[i])
               {
                  case('s'): // String
                     if(type == UNQUOTED_STRING || type == CHARACTER)
                     {  
                        char *temp = va_arg(ap,char*);
                        strcpy(temp,VarString(&result));
                     }
                     else
                     {
               	      ErrorMessage("invalid type for argument %d in (%s)",i+1,argStr);
               	      err = true;
               	      goto ex;
               	   }	                  
                     break;
                  case('t'): // CText
                     if(type == UNQUOTED_STRING || type == CHARACTER)
                     {  
                        txt = va_arg(ap,CText*);
                        txt->Assign(VarString(&result));
                     }
                     else
                     {
               	      ErrorMessage("invalid type for argument %d in (%s)",i+1,argStr);
               	      err = true;
               	      goto ex;
               	   }	                  
                     break;
                  case('l'): // Long integer
               	   if(type == FLOAT32)  
		               {
               	      *va_arg(ap,long*) = nint(VarReal(&result));
               	   }
               	   else if(type == INTEGER)  
		               {
               	      *va_arg(ap,long*) = VarInteger(&result);
               	   }               	
               	   else
               	   {
               	      ErrorMessage("invalid type for argument %d in (%s)",i+1,argStr);
         	            err = true;
               	      goto ex;
               	   }
                     break;
                  case('d'): // Short integer
               	   if(type == FLOAT32)  
		               {
               	      *va_arg(ap,short*) = (short)nint(VarReal(&result));
               	   }
               	   else if(type == INTEGER)  
		               {
               	      *va_arg(ap,short*) = (short)VarInteger(&result);
               	   }  
               	   else
               	   {
               	      ErrorMessage("invalid type for argument %d in (%s)",i+1,argStr);
         	            err = true;
               	      goto ex;
               	   }
                     break;
                  case('c'): // Character
               	   if(type == CHARACTER)  
		               {
               	      *va_arg(ap,char*) = VarString(&result)[0];
               	   }
               	   else
               	   {
               	      ErrorMessage("invalid type for argument %d in (%s)",i+1,argStr);
         	            err = true;
               	      goto ex;
               	   }
                     break;
                  case('f'): // Float
               	   if(type == FLOAT32)  
		               {
               	      *va_arg(ap,float*) = VarReal(&result);
               	   }
               	   else if(type == INTEGER)  
		               {
               	      *va_arg(ap,float*) = (float)VarInteger(&result);
               	   } 
                	   else
               	   {
               	      ErrorMessage("invalid type for argument %d",i+1);
         	            err = true;
               	      goto ex;
               	   }
                     break;  
                  case('F'): // double
               	   if(type == FLOAT64)  
		               {
                        *va_arg(ap,double*) = result.GetDouble();
               	   }
               	   else if(type == INTEGER)  
		               {
               	      *va_arg(ap,double*) = (double)VarInteger(&result);
               	   } 
                	   else
               	   {
               	      ErrorMessage("invalid type for argument %d",i+1);
         	            err = true;
               	      goto ex;
               	   }
                     break;   
                  case('q'): // Convert evaluated data to a string
                  {
                     CText *txt = va_arg(ap,CText*);
                     switch(type)
                     {
               	      case(FLOAT32): 
                           if(IsInteger(VarReal(&result)))
                           {
                              txt->Format("%ld",nint(VarReal(&result)));
                              break;
                           }
                           txt->Format("%g",VarReal(&result)); break;
               	      case(UNQUOTED_STRING):
                           txt->Format("%s",VarString(&result)); break;
               	      case(CHARACTER):
                           txt->Format("%c",VarString(&result)[0]); break;
                        default:
                           txt->Format("%s",argument); break;
               	   }
                     break;
                  }
                  case('v'): // Assign evaluated data to passed variable
                  {
                	   Variable * var = va_arg(ap,Variable*);
               //      var->CopyWithAlias(&result);
                     var->Assign(&result);
                     result.NullData();
                     var->SetName(argument);
                     break;
                  }
                                                     
                  default:	 
                     ErrorMessage("undefined variable '%s'",argument);
         	         err = true;
                     break;
               }
	            cnt++;
		      }
		      else if(dataType[i] == 'c') // Data should not be changed apart from removing quotes and white space
		      {
	            RemoveQuotes(argument);
               RemoveEndBlanks(argument);
               	           
	            switch(argType[i])
	            {
                  case('t'): // CText
                  {
                     txt = va_arg(ap,CText*);
                     txt->Assign(argument);
                     cnt++;
                     break;
                  }
	               case('s'):
		            {
		               strcpy(va_arg(ap,char*),argument);
		               cnt++;
		               break;
		            }
		            case('c'): 
	                  if(sscanf(argument,"%c",va_arg(ap,char*)) == 1)
	                     cnt++;
	                  break;
                  case('q'): // String list
		            {
                     short e;
                     va_arg(ap,char*)[0] ='\0';
                     do
                     {
                        e = carg.GetNext(argStr,argument);
                        strcat(va_arg(ap,char*),argument);
		                  if(e != FINISH)
		                     strcat(va_arg(ap,char*),";");
                     }
                     while(e == OK);
		               cnt++;
		               break;
		            }
		         }
	         }
	      }
      }
	
ex:   va_end(ap);
	   delete[] argument;

	   if(err == false && cnt < minNr)
	   {
         if(cnt == 0 && itfc->inCLI)
         {
            ListArguments(prompt,dataType,argType);
            itfc->nrRetValues = 0;
            return(-2);
         }
	      ErrorMessage("missing arguments (at least %d required)",minNr);
	      return(ERR);
	   }
	   	   
      if(err == true)
         return(ERR);
         
	   return(cnt);
	}

   itfc->nrRetValues = oldNrRetValues; // Restore number of returned values.

   return(0);
}

/************************************************************************************
   Extract arguments from argStr and pass results back in passed parameters

   argStr   : list of comma delimited arguments
   minNr    : the minimum number of acceptable arguments
   prompt   : what to print if no arguments passed
   dataType : type of data; e = expression, c = constant
   argType  : type of argument; s = string, f = float, d = short integer, t = CText,
                              l = long integer, v = variable,  q = convert to string
/************************************************************************************/

short ArgScan(Interface *itfc, char argStr[], short minNr, char prompt[], char dataType[], char argType[], ...)
{
   short nrArgsRequired;
   va_list ap;
   short cnt = 0;
   bool err = false;
   long len;
   char *argument;
   CArg carg;
   CText *txt;
   Variable result;
   int oldNrRetValues;

   if(argStr[0] == '\0' && minNr == 0)
      return(0);

// Check for valid interface 
   assert(itfc);
   
// Record the number of returned values
   oldNrRetValues = itfc->nrRetValues ;

// Start of variable argument list
   va_start(ap,argType);

// Minimum number of parameters which should have been passed
   nrArgsRequired = strlen(dataType);

// Allocate space for argument string
   len = strlen(argStr)+1;
   if(len < 200) len = 200;
   argument = new char[len];

   if(nrArgsRequired > 0)
   {  
      if(argStr[0] == '\0' || carg.GetNext(argStr,argument) != ERR)
      {
      // Check to see if user wants to return defaults
	      if(!strcmp(argStr,"\"getargs\"") || argStr[0] == '\0')
	      {
	         Variable *var;
   	      
	         for(short i = 1; i <= nrArgsRequired; i++)
	         {
               var = &itfc->retVar[i];
   	      
	            switch(argType[i-1])
	            {
	               case('d'):
                     var->MakeAndSetFloat((float)*(va_arg(ap,short*)));
	                  break;
	               case('f'):
                     var->MakeAndSetFloat((float)*(va_arg(ap,float*)));
	                  break;
	               case('l'):
                     var->MakeAndSetFloat((float)*(va_arg(ap,long*)));
	                  break;
	               case('s'):
                     var->MakeAndSetString(va_arg(ap,char*));
	                  break;
                  case('t'):
                     txt = va_arg(ap,CText*);
                     var->MakeAndSetString(txt->Str());
                     break;
                  case('v'):
                     if(CopyVariable(var,(Variable*)(va_arg(ap,Variable*)),FULL_COPY) == ERR)
                        return(ERR);
                     break;
	            }
	         }	
            itfc->nrRetValues = nrArgsRequired; 
	         delete[] argument;
	         return(-2);
	      }
         
	   // Extract data from arguments *********************/
	      for(short i = 0; i < nrArgsRequired; i++)
         {
	         err = false;

            if(i > 0)
               carg.GetNext(argStr,argument);

	         if(argument[0] == '\0')
	            goto ex;
   	
	         if(dataType[i] == 'e')  // An expression may have been passed
	         {
					short type = Evaluate(itfc,RESPECT_ALIAS,argument,&result); // Evaluate expression
          //     nrReturnedValues = 1; // Only ansVar is used here
               if(type == ERR)
               {
	               delete[] argument;
                  return(ERR);
               }

               switch(argType[i])
               {
                  case('s'): // String
                     if(type == UNQUOTED_STRING || type == CHARACTER)
                     {  
                        char *temp = va_arg(ap,char*);
                        strcpy(temp,VarString(&result));
                     }
                     else
                     {
               	      ErrorMessage("invalid type for argument %d in (%s)",i+1,argStr);
               	      err = true;
               	      goto ex;
               	   }	                  
                     break;
                  case('t'): // CText
                     if(type == UNQUOTED_STRING || type == CHARACTER)
                     {  
                        txt = va_arg(ap,CText*);
                        txt->Assign(VarString(&result));
                     }
                     else
                     {
               	      ErrorMessage("invalid type for argument %d in (%s)",i+1,argStr);
               	      err = true;
               	      goto ex;
               	   }	                  
                     break;
                  case('l'): // Long integer
               	   if(type == FLOAT32)  
		               {
               	      *va_arg(ap,long*) = nint(VarReal(&result));
               	   }
               	   else if(type == INTEGER)  
		               {
               	      *va_arg(ap,long*) = VarInteger(&result);
               	   }               	
               	   else
               	   {
               	      ErrorMessage("invalid type for argument %d in (%s)",i+1,argStr);
         	            err = true;
               	      goto ex;
               	   }
                     break;
                  case('d'): // Short integer
               	   if(type == FLOAT32)  
		               {
               	      *va_arg(ap,short*) = (short)nint(VarReal(&result));
               	   }
               	   else if(type == INTEGER)  
		               {
               	      *va_arg(ap,short*) = (short)VarInteger(&result);
               	   }  
               	   else
               	   {
               	      ErrorMessage("invalid type for argument %d in (%s)",i+1,argStr);
         	            err = true;
               	      goto ex;
               	   }
                     break;
                  case('c'): // Character
               	   if(type == CHARACTER)  
		               {
               	      *va_arg(ap,char*) = VarString(&result)[0];
               	   }
               	   else
               	   {
               	      ErrorMessage("invalid type for argument %d in (%s)",i+1,argStr);
         	            err = true;
               	      goto ex;
               	   }
                     break;
                  case('f'): // Float
               	   if(type == FLOAT32)  
		               {
               	      *va_arg(ap,float*) = VarReal(&result);
               	   }
               	   else if(type == INTEGER)  
		               {
               	      *va_arg(ap,float*) = (float)VarInteger(&result);
               	   } 
                	   else
               	   {
               	      ErrorMessage("invalid type for argument %d",i+1);
         	            err = true;
               	      goto ex;
               	   }
                     break;   
                  case('F'): // double
               	   if(type == FLOAT64)  
		               {
                        *va_arg(ap,double*) = result.GetDouble();
               	   }
               	   else if(type == INTEGER)  
		               {
               	      *va_arg(ap,double*) = (double)VarInteger(&result);
               	   } 
                	   else
               	   {
               	      ErrorMessage("invalid type for argument %d",i+1);
         	            err = true;
               	      goto ex;
               	   }
                     break;  
                  case('q'): // Convert evaluated data to a string
                  {
                     CText *txt = va_arg(ap,CText*);
                     switch(type)
                     {
               	      case(FLOAT32): 
                           if(IsInteger(VarReal(&result)))
                           {
                              txt->Format("%ld",nint(VarReal(&result)));
                              break;
                           }
                           txt->Format("%g",VarReal(&result)); break;
               	      case(UNQUOTED_STRING):
                           txt->Format("%s",VarString(&result)); break;
               	      case(CHARACTER):
                           txt->Format("%c",VarString(&result)[0]); break;
                        default:
                           txt->Format("%s",argument); break;
               	   }
                     break;
                  }
                  case('v'): // Assign evaluated data to passed variable
                  {
                	   Variable * var = va_arg(ap,Variable*);
               //      var->CopyWithAlias(&result);
                     var->Assign(&result);
                     result.NullData();
                     var->SetName(argument);
                     break;
                  }
                                                     
                  default:	 
                     ErrorMessage("undefined variable '%s'",argument);
         	         err = true;
                     break;
               }
	            cnt++;
		      }
		      else if(dataType[i] == 'c') // Data should not be changed apart from removing quotes and white space
		      {
	            RemoveQuotes(argument);
               RemoveEndBlanks(argument);
               	           
	            switch(argType[i])
	            {
                  case('t'): // CText
                  {
                     txt = va_arg(ap,CText*);
                     txt->Assign(argument);
                     cnt++;
                     break;
                  }
	               case('s'):
		            {
		               strcpy(va_arg(ap,char*),argument);
		               cnt++;
		               break;
		            }
		            case('c'): 
	                  if(sscanf(argument,"%c",va_arg(ap,char*)) == 1)
	                     cnt++;
	                  break;
                  case('q'): // String list
		            {
                     short e;
                     va_arg(ap,char*)[0] ='\0';
                     do
                     {
                        e = carg.GetNext(argStr,argument);
                        strcat(va_arg(ap,char*),argument);
		                  if(e != FINISH)
		                     strcat(va_arg(ap,char*),";");
                     }
                     while(e == OK);
		               cnt++;
		               break;
		            }
		         }
	         }
	      }
      }
	
ex:   va_end(ap);
	   delete[] argument;

	   if(err == false && cnt < minNr)
	   {
         if(cnt == 0 && itfc->inCLI)
         {
            ListArguments(prompt,dataType,argType);
            itfc->nrRetValues = 0;
            return(-2);
         }
	      ErrorMessage("missing arguments (at least %d required)",minNr);
	      return(ERR);
	   }
	   	   
      if(err == true)
         return(ERR);
         
	   return(cnt);
	}

   itfc->nrRetValues = oldNrRetValues;

   return(0);
}


/*******************************************************************************************
  Parse a string extracting arguments separated by the specified character delimiter
 ******************************************************************************************/

int ParseString(Interface* itfc ,char args[])
{
   CArg carg;
   short r;
   CText delimit;
   CText token;
   CText str;
   char **list;

   if((r = ArgScan(itfc,args,2,"string, delimiter","ee","tt",&str,&delimit)) < 0)
      return(r); 

   carg.Init(delimit[0]);

   int n = carg.Count(str.Str());

// Make the necessary list to store the tokens
	list = NULL;

// Fill up the list
   for(int i = 0; i < n; i++)
   {
      token = carg.Extract(i+1);
      AppendStringToList(token.Str(), &list, i);
   }

// Copy to ansvar
   itfc->retVar[1].AssignList(list,n);
   itfc->nrRetValues = 1;

   return(OK);
}

/*******************************************************************************************
  Remove spaces or other characters from the front, back or both ends of a string
******************************************************************************************/
int TrimString(Interface* itfc ,char args[])
{
	CText inStr, outStr;
	CText mode = "front";
	int r;
	CText ch = " ";
	int i;

   if((r = ArgScan(itfc,args,1,"string to trim, front/back/both, char","eee","ttt",&inStr,&mode,&ch)) < 0)
      return(r);

	int sz = inStr.Size();
	char* s = inStr.Str();
	char match = ch.Str()[0];

	if(mode == "front")
	{
		for(i = 0; i < sz; i++)
		{
			if(s[i] != match)
				break;
		}
		outStr = inStr.End(i);
	}
	else if(mode == "back")
	{
		for(i = sz-1; i >= 0; i--)
		{
			if(s[i] != match)
				break;
		}
		outStr = inStr.Start(i);
	}
	else
	{
		for(i = 0; i < sz; i++)
		{
			if(s[i] != match)
				break;
		}
		outStr = inStr.End(i);
	   s = outStr.Str();
	   sz = outStr.Size();
		for(i = sz-1; i >= 0; i--)
		{
			if(s[i] != match)
				break;
		}
		outStr = outStr.Start(i);
	}

	itfc->retVar[1].MakeAndSetString(outStr.Str());
	itfc->nrRetValues = 1;
	return(OK);
}

/*******************************************************************************************
  Remove a first instance of substring from a string
******************************************************************************************/
//TODO move to CText
int RemoveSubString(Interface* itfc ,char args[])
{
   short r;
   long i,j;
   char substr[MAX_STR];
   char str[MAX_STR];
   char result[MAX_STR];

   if((r = ArgScan(itfc,args,2,"string, substring","ee","ss",str,substr)) < 0)
      return(r); 

// Find out where the substring is
   char* start =  strstr(str,substr);
   if(start == NULL)
   {
	   itfc->retVar[1].MakeAndSetString(str);
	   itfc->nrRetValues = 1;
      return(OK);
   }

   short off = start - str;

// Copy from str to result ignoring substring
   for(j = 0,i = 0; i < off; i++)
      result[j++] = str[i];

   long len = strlen(str);
   for(i = off+strlen(substr); i < len; i++)
      result[j++] = str[i];

   result[j] = '\0';

// Assign to ansVar
   itfc->retVar[1].MakeAndSetString(result);
   itfc->nrRetValues = 1;

   return(OK);
}

/*******************************************************************************************
  See if a substring exists within string
  For a string or list returns 1 if found 0 if not
******************************************************************************************/

int IsSubString(Interface* itfc ,char args[])
{
   short r;
   CText substr;
   CText str;
	Variable item;

   if((r = ArgScan(itfc,args,2,"string/list, substring","ee","vt",&item,&substr)) < 0)
      return(r); 

// See if the substring exists
   if(item.GetType() == UNQUOTED_STRING) // In a string
	{
		str = item.GetString();
		if(substr.Size() == 0 && str.Size() != 0)
		{
			itfc->retVar[1].MakeAndSetFloat(0);
			itfc->nrRetValues = 1;
			return(OK);		
		}
		if(substr.Size() == 0 && str.Size() == 0)
		{
			itfc->retVar[1].MakeAndSetFloat(1);
			itfc->nrRetValues = 1;
			return(OK);		
		}

		char* start =  strstr(str.Str(),substr.Str());
		if(start == NULL)
			itfc->retVar[1].MakeAndSetFloat(0);
		else
			itfc->retVar[1].MakeAndSetFloat(1);
	}
	else if(item.GetType() == LIST) // In a list
	{
		int sz = item.GetDimX();
		char** list = item.GetList();
		for(int i = 0; i < sz; i++)
		{
			char* start =  strstr(list[i],substr.Str());
			if(start != NULL)
			{
				itfc->retVar[1].MakeAndSetFloat(1);
				itfc->nrRetValues = 1;
				return(OK);	
			}
		}
      itfc->retVar[1].MakeAndSetFloat(0);
	}

   itfc->nrRetValues = 1;

   return(OK);
}


void ListArguments(char *prompt, char *dataType, char *argType)
{
   char str[MAX_STR];

   long pos = 0;
   char* promptcpy = new char[strlen(prompt)+1];
   strcpy(promptcpy,prompt);
   if(FindSubStr(promptcpy,":",pos))
   {
         promptcpy[pos] = '\0';
         sprintf(str,"\n\n   Syntax:\n\n      %s = %s(%s) [%s:%s]",promptcpy,gCommand,prompt+pos+1,dataType,argType);
   }
   else
   {        
         sprintf(str,"\n\n   Syntax:\n\n      %s(%s) [%s:%s]",gCommand,prompt,dataType,argType);
   }
   delete [] promptcpy;
   TextMessage(str);
   TextMessage("\n");
}

int UpdateProspaArgumentVariables(Interface *itfc, char *args)
{
   CArg carg;
   CText argument;
   Variable result;

   // Get arguments from user ************* 
   short nrArgs = carg.Count(args);

   for (long i = 1; i <= nrArgs; i++)
   {
      argument = carg.Extract(i);

      if (Evaluate(itfc, RESPECT_ALIAS, argument.Str(), &result) < 0)
         return(ERR);

      if (CopyVariable(itfc->argVar[i].var, &result, RESPECT_ALIAS) == ERR)
         return(ERR);
   }
   itfc->nrProcArgs = nrArgs;

   return(nrArgs);
}



// Copy the return results in par to par->itfc for use in DLLs
int  UpdateProspaReturnVariables(DLLParameters *par, Interface *itfc)
{
   par->nrRetVar = itfc->nrRetValues;
   return(OK);
}

/***********************************************************
 Determines the data type stored in str. May be one of 
 INTEGER, FLOAT32, QUOTED_STRING, UNQUOTED_STRING. If it is a
 number the value of the number is returned in 'result'
************************************************************/

#define IS_STRING  0
#define IS_NUMBER  1
#define IS_HEX     2
#define IS_FLOAT   3
#define IS_ARRAY   4

short GetDataType(char str[],float *result)
{
   short len;
   char type = UNQUOTED_STRING;
   long hexNr;
   short curType = IS_STRING;
   short start = 1;
   
   len = strlen(str)-1;

// Is the first character numeric?
   if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
   {
      start = 2;
      curType = IS_HEX;
   }
   else if(IsADigit(str[0]) || str[0] == '-' || str[0] == '+')
      curType = IS_NUMBER;
   else if(str[0] == '.')
      curType = IS_FLOAT;
   else
      curType = IS_STRING;

// Scan string checking first character      
   for(int i = start; i <= len ; i++)
   {
      if(str[i] == '.')
      {
         if(curType == IS_HEX) // Can't be hex with a decimal point
         {
            curType = IS_STRING;
            break;
         }
         else if(curType == IS_NUMBER) // First decimal point, might be float
         {
            curType = IS_FLOAT;
            continue;
         }
         else if(curType == IS_FLOAT) // More than 1 decimal point!
         {
            curType = IS_STRING;
            break;
         }
      }
      if(str[i] == '\r')
      { 
         curType = IS_STRING;  
         break;
      }  
      if(str[i] == ':' && i != len) // Either an array or a macro:procedure name
      { 
         curType = IS_ARRAY;  
         break;
      }
      if(curType == IS_STRING && str[i] == '[')
      { 
         curType = IS_ARRAY;  
         break;
      }         
      if(str[i] > 0 && ((isalpha(str[i]) || ispunct(str[i])) && !isxdigit(str[i])))
      {
         curType = IS_STRING;  
         break;
      }
   } 


// Figure out type 
   if(curType == IS_NUMBER)
   {
      sscanf(str,"%f",result);
      type = INTEGER;
   }
   else if(curType == IS_FLOAT)
   {
      sscanf(str,"%f",result);
      type = FLOAT32;
   }
   else if(curType == IS_HEX)
   {
      if(str[1] == 'x') str += 2;
      sscanf(str,"%x",&hexNr);
      *result = (float)hexNr;
      type = INTEGER;
      str -= 2;
   }
   else if(curType == IS_STRING)
   {
      if(str[0] == QUOTE && str[len] == QUOTE)
         type = QUOTED_STRING;
      else
         type = UNQUOTED_STRING;
   }
   else if(curType == IS_ARRAY)
   {
      type = MATRIX2D;
   }

   return(type);
}

#define IS_STRING  0
#define IS_NUMBER  1
#define IS_HEX     2
#define IS_FLOAT   3

/***************************************************************
    Check to see if a string can represent a variable
****************************************************************/

bool IsVarString(char *s)
{   
   if((s[0] >= '0' && s[0] <= '9') || s[0] == '-' || s[0] == '+')
      return(false);

   short i = 0;
   while(s[i] != '\0')
   {
      if((s[i] >= 'a' && s[i] <= 'z') ||  
         (s[i] >= 'A' && s[i] <= 'Z') || 
         (s[i] >= '0' && s[i] <= '9'))
      {
         i++;
         continue;
      }
      return(false);
   }
   return(true);
}

/***************************************************************
    Check to see if a character is in a string (character must
    not be inside brackets or a string).

    Returns first character found and its position. If character
    is not found then function returns -1.
****************************************************************/

short FirstCharInString(char *s, char *chars, char &first)
{
   short curveCnt = 0;
   short curlyCnt = 0;
   short squareCnt = 0;
   bool inQuote = false;
   short ln = strlen(chars);
   long len = strlen(s);

   for(int i = 0; i < len; i++)
   {
   // See if we are in a quoted string - if so search for end
      if(!inQuote && i == 0 && s[0] == '"')
      {
         inQuote = true;
         continue;
      }
      else if(!inQuote && i > 0 && s[i] == '"' && s[i-1] != '\\')
      {
         inQuote = true;
         continue;
      }
      else if(inQuote && s[i] == '"' && s[i-1] != '\\')
      {
         inQuote = false;
         continue;
      }
      if(inQuote)
			continue;

   // Search for delimiter if not in brackets
      if(!curveCnt && !curlyCnt && !squareCnt && !inQuote)
      {
         for(int k = 0; k < ln; k++)
         {
            if(s[i] == chars[k])
            {
               first = chars[k];
               return(i);
            }
         }
      }

   // Check for different bracket types
      if(s[i] == '(')
      {
         curveCnt++;
         continue;
      }
      if(s[i] == ')')
      {
         curveCnt--;
         continue;
      }

      if(s[i] ==  '[')
      {
         squareCnt++;
         continue;
      }
      if(s[i] == ']')
      {
         squareCnt--;
         continue;
      }

      if(s[i] == '{')
      {
         curlyCnt++;
         continue;
      }
      if(s[i] == '}')
      {
         curlyCnt--;
         continue;
      }
   }

// Can't find delimiter
   return(-1);
}

/***************************************************************
    See if string s is just white space (i.e. spaces or tabs)
****************************************************************/

bool IsWhiteSpaceString(char *s)
{
   long len = strlen(s);
   for(long i = 0; i < len; i++)
   {
      if(!IsWhiteSpace(s[i]))
         return(false);
   }
   return(true);
}

bool IsCommentLine(char *s)
{
   long len = strlen(s);
   for (long i = 0; i < len; i++)
   {
      if (s[i] == '#')
         return(true);

      if (!IsWhiteSpace(s[i]))
         return(false);
   }
   return(true);
}


/***************************************************************
    See if char c is a valid hex letter
****************************************************************/
 
bool ishex(char c)
{
   if(c >= 'a' && c <= 'f')
      return(true);
   if(c >= 'A' && c <= 'F')
      return(true);
   return(false);
}

bool IsADigit(char c)
{
   return(c >= '0' && c <= '9');
}

/*****************************************************************************
*  Read a string from the terminal and return it. Skip over prompt.          *
*  (e.g. "plane planeNr fileNr ... xy 23 1")                                 *
*****************************************************************************/
#ifdef UNDEFINED
short
GetStringFromUser(char defaultTxt[]) /* default text */
{
	MSG msg;
   HWND hWnd;
   char key;
	long start,end;
	long i;
	long charStart;
	long defaultLen = strlen(defaultTxt);
	long startPos; // Start of argument on current line
	long line; // Current line number
   bool first = true;
   char buf[MAX_STR];
   extern WNDPROC OldCLIProc;
   extern LRESULT CALLBACK  CLIProc(HWND, UINT, WPARAM, LPARAM);

   hWnd = curCLIObject->hWnd;
	SetWindowLong(hWnd,GWL_WNDPROC,(LONG)OldCLIProc); 
	line = SendMessage(hWnd,EM_GETLINECOUNT,0,0);
   *(int*)buf = 100; // buffer size
	startPos = SendMessage(hWnd,EM_GETLINE,line-1,(LPARAM)buf)+1;
	GetEditSelection(hWnd,start,end);
	charStart = start+1;
   TextMessage(defaultTxt);
   
// Let user type in new data
   
   while(1)
   {
	   if(PeekMessage(&msg, NULL,0,0,PM_REMOVE))
	   {
	   	key = msg.wParam;
	   	GetEditSelection(hWnd,start,end);

	      if(msg.message == WM_CHAR)
	      {
	      
	     // If cursor insert position is invalid then reset it to start of line */
		  
			   if(end < charStart)
			   {
			      SetEditSelection(hWnd,charStart,charStart);
				   first = false;
				}                
		   
	         if(key == '\b') // Ignore backspaces which would delete prompt
            {
               first = false;
               if(start == charStart)
                  continue;
            }
            
            if(key == '\r') // Extract new argument
            {
              *(int*)buf = 100; // buffer size
		         long length = SendMessage(hWnd,EM_GETLINE,line-1,(LPARAM)buf);
		         buf[length] = '\0';
		         for(i = startPos; i < length; i++)
		            defaultTxt[i-startPos] = buf[i];
		         defaultTxt[i-startPos] = '\0';
		   	   end = SendMessage(hWnd,EM_LINELENGTH,charStart,0) + charStart;
			      SetEditSelection(hWnd,end,end);
		         break; 
            }
		      else if(key == (char)0x1B) /* escape */
		      {
		         TextMessage(" - aborted\n");
		         SetWindowLong(hWnd,GWL_WNDPROC,(LONG)CLIProc); 
		         return(-3);
		      }
		      
		      if(first == true) // Delete default arguments
	         {
			      SetEditSelection(hWnd,charStart,end);
		      	SendMessage(hWnd,EM_REPLACESEL,(WPARAM)false,(LPARAM)"");
	         }

		      first = false;
	      }
	      else if(msg.message ==  WM_KEYDOWN) // Check for arrow keys
	      {
	         long line = SendMessage(hWnd,EM_GETLINECOUNT,0,0);
	         long lineStart = SendMessage(hWnd,EM_LINEINDEX,-1,0);
	
	         if(key == VK_LEFT) // Make sure left shift stops at end of prompt
	         {
	            GetEditSelection(hWnd,start,end);
		         if(end < charStart+1)
                  SetEditSelection(hWnd,charStart+1,charStart+1);		         
				}               
	         else if(key == VK_UP) 		// Ignore up arrows */
	         {
					continue;
			   }
	         else if(key == VK_DOWN)  // Ignore down arrows */
	         {
					continue;
			   }
			}
	   
	      TranslateMessage(&msg);
	      DispatchMessage(&msg);
	   }
   }
   SetWindowLong(hWnd,GWL_WNDPROC,(LONG)CLIProc); 
   return(0);
}

#endif

// Globals used by CountArgs and ExtractArg routines

//static char *cLastStr;        // Name of string being searched
//static int cStart[50];        // Start position of argument
//static int cEnd[50];          // End position of argument
//static int nrWords;           // Number of arguments in string 

/**********************************************************************************
*
* Reset argument counter for CountArgs and ExtractArg routines
*
***********************************************************************************/

//void
//ResetArgCount()
//{
//   if(cLastStr)
//      cLastStr[0] = '\0';
//   nrWords = 0;
//}

/**********************************************************************************
*
* Scans through a string counting the number of arguments which are separated
* by the comma or colon delimiter. Spaces are ignored as are expression in arrays
* [...], {...}, strings "..." and argument lists (...).
* The routine also notes the starting and finishing positions of each argument
* which it stores in the global arrays 'cStart' and 'cEnd'. Subsequent calls to
* the routine ExtractArg() use this information to return the nth argument in a
* string.
*
***********************************************************************************/
//
//EXPORT int CountArgs(char *str)
//{
//   int len,i,cnt  = 1;
//   int inBracket  = 0;
//   bool inArg     = false;
//   bool inString  = false;
//   bool scanData = false;
//   
//// Ignore null strings
//   len = strlen(str);
//   if(len == 0) return(0);   
//
//// Don't bother if we've already checked this string
//   if(!cLastStr)
//   {
//      scanData = true;
//   }
//   else
//   {
//      scanData = (strcmp(str,cLastStr) != 0);
//   }
//   
//   if(scanData == true)
//   {
//  // Initialize argument range arrays
//      for(i = 0; i < 50; i++)
//      {
//         cStart[i] = -1;
//         cEnd[i] = -1;
//      }
//
//	   for(i = 0; i < len; i++) // Check each character in string
//	   {
//	       if(!inArg && str[i] == ' ') // Ignore space delimiters
//	          continue; 
//	          
//	       if(!inArg && str[i] != ' ') // Found start of non-string
//	       {
//	          inArg = true;
//	          if(cnt == 50) 
//                  return 50;
//	          cStart[cnt] = i;
//	       }
//	       
//	       if(inArg)
//	       {
//	          if(str[i] != ',' && str[i] != ':')
//		       {
//		          if(str[i] == '(' || str[i] == '[' || str[i] == '{')       // Found start of bracketed expression
//		            inBracket++;
//		          else if(str[i] == ')' || str[i] == ']' || str[i] == '}')  // Found end of bracketed expression
//		            inBracket--;
//		          else if(str[i] == QUOTE && !inString)     // Found start of quoted string
//		            inString = true;
//		          else if(str[i] == QUOTE && inString) // Found end of quoted string
//		            inString = false;
//		       }
//
//		       if((str[i] == ':') && inArg && !inBracket && !inString) // Found comma delimiter
//		          inArg = inArg;
//	                     
//		       if((str[i] == ',' || str[i] == ':') && inArg && !inBracket && !inString) // Found comma delimiter
//		       {
//		          cEnd[cnt++] = i-1;
//		          inArg = false;
//		       }
//		    }
//	    }
//	    cEnd[cnt] = i-1;
//	    if(cLastStr) delete[] cLastStr;
//       cLastStr = new char[strlen(str)+1];
//       strcpy(cLastStr,str);
//	    nrWords = cnt;
//    }
//    return(nrWords);
// }
//       
/*********************************************************************  
*     
* Extract the nth argument from str (n starts from 1). See CountArgs 
* for more information.  
*
**********************************************************************/
//
//EXPORT char* ExtractArg(char str[],int nr) 
//{
//   static char *extractedArg;
//   int i,j=0;
//
//// Ignore null strings
//   if(str[0] == '\0') return("");
//
//// Count arguments and record their position 
//   nrWords = CountArgs(str);
//
//// Ignore non-existant entries
//   if(cEnd[nr] == -1 || cEnd[nr] == -1)
//      return("");
//
//// Extract string allocating memory for it
//   if(extractedArg) delete[] extractedArg;
//   extractedArg = new char[cEnd[nr]-cStart[nr]+2];
//
//   if(nr <= nrWords)
//   {
//      for(i = cStart[nr]; i <= cEnd[nr]; i++,j++)
//         extractedArg[j] = str[i];
//   }
//   extractedArg[j] = '\0';
//   return(extractedArg);
//}

// Globals used by CountArgsB and ExtractArgB routines
//
//static char *cLastStrB;        // Name of string being searched
//static int cStartB[50];        // Start position of argument
//static int cEndB[50];          // End position of argument
//static int nrWordsB;           // Number of arguments in string 
//
//
///**********************************************************************************
//*
//* Reset argument counter for CountArgsB and ExtractArgB routines
//*
//***********************************************************************************/
//
//void ResetArgCountB()
//{
//   if(cLastStrB)
//      cLastStrB[0] = '\0';
//   nrWordsB = 0;
//}
//
///**********************************************************************************
//*
//* Scans through a string counting the number of arguments which are separated
//* by the specified delimiter. Spaces are ignored as are expression in arrays [...],
//* {...} strings '...' and argument lists (...).
//* The routine also notes the starting and finishing positions of each argument
//* which it stores in the global arrays 'cStartB' and 'cEndB'. Subsequent calls to
//* the routine ExtractArgB() use this information to return the nth argument in a
//* string.
//*
//***********************************************************************************/
//
//int CountArgsB(char *str, char delimiter)
//{
//   int len,i,cnt  = 1;
//   int inBracket  = 0;
//   bool inArg     = false;
//   bool inString  = false;
//   bool scanData = false;
//   
//// Ignore null strings
//   len = strlen(str);
//   if(len == 0) return(0);   
//
//// Don't bother if we've already checked this string
//   if(!cLastStrB)
//   {
//      scanData = true;
//   }
//   else
//   {
//      scanData = (strcmp(str,cLastStrB) != 0);
//   }
//   
//   if(scanData == true)
//   {
//  // Initialize argument range arrays
//      for(i = 0; i < 50; i++)
//      {
//         cStartB[i] = -1;
//         cEndB[i] = -1;
//      }
//
//	   for(i = 0; i < len; i++) // Check each character in string
//	   {
//	       if(!inArg && str[i] == ' ') // Ignore space delimiters
//	          continue; 
//	          
//	       if(!inArg && str[i] != ' ') // Found start of non-string
//	       {
//	          inArg = true;
//	          if(cnt == 50) return 50;
//	          cStartB[cnt] = i;
//	       }
//	       
//	       if(inArg)
//	       {
//	          if(str[i] != delimiter)
//		       {
//		          if(str[i] == '(' || str[i] == '['  || str[i] == '{')       // Found start of bracketed expression
//		            inBracket++;
//		          else if(str[i] == ')' || str[i] == ']' || str[i] == '}')   // Found end of bracketed expression
//		            inBracket--;
////		          else if(str[i] == QUOTE && !IsEscapedQuote(str,i) && !inString)  // Found start of quoted string
//		          else if(str[i] == QUOTE && !inString)  // Found start of quoted string
//		            inString = true;
////		          else if(str[i] == QUOTE && !IsEscapedQuote(str,i) && inString)   // Found end of quoted string
//		          else if(str[i] == QUOTE && inString)   // Found end of quoted string
//		            inString = false;
//		       }
//	                     
//		       if((str[i] == delimiter) && inArg && !inBracket && !inString) // Found  delimiter
//		       {
//		          cEndB[cnt++] = i-1;
//		          inArg = false;
//		       }
//		    }
//	    }
//	    cEndB[cnt] = i-1;
//	    if(cLastStrB) delete[] cLastStrB;
//       cLastStrB = new char[strlen(str)+1];
//       strcpy(cLastStrB,str);
//	    nrWordsB = cnt;
//    }
//    return(nrWordsB);
// }
//
///*********************************************************************  
//*     
//* Extract the nth argument from str (n starts from 1). See CountArgsB 
//* for more information.  
//*
//**********************************************************************/
//
//char* ExtractArgB(char str[],char delimiter, int nr) 
//{
//   static char *extractedArg;
//   int i,j=0;
//
//// Ignore null strings
//   if(str[0] == '\0') return("");
//
//// Count arguments and record their position 
//   nrWordsB = CountArgsB(str,delimiter);
//
//// Ignore non-existant entries
//   if(cStartB[nr] == -1 || cEndB[nr] == -1)
//      return("");
//
//// Extract string 
//   if(extractedArg) delete[] extractedArg;
//   extractedArg = new char[cEndB[nr]-cStartB[nr]+2];
//
//   if(nr <= nrWordsB)
//   {
//      for(i = cStartB[nr]; i <= cEndB[nr]; i++,j++)
//         extractedArg[j] = str[i];
//   }
//   extractedArg[j] = '\0';
//   return(extractedArg);
//}
           
#define BETWEEN  0
#define NONSTRING  1
#define STRING 2

static char *last_str;       /* Previous string analysed */
static short start[50];      /* Start position of data    */
static short end[50];        /* End position of data      */
static short nr_words;       /* Number of words in string */

int CountStrings(char str[])
{
   short len;
   bool scanData = false;

// Ignore null strings 
   len = strlen(str);
   if(len == 0) return(0);

// Find argument positions if new string 
   if(!last_str)
   {
      scanData = true;
   }
   else
   {
      scanData = (strcmp(str,last_str) != 0);
   }
   
   if(scanData == true)
   {
		short s = 1;               /* Position status           */
		short cnt = 1;             /* Word counter              */
		short i; 
      for(i = 0; i < len; i++)
      {
         if(str[i] != ' ' && str[i] != QUOTE  && str[i] != '(' &&  s == 1) /* Start of numeric data */
         {
             start[cnt] = i;
             s = 2;
             continue;
         }
         if(s == 1 && str[i] == QUOTE && !IsEscapedQuote(str,i,len)) /* Start of string data */
         {
             start[cnt] = i;
             s = 3;
             continue;
         }
         if(str[i] == '(' && s == 1) /* Start of expression */
         {
             start[cnt] = i;
             s = 4;
             continue;
         }
         if(str[i] == ' ' &&  s == 2) /* End of numeric data */
         {
             end[cnt] = i-1;
             cnt++;
             s = 1;
             continue;
         }
         if(s == 3 && str[i] == QUOTE && !IsEscapedQuote(str,i,len)) /* End of string data */
         {
             end[cnt] = i;
             cnt++;
             s = 1;
             continue;
         }
         if(str[i] == ')' && s == 4) /* End of expression */
         {
             end[cnt] = i;
             cnt++;
             s = 1;
             continue;
         }
      }
      if(s == 2)
      {
         end[cnt] = i-1;
         nr_words = cnt;
      }
      else
         nr_words = cnt-1;

      if(last_str) delete[] last_str;
      last_str = new char[strlen(str)+1];
      strcpy(last_str,str);
   }
   return(nr_words);
}

/*********************************************************************  
    Extract nth word in str (n starts from 1)    
   if presented with same word again goes faster 
**********************************************************************/

char* ExtractString(char str[],int nr) 
{
   static char *extractedStr;

// Ignore null strings 
   if(str[0] == '\0') return("");

// Count arguments and record their position 
   int nr_words = CountStrings(str);

// Extract string 
   if(extractedStr) delete[] extractedStr;
   extractedStr = new char[end[nr]-start[nr]+2];
      
   int j=0;
   if(nr <= nr_words)
   {
      for(int i = start[nr]; i <= end[nr]; i++,j++)
         extractedStr[j] = str[i];
   }
   extractedStr[j] = '\0';
   return(extractedStr);
}

/*********************************************************************  
    Remove quotes from a string "test" -> test
**********************************************************************/

void RemoveQuotes(char str[]) 
{
   short len = strlen(str)-1;
   if(str[0] == QUOTE && str[len] == QUOTE)
   {
      for(short i = 0; i < len; i++)
         str[i] = str[i+1];
      str[len-1] = '\0';
   }
}


/*********************************************************************  
    Replace all escaped characters with their internal values
**********************************************************************/

void ReplaceEscapedCharacters(char str[])
{
   long len = strlen(str);
   long i,j;

   for(i = 0, j = 0; i < len; i++)
   {
      if(i < len-1 && str[i] == '\\')
      {
         if(str[i+1] == 'n') // Linefeed
         {
            str[j++] = '\n';
            i++;
         }
         else if(str[i+1] == 'r') // Carriage return
         {
            str[j++] = '\r';
            i++;
         }
         else if(str[i+1] == 't') // Tab
         {
            str[j++] = '\t';
            i++;
         }
         else if(str[i+1] == 'a') // at
         {
            str[j++] = '@';
            i++;
         }
         else if(str[i+1] == '"') // Quote
         {
            str[j++] = '"';
            i++;
         }
         else if(str[i+1] >= '0' && str[i+1] <= '9') // Character code e.g. \123
         {
            if(len >= i+4 && (str[i+2] >= '0' && str[i+2] <= '9' &&
                              str[i+3] >= '0' && str[i+3] <= '9'))
            {
               int num;
               if(sscanf(str+i+1,"%3d",&num) == 1)
               {
                  str[j] = (char)num;
                  j++;
                  i+=3;
               }
            }
            else // Unescaped character
            {
               str[j++] = str[i];
            }
         }
         else if(str[i+1] == '\\') // Backslash
         {
            str[j++] = '\\';
            i++;
         }
         else // Not a recognised escapable character so treat as real '\'
            str[j++] = str[i];
      }
      else // Unescaped character
      {
         str[j++] = str[i];
      }
   }
   str[j] = '\0';
}

/*********************************************************************  
    Replace all escaped quotes with their internal values
**********************************************************************/

void ReplaceEscapedQuotes(char str[])
{
   long len = strlen(str);
   long i,j;

   for(i = 0, j = 0; i < len; i++)
   {
      if(i < len-1 && str[i] == '\\')
      {
         if(str[i+1] == '"') // Quote
         {
            str[j++] = '"';
            i++;
         }
         else 
            str[j++] = str[i];
      }
      else // Unescaped character
      {
         str[j++] = str[i];
      }
   }
   str[j] = '\0';
}


/*********************************************************************  
   Replace double escapes with a single e.g "\\" -> "\"
**********************************************************************/

void SimplifyDoubleEscapes(char str[])  
{                             
   short i,j;
   short len;

   len = strlen(str);

   for(i = 0, j = 0; i < len; i++)
   {
      if(i < len-1 && str[i] == '\\' && str[i+1] == '\\')
      {
         str[j++] = '\\';
         i++;
      }
      else
      {
         str[j++] = str[i];
      }
   }
   str[j] = '\0';
}

// See if str[i] represents a real unescaped quote

bool IsUnEscapedQuote(char *str, long i, long len)
{
   if(str[i] != QUOTE)  // Not a quote!
      return(false);
 
   if(i == 0)
      return(true);  // First location in string
   
   else if(i == 1)
   {
      return(str[0] != '\\'); // False if escaped \"
   }
   else
   {
      if(str[i-1] == '\\')
      {
         long cnt = 1;
         for(long j = i-2; j > 0; j--) // Search backwards for matched pairs of backslashes
         {
            if(str[j] != '\\')
               break;
            cnt++;
         }
         return(!(cnt%2)); // Check for escaped character
      }
      return(true);
   }
}

// See if the position 'i' in a string 'str' represents an escaped quote  \" (starting at ")

bool IsEscapedQuote(char *str, long i, long len)
{
   if(i == 0 || str[i] != QUOTE) 
      return(false);
  
   else if(i == 1)
   {
      return(str[0] == '\\'); // True if \"
   }
   else
   {
      if(str[i-1] == '\\')
      {
         long cnt = 1;
         for(long j = i-2; j > 0; j--) // Search backwards for matched pairs of backslashes
         {
            if(str[j] != '\\')
               break;
            cnt++;
         }
         return(cnt%2); // Check for escaped character
      }
      return(false);
   }
}


// Add quotes to a string - note str must be large enough to accomodate extra
// characters!

void AddQuotes(char str[])
{
  short len = strlen(str);
  
  for(short i = len; i >= 1; i--)
  {
     str[i] = str[i-1];
  }
  str[0] = QUOTE;
  str[len+1] = QUOTE;
  str[len+2] = '\0';
}

bool isStr(char *str)
{
   if(str[0] == QUOTE && str[strlen(str)-1] == QUOTE)
      return(true);
   else
      return(false);
}

// Remove all instances of character 'c' from string 'str'

void RemoveCharacter(char *str, char c)
{
   int j,i;
   long len = strlen(str);
   for(j = 0, i = 0; i < len; i++)
   {
      if(str[i] == c)
      {
         continue;
      }
      str[j++] = str[i];
   }
   str[j] = '\0';
}

void RemoveTrailingBlanks(char *str)
{
   long len = strlen(str);
   for(int i = len-1; i >= 0; i--)
   {
      if(str[i] != ' ' && str[i] != '\t')
      {
         str[i+1] = '\0';
         return;
      }
   }
}

// Remove blanks (spaces or tabs) from the ends of a string

void RemoveEndBlanks(char *str)
{
   int len = strlen(str);
   int start,end,i;

// Find start of non blank text   
   for(start = 0; start < len; start++)
   {
      if(str[start] != ' ' && str[start] != '\t')
         break;
   }

// Find last non-blank text   
   for(end = len-1; end >= 0; end--)
   {
      if(str[end] != ' ' && str[end] != '\t')
      {
        break;
      }
   }

// Shift non-blank part of string to start of array      
   for(i = start; i <= end; i++)
   {
      str[i-start] = str[i];
   }
   
// Move end marker to end of non-blank text
   str[i-start] = '\0';
}

//      
//short FindArg(char str[], char arg[])
//{
//   int i,j=0;
//
//// Ignore null strings
//
//   if(str[0] == '\0') return(0);
//
//// Count arguments and record their position 
//
//   nrWords = CountArgs(str);
//
//// Search for argument in string 
//
//   for(i = 1; i <= nrWords; i++)
//   {
//      for(j = cStart[i]; j <= cEnd[i]; j++)
//      {
//         if(str[j] != arg[j-cStart[i]])
//            break;
//      }
//      if(j > cEnd[i])
//         return(i);
//   }
//   return(0);
//}

// Copies a maximum of N characters from in to out - end of string terminates
// in both cases 

void StrNCopy(char *out, const char *in, long N)
{

   strncpy(out,in,N);
 //  long p = strlen(in);
   out[N] = '\0';
   //long lenIn = strlen(in);
   //long i;
   //
   //for(i = 0; i < N && i < lenIn; i++)
   //{
   //   out[i] = in[i];
   //}
   //out[i] = '\0';
}

// Concatenates A to B restricting the number of characters to a max of N
// i.e A = A + B

void StrNCat(char *A, char *B, short N)
{
   strncat(A,B,N);
   A[N] = '\0';
}

void StrSubCopy(char *out, char *in, long start, long end)
{
   short i;
   
   for(i = start; i <= end; i++)
   {
      out[i-start] = in[i];
   }
   out[i-start] = '\0';
}

// Extract substring from with extent 'start' to 'end'
// Note: allocates memory.
char* GetStrSub(char *in, long start, long end)
{
   short i;
   if(start >= end) return(NULL);
   char *out = new char[end-start+2];
   
   for(i = start; i <= end; i++)
   {
      out[i-start] = in[i];
   }
   out[i-start] = '\0';
   return(out);
}


/*************************************************************************
  Converts a string of the form 1.23i or i2.34 into a complex number
**************************************************************************/

complex StrtoComplex(char *str)
{
   complex result = {0,0};
   short len = strlen(str);
   
   if(str[len-1] == 'j' || str[len-1] == 'i')
   {
      if(len == 1) // Just j or i
         result.i = 1;
      else // Prefactor e.g. 2j
      {
         str[len-1] = '\0';
         result.i = StringToFloat(str);         
      }
      result.r = 0;
   }
   else if(str[0] == 'j' || str[0] == 'i')
   {
      if(len == 1) // Just j or i
         result.i = 1;
      else // Postfactor e.g. j2
      {
         result.i = StringToFloat(str+1);         
      }
      result.r = 0;
   }
   return(result);
}

bool SingleArgument(char *arg)
{
   long len = strlen(arg);
   for(short i = 0; i < len; i++)
   {
      if(arg[i] == ',') 
      {
         ErrorMessage("only one argument expected");
         return(ERR);
      }
   } 
   return(OK);
}

// Append string "add" to string  "in" starting a position "s"
// return new insertion position. A null character can be added
// by passing an empty string
  
void stradd(char *in, char *add, short &s)
{
   if(add[0] == '\0')
   {
      in[s] = '\0';
      s++;
   }
   else
   {
      short len = strlen(add);
           
	   for(short j = s; j < len+s; j++)
	   {
	      in[j] = add[j-s];
	   }
      s += len;
   }
}


// Extract a procedure name from a filename returning
// the base filename and the procedure name.
// Filename input has the form file:proc
// after fileName = "file" and procName = "proc"
//
void ExtractProcedureName(char *fileName, char *procName)
{
   char len = strlen(fileName);
   
   for(short i = 0; i < len; i++)
   {
      if(fileName[i] == ':')
      {
         fileName[i] = '\0';
         i++;
         for(short j = i; j <= len; j++)
         {
            procName[j-i] = fileName[j];
         }
         return;
      }
   }
   procName[0] = '\0';
}

// Extract a class member name from a procdure call returning
// the base class object name and the procedure name.
// Filename input has the form obj->proc
// after fileName = "obj" and procName = "proc"
//
void ExtractClassProcName(char *objectName, char *procName)
{
   char len = strlen(objectName);
   
   for(short i = 0; i < len; i++)
   {
      if(objectName[i] == '-' && objectName[i+1] == '>')
      {
         objectName[i] = '\0';
         i+=2;
         for(short j = i; j <= len; j++)
         {
            procName[j-i] = objectName[j];
         }
         return;
      }
   }
   procName[0] = '\0';
}

// Return the word in 'text' which is identified by the offset
// (this is simply one of the characters in the word)
// Note allocates memory for returned word.

char* GetWordAtChar(char *text, long offset, long &startCmd, long &endCmd)
{
   long i;
   char *cmd;
   long len = strlen(text);

// Check to see if valid text string is passed      
   if(!text) return(NULL);
   
// Search for word from position 'start' by looking backward and forward
   startCmd = 0;
   for(i = offset; i >= 0; i--) // Look for beginning
   { 
      if(IsOperandDelimiter(text,i,len))
      {
         startCmd = i+1;
         break;
      }
   }
   endCmd = len - 1;
   for(i = offset; i < len; i++) // Look for end
   {
      if(IsOperandDelimiter(text,i,len))
      {
         endCmd = i-1;
         break;
      }
   }
 
 // Extract command name 
   cmd = GetStrSub(text,startCmd,endCmd);
   
   return(cmd);
}

// Convert an enumeration into a string

char* EnumToString(short enumType, short enumeration)
{
   if(enumType == VARIABLE_SOURCE)
   {
      if(enumeration == 0)
         return("local variable");
      else if(enumeration == 2)
         return("window variable");        
      else if(enumeration == 1)
         return("global variable");
   }
   return("");
}

  /**********************************************************************
 * Title    : FindSubStr()
 * Function : Search through string text for the first instance of substr.
 *            If ignoreCase is true text case will be ignored during
 *            the comparison.
 * Returns  : Returns the zero based offset of any substr found in text
 *            otherwise returns -1 if nothing found.
 **********************************************************************/

long FindSubStr(char *text, char *substr, long start, bool ignoreCase)
{
long i,j;
   long txtLen = strlen(text);
   long ssLen = strlen(substr);

// Test for bad strings
   if(txtLen < ssLen || ssLen == 0)
      return(-1);

// Search substring
   for(i = start; i <= txtLen-ssLen; i++)
   {
      for( j = 0; j < ssLen; j++)
      {
         if(ignoreCase)
         {
            if(tolower(text[i+j]) != tolower(substr[j]))
               break;
         }
         else
         {
            if(text[i+j] != substr[j])
               break;
         }         
      }
      if(j == ssLen)
        return(i);
   }
   return(-1);
}

/**********************************************************************
 * Title    : FindReverseSubStr()
 * Function : Search backwards through string text from position start
 *            for the first (forward) instance of substr. If ignoreCase
 *            is true text case will be ignored during the comparison.
 *            If ignore Comment is true then it won't find substr if it
 *            is part of a comment.
 * Returns  : Returns the zero based offset of any substr found in text
 *            otherwise returns -1 if nothing found.
 **********************************************************************/

long FindReverseSubStr(char *text, char *substr, long start, bool ignoreCase, bool ignoreComment)
{
	long i,j,k;
   long txtLen = strlen(text);
   long ssLen = strlen(substr);

// Test for bad strings
   if(txtLen < ssLen || ssLen == 0)
      return(-1);

// Test for bad start positions
   if(start-ssLen <= 0 || start > txtLen)
      return(-1);

// Search substring
   for(i = start-ssLen; i >= 0; i--)
   {
      for(j = 0; j < ssLen; j++)
      {
         if(ignoreCase)
         {
            if(tolower(text[i+j]) != tolower(substr[j]))
               break;
         }
         else
         {
            if(text[i+j] != substr[j])
               break;
         }
      }
      if(j == ssLen) // Found it
      {
         if(ignoreComment)
         {
            for(k = i; k >= 0; k--)
            {
               if(text[k] == '\n') // Not a comment
                  return(i);
               if(text[k] == '#') // It in a comment
                  break;
            }
         }
         else
           return(i);
      }
   }
   return(-1);
}

/***************************************************************
  Remove from string all characters 'ign' which are not quoted
****************************************************************/

void RemoveUnquotedCharacter(char *str, char ign)
{
   long i,j;
	enum {IN_STR,OUT_STR} state = OUT_STR;
   long len = strlen(str);
   for(i = 0, j = 0; i < len; i++)
	{
		if(str[i] == '"')
      {
         if(!(i > 0 && str[i-1] == '\\'))
			   (state == OUT_STR) ? (state = IN_STR) : (state = OUT_STR);
      }
      else if(str[i] == ign && state == OUT_STR)
			continue;

		str[j++] = str[i];
	}

	str[j] = '\0';
}


/*********************************************************************  
    Replace actual quotes  in string "in" with \" in string "out"
**********************************************************************/

void AddEscapedQuotes(char* out, char* in)
{

   int j = 0;
   for(int i = 0; i < strlen(in); i++,j++)
   {
      if(in[i] == '"')
      {
         out[j] = '\\';
         out[j+1] = '"';
         j++;
      }
      else
      {
         out[j] = in[i];
      }
   }
   out[j] = '\0';
}

/***************************************************************
  Copy 'input' to 'output' starting from index 0 and ending
  on index 'end'
****************************************************************/

void LeftStr(char *input, long end, char *output)
{
   long i;
	for(i = 0; i <= end; i++)
	{
		output[i] = input[i];
	}
	output[i] = '\0';
}

/***************************************************************
  Copy 'input' to 'output' starting from index 'start' and 
  finishing at the end of the input string
****************************************************************/

void RightStr(char *input, long start, char *output)
{
   long i;
   long len = strlen(input);
	for(i = start; i < len; i++)
	{
		output[i-start] = input[i];
	}
	output[i-start] = '\0';
}

/***************************************************************
  Search for the first instance of character 'c' in 'str'
  returning the zero based position or -1 if not found
****************************************************************/

long FindCharacter(char *str, char c)
{
   long len = strlen(str);
	for(long i = 0; i < len; i++)
	{
		if(str[i] == c)
			return(i);
	}
	return(-1);
}


int ProcessEscapeCharacters(Interface* itfc, char args[])
{
   short nrArgs;
   static CText mode;

   (itfc->processEscapes) ? (mode  = "true") : (mode = "false");
   if((nrArgs = ArgScan(itfc,args,1,"mode","e","t",&mode)) < 0)
      return(nrArgs); 

   if(mode == "yes" || mode == "true")
      itfc->processEscapes = true;
   else if(mode == "no" || mode == "false")
      itfc->processEscapes = false;
   else
   {
      ErrorMessage("invalid argument - should be true/false");
      return(ERR);
   }
   
	itfc->nrRetValues = 0;
   return(OK);
}

/**********************************************************
   Check if str = "yes" or "true" if so return 1
         if str = "no" or "false" if so return 0  
   Otherwise signal an error
**********************************************************/

short StrTrueCheck(char *str, bool &response)
{
   if(!strcmp(str,"true") || !strcmp(str,"yes"))
   {
      response = true;
      return(OK);
   }
   else if(!strcmp(str,"false") || !strcmp(str,"no"))
   {
      response = false;
      return(OK);
   }
   

   ErrorMessage("Invalid parameter '%s'",str);
   return(ERR);
}


/**********************************************************
   Compare two strings - the source string and a reference
   string. The reference string can include the wild cards
   * and ?.
   Returns 1 if they are the same 0 if different
**********************************************************/

int CompareStrings(Interface* itfc, char args[])
{
   short nrArgs;
   Variable srcVar,refVar;
   CText src,ref;
	itfc->nrRetValues = 0;

   if((nrArgs = ArgScan(itfc,args,1,"source str,reference str","ee","vv",&srcVar,&refVar)) < 0)
      return(nrArgs); 

   if(srcVar.GetType() == UNQUOTED_STRING && refVar.GetType() == UNQUOTED_STRING)
   {
      src = srcVar.GetString();
      ref = refVar.GetString();

      int lenSrc = src.Size();
      int lenRef = ref.Size();

      if(lenRef > lenSrc)
      {
         itfc->retVar[1].MakeAndSetFloat(0);
         itfc->nrRetValues = 1;
         return(OK);
      }

		int r = CompareStringsCore(src,ref);

      itfc->retVar[1].MakeAndSetFloat(r);
      itfc->nrRetValues = 1;

      return(OK);
   }
   else if(srcVar.GetType() == LIST && refVar.GetType() == UNQUOTED_STRING)
   {
      char **lst = srcVar.GetList();
      int szLst = srcVar.GetDimX();

		ref = refVar.GetString();
      int lenRef = ref.Size();

      if(lst && szLst > 0)
      {
         float *result = new float[szLst];

         for(int m = 0; m < szLst; m++)
         {
            result[m] = -1;
            src = lst[m];
            int lenSrc = src.Size();
    
            if(lenRef > lenSrc)
            {
               result[m] = 0;
               continue;
            }

            result[m] = CompareStringsCore(src,ref);
         }
         itfc->retVar[1].MakeMatrix2DFromVector(result,szLst,1);
         itfc->nrRetValues = 1;
         delete [] result;
         return(OK);
      }
      else // Null list
      {
         itfc->retVar[1].MakeAndSetFloat(0);
         itfc->nrRetValues = 1;
         return(OK);
      }
   }
   else
   {
      ErrorMessage("Unsupported data type for source or reference string");
      return(ERR);
   }
}

int CompareStringsCore(CText &src, CText &ref)
{
   int i,j,k;
   CText sub;

   int lenSrc = src.Size();
   int lenRef = ref.Size();

   for(i = 0, j = 0; i < lenRef; )
   {
      if(ref[i] == '?')
      {
         j++; i++;
      }

      else if(ref[i] == '*')
      {
         for(k = i+1; k < lenRef; k++)
         {
            if(ref[k] == '?' || ref[k] == '*')
               break;
         }
         if(k-1 >= i+1)
         {
            sub = ref.Middle(i+1,k-1);
            long pos = src.FindSubStr(j,sub.Str());
            if(pos == -1)
            {
               return(0);
            }
            j = pos+sub.Size();
            i = k;
         }
         else
         {
            j = lenSrc;
            i = k;
         }
      }
      else
      {
         if(ref[i] != src[j])
         {
            return(0);
         }
         j++; i++;
      }
   }
   if(j < lenSrc)
      return(0);

   return(1);
}

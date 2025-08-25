#include "stdafx.h"
#include "../Global files/includesDLL.h"
#include <process.h>

#define VERSION 1.0

#define ABORT -5
#define QUOTE ('"')

// Locally defined procedure and global variables

EXPORT short AddCommands(char*, char*, DLLParameters*);
EXPORT void  ListCommands(void);
EXPORT bool GetCommandSyntax(char* cmd, char* syntax);
short Evaluate(DLLParameters*,char *arg);
long nint(float num);
short HelpFolder(DLLParameters*,char *args);

short EvaluateSimpleExpression(char* expression, Variable* result);
short CheckForSimpleEval(char* str);
bool IsOperator(char ch);
bool IsEscapedQuote(char* str, long i, long len);

/*******************************************************************************
    Extension procedure to add commands to Prospa 
********************************************************************************/

EXPORT short  AddCommands(char *command, char *parameters, DLLParameters *dpar)
{
   short r = RETURN_FROM_DLL;

   if(!strcmp(command,"evaluate"))         r = Evaluate(dpar,parameters);                          

   return(r);
}

/*******************************************************************************
    Extension procedure to list commands in DLL 
********************************************************************************/

EXPORT void  ListCommands(void)
{
   TextMessage("\n\n   Evaluate Module (V1.00)\n\n");
   TextMessage("   evaluate ... evaluate an expression\n");
}

/*******************************************************************************
    Extension procedure to return syntax in DLL 
********************************************************************************/

EXPORT bool GetCommandSyntax(char* cmd, char* syntax)
{
   syntax[0] = '\0';
   if(!strcmp(cmd,"evaluate"))  strcpy(syntax,"VARIOUS result = evaluate(STR expression)");

   if(syntax[0] == '\0')
      return(false);
   return(true);
}

/*******************************************************************************
  Return the name of the help file
*******************************************************************************/

short HelpFolder(DLLParameters* par,char *args)
{
   par->retVar[1].MakeAndSetString("DLLs\\Evaluate");
   par->nrRetVar = 1;
   return(OK);
}

short Evaluate(DLLParameters* par, char *arg)
{
// Some variables
   CText expression = "";
   Variable result;

   short r;

   if((r = ArgScan(par->itfc,arg,1,"expression","e","t",&expression)) < 0)
      return(r); 

   Interface* itfc = (Interface*)par;

   r = CheckForSimpleEval(expression.Str());

  // if (r == 1)
  //    r = EvaluateSimpleExpression(expression.Str(), &result);
   //else if (r == 0)
  //    r = EvaluateComplexExpression(itfc, expression, result);


   par->retVar[1].MakeAndSetFloat(r);

   par->nrRetVar = 1;

   return(OK);
}


/**********************************************************************
  Take 'expression' and evaluate it as if it was a mathematical
  string. The result is returned in variable "result" while the
  data type is returned directly.

  Note a 'simple expression' should only contain a single operand
  and no operators.
**********************************************************************/

//
//short EvaluateSimpleExpression(char* expression, Variable* result)
//{
//   short resultType, retType;
//
//   // Do a quick check to see if we are just evaluating a variable
//   Variable* var;
//   if ((var = GetVariable(itfc, ALL_VAR | DO_NOT_RESOLVE, expression, resultType)) != NULL)
//   {
//      //	   if(var == ansVar) // Don't bother copying if var already equals ansVar
//      //	      return(ansVar->GetType());
//
//      switch (mode)
//      {
//         case(FULL_COPY): // Copy the variable to ansvar
//         {
//            if (CopyVariable(result, var, FULL_COPY) == ERR)
//               return(ERR);
//            return(resultType);
//         }
//         case(RESPECT_ALIAS): // Just copy as an alias
//         {
//            if (CopyVariable(result, var, RESPECT_ALIAS) == ERR)
//               return(ERR);
//            return(resultType);
//         }
//      }
//   }
//
//   // Checking for strings which are too short or too long
//   if (strlen(expression) == 0)
//   {
//      ErrorMessage("empty expression");
//      return(ERR);
//   }
//
//   // Work our way through 'expression' evaluating as we go.
//   // i points to the character we are currently interpreting
//   short i = 0;
//
//   // Skip any white space
//   SkipWhiteSpace(expression, i);
//
//   // Evaluate bracketed expression e.g. (2+4/5) 
//   if (expression[i] == '(')
//   {
//      retType = EvaluateBracketedExpression(itfc, expression, i, result);
//   }
//
//   // Extract next operand e.g. '-12.3' or 'v1'   
//   else
//   {
//      short datType;
//      CText operand;
//
//      // Get next operand e.g. 3, 2.12, var, v1 ...
//      datType = GetNextOperandCT(i, expression, operand);
//
//      if (datType == ERR)
//      {
//         return(ERR);
//      }
//
//      // Check and evaluate array operand here e.g. v1[23] or m1[1,2] or m1[1,~]
//      if (expression[i] == '[')
//      {
//         retType = EvaluateArrayOperand(itfc, operand.Str(), expression, i, result);
//      }
//
//      // Check and evaluate function here e.g. log(y)     
//      else if (expression[i] == '(')
//      {
//         retType = EvaluateFunctionOperand(itfc, operand.Str(), expression, i, result);
//      }
//
//      // No brackets so must be a simple operand e.g. 12.3, "Hi", var       
//      else
//      {
//         retType = EvaluateSimpleOperand(itfc, operand.Str(), datType, result);
//      }
//   }
//
//   return(retType);
//}


/*************************************************************
   Check to see if a string is a simple expression
   such as b,  b[], b() or ().
   A complex expression includes operators.
   Also check to see that all brackets and quotes are matched
   Return 1 if simple otherwise 0 false return ERR if error
*************************************************************/

short CheckForSimpleEval(char* str)
{
   short inBracket = 0;
   short inArray = 0;
   short inCArray = 0;
   short inList = 0;
   //   bool inCFunction = false;
   bool inString = false;
   bool bracketFound = false; // An array or argument list has been seen
   short len = strlen(str);

   for (short i = 0; i < len; i++)
   {
      if (str[i] == ' ' || str[i] == '\t') continue; // Ignore white space
      if (str[i] == '\"' && !IsEscapedQuote(str, i, len))
         inString = !inString;
      if (inString) continue;

      if (str[i] == '(') { inBracket++; bracketFound = 1; }
      if (str[i] == ')') { inBracket--; continue; }
      if (inBracket) continue;

      if (str[i] == '[') { inArray++; bracketFound = 1; }
      if (str[i] == ']') { inArray--; continue; }
      if (inArray) continue;

      if (i < len - 1 && str[i] == '<' && str[i + 1] == '<') { inList++; i++; bracketFound = 1; }
      if (i < len - 1 && str[i] == '>' && str[i + 1] == '>') { inList--; i++; continue; }
      if (inList) continue;

      if (str[i] == '{') { inCArray++; bracketFound = 1; }
      if (str[i] == '}') { inCArray--; continue; }

      if (inCArray) continue;

      if (IsOperator(str[i])) // Any operators 
         return(0);
   }

   // Check to see if there are any missing delimiters
   if (inString)
   {
      ErrorMessage("unmatched quotes '\" \"' in expression");
      return(ERR);
   }

   if (inBracket)
   {
      ErrorMessage("unmatched brackets '(...)' in expression");
      return(ERR);
   }

   if (inArray)
   {
      ErrorMessage("unmatched brackets '[...]' in expression");
      return(ERR);
   }

   if (inList)
   {
      ErrorMessage("unmatched brackets '<<...>>' in expression");
      return(ERR);
   }

   if (inCArray)
   {
      ErrorMessage("unmatched brackets '{...}' in expression");
      return(ERR);
   }

   return(1); // Yes is a simple expression
}

/******************************************************************************
*  Check to see if a single character is an operator
******************************************************************************/

bool IsOperator(char ch)
{
   short op = (unsigned char)ch;

   // Check for single valid operators
   if (op == '*' || op == '/' || op == '+' || op == '-' ||
      op == '^' || op == '!' || op == '>' || op == '<' ||
      op == '%' || op == '|' || op == '&' || op == '=' ||
      op == '\'') // || op == '.') // ARROW
      return(true);

   return(false);
}


// See if the position 'i' in a string 'str' represents an escaped quote  \" (starting at ")

bool IsEscapedQuote(char* str, long i, long len)
{
   if (i == 0 || str[i] != QUOTE)
      return(false);

   else if (i == 1)
   {
      return(str[0] == '\\'); // True if \"
   }
   else
   {
      if (str[i - 1] == '\\')
      {
         long cnt = 1;
         for (long j = i - 2; j > 0; j--) // Search backwards for matched pairs of backslashes
         {
            if (str[j] != '\\')
               break;
            cnt++;
         }
         return(cnt % 2); // Check for escaped character
      }
      return(false);
   }
}

/*****************************************************************************************
*                           Return the nearest integer to the float num                  *
*****************************************************************************************/

long nint(float num)
{
   if(num > 0)
      return((long)(num+0.5));
   else
      return((long)(num-0.5));

}

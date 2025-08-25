#include "stdafx.h"
#include "allocate.h"
#include "control.h"
#include "evaluate.h"
#include <math.h>
#include "cArg.h"
#include "evaluate_complex.h"
#include "evaluate_simple.h"
#include "globals.h"
#include "guiWindowClass.h"
#include "guiWindowsEvents.h"
#include "interface.h"
#include "list_functions.h"
#include "macro_class.h"
#include "main.h"
#include "mymath.h"
#include "operators.h"
#include "scanstrings.h"
#include "string_utilities.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "memoryLeak.h"

#define RANGE     0
#define SPECIFIED 1

// Variable scopes.
#define VLOCAL  1
#define VGLOBAL 2
#define VWINDOW 4
#define VALL    7

bool testMode = false;
short precedence[256]; 
int ReplaceSubExpressions(Interface *itfc, char args[]);

/*********************************************************************************
* Routines to take a simple mathematical expression and return the solution      *
*********************************************************************************/

#define makepositive(op) ((op > 0) ? (op) : (256+op))

typedef struct
{
   unsigned char   op;         // Current operator (+, -, *, / ...)
   char    type;               // Type of operator (u/b)
}
OpStk; // Operation stack - used by Evaluate

// Locally defined and used procedures
void StrReplace(char *str, short pos, char *replace, short lenR);
char* ReturnExpandedOperation(unsigned char operation);
bool IsOperator(char ch);
short ExtractRealMatrixOperandCT(short &pos, char str[], CText &operand);
short ExtractQuotelessStringListOperand(short &pos, char str[], char operand[]);
short ExtractStringOperandCT(short &pos, char str[], CText &operand);
short ExtractComplexMatrixOperandCT(short &pos, char str[], CText &operand);
short ExtractQuotelessStringListOperandCT(short &pos, char str[], CText &operand);

short ExtractStringOperand(short &pos, char str[], char *operand);
short ExtractRealMatrixOperand(short &pos, char str[], char *operand);
short ExtractComplexMatrixOperand(short &pos, char str[], char *operand);

short IsOperator(char ch, char nextch);
void AppendToVarList(CText &list, char *name, long &cnt);
int VariableTypeID(Variable* var);

/********************************************************************
* Each mathematical operator has a different precedence. This       *
* routine sets up this precedence in a lookup table which is later  *
* used by the function evaluate to process complex expression       *
* contained different operator types e.g. x = -(2+3*(4^2-3/2))      *
********************************************************************/

void SetUpPrecedence()
{
   short i;

// The following table lists operator symbols and their precedence   

   for(i = 0; i < 255; i++)
   {
      if((char)i == '[')      precedence[i] = 12;
      else if((char)i == '{') precedence[i] = 12;
      else if(i == ARROW) precedence[i] = 11; // ->
 //     else if((char)i == '.') precedence[i] = 11;  ARROW
      else if((char)i == '^') precedence[i] = 10;
      else if((char)i == '!') precedence[i] = 10; 
      else if((char)i == '\'') precedence[i] = 10; 
      else if((char)i == QUOTE)precedence[i] = 10; 
      else if(i == NEG)       precedence[i] = 9; 
      
      else if((char)i == '*') precedence[i] = 8;
      else if(i == MATMUL)    precedence[i] = 8; // .*
      else if((char)i == '/') precedence[i] = 8;
      else if((char)i == '%') precedence[i] = 8;  
      else if((char)i == '+') precedence[i] = 7;
      else if((char)i == '-') precedence[i] = 7;
      else if(i == EQ)        precedence[i] = 6; // ==
      else if((char)i == '=') precedence[i] = 6;
      else if((char)i == '>') precedence[i] = 6;
      else if(i == GEQ)       precedence[i] = 6; // >=
      else if((char)i == '<') precedence[i] = 6;
      else if(i == NEQ)       precedence[i] = 6; // !=
      else if(i == LEQ)       precedence[i] = 6; // <=
      else if((char)i == '|') precedence[i] = 5;
      else if((char)i == '&') precedence[i] = 5;
      else if((char)i == ')') precedence[i] = 2;
      else if((char)i == '(') precedence[i] = 1;
      else precedence[i] = 0;
   }
}


char GetNumType(float number)
{
   if(nint(number) == number)
      return(INTEGER);
   else
      return(FLOAT32);
}

char GetNumType(double number)
{
   if ((long long)(number+0.5) == number)
      return(INTEGER);
   else
      return(FLOAT32);
}

/*************************************************************
   Evaluate the expression in the argument and return results
    in ansVar (also replaces expressions between $$ with 
    their value first).
   Can also convert a list into a 1D real, double or complex
	 vector by using the option type argument
*************************************************************/

int EvaluateExpression(Interface *itfc, char args[])
{
   Variable expressionVar;
	CText type = "float";
   short n,r;
   char name[MAX_STR];
	char value[MAX_STR];

   if((n = ArgScan(itfc,args,1,"Expression, [type]","ee","vt",&expressionVar,&type)) < 0)
      return(n);

	if(expressionVar.GetType() == UNQUOTED_STRING)
	{
		CText expression = expressionVar.GetString();
      
		if((r = ReplaceVarInString(itfc,expression)) == ERR)
			return(ERR);
      
		r = Evaluate(itfc, RESPECT_ALIAS, expression.Str(), &itfc->retVar[1]);
		return(r);
	}
	else if(expressionVar.GetType() == LIST) 
	{
		Variable retVar;
		CText temp;
		CText txt;
		char **list = expressionVar.GetList();
		int sz = expressionVar.GetDimX();
	   char **newList = NULL;

		if(ParseAssignmentString(list[0],name,value) == OK) // Evaluate all values in assignment list and return as a new list
		{
			for(int i = 0; i < sz; i++)
			{
		      if(ParseAssignmentString(list[i],name,value) == OK)
				{
					if(Evaluate(itfc, RESPECT_ALIAS, value, &retVar) != ERR)
					{
						ConvertVariableToText(&retVar, temp, "", false);	

						if(retVar.GetType() == UNQUOTED_STRING)
						{
							if(Evaluate(itfc, RESPECT_ALIAS, temp.Str(), &retVar) != ERR)
								ConvertVariableToText(&retVar, temp, "", false);	
							else
								temp = value;
						}
					}
					else
						temp = value;

					txt = name;
					txt = txt + " = " + temp;
               AppendStringToList(txt.Str(), &newList, i);
				}
				else
				{
					ErrorMessage("not all entries in list are assignments");
					return(ERR);
				}
			}
		   itfc->retVar[1].AssignList(newList, sz);
			itfc->nrRetValues = 1;
			return(OK);
		}
		else // Convert list to a real, double or complex vector
		{

			if(type == "float")
			{
				float **m = MakeMatrix2D(sz,1);
				for(int i = 0; i < sz; i++)
				{
					CText expression = list[i];
					if(ReplaceVarInString(itfc,expression) == ERR)
						return(ERR);
      
					if(Evaluate(itfc, RESPECT_ALIAS, expression.Str(), &retVar) == ERR)
						return(ERR);

					if(retVar.GetType() == FLOAT32)
					{
						m[0][i] = retVar.GetReal();
					}
					else if(retVar.GetType() == FLOAT64)
					{
						m[0][i] = (float)retVar.GetDouble();
					}
					else
					{
						ErrorMessage("List entry '%s' should evaluate to a real number", list[i]);
						return(ERR);
					}
				}
				itfc->retVar[1].AssignMatrix2D(m,sz,1);
			}
			else if(type == "double")
			{
				double **m = MakeDMatrix2D(sz,1);
				for(int i = 0; i < sz; i++)
				{
					CText expression = list[i];
					if(ReplaceVarInString(itfc,expression) == ERR)
						return(ERR);
      
					if(Evaluate(itfc, RESPECT_ALIAS, expression.Str(), &retVar) == ERR)
						return(ERR);

					if(retVar.GetType() == FLOAT32)
					{
						m[0][i] = (double)retVar.GetReal();
					}
					else if(retVar.GetType() == FLOAT64)
					{
						m[0][i] = retVar.GetDouble();
					}
					else
					{
						ErrorMessage("List entry '%s' should evaluate to a floating point number", list[i]);
						return(ERR);
					}
				}
				itfc->retVar[1].AssignDMatrix2D(m,sz,1);
			}
			else if(type == "complex")
			{
				complex **m = MakeCMatrix2D(sz,1);
				for(int i = 0; i < sz; i++)
				{
					CText expression = list[i];
					if(ReplaceVarInString(itfc,expression) == ERR)
						return(ERR);
      
					if(Evaluate(itfc, RESPECT_ALIAS, expression.Str(), &retVar) == ERR)
						return(ERR);

					if(retVar.GetType() == FLOAT32)
					{
						m[0][i].r = retVar.GetReal();
						m[0][i].i = 0.0;
					}
					else if(retVar.GetType() == FLOAT64)
					{
						m[0][i].r = retVar.GetDouble();
						m[0][i].i = 0.0;
					}
					else if(retVar.GetType() == COMPLEX)
					{
						m[0][i] = retVar.GetComplex();
					}
					else
					{
						ErrorMessage("List entry '%s' should evaluate to a real or complex number", list[i]);
						return(ERR);
					}
				}
				itfc->retVar[1].AssignCMatrix2D(m,sz,1);
			}
			else
			{
				ErrorMessage("Invalid type should be float/double/complex");
				return(ERR);
			}

			itfc->nrRetValues = 1;
			return(OK);
		}
	}
	else
	{
		ErrorMessage("Only strings and 1D lists are supported by the eval command");
		return(ERR);
	}
   
   return(OK);
}

int EvaluateSubExpression(Interface* itfc, char args[])
{
   CText expression;
   short n,r;
   
   if((n = ArgScan(itfc,args,1,"Expression","e","t",&expression)) < 0)
      return(n);
      
   if((r = ReplaceVarInString(itfc,expression)) == ERR)
      return(ERR);
      
   itfc->retVar[1].MakeAndSetString(expression.Str());
   itfc->nrRetValues = 1;
   
   return(r);
}

/*************************************************************
      Convert a real number to a string and return to user
*************************************************************/
  
int RealToStr(Interface* itfc ,char args[])
{
   short n;
   float result;
   CText txt;
   
   if((n = ArgScan(itfc,args,1,"real expression","e","f",&result)) < 0)
      return(n);

   txt.Format("%g",result);
   itfc->retVar[1].MakeAndSetString(txt.Str()); 
   itfc->nrRetValues = 1;
   return(OK);
}


/*************************************************************
   Check to see if a string is a simple expression
   such as b,  b[], b() or ().
   A complex expression includes operators.
   Also check to see that all brackets and quotes are matched
   Return 1 if simple otherwise 0 false return ERR if error
*************************************************************/
  
short CheckForSimpleEval(char *str)
{
   short inBracket = 0;
   short inArray = 0;
   short inCArray = 0;
   short inList = 0;
//   bool inCFunction = false;
   bool inString = false;
   bool bracketFound = false; // An array or argument list has been seen
   short len = strlen(str);
   
   for(short i = 0; i < len; i++)
   {
      if(str[i] == ' ' || str[i] == '\t') continue; // Ignore white space
      if(str[i] == '\"' && !IsEscapedQuote(str,i,len))
         inString = !inString;
      if(inString) continue;

      if(str[i] == '(') {inBracket++; bracketFound = 1;}
      if(str[i] == ')') {inBracket--; continue;}
      if(inBracket) continue;
      
      if(str[i] == '[') {inArray++; bracketFound = 1;}
      if(str[i] == ']') {inArray--; continue;}
      if(inArray) continue;

      if(i < len-1 && str[i] == '<' && str[i+1] == '<') {inList++; i++; bracketFound = 1;}
      if(i < len-1 && str[i] == '>' && str[i+1] == '>') {inList--; i++; continue;}
      if(inList) continue;

      if(str[i] == '{') {inCArray++; bracketFound = 1;}
      if(str[i] == '}') {inCArray--; continue;}

   //   if(str[i] == '.') {inCFunction = true;}

      if(inCArray) continue;
      
   //   if(bracketFound && !inCFunction) 
   //      return(0); // Any characters after array or argument list => complex expression

      if(IsOperator(str[i])) // Any operators 
         return(0);

   }

// Check to see if there are any missing delimiters
   if(inString)
   {
      ErrorMessage("unmatched quotes '\" \"' in expression");
      return(ERR);
   }
      
   if(inBracket)
   {
      ErrorMessage("unmatched brackets '(...)' in expression");
      return(ERR);
   }

   if(inArray)
   {
      ErrorMessage("unmatched brackets '[...]' in expression");
      return(ERR);
   }

   if(inList)
   {
      ErrorMessage("unmatched brackets '<<...>>' in expression");
      return(ERR);
   }
 
   if(inCArray)
   {
      ErrorMessage("unmatched brackets '{...}' in expression");
      return(ERR);
   }     
         
   return(1); // Yes is a simple expression
}


/*********************************************************************************
   Returns that part of the expression string where an error has occurred
*********************************************************************************/

char* GetOperationString(short type, unsigned char operation)
{
  short type_left = (type & 0xFF00)>>8;
  short type_right = (type & 0x00FF);
  char typeStr1[20];
  char typeStr2[20];
  static char response[MAX_STR];

  GetVariableTypeAsString(type_left,typeStr1);
  GetVariableTypeAsString(type_right,typeStr2);
  
  sprintf(response,"%s %s %s",typeStr2,ReturnExpandedOperation(operation),typeStr1);
  
  return(response);
}


/*********************************************************************************
   Convert operation into a string expanding those special characters which cannot
   be displayed as a single character
*********************************************************************************/

char* ReturnExpandedOperation(unsigned char operation)
{
   static char op[2] = "";
   
   switch(operation)
   {
      case LEQ :    return("<="); break;
      case GEQ :    return(">="); break;
      case NEQ :    return("!="); break;
      case MATMUL : return(".*"); break;
      case NEG :    return("-"); break;
      case EQ :     return("=="); break;
      case ARROW :  return("->"); break;
      default:
         op[0] = operation;
         op[1] = '\0';
         return(op);
   }
}
           
void SkipWhiteSpace(char *expression, short &i)
{
    while(expression[i] == ' ' || expression[i] == '\t') i++;
}

/********************************************************************
* Checks to see if the current character is a sign (+/-) and        *
* returns this information as a +1 or -1 integer.                   *
********************************************************************/

short CheckForSign(char *expression, short &i)
{
   short sign;
   
   if(expression[i] == '-')
      sign = -1, i++;
   else if(expression[i] == '+')
      sign = 1, i++;
   else
      sign = 1; 
      
   return(sign);
}
  
/*******************************************************************
* Extract sub-expression imbedded inside a larger expression and    *
* enclosed by delimiters delleft and delright.                      * 
* Procedure assumes that delleft has already be detected            *
* in position expression[i].                                        *
* This routine allows nesting of the delimiter and so will only     *
* return the matching delimiter.                                    *
********************************************************************/

short ExtractSubExpression(char expression[],char subexpression[],short &i,char delleft,char delright)
{
  short cnt = 1;
  short j,len;
   bool inString = false;

  i++;
  len = strlen(expression);
  
  for(j = i; j < len; j++) 
  {
	   if(IsUnEscapedQuote(expression,j,len)) //ESCAPE
      {                                                        
         if(inString)
           inString = false;
         else
           inString = true;
      }
            
	   if(inString)
      {
         subexpression[j-i] = expression[j];
         continue;
      }

      if(expression[j] == delleft) cnt++;
      if(expression[j] == delright)
      {
         if(cnt == 1)
         {
            subexpression[j-i] = '\0';
            cnt--;
            break;
         }
         else
         {
            cnt--;
         }
      }
      subexpression[j-i] = expression[j];
   }
   if(cnt != 0)
   {
      ErrorMessage("unmatched bracket in expression");
      return(ERR);
   }
   i = j+1;
   return(0);
}   


/*******************************************************************
* Searches str until an operator is found. If none is found the     *
* lowest precedence operator '&' is returned. Note that some        *
* operators are double characters such as <= and >=.                *
********************************************************************/

#define INVALID_SINGLE_OP 300
#define INVALID_DOUBLE_OP 301

short GetNextOperator(short &pos,char str[],unsigned char &op)
{
   short len;
   short result;
    
   len = strlen(str);

// Skip white space
   while(pos < len && (str[pos] == ' ' || str[pos] == '\t') ) pos++;
// Check for end of string
   if(pos >= len) 
   {
      op = '&';
      return(END);
   }

// Check for valid operator
   result = IsOperator(str[pos],str[pos+1]);

   if(result == INVALID_SINGLE_OP)
   {
      ErrorMessage("invalid operator '%c'",str[pos]); // Invalid single operator
      return(ERR);
   }
   else if(result == INVALID_DOUBLE_OP)
   {
      ErrorMessage("invalid operator '%c%c'",str[pos],str[pos+1]); // Invalid double operator
      return(ERR);
   }

// Was it a double operator?
   if(result >= 128) // All single operators have ASCII code < 128
      pos++;
   pos++;

// Return operator
   op = (unsigned char)result;
   return(OK);
}

/*****************************************************************************
       Check to see if two characters constitute an operator
       Possibilities include:
             operator not_operator
             ' operator (transpose followed by operator)
             dual_operator (==, >=, <=, .*, !=)
******************************************************************************/
 
short IsOperator(char ch, char nextch)
{
   short op = (unsigned char)ch;
   short nextop = (unsigned char)nextch;
   
   bool isop1 = IsOperator(ch);
   bool isop2 = IsOperator(nextch);

// Only one operator then a non-operator
   if(isop1 && !isop2)
      return(op);

// Only one operator and a unitary plus or minus
   if(isop1 && (nextop == '-' || nextop == '+'))
      return(op);

// Transpose and a operator
   if(op == '\''&& isop2)
      return(op);

// Two operators
   if(op == '<' && nextop == '=')      // <=
      return(LEQ);
   else if(op == '=' && nextop == '=') // ==
      return(EQ);      
   else if(op == '>' && nextop == '=') // >=
      return(GEQ);   
   else if(op == '!' && nextop == '=') // !=
      return(NEQ); 
   else if(op == '.' && nextop == '*') // .*
      return(MATMUL); 
   else if(op == '-' && nextop == '>') // ->
      return(ARROW);

   return(INVALID_SINGLE_OP + isop2); // INVALID_DOUBLE_OP is INVALID_SINGLE_OP+1
}

/******************************************************************************
*  Check to see if a single character is an operator
******************************************************************************/
 
bool IsOperator(char ch)
{
   short op = (unsigned char)ch;

// Check for single valid operators
   if(op == '*' || op == '/' || op == '+' || op == '-'  || 
      op == '^' || op == '!' || op == '>' || op == '<'  ||
      op == '%' || op == '|' || op == '&' || op == '='  ||
      op == '\'') // || op == '.') // ARROW
      return(true);
   
   return(false);
}



/********************************************************************
  Extracts next operand from str starting at position pos returns   
  the data type and updates pos. Data types are either:             
                                                                    
  UNQUOTED_STRING  |a-z,A-Z|ANYTHING                                
  QUOTED_STRING      "ANYTHING"                                     
  FLOAT32           2.34 or 2.34e-1 or 0x23                           
  COMPLEX         FLOATj FLOATi or jFLOAT iFLOAT                    
  MATRIX2D          [ANYTHING,ANYTHING, ...]                         
  CMATRIX2D         {ANYTHING,ANYTHING, ...}                         
********************************************************************/

#define START 		1
#define NUMBER 	2
#define POINT 		3
#define FRACTION 	4
#define ESYMBOL 	5
#define ESIGN		6		
#define EXPONENT	7
#define SUFFIX 	8
#define HEX       9
#define MEMBER    10

/**********************************************************************
   Find the next operand in string 'str' starting from position 'pos'
   also return the data type.
**********************************************************************/

short GetNextOperandCT(short &pos, char str[], CText &operand)
{
   short i,j=0,len;
   short type;
   short subtype = 0;
   short mode = START;
   char c = str[pos];

   operand.Reset();
   len = strlen(str);

// Ignore leading blanks *****************************************
   while(str[pos] == ' ') (pos)++;
   
// Try and figure out the operand type based on first character **       
   if(str[pos] == QUOTE) // Must be a string  
   {
      short r;
      r = ExtractStringOperandCT(++pos,str,operand);
      return(r);
   }
   else if(str[pos] == '[') // Must be a matrix  
   {
      return(ExtractRealMatrixOperandCT(++pos,str,operand));
   }
   else if(str[pos] == '{') // Must be a complex matrix 
   {
      return(ExtractComplexMatrixOperandCT(++pos,str,operand));
   }
   else if(pos < len-1 && str[pos] == '<' && str[pos+1] == '<') // Must be a quoteless list  
   {
      pos += 2;
      return(ExtractQuotelessStringListOperandCT(pos,str,operand));
   }
   else
      type = FLOAT32; // Otherwise assume its a number for now
   
// Start scanning from current position in expression string *****
   for(i = pos; i < len; i++)
   {
      c = str[i];

   // Check for to see if this character is consistent with a valid real number
      if(type == FLOAT32 || type == FLOAT64)
      {
         if(type == FLOAT64)
         {
            if(IsWhiteSpace(c)) // White space, so end of operand
            {
               i++;
               break;
            }
            else if(IsOperandDelimiter(str,i,len)) // Operator, so end of operand
            {
               if(j == 0)
                  type = OPERATOR_TOKEN;
               break;
            }
            else
            {
               type = UNQUOTED_STRING; // Not a number
               operand.Append(c);
               continue;
            }
         }

         if(c == '-' || c == '+') // Sign
         {
            if(i == 0)
               mode = START; // Its a unitary operator
            else if(mode == ESYMBOL)
               mode = ESIGN; // Its an exponent sign
            else
               break; // This symbol is an operator
         }
         else if((i-pos == 1) && (c == 'x' || c == 'X') && str[pos] == '0') // Start of hex number
         {
            type = FLOAT32;
            mode = HEX;
         }
         else if(mode == HEX && ((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) // Hex number
         {
            type = FLOAT32;
            mode = HEX;
         }
         else if(isdigit(c)) // Digit  ddd.dddEddd
         {
            if(mode == START)
               mode = NUMBER;
            else if(mode == NUMBER)
               mode = NUMBER;
            else if(mode == HEX)
               mode = HEX;
            else if(mode == EXPONENT)
               mode = EXPONENT;
            else if(mode == FRACTION)
               mode = FRACTION;
            else if(mode == POINT)
               mode = FRACTION;
            else if(mode == ESIGN || mode == ESYMBOL)
               mode = EXPONENT;
            else
               type = UNQUOTED_STRING;
         }
         else if(c == '.' && str[i+1] != '*') // Point
         { 
            if(mode == NUMBER || mode == START)
               mode = POINT;
            else
               type = UNQUOTED_STRING; // Wrong place!
         }
         else if(c == 'e' || c == 'E') // Exponent
         {
            if(mode == NUMBER || mode == FRACTION)
               mode = ESYMBOL;
            else
               type = UNQUOTED_STRING; // Wrong place
         } 
         else if(mode != START && c == 'd') // End of double
         {
            type = FLOAT64;
         }
         else if(c == 'i' || c == 'j') // Complex number
         {
            if(i == pos) // i[number]
            {
               operand.Append(c);
               j++;
               type = COMPLEX;
               mode = START;
               continue; // make sure number is valid
            }
            else if(IsOperandDelimiter(str,i+1,len)    || // i[operator] or i[EOL]
                     (str[i+1] == ' '))                   // i[space]
            {
               operand.Append(c);
               j++;
               type = COMPLEX;
               i++;
               break;
            }
            else
            {
               ErrorMessage("invalid complex number");
               return(ERR);
            }
         }  
         else if(IsWhiteSpace(c)) // White space, so end of operand
         {
            i++;
            break;
         }
         else if(IsOperandDelimiter(str,i,len)) // Operator, so end of operand
         {
            if(j == 0)
               type = OPERATOR_TOKEN;
            break;
         }
         else if(mode == START)
            type = UNQUOTED_STRING;
         else
         {
            type = UNQUOTED_STRING;
         }
      } // END FLOAT32             

   // Check for to see if this character is consistent with a valid complex number
      else if(type == COMPLEX)
      {
         //if(i-pos == 1 && (c == 'x' || c == 'X'))
         //{
         //   type = UNQUOTED_STRING;
         //   break;
         //}
         //else
         if(isdigit(c))
         {
            if(mode == START)
               mode = NUMBER;
            else if(mode == NUMBER)
               mode = NUMBER;
            else if(mode == EXPONENT)
               mode = EXPONENT;
            else if(mode == FRACTION)
               mode = FRACTION;
            else if(mode == POINT)
               mode = FRACTION;
            else if(mode == ESIGN || mode == ESYMBOL)
               mode = EXPONENT;
            else
               type = UNQUOTED_STRING;
         }
         else if(c == '.' && str[i+1] != '*')
         { 
            if(mode == NUMBER || mode == START)
               mode = POINT;
            else
            {
               type = UNQUOTED_STRING;
            }
         }
         else if(c == 'e' || c == 'E')
         {
            if(mode == NUMBER || mode == FRACTION)
               mode = ESYMBOL;
            else
            {
               type = UNQUOTED_STRING;
            }
         } 
         else if(c == 'i' || c == 'j') // Shouldn't appear again in complex nr.
         {
            type = UNQUOTED_STRING;
         }  
         else if(c == ' ' || c == '\t') // End of complex nr definition
         {
            i++;
            break;
         }
         else if(IsOperandDelimiter(str,i,len)) // End of complex nr definition
         {
            if(j == 0)
               type = OPERATOR_TOKEN;
            break;
         }
         else
         {
            type = UNQUOTED_STRING;
         }
      } // END COMPLEX

   // Check for end of variable or string *********
      if(type == UNQUOTED_STRING)
      {
         if(c == ' ' || c == '\t') // A space signs the end (variables can't include spaces)
         {
            i++;
            break;
         } 
         else if(c == '-' &&  str[i+1] == '>') // ARROW operator so must be a structure member reference
         {
            if(j > 0 && j < len-1)
            {
               subtype  = MEMBER;
               operand.Append(c);
               i++;
               c = str[i];
            }
         }
         else if(IsOperandDelimiter(str,i,len) && c != '"') // An operator also signals the end
         {                                                          
            break;
         }

      } // END UNQUOTED_STRING

     // if(type == CLASS)
      //{
      //   if(IsOperandDelimiter(str,i,len)) // An operator also signals the end
      //      break;
      //}

                  
   // Load up operand string *********************
      operand.Append(c); 
      j++;
   }
   
   if(mode == ESYMBOL || mode == ESIGN)
      type = UNQUOTED_STRING;

   if(type == UNQUOTED_STRING && subtype == MEMBER)
      type = STRUCTMEMBER;

// Float 64s have a d at the end - need to remove it
   if(type == FLOAT64)
   {
      if(operand[j-1] == 'd')
         operand[j-1] = '\0';
      else
         type = UNQUOTED_STRING;
   }

// Long reals become doubles
   // 'd' removal
   //   extern bool IsDoubleByPrecision(char *str);
   //if(type == FLOAT32)
   //{ 
   //   if(IsDoubleByPrecision(operand.Str()))
   //      type = FLOAT64;
   //}

   pos = i;
   return(type);
}  

/**********************************************************************
   Find the next operand in string 'str' starting from position 'pos'
   also return the data type.

   This version ignores the arrow operator 
**********************************************************************/

short GetNextOperandCTB(short &pos, char str[], CText &operand)
{
   short i,j=0,len;
   short type;
   short subtype = 0;
   short mode = START;
   char c = str[pos];

   operand.Reset();

// Ignore leading blanks *****************************************
  while(str[pos] == ' ') (pos)++;
   
// Try and figure out the operand type based on first character **       
   if(str[pos] == QUOTE) // Must be a string  
   {
      short r;
      r = ExtractStringOperandCT(++pos,str,operand);
      return(r);
   }
   else if(str[pos] == '[') // Must be a matrix  
   {
      return(ExtractRealMatrixOperandCT(++pos,str,operand));
   }
   else if(str[pos] == '{') // Must be a complex matrix 
   {
      return(ExtractComplexMatrixOperandCT(++pos,str,operand));

   }
   else
      type = FLOAT32; // Otherwise assume its a number for now
   
// Start scanning from current position in expression string *****
   len = strlen(str);
   for(i = pos; i < len; i++)
   {
      c = str[i];

   // Check for to see if this character is consistent with a valid real number
      if(type == FLOAT32 || type == FLOAT64)
      {
         if(type == FLOAT64)
         {
            if(IsWhiteSpace(c)) // White space, so end of operand
            {
               i++;
               break;
            }
            else if(IsOperandDelimiter(str,i,len)) // Operator, so end of operand
            {
               if(j == 0)
                  type = OPERATOR_TOKEN;
               break;
            }
            else
            {
               type = UNQUOTED_STRING; // Not a number
               operand.Append(c);
               continue;
            }
         }

         if(c == '-' || c == '+') // Sign
         {
            if(i == 0)
               mode = START; // Its a unitary operator
            else if(mode == ESYMBOL)
               mode = ESIGN; // Its an exponent sign
            else
               break; // This symbol is an operator
         }
         else if((i-pos == 1) && (c == 'x' || c == 'X') && str[pos] == '0') // Start of hex number
         {
            type = FLOAT32;
            mode = HEX;
         }
         else if(mode == HEX && ((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) // Hex number
         {
            type = FLOAT32;
            mode = HEX;
         }
         else if(isdigit(c)) // Digit  ddd.dddEddd
         {
            if(mode == START)
               mode = NUMBER;
            else if(mode == NUMBER)
               mode = NUMBER;
            else if(mode == HEX)
               mode = HEX;
            else if(mode == EXPONENT)
               mode = EXPONENT;
            else if(mode == FRACTION)
               mode = FRACTION;
            else if(mode == POINT)
               mode = FRACTION;
            else if(mode == ESIGN || mode == ESYMBOL)
               mode = EXPONENT;
            else
               type = UNQUOTED_STRING;
         }
         else if(c == '.' && str[i+1] != '*') // Point
         { 
            if(mode == NUMBER || mode == START)
               mode = POINT;
            else
               type = UNQUOTED_STRING; // Wrong place!
         }
         else if(c == 'e' || c == 'E') // Exponent
         {
            if(mode == NUMBER || mode == FRACTION)
               mode = ESYMBOL;
            else
               type = UNQUOTED_STRING; // Wrong place
         } 
         else if(mode != START && c == 'd') // End of double
         {
            type = FLOAT64;
         }
         else if(c == 'i' || c == 'j') // Complex number
         {
            if(i == pos) // i[number]
            {
               operand.Append(c);
               j++;
               type = COMPLEX;
               mode = START;
               continue; // make sure number is valid
            }
            else if(IsOperandDelimiter(str,i+1,len)    || // i[operator] or i[EOL]
                     (str[i+1] == ' '))                   // i[space]
            {
               operand.Append(c);
               j++;
               type = COMPLEX;
               i++;
               break;
            }
            else
            {
               ErrorMessage("invalid complex number");
               return(ERR);
            }
         }  
         else if(IsWhiteSpace(c)) // White space, so end of operand
         {
            i++;
            break;
         }
         else if(IsOperandDelimiter(str,i,len)) // Operator, so end of operand
         {
            if(j == 0)
               type = OPERATOR_TOKEN;
            break;
         }
         else if(mode == START)
            type = UNQUOTED_STRING;
         else
         {
            type = UNQUOTED_STRING;
         }
      } // END FLOAT32             

   // Check for to see if this character is consistent with a valid complex number
      else if(type == COMPLEX)
      {
         //if(i-pos == 1 && (c == 'x' || c == 'X'))
         //{
         //   type = UNQUOTED_STRING;
         //   break;
         //}
        // else
            if(isdigit(c))
         {
            if(mode == START)
               mode = NUMBER;
            else if(mode == NUMBER)
               mode = NUMBER;
            else if(mode == EXPONENT)
               mode = EXPONENT;
            else if(mode == FRACTION)
               mode = FRACTION;
            else if(mode == POINT)
               mode = FRACTION;
            else if(mode == ESIGN || mode == ESYMBOL)
               mode = EXPONENT;
            else
               type = UNQUOTED_STRING;
         }
         else if(c == '.' && str[i+1] != '*')
         { 
            if(mode == NUMBER || mode == START)
               mode = POINT;
            else
            {
               type = UNQUOTED_STRING;
            }
         }
         else if(c == 'e' || c == 'E')
         {
            if(mode == NUMBER || mode == FRACTION)
               mode = ESYMBOL;
            else
            {
               type = UNQUOTED_STRING;
            }
         } 
         else if(c == 'i' || c == 'j') // Shouldn't appear again in complex nr.
         {
            type = UNQUOTED_STRING;
         }  
         else if(c == ' ' || c == '\t') // End of complex nr definition
         {
            i++;
            break;
         }
         else if(IsOperandDelimiter(str,i,len)) // End of complex nr definition
         {
            if(j == 0)
               type = OPERATOR_TOKEN;
            break;
         }
         else
         {
            type = UNQUOTED_STRING;
         }
      } // END COMPLEX

   // Check for end of variable or string *********
      if(type == UNQUOTED_STRING)
      {
         if(c == ' ' || c == '\t') // A space signs the end (variables can't include spaces)
         {
            i++;
            break;
         } 
         //else if(c == '-' &&  str[i+1] == '>') // ARROW operator so must be a structure member reference
         //{
         //   if(j > 0 && j < len-1)
         //   {
         //      subtype  = MEMBER;
         //      operand.Append(c);
         //      i++;
         //      c = str[i];
         //   }
         //}
         else if(IsOperandDelimiter(str,i,len) && c != '"') // An operator also signals the end
         {                                                          
            break;
         }

      } // END UNQUOTED_STRING
                 
   // Load up operand string *********************
      operand.Append(c); 
      j++;
   }
   
   if(mode == ESYMBOL || mode == ESIGN)
      type = UNQUOTED_STRING;

   if(type == UNQUOTED_STRING && subtype == MEMBER)
      type = STRUCTMEMBER;

// Float 64s have a d at the end - need to remove it
   if(type == FLOAT64)
   {
      if(operand[j-1] == 'd')
         operand[j-1] = '\0';
      else
         type = UNQUOTED_STRING;
   }

   pos = i;
   return(type);
}  

short GetNextOperand(short &pos, char str[], char *operand)
{
   short i,j=0,len;
   short type;
   short subtype = 0;
   short mode = START;
   char c = str[pos];

   len = strlen(str);

// Ignore leading blanks *****************************************
  while(str[pos] == ' ') (pos)++;
   
// Try and figure out the operand type based on first character **       
   if(str[pos] == QUOTE) // Must be a string  
   {
      return(ExtractStringOperand(++pos,str,operand));
   }
   else if(str[pos] == '[') // Must be a matrix  
   {
      return(ExtractRealMatrixOperand(++pos,str,operand));
   }
   else if(pos < len-1 && str[pos] == '<' && str[pos+1] == '<') // Must be a quoteless list  
   {
      pos += 2;
      return(ExtractQuotelessStringListOperand(pos,str,operand));
   }
   else if(str[pos] == '{') // Must be a complex matrix 
   {
      return(ExtractComplexMatrixOperand(++pos,str,operand));

   }
   else
      type = FLOAT32; // Otherwise assume its a number for now
   
// Start scanning from current position in expression string *****
   for(i = pos; i < len; i++)
   {
      c = str[i];

   // Check for to see if this character is consistent with a valid real number
      if(type == FLOAT32 || type == FLOAT64)
      {
        if(type == FLOAT64)
         {
            if(IsWhiteSpace(c)) // White space, so end of operand
            {
               i++;
               break;
            }
            else if(IsOperandDelimiter(str,i,len)) // Operator, so end of operand
            {
               if(j == 0)
                  type = OPERATOR_TOKEN;
               break;
            }
            else
            {
               type = UNQUOTED_STRING; // Not a number
               operand[j++] = c; 
               continue;
            }
         }

         if(c == '-' || c == '+') // Sign
         {
            if(i == 0)
               mode = START; // Its a unitary operator
            else if(mode == ESYMBOL)
               mode = ESIGN; // Its an exponent sign
            else
               break; // This symbol is an operator
         }
         else if((i-pos == 1) && (c == 'x' || c == 'X') && str[pos] == '0') // Start of hex number
         {
            type = FLOAT32;
            mode = HEX;
         }
         else if(mode == HEX && ((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) // Hex number
         {
            type = FLOAT32;
            mode = HEX;
         }
         else if(isdigit(c)) // Digit  ddd.dddEddd
         {
            if(mode == START)
               mode = NUMBER;
            else if(mode == NUMBER)
               mode = NUMBER;
            else if(mode == HEX)
               mode = HEX;
            else if(mode == EXPONENT)
               mode = EXPONENT;
            else if(mode == FRACTION)
               mode = FRACTION;
            else if(mode == POINT)
               mode = FRACTION;
            else if(mode == ESIGN || mode == ESYMBOL)
               mode = EXPONENT;
            else
               type = UNQUOTED_STRING;
         }
         else if(c == '.' && mode == POINT) // Point
         { 
            type = UNQUOTED_STRING; 
         }
         else if(c == '.' && str[i+1] != '*') // Point
         { 
            if(mode == NUMBER || mode == START)
               mode = POINT;
            else
               type = UNQUOTED_STRING; // Wrong place!
         }
         else if(c == 'e' || c == 'E') // Exponent
         {
            if(mode == NUMBER || mode == FRACTION)
               mode = ESYMBOL;
            else
               type = UNQUOTED_STRING; // Wrong place
         } 
         else if(mode != START && c == 'd') // End of double
         {
            type = FLOAT64;
         }
         else if(c == 'i' || c == 'j') // Complex number
         {
            if(i == pos) // i[number]
            {
               operand[j++] = c;
               type = COMPLEX;
               mode = START;
               continue; // make sure number is valid
            }
            else if(IsOperandDelimiter(str,i+1,len)    || // i[operator] or i[EOL]
                     (str[i+1] == ' '))                   // i[space]
            {
               operand[j++] = c;
               type = COMPLEX;
               i++;
               break;
            }
            else
            {
               ErrorMessage("invalid complex number");
               return(ERR);
            }
         }  
         else if(IsWhiteSpace(c)) // White space, so end of operand
         {
            i++;
            break;
         }
         else if(IsOperandDelimiter(str,i,len)) // Operator, so end of operand
         {
            if(j == 0)
               type = OPERATOR_TOKEN;
            break;
         }
         else if(mode == START)
            type = UNQUOTED_STRING;
         else
         {
            type = UNQUOTED_STRING;
         }
      } // END FLOAT32/64             

   // Check for to see if this character is consistent with a valid complex number
      else if(type == COMPLEX)
      {
         //if(i-pos == 1 && (c == 'x' || c == 'X'))
         //{
         //   type = UNQUOTED_STRING;
         //   break;
         //}
      //   else
            if(isdigit(c))
         {
            if(mode == START)
               mode = NUMBER;
            else if(mode == NUMBER)
               mode = NUMBER;
            else if(mode == EXPONENT)
               mode = EXPONENT;
            else if(mode == FRACTION)
               mode = FRACTION;
            else if(mode == POINT)
               mode = FRACTION;
            else if(mode == ESIGN || mode == ESYMBOL)
               mode = EXPONENT;
            else
               type = UNQUOTED_STRING;
         }
         else if(c == '.' && str[i+1] != '*')
         { 
            if(mode == NUMBER || mode == START)
               mode = POINT;
            else
            {
               type = UNQUOTED_STRING;
            }
         }
         else if(c == 'e' || c == 'E')
         {
            if(mode == NUMBER || mode == FRACTION)
               mode = ESYMBOL;
            else
            {
               type = UNQUOTED_STRING;
            }
         } 
         else if(c == 'i' || c == 'j') // Shouldn't appear again in complex nr.
         {
            type = UNQUOTED_STRING;
         }  
         else if(c == ' ' || c == '\t') // End of complex nr definition
         {
            i++;
            break;
         }
         else if(IsOperandDelimiter(str,i,len)) // End of complex nr definition
         {
            if(j == 0)
               type = OPERATOR_TOKEN;
            break;
         }
         else
         {
            type = UNQUOTED_STRING;
         }
      } // END COMPLEX

   // Check for end of variable or string *********
      if(type == UNQUOTED_STRING)
      {
         if(c == ' ' || c == '\t') // A space signs the end (variables can't include spaces)
         {
            i++;
            break;
         } 
         else if(IsOperandDelimiter(str,i,len) && c != '"') // An operator also signals the end
         {                                                          
            break;
         }
         else if(c == '-' && str[i+1] == '>') // Dot operator so must be a structure member reference
         {
            if(j > 0 && j < len-1)
               subtype  = MEMBER;
         }

      } // END UNQUOTED_STRING

                 
   // Load up operand string *********************
      operand[j++] = c;      
   }

// Finish off *******************************************************   
   operand[j] = '\0';
 
   if(mode == POINT)
      type = UNQUOTED_STRING;

   if(mode == ESYMBOL || mode == ESIGN)
      type = UNQUOTED_STRING;

   if(type == UNQUOTED_STRING && subtype == MEMBER)
      type = STRUCTMEMBER;

// Float 64s have a d at the end - need to remove it
   if(type == FLOAT64)
   {
      if(operand[j-1] == 'd')
         operand[j-1] = '\0';
      else
         type = UNQUOTED_STRING;
   }

   pos = i;
   return(type);
}  

/*************************************************************************************
     Extract a quoted string from 'str' by searching for the next non-escaped quote.
     Starts searching from position 'pos' and returns result in operand.
**************************************************************************************/

short ExtractStringOperand(short &pos, char *str, char *operand)
{
   long len,i,j;
   char c;

// Start scanning from current position in expression string *****
   len = strlen(str);
   for(j = 0, i = pos; i < len; i++)
   {
      c = str[i];

   // Check for end of string ********************
      if(c == QUOTE && !IsEscapedQuote(str,i,len)) // Ignore escape quotes
      {
         i++;
         break;
      } 
   // Load up operand string *********************
      operand[j++] = c;
      continue;
   } 

   operand[j] = '\0';
   pos = i;
   return(QUOTED_STRING);
}


short ExtractQuotelessStringListOperand(short &pos, char str[], char operand[])
{
   bool inQuote = false;
   long len,i,j;
   char c;
   long bracketCnt = 1; // We have found a bracket already - thats why we're here

// Start scanning from current position in expression string *****
   len = strlen(str);
   for(j = 0, i = pos; i < len; i++)
   {
      c = str[i];

   // Ignore characters in embedded string ********************
      if(!inQuote && c == QUOTE && !IsEscapedQuote(str,i,len)) 
      {
         inQuote = true;
         goto update;
      }
      if(inQuote && c == QUOTE && !IsEscapedQuote(str,i,len))
      {
         inQuote = false;
         goto update;
      }
      if(!inQuote && i < len-1 && c == '<' && str[i+1] == '<')
      {
         bracketCnt++;
      }
      if(!inQuote && i < len-1 && c == '>' && str[i+1] == '>')
      {
         bracketCnt--;
         if(bracketCnt == 0)
         {
            i++;
            break;
         }
      }
update:
   // Load up operand string *********************
      operand[j++] = c;
   } 

   operand[j-1] = '\0';
   pos = i;
   return(QUOTELESS_LIST);
}


short ExtractQuotelessStringListOperandCT(short &pos, char str[], CText &operand)
{
   bool inQuote = false;
   long len,i,j;
   char c;
   long bracketCnt = 1; // We have found a bracket already - thats why we're here

// Start scanning from current position in expression string *****
   len = strlen(str);
   for(j = 0, i = pos; i < len; i++)
   {
      c = str[i];

   // Ignore characters in embedded string ********************
      if(!inQuote && c == QUOTE && !IsEscapedQuote(str,i,len)) 
      {
         inQuote = true;
         goto update;
      }
      if(inQuote && c == QUOTE && !IsEscapedQuote(str,i,len))
      {
         inQuote = false;
         goto update;
      }
      if(!inQuote && i < len-1 && c == '<' && str[i+1] == '<')
      {
         bracketCnt++;
      }
      if(!inQuote && i < len-1 && c == '>' && str[i+1] == '>')
      {
         bracketCnt--;
         if(bracketCnt == 0)
         {
            i++;
            break;
         }
      }
update:
   // Load up operand string *********************
      operand.Append(c);
   } 

   pos = i;
   return(QUOTELESS_LIST);
}

/*************************************************************************************
     Extract a matrix from 'str' by searching for the next nonquoted matching ']'.
**************************************************************************************/

short ExtractRealMatrixOperand(short &pos, char str[], char operand[])
{
   bool inQuote = false;
   long len,i,j;
   char c;
   long bracketCnt = 1; // We have found a bracket already - thats why we're here

// Start scanning from current position in expression string *****
   len = strlen(str);
   for(j = 0, i = pos; i < len; i++)
   {
      c = str[i];

   // Ignore characters in embedded string ********************
      if(!inQuote && c == QUOTE && !IsEscapedQuote(str,i,len)) 
      {
         inQuote = true;
         goto update;
      }
      if(inQuote && c == QUOTE && !IsEscapedQuote(str,i,len))
      {
         inQuote = false;
         goto update;
      }
      if(!inQuote && c == '[')
      {
         bracketCnt++;
      }
      else if(!inQuote && c == ']')
      {
         bracketCnt--;
         if(bracketCnt == 0)
         {
            i++;
            break;
         }
      }
update:
   // Load up operand string *********************
      operand[j++] = c;
   } 

   operand[j] = '\0';
   pos = i;
   return(MATRIX2D);
}


/*************************************************************************************
  Extract a complex matrix from 'str' by searching for the next nonquoted matching '}'.
**************************************************************************************/

short ExtractComplexMatrixOperand(short &pos, char str[], char operand[])
{
   bool inQuote = false;
   long len,i,j;
   char c;
   long bracketCnt = 1; // We have found a bracket already - thats why we're here

// Start scanning from current position in expression string *****
   len = strlen(str);
   for(j = 0, i = pos; i < len; i++)
   {
      c = str[i];

   // Ignore characters in embedded string ********************
      if(!inQuote && c == QUOTE && !IsEscapedQuote(str,i,len)) 
      {
         inQuote = true;
         goto update;
      }
      if(inQuote && c == QUOTE && !IsEscapedQuote(str,i,len))
      {
         inQuote = false;
         goto update;
      }
      if(!inQuote && c == '{')
      {
         bracketCnt++;
      }
      else if(!inQuote && c == '}')
      {
         bracketCnt--;
         if(bracketCnt == 0)
         {
            i++;
            break;
         }
      }
update:
   // Load up operand string *********************
      operand[j++] = c;
   } 

   operand[j] = '\0';
   pos = i;
   return(CMATRIX2D);
}


/*************************************************************************************
     Extract a quoted string from 'str' by searching for the next non-escaped quote.
     Starts searching from position 'pos' and returns result in operand.
**************************************************************************************/

short ExtractStringOperandCT(short &pos, char str[], CText &operand)
{
   long len,i,j;
   char c;

// Start scanning from current position in expression string *****
   len = strlen(str);
   for(j = 0, i = pos; i < len; i++)
   {
      c = str[i];

   // Check for end of string ********************
      if(c == QUOTE && !IsEscapedQuote(str,i,len)) // Ignore escape quotes
      {
         i++;
         break;
      } 
   // Load up operand string *********************
      operand.Append(c);
      continue;
   } 

   pos = i;
   return(QUOTED_STRING);
}

/*************************************************************************************
     Extract a matrix from 'str' by searching for the next nonquoted matching ']'.
**************************************************************************************/

short ExtractRealMatrixOperandCT(short &pos, char str[], CText &operand)
{
   bool inQuote = false;
   long len,i,j;
   char c;
   long bracketCnt = 1; // We have found a bracket already - thats why we're here

// Start scanning from current position in expression string *****
   len = strlen(str);
   for(j = 0, i = pos; i < len; i++)
   {
      c = str[i];

   // Ignore characters in embedded string ********************
      if(!inQuote && c == QUOTE && !IsEscapedQuote(str,i,len)) 
      {
         inQuote = true;
         goto update;
      }
      if(inQuote && c == QUOTE && !IsEscapedQuote(str,i,len))
      {
         inQuote = false;
         goto update;
      }
      if(!inQuote && c == '[')
      {
         bracketCnt++;
      }
      else if(!inQuote && c == ']')
      {
         bracketCnt--;
         if(bracketCnt == 0)
         {
            i++;
            break;
         }
      }
update:
   // Load up operand string *********************
      operand.Append(c);
   } 

   pos = i;
   return(MATRIX2D);

}

/*************************************************************************************
  Extract a complex matrix from 'str' by searching for the next nonquoted matching '}'.
**************************************************************************************/

short ExtractComplexMatrixOperandCT(short &pos, char str[], CText &operand)
{
   bool inQuote = false;
   long len,i,j;
   char c;
   long bracketCnt = 1; // We have found a bracket already - thats why we're here


// Start scanning from current position in expression string *****
   len = strlen(str);
   for(j = 0, i = pos; i < len; i++)
   {
      c = str[i];

   // Ignore characters in embedded string ********************
      if(!inQuote && c == QUOTE && !IsEscapedQuote(str,i,len)) 
      {
         inQuote = true;
         goto update;
      }
      if(inQuote && c == QUOTE && !IsEscapedQuote(str,i,len))
      {
         inQuote = false;
         goto update;
      }
      if(!inQuote && c == '{')
      {
         bracketCnt++;
      }
      else if(!inQuote && c == '}')
      {
         bracketCnt--;
         if(bracketCnt == 0)
         {
            i++;
            break;
         }
      }
update:
   // Load up operand string *********************
      operand.Append(c);
   } 

   pos = i;
   return(CMATRIX2D);
}

/*********************************************************************
* Expects a single number or variable and evaluates them. If a number *
* it returns it in result otherwise it returns the variable and type  *
* This is the fastest I can evaluate an argument                      *
**********************************************************************/


//short SimpleArgument(char *arg, short *type, float *result, Variable **var)
//{
//   short len = strlen(arg);
//
//// Check for a variable name
//
//   if((arg[0] >= '0' && arg[0] <= '9') || arg[0] == '+' || arg[0] == '-')
//   {
//      if((*result = StringToFloat(arg)) == NaN)
//         return(-2); // Invalid number
//      *type = FLOAT32;       
//   }
//   else
//   {
//      *var = GetVariable(ALL_VAR,arg,*type);
//      if(*var == NULL)
//         return(-3); // No such variable
//      if(*type == FLOAT32)
//         *result = VarReal((*var));
//   }
//   return(OK); // Conversion complete
//}   

/*********************************************************************
            Converts a string to a floating point number               
  Can handle Hex numbers e.g. 0x2E, Sci numbers e.g. 2.3e-4 and       
  normal numbers like 2.345 or 120.                                    
                                                                      
  Returns converted number or NaN if there is a syntax error. Test with
  isnan().
*********************************************************************/
  
EXPORT float StringToFloat(char *s)
{
   short len = strlen(s);
   short sign = 1;
   double p10 = 1;
   long exponent = 0;
   double mantissa = 0;
   double p16;
   long decimal = -2;
   long m;
   bool fac = false;
   float result;
   long start = 0;
   
// Extract sign *****************************
   if(s[0] == '-' || s[0] == '+')
   {
      start = 1;
      sign = -(s[0] == '-');
   }

// Check to see if its a hexadecimal number *
   if(s[0] == '0' && len > 1 && (s[1] == 'x' || s[1] == 'X'))
   {
      result = 0;
      p16 = 1;
      
      for(short i = len-1; i >= 2; i--)
      { 
         if(s[i] >= '0' && s[i] <= '9')
         {
            result += p16*(s[i] - '0');
         }
         else if(s[i] >= 'A' && s[i] <= 'F')
         {
            result += p16*(s[i] - 'A' + 10);
         }
         else if(s[i] >= 'a' && s[i] <= 'f')
         {
            result += p16*(s[i] - 'a' + 10);
         }           
         p16 *= 16;
      }
      return(result);
   }
   
// Extract mantissa *************************      
   for(short i = start; i < len; i++)
   {
      if(s[i] == '.') // Note position of decimal point
      {
         if(decimal > 0)
            return(NaN);
         decimal = i-1-start;
         continue;
      }
            
      if(s[i] == 'e' || s[i] == 'E') // Extract exponent
      {
         if(decimal == -2)
            decimal = i-1-start;
         i++;
         if(s[i] == '-')
         {
            fac = true;
            i++;
         }
         if(s[i] == '+')
         {
            fac = false;
            i++;
         }
         p10 = 1;
         for(short k = len-1;k >= i; k--)
         {      
            if(s[k] < '0' || s[k] > '9')
               return(NaN);
               
            exponent += p10*(s[k] - '0');
            p10 *= 10;
         }  
         break;                   
      }

      if(s[i] < '0' || s[i] > '9')
         return(NaN);

      mantissa += p10*(s[i] - '0');
      p10 /= 10;
   }
   if(decimal == -2)
      decimal = len-1-start;
      
// Figure out the power of 10 ***************************
   m = decimal + exponent*((fac == false) - (fac == true));

// and apply to mantissa
   if(m > 0)
   {
      mantissa *= pow(10.0,(int)m);
      //for(short i = 1; i <= m; i++)
      //   mantissa *= 10;
   }
   else
   {
      m = -m;
      mantissa *= pow(0.1,(int)m);
      //for(short i = 1; i <= m; i++)
      //   mantissa /= 10;
   }   

// Apply sign   
   result = mantissa*sign;
      
   return(result);
}

/*********************************************************************
            Converts a string to a long floating point number               
  Can handle Hex numbers e.g. 0x2E, Sci numbers e.g. 2.3e-4 and       
  normal numbers like 2.345 or 120.                                    
                                                                      
  Returns converted number or NaN if there is a syntax error. Test with
  isnan().
*********************************************************************/

double StringToDouble(char *s)
{
   short len = strlen(s);
   short sign = 1;
   double p10 = 1;
   long exponent = 0;
   double mantissa = 0;
   double p16;
   long decimal = -2;
   long m;
   bool fac = false;
   double result;
   long start = 0;
   
// Extract sign *****************************
   if(s[0] == '-' || s[0] == '+')
   {
      start = 1;
      sign = -(s[0] == '-');
   }

// Check to see if its a hexadecimal number *
   if(s[0] == '0' && len > 1 && (s[1] == 'x' || s[1] == 'X'))
   {
      result = 0;
      p16 = 1;
      
      for(short i = len-1; i >= 2; i--)
      { 
         if(s[i] >= '0' && s[i] <= '9')
         {
            result += p16*(s[i] - '0');
         }
         else if(s[i] >= 'A' && s[i] <= 'F')
         {
            result += p16*(s[i] - 'A' + 10);
         }
         else if(s[i] >= 'a' && s[i] <= 'f')
         {
            result += p16*(s[i] - 'a' + 10);
         }           
         p16 *= 16;
      }
      return(result);
   }
   
// Extract mantissa *************************      
   for(short i = start; i < len; i++)
   {
      if(s[i] == '.') // Note position of decimal point
      {
         if(decimal > 0)
            return(NaN);
         decimal = i-1-start;
         continue;
      }
            
      if(s[i] == 'e' || s[i] == 'E') // Extract exponent
      {
         if(decimal == -2)
            decimal = i-1-start;
         i++;
         if(s[i] == '-')
         {
            fac = true;
            i++;
         }
         if(s[i] == '+')
         {
            fac = false;
            i++;
         }
         p10 = 1;
         for(short k = len-1;k >= i; k--)
         {      
            if(s[k] < '0' || s[k] > '9')
               return(NaN);
               
            exponent += p10*(s[k] - '0');
            p10 *= 10;
         }  
         break;                   
      }

      if(s[i] < '0' || s[i] > '9')
         return(NaN);

      mantissa += p10*(s[i] - '0');
      p10 /= 10;
   }
   if(decimal == -2)
      decimal = len-1-start;
      
// Figure out the power of 10 ***************************
   m = decimal + exponent*((fac == false) - (fac == true));

// and apply to mantissa
   if(m > 0)
   {
      mantissa *= pow(10.0,(int)m);
      //for(short i = 1; i <= m; i++)
      //   mantissa *= 10;
   }
   else
   {
      m = -m;
      mantissa *= pow(0.1,(int)m);
      //for(short i = 1; i <= m; i++)
      //   mantissa /= 10;
   }   

// Apply sign   
   result = mantissa*sign;
      
   return(result);
}

/*********************************************************************
            Converts a string to a double precision  number     
            but for hex numbers also works out data type based
            on the number of digits. Type for floats is returned
            as single precision.

  Can handle Hex numbers e.g. 0x2E, Sci numbers e.g. 2.3e-4 and       
  normal numbers like 2.345 or 120.                                    
                                                                      
  Returns converted number or NaN if there is a syntax error. Test with
  isnan().
*********************************************************************/
  
double StringToNumber(char *s, short &type)
{
   short len = strlen(s);
   short sign = 1;
   double p10 = 1;
   long exponent = 0;
   double mantissa = 0;
   double p16;
   long decimal = -2;
   long m;
   bool fac = false;
   double result;
   long start = 0;
   short mantissaDigits = 0;
   short exponentDigits = 0;
   
// Extract sign *****************************
   if(s[0] == '-' || s[0] == '+')
   {
      start = 1;
      sign = -(s[0] == '-');
   }

// Check to see if its a hexadecimal number *
   if(s[0] == '0' && len > 1 && (s[1] == 'x' || s[1] == 'X'))
   {
      result = 0;
      p16 = 1;
      
      for(short i = len-1; i >= 2; i--)
      { 
         if(s[i] >= '0' && s[i] <= '9')
         {
            result += p16*(s[i] - '0');
         }
         else if(s[i] >= 'A' && s[i] <= 'F')
         {
            result += p16*(s[i] - 'A' + 10);
         }
         else if(s[i] >= 'a' && s[i] <= 'f')
         {
            result += p16*(s[i] - 'a' + 10);
         }           
         p16 *= 16;
         mantissaDigits++;
      }
      if(mantissaDigits > 6 || result > INT_MAX)
         type = FLOAT64;
      else
         type = FLOAT32;
      return(result);
   }
   
// Extract mantissa *************************      
   for(short i = start; i < len; i++)
   {
      if(s[i] == '.') // Note position of decimal point
      {
         if(decimal > 0)
            return(NaN);
         decimal = i-1-start;
         continue;
      }
            
      if(s[i] == 'e' || s[i] == 'E') // Extract exponent
      {
         if(decimal == -2)
            decimal = i-1-start;
         i++;
         if(s[i] == '-')
         {
            fac = true;
            i++;
         }
         if(s[i] == '+')
         {
            fac = false;
            i++;
         }
         p10 = 1;
         for(short k = len-1;k >= i; k--)
         {      
            if(s[k] < '0' || s[k] > '9')
               return(NaN);
               
            exponent += p10*(s[k] - '0');
            exponentDigits++;
            p10 *= 10;
         }  
         break;                   
      }

      if(s[i] < '0' || s[i] > '9')
         return(NaN);

      mantissa += p10*(s[i] - '0');
      mantissaDigits++;
      p10 /= 10;
   }
   if(decimal == -2)
      decimal = len-1-start;
      
// Figure out the power of 10 ***************************
   m = decimal + exponent*((fac == false) - (fac == true));

// and apply to mantissa
   if(m > 0)
   {
      mantissa *= pow(10.0,(int)m);
   }
   else
   {
      m = -m;
      mantissa *= pow(0.1,(int)m);
   }   

   type = FLOAT32;

// Apply sign   
   result = mantissa*sign;
      
   return(result);
}

/*****************************************************************************
*      Converts special single character operators into readable string
******************************************************************************/
   
char* GetOpString(unsigned char op)
{
   static char opstr[3];

   switch(op)
   {
      case('%'):
         strcpy(opstr,"%%"); break;
      case(EQ):
         strcpy(opstr,"=="); break;
      case(NEQ):
         strcpy(opstr,"!="); break;
      case(LEQ):
         strcpy(opstr,"<="); break;
      case(GEQ):
         strcpy(opstr,">="); break;
      case(MATMUL):
         strcpy(opstr,".*"); break;
      default:
         opstr[0] = (char)op;
         opstr[1] = '\0';
   }  
   
   return(opstr);             
}

/*****************************************************************************
*                      Make a 1D, 2D, 3D or 4D real matrix
******************************************************************************/

int NewMatrix(Interface* itfc ,char args[])
{ 
   short r;
   long xsize = 1, ysize = 1 ,zsize = 1, qsize = 1;
   
   if((r = ArgScan(itfc,args,1,"result:xsize,[ysize,[zsize,[qsize]]","eeee","llll",&xsize,&ysize,&zsize,&qsize)) < 0)
      return(r); 

   if(xsize <= 0 || ysize <= 0 || zsize <= 0 || qsize <= 0)
   {
      ErrorMessage("invalid matrix size");
      return(ERR);
   }
   
// Allocate and initialise 
   if(r < 3) // 1D or 2D become 2D
      itfc->retVar[1].MakeAndLoadMatrix2D(NULL,xsize,ysize);
   else if(r == 3) // 3D
      itfc->retVar[1].MakeAndLoadMatrix3D(NULL,xsize,ysize,zsize);
   else // 4D
      itfc->retVar[1].MakeAndLoadMatrix4D(NULL,xsize,ysize,zsize,qsize);
  
// Check for memory allocation failure
   if(itfc->retVar[1].GetType() == NULL_VARIABLE)
      return(ERR);

   itfc->nrRetValues = 1;

   return(OK);
}


/*****************************************************************************
*                      Make a 1D, 2D, 3D or 4D double matrix
******************************************************************************/

int NewDMatrix(Interface* itfc ,char args[])
{ 
   short r;
   long xsize = 1, ysize = 1 ,zsize = 1, qsize = 1;
   
   if((r = ArgScan(itfc,args,1,"result:xsize,[ysize,[zsize,[qsize]]","eeee","llll",&xsize,&ysize,&zsize,&qsize)) < 0)
      return(r); 

   if(xsize <= 0 || ysize <= 0 || zsize <= 0 || qsize <= 0)
   {
      ErrorMessage("invalid matrix size");
      return(ERR);
   }
   
// Allocate and initialise 
   if(r < 3) // 1D or 2D become 2D
      itfc->retVar[1].MakeAndLoadDMatrix2D(NULL,xsize,ysize);
//   else if(r == 3) // 3D
//      itfc->retVar[1].MakeAndLoadMatrix3D(NULL,xsize,ysize,zsize);
//   else // 4D
//     itfc->retVar[1].MakeAndLoadMatrix4D(NULL,xsize,ysize,zsize,qsize);
  
// Check for memory allocation failure
   if(itfc->retVar[1].GetType() == NULL_VARIABLE)
      return(ERR);

   itfc->nrRetValues = 1;

   return(OK);
}


/*****************************************************************************
*                      Make a 1D 2D or 3D complex matrix
******************************************************************************/

int NewComplexMatrix(Interface* itfc ,char arg[])
{ 
   short r;
   long xsize = 1, ysize = 1 ,zsize = 1, qsize = 1;
   
   if((r = ArgScan(itfc,arg,1,"xsize, [ysize, [zsize, [qsize]]]","eeee","llll",&xsize,&ysize,&zsize,&qsize)) < 0)
      return(r); 

   if(xsize <= 0 || ysize <= 0 || zsize <= 0 || qsize <= 0)
   {
      ErrorMessage("invalid matrix size");
      return(ERR);
   }
   
 // Allocate and initialise 
   if(r < 3) // 1D or 2D becomes 2D
      itfc->retVar[1].MakeAndLoadCMatrix2D(NULL,xsize,ysize);
   else if(r == 3) // 3D
      itfc->retVar[1].MakeAndLoadCMatrix3D(NULL,xsize,ysize,zsize);
   else // 4D
      itfc->retVar[1].MakeAndLoadCMatrix4D(NULL,xsize,ysize,zsize,qsize);
   
 // Check for memory allocation failure
   if(itfc->retVar[1].GetType() == NULL_VARIABLE)
      return(ERR);

   itfc->nrRetValues = 1;
   return(OK);
}

void StrReplace(char *str, short pos, char *replace, short lenR)
{
   short lenS = strlen(str);
   
   for(short i = lenS-1; i > pos; i--)
      str[i+lenR] = str[i];
      
   for(short j = 0; j <= lenR; j++)
      str[pos + j] = replace[j];
      
   str[lenS + lenR] = '\0';
}


int SpeedTest(char args[])
{

   testMode = !testMode;
   int i;
   CText operand;
  // char operand[MAX_STR];
   unsigned char op;
   short pos = 0;
   short len = strlen(args);
   for(i = 0; i < 1e6; i++)
   {
      do
      {
     //    GetNextOperand(pos,args,operand,MAX_STR-1);

         GetNextOperandCT(pos,args,operand);
         GetNextOperator(pos,args,op);
      }
      while(pos < len);
      pos = 0;
   }
   return(0);
}

/*************************************************************************
   Check to see if the text starting at position 'pos' is an operator
*************************************************************************/

bool IsOperandDelimiter(char *txt, long pos, long len) 
{
   if(pos >= len)  // End of string reached!
      return(true);

   if(precedence[txt[pos]] > 0) // Normal delimiter seen
      return(true);

   if(IsWhiteSpace(txt[pos])) // White space
      return(true);

   if(txt[pos] == ',') // Comma
      return(true);

   if(pos+1 < len) // Special delimiter seen
   {
      if(txt[pos] == '.' && txt[pos+1] == '*')
         return(true);
   }

   return(false);
}


/*************************************************************************
   Check to see if the text starting at position 'pos' is an operator
*************************************************************************/

bool IsClassFunctionDelimiter(char *txt, long pos, long len) 
{
   if(pos >= len)  // End of string reached!
      return(true);

   if(precedence[txt[pos]] > 0) // Normal delimiter seen
      return(true);

   if(IsWhiteSpace(txt[pos])) // White space
      return(true);

   if(txt[pos] == ',') // Comma
      return(true);

   if(pos+1 < len) // Special delimiter seen
   {
      if(txt[pos] == '.' && txt[pos+1] == '*')
         return(true);
   }

   return(false);
}


/****************************************************************************************
   Replaces embedded expressions in string 'b' with their value
   Note : this function may modify returnVar!! Make a copy before hand if necessary
*****************************************************************************************/
//TODO need to free retValues
short ReplaceVarInString(Interface *itfc, CText &b)
{
    CText out;
    CText expression;
    CArg carg;
    short nrArgs;
    char format[20];
    Variable result;

	 // See if we are ignoring expressions in strings
	 if(!itfc->processExpressions)
		 return(OK);

	int p1=0,p2=-1;
	int last;

	while(1)
	{
	   last = p2;
	   p1 = b.Search(last+1,'$');
		if(p1 == -1) break;
	   p2 = b.Search(p1+1,'$');
      if(p2 == -1)
      {
	      ErrorMessage("Syntax error - mismatched '$' delimiters\n");
         return(ERR);
      }
	   out.Concat(&b[last+1],(p1-1)-(last+1)+1);
	   if(p1+1 == p2) // Special case of repeated $$ if so replace with single $.
      {
	      out.Append('$');
      }
	   else
	   {
	      expression.Assign(&b[p1+1],(p2-1)-(p1+1)+1);
         if((nrArgs = carg.Count(expression.Str())) == 2)
         {
            expression.Assign(carg.Extract(1));
            strcpy(format,"%");
            strcat(format,carg.Extract(2));
         }
         else
         {
            strcpy(format,"%");
         }

         if(Evaluate(itfc,RESPECT_ALIAS,expression.Str(),&result) == ERR)
            return(ERR);

         if(itfc->nrRetValues == 1)
         {
            ConvertVariableToText(&result, expression, format, nrArgs>1);
		      out.Concat(expression.Str());
         }
         else
         {
            for(int i = 1; i <= itfc->nrRetValues; i++)
            {
               ConvertVariableToText(&(itfc->retVar[i]), expression, format, nrArgs>1);
		         out.Concat(expression.Str());
               if(i < itfc->nrRetValues)
		           out.Concat(",");
            }
            itfc->nrRetValues = 1;
         }
	   }
   }

// Add last part of string
	out.Concat(&b[p2+1],(b.Size()-1)-(p2+1)+1);
	
// Copy output into passed string 
   b.Assign(out.Str());

	return(OK);
}


/****************************************************************************************
   Convert variable var into its text equivalent form. 
   format is the format to use to print out individual member of var (if useFormat = true)
*****************************************************************************************/
   

void ConvertVariableToText(Variable *var, CText &str, char *format, bool useFormat)
{
   str.Assign("");

   switch(var->GetType())
   {
// Null variable
	   case(NULL_VARIABLE): 
      {
         str.Assign("null");    
		   break;
	   }  
// List ******************************
	   case(LIST): 
      {
         ConvertListVariableToText(var, str, format, useFormat);		  
		   break;
	   } 
	   case(LIST2D): 
      {
         Convert2DListVariableToText(var, str, format, useFormat);		  
         break;
      }
// Float ******************************
      case(FLOAT32):  
      case(FLOAT64):  
	   case(COMPLEX):
      {
         ConvertScalarVariableToText(var, str, format, useFormat);	
         break;
      }

// String ******************************
      case(QUOTED_STRING):
      case(UNQUOTED_STRING): 
      {
         if (useFormat)
            str.Format(format, var->GetString());
         else
            str.Assign(var->GetString());    
         break;
      }  
// 1D, 2D or 3D real matrix ******************************
      case(MATRIX2D): 
      case(MATRIX3D):    
      {
         ConvertRealMatrixVariableToText(var, str, format, useFormat);	
		   break;
	   } 
// 1D, 2D or 3D complex matrix ******************************   	
      case(CMATRIX2D):     
      case(CMATRIX3D):     
      {
         ConvertComplexMatrixVariableToText(var, str, format, useFormat);	   		
		   break;
	   }  		      			                            
   } 
}

void ConvertComplexMatrixVariableToText(Variable *var, CText &str, char *format, bool useFormat)		
{
	char temp[MAX_STR];
   char formatc[20];

   switch(var->GetType())
   {   
      case(CMATRIX2D):     
      {
		   long xsize = var->GetDimX();
		   long ysize = var->GetDimY();
		   complex **cmat = var->GetCMatrix2D();  
   		
         str.Assign("{");
			for(long y = 0; y < ysize; y++)
			{
				for(long x = 0; x < xsize; x++)
				{
               if(!useFormat)
               {
				      if(x < xsize-1)
				      {
					      if(cmat[y][x].i >= 0)
					         sprintf(temp,"%g+%gi,",cmat[y][x].r,fabs(cmat[y][x].i));
					      else
					         sprintf(temp,"%g-%gi,",cmat[y][x].r,fabs(cmat[y][x].i));
				      }    
				      else
				      {
					      if(cmat[y][x].i >= 0)
					         sprintf(temp,"%g+%gi",cmat[y][x].r,fabs(cmat[y][x].i));
					      else
					         sprintf(temp,"%g-%gi",cmat[y][x].r,fabs(cmat[y][x].i));
				      }  
               }
               else
               {
				      if(x < xsize-1)
				      {
					      if(cmat[y][x].i >= 0)
                     {
                        sprintf(formatc,"%s+%si,",format,format);
					         sprintf(temp,formatc,cmat[y][x].r,fabs(cmat[y][x].i));
                     }
					      else
                     {
                        sprintf(formatc,"%s-%si,",format,format);
					         sprintf(temp,formatc,cmat[y][x].r,fabs(cmat[y][x].i));
                     }
				      }    
				      else
				      {
					      if(cmat[y][x].i >= 0)
                     {
                        sprintf(formatc,"%s+%si",format,format);
					         sprintf(temp,formatc,cmat[y][x].r,fabs(cmat[y][x].i));
                     }
					      else
                     {
                        sprintf(formatc,"%s-%si",format,format);
					         sprintf(temp,formatc,cmat[y][x].r,fabs(cmat[y][x].i));
                     }
				      }  
               }
				   str.Concat(temp);
  				   
				}
				if(y < ysize-1)
				   str.Concat(";"); // End of row delimiter
			}
         str.Concat("}"); // End of matrix
  		
		   break;
	   } 
// 3D complex matrix ******************************
      case(CMATRIX3D):    
      {
		   long xsize = var->GetDimX();
		   long ysize = var->GetDimY();
		   long zsize = var->GetDimZ();
		   complex ***cmat = var->GetCMatrix3D();    
   		
         str.Assign("{");
			for(long z = 0; z < zsize; z++)
			{
				for(long y = 0; y < ysize; y++)
				{
					for(long x = 0; x < xsize; x++)
					{
                  if(!useFormat)
                  {
					      if(x < xsize-1)
					      {
						      if(cmat[z][y][x].i >= 0)
						         sprintf(temp,"%g+%gi,",cmat[z][y][x].r,fabs(cmat[z][y][x].i));
						      else
						         sprintf(temp,"%g-%gi,",cmat[z][y][x].r,fabs(cmat[z][y][x].i));
					      }    
					      else
					      {
						      if(cmat[z][y][x].i >= 0)
						         sprintf(temp,"%g+%gi",cmat[z][y][x].r,fabs(cmat[z][y][x].i));
						      else
						         sprintf(temp,"%g-%gi",cmat[z][y][x].r,fabs(cmat[z][y][x].i));
					      } 
                  }
                  else
                  {
					      if(x < xsize-1)
					      {
						      if(cmat[z][y][x].i >= 0)
                        {
                           sprintf(formatc,"%s+%si,",format,format);
						         sprintf(temp,formatc,cmat[z][y][x].r,fabs(cmat[z][y][x].i));
                        }
						      else
                        {
                           sprintf(formatc,"%s-%si,",format,format);
						         sprintf(temp,formatc,cmat[z][y][x].r,fabs(cmat[z][y][x].i));
                        }
					      }    
					      else
					      {
						      if(cmat[z][y][x].i >= 0)
                        {
                           sprintf(formatc,"%s+%si",format,format);
						         sprintf(temp,"%g+%gi",cmat[z][y][x].r,fabs(cmat[z][y][x].i));
                        }
						      else
                        {
                           sprintf(formatc,"%s-%si",format,format);
						         sprintf(temp,"%g-%gi",cmat[z][y][x].r,fabs(cmat[z][y][x].i));
                        }
					      } 

                  }
					   str.Concat(temp);
					}
					if(y < ysize-1)
				      str.Concat(";"); // End of row delimiter
				}
				if(z < zsize-1)
				   str.Concat(" ;; ");	// End of plane delimitier		         
			}
         str.Concat("}"); // End of matrix
      }
   }
}

   		
void ConvertRealMatrixVariableToText(Variable *var, CText &str, char *format, bool useFormat)		
{
	char temp[MAX_STR];
   char formatr[20];

   switch(var->GetType())
   {
      case(MATRIX2D):    
      {
		   long xsize = var->GetDimX();
		   long ysize = var->GetDimY();
		   float **mat = var->GetMatrix2D();   
         sprintf(formatr,"%s,",format);


		   str.Assign("[");
			for(long y = 0; y < ysize; y++)
			{
				for(long x = 0; x < xsize; x++)
				{
               if(!useFormat)
               {
				      if(x < xsize-1)
				         sprintf(temp,"%g,",mat[y][x]);
				      else
				         sprintf(temp,"%g",mat[y][x]);
               }
               else
               {
				      if(x < xsize-1)
				         sprintf(temp,formatr,mat[y][x]);
				      else
				         sprintf(temp,format,mat[y][x]);
               }
				   str.Concat(temp);
				}
				if(y < ysize-1)
				   str.Concat(";"); // Row delimiter
			}
		   str.Concat("]");  // End of matrix   		
		   break;
	   } 

// 3D real matrix ******************************
      case(MATRIX3D):    
      {
		   long xsize = var->GetDimX();
		   long ysize = var->GetDimY();
		   long zsize = var->GetDimZ();
		   float ***mat = var->GetMatrix3D();  
    		char format2[20];
         sprintf(format2,"%s,",format);

		   str.Assign("[");
			for(long z = 0; z < zsize; z++)
			{
				for(long y = 0; y < ysize; y++)
				{
					for(long x = 0; x < xsize; x++)
					{
                  if(!useFormat)
                  {
					      if(x < xsize-1)
					         sprintf(temp,"%g,",mat[z][y][x]);
					      else
					         sprintf(temp,"%g",mat[z][y][x]);
                  }
                  else
                  {
					      if(x < xsize-1)
					         sprintf(temp,format2,mat[z][y][x]);
					      else
					         sprintf(temp,format,mat[z][y][x]);
                  }
					   str.Concat(temp);
					}
					if(y < ysize-1)
					   str.Concat(";"); // End of row delimiter
				}
				if(z < zsize-1)
					str.Concat(" ;; "); // End of plane delimitier					         
			}
		   str.Concat("]"); // End of matrix
	   } 
   }
}

void ConvertScalarVariableToText(Variable *var, CText &str, char *format, bool useFormat)		
{
   char temp[MAX_STR];

   switch(var->GetType())
   {
      case(FLOAT32):  
      {
         if(GetNumType(var->GetReal()) == FLOAT32)
         {
            if(!useFormat)
               sprintf(temp,"%g",var->GetReal());
            else
               sprintf(temp,format,var->GetReal());                     
         }
         else // Integer
         {
            if(!useFormat)
               sprintf(temp,"%ld",nint(var->GetReal()));
            else if (strchr(format, 'c'))
               sprintf(temp,format,int(var->GetReal()));
				else if (strchr(format, 'd') || strchr(format, 'u') || strchr(format, 'X') ||
                     strchr(format, 'i') || strchr(format, 'x') || strchr(format, 'o'))
					sprintf(temp,format,nint(var->GetReal()));
            else
					sprintf(temp,format,var->GetReal());
         }

		   str.Assign(temp);
         break;
      }

// Double ******************************
      case(FLOAT64):  
      {
         if(GetNumType(var->GetDouble()) == FLOAT32)
         {
            if(!useFormat)
               sprintf(temp,"%1.16fd",var->GetDouble()); // Do I need the 'd' at the end?
            else
               sprintf(temp,format,var->GetDouble());
            
         }
         else // Integer
         {
            if(!useFormat)
               sprintf(temp,"%lldd", (long long)(var->GetDouble()));
            else if (strchr(format, 'e') || strchr(format, 'g') || strchr(format, 'E') || strchr(format, 'G') || strchr(format, 'f') || strchr(format, 'F')) // Want to float format
               sprintf(temp, format, var->GetDouble());
            else
               sprintf(temp,format, (long long)var->GetDouble()); // Another integer format e.g. x
         }

		   str.Assign(temp);
         break;
      }

// Complex ******************************
	   case(COMPLEX):
	   {
		   complex cn = var->GetComplex();
         if(!useFormat)
         {
		      if(cn.i > 0)
			      sprintf(temp,"%g+%gi",cn.r,fabs(cn.i));
		      else
			      sprintf(temp,"%g-%gi",cn.r,fabs(cn.i));   
         }
         else
         {
            char formatc[20];
		      if(cn.i > 0)
            {
               sprintf(formatc,"%s+%si",format,format);
			      sprintf(temp,formatc,cn.r,fabs(cn.i));
            }
		      else
            {
               sprintf(formatc,"%s-%si",format,format);
			      sprintf(temp,formatc,cn.r,fabs(cn.i));  
            }
         }
		   str.Assign(temp);
		   break;
	   }    
   }
}

void Convert2DListVariableToText(Variable *var, CText &str, char *format, bool useFormat)
{
   List2DData* list = (List2DData*)VarList2D(var);
   char *locstr;
   long ysize = var->GetDimY();
	char temp[MAX_STR];
   		
   if(!useFormat)
   {
	   str.Assign("[");

	   for(long y = 0; y < ysize; y++)
      {
		   for(long x = 0; x < list->rowSz[y]; x++)
		   {
			   locstr = list->strings[y][x];

			   if(x < list->rowSz[y]-1)				         
			      sprintf(temp,"\"%s\",",locstr);
			   else
			      sprintf(temp,"\"%s\"",locstr);
			   str.Concat(temp);               
		   }
         if(y < ysize-1)
            str.Concat(";");
      }
	   str.Concat("]");
   }
}

void ConvertListVariableToText(Variable *var, CText &str, char *format, bool useFormat)
{
	char *temp;
   char *lsttxt;
   char formatr[20];
   char formatc[20];
   CText ctemp;

   str.Assign("");

   long xsize = var->GetDimX();
   char **list = var->GetList();   
   		
   if(!useFormat)
   {
	   str.Assign("[");

	   for(long x = 0; x < xsize; x++)
	   {
		   if(x < xsize-1)
         {
            ctemp.Format("\"%s\",",list[x]);
         }
		   else
         {
            ctemp.Format("\"%s\"",list[x]);
         }
         str = str + ctemp;
	   }
	   str.Concat("]");
   }
   else // Print out each list value on separate line
   {    // Format value is the number of spaces to add before second and subsequent lines
	   str.Assign("[");
      int nr;
      if(sscanf(format,"%%%d",&nr) != 1)
         return;
      char s1[50],s2[50];
      sprintf(s1,"\r\n%%%dc\"%%s\"",nr);
      sprintf(s2,"\r\n%%%dc\"%%s\",",nr);

	   for(long x = 0; x < xsize; x++)
	   {
         int sz = strlen(list[x]);
         lsttxt = new char[sz*4+1]; // Allow for worst case replacement

         strcpy(lsttxt,list[x]);

         ReplaceSpecialCharacters(lsttxt,"\\","\\\\",MAX_STR); // Make sure backslashes are escaped
         ReplaceSpecialCharacters(lsttxt,"\"","\\\"",MAX_STR); // Make sure strings variables are quoted

		   if(x == 0)	
         {
            str.Concat("\"");
		      str.Concat(lsttxt);
            str.Concat("\",");
         }
		   else if(x == xsize-1)	
         {
            str.Concat("\r\n          \"");
		      str.Concat(lsttxt);
            str.Concat("\"");
         }
		   else
         {
            str.Concat("\r\n          \"");
		      str.Concat(lsttxt);
            str.Concat("\",");
         }
          
         delete [] lsttxt;

	   }
	   str.Concat("]");
   }	
}

//
//void ConvertVariableToText(Variable *var, CText &str, char *format, bool useFormat)
//{
//	char temp[MAX_STR];
//   char lsttxt[MAX_STR];
//   char formatr[20];
//   char formatc[20];
//   CText ctemp;
//
//   str.Assign("");
//
//   switch(var->GetType())
//   {
//// List ******************************
//	   case(LIST): 
//      {
//		   long xsize = var->GetDimX();
//		   char **list = var->GetList();   
//   		
//         if(!useFormat)
//         {
//	         str.Assign("[");
//
//		      for(long x = 0; x < xsize; x++)
//		      {
//			      if(x < xsize-1)
//               {
//                  ctemp.Format("\"%s\",",list[x]);
//               }
//			      else
//               {
//                  ctemp.Format("\"%s\"",list[x]);
//               }
//               str = str + ctemp;
//		      }
//	         str.Concat("]");
//         }
//         else // Print out each list value on separate line
//         {    // Format value is the number of spaces to add before second and subsequent lines
//	         str.Assign("[");
//            int nr;
//            if(sscanf(format,"%%%d",&nr) != 1)
//               return;
//            char s1[50],s2[50];
//            sprintf(s1,"\r\n%%%dc\"%%s\"",nr);
//            sprintf(s2,"\r\n%%%dc\"%%s\",",nr);
//
//		      for(long x = 0; x < xsize; x++)
//		      {
//               strcpy(lsttxt,list[x]);
//               ReplaceSpecialCharacters(lsttxt,"\\","\\\\",MAX_STR); // Make sure backslashes are escaped
//               ReplaceSpecialCharacters(lsttxt,"\"","\\\"",MAX_STR); // Make sure strings variables are quoted
//
//		         if(x == 0)	
//		            sprintf(temp,"\"%s\",",lsttxt);
//		         else if(x == xsize-1)	
//		            sprintf(temp,s1,' ',lsttxt);
//		         else
//		            sprintf(temp,s2,' ',lsttxt);
//          
//			      str.Concat(temp);
//		      }
//	         str.Concat("]");
//         }	
//		   break;
//	   }   
//	   case(LIST2D): 
//      {
//			List2DData* list = (List2DData*)VarList2D(var);
//         char *locstr;
//		   long ysize = var->GetDimY();
//   		
//         if(!useFormat)
//         {
//	         str.Assign("[");
//
//		      for(long y = 0; y < ysize; y++)
//            {
//		         for(long x = 0; x < list->rowSz[y]; x++)
//		         {
//						locstr = list->strings[y][x];
//
//			         if(x < list->rowSz[y]-1)				         
//			            sprintf(temp,"\"%s\",",locstr);
//			         else
//			            sprintf(temp,"\"%s\"",locstr);
//			         str.Concat(temp);               
//		         }
//               if(y < ysize-1)
//                  str.Concat(";");
//            }
//	         str.Concat("]");
//         }
//         break;
//      }
//// Float ******************************
//      case(FLOAT32):  
//      {
//         if(GetNumType(var->GetReal()) == FLOAT32)
//         {
//            //if(_isnan(VarReal(ansVar)))
//            //{
//            //   strcpy(temp,"NaN");
//            //}
//            //else if(!_finite(VarReal(ansVar)))
//            //{
//            //   strcpy(temp,"Inf");
//            //}
//            //else
//            {
//               if(!useFormat)
//                  sprintf(temp,"%g",var->GetReal());
//               else
//                  sprintf(temp,format,var->GetReal());
//            }            
//         }
//         else // Integer
//         {
//            if(!useFormat)
//               sprintf(temp,"%ld",nint(var->GetReal()));
//            else if (strchr(format, 'c'))
//               sprintf(temp,format,int(var->GetReal()));
//				else if (strchr(format, 'd') || strchr(format, 'u') || strchr(format, 'X') ||
//                     strchr(format, 'i') || strchr(format, 'x') || strchr(format, 'o'))
//					sprintf(temp,format,nint(var->GetReal()));
//            else
//					sprintf(temp,format,var->GetReal());
//         }
//
//		   str.Assign(temp);
//         break;
//      }
//// Double ******************************
//      case(FLOAT64):  
//      {
//         if(GetNumType(var->GetDouble()) == FLOAT32)
//         {
//            if(!useFormat)
//             //  sprintf(temp,"%Lg",var->GetDouble());
//               sprintf(temp,"%1.16fd",var->GetDouble());
//              // 'd' removall -> sprintf(temp,"%1.10f",var->GetDouble());
//            else
//               sprintf(temp,format,var->GetDouble());
//            
//         }
//         else // Integer
//         {
//            if(!useFormat)
//               sprintf(temp,"%ld",nint(var->GetDouble()));
//            else
//               sprintf(temp,format,var->GetDouble());
//         }
//
//		   str.Assign(temp);
//         break;
//      }
//// Complex ******************************
//	   case(COMPLEX):
//	   {
//		   complex cn = var->GetComplex();
//         if(!useFormat)
//         {
//		      if(cn.i > 0)
//			      sprintf(temp,"%g+%gi",cn.r,fabs(cn.i));
//		      else
//			      sprintf(temp,"%g-%gi",cn.r,fabs(cn.i));   
//         }
//         else
//         {
//            char formatc[20];
//		      if(cn.i > 0)
//            {
//               sprintf(formatc,"%s+%si",format,format);
//			      sprintf(temp,formatc,cn.r,fabs(cn.i));
//            }
//		      else
//            {
//               sprintf(formatc,"%s-%si",format,format);
//			      sprintf(temp,formatc,cn.r,fabs(cn.i));  
//            }
//         }
//
//		   str.Assign(temp);
//		   break;
//	   }      
//// String ******************************
//      case(QUOTED_STRING):
//      case(UNQUOTED_STRING): 
//      {
//         str.Assign(var->GetString());    
//         break;
//      }  
//// 1D or 2D real matrix ******************************
//      case(MATRIX2D):     
//      {
//		   long xsize = var->GetDimX();
//		   long ysize = var->GetDimY();
//		   float **mat = var->GetMatrix2D();   
//         sprintf(formatr,"%s,",format);
//
//		//   if(xsize*ysize <= 200)
//		//   {
//		      str.Assign("[");
//			   for(long y = 0; y < ysize; y++)
//			   {
//				   for(long x = 0; x < xsize; x++)
//				   {
//                  if(!useFormat)
//                  {
//				         if(x < xsize-1)
//				            sprintf(temp,"%g,",mat[y][x]);
//				         else
//				            sprintf(temp,"%g",mat[y][x]);
//                  }
//                  else
//                  {
//				         if(x < xsize-1)
//				            sprintf(temp,formatr,mat[y][x]);
//				         else
//				            sprintf(temp,format,mat[y][x]);
//                  }
//				      str.Concat(temp);
//				   }
//				   if(y < ysize-1)
//				      str.Concat(";"); // Row delimiter
//			   }
//		      str.Concat("]");  // End of matrix
//		//   }
//		//   else
//		  //    str.Assign("large matrix");
//   		
//		   break;
//	   } 
//// 3D real matrix ******************************
//      case(MATRIX3D):    
//      {
//		   long xsize = var->GetDimX();
//		   long ysize = var->GetDimY();
//		   long zsize = var->GetDimZ();
//		   float ***mat = var->GetMatrix3D();  
//    		char format2[20];
//         sprintf(format2,"%s,",format);
//
//		//   if(xsize*ysize*zsize <= 200)
//		//   {
//		      str.Assign("[");
//			   for(long z = 0; z < zsize; z++)
//			   {
//				   for(long y = 0; y < ysize; y++)
//				   {
//					   for(long x = 0; x < xsize; x++)
//					   {
//                     if(!useFormat)
//                     {
//					         if(x < xsize-1)
//					            sprintf(temp,"%g,",mat[z][y][x]);
//					         else
//					            sprintf(temp,"%g",mat[z][y][x]);
//                     }
//                     else
//                     {
//					         if(x < xsize-1)
//					            sprintf(temp,format2,mat[z][y][x]);
//					         else
//					            sprintf(temp,format,mat[z][y][x]);
//                     }
//					      str.Concat(temp);
//					   }
//					   if(y < ysize-1)
//					      str.Concat(";"); // End of row delimiter
//				   }
//				   if(z < zsize-1)
//					   str.Concat(" ;; "); // End of plane delimitier					         
//			   }
//		      str.Concat("]"); // End of matrix
//		 //  }
//		 //  else
//		 //     str.Assign("large 3D matrix");
//   		
//		   break;
//	   } 
//// 1D or 2D complex matrix ******************************   	
//      case(CMATRIX2D):     
//      {
//		   long xsize = var->GetDimX();
//		   long ysize = var->GetDimY();
//		   complex **cmat = var->GetCMatrix2D();  
//   		
//		//   if(xsize*ysize <= 200)
//		//   {
//            str.Assign("{");
//			   for(long y = 0; y < ysize; y++)
//			   {
//				   for(long x = 0; x < xsize; x++)
//				   {
//                  if(!useFormat)
//                  {
//				         if(x < xsize-1)
//				         {
//					         if(cmat[y][x].i >= 0)
//					            sprintf(temp,"%g+%gi,",cmat[y][x].r,fabs(cmat[y][x].i));
//					         else
//					            sprintf(temp,"%g-%gi,",cmat[y][x].r,fabs(cmat[y][x].i));
//				         }    
//				         else
//				         {
//					         if(cmat[y][x].i >= 0)
//					            sprintf(temp,"%g+%gi",cmat[y][x].r,fabs(cmat[y][x].i));
//					         else
//					            sprintf(temp,"%g-%gi",cmat[y][x].r,fabs(cmat[y][x].i));
//				         }  
//                  }
//                  else
//                  {
//				         if(x < xsize-1)
//				         {
//					         if(cmat[y][x].i >= 0)
//                        {
//                           sprintf(formatc,"%s+%si,",format,format);
//					            sprintf(temp,formatc,cmat[y][x].r,fabs(cmat[y][x].i));
//                        }
//					         else
//                        {
//                           sprintf(formatc,"%s-%si,",format,format);
//					            sprintf(temp,formatc,cmat[y][x].r,fabs(cmat[y][x].i));
//                        }
//				         }    
//				         else
//				         {
//					         if(cmat[y][x].i >= 0)
//                        {
//                           sprintf(formatc,"%s+%si",format,format);
//					            sprintf(temp,formatc,cmat[y][x].r,fabs(cmat[y][x].i));
//                        }
//					         else
//                        {
//                           sprintf(formatc,"%s-%si",format,format);
//					            sprintf(temp,formatc,cmat[y][x].r,fabs(cmat[y][x].i));
//                        }
//				         }  
//                  }
//				      str.Concat(temp);
//   				   
//				   }
//				   if(y < ysize-1)
//				      str.Concat(";"); // End of row delimiter
//			   }
//            str.Concat("}"); // End of matrix
//		 //  }
//		 //  else
//		 //     str.Assign("large complex matrix");
//   		
//		   break;
//	   } 
//// 3D complex matrix ******************************
//      case(CMATRIX3D):    
//      {
//		   long xsize = var->GetDimX();
//		   long ysize = var->GetDimY();
//		   long zsize = var->GetDimZ();
//		   complex ***cmat = var->GetCMatrix3D();    
//   		
//		 //  if(xsize*ysize*zsize <= 200)
//		 //  {
//            str.Assign("{");
//			   for(long z = 0; z < zsize; z++)
//			   {
//				   for(long y = 0; y < ysize; y++)
//				   {
//					   for(long x = 0; x < xsize; x++)
//					   {
//                     if(!useFormat)
//                     {
//					         if(x < xsize-1)
//					         {
//						         if(cmat[z][y][x].i >= 0)
//						            sprintf(temp,"%g+%gi,",cmat[z][y][x].r,fabs(cmat[z][y][x].i));
//						         else
//						            sprintf(temp,"%g-%gi,",cmat[z][y][x].r,fabs(cmat[z][y][x].i));
//					         }    
//					         else
//					         {
//						         if(cmat[z][y][x].i >= 0)
//						            sprintf(temp,"%g+%gi",cmat[z][y][x].r,fabs(cmat[z][y][x].i));
//						         else
//						            sprintf(temp,"%g-%gi",cmat[z][y][x].r,fabs(cmat[z][y][x].i));
//					         } 
//                     }
//                     else
//                     {
//					         if(x < xsize-1)
//					         {
//						         if(cmat[z][y][x].i >= 0)
//                           {
//                              sprintf(formatc,"%s+%si,",format,format);
//						            sprintf(temp,formatc,cmat[z][y][x].r,fabs(cmat[z][y][x].i));
//                           }
//						         else
//                           {
//                              sprintf(formatc,"%s-%si,",format,format);
//						            sprintf(temp,formatc,cmat[z][y][x].r,fabs(cmat[z][y][x].i));
//                           }
//					         }    
//					         else
//					         {
//						         if(cmat[z][y][x].i >= 0)
//                           {
//                              sprintf(formatc,"%s+%si",format,format);
//						            sprintf(temp,"%g+%gi",cmat[z][y][x].r,fabs(cmat[z][y][x].i));
//                           }
//						         else
//                           {
//                              sprintf(formatc,"%s-%si",format,format);
//						            sprintf(temp,"%g-%gi",cmat[z][y][x].r,fabs(cmat[z][y][x].i));
//                           }
//					         } 
//
//                     }
//					      str.Concat(temp);
//					   }
//					   if(y < ysize-1)
//				         str.Concat(";"); // End of row delimiter
//				   }
//				   if(z < zsize-1)
//				      str.Concat(" ;; ");	// End of plane delimitier		         
//			   }
//            str.Concat("}"); // End of matrix
//		 //  }
//		 //  else
//		 //     str.Assign("large 3D complex matrix");
//   		
//		   break; 
//	   } 		      			                            
//   } 
//}


/********************************************************************
          Given a variable return its type as a number
*********************************************************************/

int VariableTypeID(Variable* var)
{
   switch(var->GetType())
   {
      case(UNQUOTED_STRING):
         return(1);

      case(FLOAT32): 
         return(2);

      case(COMPLEX):
         return(4);

      case(LIST):
         return(8);

      case(MATRIX2D):
         if(var->GetDimY() == 1)
            return(16);
         else
            return(64);

      case(CMATRIX2D):
         if(var->GetDimY() == 1)
            return(32);
         else
            return(128);

      case(MATRIX3D): 
         return(256);

      case(CMATRIX3D):
         return(512);

      case(MATRIX4D):
         return(1024);

      case(CMATRIX4D): 
         return(2048);

      case(NULL_VARIABLE): 
         return(0);

      default:
         return(0);
   }
}

/********************************************************************
    Add name to list. Add comma if more than one in list
*********************************************************************/

void AppendToVarList(CText &list, char *name, long &cnt)
{
   if(cnt++ == 0)
      list = list + name;
   else
      list = list + "," + name;
}

int GetVariableList(Interface* itfc, char args[])
{
	short r,type;
	char* whichStr;
	char *scopeStr;
	Variable whichVar;
	Variable scopeVar;
	short scope = 7;
	short which = 8191;
	CText txtlist = "";
	long cnt = 0;

	if((r = ArgScan(itfc,args,0,"[which variable type[, scope]]","ee","vv",&whichVar,&scopeVar)) < 0)
		return(r); 

	if(r >= 1)
	{
		if(whichVar.GetType() == UNQUOTED_STRING)
		{
			whichStr = whichVar.GetString();

			if(!strcmp(whichStr,"string"))
				which = 1;
			else if(!strcmp(whichStr,"float"))
				which = 2;
			else if(!strcmp(whichStr,"complex"))
				which = 4;
			else if(!strcmp(whichStr,"list"))
				which = 8;
			else if(!strcmp(whichStr,"matrix1d"))
				which = 16;
			else if(!strcmp(whichStr,"cmatrix1d"))
				which = 32;
			else if(!strcmp(whichStr,"matrix2d"))
				which = 64;
			else if(!strcmp(whichStr,"cmatrix2d"))
				which = 128;
			else if(!strcmp(whichStr,"matrix3d"))
				which = 256;
			else if(!strcmp(whichStr,"cmatrix3d"))
				which = 512;
			else if(!strcmp(whichStr,"matrix4d"))
				which = 1024;
			else if(!strcmp(whichStr,"cmatrix4d"))
				which = 2048;
			else if(!strcmp(whichStr,"all"))
				which = 4095;
			else
			{
				ErrorMessage("invalid variable type");
				return(ERR);
			}
		}
		else if(whichVar.GetType() == FLOAT32)
		{
			which = nint(whichVar.GetReal());
		}
		else
		{
			ErrorMessage("invalid data type for variable 'type'");
			return(ERR);
		}
	}

	if(r == 2)
	{
		if(scopeVar.GetType() == UNQUOTED_STRING)
		{
			scopeStr = scopeVar.GetString();

			if(!strcmp(scopeStr,"local"))
				scope = 1;
			else if(!strcmp(scopeStr,"global"))
				scope = 2;
			else if(!strcmp(scopeStr,"window"))
				scope = 4;
			else if(!strcmp(scopeStr,"all"))
				scope = 7;
			else
			{
				ErrorMessage("invalid scope");
				return(ERR);
			}
		}
		else if(scopeVar.GetType() == FLOAT32)
		{
			scope = nint(scopeVar.GetReal());
		}
		else
		{
			ErrorMessage("invalid data type for scope");
			return(ERR);
		}
	}


	// Search global variables
	if(scope & VGLOBAL)
	{
		Variable *var = &globalVariable;
		while(var)
		{
			var = var->GetNext(type);
			if(!var) break;
			if(!var->GetVisible()) continue;

			if(which & VariableTypeID(var))
			{
				AppendToVarList(txtlist, var->GetName(), cnt);
			}
		}
	}

	// Search local variables
	if(scope & VLOCAL)
	{
		if(itfc && itfc->macro)
		{
			Variable *var = &(itfc->macro->varList);
			while(var)
			{
				var = var->GetNext(type);
				if(!var) break;
				if(!var->GetVisible()) continue;

				if(which & VariableTypeID(var))
				{
					AppendToVarList(txtlist, var->GetName(), cnt);
				}
			}
		}
	}


	// Search window variables
	if(scope & VWINDOW)
	{
		if(GetGUIWin() && (itfc->win == GetGUIWin()))
		{
			Variable *var = &GetGUIWin()->varList;
			while(var)
			{
				var = var->GetNext(type);
				if(!var) break;
				if(!var->GetVisible()) continue;

				if(which & VariableTypeID(var))
				{
					AppendToVarList(txtlist, var->GetName(), cnt);
				}
			}
		}
	}

	// Convert the text to a list and then return in ansVar		
	if(txtlist == "")
		itfc->retVar[1].MakeNullVar();
	else
	{
		char **list = MakeListFromText(txtlist.Str(),&cnt);	
		itfc->retVar[1].MakeAndSetList(list,cnt);
		FreeList(list,cnt);
	}

	itfc->nrRetValues = 1;

	return(OK);
}


/********************************************************************
 Return a unique variable name by adding a number to a passed string
*********************************************************************/

int GetUniqueVariableName(Interface* itfc, char args[])
{
   Variable var;
   short r;
   CText base;
   CText name;
   long suffix = 0;

// Extract base name from argument list
   if((r = ArgScan(itfc,args,1,"matrix name","e","t",&base)) < 0)
      return(r);

// Search for first free variable name which starts with base
   name = base;
   while(IsAVariableCore(itfc,name.Str()))
   {
      suffix++;
      name.Format("%s%ld",base.Str(),suffix);
   }

// Return this variable name
   itfc->retVar[1].MakeAndSetString(name.Str());
   itfc->nrRetValues = 1;

   return(OK);
}

int ListEnvironmentVariables (char args[]) 
{
	int     iLength ;
	TCHAR * pVarBlock, * pVarBeg, * pVarEnd, * pVarName ;

	pVarBlock = GetEnvironmentStrings () ;  // Get pointer to environment block

	while (*pVarBlock)
	{
		if (*pVarBlock != '=')   // Skip variable names beginning with '='
		{
			pVarBeg = pVarBlock ;              // Beginning of variable name
			while (*pVarBlock++ != '=') ;      // Scan until '='
			pVarEnd = pVarBlock - 1 ;          // Points to '=' sign
			iLength = pVarEnd - pVarBeg ;      // Length of variable name

			// Allocate memory for the variable name and terminating
			// zero. Copy the variable name and append a zero.

			pVarName = (char*)calloc (iLength + 1, sizeof (TCHAR)) ;
			CopyMemory (pVarName, pVarBeg, iLength * sizeof (TCHAR)) ;
			pVarName[iLength] = '\0' ;

			// Put the variable name in the list box and free memory.

			TextMessage ("\n%s\n",pVarName) ;
			free (pVarName) ;
		}
		while (*pVarBlock++ != '\0') ;     // Scan until terminating zero
	}
	FreeEnvironmentStrings (pVarBlock) ;
	return(OK);
}


/********************************************************************
   Make a comma delimited list of currently defined matrix names
   (matrices may be in the global, local or window variable lists) 
*********************************************************************/

int GetMatrixList(Interface* itfc ,char args[])
{
   short r;
   short which;
   short type;
   long cnt = 0;
   CText txtlist = "";
   
   if((r = ArgScan(itfc,args,1,"which matrices (1-1D, 2-2D, 4-3D, 8-4D)","e","d",&which)) < 0)
      return(r); 

// Search global variables
	Variable *var = &globalVariable;
	while(var)
	{
	   var = var->GetNext(type);
	   if(!var) break;
      if(!var->GetVisible()) continue;
	   if(type == MATRIX2D || type == CMATRIX2D || type == MATRIX3D || type == CMATRIX3D || type == MATRIX4D || type == CMATRIX4D)
	   {
	      if((which & 1) && ((var->GetDimX() > 1 && var->GetDimY() == 1) || (var->GetDimX() == 1 && var->GetDimY() > 1)) && var->GetDimZ() == 1 && var->GetDimQ() == 1)
	      {
            AppendToVarList(txtlist, var->GetName(), cnt);
	      }
	      if((which & 2) && (var->GetDimX() > 1 && var->GetDimY() > 1) && (var->GetDimZ() == 1) && (var->GetDimQ() == 1))
	      {
            AppendToVarList(txtlist, var->GetName(), cnt);
	      }	      
	      if((which & 4) && (var->GetDimZ() > 1) && (var->GetDimQ() == 1))
	      {
            AppendToVarList(txtlist, var->GetName(), cnt);            
	      }
	      if((which & 8) && (var->GetDimQ() > 1))
	      {
            AppendToVarList(txtlist, var->GetName(), cnt);            
	      }
	   }
	}

// Search local variables
	if(itfc && itfc->macro)
	{
	   Variable *var = &(itfc->macro->varList);
	   while(var)
	   {
		   var = var->GetNext(type);
		   if(!var) break;
	      if(type == MATRIX2D || type == CMATRIX2D || type == MATRIX3D || type == CMATRIX3D || type == MATRIX4D || type == CMATRIX4D)
	      {
	         if((which & 1) && ((var->GetDimX() > 1 && var->GetDimY() == 1) || (var->GetDimX() == 1 && var->GetDimY() > 1)) && var->GetDimZ() == 1 && var->GetDimQ() == 1)
	         {
               AppendToVarList(txtlist, var->GetName(), cnt);
	         }
	         if((which & 2) && (var->GetDimX() > 1 && var->GetDimY() > 1) && (var->GetDimZ() == 1) && (var->GetDimQ() == 1))
	         {
               AppendToVarList(txtlist, var->GetName(), cnt);
	         }	      
	         if((which & 4) && (var->GetDimZ() > 1) && (var->GetDimQ() == 1))
	         {
               AppendToVarList(txtlist, var->GetName(), cnt);            
	         }
	         if((which & 8) && (var->GetDimQ() > 1))
	         {
               AppendToVarList(txtlist, var->GetName(), cnt);            
	         }
	      }
		}	   
	}


// Search window variables
	if(GetGUIWin() && (itfc->win == GetGUIWin()))
	{
	   Variable *var = &GetGUIWin()->varList;
	   while(var)
	   {
		   var = var->GetNext(type);
		   if(!var) break;
	      if(type == MATRIX2D || type == CMATRIX2D || type == MATRIX3D || type == CMATRIX3D || type == MATRIX4D || type == CMATRIX4D)
	      {
	         if((which & 1) && ((var->GetDimX() > 1 && var->GetDimY() == 1) || (var->GetDimX() == 1 && var->GetDimY() > 1)) && var->GetDimZ() == 1 && var->GetDimQ() == 1)
	         {
               AppendToVarList(txtlist, var->GetName(), cnt);
	         }
	         if((which & 2) && (var->GetDimX() > 1 && var->GetDimY() > 1) && (var->GetDimZ() == 1) && (var->GetDimQ() == 1))
	         {
               AppendToVarList(txtlist, var->GetName(), cnt);
	         }	      
	         if((which & 4) && (var->GetDimZ() > 1) && (var->GetDimQ() == 1))
	         {
               AppendToVarList(txtlist, var->GetName(), cnt);            
	         }
	         if((which & 8) && (var->GetDimQ() > 1))
	         {
               AppendToVarList(txtlist, var->GetName(), cnt);            
	         }
	      }	
		}   
	}

// Convert the text to a list and then return in ansVar	
   
   if(txtlist == "")
      itfc->retVar[1].MakeNullVar();
   else
   {
	   char **list = MakeListFromText(txtlist.Str(),&cnt);	
	   itfc->retVar[1].MakeAndSetList(list,cnt);
	   FreeList(list,cnt);
   }
   itfc->nrRetValues = 1;

   return(OK);
}

int ReplaceSubExpressions(Interface* itfc, char args[])
{
   short nrArgs;
   static CText mode;

   (itfc->processExpressions) ? (mode  = "true") : (mode = "false");
   if((nrArgs = ArgScan(itfc,args,1,"mode","e","t",&mode)) < 0)
      return(nrArgs); 

   if(mode == "yes" || mode == "true")
      itfc->processExpressions = true;
   else if(mode == "no" || mode == "false")
      itfc->processExpressions = false;
   else
   {
      ErrorMessage("invalid argument - should be true/false");
      return(ERR);
   }
   
	itfc->nrRetValues = 0;
   return(OK);
}

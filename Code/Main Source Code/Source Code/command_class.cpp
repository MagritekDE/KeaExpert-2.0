#include "stdafx.h"
#include "command.h"
#include "control.h"
#include "ctext.h"
#include "defines.h"
#include "macro_class.h"
#include "scanstrings.h"
#include "memoryLeak.h"

#pragma warning (disable: 4996) // Ignore deprecated library functions

Command::Command()
{
	type   = COMMAND_TYPE_NOT_SET;
	lineNr = 0;
	cmdStr = NULL;
   cmdRight = NULL;
   cmdLeft = NULL;
   cmdName = NULL;
   cmdArg = NULL;
} 

Command::~Command()
{
	if(cmdStr) delete [] cmdStr;
	if(cmdRight) delete [] cmdRight;
	if(cmdLeft) delete [] cmdLeft;
	if(cmdName) delete [] cmdName;
	if(cmdArg) delete [] cmdArg;
} 
 
/**********************************************************************************
* Set type of this command.                                                       * 
**********************************************************************************/
void Command::setType(short type)
{
	this->type = type;
}

/**********************************************************************************
* Set type of this command.                                                       * 
**********************************************************************************/
short Command::getType()
{
	return this->type;
}

/**********************************************************************************
* Get type of this command.                                                       * 
**********************************************************************************/
char* Command::getCmdName()
{
	return this->cmdName;
}

/**********************************************************************************
* Get LHS of this command.                                                       * 
**********************************************************************************/
char* Command::getCmdLeft()
{
	return this->cmdLeft;
}

/**********************************************************************************
* Get RHS of this command.                                                       * 
**********************************************************************************/
char* Command::getCmdRight()
{
	return this->cmdRight;
}

/**********************************************************************************
* Get arg of this command.                                                       * 
**********************************************************************************/
char* Command::getCmdArg()
{
	return this->cmdArg;
}

/**********************************************************************************
* Get command string for this command.                                            * 
**********************************************************************************/

char* Command::getCmdStr(void)
{
   return(cmdStr);
}

/**********************************************************************************
* Set command string for this command.                                            * 
**********************************************************************************/

void Command::setCmdStr(char *str)
{
	if (cmdStr)
	{
		delete[] cmdStr;
		cmdStr = 0;
	}
   cmdStr = new char[strlen(str)+1];
   strcpy(cmdStr,str);
}

/**********************************************************************************
* Set line number for this command.                                            * 
**********************************************************************************/

void Command::setLineNr(short line)
{
   lineNr = line;
}

/**********************************************************************************
* Set line number for this command.                                            * 
**********************************************************************************/

short Command::getLineNr(void)
{
   return(lineNr);
}

/**********************************************************************************
* Free memory used by command string.                                             * 
**********************************************************************************/

void Command::freeMemory()
{
   if(cmdRight) delete [] cmdRight;
   if(cmdLeft) delete [] cmdLeft;
   if(cmdName) delete [] cmdName;
   if(cmdArg) delete [] cmdArg;
   if(cmdStr) delete [] cmdStr;
   cmdRight = NULL;
   cmdLeft = NULL;
   cmdRight = NULL;
   cmdName = NULL;
   cmdArg = NULL;
	cmdStr = NULL;
}

/**********************************************************************************
* Extract left and then right side of an expression of the form left = right.     *
* If no equals sign is found the whole string is returned. Equals signed in       *
* brackets are ignored. (As in commands). Returns 2.                              * 
* If the command string does not contain an expression then 1 is returned.        *    
**********************************************************************************/

int Command::splitExpression(char **left, char **right)
{
	long size,i,j,c;
	short inBracket = 0;
	size = strlen(cmdStr);
   bool inWord = false;
   short cnt = 0;
   
// Extract left side
	*left = new char[size+1];

// Skip leading blanks			   
	for(j = 0; j < size; j++)
	   if(cmdStr[j] != ' ') break;

// Check for multireturn assignment (a,b) = ...
   if(cmdStr[j] == '(') 
   {
		for(c = 0,i = j; i < size; i++)
		{
			if(cmdStr[i] == '(')
				inBracket++;
		
			if(cmdStr[i] == ')')
				inBracket--;
				
			if(!inBracket && (cmdStr[i] == '='))
				break;
			(*left)[c++] = cmdStr[i];
		}
   }
   else
   {
		// Check for normal assignment a = ...
		// a1 b1 = treats a1 as a command and b1 = as an argument
		for(c = 0,i = j; i < size; i++)
		{
			if(!inBracket && cnt <= 2 && (cmdStr[i] == '='))
				break;

         if(cmdStr[i] == '(') // Ignore equals in command statements
           inBracket++;	

         if(cmdStr[i] == ')') 
           inBracket--;	

           					
			if(!inBracket && cmdStr[i] == ' ' && inWord) 
			{
			   inWord = false;
			   cnt++;
			}

			if(!inBracket && cmdStr[i] != ' ' && !inWord) 
			{
			   inWord = true;
			   cnt++;
			}
							
			(*left)[c++] = cmdStr[i];
		}
   }

	j = i+1;
	(*left)[c] = '\0';
	
// Remove trailing blanks from left string
	for(i = c-1; i >= 0; i--)
	{
	   if((*left)[i] != ' ')
	   {
	      (*left)[i+1] = '\0';
	      break;
	   }
	}
	
	if(j >= size)
	{
		return(FUNCTION);
   }
		
// Extract right side
	*right = new char[size-c+1];

// Skip leading blanks
	for(i = j; i < size; i++)
	   if(cmdStr[i] != ' ') break;

	for(c = 0,j = i; j < size; j++)
	{
		(*right)[c++] = cmdStr[j];
	}
	(*right)[c] = '\0';		
	return(ASSIGNMENT);
}

/**********************************************************************************
* Extract left and then right side of an expression of the form left = right.     *
* If no equals sign is found the whole string is returned. Equals signed in       *
* brackets are ignored. (As in commands). Returns ASSIGNMENT.                     * 
* If the command string does not contain an assignment then FUNCTION is returned. *    
**********************************************************************************/

int Command::splitExpression2()
{
	long size,i,j,c;
   short inBracket = 0;
   short inArray = 0;
   size = strlen(cmdStr);
   bool inWord = false;
   short cnt = 0;

// Extract left side
	cmdLeft = new char[size+1];

// Skip leading blanks
	for(j = 0; j < size; j++)
	   if(cmdStr[j] != ' ') break;

// Check for multireturn assignment (a,b) = ...
   if(cmdStr[j] == '(') 
   {
		for(c = 0,i = j; i < size; i++)
		{
			if(cmdStr[i] == '(')
				inBracket++;
		
			if(cmdStr[i] == ')')
				inBracket--;
				
			if(!inBracket && (cmdStr[i] == '='))
         {
				break;
         }
			cmdLeft[c++] = cmdStr[i];
		}
   }
   else
   {
		// Check for normal assignment a = ...
		// a1 b1 = treats a1 as a command and b1 = as an argument
		for(c = 0,i = j; i < size; i++)
		{
			if(!inBracket && !inArray && cnt <= 2 && (cmdStr[i] == '='))
         {
				break;
         }

         if(cmdStr[i] == '(') // Ignore equals in command statements
           inBracket++;	

         if(cmdStr[i] == ')') 
           inBracket--;	
 
         if (cmdStr[i] == '[') // Ignore equals in command statements
            inArray++;

         if (cmdStr[i] == ']')
            inArray--;

			if(!inBracket && !inArray && cmdStr[i] == ' ' && inWord)
			{
			   inWord = false;
			   cnt++;
			}

			if(!inBracket && !inArray && cmdStr[i] != ' ' && !inWord)
			{
			   inWord = true;
			   cnt++;
			}							
			cmdLeft[c++] = cmdStr[i];
		}
   }

	j = i+1;
	cmdLeft[c] = '\0';
	
// Remove trailing blanks from left string
	for(i = c-1; i >= 0; i--)
	{
	   if(cmdLeft[i] != ' ')
	   {
	      cmdLeft[i+1] = '\0';
	      break;
	   }
	}

// There is no assignment
	if(j >= size)
	{
		return(FUNCTION);
   }	

// Extract right side
	cmdRight = new char[size-c+1];

// Skip leading blanks
	for(i = j; i < size; i++)
	   if(cmdStr[i] != ' ') break;

	for(c = 0,j = i; j < size; j++)
	{
		cmdRight[c++] = cmdStr[j];
	}
	cmdRight[c] = '\0';		
	return(ASSIGNMENT); // There is an assignment
}

short Command::extractCmdNameAndArg()
{
   long i,len;
   bool inQuote = false;
   long start;
   extern bool IsNumber(char *str);
   int j,k;

  if(!cmdLeft)
  {
     SimpleErrorMessage("no left command");
     return(-1);
  }

//*************** Extract the name of the command ********************

   len = strlen(cmdLeft);
    
   for(i = 0; i < len; i++) // Search for end of name
   {
      if(IsUnEscapedQuote(cmdLeft,i,len)) // Allow for quoted filename e.g. "test 1"(1,2,3)
      {
         inQuote = !inQuote;
         continue;
      }

      if(!inQuote && (cmdLeft[i] == '(' || cmdLeft[i] == ' ' ||cmdLeft[i] == '\t')) // Look for space or left bracket
         break;                                                                   // this will be the end of the name
   }
   cmdName = new char[i+1];    // Allocate space for command name

   StrNCopy(cmdName,cmdLeft,i); // Copy up to, but not including, end delimiter
   cmdName[i] = '\0';
   start = i;

// See if its an expression 
   if(cmdName[0] == '\0') // Its an expression (...)
     return(-2);

   if(IsNumber(cmdName)) // Its a number so this is an expreesion
     return(-2);

   if(cmdName[0] == '"' && cmdName[i-1] == '"') // Its a string so this might be an expression
     return(-2);

//*************** Extract the argument for the command ********************

   int bracketCnt = 0;
   bool bracketed = false;

// Find start of argument list (ignore leading spaces)
   for(i = start; i < len; i++) 
   {
      if(cmdLeft[i] != ' ')
         break;
   }

// See if argument is bracketed or not
   if(cmdLeft[i] == '(') // Yes it is
   {
      bracketed = true;
   }

// Are no arguments found?  
   if(i == len) 
   {
      cmdArg = new char[MAX_STR]; // Don't know what the user will do
      cmdArg[0] = '\0';
      return(0);
   }

// Search for end of argument list if bracketed 
   if(bracketed)
   {
      for(j = i; j < len; j++) 
      {
         if(cmdLeft[j] == '(') bracketCnt++;

         if(cmdLeft[j] == ')')
         {
            bracketCnt--;
            if(bracketCnt == 0)
               break;
         }
      }
   }
   else // Rest of string is argument
   {
      j = len;
      i--;
   }

  if(bracketCnt != 0)
  {
     SimpleErrorMessage("unmatched bracket");
     return(-1);
  }

// Check for stuff after the closing bracket
  if(j < len-1)
  {
     if(bracketed)
     {
        if(!stricmp(cmdName,"pr")) // 2.2.7 If matching brackets then return whole statement e.g. pr (2*3)/4
        {
         // Copy argument to  argument list 
            cmdArg = new char[len-i+1];
            for(k = i; k < len; k++)
               cmdArg[k-i] = cmdLeft[k];
	         cmdArg[k-i] = '\0';
            return(0);
        }
        else
        { 
           this->type = 3;
           return(0);
        }
     }
     else // Flag error for other cases i.e. sin(23)/6
     {
        for(k = j+1; k < len; k++)
        {
           if(cmdLeft[k] != ' ' && cmdLeft[k] != '\t')
           {
              SimpleErrorMessage("Statement is invalid - correct syntax is:\n\n      result = expression\n\n   or\n\n      function(arguments)\n");
               return(-1);
           }
        }
     }
  }
  
// Copy argument to  argument list  
   cmdArg = new char[j-i];
   for(k = i+1; k < j; k++)
      cmdArg[k-i-1] = cmdLeft[k];
	cmdArg[k-i-1] = '\0';
	
	return(0);
}




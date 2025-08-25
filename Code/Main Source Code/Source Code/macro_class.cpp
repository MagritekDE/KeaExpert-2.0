/**********************************************************************************
* Macro member functions for extracting commands from a raw file data             
**********************************************************************************/
#include "stdafx.h"
#include "macro_class.h"
#include "command.h"
#include "globals.h"
#include "guiWindowsEvents.h"
#include "interface.h"
#include "scanstrings.h"
#include "files.h"
#include "debug.h"
#include "memoryLeak.h"

extern CommandInfo gErrorInfo; 

// Constructor

Macro::Macro(void)
{
   rawMacro     = NULL;
   cleanedMacro = NULL;
   command      = NULL;
   tryNr        = NULL;
   jumpNr       = NULL;
   stack        = NULL;
   nextNr       = NULL;
   startLine    = 0;
   inTryBlock   = false;
}

// Destructor

Macro::~Macro(void)
{
   cleanUp();   
}

/**********************************************************************************
* Add unparsed macro data to the macro class.                                     
**********************************************************************************/

void Macro::addRawMacro(char *data)
{
   if(!data)
   {
       ErrorMessage("no data passed to macro '%s'",macroName.Str());
       throw "";
   }
   rawMacro = new char[strlen(data)+1];
   if(!rawMacro)
   {
       ErrorMessage("out of memory during macro processing for '%s' (1)",macroName.Str());
       throw "";
   }
   strncpy_s(rawMacro,strlen(data)+1,data,_TRUNCATE);
}

 
/**********************************************************************************
* Remove comments from macro                                                       
**********************************************************************************/

void Macro::removeComments()
{
   long i,j,k;
   long len = strlen(rawMacro);
   bool inString = false;
   
   cleanedMacro = new char[strlen(rawMacro)+1];
   if(!cleanedMacro)
   {
       ErrorMessage("out of memory during processing of macro '%s' (2)",macroName.Str());
       throw "";
   }

   strncpy_s(cleanedMacro,strlen(rawMacro)+1,rawMacro,_TRUNCATE);
   for(i = 0, j = 0; i < len; i++)
   {
      if(IsUnEscapedQuote(rawMacro,i,len)) // Ignore # symbol in strings
         inString = !inString;

      if(rawMacro[i] == '#' && !inString)
      {
         for(k = i; k < len; k++) // Search for end of line
         {
            if(rawMacro[k] == '\r' && rawMacro[k+1] == '\n')
            {
               i = k;
               break;
            }
         }
         if(k == len)
            break;
      }
      cleanedMacro[j++] = rawMacro[i];
   }
   cleanedMacro[j] = '\0';
}


/**********************************************************************************
* Remove EOL characters (\r\n) from macro and replace with EOL code   
* Also replace EOL characters (\r\n) by spaces if its a continuation line                                                  
**********************************************************************************/

void Macro::removeEOL()
{
   long i;
   char *txt= cleanedMacro;
   bool cont = false;
   long len = strlen(txt);

   for(i = 0; i < len; i++)
   {
      if(txt[i] == '@') // Continuation 
      {
         txt[i] = ' ';
         cont = true;
         continue;
      }
      
      if(txt[i] == '\r' && txt[i+1] == '\n') 
      {
         if(cont == true) // Continuation but not line ending
         {
            txt[i] = EOL_CONT;
            txt[i+1] = ' ';
            cont = false;
         }
         else
         {
            txt[i] = EOL;  // Note end of line (and command)
            txt[i+1] = ' ';
         }
         i++;
      }
   }
}


/**********************************************************************************
*          Removes unnecessary spaces from a macro  (all but those in strings)
**********************************************************************************/

void Macro::removeSpaces()
{
   long i,j;
   char *txt= cleanedMacro;
   bool inString = false;
   long len = strlen(txt);
   
   for(i = 0, j = 0; i < len; i++)
   {
      if(IsUnEscapedQuote(txt,i,len))  // Quote detected?
      {
          inString = !inString;
      }

      if(!inString && (txt[i] == ' ' || txt[i] == '\t')) // Ignore spaces or tabs if outside string
         continue;
      
      txt[j++] = txt[i]; // Update string
   }
   txt[j] = '\0';
}

/***************************************************************************************
* Returns the number of commands in a macro (as delineated by nonbracketed semicolons) *                                     
***************************************************************************************/

long Macro::CountMacroCommands()
{
   long i,len;
   long pos = 0;
   long cnt = 0;
   bool inString = false;
   long inRealArray = 0;
   long inCompArray = 0;
   long bracketCnt = 0;
   long inListArray = 0;
   char c;
      
   len = strlen(cleanedMacro);
   
// Keep searching for the next valid command until end of string found 
   do
   {
   // Search for the start of the command (first non-semicolon)  
	   for(i = pos; i < len; i++)  
	   {
         c = cleanedMacro[i];
	   	         
	      if(c == ';' || c == EOL) // Ignore empty commands
	         continue;
	      else if(c == ' ' || c == '\t') // Ignore spaces or tabs
	         continue;	         
	      break;
       }
       pos = i;
       if(i == len)
          return(cnt);
          
	// Search for the end of the command (a non-bracketed semicolon)
	   for(i = pos; i < len; i++)
	   {
         c = cleanedMacro[i];

    // If in a string ignore everything ***********************	      	                     
	   	if(IsUnEscapedQuote(cleanedMacro,i,len)) 
	      {
	         if(inString)
	           inString = false;
	         else
	           inString = true;
	      }
	      if(inString) continue;

      // If in an argument list ignore everything ***************	         
	      if(c == '(' && !inString) 
           bracketCnt++;
	      else if(c == ')' && !inString) 
           bracketCnt--;
	      if(bracketCnt > 0) continue;

   // Ignore anything in a real or complex array ***************
	      if(c == '[')
            inRealArray++;
	      else if(c == ']') 
            inRealArray--;
	      else if(i < len-1 && c == '<' && cleanedMacro[i+1] == '<')
            inListArray++;
	      else if(i < len-1 && c == '>' && cleanedMacro[i+1] == '>')
            inListArray--;
	      else if(c == '{') 
            inCompArray++;
	      else if(c == '}')
            inCompArray--;
	      if(inRealArray > 0 || inCompArray > 0 || inListArray > 0) continue;

    // If end of command or line is found start looking for next command ***    
	      if(c == ';' || c == EOL)
	         break;	
	   }
	   pos = i+1;
	   cnt++;
    }
    while(pos <= len);       
    return(cnt);
 }


/**********************************************************************************
* Macro initialiser - allocates space for num COMMAND structures.                 
**********************************************************************************/

void Macro::buildArray(int num)
{
   nrCommands = num;
   command = new Command[sizeof(Command)*nrCommands];
   if(!command)
   {
       ErrorMessage("out of memory during processing of macro '%s' (3)",macroName.Str());
       throw "";
   }
   stack = new ForStack[sizeof(ForStack)*nrCommands];
   if(!stack)
   {
       ErrorMessage("out of memory during processing of macro '%s' (4)",macroName.Str());
       throw "";
   }
   jumpNr = new long[sizeof(long)*nrCommands];
   if(!jumpNr)
   {
       ErrorMessage("out of memory during processing of macro '%s' (5)",macroName.Str());
       throw "";
   } 
   nextNr = new long[sizeof(long)*nrCommands];
   if(!nextNr)
   {
       ErrorMessage("out of memory during processing of macro '%s' (5)",macroName.Str());
       throw "";
   }
   for(int i = 0; i < nrCommands; i++)
      nextNr[i] = -1;

   tryNr = new long[sizeof(long)*nrCommands];
   if(!tryNr)
   {
       ErrorMessage("out of memory during processing of macro '%s' (5)",macroName.Str());
       throw "";
   } 
}

/*********************************************************************************
* Extracts the commands stored in cleanedMacro and stores then in the command    
* field of the macro COMMAND structure. Also records line numbers from raw macro.                                          
*********************************************************************************/

void Macro::extractCommands()
{
	long pos;
	char *cmd;
	long lineStart,lineEnd;
   
   if(cleanedMacro == (char*)0)
   {
      ErrorMessage("no raw data for macro '%s'",macroName.Str());
      throw "";
   }
        
	pos = 0;		
	lineStart = this->startLine;	
	for(long i = 0; i < nrCommands; i++)
	{
		if((cmd = extractNextCommand(i,pos,lineStart,lineEnd)) == (char*)0)
        throw "";
		addCmdInfo(i,cmd,lineStart);
		delete[] cmd;
		lineStart = lineEnd;
	}
}

/*********************************************************************************
*
* Extracts the command starting at position pos and ending with a semicolon.
* The line numbers in rawMacro the command came from are also returned.              
*                                                                                                                                       *
*********************************************************************************/

char* Macro::extractNextCommand(long cmdNr, long &pos, long &lineNrStart, long &lineNrEnd)
{
   long i,j,cnt;
   char c;
   char *cmd;
   bool inString = false;
   long inListArray = 0;
   long inRealArray = 0;
   long inCompArray = 0;
   long bracketCnt = 0;
   long len;
   char errStr[MAX_STR];
   char *txt;


   txt = cleanedMacro;
   
   len = strlen(txt);
   
// Search for the next valid command (ignore blank lines or comments)  
   for(i = pos; i < len; i++)
   {
      c = txt[i];
      if(c == ';') // Ignore empty commands
         continue; 
      if(c == EOL || c == EOL_CONT) // Ignore empty lines
      {
         lineNrStart++;
         continue;
      }
      else if(c == ' ' || c == '\t') // Ignore spaces or tabs
         continue;	         
      break;
   }
   if(i == len) // No command found
     return((char*)0);
     
   
   pos = i; // Reset pos to start of command
   lineNrEnd = lineNrStart;
   
// Search for the end of the command (either a semicolon or an EOL without continuation)  
   for(i = pos; i < len; i++)
   {
      c = txt[i];
      
    // If in a string ignore everything ***********************
	   if(IsUnEscapedQuote(txt,i,len)) //ESCAPE
      {                                                        
         if(inString)
           inString = false;
         else
           inString = true;
      }

      if(inString && (c == EOL || c == EOL_CONT))
      {
         lineNrEnd++;
         continue;
      }
            
	   if(inString) continue;

   // Ignore anything in a real or complex array ***************
	   if(c == '[') 
         inRealArray++;
	   else if(c == ']') 
         inRealArray--;
	   else if(c == '{') 
         inCompArray++;
	   else if(c == '}') 
         inCompArray--;
 	   else if(i < len-1 && c == '>' && txt[i+1] == '>') 
         inListArray--;
 	   else if(i < len-1 && c == '<' && txt[i+1] == '<') 
         inListArray++;

      if((inRealArray > 0 || inCompArray > 0 || inListArray > 0) && (c == EOL || c == EOL_CONT))
      {
         lineNrEnd++;
         continue;
      }
               
	   if(inRealArray > 0 || inCompArray > 0 || inListArray > 0) continue;
                
    // If in an argument list ignore everything ***************
	   if(c == '(') 
         bracketCnt++;
	   else if(c == ')') 
         bracketCnt--;
         
      if(bracketCnt > 0 && (c == EOL || c == EOL_CONT))
      {
         lineNrEnd++;      
         continue;
      }  
              
      if(bracketCnt > 0) continue;

   // Check for end of command (but ignore if in brackets array or string)             
      if(c == ';')
      {
         break;
      }
      else if(c == EOL)
      {
         lineNrEnd++;      
         txt[i] = ';'; // Replace EOL with end of command
         break;  
      } 
      else if(c == EOL_CONT)
      {
         lineNrEnd++;      
         txt[i] = ' '; // Replace EOL_CONT with blank 
      }         
   }
   
// Make space for new command
	if(!(cmd = new char[i-pos+1])) // Allocate space for line
	{
      ErrorMessage("out of memory in macro '%s'",macroName.Str()); 
      throw "";
   }
   
// Extract the command from the string replacing ignored EOL with spaces
   for(j = pos, cnt = 0; j < i; j++)
   {
      if(txt[j] == EOL || txt[j] == EOL_CONT)
         txt[j] = ' ';
		cmd[cnt++] = txt[j];
   }
	cmd[cnt] = '\0';
   pos = i+1;
   
// Check for and report any errors *******************************************
   if(bracketCnt != 0)
   {
      strncpy(errStr,cmd,30); errStr[29] = '\0';
      if(strlen(cmd) > 30) strcat(errStr," ...");
      ErrorMessage("unmatched argument brackets '(...)'");
      ReportMacroError(cmd,cmdNr,lineNrStart,0,0);  
      delete [] cmd;
      return((char*)0);
   }
   
   if(inRealArray != 0)
   {
      strncpy(errStr,cmd,30); errStr[29] = '\0'; 
      if(strlen(cmd) > 30) strcat(errStr," ...");        
      ErrorMessage("unmatched array bracket '[...]'");
      ReportMacroError(cmd,cmdNr,lineNrStart,0,0);  
      delete [] cmd;
      return((char*)0);
   }

   if(inListArray != 0)
   {
      strncpy(errStr,cmd,30); errStr[29] = '\0'; 
      if(strlen(cmd) > 30) strcat(errStr," ...");        
      ErrorMessage("unmatched array bracket '<<...>>'");
      ReportMacroError(cmd,cmdNr,lineNrStart,0,0);  
      delete [] cmd;
      return((char*)0);
   }

   if(inCompArray != 0)
   {
      strncpy(errStr,cmd,30); errStr[29] = '\0';  
      if(strlen(cmd) > 30) strcat(errStr," ...");       
      ErrorMessage("unmatched array bracket '{...}'"); 
      ReportMacroError(cmd,cmdNr,lineNrStart,0,0); 
      delete [] cmd;
      return((char*)0);
   }

   if(inString != 0)
   {
      strncpy(errStr,cmd,100);   
      ErrorMessage("unterminated string");     
      ReportMacroError(cmd,cmdNr,lineNrStart,0,0);
      delete [] cmd;
      return((char*)0);
   }
   
      
	return(cmd);
}

void Macro::ReportMacroError(char *cmd, long cmdNr, long lineNr, short maxDepth, short macroDepth)
{
	const int TEMP_LENGTH = 30;
   char temp[TEMP_LENGTH];
   char depth = maxDepth-macroDepth;
	extern bool gInTryBlock;

	
   if(cmd[0] != '\0' && !gInTryBlock && !inTryBlock)
   {
      if(depth < 0) depth = 0;

   // Indent error text based on call stack depth
      char *shift = new char[depth*2+4];
      memset(shift,(int)' ',depth*2+3);
      shift[depth*2+3] = '\0';

      if(maxDepth == macroDepth) // Report line where error occurred
      {
      // Report the contents of the error line
	      if(strlen(cmd) > 25)
	      {
	         strncpy(temp,cmd,25); temp[25] = '\0';
	         TextMessage("%sLine reads : %s ...\n",shift,temp);
	      }
	      else
	         TextMessage("%sLine reads : %s\n",shift,cmd);


      // Report the file name and line number of the error 
	      if(procName != "")	 
	      {
	         if(macroName != "")  
               TextMessage("%sError occurred in command starting on line #%ld of macro '%s' (command #%d of procedure '%s')\n",shift,lineNr+1,macroName.Str(),cmdNr+1,procName.Str());
	         else
	         {
	   	      GetErrorInfo()->macro = "current_text";
               TextMessage("%sError occurred on line #%ld of macro 'current_text:%s'\n",shift,lineNr+1,cmdNr+1,procName.Str());
            }
	      }   
         else
	         TextMessage("%sError occurred in line #%d of macro '%s'\n",shift,lineNr+1,macroName.Str());
      }
      else if(GetErrorInfo()->type != "Throw") // Report call stack
      {
	      if(strlen(cmd) > 25)
         {
	         strncpy(temp,cmd,25);
            temp[25] = '\0';
         }
	      else
            strncpy_s(temp,TEMP_LENGTH,cmd,_TRUNCATE);

	      if(procName != "")	 
	      {
	         if(macroName != "") 
               TextMessage("%sCalled from %s, line #%ld of '%s' in procedure '%s'\n",shift,cmd,lineNr+1,macroName.Str(),procName.Str());
            else
	            TextMessage("%sCalled from %s, command #%d in 'current_text:%s'\n",shift,cmd,cmdNr+1,procName.Str());
         }
         else
	         TextMessage("%sCalled procedure %s, command #%d in macro '%s'\n",shift,cmd,cmdNr+1,macroName.Str());
      }
      delete [] shift;
   }

       
// Record location of original error so user can find it using editor menu option and lasterror command.
   if(macroDepth == maxDepth)
   {
	   GetErrorInfo()->macro = macroName;
	   GetErrorInfo()->path = macroPath;
	   GetErrorInfo()->procedure = procName;
	   GetErrorInfo()->type = "Error";
		GetErrorInfo()->lineNr = lineNr;
	   GetErrorInfo()->command = cmd;
		GetErrorInfo()->description = GetErrorInfo()->lastError;
		gErrorInfo.blocked = true;
		gErrorInfo = *GetErrorInfo();
		gErrorInfo.blocked = false;
   }

}	


void Macro::ReportMacroAbort(char *cmd, long cmdNr, long lineNr)
{
   char temp[30];
   
	if(strlen(cmd) > 25)
	{
	   strncpy(temp,cmd,25); temp[25] = '\0';
	   TextMessage("   Line reads : %s ...\n",temp);
	}
	else
	    TextMessage("   Line reads : %s\n",cmd);

	GetErrorInfo()->macro = macroName;
	GetErrorInfo()->path = macroPath;
	GetErrorInfo()->procedure = procName;
	
	if(procName != "")	 
	{
	   if(macroName != "")  
	       TextMessage("   Abort occurred in command #%d of macro '%s:%s'\n",cmdNr+1,macroName.Str(),procName.Str());
	   else
	   {
	   	GetErrorInfo()->macro = "current_text";
	      TextMessage("   Abort occurred in command #%d of macro 'current_text:%s'\n",cmdNr+1,procName.Str());
      }
	}   
   else
	   TextMessage("   Abort occurred in command #%d of macro '%s'\n",cmdNr+1,macroName.Str());
	GetErrorInfo()->lineNr = lineNr;
	//GetErrorInfo()->command = cmd;
}	

/**********************************************************************************
* Free up all memory used by macro                                        
**********************************************************************************/

void Macro::cleanUp(void)
{
   if(command)
   {
      for(long i = 0; i < nrCommands; i++)
         command[i].freeMemory();
   }
   
   if(command)      delete[] command;
   if(stack)        delete[] stack;
   if(jumpNr)       delete[] jumpNr;
   if(nextNr)       delete[] nextNr;
   if(tryNr)        delete[] tryNr;
   if(rawMacro)     delete[] rawMacro;
   if(cleanedMacro) delete[] cleanedMacro;

   varList.RemoveAll(); // Remove all local variables
   procList.RemoveAll(); // Remove all macro procedures
}


/**********************************************************************************
* Return number of commands in macro                                         
**********************************************************************************/

int Macro::getSize()
{
   return(nrCommands);
}

/**********************************************************************************
* Return ith command in macro                                                          
**********************************************************************************/

Command* Macro::getCmd(long index)
{
   if(index < 0 || index >= nrCommands)
   {
      ErrorMessage("range error in getCmd in macro '%s'",macroName.Str());   
      throw "";
   }
   return(&command[index]);
}
   
/**********************************************************************************
* Add macro command to COMMAND array along with position in raw macro file.        
**********************************************************************************/

void Macro::addCmdInfo(long index, char *str, long line)
{
   Command *cmd;
   
   if(index < 0 || index >= nrCommands)
   {
      ErrorMessage("range error in addCmdInfo in macro '%s'",macroName.Str());   
      throw "";
   }   
   cmd = getCmd(index);
   cmd->setCmdStr(str);
   cmd->setLineNr(line);
   cmd->setType(cmd->splitExpression2());
   if(cmd->getType() == FUNCTION) // No assignment
   {
      short result = cmd->extractCmdNameAndArg();

      if(result == ERR)
      {        
         ReportMacroError(cmd->getCmdStr(), index, cmd->getLineNr(),0,0);  
         throw "";
      }

      if(result == -2)
        cmd->setType(EXPRESSION);
   }
}

/********************************************************************************
*                 Extracts the command name from a command string 
*                                 
* e.g. button(10,20,16,10,"test") returns "button" and position of argument
*                             
* Note command names must not contain spaces or left brackets.  
* However command can be embedded in quotes and contain anything
* this is useful when command is a filename.
*           
********************************************************************************/

short Macro::extractCmdName(char *cmdStr,char **name)
{
   long i,len;
   bool inQuote = false;
   
   len = strlen(cmdStr);
    
   for(i = 0; i < len; i++) // Search for end of name
   {
      if(IsUnEscapedQuote(cmdStr,i,len)) // Allow for quoted filename e.g. "test 1"(1,2,3)
      {
         inQuote = !inQuote;
         continue;
      }

      if(!inQuote && (cmdStr[i] == '(' || cmdStr[i] == ' ' ||cmdStr[i] == '\t')) // Look for space or left bracket
         break;                                                                   // this will be the end of the name
   }
   *name = new char[i+1];    // Allocate space for command name
   StrNCopy(*name,cmdStr,i); // Copy up to, but not including, end delimiter
   (*name)[i] = '\0';
   return(i);
}


/*********************************************************************************
*               Extract the command argument from a command string  
*                                 
*  e.g. button(10,20,16,10,"test")  returns "10,20,16,10,"test""       
*                       
* Note, argument list must be comma delimited but brackets are optional
* in which case there is a space between the command name and first argument  
*
*  e.g. button 10,20,16,10,"test" 
*
* Routine starts searching at index i       
*   
**********************************************************************************/

short Macro::extractCmdArg(char *cmdStr, long start, char **list)
{
   long i,j,k,len;
   long bracketCnt = 0;
   bool bracketed = false;
   
   len = strlen(cmdStr);

// Find start of argument list (ignore leading spaces)
   for(i = start; i < len; i++) 
   {
      if(cmdStr[i] != ' ')
         break;
   }

// See if argument is bracketed or not
   if(cmdStr[i] == '(') // Yes it is
   {
      bracketed = true;
   }

// Are no arguments found?
   if(i == len) 
   {
      *list = new char[100]; // Don't know what the user will do
      (*list)[0] = '\0';
      return(OK);
   }

// Search for end of argument list if bracketed   
   if(bracketed)
   {
      for(j = i; j < len; j++) 
      {
         if(cmdStr[j] == '(') bracketCnt++;

         if(cmdStr[j] == ')')
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
     ErrorMessage("unmatched bracket");
     return(ERR);
  }
  
// Copy argument to returned argument list  
   (*list) = new char[j-i];
   for(k = i+1; k < j; k++)
      (*list)[k-i-1] = cmdStr[k];
	(*list)[k-i-1] = '\0';
	
	return(OK);
}


/* For-next loop stack functions */

void Macro::InitStack(void)
{
   stackPos = 0;
   stack[stackPos].var         = (Variable*)0;
 //  stack[stackPos].startValue  = 0;
   stack[stackPos].steps    = 0;
   stack[stackPos].stepSize    = 1;   
}

void Macro::PushStack(Variable *var, double loopStart, double stepSize, double stepsLeft, double steps)
{
   stackPos++;
   if(stackPos == nrCommands)
   {
      TextMessage("\n\nStack overflow\n\n");
   }
   else
   {
     stack[stackPos].var         = var;
     stack[stackPos].steps       = steps;
     stack[stackPos].stepsLeft   = stepsLeft;
     stack[stackPos].loopStart   = loopStart;
     stack[stackPos].stepSize    = stepSize;
   }
}

void Macro::PopStack()
{
  stackPos--;
}

#define ASSIGNMENT 2

void Macro::ProcedureCheck(void)
{
   bool inProc = false;
   char *cmdName = 0, *left = 0, *right = 0;
   Command *cmd;
   short type;

// Check for matching procedure-endproc statement            
   for(long i = 0; i < nrCommands; i++)
   {
   	cmd = getCmd(i);
	   type = cmd->splitExpression(&left,&right);
	   if(type == ASSIGNMENT)
	   {
         if(left)  { delete [] left;   left = 0;};
         if(right) { delete [] right; right = 0;};
	      continue;
	   }
	      
	   extractCmdName(left,&cmdName); 
	   
      if(!inProc && !strcmp(cmdName,"procedure"))
      {
         inProc = true;
      }
      else if(inProc && !strcmp(cmdName,"endproc"))
      {
         inProc = false;
      }
      else if(inProc && !strcmp(cmdName,"procedure"))
      {
		   if(left)  { delete [] left;   left = 0;};
		   if(right) { delete [] right; right = 0;};
		   if(cmdName)  { delete [] cmdName;   cmdName = 0;};
         ErrorMessage("nested procedure statements or missing endproc statement");
         ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
         throw "";  
      }
      else if(!inProc && !strcmp(cmdName,"endproc"))
      {
		   if(left)  { delete [] left;   left = 0;};
		   if(right) { delete [] right; right = 0;};
		   if(cmdName)  { delete [] cmdName;   cmdName = 0;};      
         ErrorMessage("missing procedure statement");
         ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
         throw ""; 
      }      
      if(left)  { delete [] left;   left = 0;};
      if(right) { delete [] right; right = 0;};
      if(cmdName)  { delete [] cmdName;   cmdName = 0;};
   } 
   if(inProc)
   {
      ErrorMessage("missing endproc statement\n   in macro '%s'",macroName.Str());
      ReportMacroError("",0,1,0,0); 
      throw "";  
   }    
}

// Process conditional statements
 


void Macro::ForNextCheck(void)
{
   long stack[MAX_NESTING][MAX_BREAKS]; // Records forloop & exitfor statement postions
   long x[MAX_NESTING]; // Records number of exitfor statements for each forloop
   long p=-1,type,ln;
   long i;
   char *cmdName = 0, *left = 0, *right = 0;
   Command *cmd;

// Initialize all jump numbers to next instruction
   for(i = 0; i < nrCommands; i++)
   {
      jumpNr[i] = i+1;
      nextNr[i] = i+1;
      tryNr[i] = -1;
   }

// Search for for-next statements            
   for(i = 0; i < nrCommands; i++)
   {
   	cmd = getCmd(i);
	   type = cmd->splitExpression(&left,&right);
	   if(type == ASSIGNMENT)
	   {
         if(left)  { delete [] left;   left = 0;};
         if(right) { delete [] right; right = 0;};
	      continue;
	   }
	      
	   extractCmdName(left,&cmdName); 
	   
      if(!strcmp(cmdName,"for"))
      {
         stack[++p][0] = i;
         x[p] = 1;
         if(p == MAX_NESTING-1)
         {
            if(left)  { delete [] left;   left = 0;};
            if(right) { delete [] right; right = 0;};
            if(cmdName)  { delete [] cmdName;   cmdName = 0;};
            ErrorMessage("For-next loops too deeply\n   nested in macro '%s:%s'",macroName.Str(),procName.Str());
            throw "";
         }
      }
      else if(!strcmp(cmdName,"exitfor"))
      {
         if(p == -1)
         {
            ErrorMessage("exitfor not in for-loop\n    in macro '%s:%s'",macroName.Str(),procName.Str());
            ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
            throw "";
         }
         if(x[p] == MAX_BREAKS)
         {
            ErrorMessage("Too many breaks in for-loop\n    in macro '%s:%s'",macroName.Str(),procName.Str());
            ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
            throw "";
         }
         stack[p][x[p]++] = i;
      }
      else if(!strcmp(cmdName,"next"))
      {
         if(p == -1) // V2.2 21/6/6
         {
            ErrorMessage("Next without matching for statement\n    in macro '%s:%s'",macroName.Str(),procName.Str());
            ReportMacroError(cmd->getCmdStr(),i,cmd->getLineNr(),0,0); 
            throw "";
         }

         long k = stack[p][0];
         jumpNr[i] = k+1;

         for(k = 0; k < x[p]; k++) // Add jump numbers for forloop and exitfor statements
         {
           ln = stack[p][k];
           jumpNr[ln] = i+1;
         }
         x[p] = 0;
         p--;

      }
      if(left)  { delete [] left;   left = 0;};
      if(right) { delete [] right; right = 0;};
      if(cmdName)  { delete [] cmdName;   cmdName = 0;};
   }         
   if(p > -1)
   {
      ErrorMessage("missing 'next' statement\n   in macro '%s:%s'",macroName.Str(),procName.Str());
      ReportMacroError(getCmd(stack[p][0])->getCmdStr(),stack[p][0],getCmd(stack[p][0])->getLineNr(),0,0); 
      throw "";   
   }
   else if(p < -1)
   {
      ErrorMessage("missing 'for' statement\n   in macro '%s:%s'",macroName.Str(),procName.Str());
      ReportMacroError("",0,1,0,0); 
      throw "";  
   }
}


// Check for valid if statement and figure out jump numbers


#define MAX_IFELSEIF 50  // Can have up to 49 elseif statements per level

void Macro::IfEndifCheck(void)
{
   long stack[MAX_NESTING][MAX_IFELSEIF];    // Stores locations of if, else and else if statements
   long x[MAX_NESTING];                      // Index for elseif/else positions
   long  elsesfound[MAX_NESTING];            // How many else statements have been found for a matching if.
   long y=-1;                                // Stack point for if loop
   long type;
   char *cmdName = 0, *left = 0, *right = 0;
   Command *cmd;
   long i,j,k;
   
// Initialise stacks - makes it easier for debugging
   for(i = 0; i < MAX_NESTING; i++)
   {
     x[i] = -1;
     elsesfound[i] = 0;
     for(j = 0; j < 20; j++)
       stack[i][j] = 0;
   }
        
// Search for if - elseif - else - endif statements            
   for(i = 0; i < nrCommands; i++)
   {
   	cmd = getCmd(i);
	   type = cmd->splitExpression(&left,&right);
	   if(type == ASSIGNMENT)
	   {
         if(left)  { delete [] left;   left = 0;};
         if(right) { delete [] right; right = 0;};
	      continue;
	   }
	      
	   extractCmdName(left,&cmdName); 
	   
      if(!strcmp(cmdName,"if"))
      {
	      if(y+1 == MAX_NESTING)
	      {
	         ErrorMessage("if-endif statements too deeply");
            ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
	         throw "";       
	      }      
         y++;
         stack[y][0] = i;
         x[y] = 0;
         elsesfound[y] = 0;
      }
      else if(!strcmp(cmdName,"elseif"))
      {
         if(y == -1)
	      {
	         ErrorMessage("missing 'if' statement");
            ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
	         throw "";       
	      }
         if(elsesfound[y] > 0)  
	      {
	         ErrorMessage("'else' before 'elseif'");
            ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
	         throw "";       
	      } 
         j = x[y];
         k = stack[y][j];        // Points to last if or elseif statement
         jumpNr[k] = i;          // Update jump number for this statement so that it points to current location
         stack[y][j+1] = i;      // Store position of new elseif statement
         x[y]++;
      }
      else if(!strcmp(cmdName,"else"))
      {
         if(y == -1)
	      {
	         ErrorMessage("missing 'if' statement");
            ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
	         throw "";       
	      }
         if(elsesfound[y] > 0)  
	      {
	         ErrorMessage("too many 'else' statements");
            ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
	         throw "";       
	      }
         j = x[y];
         k = stack[y][j];        // Points to last if or elseif statement
         jumpNr[k] = i;          // Update jump number for this statement so that it points to current location
         stack[y][j+1] = i;      // Store position of new elseif statement
         x[y]++;  
         elsesfound[y]++;
            
      }
      else if(!strcmp(cmdName,"endif"))
      {
	      if(y == -1)
	      {
	         ErrorMessage("missing if statement");
            ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
	         throw "";       
	      }      
         if(elsesfound[y] == 0) // Last statement was not an else
         {                      // So update jump number
            j = x[y];
            k = stack[y][j];        // Points to last if or elseif statement
            jumpNr[k] = i;          // Update jump number for this statement so that it points to current location
         }

         for(j = 1; j <= x[y]; j++)   // Update all elseif or else statements
         {
            k = stack[y][j]-1;    // Points to line before elseif or else statement
            nextNr[k] = i;        // Update next number for this statement so that it points to current location
         }

      // Search for other commands which should jump to this 'endif' statement
      // these could be exitfor and exitwhile statements when the next or endwhile statements are next
      // to the endif.
 
         for(int q = 0; q < nrCommands; q++) // Loop over all commands
         {
         // Ignore commands if, elseif or else in the current stack entry
            for(j = 0; j <= x[y]; j++)   //  Loop over current if, elseif or else statements
            {
               k = stack[y][j];    // Points to line at start of if, elseif or else statement
               if(k == q)
                  break;
            }
            if(j <= x[y]) // Ignore
               continue;

         // Have found a command q which is not an if, elseif, else statement
         // Now see if the jump nr for this command is to an elseif or else statement
            for(j = 1; j <= x[y]; j++)   // Loop over elseif or else statements
            {
               k = stack[y][j];    // Points to line of elseif or else statement
               if(jumpNr[q] == k && q+1 != k) // Not just the line before
               {
                  jumpNr[q] = i;        // Update next number for this statement so that it points to current location
                  break;
               } 
            }
         }

         y--;                     // Decrement stack pointer
      }
      if(left)  { delete [] left;   left = 0;};
      if(right) { delete [] right; right = 0;};
      if(cmdName)  { delete [] cmdName;   cmdName = 0;};            
      if(y >= 0 && x[y] == MAX_IFELSEIF)
      {
         ErrorMessage("too many elseif statements");
         ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
         throw "";       
      }
   }
   if(y > -1)
   {
      ErrorMessage("missing endif statement\n   in macro '%s'",macroName.Str());
      ReportMacroError("",0,1,0,0); 
      throw "";    
   }
   else if(y < -1)
   {
      ErrorMessage("missing if statement\n   in macro '%s'",macroName.Str());
      ReportMacroError("",0,1,0,0); 
      throw "";     
   }
}

void Macro::TryCatchCheck(void)
{
   long inTry = -1;
   long inCatch = -1;
   long i,j;
   char *cmdName = 0, *left = 0, *right = 0;
   Command *cmd = 0;
   int type;

// Search for while-endwhile statements
   for(i = 0; i < nrCommands; i++)
   {
   	cmd = getCmd(i);
	   type = cmd->splitExpression(&left,&right);
	   if(type == ASSIGNMENT)
	   {
         if(left)  { delete [] left;   left = 0;};
         if(right) { delete [] right; right = 0;};
	      continue;
	   }
	      
	   extractCmdName(left,&cmdName); 
	   
      if(!strcmp(cmdName,"try"))
      {
         if(inTry >= 0)
         {
            if(left)     { delete [] left;      left = 0;};
            if(right)    { delete [] right;     right = 0;};
            if(cmdName)  { delete [] cmdName;   cmdName = 0;};
            ErrorMessage("Can't nest try statements");
            ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
            throw "";
         }
         inTry = i;
      } 
      else if(!strcmp(cmdName,"catch"))
      {
         if(inTry == -1)
         {
            if(left)     { delete [] left;      left = 0;};
            if(right)    { delete [] right;     right = 0;};
            if(cmdName)  { delete [] cmdName;   cmdName = 0;};
            ErrorMessage("Catch must follow try statement");
            ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
            throw "";
         }
      // Set up try numbers for all commands between try and catch
         for(j = i-1; j > inTry; j--)
            tryNr[j] = i;

         inCatch = i;
         inTry = -1;
      }
      else if(!strcmp(cmdName,"endtry"))
      {
         if(inTry >= 0 || inCatch == -1)
         {
            if(left)     { delete [] left;      left = 0;};
            if(right)    { delete [] right;     right = 0;};
            if(cmdName)  { delete [] cmdName;   cmdName = 0;};
            ErrorMessage("endtry must follow try and catch statement");
            ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
            throw "";
         }
         nextNr[inCatch-1] = i;
         inCatch = -1;
         inTry = -1;
      }

    // Added V2.2.4/5 to fix memory leak

      if(left)     { delete [] left;      left = 0;};
      if(right)    { delete [] right;     right = 0;};
      if(cmdName)  { delete [] cmdName;   cmdName = 0;};
 
   }  

   if(inTry >= 0 || inCatch >= 0)
   {
      ErrorMessage("missing endtry statement\n   in macro '%s:%s'",macroName.Str(),procName.Str());
		if(cmd)
		{
			ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
		}
      throw "";   
   }
}


void Macro::WhileCheck(void)
{
   long stack[MAX_NESTING][MAX_BREAKS];
   long p=-1,type,ln;
   long x[MAX_NESTING]; // Records number of break statements for each while loop
   long i;
   char *cmdName = 0, *left = 0, *right = 0;
   Command *cmd = 0;

// Search for while-endwhile statements
   for(i = 0; i < nrCommands; i++)
   {
   	cmd = getCmd(i);
	   type = cmd->splitExpression(&left,&right);
	   if(type == ASSIGNMENT)
	   {
         if(left)  { delete [] left;   left = 0;};
         if(right) { delete [] right; right = 0;};
	      continue;
	   }
	      
	   extractCmdName(left,&cmdName); 
	   
      if(!strcmp(cmdName,"while"))
      {
         stack[++p][0] = i;
         x[p] = 1;
         if(p == MAX_NESTING-1)
         {
            if(left)     { delete [] left;      left = 0;};
            if(right)    { delete [] right;     right = 0;};
            if(cmdName)  { delete [] cmdName;   cmdName = 0;};
            ErrorMessage("While loops too deeply\n   nested in macro '%s'",macroName.Str());
            ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
            throw "";
         }
      }
      else if(!strcmp(cmdName,"exitwhile"))
      {
         if(p == -1)
         {
            ErrorMessage("exitwhile not in while loop\n    in macro '%s'",macroName.Str());
            ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
            throw "";
         }
         if(x[p] == MAX_BREAKS)
         {
            ErrorMessage("Too many breaks in for-loop\n    in macro '%s'",macroName.Str());
            ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
            throw "";
         }
         stack[p][x[p]++] = i;
      }
      else if(!strcmp(cmdName,"endwhile"))
      {
         if(p == -1)
         {
            ErrorMessage("missing while statement\n   in macro '%s'",macroName.Str());
            ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0);  
            throw ""; 
         }
         long k = stack[p][0];
         nextNr[i] = i+1;
         jumpNr[i] = k;
         jumpNr[k] = i+1;

         for(k = 1; k < x[p]; k++)
         {
           ln = stack[p][k];
           jumpNr[ln] = i+1;
         }
         x[p] = 0;
         p--;
      }
      if(left)     { delete [] left;      left = 0;};
      if(right)    { delete [] right;     right = 0;};
      if(cmdName)  { delete [] cmdName;   cmdName = 0;};
   }  

   if(p > -1)
   {
      ErrorMessage("missing endwhile statement\n   in macro '%s:%s'",macroName.Str(),procName.Str());
		if (cmd)
		{
			ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
		}
      throw "";   
   }
   else if(p < -1)
   {
      ErrorMessage("missing while statement\n   in macro '%s:%s'",macroName.Str(),procName.Str());
		if (cmd)
		{
			ReportMacroError(cmd->getCmdName(),i,cmd->getLineNr(),0,0); 
		}
      throw "";  
   }
}
               
/***************************************************************************************
  Search for a procedure in the cache taken from path\macro:name  
  If a macro extension is not passed then try .mac and .pex
  Note that path and macro are modified and so should be long enough
  to hold a complete pathname.
***************************************************************************************/

Variable* Macro::GetProcedure(char *path, char *macro, char *name)
{
   ProcedureInfo *procInfo;
   Variable *var;
   char fullNameMac[MAX_PATH];
   char fullNamePex[MAX_PATH];
   char curPath[MAX_PATH];

// Assume its a macro with extension .mac
   strcpy(fullNameMac,macro);
   AddExtension(fullNameMac,".mac");
   strcpy(fullNamePex,macro);
   AddExtension(fullNamePex,".pex");

// Get the current path in case path not useful
   GetCurrentDirectory(MAX_PATH,curPath);

// First search in the cache for a complete match - same path, same macro and same procedure
	for(var = procList.next; var != NULL; var = var->next)
	{
      procInfo = (ProcedureInfo*)var->GetString(); 

      if(name[0] == '\0') // No procedure name given so search for path\macro
      {
         if(!stricmp(var->GetName(),macro))
         {
            if(path[0] != '\0') // Search for path\macro
            {
               if(ComparePathNames(procInfo->macroPath,path) &&  (!stricmp(procInfo->macroName,fullNameMac)))
               {
                  strcpy(macro,fullNameMac);
                  return(var);
               }

               if(ComparePathNames(procInfo->macroPath,path) &&  (!stricmp(procInfo->macroName,fullNamePex)))
               {
                  strcpy(macro,fullNamePex);
                  return(var);
               }
            }
            else // Search for curpath\macro
            {
               if(ComparePathNames(procInfo->macroPath,curPath) &&  (!stricmp(procInfo->macroName,fullNameMac)))
               {
                  strcpy(macro,fullNameMac);
                  strcpy(path,procInfo->macroPath);
	               return(var);
               }
               if(ComparePathNames(procInfo->macroPath,curPath) &&  (!stricmp(procInfo->macroName,fullNamePex)))
               {
                  strcpy(macro,fullNamePex);
                  strcpy(path,procInfo->macroPath);
	               return(var);
               }
            }
         }
      }
      // Procedure name has been given
      else if(!stricmp(var->GetName(),name))
	   {
         if(path[0] != '\0') // Search for path\macro:name
         {
            if(ComparePathNames(procInfo->macroPath,path) &&  (!stricmp(procInfo->macroName,fullNameMac)))
            {
               strcpy(macro,fullNameMac);
               return(var);
            }

            if(ComparePathNames(procInfo->macroPath,path) &&  (!stricmp(procInfo->macroName,fullNamePex)))
            {
               strcpy(macro,fullNamePex);
               return(var);
            }
         }
         else // Search for curPath\macro:name
         {
            if(ComparePathNames(procInfo->macroPath,curPath) &&  (!stricmp(procInfo->macroName,fullNameMac)))
            {
               strcpy(macro,fullNameMac);
               strcpy(path,procInfo->macroPath);
	            return(var);
            }
            if(ComparePathNames(procInfo->macroPath,curPath) &&  (!stricmp(procInfo->macroName,fullNamePex)))
            {
               strcpy(macro,fullNamePex);
               strcpy(path,procInfo->macroPath);
	            return(var);
            }
         }
	   }
	}

// Can't find macro in cache - perhaps it is in current directory?
  if(path[0] != '\0') // Search for path\macro:name
  {
     if(IsFile(fullNameMac))
        return(NULL);
  }

// The path was no use so return the first match found ignoring the current path
   for(var = procList.next; var != NULL; var = var->next)
	{
      procInfo = (ProcedureInfo*)var->GetString(); 

      if(name[0] == '\0') // Procedure name not given
      {
         if(!stricmp(var->GetName(),macro))
         {
            if(!stricmp(procInfo->macroName,fullNameMac))
            {
               strcpy(macro,fullNameMac);
               strcpy(path,procInfo->macroPath);
	            return(var);
            }
            if(!stricmp(procInfo->macroName,fullNamePex))
            {
               strcpy(macro,fullNamePex);
               strcpy(path,procInfo->macroPath);
	            return(var);
            } 
         }
      }
      
      else if(!stricmp(var->GetName(),name)) // Procedure name given
	   {
         if(!stricmp(procInfo->macroName,fullNameMac))
         {
            strcpy(macro,fullNameMac);
            strcpy(path,procInfo->macroPath);
	         return(var);
         }
         if(!stricmp(procInfo->macroName,fullNamePex))
         {
            strcpy(macro,fullNamePex);
            strcpy(path,procInfo->macroPath);
	         return(var);
         }  
	   }
	}

   return(NULL); // Not found
}
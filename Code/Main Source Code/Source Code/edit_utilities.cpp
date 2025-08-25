#include "stdafx.h"
#include "edit_utilities.h"
#include "htmlhelp.h"
#include "classFunctions.h"
#include "command.h"
#include "command_other.h"
#include "defineWindows.h"
#include "dll.h"
#include "edit_class.h"
#include "edit_files.h"
#include "evaluate.h"
#include "globals.h"
#include "interface.h"
#include "macro_class.h"
#include "message.h"
#include "scanstrings.h"
#include "string_utilities.h"
#include "variablesOther.h"
#include <assert.h>
#include <richedit.h>
#include "memoryLeak.h"

char delimiters[] = " =()\n\r+-&|*/^<>;,\t[]$";

// Arrays to store class name and file associations
CText *gClassName;
CText *gClassMacro;
int gNrClassDeclarations;

/***************************************************************************
                                edit_utilities
                                
   AppendLine ............... Append a line of text to the editor
   CountLinesInEditor ........... Return the number of lines of text in the editor.
   CountLines................ Returns the line number of the nth command.
   GetCommandHelp ........... Display help for the selected command.
   GetCurrentLine ........... Return the line the insertion point is on.
   GetCurrentLineNumber ..... Get the line number of the line the insertion point is on.
   GetLineByNumber .......... Return the text on a given line.
   GetLineByPosition ........ Given a character position return the line of text.
   GetLineStartByPosition ... Get the first character position on a line.                              
   GetText .................. Return the text in the editor.
   InsertLine ............... Insert text on the specified line.
   RemoveCommentsFromText ... Remove all Prospa comments from text.
   ReplaceEOLinText ......... Replace \r\n with EOL markers
   RemoveLine ............... Remove the nth line.
   ReplaceLine .............. Replace the nth line.
   ReplaceLineByPosition .... Replace the line on which the nth character occurs.  
   SelectChar ............... Select one position in the text.
   SelectCurrentLine ........ Select the line which has the insertion point.
   
****************************************************************************/

/****************************************************************************
   Add a line to the end of the text in a standard text editor 
   (note this does not append a line-feed to the end of the new line)
****************************************************************************/

void AppendLine(HWND edWin, char *txt)
{  
// Get number of characters in editor 
   unsigned long n = SendMessage(edWin,WM_GETTEXTLENGTH ,(LPARAM)0,(WPARAM)0);
// Get length of last line of text in editor   
	unsigned long len = SendMessage(edWin,EM_LINELENGTH,(n-1), (LPARAM)0);
// Make this the current line
   SendMessage(edWin,EM_SETSEL ,(LPARAM)n-1,(WPARAM)n-1);
// Allocate memory for text string and an integer to hold length   
   char* line = new char[len+4];
// Get the text 
   GetCurrentLine(edWin,line);
   if(n > 0 && line[len-1] != '\n') // Line ends without a linefeed so add one
   {                                // before adding text 
      SendMessage(edWin,EM_SETSEL,(LPARAM)n,(WPARAM)n);   
      SendMessage(edWin,EM_REPLACESEL,TRUE,(WPARAM)"\r\n");            
      SendMessage(edWin,EM_SETSEL,(LPARAM)n+2,(WPARAM)n+2);
      SendMessage(edWin,EM_REPLACESEL,TRUE,(WPARAM)txt); 
   }
   else // Text ends in '\n' or is empty so just add new string
   {   
      SendMessage(edWin,EM_SETSEL,(LPARAM)n,(WPARAM)n);
      SendMessage(edWin,EM_REPLACESEL,TRUE,(WPARAM)txt); 
   }
   delete [] line;
}

/****************************************************************************
   Add a line to the end of the text in a richedit text editor 
   (note this does not append a line-feed to the end of the new line)
****************************************************************************/

void AppendLineEx(HWND edWin, char *txt)
{ 
   CHARRANGE range;  
// Get number of characters in editor 
   unsigned long n = SendMessage(edWin,WM_GETTEXTLENGTH ,(LPARAM)0,(WPARAM)0);
// Get length of last line of text in editor   
	unsigned long len = SendMessage(edWin,EM_LINELENGTH,(n-1), (LPARAM)0);
// Make this the current line
   range.cpMin = n-1; range.cpMax = n-1;
   SendMessage(edWin,EM_EXSETSEL ,(LPARAM)0,(WPARAM)&range);
// Allocate memory for text string and an integer to hold length   
   char* line = new char[len+4];
// Get the text 
   GetCurrentLineEx(edWin,line);
   if(n > 0 && line[len-1] != '\n') // Line ends without a linefeed so add one
   {                                // before adding text
      range.cpMin = n; range.cpMax = n;   
      SendMessage(edWin,EM_EXSETSEL,(LPARAM)0,(WPARAM)&range);   
      SendMessage(edWin,EM_REPLACESEL,TRUE,(WPARAM)"\r\n"); 
      range.cpMin = n+2; range.cpMax = n+2;             
      SendMessage(edWin,EM_EXSETSEL,(LPARAM)0,(WPARAM)&range);
      SendMessage(edWin,EM_REPLACESEL,TRUE,(WPARAM)txt); 
   }
   else // Text ends in '\n' or is empty so just add new string
   {
      range.cpMin = n; range.cpMax = n;     
      SendMessage(edWin,EM_EXSETSEL,(LPARAM)0,(WPARAM)&range);
      SendMessage(edWin,EM_REPLACESEL,TRUE,(WPARAM)txt); 
   }
   delete [] line;
}


/**************************************************************************************
  Get a line of text given the position of a character (0 based from started of editor)
**************************************************************************************/

char* GetLineByPosition(HWND edWin, long charPos)
{
   char *text;
   long length;
   long lineNr;
      
	lineNr = SendMessage(edWin,EM_EXLINEFROMCHAR,(WPARAM) 0, (LPARAM) charPos);
	length = SendMessage(edWin,EM_LINELENGTH,(WPARAM) charPos, (LPARAM) 0);
   text = new char[length+4];  // Need at least 4 bytes for length of text (type long)
   *(int*)text = length+1;
	SendMessage(edWin,EM_GETLINE,(WPARAM) lineNr, (LPARAM)(LPCSTR)text);
	text[length] = '\0';
   return(text);
}


/**************************************************************************************
  Get a line of text given its number (0 based) - text is allocated and returned
  (remember to deallocate with delete [] when finished.)
**************************************************************************************/

char* GetLineByNumber(HWND edWin,long lineNr)
{
   char *text;
   long length;
   long charPos;
   
	charPos = SendMessage(edWin,EM_LINEINDEX,(WPARAM) lineNr, (LPARAM) 0);
	length = SendMessage(edWin,EM_LINELENGTH,(WPARAM) charPos, (LPARAM) 0);
   text = new char[length+4];   // Need at least 4 bytes for length of text (type long)
   *(int*)text = length+1;  
	SendMessage(edWin,EM_GETLINE,(WPARAM) lineNr, (LPARAM)(LPCSTR)text);
	text[length] = '\0';	
   return(text);
}

/**************************************************************************************
  Get a line of text given its number (0 based) - text is passed as argument.
  (text string must be long enough to take current line).
**************************************************************************************/

void GetLineByNumber(HWND edWin, long lineNr, char *text)
{
   long length;
   long charPos;
   
	charPos = SendMessage(edWin,EM_LINEINDEX,(WPARAM) lineNr, (LPARAM) 0);
	length = SendMessage(edWin,EM_LINELENGTH,(WPARAM) charPos, (LPARAM) 0);
   *(int*)text = length+1;  
	SendMessage(edWin,EM_GETLINE,(WPARAM) lineNr, (LPARAM)(LPCSTR)text);
	text[length] = '\0';	
}

/***********************************************************************************************
  Get the position of the first character on a line as specified by another character position
***********************************************************************************************/

long GetLineStartByPosition(HWND edWin,long charPos)
{
   long lineNr,startPos;

	lineNr = SendMessage(edWin,EM_EXLINEFROMCHAR,(WPARAM) 0, (LPARAM) charPos);
	startPos = SendMessage(edWin,EM_LINEINDEX,(WPARAM) lineNr, (LPARAM) 0);

   return(startPos);
}

/***********************************************************************************************
  Strip text starting with '#' from textIn and place in textOut
***********************************************************************************************/
         
short RemoveCommentsFromText(char *textIn, char **textOut)
{
   long i,j,k;
   long len = strlen(textIn);
   char *text;
   
   text = *textOut;
   
   if(!(text = new char[strlen(textIn)+1]))
   {
       ErrorMessage("out of memory during 'RemoveCommentsFromText'");
       return(ERR);
   }

   strcpy(text,textIn);
   for(i = 0, j = 0; i < len; i++)
   {
      if(textIn[i] == '#')
      {
         for(k = i; k < len; k++) // Search for end of line
         {
            if(textIn[k] == '\r' && textIn[k+1] == '\n')
            {
               i = k;
               break;
            }
         }
      }
      text[j++] = textIn[i];
   }
   text[j] = '\0';
   *textOut = text;
   return(OK);
}


/***************************************************************************************
               Removes \r\n from the text but replaces with EOL markers                * 
          If the line ends with a continuation symbol '@' then ignore EOL              *                            
***************************************************************************************/

void ReplaceEOLinText(char* txt)
{
   long i;
   bool cont = false;
   long len = strlen(txt);

   for(i = 0; i < len; i++)
   {
      if(txt[i] == '@') // Continuation 
      {
         txt[i] = EOL_CONT;
         cont = true;
         continue;
      }
      
      if(txt[i] == '\r' && txt[i+1] == '\n') 
      {
         if(cont == true) // Continuation so ignore
         {
            txt[i] = ' ';
            txt[i+1] = ' ';
            cont = false;
         }
         else
         {
            txt[i] = EOL;  // Not a continuation so note it
            txt[i+1] = ' ';
         }
         i++;
      }
   }
}

/***************************************************************************************
  Returns the line number of of the nth command                                      *                                     
***************************************************************************************/

short CountLines(char *txt, short cmdNr)
{
   int i,len;
   int pos = 0;
   int cnt = 0;
   bool inString = false;
   short inRealArray = 0;
   short inCompArray = 0;
   short bracketCnt = 0;
   char c;
   short lineNr = 0;
      
   len = strlen(txt);
   
// Keep searching for the next valid command until end of string found
  
   do
   {
   // Search for the start of the command (first non-semicolon)
   
	   for(i = pos; i < len; i++)  
	   {
         c = txt[i];
	   	
	   	if(c == EOL || c == EOL_CONT)
	   	   lineNr++;
	   	            
	      if(c == ';' || c == EOL) // Ignore empty commands
	         continue;
	      else if(c == ' ' || c == '\t') // Ignore spaces or tabs
	         continue;	         
	      break;
       }
       pos = i;
       if(i == len)
          return(cnt);

   // Is this the right one?
   
	   if(cnt+1 == cmdNr)
	      return(lineNr+1);  
	              
	// Search for the end of the command (a non-bracketed semicolon)
	   
	   for(i = pos; i < len; i++)
	   {
         c = txt[i];
         
	   	if(c == EOL || c == EOL_CONT)
	   	   lineNr++;         
         
    // If in an argument list ignore everything ***************
	         
	      if(c == '(') 
           bracketCnt++;
	      else if(c == ')') 
           bracketCnt--;
	      if(bracketCnt > 0) continue;

    // If in a string ignore everything ***********************
	      	                     
	   	if(c == QUOTE && !IsEscapedQuote(txt,i,len))
	      {
	         if(inString)
	           inString = false;
	         else
	           inString = true;
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
	      if(inRealArray > 0 || inCompArray > 0) continue;

    // If end of command or line is found start looking for next command ***
    
	      if(c == ';' || c == EOL)
	         break;	
	   }
	   pos = i+1;
	   cnt++;

    }
    while(pos <= len);       
    return(lineNr);
 }

/***************************************************************************************
   Extract the selected text and see if is a command. If it is call the command help file.
***************************************************************************************/

void GetCommandHelp(Interface *itfc, HWND edWin)
{
   long startSel;
   long offset;
   char *buf,*cmd;
   CText txt;
   CHARRANGE range;
         
// Get selection
	SendMessage(edWin,EM_EXGETSEL,(WPARAM) 0, (LPARAM) &range);
   startSel = range.cpMin; 
   		
// Get text on this line
   buf = GetLineByPosition(edWin,startSel);
   long sz = strlen(buf);
   offset = GetLineStartByPosition(edWin,startSel);
   startSel -= offset;

// Extract command at point selected by user
   long start,end;
   if(!(cmd = GetWordAtChar(buf,startSel,start,end)))
   {
      if(buf) delete [] buf;
      return;
   }

   if(start > 1)
   {
   // See if it is a class member - if so display generic class help 
      for(int i = 2; i < start-1; i++)
      {
         if(buf[i] == '-' && buf[i+1] == '>')
         {
            if(GetClassCommandVariousness(cmd))
				{
					txt.Format("Class type is ambiguous - displaying '%s' version",GetClassCommandHelpType(cmd));
					MessageDialog(prospaWin,MB_ICONWARNING,"Warning",txt.Str());
				}
				if(GetClassCommandHelpType(cmd))
				{
					txt.Format("\"Classes\\Windows\",\"%s.htm\"",GetClassCommandHelpType(cmd));
	            OpenHelpFile(itfc,txt.Str());
				}
            delete [] cmd;	   
            delete [] buf; 
            return;
         }
      }
      for(int i = end+1; i < sz; i++)
      {
         if(buf[i] == ',' || buf[i] == '(') // -> must be directly after name
            break;
         if(buf[i] == '-' && buf[i+1] == '>')
         {
	         OpenHelpFile(itfc,"\"Classes\\Windows\",\"Introduction.htm\"");
            delete [] cmd;	   
            delete [] buf; 
            return;
         }
      }
   }

// See if it is a command - if it is display help  
   ToLowerCase(cmd); 
   txt.Format("\"%s\"",cmd);
   OpenHelpFile(itfc,txt.Str());

// Tidy up
	delete [] cmd;	   
	delete [] buf; 	
}


/***************************************************************************************
   Return text from editor (memory allocated)
***************************************************************************************/

char* GetText(HWND edWin)
{
	char *text;
	long length;
		
	length = SendMessage(edWin,WM_GETTEXTLENGTH,0,0) + 1;
	text = new char[length];
	SendMessage(edWin,WM_GETTEXT,(WPARAM)length,(LPARAM)(text));
	return(text);
}

/***************************************************************************************
  Select the whole of the line which has cursor (rich texteditor)
***************************************************************************************/
  				  
long SelectCurrentLineEx(HWND edWin)
{
   CHARRANGE range;
	SendMessage(edWin,EM_EXGETSEL,(WPARAM)0, (LPARAM)&range);
   long startSel = range.cpMin; 
	long line = SendMessage(edWin,EM_EXLINEFROMCHAR,(WPARAM)0, (LPARAM)startSel);
	long start = SendMessage(edWin,EM_LINEINDEX,(WPARAM)line, (LPARAM)0);
	long len = SendMessage(edWin,EM_LINELENGTH,(WPARAM)start, (LPARAM)0);
	if(start-2 > 0)
	{
	   range.cpMin = start-2;
	   range.cpMax = start+len;
	   SendMessage(edWin,EM_EXSETSEL,(WPARAM)0, (LPARAM)&range);
	}
	else
	{
	   range.cpMin = 0;
	   range.cpMax = start+len;	
	   SendMessage(edWin,EM_EXSETSEL,(WPARAM)0, (LPARAM)&range);
	}
	return(line);
}

/***************************************************************************************
  Select the whole of the line which has cursor (normal editor)
***************************************************************************************/
  				  
long SelectCurrentLine(HWND edWin)
{
   long startSel,endSel;
	SendMessage(edWin,EM_GETSEL,(WPARAM)&startSel, (LPARAM)&endSel);
	long line = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)startSel, (LPARAM)0);
	long start = SendMessage(edWin,EM_LINEINDEX,(WPARAM)line, (LPARAM)0);
	long len = SendMessage(edWin,EM_LINELENGTH,(WPARAM)start, (LPARAM)0);
	if(start-2 > 0)
	{
	   SendMessage(edWin,EM_SETSEL,(WPARAM)start-2, (LPARAM)start+len);
	}
	else
	{
	   SendMessage(edWin,EM_SETSEL,(WPARAM)0, (LPARAM)start+len);
	}
	return(line);
}

/***************************************************************************************
  Select the current line from location begin to end (inclusive)
***************************************************************************************/
  	
long SelectPartCurrentLine(HWND edWin, short begin, short end)
{
   long startSel,endSel;
	SendMessage(edWin,EM_GETSEL,(WPARAM)&startSel, (LPARAM)&endSel);
	long line = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)startSel, (LPARAM)0);
	long lineStart = SendMessage(edWin,EM_LINEINDEX,(WPARAM)line, (LPARAM)0);
	long len = SendMessage(edWin,EM_LINELENGTH,(WPARAM)lineStart, (LPARAM)0);
   if(end == -1)
      end = len;
   if(begin == -1)
      begin = 0;
	SendMessage(edWin,EM_SETSEL,(WPARAM)lineStart+begin, (LPARAM)lineStart+end);

	return(line);
}

/***************************************************************************************
  Return the number of lines in the editor
***************************************************************************************/
   
long CountLinesInEditor(HWND edWin)
{
   return(SendMessage(edWin,EM_GETLINECOUNT,(WPARAM)0, (LPARAM)0));
}

/***************************************************************************************
   Select the nth line in the text
***************************************************************************************/

void SelectLine(HWND edWin, long n)
{
   CHARRANGE range;
	long start = SendMessage(edWin,EM_LINEINDEX,(WPARAM)n, (LPARAM)0);
	long len = SendMessage(edWin,EM_LINELENGTH,(WPARAM)start, (LPARAM)0);
	range.cpMin = start; range.cpMax = start+len+1;	
	SendMessage(edWin,EM_EXSETSEL,(WPARAM)0, (LPARAM)&range); // (Include /r)
}

/***************************************************************************************
  Select one position in the text using the character location relative to start
***************************************************************************************/

void SelectChar(HWND edWin, long ch)
{
   SendMessage(edWin, EM_SETSEL, (WPARAM)ch, (LPARAM)ch);
}

/***************************************************************************************
  Select one position in the text using the line number and character position on line
***************************************************************************************/
  
void SelectChar(HWND edWin, long n, long ch)
{
	long start = SendMessage(edWin,EM_LINEINDEX,(WPARAM)n, (LPARAM)0);	
	SendMessage(edWin,EM_SETSEL,(WPARAM)start+ch, (LPARAM)start+ch);
}

/***************************************************************************************
  Select one position in the text using the line number and character position on line
***************************************************************************************/
  
void SelectCharEx(HWND edWin, long n, long ch)
{
   CHARRANGE range;
	long start = SendMessage(edWin,EM_LINEINDEX,(WPARAM)n, (LPARAM)0);
	range.cpMin = start+ch; range.cpMax = start+ch;		
	SendMessage(edWin,EM_EXSETSEL,(WPARAM)0, (LPARAM)&range);
}

/***************************************************************************************
   Get the line the cursor is on
***************************************************************************************/

long GetCurrentLineEx(HWND edWin, char *txt)
{
   long startSel;
   CHARRANGE range;   
	SendMessage(edWin,EM_EXGETSEL,(WPARAM)0, (LPARAM)&range);
   startSel = range.cpMin;
	long line = SendMessage(edWin,EM_EXLINEFROMCHAR,(WPARAM)0, (LPARAM)startSel);
	long len = SendMessage(edWin,EM_LINELENGTH,(WPARAM)startSel, (LPARAM)0);	
   *(int*)txt = len+1;	
	SendMessage(edWin,EM_GETLINE,(WPARAM)line, (LPARAM)(LPCSTR)txt);
	txt[len] = '\0';
	return(line);
}

/***************************************************************************************
   Get the line the cursor is on
***************************************************************************************/

long GetCurrentLine(HWND edWin, char *txt)
{
   long startSel,endSel;
	SendMessage(edWin,EM_GETSEL,(WPARAM)&startSel, (LPARAM)&endSel);

	long line = SendMessage(edWin,EM_LINEFROMCHAR,(WPARAM)startSel, (LPARAM)0);
	long len = SendMessage(edWin,EM_LINELENGTH,(WPARAM)startSel, (LPARAM)0);	
   *(int*)txt = len+1;	
	SendMessage(edWin,EM_GETLINE,(WPARAM)line, (LPARAM)(LPCSTR)txt);
	txt[len] = '\0';
	return(line);
}

/***************************************************************************************
  Get the line number the cursor is on
***************************************************************************************/

long GetCurrentLineNumber(HWND edWin)
{
   CHARRANGE range;   
	SendMessage(edWin,EM_EXGETSEL,(WPARAM)0, (LPARAM)&range);
   long startSel = range.cpMin; 
	long line = SendMessage(edWin,EM_EXLINEFROMCHAR,(WPARAM)0, (LPARAM)startSel);
	return(line);
}

/***************************************************************************************
 Insert txt on line n of a rich editor
***************************************************************************************/
 
void InsertLineEx(HWND edWin, char *txt, long n)
{
   CHARRANGE range; 
	long start = SendMessage(edWin,EM_LINEINDEX,(WPARAM)n, (LPARAM)0);
	range.cpMin = start; range.cpMax = start;		
	SendMessage(edWin,EM_EXSETSEL,(WPARAM)0, (LPARAM)&range);
	SendMessage(edWin,EM_REPLACESEL,(WPARAM)TRUE, (LPARAM)txt);
	range.cpMin = start+strlen(txt); range.cpMax = range.cpMin;		
	SendMessage(edWin,EM_EXSETSEL,(WPARAM)0, (LPARAM)&range);
	SendMessage(edWin,EM_REPLACESEL,(WPARAM)TRUE, (LPARAM)"\r\n");	
}

/***************************************************************************************
 Insert txt on line n of a normal editor
***************************************************************************************/
 
void InsertLine(HWND edWin, char *txt, long n)
{
	long start = SendMessage(edWin,EM_LINEINDEX,(WPARAM)n, (LPARAM)0);	
	SendMessage(edWin,EM_SETSEL,(WPARAM)start, (LPARAM)start);
	SendMessage(edWin,EM_REPLACESEL,(WPARAM)TRUE, (LPARAM)txt);
	SendMessage(edWin,EM_SETSEL,(WPARAM)start+strlen(txt), (LPARAM)start+strlen(txt));
	SendMessage(edWin,EM_REPLACESEL,(WPARAM)TRUE, (LPARAM)"\r\n");	
}

/**************************************************************************************
   Remove line n including appended \r\n (except for last line which remove \r\n 
   from previous line))
   (Note: only use in standard text editor not richtext since this only used \r)
**************************************************************************************/
   
void RemoveLine(HWND edWin, long n)
{
   long lastLine = SendMessage(edWin,EM_GETLINECOUNT,(WPARAM)0, (LPARAM)0) - 1;
	long start = SendMessage(edWin,EM_LINEINDEX,(WPARAM)n, (LPARAM)0);
	long len = SendMessage(edWin,EM_LINELENGTH,(WPARAM)start, (LPARAM)0);
	if(n == lastLine)
	{	
	   SendMessage(edWin,EM_SETSEL,(WPARAM)start-2, (LPARAM)start+len);
	}
	else
	{
	   SendMessage(edWin,EM_SETSEL,(WPARAM)start, (LPARAM)start+len+2);
	}
	
   SendMessage(edWin,EM_REPLACESEL,0,(WPARAM)"");
}   

/**************************************************************************************
   Replace text on line 'n' with 'txt' leaving insertion point at start of line
**************************************************************************************/

void ReplaceLine(HWND edWin, long n, char *txt)
{
   long start;
   long end;
   CHARRANGE range; 
      
	start = SendMessage(edWin,EM_LINEINDEX,(WPARAM) n, (LPARAM) 0);
	end = SendMessage(edWin,EM_LINEINDEX,(WPARAM) n+1, (LPARAM) 0)-1;
   range.cpMin = start; range.cpMax = end;		
   SendMessage(edWin,EM_EXSETSEL,(LPARAM)0,(WPARAM)&range);
   SendMessage(edWin,EM_REPLACESEL,0,(WPARAM)txt); 
   range.cpMin = start; range.cpMax = start;		     
   SendMessage(edWin,EM_EXSETSEL,(LPARAM)0,(WPARAM)&range);
}

/**************************************************************************************
   Replace text on line indexed character at position by 'n' with 'txt' 
   leaving insertion point at start of line.
***************************************************************************************/
 
void ReplaceLineByPosition(HWND edWin, long pos, char *txt)
{
   CHARRANGE range; 
   
	long n = SendMessage(edWin,EM_EXLINEFROMCHAR,(WPARAM) 0, (LPARAM) pos); 
	long start = SendMessage(edWin,EM_LINEINDEX,(WPARAM) n, (LPARAM) 0);
	long end = SendMessage(edWin,EM_LINEINDEX,(WPARAM) n+1, (LPARAM) 0)-2;
   range.cpMin = start; range.cpMax = end;	
   SendMessage(edWin,EM_EXSETSEL,(LPARAM)0,(WPARAM)&range);
   SendMessage(edWin,EM_REPLACESEL,0,(WPARAM)txt); 
   range.cpMin = start; range.cpMax = start;     
   SendMessage(edWin,EM_EXSETSEL,(LPARAM)0,(WPARAM)&range);
}

/**************************************************************************************
   Set text selection in a rich editor 
***************************************************************************************/

void SetEditSelection(HWND hWnd, long start, long end)
{
   CHARRANGE range;
   assert(start >= -1 && end >= -1);   
   range.cpMin = start;
   range.cpMax = end; 
 	SendMessage(hWnd, EM_EXSETSEL,(WPARAM)0, (LPARAM)&range);
}

/**************************************************************************************
   Get current text selection in a rich editor 
***************************************************************************************/

void GetEditSelection(HWND hWnd, long &start, long &end)
{
   CHARRANGE range; 
 	SendMessage(hWnd, EM_EXGETSEL,(WPARAM)0, (LPARAM)&range);
 	start = range.cpMin;
 	end = range.cpMax;
}

/**************************************************************************************
   Replace the text between character positions start and end with string 
***************************************************************************************/

void ReplaceEditText(HWND hWnd, long start, long end, char* string)
{
   SetEditSelection(hWnd,start,end);
	SendMessage(hWnd,EM_REPLACESEL,(WPARAM)false,(LPARAM)string);
}

/**************************************************************************************
   Replace the text between character positions start and end with string 

***************************************************************************************/

void CutEditSelection(HWND hWnd)
{
	SendMessage(hWnd,WM_CUT,(WPARAM)0,(LPARAM)0);
}

/**************************************************************************************
   Colour the text in the current line 

   Note: there is a problem with this function. We can't determine when a scrollbar
   is visible or not since GetScrollPos just returns the last visible position of the 
   scrollbar. I am getting around this by seeing if the function changes the scroll
   position - if it doesn't we don't need to do anything.

***************************************************************************************/

void UpdateLineColor(EditRegion *er, HWND hWnd, long lineNr)
{ 
	long startSel,endSel; 

// Hide all drawing
	SendMessage(hWnd, WM_SETREDRAW, false, 0);  
// Record scroll bar position (since selections will change it)
   int hpos = GetScrollPos(hWnd,SB_HORZ);
// Record current selection range  
	GetEditSelection(hWnd,startSel,endSel);
// Apply syntax coloring to line (first to black and then to syntax color)
   SelectLine(hWnd,lineNr);
   ColorSelectedText(hWnd,RGB(0,0,0));
	UpdateLineColorCore(er,hWnd,lineNr);
// Restore selection
   SetEditSelection(hWnd,startSel,endSel);     
// Return scroll bar and display to original position if scrollbar has moved
   if(GetScrollPos(hWnd,SB_HORZ) != hpos)
   {
      DWORD pos = (hpos<<16) + 5;
	   SendMessage(hWnd,WM_HSCROLL,(WPARAM)pos,(LPARAM)0);
      SetScrollPos(hWnd,SB_HORZ,hpos,TRUE);  
   }
// Show all drawing
	SendMessage(hWnd, WM_SETREDRAW, true, 0); 
	MyInvalidateRect(hWnd,0,true);				
}

/********************************************
* Set the color for the selected text
********************************************/

void ColorSelectedText(HWND hWnd, COLORREF color)
{
	CHARFORMAT chFmt;
   chFmt.cbSize = sizeof(CHARFORMAT);
	chFmt.dwMask = CFM_COLOR;   
 	SendMessage(hWnd,EM_GETCHARFORMAT,(WPARAM)0,(LPARAM) (CHARFORMAT FAR *) &chFmt);

	chFmt.dwMask = CFM_COLOR;
	chFmt.dwEffects = 0;
	chFmt.crTextColor = color; // Expression color

// This command is quite slow but all we've got!
   SendMessage(hWnd,EM_SETCHARFORMAT,(WPARAM)SCF_SELECTION	,(LPARAM) (CHARFORMAT FAR *) &chFmt);
}

/**********************************************
* Set the color for all text in the editor
**********************************************/

 void SetColor(HWND hWnd, COLORREF color)
 {
	CHARFORMAT chFmt;
   chFmt.cbSize = sizeof(CHARFORMAT);
	chFmt.dwMask = CFM_COLOR;   
 	SendMessage(hWnd,EM_GETCHARFORMAT,(WPARAM)0,(LPARAM) (CHARFORMAT FAR *) &chFmt);
 
   chFmt.dwMask = CFM_COLOR;
	chFmt.dwEffects = 0;
	chFmt.crTextColor = color;
   SendMessage(hWnd,EM_SETCHARFORMAT,(WPARAM)SCF_ALL	,(LPARAM) (CHARFORMAT FAR *) &chFmt);
}


/*************************************************************
* Apply syntax coloring to line 'lineNr'.
* This command starts by looking for a comment and then
* changing its color first. Then it searches the rest of the
* line for keywords - i.e. those found in the command list,
* user defined procedures or comments. Note that the other
* words will take the default color, so it is important to
* color the text back (e.g.) before running this command.
**************************************************************/



void UpdateLineColorCore(EditRegion *er, HWND hWnd, long lineNr)
{
	long lineLength,lineStart,endLine;
	COLORREF color;
   bool inString = false;

   if(!er)
      return;

   short mode = er->syntaxColoringStyle;

// Find line extent
	lineStart = SendMessage(hWnd,EM_LINEINDEX,(WPARAM)lineNr,(LPARAM)0);
	lineLength = SendMessage(hWnd,EM_LINELENGTH,(WPARAM)lineStart,(LPARAM)0);

   if(lineLength == 0)
      return;

// Extract the line
   char *buf = new char[lineLength+4];
   char *keyWord = new char[lineLength+4];
	*(int*)buf = lineLength+4; // buffer size

	SendMessage(hWnd,EM_GETLINE,(WPARAM)lineNr,(LPARAM)(LPCSTR)buf);

// Check for comments - ensure that rest of code doesn't look at this part of line
   endLine = lineLength;
	for(long i = 0; i < lineLength; i++)
	{
      if(i == 0 || i == lineLength-1)
      {
         if(buf[i] == '"') 
            inString = !inString;
      }
      else
      {
         if(buf[i] == '"' && buf[i-1] != '\\')
            inString = !inString;
      }

	   if(mode == MACRO_STYLE && buf[i] == '#' && !inString)
	   {
         SetEditSelection(hWnd,lineStart+i,lineStart+lineLength); 
	      ColorSelectedText(hWnd,RGB(170,0,0));
	      endLine = i;
	      break;	 
	   }

	   if(mode == ASM_STYLE && buf[i] == ';' && !inString)
	   {
         SetEditSelection(hWnd,lineStart+i,lineStart+lineLength); 
	      ColorSelectedText(hWnd,RGB(170,0,0));
	      endLine = i;
	      break;	 
	   }

      if (mode == PAR_STYLE && buf[i] == '#' && !inString)
      {
         SetEditSelection(hWnd, lineStart + i, lineStart + lineLength);
         ColorSelectedText(hWnd, RGB(170, 0, 0));
         endLine = i;
         break;
      }

	} 	
	
   if(mode == MACRO_STYLE)
   {
	   long beginNoKey = 0;
	   long endNoKey = 0;
	   long startKey = 0;
	   long endKey = 0;
	   short type;
 	
   // Scan rest of line for keywords **************************  
	   for(long i = 0; i < endLine;)
	   {
	   // Get next keyword
	      beginNoKey = i;
	      GetNextWord2(i,buf,keyWord,endLine);
	      endNoKey = i-strlen(keyWord)-1;
	      startKey = endNoKey+1;
	      endKey = i;	   

			if(endNoKey >= 1 && buf[endNoKey-1] == '-' && buf[endNoKey] == '>')
			{
				if(IsProcessClassFunction(keyWord))
				{
				   color = RGB(240,64,0);

				// Colour before keyword	
					if(endNoKey > 	beginNoKey)
					{               	
						SetEditSelection(hWnd,lineStart+beginNoKey,lineStart+endNoKey);
						ColorSelectedText(hWnd,RGB(0,0,0));
					}
				// Colour keyword	
					if(i <= endLine && endKey > startKey)
					{        
						SetEditSelection(hWnd,lineStart+startKey,lineStart+endKey);
						ColorSelectedText(hWnd,color);
					}
				}
				else if(buf[i] == '(')
				{
					SetEditSelection(hWnd,lineStart+startKey,lineStart+endKey);
					ColorSelectedText(hWnd,RGB(0,128,128));
				}
			}
			else
			{
				color = RGB(0,0,0);
	         
				if(IsACommand(keyWord))
				{	      
					type = GetCommandType(keyWord);
					if(type == CONTROL_COMMAND || type == NOP_CONTROL_CMD)
						color = RGB(0,0,255);
					else // if(type | GENERAL_COMMAND)
						color = RGB(0,128,0);

				// Colour before keyword	
					if(endNoKey > 	beginNoKey)
					{               	
						SetEditSelection(hWnd,lineStart+beginNoKey,lineStart+endNoKey);
						ColorSelectedText(hWnd,RGB(0,0,0));
					}
				// Colour keyword	
					if(i <= endLine && endKey > startKey)
					{        
						SetEditSelection(hWnd,lineStart+startKey,lineStart+endKey);
						ColorSelectedText(hWnd,color);
					}	
				}
				else if(IsADLLCommand(keyWord))
				{
				// Colour before keyword	
					if(endNoKey > 	beginNoKey)
					{               	
						SetEditSelection(hWnd,lineStart+beginNoKey,lineStart+endNoKey);
						ColorSelectedText(hWnd,RGB(0,0,0));
					}
				// Colour keyword	
					if(i <= endLine && endKey > startKey)
					{        
						SetEditSelection(hWnd,lineStart+startKey,lineStart+endKey);
						ColorSelectedText(hWnd,RGB(120,0,120));
					}	
				}
				else
				{
					 short type;
					 float result;
					 short p = i;

				// If the next word is a string terminated by a '(' then assume its a macro call
				// However exception is xxx:(yyy)
					 type = GetDataType(keyWord,&result);
					 SkipWhiteSpace(buf,p);

					 if((type == UNQUOTED_STRING || type == MATRIX2D) && keyWord[strlen(keyWord)-1] != ':' && buf[p] == '(')
 						color = RGB(0,128,128);
				// If its quoted its a string
					else if(isStr(keyWord))
						color = RGB(128,128,128);

				// Color the word
					if(color != 0)
					{          
					// Colour before keyword	
						if(endNoKey > 	beginNoKey)
						{           
							SetEditSelection(hWnd,lineStart+beginNoKey,lineStart+endNoKey);
							ColorSelectedText(hWnd,RGB(0,0,0));
						}
					// Colour keyword	
						if(i <= endLine && endKey > startKey)
						{   
							SetEditSelection(hWnd,lineStart+startKey,lineStart+endKey);
							ColorSelectedText(hWnd,color);
						}
					}
				} 
			}
	   }	
   }
	delete [] keyWord;	
	delete [] buf;
}	

// Modification 12/7/07 to account for escaped quotes in strings

short GetNextWord2(long &pos, char *text, char *command, long endLine)
{
   bool quote_found = false;
   long i,j;
   long lend = strlen(delimiters);

// Find non-delimiter **********************
  for(i = pos; i < endLine; i++)
  {
     for(j = 0; j < lend; j++)
     {
        if(text[i] == delimiters[j])
           break;
     }
     if(j == lend) // Not a delimiter
        break;
  }
  pos = i;

// Search for end of next word **************
   for(i = pos; i < endLine; i++)
   {
      command[i-pos] = text[i];

      if(i == 0 && command[i-pos] == '"' && !quote_found)
      {
         quote_found = true;
         continue;
      }

      if(i > 0 && command[i-pos] == '"' && command[i-pos-1] != '\\' && !quote_found)
      {
         quote_found = true;
         continue;
      }

      if(i > 0 && command[i-pos] == '"' && command[i-pos-1] != '\\' && quote_found)
      {
         quote_found = false;
         continue;
      }

      if(quote_found) continue;

      for(j = 0; j < lend; j++)
      {
         if(text[i] == delimiters[j])
            break;
      }
      if(j < lend) break;

   }
   command[i-pos] = '\0';
   pos = i;
   return(OK);
}

/****************************************************************************************
  Find the key word under the current selection in edit window 'win'
  The delimiter string lists those characters to delimit words.
  Also returns the extend of the word.
  Note the returned string must be freed with delete [] at some stage.
*****************************************************************************************/

char* ExpandToFullWord(HWND win, char* leftDelimiter, char* rightDelimiter, long &wordStart, long &wordEnd, bool &classCmd, short &userClassCmd)
{
   char *name;
   char *buf;
   long selStart,selEnd;
   long lineStart;
   long szLeftDel;
   long szRightDel;
   long i,j;
	bool isBracket = false;

// If user has selected something return this 
   selStart = wordStart;
   selEnd = wordEnd;
	if(selEnd > selStart)
	{
	   name = new char[selEnd-selStart+2];
	   SendMessage(win,EM_GETSELTEXT,(WPARAM)0,(LPARAM)name);
	   wordStart = selStart;
	   wordEnd = selEnd-1;
	   return(name);
	}
	
// Extract line containing selected character
   lineStart = GetLineStartByPosition(win,selStart);
   buf = GetLineByPosition(win, selStart);
   int szLine = strlen(buf);
   int pos = selStart - lineStart;
   szLeftDel  = strlen(leftDelimiter);
   szRightDel  = strlen(rightDelimiter);
   
// Find start of word
   for(i = pos; i >= 0; i--)
   {
      for(j = 0; j < szLeftDel; j++)
      {
         if(buf[i] == leftDelimiter[j])
            break;
      }
      if(j < szLeftDel) break;
   }
   wordStart = i+1;

// Find end of word
   for(i = wordStart; i < szLine; i++)
   {
      for(j = 0; j < szRightDel; j++)
      {
         if(buf[i] == rightDelimiter[j])
			{
				if(rightDelimiter[j] == '(')
					isBracket = true;
            break;
			}
      }
      if(j < szRightDel) break;
   }
   wordEnd = i-1;

// User has selected a delimiter
   if(wordEnd <= wordStart)
      wordEnd = wordStart;

// Check to see if this is class command
	classCmd = false;
	if(wordStart >= 2 && wordEnd >= wordStart)
	{
 		if(buf[wordStart-2] == '-' && buf[wordStart-1] == '>')
		{
			classCmd = true;
		}
	}
// Extract the class name
	if(classCmd == true && wordStart >= 3)
	{
		for(i = wordStart-3; i >= 0 ; i--)
		{
			for(j = 0; j < szLeftDel; j++)
			{
				if(buf[i] == leftDelimiter[j])
					break;
			}
         if(j < szLeftDel) break;
		}
		wordStart = i+1;
		if(isBracket) 
			userClassCmd = 2;
		else
			userClassCmd = 1;
	}
// Extract word from line
   buf[wordEnd+1] = '\0';
   name = new char[wordEnd-wordStart+2];
   strcpy(name,&buf[wordStart]);
   delete [] buf; 
   
   wordStart += lineStart;
   wordEnd += lineStart;
   return(name);
}

/****************************************************************************************
  Find the key word under the current selection in edit window 'win'
  The delimiter string lists those characters to delimit words.
  Also returns the extend of the word.
  Note the returned string must be freed with delete [] at some stage.
*****************************************************************************************/

char* ExpandToFullWord(HWND win, char* leftDelimiter, char* rightDelimiter, long &wordStart, long &wordEnd, bool &classCmd, short &userClassCmd, bool &funcCmd)
{
   char *name;
   char *buf;
   long selStart,selEnd;
   long lineStart;
   long szLeftDel;
   long szRightDel;
   long i,j;
	bool isBracket = false;

// If user has selected something return this 
   selStart = wordStart;
   selEnd = wordEnd;
	if(selEnd > selStart)
	{
	   name = new char[selEnd-selStart+2];
	   SendMessage(win,EM_GETSELTEXT,(WPARAM)0,(LPARAM)name);
	   wordStart = selStart;
	   wordEnd = selEnd-1;
	   return(name);
	}
	
// Extract line containing selected character
   lineStart = GetLineStartByPosition(win,selStart);
   buf = GetLineByPosition(win, selStart);
   int szLine = strlen(buf);
   int pos = selStart - lineStart;
   szLeftDel  = strlen(leftDelimiter);
   szRightDel  = strlen(rightDelimiter);
   
// Find start of word
   for(i = pos; i >= 0; i--)
   {
      for(j = 0; j < szLeftDel; j++)
      {
         if(buf[i] == leftDelimiter[j])
            break;
      }
      if(j < szLeftDel) break;
   }
   wordStart = i+1;

// Find end of word
   for(i = wordStart; i < szLine; i++)
   {
      for(j = 0; j < szRightDel; j++)
      {
         if(buf[i] == rightDelimiter[j])
			{
				if(rightDelimiter[j] == '(')
					isBracket = true;
            break;
			}
      }
      if(j < szRightDel) break;
   }
   wordEnd = i-1;

// User has selected a delimiter
   if(wordEnd <= wordStart)
      wordEnd = wordStart;

// Check to see if this is class command
	classCmd = false;
	if(wordStart >= 2 && wordEnd >= wordStart)
	{
 		if(buf[wordStart-2] == '-' && buf[wordStart-1] == '>')
		{
			classCmd = true;
		}
	}
// Extract the class name
	if(classCmd == true && wordStart >= 6)
	{
		for(i = wordStart-3; i >= 0 ; i--)
		{
			for(j = 0; j < szLeftDel; j++)
			{
				if(buf[i] == leftDelimiter[j])
					break;
			}
         if(j < szLeftDel) break;
		}
		wordStart = i+1;
		if(isBracket) 
			userClassCmd = 2;
		else
			userClassCmd = 1;
	}
// Check to see if this is function command
	funcCmd = false;
	if(wordStart >= 2 && wordEnd >= wordStart)
	{
 		if(buf[wordStart-1] == ':')
		{
			funcCmd = true;
		}
	}
// Extract the class name
	if(funcCmd == true && wordStart >= 6)
	{
		for(i = wordStart-3; i >= 0 ; i--)
		{
			for(j = 0; j < szLeftDel; j++)
			{
				if(buf[i] == leftDelimiter[j])
					break;
			}
         if(j < szLeftDel) break;
		}
		wordStart = i+1;
	}
// Extract word from line
   buf[wordEnd+1] = '\0';
   name = new char[wordEnd-wordStart+2];
   strcpy(name,&buf[wordStart]);
   delete [] buf; 
   
   wordStart += lineStart;
   wordEnd += lineStart;
   return(name);
}

 
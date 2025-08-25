#include "stdafx.h"
#include "globals.h"
#include "allocate.h"
#include "variablesClass.h"
#include "string_utilities.h"
#include <ole2.h>
#include "command_other.h"
#include "globals.h"
#include "interface.h"
#include "scanstrings.h"
#include "StringPairs.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <boost/regex.hpp>
#include "memoryLeak.h"

using std::string;
using std::vector;
using std::pair;
using std::stringstream;
using std::ios_base;

void SearchForLine(FILE *fp, char *parameter, char *line, int len);
char* ExtractLineFromString(char *text, long pos, long &n);
char* ExtractLineFromStringByNumber(char* text, long &startCharNr, long &startLineNr, long findLineNr);
long MatchStrings(char *str1, long &i, long len1, char *str2, long j, long len2, char *temp);
void ShiftString(char *text, short j, short shift);
bool IsString(char *str);
int StringToAscii(Interface* itfc ,char args[]);
int AsciiToString(Interface* itfc ,char args[]);
void StripTrailingZerosFromString(char* number);

const int LEFT_PADDING = 3;
const int STATE_PADDING = 23;

/********************************************************************************
                 Removes trailing zeros from the fractional part of a number
********************************************************************************/

void StripTrailingZerosFromString(char* str)
{
   int i,j,k;
   int sz = strlen(str);

// Check for a decimal point
   if(!strchr(str,'.'))
      return;

// Handle scientific notation by ignoring exponent
   for(i = sz-1; i >= 0; i--)
   {
      if(str[i] == 'e' || str[i] == 'E')
         break;
   }

// If e/E not found start again
   if(i == -1)
      i = sz;

// Remove trailing zeroes
   for(j = i-1; j >= 0; j--)
   {
      if(str[j] != '0')
         break;
   }
   if(j > 0)
   {
      if(i == sz && j+1 < sz)
      {
         if(str[j] == '.')
            str[j] = '\0';
         else
            str[j+1] = '\0';
      }
      else
      {
         int del;
         if(str[j] == '.')
            del = 2;
         else
            del = 1;

         for(k = 0; k < sz-i; k++)
         {
            str[j+k+del] = str[k+i];
         }
         str[j+k+del] = '\0';
      }
   }
}

/*******************************************************************************************
                  Extract the nth line from a text string

 Syntax (line, nextCharNr, nextLineNr) = getline(text, lineNr, [startCharNr, startLineNr])

 If multiple lines need to be found then you can specify the start character and line
 numbers found from previous searches to speed things up.
*******************************************************************************************/

int GetLineFromText(Interface* itfc ,char args[])
{
   Variable textVar;
   char *text;
   char *line;
   long findLineNr;
   long startCharNr = 0;
   long startLineNr = 0;
   short nrArgs;

// Get string to search and line to extract *************  
   if((nrArgs = ArgScan(itfc,args,2,"text, line number, [start_char_number, start_line_number]","eeee","vlll",&textVar, &findLineNr, &startCharNr, &startLineNr)) < 0)
      return(nrArgs);  

   if(textVar.GetType() == UNQUOTED_STRING)
   {
      text = textVar.GetString();

      line =  ExtractLineFromStringByNumber(text, startCharNr, startLineNr, findLineNr);
      if(line)
      {
         itfc->retVar[1].MakeAndSetString(line);
         itfc->retVar[2].MakeAndSetFloat(startCharNr);
         itfc->retVar[3].MakeAndSetFloat(startLineNr);
         itfc->nrRetValues = 3;
         delete [] line;
      }
      else
      {
         itfc->retVar[1].MakeNullVar();
         itfc->retVar[2].MakeNullVar();
         itfc->retVar[3].MakeNullVar();
         itfc->nrRetValues = 3;
      }
   }

   return(OK);
}

/*
	RegexMatch
	----------
	Regular expression grouping.
	Uses standard POSIX regular expression syntax, as implemented in boost.
*/
int RegexMatch(Interface* itfc, char args[])
{
	Variable textVar;
	Variable regexVar;
	short nrArgs;

	if((nrArgs = ArgScan(itfc,args,2,"text, regex","ee","vv",&textVar,&regexVar)) < 0)
	{
		return nrArgs;
	}
	
	itfc->nrRetValues = 0;	

   if(VarType(&textVar) == UNQUOTED_STRING && VarType(&regexVar) == UNQUOTED_STRING)
   {
      char* text = VarString(&textVar);
      char* regex = VarString(&regexVar);
		
		// Sanity check the parameters.
		if(strlen(text) == 0)
      {
         ErrorMessage("empty text string");
         return(ERR);
      }
      if(strlen(regex) == 0)
      {
         ErrorMessage("empty regular expression string");
         return(ERR);
      }

		// Construct the regex.
		boost::regex expr;
		try
		{
			expr.assign(regex, boost::regex_constants::icase);
		}
		catch (boost::regex_error&)
		{
			ErrorMessage("%s is not a valid regular expression.", regex);
			return ERR;
		}

		// Extract the matches.
		boost::cmatch matches;
		if(boost::regex_match(text, matches, expr))
		{
			for (unsigned int i = 1; i < matches.size(); ++i)
			{
				string match(matches[i].first, matches[i].second);
				itfc->retVar[++itfc->nrRetValues].MakeAndSetString((char*)match.c_str());	
			}
		}
	}   
	else
   {
      ErrorMessage("arguments should be strings");
      return(ERR);
   }
	return OK;
}

/**************************************************************************************
   Scans through a string extracting parameters
   Syntax is
   * == matches any set of characters
   ? == matches any single character
   %1 == a parameter to be matched and returned
   All other characters must be explicitly matched
  
   Example
  
   txt = "file dimensions = (4 5 6)
   a = scanstr(txt,"file dimensions = (%1))
   will return a = "4 5 6"
***************************************************************************************/

//TODO check ansVar reference at end
int ScanString(Interface* itfc ,char args[])
{
   Variable textVar;
   Variable regexVar;
   char *text;
   char *regex;
   short nrArgs;
   char *temp;
   long maxn = 0;

// Get string to search and regular expression to search for *************  
   if((nrArgs = ArgScan(itfc,args,2,"text, regex","ee","vv",&textVar,&regexVar)) < 0)
      return(nrArgs);  

   if(VarType(&textVar) == UNQUOTED_STRING && VarType(&regexVar) == UNQUOTED_STRING)
   {
      text = VarString(&textVar);
      regex = VarString(&regexVar);

      long lenTxt = strlen(text);
      long lenReg = strlen(regex);

      if(lenTxt == 0)
      {
         ErrorMessage("empty text string");
         return(ERR);
      }

      if(lenReg == 0)
      {
         ErrorMessage("empty regular expression string");
         return(ERR);
      }

      temp = new char[lenTxt+1];

      for(long regex_index = 0, text_index = 0; regex_index < lenReg; regex_index++)
      {
      // Skip multiple characters (*)
         if(regex_index < lenReg && regex[regex_index] == '*')
         {
         // Check for illegal regex
            if(regex_index < lenReg-1 && (regex[regex_index+1] == '?' || regex[regex_index+1] == '%'))
            {
               delete [] temp;
               ErrorMessage("invalid regex combination *?");
               return(ERR);
            }
            
          // Starting from regex[regex_index+1] and ending at a *,? or %
          // find the corresponding match in string text starting
          // from text[text_index]. (k is the last matching character position).
            regex_index++;

            long k = MatchStrings(regex,regex_index,lenReg,text,text_index,lenTxt,temp);
            
            if(k < 0 || (k == lenTxt && regex_index != lenReg)) // No match found so abort
            {
               delete [] temp;
					itfc->retVar[1].MakeNullVar();
					itfc->nrRetValues = 1;
               return(OK);
            }
            text_index = k;
      
         }
      // Skip a character (?)
         else if(regex_index < lenReg-1 && regex[regex_index] == '?')
         {
            text_index++;
         }
      // Variables of the form %n
         else if(regex[regex_index] == '%')
         {
            long n = (int)(regex[regex_index+1] - '0');
            if(n < '1' && n > '9')
            {
               delete [] temp;
               ErrorMessage("invalid variable: valid range %%1 to %%9");
               return(ERR);
            }
            regex_index+=2;
            long k = MatchStrings(regex,regex_index,lenReg,text,text_index,lenTxt,temp);
            regex_index++;
            if(k == -1)
            {
               delete [] temp;
               ErrorMessage("invalid array specification");
               return(ERR);
            }
            if(k == -2)
            {
               delete [] temp;
					itfc->retVar[1].MakeNullVar();
					itfc->nrRetValues = 1;
               return(OK);
            }
				long q;
            for(q = text_index; q < k; q++) // Copy matching part of text
               temp[q-text_index] = text[q];      
            temp[q-text_index] = '\0';

            text_index = k+1;
				itfc->retVar[n].MakeAndSetString(temp);
            if(n > maxn)
               maxn = n;
         }
      // Match specific characters
         else
         {
            if(text[text_index] != regex[regex_index])
            { // String mismatch return null variable
               delete [] temp;
			   itfc->retVar[1].MakeNullVar();
			   itfc->nrRetValues = 1;
               return(OK);
            }
            text_index++;
         }
      }
      delete [] temp;
   }
   else
   {
      ErrorMessage("arguments should be strings");
      itfc->nrRetValues = 0;
      return(ERR);
   }

// Make sure ansVar is updated with a copy of the first variable
   //if(maxn >= 1)
   //{
   //   if(CopyVariable(ansVar,&returnVar[1],FULL_COPY) == ERR)
   //      return(ERR);
   //}

   itfc->nrRetValues = maxn;

   return(OK);
}

// If the first non-space character is nonnumeric than this is a string
bool IsString(char *str)
{
   extern bool IsOperator(char ch);

   int size = strlen(str);

   for(int i = 0; i < size; i++)
   {
      if(str[i] == ' ' || str[i] == '\t')
         continue;

	// if(isdigit(str[i]) || str[i] == '-' || str[i] == '+' || str[i] == '.')
    if(isdigit(str[i]) || IsOperator(str[i]) ||  str[i] == '[' || str[i] == ']' || str[i] == '.')
         return(false);

      break;
   }
   return(true);
}

// Starting from str1[i] and ending at a *,? or %
// find the corresponding match in string text starting
// from str2[j]
          
long MatchStrings(char *regex, long &regex_index, long regex_length, char *text, long text_index, long text_length, char *temp)
{
   long k,q,w;
   long len;
   long dim;
   long lasti = 0;
   
// Check for case where * in text is in last character so
// match is till end of text
   if(regex_index == regex_length)
     return(text_length);
     
// Check to see if we are reading in an array of numbers
   if(regex[regex_index] == '%')
   {
      if(sscanf(&regex[regex_index+1],"array[%d",&dim) != 1)
      {
         return(-1);
      }
      for(k = regex_index+1; k < regex_length-1; k++)
      {
         if(regex[k] == ']' && regex[k+1] == '%')
         {
            lasti = k+2;
            break;
         }
      }
      if(k == regex_length-1)
         return(-1);

      // Scan through the string searching for an array of numbers which 
      // should be delimitied by whitespace or commas.
      // Return the end position of this array plus 1

      long cnt = 0;
      enum {IN_DELIMIT,IN_NUMBER} status = IN_DELIMIT;
      for(q = text_index; q < text_length; q++)
      {
         if(status == IN_DELIMIT && ((text[q] >= '0' && text[q] <= '9') || text[q] == '.'))
         {
            status = IN_NUMBER; 
         }

         if(status == IN_NUMBER && ((text[q] < '0' || text[q] > '9') && text[q] != '.'))
         {
            cnt++;
            status = IN_DELIMIT; 
            if(cnt == dim)
               break;
         }
      }
      if(q == text_length)
         return(-1);

      regex_index = lasti;
      return(q);
   }

// Find string to match in regex
// This string will be terminated with a *, % or a ?
   for(q = regex_index; q < regex_length; q++)
   {
      if(regex[q] == '*' || regex[q] == '%' || regex[q] == '?')
         break;
      if(q-regex_index >= text_length)
         return(-2);
      temp[q-regex_index] = regex[q];
   }
   temp[q-regex_index] = '\0';
   len = q-regex_index;
   
// Check to see that this string is found in text
   for(k = text_index; k < text_length; k++) 
   {
      if(text[k] == temp[0])
      {
         for(w = 1; w < len; w++)
         {
            if(text[k+w] != temp[w]) // Mismatch so try next character
               break;
         }
         if(w == len) // text has this string
            break;
      }
   }
   if(k == text_length)
      return(-2);
   regex_index--;
   return(k);
}


/************************************************************************
    Concatenate strings of arbitary length

    Syntax: result = concatstr(str1,str2)

************************************************************************/

//int ConcatenateStrings(char args[])
//{
//   CText str1,str2,result;
//   short nrArgs;
//
//// Get string to search and substring to replace *************  
//   if((nrArgs = ArgScan(itfc,args,2,"str1, str2","ee","tt",&str1,&str2)) < 0)
//      return(nrArgs);  
//
//   result = str1 + str2;
//
//   ansVar->MakeAndSetString(result.Str());
//
//   return(OK);
//}

/************************************************************************
    Replace all instances of oldsubstr with newsubstr in string text

    Syntax: result = replacestr(str,oldsubstr,newsubstr)

************************************************************************/

int ReplaceString(Interface* itfc ,char args[])
{
   CText text;
   CText oldStr;
   CText newStr;
   CText result;
   long i,j,k;
   bool ignoreCase = false;
   short nrArgs;
   long lenText,lenOld,lenNew;

// Get string to search and substring to replace *************  
   if((nrArgs = ArgScan(itfc,args,3,"text, oldsubstr, newsubstr","eee","ttt",&text,&oldStr,&newStr)) < 0)
      return(nrArgs);  

   lenText = text.Size();
   lenOld = oldStr.Size();
   lenNew = newStr.Size();

   if(lenText == 0 || lenOld == 0)
	{
		ErrorMessage("The main string and the string to be replaced must not have zero length");
		return(ERR);
	}

   for(i = 0; i <= lenText; i++)
   {
      for(j = 0; j < lenOld; j++)
      {
         if(ignoreCase)
         {
            if(tolower(text[i+j]) != tolower(oldStr[j]))
               break;
         }
         else
         {
            if(text[i+j] != oldStr[j])
               break;
         }         
      }
      if(j == lenOld) // Match found
      {
         for(k = 0; k < lenNew; k++)
            result.Append(newStr[k]);
         i += lenOld-1;
      }
      else
         result.Append(text[i]);
   }
 
// Copy the result string to ansVar and free used memory
   itfc->retVar[1].MakeAndSetString(result.Str());
   itfc->nrRetValues = 1;

   return(OK);
}

/************************************************************************
    Convert a string or list to upper or lower case

    Syntax: out = caseset(in,mode)

    in ... input string or list
    out .. output string or list
    mode . one of "lower","upper","first"

************************************************************************/

int SetStringCase(Interface *itfc, char args[])
{
   CText text;
   CText mode = "upper";
   short nrArgs;
   Variable var;

// Get string to modify *************  
   if((nrArgs = ArgScan(itfc,args,2,"text, mode","ee","vt",&var,&mode)) < 0)
      return(nrArgs);  

   if(var.GetType() == UNQUOTED_STRING)
   {
      char *str = var.GetString();

      if(str)
      {
         if(mode == "lower")
            ToLowerCase(str);

         else if(mode == "upper")
            ToUpperCase(str);

         else if(mode == "first")
            UpperCaseForFirst(str); 

         else
         {
            ErrorMessage("Invalid mode ('upper/lower/first')");
            return(ERR);
         }
         itfc->retVar[1].MakeAndSetString(str);
         itfc->nrRetValues = 1;
      }
      else
      {
         ErrorMessage("Null string");
         return(ERR);
      }
   }
   else if(var.GetType() == LIST)
   {
      char **lst = var.GetList();
      long len = var.GetDimX();

      if(lst != NULL && len > 0)
      {
         if(mode == "lower")
         {
            for(long i = 0; i < len; i++)
               ToLowerCase(lst[i]);
         }
         else if(mode == "upper")
         {
            for(long i = 0; i < len; i++)
               ToUpperCase(lst[i]);
         }
         else if(mode == "first")
         {
            for(long i = 0; i < len; i++)
               UpperCaseForFirst(lst[i]);
         }
         else
         {
            ErrorMessage("Invalid mode ('upper/lower/first')");
            return(ERR);
         }
         itfc->retVar[1].MakeAndSetList(lst,len);
         itfc->nrRetValues = 1;
      }
      else
      {
         ErrorMessage("Null list");
         return(ERR);
      }
   }

   return(OK);
}

  
/************************************************************************
    Convert a string to lower case

    Syntax: result = lowerstr(str)

************************************************************************/
//
//int LowerString(char args[])
//{
//   CText text;
//   short nrArgs;
//
//// Get string to modify *************  
//   if((nrArgs = ArgScan(itfc,args,1,"text","e","t",&text)) < 0)
//      return(nrArgs);  
//
//// Modify it
//   text.LowerCase();
// 
//// And return it
//   ansVar->MakeAndSetString(text.Str());
//
//   return(OK);
//}

/*
int ReplaceStringB(char args[])
{
   Variable textVar;
   Variable oldVar;
   Variable newVar;
   char *text;
   char *oldStr;
   char *newStr;
   CText result;
   long i,j,k;
   bool ignoreCase = false;
   short nrArgs;
   long lenText,lenOld,lenNew,lenResult;

// Get string to search and substring to replace *************  
   if((nrArgs = ArgScan(itfc,args,3,"text, oldsubstr, newsubstr","eee","vvv",&textVar,&oldVar, &newVar)) < 0)
      return(nrArgs);  

   if(VarType(&textVar) == UNQUOTED_STRING && VarType(&oldVar) == UNQUOTED_STRING && VarType(&newVar) == UNQUOTED_STRING)
   {
      text = VarString(&textVar);
      oldStr = VarString(&oldVar);
      newStr = VarString(&newVar);
      lenText = strlen(text);
      lenOld = strlen(oldStr);
      lenNew = strlen(newStr);

      for(i = 0; i <= lenText; i++)
      {
         for(j = 0; j < lenOld; j++)
         {
            if(ignoreCase)
            {
               if(tolower(text[i+j]) != tolower(oldStr[j]))
                  break;
            }
            else
            {
               if(text[i+j] != oldStr[j])
                  break;
            }         
         }
         if(j == lenOld) // Match found
         {
            for(k = 0; k < lenNew; k++)
               result.Append(newStr[k]);

            i += lenOld-1;
         }
         else
            result.Append(text[i]);

      }
   }
   else
   {
      ErrorMessage("all arguments should be strings");
      return(ERR);
   }

// Copy the result string to ansVar and free used memory

   ansVar->MakeAndSetString(result.Str());

   return(OK);
}


  
// Replace all instances of oldsubstr with newsubstr in string text

int ReplaceStringC(char args[])
{
   Variable textVar;
   Variable oldVar;
   Variable newVar;
   char *text;
   char *oldStr;
   char *newStr;
   vector<char> result;
   long i,j,k;
   bool ignoreCase = false;
   short nrArgs;
   long lenText,lenOld,lenNew,lenResult;


// Get string to search and regular expression to search for *************  
   if((nrArgs = ArgScan(itfc,args,3,"text, oldsubstr, newsubstr","eee","vvv",&textVar,&oldVar, &newVar)) < 0)
      return(nrArgs);  

   if(VarType(&textVar) == UNQUOTED_STRING && VarType(&oldVar) == UNQUOTED_STRING && VarType(&newVar) == UNQUOTED_STRING)
   {
      text = VarString(&textVar);
      oldStr = VarString(&oldVar);
      newStr = VarString(&newVar);
      lenText = strlen(text);
      lenOld = strlen(oldStr);
      lenNew = strlen(newStr);

      for(i = 0; i <= lenText; i++)
      {
         for(j = 0; j < lenOld; j++)
         {
            if(ignoreCase)
            {
               if(tolower(text[i+j]) != tolower(oldStr[j]))
                  break;
            }
            else
            {
               if(text[i+j] != oldStr[j])
                  break;
            }         
         }
         if(j == lenOld) // Match found
         {
            for(k = 0; k < lenNew; k++)
              result.push_back(newStr[k]);
            i += lenOld-1;
         }
         else
            result.push_back(text[i]);
      }
   }
   else
   {
      ErrorMessage("all arguments should be strings");
      return(ERR);
   }

// Copy the result string to ansVar and free used memory

   lenResult = result.size();
   char *out = new char[lenResult+1];
   for(k = 0; k < lenResult; k++)
      out[k] = result[k];
   out[k] = '\0';
   ansVar->MakeAndSetString(out);
   delete [] out;

   return(OK);
}
*/



char* ExtractLineFromString(char *text, long pos, long &n)
{
   long i;
   long len = strlen(text);

   if(pos < 0 || pos >= len)
      return(NULL);

// Find line number
   n = 0;
   for(i = 0; i < pos; i++)
   {
      if(text[i] == '\r' && text[i+1] == '\n')  // PC file
      {
         i++;
         n++;
         continue;
      }

      if(text[i] == '\r' || // Mac file
         text[i] == '\n')   // Unix file
      {
         n++;
      }
   }

// Find start of line
   long start = 0;
   for(i = pos; i >= 0; i--)
   {
      if(text[i] == '\n' || text[i] == '\r')
      {
         start = i+1;
         break;
      }
   }

// Find end of line
   long end = len-1;
   for(i = pos; i < len; i++)
   {
      if(text[i] == '\r' || text[i] == '\n')
      {
         end = i-1;
         break;
      }
   }

   char *line = new char[end-start+2];
   for(i = start; i <= end; i++)
      line[i-start] = text[i];
   line[i-start] = '\0';

   return(line);
}

// Extract line number n from text

char* ExtractLineFromStringByNumber(char *text, long &startCharNr, long &startLineNr, long findLineNr)
{
   long i,pos;
   long len = strlen(text);


// Find line number
   long cnt = startLineNr;
   for(i = startCharNr; i < len; i++)
   {
      if(i < len-1 && text[i] == '\r' && text[i+1] == '\n')  // PC file
      {
         if (cnt == findLineNr)
         {
            startCharNr = i + 2;
            startLineNr = findLineNr + 1;
            break;
         }
         i++;
         cnt++;
         continue;
      }

      if(text[i] == '\r' || // Mac file
         text[i] == '\n')   // Unix file
      {
         if (cnt == findLineNr)
         {
            startCharNr = i + 1;
            startLineNr = findLineNr + 1;
            break;
         }
         cnt++;
      }
   }
   pos = i-1; // Go to character before EOL
   long end = pos; // This is the end of line

// Line not found
   if(cnt < findLineNr)
      return(NULL);
   
// Find start of line
   long start = 0;
   for(i = pos; i >= 0; i--)
   {
      if(text[i] == '\n' || text[i] == '\r')
      {
         start = i+1;
         break;
      }
   }

   char *line = new char[end-start+2];
   for(i = start; i <= end; i++)
      line[i-start] = text[i];
   line[i-start] = '\0';


   return(line);
}




void SearchForLine(FILE *fp, char *parameter, char *line, int len)
{
   line[0] = '\0';

   while(1)
   {
      if(fgets(line,len,fp) == NULL)
         break;
      if(strstr(line,parameter))
         break;
   }
   len = strlen(line);
   if(line[len-1] == '\n')
      line[len-1] = '\0';
   len = strlen(line);
   if(line[len-1] == '\r')
      line[len-1] = '\0';
   fclose(fp);
}

// Add character to the end of a string if its not already there
void AddCharToString(char *str, char c)
{
   int s = strlen(str);

   for(int i = 0; i < s; i++)
   {
      if(str[i] == c)
         return;
   }
   str[s] = c;
   str[s+1] = '\0';
}

// Remove the first instance of a character from a string
void RemoveCharFromString(char *str, char c)
{
   int s = strlen(str);

   for(int i = 0; i < s; i++)
   {
      if(str[i] == c)
      {
         for(int j = i; j < s; j++)
         {
            str[j] = str[j+1];
         }
         str[s-1] = '\0';
         return;
      }
   }
}


/***************************************************************************
  In 'text' replace 'target' substring by 'replace' string
  Note that text must be long enough to accommodate any extra characters
  maxSize is the total length of the input string including null character.
  If the final string will be smaller than the original then maxSize
  can be passed as -1.
***************************************************************************/

void ReplaceSpecialCharacters(char *text, char* target, char* replace, int maxSize)
{
   long j = 0;
   long k = 0;
   long lenT,lenR;
   bool match = false;

   lenT = strlen(target);
   lenR = strlen(replace);

// Work out total length passed string
   if(maxSize == -1)
      maxSize = strlen(text)+1;

   if(lenT >= lenR) // Target substring is longer than replacement substring
   {  
		long i = 0;
	   while(1)
	   {
	      if(text[j] == '\0') break;
	      
	      match = true;
	      for(k = 0; k < lenT; k++)
	      {
	         if(text[j+k] != target[k])
	         {
	            match = false;
	            break;
	         }
	      }
	      
	      if(match) // Found target?
	      {
	         lenR = strlen(replace);
	         for(k = 0; k < lenR; k++)
            {
               if(i+k >= maxSize)
               {
                  text[maxSize-1] = '\0';
                  return;
               }
	            text[i+k] = replace[k];
            }
	         i+=lenR;
	         j+=lenT;
	      }
	      else      
         {
            if(i >= maxSize)
            {
               text[maxSize-1] = '\0';
               return;
            }
	         text[i++] = text[j++];
         }
	   }
      if(i >= maxSize)
      {
         text[maxSize-1] = '\0';
         return;
      }
	   text[i] = '\0';
	}
	else // Target substring is shorter than replacement substring
   {  
	   while(1)
	   {
	      if(text[j] == '\0') break;
	   
	      for(k = 0; k < lenT; k++) // Search for target at current position
	      {
	         if(text[j+k] == target[k])
	            match = true;
	         else
	            match = false;
	      }	
	      
	      if(match) // Found target? If so shift contents of string  
	      {
	         ShiftString(text,j,lenR-lenT);
	         for(k = 0; k < lenR; k++)
	         {
               if(j+k >= maxSize)
               {
                  text[maxSize-1] = '\0';
                  return;
               }
	            text[j+k] = replace[k];
	         }
	         j += k;
	      }
	      else
	         j++;
	   }	
	}
}


// Convert contents of 'str' to lower case
char* ToLowerCase(char *str)
{
	long len = strlen(str);
	for(long i = 0; i < len; i++)
	{
		if(str[i] > 0 && isupper(str[i]))
			str[i] += ('a'-'A');
	}
   return(str);
}

// Convert contents of 'str' to upper case
char* ToUpperCase(char *str)
{
	long len = strlen(str);
	for(long i = 0; i < len; i++)
	{
		if(str[i] > 0 && islower(str[i]))
			str[i] -= ('a'-'A');
	}
   return(str);
}


void UpperCaseForFirst(char *str)
{
	long len = strlen(str);
	for(long i = 0; i < len; i++)
	{
		if(i == 0)
		{
			if(str[i] > 0 && islower(str[i]))
				str[i] -= ('a'-'A');
		}
		else
		{
			if(str[i] > 0 && isupper(str[i]))
				str[i] += ('a'-'A');
		}
	}
}

/********************************************************************************
   Starting from position "pos" search string text for substr
   if found returned start of substr in pos otherwise pos is
   unchanged
********************************************************************************/
	
bool FindSubStr(char *text, char *substr, long &pos)
{
   long i;
   long lenText = strlen(text);
   long lenSS = strlen(substr);
   
   for(i = pos; i < lenText; i++)
   {
      if(text[i] == substr[0])
      {
         if(!strncmp(&text[i],substr,lenSS))
         {
            pos = i;
            return(true);
         }
      }
   }
   return(false);
}


/********************************************************************************
   Extract from string text the characters from position "start" to position
   "end" storing the result in string substr
********************************************************************************/

void ExtractSubStr(char *text, char *substr, long start, long end)
{
   long i,j;
   
   for(i = start, j = 0; i < end; i++,j++)
   {
      substr[j] = text[i];
   } 
   substr[j] = '\0';
}

/**********************************************************************************************
* This function allows the printing arbirarily long strings
**********************************************************************************************/


char*  vssprintf(const char *format, va_list argptr)
{
	int n = _vscprintf(format,argptr)+1;
	char *buffer = new char[n];
	_vsnprintf(buffer,n,format,argptr);
	return (buffer);
}

/****************************************************************************************
           Shift the contents of "text" right by "shift" characters
           starting at position j
*****************************************************************************************/

void ShiftString(char *text, short j, short shift)
{   
   for(short i = strlen(text); i >= j; i--)
      text[i+shift] = text[i];
}

// Compare str1 and str2 ignoring case
int StrCmpIgnoreCase(char *str1, char *str2)
{
	long len1 = strlen(str1);
	long len2 = strlen(str2);

	if(len1 != len2)
		return(1);

	for(int i = 0; i < len1; i++)
	{
		if(tolower(str1[i]) != tolower(str2[i]))
			return(1);
	}

	return(0);
}


/*************************************************************************
    Count the number of times character c appears in string txt
*************************************************************************/
   
long CountCharacters(char *txt, char c)
{
   long cnt = 0;
   long len = strlen(txt);
   
   for(long i = 0; i < len; i++)
   {
      if(txt[i] == c) cnt++;
   }
   return(cnt);
}

/************************************************************************
If tf == true, returns "true", otherwise returns "false".
************************************************************************/
const char* const toTrueFalse(bool tf)
{
	return (tf ? "true" : "false");
}

/************************************************************************
Turns a primitive number into a string. 
************************************************************************/
const string stringifyFloat(double f)
{
	static char result[32];
	sprintf(result,"%.2f",f);
	return (string) result;
}

const string stringifyInt(long l)
{
	static char result[32];
	sprintf(result,"%d",l);
	return (string) result;
}

/************************************************************************
Generates a string suitable as a header for the "zero-arg"
response to a widget/class method.
************************************************************************/
const string formatNoArgHeader()
{
	static string header = "";

	if (!header.length())
	{
		header.append(2,'\n');
		header.append(LEFT_PADDING, ' ').append("PARAMETER").append(STATE_PADDING - strlen("PARAMETER") + 1,' ').append("VALUE");
		header.append(2,'\n');
	}
	return header;
}

/************************************************************************
Formats the current state of a class/widget's parameter-accessible
attributes.
************************************************************************/
const string FormatStates(StringPairs&  state)
{
	StringPairs::const_iterator it;
	string fstate;

	for(it = state.begin();it < state.end(); ++it)
	{
		int unpaddedLength = (*it)->first()->length();
			
		fstate.append(LEFT_PADDING, ' ');
		fstate += *((*it)->first());
		// Pad the command name out to a uniform length of STATE_PADDING
		fstate.append(STATE_PADDING - unpaddedLength, '.');
		fstate.append(" ").append(*((*it)->second())).append("\n");
	}	
	return fstate;
}

// Convert char* string to wchar_t string - note need to deallocate after use
// using SysFreeString

wchar_t* CharToWChar(const char* const txt)
{
   short len = strlen(txt);
   wchar_t *wTxt = SysAllocStringLen(NULL, len);
   MultiByteToWideChar(CP_ACP, 0, txt, len, wTxt, len);

   return(wTxt);
}



// Convert a numeric ascii array to a string

int AsciiToString(Interface* itfc ,char args[])
{
  short nrArgs;  
  Variable var;
  long i;

  // Copy passed array into a variable var
  if((nrArgs = ArgScan(itfc,args,1,"input data","e","v",&var)) < 0)
    return(nrArgs); 

  // Test for correct variable type
  if(VarType(&var) == MATRIX2D)
  {
     if(VarHeight(&var) == 1)
     {
        // Extract variable contents
        float **array = VarRealMatrix(&var);
        long width = VarWidth(&var);

        // Generate return string
        char *asciiStr = new char[width+1]; // Extra +1 for null character
        for(i = 0 ; i < width; i++)
          asciiStr[i] = (int)(array[0][i]+0.5); // Round float
        asciiStr[i] = '\0';

        // Fill the answer matrix with this array and return 
        itfc->retVar[1].MakeAndSetString(asciiStr);
        itfc->nrRetValues = 1;

        // Free up the allocated memory
        delete [] asciiStr;
     }
     else
     {
       ErrorMessage("invalid data type passed (should be row vector)");
       return(ERR);
     }
  }
  else if(VarType(&var) == FLOAT32)
  {
     float num = VarReal(&var);
     char ascii[2];

     if(num < 0 || num > 255)
     {
       ErrorMessage("invalid number passed (should be >= 0 and < 256)");
       return(ERR);
     }
     
     ascii[0] = (int)(num+0.5);
     ascii[1] = '\0';

     itfc->retVar[1].MakeAndSetString(ascii);
     itfc->nrRetValues = 1;
     return(OK);
  }

  return(OK);
}


// Convert a string to an array of ascii numbers

int StringToAscii(Interface* itfc ,char args[])
{
  short nrArgs;  
  Variable var;
  long i;

  // Copy passed array into a variable var
  if((nrArgs = ArgScan(itfc,args,1,"input data","e","v",&var)) < 0)
    return(nrArgs); 

  // Test for correct variable type
  if(VarType(&var) == UNQUOTED_STRING)
  {
     char *str = var.GetString();
     long width = strlen(str);

     if(width > 1)
     {
        float **result = MakeMatrix2D(width,1);

        for(i = 0 ; i < width; i++)
          result[0][i] = (unsigned char)str[i];

        // Fill the answer matrix with this array and return 
        itfc->retVar[1].AssignMatrix2D(result,width,1);
     }
     else
     {
        itfc->retVar[1].MakeAndSetFloat((unsigned char)str[0]);
     }
     itfc->nrRetValues = 1;
  }
  else
  {
    ErrorMessage("invalid data type passed (should be a string)");
    return(ERR);
  }

  return(OK);
}

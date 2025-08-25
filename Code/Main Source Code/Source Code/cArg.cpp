#include "stdafx.h"
#include "cArg.h"
#include "globals.h"
#include "variablesClass.h"
#include "scanstrings.h"
#include "memoryLeak.h"

  // Constructor
   CArg::CArg() 
   {
		len		    = 0;
      input        = NULL;
      extractedArg = NULL;
      argStart     = NULL;
      argEnd       = NULL;
      Init(',');	     
   }

  // Constructor
   CArg::CArg(char delimiter) 
   {
		len		    = 0;
      input        = NULL;
      extractedArg = NULL;
      argStart     = NULL;
      argEnd       = NULL;
      Init(delimiter);	     
   }

   // Destructor
   CArg::~CArg() 
   {
      if(input)        delete [] input;	
      if(extractedArg) delete[] extractedArg;
      if(argStart)     delete[] argStart;
      if(argEnd)       delete[] argEnd;
   }
   

 // Initialize the class by reseting all variables *********       
   void CArg::Init(char d)
   { 
      pos       = 0;
	   inBracket = 0;
      inString  = false;
	   argCnt    = 0;	 
	   delimiter = d; 
      nrArgs    = -1;
      maxArgs   = 20;
      if(input)        delete[] input;	
      if(extractedArg) delete[] extractedArg;
      if(argStart)     delete[] argStart;
      if(argEnd)       delete[] argEnd;
      input        = NULL;
      extractedArg = NULL;
      argStart     = new int[maxArgs];
      argEnd       = new int[maxArgs];
   }   
   
  /**********************************************************************************
    
     Scans through a string counting the number of arguments which are separated
     by the specified delimiter. Spaces are ignored as are expression in arrays [...],
     {...} strings '...' and argument lists (...).
     The routine also notes the starting and finishing positions of each argument
     which it stores in the global arrays 'argStart' and 'argEnd'. Subsequent calls to
     the routine Extract() use this information to return the nth argument in a string.
   
  ***********************************************************************************/

   int CArg::Count(char *str)
   {
      int len,i,k,cnt  = 1;
      int inBracket  = 0;
      bool inArg     = false;
      bool inString  = false;
      char c;

   // Ignore null strings
      len = strlen(str);
      if(len == 0) return(0);   

   // Check to see if we have scanned this string already 
      if(input && !strcmp(str,input))
         return(nrArgs);

   // Initialize argument range arrays to -1
      for(i = 0; i < maxArgs; i++)
         argStart[i] = argEnd[i] = -1;

   // Check each character in string
      for(i = 0; i < len; i++) 
      {
         c = str[i];

         if(!inArg && c == ' ') // Ignore space delimiters
            continue; 

         if(!inArg && c != ' ') // Found start of non-string
         {
            inArg = true;
            if(cnt == maxArgs) 
            {
               ReallocateArrays();
               for(int j = cnt; j < maxArgs; j++)
                  argStart[j] = argEnd[j] = -1;
            }
            argStart[cnt] = i;
         }

         if(inArg)
         {
            if(c != delimiter)
            {
               if(!inString && (c == '(' || c == '['  || c == '{'))       // Found start of bracketed expression
                  inBracket++;
               else if(!inString && (c == ')' || c == ']' || c == '}'))   // Found end of bracketed expression
                  inBracket--;
               else if(c == QUOTE && !IsEscapedQuote(str,i,len))  // Found start of quoted string
                  inString =  !inString;
            }

            if((c == delimiter) && inArg && !inBracket && !inString) // Found  delimiter
            {
             // Set argEnd - ignoring trailing spaces
               for(k = i-1; k > argStart[cnt]; k--)
               {
                  if(str[k] != ' ')
                     break;
               }
               argEnd[cnt++] = k;
               inArg = false;
            }
         }
      }
   // Set the last argEnd - ignoring spaces
      for(k = i-1; k > argStart[cnt]; k--)
      {
         if(str[k] != ' ')
            break;
      }
      argEnd[cnt] = k;
ex:
   // Record the number of argument found
      nrArgs = cnt;

   // Save the str to parse for later extraction
     if(input) 
         delete [] input;
      input = new char[strlen(str)+1];
      strcpy(input,str);

      return(nrArgs);
   }

/*********************************************************************   
 Extract the nth argument from str (n starts from 1). See Count 
 for more information.  
**********************************************************************/

   char* CArg::Extract(int nr) 
   {
      if(nr >= maxArgs)
      {
         TextMessage("\n\n   Argument extraction error : n >= %d\n\n",maxArgs);
         return("");
      }

    // Ignore null strings
      if(input[0] == '\0')
         return("");

    // Count arguments and record their position 
      if(nrArgs == -1)
         nrArgs = Count(input);

    // Ignore requests beyond end
      if(nr > nrArgs)
         return("");

    // Get string extent
      int start = argStart[nr];
      int end = argEnd[nr];

    // Ignore non-existant entries
      if(start == -1 || end == -1)
         return("");

    // Allocate memory 
      if(extractedArg) delete[] extractedArg;
      extractedArg = new char[end-start+2];

    // Extract string 
      memcpy(extractedArg,input+start,end-start+1);
      extractedArg[end-start+1] = '\0';

      return(extractedArg);
   }
 
 /*********************************************************************  
   Get the next argument
 **********************************************************************/

   short CArg::GetNext(char *arg, char* argnext)
   {
      register int i;
      register int k = 0;
      register bool inArg = false;
      register char c;
               
      if(pos == 0)
      {
      	len = strlen(arg);
      }
      
      for(i = pos; i < len; i++)
      {
         c = arg[i];
			if(!inArg)
			{
			   if(c == ' ') // Ignore space delimiters
            {
               pos++;
	            continue; 
            }
	         else              // Found start of non-string
				   inArg = true;
		   }
	       
			if(inArg)
			{
				if(c == delimiter)
				{
				   if(inBracket || inString) // Comma is inside brackets so not a delimiter
				   {
					  argnext[k++] = c;
					  pos++;	
					  continue;		       
				   }	
				   else  // Found comma delimiter				   			   
				   {
				 	  argnext[k++] = '\0';
				 	  pos++; // Point to 1 post delimiter
				 	  return(OK);	 
				   } 

				}			
				else // Other characters
				{
					 if(!inString && (c == '(' || c == '[' || c == '{'))       // Found start of bracketed expression
						 inBracket++;
					 else if(!inString && (c == ')' || c == ']' || c == '}'))  // Found end of bracketed expression
				       inBracket--;
					 else if(c == QUOTE && !IsEscapedQuote(arg,i,len)) // Found quote
				       inString = !inString;

				    argnext[k++] = c;
				    pos++;
				    continue;		         
				 }
		    }
		 } 
		 argnext[k++] = '\0';
		 if(k == 1)
		    return(ERR);
		 else	 
		    return(FINISH);     
   }


 /*********************************************************************  
   Double the number of entries in the argStart and argEnd arrays
 **********************************************************************/

   void CArg::ReallocateArrays(void)
   {
      int *tempStart;
      int *tempEnd;

      tempStart = new int[maxArgs*2];
      tempEnd = new int[maxArgs*2];

      memcpy(tempStart,argStart,sizeof(int)*maxArgs);
      memcpy(tempEnd,argEnd,sizeof(int)*maxArgs);

      delete [] argStart;
      delete [] argEnd;

      argStart = tempStart;
      argEnd = tempEnd;

      maxArgs*= 2;
   }


   char* CArg::Str(void)
   {
      return(input);
   }

	      
// Find argument position (1 based)

	int CArg::Find(char *str)
	{
		int i=0;

	// Search for argument in string 

		for(i = 1; i <= nrArgs; i++)
		{
			if(!strcmp(Extract(i),str))
				return(i);
		}
		return(0);
	}

static char str[] = "a12,b12,c12,d12,e12";  // 12 sec (i.e 5 Mops/argument)

int ScanTest( char args[] )
{
 //   if((r = ArgScan(itfc,args,1,"arg1,arg2,arg3","eee","fff",&arg1,&arg2,&arg3)) < 0)
 //     return(r); 


   CArg carg;
   char arg[50];
   int n = 0;
   
 //  n = CountArgs("a");

// Takes 7 us per loop
//   n = CountArgs(args);
//   for(int i = 1; i <= n; i++)
//      strcpy(arg,ExtractArg(args,i));

// Takes 10 us per loop
     n = carg.Count(args);
     for(int i = 1; i <= n; i++)
        strcpy(arg,carg.Extract(i));

// Takes 2us per loop
//   while((e = carg.GetNext(args,arg)) == OK);

   
	return 0;
}








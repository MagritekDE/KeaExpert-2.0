#include "stdafx.h"
#include "ctext.h"
#include <ctype.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "memoryLeak.h"

#define INCREMENT 50
#define QUOTE ('"')

// Constructor - allocate 'increment' bytes of memory and set string length to 0
   CText::CText() 
   {
		increment = INCREMENT;
      text = new char[increment];
      text[0] = '\0';
      length = 0;
		allocated = increment;   
   }


// Constructor - allocate sufficient space for a string of length size
// anticiating that this string will be loaded by setting the length
// Additional space is allocated allowing for small replacements
// or appendages.
   CText::CText(long size) 
   {
      assert(size > 0);
		increment = INCREMENT;
      allocated = size+increment;
      text = new char[allocated];
		text[0] = '\0';
      length = size-1;
   }

// Constructor - copy string 'txt' into CText allocated sufficient memory for this
// Additional space is allocated allowing for small replacements
// or appendages.
   CText::CText(const char txt[]) 
   {
	   increment = INCREMENT;
      length = strlen(txt);
		allocated = length+increment;
		text = new char[allocated];
		memcpy(text,txt,length);
		text[length] = '\0';
   }

   // Constructor - copy CText 'str' into CText allocated sufficient memory for this
   CText::CText(const CText& str) 
   {
	   increment = INCREMENT;
      length = str.length;
		allocated = length+increment;
      text = new char[allocated];
      memcpy(text,str.text,length);
      text[length] = '\0';
   }


// Destructor
   CText::~CText() 
   {
      if(text) delete [] text;
      text = 0;
   }

// Assigment operator
	CText& CText::operator =  (const CText &str )
	{
		if( this == &str )
		{
			return *this;
		}
		length = strlen(str.text);
		if(allocated >= length+1)
			 memcpy(text,str.text,length+1);
		else
		{
		   if(text) delete [] text;
		   text = new char[length+1];
			memcpy(text,str.text,length+1);
		   allocated = length+1;	
		}
		return *this;
	}

// Assigment operator
	CText& CText::operator = (const char* str )
	{
		length = strlen(str);
		if(allocated >= length+1)
			 memcpy(text,str,length+1);
		else
		{
			if(text) delete [] text;
		   text = new char[length+1];
         memcpy(text,str,length+1);
			allocated = length+1;			
		}
		return *this;
	}

// Add a character to the end of a string
	void CText::Append(char c)
	{
	   if(allocated >= length+2)
		{
		   text[length++] = c;
			text[length] = '\0';			
		}
		else
		{
			allocated = length+increment;
		   char *temp = new char[allocated];
         memcpy(temp,text,length);
		   temp[length++] = c;
			temp[length] = '\0';
			if(text) delete [] text;
			text = temp;
			increment *= 2;
		}
	}
	

// Substring extraction index start to end inclusive
	CText CText::Middle(long start, long end)
	{
	   int i;
      assert(start >= 0 && end < length);
	   if(end < start)
		{	      
			return("");
		}
		int sz = end-start+1;
	   CText mid(sz+1); // Include space for null symbol
		memcpy(mid.text,text+start,sz);
		mid.text[sz] = '\0';
		mid.length = sz;
		return(mid);
	}
   

// Substring extraction index 0 to end inclusive
	CText CText::Start(long end)
	{
	   int i;
      assert(end < length);
	   if(end >= length)
		   return("");
	   CText mid(end+2); // Include space for null symbol
		for(i = 0; i <= end; i++)
		   mid.text[i] = text[i];
		mid.text[i] = '\0';
		mid.length = i;
		return(mid);
	}


// Substring extraction index start to last inclusive
	CText CText::End(long start)
	{
	   int i;
      assert(start >= 0 && start < length);
	   CText mid(length-start+1); // Include space for null symbol
		for(i = start; i < length; i++)
		   mid.text[i-start] = text[i];
		mid.text[i-start] = '\0';
		mid.length = i-start;
		return(mid);
	}
	
// Search for character 's' from start
	long CText::Search(long start, char c)
	{
      assert(start >= 0);
		for(int i = start; i < length; i++)
		{
		   if(text[i] == c)
			   return(i);
		}
      return(-1);
	}

// Reverse Search for a character
	long CText::ReverseSearch(char c)
	{
		for(int i = length-1; i >= 0; i--)
		{
		   if(text[i] == c)
			   return(i);
		}
      return(-1);
	}

// Change to lower case
	void CText::LowerCase()
	{
      for(long i = 0; i < length; i++)
      {
         if(isupper(text[i]))
            text[i] += ('a'-'A');
      }
	}

// Rellocate memory for a string 
   void CText::Reallocate(long size)
	{
      if(size > allocated)
      {
         if(text) delete [] text;
         text = new char[size];
         text[0] = '\0';
         allocated = size;
      }
      length = size-1;
	}

// Set string length 
   void CText::SetSize(long size)
	{
      assert(size < allocated);
      length = size;
	}

// Change to upper case
   void CText::UpperCase()
	{
      for(long i = 0; i < length; i++)
      {
         if(islower(text[i]))
            text[i] -= ('a'-'A');
      }
	}

// Change first character to upper case reset rest to lower case
   void CText::UpperCaseForFirst()
	{
      for(long i = 0; i < length; i++)
      {
         if(i == 0)
         {
            if(islower(text[i]))
               text[i] -= ('a'-'A');
         }
         else
         {
            if(isupper(text[i]))
               text[i] += ('a'-'A');
         }
      }
	}

//  Remove quotes from a string "test" -> test
   void CText::RemoveQuotes() 
   {
      if(text[0] == QUOTE && text[length-1] == QUOTE)
      {
         for(long i = 0; i < length; i++)
            text[i] = text[i+1];
         text[length-2] = '\0';
         length -=2;
      }
   }

//  Remove blanks space from a string "test" -> test
//  Options are trim::START, END and BOTH

   void CText::Trim(short mode) 
   {
		long  i,j;
	   if(mode == trim::START || mode == trim::BOTH)
		{
			for(i = 0; i < length; i++)
			{
				if(text[i] !=  ' ')
					break;
			}
			int start = i;
			for(i = start, j = 0; i < length; i++,j++)
			{
				text[j] = text[i];
			}
			text[j] = '\0';
			length = j;
			if(mode == 1)
				return;
		}

		if(mode == trim::END || mode == trim::BOTH)
		{
			for(i = length-1; i >= 0; i--)
			{
				if(text[i] !=  ' ')
					break;
			}
			text[i+1] = '\0';
			length = i+1;
		}
   }

// Remove character from string
   void CText::RemoveChar(char c) 
   {
      long i,j;
      for(i = 0, j = 0; i < length; i++)
      {
         if(text[i] != c)
            text[j++] = text[i];
      }
      length = j;
   }

// Remove extension from string
   void CText::RemoveExtension() 
   {
      long i;
      for(i = length-1; i >= 0; i--)
      {
         if(text[i] == '.')
         {
            text[i] = '\0';
            length = i;
            break;
         }
      }
   }

// Search for a substring
	long CText::FindSubStr(long start, char* subStr)
	{
      int len = strlen(subStr);
      int i,j;
		for(i = start; i < length; i++)
		{
		   for(j = 0; j < len; j++)
         {
		      if(text[i+j] != subStr[j])
			      break;
         }
         if(j == len)
            return(i);
		}
      return(-1);
	}


// Concatenation operator
   CText operator +(const CText &str1, const CText &str2)
   {
      CText temp;
      temp.length = str1.length+str2.length;
	   if(temp.text) delete [] temp.text;
		temp.allocated = str1.allocated+str2.allocated+1;
      temp.text = new char[temp.allocated];
      memcpy(temp.text,str1.text,str1.length);
      memcpy(temp.text+str1.length,str2.text,str2.length);
      temp.text[temp.length] = '\0';
      return(temp);
   }

// Concatenate str to text - length is known
   void CText::Concat(const char* str, long strLen)
   {
      long totalLength = length+strLen+1;
      if(allocated >= totalLength)
      {
         memcpy(text+length,str,strLen);
         text[totalLength-1] = '\0';
      }
      else
      {
	      char* temp = new char[totalLength];
         memcpy(temp,text,length);
         memcpy(temp+length,str,strLen);
         temp[totalLength-1] = '\0';
         if(text) delete [] text;
         text = temp;
         allocated = totalLength;
      }
      length = totalLength-1;
   }

// Concatenate str to text
   void CText::Concat(const char* str)
   {
      long strLen = strlen(str);
      long totalLength = length+strLen+1;
      if(allocated >= totalLength)
      {
         memcpy(text+length,str,strLen);
         text[totalLength-1] = '\0';
      }
      else
      {
	      char* temp = new char[totalLength];
         memcpy(temp,text,length);
         memcpy(temp+length,str,strLen);
         temp[totalLength-1] = '\0';
         if(text) delete [] text;
         text = temp;
         allocated = totalLength;
      }
      length = totalLength-1;
   }

// Copy str to text - length is known
   void CText::Assign(const char* str, long strLen)
   {
      long totalLength = strLen+1;
      if(allocated >= totalLength)
      {
         memcpy(text,str,strLen);
         text[strLen] = '\0';
      }
      else
      {
	      char* temp = new char[totalLength];
         memcpy(temp,str,strLen);
         temp[strLen] = '\0';
         if(text) delete [] text;
         text = temp;
         allocated = totalLength;
      }
      length = totalLength-1;
   }

// Copy str to text
   void CText::Assign(const char* str)
   {
      long strLen = strlen(str);
      long totalLength = strLen+1;
      if(allocated >= totalLength)
      {
         memcpy(text,str,strLen);
         text[strLen] = '\0';
      }
      else
      {
	      char* temp = new char[totalLength];
         memcpy(temp,str,strLen);
         temp[strLen] = '\0';
         if(text) delete [] text;
         text = temp;
         allocated = totalLength;
      }
      length = totalLength-1;
   }

// Write a formated string to text
   void CText::Format(char *format, ...)
   {
      va_list ap;

      va_start(ap,format);

      int n = _vscprintf(format,ap)+1;

      if(n < allocated)
         _vsnprintf(text,n,format,ap);
      else
      {
         if(text) delete [] text;
	      text = new char[n+increment];  // Allow for some expansion
         allocated = n+increment;
         _vsnprintf(text,n,format,ap);
      }
      length = n-1;
      va_end(ap);
   }

// Make an empty string without deallocating 
   void CText::Reset()
   {
      if(allocated)
      {
         length = 0;
         text[0] = '\0';
      }
   }

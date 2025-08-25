#ifndef CTEXT_H
#define CTEXT_H

#include <cstring>

// Class 


class  __declspec(dllexport)  CText
//class CText
{
   public:
      CText(void);                                                         // Create without assignment
      CText(long);                                                         // Create with allocation
      CText(const char[]);                                                 // Assign during definition
      CText(const CText& str) ;                                            // Assign during definition
      ~CText(void);                                                        // Delete string
      CText&  operator =(const CText &str1);                               // Assign
      CText&  operator =(const char* str1);                                // Assign
      char* Str(void) {return(text);}													// Return C string
      char& operator [](long index) {return(text[index]);}                 // Get or set by index

      long Size(void) {return(length);}                                    // Get size
	   CText Middle(long start, long end);
      CText Start(long end);                                               // Extract start of string
      CText End(long start);                                               // Extract end of string
      void SetSize(long size);                                             // Set string length
		long Search(long start, char c);                                     // Search for character c starting from 'start'
		long ReverseSearch(char c);                                          // Search for character c starting from end
      long FindSubStr(long start, char* subStr);
		void LowerCase();                                                    // Convert to lower case
		void UpperCase();                                                    // Convert to upper case
		void RemoveQuotes();  
		void Append(char c);                                                 // Append a character
      void Reallocate(long size);                                          // Reallocate memory
      void RemoveChar(char c);                                             // Remove a character from a string
      void Reset();                                                        // Initialise string to null 
		void UpperCaseForFirst();                                            // Convert first character to upper case lower for rest
      long GetAllocated(void) {return(allocated);}                         // Get amount of allocated memory
      void Concat(const char* str, long strLen);                           // Fast concatenate
      void Concat(const char* str);                                        // Fast concatenate
      void Assign(const char* str, long strLen);                           // Fast assign
      void Assign(const char* str);                                        // Fast assign
      void RemoveExtension(void);                                          // Remove filename extension
      void Trim(short mode);                                               // Remove white space from ends of string

      void Format(char *output, ...);                                      // Write formated output to CText
      friend CText operator +(const CText &str1, const CText &str2);       // Concatenate
  //    friend CText operator +(const CText &str1, char *str2);              // Concatenate
  //    friend CText operator +(char *str1, const CText &str2);              // Concatenate
      friend bool operator ==(const CText &str1, const CText &str2)        // Compare strings
                   {return(strcmp(str1.text,str2.text) == 0);}
      friend bool operator ==(const CText &str1, char *str2)               // Compare strings
                   {return(strcmp(str1.text,str2) == 0);}
      friend bool operator ==(char *str1, const CText &str2)               // Compare strings
                   {return(strcmp(str1,str2.text) == 0);}
      friend bool operator !=(const CText &str1, const CText &str2)        // Compare strings
                   {return(strcmp(str1.text,str2.text) != 0);}
      friend bool operator !=(const CText &str1, char *str2)               // Compare strings
                   {return(strcmp(str1.text,str2) != 0);}
      friend bool operator !=(char *str1, const CText &str2)               // Compare strings
                   {return(strcmp(str1,str2.text) != 0);}

		enum trim {START = 1, END = 2, BOTH = 3};

private:
      char *text;                                                          // String storage
      long length;                                                         // Number of characters in string
      long allocated;                                                      // Number of allocated bytes
      long increment;                                                      // Amount by which memory is incremented
};

#endif // define CTEXT_H

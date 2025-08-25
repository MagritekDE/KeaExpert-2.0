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
      char* Str(void) {return(text);}													// Return C string
      char& operator [](long index) {return(text[index]);}                 // Get or set by index

      long Size(void) {return(length);}                                    // Get size
      long GetAllocated(void) {return(allocated);}                         // Get amount of allocated memory
		void Append(char c);                                                 // Append a character
		CText Middle(long start, long end);                                  // Extract a substring
      CText Start(long end);                                               // Extract start of string
      CText End(long start);                                               // Extract end of string
      void Reallocate(long size);                                          // Reallocate memory
      void SetSize(long size);                                             // Set string length
      void Concat(const char* str, long strLen);                           // Fast concatenate
      void Concat(const char* str);                                        // Fast concatenate
      void Assign(const char* str, long strLen);                           // Fast assign
      void Assign(const char* str);                                        // Fast assign
		long Search(long start, char c);                                     // Search for character c starting from 'start'
		long ReverseSearch(char c);                                          // Search for character c starting from end
		void Reset();                                                        // Initialise string to null 
		void LowerCase();                                                    // Convert to lower case
		void UpperCase();                                                    // Convert to upper case
		void UpperCaseForFirst();                                            // Convert first character to upper case lower for rest
		void RemoveQuotes();  
      void Format(char *output, ...);                                      // Write formated output to CText
      friend CText operator +(const CText &str1, const CText &str2);       // Concatenate
      friend bool operator ==(const CText &str1, const CText &str2)        // Compare strings
                   {return(strcmp(str1.text,str2.text) == 0);}
      friend bool operator !=(const CText &str1, const CText &str2)        // Compare strings
                   {return(strcmp(str1.text,str2.text) != 0);}
private:
      char *text;                                                          // String storage
      long length;                                                         // Number of characters in string
      long allocated;                                                      // Number of allocated bytes
      long increment;                                                      // Amount by which memory is incremented
};

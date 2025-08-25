#ifndef CARG_H
#define CARG_H

#define FINISH -2

// Class 
class  __declspec(dllexport)  CArg
//class CArg
{
   public:
      CArg(void);
      CArg(char delimiter);
      ~CArg(void);
      void Init(char d);
      int Count(char *str);
      char* Extract(int nr);
      char* Str(void);
      short GetNext(char *arg, char* argnext);
		int Find(char *arg);
      void ReallocateArrays(void); 
   		
   private:
	   int pos;
	   int len;
	   int argCnt;
	   int inBracket;
      bool inString;
      int maxArgs;            // maximum number of elements in argStart/End arrays
      char delimiter;          // Delimiter to separate arguments
      char *input;             // String to scan
      int *argStart;           // Start position of argument
      int *argEnd;             // End position of argument
      int nrArgs;              // Number of arguments in string 
      char *extractedArg;      // Extract argument string
};

#endif // define CARG_H
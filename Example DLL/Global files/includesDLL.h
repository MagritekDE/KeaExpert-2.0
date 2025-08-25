#define PI 3.1415927

// Return values from comands
#define OK						0   // All is OK
#define ERR						-1  // There was an error
#define RETURN_FROM_DLL		-7  // We have check all DLL extension commands and couldn't find a match

// Evaluate first argument
#define NORMAL 0  // Something to do with 3D matricies
#define ASSIGN 1  // Just use NORMAL for now

// Used by GetVariable
#define ALL_VAR 1
#define NO_ALIAS 2

// Abort flag
#define MACRO_RUNNING 0
#define MACRO_ABORTED 1

// EXPORT and IMPORT definitions
#define EXPORT extern "C" __declspec(dllexport)
#define IMPORT extern "C" __declspec(dllimport)

// Complex data type
typedef struct
{
   float r;
   float i;
}complex;

// Non-prospa includes
#include "stdafx.h"
#include <math.h>
#include "stdio.h"

// Prospa headers needed by DLLs
#include "utilitiesDLL.h"
#include "allocateDLL.h"
#include "variablesClassDLL.h"
#include "variablesOtherDLL.h"
#include "CTextDLL.h"
#include "CArgDLL.h"

#define isnan _isnan

#define Interface void
// Passed variables from Prospa
class DLLParameters
{
   public:
   short nrRetVar;
   Variable *retVar;
   Interface *itfc;
};

// Routines in Prospa that the DLL has access to (see below)
IMPORT extern short Evaluate(Interface*,short ,char*,Variable*);
IMPORT extern void  TextMessage(char *text,...);
IMPORT extern void  ErrorMessage(char *text,...);
IMPORT extern void  Error(Interface*,char *text,...);
IMPORT extern short ArgScan(Interface*,char[], short, char[], char[], char[], ...);
IMPORT extern int  CountArgs(char *str);
IMPORT extern float StringToFloat(char*);
IMPORT extern short ProcessBackgroundEvents(void);
IMPORT extern int SetParameter(Interface* itfc ,char arg[]);
IMPORT extern short ProcessMacroStr(Interface *itfc, bool test, char *data);
IMPORT extern Variable* GetVariable(Interface *itfc, short restrictions, char name[], short &type);
IMPORT extern int UpdateProspaArgumentVariables(Interface *itfc, char *args);
IMPORT extern int UpdateProspaReturnVariables(DLLParameters *par, Interface *itfc);

// Details for these functions


/****** type = Evaluate(mode, text)
*
* The string in "text" is parsed and evaluated mathematically. The result is stored
* in the variable ansVar. The data type of the answer is also returned for convenience.
* These types are defined in the header variables_other.h. Mode should always be NORMAL.
* Example: if text = "2*sin(pi/2)" Then type = FLOAT and VarFloat(ansVar) = 2
* See variables_other.h for the macros which can be used for extracting other data
* types from ansVar.
*
*
****** TextMessage(text,...)
*
* Prints the text string onto the command line interface window. The text string
* may contain standard C formating substrings as defined for the printf C command.
* Example: TextMessage("pi = %f\n",3.14159). Unlike ErrorMessage, TextMessage adds 
* no extra formating.
*
*
****** ErrorMessage(text,...)
*
* Much like TextMessage but adds some additional formating.
* Example: ErrorMessage("'%s' is an invalid variable,"1var") This will output:
*
*    Error : '1var' is an invalid variable
*
*
****** ret = ArgScan(argStr,minNr,prompt,dataType,argType,varPtr ...) 
* 
* Extract arguments from argStr and pass results back in supplied variables
*
* argStr   : list of comma delimited arguments
* minNr    : the minimum number of acceptable arguments
* prompt   : what to print if no arguments passed
* dataType : type of data; e = expression, c = constant
* argType  : type of argument; 
*            s = string, f = float, d = short integer,
*            l = long integer, q = figure it out and return in string
*            v = result is stored in a variable
*            a = result is returned in ansVar (a more efficient method than v for 1 variable).
* varPtr    : data extracted from arguments is stored in these variable addresses
*
* ret : returns the number of arguments converted. -1 if error in expression or insufficient
* arguments passed, -2 if prompting or current defaults requested, 0 is all ok. 
*
* If the arguments list consists of "getargs" then the current default values passed to
* ArgScan are returned to the user in the form of variables.
*
*
******  ret = ExtractArg(list,n))
* 
* Extract the nth argument from list (n starts from 1) and return in 'ret'
* Entries in the list must be separated by commas or colons. Arguments embedded
* in arrays [...] {...} strings "..." or lists (...) will be ignored.
*
* Typical use:   strcpy(out,ExtractArg(list,n))
*
*
*
****** result = StringToFloat(text)
*
* Converts the number represented by string "text" into a floating point number stored
* in "result". "text" may represent a normal real number (1.234), be in scientific notation (2.3e-4)
* or in hexidecimal notation (0x2E).
*
*/


// More functions from Prospa which DLLs have access too.

IMPORT extern bool    CompareLists(char**, char**, long, long);
IMPORT extern char**  CopyList(char **, long);
IMPORT extern void    FreeList(char**,long);
IMPORT extern short   InsertStringIntoList(char*, char***, long, long);
IMPORT extern char**  JoinLists(char**, char**, long, long);
IMPORT extern char**  MakeList(long);
IMPORT extern char**  MakeListFromEvaluatedText(char*,long*);
IMPORT extern char**  MakeListFromText(char*,long*);
IMPORT extern short   RemoveListEntry(char***, long, long);
IMPORT extern short   ReplaceStringInList(char*, char***, long, long);

// Details for these functions

/******** result = CompareLists(list1,list2,len1,len2)
*
* Compare list1 and list2. Returns 1 if equal 0 if different.
* len1 and len2 are the number entries in list1 and list2
*
*
********* copy = CopyList(list,n)
*
*  Copy the 'list' which has length 'n'.
* Returns NULL if memory allocation error occurs
*
*
********* FreeList(list,n)
*
* Free space allocated for an list of 'n' strings 
*
*
********* result = InsertStringIntoList(str,&list,len,postion)
*
* Insert string 'str' into 'list' of length 'len' at entry index 'position'
* Returns -1 if invalid position
* Returns -2 if memory allocation error
* Returns 0 if insertion successful
* Note: you must pass the address of the  list
*
*
********* result = JoinLists(list1,list2,len1,len2)
*
* Joins list1 and list2 and return result as a new list. 
* Returns NULL if memory allocation error occurs
* len1 and len2 are the number entries in list1 and list2
*
*
********* list = MakeList(n)
*
* Make a list with n entries (fill with dummy strings)
*
*
********* list = MakeListFromEvaluatedText(text, len)
*
*    Converts a comma delimited list of substrings ('text') into a string array ('list')  
*    after evaluating each item to see if it has an embedded expression.    
*    Returns new list is all ok, NULL if there is a memory allocation error.
*
*
********* list = MakeListFromText(text, len)
*
*    Converts a comma delimited list of substrings ('text') into a string array ('list')  
*    Returns new list is all ok, NULL if there is a memory allocation error.
*
*
********* result = RemoveListEntry(&list,len,position)
*
* Remove entry at index 'position' for 'list' or length 'len'
* Returns -1 if invalid position
* Returns -2 if memory allocation error
* Returns 0 if removal successful
* Note: you must pass the address of the  list 
*
*
********* result = ReplaceStringInList(newStr, &list, len, position)
*
* Replace string in 'list' at index 'position' with 'newStr'.  
* Returns -1 if invalid position
* Returns -2 if memory allocation error
* Returns 0 if insertion successful
*
*/


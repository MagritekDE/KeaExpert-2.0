#ifndef INTERFACE_H
#define INTERFACE_H

#include "guiObjectClass.h"
#include <vector>

// Command interface
#define ALLOC_ARG 1
#define ALLOC_RET 2

typedef struct 
{
   CText name;
   Variable *var;
}
ArgVar;

typedef enum argmode {NONE_NAMED, SOME_NAMED, ALL_NAMED} ArgMode;
class CText;
class Macro;
class ObjectData;
class Variable;
class WinData;

typedef struct
{
   CText macroName;
   CText macroPath;
   CText procName;
   int lineNr;
}
MacroStackInfo;

class Interface
{
   public:
      Interface(void);
      Interface(short);
      ~Interface(void);
      CText name;
      CText macroName;
      CText macroPath;
      CText procName;
      ObjectData *obj;
      WinData *win;
      long objID;

      int nrRetValues;  // Number of returned values
      int defaultVar;   // Which variable is the default if only 1 assignment value
      int nrProcArgs;
      Variable *retVar; // Array of returned variables
      ArgVar *argVar; // Array of argument variables passed to procedure
      ArgMode namedArgumentMode; // Whether names are passed with variables
      Macro *macro;
      Macro *baseMacro;
      short allocMode;
      short macroDepth;
      short varScope;
      bool cacheProc;
      bool inCLI;
      bool debugging;  // Is macro running in debug mode?
      long startLine; // Initial line number for current procedure
      long lineNr;    // Line number of current command
      bool processEscapes; // Whether escape processes are to be processed 
		bool processExpressions ; // Whether expressions instrings will be processed
      void *ptr;      // Pointer to something
		MenuInfo *menuInfo;
		int menuNr;
      long parentLineNr; // Parent line nr

      long oldStartLine; // Initial line number for previous procedure
      long oldLineNr;    // Line number of previous command
      CText oldMacroName;
      CText oldMacroPath;
      CText oldProcName;
      std::vector<MacroStackInfo*> macroStack;
};

#endif //define INTERFACE_H
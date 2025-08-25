#ifndef MACRO_CLASS_H
#define MACRO_CLASS_H

#include "trace.h"
#include "variablesClass.h"

extern short macroArgs;

class Command;

// Defines
#define COMMAND_TYPE_NOT_SET 0
#define FUNCTION   1
#define ASSIGNMENT 2
#define EXPRESSION 3

#define EOL '\1'
#define EOL_CONT '\2'

#define MAX_ERROR 500
#define MAX_BREAKS 50
#define MAX_NESTING 50

typedef struct
{
   Variable *var;      // Stack variable
   double loopStart;   // Initial value of variable
   double steps;       // Number of steps
   double stepsLeft;   // Steps left
   double stepSize;    // Size of each step
}
ForStack; // Information for each for loop


// Macro class definitions

class Macro 
{
	public :
	   Macro();
	   ~Macro();
		void		buildArray(int);						// Build macro array
		char*		getRawMacro();							// Extract complete macro file
      long     CountMacroCommands();            // Counts number of commands in macro
		void		addRawMacro(char*);					// Add raw macro file to class
		int		getSize();								// Get number of commands
		Command*	getCmd(long);							// Extract ith command
		void		extractCommands(void);					//	Extract commands from raw data
		void 		extractTokens(char*);
	   short		extractCmdName(char*,char**);				
		short		extractCmdArg(char*,long, char**);
		void     ForNextCheck(void);
		void     WhileCheck(void);
		void     TryCatchCheck(void);
		void     removeComments(void);
		void     cleanUp(void);
		void		ProcedureCheck(void);
		void		IfEndifCheck(void);
      void     PushStack(Variable *var, double loopStart, double stepSize, double stepsLeft, double steps);
		void     PopStack(void);   // Pop for-next info off the stack
		void     InitStack(void);  // Initialize the stack
		char*		extractNextCommand(long, long&, long&, long&);	    	// Called by extractCommands
		void		addCmdInfo(long,char*,long);		   // Called by extractCommands
      void     removeEOL(void);     // Remove EOL from Macro replace with semicolon
      void     removeSpaces(void);  // Remove superfluous spaces from Macro   
      void     ReportMacroError(char *cmd, long cmdNr, long lineNr, short maxDepth, short macroDepth);
      void     ReportMacroAbort(char *cmd, long cmdNr, long lineNr);
      Variable* GetProcedure(char *path, char *macro, char *name);
// Macro variables
	
		long     stackPos;                // Pointer to current position on stack
		ForStack *stack;                  // For next stack (same number of elements as macro)
		bool     jump;                    // Check jump array or not?
		long     *jumpNr;                 // Which line to jump to exceptionally
		long     *nextNr;                 // Which line to jump to normally
		long     *tryNr;                  // Which line to jump to when an error occurs in a try block
      long     startLine;               // Which line is this macro from in the original file.

      bool     inTryBlock;              // Are we between try and catch statements?
						
	   Variable varList;       // Variable list for macro
	   Variable procList;      // Procedure list for macro
	   Variable *argList;      // Argument list for macro
	   Variable *retList;      // Returned argument list for macro
		char*		rawMacro;		// Copy of macro file, including comments
		char*		cleanedMacro;  // Copy of macro file, no comments
		Command*	command;			// Array of parsed command classes
		int		nrCommands;		// Number of commands in class array
      CText    macroPath;     // Name of macro folder
      CText    macroName;     // Name of macro
      CText    procName;      // Name of procedure
		CText    errorCommands; // Commands to run when error detected
};

#endif // define MACRO_CLASS_H
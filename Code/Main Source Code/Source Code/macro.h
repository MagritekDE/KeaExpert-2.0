#ifndef MACRO_H
#define MACRO_H

#include "variablesClass.h"
// Macro class definitions

class Command;

class Macro 
{
	public :
	   char     name[MAX_NAME];              // Macro name
		void		buildArray(int);						// Build macro array
		char*		getRawMacro();							// Extract complete macro file
		void		addRawMacro(char*);					// Add raw macro file to class
		int		getSize();								// Get number of commands
		Command*	getCmd(int);							// Extract ith command
		void		extractCommands();					//	Extract commands from raw data
		void 		extractTokens(char*);
	   short		extractCmdName(char*,char**);				
		void		extractCmdArg(char*,short, char**);
		void     ForNextCheck(void);
		void     cleanUp(void);
		void		IfThenCheck(void);
		void     PushStack(long);  // Push a number onto the stack
		long     PopStack(void);   // Pop a number off the stack
		void     InitStack(void);  // Initialize the stack
		
		long     stackPos;         // Pointer to current position on stack
		long     *stack;           // Stack (same number of elements as macro
		bool     jump;             // Check jump array or not?
		long     *jumpNr;          // Which line to jump to
						
	   Variable varList;       // Variable list for macro
		char*		rawMacro;		// Copy of macro file, including comments
		Command*	command;			// Array of parsed command classes
		int		nrCommands;		// Number of commands in class array
		
		char*		extractNextCommand(int&,int&,int&);		// Called by extractCommands
	
		void		addCmdInfo(int,char*,int,int);			// Called by extractCommands
};

#endif // define MACRO_H
#ifndef DLL_H
#define DLL_H

#include "ctext.h"

class CText;
class Variable;
class Interface;

class DLLParameters
{
   public:
      short nrRetValues;
      Variable *retVar;
      void *Interface;
};


typedef short( *MYFUNC) (char*,char*,DLLParameters*);
typedef void( *MYFUNC2) (void);
typedef bool( *MYFUNC3) (char*,char*);

typedef struct
{
	HINSTANCE Instance;          // Instance handle for this DLL
	CText Name;                  // Name of the DLL
	MYFUNC AddCommands;          // Addres of AddCommands function for this DLL
	MYFUNC2 ListCommands;        // Address of ListCommands function for this DLL
	MYFUNC3 GetCommandSyntax;    // Address of GetCommandSyntax for this DLL
}
DLLINFO;

extern DLLINFO *DLLInfo; // Array of currently loaded DLLs

short CountFiles(char[]);
char* GetDLLCommandSyntax(char cmd[]);
int ListDLLCommands(Interface* itfc ,char args[]);
int GetFunctionSyntax(char *path, char* currentMacro, char *cmd, char** syntax, char* calledPath, char* calledMacro, char* calledProcedure);
bool IsADLLCommand(char* cmd);
bool GetDLLHelpFolderName(Interface *itfc, char *cmd, CText &name);
int UseDLL(Interface* itfc, char args[]);
int LoadDLLs(Interface* itfc ,char args[]);
int UnloadDLLs(Interface* itfc, char args[]);
short ProcessContinue(Interface *itfc, char*,char*);

#endif // define DLL_H
#ifndef PROCESS_H
#define PROCESS_H

#include "defines.h"

class Interface;
class Variable;
class WinData;
class ObjectData;

// Function prototypes

short ProcessMacroStr(Interface *itfc, char *data, char *macroName, char *macroPath);
short ProcessMacroStr(Interface *itfc, char *data);
short ProcessMacroStr(int scope, char *data, ...);
EXPORT short ProcessMacroStr(Interface *itfc, bool test, char *data);
short ProcessMacroStr(int scope, WinData* parent, ObjectData *obj, char *data, char *arguments, char* procName, char* macroName, char *macroPath);
int AssignSpecialVariable(Interface* itfc ,char[]);
int AssignWithLock(Interface* itfc ,char[]);
int GetMacroPath(Interface* itfc ,char args[]);
int GetReturnValue(Interface* itfc, char args[]);
int GetLastError(Interface* itfc ,char args[]);
int OnError(Interface* itfc ,char args[]);
short MainWindowRedraw(void);
int GetMacroName(Interface* itfc ,char args[]);
int ImportMacro(Interface *itfc, char arg[]);

short AssignToExpression(Interface *itfc, short,char*,Variable*,bool);
EXPORT short ProcessBackgroundEvents(void);
short ProcessAssignment(Interface*,char*,char*);

extern short gNumMacrosRunning;
extern short macroDepth;
extern bool gShowWaitCursor;
extern bool  messageSent;
extern bool gAbort;
extern bool gEventPresent;
extern bool gBlockWaitCursor;
extern bool gAbortMacro;
extern CommandInfo gErrorInfo;
extern bool gCheckForEvents;

extern HCURSOR gResetCursor;
extern bool gShutDown;

DWORD WINAPI AbortMacro(PVOID par);

#endif // define PROCESS_H
#ifndef CONTROL_H
#define CONTROL_H

class CText;
class Interface;

extern bool gInTryBlock;
extern bool abortOnError;

int NextLoop(Interface* itfc ,char[]);
int IfStatement(Interface* itfc, char args[]);
int ElseIf(Interface* itfc, char args[]);
int WhileStatement(Interface* itfc, char args[]);
int ForLoop(Interface* itfc, char args[]);
int PrintString(Interface* itfc, char args[]);
char *LoadFileMacro(char*);
int EndWhileLoop(Interface* itfc, char args[]);
int ExitWhileLoop(Interface* itfc, char args[]); 
int ExitForLoop(Interface* itfc, char args[]);
int TryForAnError(Interface* itfc, char args[]);
int EndTry(Interface* itfc, char args[]);
int CatchError(Interface* itfc, char args[]);
short PrintRetVar(Interface *itfc, char *name);

// Macro
int Abort(Interface* itfc ,char args[]);
int MakeErrorString(Interface* itfc ,char args[]);
int AbortOnError(Interface* itfc ,char args[]);
short FormatTextMessage(char*,...);
short ParseAssignmentString(char *str, char *name, char *value);
void SimpleErrorMessage(char *text);

#endif // define CONTROL_H

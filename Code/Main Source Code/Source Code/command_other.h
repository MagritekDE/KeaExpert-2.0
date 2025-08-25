#ifndef COMMAND_OTHER_H
#define COMMAND_OTHER_H

#include "CmdInfo.h"

class CText;
class Interface;

extern bool gKeepCurrentFocus; // Allows window to display without getting put into background 
extern int gFocusCnt;
extern char gCommand[];
extern char helpviewer[];
extern bool useQuotedStrings;

void InitializeProspaCommandList(void);
void GetVersionTxt(CText &txt);
int OpenHelpFile(Interface* itfc ,char args[]);
int RunCommand(Interface *itfc, char *nameIn, char *arg, short mode);

int  NoOperation(char[]);
int  ProcedureStart(char[]);

int  PrintDirectory(char[]);
short  RecursiveSearch(char file[], char extension[], CText &path);

bool IsACommand(char []);
char* GetCommandSyntax(char name[]);
LONG64 GetCommandType(char name[]);
short CheckForMacroProcedure(Interface *itfc, char *command, char *args);

#endif //define COMMAND_OTHER_H
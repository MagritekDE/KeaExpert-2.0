#ifndef DEBUG_H
#define DEBUG_H

#include "ctext.h"

class EditRegion;
class Interface;
class Variable;
class WinData;
class Command;

// Debug information
typedef struct
{
   bool enabled;
   bool step;
   CText mode;
   CText var;
   CText cmd;
   CText message;
   WinData *win;
}
DebugInfo;


typedef struct
{
   bool selected;
   CText condition;
}
BPList;

typedef struct
{
   CText macroName;
   CText macroPath;
   BPList *list;
   int size;
}
BreakPointInfo;

extern DebugInfo gDebug;
void DebugInitialise(void);
int DebugInterface(Interface* itfc, char args[]);
short DebugProcess(Interface *itfc, Command *cmd);
void  UpdateDebugBreakPointList(EditRegion *ed);
void  UpdateDebugBreakPointList(char *path, char *macro);
Variable* FindDebugBreakPointList(char *path, char *macro);
short AddDebugBreakPoint(int xPos, int yPos, EditRegion *er);
void DrawDebugBreakPointStrip(EditRegion *er);
void DebuggerClose();
void DebuggeeClose();
bool ComparePathNames(char *path1, char* path2);

#endif // define DEBUG_H
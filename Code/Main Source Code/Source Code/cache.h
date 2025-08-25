#ifndef CACHE_H
#define CACHE_H

class Interface;
class Variable;
class CText;

// Prospa command macros
int CacheMacro(Interface* itfc, char args[]);
int CacheProcedures(Interface* itfc, char args[]);
int RemoveCachedMacro(Interface *itfc,  char args[]);
int RemoveCachedMacros(Interface *itfc,  char args[]);
int IsFileCached(Interface* itfc, char args[]);

// Worker macros
void AddProcedures(char *path, char* name);
Variable* GetProcedure(char *path, char *macro, char *name);
void ListGlobalsCachedProcs(CText);

// Global variables
extern Variable gCachedProc; // The global procedure cache

#endif // define CACHE_H
#ifndef LIST_FUNCTIONS_H
#define LIST_FUNCTIONS_H


#include "list2d.h"

// CLI accessible functions
int NewList(Interface* itfc ,char arg[]);
int InsertStringIntoListCLI(Interface* itfc ,char arg[]);
int RemoveStringFromList(Interface* itfc ,char arg[]);
int MakeParameterList(Interface* itfc ,char arg[]);
int AssignParameterList(Interface* itfc ,char arg[]);
int DoesParameterExist(Interface* itfc ,char arg[]);
int FindListIndex(Interface* itfc ,char args[]);
short CompareStrings(char *str1, char* str2, short mode=0);
void SortList(char** list, long size);
void SortList(char** list, long size, short mode, int *indicies);
int RemovePrefix(Interface* itfc ,char args[]);
int ShuffleItem(Interface* itfc ,char args[]);
int GetListNames(Interface* itfc ,char args[]);

// Utility functions
EXPORT extern char**  MakeList(long);
EXPORT extern List2DData* Make2DList(long,long);
EXPORT extern void    FreeList(char**,long);
EXPORT extern char**  CopyList(char **, long);
EXPORT extern char**  MakeListFromText(char*,long*);
EXPORT extern char**  MakeListFromEvaluatedText(Interface *itfc, char*, long*);
EXPORT extern char**  MakeListFromUnquotedText(Interface *itfc, char*, long*);
EXPORT extern List2DData* Make2DListFromEvaluatedText(Interface *itfc, char*);
EXPORT extern List2DData* Make2DListFromUnquotedText(Interface *itfc, char*);
EXPORT extern short   JoinAndOverWriteLists(char***, char **, long, long);
EXPORT extern char**  JoinLists(char**, char**, long, long);
EXPORT extern short   RemoveListEntry(char***, long, long);
EXPORT extern bool    CompareLists(char**, char**, long, long);
EXPORT extern short   ReplaceStringInList(char*, char***, long, long);
EXPORT extern short   ReplaceStringIn2DList(char*,  List2DData*, long, long, long, long);
EXPORT extern short   AppendStringToList(const char* str, char ***list, long listLen);
EXPORT extern int     GetParameterListValue(Interface* itfc ,char arg[]);
EXPORT extern int     SetParameterListValue(Interface* itfc ,char arg[]);
EXPORT extern int     MergeParameterLists(Interface* itfc ,char arg[]);
EXPORT extern int     GetSubParameterList(Interface* itfc ,char arg[]);
EXPORT extern int     SortList(Interface* itfc, char arg[]);
EXPORT extern int     SortList2(Interface* itfc, char arg[]);

EXPORT extern short  InsertStringIntoList(const char* str, char ***list, long listSize, long pos);
EXPORT extern char** MakeEmptyList(long listSize);
EXPORT extern void   ClearList(char ***list, long listSize);
	

#endif // define LIST_FUNCTIONS_H

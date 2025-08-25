#ifndef FILES_H
#define FILES_H

#include <shellapi.h>
#include "defines.h"

class CText;
class Interface;
class Variable;

int GetDirectoryList(Interface* itfc ,char[]);
int GetParentDirectory(Interface *itfc, char args[]);
int FileInfo(Interface* itfc ,char args[]);
float GetFileSize(HANDLE hFile);
float GetFileAge(HANDLE hFile);
int GetFileList(Interface *itfc, char arg[]);
int GetBaseDirectory(Interface *itfc, char args[]);
int ResolveLink(Interface *itfc, char args[]);
int SelectFolder(Interface *itfc, char args[]);
int SelectFolderNoButtons(Interface *itfc, char args[]);

long Load1DRealAsciiDataFromFile(short, char[], long, float**, float**);
long Load1DRealBinaryDataFromFile(char[], long, float**, float**,long,long);
long Load1DDoubleBinaryDataFromFile(char fileName[], long type, double** x, double** y, long header, long length);
long Load1DComplexBinaryDataFromFile(char[], long, complex**, long,long);
long Load1DComplexAsciiDataFromFile(char[], long, complex**);
long Load1DXComplexAsciiDataFromFile(char fileName[], long type, float **x, complex **data);

char* LoadTextFile(Interface*,char*,char*,char*,char*);
char* LoadTextFile(Interface*,char*,char*,char*,char*,long&);
char* LoadTextFileFromFolder(char*,char*,char*);
short SaveTextFile(char*,char*,char*);
bool IsProcedure(char* path, char *name, char *procName);

UINT CentreHookProc(HWND,UINT,WPARAM,LPARAM);
UINT CentreHookSaveProc(HWND,UINT,WPARAM,LPARAM);
UINT LoadDataHookProc(HWND,UINT,WPARAM,LPARAM);
UINT SaveDataHookProc(HWND,UINT,WPARAM,LPARAM);

// Path and file name handling
//short AddExtension(char*,char*,char*);
short AddExtension(char*,char*);
void ExtractFileNames(char*, char*, char*);
void RemoveExtension(char*);
char *GetExtension(char*);
void GetExtension(char*,char*);
char* GetExtension(CText &file);
void GetExtension(CText &file, CText &extension);
bool IsDirectory(char *dir);
bool IsFile(char *file);
bool IsReadOnly(char *file);
bool IsReadOnly(char *pathName, char *file);
void GetCurrentDirectory(CText &dir);
bool SetCurrentDirectory(CText &folder);
void GetDirectory(CText &dir);
bool SetDirectory(CText &dir);
int GetBasePath(Interface *itfc, char args[]);
void GetFileNameFromPath(char *fullPath, char *fileName);
bool CompareDirectories(char* folder1, char* folder2);
bool CompareDirectories(CText &folder1, CText &folder2);
bool IsValidFileName(char *str);
bool IsValidPathName(char *str);

short GetDropFileInfo(HDROP hDrop, CText &path, CText &file, CText &ext, int which);
short GetNumberOfDroppedFiles(HDROP hDrop);

extern UINT APIENTRY LoadSaveHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);



 
#endif // define FILES_H

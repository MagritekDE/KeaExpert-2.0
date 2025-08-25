#ifndef IMPORTEXPORT_UTILITIES_H
#define IMPORTEXPORT_UTILITIES_H

#define SAVE_DATA    1
#define DISPLAY_DATA 2

class Interface;

bool IsPlacesBarHidden(void);
long  DetermineDataType(char*, char*, char*, char*, char*, char*);
short FileDialog(HWND, bool, char*, char*, char*, 
					  UINT  (*)(HWND,UINT,WPARAM,LPARAM),
					  char*,long,short,short*, ...);
short FileDialog2(HWND hWnd, bool open, char *pathName, char *fileName, 
                   char *title, 
                   UINT (*hookProc)(HWND,UINT,WPARAM,LPARAM),
                   char *hookDLG, long flags, short *index, char *filterTitles, char *filterExt);
short ExtractImportExportArguments(Interface*, char*,short,char*,char*,char*,char*,char*,char*,long&,long&);
void  InitialiseImportExportPrompts(long,char*,char*,char*,char*,char*);

int CALLBACK ImportDlgProc(HWND, UINT, WPARAM, LPARAM);
int CALLBACK ExportDlgProc(HWND, UINT, WPARAM, LPARAM);

#endif // define IMPORTEXPORT_UTILITIES_H
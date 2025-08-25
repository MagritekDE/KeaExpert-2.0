#ifndef IMPORT_EXPORT_H
#define IMPORT_EXPORT_H

#define SAVE_DATA    1
#define DISPLAY_DATA 2

bool IsPlacesBarHidden(void);
long  DetermineDataType(char*, char*, char*, char*, char*);

extern short Import1DData(Interface *itfc, bool, char[], long, char*, char*, Variable*, Variable*,long);
extern short Import2DData(Interface *itfc, short, char[], long, char*, long, long, bool);
extern short Import3DData(Interface *itfc, short, char[], long, char*, long, long, long, bool);

extern short Export1DData(Interface*, char[], long, char*, char*);
extern short Export2DData(Interface*, char[], long, char*);
extern short Export3DData(Interface*, char[], long, char*);

extern short Import1DDataDialog(HWND,char*,char*);
extern short Import2DDataDialog(HWND,char*,char*);
extern short Import3DDataDialog(HWND, char*, char*);

extern short Export1DDataDialog(HWND,char*,char*);
extern short Export2DDataDialog(HWND,char*,char*);
extern short Export3DDataDialog(HWND,char*,char*);

extern int   Import1DDataCLI(Interface* itfc ,char args[]);
extern int   Import2DDataCLI(Interface* itfc ,char args[]);
extern int   Import3DDataCLI(Interface* itfc ,char args[]);

extern int   Export1DDataCLI(Interface* itfc ,char args[]);
extern int   Export2DDataCLI(Interface* itfc ,char args[]);
extern int   Export3DDataCLI(Interface* itfc ,char args[]);

extern int   Import3DDataParameters(Interface* itfc ,char args[]);

extern int   Export2DDataParameters(Interface* itfc ,char args[]);
extern int   Export3DDataParameters(Interface* itfc ,char args[]);

extern int   CALLBACK ImportDlgProc(HWND, UINT, WPARAM, LPARAM);
extern int   CALLBACK ExportDlgProc(HWND, UINT, WPARAM, LPARAM);

long CalcExpectedSize(long type, long xsize,long ysize, long zsize, long fileHeader, long rowHeader);
short FileDialog(HWND, bool, char*, char*, char*, 
								UINT  (*)(HWND,UINT,WPARAM,LPARAM),
								char*,long,short,short*, ...);
#endif // define IMPORT_EXPORT_H
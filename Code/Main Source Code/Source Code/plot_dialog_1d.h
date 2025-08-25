#ifndef PLOT_DIALOG_1D_H
#define PLOT_DIALOG_1D_H

#include "commdlg.h"
#include "defines.h"

class Plot;

UINT APIENTRY XYLabelHookProc(HWND,UINT,WPARAM,LPARAM);;
UINT APIENTRY TitleHookProc(HWND,UINT,WPARAM,LPARAM);
void InitFont(LPCFHOOKPROC, LPCTSTR, HWND, CHOOSEFONT *, LOGFONT *, COLORREF);
UINT APIENTRY FontHookProc(HWND,UINT,WPARAM,LPARAM);

void SetDialogPosition(short dialog, short x, short y);
void GetDialogPosition(short dialog, short &x, short &y);

void UpdateAxesDialog(Plot*);
void UpdateRegionDialog(Plot*);

short GetFontStyle(LOGFONT *lf);

extern char fontDltitle[MAX_STR];
extern short gMultiPlotXCells;
extern short gMultiPlotYCells;
extern short gMultiEditXCells;
extern short gMultiEditYCells;

int CALLBACK CopyTo1DVarDlgProc (HWND, UINT, WPARAM, LPARAM);
int CALLBACK Paste1DDataDlgProc (HWND, UINT, WPARAM, LPARAM);
int CALLBACK Paste2DDataDlgProc (HWND, UINT, WPARAM, LPARAM);
int CALLBACK AboutDlgProc (HWND, UINT, WPARAM, LPARAM);
int CALLBACK ImportDlgProc(HWND, UINT, WPARAM, LPARAM);
int CALLBACK ExportDlgProc(HWND, UINT, WPARAM, LPARAM);
int CALLBACK PrintDlgProc(HWND, UINT, WPARAM, LPARAM);
int CALLBACK MultiPlotDlgProc(HWND, UINT, WPARAM, LPARAM);
int CALLBACK MultiEditDlgProc(HWND, UINT, WPARAM, LPARAM);
int CALLBACK EMFDlgProc(HWND, UINT, WPARAM, LPARAM);

extern HWND gRangeHwnd;
extern HWND gAxesHwnd;
extern HWND gColorHwnd,gSymbolHwnd;
extern HWND gImportHwnd;
extern HWND gGridHwnd;
extern HWND gPrintHwnd;
extern HWND gTextHwnd;
extern HWND gEMFHwnd;

#endif //define PLOT_DIALOG_1D_H
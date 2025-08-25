#ifndef SYNTAX_VIEWER_H
#define SYNTAX_VIEWER_H

class WinData;
class EditRegion;

extern LRESULT CALLBACK SyntaxEventsProc(HWND, UINT, WPARAM, LPARAM);
extern void UpDateCLISyntax(WinData *win, HWND syntaxWin, long startSel, long endSel, bool showProcs=false);
extern void UpDateEditSyntax(WinData *win, EditRegion *edReg, long startSel, long endSel, bool showProcs=false);
extern void UpDateEditSyntax2(WinData *win, EditRegion *edReg, long startSel, long endSel);

#endif // define SYNTAX_VIEWER_H
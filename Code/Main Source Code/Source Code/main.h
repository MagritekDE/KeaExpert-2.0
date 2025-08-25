#ifndef MAIN_H
#define MAIN_H

#define WM_USER_LOADDATA WM_APP

class Interface;
class Variable;

extern bool isModelessDialog;
extern bool prospaResizing;
extern long mainThreadID;

extern char AppName[];

extern HBITMAP iconBMPs[];

extern HINSTANCE prospaInstance;

extern HFONT numberFont;

extern HWND currentAppWindow;
extern HACCEL currentAccel;
extern HACCEL hAccelPlot1D;
extern HACCEL hAccelPlot2D;
extern HACCEL hAccelPlot3D;
extern HACCEL hAccelCLI; 
extern HACCEL hAccelEdit;
extern HACCEL hAccelUser;

extern char applicationHomeDir[];
extern char gCurrentDir[];
extern char userPrefDir[];

extern HFONT cliFont;
extern HFONT editFont;
extern HFONT controlFont;
extern short editFontSize;

extern Variable *userMenusVar;
extern Variable *macroPathVar;
extern Variable *dllPathVar;
extern Variable *prospaPrefVar;
extern Variable *userWorkingVar;
extern Variable *userMacroVar;
extern Variable *prospaTempVar;

extern FloatRect2 plot1DMinRect;
extern FloatRect2 plot2DMinRect;
extern FloatRect2 plot3DMinRect;
extern FloatRect2 cliMinRect;
extern FloatRect2 editMinRect;
extern short resizableWinBorderSize;
extern short fixedSizeWinBorderSize;
extern short titleBarNMenuHeight;
extern short titleBarHeight;
extern float NaN,Inf;


void ResizeWindowsToParent(void);
void CalculateWindowRect(HWND hWnd, FloatRect2 *wRect);
void LimitMovingRect(HWND hWnd, WPARAM wParam, LPARAM lParam);
void LimitSizingRect(HWND hWnd, WPARAM wParam, LPARAM lParam);
void GetMaximisedSize(HWND win, long &x, long &y, long &w, long &h, bool correct);
void AppendUserMenu(HMENU winMenu);
int CloseApplication(Interface* itfc, char[]);
void LoadUserData(WPARAM wParam);
int AddMenuMacros(HMENU menu, UINT &cnt);
int AddMenuFolders(HMENU menu, UINT &cnt);
int ShowNextWindow(Interface* itfc, char args[]);
int ShowLastWindow(Interface* itfc, char args[]);
int  UpdateMainMenu(Interface* itfc, char args[]);
void UpdateMainMenu(HWND hWnd);
int LeakTest(Interface *itfc, char *args);
int ProspaFunctions(Interface* itfc ,char args[]);



#endif // define MAIN_H
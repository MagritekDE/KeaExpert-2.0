#ifndef GUIWINDOWSEVENTS_H
#define GUIWINDOWSEVENTS_H

#include "defines.h"

#define DIAG_RESIZE  0
#define HORIZ_RESIZE 1
#define VERT_RESIZE  2

class Interface;
class ObjectData;
class WinData;

extern WinData *messageSource;    // window which produced the message

#define NOTHING 0
#define MAKE    1
#define RESIZE  2
#define MOVE    3
#define SELECT  4
#define MOVING_OVER_OBJECT 5

#define GUI_IDLE  0
#define GUI_CLICK 1
#define GUI_PASTE 2
#define GUI_CUT   3

LRESULT CALLBACK  GridEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  ButtonEventProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK  DebugStripEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  EdLineProc(HWND, UINT, WPARAM, LPARAM);  // FIXME: Not defined anywhere
LRESULT CALLBACK  GroupBoxEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  PanelEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  PanelScrollEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  HTMLEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  GridEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  PictureEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  ProgressBarEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  SliderEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  StatusBarEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  STextEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  SText2EventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  TabCtrlEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  TextBoxEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  PanelWinEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  TextMenuEditEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  TextMenuEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  UpDownEventProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK  UpDownEventProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK  UserWinEventsProc(HWND, UINT, WPARAM, LPARAM);

EXPORT short SendMessageToGUI(char*,short);


HRGN AddRectToRgn(HRGN hRgn, int x1, int y1, int x2, int y2);
void ControlVisibleWithTabs(WinData *win, ObjectData *tab);
WinData* GetGUIWin(void);
void RestoreCurrentWindows(HWND);
int SendGUIMessage(Interface* itfc ,char arg[]);
int SaveGUIWindowImage(Interface* itfc ,char args[]);
int ConvertColor(Interface* itfc ,char args[]);
int PreProcessCommand(LPARAM lParam, WPARAM wParam, HWND hWnd, WinData *win);
void DisplayControlInfo(WinData *win, ObjectData *obj);
void GetWindowTextEx(HWND hWnd, CText &txt);
void WaitButtonUp(HWND hWnd);
short GetButtonDown(HWND hWnd, short &x, short &y);
bool IsKeyDown(int key);
WinData *GetParentWin();
void SetParentWin(WinData*);
CommandInfo* GetErrorInfo(void);
CommandInfo* GetCmdInfo(void);

#endif // define GUIWINDOWSEVENTS_H
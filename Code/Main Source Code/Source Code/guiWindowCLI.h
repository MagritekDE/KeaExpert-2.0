#ifndef GUIWINDOWCLI_H
#define GUIWINDOWCLI_H

class Interface;
class ObjectData;
class WinData;

int GetWindowParameter(Interface* itfc, char[]);
int SetWindowParameter(Interface *itfc, char args[]);
int DestroyMyWindow(Interface* itfc ,char args[]);
int HideMyWindow(Interface* itfc ,char arg[]);
int ShowMyWindow(Interface* itfc ,char args[]);
int NextWindowNumber(Interface* itfc, char args[]);
int KeepWindowOnTop(Interface* itfc ,char arg[]);
int KeepFocus(Interface* itfc ,char args[]);
int GetControlValues(Interface *itfc, char args[]);
int SetControlValues(Interface *itfc, char arg[]);
int DrawWindow(Interface* itfc ,char args[]);
int GUIWindowNumber(Interface* itfc, char[]);
int SetWindowFocus(Interface *itfc, char args[]);
int CompleteWindow(Interface* itfc ,char args[]);
int GetObject(Interface* itfc ,char args[]);
int AssignControlObjects(Interface *itfc, char args[]);
int GetParentObject(Interface *itfc, char *args);
int ListControls(Interface* itfc ,char args[]);
int ListWindows(Interface* itfc ,char args[]);
int GetCtrlFocus(Interface* itfc ,char args[]);
int GetMenuName(Interface *itfc, char args[]);
short GetCurWinNr(Interface *itfc);
WinData *GetWinData(HWND hWnd);
ObjectData *GetObjData(HWND hWnd);
int FindWindowCLI(Interface *itfc, char args[]);
int CheckControlValues(Interface* itfc, char args[]);
bool FindGUIWindow(const char *name);
int GetOrSetCurrentWindow(Interface *itfc, char args[]);

#endif // define GUIWINDOWCLI_H
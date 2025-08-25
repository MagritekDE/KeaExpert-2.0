#ifndef DIALOG_UTILITIES_H
#define DIALOG_UTILITIES_H

#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)

void InitialiseDialogList(void);
void ShowDiagWindows(void);
void HideDiagWindows(void);
void RemoveFromDiagList(HWND key, bool usekey);
void AddToDiagList(HWND win);
void AddToDiagList(HWND key, HWND win);
BOOL MyEndDialog(HWND hWnd,int value);
void DialogTidyUp(HWND hWnd, bool subWin);
void PlaceDialogOnTopInCentre(HWND hWnd);
void PlaceDialogOnTopInCentre(HWND hWnd, HWND subWin);
void PlaceDialogAtPosition(HWND hWnd, short x, short y);
void EnableDialogs(bool enable);
void SaveDialogParent(HWND hWnd);
bool RestoreDialogParent(HWND hWnd);
void ChangeDialogParent(HWND newParent);

// dialog hook procedures

// GUI
//extern int CALLBACK EnterObjectInfoDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Editor
//extern int CALLBACK LineNumberDlgProc(HWND , UINT , WPARAM , LPARAM );

#endif // define DIALOG_UTILITIES_H
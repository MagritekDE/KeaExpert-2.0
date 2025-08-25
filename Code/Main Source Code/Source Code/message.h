#ifndef MESSAGE_H
#define MESSAGE_H

class Interface;

int YesNoDialog(long, short, char[],char[],...);
int YesNoDialogExt(long icon, short def, char title[],char text[]);
int YesNoCancelDialogExt(long icon, short def, char title[],char text[]);
int QueryDialog(long, char[],char[],...);
int QueryDialogExt(long, char[],char[]);
void MessageDialog(HWND,long,char[],char[],...);
void MessageDialogExt(HWND win,long icon, char title[],char text[]);
int MsgBoxEx(HWND hwnd, TCHAR *szText, TCHAR *szCaption, UINT uType);
int DisplayMessage(Interface* itfc ,char args[]);
int YesNoMessage(Interface* itfc, char args[]);

#endif // define MESSAGE_H
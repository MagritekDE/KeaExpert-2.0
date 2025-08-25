#ifndef EVENTS_EDIT_H
#define EVENTS_EDIT_H

class EditParent;
class Interface;
class WinData;

void  GetSubWindowRect(HWND parent,HWND child, RECT *r);
LRESULT CALLBACK EditEventsProc(HWND, UINT, WPARAM, LPARAM);

short SaveEditDialog(HWND, char*, char*);
void SetEditTitle(void);
int LoadEditor(Interface* itfc ,char args[]);
int LoadEditorGivenPath(char[], char[]);
int Clear(Interface* itfc ,char args[]);
void IndentText(short mode);
void  MakeEditWindows(bool, EditParent* parent, short rows, short cols, short saveThisOne);
void  AddProcedureNames(WinData *win, HMENU menu);
void UnIndentText(short mode);
void BlockComment(short mode);
void BlockUncomment(short mode);
int EditorFunctions(Interface* itfc ,char args[]);

#endif // define EVENTS_EDIT_H
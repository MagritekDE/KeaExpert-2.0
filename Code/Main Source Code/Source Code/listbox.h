#ifndef LISTBOX_H
#define LISTBOX_H

class Interface;
class ObjectData;
class CText;
class Variable;
class WinData;

extern WNDPROC OldListBoxProc;
void ListBoxCallBack(WinData *win, ObjectData *obj);

void MakeListBoxObject(ObjectData *obj, long x, long y, long w, long h, DWORD visibility);
int GetListBoxParameter(Interface *itfc, ObjectData *obj, CText &parameter, Variable *ans);
int SetListBoxParameter(ObjectData *obj, CText &parameter, Variable *value);
int DrawListBoxItem(ObjectData *obj, LPDRAWITEMSTRUCT pdis);

#endif // define LISTBOX_H
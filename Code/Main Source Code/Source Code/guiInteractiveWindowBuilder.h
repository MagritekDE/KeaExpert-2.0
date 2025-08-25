#ifndef GUIINTERACTIVEWINDOWBUILDER_H
#define GUIINTERACTIVEWINDOWBUILDER_H

class Interface;
class ObjectData;
class WinData;

extern short interactiveMode;
extern WinData *editGUIWindow;  // Window currently being edited

void  ActivateEditedGUIWin(void);
int ActivateWindow(Interface* itfc ,char args[]);
int AlignObjects(Interface* itfc ,char args[]);
bool AreObjectsCopied(void);
int AttachObjects(Interface* itfc ,char args[]);
void CopySelectedObjects(WinData *win, RECT *r);
void CutSelectedObjects(WinData *win, RECT *r);
int DistributeObjects(Interface* itfc ,char args[]);
int EditObject(Interface *itfc, char *args);
int MakeObjectInteractively(Interface* itfc ,char args[]);
int MoveObjectsInteractively(short,WinData*, HDC, ObjectData*, short&, short&, short, short);
void PasteSelectedObjects(WinData *win, short x, short y, RECT *r, short mode);
int ResizeObjects(Interface* itfc ,char arg[]);
int ResizeObjectsInteractively(short, WinData*, HDC, ObjectData*, short, short, short, short);
int SaveWindowLayout(Interface* itfc ,char args[]);
int SelectObjInteractively(Interface* itfc ,char arg[]);

#endif // define GUIINTERACTIVEWINDOWBUILDER_H
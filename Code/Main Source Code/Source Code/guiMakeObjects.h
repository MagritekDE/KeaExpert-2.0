#ifndef GUIMAKEOBJECTS_H
#define GUIMAKEOBJECTS_H

class Interface;
class ObjectData;

extern bool activeEditWin; // Whether to activate the edited window when a new window is made

bool AnyGUIWindowVisible(void);
void HideGUIWindows(void);
void ShowGUIWindows(int);

void ChangeGUIParent(HWND newParent);
void SetCurrentGUIParent(void);
void UpdateCurrentGUIParent(HWND parent);

int CloseDialog(Interface* itfc ,char args[]);
int ShowDialog(Interface* itfc ,char args[]);

void DisableAllWindows(HWND);
void EnableAllWindows(HWND);

int DrawObject(Interface* itfc ,char args[]);
int RemoveObject(Interface* itfc ,char args[]);
int ShowObjects(Interface* itfc ,char args[]);

int GetEditedWindow(Interface* itfc ,char args[]);
int SetEditableWindow(Interface *itfc, char arg[]);

bool IsAProspaWindow(HWND hwnd);

int MakeToolBar(Interface* itfc ,char arg[]);
int MakeToolBarWithKeys(Interface* itfc ,char arg[]);
int Make3DWindow(Interface* itfc ,char args[]);
int MakeButton(Interface* itfc ,char args[]);
int MakeCheckBox(Interface* itfc ,char args[]);
int MakeCLI(Interface* itfc ,char args[]);
int MakeColorBox(Interface* itfc ,char args[]);
int MakeComboBox(Interface* itfc ,char args[]);
int MakeDebugStrip(Interface* itfc ,char args[]);
int MakeDivider(Interface* itfc ,char args[]);
int MakeGetMessage(Interface* itfc ,char args[]);
int MakeGroupBox(Interface* itfc ,char args[]);
int MakeHTMLBox(Interface* itfc ,char args[]);
int MakeGridCtrl(Interface* itfc ,char args[]);
int MakeImageWindow(Interface* itfc ,char args[]);
int MakeListBox(Interface* itfc ,char args[]);
int MakeMenu(Interface* itfc ,char args[]);
int MakeMenuWithKeys(Interface* itfc ,char args[]);
int MakePanel(Interface* itfc ,char args[]);
int MakePicture(Interface* itfc ,char args[]);
int MakePlotWindow(Interface* itfc ,char args[]);
int MakeProgressBar(Interface* itfc ,char args[]);
int MakeRadioButtons(Interface* itfc ,char args[]);
int MakeSlider(Interface* itfc ,char args[]);
int MakeStaticText(Interface* itfc ,char args[]);
int MakeStatusBox(Interface* itfc ,char args[]);
int MakeTabCtrl(Interface* itfc ,char args[]);
int MakeTextBox(Interface* itfc ,char arg[]);
int MakeTextEditor(Interface* itfc ,char arg[]);
int MakeToolBar(Interface* itfc ,char arg[]);
int MakeUpDown(Interface* itfc ,char arg[]);
int MakeWindow(Interface* itfc ,char arg[]);

int ReplaceRadioButtonObject(Interface* itfc ,ObjectData *obj, char args[]);
int ReplaceUpDownObject(Interface* itfc ,ObjectData *obj, char args[]);

#endif // define GUIMAKEOBJECTS_H

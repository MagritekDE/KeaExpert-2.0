#ifndef GUIWINDOWCLASS_H
#define GUIWINDOWCLASS_H

class ObjectData;
class Variable;

#include "guiObjectClass.h"
#include "variablesClass.h"
#include "edit_class.h"
#include "WidgetRegistry.h"
#include <string>

#define TAB_BY_CTRL_NUMBER 1
#define TAB_BY_TAB_NUMBER  2

#pragma pack(push,8) // Must not use 1 byte packing in USB device structure
#include <dbt.h>
#pragma pack(pop)

typedef struct
{
   long minWidth;
   long maxWidth;
   long minHeight;
   long maxHeight;
}
SizeLimits;

class WinData
{
	public:
		WinData();
		~WinData();
      long validationCode;  // Should be first member
		WinData *GetNextWin(void);
		WinData *GetPreviousWin(void);
		WinData *GetLastWin(void);
		void SelectAllObjects(bool);
		void SelectObjects(short,short,short,short);
      void ResetControlNumbers();
      void ResetTabNumbers();
		WinData* AddWin(int,char*,int,int,int,int,bool,char*);
		void RemoveWin(char*);
		void MoveSelectedObjects(char,short);
		ObjectData *FindStatusBox(void);
		ObjectData *FindObjectByValueID(const char* const);
		ObjectData *FindObjectByObjectID(const char* const);
		ObjectData *FindObjectByNr(short);
		ObjectData *FindObjectByType(short);
		ObjectData* FindTabObject(short x, short y);
      void GetSelectedObjects(Variable& selObjVec);
		void MoveSelectedObjects(short,short);
		void ResizeSelectedObjects(short,short);
		void ResizeSelectedObjects(char,short);
		void EnableObjects(bool);
		void EnableResizing(bool);
		void DeselectObjects(void);
		void DeleteSelectedObjects(void);
		void AlignSelectedObjects(ObjectData*, short);
		void DistributeSelectedObjects(short);
		void EvenlySpaceSelectedObjects(short);
      void DistributeInsideObject(short);	
      void AddProcedures(char*,char*);
		short FindNextFreeNr(void);
      void RemoveFocusRectangle(void);
      short ProcessClassProc(Interface *itfc, char *cmd, char *args);
		WinData *FindWinByName(char*);
      WinData* FindWinByTitle(char*);
      WinData* FindWinByPartialTitle(char*);
		short FindNextFreeObjNr(void);
		WinData *FindWinByNr(short);
		ObjectData *FindObject(short,short);
     ObjectData* FindObjectIfInside(short x, short y);
		ObjectData *InDivider(short,short);
		WinData *FindWinByHWND(HWND);
      WinData *FindWinByPosition(POINT p);
      Variable *GetProcedure(char*,char*,char*);
      int GetNextFileNumber(ObjectData *curObj, CText &name);
      void AttachSelectedObjects(short dir);
      void SetTitle(char *title);
      void CopyAllObjects();
      bool AreObjectsSelected(void);
      void UndoObjectAction(void);
		char *GetTitle(void);
      void GetWindowText(CText&);
		char *GetObjCommand(int);
		short CountObjects(void);
		short CountSelectedObjects(void);
      void MoveObjectToStart(ObjectData *srcObj);
      void UpdateGUIObjectPositions(int ww, int wh);
      void ValidateObjRects(void);
      void InvalidateObjRects(void);
      float *GetControlList(int *sz);
		bool isAlive() {return alive_;}
		bool drawing;
		void alive(bool living) {this->alive_ = living;}

		void activate();
		/**
		* Make this window editable
		*/
		void makeEditable();
		short updateControlVisibilityFromTabs();

		void Destroy();
		// Generate a string representing the state of this object's parameter-
		//  accessible attributes.

		/**
		*	Restore the window to its original size after being maximised.
		*	Only applies to contrained sub-windows
		*
		*	If user is using dual displays then if the maximised window is in the 
		*	middle then maximising and minimising can cause a flash since the 
		*	GetWindowPlacement/SetWindowPlacement commands seems to display the maximised
		*	position momentarily before restoring it and then updated position
		*	is incorrect. By setting it to -10000 the flash is off screen.
		*/
		void restoreFromMaximised();

		/**
		* Check or uncheck a menu item defined by a menu name and menu key
		*/
		short setMenuItemCheck(const char* const menuName, const char* const menuKey, bool check);

		/**
		* Enable or disable a menu item defined by a menu name and menu key
		*/
		short setMenuItemEnable(const char* const menuName, const char* const menuKey, bool enable);

		/**
		*	Check or uncheck a toolbar button defined by a toolbar name and button key
		*/
		short setToolBarItemCheck(const char* const toolbarName, const char* const butKey, bool check);

		static void SetGUIWin(WinData* newGUI);

		HRGN CreateBgrdRegion();

		std::string FormatState();
		char macroName[MAX_STR];
		char macroPath[MAX_PATH];
		char exitProcName[MAX_NAME];
		WinData *next;
		WinData *last;
		WidgetRegistry widgets;
		Variable winVar;              // Provides class access to varList
		Variable varList;
   	RECT *rect;
  		char *name;                   // Window name
  		char *title;                  // Title text
  		short type;                   // NORMAL, DIALOG
  		short nr;
  		short wx,wy,ww,wh;            // Window dimensions
      float xSzScale;              // Size information as a number
      float xSzOffset;
      float ySzScale;
      float ySzOffset;
      float wSzScale;
      float wSzOffset;
      float hSzScale;
      float hSzOffset;
      short objectNrCnt;            // Used for renumbering objects
  		bool activated;
      bool fixedObjects;            // Consider toolbar and statusbar when calculating object positions
  		short defaultID;              // ID of default button
  		short cancelID;               // ID of cancel button
  		short abortID;                // ID of abort button
  		short panicID;                // ID of panic button
  		bool activeCtrl;              // Are there active controls in the window
  		bool inRemoval;               // Indicates window is being removed
      bool resizeable;              // Resizable or not
      bool permanent;               // Can window be deleted before parent
      bool constrained;             // Limited to parent rectangle or not
      bool displayObjCtrlNrs;       // Draw object numbers next to controls?
      bool displayObjTabNrs;        // Draw tab numbers next to controls?
      bool modifyingCtrlNrs;        // Are we modifying control numbers interactively?
      bool modifyingTabNrs;         // Are we modifying tab numbers interactively?
      bool keepInFront;             // Keep this window in front of normal windows?
      bool showGrid;                // Show an overlay grid
      bool snapToGrid;              // Snap object to grid when moving them
      bool blockNonActiveCtrl;      // prevent non-active controls from working
      short gridSpacing;            // Spacing between grid lines
      short tabMode;
      ObjectData* objWithFocus;           // Object with window focus.
      bool showMenu;                // Show a contextual menu
      bool userdef;                 // User defined boolean
      bool isMainWindow;            // Whether this is the main prospa window or not
      bool debugger;                // Is the debug window?
      bool debugging;               // Is this window being debugged?
      bool cacheProc;               // Are procedures in this window cached?
      bool visible;                 // Is this window visible?
      int threadCnt;                // How many threads are running in this window?
  		HWND hWnd;
      HWND parent;                  // Parent window (GUI window will be above this one)
	   Variable procList;            // Procedure list for window
      WinData* oldParent;           // Previous parent GUI window for current macro
      WinData* oldGUI;              // Previous current GUI window
      WidgetRegistry undoArray;     // Undo array for objects
      HACCEL hAccelTable;           // Accelerator table
      ObjectData *toolbar;          // Current toolbar object
      ObjectData *statusbox;        // Current status box object
      ObjectData *defaultToolbar;   // Toolbar object when window titlebar has focus
      ObjectData *defaultStatusbox; // Status box object when window titlebar has focus
      ObjectData *menuObj;          // Object which contains current window menu
      UINT nrMenuObjs;              // Number of menu objects in window
      HMENU bkgMenu;                // Background menu for window
      int bkgMenuNr;                // Number of background menu
      short *menuList;              // List of menu object numbers for this window
      short menuListSize;           // Size of menu list
      HMENU menu;                   // Window menu to use if window has focus
      HACCEL accelTable;            // Accelerator table to use if window has focus
      HWND blankStatusBar;
      HWND blankToolBar;
      SizeLimits sizeLimits;        // Size limit structure for this window
		bool titleUpdate;             // Whether window title should be updated when a control is selected
		bool mergeTitle;              // Whether window title can be merged with control information
      COLORREF bkgColor;
      CText dragNDropProc;          // Name of drag and drop procedure
		bool operator< (const WinData &rhs) const {return nr < rhs.nr;}
		EditRegion *currentEditor;    // The current editor (if it exists)
		HDEVNOTIFY _hNotifyDevNode;   // USB device notification handle
		DEV_BROADCAST_DEVICEINTERFACE *devIF; // USB device notification filter info
private:
		bool alive_;
};

 typedef struct
 {
    WinData *curGUIWin;
    WinData *parentWin;
	 CommandInfo errInfo;
	 CommandInfo curCmdInfo;
 }
 ThreadGlobals;

extern WinData *rootWin;


extern bool dialogDisplayed; // Whether the window is a dialog or not

#define NORMAL_WIN 1
#define DIALOG_WIN 2

#endif // define GUIWINDOWCLASS_H
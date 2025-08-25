#ifndef GUIOBJECTCLASS_H
#define GUIOBJECTCLASS_H

#include <commctrl.h>
#include "defines.h"
#include <string>
#include "variablesClass.h"

#define BUTTON         1
#define GROUP_BOX      2
#define TEXTBOX        3
#define CHECKBOX       4
#define RADIO_BUTTON   5
#define STATICTEXT     6
#define SLIDER         7
#define PROGRESSBAR    8
#define STATUSBOX      9
#define TEXTMENU      10
#define COLORSCALE    11
#define STATICTEXT2   12
#define GETMESSAGE    13
#define LISTBOX       14
#define COLORBOX      15
#define HTMLBOX       16
#define MENU          17
#define PLOTWINDOW    18
#define IMAGEWINDOW   19
#define TOOLBAR       20
#define OPENGLWINDOW  21
#define TEXTEDITOR    22
#define CLIWINDOW     23
#define DIVIDER       24
#define UPDOWN        25
#define PICTURE       26
#define DEBUGSTRIP    27
#define TABCTRL       28
#define GRIDCTRL		 29
#define PANEL		    30

extern WNDPROC OldButtonProc;
extern WNDPROC OldCLIProc;
extern WNDPROC OldPictureProc;
extern WNDPROC OldUpDownProc;
extern WNDPROC OldTabCtrlProc;
extern WNDPROC OldDebugStripProc;
extern WNDPROC OldHTMLProc;
extern WNDPROC OldGridProc;
extern WNDPROC OldSliderProc;
extern WNDPROC OldStatusBarProc;
extern WNDPROC OldProgressBarProc;
extern WNDPROC OldTextBoxProc;
extern WNDPROC OldTextMenuEditProc;
extern WNDPROC OldGroupBoxProc;
extern WNDPROC OldPanelProc;
extern WNDPROC OldPanelScrollProc;
extern WNDPROC OldTextMenuProc;
extern WNDPROC OldStaticTextProc;
extern WNDPROC OldStatic2TextProc;
extern DWORD g_objVisibility;


#define NO_TAB_NUMBER -1
#define NO_TAB_PAGE -1

void GetSizeExp(char *dir, CText *txt, float scale, float offset,bool quote);


class CText;
class WinData;
class EditParent;
class Interface;

class ObjectData
{
	public:
		ObjectData();
		ObjectData(WinData *parent, short type, short objNr, 
                      long x, long y, long w, long h, 
                      char *label, char *cmd, char *objInfo, DWORD visibility, long lineNr);
		~ObjectData();
      long validationCode;  // Should be first member
		ObjectData *Copy(HWND);
		void EnableObject(bool);
		bool IsTabbable();
      short DefaultProc(Interface *itfc, char*);
      short ProcessClassProc(Interface *itfc, char *cmd, char *args);
      bool IsAWindow(HWND);
      HWND GetCurrentWindow(void);
      void GetRectExp(CText *x, CText *y, CText *w, CText *h, bool quote);
      void GetXExp(CText *x, bool quote);
      void GetYExp(CText *y, bool quote);
      void GetWExp(CText *w, bool quote);
      void GetHExp(CText *h, bool quote);
      void MoveWindowToTop();
		void DrawRect(HDC);
		void DrawFocusRect(HDC,bool,HWND=0);
      void DrawSelectRect(HDC);
      void DrawErrorRect(HWND);
		void DrawControlNumber(HDC);
		void DrawTabNumber(HDC);
		void GetSize(short&,short&);
      char* GetToolTip(void);
		void GetWindowText(CText&);
		void GetPosition(short&,short&);
		void FreeData(void);
		void RemoveAll(void);     
      void Show(bool);
      void Move(long x, long y, long w, long h, bool redraw = true);
		const std::string FormatState();
		static const char* const GetTypeAsString(short type);
      short GetTypeAsNumber(CText &type);
		bool isTabParent();
		bool isTabChild();
		bool isTabChildOf(ObjectData* parent);
		ObjectData* getTabParent();
		bool isSelected();
		void setSelected(bool selected);
		char* getObjectName();
		bool hasDefaultObjectName();
		bool hasDefaultValueName();

      // Panel functions
      short UpdatePanelFromScrollBar(long);
      short SetPanelParent(short);
      short ProcessPanelScrollEvent(WPARAM wParam,LPARAM lParam);
      short UpdatePanelStruct(short mode);
      short UpdatePanelThumb(bool recalcObjPos);

		short nr();
		void nr(short nr);
		void resetControlNumber();
		void resetTabNumber();
		void selectIfWithinRect(FloatRect* r);

		static bool compareCtrlNumbers(ObjectData* o1, ObjectData* o2);
		
		// Predicates
		bool nrEquals(int nr){return nr_ == nr;}
		bool menuNrEquals(HMENU menuNr);
		bool sequentialMenuNrEquals(int mNr) {return seqMenuNr == mNr;}
		bool valueNameEquals(const char* const name) {return !strcmp(valueName,name);}
		bool objectNameEquals(const char* const name) {return !strcmp(objName,name);}
		bool typeEquals(short in_type){return type == in_type;}
		bool winEquals(HWND win); 

		short Place(short x, short y, short w, short h, bool updateSz);
		
		HRGN CreateTabBkgRegion();
		/**
		*  Collect the region which surrounds all groupbox drawables
		*/
		HRGN GetGroupBoxRegion();
      /**
		*  Collect the region which surrounds all panel edges
		*/
		HRGN GetPanelRegion();

		/**
		*	A tab button has been pressed while an object is selected.
		*	Move to the next (or last) tabbable object. If this is 
		*	a radio button then draw the focus rect, if its a textbox
		*	then select the text
		*/
		void processTab();

		/**
		* Force this object to redraw
		*/
		void invalidate();
		/**
		* Force this object to clear
		*/
		void ClearTabRegion();
		/**
		* Override routine for groupbox 
		*/
		int DrawGroupBox(LPDRAWITEMSTRUCT pdis);

		/**
		* Override routine for panel 
		*/
		int DrawPanel();

		/**
		* Clear the groupbox
		*/
		void EraseGroupBox();

		/**
		* Clear the checkbox
		*/
		void EraseCheckBox();

		/**
		*	Decide whether or not to decorate the widget with a selection rectangle, its control number, or its tab number.
		*/
		void decorate(HWND parentHWND);

		/**
		* Check or uncheck a menu item defined by a menu key
		*/
		short setMenuItemCheck(const char* const menuKey, bool check);
		/**
		* Enable or disable a menu item defined by a menu key
		*/
		short setMenuItemEnable(const char* const menuKey, bool enable);

		short updateVisibilityWRTTabs();
		HWND hWnd;                   // win95 object handle
		char *data;                  // pointer to user defined data
		void Draw(HWND,HDC);
		HWND hwndParent;             // win95 parent window handle
      WinData *winParent;          // Prospa window parent
		short type;		              // object type
		short seqMenuNr;             // menu sequential number
      short tabNr;                 // object tab number
		float upper;                 // Upper limit for data value
		float lower;                 // Lower limit for data value
		bool rangeCheck;             // Check the limit members
      bool debug;                  // Debug version of this object
      bool acceptKeyEvents;              // Whether object allows keydown events
		bool highLiteChanges;        // Show that an entry has been changed
		bool valueChanged;           // Has the value been changed?
		short dataType;              // Data type (INTEGER/FLOAT32/UNQUOTED_STRING)
		char *command;               // Command to run when object selected
      CText dragNDropProc;         // Drag and drop procedure name
      char valueName[MAX_NAME];    // Value name for control
      char objName[MAX_NAME];      // Object name for control
      char cb_event[MAX_NAME];     // Call back event
      char tag[MAX_NAME];          // User definable tag
      float xSzScale;              // Size information as a number
      float xSzOffset;
      float ySzScale;
      float ySzOffset;
      float wSzScale;
      float wSzOffset;
      float hSzScale;
      float hSzOffset;
      short flag;                 // User definable flag
      bool inError;               // Whether we the object contents are in error or not
      bool selected_;              // Whether we can modify the object or not
      bool active;                // Whether we can select the object while the macro is running
      bool readOnly;              // Input read only text
      bool readOnlyOutput;        // Output only text (readonly)
      bool keepFocus;             // Keeps focus after running callback procedure
      HWND toolTipHwnd;            // Tooltip window
      short xo,yo,wo,ho;          // Object region
      bool visible;
      bool enable;
      bool showFrame;             // Should a frame be drawn around certain objects
      short toolbar;              // Toolbar object associated with this control
      short statusbox;            // Statusbox object associated with this control
      short *menuList;             // List of menu object numbers for this control
      short menuListSize;          // Size of menu list
      HMENU menu;                 // Window menu to use if this object has focus
      HACCEL accelTable;          // Accelerator table to use if this object has focus
      RECT region;                // Lists objects which define size of this object
      bool regionSized;           // Whether size is determined by a region
      long cmdLineNr;             // Line number in file where command for object starts
      bool eventActive;           // Whether the object accepts certain events
		ObjectData* panelParent;    // Does this object have a panel as a parent if so gives obj 
		ObjectData* tabParent;      // Does this object have a tab as a parent if so gives obj 
		short tabPageNr;            // Which tab page this object is associated with (if any)
      COLORREF fgColor;           // Foreground color of certain controls (statictext, textbox, listbox)    
      COLORREF bgColor;           // Background color of certain controls (statictext, textbox, listbox) 
      Variable varList;           // List of user defined variables associated with object
private:
	bool nrLessThan(ObjectData* o);

		short nr_;                    // object number
};


typedef struct
{
   short parts;  // Number of parts in status box
   char **posArray; // Array of strings describing position of parts
   HWND subWindow;  // Syntax window for some status bars
   bool colorize;   // whether to modify the font shade and capitalization
   CText syntax;
}
StatusBoxInfo;

// Redefine ACCEL due to an alignment bug in the VS header files
typedef struct
{
    BYTE   fVirt;
    BYTE   dummy;
    WORD   key;
    WORD   cmd;
}
MyACCEL;

typedef struct
{
   short nrItems;
   char *name;
   char **label;
   char **cmd;
   char **key;
   ACCEL *accel;
   HMENU menu;
}
MenuInfo;

typedef struct
{
   CText label;
   CText cmd;
   CText key;
}
ToolBarItem;

typedef struct
{
   CText name;
   short nrItems;
   ToolBarItem *item;
   HBITMAP bitmap;
}
ToolBarInfo;

typedef struct
{  
   void* hImage; 
   bool italicText;
   bool boldText;
   int fontHeight;
   CText fontName;
   bool hovering;
   bool selected;
   bool defaultButton;
}
PushButtonInfo;

typedef struct
{
   bool multiLine;
   bool italicText;
   bool boldText;
   int fontHeight;
   CText fontName;
}
StaticTextInfo;

typedef struct
{
   int x,y,w,h;
   HWND hWndPanel;
   long **childInfo;
   int nrChildren;
   int minY;
   int maxY;
   int yOffset;
   ObjPos pos;
}
PanelInfo;

typedef struct
{
   HBITMAP bmp;
   bool useBlueScreen;
   bool resizePixToFitFrame;
}
PictureInfo;

typedef struct
{
   char *states;
   short init;
}
CheckButtonInfo;

typedef struct
{
   short spacing;
   char *states;
   short init;
   short nrBut;
   char orient;
   HWND *hWnd;
}
RadioButtonInfo;

typedef struct
{
   short orientation;
   bool useLimits;
   ObjPos minPos;
   ObjPos maxPos;
   short origPos;
}
DividerInfo;

typedef struct
{
   float value;
   float base;
   float stepSize;
   long nrSteps;
}
UpDownInfo;

typedef struct
{
   int nrColumns;
   float *colWidth;
	HMENU menu;
	int menuNr;
	bool hasIcons;
	bool allowMultipleSelections;
	int firstLineSelected;
	int lastLineSelected;
}
ListBoxInfo;

typedef struct
{
   short editor;
   HBITMAP hBitmap;
}
DebugStripInfo;

typedef struct
{
	short cols;
	short rows;
}
GridCtrlInfo;

typedef struct
{
  short nrTabs;     // Number of tabs 0 for none
  CText *tabLabels; // Tab titles
}
TabInfo;

short ProcessObjectClassReferences(Interface *itfc, ObjectData *obj, char *name, char *args);
bool IsTabbable(ObjectData *obj);
extern bool win7Mode;

#endif // define GUIOBJECTCLASS_H
#ifndef EDIT_CLASS_H
#define EDIT_CLASS_H

#include <richedit.h>
#include <vector>
#include "ctext.h"

class EditParent;
class EditRegion;
class ObjectData;

typedef struct
{
   short type;
   char macroPath[MAX_PATH];
   char macroName[MAX_PATH];
   char procName[MAX_PATH];
   long lineNr;
}
EditJumpType;

// Undo codes
#define CUT_TEXT             0
#define PASTE_TEXT           1
#define INDENT_TEXT          2
#define UNINDENT_TEXT        3
#define COMMENT_TEXT         4
#define UNCOMMENT_TEXT       5
#define REPLACE_TEXT_BY_CHAR 6
#define REPLACE_TEXT         7
#define ENTER_TEXT           8
#define COPY_PASTE_TEXT      9
#define BACKSPACE_TEXT       10
#define BS_REPLACE_TEXT      11
#define TAB_INSERT           12
#define ENTER_RETURN         13

#define MAX_TEXT 16777216L // maximum number of characters in rich text editor

typedef struct
{
   short type;    // Reason for text change
   char *text;    // Deleted text
   long startPos; // Start of selection
   long endPos;   // End of selection 
}
UndoType;

enum  Focus_Method {CURRENTEDITOR = 1, PARENTWINDOW =2}; 
extern Focus_Method gFocusForRendering;

class EditRegion
{
  public:
	DWORD richEditVersion;
   char edName[MAX_PATH];
   char edPath[MAX_PATH];
   CText currentProc;
   bool edModified;
   bool readOnly; 
   int lineHeight;
   float x,y,w,h; // Window position as a fraction of parent window coords.
   HWND edWin;   // win32 handle for this region
   short regNr,nrX,nrY;  // Region number and location in parent matrix
   short syntaxColoringStyle;
   long initCharInsertionPnt;
	long labelCtrlNr; // Which control will report the file name
	std::vector<EditJumpType> *jumpArray;  
   std::vector<UndoType> *undoArray; 
   long jumpIndex; 
   long undoIndex;
   bool debug;            // Is the editor debugging?
   long debugLine;
   EditRegion(EditParent*,short x,short y,short nrX,short nrY);
   ~EditRegion();   
   EditParent *edParent;  // Parent object
   void GetCommandHelp(void);
   short SelectLines(long,long);
   short FindCommand(long);
   long  CountLines(void);
   short FindString(char*);
   short FindCurrentProcedure(long pos, CText &procName);
   void SelectLastProcedure(void);   
   void SelectNextProcedure(void);
   void SelectEditorLine(long line); 
   void SelectProcedure(char *procName);  
   void CopySelectionToUndo(short type, long value);
   void CopyUndoToSelection(void); 
   void Paste(void);  
   void Cut(void);   
   void Enter(char c);   
   void ResetEditorUndoArray(void);
   void ResetEditorJumpArray(void);
   void AppendText(char* text);
   void SetLineColor(long lineNr, COLORREF color); 
   void ContextColorAllLines(bool resetView, bool redraw);
   void ContextColorLines(long startLine, long endLine); 
   void ContextColorSelectedLines(void);   
   void DisplayColoredText(char *text, bool redraw);
   void SelectEditPosition(long startPos, long endPos, long firstLine);
   void ScrollToEditorLine(long lineNr);
   void CopyTextToEditor(char *text);
   char GetCharacter(long pos);      
	short GetLineHeight(void);
   long  GetLineYPosition(long line);
   short CheckForUnsavedEdits(short edNr);
   short SaveEditContents(void);	
   short FindNextWord(char *word, long &pos, short mode);
   short FindLastWord(char *word, long &pos, short mode);
   char* GetSelection(long &startSel, long &endSel);   
   short  LoadAndSelectProcedure(char *macroName, char *procName, bool showErrors);
   short FindTextAndSelect(HWND hWnd, char *find, bool ignoreCase, long wrapPos);   
   short FindTextAndSelectUp(HWND hWnd, char *find, bool ignoreCase, long wrapPos);
   short ReplaceAndFindText(HWND hWnd,char *replace, char *find, bool ignoreCase, long wrapPos);
   short ReplaceAndFindTextUp(HWND hWnd,char *replace, char *find, bool ignoreCase, long wrapPos); 
   char* ExpandToFullWord2(char* text, long pos, char* leftDelimiter, char* rightDelimiter, long &wordStart, long &wordEnd);
   void RecordJumpHistory(long oldLine, char *oldPath, char *oldMacro, long newLine, char *newPath, char* newMacro);
   void SetEditorFocus(void);
   void RestoreEditorFocus(void);
   void SortSelection(void);
   void SortProcedures(void);
};


class ObjectData;

class EditParent
{
   public:
   
		EditParent(void); // Constructor
      void FreeEditMemory(void);
      ObjectData *parent;       // Parent gui object
      short rows;            // Number of rows of editors
      short cols;            // Number of columns of editors
      short curRegion;       // Current editor having focus (0 based)
      HWND curWnd;           // Current editor Win32 pointer
      EditRegion **editData; // Matrix of editor objects
      bool showContextualMenu; // Show a contextual menu or not
      bool showSyntaxColoring; // Show syntax coloring or not
      bool showSyntaxDescription; // Show syntax description or not
      bool wordWrap;              // Whether wordwrapping is desired
};

//extern short nrXEditRegions;
//extern short nrYEditRegions;
extern WNDPROC fnOldEdit;

//extern EditRegion **ep->editData;
extern EditRegion *curEditor;

#define JUMP_START 0
#define MID_JUMP   1
#define JUMP_END   2

#endif // define EDIT_CLASS_H
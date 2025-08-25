//BABYGRID code is copyrighted (C) 20002 by David Hillard
//
//This code must retain this copyright message
//
//Printed BABYGRID message reference and tutorial available.
//email: mudcat@mis.net for more information.

// Adapted and heavily modified by Mike Davidson 2011.

#ifndef BABYGRID_H
#define BABYGRID_H

#include "windows.h"

#define MAX_GRIDS 20
#define MIN_ROWS 1
#define MAX_ROWS 32000
#define MIN_COLS 1
#define MAX_COLS 256
// Maximum length of a string in a cell
#define MAX_CELL_DATA_LEN 2048


// Default height of a column header, when it's unhidden.
#define DEFAULT_COLHEADER_HEIGHT 30
// Default width of a row header, when it's unhidden.
#define DEFAULT_ROWHEADER_WIDTH  55

typedef enum
{
	CELLDATATYPE_NONE, ALPHA, NUMERIC, BOOL_T, BOOL_F, USER_IMG
} CellDataType;

typedef enum 
{
	GRIDSEARCHDIRECTION_NONE, FORWARDS, BACKWARDS
} GridSearchDirection;

class BabyGrid {
private:
	static HFONT hfontbody;			
	static HFONT hfontheader;		
	static HFONT hfonttitle;		
	static HFONT holdfont;			
	UINT gridmenu;						
	HWND hlist1;						
	char protect[2];					
	char title[305];					
	char editstringdisplay[305];
	int rows;							
	int cols;							
	int gridwidth;
	int gridheight;		
	int homerow;						
	int homecol;						
	int rowheight;						
	int leftvisiblecol;
	int rightvisiblecol;
	int topvisiblerow;
	int bottomvisiblerow;
	int headerrowheight;				
	int cursorrow;						
	int cursorcol;						
	int ownerdrawitem;
	int visiblecolumns;
	int titleheight;					
	int fontascentheight;
	COLORREF cursorcolor;			
	COLORREF protectcolor;			
	COLORREF unprotectcolor;		
	COLORREF textcolor;				
	COLORREF highlightcolor;		
	COLORREF gridlinecolor;			
	COLORREF highlighttextcolor;	
	BOOL DRAWHIGHLIGHT;				
	BOOL ADVANCEROW;					
	BOOL CURRENTCELLPROTECTED;
	BOOL GRIDHASFOCUS;
	BOOL AUTOROW;						
	RECT activecellrect;
	HFONT hfont;						
	HFONT hcolumnheadingfont;		
	HFONT htitlefont;					
	BOOL ROWSNUMBERED;				
	BOOL COLUMNSNUMBERED;			
	BOOL EDITABLE;						
	BOOL EXTENDLASTCOLUMN;			
	BOOL HSCROLL;
	BOOL VSCROLL;
	BOOL SHOWINTEGRALROWS;			
	BOOL SIZING;						
	BOOL ELLIPSIS;						
	BOOL COLAUTOWIDTH;				
	BOOL COLUMNSIZING;				
	BOOL ALLOWCOLUMNRESIZING;		
	int columntoresize;
	int columntoresizeinitsize;
	int columntoresizeinitx;
	int cursortype;					
	int columnwidths[MAX_COLS+1];	
	BOOL REMEMBERINTEGRALROWS;
	int wannabeheight;
	int wannabewidth;
	BOOL ZEROBASED;
	BOOL EDITING;
	char editstring[305];		
	HINSTANCE hInst;
	
	void setCursorPos(int row, int col);
	int HomeColumnNthVisible();
	int GetNextColWithWidth(int startcol, GridSearchDirection direction);
	int GetRowOfMouse(int y);
	int GetColOfMouse(int x);
	bool OutOfRange(int row, int col);
	RECT GetCellRect(int r, int c);
	void DisplayTitle();
	CellDataType DetermineDataType(char* data);
	void DisplayColumn(int c,int offset);
	char GetASCII(WPARAM wParam, LPARAM lParam);
	void SetHomeRow(int row,int col);
	void SetHomeCol(int row,int col);
	void ShowVscroll();
	void ShowHscroll();
	void GetVisibleColumns();
	int GetNthVisibleColumn(int n);
	void CloseEdit();
	void DisplayEditString(char* tstring);
	void SizeGrid();
	int FindLongestLine(HDC hdc,char* text,SIZE* size);
	COLORREF getTextColor();
	void paintGrid();
	void setTextColor(COLORREF color);
	int getLeftVisibleCol();
	int getRightVisibleCol();
	void invalidateClientRect();
	void showIntegralRows(bool show);
	void handle_VK_NEXT();
	void handle_VK_PRIOR();
	void handle_VK_DOWN();
	void handle_VK_UP();
	void handle_VK_LEFT();
	void handle_VK_RIGHT();
	void Refresh();
	void NotifyEndEdit();
	void NotifyDelete();
	void NotifyEditBegin();
	void NotifyEditEnd();
	void NotifyCellClicked();
	void handleControlKey(int code);
	void handleGenericKeypress(WPARAM wParam, LPARAM lParam);
	void CalcVisibleCellBoundaries();
	void SetCurrentCellStatus();
	void DrawCursor();
	void NotifyRowChanged();
	void NotifyColChanged();	
	CellDataType getType(int row, int col);
	int  deleteCell(int row, int col);

	int BinarySearchListBox(char*);
	void scaleCells();
	void spreadColsToMatchX();
	void spreadRowsToMatchY();

public:
	
	// Ctor
	BabyGrid(HWND hWnd, HINSTANCE hInst);
	// Copy constructor
	BabyGrid(const BabyGrid& copyMe);
	// Destructor
	virtual ~BabyGrid();
	static ATOM RegisterGridClass(HINSTANCE hInstance);
	
	//////
	// COLUMN-SPECIFIC FUNCTIONS
	//////
	// How many columns are there?
	void setColCount(int cols);
	int getCols();
	bool validColumn(int col);
	// Set and get width of individual columns
	int setColWidth(int col, int width);
	int getColWidth(int col);
	void setColAutoWidth(bool autoWidth);
	// Set the user-specified label for the specified column's header.
	int setColumnLabel(int col, char* label);
	// When true, display "A" "B" "C" ... rather than user-specified labels
	//  in the column headers.
	void setColsNumbered(bool numbered);
	bool getColsNumbered();
	// Set and get the height of the column headers.
	void setHeaderRowHeight(int height);
	int getHeaderRowHeight();
	// When true, the rightmost column is extended to fill available horizontal
	//  space in the grid window.
	void extendLastColumn(bool extend);
	// When true, prevent the GUI user from changing any values in the specified column.
	int protectColumn(int col, bool protect);
	int getCurrentCol();
	int getCurrentRow();

	//////
	// ROW-SPECIFIC FUNCTIONS
	//////
	// How many rows are there?
	void setRowCount(int rows);
	int getRows();
	bool validRow(int row);
	// Set and get height of all rows. Note this cannot be set on a per-row basis.
	void setRowHeight(int height);
	int getRowHeight();
	void setAutoRow(bool autoRow);
	// Set the user-specified label for the specified row's header.
	int setRowLabel(int row, char* label);
	// When true, display "1" "2" "3" ... rather than user-specified labels
	//  in the row headers.
	void setRowsNumbered(bool numbered);
	bool getRowsNumbered();
	// When true, prevent the GUI user from changing any values in the specified row.
	int protectRow(int row, bool protect);

	//////
	// CELL-SPECIFIC FUNCTIONS
	//////
	// Get or set values in the specified cell.
	CellDataType getCellData(int row, int col, char* result);
	int setCellData(int row, int col, char* data);
	int setCellData(int row, int col, double num);
	int setCellData(int row, int col, int num);
	int protectCell(int row, int col, bool protect);
	// Set or get the protected state of a cell.
	int getProtection(int row, int col);
	void setProtect(bool protect);

	//////
	// GRID-GLOBAL FUNCTIONS
	//////
	// Set the color used as background on un-protected cells.
	void setUnprotectColor(COLORREF color);
	// Set the color used as background on protected cells.
	void setProtectColor(COLORREF color);
	// Get or set the title of the graph, displayed above the column
	//  headers.
	char* getTitle();
	void setTitle(char* title);
	// Get or set the title's height.
	int getTitleHeight();
	void setTitleHeight(int height);
	// Allow or disallow changes to grid data.
	void setEditable(bool editable);
	// Set the height and width of the grid widget.
	void setGridWidth(int width);
	void setGridHeight(int height);
	// Choose whether cells are addressed zero-based (true) or
	//  one-based (false).
	void setZeroBased(bool zeroBased);
	bool getZeroBased();
	// Set the fonts used for column/row headings, and for the
	//  grid title.
	void setHeadingFont(HFONT headingFont);
	void setTitleFont(HFONT titleFont);
	// When true, replace overflowing grid data with "..."
	void setEllipsis(bool ellipsis);
	// Set the background colour of highlighted cells.
	void setHilightColor(COLORREF color);
	// Set the text colour in highlighted cells.
	void setHilightTextColor(COLORREF color);
	// Set the text colour in the cell containing the cursor.
	void setCursorColor(COLORREF color);
	// Set the color of internal grid lines.
	void setGridLineColor(COLORREF color);
	// Empty all data from the grid.
	void clear();

	// Event handlers
	void handle_WM_PAINT();
	void handle_WM_SETTEXT(char*);
	void handle_WM_ENABLE(BOOL enable);
	void handle_WM_MOUSEMOVE(int x, int y);
	void handle_WM_LBUTTONUP();
	void handle_WM_LBUTTONDOWN(int x, int y);
	int  handle_WM_GETDLGCODE(WPARAM wParam);
	int  handle_WM_KEYDOWN(WPARAM wParam, LPARAM lParam);
	void handle_WM_HSCROLL(WPARAM wParam, LPARAM lParam);
	void handle_WM_VSCROLL(WPARAM wParam, LPARAM lParam);
	void handle_WM_SETFOCUS();
	void handle_WM_KILLFOCUS();
	void handle_WM_SETFONT(HFONT wParam);
	void handle_WM_SIZE(int hiword, int loword);

	HWND hWnd; // my window handle						
};

#define BGN_LBUTTONDOWN 0x0001
#define BGN_MOUSEMOVE   0x0002
#define BGN_OUTOFRANGE  0x0003
#define BGN_OWNERDRAW   0x0004
#define BGN_SELCHANGE   0x0005
#define BGN_ROWCHANGED  0x0006
#define BGN_COLCHANGED  0x0007
#define BGN_EDITBEGIN   0x0008
#define BGN_DELETECELL  0x0009
#define BGN_EDITEND     0x000A
#define BGN_F1          0x000B
#define BGN_F2          0x000C
#define BGN_F3          0x000D
#define BGN_F4          0x000E
#define BGN_F5          0x000F
#define BGN_F6          0x0010
#define BGN_F7          0x0011
#define BGN_F8          0x0012
#define BGN_F9          0x0013
#define BGN_F10         0x0014
#define BGN_F11         0x0015
#define BGN_F12         0x0016
#define BGN_GOTFOCUS    0x0017
#define BGN_LOSTFOCUS   0x0018
#define BGN_CELLCLICKED 0x0019

//function forward declarations
ATOM RegisterGridClass(HINSTANCE);

#endif // define BABYGRID_H
#include "stdafx.h"
#include "BabyGrid.h"
#include "error.h"
#include "globals.h"
#include "gridcontrol.h"
#include "guiObjectClass.h"
#include "guiWindowClass.h"
#include "guiWindowCLI.h"
#include "interface.h"
#include "list_functions.h"
#include "mymath.h"
#include "scanstrings.h"
#include "variablesClass.h"
#include "variablesOther.h"
#include "memoryLeak.h"

/* Implementation of grid control */

/*
	Note that BabyGrid row/column addressing is 1-based by default, but we want
	zero-based addressing. (This can be changed at compile time).
	
	Depending on the value of ZEROBASEDGRIDS, the interface to BabyGrid translates the user's 
	zero-based addresses to 1-based ones before passing them to BabyGrid.

	I've changed BabyGrid to accommodate this at the display level, introducing
	a BGM_ZEROBASED message that gets sent along with other params when a grid 
	is created. That only has an effect on the row labels displayed, ie, on whether
	they appear to start at zero or one.
*/

//////////////////////////
// Interface to BabyGrid
//////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// Retrieve the gui object specified by winNr/objNr.
////////////////////////////////////////////////////////////////////////////////////
BabyGrid* getGridObject(Interface* itfc, short winNr, short objNr)
{
	// Check for current window (window number = 0) ***************************	
	if(winNr == 0)
	{
		if((winNr = GetCurWinNr(itfc)) == ERR)
			return(0);
	}

// Find window class ******************************************************
	WinData* win = rootWin->FindWinByNr(winNr);
	if(!win)
	{
	   ErrorMessage("window %d undefined",winNr);
	   return(0);
	}

	ObjectData* obj;
   if(!(obj = win->widgets.findByNr(objNr)))
   {
      ErrorMessage("object (%d,%d) not found",winNr,objNr);
      return(0);
   }

	// Ensure the object is a grid
	if (GRIDCTRL != obj->type)
	{
      ErrorMessage("object (%d,%d) is not a grid",winNr,objNr);
      return(0);
   }
	return (BabyGrid*)(obj->data);
}


/////////////////////////
/* Exported functions. */
/////////////////////////

int GridControlSet(Interface* itfc ,char args[])
{
	short winNr, objNr;
	short col, row;
	Variable objVar;
	int nrArgs;

// Get arguments (window number, parameter identifier, parameter value) ***
	if((nrArgs = ArgScan(itfc,args,0,"winNr, objNr, colNr, rowNr, val","eeeee","ddddv",&winNr,&objNr,&col,&row, &objVar)) < 0)
	   return(nrArgs);
  
	BabyGrid* grid;
	if (!(grid = getGridObject(itfc, winNr, objNr)))
	{
		return ERR;
	}
	int zerobased = grid->getZeroBased() ? 1 : 0;

	if (!grid->validRow(row + zerobased) || !grid->validColumn(col + zerobased))
	{
		ErrorMessage("invalid grid address (%d, %d)", col, row);
		return ERR;
	}

	if(objVar.GetType() == FLOAT32) // By number
   {
      grid->setCellData(row + zerobased, col + zerobased, objVar.GetReal());
	}
	else if (objVar.GetType() == INTEGER) // By number
   {
		grid->setCellData(row + zerobased, col + zerobased, objVar.GetLong());
	}
	else
	{
		grid->setCellData(row + zerobased, col + zerobased, objVar.GetString());
	}
   itfc->nrRetValues = 0;
   return(OK);
}


int GridControlGet(Interface* itfc ,char args[])
{
	short winNr, objNr;
	Variable colVar, rowVar;
	int nrArgs;

// Get arguments (window number, parameter identifier, parameter value) ***
	if((nrArgs = ArgScan(itfc,args,0,"winNr, objNr, colNr, rowNr","eeee","ddvv",&winNr,&objNr,&colVar,&rowVar)) < 0)
	   return(nrArgs);

	if (nrArgs != 4)
	{
		ErrorMessage("grid_get requires 4 arguments, received %d arguments", nrArgs);
		return (ERR);
	}
	
	BabyGrid* grid;
	if (!(grid = getGridObject(itfc, winNr, objNr)))
	{
		return ERR;
	}
	int zerobased = grid->getZeroBased() ? 1 : 0;

	// Make sure no more than one of col, var is a wild card.
	if ((colVar.GetType() != FLOAT32) && (rowVar.GetType() != FLOAT32) && 
		(colVar.GetType() != INTEGER) && (rowVar.GetType() != INTEGER))
	{
		ErrorMessage("One of col and row must be a number.");
		return (ERR);
	}

	// Handle col as wild card.
	if (colVar.GetType() == UNQUOTED_STRING)
	{
		if ('~' != colVar.GetString()[0])
		{
			ErrorMessage("Invalid string received as column argument.");
			return (ERR);
		}
		if (ERR == GridControlGetRow(grid, rowVar.GetReal(), itfc))
		{
			return ERR;
		}
		return OK;
	}

	// Handle row as wild card.
	if (rowVar.GetType() == UNQUOTED_STRING)
	{
		if ('~' != rowVar.GetString()[0])
		{
			ErrorMessage("Invalid string received as row argument.");
			return (ERR);
		}
		if (ERR == GridControlGetColumn(grid, colVar.GetReal(), itfc))
		{
			return ERR;
		}
		return OK;
	}

	// Only retrieving a single cell.

	short col = colVar.GetReal();
	short row = rowVar.GetReal();

	if (!grid->validRow(row + zerobased) || !grid->validColumn(col + zerobased))
	{
		ErrorMessage("invalid grid address (%d, %d)", col, row);
		return ERR;
	}

	char result[MAX_CELL_DATA_LEN];
	CellDataType dataType = grid->getCellData(row + zerobased, col + zerobased, result);
	switch (dataType)
	{
		case ALPHA:
		case BOOL_T:
		case BOOL_F:
			itfc->retVar[1].MakeAndSetString(result);
			itfc->nrRetValues = 1;
			break;
		case NUMERIC:
			itfc->retVar[1].MakeAndSetDouble(atof(result));
			itfc->nrRetValues = 1;
			break;
		default:
			itfc->nrRetValues = 0;
	}
	return (OK);
}


int GridControlClear(Interface* itfc, char args[])
{ 
	short winNr, objNr;
	int nrArgs;

// Get arguments (window number, parameter identifier, parameter value) ***
	if((nrArgs = ArgScan(itfc,args,0,"winNr, objNr, colNr, rowNr, val","ee","dd",&winNr,&objNr)) < 0)
	   return(nrArgs);

	BabyGrid* grid;
	if (!(grid = getGridObject(itfc, winNr, objNr)))
	{
		return ERR;
	}
	
	grid->clear();
   itfc->nrRetValues = 0;
   return(OK);
}


int GridControlSetRowCount(Interface* itfc, char args[])
{
	short winNr, objNr;
	short rows;
	int nrArgs;

// Get arguments (window number, parameter identifier, parameter value) ***
	if((nrArgs = ArgScan(itfc,args,0,"winNr, objNr, rows","eee","ddd",&winNr,&objNr,&rows)) < 0)
	   return(nrArgs);
	
	if ((rows < MIN_ROWS) || (rows > MAX_ROWS))
	{
		ErrorMessage("Invalid number of rows %d requested", rows);
		return ERR;
	}

	BabyGrid* grid;
	if (!(grid = getGridObject(itfc, winNr, objNr)))
	{
		return ERR;
	}

	grid->setRowCount(rows);
	itfc->nrRetValues = 0;
   return(OK);
}


int GridControlSetColumnCount(Interface* itfc, char args[])
{
	short winNr, objNr;
	short cols;
	int nrArgs;

// Get arguments (window number, parameter identifier, parameter value) ***
	if((nrArgs = ArgScan(itfc,args,0,"winNr, objNr, cols","eee","ddd",&winNr,&objNr,&cols)) < 0)
	   return(nrArgs);

	if ((cols < MIN_COLS) || (cols > MAX_COLS))
	{
		ErrorMessage("Invalid number of columns %d requested", cols);
		return ERR;
	}

	BabyGrid* grid;
	if (!(grid = getGridObject(itfc, winNr, objNr)))
	{
		return ERR;
	}

	grid->setColCount(cols);
   itfc->nrRetValues = 0;
   return(OK);
}


int GridControlSetColumnWidth(Interface* itfc, char args[])
{
	short winNr, objNr;
	short col, width;
	int nrArgs;

// Get arguments (window number, parameter identifier, parameter value) ***
	if((nrArgs = ArgScan(itfc,args,0,"winNr, objNr, col, width","eeee","dddd",&winNr,&objNr,&col,&width)) < 0)
	   return(nrArgs);

	BabyGrid* grid;
	if (!(grid = getGridObject(itfc, winNr, objNr)))
	{
		return ERR;
	}
	int zerobased = grid->getZeroBased() ? 1 : 0;

	if (ERR == grid->setColWidth(col + zerobased, width))
	{
		ErrorMessage("invalid column specified (%d)", col);
		return ERR;
	}
   itfc->nrRetValues = 0;
   return(OK);
}


int GridControlSetRowHeight(Interface* itfc, char args[])
{
	short winNr, objNr;
	short height;
	int nrArgs;

// Get arguments (window number, parameter identifier, parameter value) ***
	if((nrArgs = ArgScan(itfc,args,0,"winNr, objNr, row, height","eee","ddd",&winNr,&objNr,&height)) < 0)
	   return(nrArgs);

	BabyGrid* grid;
	if (!(grid = getGridObject(itfc, winNr, objNr)))
	{
		return ERR;
	}
	
	grid->setRowHeight(height);
   itfc->nrRetValues = 0;
   return(OK);
}


int GridControlSetTitleHeight(Interface* itfc, char args[])
{
	short winNr, objNr;
	short height;
	int nrArgs;

// Get arguments (window number, parameter identifier, parameter value) ***
	if((nrArgs = ArgScan(itfc,args,0,"winNr, objNr, height","eee","ddd",&winNr,&objNr,&height)) < 0)
	   return(nrArgs);

	BabyGrid* grid;
	if (!(grid = getGridObject(itfc, winNr, objNr)))
	{
		return ERR;
	}
	
	grid->setTitleHeight(height);
   itfc->nrRetValues = 0;
   return(OK);
}


int GridControlSetColumnHeaderHeight(Interface* itfc, char args[])
{
	short winNr, objNr;
	short height;
	int nrArgs;

// Get arguments (window number, parameter identifier, parameter value) ***
	if((nrArgs = ArgScan(itfc,args,0,"winNr, objNr, height","eee","ddd",&winNr,&objNr,&height)) < 0)
	   return(nrArgs);

	BabyGrid* grid;
	if (!(grid = getGridObject(itfc, winNr, objNr)))
	{
		return ERR;
	}

	grid->setHeaderRowHeight(height);
   itfc->nrRetValues = 0;
   return(OK);
}


int GridControlSetRowHeaderWidth(Interface* itfc, char args[])
{
	short winNr, objNr;
	short width;
	int nrArgs;

// Get arguments (window number, parameter identifier, parameter value) ***
	if((nrArgs = ArgScan(itfc,args,0,"winNr, objNr, width","eee","ddd",&winNr,&objNr,&width)) < 0)
	   return(nrArgs);

	BabyGrid* grid;
	if (!(grid = getGridObject(itfc, winNr, objNr)))
	{
		return ERR;
	}
	
	grid->setColWidth(0,width);
   itfc->nrRetValues = 0;
   return(OK);
}


int GridControlShowRowHeaders(Interface* itfc, char args[])
{
	short winNr, objNr;
   CText show;
	int nrArgs;

   if((nrArgs = ArgScan(itfc,args,1,"winNr, objNr, true/false","eee","ddt",&winNr,&objNr,&show)) < 0)
      return(nrArgs);

	BabyGrid* grid;
	if (!(grid = getGridObject(itfc, winNr, objNr)))
	{
		return ERR;
	}

	bool doShow = ("false" == show)? false : true;

	if (doShow && (grid->getColWidth(0) < 1))
	{
		// Header is not visible; make it visible.
		grid->setColWidth(0,DEFAULT_ROWHEADER_WIDTH);
	}
	if (!doShow && (grid->getColWidth(0) > 0))
	{
		// Header is visible; hide it.
		grid->setColWidth(0,0);
	}
   itfc->nrRetValues = 0;
   return(OK);
}


int GridControlShowRowLabels(Interface* itfc, char args[])
{
	short winNr, objNr;
   CText show;
	int nrArgs;

   if((nrArgs = ArgScan(itfc,args,1,"winNr, objNr, true/false","eee","ddt",&winNr,&objNr,&show)) < 0)
      return(nrArgs);

	BabyGrid* grid;
	if (!(grid = getGridObject(itfc, winNr, objNr)))
	{
		return ERR;
	}

	grid->setRowsNumbered(("false" == show)? true : false);
   itfc->nrRetValues = 0;
   return(OK);
}


int GridControlShowColumnHeaders(Interface* itfc, char args[])
{
	short winNr, objNr;
   CText show;
	int nrArgs;

   if((nrArgs = ArgScan(itfc,args,1,"winNr, objNr, true/false","eee","ddt",&winNr,&objNr,&show)) < 0)
      return(nrArgs);

	BabyGrid* grid;
	if (!(grid = getGridObject(itfc, winNr, objNr)))
	{
		return ERR;
	}

	bool doShow = ("false" == show) ? false : true;

	if (doShow && (grid->getHeaderRowHeight() < 1))
	{
		// Header is not visible; make it visible.
		grid->setHeaderRowHeight(DEFAULT_COLHEADER_HEIGHT);
	}
	if (!doShow && (grid->getHeaderRowHeight() > 0))
	{
		// Header is visible; hide it.
		grid->setHeaderRowHeight(0);
	}
   itfc->nrRetValues = 0;
   return(OK);
}


int GridControlShowColumnLabels(Interface* itfc, char args[])
{
	short winNr, objNr;
   CText show;
	int nrArgs;

   if((nrArgs = ArgScan(itfc,args,1,"winNr, objNr, true/false","eee","ddt",&winNr,&objNr,&show)) < 0)
      return(nrArgs);

	BabyGrid* grid;
	if (!(grid = getGridObject(itfc, winNr, objNr)))
	{
		return ERR;
	}

	grid->setColsNumbered(("false" == show)? true : false);
   itfc->nrRetValues = 0;
   return(OK);
}


int GridControlProtect(Interface* itfc, char args[])
{
	short winNr, objNr;
	Variable colVar, rowVar;
   CText protectText;
	int nrArgs;

   if((nrArgs = ArgScan(itfc,args,1,"winNr, objNr, colNr, rowNr, true/false","eeeee","ddvvt",&winNr,&objNr,&colVar,&rowVar,&protectText)) < 0)
      return(nrArgs);

	BabyGrid* grid;
	if (!(grid = getGridObject(itfc, winNr, objNr)))
	{
		return ERR;
	}
	int zerobased = grid->getZeroBased() ? 1 : 0;
	
	short col = colVar.GetReal();
	short row = rowVar.GetReal();
	bool protect = ("true" == protectText);

	// Protect the entire grid?
	if ((colVar.GetType() == UNQUOTED_STRING) && (rowVar.GetType() == UNQUOTED_STRING))
	{
		if (('~' == colVar.GetString()[0]) && ('~' == rowVar.GetString()[0]))
		{
			for (col = 0; col < grid->getCols(); col++)
			{
				if (ERR == GridControlProtectColumn(grid,col,protect,itfc))
				{
					return ERR;
				}
			}
		}
		else
		{
			ErrorMessage("invalid string received as column argument.");
			return ERR;
		}
	}

	// Protect a column?
	else if (rowVar.GetType() == UNQUOTED_STRING)
	{
		if ('~' == rowVar.GetString()[0])
		{
			if (grid->validColumn(col))
			{
				if (ERR == GridControlProtectColumn(grid,col,protect,itfc))
				{
					return ERR;
				}
			}
			else
			{
				return ERR;
			}
		}
		else
		{
			ErrorMessage("invalid string received as column argument.");
			return ERR;
		}
	}

	// Protect a row?
	else if (colVar.GetType() == UNQUOTED_STRING)
	{
		if ('~' == colVar.GetString()[0])
		{
			if (grid->validRow(row))
			{
				return GridControlProtectRow(grid,row,protect,itfc);
			}
			else
			{
				return ERR;
			}
		}
		else
		{
			ErrorMessage("invalid string received as row argument.");
			return ERR;
		}
	}

	// Protect one cell.
	else if (ERR == grid->protectCell(row + zerobased, col + zerobased, protect))
	{
		ErrorMessage("invalid grid address (%d, %d)", col, row);
		return ERR;
	}
   itfc->nrRetValues = 0;
   return(OK);
}


int GridControlProtectColumn(BabyGrid* grid, int col, bool protect, Interface* itfc)
{
	int zerobased = grid->getZeroBased() ? 1 : 0;

	if (ERR == grid->protectColumn(col + zerobased, protect))
	{
		ErrorMessage("invalid column specified (%d)", col);
		return ERR;
	}
   itfc->nrRetValues = 0;
   return(OK);
}


int GridControlProtectRow(BabyGrid* grid, int row, bool protect, Interface* itfc)
{
	int zerobased = grid->getZeroBased() ? 1 : 0;

	if (ERR == grid->protectRow(row + zerobased, protect))
	{
		ErrorMessage("invalid row specified (%d)", row);
		return ERR;
	}
   itfc->nrRetValues = 0;
   return(OK);
}


int GridControlSetLabel(Interface* itfc, char args[])
{
	short winNr, objNr;
	Variable colVar, rowVar;
	int nrArgs;

   if((nrArgs = ArgScan(itfc,args,1,"winNr, objNr, rowNr, label","eeee","ddvv",&winNr,&objNr,&colVar,&rowVar)) < 0)
      return(nrArgs);

	BabyGrid* grid;
	if (!(grid = getGridObject(itfc, winNr, objNr)))
	{
		return ERR;
	}
	int zerobased = grid->getZeroBased() ? 1 : 0;

	// One of colVar, rowVar must be a string (the label), and the other must be a number (the row/col to set).
	if ((colVar.GetType() == UNQUOTED_STRING) && (rowVar.GetType() == FLOAT32))
	{
		// Set the label of a row.
		short row = rowVar.GetReal();
		if (!grid->validRow(row+ zerobased))
		{	
			ErrorMessage("invalid row specified (%d)", row);
			return ERR;
		}
		if (ERR == grid->setRowLabel(row + zerobased, colVar.GetString()))
		{
			ErrorMessage("invalid row specified (%d)", row);
			return ERR;
		}
	}

	else if ((rowVar.GetType() == UNQUOTED_STRING) && (colVar.GetType() == FLOAT32))
	{
		// Set the label of a column.
		short col = colVar.GetReal();
		if (!grid->validColumn(col+ zerobased))
		{
			ErrorMessage("invalid column specified (%d)", col);
			return ERR;
		}
		if (ERR == grid->setColumnLabel(col + zerobased, rowVar.GetString()))
		{
			ErrorMessage("invalid column specified (%d)", col);
			return ERR;
		}
	}

	else
	{
		// Bad format.
		ErrorMessage("One of colVar, rowVar must be a string (the label), and the other must be a number (the row/col to set)");
		return ERR;
	}

   itfc->nrRetValues = 0;
	return(OK);
}

int GridControlGetColumn(BabyGrid* grid, int col, Interface* itfc)
{
	int zerobased = grid->getZeroBased() ? 1 : 0;

	if (!grid->validColumn(col + zerobased))
	{
		ErrorMessage("invalid column specified (%d)", col);
		return ERR;
	}

	int rows = grid->getRows();
   char **list = MakeList(rows);
	if(!list)
	{
		ErrorMessage("can't allocate memory for list");
		return(ERR);
	}  

	char result[MAX_CELL_DATA_LEN];
	for (int row = 0; row < rows; row++)
	{
		grid->getCellData(row + zerobased, col + zerobased, result);
		list[row] = new char[MAX_CELL_DATA_LEN];		
		strncpy_s(list[row],MAX_CELL_DATA_LEN, result, _TRUNCATE);
		list[row][MAX_CELL_DATA_LEN - 1] = '\0';
	}
	itfc->retVar[1].MakeAndSetList(list,rows);
   itfc->nrRetValues = 1;
	FreeList(list, rows);
	return(OK);
}


int GridControlGetRow(BabyGrid* grid, int row, Interface* itfc)
{
	int zerobased = grid->getZeroBased() ? 1 : 0;
	if (!grid->validRow(row + zerobased))
	{
		ErrorMessage("invalid row specified (%d)", row);
		return ERR;
	}

	int cols = grid->getCols();
	char **list = MakeList(cols);
	if(!list)
	{
		ErrorMessage("Can't allocate memory for list");
		return(ERR);
	}  

	char result[MAX_CELL_DATA_LEN];
	for (int col = 0; col < cols; col++)
	{
		grid->getCellData(row + zerobased, col + zerobased, result);
		list[col] = new char[MAX_CELL_DATA_LEN];		
		strncpy_s(list[col], MAX_CELL_DATA_LEN, result, _TRUNCATE);
		list[col][MAX_CELL_DATA_LEN - 1] = '\0';
	}
	itfc->retVar[1].MakeAndSetList(list,cols);
   itfc->nrRetValues = 1;
	FreeList(list, cols);
	return(OK);
}
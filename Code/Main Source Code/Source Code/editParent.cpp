#include "stdafx.h"
#include "edit_class.h"
#include "memoryLeak.h"

/*******************************************************************
  Initialise the edit parent
********************************************************************/

EditParent::EditParent()
{
   rows = 1;
   cols = 1;
   curRegion = 0;
   curWnd = NULL;
   parent = NULL;
   editData = NULL;
   showContextualMenu = true;
   showSyntaxColoring = true;
   showSyntaxDescription = true;
   wordWrap = false;
}


/**********************************************************************
  Free up all memory used by the subedit windows
***********************************************************************/
 
void EditParent::FreeEditMemory()
{
   if(editData != NULL)
   {
      curEditor = NULL;
	   for(short i = 0; i < cols*rows; i++)
		{
         ShowWindow(editData[i]->edWin,SW_HIDE);
		   DestroyWindow(editData[i]->edWin);
	      delete editData[i];
	   }
	}
	delete [] editData;
	editData = NULL;  
} 

//BABYGRID code is copyrighted (C) 20002 by David Hillard
//
//This code must retain this copyright message
//
//Printed BABYGRID message reference and tutorial available.
//email: mudcat@mis.net for more information.

// Adapted and heavily modified by Mike Davidson 2011.

#include "stdafx.h"
#include "babygrid.h"
#include "globals.h"
#include "guiWindowsEvents.h"
#include "memoryLeak.h"

#pragma warning (disable: 4311) // Ignore pointer truncation warnings

//global variables

//HFONT BabyGrid::hfontbody=CreateFont(14,0,0, 0,100,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,FIXED_PITCH|FF_SWISS ,NULL);
//HFONT BabyGrid::hfontheader=CreateFont(16,0,0, 0,FW_BOLD,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,VARIABLE_PITCH|FF_MODERN ,NULL);
//HFONT BabyGrid::hfonttitle=CreateFont(16,0,0, 0,FW_HEAVY,FALSE,FALSE,FALSE,ANSI_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,PROOF_QUALITY,VARIABLE_PITCH|FF_MODERN ,NULL);

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

BabyGrid::~BabyGrid()
{
	ShowWindow(hlist1,SW_HIDE);
	DestroyWindow(hlist1);     	
}

int BabyGrid::HomeColumnNthVisible()
{  
	int count=0;
	
	for(int col = 1;col <= homecol;col++)
	{
		if(columnwidths[col] > 0)
		{
			count++;
		}
	}
	return count;
}


void BabyGrid::Refresh()
{	
	this->scaleCells();
	invalidateClientRect();
	if(EDITING) 
	{
		DisplayEditString("");
	}
}

BabyGrid::BabyGrid(const BabyGrid& copyMe)
{
	memcpy(this, (const void*)&copyMe, sizeof(BabyGrid));
	memcpy(protect, copyMe.protect, 2);
	memcpy(title, copyMe.title, 305);
	memcpy(editstringdisplay, copyMe.editstringdisplay, 305);
	memcpy(columnwidths, copyMe.columnwidths, sizeof(int) * (MAX_COLS+1));	

	// Get the number of cells with data.
	int dataCount = SendMessage(copyMe.hlist1,LB_GETCOUNT, 0, 0);
	char text[1000];

	hlist1=CreateWindowEx(WS_EX_CLIENTEDGE,"LISTBOX","",
		WS_CHILD|LBS_STANDARD,50,150,200,100,hWnd,NULL,hInst,NULL);

	// Copy those cells.
	for (int i = 0; i < dataCount; i++)
	{		
		SendMessage(copyMe.hlist1,LB_GETTEXT,i,(long)text);
		SendMessage(hlist1, LB_ADDSTRING,0,(long)text);
	}
}

int BabyGrid::GetNextColWithWidth(int startcol, GridSearchDirection direction)
{
	//calls with direction == 1 for right, direction == -1 for left
	//returns 0 if no more cols in that direction, else column number
	int col = startcol;
	if(direction == FORWARDS){col++;}
	if(direction == BACKWARDS){col--;}

	while((columnwidths[col] == 0)&&(col <= cols)&&(col > 0))
	{
		if(direction == FORWARDS){col++;}
		if(direction == BACKWARDS){col--;}
	}

	return ((columnwidths[col] > 0)&&(col<=cols)) ? col : 0;
}


int BabyGrid::GetRowOfMouse(int y)
{
	if(y<=titleheight)
	{
		return -1;
	}
	if((y>=titleheight)&&(y<=headerrowheight + titleheight))
	{
		return 0;
	}


	y = y-(headerrowheight + titleheight);
	y = y/rowheight;
	int ReturnValue = homerow + y;
	if(ReturnValue > rows)
	{
		ReturnValue = -1;
	}
	return ReturnValue;
}


int BabyGrid::GetColOfMouse(int x)
{
	if(x<=columnwidths[0])
	{
		return 0;
	}

	x -= columnwidths[0];

	int j = homecol;
	while(x>0)
	{
		x -= columnwidths[j];
		j++;
	}
	j--;

	int ReturnValue = j;
	if(EXTENDLASTCOLUMN)
	{
		if(j>cols){ReturnValue = cols;}
	}
	else
	{
		if(j>cols){ReturnValue = -1;}
	}
	return ReturnValue;
}

bool BabyGrid::OutOfRange(int row, int col)
{
	return ((row > MAX_ROWS)||(col > MAX_COLS));
}

CellDataType BabyGrid::DetermineDataType(char* data)
{
	//return values:
	//       1 = Text or Alpha
	//       2 = Numeric
	//       3 = Boolean TRUE
	//       4 = Boolean FALSE
	//       5 = Graphic - user drawn (cell text begins with ~)
	int numberofperiods;
	char tbuffer[1000];
	BOOL DIGIT,AL,PERIOD,WHITESPACE,SYMBOL,POSITIVE,NEGATIVE;
	strcpy(tbuffer,data);
	int k=strlen(tbuffer);
	strupr(tbuffer);
	//is it boolean?
	if(!strcmp(tbuffer,"TRUE"))
	{
		return BOOL_T; 
	}
	if(!strcmp(tbuffer,"FALSE"))
	{
		return BOOL_F;
	}
	//is it graphic (~)
	if(tbuffer[0]=='~')
	{
		return USER_IMG;
	}
	DIGIT=FALSE;
	AL=FALSE;
	PERIOD=FALSE;
	WHITESPACE=FALSE;
	SYMBOL=FALSE;
	POSITIVE=FALSE;
	NEGATIVE=FALSE;

	numberofperiods=0;
	for(int j=0;j<k;j++)
	{
		if(isalpha(tbuffer[j])){AL=TRUE;}
		if(isdigit(tbuffer[j])){DIGIT=TRUE;}
		if(iswspace(tbuffer[j])){WHITESPACE=TRUE;}
		if(tbuffer[j]=='.'){PERIOD=TRUE;numberofperiods++;}
		if(tbuffer[j]=='+'){if(j>0){AL=TRUE;}}
		if(tbuffer[j]=='-'){if(j>0){AL=TRUE;}}
	}
	if((AL)||(WHITESPACE))
	{
		return ALPHA;
	}
	if((DIGIT)&&(!AL)&&(!WHITESPACE))
	{
		if(numberofperiods>1)
		{
			return ALPHA;
		}
		else
		{
			return NUMERIC;
		}
	}
	return ALPHA;
}


void BabyGrid::CalcVisibleCellBoundaries()
{
	int j =homecol;
	leftvisiblecol = homecol;
	topvisiblerow = homerow;
	//calc columns visible
	//first subtract the width of col 0;
	gridwidth -= columnwidths[0];
	do
	{
		gridwidth -= columnwidths[j];
		j++;
	}while ((gridwidth >= 0)&&(j<cols));

	if(j>cols){j=cols;}
	rightvisiblecol = j;

	//calc rows visible;
	gridheight -= headerrowheight;
	j = homerow;
	do
	{
		gridheight -= rowheight;
		j++;
	}while ((gridheight > 0)&&(j<rows));

	if(j>rows){j=rows;}
	bottomvisiblerow = j;
}


RECT BabyGrid::GetCellRect(int r, int c)
{
	RECT rect;
	int j;
	//c and r must be greater than zero

	//get column offset
	//first get col 0 width
	int offset = columnwidths[0];
	for(j = homecol;j < c;j++)
	{
		offset += columnwidths[j];
	}
	rect.left = offset;
	rect.right = offset + columnwidths[c];

	if(EXTENDLASTCOLUMN)
	{
		//see if this is the last column
		if(!GetNextColWithWidth(c,FORWARDS))
		{
			//extend this column
			RECT trect;
			int temp;
			GetClientRect(hWnd,&trect);
			temp = (offset +(trect.right - rect.left))-rect.left;
			if(temp > columnwidths[c])
			{
				rect.right = offset + (trect.right - rect.left);
			}
		}
	}

	//now get the top and bottom of the rect
	offset = headerrowheight+titleheight;
	for(j=homerow;j<r;j++)
	{
		offset += rowheight;
	}
	rect.top = offset;
	rect.bottom = offset + rowheight;
	return rect;
}


void BabyGrid::DisplayTitle()
{
	RECT rect;

	GetClientRect(hWnd,&rect);

	HDC gdc=GetDC(hWnd);
	SetBkMode(gdc,TRANSPARENT);
	HFONT holdfont=(HFONT)SelectObject(gdc,htitlefont);
	rect.bottom = titleheight;
	DrawEdge(gdc,&rect,EDGE_ETCHED,BF_MIDDLE|BF_RECT|BF_ADJUST);
	DrawTextEx(gdc,title,-1,&rect,DT_END_ELLIPSIS|DT_CENTER|DT_WORDBREAK|DT_NOPREFIX,NULL);
	SelectObject(gdc,holdfont);
	// MD:: Shouldn't this holdfont be released?
	ReleaseDC(hWnd,gdc);
}


void BabyGrid::DisplayColumn(int c,int offset)
{
	RECT rect,rectsave;
	char buffer[1000];
	int iProtection;
	if(columnwidths[c]==0){return;}

	HDC gdc=GetDC(hWnd);
	SetBkMode(gdc,TRANSPARENT);
	ShowHscroll();
	ShowVscroll();

	HFONT holdfont = (HFONT)SelectObject(gdc,hcolumnheadingfont);
	SetTextColor(gdc,textcolor);
	//display header row
	int r=0;

	rect.left = offset + 0;
	rect.top = titleheight;//0
	rect.right = columnwidths[c] + offset;
	rect.bottom = headerrowheight + titleheight;

	if(EXTENDLASTCOLUMN)
	{
		//see if this is the last column
		if(!GetNextColWithWidth(c,FORWARDS))
		{
			//extend this column
			RECT trect;
			GetClientRect(hWnd,&trect);

			rect.right = offset + (trect.right - rect.left);
		}
	}
	else
	{
		if(!GetNextColWithWidth(c,FORWARDS))
		{
			//repaint right side of grid
			RECT trect;
			HBRUSH holdbrush;
			HPEN holdpen;
			GetClientRect(hWnd,&trect);
			trect.left=offset+(rect.right-rect.left);
			holdbrush=(HBRUSH)SelectObject(gdc,GetStockObject(GRAY_BRUSH));
			holdpen=(HPEN)SelectObject(gdc,GetStockObject(NULL_PEN));
			Rectangle(gdc,trect.left,trect.top+titleheight,trect.right+1,trect.bottom+1);
			SelectObject(gdc,holdbrush);
			SelectObject(gdc,holdpen);

		}
	}

	strcpy(buffer,"");
	getCellData(r,c,buffer);
	if(COLUMNSNUMBERED)
	{
		if(c>0)
		{
			int high = ((c-1)/26);
			int low = c % 26;
			if(high == 0){high = 32;}else{high+=64;}
			if(low == 0){low=26;}
			low += 64;
			wsprintf(buffer,"%c%c",high,low);
		}
	}
	rectsave=rect;
	DrawEdge(gdc,&rect,EDGE_ETCHED,BF_MIDDLE|BF_RECT|BF_ADJUST);
	DrawTextEx(gdc,buffer,-1,&rect,DT_END_ELLIPSIS|DT_CENTER|DT_WORDBREAK|DT_NOPREFIX,NULL);
	rect=rectsave;

	r=topvisiblerow;
	//set font for grid body
	SelectObject(gdc,hfont);
	while(r<=bottomvisiblerow)
	{
		//try to set cursor row to different display color
		if((r==cursorrow)&&(c>0)&&(DRAWHIGHLIGHT))
		{
			if(GRIDHASFOCUS)
			{
				SetTextColor(gdc,highlighttextcolor);
			}
			else
			{
				SetTextColor(gdc,RGB(0,0,0));//set black text for nonfocus grid hilight
			}
		}
		else
		{
			SetTextColor(gdc,RGB(0,0,0));
		}

		rect.top = rect.bottom;
		rect.bottom = rect.top + rowheight;
		rectsave=rect;
		
		strcpy(buffer,"");
		getCellData(r,c,buffer);
		
		if((c==0)&&(ROWSNUMBERED))
		{
			if (!ZEROBASED)
				wsprintf(buffer,"%d",r);
			else
				wsprintf(buffer,"%d",r-1);
		}
		if(c==0)
		{
			DrawEdge(gdc,&rect,EDGE_ETCHED,BF_MIDDLE|BF_RECT|BF_ADJUST);
		}
		else
		{
			HBRUSH hbrush,holdbrush;
			HPEN hpen,holdpen;
			iProtection = getProtection(r,c);
			if(DRAWHIGHLIGHT)//highlight on
			{
				if(r==cursorrow)
				{
					if(GRIDHASFOCUS)
					{
						hbrush=CreateSolidBrush(highlightcolor);
					}
					else
					{
						hbrush=CreateSolidBrush(RGB(200,200,200));
					}
				}

				else
				{
					if(iProtection == 1)
					{
						hbrush=CreateSolidBrush(protectcolor);
					}
					else
					{
						hbrush=CreateSolidBrush(unprotectcolor);
					}
				}
			}
			else
			{
				if(iProtection == 1)
				{
					hbrush=CreateSolidBrush(protectcolor);
				}
				else
				{
					hbrush=CreateSolidBrush(unprotectcolor);
				}
			}
			hpen=CreatePen(PS_SOLID,1,gridlinecolor);
			holdbrush=(HBRUSH)SelectObject(gdc,hbrush);
			holdpen=(HPEN)SelectObject(gdc,hpen);
			Rectangle(gdc,rect.left,rect.top,rect.right,rect.bottom);
			SelectObject(gdc,holdbrush);
			SelectObject(gdc,holdpen);
			DeleteObject(hbrush);
			DeleteObject(hpen);
		}
		rect.right -= 2;
		rect.left += 2;

		CellDataType iDataType = getType(r,c);
		if((iDataType < ALPHA)||(iDataType > USER_IMG))
		{
			iDataType = ALPHA;//default to alphanumeric data type.. can't happen
		}
		if(c==0){iDataType = NUMERIC;}

		if(iDataType == ALPHA)//ALPHA
		{
			if(ELLIPSIS)
			{
				DrawTextEx(gdc,buffer,-1,&rect,DT_END_ELLIPSIS| DT_CENTER |DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX,NULL);
			}
			else
			{
				DrawTextEx(gdc,buffer,-1,&rect, DT_CENTER | DT_VCENTER |DT_WORDBREAK|DT_EDITCONTROL|DT_NOPREFIX,NULL);
			}
		}

		if(iDataType == NUMERIC)//NUMERIC
		{
			DrawTextEx(gdc,buffer,-1,&rect,DT_END_ELLIPSIS| DT_CENTER |DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX,NULL);
		}

		if(iDataType == BOOL_T)//BOOLEAN TRUE
		{
			int k,excess;
			k=2;
			rect.top +=k;
			rect.bottom -=k;
			rect.left +=0;
			rect.right -=0;
			if((rect.bottom - rect.top)>24)
			{
				excess=(rect.bottom - rect.top)-16;
				rect.top += (int)(excess/2);
				rect.bottom -= (int)(excess/2);
			}
			DrawFrameControl(gdc,&rect,DFC_BUTTON,DFCS_BUTTONCHECK|DFCS_CHECKED);
		}

		if(iDataType == BOOL_F)//BOOLEAN FALSE
		{
			int k,excess;
			k=2;
			rect.top +=k;
			rect.bottom -=k;
			rect.left +=0;
			rect.right -=0;
			if((rect.bottom - rect.top)>24)
			{
				excess=(rect.bottom - rect.top)-16;
				rect.top += (int)(excess/2);
				rect.bottom -= (int)(excess/2);
			}
			DrawFrameControl(gdc,&rect,DFC_BUTTON,DFCS_BUTTONCHECK);
		}

		if(iDataType == USER_IMG) //user drawn graphic
		{
			WPARAM wParam;
			buffer[0]=0x20;
			ownerdrawitem = atoi(buffer);
			wParam=MAKEWPARAM((UINT)GetMenu(hWnd),BGN_OWNERDRAW);
			SendMessage(GetParent(hWnd),WM_COMMAND,wParam,(long)&rect);
		}

		if(EDITING)
		{
			DisplayEditString("");
		}

		rect=rectsave;
		r++;
	}//end while r<=bottomvisiblerow

	{
		//repaint bottom of grid
		RECT trect;
		HBRUSH holdbrush;
		HPEN holdpen;
		GetClientRect(hWnd,&trect);
		trect.top = rect.bottom;
		trect.left = rect.left;
		trect.right = rect.right;

		holdbrush=(HBRUSH)SelectObject(gdc,GetStockObject(GRAY_BRUSH));
		holdpen=(HPEN)SelectObject(gdc,GetStockObject(NULL_PEN));

		Rectangle(gdc,trect.left,trect.top,trect.right+1,trect.bottom+1);

		SelectObject(gdc,holdbrush);
		SelectObject(gdc,holdpen);
	}
	SelectObject(gdc,holdfont);
	DeleteObject(holdfont);
	ReleaseDC(hWnd,gdc);
}


void BabyGrid::DrawCursor()
{
	RECT rect,rectwhole;
	HDC gdc;
	HPEN hpen,holdpen;
	int rop;
	if(rows == 0){return;}
	GetClientRect(hWnd,&rect);
	//if active cell has scrolled off the top, don't draw a focus rectangle
	if(cursorrow < homerow){return;}
	//if active cell has scrolled off to the left, don't draw a focus rectangle
	if(cursorcol < homecol){return;}

	rect = GetCellRect(cursorrow,cursorcol);
	rectwhole=rect;
	gdc=GetDC(hWnd);
	activecellrect = rect;
	rop=GetROP2(gdc);
	SetROP2(gdc,R2_XORPEN);
	SelectObject(gdc, (HBRUSH)GetStockObject(NULL_BRUSH));
	hpen=CreatePen(PS_SOLID,2,cursorcolor);  //width of 2
	holdpen=(HPEN)SelectObject(gdc,hpen);
	Rectangle(gdc,rect.left,rect.top,rect.right,rect.bottom);
	SelectObject(gdc,holdpen);
	DeleteObject(hpen);
	SetROP2(gdc,rop);
	ReleaseDC(hWnd,gdc);
}

void BabyGrid::SetCurrentCellStatus()
{
	int protection = getProtection(cursorrow, cursorcol);
	switch(protection)
	{
	case 0:
		CURRENTCELLPROTECTED = FALSE;
		break;
	case 1:
		CURRENTCELLPROTECTED = TRUE;
		break;
	}
}

char BabyGrid::GetASCII(WPARAM wParam, LPARAM lParam)
{
	int returnvalue;
	char mbuffer[100];
	int result;
	BYTE keys[256];
	WORD dwReturnedValue;
	GetKeyboardState(keys);
	result=ToAscii(wParam,(lParam >> 16) && 0xff,keys,&dwReturnedValue,0);
	returnvalue = (char) dwReturnedValue;
	if(returnvalue < 0){returnvalue = 0;}
	wsprintf(mbuffer,"return value = %d",returnvalue);
	if(result!=1){returnvalue = 0;}
	return (char)returnvalue;
}


void BabyGrid::SetHomeRow(int row,int col)
{
	RECT gridrect,cellrect;
	//get rect of grid window
	GetClientRect(hWnd,&gridrect);
	//get rect of current cell
	cellrect=GetCellRect(row,col);
	if((cellrect.bottom > gridrect.bottom)&&((cellrect.bottom - cellrect.top)<(gridrect.bottom-(headerrowheight+titleheight))))
	{
		while(cellrect.bottom > gridrect.bottom)
		{
			homerow++;
			if(row==rows)
			{
				gridrect.top = gridrect.bottom - (rowheight);
				MyInvalidateRect(hWnd,&gridrect,TRUE);
			}
			else
			{
				MyInvalidateRect(hWnd,&gridrect,FALSE);
			}
			cellrect=GetCellRect(row,col);
		}
	}
	else
	{
		if((cellrect.bottom - cellrect.top)>=(gridrect.bottom - (headerrowheight+titleheight)))
		{
			homerow++;
		}
	}
	cellrect=GetCellRect(row,col);
	{
		while((row < homerow))
		{
			homerow--;
			MyInvalidateRect(hWnd,&gridrect,FALSE);
			cellrect=GetCellRect(row,col);
		}
	}
	//set the vertical scrollbar position
	SetScrollPos(hWnd,SB_VERT,homerow,TRUE);
}

void BabyGrid::SetHomeCol(int row,int col)
{
	RECT gridrect;
	BOOL LASTCOLVISIBLE;
	//get rect of grid window
	GetClientRect(hWnd,&gridrect);
	//get rect of current cell
	RECT cellrect = GetCellRect(row,col);
	//determine if scroll left or right is needed
	while((cellrect.right > gridrect.right)&&(cellrect.left != columnwidths[0]))
	{
		//scroll right is needed
		homecol++;
		//see if last column is visible
		cellrect = GetCellRect(row,cols);
		if(cellrect.right <= gridrect.right)
		{
			LASTCOLVISIBLE=TRUE;
		}
		else
		{
			LASTCOLVISIBLE=FALSE;
		}
		cellrect = GetCellRect(row,col);
		MyInvalidateRect(hWnd,&gridrect,FALSE);
	}
	cellrect = GetCellRect(row,col);
	while((cursorcol < homecol)&&(homecol > 1))
	{
		//scroll left is needed
		homecol--;
		//see if last column is visible
		cellrect = GetCellRect(row,cols);
		if(cellrect.right <= gridrect.right)
		{
			LASTCOLVISIBLE=TRUE;
		}
		else
		{
			LASTCOLVISIBLE=FALSE;
		}

		cellrect = GetCellRect(row,col);
		MyInvalidateRect(hWnd,&gridrect,FALSE);
	}
	{
		SetScrollPos(hWnd,SB_HORZ,HomeColumnNthVisible(),TRUE);
	}
}

void BabyGrid::ShowVscroll()
{
	/*	ShowScrollBar(hWnd,SB_VERT,TRUE);
   	SetScrollRange(hWnd,SB_VERT,1,(rows-rowsvisibleonscreen)+1,TRUE);
		VSCROLL = TRUE;*/
	
	//if more rows than can be visible on grid, display vertical scrollbar
	//otherwise, hide it.
	RECT gridrect;
	int totalpixels;
	int rowsvisibleonscreen;
	GetClientRect(hWnd,&gridrect);
	totalpixels = gridrect.bottom;
	totalpixels -= titleheight;
	totalpixels -= headerrowheight;
	totalpixels -= rowheight * rows;
	rowsvisibleonscreen = (gridrect.bottom - (headerrowheight+titleheight)) / rowheight;
	if(totalpixels < 0)
	{
		//show vscrollbar
		ShowScrollBar(hWnd,SB_VERT,TRUE);
		SetScrollRange(hWnd,SB_VERT,1,(rows-rowsvisibleonscreen)+1,TRUE);
		VSCROLL = TRUE;
	}
	else
	{
		//hide vscrollbar
		ShowScrollBar(hWnd,SB_VERT,FALSE);
		VSCROLL = FALSE;
	}
}

void BabyGrid::ShowHscroll()
{
	//ShowScrollBar(hWnd,SB_HORZ,FALSE);
	//HSCROLL = FALSE;
	//if more rows than can be visible on grid, display vertical scrollbar
	//otherwise, hide it.
	RECT gridrect;
	int totalpixels;
	int colswithwidth;
	int j;
	GetClientRect(hWnd,&gridrect);
	totalpixels = gridrect.right;
	totalpixels -= columnwidths[0];
	colswithwidth = 0;
	for(j=1;j<=cols;j++)
	{
		totalpixels -= columnwidths[j];
		if(columnwidths[j]>0)
		{
			colswithwidth++;
		}
	}
	if(totalpixels < 0)
	{
		//show hscrollbar
		ShowScrollBar(hWnd,SB_HORZ,TRUE);
		SetScrollRange(hWnd,SB_HORZ,1,colswithwidth,TRUE);
		HSCROLL = TRUE;
	}
	else
	{
		//hide hscrollbar
		ShowScrollBar(hWnd,SB_HORZ,FALSE);
		HSCROLL = FALSE;
	}
}

void BabyGrid::NotifyRowChanged()
{
	WPARAM wParam;
	LPARAM lParam;
	lParam = MAKELPARAM(cursorrow,cursorcol);
	wParam=MAKEWPARAM((UINT)gridmenu,BGN_ROWCHANGED);
	SendMessage(GetParent(hWnd),WM_COMMAND,wParam,lParam);
	wParam=MAKEWPARAM((UINT)gridmenu,BGN_SELCHANGE);
	SendMessage(GetParent(hWnd),WM_COMMAND,wParam,lParam);
}


void BabyGrid::NotifyColChanged()
{
	WPARAM wParam;
	LPARAM lParam;
	lParam = MAKELPARAM(cursorrow,cursorcol);
	wParam=MAKEWPARAM((UINT)gridmenu,BGN_COLCHANGED);
	SendMessage(GetParent(hWnd),WM_COMMAND,wParam,lParam);
	wParam=MAKEWPARAM((UINT)gridmenu,BGN_SELCHANGE);
	SendMessage(GetParent(hWnd),WM_COMMAND,wParam,lParam);

}


void BabyGrid::NotifyEndEdit()
{
	WPARAM wParam;
	LPARAM lParam;
	lParam = MAKELPARAM(cursorrow,cursorcol);
	wParam=MAKEWPARAM((UINT)gridmenu,BGN_EDITEND);
	SendMessage(GetParent(hWnd),WM_COMMAND,wParam,lParam);

}



void BabyGrid::NotifyEditBegin()
{
	WPARAM wParam;
	LPARAM lParam;
	lParam = MAKELPARAM(cursorrow,cursorcol);
	wParam=MAKEWPARAM((UINT)gridmenu,BGN_EDITBEGIN);
	SendMessage(GetParent(hWnd),WM_COMMAND,wParam,lParam);

}

void BabyGrid::NotifyEditEnd()
{
	WPARAM wParam;
	LPARAM lParam;
	lParam = MAKELPARAM(cursorrow,cursorcol);
	wParam=MAKEWPARAM((UINT)gridmenu,BGN_EDITEND);
	SendMessage(GetParent(hWnd),WM_COMMAND,wParam,lParam);

}


void BabyGrid::handleControlKey(int code)
{
	WPARAM wParam;
	LPARAM lParam;
	lParam = MAKELPARAM(cursorrow,cursorcol);
	wParam=MAKEWPARAM((UINT)gridmenu,code);
	SendMessage(GetParent(hWnd),WM_COMMAND,wParam,lParam);

}


void BabyGrid::NotifyCellClicked()
{
	WPARAM wParam;
	LPARAM lParam;
	lParam = MAKELPARAM(cursorrow,cursorcol);
	wParam=MAKEWPARAM((UINT)gridmenu,BGN_CELLCLICKED);
	SendMessage(GetParent(hWnd),WM_COMMAND,wParam,lParam);
}


void BabyGrid::GetVisibleColumns()
{
	int j;
	int value;
	value=0;
	for(j=1;j<=cols;j++)
	{
		if(columnwidths[j] > 0)
		{
			value++;
		}
	}
	visiblecolumns = value;
	SetScrollRange(hWnd,SB_HORZ,1,value,TRUE);
}

int BabyGrid::GetNthVisibleColumn(int n)
{
	int j,count;
	int value;
	j=1;
	count=0;
	value = n-1;
	while(j<=cols)
	{
		if(columnwidths[j]>0)
		{
			count++;
			if(count==n)
			{
				value = j;
			}
		}
		j++;
	}
	return value;
}


void BabyGrid::CloseEdit()
{
	setCellData(cursorrow, cursorcol, editstring);
	strcpy(editstring,"");
	EDITING = FALSE;
	Refresh();
	HideCaret(hWnd);
	NotifyEditEnd();
}

void BabyGrid::DisplayEditString(char* tstring)
{
	int r,c;
	HFONT holdfont;
	RECT rt;
	HDC cdc;
	r=cursorrow;
	c=cursorcol;
	ShowCaret(hWnd);
	if((r<homerow)||(c<homecol))
	{
		HideCaret(hWnd);
		return;
	}
	rt=GetCellRect(r,c);
	rt.top += 2;
	rt.bottom -= 2;
	rt.right -=2;
	rt.left += 2;

	cdc=GetDC(hWnd);
	Rectangle(cdc,rt.left,rt.top,rt.right,rt.bottom);
	rt.top += 2;
	rt.bottom -= 2;
	rt.right -=2;
	rt.left += 2;

	if(strlen(editstring)<=300)
	{
		strcat(editstring,tstring);
		strcpy(editstringdisplay,editstring);
	}
	else
	{
		MessageBeep(0);
	}

	holdfont=(HFONT)SelectObject(cdc,hfont);
	rt.right -= 5;
	DrawText(cdc,editstringdisplay,-1,&rt,DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
	rt.right +=5;
	ShowCaret(hWnd);

	{
		int rh,ah;
		rh=rowheight;
		ah=fontascentheight;

		SetCaretPos(rt.right-4,rt.top+(int)(rh/2)-ah+2);

	}

	SelectObject(cdc,holdfont);
	ReleaseDC(hWnd,cdc);
}
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

BabyGrid::BabyGrid(HWND hWnd, HINSTANCE hInst)
{	
   extern HFONT cliFont,controlFont,controlFontBold;
	hlist1=CreateWindowEx(WS_EX_CLIENTEDGE,"LISTBOX","",WS_CHILD|LBS_STANDARD,50,150,200,100,hWnd,NULL,hInst,NULL);

   hfont = controlFont; 
	htitlefont = controlFont;
	hcolumnheadingfont = controlFontBold;

	this->hWnd = hWnd;
	this->hInst = hInst;

	gridmenu = NULL;
	strcpy(protect,"U");
	rows = 100;
	cols = 255;
	homerow = 1;
	homecol = 1;
	rowheight = 21;
	headerrowheight = 21;
	ROWSNUMBERED = FALSE;
	COLUMNSNUMBERED = FALSE;
	EDITABLE = TRUE;
	EDITING = FALSE;
	AUTOROW = TRUE;
	cursorcol = 1;  
	cursorrow = 1;  
	columnwidths[0]=50;
	ADVANCEROW = TRUE;
	DRAWHIGHLIGHT = FALSE;
	CURRENTCELLPROTECTED = FALSE;
	cursorcolor = RGB(128, 128, 128);
	protectcolor = RGB(255,255,255); 
	unprotectcolor = RGB(255,255,255);
	highlightcolor = RGB(128,128,128); 
	gridlinecolor = RGB(220,220,220);
	highlighttextcolor = RGB(255,255,255);
	textcolor = RGB(0,0,0);
	titleheight = 0;
	EXTENDLASTCOLUMN = FALSE;
	SHOWINTEGRALROWS = TRUE;
	SIZING = FALSE;
	ELLIPSIS = TRUE;
	COLAUTOWIDTH = FALSE;
	COLUMNSIZING = FALSE;
	ALLOWCOLUMNRESIZING = FALSE;
	cursortype = 0;
	//hcolumnheadingfont = NULL;
//	htitlefont = NULL;
	for (int i = 0; i < 305; i++)
	{
		editstring[i] = '\0';
	}
	for(int col=1;col<MAX_COLS;col++)
	{
		columnwidths[col]=50;
	}
	title[0] = '\0';
	editstringdisplay[0] = '\0';
	gridwidth = 0;
	gridheight = 0;
	leftvisiblecol = 0;
	rightvisiblecol = 0;
	topvisiblerow = 0;
	bottomvisiblerow = 0;
	ownerdrawitem = 0;
	visiblecolumns = 0;
	fontascentheight = 0;
	columntoresize = 0;
	columntoresizeinitsize = 0;
	columntoresizeinitx = 0;
	wannabeheight = 0;
	wannabewidth = 0;
}


ATOM BabyGrid::RegisterGridClass(HINSTANCE hInstance)
{	
	WNDCLASS wclass;
	//wclass.style = CS_BYTEALIGNWINDOW;//CS_HREDRAW|CS_VREDRAW;
	wclass.style = CS_HREDRAW|CS_VREDRAW;
	wclass.lpfnWndProc = (WNDPROC)GridEventProc;
	wclass.cbClsExtra = 0;
	wclass.cbWndExtra = 0;
	wclass.hInstance = hInstance;
	wclass.hIcon = NULL;
	wclass.hCursor = NULL;

	wclass.hbrBackground = (HBRUSH)(GetStockObject(GRAY_BRUSH));
	wclass.lpszClassName = "BABYGRID";
	wclass.lpszMenuName = NULL;

	return RegisterClass(&wclass);

}


void BabyGrid::SizeGrid()
{
	SendMessage(hWnd,WM_SIZE,SIZE_MAXIMIZED,MAKELPARAM(wannabewidth,wannabeheight));
	SendMessage(hWnd,WM_SIZE,SIZE_MAXIMIZED,MAKELPARAM(wannabewidth,wannabeheight));
	this->scaleCells();
}

int BabyGrid::FindLongestLine(HDC hdc,char* text,SIZE* size)
{
	char temptext[1000];
	
	int longest=0;
	int lines=1;
	for(int j=0;j<(int)strlen(text);j++)
	{
		if(text[j]=='\n')
		{
			lines++;
		}
	}
	strcpy(temptext,text);
	char* p = strtok(temptext,"\n");
	while(p)
	{
		GetTextExtentPoint32(hdc,p,strlen(p),size);
		if(size->cx > longest)
		{
			longest=size->cx;
		}
		p=strtok('\0',"\n");
	}

	//MessageBox(NULL,text,"FindLongestLine",MB_OK);
	return longest;
}

int BabyGrid::BinarySearchListBox(char* searchtext)
{
	int ReturnValue;
	int lbcount;
	int head,tail,finger;
	int FindResult;
	char tbuffer[1000];
	char headtext[1000];
	char tailtext[1000];
	int p;
	BOOL FOUND;

	FOUND=FALSE;
	//get count of items in listbox
	lbcount = SendMessage(hlist1,LB_GETCOUNT,0,0);
	if(lbcount == 0)
	{
		ReturnValue = LB_ERR;
		return ReturnValue;
	}
	if(lbcount < 12)
	{
		//not worth doing binary search, do regular search
		FindResult = SendMessage(hlist1,LB_FINDSTRING,-1,(long) searchtext);
		ReturnValue = FindResult;
		return ReturnValue;
	}

	// do a binary search
	head = 0;
	tail = lbcount - 1;

	//is it the head?
	SendMessage(hlist1,LB_GETTEXT,head,(long)headtext);
	headtext[9] = 0x00;

	p=strcmp(searchtext,headtext);
	if(p==0)
	{
		//it was the head
		ReturnValue = head;
		return ReturnValue;
	}
	if(p<0)
	{
		//it was less than the head... not found
		ReturnValue = LB_ERR;
		return ReturnValue;
	}



	//is it the tail?
	SendMessage(hlist1,LB_GETTEXT,tail,(long)tailtext);
	tailtext[9] = 0x00;
	p=strcmp(searchtext,tailtext);
	if(p==0)
	{
		//it was the tail
		ReturnValue = tail;
		return ReturnValue;
	}
	if(p>0)
	{
		//it was greater than the tail... not found
		ReturnValue = LB_ERR;
		return ReturnValue;
	}

	//is it the finger?
	ReturnValue = LB_ERR; 
	FOUND=FALSE;


	while((!FOUND)&&((tail-head)>1))
	{
		finger = head + ((tail - head) / 2);

		SendMessage(hlist1,LB_GETTEXT,finger,(long)tbuffer);
		tbuffer[9] = 0x00;
		p=strcmp(tbuffer,searchtext);
		if(p==0)
		{
			FOUND=TRUE;
			ReturnValue = finger;
		}

		if(p<0)
		{
			//change  tail to finger
			head = finger;
		}
		if(p>0)    
		{
			//change head to finger
			tail = finger;
		}


	}
	return ReturnValue;
}


int BabyGrid::getRows()
{
	return rows;
}

int BabyGrid::getCols()
{
	return cols;
}

int BabyGrid::getRowHeight()
{
	return rowheight;
}

int BabyGrid::getHeaderRowHeight()
{
	return headerrowheight;
}

void BabyGrid::paintGrid()
{
	RECT rect;
	
	GetClientRect(hWnd, &rect);
	MyInvalidateRect(hWnd,&rect,TRUE);
	MyUpdateWindow(hWnd);
	MessageBeep(0);
}

void BabyGrid::setCursorPos(int row, int col)
{
	DrawCursor();
	if(((row <= rows)&&(row > 0))&&
		((col <= cols)&&(col > 0)))
	{
		cursorrow=row;
		cursorcol=col;
	}
	else
	{
		DrawCursor();
		return;
	}
	SetHomeRow(cursorrow,cursorcol);
	SetHomeCol(cursorrow,cursorcol);
	DrawCursor();
	Refresh();
}

void BabyGrid::extendLastColumn(bool extend)
{
	EXTENDLASTCOLUMN = extend;
	Refresh();
}

void BabyGrid::showIntegralRows(bool show)
{
	SHOWINTEGRALROWS = show;
	SizeGrid();
	Refresh();
}

void BabyGrid::setColAutoWidth(bool autoWidth)
{
	COLAUTOWIDTH = autoWidth;
}

int BabyGrid::protectCell(int row, int col, bool protect)
{
	char buffer[1000];
	if (OutOfRange(row,col))
	{
		WPARAM wParam=MAKEWPARAM((UINT)GetMenu(hWnd),BGN_OUTOFRANGE);
		LPARAM lParam = 0;
		SendMessage(GetParent(hWnd),WM_COMMAND,wParam,lParam);
		return -1;
	}

	wsprintf(buffer,"%05d-%03d",row,col);
	//see if that cell is already loaded
	int FindResult = BinarySearchListBox(buffer);
	if(FindResult != LB_ERR)
	{
		//it was found, get the text, modify text delete it from list, add modified to list
		SendMessage(hlist1,LB_GETTEXT,FindResult,(long)buffer);
		buffer[10] = protect ? 'P' : 'U';
		SendMessage(hlist1,LB_DELETESTRING,FindResult,0);
		SendMessage(hlist1,LB_ADDSTRING,FindResult,(long)buffer);
	}
	else
	{
		//protecting or unprotecting a cell that isn't in the list
		//add it as blank;
		strcat(buffer,"|");
		strcat(buffer, protect ? "PA" : "UA");
		strcat(buffer,"|");
		SendMessage(hlist1,LB_ADDSTRING,FindResult,(long)buffer);
	}
	return 0;
}

void BabyGrid::setProtect(bool protect)
{
	strcpy(this->protect, protect ? "P" : "U");
}

void BabyGrid::setAutoRow(bool autoRow)
{
	AUTOROW = autoRow;
}

void BabyGrid::setEditable(bool editable)
{
	EDITABLE = editable;
}

void BabyGrid::clear()
{	
	SendMessage(hlist1,LB_RESETCONTENT,0,0);
	rows = 0;
	cursorrow = homerow = homecol = 1;
	RECT rect;
	GetClientRect(hWnd,&rect);
	MyInvalidateRect(hWnd,&rect,TRUE);
}

void BabyGrid::setColCount(int cols)
{
	if ((cols >= MIN_COLS) && (cols <= MAX_COLS))
	{
		this->cols = cols;
	}
	else 
	{
		if (cols < MIN_COLS)
		{
			this->cols = MIN_COLS;
		}
		else
		{
			this->cols = MAX_COLS;
		}
	}
	RECT rect;
	GetClientRect(hWnd, &rect);
	MyInvalidateRect(hWnd, &rect, TRUE);
	GetVisibleColumns();
}

void BabyGrid::setRowCount(int rows)
{
	if ((rows >= MIN_ROWS) && (rows <= MAX_ROWS))
	{
		this->rows = rows;
	}
	else 
	{
		if (rows < MIN_ROWS)
		{
			this->rows = MIN_ROWS;
		}
		else
		{
			this->rows = MAX_ROWS;
		}
	}
	RECT rect;
	GetClientRect(hWnd, &rect);
	MyInvalidateRect(hWnd, &rect, TRUE);
	GetVisibleColumns();
}

int BabyGrid::setCellData(int row, int col, double num)
{
	char buffer[1000];
	sprintf(buffer, "%f",num);
	buffer[999] = '\0';
	return setCellData(row,col,buffer);
}

int BabyGrid::setCellData(int row, int col, int num)
{
	char buffer[1000];
	sprintf(buffer,"%d",num);
	buffer[999] = '\0';
	return setCellData(row,col,buffer);
}

int BabyGrid::setCellData(int row, int col, char* data)
{
	char buffer[1000];

	if(OutOfRange(row,col))
	{
		WPARAM wParam=MAKEWPARAM((UINT)GetMenu(hWnd),BGN_OUTOFRANGE);
		LPARAM lParam = 0;
		SendMessage(GetParent(hWnd),WM_COMMAND,wParam,lParam);
		return -1;
	}
	wsprintf(buffer,"%05d-%03d",row,col);
	//see if that cell is already loaded
	int FindResult = BinarySearchListBox(buffer);
	if(FindResult != LB_ERR)
	{
		//it was found, delete it
		SendMessage(hlist1,LB_DELETESTRING,FindResult,0);
	}
	//now add it
	strcat(buffer,"|");
	strcat(buffer,protect);
	//determine data type (text,numeric, or boolean)(1,2,3)
	int iDataType=DetermineDataType((char*)data);
	if(iDataType==1){strcat(buffer,"A");}
	if(iDataType==2){strcat(buffer,"N");}
	if(iDataType==3){strcat(buffer,"T");}
	if(iDataType==4){strcat(buffer,"F");}
	if(iDataType==5){strcat(buffer,"G");}

	strcat(buffer,"|");
	strcat(buffer,(char*)data);
	FindResult=SendMessage(hlist1,LB_ADDSTRING,0,(long)buffer);

	if(FindResult==LB_ERR)
	{
		MessageBeep(0);
	}
	{
		RECT rect;
		rect=GetCellRect(row,col);
		MyInvalidateRect(hWnd,&rect,FALSE);
	}
	//get the last line and adjust grid dimmensions
	if(AUTOROW)
	{
		int j=SendMessage(hlist1,LB_GETCOUNT,0,0);
		if(j>0)
		{
			SendMessage(hlist1,LB_GETTEXT,j-1,(long)buffer);
			buffer[5]=0x00;
			j=atoi(buffer);
			if(j>rows)
			{
				rows = j;
			}
		}
		else
		{
			//no items in the list
			rows = j;
		}
	}

	//adjust the column width if COLAUTOWIDTH==TRUE
	if(COLAUTOWIDTH || (row == 0))
	{
		SIZE size;
		HFONT holdfont;
		HDC hdc=GetDC(hWnd);
		if(row == 0)
		{
			holdfont=(HFONT)SelectObject(hdc,hcolumnheadingfont);
		}
		else
		{
			holdfont=(HFONT)SelectObject(hdc,hfont);
		}
		//if there are \n codes in the string, find the longest line
		int longestline=FindLongestLine(hdc,data,&size);
		//GetTextExtentPoint32(hdc,(char*)lParam,strlen((char*)lParam),&size);
		int required_width = longestline+5;
		int required_height = size.cy;
		//count lines
		{
			int count=1;
			char tbuffer[255];
			strcpy(tbuffer,(char*)data);
			for(int j=0;j<(int)strlen(tbuffer);j++)
			{
				if(tbuffer[j]=='\n'){count++;}
			}
			if((!ELLIPSIS)||(row == 0))
			{
				required_height *= count;
			}
			required_height +=5;
		}
		SelectObject(hdc,holdfont);
		ReleaseDC(hWnd,hdc);
		int current_width = columnwidths[col];
		if(row == 0)
		{
			int current_height = headerrowheight;
			if(required_height > current_height)
			{
				setHeaderRowHeight(required_height);
			}
		}
		else
		{
			int current_height = rowheight;
			if(required_height > current_height)
			{
				setRowHeight(required_height);
			}
		}
		if(required_width > current_width)
		{
			setColWidth(col, required_width);
		}
		ReleaseDC(hWnd,hdc);
	}
	return 0;
}


CellDataType BabyGrid::getCellData(int row, int col, char* result)
{
	char buffer[1000];

	if(OutOfRange(row,col))
	{
		WPARAM wParam=MAKEWPARAM((UINT)GetMenu(hWnd),BGN_OUTOFRANGE);
		LPARAM lParam = 0;
		SendMessage(GetParent(hWnd),WM_COMMAND,wParam,lParam);
		return CELLDATATYPE_NONE;
	}
	wsprintf(buffer,"%05d-%03d",row,col);
	//see if that cell is already loaded
	int FindResult = BinarySearchListBox(buffer);
	if(FindResult != LB_ERR)
	{
		char tbuffer[1000];
		//it was found, get it
		SendMessage(hlist1,LB_GETTEXT,FindResult,(long)result);
		strcpy(tbuffer,result);
		int k=strlen(tbuffer);
		int c=0;
		for(int j=13;j<k;j++)
		{
			buffer[c]=tbuffer[j];
			c++;
		}
		buffer[c]=0x00;
		strcpy(result,buffer);
	}
	else
	{
		strcpy(result,"");
	}
	return DetermineDataType(result);
}

int BabyGrid::deleteCell(int row, int col)
{
	char buffer[1000];
	if(OutOfRange(row, col))
	{
		WPARAM wParam=MAKEWPARAM((UINT)GetMenu(hWnd),BGN_OUTOFRANGE);
		LPARAM lParam = 0;
		SendMessage(GetParent(hWnd),WM_COMMAND,wParam,lParam);
		return -1;
	}
	wsprintf(buffer,"%05d-%03d",row,col);
	//see if that cell is already loaded
	int FindResult = BinarySearchListBox(buffer);
	if(FindResult != LB_ERR)
	{
		//it was found, delete it
		SendMessage(hlist1,LB_DELETESTRING,FindResult,0);
		NotifyEndEdit();
	}
	return 0;
}

int BabyGrid::setColWidth(int col, int width)
{
	if ((col <= MAX_COLS) && (col >= 0) && (width >=0))
	{
		columnwidths[col] = width;
		invalidateClientRect();
		GetVisibleColumns();
		return OK;
	}
	return ERR;
}

void BabyGrid::setHeaderRowHeight(int height)
{
	if (height >= 0)
	{
		headerrowheight = height;
		SizeGrid();
		invalidateClientRect();
	}
}

int BabyGrid::getCurrentRow()
{
	return cursorrow;
}

int BabyGrid::getCurrentCol()
{
	return cursorcol;
}

CellDataType BabyGrid::getType(int row, int col)
{
	char buffer[1000];

	if(OutOfRange(row, col))
	{
		WPARAM wParam=MAKEWPARAM((UINT)GetMenu(hWnd),BGN_OUTOFRANGE);
		LPARAM lParam = 0;
		SendMessage(GetParent(hWnd),WM_COMMAND,wParam,lParam);
		return CELLDATATYPE_NONE;
	}
	wsprintf(buffer,"%05d-%03d",row,col);
	//see if that cell is already loaded
	int FindResult = BinarySearchListBox(buffer);
	if(FindResult != LB_ERR)
	{
		//it was found, get it
		SendMessage(hlist1,LB_GETTEXT,FindResult,(long)buffer);
		switch (buffer[11])
		{
		case 'A':return ALPHA;break;
		case 'N':return NUMERIC;break;
		case 'T':return BOOL_T;break;
		case 'F':return BOOL_F;break;
		case 'G':return USER_IMG;break;
		default: return ALPHA;break;
		}
	}
	return CELLDATATYPE_NONE;
}

int BabyGrid::getProtection(int row, int col)
{
	char buffer[1000];

	if(OutOfRange(row, col))
	{
		WPARAM wParam=MAKEWPARAM((UINT)GetMenu(hWnd),BGN_OUTOFRANGE);
		LPARAM lParam = 0;
		SendMessage(GetParent(hWnd),WM_COMMAND,wParam,lParam);
		return -1;
	}
	wsprintf(buffer,"%05d-%03d",row,col);
	//see if that cell is already loaded
	int FindResult = BinarySearchListBox(buffer);
	if(FindResult != LB_ERR)
	{
		//it was found, get it
		SendMessage(hlist1,LB_GETTEXT,FindResult,(long)buffer);
		switch (buffer[10])
		{
			case 'U':return 0; 
			case 'P':return 1;
			default: return 0;
		}
	}
	return -1;
}

void BabyGrid::setRowHeight(int height)
{
	if (height < 1) 
	{
		height = 1;
	}

	rowheight = height;

	SetHomeRow(cursorrow,cursorcol);
	SetHomeCol(cursorrow,cursorcol);
	SizeGrid();
	invalidateClientRect();
}

void BabyGrid::setTitleHeight(int height)
{
	if(height<0)
	{
		height=0;
	}
	titleheight = height;
	SetHomeRow(cursorrow,cursorcol);
	SetHomeCol(cursorrow,cursorcol);
	invalidateClientRect();
}

int BabyGrid::getTitleHeight()
{
	return titleheight;
}

void BabyGrid::setGridLineColor(COLORREF color)
{
	DrawCursor();
	gridlinecolor = color;
	DrawCursor();
	Refresh();
}

void BabyGrid::setCursorColor(COLORREF color)
{
	DrawCursor();
	cursorcolor = color;
	DrawCursor();
	Refresh();
}

void BabyGrid::invalidateClientRect()
{
	RECT rect;
	GetClientRect(hWnd,&rect);
	MyInvalidateRect(hWnd,&rect,FALSE);
}

void BabyGrid::setHilightTextColor(COLORREF color)
{
	highlighttextcolor = color;
	invalidateClientRect();
}

void BabyGrid::setHilightColor(COLORREF color)
{
	highlightcolor = color;
	invalidateClientRect();
}

void BabyGrid::setProtectColor(COLORREF color)
{	
	protectcolor = color;
	invalidateClientRect();	
}

void BabyGrid::setUnprotectColor(COLORREF color)
{	
	unprotectcolor = color;
	invalidateClientRect();	
}

void BabyGrid::setEllipsis(bool ellipsis)
{
	ELLIPSIS = ellipsis;
	invalidateClientRect();
}


void BabyGrid::setTitleFont(HFONT titleFont)
{
	htitlefont = titleFont;
	invalidateClientRect();
}

void BabyGrid::setHeadingFont(HFONT headingFont)
{
	hcolumnheadingfont = headingFont;
	invalidateClientRect();
}

void BabyGrid::setRowsNumbered(bool numbered)
{
	ROWSNUMBERED = numbered;
	invalidateClientRect();
}

bool BabyGrid::getRowsNumbered()
{
	return ROWSNUMBERED;
}


void BabyGrid::setColsNumbered(bool numbered)
{
	COLUMNSNUMBERED = numbered;
	invalidateClientRect();
}

bool BabyGrid::getColsNumbered()
{
	return COLUMNSNUMBERED;
}

void BabyGrid::setZeroBased(bool zeroBased)
{
	ZEROBASED = zeroBased;
	invalidateClientRect();
}

void BabyGrid::setGridWidth(int width)
{
	if (width >= 0)
	{
		gridwidth = width;
	}
}
void BabyGrid::setGridHeight(int height)
{
	if (height >= 0)
	{
		gridheight = height;
	}
}

int BabyGrid::getColWidth(int col)
{
	if ((col <= MAX_COLS) && (col >= 0))
	{
		return columnwidths[col];
	}
	return -1;
}

int BabyGrid::getLeftVisibleCol()
{
	return leftvisiblecol;
}

int BabyGrid::getRightVisibleCol()
{
	return rightvisiblecol;
}

void BabyGrid::setTitle(char* title)
{
	if (strlen(title) > 300)
		strcpy(this->title, "Title too long (300 chars max)");
	else
		strcpy(this->title, title);
}

char* BabyGrid::getTitle()
{
	return title;
}

COLORREF BabyGrid::getTextColor()
{
	return textcolor;	
}

void BabyGrid::handle_WM_PAINT()
{
	PAINTSTRUCT ps;
	BeginPaint(hWnd, &ps);
	RECT rt;
	GetClientRect(hWnd, &rt);
	
	scaleCells();
	CalcVisibleCellBoundaries();
	//display title
	DisplayTitle();
	//display column 0;

	DisplayColumn(0,0);
	{
		int offset = columnwidths[0];
		for(int c = leftvisiblecol; c <= rightvisiblecol; c++)
		{
			DisplayColumn(c,offset);
			offset += columnwidths[c];
		}
	}
	EndPaint(hWnd, &ps);
	//
	if(GetFocus()==hWnd)
	{
		DrawCursor();
	}
}

void BabyGrid::handle_WM_SETTEXT(char* title)
{ 
	SIZE size;
	
	setTitle(title);

	HDC gdc = GetDC(hWnd);
	//get linecount of title;
	char* myTitle = getTitle();
	int title_length = strlen(myTitle);
	if(title_length > 0)
	{
		int linecount=1;
		for(int j=0;j<title_length;j++)     
		{
			if(myTitle[j]=='\n')
			{
				linecount++;
			}
		}
		HFONT holdfont=(HFONT)SelectObject(gdc,htitlefont);
		GetTextExtentPoint32(gdc,myTitle,title_length,&size);
		SelectObject(gdc,holdfont);
		setTitleHeight((int)((size.cy*1.2) * linecount));
	}
	else
	{
		//no title
		setTitleHeight(0);
	}
	ReleaseDC(hWnd,gdc);
	Refresh();
	SizeGrid();
}

void BabyGrid::handle_WM_ENABLE(BOOL enable)
{
	if (enable == FALSE)
	{
		setTextColor(RGB(120,120,120));
	}
	else
	{
		setTextColor(RGB(0,0,0));
	}
}

void BabyGrid::handle_WM_MOUSEMOVE(int x, int y)
{

	int r = GetRowOfMouse(y);
	int c = GetColOfMouse(x);
	int t = GetColOfMouse(x+10);
	int z = GetColOfMouse(x-10);

	if(COLUMNSIZING)
	{
		int dx = x - columntoresizeinitx;
		int nx = columntoresizeinitsize + dx;
		if(nx<=0){nx=0;}
		setColWidth(columntoresize, nx);
	}
	if((r==0)&&(c>=-1)&&((t!=c)||(z!=c))&&(!COLUMNSIZING))
	{
		if((cursortype != 2)&&(ALLOWCOLUMNRESIZING))
		{
			cursortype = 2;
			SetCursor(LoadCursor(NULL, IDC_SIZEWE));
		}
	}
	else
	{
		if((cursortype != 1)&&(!COLUMNSIZING))
		{
			cursortype = 1;
			SetCursor(LoadCursor(NULL, IDC_ARROW));
		}
	}
	scaleCells();
}

void BabyGrid::handle_WM_LBUTTONUP()
{
	if(COLUMNSIZING)
	{
		COLUMNSIZING = FALSE;
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		cursortype = 1;
		SHOWINTEGRALROWS=REMEMBERINTEGRALROWS;
	}
}

void BabyGrid::handle_WM_LBUTTONDOWN(int x, int y)
{
	//check for column sizing
	if(cursortype == 2)
	{
		//start column sizing
		if(!COLUMNSIZING)
		{
			REMEMBERINTEGRALROWS = SHOWINTEGRALROWS;
		}
		COLUMNSIZING = TRUE;
		SHOWINTEGRALROWS = FALSE;
		columntoresizeinitx = x;
		int t = GetColOfMouse(x+10);
		int z = GetColOfMouse(x-10);
		int c = GetColOfMouse(x);
		if(t!=c)
		{
			//resizing column c
			columntoresize = c;
		}
		if(z!=c)
		{
			//resizing hidden column to the left of cursor
			if(c==-1)
			{
				c = cols;
			}
			else
			{
				c -= 1;
			}
			columntoresize = c;
		}

		columntoresizeinitsize = columnwidths[c];
	}

	if(EDITING)
	{
		CloseEdit();
	}
	else
	{
		SetFocus(hWnd);
	}
	BOOL NRC = FALSE;
	BOOL NCC = FALSE;

	if(GetFocus()==hWnd)
	{
		int r = GetRowOfMouse(y);
		int c = GetColOfMouse(x);
		DrawCursor();
		if((r>0)&&(c>0))
		{
			if(r != cursorrow)
			{
				cursorrow = r;
				NRC=TRUE;
			}
			else
			{
				cursorrow = r;
			}
			if(c != cursorcol)
			{
				cursorcol = c;
				NCC=TRUE;
			}
			else
			{
				cursorcol = c;
			}
			NotifyCellClicked();
		}
		if(NRC){NotifyRowChanged();}
		if(NCC){NotifyColChanged();}

		DrawCursor();
		SetCurrentCellStatus();
		SetHomeRow(cursorrow,cursorcol);
		SetHomeCol(cursorrow,cursorcol);
		Refresh();
	}
	else
	{
		SetFocus(hWnd);
	}
}

int BabyGrid::handle_WM_GETDLGCODE(WPARAM wParam)
{
	int ReturnValue = DLGC_WANTARROWS|DLGC_WANTCHARS|DLGC_DEFPUSHBUTTON;
	if(wParam == 13)
	{
		//same as arrow down
		if(EDITING)
		{
			CloseEdit();
		}
		DrawCursor();
		cursorrow++;
		if(cursorrow > rows)
		{
			cursorrow = rows;
		}
		else
		{
			NotifyRowChanged();
		}
		DrawCursor();
		SetCurrentCellStatus();
		SetHomeRow(cursorrow,cursorcol);
		Refresh();
		EDITING = FALSE;
	}

	if(wParam == VK_ESCAPE)
	{
		if(EDITING)
		{
			EDITING = FALSE;
			strcpy(editstring,"");
			HideCaret(hWnd);
			Refresh();
			NotifyEditEnd();
		}
		else
		{
			ReturnValue = 0;
		}
	}
	return ReturnValue;
}

void BabyGrid::handle_VK_NEXT()
{
	RECT gridrect;
	if(EDITING)
	{
		CloseEdit();
	}

	if(rows == 0){return;}
	if(cursorrow == rows){return;}
	//get rows per page
	GetClientRect(hWnd,&gridrect);
	int rpp = (gridrect.bottom - (headerrowheight+titleheight)) / rowheight;
	DrawCursor();
	cursorrow += rpp;

	if(cursorrow > rows)
	{
		cursorrow = rows;
	}
	NotifyRowChanged();
	DrawCursor();
	SetCurrentCellStatus();
	SetHomeRow(cursorrow,cursorcol);
	SetHomeCol(cursorrow,cursorcol);
	Refresh();
}

void BabyGrid::handle_VK_PRIOR()
{
	RECT gridrect;
	if(EDITING)
	{
		CloseEdit();
	}

	if(rows == 0){return;}
	if(cursorrow == 1){return;}
	//get rows per page
	GetClientRect(hWnd,&gridrect);
	int rpp = (gridrect.bottom - (headerrowheight+titleheight))/rowheight;
	DrawCursor();
	cursorrow -= rpp;
	if(cursorrow < 1)
	{
		cursorrow = 1;
	}
	NotifyRowChanged();
	DrawCursor();
	SetCurrentCellStatus();
	SetHomeRow(cursorrow,cursorcol);
	SetHomeCol(cursorrow,cursorcol);
	Refresh();
}

void BabyGrid::handle_VK_DOWN()
{
	if(EDITING)
	{
		CloseEdit();
	}
	if(rows == 0){return;}
	if(cursorrow == rows){return;}
	DrawCursor();
	cursorrow++;
	if(cursorrow > rows)
	{
		cursorrow = rows;
	}
	else
	{
		NotifyRowChanged();
	}
	DrawCursor();
	SetCurrentCellStatus();
	SetHomeRow(cursorrow,cursorcol);
	SetHomeCol(cursorrow,cursorcol);
	Refresh();
}

void BabyGrid::handle_VK_UP()
{
	if(EDITING)
	{
		CloseEdit();
	}

	if(rows == 0){return;}
	if(cursorrow == 1){return;}

	DrawCursor();
	cursorrow --;
	if(cursorrow < 1)
	{
		cursorrow = 1;
	}
	else
	{
		NotifyRowChanged();
	}
	DrawCursor();
	SetCurrentCellStatus();
	SetHomeRow(cursorrow,cursorcol);
	SetHomeCol(cursorrow,cursorcol);
	Refresh();
}

void BabyGrid::handle_VK_LEFT()
{
	if(EDITING)
	{
		CloseEdit();
	}

	if(!GetNextColWithWidth(cursorcol,BACKWARDS))
	{
		return;
	}
	DrawCursor();

	int k=GetNextColWithWidth(cursorcol,BACKWARDS);
	if(k)
	{
		cursorcol = k;
		NotifyColChanged();
	}
	DrawCursor();
	SetCurrentCellStatus();
	SetHomeRow(cursorrow,cursorcol);
	SetHomeCol(cursorrow,cursorcol);
}

void BabyGrid::handle_VK_RIGHT()
{
	if(EDITING)
	{
		CloseEdit();
	}
	DrawCursor();
	int k=GetNextColWithWidth(cursorcol,FORWARDS);
	if(k)
	{
		cursorcol = k;
		NotifyColChanged();
	}
	DrawCursor();
	SetCurrentCellStatus();
	SetHomeRow(cursorrow,cursorcol);
	SetHomeCol(cursorrow,cursorcol);
	Refresh();
}

void BabyGrid::handleGenericKeypress(WPARAM wParam, LPARAM lParam)
{
	if(CURRENTCELLPROTECTED && (wParam == 13))
	{
		DrawCursor();
		cursorrow ++;
		if(cursorrow > rows)
		{
			cursorrow = rows;
		}
		else
		{
			NotifyRowChanged();
		}
		DrawCursor();
		SetCurrentCellStatus();
		SetHomeRow(cursorrow,cursorcol);
		Refresh();
		return;
	}

	if(CURRENTCELLPROTECTED)
	{
		return;
	}

	if(!EDITABLE)
	{
		int ascii =  GetASCII(wParam,lParam);
		if(ascii == 13) //enter pressed, treat as arrow down
		{
			//same as arrow down
			DrawCursor();
			cursorrow++;
			if(cursorrow > rows)
			{
				cursorrow = rows;
			}
			else
			{
				NotifyRowChanged();
			}
			DrawCursor();
			SetCurrentCellStatus();
			SetHomeRow(cursorrow,cursorcol);
			Refresh();
			return;
		}

	}

	//if it's not an arrow key, make an edit box in the active cell rectangle
	if(EDITABLE && (rows > 0))
	{

		SetHomeRow(cursorrow,cursorcol);
		DrawCursor();
		{
			int ascii = GetASCII(wParam,lParam);
			wParam = ascii;
			if((wParam >= 32)&&(wParam <= 125))
			{
				char tstring[2];
				if(!EDITING)
				{
					NotifyEditBegin();
				}
				EDITING = TRUE;
				tstring[0]=wParam;
				tstring[1]=0x00;
				DisplayEditString(tstring);
				return;
			}
			if(wParam == 8) //backspace
			{
				if(!EDITING)
				{
					NotifyEditBegin();
				}

				EDITING = TRUE;
				if(strlen(editstring)==0)
				{
					DisplayEditString("");
					return;
				}
				else
				{
					int j=strlen(editstring);
					editstring[j-1]=0x00;
					DisplayEditString("");
				}
				return;
			}
			if(wParam == 13)
			{				
				//same as arrow down
				handle_VK_DOWN();
			}
		}
	}
}


void BabyGrid::handle_WM_HSCROLL(WPARAM wParam, LPARAM lParam)
{
	SetFocus(hWnd);
	if((LOWORD(wParam==SB_LINERIGHT))||(LOWORD(wParam)==SB_PAGERIGHT))
	{
		int cp,np;
		cp=GetScrollPos(hWnd,SB_HORZ);
		SetScrollPos(hWnd,SB_HORZ,cp+1,TRUE);
		cp=GetScrollPos(hWnd,SB_HORZ);
		np=GetNthVisibleColumn(cp);
		homecol = np;
		SetScrollPos(hWnd,SB_HORZ,cp,TRUE);
		Refresh();
	}  
	if((LOWORD(wParam==SB_LINELEFT))||(LOWORD(wParam)==SB_PAGELEFT))
	{
		int cp,np;
		cp=GetScrollPos(hWnd,SB_HORZ);
		SetScrollPos(hWnd,SB_HORZ,cp-1,TRUE);
		cp=GetScrollPos(hWnd,SB_HORZ);
		np=GetNthVisibleColumn(cp);
		homecol = np;
		SetScrollPos(hWnd,SB_HORZ,cp,TRUE);
		Refresh();
	}  
	if(LOWORD(wParam)==SB_THUMBTRACK)
	{
		int cp,np;
		cp=HIWORD(wParam);
		np=GetNthVisibleColumn(cp);
		SetScrollPos(hWnd,SB_HORZ,np,TRUE);
		homecol = np;
		SetScrollPos(hWnd,SB_HORZ,cp,TRUE);
		Refresh();
	}
}

void BabyGrid::handle_WM_VSCROLL(WPARAM wParam, LPARAM lParam)
{
	SetFocus(hWnd);
	if(LOWORD(wParam)==SB_THUMBTRACK)
	{
		RECT gridrect;
		int min,max;
		homerow = HIWORD(wParam);
		SetScrollPos(hWnd,SB_VERT,HIWORD(wParam),TRUE);
		GetClientRect(hWnd,&gridrect);
		GetScrollRange(hWnd,SB_VERT,&min,&max);
		if(HIWORD(wParam)==max)
		{
			gridrect.top = gridrect.bottom - rowheight;
			MyInvalidateRect(hWnd,&gridrect,TRUE);
		}
		else
		{
			MyInvalidateRect(hWnd,&gridrect,FALSE);
		}
	}

	if(LOWORD(wParam)==SB_PAGEDOWN)
	{
		RECT gridrect;
		int min,max,sp,rpp;
		//get rows per page
		GetClientRect(hWnd,&gridrect);
		rpp = (gridrect.bottom - (headerrowheight + titleheight)) / rowheight;
		GetScrollRange(hWnd,SB_VERT,&min,&max);
		sp=GetScrollPos(hWnd,SB_VERT);
		sp += rpp;
		if(sp > max){sp=max;}
		homerow = sp;
		SetScrollPos(hWnd,SB_VERT,sp,TRUE);
		SetHomeRow(sp,homecol);
		if(sp==max)
		{
			gridrect.top = gridrect.bottom - rowheight;
			MyInvalidateRect(hWnd,&gridrect,TRUE);
		}
		else
		{
			MyInvalidateRect(hWnd,&gridrect,FALSE);
		}

	}
	if(LOWORD(wParam)==SB_LINEDOWN)
	{
		RECT gridrect;
		int min,max,sp;
		//get rows per page
		GetClientRect(hWnd,&gridrect);
		GetScrollRange(hWnd,SB_VERT,&min,&max);
		sp=GetScrollPos(hWnd,SB_VERT);
		sp += 1;
		if(sp > max){sp=max;}
		homerow = sp;
		SetScrollPos(hWnd,SB_VERT,sp,TRUE);
		SetHomeRow(sp,homecol);
		if(sp==max)
		{
			gridrect.top = gridrect.bottom - rowheight;
			MyInvalidateRect(hWnd,&gridrect,TRUE);
		}
		else
		{
			MyInvalidateRect(hWnd,&gridrect,FALSE);
		}
	}

	if(LOWORD(wParam)==SB_PAGEUP)
	{
		RECT gridrect;
		int min,max,sp,rpp;
		//get rows per page
		GetClientRect(hWnd,&gridrect);
		rpp = (gridrect.bottom - (headerrowheight+titleheight)) / rowheight;
		GetScrollRange(hWnd,SB_VERT,&min,&max);
		sp = GetScrollPos(hWnd,SB_VERT);
		sp -= rpp;
		if(sp < 1){sp=1;}
		homerow = sp;
		SetScrollPos(hWnd,SB_VERT,sp,TRUE);
		SetHomeRow(sp,homecol);
		if(sp==max)
		{
			gridrect.top = gridrect.bottom - rowheight;
			MyInvalidateRect(hWnd,&gridrect,TRUE);
		}
		else
		{
			MyInvalidateRect(hWnd,&gridrect,FALSE);
		}

	}
	if(LOWORD(wParam)==SB_LINEUP)
	{
		RECT gridrect;
		int min,max,sp;
		//get rows per page
		GetClientRect(hWnd,&gridrect);
		sp=GetScrollPos(hWnd,SB_VERT);
		GetScrollRange(hWnd,SB_VERT,&min,&max);
		sp -= 1;
		if(sp < 1){sp=1;}
		homerow = sp;
		SetScrollPos(hWnd,SB_VERT,sp,TRUE);
		SetHomeRow(sp,homecol);
		if(sp==max)
		{
			gridrect.top = gridrect.bottom - rowheight;
			MyInvalidateRect(hWnd,&gridrect,TRUE);
		}
		else
		{
			MyInvalidateRect(hWnd,&gridrect,FALSE);
		}
	}
	Refresh();
}

void BabyGrid::handle_WM_SETFOCUS()
{
	DrawCursor();
	GRIDHASFOCUS	= TRUE; 
	DrawCursor();
	SetCurrentCellStatus();
	SetHomeRow(cursorrow,cursorcol);
	SetHomeCol(cursorrow,cursorcol);

	WPARAM wParam=MAKEWPARAM((UINT)GetMenu(hWnd),BGN_GOTFOCUS);
	LPARAM lParam = 0;
	SendMessage(GetParent(hWnd),WM_COMMAND,wParam,lParam);
	{TEXTMETRIC tm;
	HDC hdc;
	hdc=GetDC(hWnd);
	GetTextMetrics(hdc,&tm);
	ReleaseDC(hWnd,hdc);
	fontascentheight = (int)tm.tmAscent;
	CreateCaret(hWnd,NULL,3,tm.tmAscent);
	}
	Refresh();
}

void BabyGrid::handle_WM_KILLFOCUS()
{
	DestroyCaret();
	DrawCursor();
	GRIDHASFOCUS	= FALSE;

	WPARAM wParam=MAKEWPARAM((UINT)GetMenu(hWnd),BGN_LOSTFOCUS);
	LPARAM lParam = 0;
	SendMessage(GetParent(hWnd),WM_COMMAND,wParam,lParam);
	Refresh();
}

void BabyGrid::handle_WM_SETFONT(HFONT hFont)
{		
	if(!hcolumnheadingfont)
	{
		hcolumnheadingfont = hFont;
	}
	if(!htitlefont)
	{
		htitlefont = hFont;
	}
	Refresh();
}

void BabyGrid::handle_WM_SIZE(int in_height, int in_width)
{
	static int cheight;
	static int savewidth,saveheight;

	if(SIZING)
	{
		SIZING = FALSE;
		return;
	}
	ShowHscroll();
	ShowVscroll();

	if((SHOWINTEGRALROWS)&&(VSCROLL))
	{
		saveheight = in_height;
		savewidth = in_width;
		cheight = in_height;
		cheight -= titleheight;
		cheight -= headerrowheight;
		{
			int sbheight = GetSystemMetrics(SM_CYHSCROLL);
			if(HSCROLL)
			{
				cheight -= sbheight;
			}
			if(VSCROLL)
			{
				RECT grect,prect;
				GetClientRect(hWnd,&grect);
				GetClientRect(GetParent(hWnd),&prect);
				if((grect.right+sbheight) < prect.right)
				{
					savewidth += sbheight;
				}
			}
		}

		if(cheight <= rowheight)
		{
			return;
		}
		else
		{
			//calculate fractional part of cheight/rowheight
			int remainder,nrows;
			nrows=(int)(cheight/rowheight);
			remainder=cheight-(nrows * rowheight);
			//make the window remainder pixels shorter
			saveheight -= remainder;
			saveheight +=4; //+=4
			int intout=saveheight;
			WINDOWPLACEMENT wp;
			RECT crect;
			wp.length = sizeof(wp);
			GetWindowPlacement(hWnd,&wp);
			crect=wp.rcNormalPosition;
			crect.bottom=intout;
			crect.right=savewidth;
			SIZING = TRUE;

			wannabeheight = in_height;
			wannabewidth = in_width;

			MoveWindow(hWnd,crect.left,crect.top,crect.right,crect.bottom,TRUE);
		}
	}
}

void BabyGrid::setTextColor(COLORREF color)
{
	textcolor = color;
}


bool BabyGrid::validColumn(int col)
{
	if ((col < 1) || (col > cols))
	{
		return false;
	}
	return true;
}

bool BabyGrid::validRow(int row)
{
	if ((row < 1) || (row > rows))
	{
		return false;
	}
	return true;	
}


int BabyGrid::setRowLabel(int row, char* label)
{
	if (validRow(row))
	{
		this->setCellData(row, 0, label);
		return OK;
	}	
	return ERR;
}


int BabyGrid::setColumnLabel(int col, char* label)
{
	if (validColumn(col))
	{
		this->setCellData(0, col, label);
		return OK;
	}	
	return ERR;
}

bool BabyGrid::getZeroBased()
{
	return ZEROBASED;
}


int BabyGrid::protectRow(int row, bool protect)
{
	if (!this->validRow(row))
	{
		return ERR;
	}
	for (int col = 1; col <= cols; col++)
	{
		protectCell(row,col,protect);
	}
	return OK;
}

int BabyGrid::protectColumn(int col, bool protect)
{
	if (!this->validColumn(col))
	{
		return ERR;
	}
	for (int row = 1; row <= rows; row++)
	{
		protectCell(row,col,protect);
	}
	return OK;
}

int BabyGrid::handle_WM_KEYDOWN(WPARAM wParam, LPARAM lParam)
{
	if(wParam == VK_ESCAPE)
	{
		if(EDITING)
		{
			EDITING = FALSE;
			strcpy(editstring,"");
			HideCaret(hWnd);
			Refresh();
			NotifyEditEnd();
		}
		return OK;
	}
	
	if(wParam == VK_TAB)
	{
		SetFocus(GetParent(hWnd));
		return OK;
	}
	
	if(wParam == VK_NEXT)
	{
		handle_VK_NEXT();
		return OK;
	}
	if(wParam == VK_PRIOR)
	{
		handle_VK_PRIOR();
		return OK;
	}
	if(wParam == VK_DOWN)	
	{			
		handle_VK_DOWN();
		return OK;
	}
	if(wParam == VK_UP)
	{
		handle_VK_UP();
		return OK;
	}
	if(wParam == VK_LEFT)
	{
		handle_VK_LEFT();
		return OK;
	}
	if(wParam == VK_RIGHT)
	{
		handle_VK_RIGHT();
		return OK;
	}

	int code = 0;
	switch(wParam)
	{
	case VK_F1:
		code = BGN_F1;
		break;
	case VK_F2:
		code = BGN_F2;
		break;
	case VK_F3:
		code = BGN_F3;
		break;
	case VK_F4:
		code = BGN_F4;
		break;
	case VK_F5:
		code = BGN_F5;
		break;
	case VK_F6:		
		code = BGN_F6;
		break;
	case VK_F7:
		code = BGN_F7;
		break;
	case VK_F8:
		code = BGN_F8;
		break;
	case VK_F9:
		code = BGN_F9;
		break;
	case VK_F10:
		code = BGN_F10;
		break;
	case VK_F11:
		code = BGN_F11;
		break;
	case VK_F12:
		code = BGN_F12;
		break;
	case VK_DELETE:
		code = BGN_DELETECELL;
		break;
	default:
		SetCurrentCellStatus();
		handleGenericKeypress(wParam, lParam);
		return OK;
	}
	if (code)
		handleControlKey(code);
	return OK;
}

void BabyGrid::scaleCells()
{
	spreadColsToMatchX();
	spreadRowsToMatchY();
}

void BabyGrid::spreadColsToMatchX()
{	
	// Get the proportion of the new grid width vs. the total of
	//  all column widths.
	// Ignore the row headers.
	int widthSum = 0;
	for (int i = 1; i <= this->cols; i++)
		widthSum += this->columnwidths[i];
	float proportion = (float)(this->gridwidth - this->columnwidths[0]) / (float)widthSum;

	// Multiply each column's width by that proportion.
	widthSum = 0;
	for (int i = 1; i <= this->cols; i++)
	{
		this->columnwidths[i] *= proportion;
		widthSum += this->columnwidths[i];
	}

	// Get rid of any remaining discrepancy owing to integer rounding
	int diff;
	if (diff = (this->gridwidth - this->columnwidths[0] - widthSum))
	{
		// did we overshoot or undershoot?
		int diffsign = (diff > 0) ? 1 : -1;
		diff *= diffsign;
		for (int i = 0; i < diff; i++)
		{
			// Add or remove a pixel from each column in turn until the
			// discrepancy is gone.
			this->columnwidths[i % (1+this->cols)] += diffsign;
		}
	}
}

void BabyGrid::spreadRowsToMatchY()
{
	// Change the height of the rows, but don't change the heights of the
	//  title or the column headers.
	if (0 == this->rows)
		return;
	//int heightSum = (this->rowheight * this->rows);
	//int adjusted_gridheight = this->gridheight - this->titleheight - this->headerrowheight;
	//int change = (adjusted_gridheight - heightSum) / this->rows;
	//int remainder = (adjusted_gridheight - heightSum) % this->rows;
	//this->rowheight += change;
	//if (this->headerrowheight)
	//	this->headerrowheight += remainder;
	//else
	//	if (remainder < 0)
	//		this->rowheight--;
}
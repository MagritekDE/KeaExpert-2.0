#include "stdafx.h"
#include <math.h>
#include "drawing.h"
#include "defines.h"
#include "evaluate.h"
#include "font.h"
#include "globals.h"
#include "mymath.h"
#include "print.h"
#include "prospaResource.h"
#include "scanstrings.h"
#include "ShortRect.h"
#include "memoryLeak.h"

/*****************************************************************************
*  Draw a rectangle using the specified screen coordinates and device context*
*****************************************************************************/

void DrawRect(HDC hdc,short x1,short y1,short x2,short y2)
{
	MoveToEx(hdc,x1,y1,0);
	LineTo(hdc,x2,y1);
	LineTo(hdc,x2,y2);
	LineTo(hdc,x1,y2);
	LineTo(hdc,x1,y1);
}

void DrawCircle(HDC hdc,short x1,short y1,short x2,short y2)
{
	MoveToEx(hdc,(x1+x2)/2,y1,0);
	ArcTo(hdc,x1,y1,x2,y2,x1,(y1+y2)/2,x1,(y1+y2)/2);
}

// Check to see if a line intersects with the clipping region

bool LineVisible(long xL, long yT, long xR, long yB,
					  long xa, long ya, long xb, long yb, long lineWidth)
{
	if(xa < xL || xa > xR || xb < xL || xb > xR || ya > yB || ya < yT || yb > yB || yb < yT)
	{
		if((xa < xL && xb < xL && (xL - xb > lineWidth / 2)) || 
			(xa > xR && xb > xR && (xb - xR > lineWidth / 2)) || 
			(ya < yT && yb < yT && (yT - yb > lineWidth / 2)) || 
			(ya > yB && yb > yB && (yb - yB > lineWidth / 2)))
		{
			return(false);
		}
	}
	return(true);
}

 bool SegmentIntersectRectangle(double a_rectangleMinX,
                                 double a_rectangleMinY,
                                 double a_rectangleMaxX,
                                 double a_rectangleMaxY,
                                 double a_p1x,
                                 double a_p1y,
                                 double a_p2x,
                                 double a_p2y)
{
   // Find min and max X for the segment

   double minX = a_p1x;
   double maxX = a_p2x;

   if(a_p1x > a_p2x)
   {
      minX = a_p2x;
      maxX = a_p1x;
   }

   // Find the intersection of the segment's and rectangle's x-projections

   if(maxX > a_rectangleMaxX)
   {
      maxX = a_rectangleMaxX;
   }

   if(minX < a_rectangleMinX)
   {
   minX = a_rectangleMinX;
   }

   if(minX > maxX) // If their projections do not intersect return false
   {
      return false;
   }

   // Find corresponding min and max Y for min and max X we found before

      double minY = a_p1y;
      double maxY = a_p2y;

      double dx = a_p2x - a_p1x;

   if(fabs(dx) > 0.0000001)
   {
      double a = (a_p2y - a_p1y) / dx;
      double b = a_p1y - a * a_p1x;
      minY = a * minX + b;
      maxY = a * maxX + b;
   }

   if(minY > maxY)
   {
      double tmp = maxY;
      maxY = minY;
      minY = tmp;
   }

   // Find the intersection of the segment's and rectangle's y-projections

   if(maxY > a_rectangleMaxY)
   {
      maxY = a_rectangleMaxY;
   }

   if(minY < a_rectangleMinY)
   {
      minY = a_rectangleMinY;
   }

   if(minY > maxY) // If Y-projections do not intersect return false
   {
      return false;
   }

   return true;
}

bool PointInRect(RECT *r, short x, short y)
{
	if(x >= r->left && x <= r->right && y >= r->top && y <= r->bottom)
		return(true);
	else
		return(false);
}

bool PointVisible(long xL, long yT, long xR, long yB, long x, long y)
{
	if(x < xL || x > xR || y > yB || y < yT)
		return(false);
	else
		return(true);
}

bool SymbolVisible(long xL, long yT, long xR, long yB, long x, long y, long ss)
{
	if(x+ss < xL || x-ss > xR || y-ss > yB || y+ss < yT)
		return(false);
	else
		return(true);
}

/*************************************************************************
*    Convert a rectangle from client to screen coordinates for window win
*************************************************************************/

void ClientToScreen(HWND win, RECT *r)
{
	POINT p;

	p.x = r->left;
	p.y = r->top;
	ClientToScreen(win,&p);
	r->left = p.x;
	r->top = p.y;

	p.x = r->right;
	p.y = r->bottom;
	ClientToScreen(win,&p);
	r->right = p.x;
	r->bottom = p.y;
}



/*************************************************************************
*    Convert a rectangle from screen to client coordinates for window win
*************************************************************************/

void ScreenToClientRect(HWND win, RECT *r)
{
	POINT p;

	p.x = r->left;
	p.y = r->top;
	ScreenToClient(win,&p);
	r->left = p.x;
	r->top  = p.y;

	p.x = r->right;
	p.y = r->bottom;
	ScreenToClient(win,&p);
	r->right  = p.x;
	r->bottom = p.y;
}


void DrawSymbolOnDlg(LPDRAWITEMSTRUCT pdis)
{
	HPEN symbolPen,noSymbolPen,bkgPen;
	HBRUSH symbolBrush,bkgBrush;
	short sz,xp,yp;
	POINT p[7];

	HDC hdc = pdis->hDC;

	SaveDC(hdc);

	symbolPen   = CreatePen(PS_SOLID,0,RGB(0,0,0));
	symbolBrush = CreateSolidBrush(RGB(0,0,0));
	bkgBrush    = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	noSymbolPen = CreatePen(PS_SOLID,0,RGB(100,100,100));
	bkgPen      = CreatePen(PS_SOLID,0,GetSysColor(COLOR_BTNFACE));

	sz = 4;
	xp = (pdis->rcItem.left+pdis->rcItem.right)/2;
	yp = (pdis->rcItem.top+pdis->rcItem.bottom)/2; 

	SelectObject(hdc,bkgBrush);
	SelectObject(hdc,bkgPen);	      
	FillRect(hdc,&pdis->rcItem,bkgBrush);

	switch(pdis->CtlID)
	{
	case(TD_SQUARE):
		{
			SelectObject(hdc,symbolBrush);
			SelectObject(hdc,symbolPen);	      
			Rectangle(hdc,xp-sz,yp-sz,xp+sz+1,yp+sz+1);
			break;
		}
	case(TD_DIAMOND):
		{
			SelectObject(hdc,symbolBrush);
			SelectObject(hdc,symbolPen);
			p[0].x = xp; p[0].y = yp+sz;
			p[1].x = xp-sz; p[1].y = yp;
			p[2].x = xp; p[2].y = yp-sz;
			p[3].x = xp+sz; p[3].y = yp;
			Polygon(hdc,p,4);
			break;
		}
	case(TD_TRIANGLE):
		{
			SelectObject(hdc,symbolBrush);
			SelectObject(hdc,symbolPen);
			p[0].x = xp-sz; p[0].y = yp+sz;
			p[1].x = xp; p[1].y = yp-sz;
			p[2].x = xp+sz; p[2].y = yp+sz;
			Polygon(hdc,p,3);
			break;
		}
	case(TD_INV_TRIANGLE):
		{
			SelectObject(hdc,symbolBrush);
			SelectObject(hdc,symbolPen);
			p[0].x = xp-sz; p[0].y = yp-sz+1;
			p[1].x = xp+sz; p[1].y = yp-sz+1;
			p[2].x = xp; p[2].y = yp+sz+1;
			Polygon(hdc,p,3);
			break;
		}
	case(TD_CIRCLE):
		{
			SelectObject(hdc,symbolBrush);
			SelectObject(hdc,symbolPen);
			Ellipse(hdc,xp-sz,yp-sz,xp+sz+1,yp+sz+1);
			Arc(hdc,xp-sz,yp-sz,xp+sz+1,yp+sz+1,xp+1,yp+sz+1,xp+1,yp+sz+1);
			break;
		} 	      
	case(TD_OPEN_SQUARE):
		{
			SelectObject(hdc,bkgBrush);
			SelectObject(hdc,symbolPen);	      
			MoveToEx(hdc,xp-sz,yp-sz,0);
			Rectangle(hdc,xp-sz,yp-sz,xp+sz+1,yp+sz+1);
			break;
		}
	case(TD_OPEN_DIAMOND):
		{
			SelectObject(hdc,bkgBrush);
			SelectObject(hdc,symbolPen);
			p[0].x = xp; p[0].y = yp+sz;
			p[1].x = xp-sz; p[1].y = yp;
			p[2].x = xp; p[2].y = yp-sz;
			p[3].x = xp+sz; p[3].y = yp;
			Polygon(hdc,p,4);
			break;
		}	      	      
	case(TD_OPEN_TRIANGLE):
		{
			SelectObject(hdc,bkgBrush);
			SelectObject(hdc,symbolPen);
			p[0].x = xp-sz; p[0].y = yp+sz;
			p[1].x = xp+sz; p[1].y = yp+sz;
			p[2].x = xp; p[2].y = yp-sz;
			Polygon(hdc,p,3);
			break;
		}
	case(TD_OPEN_INV_TRIANGLE):
		{
			SelectObject(hdc,bkgBrush);
			SelectObject(hdc,symbolPen);	      
			p[0].x = xp-sz; p[0].y = yp-sz+1;
			p[1].x = xp+sz; p[1].y = yp-sz+1;
			p[2].x = xp; p[2].y = yp+sz+1;
			Polygon(hdc,p,3);
			break;
		}
	case(TD_CROSS):
		{
			SelectObject(hdc,symbolPen);	      
			MoveToEx(hdc,xp-sz,yp-sz,0);
			LineTo(hdc,xp+sz+1,yp+sz+1);
			MoveToEx(hdc,xp+sz,yp-sz,0);
			LineTo(hdc,xp-sz-1,yp+sz+1);
			break;
		} 
	case(TD_PLUS):
		{
			SelectObject(hdc,symbolPen);	      
			MoveToEx(hdc,xp-sz,yp,0);
			LineTo(hdc,xp+sz+1,yp);
			MoveToEx(hdc,xp,yp-sz,0);
			LineTo(hdc,xp,yp+sz+1);
			break;
		} 
	case(TD_NO_SYMBOL):
		{
			SelectObject(hdc,bkgBrush);
			SelectObject(hdc,noSymbolPen);	      
			MoveToEx(hdc,xp-sz,yp-sz,0);
			Rectangle(hdc,xp-sz,yp-sz,xp+sz+1,yp+sz+1);
			break;
		}
	case(TD_OPEN_CIRCLE):
		{
			SelectObject(hdc,bkgBrush);
			SelectObject(hdc,symbolPen);
			Ellipse(hdc,xp-sz,yp-sz,xp+sz+1,yp+sz+1);
			break;
		}  
	}
	RestoreDC(hdc,-1);
	DeleteObject(bkgBrush);
	DeleteObject(symbolBrush);
	DeleteObject(symbolPen);
	DeleteObject(noSymbolPen);
}




/******************************************************************************************************      
   Write 'text' at position specified by 'r'. If 'write' is false then just calculate extent of text
   User can include super and subscripts and anything in the symbol font
   
   To write subscripts         write _(text)
   To write superscripts       write ^(text)
   To write in the symbol font write \G(text)
*******************************************************************************************************/     

void WriteTextCore(HDC hdc, bool write, ShortRect *r, int direction, const char *txt, TEXTMETRIC *tm,
                   HFONT font, HFONT smallFont, HFONT symbolFont)
{
   short i,j;
   short start = 0;
   char out[MAX_STR];
   char text[MAX_STR];
   SIZE te; 
   short x,y;

   te.cx = 0;
   te.cy = 0;

// Check for null string
   if(txt[0] == '\0')
      return;

// Ensure we have enough space
   StrNCopy(text,(char*)txt,MAX_STR-1);

   x = r->getx0();
   y = r->gety0();

   short ascent = tm->tmAscent;
   short leading = tm->tmInternalLeading;
   
// Scan through text searching for 'TEX' delimiters
   if(direction == WindowLayout::HORIZONTAL)
   {
	   for(i = 0; i < strlen(text); i++)
	   {
	      if(text[i] == '^' && text[i+1] == '(') // Superscript
	      {
	         StrSubCopy(out,text,start,i-1);
            if(write)
	            TextOut(hdc,x,y,out,strlen(out));
	         GetTextExtentPoint32(hdc,out,strlen(out), &te);
	         x += te.cx;
	         
	         start = i+2;
	         for(j = i+2; j < strlen(text); j++)
	         {
	            if(text[j] == ')')
	            {
	               StrSubCopy(out,text,start,j-1);
	               SelectObject(hdc,smallFont);
                  GetTextMetrics(hdc,tm);
                  if(write)
	                  TextOut(hdc,x,y+(leading-tm->tmAscent/2),out,strlen(out));
	               GetTextExtentPoint32(hdc,out,strlen(out), &te);
	               SelectObject(hdc,font);
	               x += te.cx;
	               i = j;
	               start = j+1;
	               break;
	            }
	         }
	      }
	      if(text[i] == '_' && text[i+1] == '(') // Subscript
	      {
	         StrSubCopy(out,text,start,i-1);
            if(write)
	            TextOut(hdc,x,y,out,strlen(out));
	         GetTextExtentPoint32(hdc,out,strlen(out), &te);
	         x += te.cx;
	         
	         start = i+2;
	         for(j = i+2; j < strlen(text); j++)
	         {
	            if(text[j] == ')')
	            {
	               StrSubCopy(out,text,start,j-1);
	               SelectObject(hdc,smallFont);
                  GetTextMetrics(hdc,tm);
                  if(write)
	                  TextOut(hdc,x,y+ascent-tm->tmInternalLeading-tm->tmAscent/2,out,strlen(out));
	               GetTextExtentPoint32(hdc,out,strlen(out), &te);
	               SelectObject(hdc,font);
	               x += te.cx;
	               i = j;
	               start = j+1;
	               break;
	            }
	         }
	      } 
	      if(text[i] == '\\' && text[i+1] == 'N' && text[i+2] == '(') // numeric description of character
	      {
	         StrSubCopy(out,text,start,i-1);
            if(write)
	            TextOut(hdc,x,y,out,strlen(out));
	         GetTextExtentPoint32(hdc,out,strlen(out), &te);
	         x += te.cx;
	         
	         start = i+3;
	         for(j = i+3; j < strlen(text); j++)
	         {
	            if(text[j] == ')')
	            {
	               StrSubCopy(out,text,start,j-1);
                  float num = StringToFloat(out);
                  out[0] = (unsigned char)nsint(num);
                  out[1] = '\0';
	               SelectObject(hdc,font);
                  GetTextMetrics(hdc,tm);
                  if(write)
	                  TextOut(hdc,x,y,out,strlen(out));
	               GetTextExtentPoint32(hdc,out,strlen(out), &te);
	               SelectObject(hdc,font);
	               x += te.cx;
	               i = j;
	               start = j+1;
	               break;
	            }
	         }
	      } 
	      if(text[i] == '\\' && text[i+1] == 'G' && text[i+2] == '(') // Greek
	      {
	         StrSubCopy(out,text,start,i-1);
            if(write)
	            TextOut(hdc,x,y,out,strlen(out));
	         GetTextExtentPoint32(hdc,out,strlen(out), &te);
	         x += te.cx;
	         
	         start = i+3;
	         for(j = i+3; j < strlen(text); j++)
	         {
	            if(text[j] == ')')
	            {
	               StrSubCopy(out,text,start,j-1);
                  if(strlen(out) >= 2 && out[0] == '0' && out[1] == 'x')
                  {
                     float num = StringToFloat(out);
                     out[0] = (unsigned char)nsint(num);
                     out[1] = '\0';
                  }
	               SelectObject(hdc,symbolFont);
                  GetTextMetrics(hdc,tm);
                  if(write)
	                  TextOut(hdc,x,y-tm->tmAscent+ascent,out,strlen(out));
	               GetTextExtentPoint32(hdc,out,strlen(out), &te);
	               SelectObject(hdc,font);
	               x += te.cx;
	               i = j;
	               start = j+1;
	               break;
	            }
	         }
	      } 	        
	   } 
	   StrSubCopy(out,text,start,i-1);
	   if(write)
	      TextOut(hdc,x,y,out,strlen(out));
	   GetTextExtentPoint32(hdc,out,strlen(out), &te);
	   x += te.cx;	

      strcpy(out,"Test");
	   GetTextExtentPoint32(hdc,out,strlen(out), &te);
	   y += te.cy; 
		r->set(0,0,x,y);
   }
   else if(direction == WindowLayout::VERTICAL)
   {
	   for(i = 0; i < strlen(text); i++)
	   {
	      if(text[i] == '^' && text[i+1] == '(')
	      {
	         StrSubCopy(out,text,start,i-1);
            if(write)
	            TextOut(hdc,x,y,out,strlen(out));
	         GetTextExtentPoint32(hdc,out,strlen(out), &te);
	         y -= te.cx;
	         
	         start = i+2;
	         for(j = i+2; j < strlen(text); j++)
	         {
	            if(text[j] == ')')
	            {
	               StrSubCopy(out,text,start,j-1);
	               SelectObject(hdc,smallFont);
	               
                  GetTextMetrics(hdc,tm);
                  if(write)
	                   TextOut(hdc,x+(leading-tm->tmAscent/2),y,out,strlen(out));
	               GetTextExtentPoint32(hdc,out,strlen(out), &te);
	               SelectObject(hdc,font);
	               y -= te.cx;
	               i = j;
	               start = j+1;
	               break;
	            }
	         }
	      }
	      if(text[i] == '_' && text[i+1] == '(')
	      {
	         StrSubCopy(out,text,start,i-1);
            if(write)
	            TextOut(hdc,x,y,out,strlen(out));
	         GetTextExtentPoint32(hdc,out,strlen(out), &te);
	         y -= te.cx;
	         
	         start = i+2;
	         for(j = i+2; j < strlen(text); j++)
	         {
	            if(text[j] == ')')
	            {
	               StrSubCopy(out,text,start,j-1);
	               SelectObject(hdc,smallFont);
                  GetTextMetrics(hdc,tm);
                  if(write)
	                  TextOut(hdc,x+ascent-tm->tmInternalLeading-tm->tmAscent/2,y,out,strlen(out));
	               GetTextExtentPoint32(hdc,out,strlen(out), &te);
	               SelectObject(hdc,font);
	               y -= te.cx;
	               i = j;
	               start = j+1;
	               break;
	            }
	         }
	      }
	      if(text[i] == '\\' && text[i+1] == 'G' && text[i+2] == '(') // Greek
	      {
	         StrSubCopy(out,text,start,i-1);
            if(write)
	            TextOut(hdc,x,y,out,strlen(out));
	         GetTextExtentPoint32(hdc,out,strlen(out), &te);
	         y -= te.cx;
	         
	         start = i+3;
	         for(j = i+3; j < strlen(text); j++)
	         {
	            if(text[j] == ')')
	            {
	               StrSubCopy(out,text,start,j-1);
	               SelectObject(hdc,symbolFont);
                  GetTextMetrics(hdc,tm);
                  if(write)
	                  TextOut(hdc,x-tm->tmAscent+ascent,y,out,strlen(out));
	               GetTextExtentPoint32(hdc,out,strlen(out), &te);
	               SelectObject(hdc,font);
	               y -= te.cx;
	               i = j;
	               start = j+1;
	               break;
	            }
	         }
	      } 	  	      
	         
	   }
	   StrSubCopy(out,text,start,i-1);
	   if(write)
	      TextOut(hdc,x,y,out,strlen(out));
	   if(out[0] != '\0')
	   {
	      GetTextExtentPoint32(hdc,out,strlen(out), &te);
	      x += te.cy;  	   	      
	   }
	   else
	   {
			x += te.cy;  	   	      
      }
	   y -= te.cx;
		r->set(0,0,x,y);
   }
}


/******************************************************************************************************      
   Writes text to a window at the specified position in the specified font. A rectangle enclosing the
   text is returned to the calling program.
  
   (xoff,yoff) : coordinates at which text is to be written in current device context hdc
   (xAlign,yAlign) : modifiers to (xoff,yoff) depending on alignment requirements
   direction : horizontal or vertical
   fgColor,bgColor : Text and background colors
   text : text to display
   textRect : pointer to rectangle which will return text extend
   logFont : logical font to display
*****************************************************************************************************/      
  
void WriteText(HDC hdc, short xoff, short yoff, WindowLayout::Constraint xAlign, WindowLayout::Constraint yAlign, 
					WindowLayout::Orientation direction, LOGFONT logFont, 
               COLORREF fgColor, COLORREF bgColor, const char *text, ShortRect& textRect)
{	 
   HFONT font;
   HFONT lastFont;
   HFONT smallFont;
   HFONT symbolFont;
   short delSize = 4;
   TEXTMETRIC tm;
   SIZE te;
   short dx,dy;  // Size of text rectangle
   ShortRect r;
   
   if(direction == WindowLayout::HORIZONTAL)
   {
      font = GetFont(hdc,logFont,0,0); 
      symbolFont = GetNewFont(hdc, logFont,0,"Symbol");       
      smallFont = GetFont(hdc, logFont,-delSize,0); 
   }   
   else
   {
      font = GetFont(hdc,logFont,0,900); 
      symbolFont = GetNewFont(hdc, logFont,900,"Symbol");             
      smallFont = GetFont(hdc, logFont,-delSize,900); 
   }      
   
   lastFont = (HFONT)SelectObject(hdc,font);
   GetTextMetrics(hdc,&tm);
   
   if(gPlotMode == PRINT && gPrintMode == BLACK_AND_WHITE)
   {	
      SetTextColor(hdc,RGB(0,0,0));
      SetBkColor(hdc,RGB(255,255,255));
   }
   else
   {	
      SetTextColor(hdc,fgColor);
	   SetBkMode(hdc,TRANSPARENT);
      
//      SetBkColor(hdc,bgColor);
   }	   

// Pretend to write the text so we can figure out how big it is

   GetTextExtentPoint32(hdc, text, strlen(text), &te);

   r.zero();
   WriteTextCore(hdc,false,&r,direction,text,&tm,font,smallFont,symbolFont);

   dx = r.getx1();
   dy = r.gety1();
 
   if(direction == WindowLayout::HORIZONTAL)
   {
	   if(yAlign == WindowLayout::CENTRE_ALIGN)
	   {
	      yoff -= dy/2;
	   }   
	   else if(yAlign == WindowLayout::BASE_ALIGN)
	   {  
	      yoff -= dy;
	   }
	   if(xAlign == WindowLayout::CENTRE_ALIGN)
	   {
	      xoff -= dx/2;
	   }
	   else if(xAlign == WindowLayout::RIGHT_ALIGN)
	   {
	      xoff -= dx;
	   }
	   textRect.set(xoff,yoff,xoff+dx,yoff+dy);
   }
   else
   {
	   if(yAlign == WindowLayout::CENTRE_ALIGN)
	   {
	      yoff -= dy/2;
	   } 
	   if(xAlign == WindowLayout::CENTRE_ALIGN)
	   {
	      xoff -= dx/2;
	   }
	   else if(xAlign == WindowLayout::RIGHT_ALIGN)
	   {
	      xoff -= dx;
	   }
	   textRect.set(xoff,yoff+dy,xoff+dx,yoff);
   }


// Write the text this time now that we know where to put it.

   SelectObject(hdc,font);
   GetTextMetrics(hdc,&tm);

   r.setx0(xoff);
   r.sety0(yoff);   
   WriteTextCore(hdc,true,&r,direction,text,&tm,font,smallFont,symbolFont);
   
// Delete font resource after restoring old one
 
   SelectObject(hdc,lastFont);  
   DeleteObject(font);    
   DeleteObject(smallFont);    
   DeleteObject(symbolFont);    
}



int IsPointInBoundingBox(float x1, float y1, float x2, float y2, float px, float py)
{
    float left, top, right, bottom; // Bounding Box For Line Segment
    // For Bounding Box
    if(x1 < x2)
    {
        left = x1;
        right = x2;
    }
    else
    {
        left = x2;
        right = x1;
    }
    if(y1 < y2)
    {
        top = y1;
        bottom = y2;
    }
    else
    {
        top = y2;
        bottom = y1;
    }
 
    if( (px+0.01) >= left && (px-0.01) <= right && 
            (py+0.01) >= top && (py-0.01) <= bottom )
    {
        return 1;
    }
    else
        return 0;
}
 
int LineIntersection(float l1x1, float l1y1, float l1x2, float l1y2,
                            float l2x1, float l2y1, float l2x2, float l2y2,
                            float *m1, float *c1, float *m2, float *c2,
                            float* intersection_X, float* intersection_Y)
{
    float dx, dy;
 
    dx = l1x2 - l1x1;
    dy = l1y2 - l1y1;
 
    *m1 = dy / dx;
    // y = mx + c
    // intercept c = y - mx
    *c1 = l1y1 - *m1 * l1x1; // which is same as y2 - slope * x2
 
    dx = l2x2 - l2x1;
    dy = l2y2 - l2y1;
 
    *m2 = dy / dx;
    // y = mx + c
    // intercept c = y - mx
    *c2 = l2y1 - *m2 * l2x1; // which is same as y2 - slope * x2
 
    if( (*m1 - *m2) == 0)
        return 0;
    else
    {
        *intersection_X = (*c2 - *c1) / (*m1 - *m2);
        *intersection_Y = *m1 * *intersection_X + *c1;
    }
    return(1);
}
 
int LineSegmentIntersection(float l1x1, float l1y1, float l1x2, float l1y2,
                            float l2x1, float l2y1, float l2x2, float l2y2,
                            float *m1, float *c1, float *m2, float *c2,
                            float* intersection_X, float* intersection_Y)
{
    float dx, dy;
 
    dx = l1x2 - l1x1;
    dy = l1y2 - l1y1;
 
    *m1 = dy / dx;
    // y = mx + c
    // intercept c = y - mx
    *c1 = l1y1 - *m1 * l1x1; // which is same as y2 - slope * x2
 
    dx = l2x2 - l2x1;
    dy = l2y2 - l2y1;
 
    *m2 = dy / dx;
    // y = mx + c
    // intercept c = y - mx
    *c2 = l2y1 - *m2 * l2x1; // which is same as y2 - slope * x2
 
    if( (*m1 - *m2) == 0)
        return 0;
    else
    {
        *intersection_X = (*c2 - *c1) / (*m1 - *m2);
        *intersection_Y = *m1 * *intersection_X + *c1;
    }
    if(IsPointInBoundingBox(l1x1, l1y1, l1x2, l1y2, *intersection_X, *intersection_Y) == 1 &&
        IsPointInBoundingBox(l2x1, l2y1, l2x2, l2y2, *intersection_X, *intersection_Y) == 1)
        return 1;
    else
        return 0;
}
 
void testIntersection()
{
    float m1, c1, m2, c2;
    float l1x1, l1y1, l1x2, l1y2;
    float l2x1, l2y1, l2x2, l2y2;
    float intersection_X, intersection_Y;
    int nRet;
 
    printf("Program to find the intersection point of two line segments:\n");
 
    printf("Enter Line1 - X1: ");
    scanf("%f", &l1x1);
 
    printf("Enter Line1 - Y1: ");
    scanf("%f", &l1y1);
 
    printf("Enter Line1 - X2: ");
    scanf("%f", &l1x2);
 
    printf("Enter Line1 - Y2: ");
    scanf("%f", &l1y2);
 
    printf("Enter Line2 - X1: ");
    scanf("%f", &l2x1);
 
    printf("Enter Line2 - Y1: ");
    scanf("%f", &l2y1);
 
    printf("Enter Line2 - X2: ");
    scanf("%f", &l2x2);
 
    printf("Enter Line2 - Y2: ");
    scanf("%f", &l2y2);
 
 
    nRet = LineSegmentIntersection(l1x1, l1y1, l1x2, l1y2,
                            l2x1, l2y1, l2x2, l2y2,
                            &m1, &c1, &m2, &c2, &intersection_X, &intersection_Y);
 
    printf("Equation of line1: Y = %.2fX %c %.2f\n", m1, (c1 < 0) ? ' ' : '+',  c1);
    printf("Equation of line2: Y = %.2fX %c %.2f\n", m2, (c2 < 0) ? ' ' : '+',  c2);
 
    if(nRet == 0)
        printf("The two line segments do not intersect each other");
    else
        printf("The two line segments intersect each other at %.2f, %.2f", intersection_X, intersection_Y);
 
}
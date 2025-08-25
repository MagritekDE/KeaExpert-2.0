// User draw button functions -only active when global flag 'win7Mode' is set.

#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)
#include <gdiplus.h> 
#include "stdafx.h"
#include "defines.h"
#include "mymath.h"
#include "guiObjectClass.h"
#include "button.h"
#include "mytime.h"
#include <vector>
#include <string>
#include "globals.h"

using namespace std;

using namespace Gdiplus;

typedef enum {NS, EW} GradFillMode;

void GradientRectangleFill(HDC hdc, RECT *r, COLORREF startCol, COLORREF endCol, GradFillMode mode);

int DrawButton(ObjectData *obj, LPDRAWITEMSTRUCT pdis)
{
   extern HFONT controlFont;
	SIZE te; 
	RECT r = pdis->rcItem;
	r.right -= 2;
	r.bottom -= 2;

   double startTime = GetMsTime();

	HDC hdc = pdis->hDC;
	COLORREF bkgColor = gMainWindowColor;

	SetBkColor(hdc, bkgColor);

   PushButtonInfo* info = (PushButtonInfo*)obj->data;
   assert(info);

   COLORREF col1,col2,col3,col4,col5,col6,col7;

   col7 = obj->fgColor;

   if(IsWindowEnabled(obj->hWnd))
   {
      if(info->hovering)
      {
         if(info->selected)
         {
            col1 = RGB(229,244,252); // Top gradient start
            col2 = RGB(196,229,246); // Top gradient end
            col3 = RGB(152,209,239); // Bottom gradient start
            col4 = RGB(104,139,219); // Bottom gradient end
            col5 = RGB(158,136,186); // Inner border
            col6 = RGB(44,98,139);   // Outer border
         }
         else
         {
            col1 = RGB(234,246,253);
            col2 = RGB(217,240,252);
            col3 = RGB(188,229,252);
            col4 = RGB(167,217,245);
            col5 = RGB(252,252,252);
            col6 = RGB(60,127,177);
         }
      }
      else
      {
         if(info->defaultButton)
         {
            col1 = RGB(242,242,242);
            col2 = RGB(235,235,235);
            col3 = RGB(221,221,221);
            col4 = RGB(207,207,207);
            col5 = RGB(72,216,251);
            col6 = RGB(60,127,77);
         }
         else
         {
            col1 = RGB(242,242,242);
            col2 = RGB(235,235,235);
            col3 = RGB(221,221,221);
            col4 = RGB(207,207,207);
            col5 = RGB(252,252,252);
            col6 = RGB(112,112,112);
         }
      }
   }
   else
   {
      col1 = RGB(244,244,244);
      col2 = RGB(244,244,244);
      col3 = RGB(244,244,244);
      col4 = RGB(244,244,244);
      col5 = RGB(252,252,252);
      col6 = RGB(173,178,181);
      col7 = RGB(131,131,131);
   }

   int x1 = r.left;
   int y1 = r.top;
   int x2 = r.right;
   int y2 = r.bottom;
   int wBut = x2-x1;
   int hBut = y2-y1;

	HRGN hRgn = CreateRectRgn(r.left, r.top, r.right, r.bottom); 
	SelectClipRgn(hdc, hRgn); 

   // Draw the shading
   RECT r2 = pdis->rcItem;
	r2.bottom = r2.bottom/2;
	int base = r2.bottom;
   GradientRectangleFill(pdis->hDC,&r2,col1,col2,GradFillMode::NS);

	r2 = pdis->rcItem;
	r2.top = base;
   GradientRectangleFill(pdis->hDC,&r2,col3,col4,GradFillMode::NS);

   // Draw the icon (if it exists)
   if(info->hImage)
   {
      Graphics graph(hdc);
      Bitmap *bitmap = (Bitmap*)info->hImage;
      int w = bitmap->GetWidth();
      int h = bitmap->GetHeight();

		ImageAttributes  imageAttributes;

		if(IsWindowEnabled(obj->hWnd))
		{
			ColorMatrix colorMatrix = {
				1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
   
			imageAttributes.SetColorMatrix(
				&colorMatrix, 
				ColorMatrixFlagsDefault,
				ColorAdjustTypeBitmap);
		}
		else
		{
			ColorMatrix colorMatrix = {
				1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
				0.5f, 0.5f, 0.5f, 0.0f, 1.0f};
   
			imageAttributes.SetColorMatrix(
				&colorMatrix, 
				ColorMatrixFlagsDefault,
				ColorAdjustTypeBitmap);
		}

		graph.DrawImage(
			bitmap, 
			Rect((wBut-w)/2,(hBut-h)/2,w,h),  // destination rectangle 
			0, 0,        // upper-left corner of source rectangle 
			w,       // width of source rectangle
			h,      // height of source rectangle
			UnitPixel,
			&imageAttributes);

// graph.DrawImage(bitmap,(wBut-w)/2,(hBut-h)/2);
   }
   else
   {
      DisplayButtonText(hdc, obj, pdis->rcItem, col7);
   }

   COLORREF colInner = col5;
	HPEN pen1 = CreatePen(PS_SOLID,0,colInner);  
	SelectObject(hdc, pen1); 

  // Draw the inner button rectangle
	MoveToEx(hdc,x1+2,y1+1,NULL);
	LineTo(hdc,x2-3,y1+1);
	LineTo(hdc,x2-3,y1+2);
	LineTo(hdc,x2-2,y1+2);
	LineTo(hdc,x2-2,y2-3);
	LineTo(hdc,x2-3,y2-3);
	LineTo(hdc,x2-3,y2-2);
	LineTo(hdc,x1+2,y2-2);
	LineTo(hdc,x1+2,y2-3);
	LineTo(hdc,x1+1,y2-3);
	LineTo(hdc,x1+1,y1+2);
	LineTo(hdc,x1+2,y1+2);
	LineTo(hdc,x1+1,y1+2);

   COLORREF colOutter = col6;
	HPEN pen2 = CreatePen(PS_SOLID,0,colOutter);  
	SelectObject(hdc, pen2); 

   // Draw the outer button rectangle
	MoveToEx(hdc,x1+1,y1,NULL);
	LineTo(hdc,x2-2,y1);
	LineTo(hdc,x2-2,y1+1);
	LineTo(hdc,x2-1,y1+1);
	LineTo(hdc,x2-1,y2-2);
	LineTo(hdc,x2-2,y2-2);
	LineTo(hdc,x2-2,y2-1);
	LineTo(hdc,x1+1,y2-1);
	LineTo(hdc,x1+1,y2-2);
	LineTo(hdc,x1+0,y2-2);
	LineTo(hdc,x1+0,y1+1);
	LineTo(hdc,x1+1,y1+1);
	LineTo(hdc,x1+0,y1+1);


   COLORREF bakcol = GetSysColor(COLOR_BTNFACE);
   SetPixel(hdc,x1,y2-1,bakcol);
   SetPixel(hdc,x2-1,y2-1,bakcol);

   if(GetFocus() == obj->hWnd)
   {
      RECT r3 = pdis->rcItem;
		InflateRect(&r3,-2,-2);
      r3.right-=2;
      r3.bottom-=2;
	   DrawFocusRect(hdc, &r3);
   }

	SelectClipRgn(hdc, NULL); 
	DeleteObject(hRgn);

	DeleteObject(pen1);
	DeleteObject(pen2);

   return(1);
}

void GradientRectangleFill(HDC hdc, RECT *r, COLORREF startCol, COLORREF endCol, GradFillMode mode)
{
// Create an array of TRIVERTEX structures that describe 
// positional and color values for each vertex. For a rectangle, 
// only two vertices need to be defined: upper-left and lower-right. 
   TRIVERTEX vertex[2] ;
   vertex[0].x     = r->left;
   vertex[0].y     = r->top;
   vertex[0].Red   = GetRValue(startCol)<<8;
   vertex[0].Green = GetGValue(startCol)<<8;
   vertex[0].Blue  = GetBValue(startCol)<<8;
   vertex[0].Alpha = 0x0000;

   vertex[1].x     = r->right;
   vertex[1].y     = r->bottom; 
   vertex[1].Red   = GetRValue(endCol)<<8;
   vertex[1].Green = GetGValue(endCol)<<8;
   vertex[1].Blue  = GetBValue(endCol)<<8;
   vertex[1].Alpha = 0x0000;

   // Create a GRADIENT_RECT structure that 
   // references the TRIVERTEX vertices. 
   GRADIENT_RECT gRect;
   gRect.UpperLeft  = 0;
   gRect.LowerRight = 1;

   // Draw a shaded rectangle. 
   if(mode == GradFillMode::NS)
      GradientFill(hdc, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
   else
      GradientFill(hdc, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_H);

}


void DisplayButtonText(HDC hdc,  ObjectData *obj, RECT r, COLORREF col)
{
   extern HFONT controlFont;
	SIZE te; 
   HFONT newFont = NULL;
   HFONT oldFont = NULL;

	r.right -= 2;
	r.bottom -= 2;

   std::vector<string> words; 
   std::vector<string> lines; 
   string temp;

   int x1 = r.left;
   int y1 = r.top;
   int x2 = r.right;
   int y2 = r.bottom;
   int wBut = x2-x1;
   int hBut = y2-y1;

   PushButtonInfo* info = (PushButtonInfo*)obj->data;

   if(info->fontHeight > 0 || info->italicText || info->boldText || info->fontName.Size() > 0)
   {
      LOGFONT lf = {0};

      const HFONT currentFont = (HFONT)controlFont;
      int res = GetObject(currentFont,sizeof(LOGFONT),&lf); 

      if(info->italicText)
         lf.lfItalic = TRUE;

      if(info->boldText)
         lf.lfWeight = FW_BOLD;

      if(info->fontHeight > 0)
         lf.lfHeight = info->fontHeight;

      if(info->fontName.Size() > 0)
         strcpy(lf.lfFaceName,info->fontName.Str());

      newFont = CreateFontIndirect(&lf);

      oldFont = (HFONT)SelectObject(hdc,newFont);
      SelectObject(hdc,newFont);

   }
   else
      SelectObject(hdc,controlFont);

   CText label;
	obj->GetWindowText(label);

// Parse the text giving words
   int sz = label.Size();
   for(int i = 0; i < sz; i++)
   {
      if(label[i] == ' ')
      {
         words.push_back(temp);
         temp = "";
      }

      if(label[i] == '\n' || label[i] == '\r')
      {
         temp.push_back('\n');
         words.push_back(temp);
         temp = "";
      }

      else
      {
         temp.push_back(label[i]);
      }
   }

   if(label[sz-1] != ' ' && label[sz-1] != '\n')
   {
      temp.push_back('\0');
      words.push_back(temp);
   }

   string curLine = "";
   string newLine = "";
   for(int i = 0; i < words.size(); i++)
   {
       string w = (words[i]).c_str();
       int sz = w.size();

       if(curLine.size() > 0)
      //    newLine = curLine + " " + w;
          newLine = curLine + w;
       else
          newLine = w;

       GetTextExtentPoint32(hdc, newLine.c_str(), newLine.size(), &te);

       if(te.cx > wBut)
       {
          lines.push_back(curLine);
          curLine = w;
       }
       else
       {
          sz = newLine.size();
          if(sz > 0 && newLine[sz-1] == '\n')
          {
             newLine = newLine.substr(0,sz-1);
             lines.push_back(newLine);
				 curLine = "";
          }
          else
             curLine = newLine;
       }

   }
   lines.push_back(curLine);

   SetTextColor(hdc,col);
   SetBkMode(hdc, TRANSPARENT);
   string w = "AAyy";
   GetTextExtentPoint32(hdc, w.c_str(), w.size(), &te);
   int lineHeight = te.cy;

   float yStart = (y1+y2)/2.0 - lines.size()*lineHeight/2.0;

   for(int i = 0; i < lines.size(); i++)
   {
      string w = lines[i]; 
      int sz = w.size();
      if(sz > 0 &&  w[0] == ' ')
         w = w.substr(1,sz-1);
      GetTextExtentPoint32(hdc, w.c_str(), w.size(), &te);
    //  printf("'%s'\n",w.c_str());

	   TextOut(hdc,(int)((x1+x2)/2.0-te.cx/2.0),(int)(yStart+i*lineHeight), w.c_str(), w.size());
   }

   if(newFont)
   {
      SelectObject(hdc,oldFont);
      DeleteObject(newFont);
   }
}



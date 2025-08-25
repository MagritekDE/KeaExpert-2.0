#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)
#include "plotText.h"
#include "allocate.h"
#include "Translator.h"
#include <gdiplus.h> 
//#include "memoryLeak.h"

using namespace Gdiplus;

PlotText::PlotText()
{
   x = 0;
   y = 0;
   color = RGB(0,0,0);
   txt = "";
   font = "Arial";
   size = 10;
   angle = 0;
   style = FontStyle::FontStyleRegular;
   units = 0;
   xw = 0;
   xh = 0;
   yw = 0;
   yh = 0;
}


PlotText::PlotText(float x0, float y0, float** txtOffset, char* str, CText fontName, int fontSize, float fontAngle, CText fontStyle, COLORREF col, CText positionUnits)
{
   x = x0;
   y = y0;
   color = col;
   txt = str;
   font = fontName;
   size = fontSize;
   angle = fontAngle;

  if(fontStyle == "bold")
      style = FontStyle::FontStyleBold;
   else if(fontStyle == "bolditalic")
      style = FontStyle::FontStyleBoldItalic;
   else if(fontStyle == "italic")
      style = FontStyle::FontStyleItalic;
   else if(fontStyle == "underline")
      style = FontStyle::FontStyleUnderline;
   else
      style = FontStyle::FontStyleRegular;

	if(positionUnits.Size() == 2)
	{
		int digit1,digit2;
		char un1 = positionUnits[0];
		char un2 = positionUnits[1];

		if(un1 == 'd') digit1 = 0x00;
		else if(un1 == 'p') digit1 = 0x10;
		else if(un1 == 'w') digit1 = 0x20;
		else if(un1 == '0') digit1 = 0x00;
		else if(un1 == '1') digit1 = 0x10;
		else if(un1 == '2') digit1 = 0x20;
		else digit1 = 0x20;

		if(un2 == 'd') digit2 = 0x00;
		else if(un2 == 'p') digit2 = 0x01;
		else if(un2 == 'w') digit2 = 0x02;
		else if(un2 == '0') digit2 = 0x00;
		else if(un2 == '1') digit2 = 0x01;
		else if(un2 == '2') digit2 = 0x02;
		else digit2 = 0x20;

	   units = digit1 + digit2;
	}
	else
	{
		if(positionUnits == "user" || positionUnits == "data")
			units = 0x00;
		else if(positionUnits == "pixels")
			units = 0x11;
		else if(positionUnits == "window")
			units = 0x22;
		else
			units = 0x22;
	}

   xw = txtOffset[0][0];
   xh = txtOffset[0][1];
   yw = txtOffset[1][0];
   yh = txtOffset[1][1];
}

PlotText::~PlotText(void)
{
 //  TextMessage("Deleting plot info\n");
//   FreeMatrix2D(txtShift);
}

// Draw text
void PlotText::Draw(Plot *plt, HDC hdc, short dim)
{
   SaveDC(hdc);
   PointF pt;
   Graphics gfx(hdc); 
   extern WCHAR* CopyStrNarrowToWide(char* narrowStr);

   WCHAR *fontName = CopyStrNarrowToWide(font.Str());
   Gdiplus::Font font(fontName,size,style);
   delete [] fontName;

   Translator *xtrans = plt->curXAxis()->translator();
   Translator *ytrans = plt->curYAxis()->translator();
   int digit1 = units & 0xF0;
   int digit2 = units & 0x0F;

	switch(digit1)
   {
		case(0): // Data relative
		{
			if(dim == 2) // 2D
				pt.X = xtrans->userToScrn(x); 
			else // 1D
				pt.X = xtrans->dataToScrn(x); 
			break;
		}
		case(0x10):// Pixel absolute
		{
			if(x < 0)
				pt.X = plt->GetWidth() + x + plt->GetLeft();
			else
				pt.X = x + plt->GetLeft();
			break;
		}
		default: // Window relative
		{
         pt.X = plt->GetLeft() + plt->GetWidth()*x; 
			break;
		}
   }

	switch(digit2)
   {
		case(0): // Data relative
		{
			if(dim == 2) // 2D
				pt.Y = ytrans->userToScrn(y);
			else // 1D
				pt.Y = ytrans->dataToScrn(y);
			break;
		}
		case(1): // Pixel absolute
		{
			if(y < 0)
				pt.Y = plt->GetHeight() + y + plt->GetTop();
			else
				pt.Y = y + plt->GetTop();
			break;
		}
		default: // Window relative
		{
         pt.Y = plt->GetBottom() - plt->GetHeight()*(1-y);
			break;
		}
   }


   Rect clipRect(plt->GetLeft(), plt->GetTop(), plt->GetWidth(), plt->GetHeight());
   gfx.SetClip(clipRect, Gdiplus::CombineModeReplace);
   int strSz = strlen(txt.Str());
   WCHAR *wideStr = CopyStrNarrowToWide(txt.Str());
   Color gdiCol = Color::Red;
   gdiCol.SetFromCOLORREF(color);
 //  SolidBrush *mySolidBrush = ::new SolidBrush(gdiCol);
   SolidBrush *mySolidBrush = new SolidBrush(gdiCol);
   RectF layoutRect(0,0,10000,10000);
   RectF boundRect;
   gfx.MeasureString(wideStr, strSz, &font,layoutRect,&boundRect);
   float s11 = xw*boundRect.Width;
   float s21 = xh*boundRect.Height;
   float s12 = yw*boundRect.Width;
   float s22 = yh*boundRect.Height;
   gfx.TranslateTransform(pt.X+s11+s21,pt.Y+s12+s22);
   gfx.RotateTransform(angle);
   PointF pnt2(0,0);

   gfx.DrawString(wideStr, strSz, &font,pnt2,mySolidBrush);

   delete [] wideStr;
  // ::delete mySolidBrush;
   delete mySolidBrush;
	RestoreDC(hdc, -1);
}

WCHAR* CopyStrNarrowToWide(char* narrowStr)
{
    const size_t sz = strlen(narrowStr)+1;
    WCHAR* wideStr = new WCHAR[sz];
    mbstowcs(wideStr, narrowStr, sz);
    return  wideStr;
}


// Save a plot text 
void PlotText::Save(Plot *plt, FILE *fp)
{
   fwrite(&x,sizeof(float),1,fp);
   fwrite(&y,sizeof(float),1,fp);
   fwrite(&size,sizeof(int),1,fp);
   fwrite(&angle,sizeof(float),1,fp);
   fwrite(&style,sizeof(int),1,fp);
   fwrite(&units,sizeof(int),1,fp);
   fwrite(&color,sizeof(COLORREF),1,fp);
   fwrite(&xw,sizeof(float),1,fp);
   fwrite(&xh,sizeof(float),1,fp);
   fwrite(&yw,sizeof(float),1,fp);
   fwrite(&yh,sizeof(float),1,fp);
   char *c_txt = txt.Str();
   int szTxt = txt.Size()+1;
   fwrite(&szTxt,sizeof(int),1,fp);
   fwrite(c_txt,szTxt,1,fp);

   c_txt = font.Str();
   szTxt = font.Size()+1;
   fwrite(&szTxt,sizeof(int),1,fp);
   fwrite(c_txt,szTxt,1,fp);
}

// Load a plot text
void PlotText::Load(Plot *plt, FILE *fp)
{
   fread(&x,sizeof(float),1,fp);
   fread(&y,sizeof(float),1,fp);
   fread(&size,sizeof(int),1,fp);
   fread(&angle,sizeof(float),1,fp);
   fread(&style,sizeof(int),1,fp);
   fread(&units,sizeof(int),1,fp);
   fread(&color,sizeof(COLORREF),1,fp);
   fread(&xw,sizeof(float),1,fp);
   fread(&xh,sizeof(float),1,fp);
   fread(&yw,sizeof(float),1,fp);
   fread(&yh,sizeof(float),1,fp);
   int szTxt;
   fread(&szTxt,sizeof(int),1,fp);
   char *c_txt = new char[szTxt];
   fread(c_txt,szTxt,1,fp);
   txt = c_txt;
   delete [] c_txt;
   fread(&szTxt,sizeof(int),1,fp);
   c_txt = new char[szTxt];
   fread(c_txt,szTxt,1,fp);
   font = c_txt;
   delete [] c_txt;
}
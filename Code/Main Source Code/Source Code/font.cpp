#include "stdafx.h"
#include "defineWindows.h"
#include "font.h"
#include "globals.h"
#include "mymath.h"
#include "Scalable.h"
#include "memoryLeak.h"

short ProspaFont::lfHeight()
{
	HDC hDC = GetDC(prospaWin); 
	short height = nsint(-size_ * pixelsperinch(hDC) / 72.0); 
	ReleaseDC(prospaWin, hDC);
	return height;
}

short ProspaFont::lfHeightInverse()
{ 
	HDC hDC = GetDC(prospaWin); 
	short size = nsint(-(font_.lfHeight * 72.0 / pixelsperinch(hDC)));
	ReleaseDC(prospaWin, hDC);
	return size;
}

void ProspaFont::update()
{
	short weight = ((style_ & FONT_BOLD)>0)*FW_BOLD + ((style_ & FONT_BOLD)>=0)*FW_NORMAL;
   
	strncpy(font_.lfFaceName, name_, LF_FACESIZE);
	font_.lfHeight = lfHeight();
	font_.lfWidth = 0;
	font_.lfEscapement = 0;
	font_.lfOrientation = 0;
	font_.lfWeight = weight;
	font_.lfItalic = (style_ & FONT_ITALIC) > 0;
	font_.lfUnderline = (style_ & FONT_UNDERLINE) > 0;
	font_.lfStrikeOut = false;
	font_.lfCharSet = 0;
	font_.lfOutPrecision = 0;
	font_.lfClipPrecision = 0;
	font_.lfQuality = 0;
	font_.lfPitchAndFamily = 0;
}

ProspaFont::ProspaFont(const char* const name, COLORREF color, short size, short style)
{
	strncpy(name_, name, PROSPA_FONT_NAME_LEN);
	color_ = color;
	size_ = size;
	style_ = style;
	update();
}

ProspaFont::ProspaFont(const ProspaFont& copyMe)
{
	strncpy(name_, copyMe.name_, PROSPA_FONT_NAME_LEN);
	color_ = copyMe.color_;
	size_ = copyMe.size_;
	style_ = copyMe.style_;
	update();
}

ProspaFont::ProspaFont(LOGFONT font)
{
	font_ = font;
	strncpy(name_, font_.lfFaceName, LF_FACESIZE);
	color_ = 0x000000;
	size_ = lfHeightInverse();
	style_ = 0;
	if (font_.lfUnderline)
		style_ |= FONT_UNDERLINE;
	if (font_.lfItalic)
		style_ |= FONT_ITALIC;
}

ProspaFont::~ProspaFont()
{
}

void ProspaFont::scale_(double x, double y)
{
	font().lfHeight = font().lfHeight * x - 0.5;  // Rounding uses -0.5, because lfHeight is negative.
}
void ProspaFont::unscale_() 
{
	font().lfHeight = font().lfHeight * x_inverse_scaling_factor() - 0.5; // Rounding uses -0.5, because lfHeight is negative.
}


char *GetFontStyleStr(short style)
{
	static char styleStr[50];
	bool started = false;

	strcpy(styleStr,"");

	if((style & FONT_ITALIC))
	{
		strcat(styleStr,"italic");
		started = true;
	}
	if(style & FONT_BOLD)
	{
		if(started)
			strcat(styleStr,"+bold");
		else
		{
			strcat(styleStr,"bold");
			started = true;
		}
	}
	if(style & FONT_UNDERLINE)
	{
		if(started)
			strcat(styleStr,"+underline");
		else
		{
			strcat(styleStr,"underline");
			started = true;
		}
	}

	if(!started)
		return("regular");

	return(styleStr);
}

int ParseFontStyleStr(char* styleStr)
{
	short result = FONT_NORMAL;
	char word[50];
	short len = strlen(styleStr);
	int j = 0;

	for(int i = 0; i <= len; i++)
	{
		if(i < len)
		{
			if(styleStr[i] == ' ') // No spaces allowed
			{
				ErrorMessage("shouldn't be spaces in style string");
				return(ERR);
			}

			if(styleStr[i] != '+') // Normal text
			{
				word[j++] = styleStr[i];
			}
		}

		if(i == len || styleStr[i] == '+') // End of word
		{        
			word[j] = '\0';
			if(!strcmp(word,"italic"))
				result += FONT_ITALIC;
			else if(!strcmp(word,"bold"))
				result += FONT_BOLD;
			else if(!strcmp(word,"underline"))
				result += FONT_UNDERLINE;
			j = 0;
		}
	}

	return(result);
}


HFONT MakeFont(HDC hdc, char *name, short size, short weight, bool italic, short orientation)
{
	LOGFONT lf;
	float cyDpi;
	long height;

	cyDpi = (float)GetDeviceCaps(hdc,LOGPIXELSY);

	height = nint(size*cyDpi/72.0) ;

	lf.lfHeight         = height;
	lf.lfWidth          = 0 ;
	lf.lfEscapement     = orientation;
	lf.lfOrientation    = orientation;
	lf.lfWeight         = weight;
	lf.lfItalic         = italic;
	lf.lfUnderline      = 0 ;
	lf.lfStrikeOut      = 0;
	lf.lfCharSet        = ANSI_CHARSET ;
	lf.lfOutPrecision   = 0 ;
	lf.lfClipPrecision  = 0 ;
	lf.lfQuality        = 0 ;
	lf.lfPitchAndFamily = 0 ;

	strcpy(lf.lfFaceName, name);

	return(GetFont(hdc, lf, 0, orientation));
}

// Given a font description create the font and return to caller

HFONT GetFont(HDC hdc, LOGFONT lf, short delSize, short orientation)
{
	HFONT hFont ;

	//cxDpi = (float)GetDeviceCaps(hdc,LOGPIXELSX);
	//cyDpi = (float)GetDeviceCaps(hdc,LOGPIXELSY);
	//
	//height = (abs(lf.lfHeight) + delSize)*10;
	//pt.x = (int)(height*cxDpi/72) ;
	//pt.y = (int)(height*cyDpi/72) ;

	//DPtoLP(hdc,&pt,1) ;

	// lf.lfHeight        = -(int)(fabs((float)pt.y)/10.0 + 0.5);
	lf.lfHeight        = -(abs(lf.lfHeight) + delSize);
	lf.lfWidth        = 0;
	lf.lfEscapement    = orientation;
	lf.lfOrientation   = orientation;
	hFont = CreateFontIndirect(&lf);
	return(hFont);
}


HFONT GetNewFont(HDC hdc, LOGFONT lf,short orientation, char newFontName[])
{
	HFONT hFont ;

	//cxDpi = (float)GetDeviceCaps(hdc,LOGPIXELSX);
	//cyDpi = (float)GetDeviceCaps(hdc,LOGPIXELSY);
	//
	//height = (abs(lf.lfHeight))*10;
	//pt.x = (int)(height*cxDpi/72) ;
	//pt.y = (int)(height*cyDpi/72) ;

	//DPtoLP(hdc,&pt,1) ;

	//lf.lfHeight        = -(int)(fabs((float)pt.y)/10.0 + 0.5);
	lf.lfHeight        = -abs(lf.lfHeight);
	lf.lfWidth        = 0;
	lf.lfEscapement    = orientation;
	lf.lfOrientation   = orientation;
	strcpy(lf.lfFaceName,newFontName);
	hFont = CreateFontIndirect(&lf);
	return(hFont);
}

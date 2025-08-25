#include "stdafx.h"
#include "bitmap.h"
#include "memoryLeak.h"

/********************************************************************************
    Generate a bitmap of the specified size and return handle to it
********************************************************************************/
 
bool GenerateBitMap(long width, long height, HBITMAP *bitMap, HDC hdc, long &newWidth, bool widthMultipleOf4)
{
   long d4width;           // Width divisible by 4
   BITMAPINFOHEADER  bmih; // Header info for bitmap
   BYTE *pbm;              // Pointer to color values in bitmap

// Bit map width must be divisible by 4 and be >= width **********   
   if(widthMultipleOf4 && (width/4.0 - (int)(width/4.0) > 0))
   {
      d4width = ((int)(width/4.0) + 1)*4;
   }
   else
      d4width = (int)(width/4.0)*4;   

// Initialize bitmap header **************************************  
   bmih.biSize          = sizeof(BITMAPINFOHEADER);
   bmih.biWidth         = d4width;
   bmih.biHeight        = height;
   bmih.biPlanes        = 1;
   bmih.biBitCount      = 24;
   bmih.biCompression   = BI_RGB;
   bmih.biSizeImage     = 0;
   bmih.biXPelsPerMeter = 0;
   bmih.biYPelsPerMeter = 0;
   bmih.biClrUsed       = 0;
   bmih.biClrImportant  = 0;

   newWidth = d4width;
                
// Create bit map  ***********************************************
   *bitMap = CreateDIBSection(hdc, (BITMAPINFO*)&bmih, DIB_RGB_COLORS, (void**)&pbm, NULL, 0);
   if(!(*bitMap))
      return(false);
   
   return(true);
}


// Given an bitmap replace all blue regions with the background colour

void ReplaceBitMapBackGround(HBITMAP hbitmap)
{
	BITMAP bitmap;
	COLORREF bakcol = GetSysColor(COLOR_BTNFACE);
	bakcol = ((bakcol&0x00FF0000)>>16)  + ((bakcol&0x0000FF00)) + ((bakcol&0x000000FF)<<16);

	GetObject(hbitmap,sizeof(BITMAP),&bitmap);
	long w = bitmap.bmWidth;
	long h = bitmap.bmHeight;
	COLORREF *rgb = new COLORREF[w*h];
	GetBitmapBits(hbitmap,w*h*sizeof(COLORREF),(LPVOID)rgb);
	for(long i = 0; i < w*h; i++)
		if(rgb[i] == 255) rgb[i] = bakcol;
	SetBitmapBits(hbitmap,w*h*sizeof(COLORREF),(LPVOID)rgb);
	delete [] rgb;
}


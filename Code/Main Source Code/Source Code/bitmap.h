#ifndef BITMAP_H
#define BITMAP_H

bool GenerateBitMap(long width, long height, HBITMAP *bitMap, HDC hdc, long &newWidth, bool widthMultipleOf4 = true);
short FillBitMapWithColor(HBITMAP,COLORREF);

#endif BITMAP_H

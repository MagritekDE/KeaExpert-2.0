#ifndef DRAWING_H
#define DRAWING_H

#include "defines.h"

class ShortRect;

// Should contain all functions that relate to actually drawing to pixels on the screen.

// Drawing
void DrawCircle(HDC hdc, short x1, short y1, short x2, short y2);
bool LineVisible(long,long,long,long,long,long,long,long,long);
bool PointVisible(long xL, long yB, long xR, long yT, long x, long y);
bool SymbolVisible(long xL, long yT, long xR, long yB, long x, long y, long hs);
void DrawRect(HDC hdc,short x1,short y1,short x2,short y2);
bool PointInRect(RECT*,short,short);
void ScreenToClientRect(HWND win, RECT *r);
void WriteTextCore(HDC hdc, bool write, ShortRect *r, int direction, const char *txt, TEXTMETRIC *tm,
                   HFONT font, HFONT smallFont, HFONT symbolFont);  
void WriteText(HDC hdc, short xoff, short yoff, WindowLayout::Constraint xAlign, WindowLayout::Constraint yAlign, 
					WindowLayout::Orientation direction, LOGFONT logFont, 
               COLORREF fgColor, COLORREF bgColor, const char *text, ShortRect &textRect);
 bool SegmentIntersectRectangle(double,double,double,double,double,double,double);
#endif //define DRAWING_H
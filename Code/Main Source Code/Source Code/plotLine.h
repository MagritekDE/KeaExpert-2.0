#ifndef PLOTLINE_H
#define PLOTLINE_H

#include <Windows.h>
#include "Plot.h"
#include "Translator.h"

enum LineStyle
{
   SOLID = 0,
   DASH  = 1,           /* -------  */
   DOT   = 2,           /* .......  */
   DASHDOT = 3,         /* _._._._  */
   DASHDOTDOT = 4       /* _.._.._  */
};


class PlotLine
{
public:
   float x0,y0;
   float x1,y1;
   COLORREF color;
   LineStyle style;
   short thickness;
   short mode;
   int units;
   PlotLine();
   ~PlotLine();
   PlotLine(float xa, float ya, float xb, float yb, COLORREF col, short lineWidth, LineStyle sty, short units);
   PlotLine(float xa, float ya, float xb, float yb, float col, float lineWidth, float sty);
   void Draw(Plot *plt, HDC hdc, short dim);
   void Save(Plot *plt, FILE *fp);
   void Save3_8(Plot *plt, FILE *fp);
   void Load(Plot *plt, FILE *fp);
   void Load3_8(Plot *plt, FILE *fp);
};

#endif // define PLOTLINE_H

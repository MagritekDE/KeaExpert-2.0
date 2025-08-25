#ifndef PLOTTEXT_H
#define PLOTTEXT_H

#include <Windows.h>
#include "ctext.h"
#include "Plot.h"
#include "Translator.h"


class PlotText
{
public:
   PlotText();
   PlotText(float x0, float y0, float** shiftMatrix, char* str, CText fontName, int fontSize, float fontAngle, CText fontStyle, COLORREF col, CText units);
   ~PlotText(void);

   void Draw(Plot *plt, HDC hdc, short dim);
   void Save(Plot *plt, FILE *fp);
   void Load(Plot *plt, FILE *fp);

public:
   float x,y;
   int size;
   float angle;
   CText txt;
   CText font;
   int units;
   COLORREF color;
   float xw, xh;
   float yw, yh;
   int style;
};

#endif // define PLOTTEXT_H

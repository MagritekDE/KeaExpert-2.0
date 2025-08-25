#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)
#include <gdiplus.h> 
#include "Plot.h"
#include "Translator.h"
#include "plotLine.h"
#include "memoryLeak.h"

using namespace Gdiplus;

// Allows drawing of line on the 1D or 2D plots independently of the underlying data

PlotLine::PlotLine()
{
   x0 = 0;
   y0 = 0;
   x1 = 0;
   y1 = 0;
   color = RGB(0,0,0);
   style = LineStyle::SOLID;
   thickness = 1;
   units = 0;
}

PlotLine::PlotLine(float xa, float ya, float xb, float yb, COLORREF col, short lineWidth, LineStyle sty, short plotUnits)
{
   x0 = xa;
   y0 = ya;
   x1 = xb;
   y1 = yb;
   color = col;
   style = sty;
   thickness = lineWidth;
   units = plotUnits;
}

PlotLine::PlotLine(float xa, float ya, float xb, float yb, float col, float lineWidth, float sty)
{
   x0 = xa;
   y0 = ya;
   x1 = xb;
   y1 = yb;
   color = (COLORREF)(int)(col+0.5);
   style = (LineStyle)(short)(sty+0.5);
   thickness = (short)lineWidth;
   units = 0;
}

PlotLine::~PlotLine()
{
  // TextMessage("Deleting a line\n");
}

// Draw a line
void PlotLine::Draw(Plot *plt, HDC hdc, short dim)
{
   SaveDC(hdc);
   Point pt0,pt1;
   Graphics gfx(hdc); 
   gfx.SetSmoothingMode(SmoothingModeAntiAlias);
   Pen pen(Color(GetRValue(color), GetGValue(color), GetBValue(color))); 
   pen.SetWidth(thickness);
   pen.SetDashStyle((Gdiplus::DashStyle)style);
   pen.SetLineJoin(LineJoinBevel);

   Translator *xtrans = plt->curXAxis()->translator();
   Translator *ytrans = plt->curYAxis()->translator();

   if(dim == 2) // 2D
   {
      pt0.X = xtrans->userToScrn(x0); pt0.Y = ytrans->userToScrn(y0);
      pt1.X = xtrans->userToScrn(x1); pt1.Y = ytrans->userToScrn(y1);
   }
   else // 1D
   {
      short mappingBak = plt->curYAxis()->mapping();
      if(mappingBak == PLOT_LOG_Y && plt->axesMode == PLOT_AXIS_BOX_Y_INDEPENDENT) // Special case for this axes mode
      {
         plt->curYAxis()->setMapping(PLOT_LINEAR_Y); // Force plot mode to linear
         ytrans = plt->curYAxis()->translator(); // The above command destroys ytrans
      }
      if(units == 0) // Plot units
      {
         pt0.X = xtrans->dataToScrn(x0); pt0.Y = ytrans->dataToScrn(y0);
         pt1.X = xtrans->dataToScrn(x1); pt1.Y = ytrans->dataToScrn(y1);
      }
      else if(units == 1) // Pixels
      {
			if(x0 < 0)
				pt0.X = plt->GetWidth() + x0 + plt->GetLeft();
			else
				pt0.X = x0 + plt->GetLeft();

			if(y0 < 0)
				pt0.Y = plt->GetHeight() + y0 + plt->GetTop();
			else
				pt0.Y = y0 + plt->GetTop();

			if(x1 < 0)
				pt1.X = plt->GetWidth() + x1 + plt->GetLeft();
			else
				pt1.X = x1 + plt->GetLeft();

			if(y1 < 0)
				pt1.Y = plt->GetHeight() + y1 + plt->GetTop();
			else
				pt1.Y = y1 + plt->GetTop();

      }
      else if(units == 2) // Plot units (x) and fractional height (y)
      {
         pt0.X = xtrans->dataToScrn(x0);
         pt1.X = xtrans->dataToScrn(x1);
         pt0.Y = plt->GetBottom() - plt->GetHeight()*y0;
         pt1.Y = plt->GetBottom() - plt->GetHeight()*y1;
      }
      else if(units == 3) // Plot units (x) and pixels (y)
      {
         pt0.X = xtrans->dataToScrn(x0);
         pt1.X = xtrans->dataToScrn(x1);
			if(y0 >= 0)
			{
				pt0.Y = plt->GetTop() + y0;
				pt1.Y = plt->GetTop() + y1;
			}
			else
			{
				pt0.Y = plt->GetBottom() + y0;
				pt1.Y = plt->GetBottom() + y1;
			}
      }
      plt->curYAxis()->setMapping(mappingBak); // Restore plot mode
   }
   Rect clipRect(plt->GetLeft(), plt->GetTop(), plt->GetWidth(), plt->GetHeight());
   gfx.SetClip(clipRect, Gdiplus::CombineModeReplace);
   gfx.DrawLine(&pen,pt0,pt1);
	RestoreDC(hdc, -1);
}

// Load a line pre 3.8
void PlotLine::Load(Plot *plt, FILE *fp)
{
   fread(&x0,sizeof(float),1,fp);
   fread(&y0,sizeof(float),1,fp);
   fread(&x1,sizeof(float),1,fp);
   fread(&y1,sizeof(float),1,fp);
   fread(&color,sizeof(COLORREF),1,fp);
   fread(&thickness,sizeof(short),1,fp);
   fread(&style,sizeof(LineStyle),1,fp);
}

// Load a line V 3.8
void PlotLine::Load3_8(Plot *plt, FILE *fp)
{
   fread(&x0,sizeof(float),1,fp);
   fread(&y0,sizeof(float),1,fp);
   fread(&x1,sizeof(float),1,fp);
   fread(&y1,sizeof(float),1,fp);
   fread(&color,sizeof(COLORREF),1,fp);
   fread(&thickness,sizeof(short),1,fp);
   fread(&style,sizeof(LineStyle),1,fp);
   fread(&units,sizeof(float),1,fp);
}

// Save a line
void PlotLine::Save(Plot *plt, FILE *fp)
{
   fwrite(&x0,sizeof(float),1,fp);
   fwrite(&y0,sizeof(float),1,fp);
   fwrite(&x1,sizeof(float),1,fp);
   fwrite(&y1,sizeof(float),1,fp);
   fwrite(&color,sizeof(COLORREF),1,fp);
   fwrite(&thickness,sizeof(short),1,fp);
   fwrite(&style,sizeof(LineStyle),1,fp);
}

// Save a line 3.80
void PlotLine::Save3_8(Plot *plt, FILE *fp)
{
   fwrite(&x0,sizeof(float),1,fp);
   fwrite(&y0,sizeof(float),1,fp);
   fwrite(&x1,sizeof(float),1,fp);
   fwrite(&y1,sizeof(float),1,fp);
   fwrite(&color,sizeof(COLORREF),1,fp);
   fwrite(&thickness,sizeof(short),1,fp);
   fwrite(&style,sizeof(LineStyle),1,fp);
   fwrite(&units,sizeof(float),1,fp);
}


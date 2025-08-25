#ifndef PLOTWINDEFAULTS_H
#define PLOTWINDEFAULTS_H

#include "Plot.h"


class TracePar;

class PlotWinDefaults
{
   public:
		PlotWinDefaults(); // Constructor
		~PlotWinDefaults(); // Destructor

		bool antiAliasing;
	   short autoXScale;
	   short autoYScale;
	   short axesMode;   
		Ticks* ticks;
	   COLORREF bkColor;
	   COLORREF axesColor;
	   COLORREF borderColor;
	   COLORREF gridColor;
	   COLORREF fineGridColor;
	   COLORREF labelFontColor;
	   COLORREF titleFontColor;
      char axesFontName[FONT_NAME_LENGTH];
      char titleFontName[FONT_NAME_LENGTH];
      char labelFontName[FONT_NAME_LENGTH];
      short titleFontSize;
      short labelFontSize;
      short titleFontStyle;
      short labelFontStyle;
	   TracePar tracePar; // Default trace parameters
	   void LoadPlotWinParameters(FILE*);
      void Modify(int parameter, COLORREF col);	  
      char* GetAxesTypeStr();
      bool SetAxesType(CText type);
	   COLORREF zoomBkgColor;
	   COLORREF zoomBorderColor;
      CText zoomRectMode;
};

extern PlotWinDefaults *pwd;

#endif // define PLOTWINDEFAULTS_H
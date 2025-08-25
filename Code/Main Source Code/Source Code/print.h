#ifndef PRINT_H
#define PRINT_H

class PlotWindow;

extern float   gPrintWidth;
extern float   gPrintHeight;
extern float   gPrintLongTickLength; 
extern float   gPrintShortTickLength;
extern float   gPrintSymbolSize; 
extern short   gTitleFontSize;
extern bool    gFitToPage;
extern short   gPrintMode;
extern short   gPlotMode;
extern bool    gMatchWindow;

short PrintText(HWND);
short PrintDisplay(PlotWindow*);

char* GetPrintParameter(short which); 

#endif // define PRINT_H
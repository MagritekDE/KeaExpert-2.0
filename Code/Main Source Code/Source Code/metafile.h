#ifndef METAFILE_H
#define METAFILE_H

class PlotWindow1D;
class PlotWindow2D;
class Interface;

short Make1DWMF(PlotWindow1D *pw, char *fileName);
short Make2DWMF(PlotWindow2D *pw, char *fileName);
short Make3DWMF(HWND hWnd, char *fileName);

short Save3DAsImage(HWND hWnd, char *fileName);

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
int SetPlotScaleFactor(Interface* itfc ,char args[]);

extern float   gEMFWidth; // Windows meta file parameters
extern float   gEMFHeight;
extern float   gPlotSF;
extern float   gEMFLongTickLength; 
extern float   gEMFShortTickLength;
extern float   gEMFSymbolSize;

#endif //define METAFILE_H

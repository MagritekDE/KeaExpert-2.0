#ifndef PLOT2DCLI_H
#define PLOT2DCLI_H

class Interface;

extern float **gDefaultColorMap;
extern long gDefaultColorMapLength;
extern long plot2DSaveVersion;

int CALLBACK Plot2DColorDlgProc (HWND, UINT, WPARAM, LPARAM);

int Clear2D(Interface* itfc, char args[]);
int SurfacePlot(char args[]);
int SetWindowMargins(Interface *itfc, char args[]);
int DefaultWindowMargins(Interface *itfc, char args[]);
int GetDataRange(Interface* itfc ,char[]);
int Zoom2D(Interface *itfc, char args[]);
int DisplayMatrixAsImage(Interface* itfc ,char[]);
int DrawImage(Interface* itfc ,char[]);
int GetColorMap(Interface *itfc, char args[]);
int SetColorMap(Interface* itfc ,char[]);
int ContourPlot(Interface* itfc ,char[]);
int SetImageRange(Interface* itfc ,char[]);
int TrackCursor(Interface* itfc ,char[]);
int ShowColorScale(Interface* itfc ,char args[]);
int Get2DCoordinate(Interface *itfc, char args[]);
// CLI interface functions
int Plot2DFunctions(Interface *itfc,char args[]);
int ImageFileSaveVersion(Interface* itfc, char args[]);

#endif // define PLOT2DCLI_H

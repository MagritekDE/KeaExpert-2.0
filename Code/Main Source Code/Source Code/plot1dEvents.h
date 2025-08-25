#ifndef PLOT1DEVENTS_H
#define PLOT1DEVENTS_H

class Interface;
class PlotWindow1D;
class Plot;

LRESULT CALLBACK  PlotEventsProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);

// CLI interface functions
int  ViewSubPlot(Interface *itfc, char args[]);
int  KeepSubPlot(Interface* itfc ,char args[]);
int  Plot1DFunctions(Interface *itfc,char args[]);
int  ViewFullPlot(Interface *itfc, char args[]);
int RemoveSelectionRect(Interface* itfc ,char args[]);

#endif // define PLOT1DEVENTS_H
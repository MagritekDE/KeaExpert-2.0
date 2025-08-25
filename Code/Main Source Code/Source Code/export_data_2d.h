#ifndef EXPORT_DATA_2D_H
#define EXPORT_DATA_2D_H

class Interface;

short Export2DData(Interface*, char[], long, char*);
short Export2DDataDialog(HWND,char*,char*);
int Export2DDataCLI(Interface* itfc ,char args[]);
int Export2DDataParameters(Interface* itfc ,char args[]);

#endif // define EXPORT_DATA_2D_H

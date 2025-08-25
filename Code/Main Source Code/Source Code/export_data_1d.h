#ifndef EXPORT_DATA_1D_H
#define EXPORT_DATA_1D_H

class Interface;

int Export1DDataParameters(Interface* itfc ,char args[]);
short Export1DData(Interface*, char[], long, char*, char*);
short Export1DDataDialog(HWND,char*,char*);
int Export1DDataCLI(Interface* itfc ,char args[]);

#endif // define EXPORT_DATA_1D_H
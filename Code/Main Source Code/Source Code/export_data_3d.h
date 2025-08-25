#ifndef EXPORT_DATA_3D_H
#define EXPORT_DATA_3D_H

class Interface;

short Export3DData(Interface*, char[], long, char*);
short Export3DDataDialog(HWND,char*,char*);
int   Export3DDataCLI(Interface* itfc ,char args[]);
int   Export3DDataParameters(Interface* itfc ,char args[]);

#endif // define EXPORT_DATA_3D_H
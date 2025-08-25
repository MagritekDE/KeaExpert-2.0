#ifndef IMPORT_DATA_3D_H
#define IMPORT_DATA_3D_H

class Interface;

short Import3DData(Interface *itfc, short, char[], long, char*, long, long, long, bool);
short Import3DDataDialog(HWND, char*, char*);
int Import3DDataCLI(Interface* itfc ,char args[]);
int Import3DDataParameters(Interface* itfc ,char args[]);

#endif // define IMPORT_DATA_3D_H
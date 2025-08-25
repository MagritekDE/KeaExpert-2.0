#ifndef IMPORT_DATA_1D_H
#define IMPORT_DATA_1D_H

class Interface;
class Variable;

short Import1DData(Interface *itfc, bool, char[], long, char*, char*, Variable*, Variable*,long);
int Import1DDataParameters(Interface* itfc ,char arg[]);
short Import1DDataDialog(HWND,char*,char*);
int Import1DDataCLI(Interface* itfc ,char args[]);

#endif // define IMPORT_DATA_1D_H
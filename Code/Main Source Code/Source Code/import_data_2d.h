#ifndef IMPORT_DATA_2D_H
#define IMPORT_DATA_2D_H

class Interface;

void RemoveHeaders(char *buffer, size_t &bufferLength, long type, long xsize,long ysize, long zsize, long fileHeader, long rowHeader);
void ReorderBuffer(char *buffer, size_t fileLength, long type);
long DataPointsInBuffer(char *buffer, size_t fileLength, long type);

int Import2DDataParameters(Interface* itfc ,char args[]);
short Import2DData(Interface *itfc, bool, char[], long, char*, long, long, bool);
short Import2DDataDialog(HWND,char*,char*);
int Import2DDataCLI(Interface* itfc ,char args[]);

#endif // define IMPORT_DATA_2D_H

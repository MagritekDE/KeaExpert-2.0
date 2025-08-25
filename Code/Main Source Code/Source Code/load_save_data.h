#ifndef LOAD_SAVE_DATA_H
#define LOAD_SAVE_DATA_H

#include "defines.h"

class Interface;
class Variable;

short LoadFileIntoBuffer(char *fileName, size_t &fileLength, char **buffer);
short LoadDataDialog(HWND, char*, char*);
short LoadListData(char* pathName, char* fileName, Variable *ans);
short LoadListDataWithTrueDoubles(char* pathName, char* fileName, Variable *ans);
int GetFileDimensions(Interface* itfc ,char[]);
int SetCurrentFolder(Interface *itfc, char arg[]);
short SetFolderDialog(HWND hWnd);
short SaveDataDialog(HWND, char*, char*);
short LoadData(Interface*, char*, char*, char*, short);
short LoadData(Interface*, char*, char*, Variable*, Variable*);
short SaveData(HWND, char*, char*, Variable*);
short SaveData(HWND hWnd, char* pathName, char* fileName, Variable *varX, Variable *varY);
short Save2DData(char* pathName, char* fileName, Variable *varMat, Variable *varXAxis, Variable *varYAxis);

bool IsDelimiter(char c);
int GetFolderName(Interface* itfc ,char args[]);

short SaveListData(HWND, char*, char*, Variable*);
short SaveListDataTrueDoubles(HWND hWnd, char* pathName, char* fileName, Variable *var);
short SaveStructDataAsList(HWND hWnd, char* pathName, char* fileName, Variable *var);
short SaveStructDataInBinary(HWND hWnd, char* pathName, char* fileName, Variable *var);

short Save3DDataToBinaryFile(char[], long, float***, complex***, long, long, long);
short Save2DDataToBinaryFile(char[], long, float**, complex**, long, long);
short Save1DDataToBinaryFile(char[], long, float*, float*, complex*, long);
short Save1DDataToAsciiFile(char[], long, float*, float*, complex*, long);
short Save2DDataToAsciiFile(char[], long, float**, complex**, long, long);
short Save3DDataToAsciiFile(char[], long, float***, complex***, long, long, long);

unsigned long ByteReverse32(unsigned long number);
unsigned short ByteReverse16(unsigned short number);
 
#endif // define LOAD_SAVE_DATA_H


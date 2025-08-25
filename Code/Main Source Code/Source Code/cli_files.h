#ifndef CLI_FILES_H
#define CLI_FILES_H

class Interface;


extern char gPlot3DDirectory[300];
extern char gImport3DDataDirectory[300];
extern char gExport3DDataDirectory[300];

/*************************************************************************
*               CLI routines to load and save data                       *
*************************************************************************/

int Load(Interface* itfc ,char args[]);
int Save(Interface* itfc ,char args[]);
int ExtractDataFromPlot(Interface *itfc, char args[]);
int Multiplot(Interface* itfc ,char args[]);
int SetOrGetCurrentPlot(Interface *itfc, char arg[]);
int SetPathNames(Interface *itfc, char args[]);

/*************************************************************************
*               CLI routines to handle paths and filenames               *
*************************************************************************/

int RemoveFileExtension(Interface* itfc ,char args[]);
int DoesFileExist(Interface *itfc, char arg[]);
int DoesDirectoryExist(Interface *itfc, char arg[]);
int GetFileExtension(Interface* itfc, char[]);
int SimplifyDirectory(Interface* itfc ,char args[]);

#endif // define CLI_FILES_H
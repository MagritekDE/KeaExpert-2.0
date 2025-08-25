#ifndef CLI_EVENTS_H
#define CLI_EVENTS_H

#include "defines.h"

#define COPY	3
#define PASTE	22
#define UNDO	26
#define CUT		24
#define ENTER  13

class Interface;

// Locally define functions
LRESULT CALLBACK CLIEditEventsProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
int CLIFunctions(Interface* itfc ,char args[]);
int ClosePrintFile(Interface* itfc ,char args[]);
int PrintToFile(Interface* itfc, char args[]);
void SendTextToCLI(char *message);
int SetOrGetCurrentCLI(Interface *itfc, char args[]);
EXPORT void TextMessage(const char *const text,...);

// Globally defined variables
extern FILE *printFile;     // Used when sending CLI output to file


#endif // define CLI_EVENTS_H
#ifndef PLOT3DEVENTS_H
#define PLOT3DEVENTS_H

class Interface;

LRESULT CALLBACK Plot3DEventsProc(HWND, UINT, WPARAM, LPARAM);

int Set3DControlPrefs(Interface *itfc, char arg[]);
int Plot3DFunctions(Interface* itfc ,char args[]);

void Process3DScrollWheelEvents(HWND hWnd, short zDel, short fwKeys);
void OpenGLBitmapToEMF(HDC hdcEMF, HDC hdcMem, long x, long y, long w, long h);

#endif // define PLOT3DEVENTS_H
#ifndef UTILITIES_H
#define UTILITIES_H

class Interface;
class Plot;
class WinData;
class ObjectData;

// Colour selection
int SelectColour(Interface* itfc ,char arg[]);
UINT APIENTRY ChooseColorHookProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
void CorrectWindowPositionIfInvisible(short& x, short& y, short w, short h);
void GetMonitorRect(RECT* r, short* num_monitors);

// Templated functions
template <class T> void Swap (T &s1, T &s2){T temp = s1;s1 = s2;s2 = temp;}

#endif // define UTILITIES_H
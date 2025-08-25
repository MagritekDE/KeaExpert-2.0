#ifndef DIVIDERS_H
#define DIVIDERS_H

#define DIVIDER_OFFSET 10 // Closest divider can come to another divider or the window edge

class WinData;
class ObjectData;

LRESULT CALLBACK DividerEventsProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam);
void ResizeObjectsWhenParentResizes(WinData *win, ObjectData *objDiv, HWND hWnd, short x, short y);

#endif // define DIVIDERS_H
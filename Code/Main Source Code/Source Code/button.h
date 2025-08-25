#ifndef BUTTON_H
#define BUTTON_H

class Interface;
class ObjectData;
class CText;
class Variable;
class WinData;

extern int DrawButton(ObjectData *obj, LPDRAWITEMSTRUCT pdis);
extern void DisplayButtonText(HDC hdc, ObjectData *obj, RECT r, COLORREF col);

#endif // define BUTTON_H
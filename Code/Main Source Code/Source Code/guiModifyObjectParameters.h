#ifndef GUIMODIFYOBJECTPARAMETERS_H
#define GUIMODIFYOBJECTPARAMETERS_H

#include "defines.h"

class Interface;
class ObjectData;
class CText;
class Variable;
class WinData;

int GetTextBoxParameter(Interface *itfc, ObjectData *obj, CText &parameter, Variable *ans);
int GetSliderParameter(ObjectData *obj, CText &parameter, Variable *ans);
int GetButtonParameter(ObjectData *obj, CText &parameter, Variable *ans);
int GetCLIParameter(ObjectData *obj, CText &parameter, Variable *ans);

//int EnableControl(char arg[]);
int FindObjectCLI(Interface *itfc, char args[]);

short UpdateStatusWindow(HWND win, int pos, char *text);

void SelectFixedControls(WinData *win, ObjectData *src);
void SelectFixedControls(WinData *win);

// Get object parameter and helper functions
int GetParameter(Interface* itfc ,char arg[]);

// Set object parameters and helper functions
EXPORT int SetParameter(Interface* itfc ,char arg[]);

int GetParameterCore(Interface *itfc, ObjectData* obj, CText &parameter);
int SetParameterCore(Interface *itfc, ObjectData* obj, CText &parameterIn, CText &valueIn);

short GetObjectDimensions(char *str, float *scale, float *off, bool *region);
short ProcessObjectPosition(WinData *win,
									 Variable *ww, Variable *wh,
									 Variable *wx, Variable *wy,
									 short &x,short &y, short &w, short &h,
									 ObjPos *pos);

short SaveObjectPosition(ObjectData *obj, ObjPos *pos);

int LoadPictureTo3DMatrix(Interface *itfc, char *args);
void UpdateObject(ObjectData*);
int ProcessValuePosition(Variable* value, float* in_scale, float* in_offset, bool* in_region, char* description);
void SetProgressBarValue(ObjectData *obj, int value);

#endif // define GUIMODIFYOBJECTPARAMETERS_H
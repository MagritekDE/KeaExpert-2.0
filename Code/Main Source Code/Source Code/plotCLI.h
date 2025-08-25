#ifndef PLOTCLI_H
#define PLOTCLI_H

class Interface;
class CText;

int PlotPreferences(Interface* itfc, char args[]);
void ReturnColour(Interface *itfc, COLORREF colIn, float* colOut);
int ModifyGrid(Interface* itfc, char args[]);
int SetTitleParameters(Interface* itfc, char args[]);
int SetXLabelParameters(Interface* itfc, char args[]);
int SetYLabelParameters(Interface* itfc, char args[]);
int SetLeftYLabelParameters(Interface* itfc, char args[]);
int SetRightYLabelParameters(Interface* itfc, char args[]);
int LoadPlotMode(Interface* itfc ,char args[]);
int ModifyAxes(Interface* itfc, char args[]);
int ShowPlotBorder(Interface* itfc ,char args[]);
int SetBkColor(Interface *itfc, char arg[]);
int SetBorderColor(Interface *itfc, char arg[]);
int GetSelectRectangle(Interface *itfc, char args[]);

int SetOrGetCurrentAxis(Interface* itfc, char args[]);
int SetOrGetSyncAxes(Interface* itfc, char args[]);

#endif // define PLOTCLI_H
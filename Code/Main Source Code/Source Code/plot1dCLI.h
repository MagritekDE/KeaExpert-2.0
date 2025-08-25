#ifndef PLOT1DCLI_H
#define PLOT1DCLI_H

#include "defines.h"

class Interface;
class ShortRect;
class Variable;
class Plot;

short ProcessPlotClassReferences(Interface *itfc, Plot *pd, char* name, char *args);
int PlotXY(Interface* itfc ,char arg[]);
int Clear1D(Interface* itfc, char args[]);
int SetPlotHoldMode(Interface* itfc, char args[]);
int GetOrSetCurrentTrace(Interface* itfc, char args[]);
int ModifyTraceDefault(Interface* itfc ,char[]);
int GetCurrentPlot(Interface *itfc, char args[]);
int AutoRange(Interface* itfc, char args[]);
int DrawPlot(Interface* itfc, char[]);
int ModifyTrace(Interface* itfc, char args[]);
int Zoom1D(Interface* itfc, char args[]);
int Get1DCoordinate(Interface* itfc, char arg[]);
int SetPlotState(Interface *itfc, char args[]);
int GetPlotState(Interface *itfc, char arg[]);
short SetColor(COLORREF &colOut, Variable *colIn);
long fileVersionNumberToConstant(long fileVersionNumber, short dim);
long fileVersionConstantToNumber(long fileVersionConstant, short dim);
int PlotFileSaveVersion(Interface* itfc, char args[]);

short ConvertAnsVar(Variable* ans, char name[], short type);
short ConvertAnsVar(Variable*, char[], short, long&);
short ConvertAnsVar(Variable*, short, float&, float&);
short ConvertAnsVar(Variable*, char[], short, COLORREF&);
short ConvertAnsVar(Variable*, char[], short, COLORREF*);
short ConvertAnsVar(Variable*, char[], short, bool&);
short ConvertAnsVar(Variable*, char[], short, float&);
short ConvertAnsVar(Variable*, char[], short, short&);
short ConvertAnsVar(Variable*, char[], short, short*);
short ConvertAnsVar4Color(Variable *ans, char name[], short type, COLORREF &out);
short ConvertAnsVar(Variable* ans, char name[], short type, CText *out);

extern long plot1DSaveVersion;

#endif // define PLOT1DCLI_H
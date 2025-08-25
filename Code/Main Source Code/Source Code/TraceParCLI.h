#ifndef TRACEPARCLI_H
#define TRACEPARCLI_H

/************************************************************************
* TraceParCLI
*
* Command line access to TracePar
*
* (c) Magritek 2011
*************************************************************************/

class CArg;
class Trace;
class Interface;
class TracePar;
class Plot1D;

short GetOrSetTraceParameters(Interface *itfc, CArg *carg, short nrArgs, short start, Trace *di, TracePar& tracePar, Plot1D *plot);

#endif TRACEPARCLI_H

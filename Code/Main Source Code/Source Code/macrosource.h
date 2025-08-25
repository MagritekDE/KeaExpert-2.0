#ifndef MACROSOURCE_H
#define MACROSOURCE_H

#include "trace.h"

class ObjectData;
class Variable;
class WinData;

class MacroSrcInfo
{
	public :
	   MacroSrcInfo();

   CText name;
   CText macroName;
   CText macroPath;
   CText procName;
   ObjectData *obj;
   WinData *win;
   Variable *retVar;
   int nrRetValues;
   Variable *argVar;
   int nrProcArgs;
   long objID;
};

#endif // define MACROSOURCE_H
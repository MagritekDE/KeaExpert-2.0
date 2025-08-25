#include "stdafx.h"
#include "ProspaCmdInfo.h"
#include "CmdInfo.h"
#include <string>
#include "memoryLeak.h"

using std::string;

/************************************************************************
* ProspaCmdInfo
*
* Represents user information about a built in Prospa command, including 
* its command line syntax, type, and implementing function.
*
* (c) Magritek 2010
*************************************************************************/

/************************************************************************
Constructor for ProspaCmdInfo. 
************************************************************************/
ProspaCmdInfo::ProspaCmdInfo(string* name, int (*func)(Interface*,char[]), LONG64 type, string* syntax)
	:CmdInfo(name,type,syntax)
{
	this->funcI = func;
}

/************************************************************************
Invokes the function associated with this ProspaCmdInfo's command.
************************************************************************/
int ProspaCmdInfo::call(Interface* itfc, char args[])
{
	return this->funcI(itfc,args);
}
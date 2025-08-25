#ifndef PROSPACMDINFO_H
#define PROSPACMDINFO_H

#include "CmdInfo.h"
#include <string>
/************************************************************************
* ProspaCmdInfo
*
* Represents user information about a built in Prospa command, including 
* its command line syntax, type, and implementing function.
*
* (c) Magritek 2010
*************************************************************************/
class ProspaCmdInfo : public CmdInfo
{
public:
	ProspaCmdInfo(std::string* name, int (*func)(Interface*,char[]), LONG64 type, std::string* syntax);
	//Invokes the function associated with this CmdInfo's command.
	int call(Interface*, char[]);
private:
	int (*funcI)(Interface*,char[]);
};

#endif //define PROSPACMDINFO_H
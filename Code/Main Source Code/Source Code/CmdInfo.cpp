#include "stdafx.h"
#include "CmdInfo.h"
#include "command.h"
#include <string>
#include <vector>
#include "memoryLeak.h"

using std::string;
using std::vector;

/************************************************************************
* CmdInfo
*
* Represents user information about a command line command, including 
* its name, command line syntax, and type
*
* (c) Magritek 2010
*************************************************************************/

/************************************************************************
Constructor for CmdInfo. 
************************************************************************/

CmdInfo::CmdInfo(string* name, LONG64 type, string* syntax)
{
	this->name = name;
	this->type = type;
	this->syntax = syntax;
}

/************************************************************************
Destructor for CmdInfo
************************************************************************/
CmdInfo::~CmdInfo()
{
	delete name;
	delete syntax;
}

/************************************************************************
Returns the name of this CmdInfo's command.
************************************************************************/
const string* CmdInfo::getName()
{
	return this->name;
};

/************************************************************************
Returns the type of this CmdInfo's command.
************************************************************************/
LONG64 CmdInfo::getType(void)
{
	return this->type;
}

/************************************************************************
Returns the command line syntax of this CmdInfo's command.
************************************************************************/
const string* CmdInfo::getSyntax(void)
{
	return this->syntax;
}
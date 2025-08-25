#include "stdafx.h"
#include "WidgetCmdInfo.h"
#include "CmdInfo.h"
#include <string>
#include "memoryLeak.h"

using std::string;

/************************************************************************
* WidgetCmdInfo
*
* Represents user information about a built in Prospa widget command, 
* including its command line syntax and associated widget types.
*
* (c) Magritek 2010
*************************************************************************/

WidgetCmdInfo::WidgetCmdInfo(string* name, string* associatedWidgetName, string* syntax, bool various)
	:CmdInfo(name,WIDGET_CMD,syntax)
{	
	this->associatedWidgetName = associatedWidgetName;
	this->various = various;
}

WidgetCmdInfo::~WidgetCmdInfo()
{
	delete associatedWidgetName;
}

/************************************************************************
Returns the widget type associated with the command. 
************************************************************************/
const string* WidgetCmdInfo::getAssociatedWidgetName()
{
	return this->associatedWidgetName;
}

/************************************************************************
Returns true if the command is associated with multiple widget types.
************************************************************************/
bool WidgetCmdInfo::isVarious()
{
	return this->various;
}

/************************************************************************
Formats the syntax string.
This should be owned by something else.
************************************************************************/
string* WidgetCmdInfo::formatSyntaxString(const char* prefix, char* widgetNamePrefix, char* widgetSyntax)
{
	string* newstring = new string();
	if (prefix)
	{
		newstring->append(prefix);
		newstring->append("        ");
	}
	newstring->append("[");
	newstring->append(widgetNamePrefix);
	newstring->append("]: ");
	newstring->append(widgetSyntax);
	return newstring;
}
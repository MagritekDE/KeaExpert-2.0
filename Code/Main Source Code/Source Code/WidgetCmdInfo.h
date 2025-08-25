#ifndef WIDGETCMDINFO_H
#define WIDGETCMDINFO_H

#include "CmdInfo.h"
#include <string>
/************************************************************************
* WidgetCmdInfo
*
* Represents user information about a built in Prospa widget command, 
* including its command line syntax and associated widget types.
*
* (c) Magritek 2010
*************************************************************************/
class WidgetCmdInfo : public CmdInfo
{
public:
	WidgetCmdInfo(std::string* name, std::string* associatedWidgetName, std::string* syntax,bool various=false);
	~WidgetCmdInfo();
	// Returns the widget type associated with the command. 
	const std::string* getAssociatedWidgetName();
	// Returns true if the command is associated with multiple widget types.
	bool isVarious();
	// Formats the syntax string.
	// This should be owned by something else.
	static std::string* formatSyntaxString(const char* prefix, char* widgetNamePrefix, char* widgetSyntax);

private:
	std::string* associatedWidgetName;
	bool various;
};

#endif //define WIDGETCMDINFO_H
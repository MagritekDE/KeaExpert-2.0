#include "stdafx.h"
#include "CommandRegistry.h"
#include "CmdInfo.h"
#include "ProspaCmdInfo.h"
#include "WidgetCmdInfo.h"
#include "globals.h"
#include <string>
#include <vector>
#include "memoryLeak.h"

using std::string;
using std::vector;
/************************************************************************
* CommandRegistry
*
* Interface to a list of commands. Users can add command line commands, 
* query them, and invoke them.
*
* (c) Magritek 2010
************************************************************************/

CommandRegistry::CommandRegistry(RegistryType cmdType)
{
	this->registry = trie_new();
	type = cmdType;
}

CommandRegistry::~CommandRegistry()
{
	if(type == COMMAND)
	{
		for (vector<char*>::iterator it = allOfTheKeys.begin(); it < allOfTheKeys.end(); ++it)
		{
			CmdInfo* killMe = (CmdInfo*) trie_lookup(registry, (char*)(*it));
			delete killMe;
		}
		trie_free(this->registry);
	}
	else
	{
		for (vector<char*>::iterator it = allOfTheKeys.begin(); it < allOfTheKeys.end(); ++it)
		{
			WidgetCmdInfo* killMe = (WidgetCmdInfo*) trie_lookup(registry, (char*)(*it));
			delete killMe;
		}
		trie_free(this->registry);
	}
}

/************************************************************************
Invokes the specified command, returning the result or NOT_FOUND_CMD
************************************************************************/
bool CommandRegistry::has(char* name)
{
	return (NULL != trie_lookup(this->registry, name));
}

/************************************************************************
Returns the command line syntax for the specified command, or NULL
************************************************************************/
const string* CommandRegistry::syntaxFor(char* name)
{
	CmdInfo* cmd = (CmdInfo*)trie_lookup(this->registry, name);
	return cmd ? cmd->getSyntax() : NULL;
}

/************************************************************************
Returns the associated widget name for the specified command, or NULL
************************************************************************/
const string* CommandRegistry::associatedWidgetNameFor(char* name)
{
	WidgetCmdInfo* cmd = (WidgetCmdInfo*)trie_lookup(this->registry, name);
	return cmd ? cmd->getAssociatedWidgetName() : NULL;
}
/************************************************************************
Returns the type of the specified command, or NOT_FOUND_CMD 
************************************************************************/
LONG64 CommandRegistry::typeOf(char* name)
{
	CmdInfo* cmd = (CmdInfo*)trie_lookup(registry,name);
	return cmd ? cmd->getType() : NOT_FOUND_CMD;
}

/************************************************************************
Adds the supplied command. This version is for ProspaCmdInfo. 
************************************************************************/
void CommandRegistry::add(char* name, int (*func)(Interface*,char[]), LONG64 type, char* syntax)
{	
	CmdInfo* cmd = new ProspaCmdInfo(new string(name), func, type, new string(syntax));
	trie_insert(this->registry, name, cmd);
	this->allOfTheKeys.push_back(name); // so it can be destroyed later
}

/************************************************************************
Adds the supplied command. This version is for WidgetCmdInfo. 
************************************************************************/
void CommandRegistry::add(char* name, char* help, char* syntax, bool various)
{	
	WidgetCmdInfo* cmd = NULL;

	if (trie_lookup(this->registry,name))
	// The requested item is already in the trie.
	// Concatenate the versions' syntax strings.
	{
		WidgetCmdInfo* match = (WidgetCmdInfo*)(trie_lookup(this->registry,name));
		cmd = new WidgetCmdInfo(new string(name),new string(help), WidgetCmdInfo::formatSyntaxString(match->getSyntax()->c_str(),help,syntax),various || match->isVarious());
		delete(match);
	}
	else
	{
		cmd = new WidgetCmdInfo(new string(name), new string(help), WidgetCmdInfo::formatSyntaxString(NULL,help,syntax),various);
		this->allOfTheKeys.push_back(name); // so it can be destroyed later
	}
	trie_insert(this->registry, name, cmd);
}

/************************************************************************
Invokes the specified command, returning the result or CMD_NOT_FOUND
************************************************************************/
int CommandRegistry::call(char* name, Interface* itfc, char args[])
{
	ProspaCmdInfo* cmd = NULL;
	if (!(cmd = (ProspaCmdInfo*)trie_lookup(registry, name)))
	{
		return CMD_NOT_FOUND;
	}
	else
	{
		if (WIDGET_CMD == cmd->getType())
		{
			return CMD_NOT_FOUND;
		}
		return cmd->call(itfc,args);
	}
}

/************************************************************************
Returns an unsorted vector of names of all commands
************************************************************************/
const vector<char*>& CommandRegistry::allCommands(void)
{
	return this->allOfTheKeys;
}

/************************************************************************
Widget commands can apply to multiple types of widgets ("various" ones)                                                                     
************************************************************************/
bool CommandRegistry::isVarious(char* name)
{
	WidgetCmdInfo* t = (WidgetCmdInfo*)(trie_lookup(this->registry,name));
	if (t && t->isVarious())
	{
		return true;
	}
	return false;
}
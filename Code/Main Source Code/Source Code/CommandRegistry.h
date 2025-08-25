#ifndef COMMANDREGISTRY_H
#define COMMANDREGISTRY_H

#include "CmdInfo.h"
#include "trie.h"
#include <string>
#include <vector>

/************************************************************************
* CommandRegistry
*
* Interface to a list of commands. Users can add command line commands, 
* query them, and invoke them.
*
* (c) Magritek 2010
************************************************************************/

enum RegistryType {COMMAND,WIDGET}; 
class CommandRegistry
{
public:
	CommandRegistry(RegistryType type);
	~CommandRegistry();
	// Adds the supplied command
		// Add a Prospa command
	void add(char* name, int (*func)(Interface*,char[]), LONG64 type, char* syntax); 
		// Add a Widget command
	void add(char* name, char* help, char* syntax, bool various = false);  
	// Returns true if the specified name is a command in the registry, 
	//  false otherwise
	bool has(char* name);
	// Invokes the specified command, returning the result or CMD_NOT_FOUND
	int call(char* name, Interface* itfc, char args[]);
	// Returns the command line syntax for the specified command, or NULL
	const std::string* syntaxFor(char* name);
	// Returns the type of the specified command, or NOT_FOUND_CMD 
	LONG64 typeOf(char* name);
	// Returns the associated widget name for the specified command, or NULL
	const std::string* associatedWidgetNameFor(char* name);
	// Returns an unsorted vector of names of all commands
	const std::vector<char*>& allCommands(void);
	// Widget commands can apply to multiple types of widgets ("various" ones)
	bool isVarious(char* name);
	RegistryType type;

private:
	Trie* registry;
	std::vector<char*> allOfTheKeys; // Use this in the destructor.
};


#endif // #define COMMANDREGISTRY_H
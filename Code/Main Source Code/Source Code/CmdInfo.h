#ifndef CMDINFO_H
#define CMDINFO_H

#include <string>

class Interface;

/************************************************************************
* CmdInfo
*
* Represents user information about a command line command, including 
* its name, command line syntax, and type
*
* (c) Magritek 2011
*************************************************************************/

enum CommandType
{
	// Specific types of Prospa commands
	NOT_FOUND_CMD   = 0x00000000,
	CONTROL_COMMAND = 0x00000001,
	HIDDEN_COMMAND  = 0x00000002,
	NOP_CONTROL_CMD = 0x00000004,
   LICENSE_CMD     = 0x00000008,
   MACRO_CMD       = 0x00000010,
   MATH_CMD        = 0x00000020,
   TRIG_CMD        = 0x00000040,
   FOURIER_CMD     = 0x00000080,
   MATRIX_CMD      = 0x00000100,
   LIST_CMD        = 0x00000200,
   STRUCT_CMD      = 0x00000400,
   STRING_CMD      = 0x00000800,
   FILE_CMD        = 0x00001000,
   THREAD_CMD      = 0x00002000,
   GUI_CMD         = 0x00004000,
   ONED_CMD        = 0x00008000,
   TWOD_CMD        = 0x00010000,
   THREED_CMD      = 0x00020000,
   HELP_CMD        = 0x00040000,
   VAR_CMD         = 0x00080000,
   TIME_CMD        = 0x00100000,
   SOUND_CMD       = 0x00200000,
   COLOR_CMD       = 0x00400000,
   SYSTEM_CMD      = 0x00800000,
	GENERAL_COMMAND = 0x01000000,
   DLL_CMD         = 0x02000000,
   WIDGET_CMD      = 0x04000000,
   CLASS_CMD       = 0x08000000,
   DEBUG_CMD       = 0x10000000
};

class CmdInfo
{
public:
	CmdInfo(std::string* name, LONG64 type, std::string* syntax);
	~CmdInfo();
	//Returns the type of this CmdInfo's command.
	LONG64 getType(void);
	//Returns the name of this CmdInfo's command.
	const std::string* getName();
	//Returns the command line syntax of this CmdInfo's command.
	const std::string* getSyntax();

private:
	std::string* name;
	LONG64 type;
	std::string* syntax;
};

#endif // define CMDINFO_H
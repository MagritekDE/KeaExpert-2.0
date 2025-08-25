#include "stdafx.h"
#include "environment.h"
#include "error.h"
#include "globals.h"
#include "interface.h"
#include "scanstrings.h"
#include "variablesClass.h"
#include "memoryLeak.h"

//#include "windows.h"


/****************************************************************************
*                 Inform user of free memory                                *
****************************************************************************/

int MemoryStatus(Interface* itfc ,char args[])
{
	MEMORYSTATUS status;

	status.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&status);

	itfc->retVar[1].MakeAndSetFloat(status.dwAvailPhys);
	itfc->nrRetValues = 1;
	return(0);
}

/************************************************************************
Get a Windows-level environment variable.
************************************************************************/

int GetEnvironmentVariable(Interface *itfc, char args[])
{
	short nrArgs;
	CText name;
	char path[MAX_PATH];

	if((nrArgs = ArgScan(itfc,args,1,"environment variable name","e","t",&name)) < 0)
		return(nrArgs); 

	if((name == "desktop") || (name == "Desktop"))
	{
		ExpandEnvironmentStrings("%USERPROFILE%", path, MAX_PATH);
		strcat(path,"\\Desktop");
	}
	else
	{
		name = "%" + name + "%";
		if(ExpandEnvironmentStrings(name.Str(), path, MAX_PATH) == 0)
		{
			ErrorMessage("invalid environment variable");
			return(ERR);
		}
	}
	itfc->retVar[1].MakeAndSetString(path);
	itfc->nrRetValues = 1;
	return(OK);

}
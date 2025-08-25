#pragma pack(push, 8)
#include <windows.h>
#pragma pack(pop)
#include <gdiplus.h>
#include "stdafx.h"
#include "error.h"
#include "cli_events.h"
#include "control.h"
#include "defines.h"
#include "defineWindows.h"
#include "guiWindowsEvents.h"
#include "interface.h"
#include "message.h"
#include "process.h"
#include "string_utilities.h"
#include "memoryLeak.h"

using namespace Gdiplus;

bool reportErrorToDialog = false;	// Externally accessible

EXPORT void ErrorMessage(const char * const text,...)
{
	va_list ap;
	char *output; 

	// Don't report errors if not aborting
	if(abortOnError == false)
		return;

	// Don't report errors if we are the processing a user abort
	if(gAbort == true)
		return;

	va_start(ap,text);

	// Extract all arguments and format them correctly
	output =  vssprintf(text,ap);

	// Don't report errors if in try-catch block just save them
	if(gInTryBlock == true)
	{
		va_end(ap);
		*GetErrorInfo() = *GetCmdInfo();
		GetErrorInfo()->lastError = output;
		GetErrorInfo()->description = output;
		GetErrorInfo()->type = "Error";
		GetErrorInfo()->errorFound = true;
		gErrorInfo.blocked = true;
		gErrorInfo = *GetErrorInfo();
		gErrorInfo.blocked = false;
		delete [] output;
		return;
	}

	// If we are printing to a file abort
	if(printFile)
	{
		Interface itfc;
		ClosePrintFile(&itfc,"");
	}

	// Make a beep to alert the user
	MessageBeep(MB_OK);

	// Send the error message to a dialog or to the CLI
	if(reportErrorToDialog)  
	{ 
		MessageDialog(prospaWin,MB_ICONERROR,"Error",output);
	}
	else
	{
	//	TextMessage("Thread ID for error is %lX\n", GetCurrentThreadId());
		TextMessage("\n\n   Error: "); 
		TextMessage(output); 
		TextMessage("\n");
	}

	va_end(ap);

	// Note the description of this error
	// and that an error has been found
	GetErrorInfo()->lastError = output;
	GetErrorInfo()->errorFound = true;
	delete [] output;
}


EXPORT void Error(Interface *itfc, char *text,...)
{
	va_list ap;
	char *output; 

	// Don't report errors if not aborting
	if(abortOnError == false)
		return;

	// Don't report errors if we are the processing a user abort
	if(gAbort == true)
		return;


	// If we are printing to a file abort
	if(printFile)
	{
		Interface itfc;
		ClosePrintFile(&itfc,"");
	}

	va_start(ap,text);

	// Extract all arguments and format them correctly
	output =  vssprintf(text,ap);

	// Don't report errors if in try-catch block just save them
	if(gInTryBlock == true)
	{
		va_end(ap);
		*GetErrorInfo() = *GetCmdInfo();
		GetErrorInfo()->lastError = output;
		GetErrorInfo()->type = "Error";
		GetErrorInfo()->description = output;
		GetErrorInfo()->errorFound = true;
		gErrorInfo.blocked = true;
		gErrorInfo = *GetErrorInfo();
		gErrorInfo.blocked = false;
		return;
	}

	// Make a beep to alert the user
	MessageBeep(MB_OK);

	// Send the error message to a dialog or to the CLI
	if(reportErrorToDialog)  
	{ 
		MessageDialog(prospaWin,MB_ICONERROR,"Error",output);
	}
	else
	{
		TextMessage("\n\n   Error: "); 
		TextMessage(output); 
		TextMessage("\n");
	}

	va_end(ap);


	// Note the description of this error
	// and that an error has been found
	GetErrorInfo()->lastError = output;
	GetErrorInfo()->errorFound = true;
}


const char* const GdiErrorDescription(Gdiplus::Status status)
{
	switch(status)
	{
	case GenericError:
		return "Generic error";
	case InvalidParameter:
		return "Invalid parameter";
	case OutOfMemory:
		return "Out of memory";
	case ObjectBusy:
		return "Object busy";
	case InsufficientBuffer:
		return "Insufficient buffer";
	case NotImplemented:
	  return "Not implemented";
	case Win32Error:
		return "Win32 error";
	case WrongState:
		return "Wrong state";
	case Aborted:
		return "Aborted";
	case FileNotFound:
		return "File not found";
	case ValueOverflow:
		return "Value overflow";
	case AccessDenied:
		return "Access denied";
	case UnknownImageFormat:
		return "Unknown image format";
	case FontFamilyNotFound:
		return "Font family not found";
	case FontStyleNotFound:
		return "Font style not found";
	case NotTrueTypeFont:
		return "Not a TrueType font";
	case UnsupportedGdiplusVersion:
		return "Unsupported GDI+ version";
	case GdiplusNotInitialized:
		return "GDI+ not initialised";
	case PropertyNotFound :
		return "Property not found";
	case PropertyNotSupported:
		return "Property not supported";
	default:
		return "Unknown error";
	}
}
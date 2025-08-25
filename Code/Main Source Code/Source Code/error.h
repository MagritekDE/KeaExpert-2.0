#ifndef ERROR_H
#define ERROR_H

#include "defines.h"

class Interface;

extern bool reportErrorToDialog;

EXPORT void ErrorMessage(const char* const text,...);
EXPORT void Error(Interface *itfc, char *text,...);

namespace Gdiplus {
	enum Status;
}
const char* const GdiErrorDescription(Gdiplus::Status status);

#endif // define ERROR_H
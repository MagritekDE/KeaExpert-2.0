#ifndef STRING_UTILITIES_H
#define STRING_UTILITIES_H

#include <string>
#include <utility>
#include <vector>
#include "StringPairs.h"

class Interface;

void AddCharToString(char *str, char c);
int GetLineFromText(Interface* itfc ,char args[]);
void RemoveCharFromString(char *str, char c);
int ReplaceString(Interface* itfc ,char args[]);
int ScanString(Interface* itfc ,char args[]);
int SetStringCase(Interface *itfc, char args[]);

// General Text processing
void ReplaceSpecialCharacters(char*, char* , char*, int);
char* ToLowerCase(char*);
char* ToUpperCase(char*);
void UpperCaseForFirst(char*);
bool FindSubStr(char*, char*, long&);
void ExtractSubStr(char*, char*, long, long);
char*  vssprintf(const char *format, va_list argptr);
wchar_t* CharToWChar(const char* const txt);
int RegexMatch(Interface* itfc, char args[]);
int StrCmpIgnoreCase(char *str1, char *str2);
// If tf == true, returns "true", otherwise returns "false".
const char* const toTrueFalse(bool);
// Turns a primitive number into a string. 
const std::string stringifyFloat(double f);
const std::string stringifyInt(long l);
// Generates a string suitable as a header for the "zero-arg"
// response to a widget/class method.
const std::string formatNoArgHeader();
// Formats the current state of a class/widget's parameter-accessible
// attributes.
const std::string FormatStates(StringPairs&  state);


#endif // define STRING_UTILITIES_H

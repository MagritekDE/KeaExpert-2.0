#ifndef SCANSTRINGS_H
#define SCANSTRINGS_H

#include "defines.h"

class Interface;

short ArgScan(Interface *itfc,char argStr[], short minNr, char prompt[], char dataType[], char argType[], ...);
EXPORT short ArgScan(void *par,char argStr[], short minNr, char prompt[], char dataType[], char argType[], ...);

int RemoveSubString(Interface* itfc ,char args[]);
int ParseString(Interface* itfc ,char args[]);
int ProcessEscapeCharacters(Interface* itfc, char args[]);
int CompareStrings(Interface* itfc, char args[]);
void RemoveQuotes(char[]);
bool isStr(char *str);
void RemoveCharacter(char*, char);
short GetDataType(char str[],float *result);
void StrNCopy(char *out, const char *in, long N);
complex StrtoComplex(char *str);
void stradd(char *in, char *add, short &start);
void AddQuotes(char[]);
void ExtractProcedureName(char*, char*);
void ExtractClassProcName(char *objectName, char *procName);
void StrSubCopy(char*out, char*, long, long);
void SimplifyDoubleEscapes(char[]);
void RemoveEndBlanks(char*);
bool IsEscapedQuote(char* txt, long pos, long len);
int IsSubString(Interface* itfc ,char arg[]);
bool IsUnEscapedQuote(char* txt, long pos, long len);
char* GetWordAtChar(char*,long,long&,long&);
char* EnumToString(short enumType, short enumeration);
long FindSubStr(char *text, char *substr, long start, bool ignoreCase);
long FindReverseSubStr(char *text, char *substr, long start, bool ignoreCase, bool ignoreComment);
void RemoveUnquotedCharacter(char *str, char ign);
void LeftStr(char *input, long end, char *output);
void RightStr(char *input, long start, char *output);
long FindCharacter(char *str, char c);
bool IsWhiteSpaceString(char *s);
bool IsCommentLine(char* s);
short FirstCharInString(char *s, char *chars, char &first);
void ReplaceEscapedCharacters(char str[]);
void ReplaceEscapedQuotes(char str[]);
short StrTrueCheck(char *str, bool &response);
void AddEscapedQuotes(char* out, char* in);
//extern bool processEscapes; // Whether escape character should be processed or not.


#endif // define SCANSTRINGS_H
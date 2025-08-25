#ifndef EDIT_UTILITIES_H
#define EDIT_UTILITIES_H

class TextList;
class Interface;
class EditRegion;

short RemoveCommentsFromText(char *textIn, char **textOut);
void  ReplaceEOLinText(char* txt);
short CountLines(char *txt, short cmdNr);
char *GetText(HWND);
char* GetLineByPosition(HWND, long);
char* GetLineByNumber(HWND, long);
void GetLineByNumber(HWND, long, char*);
long GetCurrentLineNumber(HWND edWin);
long GetLineStartByPosition(HWND, long);
void GetCommandHelp(Interface*, HWND edWin);
void AppendLine(HWND edWin, char *txt);
void AppendLineEx(HWND edWin, char *txt);
void ReplaceLineByPosition(HWND edWin, long pos, char *txt);
void ReplaceLine(HWND edWin, long n, char *txt);
long SelectCurrentLine(HWND edWin);
long SelectCurrentLineEx(HWND edWin);
void InsertLine(HWND edWin, char *txt, long n);
void InsertLineEx(HWND edWin, char *txt, long n);
long GetCurrentLine(HWND edWin, char *txt);
long GetCurrentLineEx(HWND edWin, char *txt);
void RemoveLine(HWND edWin, long n);
void SelectLine(HWND edWin, long n);
void SelectChar(HWND edWin, long ch);
void SelectChar(HWND edWin, long n, long ch);
void SelectCharEx(HWND edWin, long n, long ch);
long CountLinesInEditor(HWND edWin);
void GetEditSelection(HWND hWnd, long &start, long &end);
void SetEditSelection(HWND hWnd, long start, long end);
void ReplaceEditText(HWND hWnd, long start, long end, char* string);
void CutEditSelection(HWND hWnd);
void UpdateLineColor(EditRegion *er, HWND hWnd, long lineNr);
void ColorSelectedText(HWND hWnd, COLORREF color);
short GetNextWord2(long &pos, char *text, char *command, long endLine);
void UpdateLineColorCore(EditRegion *er, HWND hWnd, long lineNr);
void SetColor(HWND hWnd, COLORREF color);
char* ExpandToFullWord(HWND win, char* leftDelimiter, char* rightDelimiter, long &wordStart, long &wordEnd, bool &classCmd, short &userClassCmd);
char* ExpandToFullWord(HWND win, char* leftDelimiter, char* rightDelimiter, long &wordStart, long &wordEnd, bool &classCmd, short &userClassCmd, bool &funcCmd);
long SelectPartCurrentLine(HWND edWin, short begin, short end);
void AddFilenameToList(TextList*,char* path, char* name);
short IsEditFileAlreadyLoaded(EditRegion *curRegion);
short IsEditFileAlreadyLoaded(EditRegion *curRegion, char *filePath, char *fileName);

#endif //define EDIT_UTILITIES_H 
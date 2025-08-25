#ifndef EDIT_FILES_H
#define EDIT_FILES_H

class EditRegion;
class Interface;
class WinData;

class TextList
{
	public:
		TextList();
	//	~TextList();
		void AddText(char*);
		void Remove(void);
		void RemoveFirst(void);
		void RemoveAll(void);
		TextList *GetNextText();
		TextList *GetNthText(long);
		TextList *FindText(char*);
		TextList *FindIText(char*);
		long Count();
		TextList *next;
		TextList *last;
		char *text;
};

extern TextList *procLoadList;
extern TextList *procRunList;

FILE* FindFolder(Interface *itfc, char *currentPath, char *fileName, char *extension);
int GetTextSelection(Interface* itfc, char[]);
void SaveEditList();
short  SaveWinEditSessions(WinData* win);
short  SaveAllEditSessions(void);
short FindProcedure(char *text, char *funcName);
short FindProcedure(char *text, char *funcName, long &lineNr, bool verbose=true);
long FindProcedurePosition(char *text, char *funcName);
short GetNextWord(long &pos, char *text, char *command, long);
short GetNextWord(long &pos, char *text, char *command, long maxLen, long &lineNr);
void LoadEditList(void);
short FindProcedures(char *text, long &i, char **procedure, char *procName, long len, long &lineNr, long &startLine, bool includeComment=false);
long GetFileLength(FILE *fp);
int FindMacro(Interface *itfc, char args[]);
void SetupTextLists(void);
#endif // define EDIT_FILES_H

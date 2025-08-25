#ifndef CLASSFUNCTIONS_H
#define CLASSFUNCTIONS_H

void InitializeWidgetCommandList();
bool IsProcessClassFunction(char name[]);
void GetClassCommandSyntax(char name[], char** syntax);
bool GetClassCommandVariousness(char name[]);
const char* GetClassCommandHelpType(char name[]);

#endif // ifndef CLASSFUNCTIONS_H
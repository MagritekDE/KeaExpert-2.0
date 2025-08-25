#ifndef GRIDCONTROL_H
#define GRIDCONTROL_H
/* Interface to grids. */

class Interface;
class BabyGrid;

// CLI/Macro level
int GridControlSet(Interface* itfc ,char args[]);
int GridControlGet(Interface* itfc ,char args[]);
int GridControlClear(Interface* itfc, char args[]);
int GridControlSetRowCount(Interface* itfc, char args[]);
int GridControlSetColumnCount(Interface* itfc, char args[]);
int GridControlSetColumnWidth(Interface* itfc, char args[]);
int GridControlSetRowHeight(Interface* itfc, char args[]);
int GridControlSetTitleHeight(Interface* itfc, char args[]);
int GridControlSetColumnHeaderHeight(Interface* itfc, char args[]);
int GridControlSetRowHeaderWidth(Interface* itfc, char args[]);
int GridControlShowRowHeaders(Interface* itfc, char args[]);
int GridControlShowRowLabels(Interface* itfc, char args[]);
int GridControlShowColumnHeaders(Interface* itfc, char args[]);
int GridControlShowColumnLabels(Interface* itfc, char args[]);
int GridControlProtect(Interface* itfc, char args[]);
int GridControlProtectColumn(BabyGrid* grid, int col, bool protect, Interface* itfc);
int GridControlProtectRow(BabyGrid* grid, int col, bool protect, Interface* itfc);
int GridControlSetLabel(Interface* itfc, char args[]);
int GridControlGetColumn(BabyGrid* grid, int row, Interface* itfc);
int GridControlGetRow(BabyGrid* grid, int row, Interface* itfc);

#endif //define GRIDCONTROL_H
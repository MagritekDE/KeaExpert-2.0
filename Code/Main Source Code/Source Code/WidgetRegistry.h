#ifndef WIDGETREGISTRY_H
#define WIDGETREGISTRY_H

#include <deque>

class EditParent;
class ObjectData;

typedef std::deque<ObjectData*> WidgetList;
typedef WidgetList::iterator WidgetListIterator;
typedef WidgetList::reverse_iterator WidgetListReverseIterator;

class WidgetRegistry
{
public:
	~WidgetRegistry();
	
	ObjectData* add(ObjectData* o);
	int size();

	int countSelectedObjects();
	void resetControlNumbers();
	void resetTabNumbers();
	void selectAllWithinRect(short x1, short y1, short x2, short y2);
	short updateControlVisibilityFromTabs();

	ObjectData* findByScreenPosition(POINT coord);
	ObjectData* findByNr(short nr);
	ObjectData* findByMenu(HMENU menu);
	ObjectData* findBySequentialNr(short nr);
	ObjectData* findNextByNumber(ObjectData* obj);
	ObjectData* findPrevByNumber(ObjectData* obj);
	ObjectData* findNextByTabNumber(ObjectData* obj);
	ObjectData* findPrevByTabNumber(ObjectData* obj);
	ObjectData* findTabbedNextByTabNumber(ObjectData* obj);
	ObjectData* findTabbedPrevByTabNumber(ObjectData* obj);

	ObjectData* findByWin(HWND win);
	ObjectData* findCurEditor(HWND hWnd, EditParent **ep, short *curEdNr);
	ObjectData* findByType(short type);
	ObjectData* findByValueID(const char* const name);
	ObjectData* findByObjectID(const char* const name);
	ObjectData* findSelected();

	char* getCommand(short id);
	
	WidgetList* getAllOfType(short type);
	void selectAll(bool sel);

	bool empty();
	void clear();
	void destroyAllWidgets();
	void remove(ObjectData* o);

	WidgetList& getWidgets();
	WidgetList& getWidgetsInCtrlNrOrder();
	void sort(); // Sorts tabOrderWidgets and ctrlNumOrderWidgets.

private:
	WidgetList widgets; 
	WidgetList tabOrderWidgets;
	WidgetList ctrlNumOrderWidgets;
	static bool compareTabNumbers(ObjectData* o1, ObjectData* o2);
	void regenerateTabSequence();
	ObjectData* findNext(ObjectData* obj, WidgetList& wlist);
	ObjectData* findPrev(ObjectData* obj, WidgetList& wlist);
	ObjectData* findNext(ObjectData* obj);
	ObjectData* findPrev(ObjectData* obj);
};

#endif //#define WIDGETREGISTRY_H

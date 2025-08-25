#include "stdafx.h"
#include "WidgetRegistry.h"
#include "defines.h"
#include "edit_class.h"
#include "globals.h"
#include "guiObjectClass.h"
#include "myBoost.h"
#include <algorithm>
#include <functional>
#include "hr_time.h"
#include "memoryLeak.h"

using namespace std;

WidgetRegistry::~WidgetRegistry()
{
	destroyAllWidgets();
}

void WidgetRegistry::destroyAllWidgets()
{
	WidgetList temp;
	while (!widgets.empty())
	{
		WidgetListIterator it =  widgets.begin();
		ObjectData* victim = *it;
		widgets.erase(it);
     // printf("Deleting object nr %d\n",nr);
		delete victim;
     // printf("Deleted object nr %d\n",nr);
	}
	tabOrderWidgets.clear();
	ctrlNumOrderWidgets.clear();
}

ObjectData* WidgetRegistry::add(ObjectData* o)
{
	widgets.push_front(o);
	tabOrderWidgets.push_back(o);
	ctrlNumOrderWidgets.push_back(o);
	return o;
}


int WidgetRegistry::size()
{
	return (int)widgets.size();
}

void WidgetRegistry::remove(ObjectData* o)
{
	WidgetListIterator wit = find(widgets.begin(), widgets.end(), o);
	if (wit == widgets.end())
	{
		return;
	}
	widgets.erase(wit);
	wit = find(tabOrderWidgets.begin(), tabOrderWidgets.end(), o);
	if (wit != tabOrderWidgets.end())
		tabOrderWidgets.erase(wit);
	wit = find(ctrlNumOrderWidgets.begin(), ctrlNumOrderWidgets.end(), o);
	ctrlNumOrderWidgets.erase(wit);
}

// Find the visible object which contains the coordinate pos

ObjectData* WidgetRegistry::findByScreenPosition(POINT pos)
{
	ObjectData *obj;
	RECT r;
	for (int i = 0; i < widgets.size(); i++)
	{
		obj = widgets[i];
	   GetWindowRect(obj->hWnd, &r);
		if(obj->visible && pos.x > r.left && pos.x < r.right && pos.y > r.top && pos.y < r.bottom)
			return(obj);
	}
	return(NULL);
}

ObjectData* WidgetRegistry::findByNr(short nr)
{
   

	WidgetListIterator found = find_if(widgets.begin(),widgets.end(),bind2nd(mem_fun(&ObjectData::nrEquals), nr));
//	WidgetListIterator found = find_if(widgets.begin(),widgets.end(),[nr](ObjectData* obj){ return obj->nr() == nr;});
	return (found == widgets.end()) ? 0 : *found;
}

ObjectData* WidgetRegistry::findByMenu(HMENU menu)
{
	WidgetListIterator found = find_if(widgets.begin(),widgets.end(),bind2nd(mem_fun(&ObjectData::menuNrEquals),menu));
	return (found == widgets.end()) ? 0 : *found;
}

ObjectData* WidgetRegistry::findBySequentialNr(short objNr)
{
	WidgetListIterator found = find_if(widgets.begin(),widgets.end(),bind2nd(mem_fun(&ObjectData::sequentialMenuNrEquals),objNr));
	return (found == widgets.end()) ? 0 : *found;
}

ObjectData* WidgetRegistry::findByValueID(const char* const name)
{
	WidgetListIterator found = find_if(widgets.begin(),widgets.end(),bind2nd(mem_fun(&ObjectData::valueNameEquals),name));
	return (found == widgets.end()) ? 0 : *found;
}

ObjectData* WidgetRegistry::findByObjectID(const char* const name)
{
	WidgetListIterator found = find_if(widgets.begin(),widgets.end(),bind2nd(mem_fun(&ObjectData::objectNameEquals),name));
	return (found == widgets.end()) ? 0 : *found;
}

ObjectData* WidgetRegistry::findByType(short type)
{
	WidgetListIterator found = find_if(widgets.begin(),widgets.end(),bind2nd(mem_fun(&ObjectData::typeEquals),type));
	return (found == widgets.end()) ? 0 : *found;
}

ObjectData* WidgetRegistry::findNext(ObjectData* obj)
{
	WidgetListIterator found = find_if(widgets.begin(),widgets.end(),bind2nd(equal_to<ObjectData*>(),obj));
	// If not found, or if obj is the last in the list, return 0.
	if ((found == widgets.end()) || found + 1 == widgets.end())
	{
		return 0;
	}
	return *(++found);
}

ObjectData* WidgetRegistry::findPrev(ObjectData* obj)
{
	WidgetListReverseIterator found = find_if(widgets.rbegin(), widgets.rend(),bind2nd(equal_to<ObjectData*>(),obj));
	// If not found, or if obj is the first in the list, return 0.
	if ((found == widgets.rend()) || found + 1 == widgets.rend())
	{
		return 0;
	}
	return *(++found);
}

ObjectData* WidgetRegistry::findSelected()
{
	WidgetListIterator found = find_if(widgets.begin(),widgets.end(),mem_fun(&ObjectData::isSelected));
	return (found == widgets.end()) ? 0 : *found;
}


ObjectData* WidgetRegistry::findByWin(HWND win)
{
	WidgetListIterator found = find_if(widgets.begin(),widgets.end(),bind2nd(mem_fun(&ObjectData::winEquals),win));
	return (found == widgets.end()) ? 0 : *found;
}


ObjectData* WidgetRegistry::findCurEditor(HWND hWnd, EditParent **ep, short *curEdNr)
{
   *ep = NULL;

	for(ObjectData* o: widgets)
   {
      if(o->type == TEXTEDITOR)
      {
         if(o->data)
         {
            *ep = (EditParent*)o->data;
            for(short i = 0; i < (*ep)->rows*(*ep)->cols; i++)
            {
               if((*ep)->editData && (*ep)->editData[i] && (*ep)->editData[i]->edWin == hWnd)
               {
                  *curEdNr = i;
                  return(o);
               }
            }
         }
      }
   }
   *curEdNr = -1;
   return(0);
}



ObjectData* WidgetRegistry::findNext(ObjectData* obj, WidgetList& wlist)
{
	bool found = false;
	ObjectData* next = 0;

	if (wlist.empty())
		return obj;

	for(ObjectData* o: wlist)
	{
		if (found && o->IsTabbable())
		{
			next = o;
			break;
		}
		if (o == obj)
		{
			found = true;
		}
	}
	if (found && next)
	{
		// Then we found the next widget.
		return next;
	}
	else if (found && !next)
	{
		// Then we found the requested widget, but
		// fell off the end of the list. We need to 
		// wrap (ie, return the first item in the list.
		return wlist[0];
	}
	return 0;
}

ObjectData* WidgetRegistry::findPrev(ObjectData* obj, WidgetList& wlist)
{
	ObjectData* prev = 0;
	bool found = false;
	
	reverse_foreach(ObjectData* o, wlist)
	{
		if (found && o->IsTabbable())
		{
			prev = o;
			break;
		}
		if (o == obj)
		{
			found = true;
		}
	}
	if (found && prev)
	{
		// Then we found the next widget.
		return prev;
	}
	else if (found && !prev)
	{
		// Then we found the requested widget, but
		// fell off the beginning of the list. We need to 
		// wrap (ie, return the last item in the list.
		return wlist[wlist.size() - 1];
	}
	return 0;
}

ObjectData* WidgetRegistry::findNextByNumber(ObjectData* obj)
{
	sort();
	return findNext(obj, ctrlNumOrderWidgets);
}

ObjectData* WidgetRegistry::findPrevByNumber(ObjectData* obj)
{
	sort();
	return findPrev(obj, ctrlNumOrderWidgets);
}


ObjectData* WidgetRegistry::findTabbedNextByTabNumber(ObjectData* obj)
{
	WidgetList peers;

	for(ObjectData* peer: tabOrderWidgets)
	{
		if (peer->visible && peer->IsTabbable())
		{
			peers.push_back(peer);
		}
	}
	return findNext(obj, peers);
}

ObjectData* WidgetRegistry::findTabbedPrevByTabNumber(ObjectData* obj)
{
	WidgetList peers;

	for(ObjectData* peer: tabOrderWidgets)
	{
		if (peer->visible && peer->IsTabbable())
			peers.push_front(peer);
	}
	return findNext(obj, peers);
}

ObjectData* WidgetRegistry::findNextByTabNumber(ObjectData* obj)
{
	sort();

	if (tabOrderWidgets.empty())
		return 0;
	else
		return findTabbedNextByTabNumber(obj);
}

ObjectData* WidgetRegistry::findPrevByTabNumber(ObjectData* obj)
{
	sort();
	
	if (tabOrderWidgets.empty())
		return 0;
	else
		return findTabbedPrevByTabNumber(obj);
}

void WidgetRegistry::clear()
{
	tabOrderWidgets.clear();
	ctrlNumOrderWidgets.clear();
	widgets.clear();
}


bool WidgetRegistry::compareTabNumbers(ObjectData* o1, ObjectData* o2)
{
	// Is one the tab parent of the other?
	if (o2->isTabChildOf(o1))
		return true;
	if (o1->isTabChildOf(o2))
		return false;

	// Are they children of the same parent?
	if (o1->isTabChildOf(o2->tabParent))
	{
		return o1->tabNr < o2->tabNr;
	}

	// Are they chldren of different parents?
	if (o1->isTabChild() && o2->isTabChild() && o1->tabParent != o2->tabParent)
		return o1->tabParent->tabNr < o2->tabParent->tabNr;

	// If one is a parent and the other the child of a different parent,
	// the parent comes first if its tabNr < the child's parent's tabNr.
	if ((o1->isTabParent() && !o2->isTabParent() && o2->tabParent))
	{
		return o1->tabNr < o2->tabParent->tabNr;
	}
	if ((o2->isTabParent() && !o1->isTabParent() && o1->tabParent))
	{
		return o1->tabParent->tabNr < o2->tabNr;
	}
	return (o1->tabNr < o2->tabNr);
}

void WidgetRegistry::regenerateTabSequence()
{
	tabOrderWidgets.clear();
	for(ObjectData* obj: widgets)
	{
		if (obj->tabNr != NO_TAB_NUMBER && obj->IsTabbable())
			tabOrderWidgets.push_back(obj);
	}
}

void WidgetRegistry::sort()
{
	regenerateTabSequence();
	std::sort(tabOrderWidgets.begin(), tabOrderWidgets.end(), compareTabNumbers);
	std::sort(ctrlNumOrderWidgets.begin(), ctrlNumOrderWidgets.end(), ObjectData::compareCtrlNumbers);
}


WidgetList* WidgetRegistry::getAllOfType(short type)
{
	WidgetList* returnMe = new WidgetList;
	if (!returnMe)
	{
		return 0;
	}
	
	for(ObjectData* o: widgets)
	{
		if (o->type == type)
		{
			returnMe->push_back(o);
		}
	}
	return returnMe;
}

WidgetList& WidgetRegistry::getWidgets()
{
	return widgets;
}

WidgetList& WidgetRegistry::getWidgetsInCtrlNrOrder()
{
	return ctrlNumOrderWidgets;
}


bool WidgetRegistry::empty()
{
	return widgets.empty();
}

char* WidgetRegistry::getCommand(short id)
{
	ObjectData* obj = this->findByNr(id);
	return obj ? obj->command : 0;
}

int WidgetRegistry::countSelectedObjects()
{
	return count_if(widgets.begin(),widgets.end(),mem_fun(&ObjectData::isSelected));
}

void WidgetRegistry::selectAll(bool sel)
{
	for_each(widgets.begin(),widgets.end(),bind2nd(mem_fun(&ObjectData::setSelected),sel));
}

void WidgetRegistry::resetControlNumbers()
{
	for_each(widgets.begin(),widgets.end(),mem_fun(&ObjectData::resetControlNumber));
	sort();
}

void WidgetRegistry::resetTabNumbers()
{
	for_each(widgets.begin(),widgets.end(),mem_fun(&ObjectData::resetTabNumber));
	sort();
}

void WidgetRegistry::selectAllWithinRect(short x1, short y1, short x2, short y2)
{
	FloatRect rect; rect.left = x1; rect.right = x2; rect.top = y1; rect.bottom = y2;
	for_each(widgets.begin(), widgets.end(),bind2nd(mem_fun(&ObjectData::selectIfWithinRect),&rect));
}

short WidgetRegistry::updateControlVisibilityFromTabs()
{
	for_each(widgets.begin(),widgets.end(),mem_fun(&ObjectData::updateVisibilityWRTTabs));
	return OK;
}
#include "stdafx.h"
#include "Observer.h"
#include <algorithm>
#include "memoryLeak.h"

using std::find;

void Observable::notifyObservers()
{
	for(Observer* o: observers)
	{
		o->notify(this);
	}	
}

void Observable::registerObserver(Observer* o)
{
	observers.push_back(o);
}

void Observable::unregisterObserver(Observer* o)
{
	ObservingCollectionIterator it = 
		find(observers.begin(), observers.end(), o);
	
	if (it != observers.end())
	{
		observers.erase(it);
	}
}
#ifndef OBSERVER_H
#define OBSERVER_H

#include <deque>

class Observer;
class Observable;

typedef std::deque<Observer*> ObservingCollection;
typedef std::deque<Observer*>::iterator ObservingCollectionIterator;

class Observer
{
public:
	virtual void notify(Observable* fromWhom) = 0;
};

class Observable
{
public:
	void registerObserver(Observer* o);
	void unregisterObserver(Observer* o);
	void notifyObservers();
protected:
	ObservingCollection observers;
};

#endif // ifndef OBSERVER_H
#include "stdafx.h"
#include "StringPairs.h"
#include <vector>
#include "memoryLeak.h"

using std::vector;
using std::string;

/************************************************************************
* StringPairs
*
* Stores iterable pairs of strings. Could be generalized but no reason to
* do so now...
*
* (c) Magritek 2011
*************************************************************************/

/************************************************************************
* Pair
*
* A pair of strings... used in preference to std::pair 
************************************************************************/
StringPair::StringPair(const char* const first, const char* const second)
{
	this->f = new std::string(first);
	this->s = new std::string(second);
}

StringPair::StringPair(const string& first, const string& second)
{
	this->f = new std::string(first);
	this->s = new std::string(second);
}

StringPair::~StringPair()
{
	delete f;
	delete s;
}

const std::string* const StringPair::first()
{
	return f;
}

const std::string* const StringPair::second()
{
	return s;
}

/************************************************************************
StringPairs

Stores iterable pairs of strings. Could be generalized but no reason to
do so now...
************************************************************************/

/************************************************************************
Deletes the pair and its two members.
************************************************************************/
StringPairs::~StringPairs()
{
	vector<StringPair*>::iterator it;
	for(it = sPV.begin();it < sPV.end();++it)
	{
		delete (*it);
	}
}

/************************************************************************
Add a pair of strings. These strings are kept as independent
copies that are the responsibility of this StringPair.
************************************************************************/
void StringPairs::add(const char* const a, const char* const b)
{
	sPV.push_back(new StringPair(a,b));
}

void StringPairs::add (const string& a, const string& b)
{
	sPV.push_back(new StringPair(a,b));
}
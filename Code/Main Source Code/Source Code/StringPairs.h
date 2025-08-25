#ifndef STRINGPAIRS_H
#define STRINGPAIRS_H

#include <string>
#include <vector>

/************************************************************************
* StringPairs
*
* Stores iterable pairs of strings. Could be generalized but no reason to
* do so now...
*
* (c) Magritek 2011
*************************************************************************/

/************************************************************************
StringPair                                                                   

Element stored in a StringPairs.
************************************************************************/
class StringPair
{
public:
	~StringPair();
	StringPair(const char* const, const char* const);
	StringPair(const std::string&, const std::string&);
	const std::string* const first();
	const std::string* const second();
private:
	std::string* f;
	std::string* s;
};

/************************************************************************
StringPairs

Stores iterable pairs of strings. Could be generalized but no reason to
do so now...
************************************************************************/

class StringPairs {
public:
	// Deletes the pair and its two members.
	~StringPairs();
	
	// Add a pair of strings. These strings are kept as independent
	// copies that are the responsibility of this StringPair.
	void add (const char* const, const char* const);
	void add (const std::string&, const std::string&);

	// Iterator functionality, to iterate over the contained pairs.
	typedef std::vector<StringPair*>::iterator iterator;
	typedef std::vector<StringPair*>::const_iterator const_iterator;
	iterator begin() { return sPV.begin(); }
	const_iterator begin() const { return sPV.begin(); }
	iterator end() { return sPV.end(); }
	const_iterator end() const { return sPV.end(); }

private:
   std::vector<StringPair*> sPV;
};


#endif // #define STRINGPAIRS_H
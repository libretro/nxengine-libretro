
#ifndef _STRINGLIST_H
#define _STRINGLIST_H

#include "BList.h"

class StringList : protected BList
{
public:
	StringList() { }
	
	StringList(const StringList &other)
	{
		*this = other;
	}
	
	virtual ~StringList();
	
	void AddString(const char *str);
	char *StringAt(int index) const;
	void MakeEmpty();
	
	void SwapItems(int index1, int index2);
	
	int32_t CountItems() const { return BList::CountItems(); }
	
	StringList &operator= (const StringList &other);
	bool operator== (const StringList &other) const;
	bool operator!= (const StringList &other) const;
};



#endif

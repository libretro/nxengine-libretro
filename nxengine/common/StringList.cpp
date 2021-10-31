
#include <stdlib.h>
#include <string.h>

#include "StringList.h"
#include "StringList.fdh"

#include "../nx_logger.h"

#ifdef _WIN32
#include "../libretro/msvc_compat.h"
#endif

StringList::~StringList()
{
	MakeEmpty();
}

void StringList::AddString(const char *str)
{
   BList::AddItem(strdup(str));
}

/*
void c------------------------------() {}
*/

void StringList::SwapItems(int index1, int index2)
{
	BList::SwapItems(index1, index2);
}

char *StringList::StringAt(int index) const
{
	return (char *)BList::ItemAt(index);
}

void StringList::MakeEmpty()
{
	int i, count = CountItems();
	for(i=0;i<count;i++)
		free(ItemAt(i));
	
	BList::MakeEmpty();
}

StringList &StringList::operator= (const StringList &other)
{
	StringList::MakeEmpty();
	
	for(int i=0;;i++)
	{
		char *str = other.StringAt(i);
		if (!str)
         break;
		
		AddString(str);
	}
	
	return *this;
}

bool StringList::operator== (const StringList &other) const
{
	if (CountItems() != other.CountItems())
		return false;
	
	for(int i=0;;i++)
	{
		char *str1 = StringAt(i);
		char *str2 = other.StringAt(i);
		
		if (!str1 || !str2)
			return (!str1 && !str2);
		
		if (strcmp(str1, str2) != 0)
			return false;
	}
}

bool StringList::operator!= (const StringList &other) const
{
	return !(*this == other);
}

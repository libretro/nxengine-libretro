
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

/*
void c------------------------------() {}
*/

void StringList::Shuffle()
{
   int i, count = CountItems();

   for(i=0;i<count;i++)
   {
      int swap = random(0, count - 1);
      if (swap != i)
         SwapItems(i, swap);
   }
}

bool StringList::ContainsString(const char *term)
{
   int i;
   char *str;

   for(i=0; (str = StringAt(i)); i++)
   {
      if (!strcmp(str, term))
         return true;
   }

   return false;
}

bool StringList::ContainsCaseString(const char *term)
{
   int i;
   char *str;

   for(i=0; (str = StringAt(i)); i++)
   {
      if (!strcasecmp(str, term))
         return true;
   }

   return false;
}

/*
void c------------------------------() {}
*/

void StringList::AddString(const char *str)
{
   BList::AddItem(strdup(str));
}

bool StringList::SetString(int index, const char *newstring)
{
   int copylen;
   char *str = StringAt(index);
   if (!str)
      return 1;
   if (str == newstring)
      return 0;

   copylen = strlen(newstring) + 1;
   str     = (char *)realloc(str, copylen);
   memcpy(str, newstring, copylen);

   BList::ReplaceItem(index, str);
   return 0;
}

void StringList::RemoveString(int index)
{
	char *str = StringAt(index);
	if (str)
	{
		BList::RemoveItem(index);
		free(str);
	}
}

void StringList::RemoveString(const char *str)
{
   int i;
   char *entry;

   for(i=0; (entry = StringAt(i)); i++)
   {
      if (!strcmp(entry, str))
      {
         BList::RemoveItem(i);
         free(entry);
         i--;
      }
   }
}

void StringList::RemoveIString(const char *str)
{
   int i;
   char *entry;

   for(i=0; (entry = StringAt(i)); i++)
   {
      if (!strcasecmp(entry, str))
      {
         BList::RemoveItem(i);
         free(entry);
         i--;
      }
   }
}

/*
void c------------------------------() {}
*/

void StringList::SwapItems(int index1, int index2)
{
	BList::SwapItems(index1, index2);
}

void StringList::DumpContents()
{
	int i, count = CountItems();

#ifdef DEBUG
	NX_LOG("StringList %08x; %d entries\n", this, count);
#endif
	for(i=0;i<count;i++)
	{
		char *str = StringAt(i);
#ifdef DEBUG
		NX_LOG("(%d) <%08x>: '%s'\n", i, str, str ? str : "(null)");
#endif
	}
}

/*
void c------------------------------() {}
*/

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

/*
void c------------------------------() {}
*/

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



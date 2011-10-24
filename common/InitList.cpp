
#include "InitList.h"
#include "InitList.fdh"

void InitList::AddFunction(void (*func)(void))
{
	AddFunction((void *)func);
}

void InitList::AddFunction(bool (*func)(void))
{
	AddFunction((void *)func);
}

void InitList::AddFunction(void *func)
{
	//stat("AddFunction (void)%08x [%d]", func, fCount);
	if (fCount >= MAX_INIT_RECORDS)
	{
		fCount++;
		return;
	}
	
	InitRecord *init = new InitRecord;
	{
		init->func = (void *)func;
		init->returns_value = false;
	}
	
	fFunctions[fCount++] = init;
}

InitList::~InitList()
{
	for(int i=0;i<fCount;i++)
		delete fFunctions[i];
}

/*
void c------------------------------() {}
*/

bool InitList::CallFunctions()
{
	if (fCount > MAX_INIT_RECORDS)
	{
		stat("InitList::CallFunctions(%08x): too many initilizers", this);
		return 1;
	}
	
	stat("InitList::CallFunctions(%08x): executing %d functions...", this, fCount);
	
	for(int i=0;i<fCount;i++)
	{
		if (fFunctions[i]->returns_value)
		{
			bool (*func)(void) = (bool (*)())fFunctions[i]->func;
			if ((*func)())
				return 1;
		}
		else
		{
			void (*func)(void) = (void (*)())fFunctions[i]->func;
			(*func)();
		}
	}
	
	return 0;
}

/*
void c------------------------------() {}
*/

InitAdder::InitAdder(InitList *initlist, void (*func)(void))
{
	initlist->AddFunction(func);
}

InitAdder::InitAdder(InitList *initlist, bool (*func)(void))
{
	initlist->AddFunction(func);
}

InitAdder::InitAdder(InitList &initlist, void (*func)(void))
{
	initlist.AddFunction(func);
}

InitAdder::InitAdder(InitList &initlist, bool (*func)(void))
{
	initlist.AddFunction(func);
}







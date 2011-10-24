
#ifndef _INITLIST_H
#define _INITLIST_H

#include "BList.h"
#define MAX_INIT_RECORDS		10000

struct InitRecord
{
	void *func;
	bool returns_value;
};

class InitList
{
public:
	virtual ~InitList();
	
	void AddFunction(void (*func)(void));
	void AddFunction(bool (*func)(void));
	void AddFunction(void *func);
	bool CallFunctions();
	
private:
	InitRecord *fFunctions[MAX_INIT_RECORDS];
	int fCount;		// counting on behavior of auto-initilization to 0
};

class InitAdder
{
public:
	InitAdder(InitList *initlist, void (*func)(void));
	InitAdder(InitList *initlist, bool (*func)(void));
	InitAdder(InitList &initlist, void (*func)(void));
	InitAdder(InitList &initlist, bool (*func)(void));
};

#define INITFUNC(TARGET)	\
	static void __InitFunc(void);	\
	static InitAdder _ia(TARGET, __InitFunc);	\
	static void __InitFunc(void)	\

#endif

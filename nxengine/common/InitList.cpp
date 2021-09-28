
// this is a combination of some C++ and preprocessor magic which allows a
// group of initilization functions to be declared across many different
// source files and then at the appropriate time all can be called at once.
// The "trick" is that it automatically works for all modules linked to the
// program so you don't have to keep a handmade list anywhere of the names
// of the initilization functions. This is used by the AI functions to initilize
// all the function pointers etc for the various creatures.
#include "InitList.h"
#include "../nx.h"

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
	if (fCount >= MAX_INIT_RECORDS)
		return;
	
	fFunctions[fCount++] = (void *)func;
}

bool InitList::CallFunctions()
{
   int i;

   if (fCount >= MAX_INIT_RECORDS)
      return 1;

   for(i=0;i<fCount;i++)
   {
      void (*func)(void) = (void (*)())fFunctions[i];
      (*func)();
   }

   return 0;
}

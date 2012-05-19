
#include <SDL.h>
#include <stdio.h>
#include "config.h"
#include "platform.fdh"

FILE *fileopen(const char *fname, const char *mode)
{
	return fopen(fname, mode);
}

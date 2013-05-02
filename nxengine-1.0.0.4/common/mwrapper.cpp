#include <stdio.h>
#include <stdint.h>
#include "mwrapper.h"

int mgetc(char **fp)
{
   unsigned char volatile *f = *((unsigned char volatile **)fp);
   unsigned char volatile c = *f;
   (*fp)++;
   return c;
}

uint16_t mgeti(char **fp)
{
   uint16_t a, b;
	a = mgetc(fp);
	b = mgetc(fp);
	return (b << 8) | a;
}

uint32_t mgetl(char **fp)
{
   uint32_t a, b, c, d;
	a = mgetc(fp);
	b = mgetc(fp);
	c = mgetc(fp);
	d = mgetc(fp);
	return (d<<24)|(c<<16)|(b<<8)|(a);
}


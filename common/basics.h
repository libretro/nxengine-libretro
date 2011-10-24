
#ifndef _BASICS_H
#define _BASICS_H

#include <stdint.h>
#include <sys/param.h>	// MAXPATHLEN
typedef unsigned char		uchar;

void stat(const char *fmt, ...);
void staterr(const char *fmt, ...);
#define ASSERT(X)	\
{	\
	if (!(X))	\
	{	\
		staterr("** ASSERTION FAILED: '%s' at %s(%d)", #X, __FILE__, __LINE__);	\
	}	\
}

#define SWAP(A, B)	{ A ^= B; B ^= A; A ^= B; }

#ifndef MIN
#define MIN(A, B)	( ( (A) < (B) ) ? (A) : (B) )
#endif

#ifndef MAX
#define MAX(A, B)	( ( (A) > (B) ) ? (A) : (B) )
#endif

#endif

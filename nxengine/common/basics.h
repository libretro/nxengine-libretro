
#ifndef _BASICS_H
#define _BASICS_H

#include <stdint.h>
#include <limits.h>

#define MAXPATHLEN	256

#ifndef PATH_MAX
#define PATH_MAX	4096
#endif

typedef unsigned char		uchar;

#define ASSERT(X)	\
{	\
	if (!(X))	\
	{	\
		NX_ERR("** ASSERT FAILED: '%s' at %s(%d)\n", #X, __FILE__, __LINE__);	\
		exit(1); 	\
	}	\
}

#define SWAP(A, B)	{ A ^= B; B ^= A; A ^= B; }

#ifndef MIN
#define MIN(A, B)	( ( (A) < (B) ) ? (A) : (B) )
#endif

#ifndef MAX
#define MAX(A, B)	( ( (A) > (B) ) ? (A) : (B) )
#endif

#ifdef SINGLE_PRECISION_FLOATS
typedef float float_type;
#else
typedef double float_type;
#endif

#endif

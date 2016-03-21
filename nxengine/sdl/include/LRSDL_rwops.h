/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2012 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/

/** @file SDL_rwops.h
 *  This file provides a general interface for SDL to read and write
 *  data sources.  It can easily be extended to files, memory, etc.
 */

#ifndef _LRSDL_rwops_h
#define _LRSDL_rwops_h

#include "LRSDL_stdinc.h"
#include "LRSDL_error.h"

#include "begin_code.h"
/* Set up for C function definitions, even when using C++ */
#ifdef __cplusplus
extern "C" {
#endif

/** This is the read/write operation structure -- very basic */

typedef struct LRSDL_RWops {
	/** Seek to 'offset' relative to whence, one of stdio's whence values:
	 *	SEEK_SET, SEEK_CUR, SEEK_END
	 *  Returns the final offset in the data source.
	 */
	int (SDLCALL *seek)(struct LRSDL_RWops *context, int offset, int whence);

	/** Read up to 'maxnum' objects each of size 'size' from the data
	 *  source to the area pointed at by 'ptr'.
	 *  Returns the number of objects read, or -1 if the read failed.
	 */
	int (SDLCALL *read)(struct LRSDL_RWops *context, void *ptr, int size, int maxnum);

	/** Write exactly 'num' objects each of size 'objsize' from the area
	 *  pointed at by 'ptr' to data source.
	 *  Returns 'num', or -1 if the write failed.
	 */
	int (SDLCALL *write)(struct LRSDL_RWops *context, const void *ptr, int size, int num);

	/** Close and free an allocated SDL_FSops structure */
	int (SDLCALL *close)(struct LRSDL_RWops *context);

	Uint32 type;
	union {
#if defined(__WIN32__) && !defined(__SYMBIAN32__)
	    struct {
		int   append;
		void *h;
		struct {
		    void *data;
		    int size;
		    int left;
		} buffer;
	    } win32io;
#endif
#ifdef HAVE_STDIO_H 
	    struct {
		int autoclose;
	 	FILE *fp;
	    } stdio;
#endif
	    struct {
		Uint8 *base;
	 	Uint8 *here;
		Uint8 *stop;
	    } mem;
	    struct {
		void *data1;
	    } unknown;
	} hidden;

} LRSDL_RWops;


/** @name Functions to create LRSDL_RWops structures from various data sources */
/*@{*/

extern DECLSPEC LRSDL_RWops * SDLCALL LRSDL_RWFromFile(const char *file, const char *mode);

#ifdef HAVE_STDIO_H
extern DECLSPEC LRSDL_RWops * SDLCALL LRSDL_RWFromFP(FILE *fp, int autoclose);
#endif

extern DECLSPEC LRSDL_RWops * SDLCALL LRSDL_RWFromMem(void *mem, int size);
extern DECLSPEC LRSDL_RWops * SDLCALL LRSDL_RWFromConstMem(const void *mem, int size);

extern DECLSPEC LRSDL_RWops * SDLCALL LRSDL_AllocRW(void);
extern DECLSPEC void SDLCALL LRSDL_FreeRW(LRSDL_RWops *area);

/*@}*/

/** @name Seek Reference Points */
/*@{*/
#define RW_SEEK_SET	0	/**< Seek from the beginning of data */
#define RW_SEEK_CUR	1	/**< Seek relative to current read point */
#define RW_SEEK_END	2	/**< Seek relative to the end of data */
/*@}*/

/** @name Macros to easily read and write from an LRSDL_RWops structure */
/*@{*/
#define LRSDL_RWseek(ctx, offset, whence)	(ctx)->seek(ctx, offset, whence)
#define LRSDL_RWtell(ctx)			(ctx)->seek(ctx, 0, RW_SEEK_CUR)
#define LRSDL_RWread(ctx, ptr, size, n)	(ctx)->read(ctx, ptr, size, n)
#define LRSDL_RWwrite(ctx, ptr, size, n)	(ctx)->write(ctx, ptr, size, n)
#define LRSDL_RWclose(ctx)		(ctx)->close(ctx)
/*@}*/

/** @name Read an item of the specified endianness and return in native format */
/*@{*/
extern DECLSPEC Uint16 SDLCALL LRSDL_ReadLE16(LRSDL_RWops *src);
extern DECLSPEC Uint16 SDLCALL LRSDL_ReadBE16(LRSDL_RWops *src);
extern DECLSPEC Uint32 SDLCALL LRSDL_ReadLE32(LRSDL_RWops *src);
extern DECLSPEC Uint32 SDLCALL LRSDL_ReadBE32(LRSDL_RWops *src);
extern DECLSPEC Uint64 SDLCALL LRSDL_ReadLE64(LRSDL_RWops *src);
extern DECLSPEC Uint64 SDLCALL LRSDL_ReadBE64(LRSDL_RWops *src);
/*@}*/

/** @name Write an item of native format to the specified endianness */
/*@{*/
extern DECLSPEC int SDLCALL LRSDL_WriteLE16(LRSDL_RWops *dst, Uint16 value);
extern DECLSPEC int SDLCALL LRSDL_WriteBE16(LRSDL_RWops *dst, Uint16 value);
extern DECLSPEC int SDLCALL LRSDL_WriteLE32(LRSDL_RWops *dst, Uint32 value);
extern DECLSPEC int SDLCALL LRSDL_WriteBE32(LRSDL_RWops *dst, Uint32 value);
extern DECLSPEC int SDLCALL LRSDL_WriteLE64(LRSDL_RWops *dst, Uint64 value);
extern DECLSPEC int SDLCALL LRSDL_WriteBE64(LRSDL_RWops *dst, Uint64 value);
/*@}*/

/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif
#include "close_code.h"

#endif /* _SDL_rwops_h */

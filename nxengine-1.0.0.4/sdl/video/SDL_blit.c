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
#include "SDL_config.h"

#include "SDL_video.h"
#include "SDL_sysvideo.h"
#include "SDL_blit.h"
#include "SDL_pixels_c.h"

#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)) && SDL_ASSEMBLY_ROUTINES
#define MMX_ASMBLIT
#if (__GNUC__ > 2)  /* SSE instructions aren't in GCC 2. */
#define SSE_ASMBLIT
#endif
#endif

#if defined(MMX_ASMBLIT)
#include "SDL_cpuinfo.h"
#include "mmx.h"
#endif

/* The general purpose software blit routine */
static int SDL_SoftBlit(SDL_Surface *src, SDL_Rect *srcrect,
			SDL_Surface *dst, SDL_Rect *dstrect)
{
	/* Set up source and destination buffer pointers, and BLIT! */
	if (srcrect->w && srcrect->h )
   {
		SDL_BlitInfo info;
		SDL_loblit RunBlit;

		/* Set up the blit information */
		info.s_pixels = (Uint8 *)src->pixels +
				(Uint16)srcrect->y*src->pitch +
				(Uint16)srcrect->x*src->format->BytesPerPixel;
		info.s_width = srcrect->w;
		info.s_height = srcrect->h;
		info.s_skip=src->pitch-info.s_width*src->format->BytesPerPixel;
		info.d_pixels = (Uint8 *)dst->pixels +
				(Uint16)dstrect->y*dst->pitch +
				(Uint16)dstrect->x*dst->format->BytesPerPixel;
		info.d_width = dstrect->w;
		info.d_height = dstrect->h;
		info.d_skip=dst->pitch-info.d_width*dst->format->BytesPerPixel;
		info.aux_data = src->map->sw_data->aux_data;
		info.src = src->format;
		info.table = src->map->table;
		info.dst = dst->format;
		RunBlit = src->map->sw_data->blit;

		/* Run the actual software blit */
		RunBlit(&info);
	}

   return 0;
}

#ifdef MMX_ASMBLIT
static __inline__ void SDL_memcpyMMX(Uint8 *to, const Uint8 *from, int len)
{
	int i;

	for(i=0; i<len/8; i++) {
		__asm__ __volatile__ (
		"	movq (%0), %%mm0\n"
		"	movq %%mm0, (%1)\n"
		: : "r" (from), "r" (to) : "memory");
		from+=8;
		to+=8;
	}
	if (len&7)
		SDL_memcpy(to, from, len&7);
}

#ifdef SSE_ASMBLIT
static __inline__ void SDL_memcpySSE(Uint8 *to, const Uint8 *from, int len)
{
	int i;

	__asm__ __volatile__ (
	"	prefetchnta (%0)\n"
	"	prefetchnta 64(%0)\n"
	"	prefetchnta 128(%0)\n"
	"	prefetchnta 192(%0)\n"
	: : "r" (from) );

	for(i=0; i<len/8; i++) {
		__asm__ __volatile__ (
		"	prefetchnta 256(%0)\n"
		"	movq (%0), %%mm0\n"
		"	movntq %%mm0, (%1)\n"
		: : "r" (from), "r" (to) : "memory");
		from+=8;
		to+=8;
	}
	if (len&7)
		SDL_memcpy(to, from, len&7);
}
#endif
#endif

static void SDL_BlitCopy(SDL_BlitInfo *info)
{
	Uint8 *src, *dst;
	int w, h;
	int srcskip, dstskip;

	w = info->d_width*info->dst->BytesPerPixel;
	h = info->d_height;
	src = info->s_pixels;
	dst = info->d_pixels;
	srcskip = w+info->s_skip;
	dstskip = w+info->d_skip;

#ifdef SSE_ASMBLIT
	if(SDL_HasSSE())
	{
		while ( h-- ) {
			SDL_memcpySSE(dst, src, w);
			src += srcskip;
			dst += dstskip;
		}
		__asm__ __volatile__ (
		"	emms\n"
		::);
	}
	else
#endif
#ifdef MMX_ASMBLIT
	if(SDL_HasMMX())
	{
		while ( h-- ) {
			SDL_memcpyMMX(dst, src, w);
			src += srcskip;
			dst += dstskip;
		}
		__asm__ __volatile__ (
		"	emms\n"
		::);
	}
	else
#endif
	while ( h-- ) {
		SDL_memcpy(dst, src, w);
		src += srcskip;
		dst += dstskip;
	}
}

static void SDL_BlitCopyOverlap(SDL_BlitInfo *info)
{
	Uint8 *src, *dst;
	int w, h;
	int srcskip, dstskip;

	w = info->d_width*info->dst->BytesPerPixel;
	h = info->d_height;
	src = info->s_pixels;
	dst = info->d_pixels;
	srcskip = w+info->s_skip;
	dstskip = w+info->d_skip;
	if ( dst < src )
   {
		while ( h-- )
      {
			memmove(dst, src, w);
			src += srcskip;
			dst += dstskip;
		}
	}
   else
   {
		src += ((h-1) * srcskip);
		dst += ((h-1) * dstskip);
		while ( h-- )
      {
			memmove(dst, src, w);
			src -= srcskip;
			dst -= dstskip;
		}
	}
}

/* Figure out which of many blit routines to set up on a surface */
int SDL_CalculateBlit(SDL_Surface *surface)
{
	int blit_index;

	/* Clean everything out to start */
	surface->map->sw_blit = NULL;

	/* Figure out if an accelerated hardware blit is possible */
	surface->flags &= ~SDL_HWACCEL;

	/* Get the blit function index, based on surface mode */
	/* { 0 = nothing, 1 = colorkey, 2 = alpha, 3 = colorkey+alpha } */
	blit_index = 0;
	blit_index |= (!!(surface->flags & SDL_SRCCOLORKEY))      << 0;
	if ( surface->flags & SDL_SRCALPHA
	     && (surface->format->alpha != SDL_ALPHA_OPAQUE
		 || surface->format->Amask) ) {
	        blit_index |= 2;
	}

	/* Check for special "identity" case -- copy blit */
	if ( surface->map->identity && blit_index == 0 ) {
	        surface->map->sw_data->blit = SDL_BlitCopy;

		/* Handle overlapping blits on the same surface */
		if ( surface == surface->map->dst ) {
		        surface->map->sw_data->blit = SDL_BlitCopyOverlap;
		}
	} else {
		if ( surface->format->BitsPerPixel < 8 ) {
			surface->map->sw_data->blit =
			    SDL_CalculateBlit0(surface, blit_index);
		} else {
			switch ( surface->format->BytesPerPixel ) {
			    case 1:
				surface->map->sw_data->blit =
				    SDL_CalculateBlit1(surface, blit_index);
				break;
			    case 2:
			    case 3:
			    case 4:
				surface->map->sw_data->blit =
				    SDL_CalculateBlitN(surface, blit_index);
				break;
			    default:
				surface->map->sw_data->blit = NULL;
				break;
			}
		}
	}
	/* Make sure we have a blit function */
	if ( surface->map->sw_data->blit == NULL ) {
		SDL_InvalidateMap(surface->map);
		SDL_SetError("Blit combination not supported");
		return(-1);
	}

	/* Choose software blitting function */
	if ( surface->map->sw_blit == NULL ) {
		surface->map->sw_blit = SDL_SoftBlit;
	}
	return(0);
}


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
#include "SDL_leaks.h"


/* Public routines */
/*
 * Create an empty RGB surface of the appropriate depth
 */
SDL_Surface * SDL_CreateRGBSurface (Uint32 flags,
			int width, int height, int depth,
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
	SDL_VideoDevice *video = current_video;
	SDL_Surface *screen;
	SDL_Surface *surface;

	/* Make sure the size requested doesn't overflow our datatypes */
	/* Next time I write a library like SDL, I'll use int for size. :) */
	if ( width >= 16384 || height >= 65536 ) {
		SDL_SetError("Width or height is too large");
		return(NULL);
	}

	/* Check to see if we desire the surface in video memory */
	if ( video ) {
		screen = SDL_PublicSurface;
	} else {
		screen = NULL;
	}
	flags &= ~SDL_HWSURFACE;

	/* Allocate the surface */
	surface = (SDL_Surface *)SDL_malloc(sizeof(*surface));
	if ( surface == NULL ) {
		SDL_OutOfMemory();
		return(NULL);
	}
	surface->flags = SDL_SWSURFACE;

	surface->format = SDL_AllocFormat(depth, Rmask, Gmask, Bmask, Amask);
	if ( surface->format == NULL ) {
		SDL_free(surface);
		return(NULL);
	}
	if ( Amask ) {
		surface->flags |= SDL_SRCALPHA;
	}
	surface->w = width;
	surface->h = height;
	surface->pitch = SDL_CalculatePitch(surface);
	surface->pixels = NULL;
	surface->offset = 0;
	surface->hwdata = NULL;
	surface->locked = 0;
	surface->map = NULL;
	surface->unused1 = 0;
	SDL_SetClipRect(surface, NULL);
	SDL_FormatChanged(surface);

	/* Get the pixels */
	if ( ((flags&SDL_HWSURFACE) == SDL_SWSURFACE)) {
		if ( surface->w && surface->h ) {
			surface->pixels = SDL_malloc(surface->h*surface->pitch);
			if ( surface->pixels == NULL ) {
				SDL_FreeSurface(surface);
				SDL_OutOfMemory();
				return(NULL);
			}
			/* This is important for bitmaps */
			SDL_memset(surface->pixels, 0, surface->h*surface->pitch);
		}
	}

	/* Allocate an empty mapping */
	surface->map = SDL_AllocBlitMap();
	if ( surface->map == NULL ) {
		SDL_FreeSurface(surface);
		return(NULL);
	}

	/* The surface is ready to go */
	surface->refcount = 1;
	return(surface);
}
/*
 * Create an RGB surface from an existing memory buffer
 */
SDL_Surface * SDL_CreateRGBSurfaceFrom (void *pixels,
			int width, int height, int depth, int pitch,
			Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
	SDL_Surface *surface;

	surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 0, 0, depth,
	                               Rmask, Gmask, Bmask, Amask);
	if ( surface != NULL ) {
		surface->flags |= SDL_PREALLOC;
		surface->pixels = pixels;
		surface->w = width;
		surface->h = height;
		surface->pitch = pitch;
		SDL_SetClipRect(surface, NULL);
	}
	return(surface);
}
/*
 * Set the color key in a blittable surface
 */
int SDL_SetColorKey (SDL_Surface *surface, Uint32 flag, Uint32 key)
{
	/* Sanity check the flag as it gets passed in */
	if ( flag & SDL_SRCCOLORKEY ) {
		if ( flag & (SDL_RLEACCEL|SDL_RLEACCELOK) ) {
			flag = (SDL_SRCCOLORKEY | SDL_RLEACCELOK);
		} else {
			flag = SDL_SRCCOLORKEY;
		}
	} else {
		flag = 0;
	}

	/* Optimize away operations that don't change anything */
	if ( (flag == (surface->flags & (SDL_SRCCOLORKEY|SDL_RLEACCELOK))) &&
	     (key == surface->format->colorkey) ) {
		return(0);
	}

	if ( flag ) {
		SDL_VideoDevice *video = current_video;

		surface->flags |= SDL_SRCCOLORKEY;
		surface->format->colorkey = key;
		if ( flag & SDL_RLEACCELOK ) {
			surface->flags |= SDL_RLEACCELOK;
		} else {
			surface->flags &= ~SDL_RLEACCELOK;
		}
	} else {
		surface->flags &= ~(SDL_SRCCOLORKEY|SDL_RLEACCELOK);
		surface->format->colorkey = 0;
	}
	SDL_InvalidateMap(surface->map);
	return(0);
}
/* This function sets the alpha channel of a surface */
int SDL_SetAlpha (SDL_Surface *surface, Uint32 flag, Uint8 value)
{
	Uint32 oldflags = surface->flags;
	Uint32 oldalpha = surface->format->alpha;

	/* Sanity check the flag as it gets passed in */
	if ( flag & SDL_SRCALPHA ) {
		if ( flag & (SDL_RLEACCEL|SDL_RLEACCELOK) ) {
			flag = (SDL_SRCALPHA | SDL_RLEACCELOK);
		} else {
			flag = SDL_SRCALPHA;
		}
	} else {
		flag = 0;
	}

	/* Optimize away operations that don't change anything */
	if ( (flag == (surface->flags & (SDL_SRCALPHA|SDL_RLEACCELOK))) &&
	     (!flag || value == oldalpha) ) {
		return(0);
	}

	if ( flag ) {
		SDL_VideoDevice *video = current_video;

		surface->flags |= SDL_SRCALPHA;
		surface->format->alpha = value;

		if ( flag & SDL_RLEACCELOK ) {
		        surface->flags |= SDL_RLEACCELOK;
		} else {
		        surface->flags &= ~SDL_RLEACCELOK;
		}
	} else {
		surface->flags &= ~SDL_SRCALPHA;
		surface->format->alpha = SDL_ALPHA_OPAQUE;
	}
	/*
	 * The representation for software surfaces is independent of
	 * per-surface alpha, so no need to invalidate the blit mapping
	 * if just the alpha value was changed. (If either is 255, we still
	 * need to invalidate.)
	 */
	if(oldflags != surface->flags || (((oldalpha + 1) ^ (value + 1)) & 0x100))
		SDL_InvalidateMap(surface->map);
	return(0);
}
int SDL_SetAlphaChannel(SDL_Surface *surface, Uint8 value)
{
	int row, col;
	int offset;
	Uint8 *buf;

	if ( (surface->format->Amask != 0xFF000000) &&
	     (surface->format->Amask != 0x000000FF) ) {
		SDL_SetError("Unsupported surface alpha mask format");
		return -1;
	}

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	if ( surface->format->Amask == 0xFF000000 ) {
			offset = 3;
	} else {
			offset = 0;
	}
#else
	if ( surface->format->Amask == 0xFF000000 ) {
			offset = 0;
	} else {
			offset = 3;
	}
#endif /* Byte ordering */

	/* Quickly set the alpha channel of an RGBA or ARGB surface */
	if ( SDL_MUSTLOCK(surface) ) {
		if ( SDL_LockSurface(surface) < 0 ) {
			return -1;
		}
	}
	row = surface->h;
	while (row--) {
		col = surface->w;
		buf = (Uint8 *)surface->pixels + row * surface->pitch + offset;
		while(col--) {
			*buf = value;
			buf += 4;
		}
	}
	if ( SDL_MUSTLOCK(surface) ) {
		SDL_UnlockSurface(surface);
	}
	return 0;
}

/*
 * A function to calculate the intersection of two rectangles:
 * return true if the rectangles intersect, false otherwise
 */
static __inline__
SDL_bool SDL_IntersectRect(const SDL_Rect *A, const SDL_Rect *B, SDL_Rect *intersection)
{
	int Amin, Amax, Bmin, Bmax;

	/* Horizontal intersection */
	Amin = A->x;
	Amax = Amin + A->w;
	Bmin = B->x;
	Bmax = Bmin + B->w;
	if(Bmin > Amin)
	        Amin = Bmin;
	intersection->x = Amin;
	if(Bmax < Amax)
	        Amax = Bmax;
	intersection->w = Amax - Amin > 0 ? Amax - Amin : 0;

	/* Vertical intersection */
	Amin = A->y;
	Amax = Amin + A->h;
	Bmin = B->y;
	Bmax = Bmin + B->h;
	if(Bmin > Amin)
	        Amin = Bmin;
	intersection->y = Amin;
	if(Bmax < Amax)
	        Amax = Bmax;
	intersection->h = Amax - Amin > 0 ? Amax - Amin : 0;

	return (SDL_bool)(intersection->w && intersection->h);
}
/*
 * Set the clipping rectangle for a blittable surface
 */
SDL_bool SDL_SetClipRect(SDL_Surface *surface, const SDL_Rect *rect)
{
	SDL_Rect full_rect;

	/* Don't do anything if there's no surface to act on */
	if ( ! surface ) {
		return SDL_FALSE;
	}

	/* Set up the full surface rectangle */
	full_rect.x = 0;
	full_rect.y = 0;
	full_rect.w = surface->w;
	full_rect.h = surface->h;

	/* Set the clipping rectangle */
	if ( ! rect ) {
		surface->clip_rect = full_rect;
		return (SDL_bool)1;
	}
	return SDL_IntersectRect(rect, &full_rect, &surface->clip_rect);
}
void SDL_GetClipRect(SDL_Surface *surface, SDL_Rect *rect)
{
	if ( surface && rect ) {
		*rect = surface->clip_rect;
	}
}
/* 
 * Set up a blit between two surfaces -- split into three parts:
 * The upper part, SDL_UpperBlit(), performs clipping and rectangle 
 * verification.  The lower part is a pointer to a low level
 * accelerated blitting function.
 *
 * These parts are separated out and each used internally by this 
 * library in the optimimum places.  They are exported so that if
 * you know exactly what you are doing, you can optimize your code
 * by calling the one(s) you need.
 */
int SDL_LowerBlit (SDL_Surface *src, SDL_Rect *srcrect,
				SDL_Surface *dst, SDL_Rect *dstrect)
{
	/* Check to make sure the blit mapping is valid */
	if ( (src->map->dst != dst) ||
             (src->map->dst->format_version != src->map->format_version) ) {
		if ( SDL_MapSurface(src, dst) < 0 ) {
			return(-1);
		}
	}

	return(src->map->sw_blit(src, srcrect, dst, dstrect));
}


int SDL_UpperBlit (SDL_Surface *src, SDL_Rect *srcrect,
		   SDL_Surface *dst, SDL_Rect *dstrect)
{
        SDL_Rect fulldst;
	int srcx, srcy, w, h;

	/* If the destination rectangle is NULL, use the entire dest surface */
	if ( dstrect == NULL )
   {
	        fulldst.x = fulldst.y = 0;
		dstrect = &fulldst;
	}

	/* clip the source rectangle to the source surface */
	if(srcrect) {
	        int maxw, maxh;
	
		srcx = srcrect->x;
		w = srcrect->w;
		maxw = src->w - srcx;
		if(maxw < w)
			w = maxw;

		srcy = srcrect->y;
		h = srcrect->h;
		maxh = src->h - srcy;
		if(maxh < h)
			h = maxh;
	    
	}
   else
   {
      srcx = srcy = 0;
      w = src->w;
      h = src->h;
	}

	/* clip the destination rectangle against the clip rectangle */
	{
	        SDL_Rect *clip = &dst->clip_rect;
		int dx, dy;

		dx = clip->x - dstrect->x;
		if(dx > 0) {
			w -= dx;
			dstrect->x += dx;
			srcx += dx;
		}
		dx = dstrect->x + w - clip->x - clip->w;
		if(dx > 0)
			w -= dx;

		dy = clip->y - dstrect->y;
		if(dy > 0) {
			h -= dy;
			dstrect->y += dy;
			srcy += dy;
		}
		dy = dstrect->y + h - clip->y - clip->h;
		if(dy > 0)
			h -= dy;
	}

	if(w > 0 && h > 0) {
	        SDL_Rect sr;
	        sr.x = srcx;
		sr.y = srcy;
		sr.w = dstrect->w = w;
		sr.h = dstrect->h = h;
		return SDL_LowerBlit(src, &sr, dst, dstrect);
	}
	dstrect->w = dstrect->h = 0;
	return 0;
}

/* 
 * This function performs a fast fill of the given rectangle with 'color'
 */
int SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)
{
	SDL_VideoDevice *video = current_video;
	int x, y;
	Uint8 *row;

	/* If 'dstrect' == NULL, then fill the whole surface */
	if ( dstrect ) {
		/* Perform clipping */
		if ( !SDL_IntersectRect(dstrect, &dst->clip_rect, dstrect) ) {
			return(0);
		}
	} else {
		dstrect = &dst->clip_rect;
	}

	/* Perform software fill */
	if ( SDL_LockSurface(dst) != 0 ) {
		return(-1);
	}
	row = (Uint8 *)dst->pixels+dstrect->y*dst->pitch+
			dstrect->x*dst->format->BytesPerPixel;
	if ( dst->format->palette || (color == 0) ) {
		x = dstrect->w*dst->format->BytesPerPixel;
		if ( !color && !((uintptr_t)row&3) && !(x&3) && !(dst->pitch&3) ) {
			int n = x >> 2;
			for ( y=dstrect->h; y; --y ) {
				SDL_memset4(row, 0, n);
				row += dst->pitch;
			}
		} else {
			{
				for(y = dstrect->h; y; y--) {
					SDL_memset(row, color, x);
					row += dst->pitch;
				}
			}
		}
	} else {
		switch (dst->format->BytesPerPixel) {
		    case 2:
			for ( y=dstrect->h; y; --y ) {
				Uint16 *pixels = (Uint16 *)row;
				Uint16 c = (Uint16)color;
				Uint32 cc = (Uint32)c << 16 | c;
				int n = dstrect->w;
				if((uintptr_t)pixels & 3) {
					*pixels++ = c;
					n--;
				}
				if(n >> 1)
					SDL_memset4(pixels, cc, n >> 1);
				if(n & 1)
					pixels[n - 1] = c;
				row += dst->pitch;
			}
			break;

		    case 3:
			#if SDL_BYTEORDER == SDL_BIG_ENDIAN
				color <<= 8;
			#endif
			for ( y=dstrect->h; y; --y ) {
				Uint8 *pixels = row;
				for ( x=dstrect->w; x; --x ) {
					SDL_memcpy(pixels, &color, 3);
					pixels += 3;
				}
				row += dst->pitch;
			}
			break;

		    case 4:
			for(y = dstrect->h; y; --y) {
				SDL_memset4(row, color, dstrect->w);
				row += dst->pitch;
			}
			break;
		}
	}
	SDL_UnlockSurface(dst);

	/* We're done! */
	return(0);
}

/*
 * Lock a surface to directly access the pixels
 */
int SDL_LockSurface (SDL_Surface *surface)
{
	if ( ! surface->locked ) {
		/* Perform the lock */
		/* This needs to be done here in case pixels changes value */
		surface->pixels = (Uint8 *)surface->pixels + surface->offset;
	}

	/* Increment the surface lock count, for recursive locks */
	++surface->locked;

	/* Ready to go.. */
	return(0);
}
/*
 * Unlock a previously locked surface
 */
void SDL_UnlockSurface (SDL_Surface *surface)
{
	/* Only perform an unlock if we are locked */
	if ( ! surface->locked || (--surface->locked > 0) ) {
		return;
	}

	/* Perform the unlock */
	surface->pixels = (Uint8 *)surface->pixels - surface->offset;
}

/*
 * Free a surface created by the above function.
 */
void SDL_FreeSurface (SDL_Surface *surface)
{
	/* Free anything that's not NULL, and not the screen surface */
	if ((surface == NULL) ||
	    (current_video &&
	    ((surface == SDL_ShadowSurface)||(surface == SDL_VideoSurface)))) {
		return;
	}
	if ( --surface->refcount > 0 ) {
		return;
	}
	while ( surface->locked > 0 ) {
		SDL_UnlockSurface(surface);
	}
	if ( surface->format ) {
		SDL_FreeFormat(surface->format);
		surface->format = NULL;
	}
	if ( surface->map != NULL ) {
		SDL_FreeBlitMap(surface->map);
		surface->map = NULL;
	}
	if ( surface->pixels &&
	     ((surface->flags & SDL_PREALLOC) != SDL_PREALLOC) ) {
		SDL_free(surface->pixels);
	}
	SDL_free(surface);
}

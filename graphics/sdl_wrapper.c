#include "SDL.h"

static __inline__ SDL_bool SSNES_IntersectRect(const SDL_Rect *A, const SDL_Rect *B, SDL_Rect *intersection)
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

	return (intersection->w && intersection->h);
}

SDL_bool SSNES_SetClipRect(SDL_Surface *surface, const SDL_Rect *rect)
{
	SDL_Rect full_rect;

	/* Set up the full surface rectangle */
	full_rect.x = 0;
	full_rect.y = 0;
	full_rect.w = surface->w;
	full_rect.h = surface->h;

	/* Set the clipping rectangle */
	if ( ! rect ) {
		surface->clip_rect = full_rect;
		return 1;
	}
	return SSNES_IntersectRect(rect, &full_rect, &surface->clip_rect);
}

/* 
 * This function performs a fast fill of the given rectangle with 'color'
 */
int SSNES_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)
{
	int y;
	Uint8 *row;

	/* If 'dstrect' == NULL, then fill the whole surface */
	if ( dstrect ) {
		/* Perform clipping */
		if ( !SSNES_IntersectRect(dstrect, &dst->clip_rect, dstrect) ) {
			return(0);
		}
	} else {
		dstrect = &dst->clip_rect;
	}

	SDL_LockSurface(dst);

	row = (Uint8 *)dst->pixels+dstrect->y*dst->pitch+
			dstrect->x*dst->format->BytesPerPixel;

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
	SDL_UnlockSurface(dst);

	/* We're done! */
	return(0);
}

int SSNES_SetColorKey (SDL_Surface *surface, Uint32 flag, Uint32 key)
{
	/* Optimize away operations that don't change anything */
	if ( (flag == (surface->flags & (SDL_SRCCOLORKEY))) &&
	     (key == surface->format->colorkey) ) {
		return(0);
	}

	surface->flags |= SDL_SRCCOLORKEY;
	surface->format->colorkey = key;
	surface->flags &= ~SDL_RLEACCELOK;
	return(0);
}

#include "SDL.h"

static __inline__ SDL_bool SDL_IntersectRect(const SDL_Rect *A, const SDL_Rect *B, SDL_Rect *intersection)
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

/* 
 * This function performs a fast fill of the given rectangle with 'color'
 */
int SSNES_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color)
{
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

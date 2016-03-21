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
#include "LRSDL_config.h"

#ifndef _LRSDL_sysvideo_h
#define _LRSDL_sysvideo_h

/* This file prototypes the video driver implementation.
   This is designed to be easily converted to C++ in the future.
 */

/* The SDL video driver */
typedef struct SDL_VideoDevice SDL_VideoDevice;

/* Define the SDL video driver structure */
#define _THIS	SDL_VideoDevice *_this
#ifndef _STATUS
#define _STATUS	SDL_status *status
#endif
struct SDL_VideoDevice {
	/* * * */
	/* The name of this video driver */
	const char *name;

	int (*SetColors)(_THIS, int firstcolor, int ncolors,
			 SDL_Color *colors);

	/* This pointer should exist in the native video subsystem and should
	   point to an appropriate update function for the current video mode
	 */
	void (*UpdateRects)(_THIS, int numrects, SDL_Rect *rects);

	/* * * */
	/* Data common to all drivers */
	SDL_Surface *screen;
	SDL_Surface *shadow;
	SDL_Surface *visible;
        SDL_Palette *physpal;	/* physical palette, if != logical palette */
        SDL_Color *gammacols;	/* gamma-corrected colours, or NULL */
	int offset_x;
	int offset_y;

	/* Driver information flags */
	int handles_any_size;	/* Driver handles any size video mode */

	/* * * */
	/* Data private to this driver */
	struct SDL_PrivateVideoData *hidden;

	/* * * */
	/* The function used to dispose of this structure */
	void (*free)(_THIS);
};
#undef _THIS

#endif /* _SDL_sysvideo_h */

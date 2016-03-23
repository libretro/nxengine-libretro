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

/* The high-level video driver subsystem */

#include "SDL.h"
#include "SDL_sysvideo.h"
#include "SDL_blit.h"
#include "SDL_pixels_c.h"

/*
 * Initialize the video and event subsystems -- determine native pixel format
 */
int LRSDL_VideoInit (const char *driver_name, Uint32 flags)
{
	return(0);
}

/*
 * Check to see if a particular video mode is supported.
 * It returns 0 if the requested mode is not supported under any bit depth,
 * or returns the bits-per-pixel of the closest available mode with the
 * given width and height.  If this bits-per-pixel is different from the
 * one used when setting the video mode, SDL_SetVideoMode() will succeed,
 * but will emulate the requested bits-per-pixel with a shadow surface.
 */
static Uint8 SDL_closest_depths[4][8] = {
	/* 8 bit closest depth ordering */
	{ 0, 8, 16, 15, 32, 24, 0, 0 },
	/* 15,16 bit closest depth ordering */
	{ 0, 16, 15, 32, 24, 8, 0, 0 },
	/* 24 bit closest depth ordering */
	{ 0, 24, 32, 16, 15, 8, 0, 0 },
	/* 32 bit closest depth ordering */
	{ 0, 32, 16, 15, 24, 8, 0, 0 }
};

/*
 * Performs hardware double buffering, if possible, or a full update if not.
 */
int LRSDL_Flip(SDL_Surface *screen)
{
	return(0);
}

static void LRSetPalette_logical(SDL_Surface *screen, SDL_Color *colors,
			       int firstcolor, int ncolors)
{
	SDL_Palette *pal = screen->format->palette;
	SDL_Palette *vidpal;

	if ( colors != (pal->colors + firstcolor) ) {
		SDL_memcpy(pal->colors + firstcolor, colors,
		       ncolors * sizeof(*colors));
	}

	LRSDL_FormatChanged(screen);
}

/*
 * Set the physical and/or logical colormap of a surface:
 * Only the screen has a physical colormap. It determines what is actually
 * sent to the display.
 * The logical colormap is used to map blits to/from the surface.
 * 'which' is one or both of SDL_LOGPAL, SDL_PHYSPAL
 *
 * Return nonzero if all colours were set as requested, or 0 otherwise.
 */
int LRSDL_SetPalette(SDL_Surface *screen, int which,
		   SDL_Color *colors, int firstcolor, int ncolors)
{
	SDL_Palette *pal;
	int gotall;
	int palsize;

	if ( !screen )
		return 0;

   /* only screens have physical palettes */
   which &= ~SDL_PHYSPAL;

	/* Verify the parameters */
	pal = screen->format->palette;
	if( !pal ) {
		return 0;	/* not a palettized surface */
	}
	gotall = 1;
	palsize = 1 << screen->format->BitsPerPixel;
	if ( ncolors > (palsize - firstcolor) ) {
		ncolors = (palsize - firstcolor);
		gotall = 0;
	}

	if ( which & SDL_LOGPAL )
		/*
		 * Logical palette change: The actual screen isn't affected,
		 * but the internal colormap is altered so that the
		 * interpretation of the pixel values (for blits etc) is
		 * changed.
		 */
		LRSetPalette_logical(screen, colors, firstcolor, ncolors);

	return gotall;
}

int SDL_SetColors(SDL_Surface *screen, SDL_Color *colors, int firstcolor,
		  int ncolors)
{
	return SDL_SetPalette(screen, SDL_LOGPAL | SDL_PHYSPAL,
			      colors, firstcolor, ncolors);
}

/*
 * Clean up the video subsystem
 */
void LRSDL_VideoQuit (void)
{
}

/*
 * Sets/Gets the title and icon text of the display window, if any.
 */
void SDL_WM_SetCaption (const char *title, const char *icon)
{
}

void SDL_WM_GetCaption (char **title, char **icon)
{
}

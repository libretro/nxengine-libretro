#include <string.h>
#include "../settings.h"
#include "../libretro/libretro.h"
#include "nxsurface.h"
#include "nxsurface.fdh"

#define SCREEN_BPP 15

extern char g_dir[1024];

NXSurface::NXSurface()
{
	fSurface = NULL;
	fFreeSurface = true;
}

NXSurface::NXSurface(int wd, int ht, NXFormat *format)
{
	fSurface = NULL;
	AllocNew(wd, ht, format);
	fFreeSurface = true;
}


NXSurface::NXSurface(SDL_Surface *from_sfc, bool free_surface)
{
	fSurface = from_sfc;
	fFreeSurface = free_surface;
}

NXSurface::~NXSurface()
{
	Free();
}

/*
void c------------------------------() {}
*/

// allocate for an empty surface of the given size
bool NXSurface::AllocNew(int wd, int ht, NXFormat *format)
{
	Free();
	
	fSurface = SDL_CreateRGBSurface(SDL_SRCCOLORKEY, wd, ht, \
	SCREEN_BPP, 0x1f << 10, 0x1f << 5, 0x1f << 0, 0);
	
	if (!fSurface)
	{
		staterr("NXSurface::AllocNew: failed to allocate RGB surface");
		return 1;
	}
	
	return fSurface;
}


// load the surface from a .pbm or bitmap file
bool NXSurface::LoadImage(const char *pbm_name, bool use_colorkey)
{
	SDL_Surface *image;

	Free();
	char filename[1024];

	sprintf(filename, "%s/%s", g_dir, pbm_name);
	stat("filename: %s\n", filename);

	image = SDL_LoadBMP(filename);
	if (!image)
	{
		staterr("NXSurface::LoadImage: load failed of '%s'!", filename);
		return 1;
	}

	fSurface = Scale(image, 1, use_colorkey, true);
	return (fSurface == NULL);
}


NXSurface *NXSurface::FromFile(const char *pbm_name, bool use_colorkey)
{
	NXSurface *sfc = new NXSurface;
	if (sfc->LoadImage(pbm_name, use_colorkey))
	{
		delete sfc;
		return NULL;
	}
	
	return sfc;
}


/*
void c------------------------------() {}
*/

// draw some or all of another surface onto this surface.
void NXSurface::DrawSurface(NXSurface *src, int dstx, int dsty, int srcx, int srcy, int wd, int ht)
{
	SDL_Rect srcrect, dstrect;

	srcrect.x = srcx;
	srcrect.y = srcy;
	srcrect.w = wd;
	srcrect.h = ht;

	dstrect.x = dstx;
	dstrect.y = dsty;

	SDL_BlitSurface(src->fSurface, &srcrect, fSurface, &dstrect);
}

void NXSurface::DrawSurface(NXSurface *src, int dstx, int dsty)
{
	DrawSurface(src, dstx, dsty, 0, 0, src->fSurface->w, src->fSurface->h);
}

// draw the given source surface in a repeating pattern across the entire width of the surface.
// x_dst: an starting X with which to offset the pattern horizontally (usually negative).
// y_dst: the Y coordinate to copy to on the destination.
// y_src: the Y coordinate to copy from.
// height: the number of pixels tall to copy.
void NXSurface::BlitPatternAcross(NXSurface *src, int x_dst, int y_dst, int y_src, int height)
{
	SDL_Rect srcrect, dstrect;

	srcrect.x = 0;
	srcrect.w = src->fSurface->w;
	srcrect.y = (y_src);
	srcrect.h = (height);

	int x = (x_dst);
	int y = (y_dst);
	int destwd = fSurface->w;

	do
	{
		dstrect.x = x;
		dstrect.y = y;

		SDL_LowerBlit(src->fSurface, &srcrect, fSurface, &dstrect);
		x += src->fSurface->w;
	}
	while(x < destwd);
}

void NXSurface::DrawRect(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b)
{
	SDL_Rect rect;
	uint32_t color = r << RED_SHIFT | g << GREEN_SHIFT | b << BLUE_SHIFT;

	// top and bottom
	rect.x = x1;
	rect.y = y1;
	rect.w = ((x2 - x1) + 1);
	rect.h = 1;
	SDL_FillRect(fSurface, &rect, color);

	rect.y = y2;
	SDL_FillRect(fSurface, &rect, color);

	// left and right
	rect.y = y1;
	rect.w = 1;
	rect.h = ((y2 - y1) + 1);
	SDL_FillRect(fSurface, &rect, color);

	rect.x = x2;
	SDL_FillRect(fSurface, &rect, color);
}


void NXSurface::FillRect(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b)
{
	SDL_Rect rect;
	uint32_t color = r << RED_SHIFT | g << GREEN_SHIFT | b << BLUE_SHIFT;

	rect.x = x1;
	rect.y = y1;
	rect.w = ((x2 - x1) + 1);
	rect.h = ((y2 - y1) + 1);

	SDL_FillRect(fSurface, &rect, color);
}


void NXSurface::DrawPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
	DrawRect(x, y, x, y, r, g, b);
}

/*
void c------------------------------() {}
*/

void NXSurface::set_clip_rect(int x, int y, int w, int h)
{
	NXRect rect(x, y, w, h);
	SDL_SetClipRect(fSurface, &rect);
}

void NXSurface::set_clip_rect(NXRect *rect)
{
	SDL_SetClipRect(fSurface, rect);
}

void NXSurface::clear_clip_rect()
{
	SDL_SetClipRect(fSurface, NULL);
}

// internal function which scales the given SDL surface by the given factor.
SDL_Surface *NXSurface::Scale(SDL_Surface *original, int factor, bool use_colorkey, bool free_original)
{
	uint8_t color = SDL_MapRGB(original->format, 0, 0, 0);

	// set colorkey to black if requested

	if (use_colorkey)
	{	
		// don't use SDL_RLEACCEL--it seems to actually make things a lot slower,
		// especially on maps with motion tiles.
		SDL_SetColorKey(original, SDL_SRCCOLORKEY, color);
	}

	return original;
}

void NXSurface::Free()
{
	if (fSurface)
	{
		if (fFreeSurface)
			SDL_FreeSurface(fSurface);
		
		fSurface = NULL;
	}
}

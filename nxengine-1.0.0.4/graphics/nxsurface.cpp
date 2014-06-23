
#include <string.h>
#include <stdint.h>
#include "../settings.h"
#include "../config.h"
#include "graphics.h"
#include "nxsurface.h"
#include "nxsurface.fdh"
#include "../libretro/libretro.h"
#include "../nx.h"
#include "../extract-auto/cachefiles.h"

#ifdef FRONTEND_SUPPORTS_RGB565
#define SCREEN_BPP 16
#define RED_SHIFT 11
#define GREEN_SHIFT 5
#define BLUE_SHIFT 0
#else
#define SCREEN_BPP 15
#define RED_SHIFT 10
#define GREEN_SHIFT 5
#define BLUE_SHIFT 0
#endif

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
	
#ifdef FRONTEND_SUPPORTS_RGB565
	fSurface = SDL_CreateRGBSurface(SDL_SRCCOLORKEY, wd, ht, 16, 0x1f << 11, 0x3f << 5, 0x1f << 0, 0);
#else
	fSurface = SDL_CreateRGBSurface(SDL_SRCCOLORKEY, wd, ht, 15, 0x1f << 10, 0x1f << 5, 0x1f << 0, 0);
#endif
	
	if (!fSurface)
	{
		NX_ERR("NXSurface::AllocNew: failed to allocate RGB surface\n");
		return 1;
	}
	
	return fSurface;
}


// load the surface from a .pbm or bitmap file
bool NXSurface::LoadImage(const char *pbm_name, bool use_colorkey)
{
	Free();

   CFILE *cf = copen(pbm_name, "rb");
   if (cf)
   {
      SDL_RWops *m = SDL_RWFromMem(cfile_pointer(cf), cfile_size(cf));
      cclose(cf);
      fSurface = SDL_LoadBMP_RW(m, 1);
   } else {
      fSurface = SDL_LoadBMP(pbm_name);
   }

	if (!fSurface)
	{
		NX_ERR("NXSurface::LoadImage: load failed of '%s'!\n", pbm_name);
		return 1;
	}
	
	uint8_t color = SDL_MapRGB(fSurface->format, 0, 0, 0);

	// set colorkey to black if requested
	if (use_colorkey)
		SDL_SetColorKey(fSurface, SDL_SRCCOLORKEY, color);

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
void NXSurface::DrawSurface(NXSurface *src, \
						 	int dstx, int dsty, int srcx, int srcy, int wd, int ht)
{
SDL_Rect srcrect, dstrect;

	srcrect.x = srcx;
	srcrect.y = srcy;
	srcrect.w = wd;
	srcrect.h = ht;
	
	dstrect.x = dstx;
	dstrect.y = dsty;
	
	SDL_UpperBlit(src->fSurface, &srcrect, fSurface, &dstrect);
}

void NXSurface::DrawSurface(NXSurface *src, int dstx, int dsty)
{
	DrawSurface(src, dstx, dsty, 0, 0, src->Width(), src->Height());
}

// draw the given source surface in a repeating pattern across the entire width of the surface.
// x_dst: an starting X with which to offset the pattern horizontally (usually negative).
// y_dst: the Y coordinate to copy to on the destination.
// y_src: the Y coordinate to copy from.
// height: the number of pixels tall to copy.
void NXSurface::BlitPatternAcross(NXSurface *src,
						   int x_dst, int y_dst, int y_src, int height)
{
SDL_Rect srcrect, dstrect;

	srcrect.x = 0;
	srcrect.w = src->fSurface->w;
	srcrect.y = y_src;
	srcrect.h = height;
	
	int destwd = fSurface->w;
	
	do
	{
		dstrect.x = x_dst;
		dstrect.y = y_dst;
		
		SDL_UpperBlit(src->fSurface, &srcrect, fSurface, &dstrect);
		x_dst += src->fSurface->w;
	}while(x_dst < destwd);
}


/*
void c------------------------------() {}
*/


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

void NXSurface::Clear(uint8_t r, uint8_t g, uint8_t b)
{
	SDL_FillRect(fSurface, NULL, SDL_MapRGB(fSurface->format, r, g, b));
}


void NXSurface::DrawPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
	DrawRect(x, y, x, y, r, g, b);
}

/*
void c------------------------------() {}
*/

int NXSurface::Width()
{
	return fSurface->w;
}

int NXSurface::Height()
{
	return fSurface->h;
}

NXFormat *NXSurface::Format()
{
	return fSurface->format;
}

extern void* retro_frame_buffer;
extern unsigned retro_frame_buffer_width;
extern unsigned retro_frame_buffer_height;
extern unsigned retro_frame_buffer_pitch;

void NXSurface::Flip()
{
   retro_frame_buffer = fSurface->pixels;
   retro_frame_buffer_width = fSurface->w;
   retro_frame_buffer_height = fSurface->h;
   retro_frame_buffer_pitch = fSurface->pitch;
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

/*
void c------------------------------() {}
*/

void NXSurface::Free()
{
	if (fSurface)
	{
		if (fFreeSurface)
			SDL_FreeSurface(fSurface);
		
		fSurface = NULL;
	}
}




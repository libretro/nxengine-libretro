
#include <string.h>
#include "../settings.h"
#include "nxsurface.h"
#include "nxsurface.fdh"

int SCALE = 3;


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

// static function, and requires a reload of all surfaces
void NXSurface::SetScale(int factor)
{
	SCALE = factor;
}

/*
void c------------------------------() {}
*/

// allocate for an empty surface of the given size
bool NXSurface::AllocNew(int wd, int ht, NXFormat *format)
{
	Free();
	
	fSurface = SDL_CreateRGBSurface(SDL_SRCCOLORKEY, wd*SCALE, ht*SCALE, \
			format->BitsPerPixel, format->Rmask, format->Gmask, format->Bmask, format->Amask);
	
	if (!fSurface)
	{
		staterr("NXSurface::AllocNew: failed to allocate RGB surface");
		return 1;
	}
	
	return fSurface;
}


// load the surface from a .pbm or bitmap file
bool NXSurface::LoadImage(const char *pbm_name, bool use_colorkey, int use_display_format)
{
SDL_Surface *image;

	Free();
	
	if (use_display_format == -1)
	{	// use value specified in settings
		use_display_format = settings->displayformat;
	}
	
	image = SDL_LoadBMP(pbm_name);
	if (!image)
	{
		staterr("NXSurface::LoadImage: load failed of '%s'!", pbm_name);
		return 1;
	}
	
	fSurface = Scale(image, SCALE, use_colorkey, true, use_display_format);
	return (fSurface == NULL);
}


NXSurface *NXSurface::FromFile(const char *pbm_name, bool use_colorkey, int use_display_format)
{
	NXSurface *sfc = new NXSurface;
	if (sfc->LoadImage(pbm_name, use_colorkey, use_display_format))
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

	srcrect.x = srcx * SCALE;
	srcrect.y = srcy * SCALE;
	srcrect.w = wd * SCALE;
	srcrect.h = ht * SCALE;
	
	dstrect.x = dstx * SCALE;
	dstrect.y = dsty * SCALE;
	
	SDL_BlitSurface(src->fSurface, &srcrect, fSurface, &dstrect);
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
	srcrect.y = (y_src * SCALE);
	srcrect.h = (height * SCALE);
	
	int x = (x_dst * SCALE);
	int y = (y_dst * SCALE);
	int destwd = fSurface->w;
	
	do
	{
		dstrect.x = x;
		dstrect.y = y;
		
		SDL_BlitSurface(src->fSurface, &srcrect, fSurface, &dstrect);
		x += src->fSurface->w;
	}
	while(x < destwd);
}


/*
void c------------------------------() {}
*/


void NXSurface::DrawRect(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b)
{
SDL_Rect rect;
uint32_t color = MapColor(r, g, b);

	// top and bottom
	rect.x = x1 * SCALE;
	rect.y = y1 * SCALE;
	rect.w = ((x2 - x1) + 1) * SCALE;
	rect.h = SCALE;
	SDL_FillRect(fSurface, &rect, color);
	
	rect.y = y2 * SCALE;
	SDL_FillRect(fSurface, &rect, color);
	
	// left and right
	rect.y = y1 * SCALE;
	rect.w = SCALE;
	rect.h = ((y2 - y1) + 1) * SCALE;
	SDL_FillRect(fSurface, &rect, color);
	
	rect.x = x2 * SCALE;
	SDL_FillRect(fSurface, &rect, color);
}


void NXSurface::FillRect(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b)
{
SDL_Rect rect;

	rect.x = x1 * SCALE;
	rect.y = y1 * SCALE;
	rect.w = ((x2 - x1) + 1) * SCALE;
	rect.h = ((y2 - y1) + 1) * SCALE;
	
	SDL_FillRect(fSurface, &rect, MapColor(r, g, b));
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
	return fSurface->w / SCALE;
}

int NXSurface::Height()
{
	return fSurface->h / SCALE;
}

NXFormat *NXSurface::Format()
{
	return fSurface->format;
}

void NXSurface::Flip()
{
	SDL_Flip(fSurface);
}

/*
void c------------------------------() {}
*/

void NXSurface::set_clip_rect(int x, int y, int w, int h)
{
	NXRect rect(x * SCALE, y * SCALE, w * SCALE, h * SCALE);
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

// internal function which scales the given SDL surface by the given factor.
SDL_Surface *NXSurface::Scale(SDL_Surface *original, int factor, \
		bool use_colorkey, bool free_original, bool use_display_format)
{
SDL_Surface *scaled;

	if (factor == 1 && free_original)
	{
		scaled = original;
	}
	else
	{
		scaled = SDL_CreateRGBSurface(SDL_SRCCOLORKEY, \
						original->w * SCALE, \
						original->h * SCALE, \
						original->format->BitsPerPixel, \
						original->format->Rmask, original->format->Gmask,
						original->format->Bmask, original->format->Amask);
		
		if (original->format->BitsPerPixel == 8)
		{	// copy the palette from the old surface to the new surface
			SDL_Color palette[256];
			for(int i=0;i<256;i++)
			{
				SDL_GetRGB(i, original->format, &palette[i].r, &palette[i].g, &palette[i].b);
			}
			
			SDL_SetColors(scaled, palette, 0, 256);
		}
		
		// all the .pbm files are 8bpp, so I haven't had a reason
		// to write any other scalers.
		switch(original->format->BitsPerPixel)
		{
			case 8:
				Scale8(original, scaled, factor);
			break;
			
			default:
				staterr("NXSurface::Scale: unsupported bpp %d", original->format->BitsPerPixel);
				SDL_FreeSurface(scaled);
			return NULL;
		}
		
		// can get rid of original now if they wanted us to
		if (free_original)
			SDL_FreeSurface(original);
	}
	
	// set colorkey to black if requested
	if (use_colorkey)
	{	// don't use SDL_RLEACCEL--it seems to actually make things a lot slower,
		// especially on maps with motion tiles.
		SDL_SetColorKey(scaled, SDL_SRCCOLORKEY, SDL_MapRGB(scaled->format, 0, 0, 0));
	}
	
	if (use_display_format)
	{
		SDL_Surface *ret_sfc = SDL_DisplayFormat(scaled);
		SDL_FreeSurface(scaled);
		
		return ret_sfc;
	}
	else
	{
		return scaled;
	}
}

void NXSurface::Scale8(SDL_Surface *src, SDL_Surface *dst, int factor)
{
int x, y, i;
//if (factor==2)
if (false)
	NXSurface::scale2x(src,dst);
else
{
	for(y=0;y<src->h;y++)
	{
		uint8_t *srcline = (uint8_t *)src->pixels + (y * src->pitch);
		uint8_t *dstline = (uint8_t *)dst->pixels + (y * factor * dst->pitch);
		uint8_t *dstptr = dstline;
		
		for(x=0;x<src->w;x++)
		{
			for(i=0;i<factor;i++)
				*(dstptr++) = srcline[x];
		}
		
		dstptr = dstline;
		for(i=1;i<factor;i++)
		{
			dstptr += dst->pitch;
			memcpy(dstptr, dstline, dst->pitch);
		}
	}
}
}


uint32_t NXSurface::getpixel(SDL_Surface *surface, int x, int y)
{
	int bpp = surface->format->BytesPerPixel;
	uint8_t *p = (uint8_t *)surface->pixels + y * surface->pitch + x * bpp;
	
	switch(bpp)
	{
		case 1:
			return *p;
		
		case 2:
			return *(uint16_t *)p;
		
		case 3:
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
				return p[0] << 16 | p[1] << 8 | p[2];
			else
				return p[0] | p[1] << 8 | p[2] << 16;
		
		case 4:
			return *(uint32_t *)p;
    }
}


void NXSurface::putpixel(SDL_Surface *surface, int x, int y, uint32_t color)
{
	int bpp = surface->format->BytesPerPixel;
	uint8_t *p = (uint8_t *)surface->pixels + y * surface->pitch + x * bpp;
	
	switch(bpp)
	{
		case 1:
			*p = (uint8_t)color;
		break;
		
		case 2:
			*(uint16_t *)p = (uint16_t)color;
		break;
		
		case 3:
		{
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
			{
				p[0] = (uint8_t)((color >> 16) & 0xff);
				p[1] = (uint8_t)((color >> 8) & 0xff);
				p[2] = (uint8_t)(color & 0xff);
			}
			else
			{
				p[0] = (uint8_t)(color & 0xff);
				p[1] = (uint8_t)((color >> 8) & 0xff);
				p[2] = (uint8_t)((color >> 16) & 0xff);
			}
		}
		break;
		
		case 4:
			*(uint32_t *)p = color;
		break;
	}
}


/*
void c------------------------------() {}
*/

void NXSurface::EnableColorKey()
{
	SDL_SetColorKey(fSurface, SDL_SRCCOLORKEY, SDL_MapRGB(fSurface->format, 0, 0, 0));
}

uint32_t NXSurface::MapColor(uint8_t r, uint8_t g, uint8_t b)
{
	return SDL_MapRGB(fSurface->format, r, g, b);
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



void NXSurface::scale2x(SDL_Surface* src, SDL_Surface* dst)
{
	int looph, loopw;
	
	Uint8* srcpix = (Uint8*)src->pixels;
	Uint8* dstpix = (Uint8*)dst->pixels;

	const int srcpitch = src->pitch;
	const int dstpitch = dst->pitch;
	const int width = src->w;
	const int height = src->h;

	Uint8 E0, E1, E2, E3, B, D, E, F, H;
	for(looph = 0; looph < height; ++looph)
	{
		for(loopw = 0; loopw < width; ++ loopw)
		{
		    	B = *(Uint8*)(srcpix + (MAX(0,looph-1)*srcpitch) + (1*loopw));
		    	D = *(Uint8*)(srcpix + (looph*srcpitch) + (1*MAX(0,loopw-1)));
		    	E = *(Uint8*)(srcpix + (looph*srcpitch) + (1*loopw));
		    	F = *(Uint8*)(srcpix + (looph*srcpitch) + (1*MIN(width-1,loopw+1)));
		    	H = *(Uint8*)(srcpix + (MIN(height-1,looph+1)*srcpitch) + (1*loopw));

			E0 = D == B && B != F && D != H ? D : E;
       	    	    	E1 = B == F && B != D && F != H ? F : E;
			E2 = D == H && D != B && H != F ? D : E;
			E3 = H == F && D != H && B != F ? F : E;

			*(Uint8*)(dstpix + looph*2*dstpitch + loopw*2*1) = E0;
			*(Uint8*)(dstpix + looph*2*dstpitch + (loopw*2+1)*1) = E1;
			*(Uint8*)(dstpix + (looph*2+1)*dstpitch + loopw*2*1) = E2;
			*(Uint8*)(dstpix + (looph*2+1)*dstpitch + (loopw*2+1)*1) = E3;
		}
	}
}
 

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

#ifndef _LRSDL_blit_h
#define _LRSDL_blit_h

#include "LRSDL_endian.h"

/* The structure passed to the low level blit functions */
typedef struct {
	uint8_t *s_pixels;
	int s_width;
	int s_height;
	int s_skip;
	uint8_t *d_pixels;
	int d_width;
	int d_height;
	int d_skip;
	void *aux_data;
	SDL_PixelFormat *src;
	uint8_t *table;
	SDL_PixelFormat *dst;
} SDL_BlitInfo;

/* The type definition for the low level blit functions */
typedef void (*SDL_loblit)(SDL_BlitInfo *info);

/* This is the private info structure for software accelerated blits */
struct private_swaccel {
	SDL_loblit blit;
	void *aux_data;
};

/* Blit mapping definition */
typedef struct SDL_BlitMap {
	SDL_Surface *dst;
	int identity;
	uint8_t *table;
	SDL_blit hw_blit;
	SDL_blit sw_blit;
	struct private_hwaccel *hw_data;
	struct private_swaccel *sw_data;

	/* the version count matches the destination; mismatch indicates
	   an invalid mapping */
        unsigned int format_version;
} SDL_BlitMap;


/* Functions found in SDL_blit.c */
extern int LRSDL_CalculateBlit(SDL_Surface *surface);

/* Functions found in SDL_blit_{0,1,N,A}.c */
extern SDL_loblit LRSDL_CalculateBlit0(SDL_Surface *surface, int complex);
extern SDL_loblit LRSDL_CalculateBlit1(SDL_Surface *surface, int complex);
extern SDL_loblit LRSDL_CalculateBlitN(SDL_Surface *surface, int complex);
extern SDL_loblit LRSDL_CalculateAlphaBlit(SDL_Surface *surface, int complex);

/*
 * Useful macros for blitting routines
 */

#define FORMAT_EQUAL(A, B)						\
    ((A)->BitsPerPixel == (B)->BitsPerPixel				\
     && ((A)->Rmask == (B)->Rmask) && ((A)->Amask == (B)->Amask))

static __inline void RETRIEVE_RGB_PIXEL(void *buf, int bpp,
      uint32_t *Pixel)
{
   switch (bpp)
   {
      case 2:
         *Pixel = *((uint16_t*)(buf));
         break;
      case 3:
         {
            uint8_t *B = (uint8_t *)(buf);
#ifdef MSB_FIRST
            *Pixel = (B[0] << 16) + (B[1] << 8) + B[2];
#else
            *Pixel = B[0] + (B[1] << 8) + (B[2] << 16);
#endif
         }
         break;
      case 4:
         *Pixel = *((uint32_t*)(buf));
         break;
      default:
         *Pixel = 0; /* appease gcc */
         break;
   }
}

static __inline void DISEMBLE_RGB(void *buf, int bpp,
      SDL_PixelFormat *fmt, uint32_t *Pixel,
      int *r, int *g, int *b)
{
   switch (bpp)
   {
      case 2:
         *Pixel = *((uint16_t*)(buf));
         break;
      case 3:
         {
            uint8_t *B = (uint8_t *)buf;
#ifdef MSB_FIRST
            *Pixel     = (B[0] << 16) + (B[1] << 8) + B[2];
#else
            *Pixel     = B[0] + (B[1] << 8) + (B[2] << 16);
#endif
         }
         break;
      case 4:
         *Pixel = *((uint32_t*)(buf));
         break;
      default:
         *Pixel = 0;	/* prevent gcc from complaining */
         break;
   }

   *r = ((*Pixel & fmt->Rmask) >> fmt->Rshift) << fmt->Rloss;
   *g = ((*Pixel & fmt->Gmask) >> fmt->Gshift) << fmt->Gloss;
   *b = ((*Pixel & fmt->Bmask) >> fmt->Bshift) << fmt->Bloss;
}


/* Assemble R-G-B values into a specified pixel format and store them */
#define PIXEL_FROM_RGB(fmt, r, g, b) (((r>>fmt->Rloss)<<fmt->Rshift)| ((g>>fmt->Gloss)<<fmt->Gshift) | ((b>>fmt->Bloss)<<fmt->Bshift))

#define RGB565_FROM_RGB(r, g, b) ((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3))
#define RGB555_FROM_RGB(r, g, b) ((((r) >> 3) << 10) | (((g) >> 3) << 5) | ((b) >> 3))
#define RGB888_FROM_RGB(r, g, b) (((r)        << 16) | ((g)        << 8) | (b))

static __inline void ASSEMBLE_RGB(void *buf, int bpp,
      SDL_PixelFormat *fmt, int r, int g, int b)
{
   switch (bpp)
   {
      case 2:
         *((uint16_t*)(buf)) = PIXEL_FROM_RGB(fmt, r, g, b);
         break;
      case 3:
#ifdef MSB_FIRST
         *(((uint32_t*)(buf))+2-fmt->Rshift/8) = r;
         *(((uint32_t*)(buf))+2-fmt->Gshift/8) = g;
         *(((uint32_t*)(buf))+2-fmt->Bshift/8) = b;
#else
         *(((uint32_t*)(buf))+fmt->Rshift/8) = r;
         *(((uint32_t*)(buf))+fmt->Gshift/8) = g;
         *(((uint32_t*)(buf))+fmt->Bshift/8) = b;
#endif
         break;
      case 4:
         *((uint32_t*)(buf)) = PIXEL_FROM_RGB(fmt, r, g, b);
         break;
   }
}

static __inline void DISEMBLE_RGBA(void *buf, int bpp,
      SDL_PixelFormat *fmt, uint32_t *Pixel,
      int *r, int *g, int *b, int *a)
{
   switch (bpp)
   {
      case 2:
         *Pixel = *((uint16_t*)(buf));
         break;
      case 3:
         {
            /* FIXME: broken code (no alpha) */
            uint8_t *b = (uint8_t *)buf;

#ifdef MSB_FIRST
            *Pixel = (b[0] << 16) + (b[1] << 8) + b[2];
#else
            *Pixel = b[0] + (b[1] << 8) + (b[2] << 16);
#endif
         }
         break;
      case 4:
         *Pixel = *((uint32_t*)(buf));
         break;
      default:
         *Pixel = 0; /* stop gcc complaints */
         break;
   }

   *r = ((*Pixel & fmt->Rmask) >> fmt->Rshift) << fmt->Rloss;
   *g = ((*Pixel & fmt->Gmask) >> fmt->Gshift) << fmt->Gloss; 
   *b = ((*Pixel & fmt->Bmask) >> fmt->Bshift) << fmt->Bloss;
   *a = ((*Pixel & fmt->Amask) >> fmt->Ashift) << fmt->Aloss;

   *Pixel &= ~fmt->Amask;
}

/* FIXME: this isn't correct, especially for Alpha (maximum != 255) */
#define PIXEL_FROM_RGBA(fmt, r, g, b, a) (((r>>fmt->Rloss)<<fmt->Rshift)| ((g>>fmt->Gloss)<<fmt->Gshift)| ((b>>fmt->Bloss)<<fmt->Bshift)| ((a>>fmt->Aloss)<<fmt->Ashift))

static __inline void ASSEMBLE_RGBA(void *buf, int bpp,
      SDL_PixelFormat *fmt, int r, int g, int b, int a)
{
   switch (bpp)
   {
      case 2:
         *((uint16_t*)(buf)) = PIXEL_FROM_RGBA(fmt, r, g, b, a);
         break;
      case 3:
         /* FIXME: broken code (no alpha) */
#ifdef MSB_FIRST
         *(((uint32_t*)(buf))+2-fmt->Rshift/8) = r;
         *(((uint32_t*)(buf))+2-fmt->Gshift/8) = g;
         *(((uint32_t*)(buf))+2-fmt->Bshift/8) = b;
#else
         *(((uint32_t*)(buf))+fmt->Rshift/8)   = r;
         *(((uint32_t*)(buf))+fmt->Gshift/8)   = g;
         *(((uint32_t*)(buf))+fmt->Bshift/8)   = b;
#endif
         break;
      case 4:
         *((uint32_t*)(buf)) = PIXEL_FROM_RGBA(fmt, r, g, b, a);
         break;
   }
}

/* Blend the RGB values of two Pixels based on a source alpha value */
static __inline void ALPHA_BLEND(int sR, int sG, int sB, const int A, 
      int *dR, int *dG, int *dB)
{
   *dR = (((sR - *dR) * A + 255) >> 8) + *dR;
   *dG = (((sG - *dG) * A + 255) >> 8) + *dG;
   *dB = (((sB - *dB) * A + 255) >> 8) + *dB;
}

/* Prevent Visual C++ 6.0 from printing out stupid warnings */
#if defined(_MSC_VER) && (_MSC_VER >= 600)
#pragma warning(disable: 4550)
#endif

#endif /* _SDL_blit_h */

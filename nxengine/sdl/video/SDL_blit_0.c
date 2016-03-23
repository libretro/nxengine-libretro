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
#include <stdint.h>

#include "LRSDL_config.h"

#include "LRSDL_video.h"
#include "SDL_blit.h"

#ifdef SDLPRINTF
#define sdlprintf printf
#else
#define sdlprintf
#endif

/* Functions to blit from bitmaps to other surfaces */

static void BlitBto1(SDL_BlitInfo *info)
{
   int c;
   int    width = info->d_width;
   int   height = info->d_height;
   int srcskip  = info->s_skip + width - (width + 7) / 8;
   int dstskip  = info->d_skip;
   uint8_t *map = info->table;
   uint8_t *src = info->s_pixels;
   uint8_t *dst = info->d_pixels;

   if ( map )
   {
      while ( height-- )
      {
         uint8_t bit;
         uint8_t byte = 0;

         for ( c=0; c<width; ++c )
         {
            if ( (c&7) == 0 )
               byte = *src++;
            bit = (byte&0x80)>>7;
            if (1)
               *dst = map[bit];
            dst++;
            byte <<= 1;
         }
         src += srcskip;
         dst += dstskip;
      }
   }
   else
   {
      while ( height-- )
      {
         uint8_t bit;
         uint8_t byte = 0;
         for ( c=0; c<width; ++c )
         {
            if ((c&7) == 0)
               byte = *src++;
            bit = (byte&0x80)>>7;
            if (1)
               *dst = bit;
            dst++;
            byte <<= 1;
         }
         src += srcskip;
         dst += dstskip;
      }
   }
}

static void BlitBto2(SDL_BlitInfo *info)
{
   int c;
   int width     = info->d_width;
   int height    = info->d_height;
   int dstskip   = info->d_skip/2;
   int srcskip   = info->s_skip + width-(width+7)/8;
   uint16_t *map = (uint16_t*)info->table;
   uint8_t *src  = info->s_pixels;
   uint16_t *dst = (uint16_t*)info->d_pixels;

   while ( height-- )
   {
      uint8_t bit;
      uint8_t byte = 0;

      for ( c=0; c<width; ++c )
      {
         if ( (c&7) == 0 )
            byte = *src++;
         bit = (byte&0x80)>>7;
         if (1)
            *dst = map[bit];
         byte <<= 1;
         dst++;
      }
      src += srcskip;
      dst += dstskip;
   }
}

static void BlitBto3(SDL_BlitInfo *info)
{
   int c, o;
   int width    = info->d_width;
   int height   = info->d_height;
   int dstskip  = info->d_skip;
   int srcskip  = info->s_skip + width-(width+7)/8;
   uint8_t *map = info->table;
   uint8_t *src = info->s_pixels;
   uint8_t *dst = info->d_pixels;

   while ( height-- )
   {
      uint8_t bit;
      uint8_t byte = 0;

      for ( c=0; c<width; ++c )
      {
         if ( (c&7) == 0 )
            byte = *src++;
         bit = (byte & 0x80)>>7;

         if (1)
         {
            o = bit * 4;
            dst[0] = map[o++];
            dst[1] = map[o++];
            dst[2] = map[o++];
         }
         byte <<= 1;
         dst += 3;
      }
      src += srcskip;
      dst += dstskip;
   }
}

static void BlitBto4(SDL_BlitInfo *info)
{
   int c;
   int    width  = info->d_width;
   int   height  = info->d_height;
   int dstskip   = info->d_skip/4;
   int srcskip   = info->s_skip + width - (width + 7) / 8;
   uint32_t *map = (uint32_t*)info->table;
   uint8_t *src  = info->s_pixels;
   uint32_t *dst = (uint32_t*)info->d_pixels;

   while ( height-- )
   {
      uint8_t bit;
      uint8_t byte = 0;

      for ( c=0; c<width; ++c )
      {
         if ( (c&7) == 0 )
            byte = *src++;
         bit = (byte&0x80)>>7;
         if (1)
            *dst = map[bit];
         byte <<= 1;
         dst++;
      }
      src += srcskip;
      dst += dstskip;
   }
}

static void BlitBto1Key(SDL_BlitInfo *info)
{
   int c;
   int width       = info->d_width;
   int height      = info->d_height;
   int srcskip     = info->s_skip + width - (width + 7) / 8;
   int dstskip     = info->d_skip;
   uint32_t ckey   = info->src->colorkey;
   uint8_t *palmap = info->table;
   uint8_t *src    = info->s_pixels;
   uint8_t *dst    = info->d_pixels;

   if ( palmap )
   {
      while ( height-- )
      {
         uint8_t bit;
         uint8_t byte = 0;

         for ( c=0; c<width; ++c )
         {
            if ( (c&7) == 0 )
               byte = *src++;
            bit = (byte&0x80)>>7;
            if ( bit != ckey )
               *dst = palmap[bit];
            dst++;
            byte <<= 1;
         }
         src += srcskip;
         dst += dstskip;
      }
   }
   else
   {
      while ( height-- )
      {
         uint8_t bit;
         uint8_t byte = 0;
         for ( c=0; c<width; ++c )
         {
            if ( (c&7) == 0 )
               byte = *src++;
            bit = (byte&0x80)>>7;
            if ( bit != ckey )
               *dst = bit;
            dst++;
            byte <<= 1;
         }
         src += srcskip;
         dst += dstskip;
      }
   }
}

static void BlitBto2Key(SDL_BlitInfo *info)
{
   int c;
   int width       = info->d_width;
   int height      = info->d_height;
   int srcskip     = info->s_skip + width - (width + 7) / 8;
   int dstskip     = info->d_skip / 2;
   uint32_t ckey   = info->src->colorkey;
   uint8_t *palmap = info->table;
   uint8_t *src    = info->s_pixels;
   uint16_t *dst   = (uint16_t*)info->d_pixels;

   while ( height-- )
   {
      uint8_t bit;
      uint8_t byte = 0;

      for ( c=0; c<width; ++c )
      {
         if ( (c&7) == 0 )
            byte = *src++;
         bit = (byte&0x80)>>7;
         if ( bit != ckey )
            *dst=((uint16_t*)palmap)[bit];
         byte <<= 1;
         dst++;
      }
      src  += srcskip;
      dst  += dstskip;
   }
}

static void BlitBto3Key(SDL_BlitInfo *info)
{
   int c;
   int width       = info->d_width;
   int height      = info->d_height;
   uint8_t *src    = info->s_pixels;
   uint8_t *dst    = info->d_pixels;
   int srcskip     = info->s_skip + width - (width + 7) / 8;
   int dstskip     = info->d_skip;
   uint32_t ckey   = info->src->colorkey;
   uint8_t *palmap = info->table;

   while ( height-- )
   {
      uint8_t bit;
      uint8_t byte = 0;
      for ( c=0; c<width; ++c )
      {
         if ( (c&7) == 0 )
            byte = *src++;
         bit = (byte&0x80)>>7;
         if ( bit != ckey )
            memcpy(dst, &palmap[bit*4], 3);

         byte <<= 1;
         dst   += 3;
      }
      src += srcskip;
      dst += dstskip;
   }
}

static void BlitBto4Key(SDL_BlitInfo *info)
{
   int c;
   int width       = info->d_width;
   int height      = info->d_height;
   uint8_t *src    = info->s_pixels;
   uint32_t *dst   = (uint32_t*)info->d_pixels;
   int srcskip     = info->s_skip + width - (width+7) / 8;
   int dstskip     = info->d_skip / 4;
   uint32_t ckey   = info->src->colorkey;
   uint8_t *palmap = info->table;

   while ( height-- )
   {
      uint8_t bit;
      uint8_t byte = 0;

      for ( c=0; c<width; ++c )
      {
         if ( (c&7) == 0 )
            byte = *src++;
         bit = (byte&0x80)>>7;
         if ( bit != ckey )
            *dst = ((uint32_t *)palmap)[bit];
         byte <<= 1;
         dst++;
      }
      src += srcskip;
      dst += dstskip;
   }
}

static void BlitBtoNAlpha(SDL_BlitInfo *info)
{
   int c;
   int width               = info->d_width;
   int height              = info->d_height;
   int dstskip             = info->d_skip;
   const SDL_Color *srcpal	= info->src->palette->colors;
   SDL_PixelFormat *dstfmt = info->dst;
   const int A             = info->src->alpha;
   int dstbpp              = dstfmt->BytesPerPixel;
   int srcskip             = info->s_skip + width-(width+7)/8;
   uint8_t *src            = info->s_pixels;
   uint8_t *dst            = info->d_pixels;

   while ( height-- )
   {
      uint8_t bit;
      uint8_t byte = 0;

      for ( c=0; c<width; ++c )
      {
         if ( (c&7) == 0 )
            byte = *src++;
         bit = (byte&0x80)>>7;

         if (1)
         {
            uint32_t pixel;
            int dR, dG, dB;
            int sR = srcpal[bit].r;
            int sG = srcpal[bit].g;
            int sB = srcpal[bit].b;

            DISEMBLE_RGB(dst, dstbpp, dstfmt,
                  &pixel, &dR, &dG, &dB);
            ALPHA_BLEND(sR, sG, sB, A, &dR, &dG, &dB);
            ASSEMBLE_RGB(dst, dstbpp, dstfmt, dR, dG, dB);
         }
         byte <<= 1;
         dst += dstbpp;
      }
      src += srcskip;
      dst += dstskip;
   }
}

static void BlitBtoNAlphaKey(SDL_BlitInfo *info)
{
   int c;
   int width               = info->d_width;
   int height              = info->d_height;
   int dstskip             = info->d_skip;
   SDL_PixelFormat *srcfmt = info->src;
   SDL_PixelFormat *dstfmt = info->dst;
   const SDL_Color *srcpal	= srcfmt->palette->colors;
   const int A             = srcfmt->alpha;
   uint32_t ckey           = srcfmt->colorkey;
   int dstbpp              = dstfmt->BytesPerPixel;
   int srcskip             = info->s_skip + width-(width+7)/8;
   uint8_t *src            = info->s_pixels;
   uint8_t *dst            = info->d_pixels;

   while ( height-- )
   {
      uint8_t bit;
      uint8_t byte = 0;

      for ( c=0; c<width; ++c )
      {
         if ( (c&7) == 0 )
            byte = *src++;
         bit = (byte&0x80)>>7;

         if ( bit != ckey )
         {
            int dR, dG, dB;
            uint32_t pixel;
            int sR = srcpal[bit].r;
            int sG = srcpal[bit].g;
            int sB = srcpal[bit].b;

            DISEMBLE_RGB(dst, dstbpp, dstfmt,
                  &pixel, &dR, &dG, &dB);
            ALPHA_BLEND(sR, sG, sB, A, &dR, &dG, &dB);
            ASSEMBLE_RGB(dst, dstbpp, dstfmt, dR, dG, dB);
         }
         byte <<= 1;
         dst   += dstbpp;
      }
      src += srcskip;
      dst += dstskip;
   }
}

SDL_loblit LRSDL_CalculateBlit0(SDL_Surface *surface, int blit_index)
{
   int which = 0;

   /* We don't support sub 8-bit packed pixel modes */
   if ( surface->format->BitsPerPixel != 1 )
      return NULL;

   if ( surface->map->dst->format->BitsPerPixel >= 8 )
      which = surface->map->dst->format->BytesPerPixel;

   switch(blit_index)
   {
      case 0:			/* copy */
         switch (which)
         {
            case 0:
               return NULL;
            case 1:
               sdlprintf("BlitBto1.\n");
               return BlitBto1;
            case 2:
               sdlprintf("BlitBto2.\n");
               return BlitBto2;
            case 3:
               sdlprintf("BlitBto3.\n");
               return BlitBto3;
            case 4:
               sdlprintf("BlitBto4.\n");
               return BlitBto4;
         }
      case 1:			/* colorkey */
         switch (which)
         {
            case 0:
               return NULL;
            case 1:
               sdlprintf("BlitBto1Key.\n");
               return BlitBto1Key;
            case 2:
               sdlprintf("BlitBto2Key.\n");
               return BlitBto2Key;
            case 3:
               sdlprintf("BlitBto3Key.\n");
               return BlitBto3Key;
            case 4:
               sdlprintf("BlitBto4Key.\n");
               return BlitBto4Key;
         }
      case 2:			/* alpha */
         if (which >= 2)
         {
            sdlprintf("BlitBtoNAlpha.\n");
            return BlitBtoNAlpha;
         }
         break;
      case 4:			/* alpha + colorkey */
         if (which >= 2)
         {
            sdlprintf("BlitBtoNAlphaKey.\n");
            return BlitBtoNAlphaKey;
         }
         break;
   }

   return NULL;
}

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
#include "SDL_sysvideo.h"
#include "SDL_blit.h"
#include "SDL_pixels_c.h"

/* The general purpose software blit routine */
static int LRSDL_SoftBlit(SDL_Surface *src, SDL_Rect *srcrect,
			SDL_Surface *dst, SDL_Rect *dstrect)
{
   /* Set up source and destination buffer pointers, and BLIT! */
   if (srcrect->w && srcrect->h )
   {
      SDL_BlitInfo info;
      SDL_loblit RunBlit;

      /* Set up the blit information */
      info.s_pixels = (uint8_t*)src->pixels +
         (uint16_t)srcrect->y * src->pitch +
         (uint16_t)srcrect->x * src->format->BytesPerPixel;
      info.s_width  = srcrect->w;
      info.s_height = srcrect->h;
      info.s_skip   = src->pitch-info.s_width*src->format->BytesPerPixel;
      info.d_pixels = (uint8_t*)dst->pixels +
         (uint16_t)dstrect->y * dst->pitch +
         (uint16_t)dstrect->x * dst->format->BytesPerPixel;
      info.d_width  = dstrect->w;
      info.d_height = dstrect->h;
      info.d_skip   = dst->pitch-info.d_width*dst->format->BytesPerPixel;
      info.aux_data = src->map->sw_data->aux_data;
      info.src      = src->format;
      info.table    = src->map->table;
      info.dst      = dst->format;
      RunBlit       = src->map->sw_data->blit;

      /* Run the actual software blit */
      RunBlit(&info);
   }

   return 0;
}

static void LRSDL_BlitCopy(SDL_BlitInfo *info)
{
   int w        = info->d_width*info->dst->BytesPerPixel;
   int h        = info->d_height;
   uint8_t *src = info->s_pixels;
   uint8_t *dst = info->d_pixels;
   int srcskip  = w+info->s_skip;
   int dstskip  = w+info->d_skip;

   while ( h-- )
   {
      memcpy(dst, src, w);
      src += srcskip;
      dst += dstskip;
   }
}

static void LRSDL_BlitCopyOverlap(SDL_BlitInfo *info)
{
   int        w = info->d_width*info->dst->BytesPerPixel;
   int        h = info->d_height;
   uint8_t *src = info->s_pixels;
   uint8_t *dst = info->d_pixels;
   int srcskip  = w+info->s_skip;
   int dstskip  = w+info->d_skip;

   if ( dst < src )
   {
      while ( h-- )
      {
         memmove(dst, src, w);
         src += srcskip;
         dst += dstskip;
      }
   }
   else
   {
      src += ((h-1) * srcskip);
      dst += ((h-1) * dstskip);

      while ( h-- )
      {
         memmove(dst, src, w);
         src -= srcskip;
         dst -= dstskip;
      }
   }
}

/* Figure out which of many blit routines to set up on a surface */
int LRSDL_CalculateBlit(SDL_Surface *surface)
{
   int blit_index;

   /* Clean everything out to start */
   surface->map->sw_blit = NULL;

   /* Figure out if an accelerated hardware blit is possible */
   surface->flags &= ~SDL_HWACCEL;

   /* Get the blit function index, based on surface mode */
   /* { 0 = nothing, 1 = colorkey, 2 = alpha, 3 = colorkey+alpha } */
   blit_index  = 0;
   blit_index |= (!!(surface->flags & SDL_SRCCOLORKEY)) << 0;

   if ( surface->flags & SDL_SRCALPHA
         && (surface->format->alpha != SDL_ALPHA_OPAQUE
            || surface->format->Amask) ) {
      blit_index |= 2;
   }

   /* Check for special "identity" case -- copy blit */
   if ( surface->map->identity && blit_index == 0 )
   {
      surface->map->sw_data->blit = LRSDL_BlitCopy;

      /* Handle overlapping blits on the same surface */
      if ( surface == surface->map->dst )
         surface->map->sw_data->blit = LRSDL_BlitCopyOverlap;
   }
   else
   {
      if ( surface->format->BitsPerPixel < 8 )
         surface->map->sw_data->blit =
            LRSDL_CalculateBlit0(surface, blit_index);
      else
      {
         switch ( surface->format->BytesPerPixel )
         {
            case 1:
               surface->map->sw_data->blit =
                  LRSDL_CalculateBlit1(surface, blit_index);
               break;
            case 2:
            case 3:
            case 4:
               surface->map->sw_data->blit =
                  LRSDL_CalculateBlitN(surface, blit_index);
               break;
            default:
               surface->map->sw_data->blit = NULL;
               break;
         }
      }
   }
   /* Make sure we have a blit function */
   if (!surface->map->sw_data->blit)
      goto error;

   surface->map->sw_blit = LRSDL_SoftBlit;

   return 0;

error:
   LRSDL_InvalidateMap(surface->map);
   LRSDL_SetError("Blit combination not supported");
   return -1;
}

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

#include "LRSDL_video.h"
#include "SDL_sysvideo.h"
#include "SDL_blit.h"
#include "SDL_pixels_c.h"

/* Public routines */
/*
 * Create an empty RGB surface of the appropriate depth
 */
SDL_Surface * LRSDL_CreateRGBSurface (
      Uint32 flags,
      int width, int height, int depth,
      Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
   SDL_Surface *surface;

   /* Make sure the size requested doesn't overflow our datatypes */
   /* Next time I write a library like SDL, I'll use int for size. :) */
   if ( width >= 16384 || height >= 65536 )
   {
      LRSDL_SetError("Width or height is too large");
      return NULL;
   }

   flags &= ~SDL_HWSURFACE;

   /* Allocate the surface */
   surface = (SDL_Surface *)SDL_malloc(sizeof(*surface));
   if (!surface)
   {
      LRSDL_OutOfMemory();
      return NULL;
   }

   surface->flags  = SDL_SWSURFACE;
   surface->format = LRSDL_AllocFormat(depth, Rmask, Gmask, Bmask, Amask);

   if (!surface->format)
   {
      SDL_free(surface);
      return NULL;
   }

   if (Amask)
      surface->flags |= SDL_SRCALPHA;
   surface->w         = width;
   surface->h         = height;
   surface->pitch     = LRSDL_CalculatePitch(surface);
   surface->pixels    = NULL;
   surface->offset    = 0;
   surface->hwdata    = NULL;
   surface->locked    = 0;
   surface->map       = NULL;
   surface->unused1   = 0;
   LRSDL_SetClipRect(surface, NULL);
   LRSDL_FormatChanged(surface);

   /* Get the pixels */
   if ( ((flags&SDL_HWSURFACE) == SDL_SWSURFACE))
   {
      if ( surface->w && surface->h )
      {
         surface->pixels = SDL_malloc(surface->h*surface->pitch);

         if (!surface->pixels)
         {
            LRSDL_OutOfMemory();
            goto error;
         }

         /* This is important for bitmaps */
         SDL_memset(surface->pixels, 0, surface->h*surface->pitch);
      }
   }

   /* Allocate an empty mapping */
   surface->map = LRSDL_AllocBlitMap();
   if (!surface->map)
      goto error;

   /* The surface is ready to go */
   surface->refcount = 1;
   return surface;

error:
   if (surface)
      LRSDL_FreeSurface(surface);

   return NULL;
}

/*
 * Create an RGB surface from an existing memory buffer
 */
SDL_Surface * LRSDL_CreateRGBSurfaceFrom (void *pixels,
      int width, int height, int depth, int pitch,
      Uint32 Rmask, Uint32 Gmask, Uint32 Bmask, Uint32 Amask)
{
   SDL_Surface *surface = LRSDL_CreateRGBSurface(SDL_SWSURFACE, 0, 0, depth,
         Rmask, Gmask, Bmask, Amask);

   if (!surface)
      return NULL;

   surface->flags |= SDL_PREALLOC;
   surface->pixels = pixels;
   surface->w      = width;
   surface->h      = height;
   surface->pitch  = pitch;

   LRSDL_SetClipRect(surface, NULL);

   return surface;
}

/*
 * Set the color key in a blittable surface
 */
int LRSDL_SetColorKey (SDL_Surface *surface, Uint32 flag, Uint32 key)
{
   /* Sanity check the flag as it gets passed in */
   if ( flag & SDL_SRCCOLORKEY )
   {
      if ( flag & (SDL_RLEACCEL|SDL_RLEACCELOK) )
         flag = (SDL_SRCCOLORKEY | SDL_RLEACCELOK);
      else
         flag = SDL_SRCCOLORKEY;
   }
   else
      flag = 0;

   /* Optimize away operations that don't change anything */
   if ( (flag == (surface->flags & (SDL_SRCCOLORKEY|SDL_RLEACCELOK))) &&
         (key == surface->format->colorkey) )
      return 0;

   if ( flag )
   {
      surface->flags            |= SDL_SRCCOLORKEY;
      surface->format->colorkey  = key;

      if ( flag & SDL_RLEACCELOK )
         surface->flags |= SDL_RLEACCELOK;
      else
         surface->flags &= ~SDL_RLEACCELOK;
   }
   else
   {
      surface->flags            &= ~(SDL_SRCCOLORKEY|SDL_RLEACCELOK);
      surface->format->colorkey  = 0;
   }
   LRSDL_InvalidateMap(surface->map);
   return(0);
}
/* This function sets the alpha channel of a surface */
int LRSDL_SetAlpha (SDL_Surface *surface, Uint32 flag, Uint8 value)
{
   uint32_t oldflags = surface->flags;
   uint32_t oldalpha = surface->format->alpha;

   /* Sanity check the flag as it gets passed in */
   if ( flag & SDL_SRCALPHA ) {
      if ( flag & (SDL_RLEACCEL|SDL_RLEACCELOK) )
         flag = (SDL_SRCALPHA | SDL_RLEACCELOK);
      else
         flag = SDL_SRCALPHA;
   }
   else
      flag = 0;

   /* Optimize away operations that don't change anything */
   if ( (flag == (surface->flags & (SDL_SRCALPHA|SDL_RLEACCELOK))) &&
         (!flag || value == oldalpha) )
      return 0;

   if ( flag )
   {
      surface->flags         |= SDL_SRCALPHA;
      surface->format->alpha  = value;

      if ( flag & SDL_RLEACCELOK )
         surface->flags |= SDL_RLEACCELOK;
      else
         surface->flags &= ~SDL_RLEACCELOK;
   }
   else
   {
      surface->flags         &= ~SDL_SRCALPHA;
      surface->format->alpha  = SDL_ALPHA_OPAQUE;
   }
   /*
    * The representation for software surfaces is independent of
    * per-surface alpha, so no need to invalidate the blit mapping
    * if just the alpha value was changed. (If either is 255, we still
    * need to invalidate.)
    */
   if(oldflags != surface->flags || (((oldalpha + 1) ^ (value + 1)) & 0x100))
      LRSDL_InvalidateMap(surface->map);

   return 0;
}

int LRSDL_SetAlphaChannel(SDL_Surface *surface, Uint8 value)
{
   int row;
   int offset;

   if ( (surface->format->Amask != 0xFF000000) &&
         (surface->format->Amask != 0x000000FF) )
   {
      LRSDL_SetError("Unsupported surface alpha mask format");
      return -1;
   }

#ifdef MSB_FIRST
   offset = ( surface->format->Amask == 0xFF000000 ) ? 0 : 3;
#else
   offset = ( surface->format->Amask == 0xFF000000 ) ? 3 : 0;
#endif
   row    = surface->h;

   while (row--)
   {
      int      col = surface->w;
      uint8_t *buf = (uint8_t*)surface->pixels + row * surface->pitch + offset;

      while(col--)
      {
         *buf = value;
         buf += 4;
      }
   }
   return 0;
}

/*
 * A function to calculate the intersection of two rectangles:
 * return true if the rectangles intersect, false otherwise
 */
   static __inline__
LRSDL_bool LRSDL_IntersectRect(const SDL_Rect *A,
      const SDL_Rect *B, SDL_Rect *intersection)
{
   /* Horizontal intersection */
   int Amin = A->x;
   int Amax = Amin + A->w;
   int Bmin = B->x;
   int Bmax = Bmin + B->w;

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

   return (LRSDL_bool)(intersection->w && intersection->h);
}

/*
 * Set the clipping rectangle for a blittable surface
 */
LRSDL_bool LRSDL_SetClipRect(SDL_Surface *surface, const SDL_Rect *rect)
{
   SDL_Rect full_rect;

   /* Don't do anything if there's no surface to act on */
   if ( ! surface )
      return LRSDL_FALSE;

   /* Set up the full surface rectangle */
   full_rect.x = 0;
   full_rect.y = 0;
   full_rect.w = surface->w;
   full_rect.h = surface->h;

   /* Set the clipping rectangle */
   if (!rect)
   {
      surface->clip_rect = full_rect;
      return (LRSDL_bool)1;
   }

   return LRSDL_IntersectRect(rect, &full_rect, &surface->clip_rect);
}

/* 
 * Set up a blit between two surfaces -- split into three parts:
 * The upper part, SDL_UpperBlit(), performs clipping and rectangle 
 * verification.  The lower part is a pointer to a low level
 * accelerated blitting function.
 *
 * These parts are separated out and each used internally by this 
 * library in the optimimum places.  They are exported so that if
 * you know exactly what you are doing, you can optimize your code
 * by calling the one(s) you need.
 */
int LRSDL_LowerBlit (SDL_Surface *src, SDL_Rect *srcrect,
      SDL_Surface *dst, SDL_Rect *dstrect)
{
   /* Check to make sure the blit mapping is valid */
   if ( (src->map->dst != dst) ||
         (src->map->dst->format_version != src->map->format_version) )
   {
      if ( LRSDL_MapSurface(src, dst) < 0 )
         return -1;
   }

   return(src->map->sw_blit(src, srcrect, dst, dstrect));
}


int LRSDL_UpperBlit (SDL_Surface *src, SDL_Rect *srcrect,
      SDL_Surface *dst, SDL_Rect *dstrect)
{
   SDL_Rect fulldst;
   int srcx = 0;
   int srcy = 0;
   int w    = src->w;
   int h    = src->h;

   /* If the destination rectangle is NULL, use the entire dest surface */
   if (!dstrect)
   {
      fulldst.x = fulldst.y = 0;
      dstrect   = &fulldst;
   }

   /* clip the source rectangle to the source surface */
   if(srcrect)
   {
      int maxw, maxh;
      srcx = srcrect->x;
      w    = srcrect->w;

      if(srcx < 0)
      {
         w += srcx;
         dstrect->x -= srcx;
         srcx = 0;
      }

      maxw = src->w - srcx;

      if(maxw < w)
         w = maxw;

      srcy = srcrect->y;
      h    = srcrect->h;

      if(srcy < 0)
      {
         h += srcy;
         dstrect->y -= srcy;
         srcy = 0;
      }
      maxh = src->h - srcy;
      if(maxh < h)
         h = maxh;

   }

   /* clip the destination rectangle against the clip rectangle */
   {
      int dy;
      SDL_Rect *clip = &dst->clip_rect;
      int         dx = clip->x - dstrect->x;

      if(dx > 0)
      {
         w -= dx;
         dstrect->x += dx;
         srcx += dx;
      }

      dx = dstrect->x + w - clip->x - clip->w;

      if(dx > 0)
         w -= dx;

      dy = clip->y - dstrect->y;

      if(dy > 0)
      {
         h -= dy;
         dstrect->y += dy;
         srcy += dy;
      }

      dy = dstrect->y + h - clip->y - clip->h;

      if(dy > 0)
         h -= dy;
   }

   if(w <=  0 || h <= 0)
   {
      dstrect->w = 0;
      dstrect->h = 0;
      return 0;
   }

   SDL_Rect sr;
   sr.x = srcx;
   sr.y = srcy;
   sr.w = dstrect->w = w;
   sr.h = dstrect->h = h;
   return LRSDL_LowerBlit(src, &sr, dst, dstrect);
}

/* 
 * This function performs a fast fill of the given rectangle with 'color'
 */
int LRSDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color)
{
   int x, y;
   uint8_t *row;

   if ( dstrect )
   {
      /* Perform clipping */
      if ( !LRSDL_IntersectRect(dstrect, &dst->clip_rect, dstrect) )
         return(0);
   }
   else  /* If 'dstrect' == NULL, then fill the whole surface */
      dstrect = &dst->clip_rect;

   row = (uint8_t*)dst->pixels+dstrect->y*dst->pitch+
      dstrect->x*dst->format->BytesPerPixel;

   if ( dst->format->palette || (color == 0) )
   {
      x = dstrect->w*dst->format->BytesPerPixel;
      if ( !color && !((uintptr_t)row&3) && !(x&3) && !(dst->pitch&3) )
      {
         int n = x >> 2;
         for ( y=dstrect->h; y; --y )
         {
            SDL_memset4(row, 0, n);
            row += dst->pitch;
         }
      }
      else
      {
         for(y = dstrect->h; y; y--)
         {
            SDL_memset(row, color, x);
            row += dst->pitch;
         }
      }
   }
   else
   {
      switch (dst->format->BytesPerPixel)
      {
         case 2:
            for ( y=dstrect->h; y; --y )
            {
               uint16_t *pixels = (uint16_t*)row;
               uint16_t c       = (uint16_t)color;
               uint32_t cc      = (uint32_t)c << 16 | c;
               int n            = dstrect->w;

               if((uintptr_t)pixels & 3)
               {
                  *pixels++ = c;
                  n--;
               }
               if(n >> 1)
                  SDL_memset4(pixels, cc, n >> 1);
               if(n & 1)
                  pixels[n - 1] = c;
               row += dst->pitch;
            }
            break;

         case 3:
#ifdef MSB_FIRST
            color <<= 8;
#endif
            for ( y=dstrect->h; y; --y )
            {
               uint8_t *pixels = row;

               for ( x=dstrect->w; x; --x )
               {
                  memcpy(pixels, &color, 3);
                  pixels += 3;
               }
               row += dst->pitch;
            }
            break;

         case 4:
            for(y = dstrect->h; y; --y)
            {
               SDL_memset4(row, color, dstrect->w);
               row += dst->pitch;
            }
            break;
      }
   }

   return 0;
}

/*
 * Lock a surface to directly access the pixels
 */
int LRSDL_LockSurface (SDL_Surface *surface)
{
   return 0;
}
/*
 * Unlock a previously locked surface
 */
void LRSDL_UnlockSurface (SDL_Surface *surface)
{
   surface->pixels = (uint8_t*)surface->pixels - surface->offset;
}

/* 
 * Convert a surface into the specified pixel format.
 */
SDL_Surface * LRSDL_ConvertSurface (SDL_Surface *surface,
					SDL_PixelFormat *format, Uint32 flags)
{
   SDL_Surface *convert;
   Uint32 colorkey = 0;
   Uint8 alpha = 0;
   Uint32 surface_flags;
   SDL_Rect bounds;

   /* Check for empty destination palette! (results in empty image) */
   if ( format->palette != NULL ) {
      int i;
      for ( i=0; i<format->palette->ncolors; ++i ) {
         if ( (format->palette->colors[i].r != 0) ||
               (format->palette->colors[i].g != 0) ||
               (format->palette->colors[i].b != 0) )
            break;
      }
      if ( i == format->palette->ncolors ) {
         LRSDL_SetError("Empty destination palette");
         return(NULL);
      }
   }

   flags &= ~SDL_HWSURFACE;

   /* Create a new surface with the desired format */
   convert = LRSDL_CreateRGBSurface(flags,
         surface->w, surface->h, format->BitsPerPixel,
         format->Rmask, format->Gmask, format->Bmask, format->Amask);
   if ( convert == NULL ) {
      return(NULL);
   }

   /* Copy the palette if any */
   if ( format->palette && convert->format->palette ) {
      SDL_memcpy(convert->format->palette->colors,
            format->palette->colors,
            format->palette->ncolors*sizeof(SDL_Color));
      convert->format->palette->ncolors = format->palette->ncolors;
   }

   /* Save the original surface color key and alpha */
   surface_flags = surface->flags;
   if ( (surface_flags & SDL_SRCCOLORKEY) == SDL_SRCCOLORKEY ) {
      /* Convert colourkeyed surfaces to RGBA if requested */
      if((flags & SDL_SRCCOLORKEY) != SDL_SRCCOLORKEY
            && format->Amask) {
         surface_flags &= ~SDL_SRCCOLORKEY;
      } else {
         colorkey = surface->format->colorkey;
         LRSDL_SetColorKey(surface, 0, 0);
      }
   }
   if ( (surface_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
      /* Copy over the alpha channel to RGBA if requested */
      if ( format->Amask ) {
         surface->flags &= ~SDL_SRCALPHA;
      } else {
         alpha = surface->format->alpha;
         LRSDL_SetAlpha(surface, 0, 0);
      }
   }

   /* Copy over the image data */
   bounds.x = 0;
   bounds.y = 0;
   bounds.w = surface->w;
   bounds.h = surface->h;
   LRSDL_LowerBlit(surface, &bounds, convert, &bounds);

   /* Clean up the original surface, and update converted surface */
   if ( convert != NULL ) {
      LRSDL_SetClipRect(convert, &surface->clip_rect);
   }
   if ( (surface_flags & SDL_SRCCOLORKEY) == SDL_SRCCOLORKEY ) {
      Uint32 cflags = surface_flags&(SDL_SRCCOLORKEY|SDL_RLEACCELOK);
      if ( convert != NULL ) {
         Uint8 keyR, keyG, keyB;

         LRSDL_GetRGB(colorkey,surface->format,&keyR,&keyG,&keyB);
         LRSDL_SetColorKey(convert, cflags|(flags&SDL_RLEACCELOK),
               LRSDL_MapRGB(convert->format, keyR, keyG, keyB));
      }
      LRSDL_SetColorKey(surface, cflags, colorkey);
   }
   if ( (surface_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
      Uint32 aflags = surface_flags&(SDL_SRCALPHA|SDL_RLEACCELOK);
      if ( convert != NULL ) {
         LRSDL_SetAlpha(convert, aflags|(flags&SDL_RLEACCELOK),
               alpha);
      }
      if ( format->Amask ) {
         surface->flags |= SDL_SRCALPHA;
      } else {
         LRSDL_SetAlpha(surface, aflags, alpha);
      }
   }

   /* We're ready to go! */
   return(convert);
}

/*
 * Free a surface created by the above function.
 */
void LRSDL_FreeSurface (SDL_Surface *surface)
{
   /* Free anything that's not NULL, and not the screen surface */
   if (!surface)
      return;

   if ( --surface->refcount > 0 )
      return;

   if (surface->format)
   {
      LRSDL_FreeFormat(surface->format);
      surface->format = NULL;
   }

   if (surface->map)
   {
      LRSDL_FreeBlitMap(surface->map);
      surface->map = NULL;
   }

   if ( surface->pixels &&
         ((surface->flags & SDL_PREALLOC) != SDL_PREALLOC) )
      SDL_free(surface->pixels);
   SDL_free(surface);
}

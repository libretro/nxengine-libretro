/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2013 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2013 - Daniel De Matteis
 * 
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __NX_LOGGER_H
#define __NX_LOGGER_H

#include <libretro.h>

extern retro_log_printf_t log_cb;

#ifndef NDEBUG
#define  NX_DBG(...) do { \
   if (log_cb) \
      log_cb(RETRO_LOG_DEBUG, __VA_ARGS__); \
   } while (0)
#else
#if defined(_MSC_VER) && _MSC_VER <= 1310
void NX_DBG(const char *fmt, ...);
#else
#define NX_DBG(...)
#endif
#endif

#ifdef RELEASE_BUILD
#if defined(_MSC_VER) && _MSC_VER <= 1310
void NX_LOG(const char *fmt, ...);
#else
#define NX_LOG(...)
#endif
#else

#define  NX_LOG(...) do { \
   if (log_cb) \
      log_cb(RETRO_LOG_INFO, __VA_ARGS__); \
   } while (0)

#endif

#define  NX_WARN(...) do { \
   if (log_cb) \
      log_cb(RETRO_LOG_WARN, __VA_ARGS__); \
   } while (0)
#define  NX_ERR(...) do { \
   if (log_cb) \
      log_cb(RETRO_LOG_ERROR, __VA_ARGS__); \
   } while (0)

#endif

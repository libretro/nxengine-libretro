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

#if defined(ANDROID) && defined(HAVE_LOGGER)
#include <android/log.h>
#endif

#define LOG_FILE (stderr)

#if defined(_XBOX1)
#include "logger_override_xbox1.h"
#else

#ifndef NX_LOG
#if defined(ANDROID) && defined(HAVE_LOGGER)
#define  NX_LOG(...)  __android_log_print(ANDROID_LOG_INFO, "RetroArch: ", __VA_ARGS__)
#elif defined(IS_SALAMANDER)
#define NX_LOG(...) do { \
      fprintf(LOG_FILE, "RetroArch Salamander: " __VA_ARGS__); \
      fflush(LOG_FILE); \
   } while (0)
#else
#define NX_LOG(...) do { \
   fprintf(LOG_FILE, "RetroArch: " __VA_ARGS__); \
   fflush(LOG_FILE); \
   } while (0)
#endif
#endif

#ifndef NX_LOG_OUTPUT
#if defined(ANDROID) && defined(HAVE_LOGGER)
#define  NX_LOG_OUTPUT(...)  __android_log_print(ANDROID_LOG_INFO,"stderr: ",__VA_ARGS__)
#elif defined(IS_SALAMANDER)
#define NX_LOG_OUTPUT(...) do { \
      fprintf(LOG_FILE, "stderr: " __VA_ARGS__); \
      fflush(LOG_FILE); \
   } while (0)
#else
#define NX_LOG_OUTPUT(...) do { \
   fprintf(LOG_FILE, __VA_ARGS__); \
   fflush(LOG_FILE); \
   } while (0)
#endif
#endif

#ifndef NX_ERR
#if defined(ANDROID) && defined(HAVE_LOGGER)
#define  NX_ERR(...)  __android_log_print(ANDROID_LOG_INFO, "RetroArch [ERROR] :: ", __VA_ARGS__)
#elif defined(IS_SALAMANDER)
#define NX_ERR(...) do { \
      fprintf(LOG_FILE, "RetroArch Salamander [ERROR] :: " __VA_ARGS__); \
      fflush(LOG_FILE); \
   } while (0)
#else
#define NX_ERR(...) do { \
      fprintf(LOG_FILE, "RetroArch [ERROR] :: " __VA_ARGS__); \
      fflush(LOG_FILE); \
   } while (0)
#endif
#endif

#ifndef NX_ERR_OUTPUT
#if defined(ANDROID) && defined(HAVE_LOGGER)
#define  NX_ERR_OUTPUT(...)  __android_log_print(ANDROID_LOG_INFO, "stderr [ERROR] :: ", __VA_ARGS__)
#elif defined(IS_SALAMANDER)
#define NX_ERR_OUTPUT(...) do { \
      fprintf(LOG_FILE, "stderr [ERROR] :: " __VA_ARGS__); \
      fflush(LOG_FILE); \
   } while (0)
#else
#define NX_ERR_OUTPUT(...) do { \
      fprintf(LOG_FILE, "stderr [ERROR] :: " __VA_ARGS__); \
      fflush(LOG_FILE); \
   } while (0)
#endif
#endif

#ifndef NX_WARN
#if defined(ANDROID) && defined(HAVE_LOGGER)
#define  NX_WARN(...)  __android_log_print(ANDROID_LOG_INFO, "RetroArch [WARN] :: ", __VA_ARGS__)
#elif defined(IS_SALAMANDER)
#define NX_WARN(...) do { \
      fprintf(LOG_FILE, "RetroArch Salamander [WARN] :: " __VA_ARGS__); \
      fflush(LOG_FILE); \
   } while (0)
#else
#define NX_WARN(...) do { \
      fprintf(LOG_FILE, "RetroArch [WARN] :: " __VA_ARGS__); \
      fflush(LOG_FILE); \
   } while (0)
#endif
#endif

#ifndef NX_WARN
#if defined(ANDROID) && defined(HAVE_LOGGER)
#define  NX_WARN_OUTPUT(...)  __android_log_print(ANDROID_LOG_INFO, "stderr [WARN] :: ", __VA_ARGS__)
#elif defined(IS_SALAMANDER)
#define NX_WARN_OUTPUT(...) do { \
      fprintf(LOG_FILE, "stderr [WARN] :: " __VA_ARGS__); \
      fflush(LOG_FILE); \
   } while (0)
#else
#define NX_WARN_OUTPUT(...) do { \
      fprintf(LOG_FILE, "stderr [WARN] :: " __VA_ARGS__); \
      fflush(LOG_FILE); \
   } while (0)
#endif
#endif
#endif

#endif

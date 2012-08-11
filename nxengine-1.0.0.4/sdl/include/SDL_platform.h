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

/** @file SDL_platform.h
 *  Try to get a standard set of platform defines
 */

#ifndef _SDL_platform_h
#define _SDL_platform_h

#if defined(linux) || defined(__linux) || defined(__linux__)
#undef __LINUX__
#define __LINUX__	1
#endif
#if defined(__APPLE__)
#undef __MACOSX__
#define __MACOSX__	1
#elif defined(macintosh)
#undef __MACOS__
#define __MACOS__	1
#endif
#if defined(__NetBSD__)
#undef __NETBSD__
#define __NETBSD__	1
#endif
#if defined(__OpenBSD__)
#undef __OPENBSD__
#define __OPENBSD__	1
#endif
#if defined(__OS2__)
#undef __OS2__
#define __OS2__		1
#endif
#if defined(osf) || defined(__osf) || defined(__osf__) || defined(_OSF_SOURCE)
#undef __OSF__
#define __OSF__		1
#endif
#if defined(__QNXNTO__)
#undef __QNXNTO__
#define __QNXNTO__	1
#endif
#if defined(__SVR4)
#undef __SOLARIS__
#define __SOLARIS__	1
#endif
#if defined(WIN32) || defined(_WIN32)
#undef __WIN32__
#define __WIN32__	1
#endif

#endif /* _SDL_platform_h */

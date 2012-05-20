#ifndef _LIBRETRO_SHARED_H
#define _LIBRETRO_SHARED_H

const char * retro_create_subpath_string(const char * dir, const char * subdir, const char * filename);
const char * retro_create_path_string(const char * dir, const char * filename);

extern char g_dir[1024];

#endif

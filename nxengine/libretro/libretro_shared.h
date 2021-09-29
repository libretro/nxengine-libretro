#ifndef _LIBRETRO_SHARED_H
#define _LIBRETRO_SHARED_H

#include <boolean.h>

#ifdef _WIN32
#define snprintf _snprintf
#endif

#ifdef __cplusplus
extern "C" {
#endif

void retro_create_subpath_string(char *fname, size_t fname_size, const char * dir, const char * subdir, const char * filename);

void retro_init_saves(void);

bool retro_copy_file(const char* from, const char* to);

void retro_create_path_string(char *fname, size_t fname_size, const char * dir, const char * filename);

const char* retro_get_save_dir(void);

extern char g_dir[1024];

#ifdef __cplusplus
}
#endif

#endif

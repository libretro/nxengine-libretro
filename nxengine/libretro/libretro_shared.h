#ifndef _LIBRETRO_SHARED_H
#define _LIBRETRO_SHARED_H

#ifdef _WIN32
#define snprintf _snprintf
#endif

void retro_create_subpath_string(char *fname, size_t fname_size, const char * dir, const char * subdir, const char * filename);

#ifdef __cplusplus
extern "C" {
#endif

void retro_create_path_string(char *fname, size_t fname_size, const char * dir, const char * filename);

#ifdef __cplusplus
}
#endif

const char* retro_get_save_dir(void);

extern char g_dir[1024];

#endif

#ifndef _LIBRETRO_SHARED_H
#define _LIBRETRO_SHARED_H

void retro_create_subpath_string(char *fname, size_t fname_size, const char * dir, const char * subdir, const char * filename);
void retro_create_path_string(char *fname, size_t fname_size, const char * dir, const char * filename);

extern char g_dir[1024];

#endif

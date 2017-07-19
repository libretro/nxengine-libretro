#ifndef _LIBRETRO_SHARED_H
#define _LIBRETRO_SHARED_H

#ifdef _WIN32
#define snprintf _snprintf
#endif

void retro_create_subpath_string(char *fname, size_t fname_size, const char * dir, const char * subdir, const char * filename);
void retro_create_path_string(char *fname, size_t fname_size, const char * dir, const char * filename);
const char* retro_get_save_dir();
void retro_init_saves();
int retro_copy_file(const char* old_filename, const char* new_filename);
const char *retro_GetProfileName(int num, const char* parent_dir);

extern char g_dir[1024];

#endif

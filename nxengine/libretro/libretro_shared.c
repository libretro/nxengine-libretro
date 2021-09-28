#include <stdio.h>
#include <string.h>

#include <compat/posix_string.h>
#include <libretro.h>
#include <streams/file_stream.h>
#include <file/file_path.h>

#include "libretro_shared.h"

/* Forward declarations */
int64_t rfread(void* buffer,
		size_t elem_size, size_t elem_count, RFILE* stream);
int rfputc(int character, RFILE * stream);
int rfgetc(RFILE* stream);
int rfclose(RFILE* stream);
int rferror(RFILE* stream);
RFILE* rfopen(const char *path, const char *mode);
int rfprintf(RFILE * stream, const char * format, ...);
int64_t rfwrite(void const* buffer,
		size_t elem_size, size_t elem_count, RFILE* stream);

char g_dir[1024];

void retro_create_subpath_string(char *fname, size_t fname_size, const char * dir, const char * subdir, const char * filename)
{
#ifdef _WIN32
	char slash = '\\';
#else
	char slash = '/';
#endif
	snprintf(fname, fname_size, "%s%c%s%c%s", dir, slash, subdir, slash, filename);
}

void retro_create_path_string(char *fname, size_t fname_size, const char * dir, const char * filename)
{
#ifdef _WIN32
	char slash = '\\';
#else
	char slash = '/';
#endif
	snprintf(fname, fname_size, "%s%c%s", dir, slash, filename);
}

/**
 * Copy any missing profiles from the content directory to the save directory.
 */
void retro_init_saves(void)
{
   // Copy any profiles into the save directory.
   const char* save_dir = retro_get_save_dir();
   char gamedirProfile[1024];
   char savedirProfile[1024];
   char profile_name[1024];

   // Copy profiles only if te folders are different.
   if (strcmp(save_dir, g_dir) != 0)
   {
           int i;
	   // Parse through all the different profiles.
	   for (i = 0; i < 5; i++)
	   {
		   // Create the profile filename.
		   if (i == 0)
			   snprintf(profile_name, sizeof(profile_name), "profile.dat");
		   else
			   snprintf(profile_name, sizeof(profile_name), "profile%d.dat", i + 1);

		   // Get the profile's file path in the game directory.
		   retro_create_path_string(gamedirProfile, sizeof(gamedirProfile), g_dir, profile_name);

		   // Make sure the profile exists.
		   if (path_is_valid(gamedirProfile))
		   {
			   // Create the profile's file path in the save directory.
			   retro_create_path_string(savedirProfile, sizeof(savedirProfile), save_dir, profile_name);

			   // Copy the file to the save directory only if it doesn't exist.
			   if (!path_is_valid(savedirProfile))
				   retro_copy_file(gamedirProfile, savedirProfile);
		   }
	   }
   }
}

/**
 * Copy a file to the given destination.
 */
bool retro_copy_file(const char* from, const char* to)
{
	size_t l1;
	unsigned char buffer[8192];
	RFILE *fd2 = NULL;
	RFILE *fd1 = rfopen(from, "r");
	if (!fd1)
		return false;

	// Prepare the destination.
	if (!(fd2 = rfopen(to, "w")))
	{
		rfclose(fd1);
		return false;
	}

	// Prepare the buffer.

	// Loop through the from file through the buffer.
	while((l1 = rfread(buffer, 1, sizeof buffer, fd1)) > 0)
	{
		// Write the data to the destination file.
		size_t l2 = rfwrite(buffer, 1, l1, fd2);

		// Check if there was an error writing.
		if (l2 < l1)
			return false;
	}
	rfclose(fd1);
	rfclose(fd2);
	return true;
}

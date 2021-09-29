
#include <streams/file_stream.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "settings.h"
#include "settings.fdh"

#include "libretro_shared.h"

const uint16_t SETTINGS_VERSION = 0x1602;		// serves as both a version and magic

static Settings normal_settings;
Settings *settings = &normal_settings;

static bool tryload(Settings *setfile)
{
   char setfilename_tmp[1024];
   RFILE *fp = NULL;

   retro_create_path_string(setfilename_tmp, sizeof(setfilename_tmp), g_dir, "settings.dat");

   if (!(fp = filestream_open(setfilename_tmp, RETRO_VFS_FILE_ACCESS_READ, 
         RETRO_VFS_FILE_ACCESS_HINT_NONE)))
      return 1;

   setfile->version = 0;
   filestream_read(fp, setfile, sizeof(Settings));

   if (setfile->version != SETTINGS_VERSION)
      return 1;

   filestream_close(fp);
   return 0;
}

bool settings_load(Settings *setfile)
{
	if (!setfile)
		setfile = &normal_settings;

	if (tryload(settings))
	{
		memset(setfile, 0, sizeof(Settings));
		setfile->resolution = 2;		// 640x480 Windowed, should be safe value
		setfile->last_save_slot = 0;
		setfile->multisave = true;

		setfile->enable_debug_keys = false;
		setfile->sound_enabled = true;
		setfile->music_enabled = 1;	// both Boss and Regular music

		setfile->instant_quit = false;
		setfile->emulate_bugs = false;
		setfile->no_quake_in_hell = false;
		setfile->inhibit_fullscreen = false;
		setfile->files_extracted = false;

		// I found that 8bpp->32bpp blits are actually noticably faster
		// than 32bpp->32bpp blits on several systems I tested. Not sure why
		// but calling SDL_DisplayFormat seems to actually be slowing things
		// down. This goes against established wisdom so if you want it back on,
		// run "displayformat 1" in the console and restart.
		setfile->displayformat = false;

		return 1;
	}

	return 0;
}

bool settings_save(Settings *setfile)
{
   char setfilename_tmp[1024];
   RFILE *fp = NULL;

   if (!setfile)
      setfile = &normal_settings;

   retro_create_path_string(setfilename_tmp, sizeof(setfilename_tmp), g_dir, "settings.dat");

   fp = filestream_open(setfilename_tmp, RETRO_VFS_FILE_ACCESS_WRITE, 
         RETRO_VFS_FILE_ACCESS_HINT_NONE);
   if (!fp)
      return 1;

   setfile->version = SETTINGS_VERSION;
   filestream_write(fp, setfile, sizeof(Settings));
   filestream_close(fp);
   return 0;
}

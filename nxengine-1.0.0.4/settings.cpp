
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "settings.h"
#include "nx.h"
#include "replay.h"
#include "settings.fdh"

#include "libretro_shared.h"

const char *setfilename = "settings.dat";
const uint16_t SETTINGS_VERSION = 0x1602;		// serves as both a version and magic

Settings normal_settings;
Settings replay_settings;
Settings *settings = &normal_settings;


bool settings_load(Settings *setfile)
{
	if (!setfile) setfile = &normal_settings;
	
	if (tryload(settings))
	{
		NX_LOG("No saved settings; using defaults.\n");
		
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
	else
	{
		input_set_mappings(settings->input_mappings);
	}
	
	return 0;
}

/*
void c------------------------------() {}
*/

static bool tryload(Settings *setfile)
{
        char setfilename_tmp[1024];
	FILE *fp;

	NX_LOG("Loading settings...\n");

	retro_create_path_string(setfilename_tmp, sizeof(setfilename_tmp), g_dir, setfilename);
	
	fp = fopen(setfilename_tmp, "rb");
	if (!fp)
	{
		NX_ERR("Couldn't open file %s.\n", setfilename_tmp);
		return 1;
	}
	
	setfile->version = 0;
	fread(setfile, sizeof(Settings), 1, fp);
	if (setfile->version != SETTINGS_VERSION)
	{
		NX_ERR("Wrong settings version %04x.\n", setfile->version);
		return 1;
	}
	
	fclose(fp);
	return 0;
}


bool settings_save(Settings *setfile)
{
char setfilename_tmp[1024];
FILE *fp;

	if (!setfile)
		setfile = &normal_settings;

	retro_create_path_string(setfilename_tmp, sizeof(setfilename_tmp), g_dir, setfilename);
	
	NX_LOG("Writing settings...\n");
	fp = fopen(setfilename_tmp, "wb");
	if (!fp)
	{
		NX_ERR("Couldn't open file %s.\n", setfilename_tmp);
		return 1;
	}
	
	for(int i=0;i<INPUT_COUNT;i++)
		setfile->input_mappings[i] = input_get_mapping(i);
	
	setfile->version = SETTINGS_VERSION;
	fwrite(setfile, sizeof(Settings), 1, fp);
	
	fclose(fp);
	return 0;
}





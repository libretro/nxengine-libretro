
#include <SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "extract.fdh"
#include "../libretro/libretro_shared.h"
#include "../nx_logger.h"

static int extract_do(void)
{
   char filename[1024];
	FILE *fp;

	NX_LOG("= Extracting Files =\n");

	retro_create_path_string(filename, sizeof(filename), g_dir, "Doukutsu.exe");
	
	fp = fopen(filename, "rb");
	if (!fp)
	{
		NX_ERR("cannot find executable %s\n", filename);
		NX_ERR("Please put it and it's \"data\" directory\n");
		NX_ERR("into the same folder as this program.\n");
		return 1;
	}
	
	if (extract_pxt(fp)) return 1;
	if (extract_files(fp)) return 1;
	if (extract_stages(fp)) return 1;
	
	fclose(fp);
	return 0;
}


int extract_main()
{
	int result;

	introduction();
	
	result = extract_do();
	if (!result)
		conclusion();
	
	return result;
}


void introduction()
{
	NX_LOG("I need to extract some game data\n");
	NX_LOG("before I can start up for the first time.\n");
	NX_LOG("\n");
	NX_LOG("Before beginning, you should have the Aeon Genesis\n");
	NX_LOG("English translation of version 1.0.0.6, and drop");
	NX_LOG("Doukutsu.exe and it's \"data\" directory into the same");
	NX_LOG("folder as the \"nx.bin\" file you just ran.");
	NX_LOG("\n");
	NX_LOG("If you haven't done that yet, please press ESCAPE now\n");
	NX_LOG("and come back in a moment. Otherwise, you can\n");
	NX_LOG("press any other button to start the extraction.\n");
}

void conclusion()
{
	NX_LOG("Success!\n");
	NX_LOG("\n");
	NX_LOG("You can now remove the Doukutsu.exe file\n");
	NX_LOG("if you like, as it isn't needed anymore.\n");
	NX_LOG("Please leave the \"data\" directory though.\n");
	NX_LOG("\n");
	NX_LOG("Press any button to begin\n");
}

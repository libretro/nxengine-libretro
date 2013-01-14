
#include <SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "extract.fdh"
#include "libretro_shared.h"
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
	//findfiles(fp);
	//exit(1);
	
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

/*
void c------------------------------() {}
*/

//#define FINDFILES
#ifdef FINDFILES
static struct
{
	const char *filename;
	int headersize;
}
fileinfo[] =
{
	"endpic/credit01.bmp", 25,
	"endpic/credit02.bmp", 25,
	"endpic/credit03.bmp", 25,
	"endpic/credit04.bmp", 25,
	"endpic/credit05.bmp", 25,
	"endpic/credit06.bmp", 25,
	"endpic/credit07.bmp", 25,
	"endpic/credit08.bmp", 25,
	"endpic/credit09.bmp", 25,
	"endpic/credit10.bmp", 25,
	"endpic/credit11.bmp", 25,
	"endpic/credit12.bmp", 25,
	"endpic/credit14.bmp", 25,
	"endpic/credit15.bmp", 25,
	"endpic/credit16.bmp", 25,
	"endpic/credit17.bmp", 25,
	"endpic/credit18.bmp", 25,
	"endpic/pixel.bmp", 25,
	"wavetable.dat", 0,
	"org/access.org", 0,
	"org/balcony.org", 0,
	"org/balrog.org", 0,
	"org/breakdown.org", 0,
	"org/cemetary.org", 0,
	"org/charge.org", 0,
	"org/credits.org", 0,
	"org/egg.org", 0,
	"org/eyesofflame.org", 0,
	"org/fanfale1.org", 0,
	"org/fanfale2.org", 0,
	"org/fanfale3.org", 0,
	"org/gameover.org", 0,
	"org/geothermal.org", 0,
	"org/gestation.org", 0,
	"org/gravity.org", 0,
	"org/grasstown.org", 0,
	"org/hell.org", 0,
	"org/heroend.org", 0,
	"org/jenka1.org", 0,
	"org/jenka2.org", 0,
	"org/labyrinth.org", 0,
	"org/lastbattle.org", 0,
	"org/lastcave.org", 0,
	"org/meltdown2.org", 0,
	"org/oppression.org", 0,
	"org/oside.org", 0,
	"org/plant.org", 0,
	"org/pulse.org", 0,
	"org/quiet.org", 0,
	"org/run.org", 0,
	"org/safety.org", 0,
	"org/scorching.org", 0,
	"org/seal.org", 0,
	"org/theme.org", 0,
	"org/toroko.org", 0,
	"org/town.org", 0,
	"org/tyrant.org", 0,
	"org/waterway.org", 0,
	"org/white.org", 0,
	"org/zombie.org", 0,
	NULL
};

bool findfiles(FILE *exefp)
{
int i;
FILE *fpo;
int len;
uint32_t crc;

	fpo = fopen("/tmp/files.dat", "wb");
	crc_init();
	
	for(i=0;fileinfo[i].filename;i++)
	{
		uint32_t offset = findfile(fileinfo[i].filename, exefp, fileinfo[i].headersize, \
								&len, &crc);
		
		if (offset == 0)
		{
			NX_ERR("couldn't find file %s.\n", fileinfo[i].filename);
			return 1;
		}
		
		const char *headertable = "NULL";
		if (fileinfo[i].headersize)
		{
			headertable = strstr(fileinfo[i].filename, "pixel") ? \
					"pixel_header" : "credit_header";
		}
		
		fprintf(fpo, "\t\"%s\", 0x%06x, %d, 0x%08x, %s,\n",
					fileinfo[i].filename,
					offset, len, crc, headertable);
	}
	
	fclose(fpo);
	return 0;
}


uint32_t findfile(const char *fname, FILE *exefp, int headersize, \
					int *len_out, uint32_t *crc_out)
{
FILE *fp;
uint8_t *buffer;
uint32_t hit = 0;
int len;

	fp = fopen(fname, "rb");
	if (!fp)
	{
		NX_ERR("can't open %s\n", fname);
		return 0;
	}
	
	len = filesize(fp);
	buffer = (uint8_t *)malloc(len);
	
	len -= headersize;
	fseek(fp, headersize, SEEK_SET);
	fread(buffer, len, 1, fp);
	
	*crc_out = crc_calc(buffer, len);
	*len_out = len;
	
	NX_LOG("searching for '%s'; %d bytes\n", fname, len);
	
	int match = 0;
	fseek(exefp, 0, SEEK_SET);
	while(!feof(exefp))
	{
		uint8_t ch = fgetc(exefp);
recheck: ;
		
		if (ch == buffer[match])
		{
			match++;
			if (match >= len)
			{
				hit = (ftell(exefp) - len);
				NX_LOG("hit at 0x%06x\n", hit);
				match = 0;
			}
		}
		else if (match)
		{
			match = 0;
			goto recheck;
		}
	}
	
	free(buffer);
	fclose(fp);
	return hit;
}
#endif

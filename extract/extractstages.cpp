
#include <SDL/SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include "../graphics/safemode.h"
#include "../common/StringList.h"
#include "../stagedata.h"
#include "../maprecord.h"
#include "extractstages.fdh"

using safemode::moveto;
using safemode::status;
using safemode::print;
using safemode::run_until_key;

#define NMAPS			95
#define DATA_OFFSET		0x937B0

struct EXEMapRecord
{
	char tileset[32];
	char filename[32];
	int scroll_type;
	char background[32];
	char NPCset1[32];
	char NPCset2[32];
	uint8_t bossNo;
	char caption[35];
};

EXEMapRecord exemapdata[NMAPS];
MapRecord mapdata[NMAPS];

// the NPC set system isn't used by NXEngine, but the information
// is used in a few places to figure out which sprite to be drawn.
// for example Balrog when he appears in the Gum Room is supposed to be green.
const char *npcsetnames[] =
{
	"guest", "0", "eggs1", "ravil", "weed", "maze",
	"sand", "omg", "cemet", "bllg", "plant", "frog",
	"curly", "stream", "ironh", "toro", "x", "dark",
	"almo1", "eggs2", "twind", "moon", "cent", "heri",
	"red", "miza", "dr", "almo2", "kings", "hell",
	"press", "priest", "ballos", "island", NULL
};


bool extract_stages(FILE *exefp)
{
int i;

	status("[ stage.dat ]");
	
	// load raw data into struct
	fseek(exefp, DATA_OFFSET, SEEK_SET);
	fread(exemapdata, sizeof(EXEMapRecord), NMAPS, exefp);
	
	// convert the data
	memset(mapdata, 0, sizeof(mapdata));
	
	try
	{
		for(i=0;i<NMAPS;i++)
		{
			strcpy(mapdata[i].filename, exemapdata[i].filename);
			strcpy(mapdata[i].stagename, exemapdata[i].caption);
			
			mapdata[i].scroll_type = exemapdata[i].scroll_type;
			mapdata[i].bossNo = exemapdata[i].bossNo;
			
			mapdata[i].tileset = find_index(exemapdata[i].tileset, tileset_names);
			if (mapdata[i].tileset == -1) throw "tileset";
			
			mapdata[i].bg_no   = find_index(exemapdata[i].background, backdrop_names);
			if (mapdata[i].bg_no == -1) throw "backdrop";
			
			mapdata[i].NPCset1 = find_index(exemapdata[i].NPCset1, npcsetnames);
			if (mapdata[i].bg_no == -1) throw "NPCset1";
			
			mapdata[i].NPCset2 = find_index(exemapdata[i].NPCset2, npcsetnames);
			if (mapdata[i].bg_no == -1) throw "NPCset2";
		}
	}
	catch(const char *what)
	{
		status("didn't recognize map %s name", what);
		print("on stage %d", i);
		
		run_until_key();
		return 1;
	}
	
	// write out
	FILE *fpo = fopen("stage.dat", "wb");
	if (!fpo)
	{
		status("failed to open stage.dat for writing");
		run_until_key();
		return 1;
	}
	
	fputc(NMAPS, fpo);
	for(i=0;i<NMAPS;i++)
		fwrite(&mapdata[i], sizeof(MapRecord), 1, fpo);
	
	fclose(fpo);
	return 0;
}


static int find_index(const char *fname, const char *list[])
{
	for(int i=0;list[i];i++)
	{
		if (!strcasecmp(list[i], fname))
		{
			return i;
		}
	}
	
	return -1;
}





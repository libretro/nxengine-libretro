
#include "../sdl/include/LRSDL.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../common/StringList.h"
#include "../common/basics.h"
#include "../libretro/libretro_shared.h"
#include "../stagedata.h"
#include "../maprecord.h"
#include "extractstages.fdh"
#include "../nx_logger.h"

#ifdef _WIN32
#include "../libretro/msvc_compat.h"
#endif

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
MapRecord stages[MAX_STAGES];

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

static int find_index(const char *fname, const char *list[])
{
	for(int i=0;list[i];i++)
   {
      if (!strcasecmp(list[i], fname))
         return i;
   }
	
	return 0xff;
}

bool extract_stages(FILE *exefp)
{
   int i;
	// load raw data into struct
	fseek(exefp, DATA_OFFSET, SEEK_SET);
	fread(exemapdata, sizeof(EXEMapRecord), NMAPS, exefp);
	
	// convert the data
	memset(stages, 0, sizeof(stages));
	const char *error = NULL;
	
	for(i=0;i<NMAPS;i++)
	{
		strcpy(stages[i].filename, exemapdata[i].filename);
		strcpy(stages[i].stagename, exemapdata[i].caption);

#ifdef MSB_FIRST
		stages[i].scroll_type = (exemapdata[i].scroll_type << 24) | (exemapdata[i].scroll_type << 8 & 0x00FF0000) | (exemapdata[i].scroll_type >> 8 & 0x0000FF00) | (exemapdata[i].scroll_type >> 24);
#else
		stages[i].scroll_type = exemapdata[i].scroll_type;
#endif

		stages[i].bossNo = exemapdata[i].bossNo;
		
		stages[i].tileset = find_index(exemapdata[i].tileset, tileset_names);
		if (stages[i].tileset == 0xff) { error = "tileset"; break; }
		
		stages[i].bg_no   = find_index(exemapdata[i].background, backdrop_names);
		if (stages[i].bg_no == 0xff) { error = "backdrop"; break; }
		
		stages[i].NPCset1 = find_index(exemapdata[i].NPCset1, npcsetnames);
		if (stages[i].NPCset1 == 0xff) { error = "NPCset1"; break; }
		
		stages[i].NPCset2 = find_index(exemapdata[i].NPCset2, npcsetnames);
		if (stages[i].NPCset2 == 0xff) { error = "NPCset2"; break; }
	}

	if (error)
	{
		NX_ERR("didn't recognize map %s name\n", error);
		NX_ERR("on stage %d\n", i);
		
		return 1;
	}
	
	return 0;
}

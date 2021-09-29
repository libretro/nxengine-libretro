
#include "../sdl/include/LRSDL.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../common/basics.h"
#include "../libretro/libretro_shared.h"
#include "../stagedata.h"
#include "../maprecord.h"
#include "extractstages.fdh"

#ifdef _WIN32
#include "../libretro/msvc_compat.h"
#endif

#include <streams/file_stream.h>

#define _NMAPS			95
#define DATA_OFFSET		0x937B0

int64_t rftell(RFILE* stream);
int64_t rfseek(RFILE* stream, int64_t offset, int origin);
int64_t rfread(void* buffer,
		size_t elem_size, size_t elem_count, RFILE* stream);
int rfputc(int character, RFILE * stream);
int rfgetc(RFILE* stream);
int rfclose(RFILE* stream);
RFILE* rfopen(const char *path, const char *mode);
int rfprintf(RFILE * stream, const char * format, ...);
int64_t rfwrite(void const* buffer,
		size_t elem_size, size_t elem_count, RFILE* stream);

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

struct EXEMapRecord exemapdata[_NMAPS];
struct MapRecord stages[MAX_STAGES];

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
        int i;
	for(i=0;list[i];i++)
	{
		if (!strcasecmp(list[i], fname))
			return i;
	}

	return 0xff;
}

bool extract_stages(RFILE *exefp)
{
	int i;
	const char *error = NULL;

	// load raw data into struct
	rfseek(exefp, DATA_OFFSET, SEEK_SET);
	rfread(exemapdata, sizeof(struct EXEMapRecord), _NMAPS, exefp);

	// convert the data
	memset(stages, 0, sizeof(stages));

	for(i=0;i<_NMAPS;i++)
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
		return 1;

	return 0;
}

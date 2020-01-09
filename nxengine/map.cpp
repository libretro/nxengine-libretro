
#include "nx.h"
#include "map.h"
#include "map.fdh"
#include "libretro_shared.h"
#include "../extract-auto/cachefiles.h"

stMap map;

extern MapRecord stages[MAX_STAGES];
int num_stages;

#define MAX_BACKDROPS			32
NXSurface *backdrop[MAX_BACKDROPS];

// for FindObject--finding NPC's by ID2
Object *ID2Lookup[65536];

uint8_t tilecode[MAX_TILES];			// tile codes for every tile in current tileset
uint32_t tileattr[MAX_TILES];			// tile attribute bits for every tile in current tileset
uint32_t tilekey[MAX_TILES] = {0, 0, 64, 2, 2, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 32, 39, 48, 103, 34, 32, 33, 32, 32, 32, 32, 32, 33, 33, 33, 33, 544, 544, 544, 544, 544, 544, 544, 544, 32, 32, 32, 32, 32, 32, 32, 32, 160, 39, 176, 32, 34, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 672, 672, 672, 672, 672, 672, 672, 672, 32, 32, 32, 32, 32, 32, 32, 32, 256, 256, 256, 256, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 416, 416, 416, 416, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // mapping from tile codes -> tile attributes


// load stage "stage_no", this entails loading the map (pxm), enemies (pxe), tileset (pbm),
// tile attributes (pxa), and script (tsc).
bool load_stage(int stage_no)
{
char stage[MAXPATHLEN];
char fname[MAXPATHLEN];

char slash;
#ifdef _WIN32
slash = '\\';
#else
slash = '/';
#endif

	NX_LOG(" >> Entering stage %d: '%s'.\n", stage_no, stages[stage_no].stagename);
	game.curmap = stage_no;		// do it now so onspawn events will have it
	
	if (Tileset::Load(stages[stage_no].tileset))
		return 1;
	
	// get the base name of the stage without extension
	const char *mapname = stages[stage_no].filename;
	if (!strcmp(mapname, "lounge")) mapname = "Lounge";
	snprintf(stage, sizeof(stage), "%s%c%s", stage_dir, slash, mapname);
	
	snprintf(fname, sizeof(fname), "%s.pxm", stage);
	if (load_map(fname)) return 1;
	
	snprintf(fname, sizeof(fname), "%s%c%s.pxa", stage_dir, slash, tileset_names[stages[stage_no].tileset]);
	if (load_tileattr(fname)) return 1;
	
	snprintf(fname, sizeof(fname), "%s.pxe", stage);
	if (load_entities(fname)) return 1;
	
	snprintf(fname, sizeof(fname), "%s.tsc", stage);
	if (tsc_load(fname, SP_MAP) == -1) return 1;
	
	map_set_backdrop(stages[stage_no].bg_no);
	map.scrolltype = stages[stage_no].scroll_type;
	map.motionpos = 0;

   //hack to show nice backdrop in menu, like nicalis
   stages[0].bg_no=9;
   stages[0].scroll_type=BK_FASTLEFT_LAYERS;
	
	return 0;
}

/*
void c------------------------------() {}
*/

// load a PXM map
bool load_map(const char *fname)
{
	CFILE *fp;
	int x, y;

   NX_LOG("load_map: %s\n", fname);

	fp = copen(fname, "rb");
	if (!fp)
	{
		NX_ERR("load_map: no such file: '%s'\n", fname);
		return 1;
	}
	
	if (!cverifystring(fp, "PXM"))
	{
		NX_ERR("load_map: invalid map format: '%s'\n", fname);
		return 1;
	}
	
	memset(&map, 0, sizeof(map));
	
	cgetc(fp);
	map.xsize = cgeti(fp);
	map.ysize = cgeti(fp);
	
	if (map.xsize > MAP_MAXSIZEX || map.ysize > MAP_MAXSIZEY)
	{
		NX_ERR("load_map: map is too large -- size %dx%d but max is %dx%d\n", map.xsize, map.ysize, MAP_MAXSIZEX, MAP_MAXSIZEY);
		cclose(fp);
		return 1;
	}
	else
	{
		NX_LOG("load_map: level size %dx%d\n", map.xsize, map.ysize);
	}
	
	for(y=0;y<map.ysize;y++)
	for(x=0;x<map.xsize;x++)
	{
		map.tiles[x][y] = cgetc(fp);
	}
	
	cclose(fp);
	
	map.maxxscroll = (((map.xsize * TILE_W) - SCREEN_WIDTH) - 8) << CSF;
	map.maxyscroll = (((map.ysize * TILE_H) - SCREEN_HEIGHT) - 8) << CSF;
	
	NX_LOG("load_map: '%s' loaded OK! - %dx%d\n", fname, map.xsize, map.ysize);
	return 0;
}


// load a PXE (entity list for a map)
bool load_entities(const char *fname)
{
CFILE *fp;
int i;
int nEntities;

	// gotta destroy all objects before creating new ones
	Objects::DestroyAll(false);
	FloatText::ResetAll();

	NX_LOG("load_entities: reading in %s\n", fname);
	// now we can load in the new objects
	fp = copen(fname, "rb");
	if (!fp)
	{
		NX_ERR("load_entities: no such file: '%s'\n", fname);
		return 1;
	}
	
	if (!cverifystring(fp, "PXE"))
	{
		NX_ERR("load_entities: not a PXE: '%s'\n", fname);
		return 1;
	}
	
	cgetc(fp);
	nEntities = cgetl(fp);
	
	for(i=0;i<nEntities;i++)
	{
		int x = cgeti(fp);
		int y = cgeti(fp);
		int id1 = cgeti(fp);
		int id2 = cgeti(fp);
		int type = cgeti(fp);
		int flags = cgeti(fp);
		
		int dir = (flags & FLAG_FACES_RIGHT) ? RIGHT : LEFT;
		
		//lprintf(" %d:   [%d, %d]\t id1=%d\t id2=%d   Type %d   flags %04x\n", i, x, y, id1, id2, type, flags);
		
		// most maps have apparently garbage entities--invisible do-nothing objects??
		// i dunno but no point in spawning those...
		if (type || id1 || id2 || flags)
		{
			bool addobject = false;
			
			// check if object is dependent on a flag being set/not set
			if (flags & FLAG_APPEAR_ON_FLAGID)
			{
				if (game.flags[id1])
					addobject = true;
			}
			else if (flags & FLAG_DISAPPEAR_ON_FLAGID)
			{
				if (!game.flags[id1])
					addobject = true;
			}
			else
				addobject = true;
			
			if (addobject)
			{
				// hack for chests (can we do this elsewhere?)
				if (type == OBJ_CHEST_OPEN) y++;
            // hack for skydragon in Fall end cinematic
				if (type == OBJ_SKY_DRAGON && id2 == 230) y++;
				
				Object *o = CreateObject((x * TILE_W) << CSF, \
										 (y * TILE_H) << CSF, type,
										 0, 0, dir, NULL, CF_NO_SPAWN_EVENT);
				
				o->id1 = id1;
				o->id2 = id2;
				o->flags |= flags;
				
				ID2Lookup[o->id2] = o;
				
				// now that it's all set up, execute OnSpawn,
				// since we didn't do it in CreateObject.
				o->OnSpawn();
			}
		}
	}
	
	//NX_LOG("load_entities: loaded %d objects\n", nEntities);
	cclose(fp);
	return 0;
}

// loads a pxa (tileattr) file
bool load_tileattr(const char *fname)
{
CFILE *fp;
int i;
unsigned char tc;

	map.nmotiontiles = 0;

	NX_LOG("load_pxa: reading in %s\n", fname);
	fp = copen(fname, "rb");
	if (!fp)
	{
		NX_ERR("load_pxa: no such file: '%s'\n", fname);
		return 1;
	}
	
	for(i=0;i<256;i++)
	{
		tc = cgetc(fp);
		tilecode[i] = tc;
		tileattr[i] = tilekey[tc];
		//NX_LOG("Tile %02x   TC %02x    Attr %08x   tilekey[%02x] = %08x\n", i, tc, tileattr[i], tc, tilekey[tc]);
		
		//FIXME: Destroyable star tiles not showing up right now
		if (tc == 0x43)	// destroyable block - have to replace graphics
		{
			CopySpriteToTile(SPR_DESTROYABLE, i, 0, 0);
		}
		
		// add water currents to animation list
		if (tileattr[i] & TA_CURRENT)
		{
			map.motiontiles[map.nmotiontiles].tileno = i;
			map.motiontiles[map.nmotiontiles].dir = CVTDir(tc & 3);
			map.motiontiles[map.nmotiontiles].sprite = SPR_WATER_CURRENT;
			
			map.nmotiontiles++;
			NX_LOG("Added tile %02x to animation list, tc=%02x\n", i, tc);
		}
	}
	
	cclose(fp);
	return 0;
}

bool load_stages(void)
{
	num_stages = MAX_STAGES;
	
	return 0;
}


bool initmapfirsttime(void)
{
        char fname[1024];
	FILE *fp;
	int i;

	retro_create_path_string(fname, sizeof(fname), g_dir, "tilekey.dat");

	NX_LOG("initmapfirsttime: loading %s.\n", fname);
	if (!(fp = fopen(fname, "rb")))
	{
		NX_LOG("%s is missing, using default\n", fname);
	}
   else
   {
      for(i=0;i<256;i++)
         tilekey[i] = fgetl(fp);
      
      fclose(fp);
   }
	return load_stages();
}

void initmap(void)
{
	map_focus(NULL);
	map.parscroll_x = map.parscroll_y = 0;
}

/*
void c------------------------------() {}
*/

// backdrop_no 	- backdrop # to switch to
void map_set_backdrop(int backdrop_no)
{
	if (!LoadBackdropIfNeeded(backdrop_no))
		map.backdrop = backdrop_no;
}


void map_draw_backdrop(void)
{
int x, y;

	if (!backdrop[map.backdrop])
	{
		LoadBackdropIfNeeded(map.backdrop);
		if (!backdrop[map.backdrop])
			return;
	}
	
	switch(map.scrolltype)
	{
		case BK_FIXED:
			map.parscroll_x = 0;
			map.parscroll_y = 0;
		break;
		
		case BK_FOLLOWFG:
			map.parscroll_x = (map.displayed_xscroll >> CSF);
			map.parscroll_y = (map.displayed_yscroll >> CSF);
		break;
		
		case BK_PARALLAX:
			map.parscroll_y = (map.displayed_yscroll >> CSF) / 2;
			map.parscroll_x = (map.displayed_xscroll >> CSF) / 2;
		break;
		
		case BK_FASTLEFT:		// Ironhead
			map.parscroll_x += 6;
			map.parscroll_y = 0;
		break;
		
		case BK_FASTLEFT_LAYERS:
		case BK_FASTLEFT_LAYERS_NOFALLLEFT:
		{
			DrawFastLeftLayered();
			return;
		}
		break;
		
		case BK_HIDE:
		case BK_HIDE2:
		case BK_HIDE3:
		{
			if (game.curmap == STAGE_KINGS)		// intro cutscene
				ClearScreen(BLACK);
			else
				ClearScreen(DK_BLUE);
		}
		return;
		
		default:
			map.parscroll_x = map.parscroll_y = 0;
			NX_ERR("map_draw_backdrop: unhandled map scrolling type %d\n", map.scrolltype);
		break;
	}
	
	map.parscroll_x %= backdrop[map.backdrop]->Width();
	map.parscroll_y %= backdrop[map.backdrop]->Height();
	int w = backdrop[map.backdrop]->Width();
	int h = backdrop[map.backdrop]->Height();
	
	for(y=0;y<SCREEN_HEIGHT+map.parscroll_y; y+=h)
	{
		for(x=0;x<SCREEN_WIDTH+map.parscroll_x; x+=w)
		{
			DrawSurface(backdrop[map.backdrop], x - map.parscroll_x, y - map.parscroll_y);
		}
	}
}

// blit OSide's BK_FASTLEFT_LAYERS
static void DrawFastLeftLayered(void)
{
static const int layer_ys[] = { 80, 122, 145, 176, 240 };
static const int move_spd[] = { 0,    1,   2,   4,   8 };
const int nlayers = sizeof(layer_ys) / sizeof(layer_ys[0]);
int y1, y2;
int i, x;

	const int W = backdrop[map.backdrop]->Width();

	if (--map.parscroll_x <= -(W*2))
		map.parscroll_x = 0;
	
	y1 = x = 0;
	for(i=0;i<nlayers;i++)
	{
		y2 = layer_ys[i];
		
		if (i)	// not the static moon layer?
		{
			x = (map.parscroll_x * move_spd[i]) >> 1;
			x %= W;
		}
		
		BlitPatternAcross(backdrop[map.backdrop], x, y1, y1, (y2-y1)+1);
		y1 = (y2 + 1);
	}
}


// loads a backdrop into memory, if it hasn't already been loaded
static bool LoadBackdropIfNeeded(int backdrop_no)
{
char fname[MAXPATHLEN];
char slash;
#ifdef _WIN32
slash = '\\';
#else
slash = '/';
#endif
	// load backdrop now if it hasn't already been loaded
	if (!backdrop[backdrop_no])
	{
		// use chromakey (transparency) on bkwater, all others don't
		bool use_chromakey = (backdrop_no == 8);
		
		snprintf(fname, sizeof(fname), "%s%c%s.pbm", data_dir, slash, backdrop_names[backdrop_no]);
		
		backdrop[backdrop_no] = NXSurface::FromFile(fname, use_chromakey);
		if (!backdrop[backdrop_no])
		{
			NX_ERR("Failed to load backdrop '%s'\n", fname);
			return 1;
		}
	}
	
	return 0;
}

void map_flush_graphics()
{
int i;

	for(i=0;i<MAX_BACKDROPS;i++)
	{
		delete backdrop[i];
		backdrop[i] = NULL;
	}
	
	// re-copy star files
	for(i=0;i<256;i++)
	{
		if (tilecode[i] == 0x43)
		{
			CopySpriteToTile(SPR_DESTROYABLE, i, 0, 0);
		}
	}
}


/*
void c------------------------------() {}
*/

// draw rising/falling water from eg Almond etc
void map_drawwaterlevel(void)
{
// water_sfc: 16 tall at 0
// just under: 16 tall at 32
// main tile: 32 tall at 16 (yes, overlapping)
int water_x, water_y;

	if (!map.waterlevelobject)
		return;
	
	water_x = -(map.displayed_xscroll >> CSF);
	water_x %= SCREEN_WIDTH;
	
	water_y = (map.waterlevelobject->y >> CSF) - (map.displayed_yscroll >> CSF);
	
	// draw the surface and just under the surface
	BlitPatternAcross(backdrop[map.backdrop], water_x, water_y, 0, 16);
	water_y += 16;
	
	BlitPatternAcross(backdrop[map.backdrop], water_x, water_y, 32, 16);
	water_y += 16;
	
	// draw the rest of the pattern all the way down
	while(water_y < (SCREEN_HEIGHT-1))
	{
		BlitPatternAcross(backdrop[map.backdrop], water_x, water_y, 16, 32);
		water_y += 32;
	}
}


// draw the map.
// 	if foreground = TA_FOREGROUND, draws the foreground tile layer.
//  if foreground = 0, draws backdrop and background tiles.
void map_draw(uint8_t foreground)
{
int x, y;
int mapx, mapy;
int blit_x, blit_y, blit_x_start;
int scroll_x, scroll_y;
	
	scroll_x = (map.displayed_xscroll >> CSF);
	scroll_y = (map.displayed_yscroll >> CSF);
	
	mapx = (scroll_x / TILE_W);
	mapy = (scroll_y / TILE_H);
	
	blit_y = -(scroll_y % TILE_H);
	blit_x_start = -(scroll_x % TILE_W);
	
	// MAP_DRAW_EXTRA_Y etc is 1 if resolution is changed to
	// something not a multiple of TILE_H.
	for(y=0; y <= (SCREEN_HEIGHT / TILE_H)+MAP_DRAW_EXTRA_Y; y++)
	{
		blit_x = blit_x_start;
		
		for(x=0; x <= (SCREEN_WIDTH / TILE_W)+MAP_DRAW_EXTRA_X; x++)
		{
			int t = map.tiles[mapx+x][mapy+y];
			if ((tileattr[t] & TA_FOREGROUND) == foreground)
				draw_tile(blit_x, blit_y, t);
			
			blit_x += TILE_W;
		}
		
		blit_y += TILE_H;
	}
}


/*
void c------------------------------() {}
*/

// map scrolling code
void scroll_normal(void)
{
const int scroll_adj_rate = (0x2000 / map.scrollspeed);
	
	// how many pixels to let player stray from the center of the screen
	// before we start scrolling. high numbers let him reach closer to the edges,
	// low numbers keep him real close to the center.
	#define P_VARY_FROM_CENTER			(64 << CSF)
	
	if (player->dir == LEFT)
	{
		map.scrollcenter_x -= scroll_adj_rate;
		if (map.scrollcenter_x < -P_VARY_FROM_CENTER)
			map.scrollcenter_x = -P_VARY_FROM_CENTER;
	}
	else
	{
		map.scrollcenter_x += scroll_adj_rate;
		if (map.scrollcenter_x > P_VARY_FROM_CENTER)
			map.scrollcenter_x = P_VARY_FROM_CENTER;
	}
	
	// compute where the map "wants" to be
	map.target_x = (player->CenterX() + map.scrollcenter_x) - ((SCREEN_WIDTH / 2) << CSF);
	
	// Y scrolling
	if (player->lookscroll == UP)
	{
		map.scrollcenter_y -= scroll_adj_rate;
		if (map.scrollcenter_y < -P_VARY_FROM_CENTER) map.scrollcenter_y = -P_VARY_FROM_CENTER;
	}
	else if (player->lookscroll == DOWN)
	{
		map.scrollcenter_y += scroll_adj_rate;
		if (map.scrollcenter_y > P_VARY_FROM_CENTER) map.scrollcenter_y = P_VARY_FROM_CENTER;
	}
	else
	{
		if (map.scrollcenter_y <= -scroll_adj_rate)
		{
			map.scrollcenter_y += scroll_adj_rate;
		}
		else if (map.scrollcenter_y >= scroll_adj_rate)
		{
			map.scrollcenter_y -= scroll_adj_rate;
		}
	}
	
	map.target_y = (player->CenterY() + map.scrollcenter_y) - ((SCREEN_HEIGHT / 2) << CSF);
}

void map_scroll_do(void)
{
	bool doing_normal_scroll = false;
	
	if (!map.scroll_locked)
	{
		if (map.focus.has_target)
		{	// FON command
			// this check makes it so if we <FON on an object which
			// gets destroyed, the scroll stays locked at the last known
			// position of the object.
			if (map.focus.target)
			{
				Object *t = map.focus.target;
				
				// Generally we want to focus on the center of the object, not it's UL corner.
				// But a few objects (Cage in mimiga village) have offset drawpoints
				// that affect the positioning of the scene. If the object has a drawpoint,
				// we'll assume it's in an appropriate position, otherwise, we'll try to find
				// the center ourselves.
				if (sprites[t->sprite].frame[t->frame].dir[t->dir].drawpoint.equ(0, 0))
				{
					map.target_x = map.focus.target->CenterX() - ((SCREEN_WIDTH / 2) << CSF);
					map.target_y = map.focus.target->CenterY() - ((SCREEN_HEIGHT / 2) << CSF);
				}
				else
				{
					map.target_x = map.focus.target->x - ((SCREEN_WIDTH / 2) << CSF);
					map.target_y = map.focus.target->y - ((SCREEN_HEIGHT / 2) << CSF);
				}
			}
		}
		else
		{
			if (!player->hide)
			{
				scroll_normal();
				
            doing_normal_scroll = true;
			}
		}
	}
	
	map.real_xscroll += (map.target_x - map.real_xscroll) / map.scrollspeed;
	map.real_yscroll += (map.target_y - map.real_yscroll) / map.scrollspeed;
	
	map.displayed_xscroll = (map.real_xscroll + map.phase_adj);
	map.displayed_yscroll = map.real_yscroll;	// we don't compensate on Y, because player falls > 2 pixels per frame
	
	if (doing_normal_scroll)
	{
		run_phase_compensator();
	}
	else
	{
		map.phase_adj -= MAP_PHASE_ADJ_SPEED;
		if (map.phase_adj < 0) map.phase_adj = 0;
	}
	
	map_sanitycheck();
	
	// do quaketime after sanity check so quake works in
	// small levels like Shack.
	if (game.quaketime)
	{
		if (!map.scroll_locked)
		{
			int pushx, pushy;
			
			if (game.megaquaketime)		// Ballos fight
			{
				game.megaquaketime--;
				pushx = random(-5, 5) << CSF;
				pushy = random(-3, 3) << CSF;
			}
			else
			{
				pushx = random(-1, 1) << CSF;
				pushy = random(-1, 1) << CSF;
			}
			
			map.real_xscroll += pushx;
			map.real_yscroll += pushy;
			map.displayed_xscroll += pushx;
			map.displayed_yscroll += pushy;
		}
		else
		{
			// quake after IronH battle...special case cause we don't
			// want to show the walls of the arena.
			int pushy = random(-0x500, 0x500);
			
			map.real_yscroll += pushy;
			if (map.real_yscroll < 0) map.real_yscroll = 0;
			if (map.real_yscroll > (15 << CSF)) map.real_yscroll = (15 << CSF);
			
			map.displayed_yscroll += pushy;
			if (map.displayed_yscroll < 0) map.displayed_yscroll = 0;
			if (map.displayed_yscroll > (15 << CSF)) map.displayed_yscroll = (15 << CSF);
		}
		
		game.quaketime--;
	}
}

// this attempts to prevent jitter most visible when the player is walking on a
// long straight stretch. the jitter occurs because map.xscroll and player->x
// tend to be out-of-phase, and thus cross over pixel boundaries at different times.
// what we do here is try to tweak/fudge the displayed xscroll value by up to 512 subpixels
// (1 real pixel), so that it crosses pixel boundaries on exactly the same frame as
// the player does.
void run_phase_compensator(void)
{
	int displayed_phase_offs = (map.displayed_xscroll - player->x) % 512;
	
	if (displayed_phase_offs != 0)
	{
		int phase_offs = abs(map.real_xscroll - player->x) % 512;
		//debug("%d", phase_offs);
		
		// move phase_adj towards phase_offs; phase_offs is how far
		// out of sync we are with the player and so once we reach it
		// we will compensating exactly.
		if (map.phase_adj < phase_offs)
		{
			map.phase_adj += MAP_PHASE_ADJ_SPEED;
			if (map.phase_adj > phase_offs)
				map.phase_adj = phase_offs;
		}
		else
		{
			map.phase_adj -= MAP_PHASE_ADJ_SPEED;
			if (map.phase_adj < phase_offs)
				map.phase_adj = phase_offs;
		}
	}
}

/*
void c------------------------------() {}
*/


// scroll position sanity checking
void map_sanitycheck(void)
{
	#define MAP_BORDER_AMT		(8<<CSF)
	if (map.real_xscroll < MAP_BORDER_AMT) map.real_xscroll = MAP_BORDER_AMT;
	if (map.real_yscroll < MAP_BORDER_AMT) map.real_yscroll = MAP_BORDER_AMT;
	if (map.real_xscroll > map.maxxscroll) map.real_xscroll = map.maxxscroll;
	if (map.real_yscroll > map.maxyscroll) map.real_yscroll = map.maxyscroll;
	
	if (map.displayed_xscroll < MAP_BORDER_AMT) map.displayed_xscroll = MAP_BORDER_AMT;
	if (map.displayed_yscroll < MAP_BORDER_AMT) map.displayed_yscroll = MAP_BORDER_AMT;
	if (map.displayed_xscroll > map.maxxscroll) map.displayed_xscroll = map.maxxscroll;
	if (map.displayed_yscroll > map.maxyscroll) map.displayed_yscroll = map.maxyscroll;
}


void map_scroll_jump(int x, int y)
{
	map.target_x = x - ((SCREEN_WIDTH / 2) << CSF);
	map.target_y = y - ((SCREEN_HEIGHT / 2) << CSF);
	map.real_xscroll = map.target_x;
	map.real_yscroll = map.target_y;
	
	map.displayed_xscroll = map.real_xscroll;
	map.displayed_yscroll = map.real_yscroll;
	map.phase_adj = 0;
	
	map.scrollcenter_x = map.scrollcenter_y = 0;
	map_sanitycheck();
}

// lock the scroll in it's current position. the target position will not change,
// however if the scroll is moved off the target (really only a quake could do this)
// the map will still seek it's old position.
void map_scroll_lock(bool lockstate)
{
	map.scroll_locked = lockstate;
	if (lockstate)
	{	// why do we do this?
		map.real_xscroll = map.target_x;
		map.real_yscroll = map.target_y;
	}
}

// set the map focus and scroll speed.
// if o is specified, focuses on that object.
// if o is NULL, focuses on the player.
void map_focus(Object *o, int spd)
{
	map.focus.target = o;
	map.focus.has_target = (o != NULL);
	
	map.scrollspeed = spd;
	map.scroll_locked = false;
}

/*
void c------------------------------() {}
*/

// change tile at x,y into newtile while optionally spawning smoke clouds and boomflash
void map_ChangeTileWithSmoke(int x, int y, int newtile, int nclouds, bool boomflash, Object *push_behind)
{
	if (x < 0 || y < 0 || x >= map.xsize || y >= map.ysize)
		return;
	
	map.tiles[x][y] = newtile;
	
	int xa = ((x * TILE_W) + (TILE_W / 2)) << CSF;
	int ya = ((y * TILE_H) + (TILE_H / 2)) << CSF;
	SmokeXY(xa, ya, nclouds, TILE_W/2, TILE_H/2, push_behind);
	
	if (boomflash)
		effect(xa, ya, EFFECT_BOOMFLASH);
}



const char *map_get_stage_name(int mapno)
{
	if (mapno == STAGE_KINGS)
		return "";//Studio Pixel Presents";
	
	return stages[mapno].stagename;
}

// show map name for "ticks" ticks
void map_show_map_name()
{
	game.mapname_x = (SCREEN_WIDTH / 2) - (GetFontWidth(map_get_stage_name(game.curmap), 0) / 2);
	game.showmapnametime = 120;
}

void map_draw_map_name(void)
{
	if (game.showmapnametime)
	{
		font_draw(game.mapname_x, 84, map_get_stage_name(game.curmap), 0, &bluefont); // Workaround for avoiding diacritics not showing on map location names
		game.showmapnametime--;
	}
}


// animate all motion tiles
void AnimateMotionTiles(void)
{
int i;
int x_off, y_off;

	for(i=0;i<map.nmotiontiles;i++)
	{
		switch(map.motiontiles[i].dir)
		{
			case LEFT: y_off = 0; x_off = map.motionpos; break;
			case RIGHT: y_off = 0; x_off = (TILE_W - map.motionpos); break;
			
			case UP: x_off = 0; y_off = map.motionpos; break;
			case DOWN: x_off = 0; y_off = (TILE_H - map.motionpos); break;
			
			default: x_off = y_off = 0; break;
		}
		
		CopySpriteToTile(map.motiontiles[i].sprite, map.motiontiles[i].tileno, x_off, y_off);
	}
	
	map.motionpos += 2;
	if (map.motionpos >= TILE_W) map.motionpos = 0;
}


// attempts to find an object with id2 matching the given value else returns NULL
Object *FindObjectByID2(int id2)
{
	Object *result = ID2Lookup[id2];
	
	if (!result)
		NX_ERR("FindObjectByID2: no such object %04d\n", id2);
	
	return result;
}


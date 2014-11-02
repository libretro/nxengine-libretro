
// manages the tileset
#include "graphics.h"
#include "tileset.h"
#include "tileset.fdh"
#include "libretro_shared.h"
#include "../nx.h"

using namespace Graphics;

extern const char *tileset_names[];		// from stagedata.cpp
extern const char *stage_dir;			// from main

static NXSurface *tileset;
static int current_tileset = -1;

bool Tileset::Init()
{
	tileset = NULL;
	current_tileset = -1;
	return 0;
}

void Tileset::Close()
{
	delete tileset;
}

/*
void c------------------------------() {}
*/

// load the given tileset into memory, replacing any other tileset.
bool Tileset::Load(int new_tileset)
{
char fname[MAXPATHLEN];

	if (new_tileset != current_tileset)
	{
                //char fname_tmp[1024];
		if (tileset)
		{
			delete tileset;
			current_tileset = -1;
		}
		
#ifdef _WIN32
      char slash = '\\';
#else
      char slash = '/';
#endif
		snprintf(fname, sizeof(fname), "%s%cPrt%s.pbm", stage_dir, slash, tileset_names[new_tileset]);

		//retro_create_path_string(fname_tmp, sizeof(fname_tmp), g_dir, fname);

		NX_LOG("Tileset::Load: %s\n", fname);
		
		// always use SDL_DisplayFormat on tilesets; they need to come out of 8-bit
		// so that we can replace the destroyable star tiles without them palletizing.
		tileset = NXSurface::FromFile(fname, true);
		if (!tileset)
			return 1;
		
		current_tileset = new_tileset;
	}
	
	return 0;
}

// draw the given tile from the current tileset to the screen
void Tileset::draw_tile(int x, int y, int t)
{
	// 16 tiles per row on all tilesheet
	int srcx = (t % 16) * TILE_W;
	int srcy = (t / 16) * TILE_H;
	
	DrawSurface(tileset, x, y, srcx, srcy, TILE_W, TILE_H);
}

void Tileset::Reload()
{
	if (current_tileset != -1)
	{
		int tileset = current_tileset;
		current_tileset = -1;
		Load(tileset);
	}
}

/*
void c------------------------------() {}
*/

NXSurface *Tileset::GetSurface()
{
	return tileset;
}


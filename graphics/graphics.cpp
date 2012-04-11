// graphics routines
#include "SDL.h"
#include <stdlib.h>
#include "graphics.h"
#include "tileset.h"
#include "sprites.h"
#include "../dirnames.h"
#include "graphics.fdh"
#include "../libretro/libretro.h"

NXSurface *screen = NULL;			// created from SDL's screen
#define SCREEN_BPP 15				// the default if we can't get video info

const NXColor DK_BLUE(0, 0, 0x21);		// the popular dk blue backdrop color
const NXColor BLACK(0, 0, 0);			// pure black, only works if no colorkey
const NXColor CLEAR(0, 0, 0);			// the transparent/colorkey color

bool Graphics::init()
{
	if (SetResolution())
		return 1;

	if (Tileset::Init())
		return 1;

	if (Sprites::Init())
		return 1;

	return 0;
}

/*
void c------------------------------() {}
*/

#ifdef __LIBRETRO__
extern unsigned pitch;
#endif

bool Graphics::InitVideo()
{
	SDL_Surface *sdl_screen;

	stat("Graphics::InitVideo");
	if (screen)
		delete screen;

	sdl_screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, 0x1f << 10, 0x1f << 5, 0x1f << 0, 0);

	#ifdef __LIBRETRO__
	pitch = sdl_screen->pitch;
	#endif

	if (!sdl_screen)
	{
		staterr("Graphics::InitVideo: error setting video mode");
		return 1;
	}

	screen = new NXSurface(sdl_screen, false);
	return 0;
}

bool Graphics::FlushAll()
{
	stat("Graphics::FlushAll()");
	Sprites::FlushSheets();
	Tileset::Reload();
	map_flush_graphics();
	return font_reload();
}

bool Graphics::SetResolution()
{
	if (Graphics::InitVideo())
		return 1;

	if (Graphics::FlushAll())
		return 1;

	return 0;
}

/*
void c------------------------------() {}
*/

// copy a sprite into the current tileset.
// used to obtain the images for the star tiles (breakable blocks),
// and for animated motion tiles in Waterway.
void Graphics::CopySpriteToTile(int spr, int tileno, int offset_x, int offset_y)
{
	NXRect srcrect, dstrect;

	srcrect.x = (sprites[spr].frame[0].dir[0].sheet_offset.x + offset_x);
	srcrect.y = (sprites[spr].frame[0].dir[0].sheet_offset.y + offset_y);
	srcrect.w = TILE_W;
	srcrect.h = TILE_H;

	dstrect.x = (tileno % 16) * TILE_W;
	dstrect.y = (tileno / 16) * TILE_H;
	dstrect.w = TILE_W;
	dstrect.h = TILE_H;

	NXSurface *tileset = Tileset::GetSurface();
	NXSurface *spritesheet = Sprites::get_spritesheet(sprites[spr].spritesheet);

	if (tileset && spritesheet)
	{
		// blank out the old tile data with clear
		tileset->FillRect(&dstrect, CLEAR);

		// copy the sprite over
		BlitSurface(spritesheet, &srcrect, tileset, &dstrect);
	}
}


void Graphics::ShowLoadingScreen()
{
	NXSurface loading;
	char fname[MAXPATHLEN];

	sprintf(fname, "%s/Loading.pbm", data_dir);
	if (loading.LoadImage(fname))
		return;

	int x = (SCREEN_WIDTH / 2) - (loading.Width() / 2);
	int y = (SCREEN_HEIGHT / 2) - loading.Height();

	FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);
	DrawSurface(&loading, x, y);
	screen->Flip();
}

/*
void c------------------------------() {}
*/

// blit from one surface to another, just like SDL_BlitSurface.
void Graphics::BlitSurface(NXSurface *src, NXRect *srcrect, NXSurface *dst, NXRect *dstrect)
{
	dst->DrawSurface(src, dstrect->x, dstrect->y, srcrect->x, srcrect->y, srcrect->w, srcrect->h);
}

/*
void c------------------------------() {}
*/

// draw the entire surface to the screen at the given coordinates.
void Graphics::DrawSurface(NXSurface *src, int x, int y)
{
	screen->DrawSurface(src, x, y);
}


// blit the specified portion of the surface to the screen
void Graphics::DrawSurface(NXSurface *src, int dstx, int dsty, int srcx, int srcy, int wd, int ht)
{
	screen->DrawSurface(src, dstx, dsty, srcx, srcy, wd, ht);
}


// blit the specified surface across the screen in a repeating pattern
void Graphics::BlitPatternAcross(NXSurface *sfc, int x_dst, int y_dst, int y_src, int height)
{
	screen->BlitPatternAcross(sfc, x_dst, y_dst, y_src, height);
}

/*
void c------------------------------() {}
*/

void Graphics::DrawRect(int x1, int y1, int x2, int y2, NXColor color)
{
	screen->DrawRect(x1, y1, x2, y2, color);
}

void Graphics::FillRect(int x1, int y1, int x2, int y2, NXColor color)
{
	screen->FillRect(x1, y1, x2, y2, color);
}

void Graphics::DrawPixel(int x, int y, NXColor color)
{
	screen->DrawPixel(x, y, color.r, color.g, color.b);
}

/*
void c------------------------------() {}
*/

void Graphics::DrawRect(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b)
{
	screen->DrawRect(x1, y1, x2, y2, r, g, b);
}

void Graphics::FillRect(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b)
{
	screen->FillRect(x1, y1, x2, y2, r, g, b);
}

void Graphics::DrawPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
	screen->DrawPixel(x, y, r, g, b);
}

/*
void c------------------------------() {}
*/

void Graphics::set_clip_rect(int x, int y, int w, int h)
{
	screen->set_clip_rect(x, y, w, h);
}

void Graphics::set_clip_rect(NXRect *rect)
{
	screen->set_clip_rect(rect);
}

void Graphics::clear_clip_rect()
{
	screen->clear_clip_rect();
}

/*
void c------------------------------() {}
*/

// change the target surface of operation like DrawRect to something
// other than the screen.
void Graphics::SetDrawTarget(NXSurface *surface)
{
	screen = surface;
}









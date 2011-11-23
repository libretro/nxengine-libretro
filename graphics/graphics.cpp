
// graphics routines
#include <SDL/SDL.h>
#include <SDL/SDL_getenv.h>
#include <stdlib.h>
#include "graphics.h"
#include "tileset.h"
#include "sprites.h"
#include "../dirnames.h"
#include "graphics.fdh"
#include "../libsnes.hpp"

NXSurface *screen = NULL;			// created from SDL's screen
static NXSurface *drawtarget = NULL;		// target of DrawRect etc; almost always screen
int screen_bpp = 15;				// the default if we can't get video info

const NXColor DK_BLUE(0, 0, 0x21);		// the popular dk blue backdrop color
const NXColor BLACK(0, 0, 0);			// pure black, only works if no colorkey
const NXColor CLEAR(0, 0, 0);			// the transparent/colorkey color

static bool is_fullscreen = false;
static int current_res = -1;

bool Graphics::init(int resolution)
{
   screen_bpp = 16;

	if (SetResolution(resolution, false))
		return 1;

	if (Tileset::Init())
		return 1;

	if (Sprites::Init())
		return 1;

	return 0;
}

void Graphics::close()
{
	stat("Graphics::Close()");
	//SDL_ShowCursor(true);
}

/*
void c------------------------------() {}
*/

extern snes_environment_t snes_environ_cb;
bool Graphics::InitVideo()
{
	SDL_Surface *sdl_screen;

	stat("Graphics::InitVideo");
	if (drawtarget == screen) drawtarget = NULL;
	if (screen) delete screen;

	sdl_screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH*SCALE, SCREEN_HEIGHT*SCALE, screen_bpp,
         0x1f << 10, 0x1f << 5, 0x1f << 0, 0);

   unsigned pitch = sdl_screen->pitch;
   snes_environ_cb(SNES_ENVIRONMENT_SET_PITCH, &pitch);

	if (!sdl_screen)
	{
		staterr("Graphics::InitVideo: error setting video mode");
		return 1;
	}

	screen = new NXSurface(sdl_screen, false);
	if (!drawtarget) drawtarget = screen;
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

void Graphics::SetFullscreen(bool enable)
{
	if (is_fullscreen != enable)
	{
		is_fullscreen = enable;
		InitVideo();
		Graphics::FlushAll();
	}
}

// change the video mode to one of the available resolution codes, currently:
// 0 - 640x480, Fullscreen
// 1 - Windowed scale x1 (320x240)
// 2 - Windowed scale x2 (640x480)
// 3 - Windowed scale x3 (960x720)
bool Graphics::SetResolution(int r, bool restoreOnFailure)
{
	stat("Graphics::SetResolution(%d)", r);
	if (r == current_res)
		return 0;
	
	int old_res = current_res;
	int factor;
	
	if (r == 0)
	{
		is_fullscreen = true;
		factor = 2;
	}
	else
	{
		is_fullscreen = false;
		factor = r;
	}
	
	stat("Setting scaling %d and fullscreen=%s", factor, is_fullscreen ? "yes":"no");
	NXSurface::SetScale(factor);
	
	if (Graphics::InitVideo())
	{
		staterr("Switch to resolution %d failed!", r);
		
		if (restoreOnFailure)
		{
			staterr("Trying to recover old mode %d.", r, old_res);
			if (Graphics::SetResolution(old_res, false))
			{
				staterr("Fatal error: vidmode recovery failed!!!");
			}
		}
		
		return 1;
	}
	
	if (Graphics::FlushAll()) return 1;
	return 0;
}

// return a pointer to a null-terminated list of available resolutions.
const char **Graphics::GetResolutions()
{
static const char *res_str[]   =
{
   "Fullscreen",
   "320x240",
	"640x480",
   "960x720",
	NULL
};

	return res_str;
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
	drawtarget->Flip();
}

/*
void c------------------------------() {}
*/

// blit from one surface to another, just like SDL_BlitSurface.
void Graphics::BlitSurface(NXSurface *src, NXRect *srcrect, NXSurface *dst, NXRect *dstrect)
{
	dst->DrawSurface(src, dstrect->x, dstrect->y, \
					 srcrect->x, srcrect->y, srcrect->w, srcrect->h);
}

/*
void c------------------------------() {}
*/

// draw the entire surface to the screen at the given coordinates.
void Graphics::DrawSurface(NXSurface *src, int x, int y)
{
	drawtarget->DrawSurface(src, x, y);
}


// blit the specified portion of the surface to the screen
void Graphics::DrawSurface(NXSurface *src, \
						   int dstx, int dsty, int srcx, int srcy, int wd, int ht)
{
	drawtarget->DrawSurface(src, dstx, dsty, srcx, srcy, wd, ht);
}


// blit the specified surface across the screen in a repeating pattern
void Graphics::BlitPatternAcross(NXSurface *sfc, int x_dst, int y_dst, int y_src, int height)
{
	drawtarget->BlitPatternAcross(sfc, x_dst, y_dst, y_src, height);
}

/*
void c------------------------------() {}
*/

void Graphics::DrawRect(int x1, int y1, int x2, int y2, NXColor color)
{
	drawtarget->DrawRect(x1, y1, x2, y2, color);
}

void Graphics::FillRect(int x1, int y1, int x2, int y2, NXColor color)
{
	drawtarget->FillRect(x1, y1, x2, y2, color);
}

void Graphics::DrawPixel(int x, int y, NXColor color)
{
	drawtarget->DrawPixel(x, y, color);
}

/*
void c------------------------------() {}
*/

void Graphics::DrawRect(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b)
{
	drawtarget->DrawRect(x1, y1, x2, y2, r, g, b);
}

void Graphics::FillRect(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b)
{
	drawtarget->FillRect(x1, y1, x2, y2, r, g, b);
}

void Graphics::DrawPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b)
{
	drawtarget->DrawPixel(x, y, r, g, b);
}

/*
void c------------------------------() {}
*/

void Graphics::set_clip_rect(int x, int y, int w, int h)
{
	drawtarget->set_clip_rect(x, y, w, h);
}

void Graphics::set_clip_rect(NXRect *rect)
{
	drawtarget->set_clip_rect(rect);
}

void Graphics::clear_clip_rect()
{
	drawtarget->clear_clip_rect();
}

/*
void c------------------------------() {}
*/

// change the target surface of operation like DrawRect to something
// other than the screen.
void Graphics::SetDrawTarget(NXSurface *surface)
{
	drawtarget = surface;
}










// a limited and redundant graphics system which allows placing text on the screen
// in the case of startup errors or before the real data files are extracted.
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <stdarg.h>
#include "../common/basics.h"
#include "../common/BList.h"
#include "../input.h"
#include "nxsurface.h"
#include "graphics.h"
#include "safemode.h"
#include "safemode.fdh"

using namespace safemode;
using namespace Graphics;

static BList textlist;
struct TextRecord
{
	TextRecord();
	~TextRecord();
	
	int x, y;
	SDL_Surface *sfc;
};

static SDL_Surface *sdl_screen = NULL;
static TTF_Font *font;

extern const char *fontfile;		// from font.cpp
extern int pointsize[];				// from font.cpp

static int nexty = 0;
static int fontheight = 0;
static SDL_Color textcolor;
static bool initilized = false;
static bool have_status = false;

bool safemode::init()
{
	try
	{
		sdl_screen = NULL;
		if (screen) sdl_screen = screen->GetSDLSurface();
		if (!sdl_screen) throw "graphics not initilized";
		
		if (TTF_Init() < 0)
		{
			throw stprintf("couldn't initialize SDL_ttf: %s", TTF_GetError());
		}
		
		font = TTF_OpenFont(fontfile, pointsize[SCALE]);
		if (!font)
		{
			throw stprintf("couldn't open font: '%s': %s", fontfile, TTF_GetError());
		}
		
		static SDL_Color black;
		SDL_Surface *temp = TTF_RenderText_Solid(font, "M", black);
		if (temp)
		{
			fontheight = temp->h;
			SDL_FreeSurface(temp);
		}
		else
		{
			throw "initial render failed";
		}
		
		textcolor.r = 0x00;
		textcolor.g = 0xff;
		textcolor.b = 0x80;
		textlist.MakeEmpty();
		
		moveto(SM_UPPER_THIRD);
		initilized = true;
		
		return 0;
	}
	catch(const char *error)
	{
		staterr("safemode::init: failed startup: %s", error);
		sdl_screen = NULL;
		return 1;
	}
}

void safemode::close()
{
	if (initilized)
	{
		FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0);
		screen->Flip();
		
		clear();
		
		if (font) TTF_CloseFont(font);
		sdl_screen = NULL;
	}
}

/*
void c------------------------------() {}
*/

void safemode::moveto(int y)
{
	switch(y)
	{
		case SM_CENTER:
			nexty = ((SCREEN_HEIGHT * SCALE) / 2) - (fontheight / 2);
		break;
		
		case SM_UPPER_THIRD:
			nexty = ((SCREEN_HEIGHT * SCALE) / 4) - (fontheight / 2);
		break;
		
		case SM_LOWER_THIRD:
			nexty = ((SCREEN_HEIGHT * SCALE) / 4) - (fontheight / 2);
			nexty = (SCREEN_HEIGHT * SCALE) - nexty;
		break;
		
		case SM_MIDUPPER_Y:
			nexty = ((SCREEN_HEIGHT * SCALE) / 2) - (fontheight / 2);
			nexty -= (32 * SCALE);
		break;
		
		default:
			nexty = y;
		break;
	}
}

bool safemode::print(const char *fmt, ...)
{
va_list ar;
char buffer[128];
SDL_Surface *sfc;

	va_start(ar, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ar);
	va_end(ar);
	
	stat("safemode print: '%s'", buffer);
	
	if (buffer[0])
	{
		sfc = TTF_RenderText_Solid(font, buffer, textcolor);
		if (!sfc)
		{
			staterr("safemode::print: failed to render string: '%s'", fmt);
			return 1;
		}
		
		TextRecord *tr = new TextRecord;
		tr->x = (((SCREEN_WIDTH * SCALE) / 2) - (sfc->w / 2));
		tr->y = nexty;
		tr->sfc = sfc;
		textlist.AddItem(tr);
		run();
	}
	
	nexty += (fontheight + (1 * SCALE));
	return 0;
}

void safemode::clear()
{
	while(textlist.CountItems())
		delete (TextRecord *)textlist.RemoveItem(textlist.CountItems() - 1);
	
	moveto(SM_UPPER_THIRD);
}


void safemode::run()
{
	FillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x20, 0x20, 0x20);
	
	for(int i=0;;i++)
	{
		TextRecord *tr = (TextRecord *)textlist.ItemAt(i);
		if (!tr) break;
		
		SDL_Rect dstrect;
		dstrect.x = tr->x;
		dstrect.y = tr->y;
		
		SDL_BlitSurface(tr->sfc, NULL, sdl_screen, &dstrect);
	}
	
	SDL_Flip(sdl_screen);
}


int safemode::run_until_key(bool delay)
{
	stat("run_until_key()");
	uint32_t start = SDL_GetTicks();
	
	last_sdl_key = -1;
	do
	{
		run();
		input_poll();
		SDL_Delay(50);
		
		if (delay && (SDL_GetTicks() - start) < 500)
			last_sdl_key = -1;
	}
	while(last_sdl_key == -1);
	
	return last_sdl_key;
}


void safemode::status(const char *fmt, ...)
{
va_list ar;
char buffer[128];

	va_start(ar, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ar);
	va_end(ar);
	
	clearstatus();
	moveto(SM_CENTER);
	print("%s", buffer);
	
	have_status = true;
}

void safemode::clearstatus()
{
	if (have_status)
	{
		// remove all except the header
		while(textlist.CountItems() > 1)
			delete (TextRecord *)textlist.RemoveItem(textlist.CountItems() - 1);
		
		moveto(SM_CENTER);
		have_status = false;
	}
}

/*
void c------------------------------() {}
*/

TextRecord::TextRecord()
{
	sfc = NULL;
}

TextRecord::~TextRecord()
{
	if (sfc)
	{
		SDL_FreeSurface(sfc);
		sfc = NULL;
	}
}





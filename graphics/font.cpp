
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include "../nx.h"
#include "font.h"
#include "font.fdh"

static int text_draw(int x, int y, const char *text, int spacing=0, NXFont *font=&whitefont);

const char *fontfile = "font.ttf";
static SDL_Surface *sdl_screen = NULL;
static SDL_Surface *shadesfc = NULL;

static bool initilized = false;
static bool rendering = true;
static bool shrink_spaces = true;
static int fontheight = 0;

NXFont whitefont;
NXFont greenfont;
NXFont bluefont;		// used for "F3:Options" text on pause screen
NXFont shadowfont;		// white letters w/ drop shadow

// point sizes for each valid scaling factor
int pointsize[] = { -1,  8, 17, 26 };

/*
void c------------------------------() {}
*/

bool font_init(void)
{
TTF_Font *font;
bool error = false;

	// we'll be bypassing the NXSurface automatic scaling features
	// and drawing at the real resolution so we can get better-looking fonts.
	sdl_screen = screen->GetSDLSurface();
	
	if (TTF_Init() < 0)
	{
		staterr("Couldn't initialize SDL_ttf: %s", TTF_GetError());
		return 1;
	}
	
	font = TTF_OpenFont(fontfile, pointsize[SCALE]);
	if (!font)
	{
		staterr("Couldn't open font: '%s'", fontfile);
		return 1;
	}
	
	error |= whitefont.InitChars(font, 0xffffff);
	error |= greenfont.InitChars(font, 0x00ff80);
	error |= bluefont.InitChars(font, 0xa0b5de);
	error |= shadowfont.InitCharsShadowed(font, 0xffffff, 0x000000);
	error |= create_shade_sfc();
	
	TTF_CloseFont(font);
	if (error) return 1;
	
	fontheight = (whitefont.letters['M']->h / SCALE);
	initilized = true;
	return 0;
}

void font_close(void)
{
	
}

bool font_reload()
{
	if (!initilized) return 0;
	
	whitefont.free();
	greenfont.free();
	bluefont.free();
	shadowfont.free();
	
	return font_init();
}

/*
void c------------------------------() {}
*/

NXFont::NXFont()
{
	memset(letters, 0, sizeof(letters));
}

NXFont::~NXFont()
{
	free();
}

void NXFont::free()
{
	for(int i=0;i<NUM_LETTERS_RENDERED;i++)
	{
		if (letters[i]) SDL_FreeSurface(letters[i]);
		letters[i] = NULL;
	}
}

bool NXFont::InitChars(TTF_Font *font, uint32_t color)
{
SDL_Color fgcolor;
SDL_Surface *letter;

	fgcolor.r = (uint8_t)(color >> 16);
	fgcolor.g = (uint8_t)(color >> 8);
	fgcolor.b = (uint8_t)(color);
	
	char str[2];
	str[1] = 0;
	
	for(int i=1;i<NUM_LETTERS_RENDERED;i++)
	{
		str[0] = i;
		
		letter = TTF_RenderText_Solid(font, str, fgcolor);
		if (!letter)
		{
			staterr("Font::InitChars: failed to render character %d: %s", i, TTF_GetError());
			return 1;
		}
		
		letters[i] = SDL_DisplayFormat(letter);
		SDL_FreeSurface(letter);
	}
	
	return 0;
}

// create a font with a drop-shadow (used for "MNA" stage-name displays)
bool NXFont::InitCharsShadowed(TTF_Font *font, uint32_t color, uint32_t shadowcolor)
{
SDL_Color fgcolor, bgcolor;
SDL_Surface *top, *bottom;
SDL_Rect dstrect;
const int offset = 2;

	fgcolor.r = (uint8_t)(color >> 16);
	fgcolor.g = (uint8_t)(color >> 8);
	fgcolor.b = (uint8_t)(color);
	
	bgcolor.r = (uint8_t)(shadowcolor >> 16);
	bgcolor.g = (uint8_t)(shadowcolor >> 8);
	bgcolor.b = (uint8_t)(shadowcolor);
	
	char str[2];
	str[1] = 0;
	
	SDL_PixelFormat *format = sdl_screen->format;
	uint32_t transp = SDL_MapRGB(format, 255, 0, 255);
	
	for(int i=1;i<NUM_LETTERS_RENDERED;i++)
	{
		str[0] = i;
		
		top = TTF_RenderText_Solid(font, str, fgcolor);
		bottom = TTF_RenderText_Solid(font, str, bgcolor);
		if (!top || !bottom)
		{
			staterr("Font::InitCharsShadowed: failed to render character %d: %s", i, TTF_GetError());
			return 1;
		}
		
		letters[i] = SDL_CreateRGBSurface(SDL_SRCCOLORKEY, top->w, top->h+offset,
							format->BitsPerPixel, format->Rmask, format->Gmask,
							format->Bmask, format->Amask);
		if (!letters[i])
		{
			staterr("Font::InitCharsShadowed: failed to create surface for character %d: %s", i, SDL_GetError());
			return 1;
		}
		
		SDL_FillRect(letters[i], NULL, transp);
		SDL_SetColorKey(letters[i], SDL_SRCCOLORKEY, transp);
		
		dstrect.x = 0;
		dstrect.y = offset;
		SDL_BlitSurface(bottom, NULL, letters[i], &dstrect);
		
		dstrect.x = 0;
		dstrect.y = 0;
		SDL_BlitSurface(top, NULL, letters[i], &dstrect);
	}
	
	return 0;
}

/*
void c------------------------------() {}
*/

// draw a text string
static int text_draw(int x, int y, const char *text, int spacing, NXFont *font)
{
int orgx = x;
int i;
SDL_Rect dstrect;
	
	for(i=0;text[i];i++)
	{
		char ch = text[i];
		SDL_Surface *letter = font->letters[ch];
		
		if (ch == '=' && game.mode != GM_CREDITS)
		{
			if (rendering)
				draw_sprite((x/SCALE), (y/SCALE)+2, SPR_TEXTBULLET);
		}
		else if (rendering && ch != ' ' && letter)
		{
			// must set this every time, because SDL_BlitSurface overwrites
			// dstrect with final clipping rectangle.
			dstrect.x = x;
			dstrect.y = y;
			SDL_BlitSurface(letter, NULL, sdl_screen, &dstrect);
		}
		
		if (spacing != 0)
		{	// fixed spacing
			x += spacing;
		}
		else
		{	// variable spacing
			if (ch == ' ' && shrink_spaces)
			{	// 10.5 px for spaces - make smaller than they really are - the default
				x += (SCALE == 1) ? 5 : 10;
				if (i & 1) x++;
			}
			else
			{
				if (letter)
					x += letter->w;
			}
		}
	}
	
	// return the final width of the text drawn
	return (x - orgx);
}


int GetFontWidth(const char *text, int spacing, bool is_shaded)
{
int wd;

	if (spacing)
		return (strlen(text) * spacing);
	
	rendering = false;
	shrink_spaces = is_shaded;
	
	wd = text_draw(0, 0, text, spacing * SCALE);
	
	rendering = true;
	shrink_spaces = true;
	
	return (wd / SCALE);
}

int GetFontHeight()
{
	return fontheight;
}

/*
void c------------------------------() {}
*/

// create the shadesfc, used by font_draw_shaded. It's just a big long black surface
// with 50% per-surface alpha applied, that we can use to darken the background.
static bool create_shade_sfc(void)
{
	if (shadesfc)
		SDL_FreeSurface(shadesfc);
	
	int wd = (SCREEN_WIDTH * SCALE);
	int ht = whitefont.letters['M']->h;
	
	SDL_PixelFormat *format = sdl_screen->format;
	shadesfc = SDL_CreateRGBSurface(SDL_SRCALPHA | SDL_HWSURFACE, wd, ht,
							format->BitsPerPixel, format->Rmask, format->Gmask,
							format->Bmask, format->Amask);
	
	if (!shadesfc)
	{
		staterr("create_shade_sfc: failed to create surface");
		return 1;
	}
	
	SDL_FillRect(shadesfc, NULL, SDL_MapRGB(format, 0, 0, 0));
	SDL_SetAlpha(shadesfc, SDL_SRCALPHA, 128);
	
	return 0;
}


int font_draw(int x, int y, const char *text, int spacing, NXFont *font)
{
	x *= SCALE;
	y *= SCALE;
	spacing *= SCALE;
	
	return (text_draw(x, y, text, spacing, font) / SCALE);
}

// draw a text string with a 50% dark border around it
int font_draw_shaded(int x, int y, const char *text, int spacing, NXFont *font)
{
SDL_Rect srcrect, dstrect;
int wd;

	x *= SCALE;
	y *= SCALE;
	spacing *= SCALE;
	
	// get full-res width of final text
	rendering = false;
	shrink_spaces = false;
	
	srcrect.x = 0;
	srcrect.y = 0;
	srcrect.h = shadesfc->h;
	srcrect.w = text_draw(0, 0, text, spacing, font);
	
	rendering = true;
	
	// shade
	dstrect.x = x;
	dstrect.y = y;
	SDL_BlitSurface(shadesfc, &srcrect, sdl_screen, &dstrect);
	
	// draw the text on top as normal
	wd = text_draw(x, y, text, spacing, font);
	
	shrink_spaces = true;
	return (wd / SCALE);
}






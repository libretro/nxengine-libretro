
#ifndef _FONT_H
#define _FONT_H

#define NUM_FONT_LETTERS		256
#define NUM_LETTERS_RENDERED	256 // Allow usage of the other half of the font, containing the diacritics
#define FONT_DEFAULT_SPACING	5

class NXFont
{
public:
	NXFont();
	~NXFont();
	bool InitChars(SDL_Surface *font, uint32_t color);
	bool InitCharsShadowed(SDL_Surface *top, uint32_t color, uint32_t shadowcolor);
	void free();
	
	SDL_Surface *letters[NUM_FONT_LETTERS];
};


extern NXFont whitefont;
extern NXFont greenfont;
extern NXFont bluefont;		// used for "F3:Options" text on pause screen
extern NXFont shadowfont;	// white letters w/ drop shadow

bool font_init(void);
void font_close(void);
int font_draw(int x, int y, const char *text, int spacing=0, NXFont *font=&whitefont);
int font_draw_shaded(int x, int y, const char *text, int spacing=0, NXFont *font=&whitefont);

int GetFontWidth(const char *text, int spacing=0, bool is_shaded=false);
int GetFontHeight();

#endif


#ifndef _FONT_H
#define _FONT_H

// I don't want to needlessly include <SDL_ttf.h> in every file that
// includes this one so this forward declaration makes those modules
// that don't what a TTF_Font is shut up about InitChars(); however
// technically TTF_Font is a typedef, so if the including file knows
// the real declaration, it would bawk at this.
#ifndef SDL_TTF_VERSION
	struct TTF_Font;
#endif

#define NUM_FONT_LETTERS		256
#define NUM_LETTERS_RENDERED	128
#define FONT_DEFAULT_SPACING	5

class NXFont
{
public:
	NXFont();
	~NXFont();
	
	bool InitChars(TTF_Font *font, uint32_t color);
	bool InitCharsShadowed(TTF_Font *top, uint32_t color, uint32_t shadowcolor);
	
	bool InitBitmapChars(SDL_Surface *sheet, uint32_t fgcolor, uint32_t color);
	bool InitBitmapCharsShadowed(SDL_Surface *sheet, uint32_t fgcolor, uint32_t color, uint32_t shadowcolor);
	
	void free();
	
	SDL_Surface *letters[NUM_FONT_LETTERS];

private:
	void ReplaceColor(SDL_Surface *sfc, uint32_t oldcolor, uint32_t newcolor);
};


extern NXFont whitefont;
extern NXFont greenfont;
extern NXFont bluefont;		// used for "F3:Options" text on pause screen
extern NXFont shadowfont;	// white letters w/ drop shadow

int font_draw(int x, int y, const char *text, int spacing=0, NXFont *font=&whitefont);
int font_draw_shaded(int x, int y, const char *text, int spacing=0, NXFont *font=&whitefont);

int GetFontWidth(const char *text, int spacing=0, bool is_shaded=false);
int GetFontHeight();

bool ifont_init_bitmap_chars(SDL_Surface *sheet, uint32_t fgcolor, uint32_t color);
bool ifont_init(void);
int ifont_get_height(void);
int ifont_draw_shaded(int x, int y, const char *text, int spacing, NXFont *font);
int ifont_text_draw(int x, int y, const char *text, int spacing, NXFont *font);
void ifont_replace_color(SDL_Surface *sfc, uint32_t oldcolor, uint32_t newcolor);
bool ifont_create_shade_sfc(void);
void ifont_free(void);
int ifont_get_width(const char *text, int spacing, bool is_shaded);
bool ifont_reload(void);
bool ifont_bitmap_chars_shadowed(SDL_Surface *sheet, uint32_t fgcolor, uint32_t color, uint32_t shadowcolor);

#endif

#ifndef _SDL_WRAPPER_H
#define _SDL_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

int SSNES_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, Uint32 color);
int SSNES_SetColorKey (SDL_Surface *surface, Uint32 flag, Uint32 key);
SDL_bool SSNES_SetClipRect(SDL_Surface *surface, const SDL_Rect *rect);

#ifdef __cplusplus
}
#endif

#endif

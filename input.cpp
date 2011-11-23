#include "SDL.h"
#include "nx.h"
#include "libsnes.hpp"

uint8_t mappings[SDLK_LAST];

bool inputs[INPUT_COUNT];
bool lastinputs[INPUT_COUNT];
int last_sdl_key;


bool input_init(void)
{
	memset(inputs, 0, sizeof(inputs));
	memset(lastinputs, 0, sizeof(lastinputs));
	memset(mappings, 0xff, sizeof(mappings));

   mappings[SNES_DEVICE_ID_JOYPAD_LEFT]   = LEFTKEY;  
   mappings[SNES_DEVICE_ID_JOYPAD_RIGHT]  = RIGHTKEY;  
   mappings[SNES_DEVICE_ID_JOYPAD_UP]     = UPKEY;  
   mappings[SNES_DEVICE_ID_JOYPAD_DOWN]   = DOWNKEY;  
   
   mappings[SNES_DEVICE_ID_JOYPAD_B] = JUMPKEY;
   mappings[SNES_DEVICE_ID_JOYPAD_A] = FIREKEY;
   mappings[SNES_DEVICE_ID_JOYPAD_L] = PREVWPNKEY;
   mappings[SNES_DEVICE_ID_JOYPAD_R] = NEXTWPNKEY;
   mappings[SNES_DEVICE_ID_JOYPAD_X] = INVENTORYKEY;
   mappings[SNES_DEVICE_ID_JOYPAD_Y] = MAPSYSTEMKEY;
	
#if 0
	// default mappings
	mappings[SDLK_LEFT] = LEFTKEY;
	mappings[SDLK_RIGHT] = RIGHTKEY;
	mappings[SDLK_UP] = UPKEY;
	mappings[SDLK_DOWN] = DOWNKEY;

#ifdef PANDORA
	mappings[SDLK_PAGEDOWN] = JUMPKEY;
	mappings[SDLK_END] = FIREKEY;
	mappings[SDLK_RSHIFT] = PREVWPNKEY;
	mappings[SDLK_RCTRL] = NEXTWPNKEY;
	mappings[SDLK_HOME] = INVENTORYKEY;
	mappings[SDLK_PAGEUP] = MAPSYSTEMKEY;
#else
	mappings[SDLK_z] = JUMPKEY;
	mappings[SDLK_x] = FIREKEY;
	mappings[SDLK_a] = PREVWPNKEY;
	mappings[SDLK_s] = NEXTWPNKEY;
	mappings[SDLK_q] = INVENTORYKEY;
	mappings[SDLK_w] = MAPSYSTEMKEY;
#endif

	mappings[SDLK_ESCAPE] = ESCKEY;
	
	mappings[SDLK_F1] = F1KEY;
	mappings[SDLK_F2] = F2KEY;
	mappings[SDLK_F3] = F3KEY;
	mappings[SDLK_F4] = F4KEY;
	mappings[SDLK_F5] = F5KEY;
	mappings[SDLK_F6] = F6KEY;
	mappings[SDLK_F7] = F7KEY;
	mappings[SDLK_F8] = F8KEY;
	mappings[SDLK_F9] = F9KEY;
	mappings[SDLK_F10] = F10KEY;
	mappings[SDLK_F11] = F11KEY;
	mappings[SDLK_F12] = F12KEY;
	
	mappings[SDLK_SPACE] = FREEZE_FRAME_KEY;
	mappings[SDLK_c] = FRAME_ADVANCE_KEY;
	mappings[SDLK_v] = DEBUG_FLY_KEY;
#endif
	
	return 0;
}

// set the SDL key that triggers an input
void input_remap(int keyindex, int sdl_key)
{
	stat("input_remap(%d => %d)", keyindex, sdl_key);
	//int old_mapping = input_get_mapping(keyindex);
	//if (old_mapping != -1)
	//	mappings[old_mapping] = 0xff;
	//
	//mappings[sdl_key] = keyindex;
}

// get which SDL key triggers a given input
int input_get_mapping(int keyindex)
{
int i;

	for(i=0;i<=SDLK_LAST;i++)
	{
		if (mappings[i] == keyindex)
			return i;
	}
	
	return -1;
}

const char *input_get_name(int index)
{
static const char *input_names[] =
{
	"left", "right", "up", "down",
	"jump", "fire", "pervious wpn", "next wpn",
	"inventory", "map",
	"escape",
	"f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12",
	"freeze frame", "frame advance", "debug fly"
};

	if (index < 0 || index >= INPUT_COUNT)
		return "invalid";
	
	return input_names[index];
}

void input_set_mappings(int *array)
{
	//memset(mappings, 0xff, sizeof(mappings));
	//for(int i=0;i<INPUT_COUNT;i++)
	//	mappings[array[i]] = i;
}

/*
void c------------------------------() {}
*/

void input_poll(void)
{
   extern snes_input_state_t snes_input_state_cb;

   for (unsigned i = 0; i < 12; i++)
   {
      int ino = mappings[i];
      if (ino != 0xff) inputs[ino] = snes_input_state_cb(0,
            SNES_DEVICE_JOYPAD, 0, i);
   }

#if 0
	//static uint8_t shiftstates = 0;
	SDL_Event evt;
	int ino;
	int key;

	while(SDL_PollEvent(&evt))
	{
		switch(evt.type)
		{
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				{
					key = evt.key.keysym.sym;

					ino = mappings[key];
					if (ino != 0xff) inputs[ino] = (evt.type == SDL_KEYDOWN);

					if (evt.type == SDL_KEYDOWN)
					{
						if (Replay::IsPlaying() && ino <= LASTCONTROLKEY)
						{
							stat("user interrupt - stopping playback of replay");
							Replay::end_playback();
							memset(inputs, 0, sizeof(inputs));
							inputs[ino] = true;
						}

						last_sdl_key = key;
					}
				}
				break;

			case SDL_QUIT:
				game.running = false;
				break;
		}
	}
#endif
}

// keys that we don't want to send to the console
// even if the console is up.
static int IsNonConsoleKey(int key)
{
	static const int nosend[] = { SDLK_LEFT, SDLK_RIGHT, 0 };

	for(int i=0;nosend[i];i++)
		if (key == nosend[i])
			return true;

	return false;
}


void input_close(void)
{

}

/*
void c------------------------------() {}
*/

static const int buttons[] = { JUMPKEY, FIREKEY, 0 };

bool buttondown(void)
{
	for(int i=0;buttons[i];i++)
	{
		if (inputs[buttons[i]])
			return 1;
	}
	
	return 0;
}

bool buttonjustpushed(void)
{
	for(int i=0;buttons[i];i++)
	{
		if (inputs[buttons[i]] && !lastinputs[buttons[i]])
			return 1;
	}
	
	return 0;
}

bool justpushed(int k)
{
	return (inputs[k] && !lastinputs[k]);
}






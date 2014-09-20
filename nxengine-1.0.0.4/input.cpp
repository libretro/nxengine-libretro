
#include "nx.h"
#include "input.fdh"
#include "libretro.h"

// Should this be declared in libretro.h?
const char* get_environment_variable(const char *key);

#undef SDLK_LAST
#define SDLK_LAST 16

uint8_t mappings[SDLK_LAST];

bool inputs[INPUT_COUNT];
bool lastinputs[INPUT_COUNT];
int last_sdl_key;

// (Re)initialize input mappings. Called on startup and on environment variable
// changes.
bool input_init(void)
{
   memset(inputs, 0, sizeof(inputs));
   memset(lastinputs, 0, sizeof(lastinputs));
   memset(mappings, 0xff, sizeof(mappings));

   mappings[RETRO_DEVICE_ID_JOYPAD_LEFT]   = LEFTKEY;  
   mappings[RETRO_DEVICE_ID_JOYPAD_RIGHT]  = RIGHTKEY;  
   mappings[RETRO_DEVICE_ID_JOYPAD_UP]     = UPKEY;  
   mappings[RETRO_DEVICE_ID_JOYPAD_DOWN]   = DOWNKEY;  

   // Environment variable keys that we're interested in
   static const char *env_var_keys[] = {
      "nxengine_jump_button",
      "nxengine_fire_button",
      "nxengine_previous_weapon_button",
      "nxengine_next_weapon_button",
      "nxengine_map_button",
      "nxengine_inventory_button",
      "nxengine_options_menu_button",
      NULL
   };
   // Game actions corresponding to the above environment variables
   static const int game_actions[] = {
      JUMPKEY,
      FIREKEY,
      PREVWPNKEY,
      NEXTWPNKEY,
      MAPSYSTEMKEY,
      INVENTORYKEY,
      F3KEY,
   };

   // Iterate through each environment variable, parse their values into
   // retropad buttons, and map buttons to game actions
   for(int i = 0; env_var_keys[i]; i++)
   {
      const char *env_var = get_environment_variable(env_var_keys[i]);

      if(!env_var) {
         // No value returned, so do nothing
      } else if(!strcmp(env_var, "A")) {
         mappings[RETRO_DEVICE_ID_JOYPAD_A] = game_actions[i];
      } else if(!strcmp(env_var, "B")) {
         mappings[RETRO_DEVICE_ID_JOYPAD_B] = game_actions[i];
      } else if(!strcmp(env_var, "X")) {
         mappings[RETRO_DEVICE_ID_JOYPAD_X] = game_actions[i];
      } else if(!strcmp(env_var, "Y")) {
         mappings[RETRO_DEVICE_ID_JOYPAD_Y] = game_actions[i];
      } else if(!strcmp(env_var, "L")) {
         mappings[RETRO_DEVICE_ID_JOYPAD_L] = game_actions[i];
      } else if(!strcmp(env_var, "R")) {
         mappings[RETRO_DEVICE_ID_JOYPAD_R] = game_actions[i];
      } else if(!strcmp(env_var, "L2")) {
         mappings[RETRO_DEVICE_ID_JOYPAD_L2] = game_actions[i];
      } else if(!strcmp(env_var, "R2")) {
         mappings[RETRO_DEVICE_ID_JOYPAD_R2] = game_actions[i];
      } else if(!strcmp(env_var, "L3")) {
         mappings[RETRO_DEVICE_ID_JOYPAD_L3] = game_actions[i];
      } else if(!strcmp(env_var, "R3")) {
         mappings[RETRO_DEVICE_ID_JOYPAD_R3] = game_actions[i];
      } else if(!strcmp(env_var, "select")) {
         mappings[RETRO_DEVICE_ID_JOYPAD_SELECT] = game_actions[i];
      } else if(!strcmp(env_var, "start")) {
         mappings[RETRO_DEVICE_ID_JOYPAD_START] = game_actions[i];
      }
   }

   return 0;
}


// set the SDL key that triggers an input
void input_remap(int keyindex, int sdl_key)
{
}

// get which SDL key triggers a given input
int input_get_mapping(int keyindex)
{
int i;

	for(i=0;i<SDLK_LAST;i++)
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
}

/*
void c------------------------------() {}
*/

void input_poll(void)
{
   extern retro_input_state_t input_cb;

   for (unsigned i = 0; i < SDLK_LAST; i++)
   {
      int ino = mappings[i];

      if (ino != F3KEY)
      {
         if (ino != 0xff)
            inputs[ino] = input_cb(0, RETRO_DEVICE_JOYPAD, 0, i);
      }
      else
      {
         static bool old;
         bool input = input_cb(0, RETRO_DEVICE_JOYPAD, 0, i);
         inputs[ino] = input && !old;
         old = input;
      }
   }
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






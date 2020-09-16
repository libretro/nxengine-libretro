
#include "nx.h"
#include "input.fdh"
#include "libretro.h"

extern retro_log_printf_t log_cb;

unsigned int mappings[INPUT_COUNT];
bool inputs[INPUT_COUNT];
bool lastinputs[INPUT_COUNT];
int last_sdl_key;
unsigned controller_device;

bool input_init(void)
{
   return true;
}


// set the SDL key that triggers an input
void input_remap(int keyindex, int sdl_key)
{
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
   extern bool libretro_supports_bitmasks;
   bool joypad_bitmask = false;
   int16_t joypad_bits = 0;

   if(libretro_supports_bitmasks && (mappings[LEFTKEY] == RETRO_DEVICE_ID_JOYPAD_LEFT))
   {
      joypad_bits = input_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK);
      joypad_bitmask = true;
   }

   for (unsigned ino = 0; ino < F4KEY; ino++)
   {
      int rcode = mappings[ino];

      if (ino != F3KEY)
      {
         if (rcode != RETROK_DUMMY)
         {
            if(joypad_bitmask)
               inputs[ino] = joypad_bits & (1 << rcode) ? 1 : 0;
            else
               inputs[ino] = input_cb(0, controller_device, 0, rcode);
         }
      }
      else
      {
         static bool old;
         bool input = false;

         if(joypad_bitmask)
            input = joypad_bits & (1 << rcode) ? 1 : 0;
         else
            input = input_cb(0, controller_device, 0, rcode);

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






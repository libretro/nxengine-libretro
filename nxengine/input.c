
#include "input.h"
#include "libretro.h"

unsigned int mappings[INPUT_COUNT];
bool inputs[INPUT_COUNT];
bool lastinputs[INPUT_COUNT];
int last_sdl_key;
unsigned controller_device;

int16_t input_state_wrapper(unsigned port, unsigned device,
      unsigned index, unsigned id);

const char *input_get_name(int index)
{
   static const char *input_names[] =
   {
      "left", "right", "up", "down",
      "jump", "fire", "previous wpn", "next wpn",
      "inventory", "map",
      "escape",
      "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9", "f10", "f11", "f12",
      "freeze frame", "frame advance", "debug fly"
   };

   if (index < 0 || index >= INPUT_COUNT)
      return "invalid";

   return input_names[index];
}

/*
   void c------------------------------() {}
 */

void input_poll(void)
{
   extern bool libretro_supports_bitmasks;

   if(libretro_supports_bitmasks && (mappings[LEFTKEY] == RETRO_DEVICE_ID_JOYPAD_LEFT))
   {
      unsigned ino;
      int16_t joypad_bits = input_state_wrapper(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK);
      for (ino = 0; ino < F4KEY; ino++)
      {
         int rcode = mappings[ino];

         if (ino != F3KEY)
         {
            if (rcode != RETROK_DUMMY)
               inputs[ino] = joypad_bits & (1 << rcode) ? 1 : 0;
         }
         else
         {
            static bool old;
            bool input  = joypad_bits & (1 << rcode) ? 1 : 0;
            inputs[ino] = input && !old;
            old         = input;
         }
      }
   }
   else
   {
      unsigned ino;
      for (ino = 0; ino < F4KEY; ino++)
      {
         int rcode = mappings[ino];

         if (ino != F3KEY)
         {
            if (rcode != RETROK_DUMMY)
               inputs[ino] = input_state_wrapper(0, controller_device, 0, rcode);
         }
         else
         {
            static bool old;
            bool input  = input_state_wrapper(0, controller_device, 0, rcode);

            inputs[ino] = input && !old;
            old         = input;
         }
      }
   }

}

static const int buttons[] = { JUMPKEY, FIREKEY, 0 };

bool buttondown(void)
{
   int i;
   for(i=0;buttons[i];i++)
   {
      if (inputs[buttons[i]])
         return 1;
   }

   return 0;
}

bool buttonjustpushed(void)
{
   int i;
   for(i=0;buttons[i];i++)
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

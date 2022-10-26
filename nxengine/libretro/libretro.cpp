#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>

#include <libretro.h>
#include <streams/file_stream.h>
#include <file/file_path.h>
#include <string/stdstring.h>

#include "libretro_shared.h"
#include "../common/misc.fdh"
#include "../graphics/graphics.h"
#include "../input.fdh"
#include "../input.h"
#include "../nx.h"


void post_main();
bool run_main();

void *retro_frame_buffer;
unsigned retro_frame_buffer_width;
unsigned retro_frame_buffer_height;
unsigned retro_frame_buffer_pitch;

retro_log_printf_t log_cb;
static retro_video_refresh_t video_cb;
static retro_input_poll_t poll_cb;
retro_input_state_t input_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;

extern "C" bool libretro_supports_bitmasks = false;

static unsigned g_frame_cnt;

bool retro_60hz = true;
unsigned pitch;

extern bool pre_main(void);

unsigned retro_get_tick(void)
{
	return g_frame_cnt;
}

void retro_set_environment(retro_environment_t cb)
{
   struct retro_vfs_interface_info vfs_iface_info;
   bool no_content = true;

   environ_cb = cb;

   vfs_iface_info.required_interface_version = 1;
   vfs_iface_info.iface                      = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VFS_INTERFACE, &vfs_iface_info))
	   filestream_vfs_init(&vfs_iface_info);

   environ_cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_content);
}

unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{ }

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
   audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
   poll_cb  = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
   input_cb = cb;
}

void retro_get_system_info(struct retro_system_info *info)
{
   info->need_fullpath = true;
   info->valid_extensions = "exe";
   info->library_version = "1.0.0.6";
   info->library_name = "NXEngine";
   info->block_extract = false;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
   if (port != 0) return;

   memset(inputs, 0, sizeof(inputs));
   memset(lastinputs, 0, sizeof(lastinputs));
   memset(mappings, 0, sizeof(mappings));
   for (unsigned i = 0; i < INPUT_COUNT; ++i)
      mappings[i] = RETROK_DUMMY;

   if (device == RETRO_DEVICE_KEYBOARD) {
      // Use the original keybindings
      controller_device = RETRO_DEVICE_KEYBOARD;
      mappings[LEFTKEY]  = RETROK_LEFT;
      mappings[RIGHTKEY] = RETROK_RIGHT;
      mappings[UPKEY]    = RETROK_UP;
      mappings[DOWNKEY]  = RETROK_DOWN;

      mappings[JUMPKEY] = RETROK_z;
      mappings[FIREKEY] = RETROK_x;

      mappings[PREVWPNKEY] = RETROK_a;
      mappings[NEXTWPNKEY] = RETROK_s;

      mappings[INVENTORYKEY] = RETROK_q;
      mappings[MAPSYSTEMKEY] = RETROK_w;

      mappings[ESCKEY] = RETROK_ESCAPE;
      mappings[F1KEY]  = RETROK_F1;
      mappings[F2KEY]  = RETROK_F2;
      mappings[F3KEY]  = RETROK_F3;
   } else {
      // Use a joypad model in all other cases
      controller_device = RETRO_DEVICE_JOYPAD;
      mappings[LEFTKEY]  = RETRO_DEVICE_ID_JOYPAD_LEFT;
      mappings[RIGHTKEY] = RETRO_DEVICE_ID_JOYPAD_RIGHT;
      mappings[UPKEY]    = RETRO_DEVICE_ID_JOYPAD_UP;
      mappings[DOWNKEY]  = RETRO_DEVICE_ID_JOYPAD_DOWN;

      mappings[JUMPKEY] = RETRO_DEVICE_ID_JOYPAD_B;
      mappings[FIREKEY] = RETRO_DEVICE_ID_JOYPAD_A;

      mappings[PREVWPNKEY] = RETRO_DEVICE_ID_JOYPAD_L;
      mappings[NEXTWPNKEY] = RETRO_DEVICE_ID_JOYPAD_R;

      mappings[MAPSYSTEMKEY] = RETRO_DEVICE_ID_JOYPAD_X;
      mappings[INVENTORYKEY] = RETRO_DEVICE_ID_JOYPAD_START;

      mappings[F3KEY] = RETRO_DEVICE_ID_JOYPAD_SELECT;
   }

   // Declare the bindings to the frontend
   struct retro_input_descriptor desc[INPUT_COUNT+1];
   unsigned j = 0;
   for (unsigned i = 0; i < INPUT_COUNT; ++i)
   {
      if (mappings[i] != RETROK_DUMMY)
      {
         desc[j].port        = 0;
	 desc[j].device      = controller_device;
	 desc[j].index       = 0;
	 desc[j].id          = mappings[i];
	 desc[j].description = input_get_name(i);
	 j++;
      }
   }

   desc[j].port        = 0;
   desc[j].device      = 0; 
   desc[j].index       = 0;
   desc[j].id          = 0;
   desc[j].description = NULL; 

   environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
   info->geometry.base_width   = SCREEN_WIDTH;
   info->geometry.base_height  = SCREEN_HEIGHT;
   info->geometry.max_width    = SCREEN_WIDTH; 
   info->geometry.max_height   = SCREEN_HEIGHT;
   info->geometry.aspect_ratio = (double)SCREEN_WIDTH / (double)SCREEN_HEIGHT;
   info->timing.fps            = 60.0;
   info->timing.sample_rate    = 22050.0;

}

static void check_system_specs(void)
{
   // TODO - when it starts reliably running at fullspeed on PSP, set to 4
   unsigned level = 5;
   environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);
}

void retro_init(void)
{
   struct retro_log_callback log;
   enum retro_pixel_format rgb565;

   if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
      log_cb = log.log;
   else
      log_cb = NULL;

   // initialize joypad mappings
   retro_set_controller_port_device(0, 1);

#if defined(FRONTEND_SUPPORTS_RGB565) || defined(ABGR1555)
   rgb565 = RETRO_PIXEL_FORMAT_RGB565;
   if(environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &rgb565) && log_cb)
      log_cb(RETRO_LOG_INFO, "Frontend supports RGB565 - will use that instead of XRGB1555.\n");
#endif
   check_system_specs();

   if (environ_cb(RETRO_ENVIRONMENT_GET_INPUT_BITMASKS, NULL))
      libretro_supports_bitmasks = true;
}

static void extract_directory(char *out_dir, const char *in_path, size_t size)
{
   size_t len;

   fill_pathname_parent_dir(out_dir, in_path, size);

   /* Remove trailing slash, if required */
   len = strlen(out_dir);
   if ((len > 0) &&
       (out_dir[len - 1] == PATH_DEFAULT_SLASH_C()))
      out_dir[len - 1] = '\0';

   /* If parent directory is an empty string,
    * must set it to '.' */
   if (string_is_empty(out_dir))
      strlcpy(out_dir, ".", size);
}

bool retro_load_game(const struct retro_game_info *game)
{
   g_dir[0] = '\0';

   if (game)
      extract_directory(g_dir, game->path, sizeof(g_dir));
   else
   {
      const char *system_dir = NULL;
      bool game_file_exists  = false;
      char game_file[1024];

      game_file[0] = '\0';

      /* Get system directory */
      if (environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &system_dir) &&
          system_dir)
      {
         fill_pathname_join(g_dir, system_dir, "nxengine", sizeof(g_dir));
         fill_pathname_join(game_file, g_dir, "Doukutsu.exe", sizeof(game_file));
         game_file_exists = path_is_valid(game_file);
      }

      if (!game_file_exists)
      {
         unsigned msg_interface_version = 0;
         environ_cb(RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION,
               &msg_interface_version);

         if (msg_interface_version >= 1)
         {
            struct retro_message_ext msg = {
               "NXEngine game files missing from frontend system directory",
               3000,
               3,
               RETRO_LOG_ERROR,
               RETRO_MESSAGE_TARGET_ALL,
               RETRO_MESSAGE_TYPE_NOTIFICATION,
               -1
            };
            environ_cb(RETRO_ENVIRONMENT_SET_MESSAGE_EXT, &msg);
         }
         else
         {
            struct retro_message msg = {
               "NXEngine game files missing from frontend system directory",
               180
            };
            environ_cb(RETRO_ENVIRONMENT_SET_MESSAGE, &msg);
         }

         return false;
      }
   }

   NX_LOG("g_dir: %s\n", g_dir);

   retro_init_saves();

   if (pre_main())
      return false;
   return true;
}

void retro_deinit(void)
{
   libretro_supports_bitmasks = false;
}

void retro_reset(void)
{
	lastinputs[F2KEY] = true;
	game.reset();
}

extern "C" void mixaudio(int16_t *stream, size_t len_samples);

void retro_run(void)
{
   poll_cb();
   static unsigned frame_cnt = 0;

   screen->Flip();

   if (retro_60hz)
   {
      while (!run_main());
      video_cb(retro_frame_buffer, retro_frame_buffer_width, retro_frame_buffer_height, retro_frame_buffer_pitch);
      frame_cnt++;
   }
   else
   {
      if (frame_cnt % 6)
      {
         while (!run_main());
         video_cb(retro_frame_buffer, retro_frame_buffer_width, retro_frame_buffer_height, retro_frame_buffer_pitch);
      }
      else
         video_cb(NULL, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH * sizeof(uint16_t)); // Dupe every 6th frame.

      frame_cnt++;
   }

   int16_t samples[(2 * 22050) / 60 + 1] = {0};

   // Average audio frames / video frame: 367.5.
   unsigned frames = (22050 + (frame_cnt & 1 ? 30 : -30)) / 60;

   mixaudio(samples, frames * 2);
   audio_batch_cb(samples, frames);

   g_frame_cnt++;

   if (!game.running) environ_cb(RETRO_ENVIRONMENT_SHUTDOWN, NULL);
}

size_t retro_serialize_size(void)
{
   return 0;
}

bool retro_serialize(void *data, size_t size)
{
   return false;
}

bool retro_unserialize(const void *data, size_t size)
{
   return false;
}

void retro_cheat_reset(void) {}
void retro_cheat_set(unsigned index, bool enabled, const char *code) {}

bool retro_load_game_special(
  unsigned game_type,
  const struct retro_game_info *info, size_t num_info
)
{
   return false;
}

void retro_unload_game (void)
{
   post_main();
}

unsigned retro_get_region(void)
{
   return RETRO_REGION_NTSC;
}

void * retro_get_memory_data(unsigned id) { return 0; }
size_t retro_get_memory_size(unsigned id) { return 0; }

/**
 * Retrieve the desired save directory.
 */
extern "C" const char* retro_get_save_dir(void)
{
   const char* dir = NULL;

   // Attempt to get the save directory from the frontend.
   if (environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &dir) && dir && *dir)
      return dir;

   // If the save directory isn't available, use the game path.
   return g_dir;
}

extern "C" int16_t input_state_wrapper(unsigned port, unsigned device,
      unsigned index, unsigned id)
{
	return input_cb(port, device, index, id);
}

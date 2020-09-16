#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <stdio.h>

#include <libretro.h>
#include <streams/file_stream.h>

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

bool libretro_supports_bitmasks = false;

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
   environ_cb = cb;

   vfs_iface_info.required_interface_version = 1;
   vfs_iface_info.iface                      = NULL;
   if (environ_cb(RETRO_ENVIRONMENT_GET_VFS_INTERFACE, &vfs_iface_info))
	   filestream_vfs_init(&vfs_iface_info);
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

char g_dir[1024];

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
   info->geometry.base_width = SCREEN_WIDTH;
   info->geometry.base_height = SCREEN_HEIGHT;
   info->geometry.max_width = SCREEN_WIDTH; 
   info->geometry.max_height = SCREEN_HEIGHT;
   info->timing.fps = 60.0;
   info->timing.sample_rate = 22050.0;
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

#ifdef FRONTEND_SUPPORTS_RGB565
   rgb565 = RETRO_PIXEL_FORMAT_RGB565;
   if(environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &rgb565) && log_cb)
      log_cb(RETRO_LOG_INFO, "Frontend supports RGB565 - will use that instead of XRGB1555.\n");
#endif
   check_system_specs();

   if (environ_cb(RETRO_ENVIRONMENT_GET_INPUT_BITMASKS, NULL))
      libretro_supports_bitmasks = true;
}

static void extract_directory(char *buf, const char *path, size_t size)
{
   char *base;
   strncpy(buf, path, size - 1);
   buf[size - 1] = '\0';

   base = strrchr(buf, '/');
   if (!base)
      base = strrchr(buf, '\\');

   if (base)
      *base = '\0';
   else
   {
      buf[0] = '.';
      buf[1] = '\0';
   }
}

bool retro_load_game(const struct retro_game_info *game)
{
   if (!game)
      return false;

   extract_directory(g_dir, game->path, sizeof(g_dir));
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

void mixaudio(int16_t *stream, size_t len_samples);

#if 0
#include <time.h>
static int64_t get_usec(void)
{
   struct timespec tv;
   clock_gettime(CLOCK_MONOTONIC, &tv);
   return (int64_t)tv.tv_sec * 1000000 + (int64_t)tv.tv_nsec / 1000;
}
#endif

void retro_run(void)
{
   poll_cb();
   static unsigned frame_cnt = 0;

#if 0
   if (log_cb)
      log_cb(RETRO_LOG_INFO, "[NX]: Start frame.\n");
   int64_t start_time = get_usec();
   
	platform_sync_to_vblank();
#endif
	screen->Flip();

   if (retro_60hz)
   {
      //int64_t start_time_frame = get_usec();
      while (!run_main());
#if 0
      int64_t total_time_frame = get_usec() - start_time_frame;
      if (log_cb)
         log_cb(RETRO_LOG_INFO, "[NX]: total_time_frame took %lld usec.\n", (long long)total_time_frame);
#endif

      //int64_t start_time_frame_cb = get_usec();
      video_cb(retro_frame_buffer, retro_frame_buffer_width, retro_frame_buffer_height, retro_frame_buffer_pitch);
#if 0
      int64_t total_time_frame_cb = get_usec() - start_time_frame_cb;
      if (log_cb)
         log_cb(RETRO_LOG_INFO, "[NX]: total_time_frame_cb took %lld usec.\n", (long long)total_time_frame_cb);
#endif

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

#if 0
   int64_t total_time = get_usec() - start_time;
   if (log_cb)
      log_cb(RETRO_LOG_INFO, "[NX]: Frame took %lld usec.\n", (long long)total_time);
#endif

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

void retro_create_subpath_string(char *fname, size_t fname_size, const char * dir, const char * subdir, const char * filename)
{
#ifdef _WIN32
	char slash = '\\';
#else
	char slash = '/';
#endif
	snprintf(fname, fname_size, "%s%c%s%c%s", dir, slash, subdir, slash, filename);
}

void retro_create_path_string(char *fname, size_t fname_size, const char * dir, const char * filename)
{
#ifdef _WIN32
	char slash = '\\';
#else
	char slash = '/';
#endif
	snprintf(fname, fname_size, "%s%c%s", dir, slash, filename);
}

/**
 * Retrieve the desired save directory.
 */
const char* retro_get_save_dir()
{
   const char* dir = NULL;

   // Attempt to get the save directory from the frontend.
   if (environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &dir) && dir && *dir) {
      return dir;
   }

   // If the save directory isn't available, use the game path.
   return g_dir;
}

/**
 * Copy any missing profiles from the content directory to the save directory.
 */
void retro_init_saves()
{
   // Copy any profiles into the save directory.
   const char* save_dir = retro_get_save_dir();
   char gamedirProfile[1024];
   char savedirProfile[1024];
   char profile_name[1024];

   // Copy profiles only if te folders are different.
   if (strcmp(save_dir, g_dir) != 0) {
      // Parse through all the different profiles.
      for (int i = 0; i < 5; i++) {
         // Create the profile filename.
         if (i == 0) {
            snprintf(profile_name, sizeof(profile_name), "profile.dat");
         }
         else {
            snprintf(profile_name, sizeof(profile_name), "profile%d.dat", i + 1);
         }

         // Get the profile's file path in the game directory.
         retro_create_path_string(gamedirProfile, sizeof(gamedirProfile), g_dir, profile_name);

         // Make sure the profile exists.
         if (file_exists(gamedirProfile)) {
            // Create the profile's file path in the save directory.
            retro_create_path_string(savedirProfile, sizeof(savedirProfile), save_dir, profile_name);

            // Copy the file to the save directory only if it doesn't exist.
            if (!file_exists(savedirProfile)) {
               if (retro_copy_file(gamedirProfile, savedirProfile)) {
                  NX_LOG("Copied profile %s to save directory at %s\n", gamedirProfile, savedirProfile);
               }
               else {
                  NX_ERR("Failed to copy profile %s to %s\n", gamedirProfile, savedirProfile);
               }
            }
         }
      }
   }
}

/**
 * Copy a file to the given destination.
 */
bool retro_copy_file(const char* from, const char* to)
{
   // Open the file for reading.
   FILE *fd1 = fopen(from, "r");
   if (!fd1) {
      return false;
   }

   // Prepare the destination.
   FILE *fd2 = fopen(to, "w");
   if(!fd2) {
      fclose(fd1);
      return false;
   }

   // Prepare the buffer.
   size_t l1;
   unsigned char buffer[8192];

   // Loop through the from file through the buffer.
   while((l1 = fread(buffer, 1, sizeof buffer, fd1)) > 0) {
      // Write the data to the destination file.
      size_t l2 = fwrite(buffer, 1, l1, fd2);

      // Check if there was an error writing.
      if (l2 < l1) {
         // Display an error message.
         if (ferror(fd2)) {
            NX_ERR("Error copying profile from %s to %s\n", from, to);
         }
         else {
            NX_ERR("Error copying profile, media full from %s to %s\n", from, to);
         }
         return false;
      }
   }
   fclose(fd1);
   fclose(fd2);
   return true;
}

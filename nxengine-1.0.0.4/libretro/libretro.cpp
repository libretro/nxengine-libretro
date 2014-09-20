#ifdef _WIN32
#include "msvc_compat.h"
#else
#include <unistd.h>
#endif
#include <string>

#include "libretro.h"
#include "../graphics/graphics.h"
#include "../nx.h"

void post_main();
bool run_main();
bool input_init(void);

void *retro_frame_buffer;
unsigned retro_frame_buffer_width;
unsigned retro_frame_buffer_height;
unsigned retro_frame_buffer_pitch;

static retro_log_printf_t log_cb;
static retro_video_refresh_t video_cb;
static retro_input_poll_t poll_cb;
retro_input_state_t input_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;

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
   environ_cb = cb;

   struct retro_variable variables[] = {
      { "nxengine_jump_button",
         "Jump Button; B|X|Y|L|R|L2|R2|L3|R3|select|start|A" },
      { "nxengine_fire_button",
         "Fire Button; Y|L|R|L2|R2|L3|R3|select|start|A|B|X" },
      { "nxengine_previous_weapon_button",
         "Previous Weapon Button; L|R|L2|R2|L3|R3|select|start|A|B|X|Y" },
      { "nxengine_next_weapon_button",
         "Next Weapon Button; R|L2|R2|L3|R3|select|start|A|B|X|Y|L" },
      { "nxengine_map_button",
         "Map Button; X|Y|L|R|L2|R2|L3|R3|select|start|A|B" },
      { "nxengine_inventory_button",
         "Inventory Button; start|A|B|X|Y|L|R|L2|R2|L3|R3|select" },
      { "nxengine_options_menu_button",
         "Options Menu Button; select|start|A|B|X|Y|L|R|L2|R2|L3|R3" },
      { NULL, NULL },
   };

   cb(RETRO_ENVIRONMENT_SET_VARIABLES, variables);
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
   info->library_version = "1.0.0.4";
   info->library_name = "NXEngine";
   info->block_extract = false;
}

char g_dir[1024];

void retro_set_controller_port_device(unsigned port, unsigned device) {}

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

#ifdef FRONTEND_SUPPORTS_RGB565
   rgb565 = RETRO_PIXEL_FORMAT_RGB565;
   if(environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &rgb565) && log_cb)
      log_cb(RETRO_LOG_INFO, "Frontend supports RGB565 - will use that instead of XRGB1555.\n");
#endif
   check_system_specs();
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
   extract_directory(g_dir, game->path, sizeof(g_dir));
   NX_LOG("g_dir: %s\n", g_dir);

   if (pre_main())
      return 0;
   else
      return 1;
}

void retro_deinit(void)
{
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

static void environment_variables_updated(void)
{
   // Reinitialize input mappings
   input_init();
}

// environ_cb is declared static, so we need a helper function to allow other
// modules to get environment variable values.
const char* get_environment_variable(const char *key)
{
      struct retro_variable env_var;
      env_var.key = key;
      env_var.value = NULL;
      environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &env_var);

      return env_var.value;
}

void retro_run(void)
{
   bool env_vars_updated = false;
   environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &env_vars_updated);

   if(env_vars_updated) {
      environment_variables_updated();
   }

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

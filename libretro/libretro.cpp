#ifndef _MSC_VER
#include <stdbool.h>
#else
#define TRUE 1
#define FALSE 0
typedef unsigned char bool;
#endif
#include <unistd.h>
#include <string>

#define LIBRETRO_CORE 1

#if defined(_MSC_VER) && defined(LIBRETRO_CORE)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

#include "libretro.h"
#include "../graphics/graphics.h"

bool pre_main();
void post_main();
bool run_main();

retro_video_refresh_t video_cb;
static retro_input_poll_t poll_cb;
retro_input_state_t input_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;

bool retro_60hz = true;
unsigned pitch;

void retro_set_environment(retro_environment_t cb)
{
   environ_cb = cb;
}

EXPORT unsigned retro_api_version(void)
{
   return RETRO_API_VERSION;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
   video_cb = cb;
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{ }

EXPORT void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
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

EXPORT void retro_get_system_info(struct retro_system_info *info)
{
   info->need_fullpath = false;
   info->valid_extensions = "zip|ZIP";
   info->library_version = "10.0.3";
   info->library_name = "NXEngine (Cave Story)";
   info->block_extract = false;
}

static std::string g_dir;

void retro_set_controller_port_device(unsigned port, unsigned device) {}

EXPORT void retro_get_system_av_info(struct retro_system_av_info *info)
{
   info->geometry.base_width = SCREEN_WIDTH;
   info->geometry.base_height = SCREEN_HEIGHT;
   info->geometry.max_width = SCREEN_WIDTH; 
   info->geometry.max_height = SCREEN_HEIGHT;
   info->timing.fps = 60.0;
   info->timing.sample_rate = ((2 * 22050) / 60 + 1) * 30;
}

EXPORT void retro_init(void)
{ }

EXPORT bool retro_load_game(const struct retro_game_info *game)
{
   g_dir = game->path;
   size_t pos = g_dir.find_last_of('/');
   if (pos == std::string::npos)
      pos = g_dir.find_last_of('\\');
   if (pos != std::string::npos)
   {
      g_dir = g_dir.substr(0, pos);

      fprintf(stderr, "[NX]: Setting working directory to: %s\n", g_dir.c_str());
      chdir(g_dir.c_str());
   }

   pre_main();

   return 1;
}

EXPORT void retro_deinit(void)
{
   post_main();
}

EXPORT void retro_reset(void) {}

void game_mixaudio(int16_t *stream, size_t len);

EXPORT void retro_run(void)
{
   poll_cb();

   static unsigned frame_cnt = 0;
   if (retro_60hz)
      while (!run_main());
   else
   {
      frame_cnt = (frame_cnt + 1) % 6;
      if (frame_cnt)
         while (!run_main());
   }

   int16_t samples[(2 * 22050) / 60 + 1] = {0};
   game_mixaudio(samples, sizeof(samples) / sizeof(int16_t));
   audio_batch_cb(samples, sizeof(samples) >> 2);
}

void retro_unload_cartridge(void) {}

EXPORT size_t retro_serialize_size(void)
{
   return 0;
}

EXPORT bool retro_serialize(void *data, size_t size)
{
   return false;
}

bool retro_unserialize(const void *data, size_t size)
{
   return false;
}

void retro_cheat_reset(void) {}
void retro_cheat_set(unsigned index, bool enabled, const char *code) {}

EXPORT bool retro_load_game_special(
  unsigned game_type,
  const struct retro_game_info *info, size_t num_info
)
{ return false; }

EXPORT void retro_unload_game (void)
{ }

unsigned retro_get_region(void)
{
   return RETRO_REGION_NTSC;
}

void * retro_get_memory_data(unsigned id) { return 0; }
size_t retro_get_memory_size(unsigned id) { return 0; }


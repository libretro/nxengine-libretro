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

void *retro_frame_buffer;
unsigned retro_frame_buffer_width;
unsigned retro_frame_buffer_height;
unsigned retro_frame_buffer_pitch;

static retro_video_refresh_t video_cb;
static retro_input_poll_t poll_cb;
retro_input_state_t input_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;

static unsigned g_frame_cnt;

bool retro_60hz = true;
unsigned pitch;

extern void pre_main(void);

unsigned retro_get_tick(void)
{
	return g_frame_cnt;
}

void retro_set_environment(retro_environment_t cb)
{
   environ_cb = cb;
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

void retro_init(void)
{
   enum retro_pixel_format rgb565;

#ifdef FRONTEND_SUPPORTS_RGB565
   rgb565 = RETRO_PIXEL_FORMAT_RGB565;
   if(environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &rgb565))
      fprintf(stderr, "Frontend supports RGB565 - will use that instead of XRGB1555.\n");
#endif
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

   pre_main();

   return 1;
}

void retro_deinit(void)
{
   post_main();
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

   //fprintf(stderr, "[NX]: Start frame.\n");
   //int64_t start_time = get_usec();

   if (retro_60hz)
   {
      //int64_t start_time_frame = get_usec();
      while (!run_main());
      //int64_t total_time_frame = get_usec() - start_time_frame;
      //fprintf(stderr, "[NX]: total_time_frame took %lld usec.\n", (long long)total_time_frame);

      //int64_t start_time_frame_cb = get_usec();
      video_cb(retro_frame_buffer, retro_frame_buffer_width, retro_frame_buffer_height, retro_frame_buffer_pitch);
      //int64_t total_time_frame_cb = get_usec() - start_time_frame_cb;
      //fprintf(stderr, "[NX]: total_time_frame_cb took %lld usec.\n", (long long)total_time_frame_cb);

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

   //int64_t total_time = get_usec() - start_time;
   //fprintf(stderr, "[NX]: Frame took %lld usec.\n", (long long)total_time);
}

void retro_unload_cartridge(void) {}

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
{ return false; }

void retro_unload_game (void)
{ }

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

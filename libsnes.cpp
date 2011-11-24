#include "libsnes.hpp"
#include "graphics/graphics.h"
#include <unistd.h>
#include <string>
#include <assert.h>

bool pre_main();
void post_main();
bool run_main();


unsigned snes_library_revision_major(void) { return 1; }
unsigned snes_library_revision_minor(void) { return 3; }

const char *snes_library_id(void) { return "NXEngine/libsnes"; }

snes_environment_t snes_environ_cb;
snes_video_refresh_t snes_video_cb;
snes_audio_sample_t snes_audio_cb;
snes_input_poll_t snes_input_poll_cb;
snes_input_state_t snes_input_state_cb;
bool snes_60hz = true;

void snes_set_environment(snes_environment_t cb)     { snes_environ_cb     = cb; }
void snes_set_video_refresh(snes_video_refresh_t cb) { snes_video_cb       = cb; }
void snes_set_audio_sample(snes_audio_sample_t cb)   { snes_audio_cb       = cb; }
void snes_set_input_poll(snes_input_poll_t cb)       { snes_input_poll_cb  = cb; }
void snes_set_input_state(snes_input_state_t cb)     { snes_input_state_cb = cb; }

static std::string g_dir;
void snes_set_controller_port_device(bool port, unsigned device) {}
void snes_set_cartridge_basename(const char *basename)
{
   g_dir = basename;
   size_t pos = g_dir.find_last_of('/');
   if (pos == std::string::npos)
      pos = g_dir.find_last_of('\\');
   if (pos != std::string::npos)
   {
      g_dir = g_dir.substr(0, pos);

      fprintf(stderr, "[NX]: Setting working directory to: %s\n", g_dir.c_str());
      chdir(g_dir.c_str());
   }
}

void snes_init(void)
{
   const char *fullpath;
   if (snes_environ_cb(SNES_ENVIRONMENT_GET_FULLPATH, &fullpath))
      snes_set_cartridge_basename(fullpath);

   pre_main();

   snes_geometry geom = { SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT };
   snes_environ_cb(SNES_ENVIRONMENT_SET_GEOMETRY, &geom);

   snes_system_timing timing;
   timing.fps = 60.0;
   timing.sample_rate = ((2 * 22050) / 60 + 1) * 30;
   snes_environ_cb(SNES_ENVIRONMENT_SET_TIMING, &timing);
}

void snes_term(void)
{
   post_main();
}

void snes_power(void) {}
void snes_reset(void) {}

void game_mixaudio(int16_t *stream, size_t len);

void snes_run(void)
{
   snes_input_poll_cb();

   static unsigned frame_cnt = 0;
   if (snes_60hz)
      while (!run_main());
   else
   {
      frame_cnt = (frame_cnt + 1) % 6;
      if (frame_cnt)
         while (!run_main());
   }

   int16_t samples[(2 * 22050) / 60 + 1] = {0};
   game_mixaudio(samples, sizeof(samples) / sizeof(int16_t));
   for (unsigned i = 0; i < sizeof(samples) / sizeof(int16_t); i += 2)
      snes_audio_cb(samples[i + 0], samples[i + 1]);
}

void snes_unload_cartridge(void) {}

unsigned snes_serialize_size(void) { return 0; }
bool snes_serialize(uint8_t *data, unsigned size) { return false; }
bool snes_unserialize(const uint8_t *data, unsigned size) { return false; }

void snes_cheat_reset(void) {}
void snes_cheat_set(unsigned index, bool enabled, const char *code) {}

bool snes_load_cartridge_normal(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size
) { return true; }

bool snes_load_cartridge_bsx_slotted(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *bsx_xml, const uint8_t *bsx_data, unsigned bsx_size
) { return false; }

bool snes_load_cartridge_bsx(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *bsx_xml, const uint8_t *bsx_data, unsigned bsx_size
) { return false; }

bool snes_load_cartridge_sufami_turbo(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *sta_xml, const uint8_t *sta_data, unsigned sta_size,
  const char *stb_xml, const uint8_t *stb_data, unsigned stb_size
) { return false; }

bool snes_load_cartridge_super_game_boy(
  const char *rom_xml, const uint8_t *rom_data, unsigned rom_size,
  const char *dmg_xml, const uint8_t *dmg_data, unsigned dmg_size
) { return false; }


bool snes_get_region(void) { return SNES_REGION_NTSC; }
uint8_t* snes_get_memory_data(unsigned id) { return 0; }
unsigned snes_get_memory_size(unsigned id) { return 0; }


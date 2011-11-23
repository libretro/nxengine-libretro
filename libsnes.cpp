#include "libsnes.hpp"
#include "graphics/graphics.h"

bool pre_main();
void post_main();
void run_main();


unsigned snes_library_revision_major(void) { return 1; }
unsigned snes_library_revision_minor(void) { return 3; }

const char *snes_library_id(void) { return "NXEngine/libsnes"; }

snes_environment_t snes_environ_cb;
snes_video_refresh_t snes_video_cb;
snes_audio_sample_t snes_audio_cb;
snes_input_poll_t snes_input_poll_cb;
snes_input_state_t snes_input_state_cb;

void snes_set_environment(snes_environment_t cb)     { snes_environ_cb     = cb; }
void snes_set_video_refresh(snes_video_refresh_t cb) { snes_video_cb       = cb; }
void snes_set_audio_sample(snes_audio_sample_t cb)   { snes_audio_cb       = cb; }
void snes_set_input_poll(snes_input_poll_t cb)       { snes_input_poll_cb  = cb; }
void snes_set_input_state(snes_input_state_t cb)     { snes_input_state_cb = cb; }

void snes_set_controller_port_device(bool port, unsigned device) {}
void snes_set_cartridge_basename(const char *basename) {}

void snes_init(void)
{
   pre_main();

   snes_geometry geom = { SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT };
   snes_environ_cb(SNES_ENVIRONMENT_SET_GEOMETRY, &geom);
}

void snes_term(void)
{
   post_main();
}

void snes_power(void) {}
void snes_reset(void) {}

void snes_run(void)
{
   snes_input_poll_cb();
   run_main();
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


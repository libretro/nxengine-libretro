
#include "../nx.h"
#include "../main.fdh"
#include "libretro_shared.h";
#include "extract-auto/cachefiles.h"

#ifdef _WIN32
#include "msvc_compat.h"
#endif

const char *data_dir = "data";
#ifdef _WIN32
const char *stage_dir = "data\\Stage";
#else
const char *stage_dir = "data/Stage";
#endif
const char *pic_dir = "endpic";
const char *nxdata_dir = ".";

int fps = 0;
static int fps_so_far = 0;
static uint32_t fpstimer = 0;

#define GAME_WAIT			(1000/GAME_FPS)	// sets framerate
#define VISFLAGS			(SDL_APPACTIVE | SDL_APPINPUTFOCUS)
int framecount = 0;
bool freezeframe = false;

static bool inhibit_loadfade = false;
static bool error = false;
static bool freshstart;

//extern bool extract_files(FILE *exefp);
extern bool extract_stages(FILE *exefp);

bool pre_main(void)
{
	error = false;
#ifdef DEBUG_LOG
char debug_fname[1024];
retro_create_path_string(debug_fname, sizeof(debug_fname), g_dir, "debug.txt");
SetLogFilename(debug_fname);
#endif
	// start up inputs first thing because settings_load may remap them
	input_init();
	
	// load settings, or at least get the defaults,
	// so we know the initial screen resolution.
	settings_load();

   char filename[1024];
	FILE *fp;

	NX_LOG("= Extracting Files =\n");

	retro_create_path_string(filename, sizeof(filename), g_dir, "Doukutsu.exe");
	fp = fopen(filename, "rb");

   //extract_files(fp);
   cachefiles_init(fp);

	if (sound_init(fp)) { fatal("Failed to initialize sound."); error = 1; return error; }
   
	extract_stages(fp);

	fclose(fp);
	
   settings->files_extracted = true;
   settings_save();
	
	if (Graphics::init(settings->resolution)) { NX_ERR("Failed to initialize graphics.\n"); error = 1; return error; }
	if (font_init()) { NX_ERR("Failed to load font.\n"); error = 1; return error; }
	
	//return error;
	
	if (check_data_exists())
	{
		error = 1;
		return error;
	}
	
	if (trig_init()) { fatal("Failed trig module init."); error = 1; return error; }
	
	if (tsc_init()) { fatal("Failed to initialize script engine."); error = 1; return error; }
	if (textbox.Init()) { fatal("Failed to initialize textboxes."); error = 1; return error; }
	if (Carets::init()) { fatal("Failed to initialize carets."); error = 1; return error; }
	
	if (game.init())
	{
		error = 1;
		return error;
	}
	game.setmode(GM_NORMAL);
	// set null stage just to have something to do while we go to intro
	game.switchstage.mapno = 0;
	
   //game.switchstage.mapno = LOAD_GAME;
   //game.pause(GP_OPTIONS);

   if (settings->skip_intro && file_exists(GetProfileName(settings->last_save_slot)))
      game.switchstage.mapno = LOAD_GAME;
   else
      game.setmode(GM_INTRO);
	
	// for debug
	if (game.paused) { game.switchstage.mapno = 0; game.switchstage.eventonentry = 0; }
	if (game.switchstage.mapno == LOAD_GAME) inhibit_loadfade = true;
	
	game.running = true;
	freshstart = true;
	
	NX_LOG("Entering main loop...\n");
	
	return error;
}

void post_main(void)
{
	game.close();
	Carets::close();
	
	Graphics::close();
	input_close();
	font_close();
	sound_close();
	tsc_close();
	textbox.Deinit();
}

static bool gameloop(void)
{
	//uint32_t gametimer;

	//gametimer = -GAME_WAIT*10;

	if(game.switchstage.mapno < 0)
	{
		run_tick();
		return true;
	}
	else
		return false;
}

static bool in_gameloop = false;

bool run_main(void)
{
	if (in_gameloop)
		goto loop;
	// SSS/SPS persists across stage transitions until explicitly
	// stopped, or you die & reload. It seems a bit risky to me,
	// but that's the spec.
	if (game.switchstage.mapno >= MAPNO_SPECIALS)
	{
		StopLoopSounds();
	}

	// enter next stage, whatever it may be
	if (game.switchstage.mapno == LOAD_GAME || \
			game.switchstage.mapno == LOAD_GAME_FROM_MENU)
	{
		if (game.switchstage.mapno == LOAD_GAME_FROM_MENU)
			freshstart = true;

		NX_LOG("= Loading game =\n");
		if (game_load(settings->last_save_slot))
		{
			fatal("savefile error");
			game.running = false;
			error = 1;
			return false;
		}

		if (!inhibit_loadfade) fade.Start(FADE_IN, FADE_CENTER);
		else inhibit_loadfade = false;
	}
	else
	{
		if (game.switchstage.mapno == NEW_GAME || \
				game.switchstage.mapno == NEW_GAME_FROM_MENU)
		{
			static bool show_intro = (game.switchstage.mapno == NEW_GAME_FROM_MENU);
			InitNewGame(show_intro);
		}

		// slide weapon bar on first intro to Start Point
		if (game.switchstage.mapno == STAGE_START_POINT && \
				game.switchstage.eventonentry == 91)
		{
			freshstart = true;
		}

		// switch maps
		if (load_stage(game.switchstage.mapno))
		{
			game.running = false;
			error = 1;
			return false;
		}

		player->x = (game.switchstage.playerx * TILE_W) << CSF;
		player->y = (game.switchstage.playery * TILE_H) << CSF;
	}

	// start the level
	if (game.initlevel())
	{
		game.running = false;
		error = 1;
		return false;
	}

	if (freshstart)
		weapon_introslide();

	game.switchstage.mapno = -1;
loop:
	in_gameloop = true;
	if (gameloop())
		return true;	
	in_gameloop = false;

	game.stageboss.OnMapExit();
	freshstart = false;
}

static inline void run_tick()
{
	input_poll();
	
	// input handling for a few global things
	if (justpushed(ESCKEY))
	{
		if (settings->instant_quit)
		{
			game.running = false;
		}
		else if (!game.paused)		// no pause from Options
		{
			game.pause(GP_PAUSED);
		}
	}
	else if (justpushed(F3KEY))
	{
		game.pause(GP_OPTIONS);
	}
	
	// freeze frame
	game.tick();

	org_run();

	memcpy(lastinputs, inputs, sizeof(lastinputs));
}

void InitNewGame(bool with_intro)
{
	NX_LOG("= Beginning new game =\n");
	
	memset(game.flags, 0, sizeof(game.flags));
	memset(game.skipflags, 0, sizeof(game.skipflags));
	textbox.StageSelect.ClearSlots();
	
	game.quaketime = game.megaquaketime = 0;
	game.showmapnametime = 0;
	game.debug.god = 0;
	game.running = true;
	game.frozen = false;
	
	// fully re-init the player object
	Objects::DestroyAll(true);
	game.createplayer();
	
	player->maxHealth = 3;
	player->hp = player->maxHealth;
	
	game.switchstage.mapno = STAGE_START_POINT;
	game.switchstage.playerx = 10;
	game.switchstage.playery = 8;
	game.switchstage.eventonentry = (with_intro) ? 200 : 91;
	
	fade.set_full(FADE_OUT);
}

/*
void c------------------------------() {}
*/

static void fatal(const char *str)
{
	NX_ERR("Fatal error: '%s'\n", str);
}

static bool check_data_exists()
{
   char fname[1024];
	retro_create_subpath_string(fname, sizeof(fname), g_dir, data_dir, "npc.tbl");
	NX_LOG("check_data_exists: %s\n", fname);

	if (file_exists(fname))
      return 0;
	
   NX_ERR("Fatal Error\n");

   NX_ERR("Missing \"%s\" directory.\n", data_dir);
   NX_ERR("Please copy it over from a Doukutsu installation.\n");
	
	return 1;
}

void visible_warning(const char *fmt, ...)
{
#ifndef _XBOX1
va_list ar;
char buffer[80];

	va_start(ar, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ar);
	va_end(ar);
#endif
}

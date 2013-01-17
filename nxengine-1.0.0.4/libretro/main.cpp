
#include "../nx.h"
#ifdef USE_SAFEMODE
#include "../graphics/safemode.h"
#endif
#include "../main.fdh"
#include "libretro_shared.h";

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

void pre_main(void)
{
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
	
	if (!settings->files_extracted)
	{
		if (extract_main())
		{
			error = 1;
			return;
		}
		else
		{
			settings->files_extracted = true;
			settings_save();
		}
	}
	
	if (Graphics::init(settings->resolution)) { NX_ERR("Failed to initialize graphics.\n"); error = 1; return; }
	if (font_init()) { NX_ERR("Failed to load font.\n"); error = 1; return; }
	
	//return;
	
	#ifdef CONFIG_DATA_EXTRACTOR
	if (!settings->files_extracted)
	{
		if (extract_main())
		{
			Graphics::close();
			font_close();
			error = 1;
			return;
		}
		else
		{
			settings->files_extracted = true;
			settings_save();
		}
	}
	#endif
	
	if (check_data_exists())
	{
		error = 1;
		return;
	}
	
	//Graphics::ShowLoadingScreen();
	if (sound_init()) { fatal("Failed to initialize sound."); error = 1; return; }
	if (trig_init()) { fatal("Failed trig module init."); error = 1; return; }
	
	if (tsc_init()) { fatal("Failed to initialize script engine."); error = 1; return; }
	if (textbox.Init()) { fatal("Failed to initialize textboxes."); error = 1; return; }
	if (Carets::init()) { fatal("Failed to initialize carets."); error = 1; return; }
	
	if (game.init())
	{
		error = 1;
		return;
	}
	game.setmode(GM_NORMAL);
	// set null stage just to have something to do while we go to intro
	game.switchstage.mapno = 0;
	
	//#define REPLAY
	#ifdef REPLAY
		game.switchstage.mapno = START_REPLAY;
		//Replay::set_ffwd(6000);
		//Replay::set_stopat(3500);
		game.switchstage.param = 1;
	#else
		//game.switchstage.mapno = LOAD_GAME;
		//game.pause(GP_OPTIONS);
		
		if (settings->skip_intro && file_exists(GetProfileName(settings->last_save_slot)))
			game.switchstage.mapno = LOAD_GAME;
		else
			game.setmode(GM_INTRO);
	#endif
	
	// for debug
	if (game.paused) { game.switchstage.mapno = 0; game.switchstage.eventonentry = 0; }
	if (game.switchstage.mapno == LOAD_GAME) inhibit_loadfade = true;
	
	game.running = true;
	freshstart = true;
	
	NX_LOG("Entering main loop...\n");
	
	//return;
}

void post_main(void)
{
	Replay::close();
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

		Replay::OnGameStarting();

		if (!inhibit_loadfade) fade.Start(FADE_IN, FADE_CENTER);
		else inhibit_loadfade = false;
	}
	else if (game.switchstage.mapno == START_REPLAY)
	{
		NX_LOG(">> beginning replay '%s'\n", GetReplayName(game.switchstage.param));

		StopScripts();
		if (Replay::begin_playback(GetReplayName(game.switchstage.param)))
		{
			fatal("error starting playback");
			game.running = false;
			error = 1;
			return false;
		}
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

#ifndef __LIBRETRO__
int main(int argc, char *argv[])
{

	pre_main();

	if(error)
		goto ingame_error;	
	
loop:
	while (!run_main());
shutdown: ;
	if(!game.running)
	{
		post_main();
		return error;
	}
check_error: ;
	if(error)
		goto ingame_error;
	else
		goto loop;
ingame_error: ;
	NX_LOG("\n");
	NX_LOG(" ************************************************\n");
	NX_LOG(" * An in-game error occurred. Game shutting down.\n");
	NX_LOG(" ************************************************\n");
	error = 1;
	goto shutdown;
}
#endif


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

	Replay::DrawStatus();

	org_run();

	//platform_sync_to_vblank();
	screen->Flip();

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
	NX_ERR("fatal: '%s'\n", str);
	
#ifdef USE_SAFEMODE
	if (!safemode::init())
	{
		safemode::moveto(SM_UPPER_THIRD);
		safemode::print("Fatal Error");
		
		safemode::moveto(SM_CENTER);
		safemode::print("%s", str);
		
		safemode::run_until_key();
		safemode::close();
	}
#else
		NX_LOG("Fatal Error\n");
		NX_LOG("%s\n", str);
#endif
}

static bool check_data_exists()
{
        char fname[1024];
	retro_create_subpath_string(fname, sizeof(fname), g_dir, data_dir, "npc.tbl");
	NX_LOG("check_data_exists: %s\n", fname);

	if (file_exists(fname)) return 0;
	
#ifdef USE_SAFEMODE
	if (!safemode::init())
	{
		safemode::moveto(SM_UPPER_THIRD);
		safemode::print("Fatal Error");
		
		safemode::moveto(SM_CENTER);
		safemode::print("Missing \"%s\" directory.", data_dir);
		safemode::print("Please copy it over from a Doukutsu installation.");
		
		safemode::run_until_key();
		safemode::close();
	}
#else
		NX_LOG("Fatal Error\n");
		
		NX_LOG("Missing \"%s\" directory.\n", data_dir);
		NX_LOG("Please copy it over from a Doukutsu installation.\n");
#endif
	
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
	
	console.Print(buffer);
#endif
}

#if 0
void SDL_Delay(int ms)
{
	usleep(ms * 1000);
}
#endif

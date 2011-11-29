#include <stdarg.h>
#include "common/misc.h"
#include "sound/org.h"
#include "nx.h"

const char *data_dir = "data";
const char *stage_dir = "data/Stage";
const char *pic_dir = "endpic";
const char *nxdata_dir = ".";


#define GAME_WAIT			(1000/GAME_FPS)	// sets framerate
int framecount = 0;

static void InitNewGame(bool with_intro)
{
	stat("= Beginning new game =");
	
	memset(game.flags, 0, sizeof(game.flags));
	memset(game.skipflags, 0, sizeof(game.skipflags));
	textbox.StageSelect.ClearSlots();
	
	game.quaketime = game.megaquaketime = 0;
	game.showmapnametime = 0;
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

static inline void run_tick()
{
	//static bool last_freezekey = false;
	//static bool last_framekey = false;

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

	screen->Flip();

	memcpy(lastinputs, inputs, sizeof(lastinputs));

	// immediately after a game tick is when we have the most amount of time before
	// the game needs to run again. so now's as good a time as any for some
	// BGM audio processing, wouldn't you say?

	org_run();
}

static bool gameloop(void)
{
	uint32_t gametimer;

	gametimer = -GAME_WAIT*10;
	//game.switchstage.mapno = -1;

   if (game.switchstage.mapno < 0)
   {
      run_tick();
      return true;
   }
   else
      return false;
}

static bool inhibit_loadfade = false;
//static bool error = false;
static bool freshstart;

bool pre_main()
{
#ifdef USE_LOGGING
   SetLogFilename("debug.txt");
#endif

   // start up inputs first thing because settings_load may remap them
   input_init();

   // load settings, or at least get the defaults
   settings_load();

   if (Graphics::init())
   {
      staterr("Failed to initilize graphics.");
      return false;
   }
   if (font_init())
   {
      staterr("Failed to load font.");
      return false;
   }

   //Graphics::ShowLoadingScreen();
   if (sound_init())
   {
      return false;
   }
   if (trig_init())
   {
      return false;
   }
   if (tsc_init())
   {
      return false;
   }
   if (textbox.Init())
   {
      return false;
   }
   if (Carets::init())
   {
      return false;
   }

   if (game.init()) return 1;
   game.setmode(GM_NORMAL);
   // set null stage just to have something to do while we go to intro
   game.switchstage.mapno = 0;

   //#define REPLAY
#ifdef REPLAY
   game.switchstage.mapno = START_REPLAY;
   Replay::set_ffwd(23400);
   //Replay::set_stopat(3500);
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

   return true;
}

void post_main()
{
   Replay::close();
   game.close();
   Carets::close();

   sound_close();
   tsc_close();
   textbox.Deinit();
}

// Dirty, but reasonable given that we're making a do_while loop iterative, I suppose ...
static bool in_gameloop = false;

bool run_main()
{
   // :D
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

      stat("= Loading game =");
      if (game_load(settings->last_save_slot))
      {
         return false;
      }

      Replay::OnGameStarting();

      if (!inhibit_loadfade) fade.Start(FADE_IN, FADE_CENTER);
      else inhibit_loadfade = false;
   }
   else if (game.switchstage.mapno == START_REPLAY)
   {
      stat(">> beginning replay '%s'", GetReplayName(game.switchstage.param));

      StopScripts();
      if (Replay::begin_playback(GetReplayName(game.switchstage.param)))
      {
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
      if (load_stage(game.switchstage.mapno)) return false;

      player->x = (game.switchstage.playerx * TILE_W) << CSF;
      player->y = (game.switchstage.playery * TILE_H) << CSF;
   }

   // start the level
   if (game.initlevel()) return false;

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
   return false;
}

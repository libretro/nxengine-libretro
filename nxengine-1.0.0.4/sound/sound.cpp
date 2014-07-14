
/* SOUND.C
  PXT/SS/Org sound interface
*/
#include <stdio.h>
#include <string.h>

#include "../nx.h"
#include "../settings.h"
#include "pxt.h"
#include "sound.h"
#include "sound.fdh"

#include "libretro_shared.h"

#ifdef _WIN32
#include "msvc_compat.h"
#endif

#define MUSIC_OFF		0
#define MUSIC_ON		1
#define MUSIC_BOSS_ONLY	2
static int lastsong = 0;		// this holds the previous song, for <RMU
static int cursong = 0;

// there are more than this around 9b; those are drums and are loaded by the org module
#define NUM_SOUNDS		0x75
#define ORG_VOLUME		75

const char *org_names[] =
{
	NULL,
	"egg",
   "safety",
   "gameover",
   "gravity",
   "grasstown",
   "meltdown2",
   "eyesofflame",
	"gestation",
   "town",
   "fanfale1",
   "balrog",
   "cemetary",
   "plant",
   "pulse",
   "fanfale2",
	"fanfale3",
   "tyrant",
   "run",
   "jenka1",
   "labyrinth",
   "access",
   "oppression",
   "geothermal",
	"theme",
   "oside",
   "heroend",
   "scorching",
   "quiet",
   "lastcave",
   "balcony",
   "charge",
	"lastbattle",
   "credits",
   "zombie",
   "breakdown",
   "hell",
   "jenka2",
   "waterway",
   "seal",
	"toroko",
   "white",
   "azarashi",
   NULL
};

static const char bossmusic[] = { 4, 7, 10, 11, 15, 16, 17, 18, 21, 22, 31, 33, 35, 0 };

bool sound_init(FILE *fp)
{
	if (SSInit()) return 1;
	if (pxt_init()) return 1;

	if (pxt_LoadSoundFX(fp, NUM_SOUNDS))
      return 1;

	if (org_init(fp, ORG_VOLUME))
	{
		NX_ERR("Music failed to initialize\n");
		return 1;
	}
	
	return 0;
}

void sound_close(void)
{
	pxt_freeSoundFX();
	SSClose();
}

/*
void c------------------------------() {}
*/

void sound(int snd)
{
	if (!settings->sound_enabled)
		return;
	
	pxt_Stop(snd);
	pxt_Play(-1, snd, 0);
}

void sound_loop(int snd)
{
	if (!settings->sound_enabled)
		return;
	
	pxt_Play(-1, snd, -1);
}

void sound_stop(int snd)
{
	pxt_Stop(snd);
}

bool sound_is_playing(int snd)
{
	return pxt_IsPlaying(snd);
}


void StartStreamSound(int freq)
{
	// pxt_ChangePitch(SND_STREAM1, some_formula);
	// pxt_ChangePitch(SND_STREAM2, some_other_formula);
	sound_loop(SND_STREAM1);
	sound_loop(SND_STREAM2);
}

void StartPropSound(void)
{
	sound_loop(SND_PROPELLOR);
}

void StopLoopSounds(void)
{
	sound_stop(SND_STREAM1);
	sound_stop(SND_STREAM2);
	sound_stop(SND_PROPELLOR);
}

/*
void c------------------------------() {}
*/

void music(int songno)
{
	if (songno == cursong)
		return;
	
	lastsong = cursong;
	cursong = songno;
	
	NX_LOG(" >> music(%d)\n", songno);
	
	if (songno != 0 && !should_music_play(songno, settings->music_enabled))
	{
		NX_WARN("Not playing track %d because music_enabled is %d\n", songno, settings->music_enabled);
		org_stop();
		return;
	}
	
	start_track(songno);
}


bool should_music_play(int songno, int musicmode)
{
	if (game.mode == GM_TITLE || game.mode == GM_CREDITS)
		return true;
	
	switch(musicmode)
	{
		case MUSIC_OFF: return false;
		case MUSIC_ON:  return true;
		case MUSIC_BOSS_ONLY:
			return music_is_boss(songno);
	}
	
	return false;
}

bool music_is_boss(int songno)
{
	if (strchr(bossmusic, songno))
		return true;
	else
		return false;
}

void music_set_enabled(int newstate)
{
	if (newstate != settings->music_enabled)
	{
		NX_LOG("music_set_enabled(%d)\n", newstate);
		
		settings->music_enabled = newstate;
		bool play = should_music_play(cursong, newstate);
		
		if (play != org_is_playing())
		{
			if (play)
				start_track(cursong);
			else
				org_stop();
		}
	}
}

static void start_track(int songno)
{
	if (songno == 0)
	{
		org_stop();
		return;
	}
	
   NX_LOG("start_track: %d\n\n", songno);
	
	if (!org_load(songno))
		org_start(0);
}

int music_cursong()		{ return cursong; }
int music_lastsong() 	{ return lastsong; }



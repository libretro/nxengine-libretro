
#ifndef _PXT_H
#define _PXT_H

#include "../common/basics.h"

#define PXT_NO_CHANNELS			4

enum
{
	MOD_SINE,
	MOD_TRI,
	MOD_SAWUP,
	MOD_SAWDOWN,
	MOD_SQUARE,
	MOD_NOISE,
	MOD_WHITE,
	MOD_PULSE,
	
	PXT_NO_MODELS
};

typedef struct
{
	signed char *model;		// ptr to model data
	uchar model_no;			// index of model data (0-5, which wave[] model points to)
	
	double phaseacc;
	double phaseinc;
	double repeat;			// pixtone calls it "freq"
	uchar volume;			// pixtone calls it "top"
	uchar offset;
	
	int white_ptr;			// like "phaseacc" but for MOD_WHITE (the odd one out)
} stPXWave;

#define PXENV_NUM_VERTICES		3
typedef struct
{
	int initial;
	
	int time[PXENV_NUM_VERTICES];
	int val[PXENV_NUM_VERTICES];
} stPXEnvelope;

typedef struct
{
	int size_blocks;
	char enabled;
	
	stPXWave main;
	stPXWave pitch;
	stPXWave pitch2;
	stPXWave volume;
	stPXEnvelope envelope;
	
	unsigned char envbuffer[256];
	signed char *buffer;
} stPXChannel;

typedef struct
{
	stPXChannel chan[PXT_NO_CHANNELS];
	
	signed char *final_buffer;
	int final_size;
} stPXSound;

char pxt_initsynth(void);
char pxt_load(const char *fname, stPXSound *snd);
char pxt_Render(stPXSound *snd);
void FreePXTBuf(stPXSound *snd);
char pxt_init(void);
char pxt_LoadSoundFX(const char *path, const char *cache_name, int top);
void pxt_FreeSound(int slot);
void pxt_freeSoundFX(void);
void pxt_Stop(int slot);
int pxt_Play(int chan, int slot, char loop);
char pxt_IsPlaying(int slot);

#endif


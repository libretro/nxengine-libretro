
#include "../sdl/include/LRSDL.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include "../common/basics.h"
#include "../libretro/libretro_shared.h"
#include "../nx_logger.h"
#include "../sound/pxt.h"

#include "extractpxt.fdh"

#ifdef _WIN32
#include <direct.h>
#endif

static struct
{
	const char *name;
	bool is_integer;
}
fields[] =
{
	"use  ", 1,
	"size ", 1,
	"main_model   ", 1,
	"main_freq    ", 0,
	"main_top     ", 1,
	"main_offset  ", 1,
	"pitch_model  ", 1,
	"pitch_freq   ", 0,
	"pitch_top    ", 1,
	"pitch_offset ", 1,
	"volume_model ", 1,
	"volume_freq  ", 0,
	"volume_top   ", 1,
	"volume_offset", 1,
	"initialY", 1,
	"ax      ", 1,
	"ay      ", 1,
	"bx      ", 1,
	"by      ", 1,
	"cx      ", 1,
	"cy      ", 1,
	NULL
};

static struct
{
	int id;
	int nchanl;
	uint32_t offset;
}
snd[] =
{
	0x01, 1, 0x0907b0,
	0x02, 1, 0x0909e0,
	0x03, 1, 0x0934c0,
	0x04, 1, 0x090890,
	0x05, 1, 0x090660,
	0x06, 1, 0x093530,
	0x07, 1, 0x0935a0,
	0x0b, 1, 0x090740,
	0x0c, 2, 0x090c80,
	0x0e, 1, 0x090a50,
	0x0f, 1, 0x08fbe0,
	0x10, 2, 0x090350,
	0x11, 3, 0x090430,
	0x12, 1, 0x090820,
	0x14, 2, 0x090900,
	0x15, 1, 0x090c10,
	0x16, 1, 0x0906d0,
	0x17, 1, 0x08fcc0,
	0x18, 1, 0x08fc50,
	0x19, 2, 0x090d60,
	0x1a, 2, 0x090b30,
	0x1b, 1, 0x090e40,
	0x1c, 2, 0x0910e0,
	0x1d, 1, 0x0911c0,
	0x1e, 1, 0x091ee0,
	0x1f, 1, 0x091310,
	0x20, 2, 0x08f940,
	0x21, 2, 0x08fa20,
	0x22, 2, 0x08fb00,
	0x23, 3, 0x090eb0,
	0x25, 2, 0x092810,
	0x26, 2, 0x091230,
	0x27, 3, 0x091000,
	0x28, 2, 0x092730,
	0x29, 2, 0x092730,
	0x2a, 1, 0x091380,
	0x2b, 1, 0x0913f0,
	0x2c, 3, 0x091460,
	0x2d, 1, 0x0915b0,
	0x2e, 1, 0x091620,
	0x2f, 1, 0x091700,
	0x30, 1, 0x091770,
	0x31, 2, 0x0917e0,
	0x32, 2, 0x08fd30,
	0x33, 2, 0x08fe10,
	0x34, 2, 0x08fef0,
	0x35, 2, 0x090580,
	0x36, 2, 0x091a80,
	0x37, 2, 0x092ea0,
	0x38, 2, 0x092650,
	0x39, 2, 0x0928f0,
	0x3a, 2, 0x092dc0,
	0x3b, 1, 0x093060,
	0x3c, 1, 0x0930d0,
	0x3d, 1, 0x093140,
	0x3e, 2, 0x0931b0,
	0x3f, 2, 0x093290,
	0x40, 2, 0x093370,
	0x41, 1, 0x093450,
	0x46, 2, 0x08ffd0,
	0x47, 2, 0x0900b0,
	0x48, 2, 0x090190,
	0x64, 1, 0x0918c0,
	0x65, 3, 0x091930,
	0x66, 2, 0x091b60,
	0x67, 2, 0x091c40,
	0x68, 1, 0x091cb0,
	0x68, 1, 0x092c00,
	0x69, 1, 0x091d20,
	0x6a, 2, 0x091d90,
	0x6b, 1, 0x091e70,
	0x6c, 1, 0x091f50,
	0x6d, 1, 0x091fc0,
	0x6e, 1, 0x092030,
	0x6f, 1, 0x0920a0,
	0x70, 1, 0x092110,
	0x71, 1, 0x092180,
	0x72, 2, 0x0921f0,
	0x73, 3, 0x092ab0,
	0x74, 3, 0x092c70,
	0x75, 2, 0x092f80,
	0x96, 2, 0x0922d0,
	0x97, 2, 0x0923b0,
	0x98, 1, 0x092490,
	0x99, 1, 0x092500,
	0x9a, 2, 0x092570,
	0x9b, 2, 0x0929d0,
	0, 0, 0
};

extern char pxt_SetModel(stPXWave *pxwave, int m);

bool extract_pxt(FILE *fp, int s, stPXSound *outsnd)
{
struct
{
	union
	{
		int intvalue;
		double fpvalue;
	} values[21];
} chan[4];
int c, i;
int found = 0;
char slash;
#ifdef _WIN32
slash = '\\';
#else
slash = '/';
#endif
   for (i = 0; i < sizeof(snd) / sizeof(snd[0]); i++)
   {
      if (snd[i].id == s)
      {
         found = 1;
         s = i;
         break;
      }
   }
   if (!found)
      return 1;

		fseek(fp, snd[s].offset, SEEK_SET);
		memset(chan, 0, sizeof(chan));
		
		// load data
		for(c=0;c<snd[s].nchanl;c++)
		{
			for(i=0;fields[i].name;i++)
			{
				if (fields[i].is_integer)
				{
					chan[c].values[i].intvalue = fgetl(fp);
				}
				else
				{
					chan[c].values[i].fpvalue = fgetfloat(fp);
				}
			}
			
			// skip padding between sections
			if (fgetl(fp) != 0)
			{
				NX_ERR("PXT out of sync\n");
				return 1;
			}
		}
   
      for (c = 0; c < 4; c++)
      {
         outsnd->chan[c].enabled = chan[c].values[0].intvalue;
         outsnd->chan[c].size_blocks = chan[c].values[1].intvalue;
         
         pxt_SetModel(&outsnd->chan[c].main, chan[c].values[2].intvalue);
         outsnd->chan[c].main.repeat = chan[c].values[3].fpvalue;
         outsnd->chan[c].main.volume = chan[c].values[4].intvalue;
         outsnd->chan[c].main.offset = chan[c].values[5].intvalue;
         
         pxt_SetModel(&outsnd->chan[c].pitch, chan[c].values[6].intvalue);
         outsnd->chan[c].pitch.repeat = chan[c].values[7].fpvalue;
         outsnd->chan[c].pitch.volume = chan[c].values[8].intvalue;
         outsnd->chan[c].pitch.offset = chan[c].values[9].intvalue;
         
         pxt_SetModel(&outsnd->chan[c].volume, chan[c].values[10].intvalue);
         outsnd->chan[c].volume.repeat = chan[c].values[11].fpvalue;
         outsnd->chan[c].volume.volume = chan[c].values[12].intvalue;
         outsnd->chan[c].volume.offset = chan[c].values[13].intvalue;
         
         outsnd->chan[c].envelope.initial = chan[c].values[14].intvalue;

         outsnd->chan[c].envelope.time[0] = chan[c].values[15].intvalue;
         outsnd->chan[c].envelope.val[0] = chan[c].values[16].intvalue;
         outsnd->chan[c].envelope.time[1] = chan[c].values[17].intvalue;
         outsnd->chan[c].envelope.val[1] = chan[c].values[18].intvalue;
         outsnd->chan[c].envelope.time[2] = chan[c].values[19].intvalue;
         outsnd->chan[c].envelope.val[2] = chan[c].values[20].intvalue;
      }
	
	return 0;
}


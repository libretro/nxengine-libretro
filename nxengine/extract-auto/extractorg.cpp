#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../common/basics.h"
#include "../libretro/libretro_shared.h"
//#include "extractfiles.fdh"
#include "../nx_logger.h"

#ifdef __MINGW32__
	#include <direct.h>
	#include <io.h>
#endif

#ifdef _WIN32
#include <direct.h>
#endif

#define HEADER_LEN		25
#define MAX_FILE_SIZE	32768

// Windows .bmp resources don't include the BMP-file headers
const uint8_t credit_header[] = \
{
	0x42, 0x4D, 0x76, 0x4B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x76, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0xA0, 0x00,
	0x00, 0x00, 0xF0, 0x00, 0x00
};

const uint8_t pixel_header[] = \
{
	0x42, 0x4D, 0x76, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x76, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0xA0, 0x00,
	0x00, 0x00, 0x10, 0x00, 0x00
};
static struct
{
   const char *filename;
	uint32_t offset;
	uint32_t length;
	uint32_t crc;
}
files[] =
{
   NULL, 0, 0, 0,
	"org/egg.org",         0x0feb20, 19626, 0xb651047e,
	"org/safety.org",      0x09b7d0, 9194,  0x779e83c2,
	"org/gameover.org",    0x0b412c, 1138,  0x1f87b446,
	"org/gravity.org",     0x0b9190, 20578, 0x64a9318d,
	"org/grasstown.org",   0x1037cc, 23706, 0xa27883b6,
	"org/meltdown2.org",   0x0df67c, 21074, 0x83d08aed,
	"org/eyesofflame.org", 0x0aedc0, 21354, 0x6b5ff989,
	"org/gestation.org",   0x0f83c8, 10458, 0xce2e68c1,
	"org/town.org",        0x0e48d0, 10634, 0x6a6aa627,
	"org/fanfale1.org",    0x0ae25c, 914,   0xaefd547b,
	"org/balrog.org",      0x0b45a0, 5970,  0xb02093b8,
	"org/cemetary.org",    0x09ffc8, 4578,  0x2ce377cc,
	"org/plant.org",       0x0ed680, 11378, 0x3911e040,
	"org/pulse.org",       0x0cad9c, 10418, 0x92ef0330,
	"org/fanfale2.org",    0x0ae98c, 1074,  0x3a5170a6,
	"org/fanfale3.org",    0x0ae5f0, 922,   0x85813929,
	"org/tyrant.org",      0x0a7638, 2162,  0xc64dc450,
	"org/run.org",         0x0ac498, 7618,  0x65a4bb85,
	"org/jenka1.org",      0x0c5e54, 8306,  0xb42d7eaa,
	"org/labyrinth.org",   0x0dbcb8, 14786, 0x0292cf2c,
	"org/access.org",      0x09b35c, 1138,  0xd965dddb,
	"org/oppression.org",  0x0c29c8, 13450, 0x3ce4cdbe,
	"org/geothermal.org",  0x0b5cf4, 13466, 0xdb4795ac,
	"org/theme.org",       0x0a11ac, 25738, 0xf5ace8b0,
	"org/oside.org",       0x0e725c, 25634, 0x1e33b095,
	"org/heroend.org",     0x0f1598, 9722,  0xfc64d0d0,
	"org/scorching.org",   0x0faca4, 15994, 0xd09341e2,
	"org/quiet.org",       0x0f02f4, 4770,  0x0e95a468,
	"org/lastcave.org",    0x0d33a8, 18122, 0x469b38b9,
	"org/balcony.org",     0x09dbbc, 3082,  0x892345ca,
	"org/charge.org",      0x0d28d4, 2770,  0x10dec9d5,
	"org/lastbattle.org",  0x0cd650, 21122, 0x8888dac9,
	"org/credits.org",     0x0a7eac, 17898, 0xa9ed4834,
	"org/zombie.org",      0x10f180, 5346,  0xd217cc29,
	"org/breakdown.org",   0x09f5bc, 2570,  0xf80dd62a,
	"org/hell.org",        0x0be1f4, 18386, 0x93bbf277,
	"org/jenka2.org",      0x0c7ec8, 11986, 0xc095cbe1,
	"org/waterway.org",    0x0d7a74, 16962, 0xb533d72a,
	"org/seal.org",        0x09e7c8, 3570,  0x373988ad,
	"org/toroko.org",      0x0f3b94, 18482, 0xc202de07,
	"org/white.org",       0x109468, 23714, 0xcff0fb34,
   NULL, 0, 0, 0
};

char *org_data[42];
static int org_extracted;

bool extract_org(FILE *exefp)
{
   uint8_t *buffer;
   uint8_t *file;
   uint32_t length;
   uint32_t crc;
   bool first_crc_failure = true;
   
   if (org_extracted)
      return 0;

   memset(org_data, 0, sizeof(org_data));

   buffer = (uint8_t *)malloc(MAX_FILE_SIZE);

   for(int i=1;;i++)
   {
      if (!files[i].filename) break;
      file = buffer;
      length = files[i].length;

      // read data from exe
      fseek(exefp, files[i].offset, SEEK_SET);
      fread(file, files[i].length, 1, exefp);

      NX_DBG("file: %s\n", files[i].filename);

      // write out the file
      
      org_data[i] = (char *)malloc(files[i].length);
      memcpy(org_data[i], buffer, files[i].length);
   }

   free(buffer);
   org_extracted = 1;
   return 0;
}


static void createdir(const char *fname)
{
	char *dir = strdup(fname);
#if defined(_WIN32)
	char *ptr = strrchr(dir, '\\');
#else
	char *ptr = strrchr(dir, '/');
#endif
	if (ptr)
	{
		*ptr = 0;
		
		#if defined(_WIN32)
			_mkdir(dir);
		#else
			mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
		#endif
	}
	
	free(dir);
}

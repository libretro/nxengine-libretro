
#include <SDL.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../common/basics.h"
#include "../libretro/libretro_shared.h"
#include "extractfiles.fdh"
#include "../nx_logger.h"

#ifdef __MINGW32__
	#include <direct.h>
	#include <io.h>
#endif

#ifdef _XBOX
#include <xtl.h>
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

#ifdef _WIN32
static struct
{
	const char *filename;
	uint32_t offset;
	uint32_t length;
	uint32_t crc;
	const uint8_t *header;
}
files[] =
{
	"endpic\\credit01.bmp", 0x117047, 19293, 0xeb87b19b, credit_header,
	"endpic\\credit02.bmp", 0x11bbaf, 19293, 0x239c1a37, credit_header,
	"endpic\\credit03.bmp", 0x120717, 19293, 0x4398bbda, credit_header,
	"endpic\\credit04.bmp", 0x12527f, 19293, 0x44bae3ac, credit_header,
	"endpic\\credit05.bmp", 0x129de7, 19293, 0xd1b876ad, credit_header,
	"endpic\\credit06.bmp", 0x12e94f, 19293, 0x5a60082e, credit_header,
	"endpic\\credit07.bmp", 0x1334b7, 19293, 0xc1e9db91, credit_header,
	"endpic\\credit08.bmp", 0x13801f, 19293, 0xcbbcc7fa, credit_header,
	"endpic\\credit09.bmp", 0x13cb87, 19293, 0xfa7177b1, credit_header,
	"endpic\\credit10.bmp", 0x1416ef, 19293, 0x56390a07, credit_header,
	"endpic\\credit11.bmp", 0x146257, 19293, 0xff3d6d83, credit_header,
	"endpic\\credit12.bmp", 0x14adbf, 19293, 0x9e948dc2, credit_header,
	"endpic\\credit14.bmp", 0x14f927, 19293, 0x32b6ce2d, credit_header,
	"endpic\\credit15.bmp", 0x15448f, 19293, 0x88539803, credit_header,
	"endpic\\credit16.bmp", 0x158ff7, 19293, 0xc0ef9adf, credit_header,
	"endpic\\credit17.bmp", 0x15db5f, 19293, 0x8c5a003d, credit_header,
	"endpic\\credit18.bmp", 0x1626c7, 19293, 0x66bcbf22, credit_header,
	"endpic\\pixel.bmp",    0x16722f, 1373,  0x6181d0a1, pixel_header,
	"wavetable.dat",       0x110664, 25599, 0xcaa7b1dd, NULL,
	NULL
};
#else
static struct
{
	const char *filename;
	uint32_t offset;
	uint32_t length;
	uint32_t crc;
	const uint8_t *header;
}
files[] =
{
	"endpic/credit01.bmp", 0x117047, 19293, 0xeb87b19b, credit_header,
	"endpic/credit02.bmp", 0x11bbaf, 19293, 0x239c1a37, credit_header,
	"endpic/credit03.bmp", 0x120717, 19293, 0x4398bbda, credit_header,
	"endpic/credit04.bmp", 0x12527f, 19293, 0x44bae3ac, credit_header,
	"endpic/credit05.bmp", 0x129de7, 19293, 0xd1b876ad, credit_header,
	"endpic/credit06.bmp", 0x12e94f, 19293, 0x5a60082e, credit_header,
	"endpic/credit07.bmp", 0x1334b7, 19293, 0xc1e9db91, credit_header,
	"endpic/credit08.bmp", 0x13801f, 19293, 0xcbbcc7fa, credit_header,
	"endpic/credit09.bmp", 0x13cb87, 19293, 0xfa7177b1, credit_header,
	"endpic/credit10.bmp", 0x1416ef, 19293, 0x56390a07, credit_header,
	"endpic/credit11.bmp", 0x146257, 19293, 0xff3d6d83, credit_header,
	"endpic/credit12.bmp", 0x14adbf, 19293, 0x9e948dc2, credit_header,
	"endpic/credit14.bmp", 0x14f927, 19293, 0x32b6ce2d, credit_header,
	"endpic/credit15.bmp", 0x15448f, 19293, 0x88539803, credit_header,
	"endpic/credit16.bmp", 0x158ff7, 19293, 0xc0ef9adf, credit_header,
	"endpic/credit17.bmp", 0x15db5f, 19293, 0x8c5a003d, credit_header,
	"endpic/credit18.bmp", 0x1626c7, 19293, 0x66bcbf22, credit_header,
	"endpic/pixel.bmp",    0x16722f, 1373,  0x6181d0a1, pixel_header,
	"wavetable.dat",       0x110664, 25599, 0xcaa7b1dd, NULL,
	NULL
};
#endif

extern signed short wavetable[100][256];

bool extract_files(FILE *exefp)
{
uint8_t *buffer;
uint8_t *file;
uint32_t length;
uint32_t crc;
bool check_crc = true;
bool first_crc_failure = true;

	buffer = (uint8_t *)malloc(MAX_FILE_SIZE);
	crc_init();
	
	for(int i=0;;i++)
	{
		if (!files[i].filename) break;
		char outfilename[1024];
		retro_create_path_string(outfilename, sizeof(outfilename), g_dir, files[i].filename);
		
		NX_LOG("[ %s ]\n", outfilename);
		
		// initialize header if any
		file = buffer;
		length = files[i].length;
		
		if (files[i].header)
		{
			memcpy(buffer, files[i].header, HEADER_LEN);
			file += HEADER_LEN;
			length += HEADER_LEN;
		}
		
		// read data from exe
		fseek(exefp, files[i].offset, SEEK_SET);
		fread(file, files[i].length, 1, exefp);
		
		if (check_crc)
		{
			crc = crc_calc(file, files[i].length);
			if (crc != files[i].crc)
			{
				NX_ERR("File '%s' failed CRC check.\n", outfilename);
				first_crc_failure = false;
			}
		}
      
      fprintf(stderr, "file: %s\n", files[i].filename);
      if (strcmp(files[i].filename, "wavetable.dat") == 0)
      {
         fprintf(stderr, "found wavetable.dat\n");
         // wavetable.dat
         signed char *ptr = (signed char*)&buffer[0];
         int wav, sampl;

         for(wav=0;wav<100;wav++)
            for(sampl=0;sampl<256;sampl++)
               wavetable[wav][sampl] = (signed short)((int)(*ptr++) << 8); // 256 = (32768 / 128)-- convert to 16-bit

         continue;
      }
		
		// write out the file
		createdir(outfilename);
		
		FILE *fp = fopen(outfilename, "wb");
		if (!fp)
		{
			NX_ERR("Failed to open '%s' for writing.\n", outfilename);
			free(buffer);
			return 1;
		}
		
		fwrite(buffer, length, 1, fp);
		fclose(fp);
	}
	
	free(buffer);
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
		
      #if defined(_XBOX)
      CreateDirectory(dir, NULL);
		#elif defined(_WIN32)
			_mkdir(dir);
		#else
			mkdir(dir, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
		#endif
	}
	
	free(dir);
}

#include <stdio.h>
#include <stdint.h>
#include <streams/file_stream.h>
#include "nx.h"
#include "niku.fdh"
#include "libretro_shared.h"

/*
	290.rec contains the tick value 4 times, followed by the 4 byte key
	to decrypt each instance, for a total of 20 bytes.
*/

/* load the contents of 290.rec and store in value_out.
 *
 * Returns 0 on success. If there is no such file or an error occurs, 
 * writes 0 to value_out.
 */
bool niku_load(uint32_t *value_out)
{
   char fname_tmp[1024];
   uint8_t buffer[20];
   uint32_t *result = (uint32_t *)buffer;
   int i, j;
   RFILE *fp = NULL;

   const char *fname = getfname();

   retro_create_path_string(fname_tmp, sizeof(fname_tmp), g_dir, fname);

   fp = filestream_open(fname_tmp, RETRO_VFS_FILE_ACCESS_READ, RETRO_VFS_FILE_ACCESS_HINT_NONE);
   if (!fp)
   {
#ifdef DEBUG
      NX_ERR("niku_load: couldn't open file '%s'", fname_tmp);
#endif
      if (value_out)
         *value_out = 0;
      return 1;
   }

   filestream_read(fp, buffer, 20);
   filestream_close(fp);

   for(i=0;i<4;i++)
   {
      uint8_t key = buffer[i+16];

      j = i * 4;
      buffer[j] -= key;
      buffer[j+1] -= key;
      buffer[j+2] -= key;
      buffer[j+3] -= (key / 2);
   }

   if ((result[0] != result[1]) || 
         (result[0] != result[2]) || 
         (result[0] != result[3]))
   {
#ifdef DEBUG
      NX_ERR("niku_load: value mismatch; '%s' corrupt", fname_tmp);
#endif
      if (value_out) *value_out = 0;
   }
   else
   {
#ifdef DEBUG
      NX_LOG("niku_load: loaded value 0x%x from %s", *result, fname_tmp);
#endif
      if (value_out) *value_out = *result;
   }

   return 0;
}

/* save the timestamp in value to 290.rec. */
bool niku_save(uint32_t value)
{
   int i;
   char fname_tmp[1024];
   uint8_t buf_byte[20];
   const char *fname;
   RFILE *fp = NULL;
   uint32_t *buf_dword = (uint32_t *)buf_byte;

   /* place values */
   buf_dword[0] = value;
   buf_dword[1] = value;
   buf_dword[2] = value;
   buf_dword[3] = value;

   /* generate keys */
   buf_byte[16] = random(0, 255);
   buf_byte[17] = random(0, 255);
   buf_byte[18] = random(0, 255);
   buf_byte[19] = random(0, 255);

   /* encode each copy */
   for(i = 0; i < 4; i++)
   {
      uint8_t *ptr = (uint8_t *)&buf_dword[i];
      uint8_t key = buf_byte[i+16];

      ptr[0] += key;
      ptr[1] += key;
      ptr[2] += key;
      ptr[3] += key / 2;
   }

   fname = getfname();

   retro_create_path_string(fname_tmp, sizeof(fname_tmp), g_dir, fname);

   fp = filestream_open(fname_tmp, RETRO_VFS_FILE_ACCESS_WRITE,
         RETRO_VFS_FILE_ACCESS_HINT_NONE);
   if (!fp)
   {
      NX_ERR("niku_save: failed to open '%s'", fname_tmp);
      return 1;
   }

   filestream_write(fp, buf_byte, 20);
   filestream_close(fp);

#ifdef DEBUG
   NX_LOG("niku_save: wrote value 0x%08x", value);
#endif
   return 0;
}

/* might do stuff with sprintf here to place it in other subdirectories, etc.
 * is just here for expansion.
 */
static const char *getfname(void)
{
   return "290.rec";
}







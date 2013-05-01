#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include "mwrapper.h"

int mgetc(char **fp)
{
   unsigned char volatile *f = *((unsigned char volatile **)fp);
   unsigned char volatile c = *f;
   (*fp)++;
   return c;
}

uint16_t mgeti(char **fp)
{
   uint16_t a, b;
	a = mgetc(fp);
	b = mgetc(fp);
	return (b << 8) | a;
}

uint32_t mgetl(char **fp)
{
   uint32_t a, b, c, d;
	a = mgetc(fp);
	b = mgetc(fp);
	c = mgetc(fp);
	d = mgetc(fp);
	return (d<<24)|(c<<16)|(b<<8)|(a);
}

/* This routine returns the size of the file it is called with. */

#if 0
unsigned get_file_size (const char * file_name)
{
   struct stat sb;
   if (stat (file_name, & sb) != 0) {
      fprintf (stderr, "'stat' failed for '%s': %s.\n",
            file_name, strerror (errno));
      exit (EXIT_FAILURE);
   }
   return sb.st_size;
}

/* This routine reads the entire file into memory. */

unsigned char *read_whole_file (const char * file_name)
{
   unsigned s;
   unsigned char * contents;
   FILE * f;
   size_t bytes_read;
   int status;

   s = get_file_size (file_name);
   contents = (unsigned char*)malloc (s + 1);
   if (! contents) {
      fprintf (stderr, "Not enough memory.\n");
      return NULL;
   }

   f = fopen (file_name, "r");
   if (! f) {
      fprintf (stderr, "Could not open '%s': %s.\n", file_name,
            strerror (errno));
      return NULL;
   }
   bytes_read = fread (contents, sizeof (unsigned char), s, f);
   if (bytes_read != s) {
      fprintf (stderr, "Short read of '%s': expected %d bytes "
            "but got %d: %s.\n", file_name, s, bytes_read,
            strerror (errno));
      return NULL;
   }
   status = fclose (f);
   if (status != 0) {
      fprintf (stderr, "Error closing '%s': %s.\n", file_name,
            strerror (errno));
      return NULL;
   }
   return contents;
}
#endif

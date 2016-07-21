#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct file_data CFILE;

bool cachefiles_init(FILE *exefp);
CFILE *copen(const char *fname, const char *mode);
void cclose(CFILE *f);
void cseek(CFILE *f, int offset, int origin);
size_t ctell(CFILE *f);
size_t cread(void *ptr, size_t size, size_t count, CFILE *f);
int cgetc(CFILE *f);
uint16_t cgeti(CFILE *f);
uint32_t cgetl(CFILE *f);
bool cverifystring(CFILE *f, const char *str);
void *cfile_pointer(CFILE *f);
size_t cfile_size(CFILE *f);

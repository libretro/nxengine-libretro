#ifndef M_WRAPPER_H
#define M_WRAPPER_H

int mgetc(char **fp);
uint16_t mgeti(char **fp);
uint32_t mgetl(char **fp);

unsigned get_file_size (const char * file_name);
unsigned char *read_whole_file (const char * file_name);

#endif

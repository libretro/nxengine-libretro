#ifndef _COMMON_H
#define _COMMON_H

#include <stdio.h>
#include <stdint.h>

bool file_exists(const char *fname);
bool fverifystring(FILE *fp, const char *str);
void fresetboolean(void);
char fbooleanread(FILE *fp);
void fputstringnonull(const char *buf, FILE *fp);
void fputi(unsigned short word, FILE *fp);
void fbooleanwrite(char bit, FILE *fp);
void fbooleanflush(FILE * fp);
unsigned int fgetl(FILE *fp);
void fputi(unsigned short word,  FILE *fp);
void fputl(unsigned int word, FILE *fp);
unsigned short fgeti(FILE *fp);
int fgeticsv(FILE *fp);
double fgetfcsv(FILE *fp);

uint32_t getrand();
void seedrand(uint32_t newseed);

#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <ctype.h>

#include <streams/file_stream.h>

#include "misc.fdh"

#ifdef _WIN32
#include "msvc_compat.h"
#endif

/* Forward declarations */
int64_t rfread(void* buffer,
		size_t elem_size, size_t elem_count, RFILE* stream);
int rfputc(int character, RFILE * stream);
int rfgetc(RFILE* stream);
int rfclose(RFILE* stream);
RFILE* rfopen(const char *path, const char *mode);
int rfprintf(RFILE * stream, const char * format, ...);
int64_t rfwrite(void const* buffer,
		size_t elem_size, size_t elem_count, RFILE* stream);

#ifndef MSB_FIRST
uint16_t rfgeti(RFILE *fp)
{
	uint16_t value;
	rfread(&value, 2, 1, fp);
	return value;
}

uint32_t fgetl(FILE *fp)
{
	uint32_t value;
	fread(&value, 4, 1, fp);
	return value;
}

uint32_t rfgetl(RFILE *fp)
{
	uint32_t value;
	rfread(&value, 4, 1, fp);
	return value;
}

void rfputi(uint16_t word, RFILE *fp)
{
	rfwrite(&word, 2, 1, fp);
}

void rfputl(uint32_t word, RFILE *fp)
{
	rfwrite(&word, 4, 1, fp);
}

double rfgetfloat(RFILE *fp)
{
	char buf[8];
	double *float_ptr;
	int i;

	for(i=0;i<4;i++) rfgetc(fp);
	for(i=0;i<8;i++) buf[i] = rfgetc(fp);

	float_ptr = (double *)&buf[0];
	return *float_ptr;
}

double fgetfloat(FILE *fp)
{
	char buf[8];
	double *float_ptr;
	int i;

	for(i=0;i<4;i++) fgetc(fp);
	for(i=0;i<8;i++) buf[i] = fgetc(fp);

	float_ptr = (double *)&buf[0];
	return *float_ptr;
}
#else
uint16_t rfgeti(RFILE *fp)
{
	uint16_t a = rfgetc(fp);
	uint16_t b = rfgetc(fp);
	return (b << 8) | a;
}

uint32_t rfgetl(RFILE *fp)
{
	uint32_t a = rfgetc(fp);
	uint32_t b = rfgetc(fp);
	uint32_t c = rfgetc(fp);
	uint32_t d = rfgetc(fp);
	return (d<<24)|(c<<16)|(b<<8)|(a);
}

uint32_t fgetl(FILE *fp)
{
	uint32_t a = fgetc(fp);
	uint32_t b = fgetc(fp);
	uint32_t c = fgetc(fp);
	uint32_t d = fgetc(fp);
	return (d<<24)|(c<<16)|(b<<8)|(a);
}

void rfputi(uint16_t word, RFILE *fp)
{
	rfputc(word, fp);
	rfputc(word >> 8, fp);
}

void rfputl(uint32_t word, RFILE *fp)
{
	rfputc(word, fp);
	rfputc(word >> 8, fp);
	rfputc(word >> 16, fp);
	rfputc(word >> 24, fp);
}

double rfgetfloat(RFILE *fp)
{
	char buf[8];
	double *float_ptr;
	int i;

	for(i=0;i<4;i++) rfgetc(fp);
	for(i=0;i<8;i++) buf[7 - i] = rfgetc(fp);

	float_ptr = (double *)&buf[0];
	return *float_ptr;
}

double fgetfloat(FILE *fp)
{
	char buf[8];
	double *float_ptr;
	int i;

	for(i=0;i<4;i++) fgetc(fp);
	for(i=0;i<8;i++) buf[7 - i] = fgetc(fp);

	float_ptr = (double *)&buf[0];
	return *float_ptr;
}
#endif

// write a string to a file-- does NOT null-terminate it
void rfputstringnonull(const char *buf, RFILE *fp)
{
	if (buf[0])
		rfprintf(fp, "%s", buf);
}

bool rfverifystring(RFILE *fp, const char *str)
{
	int i;
	char result = 1;
	int stringlength = strlen(str);

	for(i=0;i<stringlength;i++)
		if (rfgetc(fp) != str[i])
			result = 0;

	return result;
}

// read data from a file until CR
void fgetline(FILE *fp, char *str, int maxlen)
{
	int k;
	str[0] = 0;
	fgets(str, maxlen - 1, fp);

	// trim the CRLF that fgets appends
	for(k=strlen(str)-1;k>=0;k--)
	{
		if (str[k] != 13 && str[k] != 10) break;
		str[k] = 0;
	}
}

static uint32_t seed = 0;

// return a random number between min and max inclusive
int nx_random(int min, int max)
{
	int range, val;

	if (max < min)
	{
		min ^= max;
		max ^= min;
		min ^= max;
	}

	range = (max - min);

	if (range >= RAND_MAX)
		return 0;

	val = getrand() % (range + 1);
	return val + min;
}

uint32_t getrand(void)
{
	seed = (seed * 0x343FD) + 0x269EC3;
	return seed;
}

void seedrand(uint32_t newseed)
{
	seed = newseed;
}

bool strbegin(const char *bigstr, const char *smallstr)
{
	int i;

	for(i=0;smallstr[i];i++)
		if (bigstr[i] != smallstr[i]) return false;

	return true;
}

// a strncpy that works as you might expect
void maxcpy(char *dst, const char *src, int maxlen)
{
	int len = strlen(src);

	if (len >= maxlen)
	{
		if (maxlen >= 2) memcpy(dst, src, maxlen - 2);
		if (maxlen >= 1) dst[maxlen - 1] = 0;
	}
	else
		memcpy(dst, src, len + 1);
}

static int boolbyte, boolmask_r, boolmask_w;

// prepare for a boolean read operation
void fresetboolean(void)
{
	boolmask_r = 256;
	boolmask_w = 1;
	boolbyte = 0;
}

// read a boolean value (a single bit) from a file
char rfbooleanread(RFILE *fp)
{
	char value;

	if (boolmask_r == 256)
	{
		boolbyte   = rfgetc(fp);
		boolmask_r = 1;
	}

	value        = (boolbyte & boolmask_r) ? 1:0;
	boolmask_r <<= 1;
	return value;
}

void rfbooleanwrite(char bit, RFILE *fp)
{
	if (boolmask_w == 256)
	{
		rfputc(boolbyte, fp);
		boolmask_w = 1;
		boolbyte = 0;
	}

	if (bit)
		boolbyte |= boolmask_w;

	boolmask_w <<= 1;
}

void rfbooleanflush(RFILE *fp)
{
	rfputc(boolbyte, fp);
	boolmask_w = 1;
}

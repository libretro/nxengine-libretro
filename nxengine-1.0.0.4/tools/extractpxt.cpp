
#include <stdio.h>
#include <string.h>

#include "xtract.fdh"

struct PXTFields
{
	char *name;
	char is_integer;
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
NULL, 0
};


char extract_pxt(char *fname)
{
FILE *fp;
int offset_start = 0x4115ff - 0x400000;
unsigned char adder[] = { 0x83, 0xC4, 0x0C,			//add esp, 0c
						  0x03, 0x45, 0xFC,			//add eax, dword ptr[ebp-4]
						  0x89, 0x45, 0xFC };		//mov dword ptr ss:[ebp-4], eax
unsigned char ch;
static const char *error = "extract_pxt Error byte %02x expect %02x\n";
#define NUMS	128
struct
{
	int id, chan, offs;
} snd[NUMS];
int ns;
int i, s, c;
struct
{
	int values[20];
	double fp_values[20];
} chan[4];
char outfname[80];
FILE *fpo;

#define VERIFY(Q)	\
{		\
	ch = fgetc(fp);	\
	if (ch != Q)	\
	{	\
		lprintf(error, ch, Q);	\
		goto fail;	\
	}	\
}

	// pull sound offsets out of exe
	fp = fileopen(fname, "rb");
	
	fseek(fp, offset_start, SEEK_SET);
	
	ns = 0;
	for(s=0;s<NUMS;s++)
	{
		ch = fgetc(fp);
		if (ch==0x6a || ch==0x68)
		{		// PUSH - start of another sound
			if (ch==0x6a)
				snd[ns].id = fgetc(fp);
			else
				snd[ns].id = fgetl(fp);
			
			VERIFY(0x6a);			// PUSH
			snd[ns].chan = fgetc(fp);
			VERIFY(0x68);			// PUSH (32-bit)
			snd[ns].offs = fgetl(fp) - 0x400000;
			VERIFY(0xe8);			// CALL
			fgetl(fp);				// skip the call destination
			
			for(i=0;i<9;i++) VERIFY(adder[i]);
			
			if (++ns > 128) { lprintf("too many sounds"); goto fail; }
		}
		else if (ch==0x8b) break;
		else { lprintf("Endofsound fail %02x xpect 6a\n", ch); goto fail; }
	}
	
	
	// now pull out the sound data
	lprintf("%d sounds\n", ns);
	for(s=0;s<ns;s++)
	{
		fseek(fp, snd[s].offs, SEEK_SET);
		//lprintf("id %02x chanl %d offs %08x\n", snd[s].id, snd[s].chan, snd[s].offs);
		
		sprintf(outfname, "pxt/fx%02x.pxt", snd[s].id);
		fpo = fileopen(outfname, "wb");
		if (!fpo) { lprintf("extract_pxt: failed to open %s\n", fname); goto fail; }
		
		memset(chan, 0, sizeof(chan));
		
		// load data
		for(c=0;c<snd[s].chan;c++)
		{
			for(i=0;fields[i].name;i++)
			{
				if (fields[i].is_integer)
				{
					chan[c].values[i] = fgetl(fp);
				}
				else
				{
					chan[c].fp_values[i] = fgetfloat(fp);
				}
			}
			// skipping padding between sections
			if (fgetl(fp) != 0) { lprintf("Out of sync"); goto fail; }
		}
		
		// write human-readable section
		for(c=0;c<4;c++)
		{
			for(i=0;fields[i].name;i++)
			{
				if (fields[i].is_integer)
					fprintf(fpo, "%s:%d\r\n", fields[i].name, chan[c].values[i]);
				else
					fprintf(fpo, "%s:%.2f\r\n", fields[i].name, chan[c].fp_values[i]);
			}
			fprintf(fpo, "\r\n");
		}
		
		// write machine-readable section
		for(c=0;c<4;c++)
		{
			fprintf(fpo, "{");
			
			for(i=0;fields[i].name;i++)
			{
				const char *suffix = (fields[i+1].name == NULL) ? "},\r\n" : ",";
				if (fields[i].is_integer) fprintf(fpo, "%d%s", chan[c].values[i], suffix);
									 else fprintf(fpo, "%.2f%s", chan[c].fp_values[i], suffix);
			}
		}
		
		fclose(fpo);
		lprintf("wrote %s\n", outfname);
	}
	
	fclose(fp);
	return 0;
fail: ;
	fclose(fp); return 1;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sifloader.h"
#include "sifloader.fdh"
#include "../nx.h"
#include "../extract-auto/cachefiles.h"

#define SIF_MAGICK	'SIF2'		// SIF magick and version denotation; first 4 bytes of file

SIFLoader::SIFLoader()
{
	fFP = NULL;
}

SIFLoader::~SIFLoader()
{
	ClearIndex();
	if (fFP) cclose(fFP);
}

/*
void c------------------------------() {}
*/

void SIFLoader::ClearIndex()
{
	for(int i=0;;i++)
	{
		SIFIndexEntry *entry = (SIFIndexEntry *)fIndex.ItemAt(i);
		if (!entry) break;
		
		if (entry->data) free(entry->data);
		delete entry;
	}
	
	fIndex.MakeEmpty();
}

void SIFLoader::CloseFile()
{
	ClearIndex();
	
	if (fFP)
	{
		cclose(fFP);
		fFP = NULL;
	}
}

/*
void c------------------------------() {}
*/

bool SIFLoader::LoadHeader(const char *filename)
{
CFILE *fp;
uint32_t magick;

	ClearIndex();
	
	if (fFP) cclose(fFP);
	fp = fFP = copen(filename, "rb");
	
	if (!fp)
	{
		NX_ERR("SIFLoader::LoadHeader: failed to open file '%s'\n", filename);
		return 1;
	}
	
	if ((magick = cgetl(fp)) != SIF_MAGICK)
	{
		NX_ERR("SIFLoader::LoadHeader: magick check failed--this isn't a SIF file or is wrong version?\n");
		NX_ERR(" (expected %08x, got %08x)\n", SIF_MAGICK, magick);
		return 1;
	}
	
	int nsections = cgetc(fp);
	NX_LOG("SIFLoader::LoadHeader: read index of %d sections\n", nsections);
	
	for(int i=0;i<nsections;i++)
	{
		SIFIndexEntry *entry = new SIFIndexEntry;
		
		entry->type = cgetc(fp);		// section type
		entry->foffset = cgetl(fp);		// absolute offset in file
		entry->length = cgetl(fp);		// length of section data
		entry->data = NULL;				// we won't load it until asked
		
		fIndex.AddItem(entry);
		//NX_LOG("  - Sect%02d @ %04x\n", entry->type, entry->foffset);
	}
	
	// ..leave file handle open, its ok
	return 0;
}

// load into memory and return a pointer to the section of type 'type',
// or NULL if the file doesn't have a section of that type.
uint8_t *SIFLoader::FindSection(int type, int *length_out)
{
	// try and find the section in the index
	for(int i=0;;i++)
	{
		SIFIndexEntry *entry = (SIFIndexEntry *)fIndex.ItemAt(i);
		if (!entry) break;
		
		if (entry->type == type)
		{	// got it!
			
			// haven't loaded it yet? need to fetch it from file?
			if (!entry->data)
			{
				if (!fFP)
				{
					NX_ERR("SIFLoader::FindSection: entry found and need to load it, but file handle closed\n");
					if (length_out) *length_out = 0;
					return NULL;
				}
				
				NX_LOG("Loading SIF section %d from address %04x\n", type, entry->foffset);
				
				entry->data = (uint8_t *)malloc(entry->length);
				cseek(fFP, entry->foffset, SEEK_SET);
				cread(entry->data, entry->length, 1, fFP);
			}
			
			if (length_out) *length_out = entry->length;
			return entry->data;
		}
	}
	
	if (length_out) *length_out = 0;
	return NULL;
}

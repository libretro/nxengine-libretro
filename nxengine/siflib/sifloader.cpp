
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sifloader.h"
#include "sifloader.fdh"
#include "../nx.h"
#include "../extract-auto/cachefiles.h"

// SIF magick and version denotation; first 4 bytes of file
#define SIF_MAGICK	'SIF2'

SIFLoader::SIFLoader()
{
	fFP = NULL;
}

SIFLoader::~SIFLoader()
{
	ClearIndex();
	if (fFP) cclose(fFP);
}

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

bool SIFLoader::LoadHeader(const char *filename)
{
   CFILE *fp;
   uint32_t magick;

   ClearIndex();

   if (fFP)
      cclose(fFP);
   fp = fFP = copen(filename, "rb");

   if (!fp)
      return 1;

   if ((magick = cgetl(fp)) != SIF_MAGICK)
      return 1;

   int nsections = cgetc(fp);

   for(int i=0;i<nsections;i++)
   {
      SIFIndexEntry *entry = new SIFIndexEntry;

      entry->type          = cgetc(fp);		// section type
      entry->foffset       = cgetl(fp);		// absolute offset in file
      entry->length        = cgetl(fp);		// length of section data
      entry->data          = NULL;				// we won't load it until asked

      fIndex.AddItem(entry);
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
               if (length_out) *length_out = 0;
               return NULL;
            }

            entry->data = (uint8_t *)malloc(entry->length);
            cseek(fFP, entry->foffset, SEEK_SET);
            cread(entry->data, entry->length, 1, fFP);
         }

         if (length_out) *length_out = entry->length;
         return entry->data;
      }
   }

   if (length_out)
      *length_out = 0;
   return NULL;
}

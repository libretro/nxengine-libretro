
// Sound System
// more or less, my own version of SDL_mixer

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../common/basics.h"

#include "../nx_logger.h"
#include "sslib.h"
#include "sslib.fdh"

#define SDL_MIX_MAXVOLUME 128

SSChannel channel[SS_NUM_CHANNELS];

uint8_t *mixbuffer = NULL;
int mix_pos;

int lockcount = 0;

// add the contents of the chunk at head to the mix_buffer.
// don't add more than bytes.
// return the number of bytes that were added.
static int AddBuffer(SSChannel *chan, int bytes)
{
	SSChunk *chunk = &chan->chunks[chan->head];
	
	if (bytes > chunk->bytelength)
		bytes = chunk->bytelength;
	
	// don't copy past end of chunk
	if (chunk->bytepos+bytes > chunk->bytelength)
	{
		// add it to list of finished chunks
		chan->FinishedChunkUserdata[chan->nFinishedChunks++] = chunk->userdata;
		
		// only add what's left. and advance the head pointer to the next chunk.
		bytes = chunk->bytelength - chunk->bytepos;
		if (++chan->head >= MAX_QUEUED_CHUNKS)
         chan->head = 0;
	}
	
	memcpy(&mixbuffer[mix_pos], &chunk->bytebuffer[chunk->bytepos], bytes);
	mix_pos += bytes;
	chunk->bytepos += bytes;
	
	return bytes;
}

void mixaudio(int16_t *stream, size_t len_samples)
{
	int bytes_copied;
	int bytestogo;
	int c;
	int i;

	size_t len = len_samples * sizeof(int16_t);

	// get data for all channels and add it to the mix
	for(c=0;c<SS_NUM_CHANNELS;c++)
	{
		if (channel[c].head==channel[c].tail) continue;
		
		bytestogo = len;
		mix_pos = 0;
		while(bytestogo > 0)
		{
			bytes_copied = AddBuffer(&channel[c], bytestogo);
			bytestogo -= bytes_copied;
			
			if (channel[c].head==channel[c].tail)
			{
				if (bytestogo)
					memset(&mixbuffer[mix_pos], 0, bytestogo);
				
				break;
			}
		}
	
	// tell any callbacks that had a chunk finish, that their chunk finished
	const int16_t *mixbuf = (const int16_t*)mixbuffer;

	for(unsigned i = 0; i < len_samples; i++)
	{
		int32_t current = stream[i];
		current += (int32_t)mixbuf[i] * channel[c].volume / (2 * SDL_MIX_MAXVOLUME);
		if (current > 0x7fff)
			stream[i] = 0x7fff;
		else if (current < -0x8000)
			stream[i] = -0x8000;
		else
			stream[i] = current;
	}
	}

	for(c=0;c<SS_NUM_CHANNELS;c++)
	{
		if (channel[c].FinishedCB)
		{
			for(i=0;i<channel[c].nFinishedChunks;i++)
            (*channel[c].FinishedCB)(c, channel[c].FinishedChunkUserdata[i]);
		}
		
		channel[c].nFinishedChunks = 0;
	}
}

char SSInit(void)
{
	mixbuffer = (uint8_t *)malloc(4096 * 2 * 2);
	
	// zero everything in all channels
	memset(channel, 0, sizeof(channel));
	for(int i=0;i<SS_NUM_CHANNELS;i++)
		channel[i].volume = SDL_MIX_MAXVOLUME;
	
	NX_LOG("sslib: initilization was successful.\n");
	
	lockcount = 0;
	return 0;
}

void SSClose(void)
{
	if (mixbuffer) free(mixbuffer);
}

/*
void c------------------------------() {}
*/

// reserves a channel so that it will not be returned by SSFindFreeChannel and no
// sounds will be allocated to it by SSEnqueueChunk with a -1 parameter.
// thus, the channel can only be played on by explicitly playing a sound to it.
void SSReserveChannel(int c)
{
	channel[c].reserved = 1;
}

// returns the first available channel that is not playing a sound.
// if all chans are full, returns -1.
int SSFindFreeChannel(void)
{
int i;
	for(i=0;i<SS_NUM_CHANNELS;i++)
	{
		if (channel[i].head==channel[i].tail && !channel[i].reserved) return i;
	}
	return -1;
}

// enqueue a chunk of sound to a channel.
// c:			channel to play on, or pass -1 to automatically find a free one
// buffer:		16-bit S16 22050Hz audio data to play
// len:			buffer length in stereo samples. len=1 means 4 bytes: *2 for 16-bit, and *2 for stereo.
// userdata:	a bit of application-defined data to associate with the chunk,
// 				such as a game sound ID. this value will be passed to the FinishedCallback()
//				when the chunk completes.
// FinishedCB:	an optional callback function to call when the chunk stops playing.
//
// returns:		the channel sound was started on, or -1 if failure.
int SSEnqueueChunk(int c, signed short *buffer, int len, int userdata, void(*FinishedCB)(int, int))
{
SSChannel *chan;
SSChunk *chunk;

	if (c >= SS_NUM_CHANNELS)
	{
		NX_ERR("SSEnqueueChunk: channel %d is higher than SS_NUM_CHANNELS\n", c);
		return -1;
	}
	
	if (c < 0) c = SSFindFreeChannel();
	if (c==-1)
	{
		NX_ERR("SSEnqueueChunk: no available sound channels!\n");
		return -1;
	}
	
	chan = &channel[c];
	
	chan->FinishedCB = FinishedCB;
	
	chunk = &chan->chunks[chan->tail];
	chunk->buffer = buffer;
	chunk->length = len;							// in 16-bit stereo samples
	chunk->userdata = userdata;
	
	chunk->bytebuffer = (signed char *)buffer;
	chunk->bytelength = chunk->length * 2 * 2;		// in bytes
	
	chunk->bytepos = 0;
	
	// advance tail pointer
	if (++chan->tail >= MAX_QUEUED_CHUNKS) chan->tail = 0;
	
	if (chan->tail==chan->head)
	{
		NX_ERR("SS: overqueued channel %d; Bad Things about to happen\n", c);
		return -1;
	}
	
	return c;
}

// works like SSEnqueueChunk, only it does not enqueue. Instead, if a sound
// is already playing on the channel, it is stopped and the new sound takes it's place.
// if c==-1, it acts identically to SSEnqueueChunk since a "free channel" by definition
// has no existing sound to be affected by a queueing operation.
int SSPlayChunk(int c, signed short *buffer, int len, int userdata, void(*FinishedCB)(int, int))
{
	if (c != -1) SSAbortChannel(c);
	
	return SSEnqueueChunk(c, buffer, len, userdata, FinishedCB);
}

// returns true if channel c is currently playing
char SSChannelPlaying(int c)
{
	return (channel[c].head != channel[c].tail);
}

// returns the userdata member of the currently playing chunk on channel c.
// if channel c is not playing, the results are undefined.
int SSGetCurUserData(int c)
{
int result;

	if (channel[c].head != channel[c].tail)
		result = (channel[c].chunks[channel[c].head].userdata);
	else
	{
		NX_ERR("SSGetUserData: channel %d is not playing!\n", c);
		result = -1;
	}
	
	return result;
}

// returns the currently playing sample within the currently playing chunk
// of channel c. If no chunk is playing, the results are undefined.
// as with "len" parameter to SSEnqueueChunk, the count counts the two
// components of a stereo sample as a single sample.
int SSGetSamplePos(int c)
{
   int result;

	if (channel[c].head != channel[c].tail)
	{
		result = (channel[c].chunks[channel[c].head].bytepos / 4);
	}
	else
	{
		NX_ERR("SSGetSamplePos: channel %d is not playing!\n", c);
		result = -1;
	}
	
	return result;
}


// if a sound is playing on channel c, stops it immediately.
// if not, does nothing.
void SSAbortChannel(int c)
{
	channel[c].head = channel[c].tail;
}


// aborts all sounds with a userdata value matching "ud".
void SSAbortChannelByUserData(int ud)
{
	for(int c=0;c<SS_NUM_CHANNELS;c++)
		if (SSChannelPlaying(c) && SSGetCurUserData(c)==ud)
			SSAbortChannel(c);
}

// changes the volume of a channel.
// any currently playing chunks are immediately affected, and any future chunks queued
// will have the new volume setting, until the SSSetVolume function is removed.
void SSSetVolume(int c, int newvol)
{
	channel[c].volume = newvol;
}

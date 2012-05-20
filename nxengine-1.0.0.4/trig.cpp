
#include <math.h>
#include "nx.h"

#include "trig.fdh"

signed int sin_table[256];
signed int tan_table[64];


char trig_init(void)
{
int degrees;

// converts from 0-256 scale to 0-360 scale, then from degrees to radians
	#define PIBT 	((360.00f / 256.00f) * (3.14159265f / 180.00f))
	
	for(degrees=0;degrees<256;degrees++)
	{
		sin_table[degrees] = (int)(sin((double)degrees * PIBT) * (1 << CSF));
	}
	
	for(degrees=0;degrees<64;degrees++)
	{
		tan_table[degrees] = (int)(tan((double)degrees * PIBT) * (1 << 13));
	}
	
	//SetFullscreen(1);
	return 0;
}

// given an angle and a speed, places the X and Y speeds in xs and ys.
// note: the output values _ARE_ CSF'd, despite the >>= CSF done on them at the end.
void vector_from_angle(uint8_t angle, int speed, int *xs, int *ys)
{
	if (ys)
	{
		*ys = sin_table[angle];
		*ys *= speed; *ys >>= CSF;
	}
	
	if (xs)
	{
		angle += 64;			// wraps at 255 because it's a char
		*xs = sin_table[angle];
		
		// what's going on here is that when we calculated sin_table, we could not hold the
		// fractional (0-1.00f) values outputted from sin(), so we scaled them from 0-0x200.
		// so now we basically are >>= CSFing the value back to it's original 0-1.00, then
		// multiplying by speed. We're just doing it backwards so as the precision will stay.
		// which is ok because multiplication and division are on the same level of OoO.
		*xs *= speed; *xs >>= CSF;
	}
}

int xinertia_from_angle(uint8_t angle, int speed)
{
	angle += 64;
	int xs = sin_table[angle];
	xs *= speed; xs >>= CSF;
	
	return xs;
}

int yinertia_from_angle(uint8_t angle, int speed)
{
	int ys = sin_table[angle];
	ys *= speed; ys >>= CSF;
	
	return ys;
}

// give it your position and a target position, and it tells you what angle you should travel at.
uint8_t GetAngle(int curx, int cury, int tgtx, int tgty)
{
int xdist, ydist;
int ratio;
int angle;

	xdist = (tgtx - curx);
	ydist = (tgty - cury);
	
	if (xdist==0)
	{	// fixup for undefined slope
		if (tgty > cury) return 0x40;		// straight down
		return 0xC0;						// straight up
	}
	
	// (ydist / xdist) * 512	[scale it for integer floating point]
	ratio = (abs(ydist) << 13) / abs(xdist);
	
	if (ratio > tan_table[63])
	{
		angle = 0x40;
	}
	else
	{
		for(angle=0;angle<64;angle++)
		{
			if (tan_table[angle] >= ratio) break;
		}
	}
	
	if (curx > tgtx) angle = 0x80 - angle;
	if (cury > tgty) angle = 0x100 - angle;
	return angle;
}

/*
void c------------------------------() {}
*/

// convenience function.
//  * spawn an object at o's action point.
//  * launch it at the player at speed.
//  * introduce "rand_variance" random error/variation into the launch angle.
void EmFireAngledShot(Object *o, int objtype, int rand_variance, int speed)
{
Object *shot;

	shot = SpawnObjectAtActionPoint(o, objtype);
	ThrowObjectAtPlayer(shot, rand_variance, speed);
}


// like EmFireAngledShot, but it's throws an already existing object
// instead of spawning a new one
void ThrowObjectAtPlayer(Object *o, int rand_variance, int speed)
{
	ThrowObject(o, player->x, player->y, rand_variance, speed);
}

// set the x&y inertia of object o so that it travels towards [destx,desty].
// rand_variance is a random amount of inaccuracy, in 0-255 degrees, to introduce
// into the toss.
// speed is how quickly to throw the object, in CSF'd coordinates.
void ThrowObject(Object *o, int destx, int desty, int rand_variance, int speed)
{
	uint8_t angle = GetAngle(o->x, o->y, destx, desty);
	
	if (rand_variance)
		angle += random(-rand_variance, rand_variance);
	
	ThrowObjectAtAngle(o, angle, speed);
}

// toss object o along angle angle at speed speed
void ThrowObjectAtAngle(Object *o, uint8_t angle, int speed)
{
	o->yinertia = (sin_table[angle] * speed) >> CSF;
	angle += 64;
	o->xinertia = (sin_table[angle] * speed) >> CSF;
}







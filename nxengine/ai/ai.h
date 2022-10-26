
#ifndef _AI_H
#define _AI_H

void randblink(Object *o, int blinkframe = 1, int blinktime = 8, int prob = 120);

void SmokeClouds(Object *o, int nclouds, int rangex=0, int rangey=0, Object *pushbehind=NULL);
void SmokeXY(int x, int y, int nclouds, int rangex=0, int rangey=0, Object *pushbehind=NULL);


#define ANIMATE(SPEED, FIRSTFRAME, LASTFRAME)		\
{													\
	if (++o->animtimer > SPEED)						\
	{												\
		o->animtimer = 0;							\
		o->frame++;									\
	}												\
	if (o->frame > LASTFRAME) o->frame = FIRSTFRAME;	\
}

#define ANIMATE_FWD(SPEED)		\
{													\
	if (++o->animtimer > SPEED)						\
	{												\
		o->animtimer = 0;							\
		o->frame++;									\
	}	\
}

#define FACEPLAYER	\
{	\
	o->dir = (o->CenterX() > player->CenterX()) ? LEFT:RIGHT;	\
}

#define FACEAWAYPLAYER	\
{	\
	o->dir = (o->CenterX() > player->CenterX()) ? RIGHT:LEFT;	\
}

#define LIMITX(K)		\
{						\
	if (o->xinertia > K) o->xinertia = K;		\
	if (o->xinertia < -K) o->xinertia = -K;		\
}
#define LIMITY(K)		\
{						\
	if (o->yinertia > K) o->yinertia = K;		\
	if (o->yinertia < -K) o->yinertia = -K;		\
}

#define pdistlx(K) ( abs(player->CenterX() - o->CenterX()) <= (K) )
#define pdistly(K) ( abs(player->CenterY() - o->CenterY()) <= (K) )
#define pdistly2(ABOVE,BELOW)	(pdistly(((player->CenterY() > o->CenterY()) ? (BELOW):(ABOVE))))
#define pdistl(K)  ( pdistlx((K)) && pdistly((K)) )

#define XMOVE(SPD)  { o->xinertia = (o->dir == RIGHT) ? (SPD) : -(SPD); }
#define XACCEL(SPD) { o->xinertia += (o->dir == RIGHT) ? (SPD) : -(SPD); }

#define YMOVE(SPD)  { o->yinertia = (o->dir == DOWN) ? (SPD) : -(SPD); }
#define YACCEL(SPD) { o->yinertia += (o->dir == DOWN) ? (SPD) : -(SPD); }

#define COPY_PFBOX	\
	{ sprites[o->sprite].bbox = sprites[o->sprite].frame[o->frame].dir[o->dir].pf_bbox; }

#endif

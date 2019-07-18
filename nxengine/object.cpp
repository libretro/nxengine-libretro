
#include "nx.h"
#include "common/llist.h"
#include "object.fdh"

// deletes the specified object, or well, marks it to be deleted.
// it's not actually freed till the end of the tick.
void Object::Delete()
{
Object * const &o = this;

	if (o->deleted)
		return;
	
	// make sure no pointers are pointing at us
	DisconnectGamePointers();
	
	// show any damage waiting to be added NOW instead of later
	if (o->DamageWaiting > 0)
	{
		DamageText->AddQty(o->DamageWaiting);
		o->DamageWaiting = 0;
	}
	
	// set it's id1 flag, required for some scripts
	game.flags[o->id1] = true;
	
	// mark it for deletion at end of loop
	// (can't delete now as it may invalidate pointers--we don't know where we were called from)
	o->deleted = true;
}

void Object::Destroy()
{
Object * const &o = this;

	// make sure no pointers are pointing at us
	DisconnectGamePointers();
	// delete associated floaty text as soon as it's animation is done
	DamageText->ObjectDestroyed = true;
	
	// if any objects are linked to this obj then unlink them
	Object *link;
	for(link = firstobject; link; link = link->next)
	{
		if (link->linkedobject == o)
			link->linkedobject = NULL;
	}
	
	// remove from list and free
	LL_REMOVE(o, prev, next, firstobject, lastobject);
	LL_REMOVE(o, lower, higher, lowestobject, highestobject);
	if (o == player) player = NULL;
	
	delete o;
}

// checks all the games pointers that point to an object
// record and disconnects them if they are pointing at object o.
// used in preparation to delete the object.
// protects against dangling pointers.
void Object::DisconnectGamePointers()
{
Object * const &o = this;

	if (o == player->riding) player->riding = NULL;
	if (o == player->lastriding) player->lastriding = NULL;
	if (o == player->cannotride) player->cannotride = NULL;
	if (o == game.bossbar.object) game.bossbar.object = NULL;	// any enemy with a boss bar
	if (o == game.stageboss.object) game.stageboss.object = NULL;	// the stage boss
	if (o == map.focus.target) map.focus.target = NULL;
	if (o == ID2Lookup[this->id2]) ID2Lookup[this->id2] = NULL;
	if (o == map.waterlevelobject) map.waterlevelobject = NULL;
}

/*
void c------------------------------() {}
*/

void Object::SetType(int type)
{
Object * const &o = this;

	o->type = type;
	o->sprite = objprop[type].sprite;
	o->hp = objprop[type].initial_hp;
	o->damage = objprop[o->type].damage;
	o->frame = 0;
	
	// apply nxflags to new object type!
	// (did this so toroko would handle slopes properly in Gard cutscene)
	o->nxflags = objprop[type].defaultnxflags;
	
	// apply defaultflags to new object type, but NOT ALL defaultflags.
	// otherwise <CNP's _WILL_ get messed up.
	const static int flags_to_keep = \
		(FLAG_SCRIPTONTOUCH | FLAG_SCRIPTONDEATH | FLAG_SCRIPTONACTIVATE | \
		 FLAG_APPEAR_ON_FLAGID | FLAG_DISAPPEAR_ON_FLAGID | \
		 FLAG_FACES_RIGHT);
	
	uint32_t keep = (o->flags & flags_to_keep);
	o->flags = (objprop[type].defaultflags & ~flags_to_keep) | keep;
	
#ifdef DEBUG
	NX_LOG("new flags: %04x", o->flags);
#endif
	
	// setup default clipping extents, in case object turns on clip_enable
	if (!o->clip_enable)
		o->ResetClip();
}

void Object::ChangeType(int type)
{
Object * const &o = this;

	int oldsprite = o->sprite;
	
	o->state = 0;
	o->substate = 0;
	o->frame = 0;
	o->timer = 0;
	o->timer2 = 0;
	o->animtimer = 0;
	
	SetType(type);
	
	// adjust position so spawn points of old object and new object line up
	o->x >>= CSF; o->x <<= CSF;
	o->y >>= CSF; o->y <<= CSF;
	o->x += (sprites[oldsprite].spawn_point.x << CSF);
	o->y += (sprites[oldsprite].spawn_point.y << CSF);
	o->x -= (sprites[this->sprite].spawn_point.x << CSF);
	o->y -= (sprites[this->sprite].spawn_point.y << CSF);
	
	// added this for when you pick up the puppy in the Deserted House in SZ--
	// makes objects <CNPed during a <PRI initialize immediately instead of waiting
	// for <PRI to be released.
	if (game.frozen)
	{
		OnTick();
		OnAftermove();
	}
	
	// Sprites appearing out of an OBJ_NULL should generally go to the top of the z-order.
	// this was originally added so that the Doctor would appear in front of the core
	// when he teleports in at end of Almond battle (it's since been used in a lot of
	// other places though).
	if (oldsprite == SPR_NULL)
	{
		BringToFront();
	}
	
	OnSpawn();
}

// moves an object to the top of the Z-order,
// so that it is drawn in front of all other objects.
void Object::BringToFront()
{
	LL_REMOVE(this, lower, higher, lowestobject, highestobject);
	LL_ADD_END(this, lower, higher, lowestobject, highestobject);
}

// move an object in the z-order to just below object "behind".
void Object::PushBehind(Object *behind)
{
	if (behind == this)
		return;
	
	LL_REMOVE(this, lower, higher, lowestobject, highestobject);
	LL_INSERT_BEFORE(this, behind, lower, higher, lowestobject, highestobject);
}

void Object::PushBehind(int objtype)
{
	Object *target = Objects::FindByType(objtype);
	if (target)
		PushBehind(target);
	else
		NX_ERR("PushBehind: could not find object %d\n", objtype);
}

/*
void c------------------------------() {}
*/

// for each point in pointlist, treats the point as a CSF'ed offset
// within the object's sprite. Then checks the attributes of the tile
// under each point. Returns an attribute mask containing the cumulative
// attributes of all the tiles under each point in the list.
//
// if tile is non-null, it is set to the tile type of the last tile checked.
uint32_t Object::GetAttributes(const Point *pointlist, int npoints, int *tile)
{
int tileno = 0;
uint32_t attr = 0;

	int xoff = (this->x >> CSF);
	int yoff = (this->y >> CSF);
	
	for(int i=0;i<npoints;i++)
	{
		int x = (xoff + pointlist[i].x) / TILE_W;
		int y = (yoff + pointlist[i].y) / TILE_H;
		
		if (x >= 0 && y >= 0 && x < map.xsize && y < map.ysize)
		{
			tileno = map.tiles[x][y];
			attr |= tileattr[tileno];
		}
	}
	
	// also go underwater if we go under the variable waterlevel in Almond
	if (map.waterlevelobject && (this->y + (2<<CSF)) > map.waterlevelobject->y)
	{
		attr |= TA_WATER;
	}
	
	if (tile) *tile = tileno;
	return attr;
}

// for each point in pointlist, treats the point as a CSF'ed offset
// within the object's sprite. The tile under each position is checked
// to see if it's attributes contain one or more of the attributes
// specified in attrmask.
//
// If any of the points match, returns 1, and optionally returns
// the map coordinates of the first matched tile in tile_x/y.
bool Object::CheckAttribute(const Point *pointlist, int npoints, uint32_t attrmask,
							int *tile_x, int *tile_y)
{
int x, y, xoff, yoff;

	xoff = (this->x >> CSF);
	yoff = (this->y >> CSF);
	
	for(int i=0;i<npoints;i++)
	{
		x = (xoff + pointlist[i].x) / TILE_W;
		y = (yoff + pointlist[i].y) / TILE_H;
		
		if (x >= 0 && y >= 0 && x < map.xsize && y < map.ysize)
		{
			if ((tileattr[map.tiles[x][y]] & attrmask) != 0)
			{
				if (tile_x) *tile_x = x;
				if (tile_y) *tile_y = y;
				return true;
			}
		}
	}
	
	return false;
}

// treats each point in pointlist as an offset within the object, and returns
// true if any of the points intersect with object o2's solidbox.
bool Object::CheckSolidIntersect(Object *other, const Point *pointlist, int npoints)
{
int x, y;
int ox, oy, o2x, o2y;
SIFSprite *s2 = other->Sprite();
	
	ox = (this->x >> CSF);
	oy = (this->y >> CSF);
	o2x = (other->x >> CSF);
	o2y = (other->y >> CSF);
	
	for(int i=0;i<npoints;i++)
	{
		x = ox + pointlist[i].x;
		y = oy + pointlist[i].y;
		
		if (x >= (o2x + s2->solidbox.x1) && x <= (o2x + s2->solidbox.x2))
		{
			if (y >= (o2y + s2->solidbox.y1) && y <= (o2y + s2->solidbox.y2))
			{
				return true;
			}
		}
	}
	
	return false;
}


// update the blocked states of object o.
// updatemask specifies which states are in need of updating.
void Object::UpdateBlockStates(uint8_t updatemask)
{
Object * const &o = this;
SIFSprite *sprite = Sprite();
int mask = GetBlockingType();

	if (updatemask & LEFTMASK)
	{
		o->blockl = CheckAttribute(&sprite->block_l, mask);
		
		// for objects which don't follow slope, have them see the slope as a wall so they
		// won't just go right through it (looks really weird)
		if (!(o->nxflags & NXFLAG_FOLLOW_SLOPE))
		{
			if (!o->blockl)
				o->blockl = IsSlopeAtPointList(o, &sprite->block_l);
		}
	}
	
	if (updatemask & RIGHTMASK)
	{
		o->blockr = CheckAttribute(&sprite->block_r, mask);
		
		// for objects which don't follow slope, have them see the slope as a wall so they
		// won't just go right through it (looks really weird).
		if (!(o->nxflags & NXFLAG_FOLLOW_SLOPE))
		{
			if (!o->blockr)
				o->blockr = IsSlopeAtPointList(o, &sprite->block_r);
		}
	}
	
	if (updatemask & UPMASK)
	{
		o->blocku = CheckAttribute(&sprite->block_u, mask);
		if (!o->blocku) o->blocku = CheckBoppedHeadOnSlope(o) ? 1 : 0;
	}
	
	if (updatemask & DOWNMASK)
	{
		o->blockd = CheckAttribute(&sprite->block_d, mask);
		if (!o->blockd) o->blockd = CheckStandOnSlope(o) ? 1 : 0;
	}
	
	// have player be blocked by objects with FLAG_SOLID_BRICK set
	if (o == player)
		o->SetBlockForSolidBrick(updatemask);
}

// called from UpdateBlockedStates used w/ player.
// sets the object's block/l/r/u/d flags if it is in contact with a SOLID_BRICK object.
void Object::SetBlockForSolidBrick(uint8_t updatemask)
{
SIFSprite *thissprite = this->Sprite();
Object *o;

	// no need to check blockpoints that are already set
	if (this->blockl) updatemask &= ~LEFTMASK;
	if (this->blockr) updatemask &= ~RIGHTMASK;
	if (this->blocku) updatemask &= ~UPMASK;
	if (this->blockd) updatemask &= ~DOWNMASK;
	
	FOREACH_OBJECT(o)
	{
		if (!(o->flags & FLAG_SOLID_BRICK)) continue;
		
		if (updatemask & LEFTMASK)
		{
			if (this->CheckSolidIntersect(o, &thissprite->block_l))
			{
				this->blockl = BLOCKED_OBJECT;	// value of 2 instead of 1
				updatemask &= ~LEFTMASK;		// no need to keep checking
			}
		}
		
		if (updatemask & RIGHTMASK)
		{
			if (this->CheckSolidIntersect(o, &thissprite->block_r))
			{
				this->blockr = BLOCKED_OBJECT;
				updatemask &= ~RIGHTMASK;
			}
		}
		
		if (updatemask & UPMASK)
		{
			if (this->CheckSolidIntersect(o, &thissprite->block_u))
			{
				this->blocku = BLOCKED_OBJECT;
				updatemask &= ~UPMASK;
				
				if (this == player)
					player->bopped_object = o;
			}
		}
		
		if (updatemask & DOWNMASK)
		{
			if (this->CheckSolidIntersect(o, &thissprite->block_d))
			{
				this->blockd = BLOCKED_OBJECT;
				updatemask &= ~DOWNMASK;
				
				if (this == player)
					player->riding = o;
			}
		}
	}
}

/*
void c------------------------------() {}
*/

// given an object, returns which tile attribute affects it's blocked state.
int Object::GetBlockingType()
{
Object * const &o = this;

	if (o == player)
		return TA_SOLID_PLAYER;
	
	if (o->type >= OBJ_SHOTS_START && \
		o->type <= OBJ_SHOTS_END)
	{
		// Bubbler L1 can't pass tile 44.
		if (o->type == OBJ_BUBBLER12_SHOT && o->shot.level == 0)
			return (TA_SOLID_SHOT | TA_SOLID_NPC);
		
		return TA_SOLID_SHOT;
	}
	
	if (o->flags & FLAG_IGNORETILE44)
		return TA_SOLID_PLAYER;
	
	return TA_SOLID_NPC;
}

/*
void c------------------------------() {}
*/

// tries to move the object in the X direction by the given amount.
// returns nonzero if the object was blocked.
bool Object::apply_xinertia(int inertia)
{
Object * const &o = this;

	if (inertia == 0)
		return 0;
	
	if (o->flags & FLAG_IGNORE_SOLID)
	{
		o->x += inertia;
		return 0;
	}
	
	// only apply inertia one pixel at a time so we have
	// proper hit detection--prevents objects traveling at
	// high speed from becoming embedded in walls
	if (inertia > 0)
	{
		while(inertia > (1<<CSF))
		{
			if (movehandleslope(o, (1<<CSF))) return 1;
			inertia -= (1<<CSF);
			
			o->UpdateBlockStates(RIGHTMASK);
		}
	}
	else if (inertia < 0)
	{
		while(inertia < -(1<<CSF))
		{
			if (movehandleslope(o, -(1<<CSF))) return 1;
			inertia += (1<<CSF);
			
			o->UpdateBlockStates(LEFTMASK);
		}
	}
	
	// apply any remaining inertia
	if (inertia)
		movehandleslope(o, inertia);
	
	return 0;
}

// tries to move the object in the Y direction by the given amount.
// returns nonzero if the object was blocked.
bool Object::apply_yinertia(int inertia)
{
Object * const &o = this;

	if (inertia == 0)
		return 0;
	
	if (o->flags & FLAG_IGNORE_SOLID)
	{
		o->y += inertia;
		return 0;
	}
	
	// only apply inertia one pixel at a time so we have
	// proper hit detection--prevents objects traveling at
	// high speed from becoming embedded in walls
	if (inertia > 0)
	{
		if (o->blockd) return 1;
		
		while(inertia > (1<<CSF))
		{
			o->y += (1<<CSF);
			inertia -= (1<<CSF);
			
			o->UpdateBlockStates(DOWNMASK);
			if (o->blockd) return 1;
		}
	}
	else if (inertia < 0)
	{
		if (o->blocku) return 1;
		
		while(inertia < -(1<<CSF))
		{
			o->y -= (1<<CSF);
			inertia += (1<<CSF);
			
			o->UpdateBlockStates(UPMASK);
			if (o->blocku) return 1;
		}
	}
	
	// apply any remaining inertia
	if (inertia)
		o->y += inertia;
	
	return 0;
}


// handles a moving object with "FLAG_SOLID_BRICK" set
// pushing the player as it moves.
void Object::PushPlayerOutOfWay(int xinertia, int yinertia)
{
Object * const &o = this;

	if (xinertia)
	{
		// give a bit of a gap where they must be--i.e. don't push them if they're right
		// at the top or the bottom of the brick: needed when he rides it and falls off, then it
		// turns around and touches him again. in that case what we actually want to do is push him
		// to the top, not push him side-to-side.
		if ((player->SolidBottom() - (2<<CSF)) > o->SolidTop() &&\
			(player->SolidTop() + (2<<CSF)) < o->SolidBottom())
		{
			if (xinertia > 0 && player->SolidRight() > o->SolidRight() && solidhitdetect(o, player))
			{	// pushing player right
				if (player->blockr)
				{	// squish!
					hurtplayer(o->smushdamage);
				}
				else
				{
					// align player's blockl grid with our right side
					player->x = o->SolidRight() - (sprites[player->sprite].block_l[0].x << CSF);
					
					// get player a xinertia equal to our own. You can see this
					// with the moving blocks in Labyrinth H.
					player->xinertia = xinertia;
					player->x += -player->xinertia;
				}
			}
			else if (xinertia < 0 && player->SolidLeft() < o->SolidLeft() && solidhitdetect(o, player))
			{	// pushing player left
				if (player->blockl)
				{	// squish!
					hurtplayer(o->smushdamage);
				}
				else
				{
					// align player's blockr grid with our left side
					player->x = o->SolidLeft() - (sprites[player->sprite].block_r[0].x << CSF);
					
					// get player a xinertia equal to our own. You can see this
					// with the moving blocks in Labyrinth H.
					player->xinertia = xinertia;
					player->x += -player->xinertia;
				}
			}
		}
	}
	
	if (yinertia < 0)
	{
		if (player->blocku && player->riding == o)	// smushed into ceiling!
			hurtplayer(o->smushdamage);
	}
	else if (yinertia > 0)	// object heading downwards?
	{
		// player riding object down
		if (player->riding == o)
		{
			if (player->yinertia >= 0)		// don't do this if he's trying to jump away
			{
				// align player's blockd grid with our top side so player
				// doesn't perpetually fall.
				player->y = o->SolidTop() - (sprites[player->sprite].block_d[0].y << CSF);
			}
		}
		else if (player->Top() >= o->CenterY() && solidhitdetect(o, player))	// underneath object
		{
			// push him down if he's underneath us and we're going faster than he is.
			if (yinertia >= player->yinertia)
			{
				if (player->blockd)		// squished into floor!
					hurtplayer(o->smushdamage);
				
				// align his blocku grid with our bottom side
				player->y = o->SolidBottom() - (sprites[player->sprite].block_u[0].y << CSF);
			}
		}
	}
}

// snap the object down to the nearest solid tile.
// the object must have at least one blockd point for this to work.
void Object::SnapToGround()
{
Object * const &o = this;

	uint32_t flags = o->flags;
	o->flags &= ~FLAG_IGNORE_SOLID;
	
	UpdateBlockStates(DOWNMASK);
	apply_yinertia(SCREEN_HEIGHT << CSF);
	
	o->flags = flags;
	o->blockd = true;
}

/*
void c------------------------------() {}
*/

// deals the specified amount of damage to the object,
// and kills it if it's hitpoints reach 0.
//
// It is valid to deal 0 damage. The trails of the Spur do this
// to keep the enemy shaking and making noise for as long as
// it's in the beam.
//
// shot is an optional parameter specifying a pointer to
// the shot that hit the object, and is used to spawn
// blood spatter at the correct location.
void Object::DealDamage(int dmg, Object *shot)
{
Object * const &o = this;

	if (o->flags & FLAG_INVULNERABLE)
		return;
	
	o->hp -= dmg;
	
	if (o->flags & FLAG_SHOW_FLOATTEXT)
		o->DamageWaiting += dmg;
	
	if (o->hp > 0)
	{
		if (o->shaketime < objprop[o->type].shaketime - 2)
		{
			o->shaketime = objprop[o->type].shaketime;
			
			if (objprop[o->type].hurt_sound)
				sound(objprop[o->type].hurt_sound);
			
			if (shot)
				effect(shot->CenterX(), shot->CenterY(), EFFECT_BLOODSPLATTER);
		}
	}
	else
	{
		o->Kill();
	}
}

// kills the specified object, performing whatever action is
// applicable to that, such as spawning powerups or running scripts.
void Object::Kill()
{
Object * const &o = this;

	o->hp = 0;
	o->flags &= ~FLAG_SHOOTABLE;
	
	// auto disappear the bossbar if we have just killed a boss
	if (o == game.bossbar.object)
		game.bossbar.defeated = true;
	
	// if a script is set to run on death, run it instead of the usual explosion
	if (o->flags & FLAG_SCRIPTONDEATH)
	{
		o->OnDeath();
		StartScript(o->id2);
	}
	else
	{
		// should spawn the smokeclouds first, for z-order reasons
		SmokeClouds(o, objprop[o->type].death_smoke_amt, 8, 8);
		effect(o->CenterX(), o->CenterY(), EFFECT_BOOMFLASH);
		
		if (objprop[o->type].death_sound)
			sound(objprop[o->type].death_sound);
		
		if (objprop[o->type].ai_routines.ondeath)
		{
			o->OnDeath();
		}
		else
		{
			SpawnPowerups();
			o->Delete();
		}
	}
}


// spawn the powerups you get when you kill an enemy
void Object::SpawnPowerups()
{
Object * const &o = this;
int objectType, bonusType;

	if (!objprop[o->type].xponkill)
		return;
	
	bonusType = random(1, 5);
	if (bonusType >= 3)
	{
		SpawnXP(objprop[o->type].xponkill);
		return;
	}
	
	if (bonusType == 2 && \
		(player->weapons[WPN_MISSILE].hasWeapon || \
		 player->weapons[WPN_SUPER_MISSILE].hasWeapon))
	{
		objectType = OBJ_MISSILE;
	}
	else
	{
		objectType = OBJ_HEART;
	}
	
	// upgrade to big 3-cluster versions of powerups
	// for big enemies.
	if (objprop[o->type].xponkill > 6)
	{
		if (objectType == OBJ_HEART)
		{
			objectType = OBJ_HEART3;
		}
		else
		{
			objectType = OBJ_MISSILE3;
		}
	}
	
	// create the powerup
	Object *powerup = CreateObject(o->CenterX(), o->CenterY(), objectType);
	powerup->x -= (powerup->Width() / 2);
	powerup->y -= (powerup->Height() / 2);
	
	powerup->state = 1;			// make it animate
}


// spawn the given quantity of XP at the center of the object.
// amt indicates the total number of XP points to spawn.
// these will be collated into the appropriate sizes of XP triangles.
void Object::SpawnXP(int amt)
{
Object * const &o = this;

	int x = o->CenterX();
	int y = o->CenterY();
	
	while(amt > 0)
	{
		Object *xp = CreateObject(x, y, OBJ_XP);
		xp->xinertia = random(-0x200, 0x200);
		
		if (amt >= XP_LARGE_AMT)
		{
			xp->sprite = SPR_XP_LARGE;
			amt -= XP_LARGE_AMT;
		}
		else if (amt >= XP_MED_AMT)
		{
			xp->sprite = SPR_XP_MED;
			amt -= XP_MED_AMT;
		}
		else
		{
			xp->sprite = SPR_XP_SMALL;
			amt -= XP_SMALL_AMT;
		}
		
		// center the sprite at the center of the object
		xp->x -= (xp->Width() / 2);
		xp->y -= (xp->Height() / 2);
		
		xp->UpdateBlockStates(ALLDIRMASK);
	}
}

/*
void c------------------------------() {}
*/

void Object::RunAI()
{
Object * const &o = this;

	o->OnTick();
	
	// trigger touch-activated scripts.
	// it actually only triggers once his centerline touches the object.
	// see the passageway between the Throne Room and Kings Table for a
	// clear example of the correct coordinates.
	if (o->flags & FLAG_SCRIPTONTOUCH)
	{
		if (pdistlx(8<<CSF))
		{
			int y = player->y + (6 << CSF);
			
			// player->riding check is for fans in Final Cave
			if ((y > o->Top() && y < o->Bottom()) || player->riding == o)
			{
				if (GetCurrentScript() == -1 &&		// no override other scripts
					game.switchstage.mapno == -1)	// no repeat exec after <TRA
				{
					NX_LOG("On-touch script %d triggered\n", o->id2);
					StartScript(o->id2);
				}
			}
		}
	}
}

// deals contact damage to player of o->damage, if applicable.
void Object::DealContactDamage()
{
Object * const &o = this;

	// no contact damage to player while scripts running
	if (GetCurrentScript() != -1 || player->inputs_locked)
		return;
	
	if (!(o->flags & FLAG_NOREARTOPATTACK))
	{
		hurtplayer(o->damage);
		return;
	}
	
	// else, the no rear/top attack flag is set, so only
	// frontal or bottom contact are harmful to the player
	switch(o->GetAttackDirection())
	{
		case -1:	// head-on
			hurtplayer(o->damage);
		break;
		
		case LEFT:	// rear attack, p to left
			if (player->xinertia > -0x100)
				player->xinertia = -0x100;
		break;
		
		case RIGHT:	// rear attack, p to right
			if (player->xinertia < 0x100)
				player->xinertia = 0x100;
		break;
	}
}

// subfunction of HandleContactDamage. On entry, we assume that the player
// is in contact with this object, and that the object is trying to deal
// damage to him.
// returns the type of attack:
//	- UP	a top attack (player hit top of object)
//	- LEFT	rear attack, player to left
//	- RIGHT	rear attack, player to right
//	- -1	head-on or bottom attack
int Object::GetAttackDirection()
{
Object * const &o = this;
const int VARIANCE = (5 << CSF);

	if (player->riding == o)
		return UP;
	
	if (player->Bottom() <= (o->Top() + VARIANCE))
		return UP;
	
	// (added for X treads) if the object is moving, then the "front"
	// for purposes of this flag is the direction it's moving in.
	// if it's still, the "front" is the actual direction it's facing.
	int rtdir = o->dir;
	if (o->xinertia > 0) rtdir = RIGHT;
	if (o->xinertia < 0) rtdir = LEFT;
	
	if (rtdir == RIGHT)
	{
		if (player->Right() <= (o->Left() + VARIANCE))
			return RIGHT;
	}
	else if (rtdir == LEFT)		// the double check makes sense, what if o->dir was UP or DOWN
	{
		if (player->Left() >= (o->Right() - VARIANCE))
			return LEFT;
	}
	
	return -1;
}

void Object::MoveAtDir(int dir, int speed)
{
	this->xinertia = 0;
	this->yinertia = 0;
	
	switch(dir)
	{
		case LEFT:  this->xinertia = -speed; break;
		case RIGHT: this->xinertia = speed; break;
		case UP:	this->yinertia = -speed; break;
		case DOWN:  this->yinertia = speed; break;
	}
}

/*
void c------------------------------() {}
*/

// animate over a list of frames, where the frames need not be consecutive.
// every speed ticks we will display a new frame from framelist.
// this function requires initilization of animframe and animtimer.
void Object::animate_seq(int speed, const int *framelist, int nframes)
{
Object * const &o = this;

	if (++o->animtimer > speed)
	{
		o->animtimer = 0;
		o->animframe++;
	}
	
	if (o->animframe >= nframes)
		o->animframe = 0;
	
	o->frame = framelist[o->animframe];
}

// used by objects in Maze M, this hints to curly's AI that the object is attacking.
void Object::CurlyTargetHere(int mintime, int maxtime)
{
Object * const &o = this;

	game.curlytarget.x = o->CenterX();
	game.curlytarget.y = o->CenterY();
	game.curlytarget.timeleft = random(mintime, maxtime);
}

// reset the objects clip-extent fields (tp effects etc) to their defaults.
// i.e. such that if clip_enable were to be turned on it would have no immediate effect.
void Object::ResetClip()
{
Object * const &o = this;

	o->clipx1 = o->clipy1 = 0;
	o->clipx2 = sprites[o->sprite].w;
	o->clipy2 = sprites[o->sprite].h;
}
/*
void c------------------------------() {}
*/

#if defined(_XBOX) || defined(PSP)
#define AVOID_POINTER_TABLE
#endif

#ifdef AVOID_POINTER_TABLE
#include "objfunc_ptrs.h"
#endif

void Object::OnTick()
{
#ifdef AVOID_POINTER_TABLE
   switch(type)
   {
      case OBJ_BAT_BLUE:
         ai_bat_up_down(this);
         break;
      case OBJ_CRITTER_HOPPING_BLUE:
      case OBJ_CRITTER_FLYING:
      case OBJ_POWER_CRITTER:
      case OBJ_CRITTER_HOPPING_GREEN:
         ai_critter(this);
         break;
      case OBJ_BASIL:
         ai_basil(this);
         break;
      case OBJ_BEHEMOTH:
         ai_behemoth(this);
         break;
      case OBJ_HERMIT_GUNSMITH:
         ai_hermit_gunsmith(this);
         break;
      case  OBJ_DOOR_ENEMY:
         ai_door_enemy(this);
         break;
      case OBJ_BLADE3_SHOT:
         ai_blade_l3_shot(this);
         break;
      case OBJ_BUBBLER12_SHOT:
         ai_bubbler_l12(this);
         break;
      case OBJ_BUBBLER3_SHOT:
         ai_bubbler_l3(this);
         break;
      case OBJ_BUBBLER_SHARP:
         ai_bubbler_sharp(this);
         break;
      case OBJ_FIREBALL1:
         ai_fireball(this);
         break;
      case OBJ_FIREBALL23:
         ai_fireball_level_23(this);
         break;
      case OBJ_FIREBALL_TRAIL:
         ai_fireball_trail(this);
         break;
      case OBJ_NEMESIS_SHOT:
      case OBJ_NEMESIS_SHOT_CURLY:
         ai_nemesis_shot(this);
         break;
      case OBJ_POLAR_SHOT:
      case OBJ_MGUN_L1_SHOT:
      case OBJ_MGUN_LEADER:
         ai_polar_shot(this);
         break;
      case OBJ_MGUN_TRAIL:
         ai_mgun_trail(this);
         break;
      case OBJ_MGUN_SPAWNER:
         ai_mgun_spawner(this);
         break;
      case OBJ_SPUR_SHOT:
         ai_spur_shot(this);
         break;
      case OBJ_SPUR_TRAIL:
         ai_spur_trail(this);
         break;
      case OBJ_SNAKE1_SHOT:
         ai_snake(this);
         break;
      case OBJ_SNAKE23_SHOT:
         ai_snake_23(this);
         break;
      case OBJ_SNAKE_TRAIL:
         ai_snake_trail(this);
         break;
      case OBJ_WHIMSICAL_STAR:
         ai_whimsical_star(this);
         break;
      case OBJ_SMOKE_CLOUD:
         ai_smokecloud(this);
         break;
      case OBJ_SKY_DRAGON:
         ai_sky_dragon(this);
         break;
      case OBJ_SANDCROC_OSIDE:
         ai_sandcroc(this);
         break;
      case OBJ_NIGHT_SPIRIT:
         ai_night_spirit(this);
         break;
      case OBJ_NIGHT_SPIRIT_SHOT:
         ai_night_spirit_shot(this);
         break;
      case OBJ_HOPPY:
         ai_hoppy(this);
         break;
      case OBJ_PIXEL_CAT:
         ai_pixel_cat(this);
         break;
      case OBJ_LITTLE_FAMILY:
         ai_little_family(this);
         break;
      case OBJ_NPC_PLAYER:
         ai_npc_player(this);
         break;
      case OBJ_PTELIN:
         ai_ptelin(this);
         break;
      case OBJ_PTELOUT:
         ai_ptelout(this);
         break;
      case OBJ_NULL:
         ai_null(this);
         break;
      case OBJ_HVTRIGGER:
         ai_hvtrigger(this);
         break;
      case OBJ_XP:
         ai_xp(this);
         break;
      case OBJ_HEART:
      case OBJ_HEART3:
      case OBJ_MISSILE:
      case OBJ_MISSILE3:
         ai_powerup(this);
         break;
      case OBJ_HIDDEN_POWERUP:
         ai_hidden_powerup(this);
         break;
      case OBJ_DOOR:
         ai_door(this);
         break;
      case OBJ_LARGEDOOR:
         ai_largedoor(this);
         break;
      case OBJ_SAVE_POINT:
         ai_save_point(this);
         break;
      case OBJ_RECHARGE:
         ai_largedoor(this);
         break; 
      case OBJ_CHEST_CLOSED:
         ai_chest_closed(this);
         break;
      case OBJ_CHEST_OPEN:
         ai_chest_open(this);
         break;
      case OBJ_TELEPORTER:
         ai_teleporter(this);
         break;
      case OBJ_TELEPORTER_LIGHTS:
      case OBJ_SANTAS_KEY:
      case OBJ_BUILDING_FAN:
         ai_animate2(this);
         break;
      case OBJ_COMPUTER:
      case OBJ_LIFE_CAPSULE:
      case OBJ_HIDDEN_SPARKLE:
         ai_animate4(this);
         break;
      case OBJ_TERMINAL:
         ai_terminal(this);
         break;
      case OBJ_XP_CAPSULE:
         ai_xp_capsule(this);
         break;
      case OBJ_SPRINKLER:
         ai_sprinkler(this);
         break;
      case OBJ_WATER_DROPLET:
      case OBJ_LAVA_DROPLET:
         ai_water_droplet(this);
         break;
      case OBJ_DROPLET_SPAWNER:
         ai_droplet_spawner(this);
         break;
      case OBJ_FAN_UP:
      case OBJ_FAN_DOWN:
         ai_fan_vert(this);
         break;
      case OBJ_FAN_LEFT:
      case OBJ_FAN_RIGHT:
         ai_fan_hoz(this);
         break;
      case OBJ_FAN_DROPLET:
         ai_fan_droplet(this);
         break;
      case OBJ_PRESS:
         ai_press(this);
         break;
      case OBJ_LIGHTNING:
         ai_lightning(this);
         break;
      case OBJ_STRAINING:
         ai_straining(this);
         break;
      case OBJ_BUBBLE_SPAWNER:
         ai_bubble_spawner(this);
         break;
      case OBJ_CHINFISH:
         ai_chinfish(this);
         break;
      case OBJ_FIREPLACE:
         ai_fireplace(this);
         break;
      case OBJ_SMOKE_DROPPER:
         ai_smoke_dropper(this);
         break;
      case OBJ_SCROLL_CONTROLLER:
         ai_scroll_controller(this);
         break;
      case OBJ_QUAKE:
         ai_quake(this);
         break;
      case OBJ_BALROG:
         ai_balrog(this);
         break;
      case OBJ_BALROG_DROP_IN:
         ai_balrog_drop_in(this);
         break;
      case OBJ_BALROG_BUST_IN:
         ai_balrog_bust_in(this);
         break;
      case OBJ_MISERY_FLOAT:
         ai_misery_float(this);
         break;
      case OBJ_MISERY_STAND:
         ai_misery_stand(this);
         break;
      case OBJ_MISERYS_BUBBLE:
         ai_miserys_bubble(this);
         break;
      case OBJ_CURLY_BOSS:
         ai_curly_boss(this);
         break;
      case OBJ_CURLYBOSS_SHOT:
         ai_curlyboss_shot(this);
         break;
      case OBJ_PUPPY_WAG:
         ai_puppy_wag(this);
         break;
      case OBJ_PUPPY_BARK:
         ai_puppy_bark(this);
         break;
      case OBJ_PUPPY_SLEEP:
         ai_zzzz_spawner(this);
         break;
      case OBJ_PUPPY_RUN:
         ai_puppy_run(this);
         break;
      case OBJ_TOROKO_FRENZIED:
         ai_toroko_frenzied(this);
         break;
      case OBJ_TOROKO_BLOCK:
         ai_toroko_block(this);
         break;
      case OBJ_TOROKO_FLOWER:
         ai_toroko_flower(this);
         break;
      case OBJ_BEETLE_BROWN:
      case OBJ_BEETLE_GREEN:
         ai_beetle_horiz(this);
         break;
      case OBJ_POLISH:
         ai_polish(this);
         break;
      case OBJ_POLISHBABY:
         ai_polishbaby(this);
         break;
      case OBJ_SANDCROC:
         ai_sandcroc(this);
         break;
      case OBJ_MIMIGAC1:
      case OBJ_MIMIGAC2:
      case OBJ_MIMIGAC_ENEMY:
         ai_curlys_mimigas(this);
         break;
      case OBJ_SUNSTONE:
         ai_sunstone(this);
         break;
      case OBJ_ARMADILLO:
         ai_armadillo(this);
         break;
      case OBJ_CROW:
         ai_crow(this);
         break;
      case OBJ_CROWWITHSKULL:
         ai_crowwithskull(this);
         break;
      case OBJ_SKULLHEAD:
         ai_skullhead(this);
         break;
      case OBJ_SKULLHEAD_CARRIED:
         ai_skullhead_carried(this);
         break;
      case OBJ_SKULLSTEP:
         ai_skullstep(this);
         break;
      case OBJ_SKULLSTEP_FOOT:
         ai_skullstep_foot(this);
         break;
      case OBJ_SKELETON:
         ai_skeleton(this);
         break;
      case OBJ_SKELETON_SHOT:
         ai_skeleton_shot(this);
         break;
      case OBJ_BALROG_BOSS_RUNNING:
         ai_balrog_boss_running(this);
         break;
      case OBJ_MA_PIGNON:
         ai_ma_pignon(this);
         break;
      case OBJ_MA_PIGNON_ROCK:
         ai_ma_pignon_rock(this);
         break;
      case OBJ_MA_PIGNON_CLONE:
         ai_ma_pignon_clone(this);
         break;
      case OBJ_TOROKO_SHACK:
         ai_toroko_shack(this);
         break;
      case OBJ_MUSHROOM_ENEMY:
      case OBJ_GIANT_MUSHROOM_ENEMY:
         ai_mushroom_enemy(this);
         break;
      case OBJ_GRAVEKEEPER:
         ai_gravekeeper(this);
         break;
      case OBJ_CAGE:
         ai_cage(this);
         break;
      case OBJ_MAHIN:
         ai_npc_mahin(this);
         break;
      case OBJ_YAMASHITA_PAVILION:
         ai_yamashita_pavilion(this);
         break;
      case OBJ_CHTHULU:
         ai_chthulu(this);
         break;
      case OBJ_DR_GERO:
      case OBJ_NURSE_HASUMI:
      case OBJ_KAZUMA:
      case OBJ_MEGANE:
      case OBJ_CHIE:
      case OBJ_GAUDI_SHOPKEEP:
         ai_generic_npc_nofaceplayer(this);
         break;
      case OBJ_CURLY:
         ai_curly(this);
         break;
      case OBJ_CURLY_CARRIED_SHOOTING:
         ai_curly_carried_shooting(this);
         break;
      case OBJ_CCS_GUN:
         ai_ccs_gun(this);
         break;
      case OBJ_CURLY_AI:
         ai_curly_ai(this);
         break;
      case OBJ_CAI_GUN:
         ai_cai_gun(this);
         break;
      case OBJ_KAZUMA_AT_COMPUTER:
      case OBJ_SUE_AT_COMPUTER:
         ai_npc_at_computer(this);
         break;
      case OBJ_JENKA:
         ai_jenka(this);
         break;
      case OBJ_BLUE_ROBOT:
         ai_blue_robot(this);
         break;
      case OBJ_DOCTOR:
         ai_doctor(this);
         break;
      case OBJ_TOROKO:
         ai_toroko(this);
         break;
      case OBJ_TOROKO_TELEPORT_IN:
         ai_toroko_teleport_in(this);
         break;
      case OBJ_SUE:
         ai_npc_sue(this);
         break;
      case OBJ_SUE_TELEPORT_IN:
         ai_sue_teleport_in(this);
         break;
      case OBJ_KING:
         ai_king(this);
         break;
      case OBJ_KANPACHI_FISHING:
         ai_kanpachi_fishing(this);
         break;
      case OBJ_PROFESSOR_BOOSTER:
         ai_professor_booster(this);
         break;
      case OBJ_BOOSTER_FALLING:
         ai_booster_falling(this);
         break;
      case OBJ_SANTA:
      case OBJ_CHACO:
      case OBJ_JACK:
         ai_generic_npc(this);
         break;
      case OBJ_WATERLEVEL:
         ai_waterlevel(this);
         break;
      case OBJ_SHUTTER:
      case OBJ_SHUTTER_BIG:
      case OBJ_ALMOND_LIFT:
         ai_shutter(this);
         break;
      case OBJ_SHUTTER_STUCK:
         ai_shutter_stuck(this);
         break;
      case OBJ_ALMOND_ROBOT:
         ai_almond_robot(this);
         break;
      case OBJ_BALROG_BOSS_FLYING:
         ai_balrog_boss_flying(this);
         break;
      case OBJ_BALROG_SHOT_BOUNCE:
         ai_balrog_shot_bounce(this);
         break;
      case OBJ_FRENZIED_MIMIGA:
         ai_frenzied_mimiga(this);
         break;
      case OBJ_BAT_HANG:
         ai_bat_hang(this);
         break;
      case OBJ_BAT_CIRCLE:
         ai_bat_circle(this);
         break;
      case OBJ_JELLY:
         ai_jelly(this);
         break;
      case OBJ_GIANT_JELLY:
         ai_giant_jelly(this);
         break;
      case OBJ_MANNAN:
         ai_mannan(this);
         break;
      case OBJ_MANNAN_SHOT:
         ai_mannan_shot(this);
         break;
      case OBJ_FROG:
      case OBJ_MINIFROG:
         ai_frog(this);
         break;
      case OBJ_HEY_SPAWNER:
         ai_hey_spawner(this);
         break;
      case OBJ_MOTORBIKE:
         ai_motorbike(this);
         break;
      case OBJ_POWERCOMP:
         ai_animate3(this);
         break;
      case OBJ_POWERSINE:
         ai_animate1(this);
         break;
      case OBJ_MALCO:
         ai_malco(this);
         break;
      case OBJ_MALCO_BROKEN:
         ai_malco_broken(this);
         break;
      case OBJ_ORANGEBELL:
         ai_orangebell(this);
         break;
      case OBJ_ORANGEBELL_BABY:
         ai_orangebell_baby(this);
         break;
      case OBJ_STUMPY:
         ai_stumpy(this);
         break;
      case OBJ_MIDORIN:
         ai_midorin(this);
         break;
      case OBJ_GUNFISH:
         ai_gunfish(this);
         break;
      case OBJ_GUNFISH_SHOT:
         ai_gunfish_shot(this);
         break;
      case OBJ_DROLL:
         ai_droll(this);
         break;
      case OBJ_DROLL_SHOT:
         ai_droll_shot(this);
         break;
      case OBJ_DROLL_GUARD:
         ai_droll_guard(this);
         break;
      case OBJ_MIMIGA_FARMER_STANDING:
      case OBJ_MIMIGA_FARMER_WALKING:
      case OBJ_MIMIGA_JAILED:
         ai_mimiga_farmer(this);
         break;
      case OBJ_ROCKET:
         ai_rocket(this);
         break;
      case OBJ_PROXIMITY_PRESS_HOZ:
         ai_proximity_press_hoz(this);
         break;
      case OBJ_PUPPY_ITEMS:
         ai_puppy_wag(this);
         break;
      case OBJ_NUMAHACHI:
         ai_numahachi(this);
         break;
      case OBJ_ITOH:
         ai_npc_itoh(this);
         break;
      case OBJ_KANPACHI_STANDING:
         ai_kanpachi_standing(this);
         break;
      case OBJ_MOMORIN:
         ai_npc_momorin(this);
         break;
      case OBJ_CRITTER_HOPPING_RED:
         ai_critter_hopping_red(this);
         break;
      case OBJ_LAVA_DRIP_SPAWNER:
         ai_lava_drip_spawner(this);
         break;
      case OBJ_LAVA_DRIP:
         ai_lava_drip(this);
         break;
      case OBJ_RED_BAT_SPAWNER:
         ai_red_bat_spawner(this);
         break;
      case OBJ_RED_BAT:
         ai_red_bat(this);
         break;
      case OBJ_RED_DEMON:
         ai_red_demon(this);
         break;
      case OBJ_RED_DEMON_SHOT:
         ai_droll_shot(this);
         break;
      case OBJ_PROXIMITY_PRESS_VERT:
         ai_proximity_press_vert(this);
         break;
      case OBJ_NPC_IGOR:
         ai_npc_igor(this);
         break;
      case OBJ_BOSS_IGOR:
         ai_boss_igor(this);
         break;
      case OBJ_IGOR_SHOT:
      case OBJ_DRAGON_ZOMBIE_SHOT:
      case OBJ_GIANT_BEETLE_SHOT:
      case OBJ_BALFROG_SHOT:
      case OBJ_MISERY_SHOT:
      case OBJ_CRITTER_SHOT:
      case OBJ_GAUDI_FLYING_SHOT:
         ai_generic_angled_shot(this);
         break;
      case OBJ_FORCEFIELD:
         ai_forcefield(this);
         break;
      case OBJ_EGG_ELEVATOR:
         ai_egg_elevator(this);
         break;
      case OBJ_BOSS_IGOR_DEFEATED:
         ai_boss_igor_defeated(this);
         break;
      case OBJ_CRITTER_HOPPING_AQUA:
         ai_critter(this);
         break;
      case OBJ_BEETLE_FREEFLY:
      case OBJ_BEETLE_FREEFLY_2:
         ai_beetle_freefly(this);
         break;
      case OBJ_GIANT_BEETLE:
      case OBJ_GIANT_BEETLE_2:
         ai_giant_beetle(this);
         break;
      case OBJ_DRAGON_ZOMBIE:
         ai_dragon_zombie(this);
         break;
      case OBJ_FALLING_SPIKE_SMALL:
         ai_falling_spike_small(this);
         break;
      case OBJ_FALLING_SPIKE_LARGE:
         ai_falling_spike_large(this);
         break;
      case OBJ_COUNTER_BOMB:
         ai_counter_bomb(this);
         break;
      case OBJ_COUNTER_BOMB_NUMBER:
         ai_counter_bomb_number(this);
         break;
      case OBJ_BALLOS_ROTATOR:
         ai_ballos_rotator(this);
         break;
      case OBJ_BALLOS_PLATFORM:
         ai_ballos_platform(this);
         break;
      case OBJ_CORE_GHOSTIE:
         ai_core_ghostie(this);
         break;
      case OBJ_CORE_BLAST:
         ai_core_blast(this);
         break;
      case OBJ_MINICORE:
         ai_minicore(this);
         break;
      case OBJ_MINICORE_SHOT:
         ai_minicore_shot(this);
         break;
      case OBJ_HP_LIGHTNING:
         ai_hp_lightning(this);
         break;
      case OBJ_X_FISHY_MISSILE:
         ai_x_fishy_missile(this);
         break;
      case OBJ_X_DEFEATED:
         ai_x_defeated(this);
         break;
      case OBJ_UDMINI_PLATFORM:
         ai_udmini_platform(this);
         break;
      case OBJ_UD_PELLET:
         ai_ud_pellet(this);
         break;
      case OBJ_UD_SMOKE:
         ai_ud_smoke(this);
         break;
      case OBJ_UD_SPINNER:
         ai_ud_spinner(this);
         break;
      case OBJ_UD_SPINNER_TRAIL:
         ai_ud_spinner_trail(this);
         break;
      case OBJ_UD_BLAST:
         ai_ud_blast(this);
         break;
      case OBJ_OMEGA_SHOT:
         ai_omega_shot(this);
         break;
      case OBJ_IRONH_FISHY:
         ai_ironh_fishy(this);
         break;
      case OBJ_IRONH_SHOT:
         ai_ironh_shot(this);
         break;
      case OBJ_BRICK_SPAWNER:
         ai_brick_spawner(this);
         break;
      case OBJ_IRONH_BRICK:
         ai_ironh_brick(this);
         break;
      case OBJ_IKACHAN_SPAWNER:
         ai_ikachan_spawner(this);
         break;
      case OBJ_IKACHAN:
         ai_ikachan(this);
         break;
      case OBJ_MOTION_WALL:
         ai_motion_wall(this);
         break;
      case OBJ_HELICOPTER:
         ai_helicopter(this);
         break;
      case OBJ_HELICOPTER_BLADE:
         ai_helicopter_blade(this);
         break;
      case OBJ_IGOR_BALCONY:
         ai_igor_balcony(this);
         break;
      case OBJ_FALLING_BLOCK:
         ai_falling_block(this);
         break;
      case OBJ_FALLING_BLOCK_SPAWNER:
         ai_falling_block_spawner(this);
         break;
      case OBJ_BOSS_DOCTOR:
         ai_boss_doctor(this);
         break;
      case OBJ_DOCTOR_SHOT:
         ai_doctor_shot(this);
         break;
      case OBJ_DOCTOR_SHOT_TRAIL:
         ai_doctor_shot_trail(this);
         break;
      case OBJ_DOCTOR_BLAST:
         ai_doctor_blast(this);
         break;
      case OBJ_DOCTOR_CROWNED:
         ai_doctor_crowned(this);
         break;
      case OBJ_BOSS_DOCTOR_FRENZIED:
         ai_boss_doctor_frenzied(this);
         break;
      case OBJ_DOCTOR_BAT:
         ai_doctor_bat(this);
         break;
      case OBJ_MIMIGA_CAGED:
      case OBJ_CHIE_CAGED:
      case OBJ_CHACO_CAGED:
      case OBJ_SANTA_CAGED:
         ai_mimiga_caged(this);
         break;
      case OBJ_DOCTOR_GHOST:
         ai_doctor_ghost(this);
         break;
      case OBJ_RED_ENERGY:
         ai_red_energy(this);
         break;
      case OBJ_BOSS_MISERY:
         ai_boss_misery(this);
         break;
      case OBJ_MISERY_PHASE:
         ai_misery_phase(this);
         break;
      case OBJ_MISERY_RING:
         ai_misery_ring(this);
         break;
      case OBJ_MISERY_BALL:
         ai_misery_ball(this);
         break;
      case OBJ_BLACK_LIGHTNING:
         ai_black_lightning(this);
         break;
      case OBJ_BUTE_SWORD_RED:
         ai_bute_sword_red(this);
         break;
      case OBJ_BUTE_ARCHER_RED:
         ai_bute_archer_red(this);
         break;
      case OBJ_WALL_COLLAPSER:
         ai_wall_collapser(this);
         break;
      case OBJ_GREEN_DEVIL_SPAWNER:
         ai_green_devil_spawner(this);
         break;
      case OBJ_GREEN_DEVIL:
         ai_green_devil(this);
         break;
      case OBJ_BALLOS_SPIKES:
         ai_ballos_spikes(this);
         break;
      case OBJ_BALLOS_SKULL:
         ai_ballos_skull(this);
         break;
      case OBJ_BALLOS_PRIEST:
         ai_ballos_priest(this);
         break;
      case OBJ_BALLOS_TARGET:
         ai_ballos_target(this);
         break;
      case OBJ_BALLOS_BONE_SPAWNER:
         ai_ballos_bone_spawner(this);
         break;
      case OBJ_BALLOS_BONE:
         ai_ballos_bone(this);
         break;
      case OBJ_BUTE_FLYING:
         ai_bute_flying(this);
         break;
      case OBJ_BUTE_DYING:
         ai_bute_dying(this);
         break;
      case OBJ_BUTE_SPAWNER:
         ai_bute_spawner(this);
         break;
      case OBJ_BUTE_FALLING:
         ai_bute_falling(this);
         break;
      case OBJ_BUTE_SWORD:
         ai_bute_sword(this);
         break;
      case OBJ_BUTE_ARCHER:
         ai_bute_archer(this);
         break;
      case OBJ_BUTE_ARROW:
         ai_bute_arrow(this);
         break;
      case OBJ_MESA:
         ai_mesa(this);
         break;
      case OBJ_MESA_BLOCK:
         ai_mesa_block(this);
         break;
      case OBJ_MESA_DYING:
         ai_bute_dying(this);
         break;
      case OBJ_DELEET:
         ai_deleet(this);
         break;
      case OBJ_ROLLING:
         ai_rolling(this);
         break;
      case OBJ_STATUE:
         ai_statue(this);
         break;
      case OBJ_STATUE_BASE:
         ai_statue_base(this);
         break;
      case OBJ_PUPPY_GHOST:
         ai_puppy_ghost(this);
         break;
      case OBJ_BALROG_MISSILE:
         ai_balrog_missile(this);
         break;
      case OBJ_BALROG_BOSS_MISSILES:
         ai_balrog_boss_missiles(this);
         break;
      case OBJ_CRITTER_SHOOTING_PURPLE:
         ai_critter_shooting_purple(this);
         break;
      case OBJ_GAUDI:
         ai_gaudi(this);
         break;
      case OBJ_GAUDI_ARMORED:
         ai_gaudi_armored(this);
         break;
      case OBJ_GAUDI_ARMORED_SHOT:
         ai_gaudi_armored_shot(this);
         break;
      case OBJ_GAUDI_DYING:
         ai_gaudi_dying(this);
         break;
      case OBJ_GAUDI_FLYING:
         ai_gaudi_flying(this);
         break;
      case OBJ_INTRO_KINGS:
         ai_intro_kings(this);
         break;
      case OBJ_INTRO_CROWN:
         ai_intro_crown(this);
         break;
      case OBJ_INTRO_DOCTOR:
         ai_intro_doctor(this);
         break;
      case OBJ_CLOUD_SPAWNER:
         ai_cloud_spawner(this);
         break;
      case OBJ_CLOUD:
         ai_cloud(this);
         break;
      case OBJ_BALROG_FLYING:
         ai_balrog_flying(this);
         break;
      case OBJ_BALROG_MEDIC:
         ai_balrog_medic(this);
         break;
      case OBJ_GAUDI_PATIENT:
         ai_gaudi_patient(this);
         break;
      case OBJ_BABY_PUPPY:
         ai_baby_puppy(this);
         break;
      case OBJ_TURNING_HUMAN:
         ai_turning_human(this);
         break;
      case OBJ_AHCHOO:
         ai_ahchoo(this);
         break;
      case OBJ_MISERY_WIND:
         ai_misery_wind(this);
         break;
      case OBJ_THE_CAST:
         ai_the_cast(this);
         break;
      case OBJ_FIREWHIRR:
         ai_firewhirr(this);
         break;
      case OBJ_FIREWHIRR_SHOT:
         ai_firewhirr_shot(this);
         break;
      case OBJ_GAUDI_EGG:
         ai_gaudi_egg(this);
         break;
      case OBJ_FUZZ_CORE:
         ai_fuzz_core(this);
         break;
      case OBJ_FUZZ:
         ai_fuzz(this);
         break;
      case OBJ_BUYOBUYO_BASE:
         ai_buyobuyo_base(this);
         break;
      case OBJ_BUYOBUYO:
         ai_buyobuyo(this);
         break;
      case OBJ_POOH_BLACK:
         ai_pooh_black(this);
         break;
      case OBJ_POOH_BLACK_BUBBLE:
         ai_pooh_black_bubble(this);
         break;
      case OBJ_POOH_BLACK_DYING:
         ai_pooh_black_dying(this);
         break;
      case OBJ_BLOCK_MOVEH:
         ai_block_moveh(this);
         break;
      case OBJ_BLOCK_MOVEV:
         ai_block_movev(this);
         break;
      case OBJ_BOULDER:
         ai_boulder(this);
         break;
   }
#else
	if (objprop[this->type].ai_routines.ontick)
		(*objprop[this->type].ai_routines.ontick)(this);
#endif
}

void Object::OnAftermove()
{
#ifdef AVOID_POINTER_TABLE
   switch (type)
   {
      case OBJ_BLADE12_SHOT:
         aftermove_blade_l12_shot(this);
         break;
      case OBJ_BLADE_SLASH:
         aftermove_blade_slash(this);
         break;
      case OBJ_MISSILE_SHOT:
      case OBJ_SUPERMISSILE_SHOT:
         ai_missile_shot(this);
         break;
      case OBJ_MISSILE_BOOM_SPAWNER:
         ai_missile_boom_spawner(this);
         break;
      case OBJ_SPIKE_SMALL:
         onspawn_spike_small(this);
         break;
      case OBJ_PUPPY_CARRY:
         aftermove_puppy_carry(this);
         break;
      case OBJ_TOROKO_BLOCK:
         aftermove_toroko_block(this);
         break;
      case OBJ_SKULLHEAD_CARRIED:
         aftermove_skullhead_carried(this);
         break;
      case OBJ_CURLY_CARRIED:
         aftermove_curly_carried(this);
         break;
      case OBJ_CAI_GUN:
         aftermove_cai_gun(this);
         break;
      case OBJ_CAI_WATERSHIELD:
         aftermove_cai_watershield(this);
         break;
      case OBJ_SUE:
         aftermove_npc_sue(this);
         break;
      case OBJ_KINGS_SWORD:
         aftermove_StickToLinkedActionPoint(this);
         break;
      case OBJ_BALLOS_ROTATOR:
         aftermove_ballos_rotator(this);
         break;
      case OBJ_CORE_BACK:
         ai_core_back(this);
         break;
      case OBJ_CORE_FRONT:
         ai_core_front(this);
         break;
      case OBJ_RED_CRYSTAL:
         aftermove_red_crystal(this);
         break;
      case OBJ_MISERY_RING:
         aftermove_misery_ring(this);
         break;
      case OBJ_BALROG_PASSENGER:
         aftermove_balrog_passenger(this);
         break;
   }
#else
	if (objprop[this->type].ai_routines.aftermove)
		(*objprop[this->type].ai_routines.aftermove)(this);
#endif
}

void Object::OnSpawn()
{
#ifdef AVOID_POINTER_TABLE
   switch (type)
   {
      case OBJ_BALFROG:
         onspawn_balrog(this);
         break;
      case OBJ_FLOWERS_PENS1:
         onspawn_snap_to_ground(this);
         break;
      case OBJ_YAMASHITA_FLOWERS:
         onspawn_set_frame_from_id2(this);
         break;
      case OBJ_DR_GERO:
      case OBJ_NURSE_HASUMI:
         onspawn_generic_npc(this);
         break;
      case OBJ_BALROG:
         onspawn_balrog(this);
         break;
      case OBJ_SUE:
         onspawn_npc_sue(this);
         break;
      case OBJ_SANTA:
      case OBJ_CHACO:
      case OBJ_JACK:
      case OBJ_KAZUMA:
      case OBJ_MEGANE:
      case OBJ_CHIE:
      case OBJ_GAUDI_SHOPKEEP:
         onspawn_generic_npc(this);
         break;
      case OBJ_MIMIGA_CAGE:
         onspawn_mimiga_cage(this);
         break;
      case OBJ_UD_MINICORE_IDLE:
	 onspawn_ud_minicore_idle(this);
	 break;
   }
#else
	if (objprop[this->type].ai_routines.onspawn)
		(*objprop[this->type].ai_routines.onspawn)(this);
#endif
}

void Object::OnDeath()
{
#ifdef AVOID_POINTER_TABLE
   switch (type)
   {
      case OBJ_POLISH:
         ondeath_polish(this);
         break;
      case OBJ_BALROG_BOSS_RUNNING:
         ondeath_balrog_boss_running(this);
         break;
      case OBJ_BALROG_BOSS_FLYING:
         ondeath_balrog_boss_flying(this);
         break;
      case OBJ_BALFROG:
	 ondeath_balfrog(this);
	 break;
      case OBJ_BALLOS_MAIN:
	 ondeath_ballos(this);
	 break;
      case OBJ_X_TARGET:
	 ondeath_x_target(this);
	 break;
      case OBJ_X_MAINOBJECT:
	 ondeath_x_mainobject(this);
	 break;
      case OBJ_OMEGA_BODY:
	 ondeath_omega_body(this);
	 break;
      case OBJ_IRONH:
	 ondeath_ironhead(this);
	 break;
      case OBJ_BALROG_BOSS_MISSILES:
	 ondeath_balrog_boss_missiles(this);
	 break;
   }
#else
	if (objprop[this->type].ai_routines.ondeath)
		(*objprop[this->type].ai_routines.ondeath)(this);
#endif
}



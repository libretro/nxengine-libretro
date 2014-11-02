
#include "nx.h"
#include "stageboss.fdh"

#include "ai/boss/omega.h"
#include "ai/boss/balfrog.h"
#include "ai/boss/x.h"
#include "ai/boss/core.h"
#include "ai/boss/ironhead.h"
#include "ai/boss/sisters.h"
#include "ai/boss/undead_core.h"
#include "ai/boss/heavypress.h"
#include "ai/boss/ballos.h"


StageBossManager::StageBossManager()
{
	fBoss = NULL;
	fBossType = BOSS_NONE;
}

/*
void c------------------------------() {}
*/

bool StageBossManager::SetType(int newtype)
{
	NX_LOG("StageBossManager::SetType(%d)\n", newtype);
	
	if (fBoss)
	{
		delete fBoss;
		if (game.stageboss.object)
		{
			NX_ERR(" ** warning: game.stageboss.object not properly cleaned up in OnMapExit\n");
			game.stageboss.object->Delete();
			game.stageboss.object = NULL;
		}
	}
	
	fBossType = newtype;
	fBoss = NULL;
	
	switch(newtype)
	{
		case BOSS_NONE: break;
		
		case BOSS_OMEGA: fBoss = new OmegaBoss; break;
		case BOSS_BALFROG: fBoss = new BalfrogBoss; break;
		case BOSS_MONSTER_X: fBoss = new XBoss; break;
		case BOSS_CORE: fBoss = new CoreBoss; break;
		case BOSS_IRONH: fBoss = new IronheadBoss; break;
		case BOSS_SISTERS: fBoss = new SistersBoss; break;
		case BOSS_UNDEAD_CORE: fBoss = new UDCoreBoss; break;
		case BOSS_HEAVY_PRESS: fBoss = new HeavyPress; break;
		case BOSS_BALLOS: fBoss = new BallosBoss; break;
		
		default:
			NX_ERR("StageBossManager::SetType: unhandled boss type %d\n", newtype);
			fBossType = BOSS_NONE;
			return 1;
	}
	
	return 0;
}

int StageBossManager::Type()
{
	return fBossType;
}

/*
void c------------------------------() {}
*/

void StageBossManager::OnMapEntry()
{
	if (fBoss) fBoss->OnMapEntry();
}

void StageBossManager::OnMapExit()
{
	if (fBoss) fBoss->OnMapExit();
}

void StageBossManager::Run()
{
	if (fBoss) fBoss->Run();
}

void StageBossManager::RunAftermove()
{
	if (fBoss) fBoss->RunAftermove();
}

/*
void c------------------------------() {}
*/

void StageBossManager::SetState(int newstate)
{
	if (fBoss)
	{
		fBoss->SetState(newstate);
	}
	else
	{
		NX_ERR("StageBossManager::SetState(%d): no stageboss object in existance!\n", newstate);
	}
}

/*
void c------------------------------() {}
*/

// these are default implementation, bosses can override them if they want to.

void StageBoss::SetState(int newstate)
{
	if (game.stageboss.object)
	{
		game.stageboss.object->state = newstate;
	}
	else
	{
		NX_ERR("StageBoss::SetState(%d): no stageboss object!\n", newstate);
	}
}


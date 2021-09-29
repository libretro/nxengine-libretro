
#ifndef _PROFILE_H
#define _PROFILE_H

// how many bytes of data long a profile.dat is.
// used by the replays which use the regular profile functions
// to write a savefile then tack their own data onto the end.
#define PROFILE_LENGTH		0x604

#ifdef __cplusplus
extern "C" {
#endif

struct Profile
{
	int stage;
	int songno;
	int px, py, pdir;
	int hp, maxhp, num_whimstars;
	uint32_t equipmask;
	
	int curWeapon;
        // 14 is WPN_COUNT
	struct
	{
		bool hasWeapon;
		int level;
		int xp;
		int ammo, maxammo;
	} weapons[14];
	
        // 42 is MAX_INVENTORY
	int inventory[42];
	int ninventory;
	
        // 8000 is NUM_GAMEFLAGS
	bool flags[8000];
	
        // 8 is NUM_TELEPORTER_SLOTS
	struct
	{
		int slotno;
		int scriptno;
	} teleslots[8];
	int num_teleslots;
};

#ifdef __cplusplus
}
#endif

#endif

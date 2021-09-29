#include <stdint.h>
#include <string.h>

#include <streams/file_stream.h>
#include <file/file_path.h>

#include "profile.h"
#include "profile.fdh"
#include "libretro/libretro_shared.h"

#define PF_WEAPONS_OFFS		0x38
#define PF_CURWEAPON_OFFS	0x24
#define PF_INVENTORY_OFFS	0xD8
#define PF_TELEPORTER_OFFS	0x158
#define PF_FLAGS_OFFS		0x218

#define MAX_WPN_SLOTS		8
#define MAX_TELE_SLOTS		8

// forward declarations
int64_t rfseek(RFILE* stream, int64_t offset, int origin);
int rfclose(RFILE* stream);
int rfgetc(RFILE* stream);
RFILE* rfopen(const char *path, const char *mode);
int rfprintf(RFILE * stream, const char * format, ...);

uint32_t rfgetl(RFILE *fp);
uint16_t rfgeti(RFILE *fp);
void rfputl(uint32_t word, RFILE *fp);
void rfputi(uint16_t word, RFILE *fp);
char rfbooleanread(RFILE *fp);
void rfputstringnonull(const char *buf, RFILE *fp);
void rfbooleanwrite(char bit, RFILE *fp);
bool rfverifystring(RFILE *fp, const char *str);
void rfbooleanflush(RFILE *fp);

// load savefile #num into the given Profile structure.
bool profile_load(const char *pfname, struct Profile *file)
{
   int i, curweaponslot;
   RFILE *fp = rfopen(pfname, "rb");

   memset(file, 0, sizeof(struct Profile));

   if (!fp)
      return 1;

   if (!rfverifystring(fp, "Do041220"))
      goto error;

   file->stage         = rfgetl(fp);
   file->songno        = rfgetl(fp);

   file->px            = rfgetl(fp);
   file->py            = rfgetl(fp);
   file->pdir          = CVTDir(rfgetl(fp));

   file->maxhp         = rfgeti(fp);
   file->num_whimstars = rfgeti(fp);
   file->hp            = rfgeti(fp);

   rfgeti(fp);				// unknown value
   curweaponslot       = rfgetl(fp);	// current weapon (slot, not number, converted below)
   rfgetl(fp);				// unknown value
   file->equipmask     = rfgetl(fp);	// equipped items

   // load weapons
   rfseek(fp, PF_WEAPONS_OFFS, SEEK_SET);
   for(i=0;i<MAX_WPN_SLOTS;i++)
   {
      int level, xp, maxammo, ammo;
      int type = rfgetl(fp);
      if (!type)
         break;

      level    = rfgetl(fp);
      xp       = rfgetl(fp);
      maxammo  = rfgetl(fp);
      ammo     = rfgetl(fp);

      file->weapons[type].hasWeapon = true;
      file->weapons[type].level     = (level - 1);
      file->weapons[type].xp        = xp;
      file->weapons[type].ammo      = ammo;
      file->weapons[type].maxammo   = maxammo;

      if (i == curweaponslot)
         file->curWeapon = type;
   }

   /* load inventory */
   file->ninventory = 0;
   rfseek(fp, PF_INVENTORY_OFFS, SEEK_SET);

   // 42 is MAX_INVENTORY
   for(i = 0; i < 42; i++)
   {
      int item = rfgetl(fp);
      if (!item)
         break;

      file->inventory[file->ninventory++] = item;
   }

   /* load teleporter slots */
   file->num_teleslots = 0;
   rfseek(fp, PF_TELEPORTER_OFFS, SEEK_SET);

   // 8 is NUM_TELEPORTER_SLOTS
   for(i = 0; i < 8; i++)
   {
      int slotno   = rfgetl(fp);
      int scriptno = rfgetl(fp);
      if (slotno == 0)
         break;

      file->teleslots[file->num_teleslots].slotno = slotno;
      file->teleslots[file->num_teleslots].scriptno = scriptno;
      file->num_teleslots++;
   }

   /* load flags */
   rfseek(fp, PF_FLAGS_OFFS, SEEK_SET);
   if (!rfverifystring(fp, "FLAG"))
      goto error;

   fresetboolean();
 
   // 8000 is NUM_GAMEFLAGS
   for(i = 0; i < 8000; i++)
      file->flags[i] = rfbooleanread(fp);

   rfclose(fp);
   return 0;

error:
   rfclose(fp);
   return 1;
}


bool profile_save(const char *pfname, struct Profile *file)
{
   int i, slotno = 0, curweaponslot = 0;
   RFILE *fp = rfopen(pfname, "wb");

   if (!fp)
      return 1;

   rfputstringnonull("Do041220", fp);

   rfputl(file->stage, fp);
   rfputl(file->songno, fp);

   rfputl(file->px, fp);
   rfputl(file->py, fp);
   // 0 is RIGHT 
   rfputl((file->pdir == 0) ? 2:0, fp);

   rfputi(file->maxhp, fp);
   rfputi(file->num_whimstars, fp);
   rfputi(file->hp, fp);

   rfseek(fp, 0x2C, SEEK_SET);
   rfputi(file->equipmask, fp);

   /* save weapons */
   rfseek(fp, PF_WEAPONS_OFFS, SEEK_SET);

   // 14 is WPN_COUNT
   for(i = 0; i < 14; i++)
   {
      if (file->weapons[i].hasWeapon)
      {
         rfputl(i, fp);
         rfputl(file->weapons[i].level + 1, fp);
         rfputl(file->weapons[i].xp, fp);
         rfputl(file->weapons[i].maxammo, fp);
         rfputl(file->weapons[i].ammo, fp);

         if (i == file->curWeapon)
            curweaponslot = slotno;

         slotno++;
         if (slotno >= MAX_WPN_SLOTS)
            break;
      }
   }

   if (slotno < MAX_WPN_SLOTS)
      rfputl(0, fp);	// 0-type weapon: terminator

   /* go back and save slot no of current weapon */
   rfseek(fp, PF_CURWEAPON_OFFS, SEEK_SET);
   rfputl(curweaponslot, fp);

   /* save inventory */
   rfseek(fp, PF_INVENTORY_OFFS, SEEK_SET);
   for(i=0;i<file->ninventory;i++)
      rfputl(file->inventory[i], fp);

   rfputl(0, fp);

   /* write teleporter slots */
   rfseek(fp, PF_TELEPORTER_OFFS, SEEK_SET);
   for(i=0;i<MAX_TELE_SLOTS;i++)
   {
      if (i < file->num_teleslots)
      {
         rfputl(file->teleslots[i].slotno, fp);
         rfputl(file->teleslots[i].scriptno, fp);
      }
      else
      {
         rfputl(0, fp);
         rfputl(0, fp);
      }
   }

   /* write flags */
   rfseek(fp, PF_FLAGS_OFFS, SEEK_SET);
   rfputstringnonull("FLAG", fp);

   fresetboolean();

   // 8000 is NUM_GAMEFLAGS
   for(i = 0; i < 8000; i++)
      rfbooleanwrite(file->flags[i], fp);

   rfbooleanflush(fp);

   rfclose(fp);
   return 0;
}

// returns the filename for a save file given it's number
const char *GetProfileName(int num)
{
   static char pfname_tmp[1024];
   char profile_name[1024];
   const char* save_dir = retro_get_save_dir();

   if (num == 0)
      snprintf(profile_name, sizeof(profile_name), "profile.dat");
   else
      snprintf(profile_name, sizeof(profile_name), "profile%d.dat", num+1);

   retro_create_path_string(pfname_tmp, sizeof(pfname_tmp), save_dir, profile_name);
   return pfname_tmp;
}

// returns whether the given save file slot exists
bool ProfileExists(int num)
{
   return path_is_valid(GetProfileName(num));
}

bool AnyProfileExists(void)
{
   int i;
   // 5 is MAX_SAVE_SLOTS
   for(i = 0; i < 5; i++)
      if (ProfileExists(i))
         return true;

   return false;
}

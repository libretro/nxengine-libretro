#include "cachefiles.h"
#include "../common/basics.h"
#include "../libretro/libretro_shared.h"
#include <map>
#include <stdio.h>
#include <stdint.h>

struct file_data
{
   uint8_t *data;
   size_t size;
   size_t offset;
};

#ifdef _WIN32
#define SLASH "\\"
#else
#define SLASH "/"
#endif

static const char *filenames[] = {
"Arms.pbm",
"ArmsImage.pbm",
"ArmsItem.tsc",
"bk0.pbm",
"bkBlack.pbm",
"bkBlue.pbm",
"bkFall.pbm",
"bkFog.pbm",
"bkGard.pbm",
"bkGray.pbm",
"bkGreen.pbm",
"bkMaze.pbm",
"bkMoon.pbm",
"bkRed.pbm",
"bkWater.pbm",
"Bullet.pbm",
"Caret.pbm",
"casts.pbm",
"Credit.tsc",
"Face.pbm",
"Fade.pbm",
"Head.tsc",
"ItemImage.pbm",
"Loading.pbm",
"MyChar.pbm",
"Npc" SLASH "Npc0.pbm",
"Npc" SLASH "NpcAlmo1.pbm",
"Npc" SLASH "NpcAlmo2.pbm",
"Npc" SLASH "NpcBallos.pbm",
"Npc" SLASH "NpcBllg.pbm",
"Npc" SLASH "NpcCemet.pbm",
"Npc" SLASH "NpcCent.pbm",
"Npc" SLASH "NpcCurly.pbm",
"Npc" SLASH "NpcDark.pbm",
"Npc" SLASH "NpcDr.pbm",
"Npc" SLASH "NpcEggs1.pbm",
"Npc" SLASH "NpcEggs2.pbm",
"Npc" SLASH "NpcFrog.pbm",
"Npc" SLASH "NpcGuest.pbm",
"Npc" SLASH "NpcHell.pbm",
"Npc" SLASH "NpcHeri.pbm",
"Npc" SLASH "NpcIronH.pbm",
"Npc" SLASH "NpcIsland.pbm",
"Npc" SLASH "NpcKings.pbm",
"Npc" SLASH "NpcMaze.pbm",
"Npc" SLASH "NpcMiza.pbm",
"Npc" SLASH "NpcMoon.pbm",
"Npc" SLASH "NpcOmg.pbm",
"Npc" SLASH "NpcPlant.pbm",
"Npc" SLASH "NpcPress.pbm",
"Npc" SLASH "NpcPriest.pbm",
"Npc" SLASH "NpcRavil.pbm",
"Npc" SLASH "NpcRed.pbm",
"Npc" SLASH "NpcRegu.pbm",
"Npc" SLASH "NpcSand.pbm",
"Npc" SLASH "NpcStream.pbm",
"Npc" SLASH "NpcSym.pbm",
"Npc" SLASH "NpcToro.pbm",
"Npc" SLASH "NpcTwinD.pbm",
"Npc" SLASH "NpcWeed.pbm",
"Npc" SLASH "NpcX.pbm",
"npc.tbl",
"sprites.sif",
"Stage" SLASH "0.pxa",
"Stage" SLASH "0.pxe",
"Stage" SLASH "0.pxm",
"Stage" SLASH "0.tsc",
"Stage" SLASH "555.pxe",
"Stage" SLASH "Almond.pxa",
"Stage" SLASH "Almond.pxe",
"Stage" SLASH "Almond.pxm",
"Stage" SLASH "Almond.tsc",
"Stage" SLASH "Ballo1.pxe",
"Stage" SLASH "Ballo1.pxm",
"Stage" SLASH "Ballo1.tsc",
"Stage" SLASH "Ballo2.pxe",
"Stage" SLASH "Ballo2.pxm",
"Stage" SLASH "Ballo2.tsc",
"Stage" SLASH "Barr.pxa",
"Stage" SLASH "Barr.pxe",
"Stage" SLASH "Barr.pxm",
"Stage" SLASH "Barr.tsc",
"Stage" SLASH "Blcny1.pxe",
"Stage" SLASH "Blcny1.pxm",
"Stage" SLASH "Blcny1.tsc",
"Stage" SLASH "Blcny2.pxe",
"Stage" SLASH "Blcny2.pxm",
"Stage" SLASH "Blcny2.tsc",
"Stage" SLASH "Cave.pxa",
"Stage" SLASH "Cave.pxe",
"Stage" SLASH "Cave.pxm",
"Stage" SLASH "Cave.tsc",
"Stage" SLASH "Cemet.pxe",
"Stage" SLASH "Cemet.pxm",
"Stage" SLASH "Cemet.tsc",
"Stage" SLASH "Cent.pxa",
"Stage" SLASH "Cent.pxe",
"Stage" SLASH "Cent.pxm",
"Stage" SLASH "Cent.tsc",
"Stage" SLASH "CentW.pxe",
"Stage" SLASH "CentW.pxm",
"Stage" SLASH "CentW.tsc",
"Stage" SLASH "Chako.pxe",
"Stage" SLASH "Chako.pxm",
"Stage" SLASH "Chako.tsc",
"Stage" SLASH "Clock.pxe",
"Stage" SLASH "Clock.pxm",
"Stage" SLASH "Clock.tsc",
"Stage" SLASH "Comu.pxe",
"Stage" SLASH "Comu.pxm",
"Stage" SLASH "Comu.tsc",
"Stage" SLASH "Cook.pxm",
"Stage" SLASH "Cthu.pxe",
"Stage" SLASH "Cthu.pxm",
"Stage" SLASH "Cthu.tsc",
"Stage" SLASH "Cthu2.pxe",
"Stage" SLASH "Cthu2.pxm",
"Stage" SLASH "Cthu2.tsc",
"Stage" SLASH "Curly.pxe",
"Stage" SLASH "Curly.pxm",
"Stage" SLASH "Curly.tsc",
"Stage" SLASH "CurlyS.pxe",
"Stage" SLASH "CurlyS.pxm",
"Stage" SLASH "CurlyS.tsc",
"Stage" SLASH "Dark.pxe",
"Stage" SLASH "Dark.pxm",
"Stage" SLASH "Dark.tsc",
"Stage" SLASH "Drain.pxe",
"Stage" SLASH "Drain.pxm",
"Stage" SLASH "Drain.tsc",
"Stage" SLASH "EgEnd1.pxe",
"Stage" SLASH "EgEnd1.pxm",
"Stage" SLASH "EgEnd1.tsc",
"Stage" SLASH "EgEnd2.pxe",
"Stage" SLASH "EgEnd2.pxm",
"Stage" SLASH "EgEnd2.tsc",
"Stage" SLASH "Egg1.pxe",
"Stage" SLASH "Egg1.pxm",
"Stage" SLASH "Egg1.tsc",
"Stage" SLASH "Egg6.pxe",
"Stage" SLASH "Egg6.pxm",
"Stage" SLASH "Egg6.tsc",
"Stage" SLASH "EggIn.pxa",
"Stage" SLASH "EggR.pxe",
"Stage" SLASH "EggR.pxm",
"Stage" SLASH "EggR.tsc",
"Stage" SLASH "EggR2.pxe",
"Stage" SLASH "EggR2.pxm",
"Stage" SLASH "EggR2.tsc",
"Stage" SLASH "Eggs.pxa",
"Stage" SLASH "Eggs.pxe",
"Stage" SLASH "Eggs.pxm",
"Stage" SLASH "Eggs.tsc",
"Stage" SLASH "Eggs2.pxe",
"Stage" SLASH "Eggs2.pxm",
"Stage" SLASH "Eggs2.tsc",
"Stage" SLASH "EggX.pxa",
"Stage" SLASH "EggX.pxe",
"Stage" SLASH "EggX.pxm",
"Stage" SLASH "EggX.tsc",
"Stage" SLASH "EggX2.pxe",
"Stage" SLASH "EggX2.pxm",
"Stage" SLASH "EggX2.tsc",
"Stage" SLASH "e_Blcn.pxe",
"Stage" SLASH "e_Blcn.pxm",
"Stage" SLASH "e_Blcn.tsc",
"Stage" SLASH "e_Ceme.pxe",
"Stage" SLASH "e_Ceme.pxm",
"Stage" SLASH "e_Ceme.tsc",
"Stage" SLASH "e_Jenk.pxe",
"Stage" SLASH "e_Jenk.pxm",
"Stage" SLASH "e_Jenk.tsc",
"Stage" SLASH "e_Labo.pxe",
"Stage" SLASH "e_Labo.pxm",
"Stage" SLASH "e_Labo.tsc",
"Stage" SLASH "e_Malc.pxe",
"Stage" SLASH "e_Malc.pxm",
"Stage" SLASH "e_Malc.tsc",
"Stage" SLASH "e_Maze.pxe",
"Stage" SLASH "e_Maze.pxm",
"Stage" SLASH "e_Maze.tsc",
"Stage" SLASH "e_Sky.pxe",
"Stage" SLASH "e_Sky.pxm",
"Stage" SLASH "e_Sky.tsc",
"Stage" SLASH "Fall.pxa",
"Stage" SLASH "Fall.pxe",
"Stage" SLASH "Fall.pxm",
"Stage" SLASH "Fall.tsc",
"Stage" SLASH "Frog.pxe",
"Stage" SLASH "Frog.pxm",
"Stage" SLASH "Frog.tsc",
"Stage" SLASH "Gard.pxa",
"Stage" SLASH "Gard.pxe",
"Stage" SLASH "Gard.pxm",
"Stage" SLASH "Gard.tsc",
"Stage" SLASH "Hell.pxa",
"Stage" SLASH "Hell1.pxe",
"Stage" SLASH "Hell1.pxm",
"Stage" SLASH "Hell1.tsc",
"Stage" SLASH "Hell2.pxe",
"Stage" SLASH "Hell2.pxm",
"Stage" SLASH "Hell2.tsc",
"Stage" SLASH "Hell3.pxe",
"Stage" SLASH "Hell3.pxm",
"Stage" SLASH "Hell3.tsc",
"Stage" SLASH "Hell4.pxe",
"Stage" SLASH "Hell4.pxm",
"Stage" SLASH "Hell4.tsc",
"Stage" SLASH "Hell42.pxe",
"Stage" SLASH "Hell42.pxm",
"Stage" SLASH "Hell42.tsc",
"Stage" SLASH "Island.pxe",
"Stage" SLASH "Island.pxm",
"Stage" SLASH "Island.tsc",
"Stage" SLASH "Itoh.pxe",
"Stage" SLASH "Itoh.pxm",
"Stage" SLASH "Itoh.tsc",
"Stage" SLASH "Jail.pxa",
"Stage" SLASH "Jail1.pxe",
"Stage" SLASH "Jail1.pxm",
"Stage" SLASH "Jail1.tsc",
"Stage" SLASH "Jail2.pxe",
"Stage" SLASH "Jail2.pxm",
"Stage" SLASH "Jail2.tsc",
"Stage" SLASH "Jenka1.pxe",
"Stage" SLASH "Jenka1.pxm",
"Stage" SLASH "Jenka1.tsc",
"Stage" SLASH "Jenka2.pxe",
"Stage" SLASH "Jenka2.pxm",
"Stage" SLASH "Jenka2.tsc",
"Stage" SLASH "Kings.pxe",
"Stage" SLASH "Kings.pxm",
"Stage" SLASH "Kings.tsc",
"Stage" SLASH "Labo.pxa",
"Stage" SLASH "Little.pxe",
"Stage" SLASH "Little.pxm",
"Stage" SLASH "Little.tsc",
"Stage" SLASH "Lounge.pxe",
"Stage" SLASH "Lounge.pxm",
"Stage" SLASH "Lounge.tsc",
"Stage" SLASH "Malco.pxe",
"Stage" SLASH "Malco.pxm",
"Stage" SLASH "Malco.tsc",
"Stage" SLASH "Mapi.pxe",
"Stage" SLASH "Mapi.pxm",
"Stage" SLASH "Mapi.tsc",
"Stage" SLASH "Maze.pxa",
"Stage" SLASH "MazeA.pxe",
"Stage" SLASH "MazeA.pxm",
"Stage" SLASH "MazeA.tsc",
"Stage" SLASH "MazeB.pxe",
"Stage" SLASH "MazeB.pxm",
"Stage" SLASH "MazeB.tsc",
"Stage" SLASH "MazeD.pxe",
"Stage" SLASH "MazeD.pxm",
"Stage" SLASH "MazeD.tsc",
"Stage" SLASH "MazeH.pxe",
"Stage" SLASH "MazeH.pxm",
"Stage" SLASH "MazeH.tsc",
"Stage" SLASH "MazeI.pxe",
"Stage" SLASH "MazeI.pxm",
"Stage" SLASH "MazeI.tsc",
"Stage" SLASH "MazeM.pxe",
"Stage" SLASH "MazeM.pxm",
"Stage" SLASH "MazeM.tsc",
"Stage" SLASH "MazeO.pxe",
"Stage" SLASH "MazeO.pxm",
"Stage" SLASH "MazeO.tsc",
"Stage" SLASH "MazeS.pxe",
"Stage" SLASH "MazeS.pxm",
"Stage" SLASH "MazeS.tsc",
"Stage" SLASH "MazeW.pxe",
"Stage" SLASH "MazeW.pxm",
"Stage" SLASH "MazeW.tsc",
"Stage" SLASH "MiBox.pxe",
"Stage" SLASH "MiBox.pxm",
"Stage" SLASH "MiBox.tsc",
"Stage" SLASH "Mimi.pxa",
"Stage" SLASH "Mimi.pxe",
"Stage" SLASH "Mimi.pxm",
"Stage" SLASH "Mimi.tsc",
"Stage" SLASH "Momo.pxe",
"Stage" SLASH "Momo.pxm",
"Stage" SLASH "Momo.tsc",
"Stage" SLASH "New.pxe",
"Stage" SLASH "Oside.pxa",
"Stage" SLASH "Oside.pxe",
"Stage" SLASH "Oside.pxm",
"Stage" SLASH "Oside.tsc",
"Stage" SLASH "Ostep.pxe",
"Stage" SLASH "Ostep.pxm",
"Stage" SLASH "Ostep.tsc",
"Stage" SLASH "Pens.pxa",
"Stage" SLASH "Pens1.pxe",
"Stage" SLASH "Pens1.pxm",
"Stage" SLASH "Pens1.tsc",
"Stage" SLASH "Pens2.pxe",
"Stage" SLASH "Pens2.pxm",
"Stage" SLASH "Pens2.tsc",
"Stage" SLASH "Pixel.pxe",
"Stage" SLASH "Pixel.pxm",
"Stage" SLASH "Pixel.tsc",
"Stage" SLASH "Plant.pxe",
"Stage" SLASH "Plant.pxm",
"Stage" SLASH "Plant.tsc",
"Stage" SLASH "Pole.pxe",
"Stage" SLASH "Pole.pxm",
"Stage" SLASH "Pole.tsc",
"Stage" SLASH "Pool.pxe",
"Stage" SLASH "Pool.pxm",
"Stage" SLASH "Pool.tsc",
"Stage" SLASH "Prefa1.pxe",
"Stage" SLASH "Prefa1.pxm",
"Stage" SLASH "Prefa1.tsc",
"Stage" SLASH "Prefa2.pxe",
"Stage" SLASH "Prefa2.pxm",
"Stage" SLASH "Prefa2.tsc",
"Stage" SLASH "Priso1.pxe",
"Stage" SLASH "Priso1.pxm",
"Stage" SLASH "Priso1.tsc",
"Stage" SLASH "Priso2.pxe",
"Stage" SLASH "Priso2.pxm",
"Stage" SLASH "Priso2.tsc",
"Stage" SLASH "Prt0.pbm",
"Stage" SLASH "PrtAlmond.pbm",
"Stage" SLASH "PrtBarr.pbm",
"Stage" SLASH "PrtCave.pbm",
"Stage" SLASH "PrtCent.pbm",
"Stage" SLASH "PrtEggIn.pbm",
"Stage" SLASH "PrtEggs.pbm",
"Stage" SLASH "PrtEggX.pbm",
"Stage" SLASH "PrtFall.pbm",
"Stage" SLASH "PrtGard.pbm",
"Stage" SLASH "PrtHell.pbm",
"Stage" SLASH "PrtJail.pbm",
"Stage" SLASH "PrtLabo.pbm",
"Stage" SLASH "PrtMaze.pbm",
"Stage" SLASH "PrtMimi.pbm",
"Stage" SLASH "PrtOside.pbm",
"Stage" SLASH "PrtPens.pbm",
"Stage" SLASH "PrtRiver.pbm",
"Stage" SLASH "PrtSand.pbm",
"Stage" SLASH "PrtStore.pbm",
"Stage" SLASH "PrtWeed.pbm",
"Stage" SLASH "PrtWhite.pbm",
"Stage" SLASH "Ring1.pxe",
"Stage" SLASH "Ring1.pxm",
"Stage" SLASH "Ring1.tsc",
"Stage" SLASH "Ring2.pxe",
"Stage" SLASH "Ring2.pxm",
"Stage" SLASH "Ring2.tsc",
"Stage" SLASH "Ring3.pxe",
"Stage" SLASH "Ring3.pxm",
"Stage" SLASH "Ring3.tsc",
"Stage" SLASH "River.pxa",
"Stage" SLASH "River.pxe",
"Stage" SLASH "River.pxm",
"Stage" SLASH "River.tsc",
"Stage" SLASH "Sand.pxa",
"Stage" SLASH "Sand.pxe",
"Stage" SLASH "Sand.pxm",
"Stage" SLASH "Sand.tsc",
"Stage" SLASH "SandE.pxe",
"Stage" SLASH "SandE.pxm",
"Stage" SLASH "SandE.tsc",
"Stage" SLASH "Santa.pxe",
"Stage" SLASH "Santa.pxm",
"Stage" SLASH "Santa.tsc",
"Stage" SLASH "Shelt.pxa",
"Stage" SLASH "Shelt.pxe",
"Stage" SLASH "Shelt.pxm",
"Stage" SLASH "Shelt.tsc",
"Stage" SLASH "Start.pxe",
"Stage" SLASH "Start.pxm",
"Stage" SLASH "Start.tsc",
"Stage" SLASH "Statue.pxe",
"Stage" SLASH "Statue.pxm",
"Stage" SLASH "Statue.tsc",
"Stage" SLASH "Store.pxa",
"Stage" SLASH "Stream.pxe",
"Stage" SLASH "Stream.pxm",
"Stage" SLASH "Stream.tsc",
"Stage" SLASH "Weed.pxa",
"Stage" SLASH "Weed.pxe",
"Stage" SLASH "Weed.pxm",
"Stage" SLASH "Weed.tsc",
"Stage" SLASH "WeedB.pxe",
"Stage" SLASH "WeedB.pxm",
"Stage" SLASH "WeedB.tsc",
"Stage" SLASH "WeedD.pxe",
"Stage" SLASH "WeedD.pxm",
"Stage" SLASH "WeedD.tsc",
"Stage" SLASH "WeedS.pxe",
"Stage" SLASH "WeedS.pxm",
"Stage" SLASH "WeedS.tsc",
"Stage" SLASH "White.pxa",
"StageImage.pbm",
"StageSelect.tsc",
"TextBox.pbm",
"Title.pbm"
};

static std::map<const char *, CFILE> filemap;

void cachefiles_init()
{
   for (unsigned i = 0; i < sizeof(filenames) / sizeof(filenames[0]); i++)
   {
      // reentrancy test
      if (filemap.find(filenames[i]) != filemap.end())
         continue;

      CFILE fd = {0};
      char fname[1024];
      retro_create_path_string(fname, sizeof(fname), g_dir, filenames[i]);
      FILE *f = fopen(fname, "r");
      if (!f)
         continue;

      fseek(f, 0, SEEK_END);
      fd.size = ftell(f);
      fseek(f, 0, SEEK_SET);

      fd.data = (uint8_t *) malloc(fd.size);
      if (!fd.data)
      {
         fclose(f);
         continue;
      }

      fread(fd.data, fd.size, 1, f);
      fclose(f);

      filemap[filenames[i]] = fd;
   }
}

CFILE *copen(const char *fname, const char *mode)
{
   (void)mode;
   if (filemap.find(fname) == filemap.end())
      return NULL;
   // create local copy in case we have the same file open multiple times
   CFILE *f = (CFILE *)malloc(sizeof(CFILE));
   if (!f)
      return NULL;
   *f = filemap[fname];
   f->offset = 0;
   return f;
}

void cclose(CFILE *f)
{
   free(f);
}

void cseek(CFILE *f, int offset, int origin)
{
   if (origin == SEEK_SET)
      f->offset = offset;
   else if (origin == SEEK_END)
      f->offset = f->size - offset;
   else if (origin == SEEK_CUR)
      f->offset += offset;
}

size_t ctell(CFILE *f)
{
   return f->offset;
}

int cgetc(CFILE *f)
{
   int c = f->data[f->offset++];
   return c;
}

uint16_t cgeti(CFILE *f)
{
   uint16_t a, b;
	a = cgetc(f);
	b = cgetc(f);
	return (b << 8) | (a);
}

uint32_t cgetl(CFILE *f)
{
   uint32_t a, b, c, d;
	a = cgetc(f);
	b = cgetc(f);
	c = cgetc(f);
	d = cgetc(f);
	return (d << 24) | (c << 16) | (b << 8) | (a);
}

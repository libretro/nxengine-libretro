#include "cachefiles.h"
#include "../common/basics.h"
#include "../libretro/libretro_shared.h"
#include "../nx.h"
#include <map>
#include <string>
#include <stdio.h>
#include <string.h>

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
"data" SLASH "Arms.pbm",
"data" SLASH "ArmsImage.pbm",
"data" SLASH "ArmsItem.tsc",
"data" SLASH "bk0.pbm",
"data" SLASH "bkBlack.pbm",
"data" SLASH "bkBlue.pbm",
"data" SLASH "bkFall.pbm",
"data" SLASH "bkFog.pbm",
"data" SLASH "bkGard.pbm",
"data" SLASH "bkGray.pbm",
"data" SLASH "bkGreen.pbm",
"data" SLASH "bkMaze.pbm",
"data" SLASH "bkMoon.pbm",
"data" SLASH "bkRed.pbm",
"data" SLASH "bkWater.pbm",
"data" SLASH "Bullet.pbm",
"data" SLASH "Caret.pbm",
"data" SLASH "casts.pbm",
"data" SLASH "Credit.tsc",
"data" SLASH "Face.pbm",
"data" SLASH "Fade.pbm",
"data" SLASH "Head.tsc",
"data" SLASH "ItemImage.pbm",
"data" SLASH "Loading.pbm",
"data" SLASH "MyChar.pbm",
"data" SLASH "Npc" SLASH "Npc0.pbm",
"data" SLASH "Npc" SLASH "NpcAlmo1.pbm",
"data" SLASH "Npc" SLASH "NpcAlmo2.pbm",
"data" SLASH "Npc" SLASH "NpcBallos.pbm",
"data" SLASH "Npc" SLASH "NpcBllg.pbm",
"data" SLASH "Npc" SLASH "NpcCemet.pbm",
"data" SLASH "Npc" SLASH "NpcCent.pbm",
"data" SLASH "Npc" SLASH "NpcCurly.pbm",
"data" SLASH "Npc" SLASH "NpcDark.pbm",
"data" SLASH "Npc" SLASH "NpcDr.pbm",
"data" SLASH "Npc" SLASH "NpcEggs1.pbm",
"data" SLASH "Npc" SLASH "NpcEggs2.pbm",
"data" SLASH "Npc" SLASH "NpcFrog.pbm",
"data" SLASH "Npc" SLASH "NpcGuest.pbm",
"data" SLASH "Npc" SLASH "NpcHell.pbm",
"data" SLASH "Npc" SLASH "NpcHeri.pbm",
"data" SLASH "Npc" SLASH "NpcIronH.pbm",
"data" SLASH "Npc" SLASH "NpcIsland.pbm",
"data" SLASH "Npc" SLASH "NpcKings.pbm",
"data" SLASH "Npc" SLASH "NpcMaze.pbm",
"data" SLASH "Npc" SLASH "NpcMiza.pbm",
"data" SLASH "Npc" SLASH "NpcMoon.pbm",
"data" SLASH "Npc" SLASH "NpcOmg.pbm",
"data" SLASH "Npc" SLASH "NpcPlant.pbm",
"data" SLASH "Npc" SLASH "NpcPress.pbm",
"data" SLASH "Npc" SLASH "NpcPriest.pbm",
"data" SLASH "Npc" SLASH "NpcRavil.pbm",
"data" SLASH "Npc" SLASH "NpcRed.pbm",
"data" SLASH "Npc" SLASH "NpcRegu.pbm",
"data" SLASH "Npc" SLASH "NpcSand.pbm",
"data" SLASH "Npc" SLASH "NpcStream.pbm",
"data" SLASH "Npc" SLASH "NpcSym.pbm",
"data" SLASH "Npc" SLASH "NpcToro.pbm",
"data" SLASH "Npc" SLASH "NpcTwinD.pbm",
"data" SLASH "Npc" SLASH "NpcWeed.pbm",
"data" SLASH "Npc" SLASH "NpcX.pbm",
"data" SLASH "npc.tbl",
"data" SLASH "sprites.sif",
"data" SLASH "Stage" SLASH "0.pxa",
"data" SLASH "Stage" SLASH "0.pxe",
"data" SLASH "Stage" SLASH "0.pxm",
"data" SLASH "Stage" SLASH "0.tsc",
"data" SLASH "Stage" SLASH "555.pxe",
"data" SLASH "Stage" SLASH "Almond.pxa",
"data" SLASH "Stage" SLASH "Almond.pxe",
"data" SLASH "Stage" SLASH "Almond.pxm",
"data" SLASH "Stage" SLASH "Almond.tsc",
"data" SLASH "Stage" SLASH "Ballo1.pxe",
"data" SLASH "Stage" SLASH "Ballo1.pxm",
"data" SLASH "Stage" SLASH "Ballo1.tsc",
"data" SLASH "Stage" SLASH "Ballo2.pxe",
"data" SLASH "Stage" SLASH "Ballo2.pxm",
"data" SLASH "Stage" SLASH "Ballo2.tsc",
"data" SLASH "Stage" SLASH "Barr.pxa",
"data" SLASH "Stage" SLASH "Barr.pxe",
"data" SLASH "Stage" SLASH "Barr.pxm",
"data" SLASH "Stage" SLASH "Barr.tsc",
"data" SLASH "Stage" SLASH "Blcny1.pxe",
"data" SLASH "Stage" SLASH "Blcny1.pxm",
"data" SLASH "Stage" SLASH "Blcny1.tsc",
"data" SLASH "Stage" SLASH "Blcny2.pxe",
"data" SLASH "Stage" SLASH "Blcny2.pxm",
"data" SLASH "Stage" SLASH "Blcny2.tsc",
"data" SLASH "Stage" SLASH "Cave.pxa",
"data" SLASH "Stage" SLASH "Cave.pxe",
"data" SLASH "Stage" SLASH "Cave.pxm",
"data" SLASH "Stage" SLASH "Cave.tsc",
"data" SLASH "Stage" SLASH "Cemet.pxe",
"data" SLASH "Stage" SLASH "Cemet.pxm",
"data" SLASH "Stage" SLASH "Cemet.tsc",
"data" SLASH "Stage" SLASH "Cent.pxa",
"data" SLASH "Stage" SLASH "Cent.pxe",
"data" SLASH "Stage" SLASH "Cent.pxm",
"data" SLASH "Stage" SLASH "Cent.tsc",
"data" SLASH "Stage" SLASH "CentW.pxe",
"data" SLASH "Stage" SLASH "CentW.pxm",
"data" SLASH "Stage" SLASH "CentW.tsc",
"data" SLASH "Stage" SLASH "Chako.pxe",
"data" SLASH "Stage" SLASH "Chako.pxm",
"data" SLASH "Stage" SLASH "Chako.tsc",
"data" SLASH "Stage" SLASH "Clock.pxe",
"data" SLASH "Stage" SLASH "Clock.pxm",
"data" SLASH "Stage" SLASH "Clock.tsc",
"data" SLASH "Stage" SLASH "Comu.pxe",
"data" SLASH "Stage" SLASH "Comu.pxm",
"data" SLASH "Stage" SLASH "Comu.tsc",
"data" SLASH "Stage" SLASH "Cook.pxm",
"data" SLASH "Stage" SLASH "Cthu.pxe",
"data" SLASH "Stage" SLASH "Cthu.pxm",
"data" SLASH "Stage" SLASH "Cthu.tsc",
"data" SLASH "Stage" SLASH "Cthu2.pxe",
"data" SLASH "Stage" SLASH "Cthu2.pxm",
"data" SLASH "Stage" SLASH "Cthu2.tsc",
"data" SLASH "Stage" SLASH "Curly.pxe",
"data" SLASH "Stage" SLASH "Curly.pxm",
"data" SLASH "Stage" SLASH "Curly.tsc",
"data" SLASH "Stage" SLASH "CurlyS.pxe",
"data" SLASH "Stage" SLASH "CurlyS.pxm",
"data" SLASH "Stage" SLASH "CurlyS.tsc",
"data" SLASH "Stage" SLASH "Dark.pxe",
"data" SLASH "Stage" SLASH "Dark.pxm",
"data" SLASH "Stage" SLASH "Dark.tsc",
"data" SLASH "Stage" SLASH "Drain.pxe",
"data" SLASH "Stage" SLASH "Drain.pxm",
"data" SLASH "Stage" SLASH "Drain.tsc",
"data" SLASH "Stage" SLASH "EgEnd1.pxe",
"data" SLASH "Stage" SLASH "EgEnd1.pxm",
"data" SLASH "Stage" SLASH "EgEnd1.tsc",
"data" SLASH "Stage" SLASH "EgEnd2.pxe",
"data" SLASH "Stage" SLASH "EgEnd2.pxm",
"data" SLASH "Stage" SLASH "EgEnd2.tsc",
"data" SLASH "Stage" SLASH "Egg1.pxe",
"data" SLASH "Stage" SLASH "Egg1.pxm",
"data" SLASH "Stage" SLASH "Egg1.tsc",
"data" SLASH "Stage" SLASH "Egg6.pxe",
"data" SLASH "Stage" SLASH "Egg6.pxm",
"data" SLASH "Stage" SLASH "Egg6.tsc",
"data" SLASH "Stage" SLASH "EggIn.pxa",
"data" SLASH "Stage" SLASH "EggR.pxe",
"data" SLASH "Stage" SLASH "EggR.pxm",
"data" SLASH "Stage" SLASH "EggR.tsc",
"data" SLASH "Stage" SLASH "EggR2.pxe",
"data" SLASH "Stage" SLASH "EggR2.pxm",
"data" SLASH "Stage" SLASH "EggR2.tsc",
"data" SLASH "Stage" SLASH "Eggs.pxa",
"data" SLASH "Stage" SLASH "Eggs.pxe",
"data" SLASH "Stage" SLASH "Eggs.pxm",
"data" SLASH "Stage" SLASH "Eggs.tsc",
"data" SLASH "Stage" SLASH "Eggs2.pxe",
"data" SLASH "Stage" SLASH "Eggs2.pxm",
"data" SLASH "Stage" SLASH "Eggs2.tsc",
"data" SLASH "Stage" SLASH "EggX.pxa",
"data" SLASH "Stage" SLASH "EggX.pxe",
"data" SLASH "Stage" SLASH "EggX.pxm",
"data" SLASH "Stage" SLASH "EggX.tsc",
"data" SLASH "Stage" SLASH "EggX2.pxe",
"data" SLASH "Stage" SLASH "EggX2.pxm",
"data" SLASH "Stage" SLASH "EggX2.tsc",
"data" SLASH "Stage" SLASH "e_Blcn.pxe",
"data" SLASH "Stage" SLASH "e_Blcn.pxm",
"data" SLASH "Stage" SLASH "e_Blcn.tsc",
"data" SLASH "Stage" SLASH "e_Ceme.pxe",
"data" SLASH "Stage" SLASH "e_Ceme.pxm",
"data" SLASH "Stage" SLASH "e_Ceme.tsc",
"data" SLASH "Stage" SLASH "e_Jenk.pxe",
"data" SLASH "Stage" SLASH "e_Jenk.pxm",
"data" SLASH "Stage" SLASH "e_Jenk.tsc",
"data" SLASH "Stage" SLASH "e_Labo.pxe",
"data" SLASH "Stage" SLASH "e_Labo.pxm",
"data" SLASH "Stage" SLASH "e_Labo.tsc",
"data" SLASH "Stage" SLASH "e_Malc.pxe",
"data" SLASH "Stage" SLASH "e_Malc.pxm",
"data" SLASH "Stage" SLASH "e_Malc.tsc",
"data" SLASH "Stage" SLASH "e_Maze.pxe",
"data" SLASH "Stage" SLASH "e_Maze.pxm",
"data" SLASH "Stage" SLASH "e_Maze.tsc",
"data" SLASH "Stage" SLASH "e_Sky.pxe",
"data" SLASH "Stage" SLASH "e_Sky.pxm",
"data" SLASH "Stage" SLASH "e_Sky.tsc",
"data" SLASH "Stage" SLASH "Fall.pxa",
"data" SLASH "Stage" SLASH "Fall.pxe",
"data" SLASH "Stage" SLASH "Fall.pxm",
"data" SLASH "Stage" SLASH "Fall.tsc",
"data" SLASH "Stage" SLASH "Frog.pxe",
"data" SLASH "Stage" SLASH "Frog.pxm",
"data" SLASH "Stage" SLASH "Frog.tsc",
"data" SLASH "Stage" SLASH "Gard.pxa",
"data" SLASH "Stage" SLASH "Gard.pxe",
"data" SLASH "Stage" SLASH "Gard.pxm",
"data" SLASH "Stage" SLASH "Gard.tsc",
"data" SLASH "Stage" SLASH "Hell.pxa",
"data" SLASH "Stage" SLASH "Hell1.pxe",
"data" SLASH "Stage" SLASH "Hell1.pxm",
"data" SLASH "Stage" SLASH "Hell1.tsc",
"data" SLASH "Stage" SLASH "Hell2.pxe",
"data" SLASH "Stage" SLASH "Hell2.pxm",
"data" SLASH "Stage" SLASH "Hell2.tsc",
"data" SLASH "Stage" SLASH "Hell3.pxe",
"data" SLASH "Stage" SLASH "Hell3.pxm",
"data" SLASH "Stage" SLASH "Hell3.tsc",
"data" SLASH "Stage" SLASH "Hell4.pxe",
"data" SLASH "Stage" SLASH "Hell4.pxm",
"data" SLASH "Stage" SLASH "Hell4.tsc",
"data" SLASH "Stage" SLASH "Hell42.pxe",
"data" SLASH "Stage" SLASH "Hell42.pxm",
"data" SLASH "Stage" SLASH "Hell42.tsc",
"data" SLASH "Stage" SLASH "Island.pxe",
"data" SLASH "Stage" SLASH "Island.pxm",
"data" SLASH "Stage" SLASH "Island.tsc",
"data" SLASH "Stage" SLASH "Itoh.pxe",
"data" SLASH "Stage" SLASH "Itoh.pxm",
"data" SLASH "Stage" SLASH "Itoh.tsc",
"data" SLASH "Stage" SLASH "Jail.pxa",
"data" SLASH "Stage" SLASH "Jail1.pxe",
"data" SLASH "Stage" SLASH "Jail1.pxm",
"data" SLASH "Stage" SLASH "Jail1.tsc",
"data" SLASH "Stage" SLASH "Jail2.pxe",
"data" SLASH "Stage" SLASH "Jail2.pxm",
"data" SLASH "Stage" SLASH "Jail2.tsc",
"data" SLASH "Stage" SLASH "Jenka1.pxe",
"data" SLASH "Stage" SLASH "Jenka1.pxm",
"data" SLASH "Stage" SLASH "Jenka1.tsc",
"data" SLASH "Stage" SLASH "Jenka2.pxe",
"data" SLASH "Stage" SLASH "Jenka2.pxm",
"data" SLASH "Stage" SLASH "Jenka2.tsc",
"data" SLASH "Stage" SLASH "Kings.pxe",
"data" SLASH "Stage" SLASH "Kings.pxm",
"data" SLASH "Stage" SLASH "Kings.tsc",
"data" SLASH "Stage" SLASH "Labo.pxa",
"data" SLASH "Stage" SLASH "Little.pxe",
"data" SLASH "Stage" SLASH "Little.pxm",
"data" SLASH "Stage" SLASH "Little.tsc",
"data" SLASH "Stage" SLASH "Lounge.pxe",
"data" SLASH "Stage" SLASH "Lounge.pxm",
"data" SLASH "Stage" SLASH "Lounge.tsc",
"data" SLASH "Stage" SLASH "Malco.pxe",
"data" SLASH "Stage" SLASH "Malco.pxm",
"data" SLASH "Stage" SLASH "Malco.tsc",
"data" SLASH "Stage" SLASH "Mapi.pxe",
"data" SLASH "Stage" SLASH "Mapi.pxm",
"data" SLASH "Stage" SLASH "Mapi.tsc",
"data" SLASH "Stage" SLASH "Maze.pxa",
"data" SLASH "Stage" SLASH "MazeA.pxe",
"data" SLASH "Stage" SLASH "MazeA.pxm",
"data" SLASH "Stage" SLASH "MazeA.tsc",
"data" SLASH "Stage" SLASH "MazeB.pxe",
"data" SLASH "Stage" SLASH "MazeB.pxm",
"data" SLASH "Stage" SLASH "MazeB.tsc",
"data" SLASH "Stage" SLASH "MazeD.pxe",
"data" SLASH "Stage" SLASH "MazeD.pxm",
"data" SLASH "Stage" SLASH "MazeD.tsc",
"data" SLASH "Stage" SLASH "MazeH.pxe",
"data" SLASH "Stage" SLASH "MazeH.pxm",
"data" SLASH "Stage" SLASH "MazeH.tsc",
"data" SLASH "Stage" SLASH "MazeI.pxe",
"data" SLASH "Stage" SLASH "MazeI.pxm",
"data" SLASH "Stage" SLASH "MazeI.tsc",
"data" SLASH "Stage" SLASH "MazeM.pxe",
"data" SLASH "Stage" SLASH "MazeM.pxm",
"data" SLASH "Stage" SLASH "MazeM.tsc",
"data" SLASH "Stage" SLASH "MazeO.pxe",
"data" SLASH "Stage" SLASH "MazeO.pxm",
"data" SLASH "Stage" SLASH "MazeO.tsc",
"data" SLASH "Stage" SLASH "MazeS.pxe",
"data" SLASH "Stage" SLASH "MazeS.pxm",
"data" SLASH "Stage" SLASH "MazeS.tsc",
"data" SLASH "Stage" SLASH "MazeW.pxe",
"data" SLASH "Stage" SLASH "MazeW.pxm",
"data" SLASH "Stage" SLASH "MazeW.tsc",
"data" SLASH "Stage" SLASH "MiBox.pxe",
"data" SLASH "Stage" SLASH "MiBox.pxm",
"data" SLASH "Stage" SLASH "MiBox.tsc",
"data" SLASH "Stage" SLASH "Mimi.pxa",
"data" SLASH "Stage" SLASH "Mimi.pxe",
"data" SLASH "Stage" SLASH "Mimi.pxm",
"data" SLASH "Stage" SLASH "Mimi.tsc",
"data" SLASH "Stage" SLASH "Momo.pxe",
"data" SLASH "Stage" SLASH "Momo.pxm",
"data" SLASH "Stage" SLASH "Momo.tsc",
"data" SLASH "Stage" SLASH "New.pxe",
"data" SLASH "Stage" SLASH "Oside.pxa",
"data" SLASH "Stage" SLASH "Oside.pxe",
"data" SLASH "Stage" SLASH "Oside.pxm",
"data" SLASH "Stage" SLASH "Oside.tsc",
"data" SLASH "Stage" SLASH "Ostep.pxe",
"data" SLASH "Stage" SLASH "Ostep.pxm",
"data" SLASH "Stage" SLASH "Ostep.tsc",
"data" SLASH "Stage" SLASH "Pens.pxa",
"data" SLASH "Stage" SLASH "Pens1.pxe",
"data" SLASH "Stage" SLASH "Pens1.pxm",
"data" SLASH "Stage" SLASH "Pens1.tsc",
"data" SLASH "Stage" SLASH "Pens2.pxe",
"data" SLASH "Stage" SLASH "Pens2.pxm",
"data" SLASH "Stage" SLASH "Pens2.tsc",
"data" SLASH "Stage" SLASH "Pixel.pxe",
"data" SLASH "Stage" SLASH "Pixel.pxm",
"data" SLASH "Stage" SLASH "Pixel.tsc",
"data" SLASH "Stage" SLASH "Plant.pxe",
"data" SLASH "Stage" SLASH "Plant.pxm",
"data" SLASH "Stage" SLASH "Plant.tsc",
"data" SLASH "Stage" SLASH "Pole.pxe",
"data" SLASH "Stage" SLASH "Pole.pxm",
"data" SLASH "Stage" SLASH "Pole.tsc",
"data" SLASH "Stage" SLASH "Pool.pxe",
"data" SLASH "Stage" SLASH "Pool.pxm",
"data" SLASH "Stage" SLASH "Pool.tsc",
"data" SLASH "Stage" SLASH "Prefa1.pxe",
"data" SLASH "Stage" SLASH "Prefa1.pxm",
"data" SLASH "Stage" SLASH "Prefa1.tsc",
"data" SLASH "Stage" SLASH "Prefa2.pxe",
"data" SLASH "Stage" SLASH "Prefa2.pxm",
"data" SLASH "Stage" SLASH "Prefa2.tsc",
"data" SLASH "Stage" SLASH "Priso1.pxe",
"data" SLASH "Stage" SLASH "Priso1.pxm",
"data" SLASH "Stage" SLASH "Priso1.tsc",
"data" SLASH "Stage" SLASH "Priso2.pxe",
"data" SLASH "Stage" SLASH "Priso2.pxm",
"data" SLASH "Stage" SLASH "Priso2.tsc",
"data" SLASH "Stage" SLASH "Prt0.pbm",
"data" SLASH "Stage" SLASH "PrtAlmond.pbm",
"data" SLASH "Stage" SLASH "PrtBarr.pbm",
"data" SLASH "Stage" SLASH "PrtCave.pbm",
"data" SLASH "Stage" SLASH "PrtCent.pbm",
"data" SLASH "Stage" SLASH "PrtEggIn.pbm",
"data" SLASH "Stage" SLASH "PrtEggs.pbm",
"data" SLASH "Stage" SLASH "PrtEggX.pbm",
"data" SLASH "Stage" SLASH "PrtFall.pbm",
"data" SLASH "Stage" SLASH "PrtGard.pbm",
"data" SLASH "Stage" SLASH "PrtHell.pbm",
"data" SLASH "Stage" SLASH "PrtJail.pbm",
"data" SLASH "Stage" SLASH "PrtLabo.pbm",
"data" SLASH "Stage" SLASH "PrtMaze.pbm",
"data" SLASH "Stage" SLASH "PrtMimi.pbm",
"data" SLASH "Stage" SLASH "PrtOside.pbm",
"data" SLASH "Stage" SLASH "PrtPens.pbm",
"data" SLASH "Stage" SLASH "PrtRiver.pbm",
"data" SLASH "Stage" SLASH "PrtSand.pbm",
"data" SLASH "Stage" SLASH "PrtStore.pbm",
"data" SLASH "Stage" SLASH "PrtWeed.pbm",
"data" SLASH "Stage" SLASH "PrtWhite.pbm",
"data" SLASH "Stage" SLASH "Ring1.pxe",
"data" SLASH "Stage" SLASH "Ring1.pxm",
"data" SLASH "Stage" SLASH "Ring1.tsc",
"data" SLASH "Stage" SLASH "Ring2.pxe",
"data" SLASH "Stage" SLASH "Ring2.pxm",
"data" SLASH "Stage" SLASH "Ring2.tsc",
"data" SLASH "Stage" SLASH "Ring3.pxe",
"data" SLASH "Stage" SLASH "Ring3.pxm",
"data" SLASH "Stage" SLASH "Ring3.tsc",
"data" SLASH "Stage" SLASH "River.pxa",
"data" SLASH "Stage" SLASH "River.pxe",
"data" SLASH "Stage" SLASH "River.pxm",
"data" SLASH "Stage" SLASH "River.tsc",
"data" SLASH "Stage" SLASH "Sand.pxa",
"data" SLASH "Stage" SLASH "Sand.pxe",
"data" SLASH "Stage" SLASH "Sand.pxm",
"data" SLASH "Stage" SLASH "Sand.tsc",
"data" SLASH "Stage" SLASH "SandE.pxe",
"data" SLASH "Stage" SLASH "SandE.pxm",
"data" SLASH "Stage" SLASH "SandE.tsc",
"data" SLASH "Stage" SLASH "Santa.pxe",
"data" SLASH "Stage" SLASH "Santa.pxm",
"data" SLASH "Stage" SLASH "Santa.tsc",
"data" SLASH "Stage" SLASH "Shelt.pxa",
"data" SLASH "Stage" SLASH "Shelt.pxe",
"data" SLASH "Stage" SLASH "Shelt.pxm",
"data" SLASH "Stage" SLASH "Shelt.tsc",
"data" SLASH "Stage" SLASH "Start.pxe",
"data" SLASH "Stage" SLASH "Start.pxm",
"data" SLASH "Stage" SLASH "Start.tsc",
"data" SLASH "Stage" SLASH "Statue.pxe",
"data" SLASH "Stage" SLASH "Statue.pxm",
"data" SLASH "Stage" SLASH "Statue.tsc",
"data" SLASH "Stage" SLASH "Store.pxa",
"data" SLASH "Stage" SLASH "Stream.pxe",
"data" SLASH "Stage" SLASH "Stream.pxm",
"data" SLASH "Stage" SLASH "Stream.tsc",
"data" SLASH "Stage" SLASH "Weed.pxa",
"data" SLASH "Stage" SLASH "Weed.pxe",
"data" SLASH "Stage" SLASH "Weed.pxm",
"data" SLASH "Stage" SLASH "Weed.tsc",
"data" SLASH "Stage" SLASH "WeedB.pxe",
"data" SLASH "Stage" SLASH "WeedB.pxm",
"data" SLASH "Stage" SLASH "WeedB.tsc",
"data" SLASH "Stage" SLASH "WeedD.pxe",
"data" SLASH "Stage" SLASH "WeedD.pxm",
"data" SLASH "Stage" SLASH "WeedD.tsc",
"data" SLASH "Stage" SLASH "WeedS.pxe",
"data" SLASH "Stage" SLASH "WeedS.pxm",
"data" SLASH "Stage" SLASH "WeedS.tsc",
"data" SLASH "Stage" SLASH "White.pxa",
"data" SLASH "StageImage.pbm",
"data" SLASH "StageSelect.tsc",
"data" SLASH "TextBox.pbm",
"data" SLASH "Title.pbm"
};

static std::map<std::string, CFILE> filemap;

void cachefiles_init()
{
   for (unsigned i = 0; i < sizeof(filenames) / sizeof(filenames[0]); i++)
   {
      printf("%s\n", filenames[i]);
      // reentrancy test
      if (filemap.find(filenames[i]) != filemap.end())
         continue;

      CFILE fd = {0};
      char fname[1024];
      retro_create_path_string(fname, sizeof(fname), g_dir, filenames[i]);
      FILE *f = fopen(fname, "rb");
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
   printf("copen %s\n", fname);

   // create local copy in case we have the same file open multiple times
   CFILE *f = (CFILE *)malloc(sizeof(CFILE));
   if (!f)
      return NULL;
   *f = filemap[fname];
   if (!f->data)
   {
      free(f);
      return NULL;
   }
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

size_t cread(void *ptr, size_t size, size_t count, CFILE *f)
{
   // not correct but good enough for our cases
   memcpy(ptr, &f->data[f->offset], size * count);
   f->offset += size * count;
   return count;
}

int cgetc(CFILE *f)
{
   if (f->offset >= f->size)
      return EOF;
   else
      return f->data[f->offset++];
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

bool cverifystring(CFILE *f, const char *str)
{
   int i;
   char result = 1;
   int stringlength = strlen(str);

   for(i = 0; i < stringlength; i++)
      if (cgetc(f) != str[i])
         result = 0;

   return result;
}

void *cfile_pointer(CFILE *f)
{
   return f->data;
}

size_t cfile_size(CFILE *f)
{
   return f->size;
}

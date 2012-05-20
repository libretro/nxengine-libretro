DEBUG=0
SAFEMODE=0
DEBUGLOG=0

ifeq ($(platform),)
platform = unix
ifeq ($(shell uname -a),)
   platform = win
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
   platform = osx
else ifneq ($(findstring win,$(shell uname -a)),)
   platform = win
endif
endif

CC         = gcc
EXTRACTDIR = extract-auto

ifeq ($(platform), unix)
   TARGET := libretro.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=port-libretro/link.T -Wl,-no-undefined
   CFLAGS += -D_GNU_SOURCE=1
else ifeq ($(platform), osx)
   TARGET := libretro.dylib
   fpic := -fPIC
   SHARED := -dynamiclib
else ifeq ($(platform), ps3)
   TARGET := libretro.a
   CC = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-gcc.exe
   CXX = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-g++.exe
   AR = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-ar.exe
   CFLAGS += -DMSB_FIRST=1
else ifeq ($(platform), sncps3)
   TARGET := libretro.a
   CC = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   CXX = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   AR = $(CELL_SDK)/host-win32/sn/bin/ps3snarl.exe
   CFLAGS +=  -DMSB_FIRST=1
else ifeq ($(platform), xenon)
   TARGET := libretro.a
   CC = xenon-gcc
   CXX = xenon-g++
   AR = xenon-ar
   CFLAGS += -D__LIBXENON__ -D__ppc_ -DMSB_FIRST=1
else ifeq ($(platform), wii)
   TARGET := libretro.a
   CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc
   CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar
   CFLAGS += -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float -DMSB_FIRST=1
else
   TARGET := retro.dll
   CC = gcc
   SHARED := -shared -static-libgcc -static-libstdc++ -s -Wl,--version-script=port-libretro/link.T
   CFLAGS += -D__WIN32__ -D__WIN32_LIBRETRO__ -Wno-missing-field-initializers
endif

ifeq ($(DEBUG), 1)
CFLAGS += -O0 -g
else
CFLAGS += -O3
endif

AI_OBJS := ai/ai.o ai/balrog_common.o ai/IrregularBBox.o ai/almond/almond.o ai/boss/balfrog.o ai/boss/ballos.o ai/boss/core.o ai/boss/heavypress.o ai/boss/ironhead.o ai/boss/omega.o ai/boss/sisters.o ai/boss/undead_core.o ai/boss/x.o ai/egg/egg.o ai/egg/egg2.o ai/egg/igor.o ai/final_battle/balcony.o ai/final_battle/doctor.o ai/final_battle/doctor_common.o ai/final_battle/doctor_frenzied.o ai/final_battle/final_misc.o ai/final_battle/misery.o ai/final_battle/sidekicks.o ai/first_cave/first_cave.o ai/hell/ballos_misc.o ai/hell/ballos_priest.o ai/hell/hell.o ai/last_cave/last_cave.o ai/maze/balrog_boss_missiles.o  ai/maze/critter_purple.o ai/maze/gaudi.o ai/maze/labyrinth_m.o ai/maze/pooh_black.o ai/npc/balrog.o ai/npc/curly.o ai/npc/curly_ai.o ai/npc/misery.o ai/npc/npcguest.o ai/npc/npcplayer.o ai/npc/npcregu.o ai/oside/oside.o ai/plantation/plantation.o ai/sand/curly_boss.o ai/sand/puppy.o ai/sand/sand.o ai/sand/toroko_frenzied.o ai/sym/smoke.o ai/sym/sym.o ai/village/balrog_boss_running.o ai/village/ma_pignon.o ai/village/village.o ai/weapons/blade.o ai/weapons/bubbler.o ai/weapons/fireball.o ai/weapons/missile.o ai/weapons/nemesis.o ai/weapons/polar_mgun.o ai/weapons/snake.o ai/weapons/spur.o ai/weapons/weapons.o ai/weapons/whimstar.o ai/weed/balrog_boss_flying.o ai/weed/frenzied_mimiga.o ai/weed/weed.o

COMMON_OBJS := common/BList.o common/bufio.o common/DBuffer.o common/DString.o common/FileBuffer.o common/InitList.o common/misc.o common/stat.o common/StringList.o

ENDGAME_OBJS := endgame/credits.o endgame/CredReader.o endgame/island.o endgame/misc.o

EXTRACT_OBJS := $(EXTRACTDIR)/crc.o $(EXTRACTDIR)/extract.o $(EXTRACTDIR)/extractfiles.o $(EXTRACTDIR)/extractpxt.o $(EXTRACTDIR)/extractstages.o

GRAPHICS_OBJS := graphics/graphics.o graphics/nxsurface.o graphics/font.o graphics/sprites.o graphics/tileset.o

ifeq ($(SAFEMODE), 1)
SAFEMODE_OBJS := graphics/safemode.o
CFLAGS += -DUSE_SAFEMODE=1
else
SAFEMODE_OBJS :=
endif

ifeq ($(DEBUGLOG), 1)
CFLAGS += -DDEBUG_LOG=1
endif

INTRO_OBJS := intro/intro.o intro/title.o

PAUSE_OBJS := pause/dialog.o pause/message.o pause/objects.o pause/options.o pause/pause.o

LIBRETRO_OBJS := port-libretro/libretro.o

PORT_OBJS := port-libretro/

MAIN_OBJS := port-libretro/main.o

SIFLIB_OBJS := siflib/sectSprites.o siflib/sectStringArray.o siflib/sif.o siflib/sifloader.o

SOUND_OBJS := sound/org.o sound/pxt.o sound/sound.o sound/sslib.o

TEXTBOX_OBJS := TextBox/ItemImage.o TextBox/SaveSelect.o TextBox/StageSelect.o TextBox/TextBox.o TextBox/YesNoPrompt.o

SDL_OBJS := sdl/SDL_error.o sdl/file/SDL_rwops.o sdl/stdlib/SDL_string.o sdl/video/SDL_blit.o sdl/video/SDL_blit_1.o sdl/video/SDL_blit_A.o sdl/video/SDL_blit_N.o sdl/video/SDL_bmp.o sdl/video/SDL_pixels.o sdl/video/SDL_surface.o sdl/video/SDL_video.o

AUTOGEN_OBJS := autogen/AssignSprites.o autogen/objnames.o

DEBUG_OBJS := debug.o

OBJECTS    := 	caret.o console.o floattext.o game.o input.o inventory.o $(MAIN_OBJS) map.o map_system.o niku.o object.o ObjManager.o p_arms.o platform.o player.o playerstats.o profile.o replay.o screeneffect.o settings.o slope.o stageboss.o stagedata.o statusbar.o trig.o tsc.o  $(AI_OBJS) $(SAFEMODE_OBJS) $(COMMON_OBJS) $(ENDGAME_OBJS) $(EXTRACT_OBJS) $(GRAPHICS_OBJS) $(INTRO_OBJS) $(PAUSE_OBJS) $(SIFLIB_OBJS) $(SOUND_OBJS) $(TEXTBOX_OBJS) $(SDL_OBJS) $(AUTOGEN_OBJS) $(DEBUG_OBJS)

OBJECTS += $(LIBRETRO_OBJS)

INCLUDES   = -I. -I.. -Igraphics -Iport-libretro -Isdl/include
DEFINES    = -DHAVE_INTTYPES_H -D__LIBRETRO__ -DINLINE=inline

ifeq ($(platform), sncps3)
WARNINGS_DEFINES =
CODE_DEFINES =
else
WARNINGS_DEFINES = -Wall -W -Wno-unused-parameter
CODE_DEFINES = -fomit-frame-pointer
endif

COMMON_DEFINES += $(CODE_DEFINES) $(WARNINGS_DEFINES) -DNDEBUG=1 $(fpic)

CFLAGS     += $(DEFINES) $(COMMON_DEFINES)

all: $(TARGET)

$(TARGET): $(OBJECTS)
ifeq ($(platform), ps3)
	$(AR) rcs $@ $(OBJECTS)
else ifeq ($(platform), sncps3)
	$(AR) rcs $@ $(OBJECTS)
else ifeq ($(platform), xenon)
	$(AR) rcs $@ $(OBJECTS)
else ifeq ($(platform), wii)
	$(AR) rcs $@ $(OBJECTS)
else
	$(CXX) $(fpic) $(SHARED) $(INCLUDES) $(CFLAGS) -o $@ $(OBJECTS) -lm
endif

%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(INCLUDES) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

cleandata:
	rm -f wavetable.dat
	rm -f stage.dat
	rm -f settings.dat
	rm -rf pxt
	rm -rf org
	rm -rf endpic

.PHONY: clean


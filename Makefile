DEBUG=0
SAFEMODE=0
DEBUGLOG=0
RELEASE_BUILD=1

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

# system platform
system_platform = unix
ifeq ($(shell uname -a),)
EXE_EXT = .exe
   system_platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
   system_platform = osx
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   system_platform = win
endif

NX_DIR     = nxengine-1.0.0.4
EXTRACTDIR = $(NX_DIR)/extract-auto

CC         = gcc

ifeq ($(platform), unix)
   TARGET := nxengine_libretro.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=$(NX_DIR)/libretro/link.T -Wl,-no-undefined
   CFLAGS += -D_GNU_SOURCE=1
else ifeq ($(platform), osx)
   TARGET := nxengine_libretro.dylib
   fpic := -fPIC
   SHARED := -dynamiclib
else ifeq ($(platform), ps3)
   TARGET := nxengine_libretro_ps3.a
   CC = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-gcc.exe
   CXX = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-g++.exe
   AR = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-ar.exe
   CFLAGS += -DMSB_FIRST=1 -DSDL_BYTEORDER=SDL_BIG_ENDIAN
else ifeq ($(platform), sncps3)
   TARGET := nxengine_libretro_ps3.a
   CC = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   CXX = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   AR = $(CELL_SDK)/host-win32/sn/bin/ps3snarl.exe
   CFLAGS +=  -DMSB_FIRST=1 -DSDL_BYTEORDER=SDL_BIG_ENDIAN
else ifeq ($(platform), psl1ght)
   TARGET := nxengine_libretro_psl1ght.a
   CC = $(PS3DEV)/ppu/bin/ppu-gcc$(EXE_EXT)
   CXX = $(PS3DEV)/ppu/bin/ppu-g++$(EXE_EXT)
   AR = $(PS3DEV)/ppu/bin/ppu-ar$(EXE_EXT)
   CFLAGS += -DMSB_FIRST=1 -DSDL_BYTEORDER=SDL_BIG_ENDIAN
else ifeq ($(platform), psp1)
   TARGET := nxengine_libretro_psp1.a
   CC = psp-gcc$(EXE_EXT)
   CXX = psp-g++$(EXE_EXT)
   AR = psp-ar$(EXE_EXT)
   CFLAGS += -DGNU_SOURCE=1 -G0
else ifeq ($(platform), xenon)
   TARGET := nxengine_libretro_xenon360.a
   CC = xenon-gcc$(EXE_EXT)
   CXX = xenon-g++$(EXE_EXT)
   AR = xenon-ar$(EXE_EXT)
   CFLAGS += -D__LIBXENON__ -D__ppc_ -DMSB_FIRST=1 -DSDL_BYTEORDER=SDL_BIG_ENDIAN
else ifeq ($(platform), ngc)
   TARGET := nxengine_libretro_ngc.a
   CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
   CFLAGS += -DGEKKO -DHW_DOL -mrvl -mcpu=750 -meabi -mhard-float -DMSB_FIRST=1 -DSDL_BYTEORDER=SDL_BIG_ENDIAN
else ifeq ($(platform), wii)
   TARGET := nxengine_libretro_wii.a
   CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
   CFLAGS += -DGEKKO _DHW_RVL -mrvl -mcpu=750 -meabi -mhard-float -DMSB_FIRST=1 -DSDL_BYTEORDER=SDL_BIG_ENDIAN
else
   TARGET := nxengine_retro.dll
   CC = gcc
   SHARED := -shared -static-libgcc -static-libstdc++ -s -Wl,--version-script=libretro/link.T
   CFLAGS += -D__WIN32__ -D__WIN32_LIBRETRO__ -Wno-missing-field-initializers
endif

ifeq ($(DEBUG), 1)
CFLAGS += -O0 -g
else
CFLAGS += -O3
endif

ifeq ($(RELEASE_BUILD), 1)
CFLAGS += -DRELEASE_BUILD
endif

AI_OBJS := $(NX_DIR)/ai/ai.o $(NX_DIR)/ai/balrog_common.o $(NX_DIR)/ai/IrregularBBox.o $(NX_DIR)/ai/almond/almond.o $(NX_DIR)/ai/boss/balfrog.o $(NX_DIR)/ai/boss/ballos.o $(NX_DIR)/ai/boss/core.o $(NX_DIR)/ai/boss/heavypress.o $(NX_DIR)/ai/boss/ironhead.o $(NX_DIR)/ai/boss/omega.o $(NX_DIR)/ai/boss/sisters.o $(NX_DIR)/ai/boss/undead_core.o $(NX_DIR)/ai/boss/x.o $(NX_DIR)/ai/egg/egg.o $(NX_DIR)/ai/egg/egg2.o $(NX_DIR)/ai/egg/igor.o $(NX_DIR)/ai/final_battle/balcony.o $(NX_DIR)/ai/final_battle/doctor.o $(NX_DIR)/ai/final_battle/doctor_common.o $(NX_DIR)/ai/final_battle/doctor_frenzied.o $(NX_DIR)/ai/final_battle/final_misc.o $(NX_DIR)/ai/final_battle/misery.o $(NX_DIR)/ai/final_battle/sidekicks.o $(NX_DIR)/ai/first_cave/first_cave.o $(NX_DIR)/ai/hell/ballos_misc.o $(NX_DIR)/ai/hell/ballos_priest.o $(NX_DIR)/ai/hell/hell.o $(NX_DIR)/ai/last_cave/last_cave.o $(NX_DIR)/ai/maze/balrog_boss_missiles.o  $(NX_DIR)/ai/maze/critter_purple.o $(NX_DIR)/ai/maze/gaudi.o $(NX_DIR)/ai/maze/labyrinth_m.o $(NX_DIR)/ai/maze/pooh_black.o $(NX_DIR)/ai/npc/balrog.o $(NX_DIR)/ai/npc/curly.o $(NX_DIR)/ai/npc/curly_ai.o $(NX_DIR)/ai/npc/misery.o $(NX_DIR)/ai/npc/npcguest.o $(NX_DIR)/ai/npc/npcplayer.o $(NX_DIR)/ai/npc/npcregu.o $(NX_DIR)/ai/oside/oside.o $(NX_DIR)/ai/plantation/plantation.o $(NX_DIR)/ai/sand/curly_boss.o $(NX_DIR)/ai/sand/puppy.o $(NX_DIR)/ai/sand/sand.o $(NX_DIR)/ai/sand/toroko_frenzied.o $(NX_DIR)/ai/sym/smoke.o $(NX_DIR)/ai/sym/sym.o $(NX_DIR)/ai/village/balrog_boss_running.o $(NX_DIR)/ai/village/ma_pignon.o $(NX_DIR)/ai/village/village.o $(NX_DIR)/ai/weapons/blade.o $(NX_DIR)/ai/weapons/bubbler.o $(NX_DIR)/ai/weapons/fireball.o $(NX_DIR)/ai/weapons/missile.o $(NX_DIR)/ai/weapons/nemesis.o $(NX_DIR)/ai/weapons/polar_mgun.o $(NX_DIR)/ai/weapons/snake.o $(NX_DIR)/ai/weapons/spur.o $(NX_DIR)/ai/weapons/weapons.o $(NX_DIR)/ai/weapons/whimstar.o $(NX_DIR)/ai/weed/balrog_boss_flying.o $(NX_DIR)/ai/weed/frenzied_mimiga.o $(NX_DIR)/ai/weed/weed.o

COMMON_OBJS := $(NX_DIR)/common/BList.o $(NX_DIR)/common/bufio.o $(NX_DIR)/common/DBuffer.o $(NX_DIR)/common/DString.o $(NX_DIR)/common/FileBuffer.o $(NX_DIR)/common/InitList.o $(NX_DIR)/common/misc.o $(NX_DIR)/common/StringList.o

ENDGAME_OBJS := $(NX_DIR)/endgame/credits.o $(NX_DIR)/endgame/CredReader.o $(NX_DIR)/endgame/island.o $(NX_DIR)/endgame/misc.o

EXTRACT_OBJS := $(EXTRACTDIR)/crc.o $(EXTRACTDIR)/extract.o $(EXTRACTDIR)/extractfiles.o $(EXTRACTDIR)/extractpxt.o $(EXTRACTDIR)/extractstages.o

GRAPHICS_OBJS := $(NX_DIR)/graphics/graphics.o $(NX_DIR)/graphics/nxsurface.o $(NX_DIR)/graphics/font.o $(NX_DIR)/graphics/sprites.o $(NX_DIR)/graphics/tileset.o

ifeq ($(SAFEMODE), 1)
SAFEMODE_OBJS := $(NX_DIR)/graphics/safemode.o
CFLAGS += -DUSE_SAFEMODE=1
else
SAFEMODE_OBJS :=
endif

ifeq ($(DEBUGLOG), 1)
CFLAGS += -DDEBUG_LOG=1
endif

INTRO_OBJS := $(NX_DIR)/intro/intro.o $(NX_DIR)/intro/title.o

PAUSE_OBJS := $(NX_DIR)/pause/dialog.o $(NX_DIR)/pause/message.o $(NX_DIR)/pause/objects.o $(NX_DIR)/pause/options.o $(NX_DIR)/pause/pause.o

LIBRETRO_OBJS := $(NX_DIR)/libretro/libretro.o

PORT_OBJS := $(NX_DIR)/libretro/

MAIN_OBJS := $(NX_DIR)/libretro/main.o

SIFLIB_OBJS := $(NX_DIR)/siflib/sectSprites.o $(NX_DIR)/siflib/sectStringArray.o $(NX_DIR)/siflib/sif.o $(NX_DIR)/siflib/sifloader.o

SOUND_OBJS := $(NX_DIR)/sound/org.o $(NX_DIR)/sound/pxt.o $(NX_DIR)/sound/sound.o $(NX_DIR)/sound/sslib.o

TEXTBOX_OBJS := $(NX_DIR)/TextBox/ItemImage.o $(NX_DIR)/TextBox/SaveSelect.o $(NX_DIR)/TextBox/StageSelect.o $(NX_DIR)/TextBox/TextBox.o $(NX_DIR)/TextBox/YesNoPrompt.o

SDL_OBJS := $(NX_DIR)/sdl/SDL_error.o $(NX_DIR)/sdl/file/SDL_rwops.o $(NX_DIR)/sdl/stdlib/SDL_string.o $(NX_DIR)/sdl/video/SDL_blit.o $(NX_DIR)/sdl/video/SDL_blit_0.o $(NX_DIR)/sdl/video/SDL_blit_1.o $(NX_DIR)/sdl/video/SDL_blit_A.o $(NX_DIR)/sdl/video/SDL_blit_N.o $(NX_DIR)/sdl/video/SDL_bmp.o $(NX_DIR)/sdl/video/SDL_pixels.o $(NX_DIR)/sdl/video/SDL_surface.o $(NX_DIR)/sdl/video/SDL_video.o $(NX_DIR)/sdl/cpuinfo/SDL_cpuinfo.o

AUTOGEN_OBJS := $(NX_DIR)/autogen/AssignSprites.o $(NX_DIR)/autogen/objnames.o

DEBUG_OBJS := $(NX_DIR)/debug.o

OBJECTS    := 	$(NX_DIR)/caret.o $(NX_DIR)/console.o $(NX_DIR)/floattext.o $(NX_DIR)/game.o $(NX_DIR)/input.o $(NX_DIR)/inventory.o $(MAIN_OBJS) $(NX_DIR)/map.o $(NX_DIR)/map_system.o $(NX_DIR)/niku.o $(NX_DIR)/object.o $(NX_DIR)/ObjManager.o $(NX_DIR)/p_arms.o $(NX_DIR)/platform.o $(NX_DIR)/player.o $(NX_DIR)/playerstats.o $(NX_DIR)/profile.o $(NX_DIR)/replay.o $(NX_DIR)/screeneffect.o $(NX_DIR)/settings.o $(NX_DIR)/slope.o $(NX_DIR)/stageboss.o $(NX_DIR)/stagedata.o $(NX_DIR)/statusbar.o $(NX_DIR)/trig.o $(NX_DIR)/tsc.o  $(AI_OBJS) $(SAFEMODE_OBJS) $(COMMON_OBJS) $(ENDGAME_OBJS) $(EXTRACT_OBJS) $(GRAPHICS_OBJS) $(INTRO_OBJS) $(PAUSE_OBJS) $(SIFLIB_OBJS) $(SOUND_OBJS) $(TEXTBOX_OBJS) $(SDL_OBJS) $(AUTOGEN_OBJS) $(DEBUG_OBJS)

OBJECTS += $(LIBRETRO_OBJS)

INCLUDES   = -I. -I$(NX_DIR) -I$(NX_DIR)/graphics -I$(NX_DIR)/libretro -I$(NX_DIR)/sdl/include
DEFINES    = -DHAVE_INTTYPES_H -D__LIBRETRO__ -DINLINE=inline -DFRONTEND_SUPPORTS_RGB565

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
else ifeq ($(platform), psl1ght)
	$(AR) rcs $@ $(OBJECTS)
else ifeq ($(platform), psp1)
	$(AR) rcs $@ $(OBJECTS)
else ifeq ($(platform), xenon)
	$(AR) rcs $@ $(OBJECTS)
else ifeq ($(platform), ngc)
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


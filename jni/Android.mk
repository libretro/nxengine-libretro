LOCAL_PATH := $(call my-dir)
RELEASE_BUILD=1

include $(CLEAR_VARS)

NX_DIR     = ../nxengine-1.0.0.4
EXTRACTDIR = $(NX_DIR)/extract-auto

LOCAL_MODULE    := libretro

ifeq ($(RELEASE_BUILD), 1)
LOCAL_CXXFLAGS += -DRELEASE_BUILD
endif

ifeq ($(TARGET_ARCH),arm)
LOCAL_CXXFLAGS += -DANDROID_ARM
LOCAL_ARM_MODE := arm
endif

ifeq ($(TARGET_ARCH),x86)
LOCAL_CXXFLAGS +=  -DANDROID_X86
endif

ifeq ($(TARGET_ARCH),mips)
LOCAL_CXXFLAGS += -DANDROID_MIPS
endif

AI_OBJS := $(NX_DIR)/ai/ai.cpp $(NX_DIR)/ai/balrog_common.cpp $(NX_DIR)/ai/IrregularBBox.cpp $(NX_DIR)/ai/almond/almond.cpp $(NX_DIR)/ai/boss/balfrog.cpp $(NX_DIR)/ai/boss/ballos.cpp $(NX_DIR)/ai/boss/core.cpp $(NX_DIR)/ai/boss/heavypress.cpp $(NX_DIR)/ai/boss/ironhead.cpp $(NX_DIR)/ai/boss/omega.cpp $(NX_DIR)/ai/boss/sisters.cpp $(NX_DIR)/ai/boss/undead_core.cpp $(NX_DIR)/ai/boss/x.cpp $(NX_DIR)/ai/egg/egg.cpp $(NX_DIR)/ai/egg/egg2.cpp $(NX_DIR)/ai/egg/igor.cpp $(NX_DIR)/ai/final_battle/balcony.cpp $(NX_DIR)/ai/final_battle/doctor.cpp $(NX_DIR)/ai/final_battle/doctor_common.cpp $(NX_DIR)/ai/final_battle/doctor_frenzied.cpp $(NX_DIR)/ai/final_battle/final_misc.cpp $(NX_DIR)/ai/final_battle/misery.cpp $(NX_DIR)/ai/final_battle/sidekicks.cpp $(NX_DIR)/ai/first_cave/first_cave.cpp $(NX_DIR)/ai/hell/ballos_misc.cpp $(NX_DIR)/ai/hell/ballos_priest.cpp $(NX_DIR)/ai/hell/hell.cpp $(NX_DIR)/ai/last_cave/last_cave.cpp $(NX_DIR)/ai/maze/balrog_boss_missiles.cpp  $(NX_DIR)/ai/maze/critter_purple.cpp $(NX_DIR)/ai/maze/gaudi.cpp $(NX_DIR)/ai/maze/labyrinth_m.cpp $(NX_DIR)/ai/maze/pooh_black.cpp $(NX_DIR)/ai/maze/maze.cpp $(NX_DIR)/ai/npc/balrog.cpp $(NX_DIR)/ai/npc/curly.cpp $(NX_DIR)/ai/npc/curly_ai.cpp $(NX_DIR)/ai/npc/misery.cpp $(NX_DIR)/ai/npc/npcguest.cpp $(NX_DIR)/ai/npc/npcplayer.cpp $(NX_DIR)/ai/npc/npcregu.cpp $(NX_DIR)/ai/oside/oside.cpp $(NX_DIR)/ai/plantation/plantation.cpp $(NX_DIR)/ai/sand/curly_boss.cpp $(NX_DIR)/ai/sand/puppy.cpp $(NX_DIR)/ai/sand/sand.cpp $(NX_DIR)/ai/sand/toroko_frenzied.cpp $(NX_DIR)/ai/sym/smoke.cpp $(NX_DIR)/ai/sym/sym.cpp $(NX_DIR)/ai/village/balrog_boss_running.cpp $(NX_DIR)/ai/village/ma_pignon.cpp $(NX_DIR)/ai/village/village.cpp $(NX_DIR)/ai/weapons/blade.cpp $(NX_DIR)/ai/weapons/bubbler.cpp $(NX_DIR)/ai/weapons/fireball.cpp $(NX_DIR)/ai/weapons/missile.cpp $(NX_DIR)/ai/weapons/nemesis.cpp $(NX_DIR)/ai/weapons/polar_mgun.cpp $(NX_DIR)/ai/weapons/snake.cpp $(NX_DIR)/ai/weapons/spur.cpp $(NX_DIR)/ai/weapons/weapons.cpp $(NX_DIR)/ai/weapons/whimstar.cpp $(NX_DIR)/ai/weed/balrog_boss_flying.cpp $(NX_DIR)/ai/weed/frenzied_mimiga.cpp $(NX_DIR)/ai/weed/weed.cpp

COMMON_OBJS := $(NX_DIR)/common/BList.cpp $(NX_DIR)/common/bufio.cpp $(NX_DIR)/common/DBuffer.cpp $(NX_DIR)/common/DString.cpp $(NX_DIR)/common/FileBuffer.cpp $(NX_DIR)/common/InitList.cpp $(NX_DIR)/common/misc.cpp $(NX_DIR)/common/StringList.cpp

ENDGAME_OBJS := $(NX_DIR)/endgame/credits.cpp $(NX_DIR)/endgame/CredReader.cpp $(NX_DIR)/endgame/island.cpp $(NX_DIR)/endgame/misc.cpp

EXTRACT_OBJS := $(EXTRACTDIR)/crc.cpp $(EXTRACTDIR)/extract.cpp $(EXTRACTDIR)/extractfiles.cpp $(EXTRACTDIR)/extractpxt.cpp $(EXTRACTDIR)/extractstages.cpp

GRAPHICS_OBJS := $(NX_DIR)/graphics/graphics.cpp $(NX_DIR)/graphics/nxsurface.cpp $(NX_DIR)/graphics/font.cpp $(NX_DIR)/graphics/sprites.cpp $(NX_DIR)/graphics/tileset.cpp

ifeq ($(DEBUGLOG), 1)
CFLAGS += -DDEBUG_LOG=1
endif

INTRO_OBJS := $(NX_DIR)/intro/intro.cpp $(NX_DIR)/intro/title.cpp

PAUSE_OBJS := $(NX_DIR)/pause/dialog.cpp $(NX_DIR)/pause/message.cpp $(NX_DIR)/pause/objects.cpp $(NX_DIR)/pause/options.cpp $(NX_DIR)/pause/pause.cpp

LIBRETRO_OBJS := $(NX_DIR)/libretro/libretro.cpp

PORT_OBJS := $(NX_DIR)/libretro/

MAIN_OBJS := $(NX_DIR)/main.cpp

SIFLIB_OBJS := $(NX_DIR)/siflib/sectSprites.cpp $(NX_DIR)/siflib/sectStringArray.cpp $(NX_DIR)/siflib/sif.cpp $(NX_DIR)/siflib/sifloader.cpp

SOUND_OBJS := $(NX_DIR)/sound/org.cpp $(NX_DIR)/sound/pxt.cpp $(NX_DIR)/sound/sound.cpp $(NX_DIR)/sound/sslib.cpp

TEXTBOX_OBJS := $(NX_DIR)/TextBox/ItemImage.cpp $(NX_DIR)/TextBox/SaveSelect.cpp $(NX_DIR)/TextBox/StageSelect.cpp $(NX_DIR)/TextBox/TextBox.cpp $(NX_DIR)/TextBox/YesNoPrompt.cpp

SDL_OBJS := $(NX_DIR)/sdl/SDL_error.c $(NX_DIR)/sdl/file/SDL_rwops.c $(NX_DIR)/sdl/stdlib/SDL_string.c $(NX_DIR)/sdl/video/SDL_blit.c $(NX_DIR)/sdl/video/SDL_blit_0.c $(NX_DIR)/sdl/video/SDL_blit_1.c $(NX_DIR)/sdl/video/SDL_blit_A.c $(NX_DIR)/sdl/video/SDL_blit_N.c $(NX_DIR)/sdl/video/SDL_bmp.c $(NX_DIR)/sdl/video/SDL_pixels.c $(NX_DIR)/sdl/video/SDL_surface.c $(NX_DIR)/sdl/video/SDL_video.c $(NX_DIR)/sdl/cpuinfo/SDL_cpuinfo.c

AUTOGEN_OBJS := $(NX_DIR)/autogen/AssignSprites.cpp $(NX_DIR)/autogen/objnames.cpp

DEBUG_OBJS := $(NX_DIR)/debug.cpp

OBJECTS    := 	$(NX_DIR)/caret.cpp $(NX_DIR)/console.cpp $(NX_DIR)/floattext.cpp $(NX_DIR)/game.cpp $(NX_DIR)/input.cpp $(NX_DIR)/inventory.cpp $(MAIN_OBJS) $(NX_DIR)/map.cpp $(NX_DIR)/map_system.cpp $(NX_DIR)/niku.cpp $(NX_DIR)/object.cpp $(NX_DIR)/ObjManager.cpp $(NX_DIR)/p_arms.cpp $(NX_DIR)/platform.cpp $(NX_DIR)/player.cpp $(NX_DIR)/playerstats.cpp $(NX_DIR)/profile.cpp $(NX_DIR)/replay.cpp $(NX_DIR)/screeneffect.cpp $(NX_DIR)/settings.cpp $(NX_DIR)/slope.cpp $(NX_DIR)/stageboss.cpp $(NX_DIR)/stagedata.cpp $(NX_DIR)/statusbar.cpp $(NX_DIR)/trig.cpp $(NX_DIR)/tsc.cpp  $(AI_OBJS) $(SAFEMODE_OBJS) $(COMMON_OBJS) $(ENDGAME_OBJS) $(EXTRACT_OBJS) $(GRAPHICS_OBJS) $(INTRO_OBJS) $(PAUSE_OBJS) $(SIFLIB_OBJS) $(SOUND_OBJS) $(TEXTBOX_OBJS) $(SDL_OBJS) $(AUTOGEN_OBJS) $(DEBUG_OBJS) $(LIBRETRO_OBJS)

LOCAL_SRC_FILES := $(OBJECTS)

LOCAL_CXXFLAGS += -DINLINE=inline -DHAVE_STDINT_H -DHAVE_INTTYPES_H -D__LIBRETRO__ -DFRONTEND_SUPPORTS_RGB565
LOCAL_C_INCLUDES  = $(NX_DIR) $(NX_DIR)/graphics $(NX_DIR)/libretro $(NX_DIR)/sdl/include

include $(BUILD_SHARED_LIBRARY)

DEBUG=0
DEBUGLOG=0
RELEASE_BUILD=1
SINGLE_PRECISION_FLOATS=0
MIN_AUDIO_PROCESSING_PER_FRAME=0

ifeq ($(platform),)
platform = unix
ifeq ($(shell uname -a),)
   platform = win
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
   platform = osx
	arch = intel
ifeq ($(shell uname -p),powerpc)
	arch = ppc
endif
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
	arch = intel
ifeq ($(shell uname -p),powerpc)
	arch = ppc
endif
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   system_platform = win
endif

TARGET_NAME := nxengine

CORE_DIR     := nxengine
EXTRACTDIR   := $(CORE_DIR)/extract-auto

CC         = gcc

ifeq ($(ARCHFLAGS),)
ifeq ($(archs),ppc)
   ARCHFLAGS = -arch ppc -arch ppc64
else
   ARCHFLAGS = -arch i386 -arch x86_64
endif
endif

ifeq ($(platform), unix)
   TARGET := $(TARGET_NAME)_libretro.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=$(CORE_DIR)/libretro/link.T -Wl,-no-undefined
   CFLAGS += -D_GNU_SOURCE=1
else ifeq ($(platform), osx)
   TARGET := $(TARGET_NAME)_libretro.dylib
   fpic := -fPIC
   SHARED := -dynamiclib
ifeq ($(arch),ppc)
   CFLAGS += -DMSB_FIRST=1 -DSDL_BYTEORDER=SDL_BIG_ENDIAN
endif
   CFLAGS += -DOSX
   OSXVER = `sw_vers -productVersion | cut -d. -f 2`
   OSX_LT_MAVERICKS = `(( $(OSXVER) <= 9)) && echo "YES"`
   fpic += -mmacosx-version-min=10.1

# iOS
else ifneq (,$(findstring ios,$(platform)))
   TARGET := $(TARGET_NAME)_libretro_ios.dylib
   fpic := -fPIC
   SHARED := -dynamiclib

ifeq ($(IOSSDK),)
   IOSSDK := $(shell xcodebuild -version -sdk iphoneos Path)
endif

   CC = clang -arch armv7 -isysroot $(IOSSDK)
   CXX = clang++ -arch armv7 -isysroot $(IOSSDK)
   CFLAGS += -DIOS
ifeq ($(platform),ios9)
   CC     +=  -miphoneos-version-min=8.0
   CXX    +=  -miphoneos-version-min=8.0
   CFLAGS +=  -miphoneos-version-min=8.0
else
   CC     +=  -miphoneos-version-min=5.0
   CXX    +=  -miphoneos-version-min=5.0
   CFLAGS +=  -miphoneos-version-min=5.0
endif
else ifeq ($(platform), theos_ios)
DEPLOYMENT_IOSVERSION = 5.0
TARGET = iphone:latest:$(DEPLOYMENT_IOSVERSION)
ARCHS = armv7 armv7s
TARGET_IPHONEOS_DEPLOYMENT_VERSION=$(DEPLOYMENT_IOSVERSION)
THEOS_BUILD_DIR := objs
include $(THEOS)/makefiles/common.mk

LIBRARY_NAME = $(TARGET_NAME)_libretro_ios
else ifeq ($(platform), qnx)
   TARGET := $(TARGET_NAME)_libretro_qnx.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=$(CORE_DIR)/libretro/link.T -Wl,-no-undefined
   CFLAGS += -D_GNU_SOURCE=1

   CC = qcc -Vgcc_ntoarmv7le
   CXX = QCC -Vgcc_ntoarmv7le_cpp
   AR = QCC -Vgcc_ntoarmv7le
	CFLAGS += -D__BLACKBERRY_QNX__ -marm -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=softfp -lcpp
else ifeq ($(platform), ps3)
   TARGET := $(TARGET_NAME)_libretro_ps3.a
   CC = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-gcc.exe
   CXX = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-g++.exe
   AR = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-ar.exe
   CFLAGS += -DMSB_FIRST=1 -DSDL_BYTEORDER=SDL_BIG_ENDIAN
	STATIC_LINKING = 1
else ifeq ($(platform), sncps3)
   TARGET := $(TARGET_NAME)_libretro_ps3.a
   CC = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   CXX = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   AR = $(CELL_SDK)/host-win32/sn/bin/ps3snarl.exe
   CFLAGS +=  -DMSB_FIRST=1 -DSDL_BYTEORDER=SDL_BIG_ENDIAN
	STATIC_LINKING = 1
else ifeq ($(platform), psl1ght)
   TARGET := $(TARGET_NAME)_libretro_psl1ght.a
   CC = $(PS3DEV)/ppu/bin/ppu-gcc$(EXE_EXT)
   CXX = $(PS3DEV)/ppu/bin/ppu-g++$(EXE_EXT)
   AR = $(PS3DEV)/ppu/bin/ppu-ar$(EXE_EXT)
   CFLAGS += -DMSB_FIRST=1 -DSDL_BYTEORDER=SDL_BIG_ENDIAN
	STATIC_LINKING = 1
else ifeq ($(platform), psp1)
   TARGET := $(TARGET_NAME)_libretro_psp1.a
   CC = psp-gcc$(EXE_EXT)
   CXX = psp-g++$(EXE_EXT)
   AR = psp-ar$(EXE_EXT)
   CFLAGS += -DGNU_SOURCE=1 -G0 -I$(shell psp-config --pspsdk-path)/include
   STATIC_LINKING = 1
   SINGLE_PRECISION_FLOATS = 1
   MIN_AUDIO_PROCESSING_PER_FRAME = 1

else ifeq ($(platform), vita)
   TARGET := $(TARGET_NAME)_libretro_vita.a
	CC = arm-vita-eabi-gcc$(EXE_EXT)
	CXX = arm-vita-eabi-g++$(EXE_EXT)
	AR = arm-vita-eabi-ar$(EXE_EXT)
   CFLAGS += -DGNU_SOURCE=1
   STATIC_LINKING = 1
   SINGLE_PRECISION_FLOATS = 1
   MIN_AUDIO_PROCESSING_PER_FRAME = 1
else ifeq ($(platform), ctr)
   TARGET := $(TARGET_NAME)_libretro_ctr.a
   CC = $(DEVKITARM)/bin/arm-none-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITARM)/bin/arm-none-eabi-g++$(EXE_EXT)
   AR = $(DEVKITARM)/bin/arm-none-eabi-ar$(EXE_EXT)
   CFLAGS += -DARM11 -D_3DS -DGNU_SOURCE=1
   #workaround the sdl sizeof(enum)=sizeof(int) assert
   CFLAGS += -D__NDS__
   CFLAGS += -march=armv6k -mtune=mpcore -mfloat-abi=hard
   CFLAGS += -mword-relocations
   CFLAGS += -fomit-frame-pointer -fstrict-aliasing -ffast-math
   CFLAGS += -fno-rtti -fno-exceptions -std=gnu++11
   STATIC_LINKING = 1
   SINGLE_PRECISION_FLOATS = 1
   MIN_AUDIO_PROCESSING_PER_FRAME = 1
else ifeq ($(platform), rpi1)
   TARGET := $(TARGET_NAME)_libretro.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=$(CORE_DIR)/libretro/link.T -Wl,-no-undefined
   CFLAGS += -DARM -DGNU_SOURCE=1
   CFLAGS += -marm -march=armv6j -mfpu=vfp -mfloat-abi=hard
   CFLAGS += -fomit-frame-pointer -fstrict-aliasing -ffast-math
   CFLAGS += -fno-rtti -fno-exceptions -std=gnu++11
   SINGLE_PRECISION_FLOATS = 1
   MIN_AUDIO_PROCESSING_PER_FRAME = 1
   HAVE_NEON = 1
else ifeq ($(platform), rpi2)
   TARGET := $(TARGET_NAME)_libretro.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=$(CORE_DIR)/libretro/link.T -Wl,-no-undefined
   CFLAGS += -DARM -DGNU_SOURCE=1
   CFLAGS += -marm -mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard
   CFLAGS += -fomit-frame-pointer -fstrict-aliasing -ffast-math
   CFLAGS += -fno-rtti -fno-exceptions -std=gnu++11
   SINGLE_PRECISION_FLOATS = 1
   MIN_AUDIO_PROCESSING_PER_FRAME = 1
   HAVE_NEON = 1
else ifeq ($(platform), rpi3)
   TARGET := $(TARGET_NAME)_libretro.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=$(CORE_DIR)/libretro/link.T -Wl,-no-undefined
   CFLAGS += -DARM -DGNU_SOURCE=1
   CFLAGS += -marm -mcpu=cortex-a53 -mfpu=neon-fp-armv8 -mfloat-abi=hard
   CFLAGS += -fomit-frame-pointer -fstrict-aliasing -ffast-math
   CFLAGS += -fno-rtti -fno-exceptions -std=gnu++11
   SINGLE_PRECISION_FLOATS = 1
   MIN_AUDIO_PROCESSING_PER_FRAME = 1
   HAVE_NEON = 1
else ifeq ($(platform), xenon)
   TARGET := $(TARGET_NAME)_libretro_xenon360.a
   CC = xenon-gcc$(EXE_EXT)
   CXX = xenon-g++$(EXE_EXT)
   AR = xenon-ar$(EXE_EXT)
   CFLAGS += -D__LIBXENON__ -D__ppc_ -DMSB_FIRST=1 -DSDL_BYTEORDER=SDL_BIG_ENDIAN
	STATIC_LINKING = 1
else ifeq ($(platform), ngc)
   TARGET := $(TARGET_NAME)_libretro_ngc.a
   CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
   CFLAGS += -DGEKKO -DHW_DOL -mrvl -mcpu=750 -meabi -mhard-float -DMSB_FIRST=1 -DSDL_BYTEORDER=SDL_BIG_ENDIAN
	STATIC_LINKING = 1
else ifeq ($(platform), wii)
   TARGET := $(TARGET_NAME)_libretro_wii.a
   CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
   CFLAGS += -DGEKKO -DHW_RVL -mrvl -mcpu=750 -meabi -mhard-float -DMSB_FIRST=1 -DSDL_BYTEORDER=SDL_BIG_ENDIAN
	STATIC_LINKING = 1
else ifneq (,$(findstring armv,$(platform)))
   TARGET := $(TARGET_NAME)_libretro.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=$(CORE_DIR)/libretro/link.T -Wl,-no-undefined
   CFLAGS += -D_GNU_SOURCE=1
ifneq (,$(findstring cortexa8,$(platform)))
   CFLAGS += -marm -mcpu=cortex-a8
else ifneq (,$(findstring cortexa9,$(platform)))
   CFLAGS += -marm -mcpu=cortex-a9
endif
   CFLAGS += -marm
ifneq (,$(findstring neon,$(platform)))
   CFLAGS += -mfpu=neon
   HAVE_NEON = 1
endif
ifneq (,$(findstring softfloat,$(platform)))
   CFLAGS += -mfloat-abi=softfp
else ifneq (,$(findstring hardfloat,$(platform)))
   CFLAGS += -mfloat-abi=hard
endif
   CFLAGS += -DARM
else
   TARGET := $(TARGET_NAME)_libretro.dll
   CC = gcc
   SHARED := -shared -static-libgcc -static-libstdc++ -s -Wl,--version-script=$(CORE_DIR)/libretro/link.T
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

ifeq ($(SINGLE_PRECISION_FLOATS), 1)
CFLAGS += -DSINGLE_PRECISION_FLOATS
endif

ifeq ($(MIN_AUDIO_PROCESSING_PER_FRAME), 1)
CFLAGS += -DMIN_AUDIO_PROCESSING_PER_FRAME
endif

ifeq ($(DEBUGLOG), 1)
CFLAGS += -DDEBUG_LOG=1
endif

include Makefile.common

OBJECTS := $(SOURCES_CXX:.cpp=.o) $(SOURCES_C:.c=.o)

DEFINES := -DHAVE_INTTYPES_H -D__LIBRETRO__ -DINLINE=inline -DFRONTEND_SUPPORTS_RGB565

ifeq ($(platform), sncps3)
WARNINGS_DEFINES =
CODE_DEFINES =
else
WARNINGS_DEFINES = -Wall -W -Wno-unused-parameter
CODE_DEFINES = -fomit-frame-pointer
endif

ifeq ($(platform), osx)
ifndef ($(NOUNIVERSAL))
   CFLAGS += $(ARCHFLAGS)
	CXXFLAGS += $(ARCHFLAGS)
   LFLAGS += $(ARCHFLAGS)
endif
endif

COMMON_DEFINES += $(CODE_DEFINES) $(WARNINGS_DEFINES) -DNDEBUG=1 $(fpic)

CFLAGS     += $(DEFINES) $(COMMON_DEFINES)

%.o: %.c
	$(CC) $(INCFLAGS) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(INCFLAGS) $(CFLAGS) -c -o $@ $<

ifeq ($(platform), theos_ios)
COMMON_FLAGS := -DIOS -DARM $(COMMON_DEFINES) $(INCFLAGS) -I$(THEOS_INCLUDE_PATH) -Wno-error
$(LIBRARY_NAME)_CFLAGS += $(CFLAGS) $(COMMON_FLAGS)
$(LIBRARY_NAME)_CXXFLAGS += $(CXXFLAGS) $(COMMON_FLAGS)
${LIBRARY_NAME}_FILES = $(SOURCES_CXX) $(SOURCES_C)
include $(THEOS_MAKE_PATH)/library.mk
else
all: $(TARGET)

$(TARGET): $(OBJECTS)
ifeq ($(STATIC_LINKING), 1)
	$(AR) rcs $@ $(OBJECTS)
else
	$(CXX) $(fpic) $(SHARED) $(INCFLAGS) $(CFLAGS) -o $@ $(OBJECTS) -lm
endif


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
endif

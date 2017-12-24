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
LIBS         :=
ifeq (,$(findstring msvc,$(platform)))
LIBS         += -lm
endif


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
   CFLAGS += -DMSB_FIRST=1 
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

ifeq ($(platform),ios-arm64)
	 CC = clang -arch armv64-isysroot $(IOSSDK)
	 CXX = clang++ -arch arm64 -isysroot $(IOSSDK)
else
	 CC = clang -arch armv7 -isysroot $(IOSSDK)
   CXX = clang++ -arch armv7 -isysroot $(IOSSDK)
endif
CFLAGS += -DIOS
ifeq ($(platform),$(filter $(platform),ios9 ios-arm64))
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
   TARGET := $(TARGET_NAME)_libretro_$(platform).a
   CC = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-gcc.exe
   CXX = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-g++.exe
   AR = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-ar.exe
   CFLAGS += -DMSB_FIRST=1
	STATIC_LINKING = 1
else ifeq ($(platform), sncps3)
   TARGET := $(TARGET_NAME)_libretro_ps3.a
   CC = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   CXX = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   AR = $(CELL_SDK)/host-win32/sn/bin/ps3snarl.exe
   CFLAGS +=  -DMSB_FIRST=1
	STATIC_LINKING = 1
else ifeq ($(platform), psl1ght)
   TARGET := $(TARGET_NAME)_libretro_$(platform).a
   CC = $(PS3DEV)/ppu/bin/ppu-gcc$(EXE_EXT)
   CXX = $(PS3DEV)/ppu/bin/ppu-g++$(EXE_EXT)
   AR = $(PS3DEV)/ppu/bin/ppu-ar$(EXE_EXT)
   CFLAGS += -DMSB_FIRST=1
	STATIC_LINKING = 1
else ifeq ($(platform), psp1)
   TARGET := $(TARGET_NAME)_libretro_$(platform).a
   CC = psp-gcc$(EXE_EXT)
   CXX = psp-g++$(EXE_EXT)
   AR = psp-ar$(EXE_EXT)
   CFLAGS += -DGNU_SOURCE=1 -G0 -I$(shell psp-config --pspsdk-path)/include
   STATIC_LINKING = 1
   SINGLE_PRECISION_FLOATS = 1
   MIN_AUDIO_PROCESSING_PER_FRAME = 1

else ifeq ($(platform), vita)
   TARGET := $(TARGET_NAME)_libretro_$(platform).a
	CC = arm-vita-eabi-gcc$(EXE_EXT)
	CXX = arm-vita-eabi-g++$(EXE_EXT)
	AR = arm-vita-eabi-ar$(EXE_EXT)
   CFLAGS += -DGNU_SOURCE=1
   STATIC_LINKING = 1
   SINGLE_PRECISION_FLOATS = 1
   MIN_AUDIO_PROCESSING_PER_FRAME = 1
else ifeq ($(platform), ctr)
   TARGET := $(TARGET_NAME)_libretro_$(platform).a
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
   CFLAGS += -D__LIBXENON__ -D__ppc_ -DMSB_FIRST=1
	STATIC_LINKING = 1
else ifeq ($(platform), ngc)
   TARGET := $(TARGET_NAME)_libretro_$(platform).a
   CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
   CFLAGS += -DGEKKO -DHW_DOL -mrvl -mcpu=750 -meabi -mhard-float -DMSB_FIRST=1
   CFLAGS += -U__INT32_TYPE__ -U __UINT32_TYPE__ -D__INT32_TYPE__=int
	STATIC_LINKING = 1

else ifeq ($(platform), wii)
   TARGET := $(TARGET_NAME)_libretro_$(platform).a
   CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
   CFLAGS += -DGEKKO -DHW_RVL -mrvl -mcpu=750 -meabi -mhard-float -DMSB_FIRST=1
   CFLAGS += -U__INT32_TYPE__ -U __UINT32_TYPE__ -D__INT32_TYPE__=int
	STATIC_LINKING = 1

else ifeq ($(platform), wiiu)
   TARGET := $(TARGET_NAME)_libretro_$(platform).a
   CC = $(DEVKITPPC)/bin/powerpc-eabi-gcc$(EXE_EXT)
   CXX = $(DEVKITPPC)/bin/powerpc-eabi-g++$(EXE_EXT)
   AR = $(DEVKITPPC)/bin/powerpc-eabi-ar$(EXE_EXT)
   CFLAGS += -DGEKKO -DWIIU -DHW_RVL -mwup -mcpu=750 -meabi -mhard-float -DMSB_FIRST=1
   CFLAGS += -U__INT32_TYPE__ -U __UINT32_TYPE__ -D__INT32_TYPE__=int
	STATIC_LINKING = 1

else ifeq ($(platform),emscripten)
   TARGET := $(TARGET_NAME)_libretro_$(platform).bc
	STATIC_LINKING = 1
   SINGLE_PRECISION_FLOATS = 1
   MIN_AUDIO_PROCESSING_PER_FRAME = 1

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
# Windows MSVC 2003 Xbox 1
else ifeq ($(platform), xbox1_msvc2003)
TARGET := $(TARGET_NAME)_libretro_xdk1.lib
MSVCBINDIRPREFIX = $(XDK)/xbox/bin/vc71
CC  = "$(MSVCBINDIRPREFIX)/CL.exe"
CXX  = "$(MSVCBINDIRPREFIX)/CL.exe"
LD   = "$(MSVCBINDIRPREFIX)/lib.exe"

export INCLUDE := $(XDK)/xbox/include
export LIB := $(XDK)/xbox/lib
PSS_STYLE :=2
CFLAGS   += -D_XBOX -D_XBOX1
CXXFLAGS += -D_XBOX -D_XBOX1
STATIC_LINKING=1
HAS_GCC := 0
# Windows MSVC 2010 Xbox 360
else ifeq ($(platform), xbox360_msvc2010)
TARGET := $(TARGET_NAME)_libretro_xdk360.lib
MSVCBINDIRPREFIX = $(XEDK)/bin/win32
CC  = "$(MSVCBINDIRPREFIX)/cl.exe"
CXX  = "$(MSVCBINDIRPREFIX)/cl.exe"
LD   = "$(MSVCBINDIRPREFIX)/lib.exe"

export INCLUDE := $(XEDK)/include/xbox
export LIB := $(XEDK)/lib/xbox
PSS_STYLE :=2
CFLAGS   += -D_XBOX -D_XBOX360
CXXFLAGS += -D_XBOX -D_XBOX360
STATIC_LINKING=1
HAS_GCC := 0

# Windows MSVC 2010 x64
else ifeq ($(platform), windows_msvc2010_x64)
	LIBS=
	CC  = cl.exe
	CXX = cl.exe

PATH := $(shell IFS=$$'\n'; cygpath "$(VS100COMNTOOLS)../../VC/bin/amd64"):$(PATH)
PATH := $(PATH):$(shell IFS=$$'\n'; cygpath "$(VS100COMNTOOLS)../IDE")
LIB := $(shell IFS=$$'\n'; cygpath "$(VS100COMNTOOLS)../../VC/lib/amd64")
INCLUDE := $(shell IFS=$$'\n'; cygpath "$(VS100COMNTOOLS)../../VC/include")

WindowsSdkDir := $(shell reg query "HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v7.0A" -v "InstallationFolder" | grep -o '[A-Z]:\\.*')lib/x64
WindowsSdkDir ?= $(shell reg query "HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v7.1A" -v "InstallationFolder" | grep -o '[A-Z]:\\.*')lib/x64

WindowsSdkDirInc := $(shell reg query "HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v7.0A" -v "InstallationFolder" | grep -o '[A-Z]:\\.*')Include
WindowsSdkDirInc ?= $(shell reg query "HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v7.1A" -v "InstallationFolder" | grep -o '[A-Z]:\\.*')Include


INCFLAGS_PLATFORM = -I"$(WindowsSdkDirInc)"
export INCLUDE := $(INCLUDE)
export LIB := $(LIB);$(WindowsSdkDir)
TARGET := $(TARGET_NAME)_libretro.dll
PSS_STYLE :=2
LDFLAGS += -DLL
LIBS =
# Windows MSVC 2010 x86
else ifeq ($(platform), windows_msvc2010_x86)
	LIBS=
	CC  = cl.exe
	CXX = cl.exe

PATH := $(shell IFS=$$'\n'; cygpath "$(VS100COMNTOOLS)../../VC/bin"):$(PATH)
PATH := $(PATH):$(shell IFS=$$'\n'; cygpath "$(VS100COMNTOOLS)../IDE")
LIB := $(shell IFS=$$'\n'; cygpath -w "$(VS100COMNTOOLS)../../VC/lib")
INCLUDE := $(shell IFS=$$'\n'; cygpath "$(VS100COMNTOOLS)../../VC/include")

WindowsSdkDir := $(shell reg query "HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v7.0A" -v "InstallationFolder" | grep -o '[A-Z]:\\.*')lib
WindowsSdkDir ?= $(shell reg query "HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v7.1A" -v "InstallationFolder" | grep -o '[A-Z]:\\.*')lib

WindowsSdkDirInc := $(shell reg query "HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v7.0A" -v "InstallationFolder" | grep -o '[A-Z]:\\.*')Include
WindowsSdkDirInc ?= $(shell reg query "HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v7.1A" -v "InstallationFolder" | grep -o '[A-Z]:\\.*')Include


INCFLAGS_PLATFORM = -I"$(WindowsSdkDirInc)"
export INCLUDE := $(INCLUDE)
export LIB := $(LIB);$(WindowsSdkDir)
TARGET := $(TARGET_NAME)_libretro.dll
PSS_STYLE :=2
LDFLAGS += -DLL
LIBS =

# Windows MSVC 2005 x86
else ifeq ($(platform), windows_msvc2005_x86)
	CC  = cl.exe
	CXX = cl.exe
	HAS_GCC := 0

PATH := $(shell IFS=$$'\n'; cygpath "$(VS80COMNTOOLS)../../VC/bin"):$(PATH)
PATH := $(PATH):$(shell IFS=$$'\n'; cygpath "$(VS80COMNTOOLS)../IDE")
INCLUDE := $(shell IFS=$$'\n'; cygpath "$(VS80COMNTOOLS)../../VC/include")
LIB := $(shell IFS=$$'\n'; cygpath -w "$(VS80COMNTOOLS)../../VC/lib")
BIN := $(shell IFS=$$'\n'; cygpath "$(VS80COMNTOOLS)../../VC/bin")

WindowsSdkDir := $(INETSDK)

export INCLUDE := $(INCLUDE);$(INETSDK)/Include;nxengine/libretro/libretro-common/include/compat/msvc
export LIB := $(LIB);$(WindowsSdkDir);$(INETSDK)/Lib
TARGET := $(TARGET_NAME)_libretro.dll
PSS_STYLE :=2
LDFLAGS += -DLL
CFLAGS += -D_CRT_SECURE_NO_DEPRECATE
LIBS =

# Windows MSVC 2003 x86
else ifeq ($(platform), windows_msvc2003_x86)
	CC  = cl.exe
	CXX = cl.exe

PATH := $(shell IFS=$$'\n'; cygpath "$(VS71COMNTOOLS)../../Vc7/bin"):$(PATH)
PATH := $(PATH):$(shell IFS=$$'\n'; cygpath "$(VS71COMNTOOLS)../IDE")
INCLUDE := $(shell IFS=$$'\n'; cygpath "$(VS71COMNTOOLS)../../Vc7/include")
LIB := $(shell IFS=$$'\n'; cygpath -w "$(VS71COMNTOOLS)../../Vc7/lib")
BIN := $(shell IFS=$$'\n'; cygpath "$(VS71COMNTOOLS)../../Vc7/bin")

WindowsSdkDir := $(INETSDK)

export INCLUDE := $(INCLUDE);$(INETSDK)/Include;src/drivers/libretro/msvc/msvc-2005
export LIB := $(LIB);$(WindowsSdkDir);$(INETSDK)/Lib
TARGET := $(TARGET_NAME)_libretro.dll
PSS_STYLE :=2
LDFLAGS += -DLL
CFLAGS += -D_CRT_SECURE_NO_DEPRECATE

# Windows
else
   TARGET := $(TARGET_NAME)_libretro.dll
   CC = gcc
   SHARED := -shared -static-libgcc -static-libstdc++ -s -Wl,--version-script=$(CORE_DIR)/libretro/link.T
   CFLAGS += -D__WIN32__ -D__WIN32_LIBRETRO__ -Wno-missing-field-initializers
endif

ifeq ($(DEBUG), 1)
ifneq (,$(findstring msvc,$(platform)))
	ifeq ($(STATIC_LINKING),1)
	CFLAGS += -MTd
	CXXFLAGS += -MTd
else
	CFLAGS += -MDd
	CXXFLAGS += -MDd
endif

CFLAGS += -Od -Zi -DDEBUG -D_DEBUG
CXXFLAGS += -Od -Zi -DDEBUG -D_DEBUG
	else
	CFLAGS += -O0 -g -DDEBUG
	CXXFLAGS += -O0 -g -DDEBUG
endif
else
ifneq (,$(findstring msvc,$(platform)))
ifeq ($(STATIC_LINKING),1)
	CFLAGS += -MT
	CXXFLAGS += -MT
else
	CFLAGS += -MD
	CXXFLAGS += -MD
endif

CFLAGS += -O2 -DNDEBUG
CXXFLAGS += -O2 -DNDEBUG
else
	CFLAGS += -O2 -DNDEBUG
	CXXFLAGS += -O2 -DNDEBUG
endif
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

DEFINES := -DHAVE_INTTYPES_H -D__LIBRETRO__ -DFRONTEND_SUPPORTS_RGB565

ifneq (,$(findstring msvc,$(platform)))
DEFINES += -DINLINE="_inline"
else
DEFINES += -DINLINE="inline"
endif

ifeq ($(platform), sncps3)
WARNINGS_DEFINES =
CODE_DEFINES =
else ifneq (,$(findstring msvc,$(platform)))
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

COMMON_DEFINES += $(CODE_DEFINES) $(WARNINGS_DEFINES) $(fpic) $(INCFLAGS) $(INCFLAGS_PLATFORM)

CFLAGS     += $(DEFINES) $(COMMON_DEFINES)

OBJOUT   = -o
LINKOUT  = -o 

ifneq (,$(findstring msvc,$(platform)))
	OBJOUT = -Fo
	LINKOUT = -out:
ifeq ($(STATIC_LINKING),1)
	LD ?= lib.exe
	STATIC_LINKING=0
else
	LD = link.exe
endif
else
	LD = $(CXX)
endif

%.o: %.c
	$(CC) $(CFLAGS) -c $(OBJOUT)$@ $<

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $(OBJOUT)$@ $<

ifeq ($(platform), theos_ios)
COMMON_FLAGS := -DIOS -DARM $(COMMON_DEFINES) -I$(THEOS_INCLUDE_PATH) -Wno-error
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
	$(LD) $(fpic) $(SHARED) $(LDFLAGS) $(LINKOUT)$@ $(OBJECTS) $(LIBS)
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

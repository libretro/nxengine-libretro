
ifeq ($(platform),)
platform = unix
ifeq ($(shell uname -a),)
   platform = win
else ifneq ($(findstring MINGW,$(shell uname -a)),)
   platform = win
else ifneq ($(findstring Darwin,$(shell uname -a)),)
   platform = osx
endif
endif

ifeq ($(platform), unix)
   TARGET := libretro.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=libretro/link.T -Wl,--no-undefined
else ifeq ($(platform), osx)
   TARGET := libretro.dylib
   fpic := -fPIC
   SHARED := -dynamiclib
else ifeq ($(platform), ps3)
   TARGET := libretro_ps3.a
   CC = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-gcc.exe
   CXX = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-g++.exe
   AR = $(CELL_SDK)/host-win32/ppu/bin/ppu-lv2-ar.exe
   CFLAGS += -D__ppc__
else ifeq ($(platform), sncps3)
   TARGET := libretro_ps3.a
   CC = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   CXX = $(CELL_SDK)/host-win32/sn/bin/ps3ppusnc.exe
   AR = $(CELL_SDK)/host-win32/sn/bin/ps3snarl.exe
   CFLAGS += -D__ppc__
else ifeq ($(platform), xenon)
   TARGET := libretro_xenon360.a
   SHARED := -shared -Wl,--version-script=libretro/link.T -Wl,--no-undefined
   CC = xenon-gcc
   CXX = xenon-g++
   AR = xenon-ar
   ENDIANNESS_DEFINES = 
   CFLAGS += -D__LIBXENON__
   CXXFLAGS += -D__LIBXENON__
else
   TARGET := libretro.dll
   CC = gcc
   CXX = g++
   SHARED := -shared -static-libgcc -static-libstdc++ -Wl,--version-script=libretro/link.T -Wl,--no-undefined
endif

RM       = rm -f
JUNK    := $(shell find . -name '*~')
SRCS    := $(shell find . -name '*.cpp')
SRCS_C  := $(shell find . -name '*.c')
OBJS    := $(patsubst %.cpp,%.o,$(SRCS)) $(patsubst %.c,%.o,$(SRCS_C))
TARGETS := libretro.so

# Add SDL dependency
DEFINES += -D__LIBRETRO__
INCDIRS := -Isdl/include
CXXFLAGS += $(SDL_CFLAGS) $(DEFINES) $(INCDIRS) -O3 -Wreturn-type -Wunused-variable -Wno-multichar -Wl,--no-undefined $(fpic)
CFLAGS += $(SDL_CFLAGS) $(DEFINES) $(INCDIRS) -O3 -Wreturn-type -Wunused-variable -Wno-multichar -Wl,--no-undefined $(fpic)
LDFLAGS += -lm $(SDL_LIBS) $(DEFINES) $(SHARED)

all: $(TARGET)

$(TARGET): $(OBJS)
ifeq ($(platform), ps3)
	$(AR) rcs $@ $(OBJS)
else ifeq ($(platform), sncps3)
	$(AR) rcs $@ $(OBJS)
else ifeq ($(platform), xenon)
	$(AR) rcs $@ $(OBJS)
else ifeq ($(platform), wii)
	$(AR) rcs $@ $(OBJS)
else
	$(CXX) $(OBJS) $(LDFLAGS) -o $@
endif

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(TARGETS) $(OBJS) $(JUNK)

.PHONY: clean all


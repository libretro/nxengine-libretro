
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
else ifeq ($(platform), xenon)
   TARGET := libretro.a
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
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(TARGETS) $(OBJS) $(JUNK)

.PHONY: clean all


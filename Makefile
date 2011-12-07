
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
   TARGET := nx.so
   fpic := -fPIC
   SHARED := -shared -Wl,--version-script=link.T -Wl,--no-undefined
else ifeq ($(platform), osx)
   TARGET := nx.dylib
   fpic := -fPIC
   SHARED := -dynamiclib
else ifeq ($(platform), xenon)
   TARGET := libsnes.a
   SHARED := -shared -Wl,--version-script=link.T -Wl,--no-undefined
   CC = xenon-gcc
   CXX = xenon-g++
   AR = xenon-ar
   ENDIANNESS_DEFINES = 
   CFLAGS += -D__LIBXENON__
   CXXFLAGS += -D__LIBXENON__
else
   TARGET := nx.dll
   CC = gcc
   CXX = g++
   SHARED := -shared -static-libgcc -static-libstdc++ -Wl,--version-script=link.T -Wl,--no-undefined
endif

RM       = rm -f
JUNK    := $(shell find . -name '*~')
SRCS    := $(shell find . -name '*.cpp')
OBJS    := $(patsubst %.cpp,%.o,$(SRCS))
TARGETS := nx.so

ifeq ($(platform), win)
   SDL_CFLAGS := -ISDL
   SDL_LIBS := -L. -lSDL
else ifeq($(platform), xenon)
   SDL_CFLAGS := -I$(DEVKITXENON)/usr/include -I$(DEVKITXENON)/usr/include/SDL
   SDL_LIBS := -L. -L$(DEVKITXENON)/usr/lib -lSDL
else
   SDL_CFLAGS := $(shell pkg-config sdl --cflags)
   SDL_LIBS := $(shell pkg-config sdl --libs)
endif

# Add SDL dependency
DEFINES += -D__LIBSNES__
CXXFLAGS += $(SDL_CFLAGS) $(DEFINES) -O3 -Wreturn-type -Wunused-variable -Wno-multichar $(fpic)
LDFLAGS += -lm $(SDL_LIBS) $(DEFINES) $(SHARED)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	$(RM) $(TARGETS) $(OBJS) $(JUNK)

.PHONY: clean all


PRJ      = drawingTest
BINDIR   = bin
PREFIX   = /mnt/utmp/$(PRJ)
RM       = rm -f
STRIP    = $(CXX:%g++=%strip)
JUNK    := $(shell find . -name '*~')
SRCS    := $(shell find . -name '*.cpp')
OBJS    := $(patsubst %.cpp,%.o,$(SRCS))
TARGETS := nx

# Add SDL dependency
CFLAGS  =$(shell sdl-config --cflags) -O2 -Wreturn-type -Wunused-variable -Wno-multichar -DPANDORA
LDFLAGS +=-lSDL_gfx -lSDL_ttf -lSDL_image -lSDL_mixer -lstdc++ -lm  $(shell sdl-config --libs)

all: $(TARGETS)

nx: $(OBJS) $(SRCS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(TARGETS) $(OBJS) $(JUNK)

install: all
	mkdir -p $(PREFIX)/bin
	cp $(TARGETS) $(PREFIX)/bin
	cp -rpf data $(PREFIX)


.PHONY:clean all install

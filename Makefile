PRJ      = drawingTest
BINDIR   = bin
PREFIX   = /mnt/utmp/$(PRJ)
RM       = rm -f
STRIP    = $(CXX:%g++=%strip)
JUNK    := $(shell find . -name '*~')
SRCS    := $(shell find . -name '*.cpp')
OBJS    := $(patsubst %.cpp,%.o,$(SRCS))
TARGETS := nx.so

# Add SDL dependency
CFLAGS  = $(shell pkg-config sdl --cflags) -O0 -g -Wreturn-type -Wunused-variable -Wno-multichar -fPIC
LDFLAGS += -lSDL_ttf -lm  $(shell pkg-config sdl --libs) -shared -Wl,--no-undefined

all: $(TARGETS)

nx.so: $(OBJS) $(SRCS)
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

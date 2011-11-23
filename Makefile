RM       = rm -f
JUNK    := $(shell find . -name '*~')
SRCS    := $(shell find . -name '*.cpp')
OBJS    := $(patsubst %.cpp,%.o,$(SRCS))
TARGETS := nx.so

# Add SDL dependency
CFLAGS  = $(shell pkg-config sdl --cflags) -O3 -Wreturn-type -Wunused-variable -Wno-multichar -fPIC
LDFLAGS += -lm  $(shell pkg-config sdl --libs) -shared -Wl,--no-undefined

all: $(TARGETS)

nx.so: $(OBJS) $(SRCS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(TARGETS) $(OBJS) $(JUNK)

.PHONY: clean all


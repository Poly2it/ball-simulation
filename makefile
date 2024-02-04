.POSIX:

# How-to:
# - When adding CFLAGS, prepend them in the CFLAGS_I variable. That way they can 
# be overwritten by the user with the CFLAGS variable. The last values have 
# priority.

SHELL = sh
PLATFORM_OS = $(shell uname -o)

ifeq      ($(PLATFORM_OS),Linux)
	EXTERNAL_SHARE = $(shell source scripts/lib_linux.sh && path_local_share_get)
else ifeq ($(PLATFORM_OS),MS/Windows)
	EXTERNAL_SHARE = $(shell source scripts/lib_w64devkit.sh && path_local_share_get)
endif

PREFIX     = /usr/local
BUILD      = ./.build
DEPENDENCY = ./scripts/dependencies.sh
SOURCE     = ./src

# release | debug
BUILD_TYPE = debug

CC       = cc
CFLAGS_I = -L$(PREFIX)/include -L$(EXTERNAL_SHARE)/include -W -O
LDLIBS_I = -L$(PREFIX)/lib -L$(EXTERNAL_SHARE)/lib -lraylib -lm

ifeq ($(PLATFORM_OS),MS/Windows)
	LDLIBS_I += -lopengl32 -lgdi32 -lwinmm
	ifeq ($(BUILD_TYPE),release)
		LDLIBS_I += -mwindows
	endif
endif

ifeq      ($(BUILD_TYPE),release)
	OPTIMIZATION_LEVEL = O3
else ifeq ($(BUILD_TYPE),debug)
	OPTIMIZATION_LEVEL = Og
endif

CFLAGS_I += $(CFLAGS) -$(OPTIMIZATION_LEVEL)
LDLIBS_I += $(LDLIBS)


all: game
install: game
	mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	cp -f game "$(DESTDIR)$(PREFIX)/bin"
uninstall: game
	rm "$(DESTDIR)$(PREFIX)/bin/game"
clean:
	rm -f "$(BUILD)/"*
	touch "$(BUILD)/".gitkeep


game: $(BUILD)/main.o $(BUILD)/xoshiro128plus.o $(BUILD)/vector.o raylib
	$(CC) $(LDFLAGS) \
		-o game \
		$(BUILD)/main.o \
		$(BUILD)/xoshiro128plus.o \
		$(BUILD)/vector.o \
		$(LDLIBS_I)

$(BUILD)/main.o: $(SOURCE)/main.c
	$(CC) \
		-c $(CFLAGS_I) \
		-o $(BUILD)/main.o \
		$(SOURCE)/main.c

$(BUILD)/xoshiro128plus.o: $(SOURCE)/algorithms/xoshiro128plus.c
	$(CC) \
		-c $(CFLAGS_I) \
		-o $(BUILD)/xoshiro128plus.o \
		$(SOURCE)/algorithms/xoshiro128plus.c

$(BUILD)/vector.o: $(SOURCE)/vector.c
	$(CC) \
		-c $(CFLAGS_I) \
		-o $(BUILD)/vector.o \
		$(SOURCE)/vector.c


raylib:
	"$(DEPENDENCY)" raylib all

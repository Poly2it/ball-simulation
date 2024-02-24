.POSIX:

# How-to:
# - When adding CFLAGS, prepend them in the CFLAGS_I variable. That way they can 
# be overwritten by the user with the CFLAGS variable. The last values have 
# priority.

SHELL = sh
PLATFORM_OS = $(shell uname -o)

ifeq ($(PLATFORM_OS),Linux)
	EXTERNAL_SHARE = $(shell . scripts/lib_linux.sh && path_local_share_get)
else
ifeq ($(PLATFORM_OS),MS/Windows)
	EXTERNAL_SHARE = $(shell . scripts/lib_w64devkit.sh && path_local_share_get)
else
ifeq ($(PLATFORM_OS),Darwin)
	EXTERNAL_SHARE = $(shell . scripts/lib_darwin.sh && path_local_share_get)
endif
endif
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
        # This must be included in the linking stage, or a debug console window
        # will appear with the application. Welcome to Windows.
		LDLIBS_I += -mwindows
	endif
endif

ifeq ($(PLATFORM_OS),Darwin)
	LDLIBS_I += -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL
endif

ifeq ($(BUILD_TYPE),release)
	OPTIMIZATION_LEVEL = O3
else
ifeq ($(BUILD_TYPE),debug)
	OPTIMIZATION_LEVEL = Og
endif
endif

CFLAGS_I += $(CFLAGS) -$(OPTIMIZATION_LEVEL)
LDLIBS_I += $(LDLIBS)


.PHONY: all install uninstall clean


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
    ifeq ($(PLATFORM_OS),Darwin)
		@echo "Building for Darwin. Make sure to run"
		@echo "# export MACOSX_DEPLOYMENT_TARGET=10.9 && xcode-select --install"
		@echo "to prepare the system libraries for compilation of this raylib-based project."
		@echo "See the the guide at https://github.com/raysan5/raylib/wiki/Working-on-macOS for more information."
    endif
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


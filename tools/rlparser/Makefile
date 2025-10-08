EXTENSION?=txt
FORMAT?=DEFAULT
.PHONY: all parse clean raylib_api

# Determine PLATFORM_OS
# No uname.exe on MinGW!, but OS=Windows_NT on Windows!
# ifeq ($(UNAME),Msys) -> Windows
ifeq ($(OS),Windows_NT)
    PLATFORM_OS = WINDOWS
else
    UNAMEOS = $(shell uname)
    ifeq ($(UNAMEOS),Linux)
        PLATFORM_OS = LINUX
    endif
    ifeq ($(UNAMEOS),FreeBSD)
        PLATFORM_OS = BSD
    endif
    ifeq ($(UNAMEOS),OpenBSD)
        PLATFORM_OS = BSD
    endif
    ifeq ($(UNAMEOS),NetBSD)
        PLATFORM_OS = BSD
    endif
    ifeq ($(UNAMEOS),DragonFly)
        PLATFORM_OS = BSD
    endif
    ifeq ($(UNAMEOS),Darwin)
        PLATFORM_OS = OSX
    endif
endif

# Define default C compiler: CC
#------------------------------------------------------------------------------------------------
CC = gcc
ifeq ($(PLATFORM_OS),OSX)
    # OSX default compiler
    CC = clang
endif
ifeq ($(PLATFORM_OS),BSD)
    # FreeBSD, OpenBSD, NetBSD, DragonFly default compiler
    CC = clang
endif

# Define default make program: MAKE
#------------------------------------------------------------------------------------------------
MAKE ?= make
ifeq ($(PLATFORM_OS),WINDOWS)
    MAKE = mingw32-make
endif

# Define compiler flags: CFLAGS
#------------------------------------------------------------------------------------------------
CFLAGS = -Wall -std=c99
#CFLAGS += -Wextra -Wmissing-prototypes -Wstrict-prototypes

ifeq ($(BUILD_MODE),DEBUG)
    CFLAGS += -g -D_DEBUG
else
    ifeq ($(PLATFORM_OS),OSX)
        CFLAGS += -O2
    else
        CFLAGS += -s -O2
    endif
endif
ifeq ($(PLATFORM_OS),WINDOWS)
    # NOTE: The resource .rc file contains windows executable icon and properties
    CFLAGS += rlparser.rc.data
endif

# Define processes to execute
#------------------------------------------------------------------------------------------------
# rlparser compilation
rlparser: rlparser.c
	$(CC) rlparser.c -o rlparser $(CFLAGS)

# rlparser execution: [raylib.h] parse, generating some output files
raylib_api: ../../src/raylib.h rlparser
	FORMAT=DEFAULT EXTENSION=txt $(MAKE) raylib_api.txt
	FORMAT=JSON EXTENSION=json $(MAKE) raylib_api.json
	FORMAT=XML EXTENSION=xml $(MAKE) raylib_api.xml
	FORMAT=LUA EXTENSION=lua $(MAKE) raylib_api.lua

# rlparser execution: [raylib.h] parse, generating some output files
raylib_api.$(EXTENSION): ../../src/raylib.h rlparser
	./rlparser -i ../../src/raylib.h -o raylib_api.$(EXTENSION) -f $(FORMAT) -d RLAPI

# rlparser execution: [rlgl.h] parse, generating some output files
rlgl_api.$(EXTENSION): ../../src/rlgl.h rlparser
	./rlparser -i ../../src/rlgl.h -o rlgl_api.$(EXTENSION) -f $(FORMAT) -d RLAPI -t "RLGL IMPLEMENTATION"

# rlparser execution: [raymath.h] parse, generating some output files
raymath_api.$(EXTENSION): ../../src/raymath.h rlparser
	./rlparser -i ../../src/raymath.h -o raymath_api.$(EXTENSION) -f $(FORMAT) -d RMAPI

# rlparser execution: [reasings.h] parse, generating some output files
reasings_api.$(EXTENSION): ../../examples/others/reasings.h rlparser
	./rlparser -i ../../examples/others/reasings.h -o reasings_api.$(EXTENSION) -f $(FORMAT) -d EASEDEF

# rlparser execution: [raygui.h] parse, generating some output files
raygui_api.$(EXTENSION): ../raygui.h rlparser
	./rlparser -i ../raygui.h -o raygui_api.$(EXTENSION) -f $(FORMAT) -d RAYGUIAPI -t "RAYGUI IMPLEMENTATION"

# Target to generate required APIs output files
parse: raylib_api.$(EXTENSION) raymath_api.$(EXTENSION) rlgl_api.$(EXTENSION) raygui_api.$(EXTENSION)

# "make parse" (and therefore "make all") requires
# raygui.h and reasings_api.h to exist in the correct directory
# API files for individual headers can be created likeso, provided the relevant header exists:
# FORMAT=JSON EXTENSION=json make raygui_api.json
all: rlparser
	FORMAT=DEFAULT EXTENSION=txt $(MAKE) parse
	FORMAT=JSON EXTENSION=json $(MAKE) parse
	FORMAT=XML EXTENSION=xml $(MAKE) parse
	FORMAT=LUA EXTENSION=lua $(MAKE) parse

# Clean rlparser and generated output files 
clean:
	rm -f rlparser *.json *.txt *.xml *.lua

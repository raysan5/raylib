#******************************************************************************
#
#  raylib makefile for desktop platforms, Raspberry Pi and HTML5 (emscripten)
#
#  Copyright (c) 2014 Ramon Santamaria (@raysan5)
#
#  This software is provided "as-is", without any express or implied warranty.
#  In no event will the authors be held liable for any damages arising from
#  the use of this software.
#
#  Permission is granted to anyone to use this software for any purpose,
#  including commercial applications, and to alter it and redistribute it
#  freely, subject to the following restrictions:
#
#    1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software in a
#    product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
#
#    2. Altered source versions must be plainly marked as such, and must not
#    be misrepresented as being the original software.
#
#    3. This notice may not be removed or altered from any source distribution.
#
#******************************************************************************

# Please read the wiki to know how to compile raylib, because there are
# different methods.

.PHONY: all clean install unistall

# define raylib platform to compile for
# possible platforms: PLATFORM_DESKTOP PLATFORM_RPI PLATFORM_WEB
PLATFORM ?= PLATFORM_DESKTOP

# define if you want shared or static version of library.
SHARED ?= NO

# determine if the file has root access (only for installing raylib)
# "whoami" prints the name of the user that calls him (so, if it is the root
# user, "whoami" prints "root").
ROOT = $(shell whoami)

# determine PLATFORM_OS in case PLATFORM_DESKTOP selected
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    # No uname.exe on MinGW!, but OS=Windows_NT on Windows!
    # ifeq ($(UNAME),Msys) -> Windows
    ifeq ($(OS),Windows_NT)
        PLATFORM_OS=WINDOWS
    else
        UNAMEOS:=$(shell uname)
        ifeq ($(UNAMEOS),Linux)
            PLATFORM_OS=LINUX
        else
        ifeq ($(UNAMEOS),Darwin)
            PLATFORM_OS=OSX
        endif
        endif
    endif
endif

# define raylib graphics api depending on selected platform
ifeq ($(PLATFORM),PLATFORM_RPI)
    # define raylib graphics api to use (on RPI, OpenGL ES 2.0 must be used)
    GRAPHICS = GRAPHICS_API_OPENGL_ES2
else
    # define raylib graphics api to use (OpenGL 3.3 by default)
    GRAPHICS ?= GRAPHICS_API_OPENGL_33
    #GRAPHICS = GRAPHICS_API_OPENGL_11  # Uncomment to use OpenGL 1.1
    #GRAPHICS = GRAPHICS_API_OPENGL_21  # Uncomment to use OpenGL 2.1
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    GRAPHICS = GRAPHICS_API_OPENGL_ES2
endif

# NOTE: makefiles targets require tab indentation

# define compiler: gcc for C program, define as g++ for C++
ifeq ($(PLATFORM),PLATFORM_WEB)
    CC = emcc # emscripten compiler
else
    CC = gcc # default gcc compiler
endif

# define compiler flags:
#  -O1                  defines optimization level
#  -Wall                turns on most, but not all, compiler warnings
#  -std=c99             defines C language mode (standard C from 1999 revision)
#  -std=gnu99           defines C language mode (GNU C from 1999 revision)
#  -fgnu89-inline       declaring inline functions support (GCC optimized)
#  -Wno-missing-braces  ignore invalid warning (GCC bug 53119)
CFLAGS = -O1 -Wall -std=gnu99 -fgnu89-inline -Wno-missing-braces
ifeq ($(SHARED),YES)
    CFLAGS += -fPIC
endif

#CFLAGSEXTRA = -Wextra -Wmissing-prototypes -Wstrict-prototypes

# define any directories containing required header files
ifeq ($(PLATFORM),PLATFORM_RPI)
    INCLUDES = -I. -Iexternal -I/opt/vc/include \
    -I/opt/vc/include/interface/vmcs_host/linux \
    -I/opt/vc/include/interface/vcos/pthreads
else
# STB libraries and others
    INCLUDES = -I. -Iexternal
# GLFW3 library
    INCLUDES += -Iexternal/glfw3/include
# OpenAL Soft library
    INCLUDES += -Iexternal/openal_soft/include
endif

# define all object files required with a wildcard
# The wildcard takes all files that finish with ".c", then it replaces the
# extentions with ".o", that are the object files.
OBJS = $(patsubst %.c, %.o, $(wildcard *.c))
OBJS += external/stb_vorbis.o

# compile raylib static library for desktop platforms.
# The automatic variable "$@" is the target name, while "$<" is the first
# prerequisite.
libraylib.a : $(OBJS)
	ar rcs $@ $(OBJS)
	@echo "libraylib.a generated (static library)!"

# compile raylib to shared library version.
libraylib.so : $(OBJS)
	$(CC) -shared -o $@ $(OBJS)

# compile raylib for web.
libraylib.bc : $(OBJS)
	emcc -O1 $(OBJS) -o $@
	@echo "libraylib.bc generated (web version)!"

# compile all modules with relative prerequisites (header files of the
# project).
camera.o : camera.c raylib.h
	$(CC) -c $< $(CFLAGS) $(INCLUDES)

core.o : core.c raylib.h rlgl.h utils.h raymath.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE) -D$(PLATFORM)

gestures.o : gestures.c raylib.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE)

models.o : models.c raylib.h rlgl.h raymath.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE) -D$(PLATFORM)

rlgl.o : rlgl.c rlgl.h 
	$(CC) -c $< $(CFLAGS) $(INCLUDE) -D$(GRAPHICS)

shapes.o : shapes.c raylib.h rlgl.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE)

text.o : text.c raylib.h utils.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE)

textures.o : textures.c rlgl.h utils.h
	$(CC) -c $< $(CFLAGS) $(INCLUDES) -D$(PLATFORM) -D$(GRAPHICS)

utils.o : utils.c utils.h
	$(CC) -c $< $(CFLAGS) $(INCLUDES) -D$(PLATFORM)

audio.o : audio.c audio.h
	$(CC) -c $< $(CFLAGS) $(INCLUDES) -D$(PLATFORM)

# compile stb_vorbis library
external/stb_vorbis.o: external/stb_vorbis.c external/stb_vorbis.h
	$(CC) -c -o $@ $< -O1 $(CFLAGS) $(INCLUDES) -D$(PLATFORM)

# It installs (copy) raylib dev files (static library and header) to standard
# directories on GNU/Linux platform.
# TODO: add other platforms.
install :
ifeq ($(ROOT),root)
    ifeq ($(PLATFORM_OS),LINUX)
        # On GNU/Linux there are some standard directories that contain
        # libraries and header files. These directory (/usr/local/lib and
        # /usr/local/include/) are for libraries that are installed
        # manually (without a package manager).
		cp --update raylib.h /usr/local/include/raylib.h
        ifeq ($(SHARED),YES)
			cp --update libraylib.so /usr/local/lib/libraylib.so
        else
			cp --update libraylib.a /usr/local/lib/libraylib.a
        endif
		@echo "raylib dev files installed/updated!"
    else
		@echo "This function works only on GNU/Linux systems"
    endif
else
	@echo "Error: no root permissions"
endif

# it removes raylib dev files installed on the system.
# TODO: see 'install' target.
unistall :
ifeq ($(ROOT),root)
    ifeq ($(PLATFORM_OS),LINUX)
			rm --force /usr/local/include/raylib.h
        ifeq ($(SHARED),YES)
			rm --force /usr/local/lib/libraylib.so
        else
			rm --force /usr/local/lib/libraylib.a
        endif
		@echo "raylib dev files removed!"
    else
		@echo "This function works only on GNU/Linux systems"
    endif
else
	@echo "Error: no root permissions"
endif

# clean everything
clean:
ifeq ($(PLATFORM_OS),WINDOWS)
	del *.o libraylib.a libraylib.bc libraylib.so external/stb_vorbis.o
else
	rm -f *.o libraylib.a libraylib.bc libraylib.so external/stb_vorbis.o
endif
	@echo "removed all generated files!"

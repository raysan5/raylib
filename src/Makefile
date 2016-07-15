#**************************************************************************************************
#
#   raylib makefile for desktop platforms, Raspberry Pi and HTML5 (emscripten)
#
#   Copyright (c) 2014 Ramon Santamaria (@raysan5)
#    
#   This software is provided "as-is", without any express or implied warranty. In no event 
#   will the authors be held liable for any damages arising from the use of this software.
#
#   Permission is granted to anyone to use this software for any purpose, including commercial 
#   applications, and to alter it and redistribute it freely, subject to the following restrictions:
#
#     1. The origin of this software must not be misrepresented; you must not claim that you 
#     wrote the original software. If you use this software in a product, an acknowledgment 
#     in the product documentation would be appreciated but is not required.
#
#     2. Altered source versions must be plainly marked as such, and must not be misrepresented
#     as being the original software.
#
#     3. This notice may not be removed or altered from any source distribution.
#
#**************************************************************************************************

.PHONY: all clean

# define raylib platform to compile for
# possible platforms: PLATFORM_DESKTOP PLATFORM_RPI PLATFORM_WEB
PLATFORM ?= PLATFORM_DESKTOP

# determine PLATFORM_OS in case PLATFORM_DESKTOP selected
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    # No uname.exe on MinGW!, but OS=Windows_NT on Windows! ifeq ($(UNAME),Msys) -> Windows
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
    # define raylib graphics api to use (OpenGL 1.1 by default)
    GRAPHICS ?= GRAPHICS_API_OPENGL_11
    #GRAPHICS = GRAPHICS_API_OPENGL_21  # Uncomment to use OpenGL 2.1
    #GRAPHICS = GRAPHICS_API_OPENGL_33  # Uncomment to use OpenGL 3.3
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    GRAPHICS = GRAPHICS_API_OPENGL_ES2
endif

# NOTE: makefiles targets require tab indentation

# define compiler: gcc for C program, define as g++ for C++
ifeq ($(PLATFORM),PLATFORM_WEB)
    # define emscripten compiler
    CC = emcc
else
    # define default gcc compiler
    CC = gcc
endif

# define compiler flags:
#  -O1                  defines optimization level
#  -Wall                turns on most, but not all, compiler warnings
#  -std=c99             defines C language mode (standard C from 1999 revision)
#  -std=gnu99           defines C language mode (GNU C from 1999 revision)
#  -fgnu89-inline       declaring inline functions support (GCC optimized, faster)
#  -Wno-missing-braces  ignore invalid warning (GCC bug 53119)
CFLAGS = -O1 -Wall -std=gnu99 -fgnu89-inline -Wno-missing-braces

#CFLAGSEXTRA = -Wextra -Wmissing-prototypes -Wstrict-prototypes

# define any directories containing required header files
ifeq ($(PLATFORM),PLATFORM_RPI)
    INCLUDES = -I. -Iexternal -I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/include/interface/vcos/pthreads
else
# STB libraries and others
    INCLUDES = -I. -Iexternal
# GLFW3 library
    INCLUDES += -Iexternal/glfw3/include
# OpenAL Soft library
    INCLUDES += -Iexternal/openal_soft/include
endif

# define all object files required
OBJS = core.o rlgl.o shapes.o text.o textures.o models.o audio.o utils.o camera.o gestures.o stb_vorbis.o

# typing 'make' will invoke the default target entry called 'all',
# in this case, the 'default' target entry is raylib
all: raylib

# compile raylib library
raylib: $(OBJS)
ifeq ($(PLATFORM),PLATFORM_WEB)
	emcc -O1 $(OBJS) -o libraylib.bc
else
	ar rcs libraylib.a $(OBJS)
endif

# compile core module
# emcc core.c -o core.bc -DPLATFORM_DESKTOP
core.o: core.c
	$(CC) -c core.c $(CFLAGS) $(INCLUDES) -D$(PLATFORM) -D$(GRAPHICS)

# compile rlgl module
rlgl.o: rlgl.c
	$(CC) -c rlgl.c $(CFLAGS) $(INCLUDES) -D$(PLATFORM) -D$(GRAPHICS)

# compile shapes module
shapes.o: shapes.c
	$(CC) -c shapes.c $(CFLAGS) $(INCLUDES) -D$(PLATFORM) -D$(GRAPHICS)

# compile textures module
textures.o: textures.c
	$(CC) -c textures.c $(CFLAGS) $(INCLUDES) -D$(PLATFORM) -D$(GRAPHICS)

# compile text module
text.o: text.c
	$(CC) -c text.c $(CFLAGS) $(INCLUDES) -D$(PLATFORM) -D$(GRAPHICS)

# compile models module
models.o: models.c
	$(CC) -c models.c $(CFLAGS) $(INCLUDES) -D$(PLATFORM) -D$(GRAPHICS)

# compile audio module
audio.o: audio.c
	$(CC) -c audio.c $(CFLAGS) $(INCLUDES) -D$(PLATFORM)

# compile utils module
utils.o: utils.c
	$(CC) -c utils.c $(CFLAGS) $(INCLUDES) -D$(PLATFORM)

# compile camera module
camera.o: camera.c
	$(CC) -c camera.c $(CFLAGS) $(INCLUDES) -D$(PLATFORM)

# compile gestures module
gestures.o: gestures.c
	$(CC) -c gestures.c $(CFLAGS) $(INCLUDES) -D$(PLATFORM)

# compile stb_vorbis library
stb_vorbis.o: external/stb_vorbis.c
	$(CC) -c external/stb_vorbis.c -O1 $(INCLUDES) -D$(PLATFORM)

# clean everything
clean:
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
		del *.o libraylib.a
    else
		rm -f *.o libraylib.a
    endif
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    ifeq ($(PLATFORM_OS),WINDOWS)
		del *.o libraylib.bc
    else
		rm -f *.o libraylib.bc
    endif
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
	rm -f *.o libraylib.a
endif
	@echo Cleaning done

# instead of defining every module one by one, we can define a pattern
# this pattern below will automatically compile every module defined on $(OBJS)
#%.o : %.c
#	$(CC) -c $< $(CFLAGS) $(INCLUDES) -D$(PLATFORM) -D$(GRAPHICS)

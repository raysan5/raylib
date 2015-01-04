#**************************************************************************************************
#
#   raylib makefile for desktop platforms, Raspberry Pi and HTML5 (emscripten)
#
#   Copyright (c) 2014 Ramon Santamaria (Ray San - raysan@raysanweb.com)
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

PLATFORM_OS = UNDEFINED
PLATFORM_ARCH = UNDEFINED

# detect OS and architecture
ifeq ($(OS),Windows_NT)
	PLATFORM_OS = WIN
    ifeq ($(PROCESSOR_ARCHITECTURE),AMD64)
        PLATFORM_ARCH = 64
    endif
    ifeq ($(PROCESSOR_ARCHITECTURE),x86)
        PLATFORM_ARCH = 32
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
    	UNAME_P := $(shell uname -mp)
        PLATFORM_OS = LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
    	UNAME_P := $(shell uname -m)
    	PLATFORM_OS = OSX
    endif

    ifeq ($(UNAME_P),x86_64)
    	PLATFORM_ARCH = 64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
        PLATFORM_ARCH = 32
    endif
endif

# define raylib platform to compile for
# possible platforms: PLATFORM_DESKTOP PLATFORM_RPI PLATFORM_WEB
PLATFORM ?= PLATFORM_DESKTOP

# define default targets to compile for
TARGET_OS ?= $(PLATFORM_OS)
TARGET_ARCH ?= $(PLATFORM_ARCH)

ifeq ($(PLATFORM),PLATFORM_RPI)
	TARGET_OS = LINUX
	TARGET_ARCH = ARM
endif

ifeq ($(PLATFORM),PLATFORM_WEB)
	TARGET_OS = WEB
	TARGET_ARCH = WEB
endif

# prompt current target
$(info **** RUNNING OS = $(PLATFORM_OS), RUNNING ARCH = $(PLATFORM_ARCH))
$(info **** TARGET PLATFORM = $(PLATFORM), TARGET OS = $(TARGET_OS), TARGET ARCH = $(TARGET_ARCH))

# define raylib graphics api depending on selected platform
ifeq ($(PLATFORM),PLATFORM_RPI)
    # define raylib graphics api to use (on RPI, OpenGL ES 2.0 must be used)
    GRAPHICS = GRAPHICS_API_OPENGL_ES2
else
    # define raylib graphics api to use (on Windows desktop, OpenGL 1.1 by default)
    GRAPHICS ?= GRAPHICS_API_OPENGL_11
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
	ifeq ($(PLATFORM_OS),OSX)
		# define llvm compiler for mac
		CC = clang
	else
	    # define default gcc compiler
	    CC = gcc
    endif
endif

# define compiler flags:
#  -O2         defines optimization level
#  -Wall       turns on most, but not all, compiler warnings
#  -std=c99    use standard C from 1999 revision
ifeq ($(PLATFORM),PLATFORM_RPI)
    CFLAGS = -O1 -Wall -std=gnu99 -fgnu89-inline
else
    CFLAGS = -O1 -Wall -std=c99
endif

#CFLAGSEXTRA = -Wextra -Wmissing-prototypes -Wstrict-prototypes

# define any directories containing required header files
ifeq ($(PLATFORM),PLATFORM_RPI)
    INCLUDES = -I. -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads
else
    INCLUDES = -I.
endif

# include external libraries
#glfw3
INCLUDES += -I. -I../external/glfw3/include
#glew
INCLUDES += -I. -I../external/glew/include
#Open AL
INCLUDES += -I. -I../external/openal_soft/include

# define all object files required
OBJS = core.o rlgl.o raymath.o shapes.o text.o textures.o models.o audio.o utils.o stb_vorbis.o

# typing 'make' will invoke the first target entry in the file,
# in this case, the 'default' target entry is raylib
default: raylib

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

# compile raymath module
raymath.o: raymath.c
	$(CC) -c raymath.c $(CFLAGS) $(INCLUDES)

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

# compile stb_vorbis library
stb_vorbis.o: stb_vorbis.c
	$(CC) -c stb_vorbis.c $(CFLAGS) $(INCLUDES) -D$(PLATFORM)

# clean everything
clean:
ifeq ($(PLATFORM),PLATFORM_RPI)
	rm -f *.o libraylib.a
else
ifeq ($(PLATFORM_OS),LINUX)
	find . -type f -executable -delete
	rm -f *.o libraylib.a
else
ifeq ($(PLATFORM_OS),OSX)
	find . -type f -perm +ugo+x -delete
	rm -f *.o libraylib.a
else
ifeq ($(PLATFORM),PLATFORM_WEB)
	del *.o libraylib.bc
else
	del *.o libraylib.a
endif
endif
endif
endif
	@echo Cleaning done

# instead of defining every module one by one, we can define a pattern
# this pattern below will automatically compile every module defined on $(OBJS)
#%.o : %.c
#	$(CC) -c $< $(CFLAGS) $(INCLUDES) -D$(PLATFORM) -D$(GRAPHICS)

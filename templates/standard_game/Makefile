#**************************************************************************************************
#
#   raylib makefile for desktop platforms, Raspberry Pi and HTML5 (emscripten)
#
#   Copyright (c) 2013-2017 Ramon Santamaria (@raysan5)
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
# WARNING: To compile to HTML5, code must be redesigned to use emscripten.h and emscripten_set_main_loop()
PLATFORM ?= PLATFORM_DESKTOP

# define NO to use OpenAL Soft as static library (shared by default)
SHARED_OPENAL ?= NO

ifeq ($(PLATFORM),PLATFORM_WEB)
    SHARED_OPENAL = NO
endif

# define raylib directory for include and library
RAYLIB_PATH ?= C:\raylib\raylib

# determine PLATFORM_OS in case PLATFORM_DESKTOP selected
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    # No uname.exe on MinGW!, but OS=Windows_NT on Windows! ifeq ($(UNAME),Msys) -> Windows
    ifeq ($(OS),Windows_NT)
        PLATFORM_OS=WINDOWS
        LIBPATH=win32
    else
        UNAMEOS:=$(shell uname)
        ifeq ($(UNAMEOS),Linux)
            PLATFORM_OS=LINUX
            LIBPATH=linux
        else
        ifeq ($(UNAMEOS),Darwin)
            PLATFORM_OS=OSX
            LIBPATH=osx
        endif
        endif
    endif
endif

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
#  -O2                  defines optimization level
#  -Og                  enable debugging
#  -s                   strip unnecessary data from build
#  -Wall                turns on most, but not all, compiler warnings
#  -std=c99             defines C language mode (standard C from 1999 revision)
#  -std=gnu99           defines C language mode (GNU C from 1999 revision)
#  -fgnu89-inline       declaring inline functions support (GCC optimized)
#  -Wno-missing-braces  ignore invalid warning (GCC bug 53119)
#  -D_DEFAULT_SOURCE    use with -std=c99 on Linux and PLATFORM_WEB, required for timespec
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
        CFLAGS = -O2 -s -Wall -std=c99
    endif
    ifeq ($(PLATFORM_OS),LINUX)
        CFLAGS = -O2 -s -Wall -std=c99 -D_DEFAULT_SOURCE
    endif
    ifeq ($(PLATFORM_OS),OSX)
        CFLAGS = -O2 -s -Wall -std=c99
    endif
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    CFLAGS = -O1 -Wall -std=c99 -D_DEFAULT_SOURCE -s USE_GLFW=3 -s ASSERTIONS=1 -s TOTAL_MEMORY=67108864 --profiling --preload-file resources
    # -O2                         # if used, also set --memory-init-file 0
    # --memory-init-file 0        # to avoid an external memory initialization code file (.mem)
    # -s ALLOW_MEMORY_GROWTH=1    # to allow memory resizing
    # -s TOTAL_MEMORY=16777216    # to specify heap memory size (default = 16MB)
    # --preload-file file.res     # embbed file.res resource into .data file
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
    CFLAGS = -O2 -s -Wall -std=gnu99 -fgnu89-inline
endif
#CFLAGSEXTRA = -Wextra -Wmissing-prototypes -Wstrict-prototypes

# define raylib release directory for compiled library
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
        RAYLIB_RELEASE = $(RAYLIB_PATH)/release/win32/mingw32
    endif
    ifeq ($(PLATFORM_OS),LINUX)
        RAYLIB_RELEASE = $(RAYLIB_PATH)/release/linux
    endif
    ifeq ($(PLATFORM_OS),OSX)
        RAYLIB_RELEASE = $(RAYLIB_PATH)/release/osx
    endif
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    RAYLIB_RELEASE = $(RAYLIB_PATH)/release/html5
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
    RAYLIB_RELEASE = $(RAYLIB_PATH)/release/rpi
endif

# define any directories containing required header files
INCLUDES = -I. -I$(RAYLIB_RELEASE) -I$(RAYLIB_PATH)/src -I$(RAYLIB_PATH)/src/external 

ifeq ($(PLATFORM),PLATFORM_RPI)
    INCLUDES += -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads
endif
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
        # external libraries headers
        # GLFW3
            INCLUDES += -I$(RAYLIB_PATH)/src/external/glfw3/include
        # OpenAL Soft
            INCLUDES += -I$(RAYLIB_PATH)/src/external/openal_soft/include
    endif
    ifeq ($(PLATFORM_OS),LINUX)
        # you may optionally create this directory and install raylib 
        # and related headers there. Edit ../src/Makefile appropriately.
            INCLUDES += -I/usr/local/include/raylib
    endif
    ifeq ($(PLATFORM_OS),OSX)
        # additional directories for MacOS
    endif
endif

# define library paths containing required libs
LFLAGS = -L. -L$(RAYLIB_RELEASE) -L$(RAYLIB_PATH)/src 

ifeq ($(PLATFORM),PLATFORM_RPI)
    LFLAGS += -L/opt/vc/lib
endif
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    # add standard directories for GNU/Linux
    ifeq ($(PLATFORM_OS),WINDOWS)
        # external libraries to link with
        # GLFW3
            LFLAGS += -L$(RAYLIB_PATH)/src/external/glfw3/lib/$(LIBPATH)
        # OpenAL Soft
            LFLAGS += -L$(RAYLIB_PATH)/src/external/openal_soft/lib/$(LIBPATH)
    endif
endif

# define any libraries to link into executable
# if you want to link libraries (libname.so or libname.a), use the -lname
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),LINUX)
        # libraries for Debian GNU/Linux desktop compiling
        # requires the following packages:
        # libglfw3-dev libopenal-dev libegl1-mesa-dev
        LIBS = -lraylib -lglfw3 -lGL -lopenal -lm -lpthread -ldl
        # on XWindow requires also below libraries
        LIBS += -lX11 -lXrandr -lXinerama -lXi -lXxf86vm -lXcursor
    else
    ifeq ($(PLATFORM_OS),OSX)
        # libraries for OSX 10.9 desktop compiling
        # requires the following packages:
        # libglfw3-dev libopenal-dev libegl1-mesa-dev
        LIBS = -lraylib -lglfw -framework OpenGL -framework OpenAL -framework Cocoa
    else
        # libraries for Windows desktop compiling
        # NOTE: GLFW3 and OpenAL Soft libraries should be installed
        LIBS = -lraylib -lglfw3 -lopengl32 -lgdi32
        # if static OpenAL Soft required, define the corresponding libs
        ifeq ($(SHARED_OPENAL),NO)
            LIBS += -lopenal32 -lwinmm
            CFLAGS += -Wl,-allow-multiple-definition
        else
            LIBS += -lopenal32dll
        endif
    endif
    endif
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
    # libraries for Raspberry Pi compiling
    # NOTE: OpenAL Soft library should be installed (libopenal1 package)
    LIBS = -lraylib -lGLESv2 -lEGL -lpthread -lrt -lm -lbcm_host -lopenal
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    # NOTE: Set the correct path to libraylib.bc
    LIBS = $(RAYLIB_RELEASE)/libraylib.bc
endif

# define additional parameters and flags for windows
ifeq ($(PLATFORM_OS),WINDOWS)
    # resources file contains windows exe icon
    # -Wl,--subsystem,windows hides the console window
    WINFLAGS = $(RAYLIB_PATH)/src/resources -Wl,--subsystem,windows
endif

ifeq ($(PLATFORM),PLATFORM_WEB)
    EXT = .html
    WEB_SHELL = --shell-file $(RAYLIB_PATH)\templates\web_shell\shell.html
endif

# define all screen object files required
SCREENS = \
	screens/screen_logo.o \
	screens/screen_title.o \
	screens/screen_options.o \
	screens/screen_gameplay.o \
	screens/screen_ending.o \

# typing 'make' will invoke the default target entry called 'all',
# in this case, the 'default' target entry is standard_game
all: standard_game

# compile template - standard_game
standard_game: standard_game.c $(SCREENS)
	$(CC) -o $@$(EXT) $< $(SCREENS) $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile screen LOGO
screens/screen_logo.o: screens/screen_logo.c
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDES) -D$(PLATFORM)

# compile screen TITLE
screens/screen_title.o: screens/screen_title.c
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDES) -D$(PLATFORM)

# compile screen OPTIONS
screens/screen_options.o: screens/screen_options.c
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDES) -D$(PLATFORM)

# compile screen GAMEPLAY
screens/screen_gameplay.o: screens/screen_gameplay.c
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDES) -D$(PLATFORM)

# compile screen ENDING
screens/screen_ending.o: screens/screen_ending.c
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDES) -D$(PLATFORM)

# clean everything
clean:
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),OSX)
		find . -type f -perm +ugo+x -delete
		rm -f *.o
    else
    ifeq ($(PLATFORM_OS),LINUX)
		find -type f -executable | xargs file -i | grep -E 'x-object|x-archive|x-sharedlib|x-executable' | rev | cut -d ':' -f 2- | rev | xargs rm -f
    else
		del *.o *.exe /s
    endif
    endif
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
	find . -type f -executable -delete
	rm -f *.o
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
	del *.o *.html *.js
endif
	@echo Cleaning done

# instead of defining every module one by one, we can define a pattern
# this pattern below will automatically compile every module defined on $(OBJS)
#%.exe : %.c
#	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM)

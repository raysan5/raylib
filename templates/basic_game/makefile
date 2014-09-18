#**************************************************************************************************
#
#   raylib - Basic Game
#
#   makefile to compile basic game
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

# define raylib platform (by default, compile for RPI)
# Other possible platform: PLATFORM_DESKTOP
PLATFORM ?= PLATFORM_RPI

# define compiler: gcc for C program, define as g++ for C++
CC = gcc

# define compiler flags:
#  -O2         defines optimization level
#  -Wall       turns on most, but not all, compiler warnings
#  -std=c99    use standard C from 1999 revision
CFLAGS = -O2 -Wall -std=c99
#CFLAGSEXTRA = -Wextra -Wmissing-prototypes -Wstrict-prototypes

# define any directories containing required header files
ifeq ($(PLATFORM),PLATFORM_RPI)
    INCLUDES = -I. -I../../src -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads
else
    INCLUDES = -I. -I../../src
endif

# define library paths containing required libs
LFLAGS = -L. -L../../src -L/opt/vc/lib

# define any libraries to link into executable
# if you want to link libraries (libname.so or libname.a), use the -lname
ifeq ($(PLATFORM),PLATFORM_RPI)
    # libraries for Raspberry Pi compiling
    # NOTE: OpenAL Soft library should be installed (libopenal1 package)
    LIBS = -lraylib -lGLESv2 -lEGL -lm -lbcm_host -lopenal
else
    # libraries for Windows desktop compiling
    # NOTE: GLFW3 and OpenAL Soft libraries should be installed
    LIBS = -lraylib -lglfw3 -lglew32 -lopengl32 -lopenal32 -lgdi32
endif

# define additional parameters and flags for windows
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    # resources file contains windows exe icon
    # -Wl,--subsystem,windows hides the console window
    WINFLAGS = ../../src/resources -Wl,--subsystem,windows
endif

# typing 'make' will invoke the first target entry in the file,
# in this case, the 'default' target entry is basic_game
default: basic_game

# compile template - basic_game
basic_game: basic_game.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# clean everything
clean:
ifeq ($(PLATFORM),PLATFORM_RPI)
	rm -f *.o
#	find . -executable -delete
else
	del *.o *.exe
endif
	@echo Cleaning done

# instead of defining every module one by one, we can define a pattern
# this pattern below will automatically compile every module defined on $(OBJS)
#%.exe : %.c
#	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM)

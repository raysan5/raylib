#******************************************************************************
#
#  raylib makefile
#
#  Platforms supported: 
#    PLATFORM_DESKTOP:  Windows (win32/Win64)
#    PLATFORM_DESKTOP:  Linux
#    PLATFORM_DESKTOP:  OSX (Mac)
#    PLATFORM_ANDROID:  Android (ARM or ARM64)
#    PLATFORM_RPI:      Raspberry Pi (Raspbian)
#    PLATFORM_WEB:      HTML5 (Chrome, Firefox)
#
#  Many thanks to Milan Nikolic (@gen2brain) for implementing Android platform pipeline.
#  Many thanks to Emanuele Petriglia for his contribution on GNU/Linux pipeline.
#
#  Copyright (c) 2014-2017 Ramon Santamaria (@raysan5)
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

.PHONY: all clean install uninstall

# define raylib platform to compile for
# possible platforms: PLATFORM_DESKTOP PLATFORM_ANDROID PLATFORM_RPI PLATFORM_WEB
PLATFORM ?= PLATFORM_DESKTOP

# define raylib default path, required to look for emsdk and android-toolchain
RAYLIB_PATH ?= C:/raylib

# define YES if you want shared/dynamic version of library instead of static (default)
SHARED_RAYLIB ?= NO

# use OpenAL Soft as shared library (.so / .dll)
# NOTE: If defined NO, static OpenAL Soft library should be provided
SHARED_OPENAL ?= NO

# on PLATFORM_WEB force OpenAL Soft shared library
ifeq ($(PLATFORM),PLATFORM_WEB)
    SHARED_OPENAL = YES
endif
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    SHARED_OPENAL = YES
endif

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

ifeq ($(PLATFORM),PLATFORM_WEB)
    # Emscripten required variables
    EMSDK_PATH = $(RAYLIB_PATH)/emsdk
    EMSCRIPTEN_VERSION = 1.37.9
    CLANG_VERSION=e1.37.9_64bit
    PYTHON_VERSION=2.7.5.3_64bit
    NODE_VERSION=4.1.1_64bit
    export PATH=$(EMSDK_PATH);$(EMSDK_PATH)\clang\$(CLANG_VERSION);$(EMSDK_PATH)\node\$(NODE_VERSION)\bin;$(EMSDK_PATH)\python\$(PYTHON_VERSION);$(EMSDK_PATH)\emscripten\$(EMSCRIPTEN_VERSION);C:\raylib\MinGW\bin:$$(PATH)
    EMSCRIPTEN=$(EMSDK_PATH)\emscripten\$(EMSCRIPTEN_VERSION)
endif

ifeq ($(PLATFORM),PLATFORM_ANDROID)
    # Android NDK path
    # NOTE: Required for standalone toolchain generation
    ANDROID_NDK = $(ANDROID_NDK_HOME)
    
    # Android standalone toolchain path
    # NOTE: This path is also used if toolchain generation
    #ANDROID_TOOLCHAIN = $(CURDIR)/toolchain
    ANDROID_TOOLCHAIN = $(RAYLIB_PATH)/android-toolchain

    # Android architecture: ARM or ARM64
    ANDROID_ARCH ?= ARM

    # Android compiler: gcc or clang 
    # NOTE: Define YES to use clang instead of gcc
    ANDROID_LLVM ?= NO
endif

ifeq ($(PLATFORM),PLATFORM_RPI)
    # RPI cross-compiler
    CROSS_COMPILE ?= NO
endif

# define raylib graphics api depending on selected platform
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    # by default use OpenGL 3.3 on desktop platforms
    GRAPHICS ?= GRAPHICS_API_OPENGL_33
    #GRAPHICS = GRAPHICS_API_OPENGL_11  # Uncomment to use OpenGL 1.1
    #GRAPHICS = GRAPHICS_API_OPENGL_21  # Uncomment to use OpenGL 2.1
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
    # on RPI OpenGL ES 2.0 must be used
    GRAPHICS = GRAPHICS_API_OPENGL_ES2
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    # on HTML5 OpenGL ES 2.0 is used, emscripten translates it to WebGL 1.0
    GRAPHICS = GRAPHICS_API_OPENGL_ES2
endif
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    # by default use OpenGL ES 2.0 on Android
    GRAPHICS = GRAPHICS_API_OPENGL_ES2
endif

# NOTE: makefiles targets require tab indentation
# NOTE: define compiler: gcc for C program, define as g++ for C++

# default gcc compiler
CC = gcc


# For OS X
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),OSX)
		CC = clang
	endif
endif

# Android toolchain compiler
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    ifeq ($(ANDROID_ARCH),ARM)
        ifeq ($(ANDROID_LLVM),YES)
            CC = $(ANDROID_TOOLCHAIN)/bin/arm-linux-androideabi-clang
        else
            CC = $(ANDROID_TOOLCHAIN)/bin/arm-linux-androideabi-gcc
        endif
    endif
    ifeq ($(ANDROID_ARCH),ARM64)
        ifeq ($(ANDROID_LLVM),YES)
            CC = $(ANDROID_TOOLCHAIN)/bin/aarch64-linux-android-clang
        else
            CC = $(ANDROID_TOOLCHAIN)/bin/aarch64-linux-android-gcc
        endif
    endif
endif

# RPI cross-compiler
ifeq ($(PLATFORM),PLATFORM_RPI)
    ifeq ($(CROSS_COMPILE),YES)
    	# rpi compiler
        CC = armv6j-hardfloat-linux-gnueabi-gcc
    endif
endif

# HTML5 emscripten compiler
ifeq ($(PLATFORM),PLATFORM_WEB)
    CC = emcc
endif

# default archiver program to pack libraries
AR = ar

# Android archiver
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    ifeq ($(ANDROID_ARCH),ARM)
        AR = $(ANDROID_TOOLCHAIN)/bin/arm-linux-androideabi-ar
    endif
    ifeq ($(ANDROID_ARCH),ARM64)
        AR = $(ANDROID_TOOLCHAIN)/bin/aarch64-linux-android-ar
    endif
endif

# define compiler flags:
#  -O1                  defines optimization level
#  -Og                  enable debugging
#  -Wall                turns on most, but not all, compiler warnings
#  -std=c99             defines C language mode (standard C from 1999 revision)
#  -std=gnu99           defines C language mode (GNU C from 1999 revision)
#  -fgnu89-inline       declaring inline functions support (GCC optimized)
#  -Wno-missing-braces  ignore invalid warning (GCC bug 53119)
#  -D_DEFAULT_SOURCE    use with -std=c99
CFLAGS += -O1 -Wall -std=c99 -D_DEFAULT_SOURCE -fgnu89-inline -Wno-missing-braces

ifeq ($(PLATFORM),PLATFORM_WEB)
    CFLAGS += -s USE_GLFW=3 -s ASSERTIONS=1 --profiling --preload-file resources
    # -O2                        # if used, also set --memory-init-file 0
    # --memory-init-file 0       # to avoid an external memory initialization code file (.mem)
    # -s ALLOW_MEMORY_GROWTH=1   # to allow memory resizing
    # -s TOTAL_MEMORY=16777216   # to specify heap memory size (default = 16MB)
    # -s USE_PTHREADS=1          # multithreading support
endif

#CFLAGSEXTRA = -Wextra -Wmissing-prototypes -Wstrict-prototypes

# if shared library required, make sure code is compiled as position independent
ifeq ($(SHARED_RAYLIB),YES)
    CFLAGS += -fPIC
    SHAREDFLAG = BUILDING_DLL
else
    SHAREDFLAG = BUILDING_STATIC
endif

# if static OpenAL Soft required, define the corresponding flags
ifeq ($(SHARED_OPENAL),NO)
    SHAREDLIBS += -lopenal32 -lwinmm
    SHAREDOPENALFLAG = AL_LIBTYPE_STATIC
else
    SHAREDLIBS += -lopenal32dll
    SHAREDOPENALFLAG = SHARED_OPENAL
endif

#CFLAGSEXTRA = -Wextra -Wmissing-prototypes -Wstrict-prototypes

# external required libraries (stb and others)
INCLUDES = -I. -Iexternal

# OpenAL Soft library
INCLUDES += -Iexternal/openal_soft/include

# define any directories containing required header files
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    # GLFW3 library
    INCLUDES += -Iexternal/glfw3/include
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
    # RPI requried libraries
    INCLUDES += -I/opt/vc/include
    INCLUDES += -I/opt/vc/include/interface/vmcs_host/linux
    INCLUDES += -I/opt/vc/include/interface/vcos/pthreads
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    # GLFW3 library
    INCLUDES += -Iexternal/glfw3/include
endif
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    # Android required libraries
    INCLUDES += -I$(ANDROID_TOOLCHAIN)/sysroot/usr/include
    # Include android_native_app_glue.h
    INCLUDES += -Iexternal/android/native_app_glue
    #INCLUDES += -I$(ANDROID_NDK)/sources/android/native_app_glue
endif

# define output directory for compiled library
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
        OUTPUT_PATH = ../release/win32/mingw32
    endif
    ifeq ($(PLATFORM_OS),LINUX)
        OUTPUT_PATH = ../release/linux
    endif
    ifeq ($(PLATFORM_OS),OSX)
        OUTPUT_PATH = ../release/osx
    endif
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
    OUTPUT_PATH = ../release/rpi
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    OUTPUT_PATH = ../release/html5
endif
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    ifeq ($(ANDROID_ARCH),ARM)
        OUTPUT_PATH = ../release/android/armeabi-v7a
    endif
    ifeq ($(ANDROID_ARCH),ARM64)
        OUTPUT_PATH = ../release/android/arm64-v8a
    endif
endif

# define all object files required with a wildcard
# The wildcard takes all files that finish with ".c", then it replaces the
# extentions with ".o", that are the object files.
OBJS = $(patsubst %.c, %.o, $(wildcard *.c))
OBJS += external/stb_vorbis.o

# typing 'make' will invoke the default target entry called 'all',
# in this case, the 'default' target entry is raylib
all: raylib

# generate standalone Android toolchain
# NOTE: Android toolchain could already be provided
toolchain:
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    ifeq ($(ANDROID_ARCH),ARM)
		$(ANDROID_NDK)/build/tools/make-standalone-toolchain.sh --platform=android-9 --toolchain=arm-linux-androideabi-4.9 --use-llvm --install-dir=$(ANDROID_TOOLCHAIN)
    endif
    ifeq ($(ANDROID_ARCH),ARM64)
		$(ANDROID_NDK)/build/tools/make-standalone-toolchain.sh --platform=android-21 --toolchain=aarch64-linux-androideabi-4.9 --use-llvm --install-dir=$(ANDROID_TOOLCHAIN)
    endif
endif

# compile raylib library
raylib: $(OBJS)
ifeq ($(PLATFORM),PLATFORM_WEB)
    # compile raylib for web.
	emcc -O1 $(OBJS) -o $(OUTPUT_PATH)/libraylib.bc
	@echo "libraylib.bc generated (web version)!"
else
    ifeq ($(SHARED_RAYLIB),YES)
        # NOTE: If using OpenAL Soft as static library, all its dependencies must be also linked in the shared library
        ifeq ($(PLATFORM_OS),WINDOWS)
			$(CC) -shared -o $(OUTPUT_PATH)/raylib.dll $(OBJS) $(SHAREDLIBS) -Lexternal/glfw3/lib/win32 -Lexternal/openal_soft/lib/win32 -lglfw3 -lgdi32 -Wl,--out-implib,$(OUTPUT_PATH)/libraylibdll.a
			@echo "raylib dynamic library (raylib.dll) and import library (libraylibdll.a) generated!"
        endif
        ifeq ($(PLATFORM_OS),LINUX)
            # compile raylib to shared library version for GNU/Linux.
            # WARNING: you should type "make clean" before doing this target
			$(CC) -shared -o $(OUTPUT_PATH)/libraylib.so $(OBJS) -lglfw3 -lGL -lopenal -lm -lpthread -ldl
			@echo "raylib shared library generated (libraylib.so)!"
        endif
        ifeq ($(PLATFORM_OS),OSX)
			$(CC) -dynamiclib -o $(OUTPUT_PATH)/libraylib.dylib $(OBJS) -L/usr/local/Cellar/glfw/3.2.1/lib -lglfw -framework OpenGL -framework OpenAL -framework Cocoa
			install_name_tool -id "libraylib.dylib" $(OUTPUT_PATH)/libraylib.dylib
			@echo "raylib shared library generated (libraylib.dylib)!"
        endif
        ifeq ($(PLATFORM),PLATFORM_ANDROID)
			$(CC) -shared -o $(OUTPUT_PATH)/libraylib.so $(OBJS)
			@echo "raylib shared library generated (libraylib.so)!"
        endif
    else
        # compile raylib static library.
		$(AR) rcs $(OUTPUT_PATH)/libraylib.a $(OBJS)
		@echo "raylib static library generated (libraylib.a)!"
        ifeq ($(SHARED_OPENAL),NO)
			@echo "expected OpenAL Soft static library linking"
        else
			@echo "expected OpenAL Soft shared library linking"
        endif
    endif
endif

# compile all modules with their prerequisites

# compile core module
core.o : core.c raylib.h rlgl.h utils.h raymath.h gestures.h
	$(CC) -c $< $(CFLAGS) $(INCLUDES) -D$(PLATFORM) -D$(SHAREDFLAG)

# compile rlgl module
rlgl.o : rlgl.c rlgl.h raymath.h
	$(CC) -c $< $(CFLAGS) $(INCLUDES) -D$(GRAPHICS)

# compile shapes module
shapes.o : shapes.c raylib.h rlgl.h
	$(CC) -c $< $(CFLAGS) $(INCLUDES) -D$(SHAREDFLAG)

# compile textures module
textures.o : textures.c rlgl.h utils.h
	$(CC) -c $< $(CFLAGS) $(INCLUDES) -D$(PLATFORM) -D$(GRAPHICS) -D$(SHAREDFLAG)

# compile text module
text.o : text.c raylib.h utils.h
	$(CC) -c $< $(CFLAGS) $(INCLUDES) -D$(SHAREDFLAG)

# compile models module
models.o : models.c raylib.h rlgl.h raymath.h
	$(CC) -c $< $(CFLAGS) $(INCLUDES) -D$(PLATFORM) -D$(SHAREDFLAG)

# compile audio module
audio.o : audio.c raylib.h
	$(CC) -c $< $(CFLAGS) $(INCLUDES) -D$(PLATFORM) -D$(SHAREDFLAG) -D$(SHAREDOPENALFLAG)

# compile stb_vorbis library
external/stb_vorbis.o: external/stb_vorbis.c external/stb_vorbis.h
	$(CC) -c -o $@ $< -O1 $(CFLAGS) $(INCLUDES) -D$(PLATFORM)

# compile utils module
utils.o : utils.c utils.h
	$(CC) -c $< $(CFLAGS) $(INCLUDES) -D$(PLATFORM) -D$(SHAREDFLAG)

# It installs generated and needed files to compile projects using raylib.
# The installation works manually.
# TODO: add other platforms.
install :
ifeq ($(ROOT),root)
    ifeq ($(PLATFORM_OS),LINUX)
        # On GNU/Linux there are some standard directories that contain
        # libraries and header files. These directory (/usr/local/lib and
        # /usr/local/include/) are for libraries that are installed
        # manually (without a package manager).
        ifeq ($(SHARED_RAYLIB),YES)
			cp --update $(OUTPUT_PATH)/libraylib.so /usr/local/lib/libraylib.so
        else
			cp --update raylib.h /usr/local/include/raylib.h
			cp --update $(OUTPUT_PATH)/libraylib.a /usr/local/lib/libraylib.a
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
uninstall :
ifeq ($(ROOT),root)
    ifeq ($(PLATFORM_OS),LINUX)
			rm --force /usr/local/include/raylib.h
        ifeq ($(SHARED_RAYLIB),YES)
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
	del *.o $(OUTPUT_PATH)/libraylib.a $(OUTPUT_PATH)/libraylib.bc $(OUTPUT_PATH)/libraylib.so external/stb_vorbis.o
else
	rm -f *.o $(OUTPUT_PATH)/libraylib.a $(OUTPUT_PATH)/libraylib.bc $(OUTPUT_PATH)/libraylib.so external/stb_vorbis.o
endif
ifeq ($(PLATFORM),PLATFORM_ANDROID)
	rm -rf $(ANDROID_TOOLCHAIN)
endif
	@echo "removed all generated files!"

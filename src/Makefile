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

# Define required raylib variables
PLATFORM ?= PLATFORM_DESKTOP
RAYLIB_PATH ?= ..

# Library type used for raylib and OpenAL Soft: STATIC (.a) or SHARED (.so/.dll)
# NOTE: OpenAL Soft library should be provided in the selected form
RAYLIB_LIBTYPE ?= STATIC
OPENAL_LIBTYPE ?= STATIC

# On PLATFORM_WEB force OpenAL Soft shared library
ifeq ($(PLATFORM),PLATFORM_WEB)
    OPENAL_LIBTYPE = SHARED
endif

# Use cross-compiler for PLATFORM_RPI
ifeq ($(PLATFORM),PLATFORM_RPI)
    RPI_CROSS_COMPILE ?= YES
    RPI_TOOLCHAIN ?= C:/SysGCC/Raspberry
    RPI_TOOLCHAIN_SYSROOT ?= $(RPI_TOOLCHAIN)/arm-linux-gnueabihf/sysroot
endif

# Determine if the file has root access (only for installing raylib)
# "whoami" prints the name of the user that calls him (so, if it is the root
# user, "whoami" prints "root").
ROOT = $(shell whoami)

# Determine PLATFORM_OS in case PLATFORM_DESKTOP selected
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    # No uname.exe on MinGW!, but OS=Windows_NT on Windows!
    # ifeq ($(UNAME),Msys) -> Windows
    ifeq ($(OS),Windows_NT)
        PLATFORM_OS=WINDOWS
    else
        UNAMEOS=$(shell uname)
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
    EMSDK_PATH = C:/emsdk
    EMSCRIPTEN_VERSION = 1.37.21
    CLANG_VERSION=e1.37.21_64bit
    PYTHON_VERSION=2.7.5.3_64bit
    NODE_VERSION=4.1.1_64bit
    export PATH=$(EMSDK_PATH);$(EMSDK_PATH)\clang\$(CLANG_VERSION);$(EMSDK_PATH)\node\$(NODE_VERSION)\bin;$(EMSDK_PATH)\python\$(PYTHON_VERSION);$(EMSDK_PATH)\emscripten\$(EMSCRIPTEN_VERSION);C:\raylib\MinGW\bin:$$(PATH)
    EMSCRIPTEN=$(EMSDK_PATH)\emscripten\$(EMSCRIPTEN_VERSION)
endif

ifeq ($(PLATFORM),PLATFORM_ANDROID)
    # Android required path variables
    ANDROID_NDK = C:/android-ndk
    ANDROID_TOOLCHAIN = C:/android_toolchain_arm_api16

    # Android architecture: ARM or ARM64
    ANDROID_ARCH ?= ARM
endif

RAYLIB_RELEASE_PATH ?= $(RAYLIB_PATH)/release/libs

# Define output directory for compiled library
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
        RAYLIB_RELEASE_PATH = $(RAYLIB_PATH)/release/libs/win32/mingw32
    endif
    ifeq ($(PLATFORM_OS),LINUX)
        RAYLIB_RELEASE_PATH = $(RAYLIB_PATH)/release/libs/linux
    endif
    ifeq ($(PLATFORM_OS),OSX)
        RAYLIB_RELEASE_PATH = $(RAYLIB_PATH)/release/libs/osx
    endif
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
    RAYLIB_RELEASE_PATH = $(RAYLIB_PATH)/release/libs/rpi
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    RAYLIB_RELEASE_PATH = $(RAYLIB_PATH)/release/libs/html5
endif
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    ifeq ($(ANDROID_ARCH),ARM)
        RAYLIB_RELEASE_PATH = $(RAYLIB_PATH)/release/libs/android/armeabi-v7a
    endif
    ifeq ($(ANDROID_ARCH),ARM64)
        RAYLIB_RELEASE_PATH = $(RAYLIB_PATH)/release/libs/android/arm64-v8a
    endif
endif

# Define raylib graphics api depending on selected platform
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    # By default use OpenGL 3.3 on desktop platforms
    GRAPHICS ?= GRAPHICS_API_OPENGL_33
    #GRAPHICS = GRAPHICS_API_OPENGL_11  # Uncomment to use OpenGL 1.1
    #GRAPHICS = GRAPHICS_API_OPENGL_21  # Uncomment to use OpenGL 2.1
endif

ifeq ($(PLATFORM),PLATFORM_RPI)
    # On RPI OpenGL ES 2.0 must be used
    GRAPHICS = GRAPHICS_API_OPENGL_ES2
endif

ifeq ($(PLATFORM),PLATFORM_WEB)
    # On HTML5 OpenGL ES 2.0 is used, emscripten translates it to WebGL 1.0
    GRAPHICS = GRAPHICS_API_OPENGL_ES2
endif

ifeq ($(PLATFORM),PLATFORM_ANDROID)
    # By default use OpenGL ES 2.0 on Android
    GRAPHICS = GRAPHICS_API_OPENGL_ES2
endif

# Default C compiler: gcc
# NOTE: define g++ compiler if using C++
CC = gcc

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),OSX)
        # OSX default compiler
        CC = clang
    endif
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
    ifeq ($(RPI_CROSS_COMPILE),YES)
        # Define RPI cross-compiler
        #CC = armv6j-hardfloat-linux-gnueabi-gcc
        CC = $(RPI_TOOLCHAIN)/bin/arm-linux-gnueabihf-gcc
    endif
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    # HTML5 emscripten compiler
    CC = emcc
endif
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    # Android toolchain (must be provided for desired architecture and compiler)
    # NOTE: gcc compiler is being deprecated in Android NDK r16
    ifeq ($(ANDROID_ARCH),ARM)
        CC = $(ANDROID_TOOLCHAIN)/bin/arm-linux-androideabi-gcc
    endif
    ifeq ($(ANDROID_ARCH),ARM64)
        CC = $(ANDROID_TOOLCHAIN)/bin/aarch64-linux-android-gcc
    endif
endif

# Default archiver program to pack libraries
AR = ar

ifeq ($(PLATFORM),PLATFORM_RPI)
    ifeq ($(RPI_CROSS_COMPILE),YES)
        # Define RPI cross-archiver
        #CC = armv6j-hardfloat-linux-gnueabi-gcc
        AR = $(RPI_TOOLCHAIN)/bin/arm-linux-gnueabihf-ar
    endif
endif

# Android archiver (also depends on desired architecture)
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    ifeq ($(ANDROID_ARCH),ARM)
        AR = $(ANDROID_TOOLCHAIN)/bin/arm-linux-androideabi-ar
    endif
    ifeq ($(ANDROID_ARCH),ARM64)
        AR = $(ANDROID_TOOLCHAIN)/bin/aarch64-linux-android-ar
    endif
endif

# Define compiler flags:
#  -O1                  defines optimization level
#  -Og                  enable debugging
#  -s                   strip unnecessary data from build
#  -Wall                turns on most, but not all, compiler warnings
#  -std=c99             defines C language mode (standard C from 1999 revision)
#  -std=gnu99           defines C language mode (GNU C from 1999 revision)
#  -fgnu89-inline       declaring inline functions support (GCC optimized)
#  -Wno-missing-braces  ignore invalid warning (GCC bug 53119)
#  -D_DEFAULT_SOURCE    use with -std=c99
CFLAGS += -O1 -Wall -std=c99 -D_DEFAULT_SOURCE -fgnu89-inline -Wno-missing-braces

# Additional flags for compiler (if desired)
#CFLAGS += -Wextra -Wmissing-prototypes -Wstrict-prototypes
ifeq ($(PLATFORM),PLATFORM_WEB)
    # -O2                        # if used, also set --memory-init-file 0
    # --memory-init-file 0       # to avoid an external memory initialization code file (.mem)
    # -s ALLOW_MEMORY_GROWTH=1   # to allow memory resizing
    # -s TOTAL_MEMORY=16777216   # to specify heap memory size (default = 16MB)
    # -s USE_PTHREADS=1          # multithreading support
    CFLAGS += -s USE_GLFW=3 -s ASSERTIONS=1 --profiling
endif
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    # Compiler flags for arquitecture
    CFLAGS += -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16
    # Compilation functions attributes options
    CFLAGS += -ffunction-sections -funwind-tables -fstack-protector-strong -fPIC
    # Compiler options for the linker
    CFLAGS += -Wa,--noexecstack -Wformat -Werror=format-security -no-canonical-prefixes
    # Preprocessor macro definitions
    CFLAGS += -DANDROID -DPLATFORM_ANDROID -D__ANDROID_API__=16
endif

# Define required compilation flags for raylib SHARED lib
ifeq ($(RAYLIB_LIBTYPE),SHARED)
    # make sure code is compiled as position independent
    # BE CAREFUL: It seems that for gcc -fpic si not the same as -fPIC
    # MinGW32 just doesn't need -fPIC, it shows warnings
    CFLAGS += -fPIC -DBUILD_LIBTYPE_SHARED
endif

# Define required compilation flags for OpenAL Soft STATIC lib
ifeq ($(OPENAL_LIBTYPE),STATIC)
    ALFLAGS = -DAL_LIBTYPE_STATIC -Wl,-allow-multiple-definition
endif

# Define include paths for required headers
# NOTE: Several external required libraries (stb and others)
INCLUDE_PATHS = -I. -Iexternal -Iexternal/include

# Define additional directories containing required header files
ifeq ($(PLATFORM),PLATFORM_RPI)
    # RPI requried libraries
    INCLUDE_PATHS += -I$(RPI_TOOLCHAIN_SYSROOT)/opt/vc/include
    INCLUDE_PATHS += -I$(RPI_TOOLCHAIN_SYSROOT)/opt/vc/include/interface/vmcs_host/linux
    INCLUDE_PATHS += -I$(RPI_TOOLCHAIN_SYSROOT)/opt/vc/include/interface/vcos/pthreads
    INCLUDE_PATHS += -I$(RPI_TOOLCHAIN_SYSROOT)/opt/vc/include/interface/vcos/pthreads
endif
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    # Android required libraries
    INCLUDE_PATHS += -I$(ANDROID_TOOLCHAIN)/sysroot/usr/include
    # Include android_native_app_glue.h
    INCLUDE_PATHS += -Iexternal/android/native_app_glue
    #INCLUDE_PATHS += -I$(ANDROID_NDK)/sources/android/native_app_glue
endif

# Define linker options
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    LDFLAGS = -Wl,-soname,libraylib.so -Wl,--exclude-libs,libatomic.a 
    LDFLAGS += -Wl,--build-id -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -Wl,--warn-shared-textrel -Wl,--fatal-warnings 
    # Force linking of library module to define symbol
    LDFLAGS += -u ANativeActivity_onCreate
    # Library paths containing required libs
    LDFLAGS += -L. -Lsrc -L$(RAYLIB_RELEASE_PATH)

    LDLIBS = -lopenal -llog -landroid -lEGL -lGLESv2 -lOpenSLES -latomic -lc -lm
endif

# Define all object files required with a wildcard
# The wildcard takes all files that finish with ".c",
# and replaces extentions with ".o", that are the object files
OBJS = $(patsubst %.c, %.o, $(wildcard *.c))
OBJS += stb_vorbis.o

# Default target entry
all: raylib

# Generate standalone Android toolchain
# NOTE: Android toolchain could already be provided
generate_android_toolchain:
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    ifeq ($(ANDROID_ARCH),ARM)
		$(ANDROID_NDK)/build/tools/make-standalone-toolchain.sh --platform=android-9 --toolchain=arm-linux-androideabi-4.9 --use-llvm --install-dir=$(ANDROID_TOOLCHAIN)
    endif
    ifeq ($(ANDROID_ARCH),ARM64)
		$(ANDROID_NDK)/build/tools/make-standalone-toolchain.sh --platform=android-21 --toolchain=aarch64-linux-androideabi-4.9 --use-llvm --install-dir=$(ANDROID_TOOLCHAIN)
    endif
endif

# Compile raylib library
raylib: $(OBJS)
ifeq ($(PLATFORM),PLATFORM_WEB)
    # Compile raylib for web.
	emcc -O1 $(OBJS) -o $(RAYLIB_RELEASE_PATH)/libraylib.bc
	@echo "raylib library generated (libraylib.bc)!"
else
    ifeq ($(RAYLIB_LIBTYPE),SHARED)
        # NOTE: If using OpenAL Soft as static library, all its dependencies must be also linked in the shared library
        ifeq ($(PLATFORM_OS),WINDOWS)
			$(CC) -shared -o $(RAYLIB_RELEASE_PATH)/raylib.dll $(OBJS) -L$(RAYLIB_RELEASE_PATH) -lglfw3 -lgdi32 -lopenal32 -lwinmm -Wl,--out-implib,$(RAYLIB_RELEASE_PATH)/libraylibdll.a
			@echo "raylib dynamic library (raylib.dll) and import library (libraylibdll.a) generated!"
            @echo "expected OpenAL Soft static library linking"
        endif
        ifeq ($(PLATFORM_OS),LINUX)
            # Compile raylib to shared library version for GNU/Linux.
            # WARNING: you should type "make clean" before doing this target
			$(CC) -shared -o $(RAYLIB_RELEASE_PATH)/libraylib.so $(OBJS) -lglfw -lGL -lopenal -lm -lpthread -ldl
			@echo "raylib shared library generated (libraylib.so)!"
        endif
        ifeq ($(PLATFORM_OS),OSX)
			$(CC) -dynamiclib -o $(RAYLIB_RELEASE_PATH)/libraylib.dylib $(OBJS) -L/usr/local/Cellar/glfw/3.2.1/lib -lglfw -framework OpenGL -framework OpenAL -framework Cocoa
			install_name_tool -id "libraylib.dylib" $(RAYLIB_RELEASE_PATH)/libraylib.dylib
			@echo "raylib shared library generated (libraylib.dylib)!"
        endif
        ifeq ($(PLATFORM),PLATFORM_ANDROID)
			$(CC) -shared -o $(RAYLIB_RELEASE_PATH)/libraylib.so $(OBJS) $(LDFLAGS) $(LDLIBS)
			@echo "raylib shared library generated (libraylib.so)!"
        endif
    else
        # Compile raylib static library
		@echo raylib library release path is $(RAYLIB_RELEASE_PATH)
		$(AR) rcs $(RAYLIB_RELEASE_PATH)/libraylib.a $(OBJS)
		@echo "raylib static library generated (libraylib.a)!"
        ifeq ($(OPENAL_LIBTYPE),STATIC)
			@echo "expected OpenAL Soft static library linking"
        else
			@echo "expected OpenAL Soft shared library linking"
        endif
    endif
endif

# Compile all modules with their prerequisites

# Compile core module
core.o : core.c raylib.h rlgl.h utils.h raymath.h gestures.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM) -D$(GRAPHICS)

# Compile rlgl module
rlgl.o : rlgl.c rlgl.h raymath.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM) -D$(GRAPHICS)

# Compile shapes module
shapes.o : shapes.c raylib.h rlgl.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM) -D$(GRAPHICS)

# Compile textures module
textures.o : textures.c rlgl.h utils.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM) -D$(GRAPHICS)

# Compile text module
text.o : text.c raylib.h utils.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM) -D$(GRAPHICS)

# Compile models module
models.o : models.c raylib.h rlgl.h raymath.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM) -D$(GRAPHICS)

# Compile audio module
audio.o : audio.c raylib.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM) $(ALFLAGS)

# Compile stb_vorbis library
stb_vorbis.o: external/stb_vorbis.c external/stb_vorbis.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM)

# Compile utils module
utils.o : utils.c utils.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM)

# Install generated and needed files to required directories
# TODO: add other platforms.
install :
ifeq ($(ROOT),root)
    ifeq ($(PLATFORM_OS),LINUX)
        # On GNU/Linux there are some standard directories that contain
        # libraries and header files. These directory (/usr/local/lib and
        # /usr/local/include/) are for libraries that are installed
        # manually (without a package manager).
        ifeq ($(RAYLIB_LIBTYPE),SHARED)
			cp --update $(RAYLIB_RELEASE_PATH)/libraylib.so /usr/local/lib/libraylib.so
        else
			cp --update raylib.h /usr/local/include/raylib.h
			cp --update $(RAYLIB_RELEASE_PATH)/libraylib.a /usr/local/lib/libraylib.a
        endif
		@echo "raylib dev files installed/updated!"
    else
		@echo "This function works only on GNU/Linux systems"
    endif
else
	@echo "Error: no root permissions"
endif

# Remove raylib dev files installed on the system
# TODO: see 'install' target.
uninstall :
ifeq ($(ROOT),root)
    ifeq ($(PLATFORM_OS),LINUX)
			rm --force /usr/local/include/raylib.h
        ifeq ($(RAYLIB_LIBTYPE),SHARED)
			rm --force /usr/local/lib/libraylib.so
        else
			rm --force /usr/local/lib/libraylib.a
        endif
		@echo "raylib development files removed!"
    else
		@echo "This function works only on GNU/Linux systems"
    endif
else
	@echo "Error: no root permissions"
endif

# Clean everything
clean:
ifeq ($(PLATFORM_OS),WINDOWS)
	del *.o $(RAYLIB_RELEASE_PATH)/libraylib.a $(RAYLIB_RELEASE_PATH)/libraylib.bc $(RAYLIB_RELEASE_PATH)/libraylib.so external/stb_vorbis.o
else
	rm -f *.o $(RAYLIB_RELEASE_PATH)/libraylib.a $(RAYLIB_RELEASE_PATH)/libraylib.bc $(RAYLIB_RELEASE_PATH)/libraylib.so external/stb_vorbis.o
endif
ifeq ($(PLATFORM),PLATFORM_ANDROID)
	rm -rf $(ANDROID_TOOLCHAIN)
endif
	@echo "removed all generated files!"

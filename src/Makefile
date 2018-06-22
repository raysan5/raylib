#******************************************************************************
#
#  raylib makefile
#
#  Platforms supported:
#    PLATFORM_DESKTOP:  Windows (Win32, Win64)
#    PLATFORM_DESKTOP:  Linux (32 and 64 bit)
#    PLATFORM_DESKTOP:  OSX/macOS
#    PLATFORM_DESKTOP:  FreeBSD, OpenBSD, NetBSD, DragonFly
#    PLATFORM_ANDROID:  Android (ARM, ARM64)
#    PLATFORM_RPI:      Raspberry Pi (Raspbian)
#    PLATFORM_WEB:      HTML5 (Chrome, Firefox)
#
#  Many thanks to Milan Nikolic (@gen2brain) for implementing Android platform pipeline.
#  Many thanks to Emanuele Petriglia for his contribution on GNU/Linux pipeline.
#
#  Copyright (c) 2014-2018 Ramon Santamaria (@raysan5)
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

# Please read the wiki to know how to compile raylib, because there are different methods.
# https://github.com/raysan5/raylib/wiki

.PHONY: all clean install uninstall

# Define required raylib variables
RAYLIB_VERSION     = 2.0.0
RAYLIB_API_VERSION = 2

# See below for alternatives.
RAYLIB_PATH        = ..

# Define default options

# One of PLATFORM_DESKTOP, PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
PLATFORM             ?= PLATFORM_DESKTOP

# Library type used for raylib: STATIC (.a) or SHARED (.so/.dll)
RAYLIB_LIBTYPE       ?= STATIC

# Build mode for library: DEBUG or RELEASE
RAYLIB_BUILD_MODE    ?= RELEASE

# Included raylib audio module on compilation
# NOTE: Some programs like tools could not require audio support
INCLUDE_AUDIO_MODULE ?= TRUE

# Use OpenAL Soft backend for audio
USE_OPENAL_BACKEND   ?= FALSE

# OpenAL Soft audio backend forced on HTML5 and OSX (see below)
ifeq ($(PLATFORM),PLATFORM_WEB)
    USE_OPENAL_BACKEND = TRUE
endif

# Use external GLFW library instead of rglfw module
# TODO: Review usage of examples on Linux.
USE_EXTERNAL_GLFW    ?= FALSE

# Use Wayland display server protocol on Linux desktop
# by default it uses X11 windowing system
USE_WAYLAND_DISPLAY  ?= FALSE

# See below for more GRAPHICS options.

# See below for RAYLIB_RELEASE_PATH.

# See install target for *_INSTALL_PATH locations.

# Use cross-compiler for PLATFORM_RPI
ifeq ($(PLATFORM),PLATFORM_RPI)
    USE_RPI_CROSS_COMPILER ?= FALSE
    ifeq ($(USE_RPI_CROSS_COMPILER),TRUE)
        RPI_TOOLCHAIN ?= C:/SysGCC/Raspberry
        RPI_TOOLCHAIN_SYSROOT ?= $(RPI_TOOLCHAIN)/arm-linux-gnueabihf/sysroot
    endif
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
        endif
        ifeq ($(UNAMEOS),FreeBSD)
            PLATFORM_OS=BSD
        endif
        ifeq ($(UNAMEOS),OpenBSD)
            PLATFORM_OS=BSD
        endif
        ifeq ($(UNAMEOS),NetBSD)
            PLATFORM_OS=BSD
        endif
        ifeq ($(UNAMEOS),DragonFly)
            PLATFORM_OS=BSD
        endif
        ifeq ($(UNAMEOS),Darwin)
            PLATFORM_OS=OSX
        endif
    endif
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
    UNAMEOS=$(shell uname)
    ifeq ($(UNAMEOS),Linux)
        PLATFORM_OS=LINUX
    endif
endif

# RAYLIB_PATH adjustment for different platforms.
# If using GNU make, we can get the full path to the top of the tree. Windows? BSD?
# Required for ldconfig or other tools that do not perform path expansion.
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),LINUX)
        RAYLIB_PREFIX ?= ..
        RAYLIB_PATH    = $(realpath $(RAYLIB_PREFIX))
    endif
endif
# Default path for raylib on Raspberry Pi, if installed in different path, update it!
# TODO: update install: target in src/Makefile for RPI, consider relation to LINUX.
# WARNING: The following is copied from examples/Makefile and is here only for reference.
# Consequences of enabling this are UNKNOWN. Please test and report.
#ifeq ($(PLATFORM),PLATFORM_RPI)
#    RAYLIB_PATH       ?= /home/pi/raylib
#endif

# Force OpenAL Soft audio backend for OSX platform
# NOTE 1: mini_al library does not support CoreAudio yet
# NOTE 2: Required OpenAL libraries should be available on OSX
ifeq ($(PLATFORM_OS),OSX)
    USE_OPENAL_BACKEND = TRUE
endif

ifeq ($(PLATFORM),PLATFORM_WEB)
    # Emscripten required variables
    EMSDK_PATH = C:/emsdk
    EMSCRIPTEN_VERSION = 1.37.28
    CLANG_VERSION=e1.37.28_64bit
    PYTHON_VERSION=2.7.5.3_64bit
    NODE_VERSION=4.1.1_64bit
    export PATH=$(EMSDK_PATH);$(EMSDK_PATH)\clang\$(CLANG_VERSION);$(EMSDK_PATH)\node\$(NODE_VERSION)\bin;$(EMSDK_PATH)\python\$(PYTHON_VERSION);$(EMSDK_PATH)\emscripten\$(EMSCRIPTEN_VERSION);C:\raylib\MinGW\bin:$$(PATH)
    EMSCRIPTEN=$(EMSDK_PATH)\emscripten\$(EMSCRIPTEN_VERSION)
endif

ifeq ($(PLATFORM),PLATFORM_ANDROID)
    # Android required path variables
    ANDROID_NDK = C:/android-ndk
    ANDROID_TOOLCHAIN = C:/android_toolchain_arm64_api21

    # Android architecture: ARM64
    # Starting at 2019 using ARM64 is mandatory for published apps
    ANDROID_ARCH ?= ARM64
endif

# RAYLIB_RELEASE_PATH points to provided binaries or your immediate build of raylib.
# It is further modified below by PLATFORM below.
RAYLIB_RELEASE_PATH  ?= $(RAYLIB_PATH)/release/libs

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
    ifeq ($(PLATFORM_OS),BSD)
        RAYLIB_RELEASE_PATH = $(RAYLIB_PATH)/release/libs/bsd
    endif
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
    RAYLIB_RELEASE_PATH = $(RAYLIB_PATH)/release/libs/rpi
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    RAYLIB_RELEASE_PATH = $(RAYLIB_PATH)/release/libs/html5
endif
ifeq ($(PLATFORM),PLATFORM_ANDROID)
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

# Define default C compiler: gcc
# NOTE: define g++ compiler if using C++
CC = gcc

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),OSX)
        # OSX default compiler
        CC = clang
        GLFW_CFLAGS = -x objective-c
    endif
    ifeq ($(PLATFORM_OS),BSD)
        # FreeBSD, OpenBSD, NetBSD, DragonFly default compiler
        CC = clang
    endif
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
    ifeq ($(USE_RPI_CROSS_COMPILER),TRUE)
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
    ifeq ($(ANDROID_ARCH),ARM64)
        CC = $(ANDROID_TOOLCHAIN)/bin/aarch64-linux-android-clang
    endif
endif

# Default archiver program to pack libraries
AR = ar

ifeq ($(PLATFORM),PLATFORM_RPI)
    ifeq ($(USE_RPI_CROSS_COMPILER),TRUE)
        # Define RPI cross-archiver
        #CC = armv6j-hardfloat-linux-gnueabi-gcc
        AR = $(RPI_TOOLCHAIN)/bin/arm-linux-gnueabihf-ar
    endif
endif

# Android archiver (also depends on desired architecture)
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    ifeq ($(ANDROID_ARCH),ARM64)
        AR = $(ANDROID_TOOLCHAIN)/bin/aarch64-linux-android-ar
    endif
endif

# Define compiler flags:
#  -O1                   defines optimization level
#  -g                    enable debugging
#  -s                    strip unnecessary data from build
#  -Wall                 turns on most, but not all, compiler warnings
#  -std=c99              defines C language mode (standard C from 1999 revision)
#  -std=gnu99            defines C language mode (GNU C from 1999 revision)
#  -Wno-missing-braces   ignore invalid warning (GCC bug 53119)
#  -D_DEFAULT_SOURCE     use with -std=c99 on Linux and PLATFORM_WEB, required for timespec
#  -Werror=pointer-arith catch unportable code that does direct arithmetic on void pointers
#  -fno-strict-aliasing  jar_xm.h does shady stuff (breaks strict aliasing)
CFLAGS += -O1 -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing

ifeq ($(RAYLIB_BUILD_MODE), DEBUG)
    CFLAGS += -g
    #CC      = clang
endif

# Additional flags for compiler (if desired)
#  -Wextra                  enables some extra warning flags that are not enabled by -Wall
#  -Wmissing-prototypes     warn if a global function is defined without a previous prototype declaration
#  -Wstrict-prototypes      warn if a function is declared or defined without specifying the argument types
#  -Werror=implicit-function-declaration   catch function calls without prior declaration
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    CFLAGS += -Werror=implicit-function-declaration
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    # -O2                        # if used, also set --memory-init-file 0
    # --memory-init-file 0       # to avoid an external memory initialization code file (.mem)
    # -s ALLOW_MEMORY_GROWTH=1   # to allow memory resizing
    # -s TOTAL_MEMORY=16777216   # to specify heap memory size (default = 16MB)
    # -s USE_PTHREADS=1          # multithreading support
    CFLAGS += -s USE_GLFW=3 -s ASSERTIONS=1 --profiling
endif
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    # Compiler flags for arquitecture (only ARM, not ARM64)
    #CFLAGS += -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16
    CFLAGS += -target aarch64 -mfix-cortex-a53-835769
    # Compilation functions attributes options
    CFLAGS += -ffunction-sections -funwind-tables -fstack-protector-strong -fPIE -fPIC
    # Compiler options for the linker
    # -Werror=format-security
    CFLAGS += -Wa,--noexecstack -Wformat -no-canonical-prefixes
    # Preprocessor macro definitions
    CFLAGS += -DANDROID -DPLATFORM_ANDROID -D__ANDROID_API__=21 -DMAL_NO_OSS
endif

# Define required compilation flags for raylib SHARED lib
ifeq ($(RAYLIB_LIBTYPE),SHARED)
    # make sure code is compiled as position independent
    # BE CAREFUL: It seems that for gcc -fpic is not the same as -fPIC
    # MinGW32 just doesn't need -fPIC, it shows warnings
    CFLAGS += -fPIC -DBUILD_LIBTYPE_SHARED
endif

# Use OpenAL Soft backend instead of mini_al
ifeq ($(USE_OPENAL_BACKEND),TRUE)
    CFLAGS += -DUSE_OPENAL_BACKEND
endif

# Use Wayland display on Linux desktop
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS), LINUX)
        ifeq ($(USE_WAYLAND_DISPLAY),TRUE)
            CFLAGS += -D_GLFW_WAYLAND
        endif
    endif
endif

# Define include paths for required headers
# NOTE: Several external required libraries (stb and others)
INCLUDE_PATHS = -I. -Iexternal -Iexternal/glfw/include

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),BSD)
        INCLUDE_PATHS += -I/usr/local/include
        LDFLAGS += -L. -Lsrc -L/usr/local/lib -L$(RAYLIB_RELEASE_PATH)
    endif
    ifeq ($(USE_EXTERNAL_GLFW),TRUE)
        # Check the version name. If GLFW3 was built manually, it may have produced
        # a static library known as libglfw3.a. In that case, the name should be -lglfw3
        LDFLAGS += -lglfw
    endif
endif

# Define additional directories containing required header files
ifeq ($(PLATFORM),PLATFORM_RPI)
    # RPI required libraries
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
    LDFLAGS = -Wl,-soname,libraylib.$(API_VERSION).so -Wl,--exclude-libs,libatomic.a
    LDFLAGS += -Wl,--build-id -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -Wl,--warn-shared-textrel -Wl,--fatal-warnings
    # Force linking of library module to define symbol
    LDFLAGS += -u ANativeActivity_onCreate
    # Library paths containing required libs
    LDFLAGS += -L. -Lsrc -L$(RAYLIB_RELEASE_PATH)
    # Avoid unresolved symbol pointing to external main()
    LDFLAGS += -Wl,-undefined,dynamic_lookup

    LDLIBS = -llog -landroid -lEGL -lGLESv2 -lOpenSLES -latomic -lc -lm
endif

# Define all object files required with a wildcard
# The wildcard takes all files that finish with ".c",
# and replaces extentions with ".o", that are the object files
# NOTE: Some objects depend on the PLATFORM to be added or not!
# OBJS = $(patsubst %.c, %.o, $(wildcard *.c))

# Define object required on compilation
OBJS = core.o \
       rlgl.o \
       shapes.o \
       textures.o \
       text.o \
       models.o \
       utils.o

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(USE_EXTERNAL_GLFW),FALSE)
        OBJS += rglfw.o
    endif
endif

ifeq ($(INCLUDE_AUDIO_MODULE),TRUE)
    OBJS += audio.o
    OBJS += stb_vorbis.o
    ifeq ($(USE_OPENAL_BACKEND),FALSE)
        OBJS += mini_al.o
    endif
endif

ifeq ($(PLATFORM),PLATFORM_ANDROID)
    OBJS += external/android/native_app_glue/android_native_app_glue.o
endif

# Default target entry
all: raylib

# Generate standalone Android toolchain
# NOTE: Android toolchain could already be provided
generate_android_toolchain:
ifeq ($(PLATFORM),PLATFORM_ANDROID)
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
        ifeq ($(PLATFORM_OS),WINDOWS)
			$(CC) -shared -o $(RAYLIB_RELEASE_PATH)/raylib.dll $(OBJS) -L$(RAYLIB_RELEASE_PATH) -static-libgcc -lopengl32 -lgdi32 -Wl,--out-implib,$(RAYLIB_RELEASE_PATH)/libraylibdll.a
			@echo "raylib dynamic library (raylib.dll) and import library (libraylibdll.a) generated!"
        endif
        ifeq ($(PLATFORM_OS),LINUX)
            # Compile raylib shared library version $(RAYLIB_VERSION).
            # WARNING: you should type "make clean" before doing this target
			$(CC) -shared -o $(RAYLIB_RELEASE_PATH)/libraylib.so.$(RAYLIB_VERSION) $(OBJS) -shared -Wl,-soname,libraylib.so.$(RAYLIB_API_VERSION) -lGL -lc -lm -lpthread -ldl -lrt
			@echo "raylib shared library generated (libraylib.so.$(RAYLIB_VERSION)) in $(RAYLIB_RELEASE_PATH)!"
			cd $(RAYLIB_RELEASE_PATH) && ln -fsv libraylib.so.$(RAYLIB_VERSION) libraylib.so.$(RAYLIB_API_VERSION)
			cd $(RAYLIB_RELEASE_PATH) && ln -fsv libraylib.so.$(RAYLIB_API_VERSION) libraylib.so
        endif
        ifeq ($(PLATFORM_OS),OSX)
			$(CC) -dynamiclib -o $(RAYLIB_RELEASE_PATH)/libraylib.$(RAYLIB_VERSION).dylib $(OBJS) -compatibility_version $(RAYLIB_API_VERSION) -current_version $(RAYLIB_VERSION) -framework OpenGL -framework OpenAL -framework IOKit -framework CoreVideo -framework Cocoa
			install_name_tool -id "libraylib.$(VERSION).dylib" $(RAYLIB_RELEASE_PATH)/libraylib.$(RAYLIB_VERSION).dylib
			@echo "raylib shared library generated (libraylib.$(RAYLIB_VERSION).dylib)!"
			cd $(RAYLIB_RELEASE_PATH) && ln -fs libraylib.$(RAYLIB_VERSION).dylib libraylib.$(RAYLIB_API_VERSION).dylib
			cd $(RAYLIB_RELEASE_PATH) && ln -fs libraylib.$(RAYLIB_VERSION).dylib libraylib.dylib
        endif
        ifeq ($(PLATFORM_OS),BSD)
            # WARNING: you should type "gmake clean" before doing this target
			$(CC) -shared -o $(RAYLIB_RELEASE_PATH)/libraylib.$(RAYLIB_VERSION).so $(OBJS) -Wl,-soname,libraylib.$(RAYLIB_API_VERSION).so -lGL -lpthread
			@echo "raylib shared library generated (libraylib.$(RAYLIB_VERSION).so)!"
			cd $(RAYLIB_RELEASE_PATH) && ln -fs libraylib.$(RAYLIB_VERSION).so libraylib.$(RAYLIB_API_VERSION).so
			cd $(RAYLIB_RELEASE_PATH) && ln -fs libraylib.$(RAYLIB_VERSION).so libraylib.so
        endif
        ifeq ($(PLATFORM),PLATFORM_ANDROID)
			$(CC) -shared -o $(RAYLIB_RELEASE_PATH)/libraylib.$(RAYLIB_VERSION).so $(OBJS) $(LDFLAGS) $(LDLIBS)
			@echo "raylib shared library generated (libraylib.$(RAYLIB_VERSION).so)!"
			# WARNING: symbolic links creation on Windows should be done using mklink command, no ln available
			cd $(RAYLIB_RELEASE_PATH) && ln -fs libraylib.$(RAYLIB_VERSION).so libraylib.$(RAYLIB_API_VERSION).so
			cd $(RAYLIB_RELEASE_PATH) && ln -fs libraylib.$(RAYLIB_VERSION).so libraylib.so
        endif
    else
        # Compile raylib static library version $(RAYLIB_VERSION)
        # WARNING: You should type "make clean" before doing this target.
		$(AR) rcs $(RAYLIB_RELEASE_PATH)/libraylib.a $(OBJS)
		@echo "raylib static library generated (libraylib.a) in $(RAYLIB_RELEASE_PATH)!"
    endif
endif

# Compile all modules with their prerequisites

# Compile core module
core.o : core.c raylib.h rlgl.h utils.h raymath.h gestures.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM) -D$(GRAPHICS)

# Compile rglfw module
rglfw.o : rglfw.c
	$(CC) $(GLFW_CFLAGS) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM) -D$(GRAPHICS)

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
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM)

# Compile mini_al audio library
mini_al.o : external/mini_al.c external/mini_al.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM)

# Compile stb_vorbis library
stb_vorbis.o: external/stb_vorbis.c external/stb_vorbis.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM)

# Compile utils module
utils.o : utils.c utils.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM)

# Install generated and needed files to desired directories.
# On GNU/Linux and BSDs, there are some standard directories that contain extra
# libraries and header files. These directories (often /usr/local/lib and
# /usr/local/include) are for libraries that are installed manually
# (without a package manager). We'll use /usr/local/lib/raysan5 and /usr/local/include/raysan5
# for our -L and -I specification to simplify management of the raylib source package.
# Customize these locations if you like but don't forget to pass them to make
# for compilation and enable runtime linking with -rpath, LD_LIBRARY_PATH, or ldconfig.
# Hint: add -L$(RAYLIB_INSTALL_PATH) -I$(RAYLIB_H_INSTALL_PATH) to your own makefiles.
# See below and ../examples/Makefile for more information.
# TODO: Add other platforms. Remove sudo requirement, i.e. add USER mode.

# RAYLIB_INSTALL_PATH should be the desired full path to libraylib. No relative paths.
DESTDIR ?= /usr/local
RAYLIB_INSTALL_PATH ?= $(DESTDIR)/lib
# RAYLIB_H_INSTALL_PATH locates the installed raylib header and associated source files.
RAYLIB_H_INSTALL_PATH ?= $(DESTDIR)/include

install :
ifeq ($(ROOT),root)
    ifeq ($(PLATFORM_OS),LINUX)
        # Attention! You are root, writing files to $(RAYLIB_INSTALL_PATH)
        # and $(RAYLIB_H_INSTALL_PATH). Consult this Makefile for more information.
        # Prepare the environment as needed.
		mkdir --parents --verbose $(RAYLIB_INSTALL_PATH)
		mkdir --parents --verbose $(RAYLIB_H_INSTALL_PATH)
        ifeq ($(RAYLIB_LIBTYPE),SHARED)
            # Installing raylib to $(RAYLIB_INSTALL_PATH).
			cp --update --verbose $(RAYLIB_RELEASE_PATH)/libraylib.so.$(RAYLIB_VERSION) $(RAYLIB_INSTALL_PATH)/libraylib.so.$(RAYLIB_VERSION)
			cd $(RAYLIB_INSTALL_PATH); ln -fsv libraylib.so.$(RAYLIB_VERSION) libraylib.so.$(RAYLIB_API_VERSION)
			cd $(RAYLIB_INSTALL_PATH); ln -fsv libraylib.so.$(RAYLIB_API_VERSION) libraylib.so
            # Uncomment to update the runtime linker cache with RAYLIB_INSTALL_PATH.
            # Not necessary if later embedding RPATH in your executable. See examples/Makefile.
			ldconfig $(RAYLIB_INSTALL_PATH)
        else
            # Installing raylib to $(RAYLIB_INSTALL_PATH).
			cp --update --verbose $(RAYLIB_RELEASE_PATH)/libraylib.a $(RAYLIB_INSTALL_PATH)/libraylib.a
        endif
        # Copying raylib development files to $(RAYLIB_H_INSTALL_PATH).
		cp --update raylib.h $(RAYLIB_H_INSTALL_PATH)/raylib.h
		cp --update raymath.h $(RAYLIB_H_INSTALL_PATH)/raymath.h
		cp --update rlgl.h $(RAYLIB_H_INSTALL_PATH)/rlgl.h
		cp --update rlgl.c $(RAYLIB_H_INSTALL_PATH)/rlgl.c
		cp --update physac.h $(RAYLIB_H_INSTALL_PATH)/physac.h
		@echo "raylib development files installed/updated!"
    else
		@echo "This function currently works on GNU/Linux systems. Add yours today (^;"
    endif
else
	@echo "Error: Root permissions needed for installation. Try sudo make install"
endif

# Remove raylib dev files installed on the system
# TODO: see 'install' target.
uninstall :
ifeq ($(ROOT),root)
    # WARNING: You are root, about to delete items from $(RAYLIB_INSTALL_PATH).
    # and $(RAYLIB_H_INSTALL_PATH). Please confirm each item.
    ifeq ($(PLATFORM_OS),LINUX)
        ifeq ($(RAYLIB_LIBTYPE),SHARED)
		rm --force --interactive --verbose $(RAYLIB_INSTALL_PATH)/libraylib.so
		rm --force --interactive --verbose $(RAYLIB_INSTALL_PATH)/libraylib.so.$(RAYLIB_API_VERSION)
		rm --force --interactive --verbose $(RAYLIB_INSTALL_PATH)/libraylib.so.$(RAYLIB_VERSION)
        # Uncomment to clean up the runtime linker cache. See install target.
		ldconfig
        else
		rm --force --interactive --verbose $(RAYLIB_INSTALL_PATH)/libraylib.a
        endif
		rm --force --interactive --verbose $(RAYLIB_H_INSTALL_PATH)/raylib.h
		rm --force --interactive --verbose $(RAYLIB_H_INSTALL_PATH)/raymath.h
		rm --force --interactive --verbose $(RAYLIB_H_INSTALL_PATH)/rlgl.h
		rm --force --interactive --verbose $(RAYLIB_H_INSTALL_PATH)/rlgl.c
		rm --force --interactive --verbose $(RAYLIB_H_INSTALL_PATH)/physac.h
		@echo "raylib development files removed!"
        else
		@echo "This function currently works on GNU/Linux systems. Add yours today (^;"
    endif
else
	@echo "Error: Root permissions needed for uninstallation. Try sudo make uninstall"
endif

# Clean everything
clean:
ifeq ($(PLATFORM_OS),WINDOWS)
	del *.o $(RAYLIB_RELEASE_PATH)/libraylib.a $(RAYLIB_RELEASE_PATH)/libraylib.bc $(RAYLIB_RELEASE_PATH)/libraylib.so external/stb_vorbis.o
else
	rm -fv *.o $(RAYLIB_RELEASE_PATH)/libraylib.a $(RAYLIB_RELEASE_PATH)/libraylib.bc $(RAYLIB_RELEASE_PATH)/libraylib.so* external/stb_vorbis.o
endif
ifeq ($(PLATFORM),PLATFORM_ANDROID)
	rm -rf $(ANDROID_TOOLCHAIN)
endif
	@echo "removed all generated files!"

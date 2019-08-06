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
#  Copyright (c) 2014-2019 Ramon Santamaria (@raysan5)
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
RAYLIB_VERSION     = 2.5.0
RAYLIB_API_VERSION = 251

# See below for alternatives.
RAYLIB_PATH        = ..

# Define default options

# One of PLATFORM_DESKTOP, PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
PLATFORM             ?= PLATFORM_DESKTOP

# Library type used for raylib: STATIC (.a) or SHARED (.so/.dll)
RAYLIB_LIBTYPE       ?= STATIC

# Build mode for library: DEBUG or RELEASE
RAYLIB_BUILD_MODE    ?= RELEASE

# Include raylib modules on compilation
# NOTE: Some programs like tools could not require those modules
RAYLIB_MODULE_AUDIO  ?= TRUE
RAYLIB_MODULE_MODELS ?= TRUE
RAYLIB_MODULE_RAYGUI ?= FALSE
RAYLIB_MODULE_PHYSAC ?= FALSE

RAYLIB_MODULE_RAYGUI_PATH ?= .
RAYLIB_MODULE_PHYSAC_PATH ?= .

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

# By default we suppose we are working on Windows
HOST_PLATFORM_OS ?= WINDOWS

# Determine PLATFORM_OS in case PLATFORM_DESKTOP selected
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
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
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
    UNAMEOS = $(shell uname)
    ifeq ($(UNAMEOS),Linux)
        PLATFORM_OS = LINUX
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

ifeq ($(PLATFORM),PLATFORM_WEB)
    # Emscripten required variables
    EMSDK_PATH          ?= C:/emsdk
    EMSCRIPTEN_VERSION  ?= 1.38.32
    CLANG_VERSION       = e$(EMSCRIPTEN_VERSION)_64bit
    PYTHON_VERSION      = 2.7.13.1_64bit\python-2.7.13.amd64
    NODE_VERSION        = 8.9.1_64bit
    export PATH         = $(EMSDK_PATH);$(EMSDK_PATH)\clang\$(CLANG_VERSION);$(EMSDK_PATH)\node\$(NODE_VERSION)\bin;$(EMSDK_PATH)\python\$(PYTHON_VERSION);$(EMSDK_PATH)\emscripten\$(EMSCRIPTEN_VERSION);C:\raylib\MinGW\bin:$$(PATH)
    EMSCRIPTEN          = $(EMSDK_PATH)\emscripten\$(EMSCRIPTEN_VERSION)
endif

ifeq ($(PLATFORM),PLATFORM_ANDROID)
    # Android architecture: ARM64
    # Starting at 2019 using ARM64 is mandatory for published apps
    ANDROID_ARCH ?= ARM
    ANDROID_API_VERSION = 26

    # Android required path variables
    # NOTE: Android NDK is just required to generate the standalone toolchain,
    # in case is not already provided
    ifeq ($(OS),Windows_NT)
        ANDROID_NDK = C:/android-ndk
    else
        ANDROID_NDK = /usr/lib/android/ndk
    endif

    # Android standalone toolchain path
    ifeq ($(OS),Windows_NT)
        ANDROID_TOOLCHAIN = C:/android_toolchain_$(ANDROID_ARCH)_API$(ANDROID_API_VERSION)
    else
        ANDROID_TOOLCHAIN = /usr/lib/android/toolchain_$(ANDROID_ARCH)_API$(ANDROID_API_VERSION)
    endif

    ifeq ($(ANDROID_ARCH),ARM)
        ANDROID_ARCH_NAME = armeabi-v7a
    endif
    ifeq ($(ANDROID_ARCH),ARM64)
        ANDROID_ARCH_NAME = arm64-v8a
    endif
    ifeq ($(ANDROID_ARCH),x86)
        ANDROID_ARCH_NAME = i686
    endif
    ifeq ($(ANDROID_ARCH),x86_64)
        ANDROID_ARCH_NAME = x86_64
    endif
endif

# Define raylib source code path
RAYLIB_SRC_PATH ?= $(RAYLIB_PATH)/src

# Define output directory for compiled library, defaults to src directory
# NOTE: If externally provided, make sure directory exists
RAYLIB_RELEASE_PATH  ?= $(RAYLIB_PATH)/src

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

# Define default C compiler and archiver to pack library
CC ?= gcc
AR = ar

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
        AR = $(RPI_TOOLCHAIN)/bin/arm-linux-gnueabihf-ar
    endif
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    # HTML5 emscripten compiler
    CC = emcc
endif
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    # Android toolchain (must be provided for desired architecture and compiler)
    ifeq ($(ANDROID_ARCH),ARM)
        CC = $(ANDROID_TOOLCHAIN)/bin/arm-linux-androideabi-clang
        AR = $(ANDROID_TOOLCHAIN)/bin/arm-linux-androideabi-ar
    endif
    ifeq ($(ANDROID_ARCH),ARM64)
        CC = $(ANDROID_TOOLCHAIN)/bin/aarch64-linux-android-clang
        AR = $(ANDROID_TOOLCHAIN)/bin/aarch64-linux-android-ar
    endif
    ifeq ($(ANDROID_ARCH),x86)
        CC = $(ANDROID_TOOLCHAIN)/bin/i686-linux-android$(ANDROID_API_VERSION)-clang
        AR = $(ANDROID_TOOLCHAIN)/bin/i686-linux-android-ar
    endif
    ifeq ($(ANDROID_ARCH),x86_64)
        CC = $(ANDROID_TOOLCHAIN)/bin/x86_64-linux-android$(ANDROID_API_VERSION)-clang
        AR = $(ANDROID_TOOLCHAIN)/bin/x86_64-linux-android-ar
    endif
endif


# Define compiler flags:
#  -O1                      defines optimization level
#  -g                       include debug information on compilation
#  -s                       strip unnecessary data from build
#  -Wall                    turns on most, but not all, compiler warnings
#  -std=c99                 defines C language mode (standard C from 1999 revision)
#  -std=gnu99               defines C language mode (GNU C from 1999 revision)
#  -Wno-missing-braces      ignore invalid warning (GCC bug 53119)
#  -D_DEFAULT_SOURCE        use with -std=c99 on Linux and PLATFORM_WEB, required for timespec
#  -Werror=pointer-arith    catch unportable code that does direct arithmetic on void pointers
#  -fno-strict-aliasing     jar_xm.h does shady stuff (breaks strict aliasing)
CFLAGS += -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -Werror=pointer-arith -fno-strict-aliasing

ifeq ($(PLATFORM_OS),LINUX)
	CFLAGS += -fPIC
endif

ifeq ($(RAYLIB_BUILD_MODE),DEBUG)
    CFLAGS += -g
endif
ifeq ($(RAYLIB_BUILD_MODE),RELEASE)
    CFLAGS += -O1
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
    # -Os                        # size optimization
    # -O2                        # optimization level 2, if used, also set --memory-init-file 0
    # -s USE_GLFW=3              # Use glfw3 library (context/input management)
    # -s ALLOW_MEMORY_GROWTH=1   # to allow memory resizing -> WARNING: Audio buffers could FAIL!
    # -s TOTAL_MEMORY=16777216   # to specify heap memory size (default = 16MB)
    # -s USE_PTHREADS=1          # multithreading support
    # -s WASM=0                  # disable Web Assembly, emitted by default
    # -s EMTERPRETIFY=1          # enable emscripten code interpreter (very slow)
    # -s EMTERPRETIFY_ASYNC=1    # support synchronous loops by emterpreter
    # -s FORCE_FILESYSTEM=1      # force filesystem to load/save files data
    # -s ASSERTIONS=1            # enable runtime checks for common memory allocation errors (-O1 and above turn it off)
    # --profiling                # include information for code profiling
    # --memory-init-file 0       # to avoid an external memory initialization code file (.mem)
    # --preload-file resources   # specify a resources folder for data compilation
    CFLAGS += -s USE_GLFW=3
    ifeq ($(RAYLIB_BUILD_MODE),DEBUG)
        CFLAGS += -s ASSERTIONS=1 --profiling
    endif
endif
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    # Compiler flags for arquitecture
    ifeq ($(ANDROID_ARCH),ARM)
        CFLAGS += -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16
    endif
    ifeq ($(ANDROID_ARCH),ARM64)
        CFLAGS += -target aarch64 -mfix-cortex-a53-835769
    endif
    ifeq ($(ANDROID_ARCH), x86)
        CFLAGS += -march=i686
    endif
    ifeq ($(ANDROID_ARCH), x86_64)
        CFLAGS += -march=x86-64
    endif
    # Compilation functions attributes options
    CFLAGS += -ffunction-sections -funwind-tables -fstack-protector-strong -fPIE -fPIC
    # Compiler options for the linker
    # -Werror=format-security
    CFLAGS += -Wa,--noexecstack -Wformat -no-canonical-prefixes
    # Preprocessor macro definitions
    CFLAGS += -DANDROID -DPLATFORM_ANDROID -D__ANDROID_API__=$(ANDROID_API_VERSION) -DMAL_NO_OSS
endif

# Define required compilation flags for raylib SHARED lib
ifeq ($(RAYLIB_LIBTYPE),SHARED)
    # make sure code is compiled as position independent
    # BE CAREFUL: It seems that for gcc -fpic is not the same as -fPIC
    # MinGW32 just doesn't need -fPIC, it shows warnings
    CFLAGS += -fPIC -DBUILD_LIBTYPE_SHARED
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
INCLUDE_PATHS = -I. -Iexternal/glfw/include -Iexternal/glfw/deps/mingw

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
    NATIVE_APP_GLUE = $(RAYLIB_RELEASE_PATH)/external/android/native_app_glue
    #NATIVE_APP_GLUE = $(ANDROID_NDK)/sources/android/native_app_glue
    # Android required libraries
    INCLUDE_PATHS += -I$(ANDROID_TOOLCHAIN)/sysroot/usr/include
    # Include android_native_app_glue.h
    INCLUDE_PATHS += -I$(NATIVE_APP_GLUE)
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

    LDLIBS = -llog -landroid -lEGL -lGLESv2 -lOpenSLES -lc -lm
endif

# Define all object files required with a wildcard
# The wildcard takes all files that finish with ".c",
# and replaces extentions with ".o", that are the object files
# NOTE: Some objects depend on the PLATFORM to be added or not!
# OBJS = $(patsubst %.c, %.o, $(wildcard *.c))

# Define object required on compilation
OBJS = core.o \
       shapes.o \
       textures.o \
       text.o \
       utils.o

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(USE_EXTERNAL_GLFW),FALSE)
        OBJS += rglfw.o
    endif
endif
ifeq ($(RAYLIB_MODULE_MODELS),TRUE)
    OBJS += models.o
endif
ifeq ($(RAYLIB_MODULE_AUDIO),TRUE)
    OBJS += raudio.o
endif
ifeq ($(RAYLIB_MODULE_RAYGUI),TRUE)
    OBJS += raygui.o
endif
ifeq ($(RAYLIB_MODULE_PHYSAC),TRUE)
    OBJS += physac.o
endif

ifeq ($(PLATFORM),PLATFORM_ANDROID)
    OBJS += external/android/native_app_glue/android_native_app_glue.o
endif

# Default target entry
all: raylib

# Generate standalone Android toolchain
# NOTE: If the host system is Windows or Linux, generation is different
generate_android_toolchain:
ifeq ($(PLATFORM_OS),WINDOWS)
    # NOTE: python should be available in the path
    ifeq ($(ANDROID_ARCH),ARM)
		$(ANDROID_NDK)/build/tools/python make_standalone_toolchain.py --arch arm --api $(ANDROID_API_VERSION) --install-dir $(ANDROID_TOOLCHAIN)
    endif
    ifeq ($(ANDROID_ARCH),ARM64)
		$(ANDROID_NDK)/build/tools/python make_standalone_toolchain.py --arch arm64 --api $(ANDROID_API_VERSION) --install-dir $(ANDROID_TOOLCHAIN)
    endif
endif
ifeq ($(PLATFORM_OS),LINUX)
    ifeq ($(ANDROID_ARCH),ARM)
		$(ANDROID_NDK)/build/tools/make-standalone-toolchain.sh --platform=android-$(ANDROID_API_VERSION) --toolchain=arm-linux-androideabi-4.9 --use-llvm --install-dir=$(ANDROID_TOOLCHAIN)
    endif
    ifeq ($(ANDROID_ARCH),ARM64)
		$(ANDROID_NDK)/build/tools/make-standalone-toolchain.sh --platform=android-$(ANDROID_API_VERSION) --toolchain=aarch64-linux-androideabi-4.9 --use-llvm --install-dir=$(ANDROID_TOOLCHAIN)
    endif
endif

# Compile raylib library
# NOTE: Release directory is created if not exist
raylib: $(OBJS)
ifeq ($(PLATFORM),PLATFORM_WEB)
    # Compile raylib for web.
	emcc -O1 $(OBJS) -o $(RAYLIB_RELEASE_PATH)/libraylib.bc
	@echo "raylib library generated (libraylib.bc)!"
else
    ifeq ($(RAYLIB_LIBTYPE),SHARED)
        ifeq ($(PLATFORM),PLATFORM_DESKTOP)
            ifeq ($(PLATFORM_OS),WINDOWS)
                # TODO: Compile resource file raylib.dll.rc for linkage on raylib.dll generation
				$(CC) -shared -o $(RAYLIB_RELEASE_PATH)/raylib.dll $(OBJS) -L$(RAYLIB_RELEASE_PATH) -static-libgcc -lopengl32 -lgdi32 -lwinmm -Wl,--out-implib,$(RAYLIB_RELEASE_PATH)/libraylibdll.a
				@echo "raylib dynamic library (raylib.dll) and import library (libraylibdll.a) generated!"
            endif
            ifeq ($(PLATFORM_OS),LINUX)
                # Compile raylib shared library version $(RAYLIB_VERSION).
                # WARNING: you should type "make clean" before doing this target
				$(CC) -shared -o $(RAYLIB_RELEASE_PATH)/libraylib.so.$(RAYLIB_VERSION) $(OBJS) -Wl,-soname,libraylib.so.$(RAYLIB_API_VERSION) -lGL -lc -lm -lpthread -ldl -lrt
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
        endif
        ifeq ($(PLATFORM),PLATFORM_RPI)
                # Compile raylib shared library version $(RAYLIB_VERSION).
                # WARNING: you should type "make clean" before doing this target
				$(CC) -shared -o $(RAYLIB_RELEASE_PATH)/libraylib.so.$(RAYLIB_VERSION) $(OBJS) -Wl,-soname,libraylib.so.$(RAYLIB_API_VERSION) -L/opt/vc/lib -lbrcmGLESv2 -lbrcmEGL -lpthread -lrt -lm -lbcm_host -ldl
				@echo "raylib shared library generated (libraylib.so.$(RAYLIB_VERSION)) in $(RAYLIB_RELEASE_PATH)!"
				cd $(RAYLIB_RELEASE_PATH) && ln -fsv libraylib.so.$(RAYLIB_VERSION) libraylib.so.$(RAYLIB_API_VERSION)
				cd $(RAYLIB_RELEASE_PATH) && ln -fsv libraylib.so.$(RAYLIB_API_VERSION) libraylib.so
        endif
        ifeq ($(PLATFORM),PLATFORM_ANDROID)
			$(CC) -shared -o $(RAYLIB_RELEASE_PATH)/libraylib.$(RAYLIB_VERSION).so $(OBJS) $(LDFLAGS) $(LDLIBS)
			@echo "raylib shared library generated (libraylib.$(RAYLIB_VERSION).so)!"
            # WARNING: symbolic links creation on Windows should be done using mklink command, no ln available
            ifeq ($(HOST_PLATFORM_OS),LINUX)
				cd $(RAYLIB_RELEASE_PATH) && ln -fs libraylib.$(RAYLIB_VERSION).so libraylib.$(RAYLIB_API_VERSION).so
				cd $(RAYLIB_RELEASE_PATH) && ln -fs libraylib.$(RAYLIB_VERSION).so libraylib.so
            endif
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
core.o : core.c raylib.h rlgl.h utils.h raymath.h camera.h gestures.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM) -D$(GRAPHICS)

# Compile rglfw module
rglfw.o : rglfw.c
	$(CC) $(CFLAGS) $(GLFW_CFLAGS) -c $<  $(INCLUDE_PATHS) -D$(PLATFORM) -D$(GRAPHICS)

# Compile shapes module
shapes.o : shapes.c raylib.h rlgl.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM) -D$(GRAPHICS)

# Compile textures module
textures.o : textures.c raylib.h rlgl.h utils.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM) -D$(GRAPHICS)

# Compile text module
text.o : text.c raylib.h utils.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM) -D$(GRAPHICS)

# Compile utils module
utils.o : utils.c utils.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM)

# Compile models module
models.o : models.c raylib.h rlgl.h raymath.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM) -D$(GRAPHICS)

# Compile audio module
raudio.o : raudio.c raylib.h
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM)

# Compile raygui module
# NOTE: raygui header should be distributed with raylib.h
raygui.o : raygui.c raygui.h
	@echo #define RAYGUI_IMPLEMENTATION > raygui.c
	@echo #include "$(RAYLIB_MODULE_RAYGUI_PATH)/raygui.h" > raygui.c
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM) -DRAYGUI_IMPLEMENTATION

# Compile physac module
# NOTE: physac header should be distributed with raylib.h
physac.o : physac.c physac.h
	@echo #define PHYSAC_IMPLEMENTATION > physac.c
	@echo #include "$(RAYLIB_MODULE_PHYSAC_PATH)/physac.h" > physac.c
	$(CC) -c $< $(CFLAGS) $(INCLUDE_PATHS) -D$(PLATFORM) -DPHYSAC_IMPLEMENTATION


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
	del *.o $(RAYLIB_RELEASE_PATH)/libraylib.a $(RAYLIB_RELEASE_PATH)/libraylib.bc $(RAYLIB_RELEASE_PATH)/libraylib.so
else
	rm -fv *.o $(RAYLIB_RELEASE_PATH)/libraylib.a $(RAYLIB_RELEASE_PATH)/libraylib.bc $(RAYLIB_RELEASE_PATH)/libraylib.so*
endif
ifeq ($(PLATFORM),PLATFORM_ANDROID)
	rm -rf $(ANDROID_TOOLCHAIN) $(NATIVE_APP_GLUE)/android_native_app_glue.o
endif
	@echo "removed all generated files!"

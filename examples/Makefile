#**************************************************************************************************
#
#   raylib makefile for Desktop platforms, Raspberry Pi, Android and HTML5
#
#   Copyright (c) 2013-2018 Ramon Santamaria (@raysan5)
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

# Define required raylib variables
# WARNING: To compile to HTML5, code must be redesigned to use emscripten.h and emscripten_set_main_loop()
PLATFORM ?= PLATFORM_DESKTOP
RAYLIB_PATH ?= ..
PROJECT_NAME ?= raylib_example

# Library type used for raylib: STATIC (.a) or SHARED (.so/.dll)
RAYLIB_LIBTYPE ?= STATIC

# Use external GLFW library instead of rglfw module
USE_EXTERNAL_GLFW ?= FALSE

# Use Wayland display server protocol on Linux desktop
# by default it uses X11 windowing system
USE_WAYLAND_DISPLAY ?= FALSE

ifeq ($(PLATFORM),PLATFORM_RPI)
    RAYLIB_PATH ?= /home/pi/raylib
endif

# Determine PLATFORM_OS in case PLATFORM_DESKTOP selected
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    # No uname.exe on MinGW!, but OS=Windows_NT on Windows! ifeq ($(UNAME),Msys) -> Windows
    ifeq ($(OS),Windows_NT)
        PLATFORM_OS=WINDOWS
        LIBPATH=win32
    else
        UNAMEOS=$(shell uname)
        ifeq ($(UNAMEOS),Linux)
            PLATFORM_OS=LINUX
            LIBPATH=linux
        endif
        UNAMEOS=$(shell uname)
        ifeq ($(UNAMEOS),FreeBSD)
            PLATFORM_OS=FREEBSD
            LIBPATH=freebsd
        endif
        ifeq ($(UNAMEOS),Darwin)
            PLATFORM_OS=OSX
            LIBPATH=osx
        endif
    endif
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
    UNAMEOS=$(shell uname)
    ifeq ($(UNAMEOS),Linux)
        PLATFORM_OS=LINUX
    endif
endif

ifeq ($(PLATFORM),PLATFORM_RPI)
    # RPI cross-compiler
    RPI_CROSS_COMPILE ?= NO
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

# Define raylib release directory for compiled library
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
        RAYLIB_RELEASE = $(RAYLIB_PATH)/release/libs/win32/mingw32
    endif
    ifeq ($(PLATFORM_OS),LINUX)
        RAYLIB_RELEASE = $(RAYLIB_PATH)/release/libs/linux
    endif
    ifeq ($(PLATFORM_OS),OSX)
        RAYLIB_RELEASE = $(RAYLIB_PATH)/release/libs/osx
    endif
    ifeq ($(PLATFORM_OS),FREEBSD)
        RAYLIB_RELEASE = $(RAYLIB_PATH)/release/libs/freebsd
    endif
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    RAYLIB_RELEASE = $(RAYLIB_PATH)/release/libs/html5
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
    RAYLIB_RELEASE = $(RAYLIB_PATH)/release/libs/rpi
endif

# Define default C compiler: gcc
CC = gcc

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),OSX)
        # OSX default compiler
        CC = clang
    endif
    ifeq ($(PLATFORM_OS),FREEBSD)
        # FreeBSD default compiler
        CC = clang
    endif
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
    ifeq ($(RPI_CROSS_COMPILE),YES)
        # RPI cross-compiler
        CC = armv6j-hardfloat-linux-gnueabi-gcc
    endif
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    # HTML5 emscripten compiler
    CC = emcc
endif

# Define default make program: Mingw32-make
MAKE = mingw32-make

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),LINUX)
        MAKE = make
    endif
endif

# Define compiler flags:
#  -O1                  defines optimization level
#  -g                   enable debugging
#  -s                   strip unnecessary data from build
#  -Wall                turns on most, but not all, compiler warnings
#  -std=c99             defines C language mode (standard C from 1999 revision)
#  -std=gnu99           defines C language mode (GNU C from 1999 revision)
#  -fgnu89-inline       declaring inline functions support (GCC optimized)
#  -Wno-missing-braces  ignore invalid warning (GCC bug 53119)
#  -D_DEFAULT_SOURCE    use with -std=c99 on Linux and PLATFORM_WEB, required for timespec
CFLAGS += -O1 -Wall -std=c99 -D_DEFAULT_SOURCE -fgnu89-inline -Wno-missing-braces

# Additional flags for compiler (if desired)
#CFLAGS += -Wextra -Wmissing-prototypes -Wstrict-prototypes
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
        # resources file contains windows exe icon
        # -Wl,--subsystem,windows hides the console window
        CFLAGS += $(RAYLIB_PATH)/src/resources -Wl,--subsystem,windows
    endif
    ifeq ($(PLATFORM_OS),LINUX)
        CFLAGS += -no-pie -D_DEFAULT_SOURCE
    endif
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
    CFLAGS += -std=gnu99
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    # -O2                        # if used, also set --memory-init-file 0
    # --memory-init-file 0       # to avoid an external memory initialization code file (.mem)
    # -s ALLOW_MEMORY_GROWTH=1   # to allow memory resizing
    # -s TOTAL_MEMORY=16777216   # to specify heap memory size (default = 16MB)
    # -s USE_PTHREADS=1          # multithreading support
    CFLAGS += -s USE_GLFW=3 -s ASSERTIONS=1 --profiling -s TOTAL_MEMORY=16777216 --preload-file resources
endif

# Define include paths for required headers
# NOTE: Several external required libraries (stb and others)
INCLUDE_PATHS = -I. -I$(RAYLIB_PATH)/src -I$(RAYLIB_PATH)/src/external

# Define additional directories containing required header files
ifeq ($(PLATFORM),PLATFORM_RPI)
    # RPI requried libraries
    INCLUDE_PATHS += -I/opt/vc/include
    INCLUDE_PATHS += -I/opt/vc/include/interface/vmcs_host/linux
    INCLUDE_PATHS += -I/opt/vc/include/interface/vcos/pthreads
endif

# Define library paths containing required libs
LDFLAGS = -L. -L$(RAYLIB_RELEASE) -L$(RAYLIB_PATH)/src 

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),FREEBSD)
        INCLUDE_PATHS += -I/usr/local/include
        LDFLAGS += -L. -Lsrc -L/usr/local/lib
    endif
endif

ifeq ($(PLATFORM),PLATFORM_RPI)
    LDFLAGS += -L/opt/vc/lib
endif

# Define any libraries required on linking
# if you want to link libraries (libname.so or libname.a), use the -lname
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
        # Libraries for Windows desktop compiling
        LDLIBS = -lraylib -lopengl32 -lgdi32
        
        # Required for physac examples
        LDLIBS += -static -lpthread
    endif
    ifeq ($(PLATFORM_OS),LINUX)
        # Libraries for Debian GNU/Linux desktop compiling
        # NOTE: Required packages: libegl1-mesa-dev
        LDLIBS = -lraylib -lGL -lm -lpthread -ldl -lrt
        
        # On X11 requires also below libraries
        LDLIBS += -lX11
        # NOTE: It seems additional libraries are not required any more, latest GLFW just dlopen them
        #LDLIBS += -lXrandr -lXinerama -lXi -lXxf86vm -lXcursor
        
        # On Wayland windowing system, additional libraries requires
        ifeq ($(USE_WAYLAND_DISPLAY),TRUE)
            LDLIBS += -lwayland-client -lwayland-cursor -lwayland-egl -lxkbcommon
        endif
    endif
    ifeq ($(PLATFORM_OS),OSX)
        # Libraries for OSX 10.9 desktop compiling
        # NOTE: Required packages: libopenal-dev libegl1-mesa-dev
        LDLIBS = -lraylib -framework OpenGL -framework OpenAL -framework Cocoa
    endif
    ifeq ($(PLATFORM_OS),FREEBSD)
        # Libraries for FreeBSD desktop compiling
        # NOTE: Required packages: mesa-libs
        LDLIBS = -lraylib -lGL -lpthread -lm

        # On XWindow requires also below libraries
        LDLIBS += -lX11 -lXrandr -lXinerama -lXi -lXxf86vm -lXcursor
    endif
    ifeq ($(USE_EXTERNAL_GLFW),TRUE)
        LDLIBS += -lglfw
    endif
endif
ifeq ($(PLATFORM),PLATFORM_RPI)
    # Libraries for Raspberry Pi compiling
    # NOTE: Required packages: libasound2-dev (ALSA)
    LDLIBS = -lraylib -lbrcmGLESv2 -lbrcmEGL -lpthread -lrt -lm -lbcm_host -ldl
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    # Libraries for web (HTML5) compiling
    LDLIBS = $(RAYLIB_RELEASE)/libraylib.bc
endif

# Define output extension to generate a .html file using provided shell
ifeq ($(PLATFORM),PLATFORM_WEB)
    EXT = .html
    WEB_SHELL = --shell-file $(RAYLIB_PATH)\templates\web_shell\shell.html
endif

# Define all object files required
EXAMPLES = \
    core/core_basic_window \
    core/core_input_keys \
    core/core_input_mouse \
    core/core_mouse_wheel \
    core/core_input_gamepad \
    core/core_random_values \
    core/core_color_select \
    core/core_storage_values \
    core/core_gestures_detection \
    core/core_3d_mode \
    core/core_3d_picking \
    core/core_3d_camera_free \
    core/core_3d_camera_first_person \
    core/core_2d_camera \
    core/core_world_screen \
    core/core_vr_simulator \
    shapes/shapes_logo_raylib \
    shapes/shapes_basic_shapes \
    shapes/shapes_colors_palette \
    shapes/shapes_logo_raylib_anim \
    shapes/shapes_lines_bezier \
    textures/textures_logo_raylib \
    textures/textures_image_loading \
    textures/textures_rectangle \
    textures/textures_srcrec_dstrec \
    textures/textures_to_image \
    textures/textures_raw_data \
    textures/textures_particles_blending \
    textures/textures_image_processing \
    textures/textures_image_drawing \
    textures/textures_image_generation \
    textures/textures_image_text \
    text/text_sprite_fonts \
    text/text_bmfont_ttf \
    text/text_raylib_fonts \
    text/text_format_text \
    text/text_writing_anim \
    text/text_ttf_loading \
    text/text_bmfont_unordered \
    text/text_input_box \
    models/models_geometric_shapes \
    models/models_box_collisions \
    models/models_billboard \
    models/models_obj_loading \
    models/models_heightmap \
    models/models_cubicmap \
    models/models_mesh_picking \
    models/models_mesh_generation \
    models/models_yaw_pitch_roll \
    models/models_material_pbr \
    models/models_skybox \
    shaders/shaders_model_shader \
    shaders/shaders_shapes_textures \
    shaders/shaders_custom_uniform \
    shaders/shaders_postprocessing \
    audio/audio_sound_loading \
    audio/audio_music_stream \
    audio/audio_module_playing \
    audio/audio_raw_stream \
    physac/physics_demo \
    physac/physics_friction \
    physac/physics_movement \
    physac/physics_restitution \
    physac/physics_shatter \
    fix_dylib \
    
ifneq ($(PLATFORM),PLATFORM_RPI)
    EXAMPLES += core/core_drop_files
endif

CURRENT_MAKEFILE = $(lastword $(MAKEFILE_LIST))

# Default target entry
all: $(EXAMPLES)

# Generic compilation pattern
# NOTE: Examples must be ready for Android compilation!
%: %.c
ifeq ($(PLATFORM),PLATFORM_ANDROID)
	$(MAKE) -f Makefile.Android PROJECT_NAME=$@ PROJECT_SOURCE_FILES=$<
else
	$(CC) -o $@$(EXT) $< $(CFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS) -D$(PLATFORM)
endif

# fix dylib install path name for each executable (MAC)
fix_dylib:
ifeq ($(PLATFORM_OS),OSX)
	find . -type f -perm +ugo+x -print0 | xargs -t -0 -R 1 -I file install_name_tool -change libglfw.3.0.dylib ../external/glfw3/lib/osx/libglfw.3.0.dylib file
endif

# Clean everything
clean:
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
		del *.o *.exe /s
    endif
    ifeq ($(PLATFORM_OS),LINUX)
		find -type f -executable | xargs file -i | grep -E 'x-object|x-archive|x-sharedlib|x-executable' | rev | cut -d ':' -f 2- | rev | xargs rm -f
    endif
    ifeq ($(PLATFORM_OS),OSX)
		find . -type f -perm +ugo+x -delete
		rm -f *.o
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

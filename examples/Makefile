#**************************************************************************************************
#
#   raylib makefile for multiple platforms
#
#   This file supports building raylib examples for the following platforms:
#
#     > PLATFORM_DESKTOP (GLFW backend):
#         - Windows (Win32, Win64)
#         - Linux (X11/Wayland desktop mode)
#         - macOS/OSX (x64, arm64)
#         - FreeBSD, OpenBSD, NetBSD, DragonFly (X11 desktop)
#     > PLATFORM_DESKTOP_SDL (SDL backend):
#         - Windows (Win32, Win64)
#         - Linux (X11/Wayland desktop mode)
#         - Others (not tested)
#     > PLATFORM_WEB:
#         - HTML5 (WebAssembly)
#     > PLATFORM_DRM:
#         - Raspberry Pi 0-5 (DRM/KMS)
#         - Linux DRM subsystem (KMS mode)
#     > PLATFORM_ANDROID:
#         - Android (ARM, ARM64)
#
#   Copyright (c) 2013-2023 Ramon Santamaria (@raysan5)
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

# Define required environment variables
#------------------------------------------------------------------------------------------------
# Define target platform: PLATFORM_DESKTOP, PLATFORM_DESKTOP_SDL, PLATFORM_DRM, PLATFORM_ANDROID, PLATFORM_WEB
PLATFORM              ?= PLATFORM_DESKTOP

# Define required raylib variables
PROJECT_NAME          ?= raylib_examples
RAYLIB_VERSION        ?= 5.0.0
RAYLIB_PATH           ?= ..

# Define raylib source code path
RAYLIB_SRC_PATH       ?= ../src

# Locations of raylib.h and libraylib.a/libraylib.so
# NOTE: Those variables are only used for PLATFORM_OS: LINUX, BSD
RAYLIB_INCLUDE_PATH   ?= /usr/local/include
RAYLIB_LIB_PATH       ?= /usr/local/lib

# Library type compilation: STATIC (.a) or SHARED (.so/.dll)
RAYLIB_LIBTYPE        ?= STATIC

# Build mode for project: DEBUG or RELEASE
BUILD_MODE            ?= RELEASE

# Use external GLFW library instead of rglfw module
USE_EXTERNAL_GLFW     ?= FALSE

# PLATFORM_DESKTOP_SDL: It requires SDL library to be provided externally
# WARNING: Library is not included in raylib, it MUST be configured by users
SDL_INCLUDE_PATH      ?= $(RAYLIB_SRC_PATH)/external/SDL2-2.28.4/include
SDL_LIBRARY_PATH      ?= $(RAYLIB_SRC_PATH)/external/SDL2-2.28.4/lib/x64

# Use Wayland display server protocol on Linux desktop (by default it uses X11 windowing system)
# NOTE: This variable is only used for PLATFORM_OS: LINUX
USE_WAYLAND_DISPLAY   ?= FALSE

# PLATFORM_WEB: Default properties
BUILD_WEB_ASYNCIFY    ?= TRUE
BUILD_WEB_SHELL       ?= $(RAYLIB_PATH)/src/minshell.html
BUILD_WEB_HEAP_SIZE   ?= 134217728
BUILD_WEB_RESOURCES   ?= TRUE
BUILD_WEB_RESOURCES_PATH  ?= $(dir $<)resources@resources

# Determine PLATFORM_OS when required
ifeq ($(PLATFORM),$(filter $(PLATFORM),PLATFORM_DESKTOP PLATFORM_DESKTOP_SDL PLATFORM_WEB))
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
ifeq ($(PLATFORM),PLATFORM_DRM)
    UNAMEOS = $(shell uname)
    ifeq ($(UNAMEOS),Linux)
        PLATFORM_OS = LINUX
    endif
endif

# RAYLIB_PATH adjustment for LINUX platform
# TODO: Do we really need this?
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),LINUX)
        RAYLIB_PREFIX  ?= ..
        RAYLIB_PATH     = $(realpath $(RAYLIB_PREFIX))
    endif
endif

# Default path for raylib on Raspberry Pi
ifeq ($(PLATFORM),PLATFORM_DRM)
    RAYLIB_PATH        ?= /home/pi/raylib
endif

# Define raylib release directory for compiled library
RAYLIB_RELEASE_PATH    ?= $(RAYLIB_PATH)/src

ifeq ($(PLATFORM),PLATFORM_WEB)
    ifeq ($(PLATFORM_OS),WINDOWS)
        # Emscripten required variables
		EMSDK_PATH         ?= C:/emsdk
		EMSCRIPTEN_PATH    ?= $(EMSDK_PATH)/upstream/emscripten
		CLANG_PATH          = $(EMSDK_PATH)/upstream/bin
		PYTHON_PATH         = $(EMSDK_PATH)/python/3.9.2-1_64bit
		NODE_PATH           = $(EMSDK_PATH)/node/14.15.5_64bit/bin
		export PATH         = $(EMSDK_PATH);$(EMSCRIPTEN_PATH);$(CLANG_PATH);$(NODE_PATH);$(PYTHON_PATH):$$(PATH)
    endif
endif

# Define default C compiler: CC
#------------------------------------------------------------------------------------------------
CC = gcc

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),OSX)
        # OSX default compiler
        CC = clang
    endif
    ifeq ($(PLATFORM_OS),BSD)
        # FreeBSD, OpenBSD, NetBSD, DragonFly default compiler
        CC = clang
    endif
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    # HTML5 emscripten compiler
    # WARNING: To compile to HTML5, code must be redesigned
    # to use emscripten.h and emscripten_set_main_loop()
    CC = emcc
endif

# Define default make program: MAKE
#------------------------------------------------------------------------------------------------
MAKE ?= make

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
        MAKE = mingw32-make
    endif
endif
ifeq ($(PLATFORM),PLATFORM_ANDROID)
    MAKE = mingw32-make
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    MAKE = emmake make
endif

# Define compiler flags: CFLAGS
#------------------------------------------------------------------------------------------------
#  -O1                  defines optimization level
#  -g                   include debug information on compilation
#  -s                   strip unnecessary data from build
#  -Wall                turns on most, but not all, compiler warnings
#  -std=c99             defines C language mode (standard C from 1999 revision)
#  -std=gnu99           defines C language mode (GNU C from 1999 revision)
#  -Wno-missing-braces  ignore invalid warning (GCC bug 53119)
#  -Wno-unused-value    ignore unused return values of some functions (i.e. fread())
#  -D_DEFAULT_SOURCE    use with -std=c99 on Linux and PLATFORM_WEB, required for timespec
CFLAGS = -Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result

ifeq ($(BUILD_MODE),DEBUG)
    CFLAGS += -g -D_DEBUG
    ifeq ($(PLATFORM),PLATFORM_WEB)
        CFLAGS += -s ASSERTIONS=1 --profiling
    endif
else
    ifeq ($(PLATFORM),PLATFORM_WEB)
        ifeq ($(BUILD_WEB_ASYNCIFY),TRUE)
            CFLAGS += -O3
        else
            CFLAGS += -Os
        endif
    else
        CFLAGS += -O2
    endif
endif

# Additional flags for compiler (if desired)
#  -Wextra                  enables some extra warning flags that are not enabled by -Wall
#  -Wmissing-prototypes     warn if a global function is defined without a previous prototype declaration
#  -Wstrict-prototypes      warn if a function is declared or defined without specifying the argument types
#  -Werror=implicit-function-declaration   catch function calls without prior declaration
#CFLAGS += -Wextra -Wmissing-prototypes -Wstrict-prototypes
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),LINUX)
        ifeq ($(RAYLIB_LIBTYPE),STATIC)
            CFLAGS += -D_DEFAULT_SOURCE
        endif
        ifeq ($(RAYLIB_LIBTYPE),SHARED)
            # Explicitly enable runtime link to libraylib.so
            CFLAGS += -Wl,-rpath,$(RAYLIB_RELEASE_PATH)
        endif
    endif
endif
ifeq ($(PLATFORM),PLATFORM_DRM)
    CFLAGS += -std=gnu99 -DEGL_NO_X11
endif

# Define include paths for required headers: INCLUDE_PATHS
# NOTE: Some external/extras libraries could be required (stb, easings...)
#------------------------------------------------------------------------------------------------
INCLUDE_PATHS = -I. -I$(RAYLIB_PATH)/src -I$(RAYLIB_PATH)/src/external

# Define additional directories containing required header files
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),BSD)
        INCLUDE_PATHS += -I$(RAYLIB_INCLUDE_PATH)
    endif
    ifeq ($(PLATFORM_OS),LINUX)
        INCLUDE_PATHS += -I$(RAYLIB_INCLUDE_PATH)
    endif
endif
ifeq ($(PLATFORM),PLATFORM_DESKTOP_SDL)
    INCLUDE_PATHS += -I$(SDL_INCLUDE_PATH)
endif
ifeq ($(PLATFORM),PLATFORM_DRM)
    INCLUDE_PATHS += -I$(RAYLIB_INCLUDE_PATH)
    INCLUDE_PATHS += -I/usr/include/libdrm
endif

# Include GLFW required for examples/others/rlgl_standalone.c
ifeq ($(USE_EXTERNAL_GLFW),FALSE)
all others: INCLUDE_PATHS += -I$(RAYLIB_PATH)/src/external/glfw/include
endif

# Define library paths containing required libs: LDFLAGS
#------------------------------------------------------------------------------------------------
LDFLAGS = -L. -L$(RAYLIB_RELEASE_PATH) -L$(RAYLIB_PATH)/src

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
        # NOTE: The resource .rc file contains windows executable icon and properties
        LDFLAGS += $(RAYLIB_PATH)/src/raylib.rc.data
        # -Wl,--subsystem,windows hides the console window
        ifeq ($(BUILD_MODE), RELEASE)
            LDFLAGS += -Wl,--subsystem,windows
        endif
    endif
    ifeq ($(PLATFORM_OS),LINUX)
        LDFLAGS += -L$(RAYLIB_LIB_PATH)
    endif
    ifeq ($(PLATFORM_OS),BSD)
        LDFLAGS += -Lsrc -L$(RAYLIB_LIB_PATH)
    endif
endif
ifeq ($(PLATFORM),PLATFORM_DESKTOP_SDL)
    ifeq ($(PLATFORM_OS),WINDOWS)
        # NOTE: The resource .rc file contains windows executable icon and properties
        LDFLAGS += $(RAYLIB_PATH)/src/raylib.rc.data
        # -Wl,--subsystem,windows hides the console window
        ifeq ($(BUILD_MODE), RELEASE)
            LDFLAGS += -Wl,--subsystem,windows
        endif
    endif
    LDFLAGS += -L$(SDL_LIBRARY_PATH)
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    # -Os                        # size optimization
    # -O2                        # optimization level 2, if used, also set --memory-init-file 0
    # -s USE_GLFW=3              # Use glfw3 library (context/input management)
    # -s ALLOW_MEMORY_GROWTH=1   # to allow memory resizing -> WARNING: Audio buffers could FAIL!
    # -s TOTAL_MEMORY=16777216   # to specify heap memory size (default = 16MB) (67108864 = 64MB)
    # -s USE_PTHREADS=1          # multithreading support
    # -s WASM=0                  # disable Web Assembly, emitted by default
    # -s ASYNCIFY                # lets synchronous C/C++ code interact with asynchronous JS
    # -s FORCE_FILESYSTEM=1      # force filesystem to load/save files data
    # -s ASSERTIONS=1            # enable runtime checks for common memory allocation errors (-O1 and above turn it off)
    # --profiling                # include information for code profiling
    # --memory-init-file 0       # to avoid an external memory initialization code file (.mem)
    # --preload-file resources   # specify a resources folder for data compilation
    # --source-map-base          # allow debugging in browser with source map
    LDFLAGS += -s USE_GLFW=3 -s TOTAL_MEMORY=$(BUILD_WEB_HEAP_SIZE) -s FORCE_FILESYSTEM=1

    # Build using asyncify
    ifeq ($(BUILD_WEB_ASYNCIFY),TRUE)
        LDFLAGS += -s ASYNCIFY
    endif

    # Add resources building if required
    ifeq ($(BUILD_WEB_RESOURCES),TRUE)
        LDFLAGS += --preload-file $(BUILD_WEB_RESOURCES_PATH)
    endif

    # Add debug mode flags if required
    ifeq ($(BUILD_MODE),DEBUG)
        LDFLAGS += -s ASSERTIONS=1 --profiling
    endif

    # Define a custom shell .html and output extension
    LDFLAGS += --shell-file $(BUILD_WEB_SHELL)
    EXT = .html

    # NOTE: Simple raylib examples are compiled to be interpreter with asyncify, that way,
    # we can compile same code for ALL platforms with no change required, but, working on bigger
    # projects, code needs to be refactored to avoid a blocking while() loop, moving Update and Draw
    # logic to a self contained function: UpdateDrawFrame(), check core_basic_window_web.c for reference.

endif

# Define libraries required on linking: LDLIBS
# NOTE: To link libraries (lib<name>.so or lib<name>.a), use -l<name>
#------------------------------------------------------------------------------------------------
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
        # Libraries for Windows desktop compilation
        # NOTE: WinMM library required to set high-res timer resolution
        LDLIBS = -lraylib -lopengl32 -lgdi32 -lwinmm
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
        # Explicit link to libc
        ifeq ($(RAYLIB_LIBTYPE),SHARED)
            LDLIBS += -lc
        endif

        # NOTE: On ARM 32bit arch, miniaudio requires atomics library
        LDLIBS += -latomic
    endif
    ifeq ($(PLATFORM_OS),OSX)
        # Libraries for OSX 10.9 desktop compiling
        # NOTE: Required packages: libopenal-dev libegl1-mesa-dev
        LDLIBS = -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreAudio -framework CoreVideo
    endif
    ifeq ($(PLATFORM_OS),BSD)
        # Libraries for FreeBSD, OpenBSD, NetBSD, DragonFly desktop compiling
        # NOTE: Required packages: mesa-libs
        LDLIBS = -lraylib -lGL -lpthread -lm

        # On XWindow requires also below libraries
        LDLIBS += -lX11 -lXrandr -lXinerama -lXi -lXxf86vm -lXcursor
    endif
    ifeq ($(USE_EXTERNAL_GLFW),TRUE)
        # NOTE: It could require additional packages installed: libglfw3-dev
        LDLIBS += -lglfw
    endif
endif
ifeq ($(PLATFORM),PLATFORM_DESKTOP_SDL)
    ifeq ($(PLATFORM_OS),WINDOWS)
        # Libraries for Windows desktop compilation
        LDLIBS = -lraylib -lSDL2 -lSDL2main -lopengl32 -lgdi32
    endif
    ifeq ($(PLATFORM_OS),LINUX)
        # Libraries for Debian GNU/Linux desktop compiling
        # NOTE: Required packages: libegl1-mesa-dev
        LDLIBS = -lraylib -lSDL2 -lSDL2main -lGL -lm -lpthread -ldl -lrt

        # On X11 requires also below libraries
        LDLIBS += -lX11
        # NOTE: It seems additional libraries are not required any more, latest GLFW just dlopen them
        #LDLIBS += -lXrandr -lXinerama -lXi -lXxf86vm -lXcursor

        # On Wayland windowing system, additional libraries requires
        ifeq ($(USE_WAYLAND_DISPLAY),TRUE)
            LDLIBS += -lwayland-client -lwayland-cursor -lwayland-egl -lxkbcommon
        endif
        # Explicit link to libc
        ifeq ($(RAYLIB_LIBTYPE),SHARED)
            LDLIBS += -lc
        endif

        # NOTE: On ARM 32bit arch, miniaudio requires atomics library
        LDLIBS += -latomic
    endif
endif
ifeq ($(PLATFORM),PLATFORM_DRM)
    # Libraries for DRM compiling
    # NOTE: Required packages: libasound2-dev (ALSA)
    LDLIBS = -lraylib -lGLESv2 -lEGL -lpthread -lrt -lm -lgbm -ldrm -ldl -latomic
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    # Libraries for web (HTML5) compiling
    LDLIBS = $(RAYLIB_RELEASE_PATH)/libraylib.a
endif

# Define source code object files required
#------------------------------------------------------------------------------------------------
CORE = \
    core/core_2d_camera \
    core/core_2d_camera_mouse_zoom \
    core/core_2d_camera_platformer \
    core/core_2d_camera_split_screen \
    core/core_3d_camera_first_person \
    core/core_3d_camera_free \
    core/core_3d_camera_mode \
    core/core_3d_camera_split_screen \
    core/core_3d_picking \
    core/core_automation_events \
    core/core_basic_screen_manager \
    core/core_basic_window \
    core/core_custom_frame_control \
    core/core_custom_logging \
    core/core_drop_files \
    core/core_input_gamepad \
    core/core_input_gamepad_info \
    core/core_input_gestures \
    core/core_input_gestures_web \
    core/core_input_keys \
    core/core_input_mouse \
    core/core_input_mouse_wheel \
    core/core_input_multitouch \
    core/core_loading_thread \
    core/core_random_values \
    core/core_scissor_test \
    core/core_smooth_pixelperfect \
    core/core_storage_values \
    core/core_vr_simulator \
    core/core_window_flags \
    core/core_window_letterbox \
    core/core_window_should_close \
    core/core_world_screen

SHAPES = \
    shapes/shapes_basic_shapes \
    shapes/shapes_bouncing_ball \
    shapes/shapes_collision_area \
    shapes/shapes_colors_palette \
    shapes/shapes_draw_circle_sector \
    shapes/shapes_draw_rectangle_rounded \
    shapes/shapes_draw_ring \
    shapes/shapes_easings_ball_anim \
    shapes/shapes_easings_box_anim \
    shapes/shapes_easings_rectangle_array \
    shapes/shapes_following_eyes \
    shapes/shapes_lines_bezier \
    shapes/shapes_logo_raylib \
    shapes/shapes_logo_raylib_anim \
    shapes/shapes_rectangle_scaling \
    shapes/shapes_splines_drawing \
    shapes/shapes_top_down_lights

TEXTURES = \
    textures/textures_background_scrolling \
    textures/textures_blend_modes \
    textures/textures_bunnymark \
    textures/textures_draw_tiled \
    textures/textures_fog_of_war \
    textures/textures_gif_player \
    textures/textures_image_drawing \
    textures/textures_image_generation \
    textures/textures_image_kernel \
    textures/textures_image_loading \
    textures/textures_image_processing \
    textures/textures_image_rotate \
    textures/textures_image_text \
    textures/textures_logo_raylib \
    textures/textures_mouse_painting \
    textures/textures_npatch_drawing \
    textures/textures_particles_blending \
    textures/textures_polygon \
    textures/textures_raw_data \
    textures/textures_sprite_anim \
    textures/textures_sprite_button \
    textures/textures_sprite_explosion \
    textures/textures_srcrec_dstrec \
    textures/textures_svg_loading \
    textures/textures_textured_curve \
    textures/textures_to_image

TEXT = \
    text/text_codepoints_loading \
    text/text_draw_3d \
    text/text_font_filters \
    text/text_font_loading \
    text/text_font_sdf \
    text/text_font_spritefont \
    text/text_format_text \
    text/text_input_box \
    text/text_raylib_fonts \
    text/text_rectangle_bounds \
    text/text_unicode \
    text/text_writing_anim

MODELS = \
    models/models_animation \
    models/models_billboard \
    models/models_box_collisions \
    models/models_cubicmap \
    models/models_draw_cube_texture \
    models/models_first_person_maze \
    models/models_geometric_shapes \
    models/models_heightmap \
    models/models_loading \
    models/models_loading_gltf \
    models/models_loading_m3d \
    models/models_loading_vox \
    models/models_mesh_generation \
    models/models_mesh_picking \
    models/models_orthographic_projection \
    models/models_rlgl_solar_system \
    models/models_skybox \
    models/models_waving_cubes \
    models/models_yaw_pitch_roll

SHADERS = \
    shaders/shaders_basic_lighting \
    shaders/shaders_custom_uniform \
    shaders/shaders_deferred_render \
    shaders/shaders_eratosthenes \
    shaders/shaders_fog \
    shaders/shaders_hot_reloading \
    shaders/shaders_hybrid_render \
    shaders/shaders_julia_set \
    shaders/shaders_lightmap \
    shaders/shaders_mesh_instancing \
    shaders/shaders_model_shader \
    shaders/shaders_multi_sample2d \
    shaders/shaders_palette_switch \
    shaders/shaders_postprocessing \
    shaders/shaders_raymarching \
    shaders/shaders_shapes_textures \
    shaders/shaders_simple_mask \
    shaders/shaders_spotlight \
    shaders/shaders_texture_drawing \
    shaders/shaders_texture_outline \
    shaders/shaders_texture_tiling \
    shaders/shaders_texture_waves \
    shaders/shaders_write_depth

AUDIO = \
    audio/audio_mixed_processor \
    audio/audio_module_playing \
    audio/audio_music_stream \
    audio/audio_raw_stream \
    audio/audio_sound_loading \
    audio/audio_sound_multi \
    audio/audio_stream_effects

OTHERS = \
    others/easings_testbed \
    others/embedded_files_loading \
    others/raylib_opengl_interop \
    others/raymath_vector_angle \
    others/rlgl_compute_shader \
    others/rlgl_standalone

CURRENT_MAKEFILE = $(lastword $(MAKEFILE_LIST))

# Define processes to execute
#------------------------------------------------------------------------------------------------
# Default target entry
all: $(CORE) $(SHAPES) $(TEXT) $(TEXTURES) $(MODELS) $(SHADERS) $(AUDIO) $(OTHERS)

core: $(CORE)
shapes: $(SHAPES)
textures: $(TEXTURES)
text: $(TEXT)
models: $(MODELS)
shaders: $(SHADERS)
audio: $(AUDIO)
others: $(OTHERS)

# Generic compilation pattern
# NOTE: Examples must be ready for Android compilation!
%: %.c
ifeq ($(PLATFORM),PLATFORM_ANDROID)
	$(MAKE) -f Makefile.Android PROJECT_NAME=$@ PROJECT_SOURCE_FILES=$<
else ifeq ($(PLATFORM),PLATFORM_WEB)
	$(MAKE) -f Makefile.Web $@
else
	$(CC) -o $@$(EXT) $< $(CFLAGS) $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS) -D$(PLATFORM)
endif

# Clean everything
clean:
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    ifeq ($(PLATFORM_OS),WINDOWS)
		del *.o *.exe /s
    endif
    ifeq ($(PLATFORM_OS),LINUX)
		find . -type f -executable -delete
		rm -fv *.o
    endif
    ifeq ($(PLATFORM_OS),OSX)
		find . -type f -perm +ugo+x -delete
		rm -f *.o
    endif
endif
ifeq ($(PLATFORM),PLATFORM_DRM)
	find . -type f -executable -delete
	rm -fv *.o
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    ifeq ($(PLATFORM_OS),WINDOWS)
		del *.wasm *.html *.js *.data
    else
		rm -f */*.wasm */*.html */*.js */*.data
    endif
endif
	@echo Cleaning done

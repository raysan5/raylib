#**************************************************************************************************
#
#   raylib for Raspberry Pi and Windows desktop
#
#   makefile to compile raylib examples
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
# Other possible platforms: PLATFORM_DESKTOP PLATFORM_DESKTOP_LINUX
PLATFORM ?= PLATFORM_RPI

# define compiler: gcc for C program, define as g++ for C++
CC = gcc

# define compiler flags:
#  -O2         defines optimization level
#  -Wall       turns on most, but not all, compiler warnings
#  -std=c99    use standard C from 1999 revision
ifeq ($(PLATFORM),PLATFORM_RPI)
    CFLAGS = -O2 -Wall -std=gnu99 -fgnu89-inline
else
    CFLAGS = -O2 -Wall -std=c99
endif
#CFLAGSEXTRA = -Wextra -Wmissing-prototypes -Wstrict-prototypes

# define any directories containing required header files
ifeq ($(PLATFORM),PLATFORM_RPI)
    INCLUDES = -I. -I../src -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads
else
    INCLUDES = -I. -I../src
endif

# define library paths containing required libs
ifeq ($(PLATFORM),PLATFORM_DESKTOP_LINUX)
	LFLAGS = -L. -L../src
else
	LFLAGS = -L. -L../src -L/opt/vc/lib
endif

# define any libraries to link into executable
# if you want to link libraries (libname.so or libname.a), use the -lname
ifeq ($(PLATFORM),PLATFORM_RPI)
    # libraries for Raspberry Pi compiling
    # NOTE: OpenAL Soft library should be installed (libopenal1 package)
    LIBS = -lraylib -lGLESv2 -lEGL -lpthread -lrt -lm -lbcm_host -lopenal
else
ifeq ($(PLATFORM),PLATFORM_DESKTOP_LINUX)
    # libraries for Debian GNU/Linux desktop compiling
	# requires the following packages:
	# libglfw3-dev libopenal-dev libglew-dev libegl1-mesa-dev
    LIBS = -lraylib -lglfw -lGLEW -lGL -lopenal
else
    # libraries for Windows desktop compiling
    # NOTE: GLFW3 and OpenAL Soft libraries should be installed
    LIBS = -lraylib -lglfw3 -lglew32 -lopengl32 -lopenal32 -lgdi32
endif
endif

# define additional parameters and flags for windows
ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    # resources file contains windows exe icon
    # -Wl,--subsystem,windows hides the console window
    WINFLAGS = ../src/resources -Wl,--subsystem,windows
endif

# define all object files required
EXAMPLES = \
    core_basic_window \
    core_input_keys \
    core_input_mouse \
    core_mouse_wheel \
    core_random_values \
    core_color_select \
    core_3d_mode \
    shapes_logo_raylib \
    shapes_basic_shapes \
    shapes_colors_palette \
    shapes_logo_raylib_anim \
    textures_logo_raylib \
    textures_image_loading \
    textures_rectangle \
    textures_compressed_dds \
    textures_mipmaps \
    textures_srcrec_dstrec \
    text_sprite_fonts \
    text_rbmf_fonts \
    text_format_text \
    text_font_select \
    models_geometric_shapes \
    models_planes \
    models_billboard \
    models_obj_loading \
    models_heightmap \
    models_cubesmap \
    audio_sound_loading \
    audio_music_stream \
    #core_input_gamepad \


# typing 'make' will invoke the first target entry in the file,
# in this case, the 'default' target entry is raylib
default: examples

# compile all examples
examples: $(EXAMPLES)

# compile [core] example - basic window
core_basic_window: core_basic_window.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [core] example - keyboard input
core_input_keys: core_input_keys.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [core] example - mouse input
core_input_mouse: core_input_mouse.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
# compile [core] example - gamepad input
core_input_gamepad: core_input_gamepad.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)
endif

# compile [core] example - mouse wheel
core_mouse_wheel: core_mouse_wheel.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [core] example - generate random values
core_random_values: core_random_values.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [core] example - color selection (collision detection)
core_color_select: core_color_select.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [core] example - 3d mode
core_3d_mode: core_3d_mode.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [shapes] example - raylib logo (with basic shapes)
shapes_logo_raylib: shapes_logo_raylib.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [shapes] example - basic shapes usage (rectangle, circle, ...)
shapes_basic_shapes: shapes_basic_shapes.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [shapes] example - raylib color palette
shapes_colors_palette: shapes_colors_palette.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [shapes] example - raylib logo animation
shapes_logo_raylib_anim: shapes_logo_raylib_anim.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [textures] example - raylib logo texture loading
textures_logo_raylib: textures_logo_raylib.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [textures] example - image loading and conversion to texture
textures_image_loading: textures_image_loading.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [textures] example - texture rectangle drawing
textures_rectangle: textures_rectangle.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [textures] example - compressed texture loading (DDS)
textures_compressed_dds: textures_compressed_dds.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [textures] example - texture mipmaps generation
textures_mipmaps: textures_mipmaps.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [textures] example - texture source and destination rectangles
textures_srcrec_dstrec: textures_srcrec_dstrec.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [text] example - sprite fonts loading
text_sprite_fonts: text_sprite_fonts.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [text] example - raylib bitmap fonts (rBMF)
text_rbmf_fonts: text_rbmf_fonts.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [text] example - text formatting
text_format_text: text_format_text.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [text] example - font selection program
text_font_select: text_font_select.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [models] example - basic geometric 3d shapes
models_geometric_shapes: models_geometric_shapes.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [models] example - basic window
models_planes: models_planes.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [models] example - billboard usage
models_billboard: models_billboard.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [models] example - OBJ model loading
models_obj_loading: models_obj_loading.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [models] example - heightmap loading
models_heightmap: models_heightmap.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [models] example - cubesmap loading
models_cubesmap: models_cubesmap.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [audio] example - sound loading and playing (WAV and OGG)
audio_sound_loading: audio_sound_loading.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# compile [audio] example - music stream playing (OGG)
audio_music_stream: audio_music_stream.c
	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM) $(WINFLAGS)

# clean everything
clean:
ifeq ($(PLATFORM),PLATFORM_RPI)
	rm -f *.o
#	find . -executable -delete
else
ifeq ($(PLATFORM),PLATFORM_DESKTOP_LINUX)
	find . -type f -executable -delete
	rm -f *.o
else
	del *.o *.exe
endif
endif
	@echo Cleaning done

# instead of defining every module one by one, we can define a pattern
# this pattern below will automatically compile every module defined on $(OBJS)
#%.exe : %.c
#	$(CC) -o $@ $< $(CFLAGS) $(INCLUDES) $(LFLAGS) $(LIBS) -D$(PLATFORM)

#**************************************************************************************************
#
#   raylib for Android
#
#   Static library compilation
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

# Path of the current directory (i.e. the directory containing the Android.mk file itself)
LOCAL_PATH := $(call my-dir)

# raylib static library compilation
# NOTE: It uses source placed on relative path ../../src from this file
#-----------------------------------------------------------------------
# Makefile that will clear many LOCAL_XXX variables for you
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE     := raylib

# Module source files
LOCAL_SRC_FILES  :=\
                    ../../src/core.c \
                    ../../src/rlgl.c \
                    ../../src/textures.c \
                    ../../src/text.c \
                    ../../src/shapes.c \
                    ../../src/gestures.c \
                    ../../src/models.c \
                    ../../src/utils.c \
                    ../../src/audio.c \
                    ../../src/stb_vorbis.c \

# Required includes paths (.h)
LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/include $(LOCAL_PATH)/../../src

# Required flags for compilation: defines PLATFORM_ANDROID and GRAPHICS_API_OPENGL_ES2
LOCAL_CFLAGS     := -Wall -std=c99 -g -DPLATFORM_ANDROID -DGRAPHICS_API_OPENGL_ES2

# Build the static library libraylib.a
include $(BUILD_STATIC_LIBRARY)
#--------------------------------------------------------------------

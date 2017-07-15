#**************************************************************************************************
#
#   raylib for Android
#
#   Game template makefile
#
#   Copyright (c) 2014-2016 Ramon Santamaria (@raysan5)
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

# OpenAL module (prebuilt static library)
#--------------------------------------------------------------------
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE := openal

# Precompiled lib
LOCAL_SRC_FILES := libs/libopenal.so

# Export headers
LOCAL_EXPORT_C_INCLUDES := include

# Build static library
include $(PREBUILT_SHARED_LIBRARY)
#--------------------------------------------------------------------


# raylib module (prebuilt static library)
#--------------------------------------------------------------------
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE := raylib

# Precompiled lib
LOCAL_SRC_FILES := libs/libraylib.a

# Export headers
LOCAL_EXPORT_C_INCLUDES := include

# Static library dependency
LOCAL_STATIC_LIBRARIES := android_native_app_glue

# Build static library
include $(PREBUILT_STATIC_LIBRARY)
#--------------------------------------------------------------------


# raylib game module (shared library)
#--------------------------------------------------------------------
# Makefile that will clear many LOCAL_XXX variables for you
include $(CLEAR_VARS)

# Module name
LOCAL_MODULE     := raylib_game

# Module source files
LOCAL_SRC_FILES  := basic_game.c

# Required includes paths (.h)
# NOTE: raylib header and openal headers are included using LOCAL_EXPORT_C_INCLUDES
LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/include

# Required flags for compilation: defines PLATFORM_ANDROID
LOCAL_CFLAGS     := -Wall -std=c99 -DPLATFORM_ANDROID

# Linker required libraries (not many...)
LOCAL_LDLIBS     := -llog -landroid -lEGL -lGLESv2 -lOpenSLES

# Required static library
LOCAL_STATIC_LIBRARIES := android_native_app_glue raylib openal

# Required shared library
# NOTE: It brokes the build, using static library instead
#LOCAL_SHARED_LIBRARIES := openal

# Build the shared library libraylib_game.so
include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
#--------------------------------------------------------------------

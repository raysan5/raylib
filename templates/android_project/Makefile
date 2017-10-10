#**************************************************************************************************
#
#   raylib makefile for Android project (APK building)
#
#   Copyright (c) 2017 Ramon Santamaria (@raysan5)
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

# Define raylib platform to compile for
PLATFORM ?= PLATFORM_ANDROID

# Android project name (.apk)
PROJECT_NAME = raylib_game
OUTPUT_PATH = output
SOURCE_PATH = src

# Generated shared library name
# NOTE: It should match the name defined in the AndroidManifest.xml
LIBRARY_NAME = raylib_game

# define libtype for raylib and OpenAL Soft: STATIC (.a) or SHARED (.so/.dll)
RAYLIB_LIBTYPE ?= STATIC
OPENAL_LIBTYPE ?= STATIC

LIBS_PATH = libs\static

# add shared libs to APK if required
ifeq ($(RAYLIB_LIBTYPE),SHARED)
    LIBS_PATH = libs\shared
    NATIVE_LOADER = NativeLoader.java
    PROJECT_SHARED_LIBS = lib/armeabi-v7a/libraylib.so 
endif
ifeq ($(OPENAL_LIBTYPE),SHARED)
    LIBS_PATH = libs\shared
    NATIVE_LOADER = NativeLoader.java
    PROJECT_SHARED_LIBS += lib/armeabi-v7a/libopenal.so
endif

# Generated key pass
KEYSTORE_PASS = raylib

# Required path variables
# NOTE: JAVA_HOME must be set to JDK
ANDROID_HOME = C:/android-sdk
ANDROID_NDK = C:/android-ndk
ANDROID_TOOLCHAIN = C:/android_toolchain_arm_api16
ANDROID_BUILD_TOOLS = $(ANDROID_HOME)/build-tools/26.0.1
ANDROID_PLATFORM_TOOLS = $(ANDROID_HOME)/platform-tools
JAVA_HOME = C:/PROGRA~1/Java/jdk1.8.0_144

# Compilers
CC = $(ANDROID_TOOLCHAIN)/bin/arm-linux-androideabi-gcc
AR = $(ANDROID_TOOLCHAIN)/bin/arm-linux-androideabi-ar

# Compiler flags for arquitecture
CFLAGS = -std=c99 -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16
# Compilation functions attributes options
CFLAGS += -ffunction-sections -funwind-tables -fstack-protector-strong -fPIC
# Compiler options for the linker
CFLAGS += -Wall -Wa,--noexecstack -Wformat -Werror=format-security -no-canonical-prefixes
# Preprocessor macro definitions
CFLAGS += -DANDROID -DPLATFORM_ANDROID -D__ANDROID_API__=16

# Paths containing required header files
INCLUDE_PATHS = -I. -Iinclude -I$(ANDROID_NDK)/sources/android/native_app_glue

# Linker options
LDFLAGS = -Wl,-soname,lib$(LIBRARY_NAME).so -Wl,--exclude-libs,libatomic.a 
LDFLAGS += -Wl,--build-id -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -Wl,--warn-shared-textrel -Wl,--fatal-warnings 
# Force linking of library module to define symbol
LDFLAGS += -u ANativeActivity_onCreate
# Library paths containing required libs
LDFLAGS += -L. -L$(SOURCE_PATH) -L$(LIBS_PATH) -L$(OUTPUT_PATH)/obj

# Define any libraries to link into executable
# if you want to link libraries (libname.so or libname.a), use the -lname
LDLIBS = -lraylib -lnative_app_glue -lopenal -llog -landroid -lEGL -lGLESv2 -lOpenSLES -latomic -lc -lm -ldl

# Building APK
# NOTE: typing 'make' will invoke the default target entry called 'all',
all: create_temp_project_dirs \
     compile_native_app_glue \
     compile_project_code \
     copy_project_shared_libs \
     copy_project_resources \
     config_project_package \
     compile_project_class \
     compile_project_class_dex \
     create_project_apk_package \
     generate_apk_keystore \
     sign_project_apk_package \
     zipalign_project_apk_package

# Create required temp directories for APK building
create_temp_project_dirs:
	if not exist $(OUTPUT_PATH) mkdir $(OUTPUT_PATH) 
	if not exist $(OUTPUT_PATH)\obj mkdir $(OUTPUT_PATH)\obj
	if not exist $(OUTPUT_PATH)\src mkdir $(OUTPUT_PATH)\src
	if not exist $(OUTPUT_PATH)\lib mkdir $(OUTPUT_PATH)\lib
	if not exist $(OUTPUT_PATH)\lib\armeabi-v7a mkdir $(OUTPUT_PATH)\lib\armeabi-v7a
	if not exist $(OUTPUT_PATH)\bin mkdir $(OUTPUT_PATH)\bin
	if not exist $(OUTPUT_PATH)\res mkdir $(OUTPUT_PATH)\res
	if not exist $(OUTPUT_PATH)\assets mkdir $(OUTPUT_PATH)\assets

# Compile native_app_glue as static library
# OUTPUT: $(PROJECT_DIR)/obj/libnative_app_glue.a
compile_native_app_glue:
	$(CC) -c $(ANDROID_NDK)/sources/android/native_app_glue/android_native_app_glue.c -o $(OUTPUT_PATH)/obj/native_app_glue.o $(CFLAGS)
	$(AR) rcs $(OUTPUT_PATH)/obj/libnative_app_glue.a $(OUTPUT_PATH)/obj/native_app_glue.o

# Compile project code as shared libraries
# OUTPUT: $(PROJECT_DIR)/lib/lib$(LIBRARY_NAME).so 
compile_project_code:
	$(CC) -c $(SOURCE_PATH)/game_basic.c -o $(OUTPUT_PATH)/obj/game_basic.o $(INCLUDE_PATHS) $(CFLAGS) --sysroot=$(ANDROID_TOOLCHAIN)/sysroot 
	$(CC) -o $(OUTPUT_PATH)/lib/armeabi-v7a/lib$(LIBRARY_NAME).so $(OUTPUT_PATH)/obj/game_basic.o -shared $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS)

compile_project_code_raylib_stripped:
	$(CC) -c $(SOURCE_PATH)/raylib_stripped/core.c -o $(OUTPUT_PATH)/obj/core.o $(INCLUDE_PATHS) $(CFLAGS) --sysroot=$(ANDROID_TOOLCHAIN)/sysroot
	$(CC) -c $(SOURCE_PATH)/raylib_stripped/rlgl.c -o $(OUTPUT_PATH)/obj/rlgl.o $(INCLUDE_PATHS) $(CFLAGS) --sysroot=$(ANDROID_TOOLCHAIN)/sysroot -DGRAPHICS_API_OPENGL_ES2
	$(CC) -c $(SOURCE_PATH)/raylib_stripped/utils.c -o $(OUTPUT_PATH)/obj/utils.o $(INCLUDE_PATHS) $(CFLAGS) --sysroot=$(ANDROID_TOOLCHAIN)/sysroot
	$(CC) -c $(SOURCE_PATH)/game_raylib_stripped.c -o $(OUTPUT_PATH)/obj/game_raylib_stripped.o $(INCLUDE_PATHS) $(CFLAGS) --sysroot=$(ANDROID_TOOLCHAIN)/sysroot
	$(CC) -o lib/armeabi-v7a/lib$(LIBRARY_NAME).so $(OUTPUT_PATH)/obj/game_raylib_stripped.o $(OUTPUT_PATH)/obj/core.o $(OUTPUT_PATH)/obj/rlgl.o $(OUTPUT_PATH)/obj/utils.o -shared $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS)
    
copy_project_shared_libs:
ifeq ($(RAYLIB_LIBTYPE),SHARED)
	copy /Y $(LIBS_PATH)\libraylib.so $(OUTPUT_PATH)\lib\armeabi-v7a\libraylib.so 
endif
ifeq ($(OPENAL_LIBTYPE),SHARED)
	copy /Y $(LIBS_PATH)\libopenal.so $(OUTPUT_PATH)\lib\armeabi-v7a\libopenal.so
endif

copy_project_resources:
	xcopy res $(OUTPUT_PATH)\res /Y /E /F
	xcopy src\resources $(OUTPUT_PATH)\assets /Y /E /F
ifeq ($(RAYLIB_LIBTYPE),SHARED)
	xcopy AndroidManifest_shared.xml $(OUTPUT_PATH)\AndroidManifest.xml* /Y /F
endif
ifeq ($(RAYLIB_LIBTYPE),STATIC)
	xcopy AndroidManifest_static.xml $(OUTPUT_PATH)\AndroidManifest.xml* /Y /F
endif

generate_loader_script:
	@echo "package com.raylib.game_sample; public class NativeLoader extends android.app.NativeActivity { static { System.loadLibrary("openal"); System.loadLibrary("raylib"); System.loadLibrary("raylib_game"); }}" > $(OUTPUT_PATH)/obj/NativeLoader.java

# Generate key for APK signing
# OUTPUT: $(PROJECT_DIR)/temp/$(PROJECT_NAME).keystore
generate_apk_keystore: 
	$(JAVA_HOME)/bin/keytool -genkeypair -validity 1000 -dname "CN=raylib,O=Android,C=JPN" -keystore $(OUTPUT_PATH)/$(PROJECT_NAME).keystore -storepass $(KEYSTORE_PASS) -keypass $(KEYSTORE_PASS) -alias $(PROJECT_NAME)Key -keyalg RSA

# Create temp/src/com/raylib/$(LIBRARY_NAME)/R.java
# OUTPUT: $(PROJECT_DIR)/temp/src/com/raylib/$(LIBRARY_NAME)/R.java
# NOTE: DEPENDS on res/values/strings.xml
config_project_package:
	$(ANDROID_BUILD_TOOLS)/aapt package -f -m -S $(OUTPUT_PATH)/res -J $(OUTPUT_PATH)/src -M $(OUTPUT_PATH)/AndroidManifest.xml -I $(ANDROID_HOME)/platforms/android-16/android.jar

# Create temp/obj/com/raylib/$(LIBRARY_NAME)/R.class   
# OUTPUT: $(PROJECT_DIR)/temp/obj/com/raylib/$(LIBRARY_NAME)/R.class
compile_project_class:
	$(JAVA_HOME)/bin/javac -verbose -source 1.7 -target 1.7 -d $(OUTPUT_PATH)/obj -bootclasspath $(JAVA_HOME)/jre/lib/rt.jar -classpath $(ANDROID_HOME)/platforms/android-16/android.jar;$(OUTPUT_PATH)/obj -sourcepath $(OUTPUT_PATH)/src $(OUTPUT_PATH)/src/com/raylib/game_sample/R.java $(NATIVE_LOADER)
#$(JAVA_HOME)/bin/javac -source 1.7 -target 1.7 -d temp/obj -bootclasspath $(JAVA_HOME)/jre/lib/rt.jar -classpath $(ANDROID_HOME)/platforms/android-16/android.jar -sourcepath temp/src temp/src/com/raylib/game_sample/R.java

# Create temp/bin/classes.dex
# OUTPUT: $(PROJECT_DIR)/bin/classes.dex
# NOTE: DEPENDS on temp/obj/com/raylib/$(LIBRARY_NAME)/R.class
compile_project_class_dex:
	$(ANDROID_BUILD_TOOLS)/dx --verbose --dex --output=$(OUTPUT_PATH)/bin/classes.dex $(OUTPUT_PATH)/obj

# Create temp/bin/$(PROJECT_NAME).unsigned.apk
# NOTE: DEPENDS on temp/bin/classes.dex and lib/lib$(LIBRARY_NAME).so
# NOTE: Use -A resources to define additional directory in which to find raw asset files
create_project_apk_package:
	$(ANDROID_BUILD_TOOLS)/aapt package -f -M $(OUTPUT_PATH)/AndroidManifest.xml -S $(OUTPUT_PATH)/res -A $(OUTPUT_PATH)/assets -I $(ANDROID_HOME)/platforms/android-16/android.jar -F $(OUTPUT_PATH)/bin/$(PROJECT_NAME).unsigned.apk $(OUTPUT_PATH)/bin
	cd $(OUTPUT_PATH) && $(ANDROID_BUILD_TOOLS)/aapt add bin/$(PROJECT_NAME).unsigned.apk lib/armeabi-v7a/lib$(LIBRARY_NAME).so $(PROJECT_SHARED_LIBS)

# Create temp/bin/$(PROJECT_NAME).signed.apk
sign_project_apk_package:
	$(JAVA_HOME)/bin/jarsigner -keystore $(OUTPUT_PATH)/$(PROJECT_NAME).keystore -storepass $(KEYSTORE_PASS) -keypass $(KEYSTORE_PASS) -signedjar $(OUTPUT_PATH)/bin/$(PROJECT_NAME).signed.apk $(OUTPUT_PATH)/bin/$(PROJECT_NAME).unsigned.apk $(PROJECT_NAME)Key

# Create temp/bin/$(PROJECT_NAME).apk 
zipalign_project_apk_package:
	$(ANDROID_BUILD_TOOLS)/zipalign -f 4 $(OUTPUT_PATH)/bin/$(PROJECT_NAME).signed.apk $(PROJECT_NAME).apk

# Deploy $(PROJECT_NAME).apk to device
install_project_apk_package:
	$(ANDROID_PLATFORM_TOOLS)/adb install -r $(PROJECT_NAME).apk
    
logcat_project_apk_package:
	$(ANDROID_PLATFORM_TOOLS)/adb logcat -c
	$(ANDROID_PLATFORM_TOOLS)/adb logcat raylib:V *:S
    
deploy:
	$(ANDROID_PLATFORM_TOOLS)/adb install -r $(PROJECT_NAME).apk
	$(ANDROID_PLATFORM_TOOLS)/adb logcat -c
	$(ANDROID_PLATFORM_TOOLS)/adb logcat *:W

#$(ANDROID_PLATFORM_TOOLS)/adb logcat *:W
#$(ANDROID_PLATFORM_TOOLS)/adb -d logcat raylib:V *:S

# Clean everything
clean:
	del temp\bin\* lib\* temp\obj\* temp\src\* /f/s/q
	del temp\*.keystore
	rmdir temp /s /q
	@echo Cleaning done

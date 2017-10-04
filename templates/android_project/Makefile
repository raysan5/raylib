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
PROJECT_NAME = raylibGame
PROJECT_DIR = ./

# Generated shared library name
# NOTE: It should match the name defined in the AndroidManifest.xml
LIBRARY_NAME = raylib_game

# define libtype for raylib and OpenAL Soft: STATIC (.a) or SHARED (.so/.dll)
RAYLIB_LIBTYPE ?= SHARED
OPENAL_LIBTYPE ?= SHARED

# add shared libs to APK if required
ifeq ($(RAYLIB_LIBTYPE),SHARED)
    PROJECT_SHARED_LIBS = lib/armeabi-v7a/libraylib.so 
endif
ifeq ($(OPENAL_LIBTYPE),SHARED)
    PROJECT_SHARED_LIBS += lib/armeabi-v7a/libopenal.so
endif

# Generated key pass
KEYSTORE_PASS = raylib

# Required path variables
# NOTE: JAVA_HOME must be set to JDK
ANDROID_HOME = C:/android-sdk
ANDROID_NDK = C:/android-ndk
ANDROID_TOOLCHAIN = C:/android_toolchain_arm_api16
ANDROID_BUILD_TOOLS = C:/android-sdk/build-tools/26.0.1
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
INCLUDE_PATHS = -I. -Isrc/include -I$(ANDROID_NDK)/sources/android/native_app_glue

# Linker options
LDFLAGS = -Wl,-soname,lib$(LIBRARY_NAME).so -Wl,--exclude-libs,libatomic.a 
LDFLAGS += -Wl,--build-id -Wl,--no-undefined -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now -Wl,--warn-shared-textrel -Wl,--fatal-warnings 
# Force linking of library module to define symbol
LDFLAGS += -u ANativeActivity_onCreate
# Library paths containing required libs
LDFLAGS += -L. -Lsrc -Llib -Lsrc/libs

# Define any libraries to link into executable
# if you want to link libraries (libname.so or libname.a), use the -lname
LDLIBS = -lnative_app_glue -lraylib -lopenal -llog -landroid -lEGL -lGLESv2 -lOpenSLES -latomic -lc -lm -ldl

# Building APK
# NOTE: typing 'make' will invoke the default target entry called 'all',
all: project_dirs \
     native_app_glue \
     project_code_ok \
     gen_keystore \
     project_package \
     project_class \
     project_class_dex \
     project_apk \
     apk_signing \
     apk_zip_align

# Create required temp directories for APK building
project_dirs:
	if not exist temp mkdir temp 
	if not exist temp\obj mkdir temp\obj
	if not exist temp\src mkdir temp\src
	if not exist lib mkdir lib
	if not exist lib\armeabi-v7a mkdir lib\armeabi-v7a
	if not exist temp\bin mkdir temp\bin
     
# Compile native_app_glue as static library
# OUTPUT: $(PROJECT_DIR)/temp/obj/libnative_app_glue.a
native_app_glue:
	$(CC) -c $(ANDROID_NDK)/sources/android/native_app_glue/android_native_app_glue.c -o temp/obj/native_app_glue.o $(CFLAGS)
	$(AR) rcs $(PROJECT_DIR)/lib/libnative_app_glue.a temp/obj/native_app_glue.o

# Compile project code as shared libraries
# OUTPUT: $(PROJECT_DIR)/lib/lib$(LIBRARY_NAME).so 
project_code_ok:
	$(CC) -c src/game_basic.c -o temp/obj/game_basic.o $(INCLUDE_PATHS) $(CFLAGS) --sysroot=$(ANDROID_TOOLCHAIN)/sysroot 
	$(CC) -o lib/armeabi-v7a/lib$(LIBRARY_NAME).so temp/obj/game_basic.o -shared $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS)

project_code:
	$(CC) -c src/core.c -o temp/obj/core.o $(INCLUDE_PATHS) $(CFLAGS) --sysroot=$(ANDROID_TOOLCHAIN)/sysroot
	$(CC) -c src/rlgl.c -o temp/obj/rlgl.o $(INCLUDE_PATHS) $(CFLAGS) --sysroot=$(ANDROID_TOOLCHAIN)/sysroot -DGRAPHICS_API_OPENGL_ES2
	$(CC) -c src/utils.c -o temp/obj/utils.o $(INCLUDE_PATHS) $(CFLAGS) --sysroot=$(ANDROID_TOOLCHAIN)/sysroot
	$(CC) -c src/game_crash.c -o temp/obj/game_crash.o $(INCLUDE_PATHS) $(CFLAGS) --sysroot=$(ANDROID_TOOLCHAIN)/sysroot
	$(CC) -o lib/armeabi-v7a/lib$(LIBRARY_NAME).so temp/obj/game_crash.o temp/obj/core.o temp/obj/rlgl.o temp/obj/utils.o -shared $(INCLUDE_PATHS) $(LDFLAGS) $(LDLIBS)
    
# Generate key for APK signing
# OUTPUT: $(PROJECT_DIR)/temp/$(PROJECT_NAME).keystore
gen_keystore: 
	$(JAVA_HOME)/bin/keytool -genkeypair -validity 1000 -dname "CN=raylib,O=Android,C=JPN" -keystore temp/$(PROJECT_NAME).keystore -storepass $(KEYSTORE_PASS) -keypass $(KEYSTORE_PASS) -alias $(PROJECT_NAME)Key -keyalg RSA

# Create temp/src/com/raylib/$(LIBRARY_NAME)/R.java
# OUTPUT: $(PROJECT_DIR)/temp/src/com/raylib/$(LIBRARY_NAME)/R.java
# NOTE: DEPENDS on res/values/strings.xml
project_package:
	$(ANDROID_BUILD_TOOLS)/aapt package -v -f -m -S res -J temp/src -M AndroidManifest.xml -I $(ANDROID_HOME)/platforms/android-16/android.jar

# Create temp/obj/com/raylib/$(LIBRARY_NAME)/R.class   
# OUTPUT: $(PROJECT_DIR)/temp/obj/com/raylib/$(LIBRARY_NAME)/R.class
project_class:
	$(JAVA_HOME)/bin/javac -verbose -source 1.7 -target 1.7 -d temp/obj -bootclasspath $(JAVA_HOME)/jre/lib/rt.jar -classpath $(ANDROID_HOME)/platforms/android-16/android.jar;temp/obj -sourcepath temp/src temp/src/com/raylib/game_sample/R.java
#$(JAVA_HOME)/bin/javac -source 1.7 -target 1.7 -d temp/obj -bootclasspath $(JAVA_HOME)/jre/lib/rt.jar -classpath $(ANDROID_HOME)/platforms/android-16/android.jar -sourcepath temp/src temp/src/com/raylib/game_sample/R.java

# Create temp/bin/classes.dex
# OUTPUT: $(PROJECT_DIR)/bin/classes.dex
# NOTE: DEPENDS on temp/obj/com/raylib/$(LIBRARY_NAME)/R.class
project_class_dex:
	$(ANDROID_BUILD_TOOLS)/dx --verbose --dex --output=temp/bin/classes.dex temp/obj

# Create temp/bin/$(PROJECT_NAME).unsigned.apk
# NOTE: DEPENDS on temp/bin/classes.dex and lib/lib$(LIBRARY_NAME).so
# NOTE: Use -A resources to define additional directory in which to find raw asset files
project_apk:
	$(ANDROID_BUILD_TOOLS)/aapt package -v -f -M AndroidManifest.xml -S res -A assets -I $(ANDROID_HOME)/platforms/android-16/android.jar -F temp/bin/$(PROJECT_NAME).unsigned.apk temp/bin
	$(ANDROID_BUILD_TOOLS)/aapt add -v $(PROJECT_DIR)/temp/bin/$(PROJECT_NAME).unsigned.apk lib/armeabi-v7a/lib$(LIBRARY_NAME).so $(PROJECT_SHARED_LIBS)

# Create temp/bin/$(PROJECT_NAME).signed.apk
apk_signing:
	$(JAVA_HOME)/bin/jarsigner -keystore temp/$(PROJECT_NAME).keystore -storepass $(KEYSTORE_PASS) -keypass $(KEYSTORE_PASS) -signedjar $(PROJECT_DIR)/temp/bin/$(PROJECT_NAME).signed.apk temp/bin/$(PROJECT_NAME).unsigned.apk $(PROJECT_NAME)Key

# Create temp/bin/$(PROJECT_NAME).apk 
apk_zip_align:
	$(ANDROID_BUILD_TOOLS)/zipalign -f 4 temp/bin/$(PROJECT_NAME).signed.apk $(PROJECT_NAME).apk

# Deploy $(PROJECT_NAME).apk to device
deploy:
	$(ANDROID_HOME)/platform-tools/adb install -r $(PROJECT_NAME).apk
	$(ANDROID_HOME)/platform-tools/adb logcat -c
	$(ANDROID_HOME)/platform-tools/adb -d logcat raylib:V *:S

#$(ANDROID_HOME)/platform-tools/adb logcat *:W
#$(ANDROID_HOME)/platform-tools/adb -d logcat raylib:V *:S

# Clean everything
clean:
	del temp\bin\* lib\* temp\obj\* temp\src\* /f/s/q
	del temp\*.keystore
	rmdir temp /s /q
	@echo Cleaning done

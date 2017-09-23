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
PROJECT_NAME = NativeActivity
PROJECT_DIR = ./

# Generated shared library name
# NOTE: It should match the name defined in the AndroidManifest.xml
LIBRARY_NAME = raylib_game

# Generated key pass
KEYSTORE_PASS = raylib

# Required path variables
# NOTE: JAVA_HOME must be set to JDK
ANDROID_HOME = C:/android-sdk
ANDROID_NDK = C:/android-ndk
ANDROID_TOOLCHAIN = C:/android_toolchain_arm_api16
ANDROID_BUILD_TOOLS = C:/android-sdk/build-tools/26.0.1
JAVA_HOME = C:/PROGRA~1/Java/jdk1.8.0_25

# Compilers
CC = $(ANDROID_TOOLCHAIN)/bin/arm-linux-androideabi-gcc
AR = $(ANDROID_TOOLCHAIN)/bin/arm-linux-androideabi-ar

# Define compiler flags
CFLAGS = -O2 -s -Wall -std=c99 -DPLATFORM_ANDROID -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16

# Define any directories containing required header files
INCLUDES = -I. -Ijni/include -I$(ANDROID_NDK)/sources/android/native_app_glue

# Define library paths containing required libs
LFLAGS = -L. -Ljni/libs -Ljni -Ltemp/lib

# Define any libraries to link into executable
# if you want to link libraries (libname.so or libname.a), use the -lname
LIBS = -lraylib -lopenal -llog -landroid -lEGL -lGLESv2 -lOpenSLES

# Building APK
# NOTE: typing 'make' will invoke the default target entry called 'all',
all: project_dirs \
     native_app_glue \
     project_code \
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
	if not exist temp\lib mkdir temp\lib
	if not exist temp\bin mkdir temp\bin
     
# Compile native_app_glue as static library
# OUTPUT: $(PROJECT_DIR)/temp/obj/libnative_app_glue.a
native_app_glue:
	$(CC) -c $(ANDROID_NDK)/sources/android/native_app_glue/android_native_app_glue.c -o temp/obj/native_app_glue.o $(CFLAGS)
	$(AR) rcs $(PROJECT_DIR)/temp/lib/libnative_app_glue.a temp/obj/native_app_glue.o

# Compile project code as shared libraries
# OUTPUT: $(PROJECT_DIR)/temp/lib/lib$(LIBRARY_NAME).so 
project_code:
	$(CC) -c jni/basic_game.c -o temp/obj/basic_game.o $(INCLUDES) $(CFLAGS) --sysroot=$(ANDROID_TOOLCHAIN)/sysroot -fPIC 
	$(CC) -o temp/lib/lib$(LIBRARY_NAME).so temp/obj/basic_game.o -shared $(INCLUDES) $(LFLAGS) $(LIBS) -lnative_app_glue

# Generate key for APK signing
# OUTPUT: $(PROJECT_DIR)/temp/$(PROJECT_NAME).keystore
gen_keystore: 
	$(JAVA_HOME)/bin/keytool -genkeypair -validity 1000 -dname "CN=raylib,O=Android,C=JPN" -keystore temp/$(PROJECT_NAME).keystore -storepass $(KEYSTORE_PASS) -keypass $(KEYSTORE_PASS) -alias $(PROJECT_NAME)Key -keyalg RSA

# Create temp/src/com/raylib/$(LIBRARY_NAME)/R.java
# OUTPUT: $(PROJECT_DIR)/temp/src/com/raylib/$(LIBRARY_NAME)/R.java
# NOTE: DEPENDS on res/values/strings.xml
project_package:
	$(ANDROID_BUILD_TOOLS)/aapt package -f -m -S res -J temp/src -M AndroidManifest.xml -I $(ANDROID_HOME)/platforms/android-16/android.jar

# Create temp/obj/com/raylib/$(LIBRARY_NAME)/R.class   
# OUTPUT: $(PROJECT_DIR)/temp/obj/com/raylib/$(LIBRARY_NAME)/R.class
project_class:
	$(JAVA_HOME)/bin/javac -source 1.7 -target 1.7 -d temp/obj -classpath $(ANDROID_HOME)/platforms/android-16/android.jar -sourcepath temp/src temp/src/com/raylib/game_sample/R.java

# Create temp/bin/classes.dex
# OUTPUT: $(PROJECT_DIR)/bin/classes.dex
# NOTE: DEPENDS on temp/obj/com/raylib/$(LIBRARY_NAME)/R.class
project_class_dex:
	$(ANDROID_BUILD_TOOLS)/dx --dex --output=temp/bin/classes.dex temp/obj

# Create temp/bin/$(PROJECT_NAME).unsigned.apk
# NOTE: DEPENDS on temp/bin/classes.dex and temp/lib/lib$(LIBRARY_NAME).so
# NOTE: Use -A resources to define additional directory in which to find raw asset files
project_apk:
	$(ANDROID_BUILD_TOOLS)/aapt package -f -m -M AndroidManifest.xml -S res -A assets -I $(ANDROID_HOME)/platforms/android-16/android.jar -F temp/bin/$(PROJECT_NAME).unsigned.apk -J temp/bin
	$(ANDROID_BUILD_TOOLS)/aapt add $(PROJECT_DIR)/temp/bin/$(PROJECT_NAME).unsigned.apk temp/lib/lib$(LIBRARY_NAME).so

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
	$(ANDROID_HOME)/platform-tools/adb logcat *:W

# Clean everything
clean:
	del temp\bin\* temp\lib\* temp\obj\* temp\src\* /f/s/q
	del temp\*.keystore
	rmdir temp /s /q
	@echo Cleaning done

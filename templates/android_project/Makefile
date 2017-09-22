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

# Android project name (.apk)  
PROJECT_NAME = NativeActivity
PROJECT_DIR = ./

# define raylib platform to compile for
PLATFORM ?= PLATFORM_ANDROID


# Required path variables
# NOTE: JAVA_HOME must be set to JDK
ANDROID_HOME = C:/android-sdk
ANDROID_NDK = C:/android-ndk
ANDROID_TOOLCHAIN = C:/android_toolchain_arm_api16
ANDROID_BUILD_TOOLS = C:/android-sdk/build-tools/25.0.3

# Compilers
CC = $(ANDROID_TOOLCHAIN)/bin/arm-linux-androideabi-gcc
AR = $(ANDROID_TOOLCHAIN)/bin/arm-linux-androideabi-ar

# define compiler flags
CFLAGS = -O2 -s -Wall -std=c99 -DPLATFORM_ANDROID -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16

# define any directories containing required header files
INCLUDES = -I. -Ijni/include -I$(ANDROID_NDK)/sources/android/native_app_glue

# define library paths containing required libs
LFLAGS = -L. -Ljni/libs -Ljni

# define any libraries to link into executable
# if you want to link libraries (libname.so or libname.a), use the -lname
LIBS = -lraylib -lopenal -llog -landroid -lEGL -lGLESv2 -lOpenSLES

# Building APK
#-----------------
# typing 'make' will invoke the default target entry called 'all',
all: native_app_glue \
     project_code \
     gen_keystore \
     project_package \
     project_class \
     project_class_dex \
     project_apk \
     apk_signing \
     apk_zip_align

# compile native_app_glue static library
# OUTPUT $(PROJECT_DIR)/jni/libnative_app_glue.a
native_app_glue:
	$(CC) -c $(ANDROID_NDK)/sources/android/native_app_glue/android_native_app_glue.c -o jni/native_app_glue.o $(CFLAGS)
	$(AR) rcs $(PROJECT_DIR)/jni/libnative_app_glue.a jni/native_app_glue.o

# compile project code as shared libraries
# OUTPUT $(PROJECT_DIR)/lib/arm64-v8a/libnative-activity.so 
project_code:
	$(CC) -c jni/basic_game.c -o jni/basic_game.o $(INCLUDES) -I$(ANDROID_NDK)/sources/android/native_app_glue $(CFLAGS) --sysroot=$(ANDROID_TOOLCHAIN)/sysroot -fPIC 
	$(CC) -o lib/libnative-activity.so jni/basic_game.o -shared $(INCLUDES) $(LFLAGS) $(LIBS) -lnative_app_glue

# Generate key for APK
# OUTPUT $(PROJECT_DIR)/ToyKey.keystore
gen_keystore: 
#	$(JAVA_HOME)/bin/keytool -genkeypair -validity 1000 -dname "CN=raylib,O=Android,C=JPN" -keystore ToyKey.keystore -storepass raylib -keypass raylib -alias $(PROJECT_NAME)Key -keyalg RSA

# Creating src/com/example/native_activity/R.java
# OUTPUT $(PROJECT_DIR)/src/com/example/native_activity/R.java
# NOTE: DEPENDS on res/values/strings.xml
project_package:
	$(ANDROID_BUILD_TOOLS)/aapt package -f -m -S res -J src -M AndroidManifest.xml -I $(ANDROID_HOME)/platforms/android-16/android.jar

# Creating obj/com/example/native_activity/R.class   
# OUTPUT $(PROJECT_DIR)/obj/com/example/native_activity/R.class
project_class:
	$(JAVA_HOME)/bin/javac -source 1.7 -target 1.7 -d obj -classpath $(ANDROID_HOME)/platforms/android-16/android.jar -sourcepath src src/com/raylib/game_sample/R.java

# Creating bin/classes/dex
# OUTPUT $(PROJECT_DIR)/bin/classes.dex
# NOTE: DEPENDS on obj/com/example/native_activity/R.class
project_class_dex:
	$(ANDROID_BUILD_TOOLS)/dx --dex --output=bin/classes.dex obj

# Creating bin/$(PROJECT_NAME).unsigned.apk
# NOTE: DEPENDS on bin/classes.dex lib/arm64-v8a/libnative-activity.so
# Use -A resources to define additional directory in which to find raw asset files
project_apk:
	$(ANDROID_BUILD_TOOLS)/aapt package -f -m -M AndroidManifest.xml -S res -A assets -I $(ANDROID_HOME)/platforms/android-16/android.jar -F bin/$(PROJECT_NAME).unsigned.apk -J bin
	$(ANDROID_BUILD_TOOLS)/aapt add $(PROJECT_DIR)/bin/$(PROJECT_NAME).unsigned.apk lib/libnative-activity.so

# Creating bin/$(PROJECT_NAME).signed.apk
apk_signing:
	$(JAVA_HOME)/bin/jarsigner -keystore ToyKey.keystore -storepass raylib -keypass raylib -signedjar $(PROJECT_DIR)/bin/$(PROJECT_NAME).signed.apk bin/$(PROJECT_NAME).unsigned.apk $(PROJECT_NAME)Key

# Creating bin/$(PROJECT_NAME).apk 
apk_zip_align:
	$(ANDROID_BUILD_TOOLS)/zipalign -f 4 bin/$(PROJECT_NAME).signed.apk bin/$(PROJECT_NAME).apk

# Deploy to device
deploy:
	$(ANDROID_HOME)/platform-tools/adb install -r bin/$(PROJECT_NAME).apk
	$(ANDROID_HOME)/platform-tools/adb logcat -c
	$(ANDROID_HOME)/platform-tools/adb logcat *:W

# clean everything
clean:
	del bin\* lib\* obj\* src\* /f/s/q
	@echo Cleaning done

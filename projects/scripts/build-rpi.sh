#!/bin/sh
# Change your executable name here
GAME_NAME="game"

# Set your sources here (relative paths!)
# Example with two source folders:
# SOURCES="src/*.c src/submodule/*.c"
SOURCES="core_basic_window.c"

# Set your raylib/src location here (relative path!)
RAYLIB_SRC="../../src"

# About this build script: it does many things, but in essence, it's
# very simple. It has 3 compiler invocations: building raylib (which
# is not done always, see logic by searching "Build raylib"), building
# src/*.c files, and linking together those two. Each invocation is
# wrapped in an if statement to make the -qq flag work, it's pretty
# verbose, sorry.

# Stop the script if a compilation (or something else?) fails
set -e

# Get arguments
while getopts ":hdusrcq" opt; do
    case $opt in
        h)
            echo "Usage: ./build-rpi.sh [-hdusrcqq]"
            echo " -h  Show this information"
            echo " -d  Faster builds that have debug symbols, and enable warnings"
            echo " -u  Run upx* on the executable after compilation (before -r)"
            echo " -s  Run strip on the executable after compilation (before -r)"
            echo " -r  Run the executable after compilation"
            echo " -c  Remove the temp/(debug|release) directory, ie. full recompile"
            echo " -q  Suppress this script's informational prints"
            echo " -qq Suppress all prints, complete silence (> /dev/null 2>&1)"
            echo ""
            echo "* This is mostly here to make building simple \"shipping\" versions"
            echo "  easier, and it's a very small bit in the build scripts. The option"
            echo "  requires that you have upx installed and on your path, of course."
            echo ""
            echo "Examples:"
            echo " Build a release build:                    ./build-rpi.sh"
            echo " Build a release build, full recompile:    ./build-rpi.sh -c"
            echo " Build a debug build and run:              ./build-rpi.sh -d -r"
            echo " Build in debug, run, don't print at all:  ./build-rpi.sh -drqq"
            exit 0
            ;;
        d)
            BUILD_DEBUG="1"
            ;;
        u)
            UPX_IT="1"
            ;;
        s)
            STRIP_IT="1"
            ;;
        r)
            RUN_AFTER_BUILD="1"
            ;;
        c)
            BUILD_ALL="1"
            ;;
        q)
            if [ -n "$QUIET" ]; then
                REALLY_QUIET="1"
            else
                QUIET="1"
            fi
            ;;
        \?)
            echo "Invalid option: -$OPTARG" >&2
            exit 1
            ;;
    esac
done

# Set CC if it's not set already
if [ -z "$CC" ]; then
    CC=cc
fi

# Directories
ROOT_DIR=$PWD
SOURCES="$ROOT_DIR/$SOURCES"
RAYLIB_SRC="$ROOT_DIR/$RAYLIB_SRC"

# Flags
OUTPUT_DIR="builds/linux"
COMPILATION_FLAGS="-std=c99 -Os -flto"
FINAL_COMPILE_FLAGS="-s"
WARNING_FLAGS="-Wall -Wextra -Wpedantic"
LINK_FLAGS="-flto -lm -ldl -lrt -lpthread -lv4l2 -lbrcmGLESv2 -lbrcmEGL -lbcm_host -L/opt/vc/lib"
# Debug changes to flags
if [ -n "$BUILD_DEBUG" ]; then
    OUTPUT_DIR="builds-debug/linux"
    COMPILATION_FLAGS="-std=c99 -O0 -g"
    FINAL_COMPILE_FLAGS=""
    LINK_FLAGS="-lm -ldl -lrt -lpthread -lv4l2 -lbrcmGLESv2 -lbrcmEGL -lbcm_host -L/opt/vc/lib"
fi

# Display what we're doing
if [ -n "$BUILD_DEBUG" ]; then
    [ -z "$QUIET" ] && echo "COMPILE-INFO: Compiling in debug mode. ($COMPILATION_FLAGS $WARNING_FLAGS)"
else
    [ -z "$QUIET" ] && echo "COMPILE-INFO: Compiling in release mode. ($COMPILATION_FLAGS $FINAL_COMPILE_FLAGS)"
fi

# Create the raylib cache directory
TEMP_DIR="temp/release"
if [ -n "$BUILD_DEBUG" ]; then
    TEMP_DIR="temp/debug"
fi
# If there's a -c flag, remove the cache
if [ -d "$TEMP_DIR" ] && [ -n "$BUILD_ALL" ]; then
    [ -z "$QUIET" ] && echo "COMPILE-INFO: Found cached raylib, rebuilding."
    rm -r "$TEMP_DIR"
fi
# If temp directory doesn't exist, build raylib
if [ ! -d "$TEMP_DIR" ]; then
    mkdir -p $TEMP_DIR
    cd $TEMP_DIR
    RAYLIB_DEFINES="-D_DEFAULT_SOURCE -DPLATFORM_RPI -DGRAPHICS_API_OPENGL_ES2"
    RAYLIB_C_FILES="$RAYLIB_SRC/rcore.c $RAYLIB_SRC/rshapes.c $RAYLIB_SRC/rtextures.c $RAYLIB_SRC/rtext.c $RAYLIB_SRC/rmodels.c $RAYLIB_SRC/utils.c $RAYLIB_SRC/raudio.c"
    RAYLIB_INCLUDE_FLAGS="-I$RAYLIB_SRC -I/opt/vc/include"

    if [ -n "$REALLY_QUIET" ]; then
        $CC -c $RAYLIB_DEFINES $RAYLIB_INCLUDE_FLAGS $COMPILATION_FLAGS $RAYLIB_C_FILES > /dev/null 2>&1
    else
        $CC -c $RAYLIB_DEFINES $RAYLIB_INCLUDE_FLAGS $COMPILATION_FLAGS $RAYLIB_C_FILES
    fi
    [ -z "$QUIET" ] && echo "COMPILE-INFO: raylib compiled into object files in: $TEMP_DIR/"
    cd $ROOT_DIR
fi

# Build the actual game
mkdir -p $OUTPUT_DIR
cd $OUTPUT_DIR
[ -z "$QUIET" ] && echo "COMPILE-INFO: Compiling game code."
if [ -n "$REALLY_QUIET" ]; then
    $CC -c -I$RAYLIB_SRC $COMPILATION_FLAGS $WARNING_FLAGS $SOURCES > /dev/null 2>&1
    $CC -o $GAME_NAME $ROOT_DIR/$TEMP_DIR/*.o *.o $LINK_FLAGS > /dev/null 2>&1
else
    $CC -c -I$RAYLIB_SRC $COMPILATION_FLAGS $WARNING_FLAGS $SOURCES
    $CC -o $GAME_NAME $ROOT_DIR/$TEMP_DIR/*.o *.o $LINK_FLAGS
fi
rm *.o
[ -z "$QUIET" ] && echo "COMPILE-INFO: Game compiled into an executable in: $OUTPUT_DIR/"

if [ -n "$STRIP_IT" ]; then
    [ -z "$QUIET" ] && echo "COMPILE-INFO: Stripping $GAME_NAME."
    strip $GAME_NAME
fi

if [ -n "$UPX_IT" ]; then
    [ -z "$QUIET" ] && echo "COMPILE-INFO: Packing $GAME_NAME with upx."
    upx $GAME_NAME > /dev/null 2>&1
fi

if [ -n "$RUN_AFTER_BUILD" ]; then
    [ -z "$QUIET" ] && echo "COMPILE-INFO: Running."
    if [ -n "$REALLY_QUIET" ]; then
        ./$GAME_NAME > /dev/null 2>&1
    else
        ./$GAME_NAME
    fi
fi
cd $ROOT_DIR

[ -z "$QUIET" ] && echo "COMPILE-INFO: All done."

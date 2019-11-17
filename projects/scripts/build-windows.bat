@echo off
REM Change your executable name here
set GAME_NAME=game.exe

REM Set your sources here (relative paths!)
REM Example with two source folders:
REM set SOURCES=src\*.c src\submodule\*.c
set SOURCES=core_basic_window.c

REM Set your raylib\src location here (relative path!)
set RAYLIB_SRC=..\..\src

REM About this build script: it does many things, but in essence, it's
REM very simple. It has 3 compiler invocations: building raylib (which
REM is not done always, see logic by searching "Build raylib"), building
REM src/*.c files, and linking together those two. Each invocation is
REM wrapped in an if statement to make the -qq flag work, it's pretty
REM verbose, sorry.

REM To skip to the actual building part of the script, search for ":BUILD"

REM For the ! variable notation
setlocal EnableDelayedExpansion
REM For shifting, which the command line argument parsing needs
setlocal EnableExtensions


:ARG_LOOP
set ARG=%1
if "!ARG!" == "" ( goto PREPARE )
IF NOT "x!ARG!" == "x!ARG:h=!" (
  goto HELP
)
IF NOT "x!ARG!" == "x!ARG:d=!" (
  set BUILD_DEBUG=1
)
IF NOT "x!ARG!" == "x!ARG:u=!" (
  set UPX_IT=1
)
IF NOT "x!ARG!" == "x!ARG:r=!" (
  set RUN_AFTER_BUILD=1
)
IF NOT "x!ARG!" == "x!ARG:c=!" (
  set BUILD_ALL=1
)
IF NOT "x!ARG!" == "x!ARG:qq=!" (
  set QUIET=1
  set REALLY_QUIET=1
) ELSE IF NOT "x!ARG!" == "x!ARG:q=!" (
  IF DEFINED QUIET (
    set REALLY_QUIET=1
  ) ELSE (
    set QUIET=1
  )
)
IF NOT "x!ARG!" == "x!ARG:v=!" (
  set VERBOSE=1
)
IF NOT "%1" == "" (
  shift /1
  goto ARG_LOOP
)


:HELP
echo Usage: windows-build.bat [-hdurcqqv]
echo  -h  Show this information
echo  -d  Faster builds that have debug symbols, and enable warnings
echo  -u  Run upx* on the executable after compilation (before -r)
echo  -r  Run the executable after compilation
echo  -c  Remove the temp\{debug,release} directory, ie. full recompile
echo  -q  Suppress this script's informational prints
echo  -qq Suppress all prints, complete silence
echo  -v  cl.exe normally prints out a lot of superficial information, as
echo      well as the MSVC build environment activation scripts, but these are
echo      mostly suppressed by default. If you do want to see everything, use
echo      this flag.
echo.
echo * This is mostly here to make building simple "shipping" versions
echo   easier, and it's a very small bit in the build scripts. The option
echo   requires that you have upx installed and on your path, of course.
echo.
echo Examples:
echo  Build a release build:                    windows-build.bat
echo  Build a release build, full recompile:    windows-build.bat -c
echo  Build a debug build and run:              windows-build.bat -d -r
echo  Build in debug, run, don't print at all:  windows-build.bat -drqq
exit /B


:PREPARE
REM Activate the msvc build environment
IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" (
  set VC_INIT="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat"
) ELSE IF EXIST "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" (
  set VC_INIT="C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvarsall.bat"
) ELSE IF EXIST "C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools.bat" (
  set VC_INIT="C:\Program Files (x86)\Microsoft Visual C++ Build Tools\vcbuildtools.bat"
) ELSE (
  REM Initialize your vc environment here if the defaults don't work
  REM  set VC_INIT="C:\your\path\here\vcvarsall.bat"
  REM And then remove/comment out the following two lines
  echo "Couldn't find vcvarsall.bat or vcbuildtools.bat, please set it manually."
  exit /B
)
IF DEFINED VERBOSE (
  call !VC_INIT! x86
) ELSE (
  call !VC_INIT! x86 > NUL 2>&1
)


:BUILD
REM Directories
set "ROOT_DIR=%CD%"
set "SOURCES=!ROOT_DIR!\!SOURCES!"
set "RAYLIB_SRC=!ROOT_DIR!\!RAYLIB_SRC!"

REM Flags
set OUTPUT_FLAG=/Fe: "!GAME_NAME!"
set COMPILATION_FLAGS=/O1 /GL
set WARNING_FLAGS=
set SUBSYSTEM_FLAGS=/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup
set LINK_FLAGS=/link /LTCG kernel32.lib user32.lib shell32.lib winmm.lib gdi32.lib opengl32.lib
set OUTPUT_DIR=builds\windows-msvc
REM Debug changes to flags
IF DEFINED BUILD_DEBUG (
  set OUTPUT_FLAG=/Fe: "!GAME_NAME!"
  set COMPILATION_FLAGS=/Od /Zi
  set WARNING_FLAGS=/Wall
  set SUBSYSTEM_FLAGS=
  set LINK_FLAGS=/link kernel32.lib user32.lib shell32.lib winmm.lib gdi32.lib opengl32.lib
  set OUTPUT_DIR=builds-debug\windows-msvc
)
IF NOT DEFINED VERBOSE (
  set VERBOSITY_FLAG=/nologo
)

REM Display what we're doing
IF DEFINED BUILD_DEBUG (
  IF NOT DEFINED QUIET echo COMPILE-INFO: Compiling in debug mode, flags: !COMPILATION_FLAGS! !WARNING_FLAGS!
) ELSE (
  IF NOT DEFINED QUIET echo COMPILE-INFO: Compiling in release mode, flags: !COMPILATION_FLAGS! /link /LTCG
)

REM Create the temp directory for raylib
set "TEMP_DIR=temp\release"
IF DEFINED BUILD_DEBUG (
  set "TEMP_DIR=temp\debug"
)

IF DEFINED BUILD_ALL (
  IF EXIST !TEMP_DIR!\ (
    IF NOT DEFINED QUIET echo COMPILE-INFO: Found cached raylib, rebuilding.
    del /Q !TEMP_DIR!
    rmdir !TEMP_DIR!
  )
)

REM Build raylib if it hasn't been cached in TEMP_DIR
IF NOT EXIST !TEMP_DIR!\ (
  mkdir !TEMP_DIR!
  cd !TEMP_DIR!
  REM Raylib's src folder
  set "RAYLIB_DEFINES=/D_DEFAULT_SOURCE /DPLATFORM_DESKTOP /DGRAPHICS_API_OPENGL_33"
  set RAYLIB_C_FILES="!RAYLIB_SRC!\core.c" "!RAYLIB_SRC!\shapes.c" "!RAYLIB_SRC!\textures.c" "!RAYLIB_SRC!\text.c" "!RAYLIB_SRC!\models.c" "!RAYLIB_SRC!\utils.c" "!RAYLIB_SRC!\raudio.c" "!RAYLIB_SRC!\rglfw.c"
  set RAYLIB_INCLUDE_FLAGS=/I"!RAYLIB_SRC!" /I"!RAYLIB_SRC!\external\glfw\include"

  IF DEFINED REALLY_QUIET (
    cl.exe /w /c !RAYLIB_DEFINES! !RAYLIB_INCLUDE_FLAGS! !COMPILATION_FLAGS! !RAYLIB_C_FILES! > NUL 2>&1 || exit /B
  ) ELSE (
    cl.exe /w /c !VERBOSITY_FLAG! !RAYLIB_DEFINES! !RAYLIB_INCLUDE_FLAGS! !COMPILATION_FLAGS! !RAYLIB_C_FILES! || exit /B
  )
  IF NOT DEFINED QUIET echo COMPILE-INFO: Raylib compiled into object files in: !TEMP_DIR!\

  REM Out of the temp directory
  cd !ROOT_DIR!
)

REM Move to the build directory
IF NOT EXIST !OUTPUT_DIR! mkdir !OUTPUT_DIR!
cd !OUTPUT_DIR!

REM Build the actual game
IF NOT DEFINED QUIET echo COMPILE-INFO: Compiling game code.
IF DEFINED REALLY_QUIET (
  cl.exe !VERBOSITY_FLAG! !COMPILATION_FLAGS! !WARNING_FLAGS! /c /I"!RAYLIB_SRC!" !SOURCES! > NUL 2>&1 || exit /B
  cl.exe !VERBOSITY_FLAG! !OUTPUT_FLAG! "!ROOT_DIR!\!TEMP_DIR!\*.obj" *.obj !LINK_FLAGS! !SUBSYSTEM_FLAGS! > NUL 2>&1 || exit /B
) ELSE (
  cl.exe !VERBOSITY_FLAG! !COMPILATION_FLAGS! !WARNING_FLAGS! /c /I"!RAYLIB_SRC!" !SOURCES! || exit /B
  cl.exe !VERBOSITY_FLAG! !OUTPUT_FLAG! "!ROOT_DIR!\!TEMP_DIR!\*.obj" *.obj !LINK_FLAGS! !SUBSYSTEM_FLAGS! || exit /B
)
del *.obj
IF NOT DEFINED QUIET echo COMPILE-INFO: Game compiled into an executable in: !OUTPUT_DIR!\

REM Run upx
IF DEFINED UPX_IT (
  IF NOT DEFINED QUIET echo COMPILE-INFO: Packing !GAME_NAME! with upx.
  upx !GAME_NAME! > NUL 2>&1
)

REM Finally, run the produced executable
IF DEFINED RUN_AFTER_BUILD (
  IF NOT DEFINED QUIET echo COMPILE-INFO: Running.
  IF DEFINED REALLY_QUIET (
    !GAME_NAME! > NUL 2>&1
  ) ELSE (
    !GAME_NAME!
  )
)

REM Back to development directory
cd !ROOT_DIR!

IF NOT DEFINED QUIET echo COMPILE-INFO: All done.

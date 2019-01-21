::@echo off
:: .
:: Compile your examples using:  raylib_makefile_example.bat core/core_basic_window
:: .
:: > Setup required Environment
:: -------------------------------------
set RAYLIB_INCLUDE_DIR=C:\GitHub\raylib\src
set RAYLIB_LIB_DIR=C:\GitHub\raylib\src
set RAYLIB_RES_FILE=C:\GitHub\raylib\raylib.rc.data
set COMPILER_DIR=C:\raylib\mingw\bin
set PATH=%PATH%;%COMPILER_DIR%
set FILENAME=%1
set FILENAME_FULL_PATH=%~f1
cd %~dp0
:: .
:: > Cleaning latest build
:: ---------------------------
cmd /c if exist %FILENAME_FULL_PATH%.exe del /F %FILENAME_FULL_PATH%.exe
:: .
:: > Compiling program
:: --------------------------
:: -B  : Force make recompilation despite file not changed
mingw32-make %FILENAME% -B PLATFORM=PLATFORM_DESKTOP
:: .
:: > Executing program
:: -------------------------
cmd /c if exist %FILENAME_FULL_PATH%.exe %FILENAME_FULL_PATH%.exe
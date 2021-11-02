::@echo off
:: > Setup required Environment
:: -------------------------------------
set RAYLIB_DIR=C:\raylib
set COMPILER_DIR=C:\raylib\mingw\bin
set PATH=%PATH%;%COMPILER_DIR%
set FILE_NAME=%1
set NAME_PART=%FILE_NAME:~0,-2%
cd %~dp0
:: .
:: > Cleaning latest build
:: ---------------------------
cmd /c if exist %NAME_PART%.exe del /F %NAME_PART%.exe
:: .
:: > Compiling program
:: --------------------------
gcc -o %NAME_PART%.exe %FILE_NAME% %RAYLIB_DIR%\src\raylib.rc.data -s -O2 -I../../src -Iexternal -lraylib -lopengl32 -lgdi32 -lwinmm -std=c99 -Wall -mwindows
:: .
:: . > Executing program
:: -------------------------
cmd /c if exist %NAME_PART%.exe %NAME_PART%.exe
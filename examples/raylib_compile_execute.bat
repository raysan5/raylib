::@echo off
:: .
:: Compile your examples using:  raylib_compile_execute.bat core/core_basic_window.c
:: .
:: > Setup required Environment
:: -------------------------------------
set RAYLIB_INCLUDE_DIR=C:\GitHub\raylib\src
set RAYLIB_LIB_DIR=C:\GitHub\raylib\src
set RAYLIB_RES_FILE=C:\GitHub\raylib\raylib.rc.data
set COMPILER_DIR=C:\raylib\mingw\bin
set PATH=%PATH%;%COMPILER_DIR%
:: Get full filename path for input file %1
set FILENAME=%~f1
set NAMEPART=%FILENAME:~0,-2%
cd %~dp0
:: .
:: > Cleaning latest build
:: ---------------------------
cmd /c if exist %NAMEPART%.exe del /F %NAMEPART%.exe
:: .
:: > Compiling program
:: --------------------------
:: -s        : Remove all symbol table and relocation information from the executable
:: -O2       : Optimization Level 2, this option increases both compilation time and the performance of the generated code
:: -std=c99  : Use C99 language standard
:: -Wall     : Enable all compilation Warnings
:: -mwindows : Compile a Windows executable, no cmd window
gcc -o %NAMEPART%.exe %FILENAME% %RAYLIB_RES_FILE% -s -O2 -I%RAYLIB_INCLUDE_DIR% -I%RAYLIB_LIB_DIR% -lraylib -lopengl32 -lgdi32 -lwinmm -std=c99 -Wall -mwindows
:: .
:: > Executing program
:: -------------------------
cmd /c if exist %NAMEPART%.exe %NAMEPART%.exe
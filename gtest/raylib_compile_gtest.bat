::@echo off
:: .
:: Compile your test using :  raylib_compile_gtest.bat core_ut.cpp
:: .
:: > Setup required Environment
:: -------------------------------------
set GTEST_INCLUDE_DIR=C:\GitHub\googletest\googletest\include\
set GTEST_LIB=C:\GitHub\googletest\mybuild\lib\libgtest.a
set GTEST_LIB_MAIN=C:\GitHub\googletest\mybuild\lib\libgtest_main.a

:: Get full filename path for input file %1
set FILENAME=%~f1
set NAMEPART=%FILENAME:~0,-4%
cd %~dp0
:: .
:: > Cleaning latest build
:: ---------------------------
cmd /c if exist %NAMEPART%.exe del /F %NAMEPART%.exe

:: > Compiling program
:: --------------------------

g++  -c %NAMEPART%.cpp -I%GTEST_INCLUDE_DIR%

:: .
:: > Linking program
:: --------------------------
:: -s        : Remove all symbol table and relocation information from the executable
:: -O2       : Optimization Level 2, this option increases both compilation time and the performance of the generated code
:: -std=c99  : Use C99 language standard
:: -Wall     : Enable all compilation Warnings

g++ -pthread -o %NAMEPART%.exe %NAMEPART%.o %GTEST_LIB% %GTEST_LIB_MAIN% -lraylib -lopengl32 -lgdi32 -lwinmm -std=c99 -Wall 
:: .
:: > Executing program
:: -------------------------
cmd /c if exist %NAMEPART%.exe %NAMEPART%.exe




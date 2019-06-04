::@echo off
:: > Choose compile options
:: -------------------------------
:: Set desired OpenGL API version: 1.1, 2.1, 3.3
set GRAPHICS_API=GRAPHICS_API_OPENGL_33
:: .
:: > Setup required Environment
:: -------------------------------------
set RAYLIB_DIR=C:\raylib
set PATH=%PATH%;%RAYLIB_DIR%\mingw\bin
cd %RAYLIB_DIR%\raylib\src
:: .
:: > Cleaning latest build
:: ---------------------------
cmd /c del /F *.o
cmd /c del /F libraylib.a
:: .
:: > Compile raylib modules
:: ------------------------------
gcc -O2 -c rglfw.c -Wall -I. -Iexternal/glfw/include
gcc -O2 -c core.c -std=c99 -Wall -Iexternal/glfw/include -DPLATFORM_DESKTOP -D%GRAPHICS_API%
gcc -O2 -c shapes.c -std=c99 -Wall -DPLATFORM_DESKTOP
gcc -O2 -c textures.c -std=c99 -Wall -DPLATFORM_DESKTOP
gcc -O2 -c text.c -std=c99 -Wall -DPLATFORM_DESKTOP
gcc -O2 -c models.c -std=c99 -Wall -DPLATFORM_DESKTOP
gcc -O2 -c raudio.c -std=c99 -Wall -DPLATFORM_DESKTOP
gcc -O2 -c utils.c -std=c99 -Wall -DPLATFORM_DESKTOP

:: .
:: . > Generate raylib library
:: ------------------------------
ar rcs libraylib.a core.o rglfw.o shapes.o textures.o text.o models.o raudio.o utils.o
:: .
:: > Installing raylib library
:: -----------------------------
cmd /c copy raylib.h %RAYLIB_DIR%\mingw\i686-w64-mingw32\include /Y
cmd /c copy libraylib.a %RAYLIB_DIR%\mingw\i686-w64-mingw32\lib /Y
:: .
:: > Restore environment
:: -----------------------------
cd %~dp0
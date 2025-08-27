::@echo off
:: .
:: Compile your examples for web using:  build_example_web.bat <example_category>/<example_name>
:: .
SET "INPUT_FILE=%1"
:: Change delimiter for the FOR loop
FOR /f "tokens=1-10 delims=/" %%a IN ("%INPUT_FILE%") DO (
    SET CATEGORY=%%a
    SET FILENAME=%%b
)
:: > SETup required Environment
:: -------------------------------------
SET RAYLIB_PATH=C:\GitHub\raylib
SET EMSDK_PATH=C:\raylib\emsdk
SET COMPILER_PATH=C:\raylib\w64devkit\bin
ENV_SET PATH=%COMPILER_PATH%
SET MAKE=mingw32-make
echo
:: Set required web compilation options
:: -------------------------------------
::SET CC=%EMSDK_PATH%\upstream\emscripten\emcc
::SET CFLAGS=-Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -O3 -I. -Iexternal -I%RAYLIB_PATH%\src -I%RAYLIB_PATH%\external -DPLATFORM_WEB
::SET LDFLAGS=-L. -L$(RAYLIB_PATH)\src -sUSE_GLFW=3 -sEXPORTED_RUNTIME_METHODS=ccall -sASYNCIFY --shell-file %RAYLIB_PATH%\src\shell.html
::SET LDLIBS=%RAYLIB_PATH%\src\libraylib.web.a
echo
:: Clean latest build
:: ------------------------
cmd /c if exist %FILENAME%.html del /F %FILENAME%.html
cmd /c if exist %FILENAME%.wasm del /F %FILENAME%.wasm
cmd /c if exist %FILENAME%.js del /F %FILENAME%.js
cmd /c if exist %FILENAME%.data del /F %FILENAME%.data
echo
:: Setup emsdk environment
:: --------------------------
call %EMSDK_PATH%\emsdk_env.bat
echo on
:: Compile program
:: -----------------------
C:
cd %RAYLIB_PATH%\examples
%MAKE% -f Makefile.Web %CATEGORY%/%FILENAME% PLATFORM=PLATFORM_WEB -B
::%CC% -o %FILENAME%.html %FILENAME%.c %CFLAGS% %LDFLAGS% %LDLIBS% %RESOURCES%
cd ..
echo

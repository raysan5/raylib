::@echo off
:: .
:: Compile your examples for web using:  build_example_web.bat <example_category>/<example_name>
:: .
set "INPUT_FILE=%1"
:: Change delimiter for the FOR loop
for /f "tokens=1-10 delims=/" %%a in ("%INPUT_FILE%") do (
    set CATEGORY=%%a
    set FILENAME=%%b
)
:: > Setup required Environment
:: -------------------------------------
set RAYLIB_PATH=C:\GitHub\raylib
set EMSDK_PATH=C:\raylib\emsdk
echo
:: Set required web compilation options
:: -------------------------------------
set CC=%EMSDK_PATH%\upstream\emscripten\emcc
set CFLAGS=-Wall -std=c99 -D_DEFAULT_SOURCE -Wno-missing-braces -Wunused-result -O3 -I. -Iexternal -I%RAYLIB_PATH%\src -I%RAYLIB_PATH%\external -DPLATFORM_WEB
set LDFLAGS=-L. -L$(RAYLIB_PATH)\src -sUSE_GLFW=3 -sEXPORTED_RUNTIME_METHODS=ccall -sASYNCIFY --shell-file %RAYLIB_PATH%\src\shell.html
set LDLIBS=%RAYLIB_PATH%\src\libraylib.web.a
:: TODO: If using some resources/assets, set the directory path as shown in the commented line!
set RESOURCES=
::set RESOURCES=--preload-file $(RESOURCES_PATH)
cd %RAYLIB_PATH%\examples\%CATEGORY%
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
%CC% -o %FILENAME%.html %FILENAME%.c %CFLAGS% %LDFLAGS% %LDLIBS% %RESOURCES%
cd ..
echo

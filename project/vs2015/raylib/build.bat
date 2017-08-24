@echo off
set Libraries=glfw3.lib raylib.lib user32.lib gdi32.lib shell32.lib
set CompilerFlags= /MD /Z7 /FC /nologo 
set LinkerFlags=-subsystem:Console
set bits=x86
set LibraryLocation=..\deps\lib\%bits%\

for %%* in (.) do set CurrDirName=%%~nx*
mkdir build > NUL 2> NUL
pushd build
IF NOT DEFINED vcvars_called (
    call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" %bits%
    set vcvars_called=1
)
REM xcopy %LibraryLocation%glfw3.dll > NUL 2> NUL
ctime -begin %CurrDirName%.ctm
cl %CompilerFlags% ..\code\main.cpp /I..\deps\include /link -incremental:no /LIBPATH:%LibraryLocation% %Libraries% %LinkerFlags% -out:main.exe
set LastError=%ERRORLEVEL%
ctime -end %CurrDirName%.ctm
popd
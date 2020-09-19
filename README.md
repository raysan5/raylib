Fork of the wonderful https://github.com/raysan5/raylib to implement native rendering (without X11) using DRM and GBM. Runs on Raspberry Pi 4 and should run on any Linux distribution with appropriate graphic drivers (see https://github.com/raysan5/raylib/issues/1096).

Build raylib using the Makefile in src with the option PLATFORM=PLATFORM_DRM.

If you want to build the examples then use the Makefile in examples again with the option PLATFORM=PLATFORM_DRM.

There may be one caveat for compiling: Because raylib defines a Font structure and X11 does so, too, you need to define EGL_NO_X11, otherwise egl.h tears in the X11 headers and the second Font structure.

There should be an error message or a failing assertion if it doesn't work properly. Otherwise it's considered to be a bug, please file an issue.

Successfully compiled raylib and ran the core_basic_window example on a Raspberry Pi 4 and a current Ubuntu 20.04 in a current virtual machine.

!!! It is necessary to switch from the graphical login screen to a console window to allow raylib to aquire the graphics buffer. !!!

#define MATE_IMPLEMENTATION
#include "mate.h"

i32 main() {
  StartBuild();
  {
    // Asume it will be GLFW and X11, can later get options from `String arg = Argument("target")`
    char *cflags = "-fno-sanitize=undefined -D_GNU_SOURCE -DGL_SILENCE_DEPRECATION=199309L -DPLATFORM_DESKTOP -DPLATFORM_DESKTOP_GLFW -D_GLFW_X11";
    CreateStaticLib((StaticLibOptions){.output = "libraylib", .std = FLAG_STD_C99, .warnings = FLAG_WARNINGS, .flags = cflags});

    // Add the core files
    AddFile("./src/rcore.c");
    AddFile("./src/utils.c");
    AddFile("./src/rglfw.c");

    // Add optional modules (these can be made configurable with command line args)
    AddFile("./src/rshapes.c");
    AddFile("./src/rtextures.c");
    AddFile("./src/rtext.c");
    AddFile("./src/rmodels.c");
    AddFile("./src/raudio.c");

    // Add include directories
    AddIncludePaths("./src/platforms");
    AddIncludePaths("./src/external/glfw/include");

    // Linux-specific libraries
    LinkSystemLibraries("GL", "rt", "dl", "m", "X11", "Xcursor", "Xext", "Xfixes", "Xi", "Xinerama", "Xrandr", "Xrender");

    InstallStaticLib();
  }
  EndBuild();
}

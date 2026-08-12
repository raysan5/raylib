# GLFW

[![Build status](https://github.com/glfw/glfw/actions/workflows/build.yml/badge.svg)](https://github.com/glfw/glfw/actions)
[![Build status](https://ci.appveyor.com/api/projects/status/0kf0ct9831i5l6sp/branch/master?svg=true)](https://ci.appveyor.com/project/elmindreda/glfw)

## Introduction

GLFW is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan
application development.  It provides a simple, platform-independent API for
creating windows, contexts and surfaces, reading input, handling events, etc.

GLFW is written primarily in C99, with parts of macOS support being written in
Objective-C.

GLFW supports Windows, macOS and Linux, and also works on many other Unix-like
systems.  On Linux both Wayland and X11 are supported.

GLFW is licensed under the [zlib/libpng
license](https://www.glfw.org/license.html).

You can [download](https://www.glfw.org/download.html) the latest stable release
as source or Windows and macOS binaries.  There are [release
tags](https://github.com/glfw/glfw/releases) with source and binary archives
attached for every version since 3.0.

The [documentation](https://www.glfw.org/docs/latest/) is available online and is
also included in source and binary archives, except those generated
automatically by Github.  The documentation contains guides, a tutorial and the
API reference.  The [release
notes](https://www.glfw.org/docs/latest/news.html) list the new features,
caveats and deprecations in the latest release.  The [version
history](https://www.glfw.org/changelog.html) lists every user-visible change
for every release.

GLFW exists because of the contributions of [many people](CONTRIBUTORS.md)
around the world, whether by reporting bugs, providing community support, adding
features, reviewing or testing code, debugging, proofreading docs, suggesting
features or fixing bugs.


## System requirements

GLFW supports Windows 7 and later and macOS 10.11 and later.  On GNOME Wayland,
window decorations will be very basic unless the
[libdecor](https://gitlab.freedesktop.org/libdecor/libdecor) package is
installed.  Linux and other Unix-like systems running X11 are supported even
without a desktop environment or modern extensions, although some features
require a clipboard manager or a modern window manager.

See the [compatibility guide](https://www.glfw.org/docs/latest/compat.html)
for more detailed information.


## Compiling GLFW

GLFW supports compilation with Visual C++ (2013 and later), GCC and Clang.  Both
Clang-CL and MinGW-w64 are supported.  Other C99 compilers will likely also
work, but this is not regularly tested.

There are [pre-compiled binaries](https://www.glfw.org/download.html)
available for Windows and macOS.

GLFW itself needs only CMake and the headers and libraries for your operating
system and window system.  No other SDKs are required.

See the [compilation guide](https://www.glfw.org/docs/latest/compile.html) for
more information about compiling GLFW and the exact dependencies required for
each window system.

The examples and test programs depend on a number of tiny libraries.  These are
bundled in the `deps/` directory.  The repository has no submodules.

 - [getopt\_port](https://github.com/kimgr/getopt_port/) for examples
   with command-line options
 - [TinyCThread](https://github.com/tinycthread/tinycthread) for threaded
   examples
 - [glad2](https://github.com/Dav1dde/glad) for loading OpenGL and Vulkan
   functions
 - [linmath.h](https://github.com/datenwolf/linmath.h) for linear algebra in
   examples
 - [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear) for test and example UI
 - [stb\_image\_write](https://github.com/nothings/stb) for writing images to disk

The documentation is generated with [Doxygen](https://doxygen.org/) when the
library is built, provided CMake could find a sufficiently new version of it
during configuration.


## Using GLFW

See the [HTML documentation](https://www.glfw.org/docs/latest/) for a tutorial,
guides and the API reference.


## Contributing to GLFW

See the [contribution
guide](https://github.com/glfw/glfw/blob/master/docs/CONTRIBUTING.md) for
more information.

The `master` branch is the stable integration branch and _should_ always compile
and run on all supported platforms.  Details of a newly added feature,
including the public API, may change until it has been included in a release.

The `latest` branch is equivalent to the [highest numbered](https://semver.org/)
release, although it may not always point to the same commit as the tag for that
release.

The `ci` branch is used to trigger continuous integration jobs for code under
testing and should never be relied on for any purpose.


## Reporting bugs

Bugs are reported to our [issue tracker](https://github.com/glfw/glfw/issues).
Please check the [contribution
guide](https://github.com/glfw/glfw/blob/master/docs/CONTRIBUTING.md) for
information on what to include when reporting a bug.


## Changelog since 3.5

None.

## Changelog since 3.4

 - Added `GLFW_UNLIMITED_MOUSE_BUTTONS` input mode that allows mouse buttons beyond
   the limit of the mouse button tokens to be reported (#2423)
 - Added `glfwGetEGLConfig` function to query the `EGLConfig` of a window (#2045)
 - Added `glfwGetGLXFBConfig` function to query the `GLXFBConfig` of a window (#1925)
 - Updated minimum CMake version to 3.16 (#2541)
 - Removed support for building with original MinGW (#2540)
 - [Win32] Removed support for Windows XP and Vista (#2505)
 - [Win32] Bugfix: Media keys were reported with a scancode of 256 (#1768,#2417,#2625)
 - [Cocoa] Added `QuartzCore` framework as link-time dependency
 - [Cocoa] Removed support for OS X 10.10 Yosemite and earlier (#2506)
 - [Cocoa] Bugfix: Cmd+Period, Ctrl+Tab and Ctrl+Esc key events were not emitted
   (#1362,#2278)
 - [Wayland] Bugfix: The fractional scaling related objects were not destroyed
 - [Wayland] Bugfix: `glfwInit` would segfault on compositor with no seat (#2517)
 - [Wayland] Bugfix: A drag entering a non-GLFW surface could cause a segfault
 - [Wayland] Bugfix: Ignore key repeat events when no window has keyboard focus (#2727)
 - [Wayland] Bugfix: Reset key repeat timer when window destroyed (#2741,#2727)
 - [Wayland] Bugfix: Memory would leak if reading a data offer failed midway
 - [Wayland] Bugfix: Retrieved cursor position would be incorrect when hovering over
                     fallback decorations
 - [Wayland] Bugfix: Fallback decorations would report scroll events
 - [Wayland] Bugfix: Keyboard repeat events halted when any key is released (#2568)
 - [Wayland] Bugfix: Fallback decorations would show menu at wrong position
 - [Wayland] Bugfix: The cursor was not updated when clicking through from
   a modal to a fallback decoration
 - [Wayland] Bugfix: The cursor position was not updated when clicking through
   from a modal to the content area
 - [Wayland] Bugfix: free modules at end of terminate function to resolve
   potential segmentation fault (#2744)
 - [Wayland] Bugfix: Confining or disabling the cursor could segfault on
   compositors without `pointer-constraints-unstable-v1`
 - [Wayland] Bugfix: Key repeat did not function on very old compositors
 - [Wayland] Bugfix: The `libwayland-client` library was not unloaded at termination
 - [Wayland] Bugfix: Scroll events were sent twice on some versions of GNOME (#2494)
 - [Wayland] Bugfix: Two-dimensional scroll input was emitted as separate axes
 - [Wayland] Bugfix: Mouse wheel scroll distance was incorrect on some compositors
 - [Wayland] Bugfix: `glfwSwapBuffers` would halt with nonzero swap interval when window
   was suspended (#1350,#2582,#2640,#2719,#2723,#2800,#2827)
 - [Wayland] Bugfix: `glfwPostEmptyEvent` would leak a callback proxy (#2836)
 - [Wayland] Bugfix: `glfwHideWindow` did not always send its request immediately
 - [Wayland] Bugfix: Some event types were not always processed by `glfwPollEvents` or
   `glfwWait*Events` (#2793,#2795)
 - [Wayland] Bugfix: Scaled framebuffer size was sometimes incorect (#2713,#2810)
 - [X11] Bugfix: Running without a WM could trigger an assert (#2593,#2601,#2631)
 - [X11] Bugfix: Occasional crash when an idle display awakes (#2766) 
 - [X11] Bugfix: Prevent BadWindow when creating small windows with a content scale
   less than 1 (#2754)
 - [X11] Bugfix: Clamp width and height to >= 1 to prevent BadValue error and app exit
 - [X11] Bugfix: Floating windows silently became non-floating when hidden (#2276)
 - [X11] Bugfix: The `libXext` library was not unloaded at termination
 - [Linux] Bugfix: The header for `ioctl` was only implicitly included (#2778)
 - [Null] Added Vulkan 'window' surface creation via `VK_EXT_headless_surface`
 - [Null] Added EGL context creation on Mesa via `EGL_MESA_platform_surfaceless`
 - [EGL] Allowed native access on Wayland with `GLFW_CONTEXT_CREATION_API` set to
   `GLFW_NATIVE_CONTEXT_API` (#2518)


## Contact

On [glfw.org](https://www.glfw.org/) you can find the latest version of GLFW, as
well as news, documentation and other information about the project.

If you have questions related to the use of GLFW, we have a
[forum](https://discourse.glfw.org/).

If you have a bug to report, a patch to submit or a feature you'd like to
request, please file it in the
[issue tracker](https://github.com/glfw/glfw/issues) on GitHub.

Finally, if you're interested in helping out with the development of GLFW or
porting it to your favorite platform, join us on the forum or GitHub.


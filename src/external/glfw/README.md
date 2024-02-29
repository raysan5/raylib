# GLFW

[![Build status](https://github.com/glfw/glfw/actions/workflows/build.yml/badge.svg)](https://github.com/glfw/glfw/actions)
[![Build status](https://ci.appveyor.com/api/projects/status/0kf0ct9831i5l6sp/branch/master?svg=true)](https://ci.appveyor.com/project/elmindreda/glfw)

## Introduction

GLFW is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan
application development.  It provides a simple, platform-independent API for
creating windows, contexts and surfaces, reading input, handling events, etc.

GLFW natively supports Windows, macOS and Linux and other Unix-like systems.  On
Linux both Wayland and X11 are supported.

GLFW is licensed under the [zlib/libpng
license](https://www.glfw.org/license.html).

You can [download](https://www.glfw.org/download.html) the latest stable release
as source or Windows binaries.  Each release starting with 3.0 also has
a corresponding [annotated tag](https://github.com/glfw/glfw/releases) with
source and binary archives.

The [documentation](https://www.glfw.org/docs/latest/) is available online and is
included in all source and binary archives.  See the [release
notes](https://www.glfw.org/docs/latest/news.html) for new features, caveats and
deprecations in the latest release.  For more details see the [version
history](https://www.glfw.org/changelog.html).

The `master` branch is the stable integration branch and _should_ always compile
and run on all supported platforms, although details of newly added features may
change until they have been included in a release.  New features and many bug
fixes live in [other branches](https://github.com/glfw/glfw/branches/all) until
they are stable enough to merge.

If you are new to GLFW, you may find the
[tutorial](https://www.glfw.org/docs/latest/quick.html) for GLFW 3 useful.  If
you have used GLFW 2 in the past, there is a [transition
guide](https://www.glfw.org/docs/latest/moving.html) for moving to the GLFW
3 API.

GLFW exists because of the contributions of [many people](CONTRIBUTORS.md)
around the world, whether by reporting bugs, providing community support, adding
features, reviewing or testing code, debugging, proofreading docs, suggesting
features or fixing bugs.


## Compiling GLFW

GLFW is written primarily in C99, with parts of macOS support being written in
Objective-C.  GLFW itself requires only the headers and libraries for your OS
and window system.  It does not need any additional headers for context creation
APIs (WGL, GLX, EGL, NSGL, OSMesa) or rendering APIs (OpenGL, OpenGL ES, Vulkan)
to enable support for them.

GLFW supports compilation on Windows with Visual C++ 2013 and later, MinGW and
MinGW-w64, on macOS with Clang and on Linux and other Unix-like systems with GCC
and Clang.  It will likely compile in other environments as well, but this is
not regularly tested.

There are [pre-compiled binaries](https://www.glfw.org/download.html) available
for all supported compilers on Windows and macOS.

See the [compilation guide](https://www.glfw.org/docs/latest/compile.html) for
more information about how to compile GLFW yourself.


## Using GLFW

See the [documentation](https://www.glfw.org/docs/latest/) for tutorials, guides
and the API reference.


## Contributing to GLFW

See the [contribution
guide](https://github.com/glfw/glfw/blob/master/docs/CONTRIBUTING.md) for
more information.


## System requirements

GLFW supports Windows XP and later and macOS 10.8 and later.  Linux and other
Unix-like systems running the X Window System are supported even without
a desktop environment or modern extensions, although some features require
a running window or clipboard manager.  The OSMesa backend requires Mesa 6.3.

See the [compatibility guide](https://www.glfw.org/docs/latest/compat.html)
in the documentation for more information.


## Dependencies

GLFW itself needs only CMake 3.1 or later and the headers and libraries for your
OS and window system.

The examples and test programs depend on a number of tiny libraries.  These are
located in the `deps/` directory.

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

The documentation is generated with [Doxygen](https://doxygen.org/) if CMake can
find that tool.


## Reporting bugs

Bugs are reported to our [issue tracker](https://github.com/glfw/glfw/issues).
Please check the [contribution
guide](https://github.com/glfw/glfw/blob/master/docs/CONTRIBUTING.md) for
information on what to include when reporting a bug.


## Changelog since 3.3.10

 - Added `GLFW_PLATFORM` init hint for runtime platform selection (#1958)
 - Added `GLFW_ANY_PLATFORM`, `GLFW_PLATFORM_WIN32`, `GLFW_PLATFORM_COCOA`,
   `GLFW_PLATFORM_WAYLAND`, `GLFW_PLATFORM_X11` and `GLFW_PLATFORM_NULL` symbols to
   specify the desired platform (#1958)
 - Added `glfwGetPlatform` function to query what platform was selected (#1655,#1958)
 - Added `glfwPlatformSupported` function to query if a platform is supported
   (#1655,#1958)
 - Added `glfwInitAllocator` for setting a custom memory allocator (#544,#1628,#1947)
 - Added `GLFWallocator` struct and `GLFWallocatefun`, `GLFWreallocatefun` and
   `GLFWdeallocatefun` types (#544,#1628,#1947)
 - Added `glfwGetWindowTitle` function for querying window title (#1448,#1909,#2482)
 - Added `glfwInitVulkanLoader` for using a non-default Vulkan loader (#1374,#1890)
 - Added `GLFW_RESIZE_NWSE_CURSOR`, `GLFW_RESIZE_NESW_CURSOR`,
   `GLFW_RESIZE_ALL_CURSOR` and `GLFW_NOT_ALLOWED_CURSOR` cursor shapes (#427)
 - Added `GLFW_RESIZE_EW_CURSOR` alias for `GLFW_HRESIZE_CURSOR` (#427)
 - Added `GLFW_RESIZE_NS_CURSOR` alias for `GLFW_VRESIZE_CURSOR` (#427)
 - Added `GLFW_POINTING_HAND_CURSOR` alias for `GLFW_HAND_CURSOR` (#427)
 - Added `GLFW_MOUSE_PASSTHROUGH` window hint for letting mouse input pass
   through the window (#1236,#1568)
 - Added `GLFW_CURSOR_CAPTURED` cursor mode to confine the cursor to the window
   content area (#58)
 - Added `GLFW_POSITION_X` and `GLFW_POSITION_Y` window hints for initial position
   (#1603,#1747)
 - Added `GLFW_SCALE_FRAMEBUFFER` window hint for Wayland and macOS scaling (#2457)
 - Added `GLFW_ANY_POSITION` hint value for letting the window manager choose (#1603,#1747)
 - Added `GLFW_PLATFORM_UNAVAILABLE` error for platform detection failures (#1958)
 - Added `GLFW_FEATURE_UNAVAILABLE` error for platform limitations (#1692)
 - Added `GLFW_FEATURE_UNIMPLEMENTED` error for incomplete backends (#1692)
 - Added `GLFW_WAYLAND_APP_ID` window hint string for Wayland app\_id selection
   (#2121,#2122)
 - Added `GLFW_ANGLE_PLATFORM_TYPE` init hint and `GLFW_ANGLE_PLATFORM_TYPE_*`
   values to select ANGLE backend (#1380)
 - Added `GLFW_X11_XCB_VULKAN_SURFACE` init hint for selecting X11 Vulkan
   surface extension (#1793)
 - Added `GLFW_WIN32_KEYBOARD_MENU` window hint for enabling access to the window menu
 - Added `GLFW_WIN32_SHOWDEFAULT` window hint for applying the parent process
   show command (#2359)
 - Added `GLFW_NATIVE_INCLUDE_NONE` for disabling inclusion of native headers (#1348)
 - Added `GLFW_BUILD_WIN32` CMake option for enabling Win32 support (#1958)
 - Added `GLFW_BUILD_COCOA` CMake option for enabling Cocoa support (#1958)
 - Added `GLFW_BUILD_X11` CMake option for enabling X11 support (#1958)
 - Added `GLFW_LIBRARY_TYPE` CMake variable for overriding the library type
   (#279,#1307,#1497,#1574,#1928)
 - Added support for `XDG_SESSION_TYPE` environment variable
 - Added `GLFW_PKG_CONFIG_REQUIRES_PRIVATE` and `GLFW_PKG_CONFIG_LIBS_PRIVATE` CMake
   variables exposing pkg-config dependencies (#1307)
 - Made joystick subsystem initialize at first use (#1284,#1646)
 - Made `GLFW_DOUBLEBUFFER` a read-only window attribute
 - Made Wayland the preferred platform over X11 if both are available (#2035)
 - Updated the minimum required CMake version to 3.4
 - Updated gamepad mappings from upstream
 - Renamed `GLFW_USE_WAYLAND` CMake option to `GLFW_BUILD_WAYLAND` (#1958)
 - Disabled tests and examples by default when built as a CMake subdirectory
 - Removed `GLFW_USE_OSMESA` CMake option enabling the Null platform (#1958)
 - Removed CMake generated configuration header
 - Bugfix: `glfwGetVideoMode` returned an invalid mode on error (#1292)
 - [Win32] Added a version info resource to the GLFW DLL
 - [Win32] Made hidden helper window use its own window class
 - [Win32] Bugfix: The foreground lock timeout was overridden, ignoring the user
 - [Cocoa] Added `glfwGetCocoaView` native access function (#2235)
 - [Cocoa] Moved main menu creation to GLFW initialization time (#1649)
 - [Cocoa] Bugfix: Touching event queue from secondary thread before main thread
   would abort (#1649)
 - [Wayland] Added support for `glfwRequestWindowAttention` (#2287)
 - [Wayland] Added support for `glfwFocusWindow`
 - [Wayland] Added support for `GLFW_RESIZABLE` (#2203)
 - [Wayland] Added support for fractional scaling of window contents
 - [Wayland] Added dynamic loading of all Wayland libraries
 - [Wayland] Bugfix: `CLOCK_MONOTONIC` was not correctly enabled
 - [Wayland] Bugfix: `GLFW_HOVERED` was true when the cursor was over any
   fallback window decoration
 - [Wayland] Bugfix: Fallback decorations allowed resizing to invalid size
   (#2204)
 - [X11] Bugfix: Termination would segfault if the IM had been destroyed
 - [X11] Bugfix: Any IM started after initialization would not be detected
 - [Linux] Bugfix: Joystick evdev fds remained open in forks (#2446)
 - [POSIX] Removed use of deprecated function `gettimeofday`
 - [POSIX] Bugfix: `CLOCK_MONOTONIC` was not correctly tested for or enabled
 - [WGL] Disabled the DWM swap interval hack for Windows 8 and later (#1072)
 - [NSGL] Removed enforcement of forward-compatible flag for core contexts
 - [NSGL] Bugfix: A core profile OpenGL context was returned if 3.2+
   compatibility profile was requested
 - [EGL] Added platform selection via the `EGL_EXT_platform_base` extension
   (#442)
 - [EGL] Added ANGLE backend selection via `EGL_ANGLE_platform_angle` extension
   (#1380)


## Changelog since 3.3

 - Added `GLFW_WAYLAND_LIBDECOR` init hint for disabling libdecor support (#1639,#1693)
 - Bugfix: The CMake config-file package used an absolute path and was not
   relocatable (#1470)
 - Bugfix: Video modes with a duplicate screen area were discarded (#1555,#1556)
 - Bugfix: Compiling with -Wextra-semi caused warnings (#1440)
 - Bugfix: Built-in mappings failed because some OEMs re-used VID/PID (#1583)
 - Bugfix: Some extension loader headers did not prevent default OpenGL header
   inclusion (#1695)
 - Bugfix: Buffers were swapped at creation on single-buffered windows (#1873)
 - Bugfix: Gamepad mapping updates could spam `GLFW_INVALID_VALUE` due to
   incompatible controllers sharing hardware ID (#1763)
 - Bugfix: Native access functions for context handles did not check that the API matched
 - Bugfix: `glfwMakeContextCurrent` would access TLS slot before initialization
 - Bugfix: `glfwSetGammaRamp` could emit `GLFW_INVALID_VALUE` before initialization
 - Bugfix: `glfwGetJoystickUserPointer` returned `NULL` during disconnection (#2092)
 - Bugfix: `glfwGetKeyScancode` returned `0` on error when initialized instead of `-1`
 - Bugfix: Failure to make a newly created context current could cause segfault (#2327)
 - [Win32] Disabled framebuffer transparency on Windows 7 when DWM windows are
   opaque (#1512)
 - [Win32] Bugfix: `GLFW_INCLUDE_VULKAN` plus `VK_USE_PLATFORM_WIN32_KHR` caused
   symbol redefinition (#1524)
 - [Win32] Bugfix: The cursor position event was emitted before its cursor enter
   event (#1490)
 - [Win32] Bugfix: The window hint `GLFW_MAXIMIZED` did not move or resize the
   window (#1499)
 - [Win32] Bugfix: Disabled cursor mode interfered with some non-client actions
 - [Win32] Bugfix: Super key was not released after Win+V hotkey (#1622)
 - [Win32] Bugfix: `glfwGetKeyName` could access out of bounds and return an
   invalid pointer
 - [Win32] Bugfix: Some synthetic key events were reported as `GLFW_KEY_UNKNOWN`
   (#1623)
 - [Win32] Bugfix: Non-BMP Unicode codepoint input was reported as UTF-16
 - [Win32] Bugfix: Monitor functions could return invalid values after
   configuration change (#1761)
 - [Win32] Bugfix: Initialization would segfault on Windows 8 (not 8.1) (#1775)
 - [Win32] Bugfix: Duplicate size events were not filtered (#1610)
 - [Win32] Bugfix: Full screen windows were incorrectly resized by DPI changes
   (#1582)
 - [Win32] Bugfix: `GLFW_SCALE_TO_MONITOR` had no effect on systems older than
   Windows 10 version 1703 (#1511)
 - [Win32] Bugfix: `USE_MSVC_RUNTIME_LIBRARY_DLL` had no effect on CMake 3.15 or
   later (#1783,#1796)
 - [Win32] Bugfix: Compilation with LLVM for Windows failed (#1807,#1824,#1874)
 - [Win32] Bugfix: Content scale queries could fail silently (#1615)
 - [Win32] Bugfix: Content scales could have garbage values if monitor was recently
   disconnected (#1615)
 - [Win32] Bugfix: A window created maximized and undecorated would cover the whole
   monitor (#1806)
 - [Win32] Bugfix: The default restored window position was lost when creating a maximized
   window
 - [Win32] Bugfix: `glfwMaximizeWindow` would make a hidden window visible
 - [Win32] Bugfix: `Alt+PrtSc` would emit `GLFW_KEY_UNKNOWN` and a different
   scancode than `PrtSc` (#1993)
 - [Win32] Bugfix: `GLFW_KEY_PAUSE` scancode from `glfwGetKeyScancode` did not
   match event scancode (#1993)
 - [Win32] Bugfix: Instance-local operations used executable instance (#469,#1296,#1395)
 - [Win32] Bugfix: The OSMesa library was not unloaded on termination
 - [Win32] Bugfix: Right shift emitted `GLFW_KEY_UNKNOWN` when using a CJK IME (#2050)
 - [Win32] Bugfix: `glfwWaitEventsTimeout` did not return for some sent messages (#2408)
 - [Win32] Bugfix: Fix pkg-config for dynamic library on Windows (#2386, #2420)
 - [Win32] Bugfix: XInput could reportedly provide invalid DPad bit masks (#2291)
 - [Win32] Bugfix: Rapid clipboard calls could fail due to Clipboard History
 - [Win32] Bugfix: Disabled cursor mode doesn't work right when connected over RDP (#1276,#1279,#2431)
 - [Cocoa] Added support for `VK_EXT_metal_surface` (#1619)
 - [Cocoa] Added locating the Vulkan loader at runtime in an application bundle
 - [Cocoa] Changed `EGLNativeWindowType` from `NSView` to `CALayer` (#1169)
 - [Cocoa] Changed F13 key to report Print Screen for cross-platform consistency
   (#1786)
 - [Cocoa] Disabled macOS fullscreen when `GLFW_RESIZABLE` is false
 - [Cocoa] Removed dependency on the CoreVideo framework
 - [Cocoa] Bugfix: `glfwSetWindowSize` used a bottom-left anchor point (#1553)
 - [Cocoa] Bugfix: Window remained on screen after destruction until event poll
   (#1412)
 - [Cocoa] Bugfix: Event processing before window creation would assert (#1543)
 - [Cocoa] Bugfix: Undecorated windows could not be iconified on recent macOS
 - [Cocoa] Bugfix: Non-BMP Unicode codepoint input was reported as UTF-16
   (#1635)
 - [Cocoa] Bugfix: Failing to retrieve the refresh rate of built-in displays
   could leak memory
 - [Cocoa] Bugfix: Objective-C files were compiled as C with CMake 3.19 (#1787)
 - [Cocoa] Bugfix: Duplicate video modes were not filtered out (#1830)
 - [Cocoa] Bugfix: Menu bar was not clickable on macOS 10.15+ until it lost and
   regained focus (#1648,#1802)
 - [Cocoa] Bugfix: Monitor name query could segfault on macOS 11 (#1809,#1833)
 - [Cocoa] Bugfix: The install name of the installed dylib was relative (#1504)
 - [Cocoa] Bugfix: The MoltenVK layer contents scale was updated only after
   related events were emitted
 - [Cocoa] Bugfix: Moving the cursor programmatically would freeze it for
   a fraction of a second (#1962)
 - [Cocoa] Bugfix: `kIOMasterPortDefault` was deprecated in macOS 12.0 (#1980)
 - [Cocoa] Bugfix: `kUTTypeURL` was deprecated in macOS 12.0 (#2003)
 - [Cocoa] Bugfix: A connected Apple AirPlay would emit a useless error (#1791)
 - [Cocoa] Bugfix: The EGL and OSMesa libraries were not unloaded on termination
 - [Cocoa] Bugfix: `GLFW_MAXIMIZED` was always true when `GLFW_RESIZABLE` was false
 - [Cocoa] Bugfix: Changing `GLFW_DECORATED` in macOS fullscreen would abort
   application (#1886)
 - [Cocoa] Bugfix: Setting a monitor from macOS fullscreen would abort
   application (#2110)
 - [Cocoa] Bugfix: The Vulkan loader was not loaded from the `Frameworks` bundle
   subdirectory (#2113,#2120)
 - [Cocoa] Bugfix: Compilation failed on OS X 10.8 due to unconditional use of 10.9+
   symbols (#2161)
 - [Cocoa] Bugfix: Querying joystick elements could reportedly segfault on macOS
   13 Ventura (#2320)
 - [X11] Bugfix: The CMake files did not check for the XInput headers (#1480)
 - [X11] Bugfix: Key names were not updated when the keyboard layout changed
   (#1462,#1528)
 - [X11] Bugfix: Decorations could not be enabled after window creation (#1566)
 - [X11] Bugfix: Content scale fallback value could be inconsistent (#1578)
 - [X11] Bugfix: `glfwMaximizeWindow` had no effect on hidden windows
 - [X11] Bugfix: Clearing `GLFW_FLOATING` on a hidden window caused invalid read
 - [X11] Bugfix: Changing `GLFW_FLOATING` on a hidden window could silently fail
 - [X11] Bugfix: Disabled cursor mode was interrupted by indicator windows
 - [X11] Bugfix: Monitor physical dimensions could be reported as zero mm
 - [X11] Bugfix: Window position events were not emitted during resizing (#1613)
 - [X11] Bugfix: `glfwFocusWindow` could terminate on older WMs or without a WM
 - [X11] Bugfix: Querying a disconnected monitor could segfault (#1602)
 - [X11] Bugfix: IME input of CJK was broken for "C" locale (#1587,#1636)
 - [X11] Bugfix: Xlib errors caused by other parts of the application could be
   reported as GLFW errors
 - [X11] Bugfix: A handle race condition could cause a `BadWindow` error (#1633)
 - [X11] Bugfix: XKB path used keysyms instead of physical locations for
   non-printable keys (#1598)
 - [X11] Bugfix: Function keys were mapped to `GLFW_KEY_UNKNOWN` for some layout
   combinations (#1598)
 - [X11] Bugfix: Keys pressed simultaneously with others were not always
   reported (#1112,#1415,#1472,#1616)
 - [X11] Bugfix: Some window attributes were not applied on leaving fullscreen
   (#1863)
 - [X11] Bugfix: Changing `GLFW_FLOATING` could leak memory
 - [X11] Bugfix: Icon pixel format conversion worked only by accident, relying on
   undefined behavior (#1986)
 - [X11] Bugfix: Dynamic loading on OpenBSD failed due to soname differences
 - [X11] Bugfix: Waiting for events would fail if file descriptor was too large
   (#2024)
 - [X11] Bugfix: Joystick events could lead to busy-waiting (#1872)
 - [X11] Bugfix: `glfwWaitEvents*` did not continue for joystick events
 - [X11] Bugfix: `glfwPostEmptyEvent` could be ignored due to race condition
   (#379,#1281,#1285,#2033)
 - [X11] Bugfix: Dynamic loading on NetBSD failed due to soname differences
 - [X11] Bugfix: Left shift of int constant relied on undefined behavior (#1951)
 - [X11] Bugfix: The OSMesa libray was not unloaded on termination
 - [X11] Bugfix: A malformed response during selection transfer could cause a segfault
 - [X11] Bugfix: Some calls would reset Xlib to the default error handler (#2108)
 - [Wayland] Added improved fallback window decorations via libdecor (#1639,#1693)
 - [Wayland] Added support for key names via xkbcommon
 - [Wayland] Added support for file path drop events (#2040)
 - [Wayland] Added support for more human-readable monitor names where available
 - [Wayland] Disabled alpha channel for opaque windows on systems lacking
   `EGL_EXT_present_opaque` (#1895)
 - [Wayland] Removed support for `wl_shell` (#1443)
 - [Wayland] Bugfix: The `GLFW_HAND_CURSOR` shape used the wrong image (#1432)
 - [Wayland] Bugfix: Repeated keys could be reported with `NULL` window (#1704)
 - [Wayland] Bugfix: Retrieving partial framebuffer size would segfault
 - [Wayland] Bugfix: Scrolling offsets were inverted compared to other platforms
   (#1463)
 - [Wayland] Bugfix: Client-Side Decorations were destroyed in the wrong order
   (#1798)
 - [Wayland] Bugfix: Monitors physical size could report zero (#1784,#1792)
 - [Wayland] Bugfix: Some keys were not repeating in Wayland (#1908)
 - [Wayland] Bugfix: Non-arrow cursors are offset from the hotspot (#1706,#1899)
 - [Wayland] Bugfix: The `O_CLOEXEC` flag was not defined on FreeBSD
 - [Wayland] Bugfix: Key repeat could lead to a race condition (#1710)
 - [Wayland] Bugfix: Activating a window would emit two input focus events
 - [Wayland] Bugfix: Disable key repeat mechanism when window loses input focus
 - [Wayland] Bugfix: Window hiding and showing did not work (#1492,#1731)
 - [Wayland] Bugfix: A key being repeated was not released when window lost focus
 - [Wayland] Bugfix: Showing a hidden window did not emit a window refresh event
 - [Wayland] Bugfix: Full screen window creation did not ignore `GLFW_VISIBLE`
 - [Wayland] Bugfix: Some keys were reported as wrong key or `GLFW_KEY_UNKNOWN`
 - [Wayland] Bugfix: Text input did not repeat along with key repeat
 - [Wayland] Bugfix: `glfwPostEmptyEvent` sometimes had no effect (#1520,#1521)
 - [Wayland] Bugfix: `glfwSetClipboardString` would fail if set to result of
   `glfwGetClipboardString`
 - [Wayland] Bugfix: Data source creation error would cause double free at termination
 - [Wayland] Bugfix: Partial writes of clipboard string would cause beginning to repeat
 - [Wayland] Bugfix: Some errors would cause clipboard string transfer to hang
 - [Wayland] Bugfix: Drag and drop data was misinterpreted as clipboard string
 - [Wayland] Bugfix: MIME type matching was not performed for clipboard string
 - [Wayland] Bugfix: The OSMesa library was not unloaded on termination
 - [Wayland] Bugfix: `glfwCreateWindow` could emit `GLFW_FEATURE_UNAVAILABLE`
 - [Wayland] Bugfix: Lock key modifier bits were only set when lock keys were pressed
 - [Wayland] Bugfix: A window leaving full screen mode would be iconified (#1995)
 - [Wayland] Bugfix: A window leaving full screen mode ignored its desired size
 - [Wayland] Bugfix: `glfwSetWindowMonitor` did not update windowed mode size
 - [Wayland] Bugfix: `glfwRestoreWindow` would make a full screen window windowed
 - [Wayland] Bugfix: A window maximized or restored by the user would enter an
   inconsistent state
 - [Wayland] Bugfix: Window maximization events were not emitted
 - [Wayland] Bugfix: `glfwRestoreWindow` assumed it was always in windowed mode
 - [Wayland] Bugfix: `glfwSetWindowSize` would resize a full screen window
 - [Wayland] Bugfix: A window content scale event would be emitted every time
   the window resized
 - [Wayland] Bugfix: If `glfwInit` failed it would close stdin
 - [Wayland] Bugfix: Manual resizing with fallback decorations behaved erratically
   (#1991,#2115,#2127)
 - [Wayland] Bugfix: Size limits included frame size for fallback decorations
 - [Wayland] Bugfix: Updating `GLFW_DECORATED` had no effect on server-side
   decorations
 - [Wayland] Bugfix: A monitor would be reported as connected again if its scale
   changed
 - [Wayland] Bugfix: `glfwTerminate` would segfault if any monitor had changed
   scale
 - [Wayland] Bugfix: Window content scale events were not emitted when monitor
   scale changed
 - [Wayland] Bugfix: `glfwSetWindowAspectRatio` reported an error instead of
   applying the specified ratio
 - [Wayland] Bugfix: `GLFW_MAXIMIZED` window hint had no effect
 - [Wayland] Bugfix: `glfwRestoreWindow` had no effect before first show
 - [Wayland] Bugfix: Hiding and then showing a window caused program abort on
   wlroots compositors (#1268)
 - [Wayland] Bugfix: `GLFW_DECORATED` was ignored when showing a window with XDG
   decorations
 - [Wayland] Bugfix: Connecting a mouse after `glfwInit` would segfault (#1450)
 - [Wayland] Bugfix: Joysticks connected after `glfwInit` were not detected (#2198)
 - [Wayland] Bugfix: Fallback decorations emitted `GLFW_CURSOR_UNAVAILABLE` errors
 - [Linux] Bugfix: Joysticks without buttons were ignored (#2042,#2043)
 - [Linux] Bugfix: A small amount of memory could leak if initialization failed (#2229)
 - [NSGL] Bugfix: `GLFW_COCOA_RETINA_FRAMEBUFFER` had no effect on newer
   macOS versions (#1442)
 - [NSGL] Bugfix: Workaround for swap interval on 10.14 broke on 10.12 (#1483)
 - [NSGL] Bugfix: Defining `GL_SILENCE_DEPRECATION` externally caused
   a duplicate definition warning (#1840)
 - [EGL] Added loading of glvnd `libOpenGL.so.0` where available for OpenGL
 - [EGL] Bugfix: The `GLFW_DOUBLEBUFFER` context attribute was ignored (#1843)
 - [EGL] Bugfix: Setting `GLFW_CONTEXT_DEBUG` caused creation to fail (#2348)
 - [GLX] Added loading of glvnd `libGLX.so.0` where available
 - [GLX] Bugfix: Context creation failed if GLX 1.4 was not exported by GLX library


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


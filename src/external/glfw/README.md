# GLFW

[![Build status](https://travis-ci.org/glfw/glfw.svg?branch=master)](https://travis-ci.org/glfw/glfw)
[![Build status](https://ci.appveyor.com/api/projects/status/0kf0ct9831i5l6sp/branch/master?svg=true)](https://ci.appveyor.com/project/elmindreda/glfw)
[![Coverity Scan](https://scan.coverity.com/projects/4884/badge.svg)](https://scan.coverity.com/projects/glfw-glfw)

## Introduction

GLFW is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan
application development.  It provides a simple, platform-independent API for
creating windows, contexts and surfaces, reading input, handling events, etc.

GLFW natively supports Windows, macOS and Linux and other Unix-like systems.  On
Linux both X11 and Wayland are supported.

GLFW is licensed under the [zlib/libpng
license](http://www.glfw.org/license.html).

You can [download](http://www.glfw.org/download.html) the latest stable release
as source or Windows binaries, or fetch the `latest` branch from GitHub.  Each
release starting with 3.0 also has a corresponding [annotated
tag](https://github.com/glfw/glfw/releases) with source and binary archives.

The [documentation](http://www.glfw.org/docs/latest/) is available online and is
included in all source and binary archives.  See the [release
notes](https://www.glfw.org/docs/latest/news.html) for new features, caveats and
deprecations in the latest release.  For more details see the [version
history](http://www.glfw.org/changelog.html).

The `master` branch is the stable integration branch and _should_ always compile
and run on all supported platforms, although details of newly added features may
change until they have been included in a release.  New features and many bug
fixes live in [other branches](https://github.com/glfw/glfw/branches/all) until
they are stable enough to merge.

If you are new to GLFW, you may find the
[tutorial](http://www.glfw.org/docs/latest/quick.html) for GLFW 3 useful.  If
you have used GLFW 2 in the past, there is a [transition
guide](http://www.glfw.org/docs/latest/moving.html) for moving to the GLFW
3 API.


## Compiling GLFW

GLFW itself requires only the headers and libraries for your OS and window
system.  It does not need the headers for any context creation API (WGL, GLX,
EGL, NSGL, OSMesa) or rendering API (OpenGL, OpenGL ES, Vulkan) to enable
support for them.

GLFW supports compilation on Windows with Visual C++ 2010 and later, MinGW and
MinGW-w64, on macOS with Clang and on Linux and other Unix-like systems with GCC
and Clang.  It will likely compile in other environments as well, but this is
not regularly tested.

There are [pre-compiled Windows binaries](http://www.glfw.org/download.html)
available for all supported compilers.

See the [compilation guide](http://www.glfw.org/docs/latest/compile.html) for
more information about how to compile GLFW yourself.


## Using GLFW

See the [documentation](http://www.glfw.org/docs/latest/) for tutorials, guides
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

See the [compatibility guide](http://www.glfw.org/docs/latest/compat.html)
in the documentation for more information.


## Dependencies

GLFW itself needs only CMake 3.1 or later and the headers and libraries for your
OS and window system.

The (experimental) Wayland backend also depends on the `extra-cmake-modules`
package, which is used to generate Wayland protocol headers.

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
 - [Nuklear](https://github.com/vurtun/nuklear) for test and example UI
 - [stb\_image\_write](https://github.com/nothings/stb) for writing images to disk

The documentation is generated with [Doxygen](http://doxygen.org/) if CMake can
find that tool.


## Reporting bugs

Bugs are reported to our [issue tracker](https://github.com/glfw/glfw/issues).
Please check the [contribution
guide](https://github.com/glfw/glfw/blob/master/docs/CONTRIBUTING.md) for
information on what to include when reporting a bug.


## Changelog

 - Added `GLFW_RESIZE_NWSE_CURSOR`, `GLFW_RESIZE_NESW_CURSOR`,
   `GLFW_RESIZE_ALL_CURSOR` and `GLFW_NOT_ALLOWED_CURSOR` cursor shapes (#427)
 - Added `GLFW_RESIZE_EW_CURSOR` alias for `GLFW_HRESIZE_CURSOR` (#427)
 - Added `GLFW_RESIZE_NS_CURSOR` alias for `GLFW_VRESIZE_CURSOR` (#427)
 - Added `GLFW_POINTING_HAND_CURSOR` alias for `GLFW_HAND_CURSOR` (#427)
 - Updated the minimum required CMake version to 3.1
 - Disabled tests and examples by default when built as a CMake subdirectory
 - Bugfix: The CMake config-file package used an absolute path and was not
   relocatable (#1470)
 - Bugfix: Video modes with a duplicate screen area were discarded (#1555,#1556)
 - Bugfix: Compiling with -Wextra-semi caused warnings (#1440)
 - Bugfix: Built-in mappings failed because some OEMs re-used VID/PID (#1583)
 - [Win32] Added the `GLFW_WIN32_KEYBOARD_MENU` window hint for enabling access
           to the window menu
 - [Win32] Added a version info resource to the GLFW DLL
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
 - [Cocoa] Added support for `VK_EXT_metal_surface` (#1619)
 - [Cocoa] Added locating the Vulkan loader at runtime in an application bundle
 - [Cocoa] Removed dependency on the CoreVideo framework
 - [Cocoa] Bugfix: `glfwSetWindowSize` used a bottom-left anchor point (#1553)
 - [Cocoa] Bugfix: Window remained on screen after destruction until event poll
   (#1412)
 - [Cocoa] Bugfix: Event processing before window creation would assert (#1543)
 - [Cocoa] Bugfix: Undecorated windows could not be iconified on recent macOS
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
 - [Wayland] Removed support for `wl_shell` (#1443)
 - [Wayland] Bugfix: The `GLFW_HAND_CURSOR` shape used the wrong image (#1432)
 - [Wayland] Bugfix: `CLOCK_MONOTONIC` was not correctly enabled
 - [POSIX] Bugfix: `CLOCK_MONOTONIC` was not correctly tested for or enabled
 - [NSGL] Removed enforcement of forward-compatible flag for core contexts
 - [NSGL] Bugfix: `GLFW_COCOA_RETINA_FRAMEBUFFER` had no effect on newer
   macOS versions (#1442)
 - [NSGL] Bugfix: Workaround for swap interval on 10.14 broke on 10.12 (#1483)


## Contact

On [glfw.org](http://www.glfw.org/) you can find the latest version of GLFW, as
well as news, documentation and other information about the project.

If you have questions related to the use of GLFW, we have a
[forum](https://discourse.glfw.org/), and the `#glfw` IRC channel on
[Freenode](http://freenode.net/).

If you have a bug to report, a patch to submit or a feature you'd like to
request, please file it in the
[issue tracker](https://github.com/glfw/glfw/issues) on GitHub.

Finally, if you're interested in helping out with the development of GLFW or
porting it to your favorite platform, join us on the forum, GitHub or IRC.


## Acknowledgements

GLFW exists because people around the world donated their time and lent their
skills.

 - Bobyshev Alexander
 - Matt Arsenault
 - David Avedissian
 - Keith Bauer
 - John Bartholomew
 - Coşku Baş
 - Niklas Behrens
 - Andrew Belt
 - Niklas Bergström
 - Denis Bernard
 - Doug Binks
 - blanco
 - Kyle Brenneman
 - Rok Breulj
 - Kai Burjack
 - Martin Capitanio
 - David Carlier
 - Arturo Castro
 - Chi-kwan Chan
 - Ian Clarkson
 - Michał Cichoń
 - Lambert Clara
 - Anna Clarke
 - Yaron Cohen-Tal
 - Omar Cornut
 - Andrew Corrigan
 - Bailey Cosier
 - Noel Cower
 - Jason Daly
 - Jarrod Davis
 - Olivier Delannoy
 - Paul R. Deppe
 - Michael Dickens
 - Роман Донченко
 - Mario Dorn
 - Wolfgang Draxinger
 - Jonathan Dummer
 - Ralph Eastwood
 - Fredrik Ehnbom
 - Robin Eklind
 - Siavash Eliasi
 - Felipe Ferreira
 - Michael Fogleman
 - Gerald Franz
 - Mário Freitas
 - GeO4d
 - Marcus Geelnard
 - Charles Giessen
 - Ryan C. Gordon
 - Stephen Gowen
 - Kovid Goyal
 - Eloi Marín Gratacós
 - Stefan Gustavson
 - Jonathan Hale
 - Sylvain Hellegouarch
 - Matthew Henry
 - heromyth
 - Lucas Hinderberger
 - Paul Holden
 - Warren Hu
 - Charles Huber
 - IntellectualKitty
 - Aaron Jacobs
 - Erik S. V. Jansson
 - Toni Jovanoski
 - Arseny Kapoulkine
 - Cem Karan
 - Osman Keskin
 - Josh Kilmer
 - Byunghoon Kim
 - Cameron King
 - Peter Knut
 - Christoph Kubisch
 - Yuri Kunde Schlesner
 - Rokas Kupstys
 - Konstantin Käfer
 - Eric Larson
 - Francis Lecavalier
 - Robin Leffmann
 - Glenn Lewis
 - Shane Liesegang
 - Anders Lindqvist
 - Leon Linhart
 - Eyal Lotem
 - Aaron Loucks
 - Luflosi
 - lukect
 - Tristam MacDonald
 - Hans Mackowiak
 - Дмитри Малышев
 - Zbigniew Mandziejewicz
 - Adam Marcus
 - Célestin Marot
 - Kyle McDonald
 - David Medlock
 - Bryce Mehring
 - Jonathan Mercier
 - Marcel Metz
 - Liam Middlebrook
 - Ave Milia
 - Jonathan Miller
 - Kenneth Miller
 - Bruce Mitchener
 - Jack Moffitt
 - Jeff Molofee
 - Alexander Monakov
 - Pierre Morel
 - Jon Morton
 - Pierre Moulon
 - Martins Mozeiko
 - Julian Møller
 - ndogxj
 - Kristian Nielsen
 - Kamil Nowakowski
 - Denis Ovod
 - Ozzy
 - Andri Pálsson
 - Peoro
 - Braden Pellett
 - Christopher Pelloux
 - Arturo J. Pérez
 - Anthony Pesch
 - Orson Peters
 - Emmanuel Gil Peyrot
 - Cyril Pichard
 - Keith Pitt
 - Stanislav Podgorskiy
 - Konstantin Podsvirov
 - Nathan Poirier
 - Alexandre Pretyman
 - Pablo Prietz
 - przemekmirek
 - pthom
 - Guillaume Racicot
 - Philip Rideout
 - Eddie Ringle
 - Max Risuhin
 - Jorge Rodriguez
 - Ed Ropple
 - Aleksey Rybalkin
 - Riku Salminen
 - Brandon Schaefer
 - Sebastian Schuberth
 - Christian Sdunek
 - Matt Sealey
 - Steve Sexton
 - Arkady Shapkin
 - Yoshiki Shibukawa
 - Dmitri Shuralyov
 - Daniel Skorupski
 - Bradley Smith
 - Cliff Smolinsky
 - Patrick Snape
 - Erlend Sogge Heggen
 - Julian Squires
 - Johannes Stein
 - Pontus Stenetorp
 - Michael Stocker
 - Justin Stoecker
 - Elviss Strazdins
 - Paul Sultana
 - Nathan Sweet
 - TTK-Bandit
 - Jared Tiala
 - Sergey Tikhomirov
 - Arthur Tombs
 - Ioannis Tsakpinis
 - Samuli Tuomola
 - Matthew Turner
 - urraka
 - Elias Vanderstuyft
 - Stef Velzel
 - Jari Vetoniemi
 - Ricardo Vieira
 - Nicholas Vitovitch
 - Simon Voordouw
 - Corentin Wallez
 - Torsten Walluhn
 - Patrick Walton
 - Xo Wang
 - Jay Weisskopf
 - Frank Wille
 - Ryogo Yoshimura
 - Lukas Zanner
 - Andrey Zholos
 - Santi Zupancic
 - Jonas Ådahl
 - Lasse Öörni
 - All the unmentioned and anonymous contributors in the GLFW community, for bug
   reports, patches, feedback, testing and encouragement


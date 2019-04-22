# GLFW

[![Build status](https://travis-ci.org/glfw/glfw.svg?branch=master)](https://travis-ci.org/glfw/glfw)
[![Build status](https://ci.appveyor.com/api/projects/status/0kf0ct9831i5l6sp/branch/master?svg=true)](https://ci.appveyor.com/project/elmindreda/glfw)
[![Coverity Scan](https://scan.coverity.com/projects/4884/badge.svg)](https://scan.coverity.com/projects/glfw-glfw)

## Introduction

GLFW is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan
application development.  It provides a simple, platform-independent API for
creating windows, contexts and surfaces, reading input, handling events, etc.

GLFW natively supports Windows, macOS and Linux and other Unix-like systems.  On
Linux both X11 and Wayland is supported.

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

GLFW itself depends only on the headers and libraries for your window system.

The (experimental) Wayland backend also depends on the `extra-cmake-modules`
package, which is used to generated Wayland protocol headers.

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

User-visible changes since the last release.


## Contact

On [glfw.org](http://www.glfw.org/) you can find the latest version of GLFW, as
well as news, documentation and other information about the project.

If you have questions related to the use of GLFW, we have a
[forum](http://discourse.glfw.org/), and the `#glfw` IRC channel on
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
 - IntellectualKitty
 - Aaron Jacobs
 - Erik S. V. Jansson
 - Toni Jovanoski
 - Arseny Kapoulkine
 - Cem Karan
 - Osman Keskin
 - Josh Kilmer
 - Cameron King
 - Peter Knut
 - Christoph Kubisch
 - Yuri Kunde Schlesner
 - Konstantin Käfer
 - Eric Larson
 - Robin Leffmann
 - Glenn Lewis
 - Shane Liesegang
 - Eyal Lotem
 - Aaron Loucks
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
 - Nathan Poirier
 - Alexandre Pretyman
 - przemekmirek
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
 - Andrey Zholos
 - Santi Zupancic
 - Jonas Ådahl
 - Lasse Öörni
 - All the unmentioned and anonymous contributors in the GLFW community, for bug
   reports, patches, feedback, testing and encouragement


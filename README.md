<img align="left" src="https://github.com/raysan5/raylib/blob/master/logo/raylib_256x256.png" width=256>

**raylib is a simple and easy-to-use library to enjoy videogames programming.**

raylib is highly inspired by Borland BGI graphics lib and by XNA framework and it's specially well suited for prototyping, tooling, graphical applications, embedded systems and education.

*NOTE for ADVENTURERS: raylib is a programming library to enjoy videogames programming; no fancy interface, no visual helpers, no auto-debugging... just coding in the most pure spartan-programmers way.*

Ready to learn? Jump to [code examples!](http://www.raylib.com/examples.html)

<br>

[![GitHub contributors](https://img.shields.io/github/contributors/raysan5/raylib)](https://github.com/raysan5/raylib/graphs/contributors)
[![GitHub All Releases](https://img.shields.io/github/downloads/raysan5/raylib/total)](https://github.com/raysan5/raylib/releases)
[![GitHub commits since tagged version](https://img.shields.io/github/commits-since/raysan5/raylib/2.6.0)](https://github.com/raysan5/raylib/commits/master)
[![License](https://img.shields.io/badge/license-zlib%2Flibpng-blue.svg)](LICENSE)

[![Chat on Discord](https://img.shields.io/discord/426912293134270465.svg?logo=discord)](https://discord.gg/VkzNHUE)
[![GitHub stars](https://img.shields.io/github/stars/raysan5/raylib?style=social)](https://github.com/raysan5/raylib/stargazers)
[![Twitter Follow](https://img.shields.io/twitter/follow/raysan5?style=social)](https://twitter.com/raysan5)
[![Subreddit subscribers](https://img.shields.io/reddit/subreddit-subscribers/raylib?style=social)](https://www.reddit.com/r/raylib/)

[![Travis (.org)](https://img.shields.io/travis/raysan5/raylib?label=Travis%20CI%20Build%20Status%20-%20Linux,%20OSX,%20Android,%20Windows)](https://travis-ci.org/raysan5/raylib)
[![AppVeyor](https://img.shields.io/appveyor/build/raysan5/raylib?label=AppVeyor%20CI%20Build%20Status%20-%20Windows%20(mingw,%20msvc15))](https://ci.appveyor.com/project/raysan5/raylib)

[![Actions Status](https://github.com/raysan5/raylib/workflows/CI%20-%20Source%20&%20Examples%20-%20Windows/badge.svg)](https://github.com/raysan5/raylib/actions)
[![Actions Status](https://github.com/raysan5/raylib/workflows/CI%20-%20Source%20&%20Examples%20-%20Linux/badge.svg)](https://github.com/raysan5/raylib/actions)
[![Actions Status](https://github.com/raysan5/raylib/workflows/CI%20-%20Source%20&%20Examples%20-%20macOS/badge.svg)](https://github.com/raysan5/raylib/actions)

features
--------
  - **NO external dependencies**, all required libraries are bundled into raylib
  - Multiple platforms supported: **Windows, Linux, MacOS, Android, HTML5... and more!**
  - Written in plain C code (C99) in PascalCase/camelCase notation
  - Hardware accelerated with OpenGL (**1.1, 2.1, 3.3 or ES 2.0**)
  - **Unique OpenGL abstraction layer** (usable as standalone module): [rlgl](https://github.com/raysan5/raylib/blob/master/src/rlgl.h)
  - Multiple **Fonts** formats supported (TTF, XNA fonts, AngelCode fonts)
  - Outstanding texture formats support, including compressed formats (DXT, ETC, ASTC)
  - **Full 3D support**, including 3D Shapes, Models, Billboards, Heightmaps and more! 
  - Flexible Materials system, supporting classic maps and **PBR maps**
  - **Animated 3D models** supported (skeletal bones animation)
  - Shaders support, including model and **postprocessing** shaders.
  - **Powerful math module** for Vector, Matrix and Quaternion operations: [raymath](https://github.com/raysan5/raylib/blob/master/src/raymath.h)
  - Audio loading and playing with streaming support (WAV, OGG, MP3, FLAC, XM, MOD)
  - **VR stereo rendering** support with configurable HMD device parameters
  - Huge examples collection with [+115 code examples](https://github.com/raysan5/raylib/tree/master/examples)!
  - Bindings to [+40 programming languages](https://github.com/raysan5/raylib/blob/master/BINDINGS.md)!
  - Free and open source.

raylib uses on its [core](https://github.com/raysan5/raylib/blob/master/src/core.c) module the outstanding [GLFW3](http://www.glfw.org/) library, embedded in the form of [rglfw](https://github.com/raysan5/raylib/blob/master/src/rglfw.c) module, to avoid external dependencies.

raylib uses on its [raudio](https://github.com/raysan5/raylib/blob/master/src/raudio.c) module, the amazing [miniaudio](https://github.com/dr-soft/miniaudio) library to support multiple platforms and multiple audio backends.

raylib uses internally several single-file header-only libraries to support different fileformats loading and saving, all those libraries are embedded with raylib and available in [src/external](https://github.com/raysan5/raylib/tree/master/src/external) directory. Check [raylib Wiki](https://github.com/raysan5/raylib/wiki/raylib-dependencies) for a detailed list.

*On Android platform, `native_app_glue` module (provided by Android NDK) and native Android libraries are used to manage window/context, inputs and activity life cycle.*

*On Raspberry Pi platform (native mode), `Videocore API` and `EGL` libraries are used for window/context management. Inputs are processed using `evdev` Linux libraries*

*On Web platform, raylib uses `emscripten` provided libraries for several input events management, specially noticeable the touch events support.*

build and installation
----------------------

raylib binary releases for Windows, Linux and macOS are available at the [Github Releases page](https://github.com/raysan5/raylib/releases).

raylib is also available via multiple [package managers](https://github.com/raysan5/raylib/issues/613) on multiple OS distributions.

#### Installing and building raylib via vcpkg

You can download and install raylib using the [vcpkg](https://github.com/Microsoft/vcpkg) dependency manager:

      git clone https://github.com/Microsoft/vcpkg.git
      cd vcpkg
      ./bootstrap-vcpkg.sh
      ./vcpkg integrate install
      vcpkg install raylib

*The raylib port in vcpkg is kept up to date by Microsoft team members and community contributors. If the version is out of date, please [create an issue or pull request](https://github.com/Microsoft/vcpkg) on the vcpkg repository.*

#### Building raylib on multiple platforms

[raylib Wiki](https://github.com/raysan5/raylib/wiki#development-platforms) contains detailed instructions on building and usage on multiple platforms.

 - [Working on Windows](https://github.com/raysan5/raylib/wiki/Working-on-Windows)
 - [Working on macOS](https://github.com/raysan5/raylib/wiki/Working-on-macOS)
 - [Working on GNU Linux](https://github.com/raysan5/raylib/wiki/Working-on-GNU-Linux)
 - [Working on FreeBSD](https://github.com/raysan5/raylib/wiki/Working-on-FreeBSD)
 - [Working on Raspberry Pi](https://github.com/raysan5/raylib/wiki/Working-on-Raspberry-Pi)
 - [Working for Android](https://github.com/raysan5/raylib/wiki/Working-for-Android)
 - [Working for Web (HTML5)](https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5))
 - [Working for UWP (Universal Window Platform)](https://github.com/raysan5/raylib/wiki/Working-for-UWP)
 - [Working anywhere with CMake](https://github.com/raysan5/raylib/wiki/Working-with-CMake)

*Note that Wiki is open for edit, if you find some issue while building raylib for your target platform, feel free to edit the Wiki or open and issue related to it.*

#### Using raylib with multiple IDEs

raylib has been developed on Windows platform using [Notepad++](https://notepad-plus-plus.org/) and [MinGW GCC](http://mingw-w64.org/doku.php) compiler but it can be used with other IDEs on multiple platforms.

[Projects directory](https://github.com/raysan5/raylib/tree/master/projects) contains several ready-to-use **project templates** to build raylib and code examples with multiple IDEs.

*Note that there are lots of IDEs supported, some of the provided templates could require some review, please, if you find some issue with some template or you think they could be improved, feel free to send a PR or open a related issue.*

contact
-------

   * Webpage: [http://www.raylib.com](http://www.raylib.com)
   * Discord: [https://discord.gg/raylib](https://discord.gg/VkzNHUE)
   * Twitter: [http://www.twitter.com/raysan5](http://www.twitter.com/raysan5)
   * Twitch: [http://www.twitch.tv/raysan5](http://www.twitch.tv/raysan5)
   * Reddit: [https://www.reddit.com/r/raylib](https://www.reddit.com/r/raylib)
   * Patreon: [https://www.patreon.com/raylib](https://www.patreon.com/raylib)
   * YouTube: [https://www.youtube.com/channel/raylib](https://www.youtube.com/channel/UC8WIBkhYb5sBNqXO1mZ7WSQ)

If you are using raylib and enjoying it, please, join our [Discord server](https://discord.gg/VkzNHUE) and let us know! :)

license
-------

raylib is licensed under an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. Check [LICENSE](LICENSE.md) for further details.

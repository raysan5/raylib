<img align="left" src="https://github.com/raysan5/raylib/blob/master/logo/raylib_256x256.png" width=256>

**raylib is a simple and easy-to-use library to enjoy videogames programming.**

raylib is highly inspired by Borland BGI graphics lib and by XNA framework and it's specially well suited for prototyping, tooling, graphical applications, embedded systems and education.

*NOTE for ADVENTURERS: raylib is a programming library to enjoy videogames programming; no fancy interface, no visual helpers, no auto-debugging... just coding in the most pure spartan-programmers way.*

Ready to learn? Jump to [code examples!](http://www.raylib.com/examples.html)


[![Build Status](https://travis-ci.org/raysan5/raylib.svg?branch=master)](https://travis-ci.org/raysan5/raylib)
[![https://ci.appveyor.com/api/projects/status/github/raysan5/raylib?svg=true](https://ci.appveyor.com/api/projects/status/github/raysan5/raylib?svg=true)](https://ci.appveyor.com/project/raysan5/raylib)
[![Chat on Discord](https://img.shields.io/discord/426912293134270465.svg?logo=discord)](https://discord.gg/VkzNHUE)
[![License](https://img.shields.io/badge/license-zlib%2Flibpng-blue.svg)](LICENSE.md)
[![Twitter URL](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Follow)](https://twitter.com/raysan5)

features
--------
  - **NO external dependencies**, all required libraries are bundled into raylib
  - Multiple platforms supported: **Windows, Linux, MacOS, Android... and many more!**
  - Written in plain C code (C99) in PascalCase/camelCase notation
  - Hardware accelerated with OpenGL (**1.1, 2.1, 3.3 or ES 2.0**)
  - **Unique OpenGL abstraction layer** (usable as standalone module): [rlgl](https://github.com/raysan5/raylib/blob/master/src/rlgl.h)
  - Multiple **Fonts** formats supported (XNA fonts, AngelCode fonts, TTF)
  - Outstanding texture formats support, including compressed formats (DXT, ETC, ASTC)
  - **Full 3D support**, including 3D Shapes, Models, Billboards, Heightmaps and more! 
  - Flexible Materials system, supporting classic maps and **PBR maps**
  - **Animated 3D models** supported (skeletal bones animation)
  - Shaders support, including model and postprocessing shaders.
  - **Powerful math module** for Vector, Matrix and Quaternion operations: [raymath](https://github.com/raysan5/raylib/blob/master/src/raymath.h)
  - Audio loading and playing with streaming support (WAV, OGG, MP3, FLAC, XM, MOD)
  - **VR stereo rendering** support with configurable HMD device parameters
  - Huge examples collection with [+95 code examples](https://www.raylib.com/examples.html)!
  - Bindings to [+25 programming languages](https://github.com/raysan5/raylib/blob/master/BINDINGS.md)!


raylib uses on its [core](https://github.com/raysan5/raylib/blob/master/src/core.c) module the outstanding [GLFW3](http://www.glfw.org/) library, embedded in the form of [rglfw](https://github.com/raysan5/raylib/blob/master/src/rglfw.c) module, to avoid external dependencies.

raylib uses on its [raudio](https://github.com/raysan5/raylib/blob/master/src/raudio.c) module, the amazing [miniaudio](https://github.com/dr-soft/miniaudio) library to support multiple platforms and multiple audio backends.

raylib uses internally multiple single-file header-only libraries to support different fileformats loading and saving, all those libraries are embedded with raylib and available in [src/external](https://github.com/raysan5/raylib/tree/master/src/external) directory. Check [raylib Wiki](https://github.com/raysan5/raylib/wiki/raylib-dependencies) for a detailed list.

*On Android platform, `native_app_glue module` module (provided by Android NDK) and native Android libraries are used to manage window/context, inputs and activity life cycle.*

*On Raspberry Pi platform (native mode), `Videocore API` and `EGL` libraries are used for window/context management. Inputs are processed using `evdev` Linux libraries*

build and installation
----------------------

Binary releases for Windows, Linux and macOS are available at the [Github Releases page](https://github.com/raysan5/raylib/releases). raylib is also available via multiple package managers on multiple OS distributions. Check [raylib Wiki](https://github.com/raysan5/raylib/wiki) for more info.

If you wish to build raylib yourself, [the raylib Wiki](https://github.com/raysan5/raylib/wiki) also contains detailed instructions on how to approach that.

raylib has been developed using two tools:

   * Notepad++ (text editor) - [http://notepad-plus-plus.org](http://notepad-plus-plus.org/)
   * MinGW (GCC compiler) - [http://www.mingw.org](http://www.mingw.org/)

Those are the tools recommended to enjoy raylib development.

contact
-------

   * Webpage: [http://www.raylib.com](http://www.raylib.com)
   * Discord: [https://discord.gg/raylib](https://discord.gg/VkzNHUE)
   * Twitter: [http://www.twitter.com/raysan5](http://www.twitter.com/raysan5)
   * Twitch: [http://www.twitch.tv/raysan5](http://www.twitch.tv/raysan5)
   * Reddit: [https://www.reddit.com/r/raylib](https://www.reddit.com/r/raylib)
   * Patreon: [https://www.patreon.com/raylib](https://www.patreon.com/raylib)
   * YouTube: [https://www.youtube.com/channel/raylib](https://www.youtube.com/channel/UC8WIBkhYb5sBNqXO1mZ7WSQ)

If you are using raylib and you enjoy it, please, join our [Discord server](https://discord.gg/VkzNHUE).

license
-------

raylib is licensed under an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. Check [LICENSE](LICENSE.md) for further details.

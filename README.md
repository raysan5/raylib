<img align="left" src="https://github.com/raysan5/raylib/blob/master/logo/raylib_256x256.png" width=256>

**raylib is a simple and accessible library that makes videogames programming enjoyable.**

raylib is heavily inspired by the Borland Graphics Interface and the XNA Framework.

raylib is especially well suited for prototyping, tooling, graphical applications, embedded systems and education.

> NOTE for ADVENTURERS: raylib is a *programming library* for *enjoying videogames programming*.
There will never be a fancy interface, visual helpers, auto-debugging, or anything else that detracts from the art and practice of programming videogames. raylib augments your C or C++ runtime with [a comprehensive API](https://www.raylib.com/cheatsheet/cheatsheet.html) that makes it easy to build videogames from scratch.

Ready to learn? Jump to [the code examples](http://www.raylib.com/examples.html)!

[![Build Status](https://travis-ci.org/raysan5/raylib.svg?branch=master)](https://travis-ci.org/raysan5/raylib)
[![https://ci.appveyor.com/api/projects/status/github/raysan5/raylib?svg=true](https://ci.appveyor.com/api/projects/status/github/raysan5/raylib?svg=true)](https://ci.appveyor.com/project/raysan5/raylib)
[![Chat on Discord](https://img.shields.io/discord/308323056592486420.svg?logo=discord)](https://discord.gg/VkzNHUE)
[![License](https://img.shields.io/badge/license-zlib%2Flibpng-blue.svg)](LICENSE.md)
[![Twitter URL](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Follow)](https://twitter.com/raysan5)

Features
--------

  - **No external dependencies**. All required libraries are bundled into raylib.
  - Crossplatform. Supports **Windows, Linux, MacOS, Android... and many more!**
  - Written in plain C code. Uses C99, with PascalCase/camelCase notation.
  - Hardware accelerated with OpenGL (**1.1, 2.1, 3.3 or ES 2.0**).
  - **Unique OpenGL abstraction layer** (usable as a standalone module): [rlgl](https://github.com/raysan5/raylib/blob/master/src/rlgl.h).
  - Multiple font format support (XNA fonts, AngelCode fonts and TTF).
  - Outstanding texture formats support, including compressed formats (DXT, ETC and ASTC).
  - **Full 3D support**, including 3D Shapes, Models, Billboards, Heightmaps and more! 
  - Flexible materials system, supporting classic maps and **PBR maps**.
  - Custom shaders, including model and postprocessing shaders.
  - **Powerful math module** for vector, matrix and quaternion operations: [raymath](https://github.com/raysan5/raylib/blob/master/src/raymath.h).
  - Support for loading, playing and streaming audio (WAV, OGG, MP3, FLAC, XM and MOD).
  - **VR stereo rendering** support with configurable HMD device parameters.
  - Language bindings for **Lua** ([raylib-lua](https://github.com/raysan5/raylib-lua)), **Go** ([raylib-go](https://github.com/gen2brain/raylib-go)) and [more](https://github.com/raysan5/raylib/blob/master/CONTRIBUTING.md#raylib-bindings)!

raylib has its own [core](https://github.com/raysan5/raylib/blob/master/src/core.c) module (derived from the outstanding [GLFW3](http://www.glfw.org/) library), bundled with raylib as the [rglfw](https://github.com/raysan5/raylib/blob/master/src/rglfw.c) module (avoiding external dependencies).

raylib also has its own [audio](https://github.com/raysan5/raylib/blob/master/src/raudio.c) module, based on the amazing [mini_al](https://github.com/dr-soft/mini_al) audio library, provided as a single file (header-only), and supporting multiple platforms and various audio backends.

raylib relies internally on multiple single-file, header-only libraries that support loading and saving multiple file formats. All of those libraries are bundled with raylib, and are available in the [`src/external`](https://github.com/raysan5/raylib/tree/master/src/external) directory.

*On Android, the `native_app_glue module` (provided by the Android NDK) and native Android libraries are used to manage the window and context, inputs and the activity life cycle.*

*On Raspberry Pi, the `Videocore API` and `EGL` libraries are used for window and context management, and for reading raw inputs.*

Building and Installation
-------------------------

Binary releases for Windows, Linux and macOS are available at [the Github Releases page](https://github.com/raysan5/raylib/releases). raylib is also available via multiple package managers, across multiple OS distributions. For more info, check [the raylib Wiki](https://github.com/raysan5/raylib/wiki).

If you wish to build raylib yourself, [the raylib Wiki](https://github.com/raysan5/raylib/wiki) also contains detailed instructions on how to approach that.

raylib has been developed exclusively using two tools:

   * Notepad++ (text editor) - [http://notepad-plus-plus.org](http://notepad-plus-plus.org/)
   * MinGW (GCC compiler) - [http://www.mingw.org](http://www.mingw.org/)

Those tools are recommended for raylib development, and are the tools that my students use.

Contact
-------

   * Webpage: [http://www.raylib.com](http://www.raylib.com)
   * Twitter: [http://www.twitter.com/raysan5](http://www.twitter.com/raysan5)
   * Facebook: [http://www.facebook.com/raylibgames](http://www.facebook.com/raylibgames)
   * Reddit: [https://www.reddit.com/r/raylib](https://www.reddit.com/r/raylib)
   * Twitch: [http://www.twitch.tv/raysan5](http://www.twitch.tv/raysan5)
   * Patreon: [https://www.patreon.com/raylib](https://www.patreon.com/raylib)
   * Discord: [https://discord.gg/raylib](https://discord.gg/VkzNHUE)
   * YouTube: [https://www.youtube.com/channel/raylib](https://www.youtube.com/channel/UC8WIBkhYb5sBNqXO1mZ7WSQ)

If you use raylib and enjoy it, please [let me know][raysan5].

If you are able to contribute, then please [helpme](http://www.raylib.com/helpme.html)!

License
-------

raylib is licensed under an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. See [the LICENSE file](LICENSE.md) for further details.

[raysan5]: mailto:ray@raylib.com "Ramon Santamaria - Ray San"

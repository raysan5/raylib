<img align="left" src="https://github.com/raysan5/raylib/blob/master/logo/raylib_256x256.png" width=256>

**raylib is a simple and easy-to-use library to learn videogames programming.**

raylib is highly inspired by Borland BGI graphics lib and by XNA framework.

raylib could be useful for prototyping, tools development, graphic applications, embedded systems and education.

NOTE for ADVENTURERS: raylib is a programming library to learn videogames programming; 
no fancy interface, no visual helpers, no auto-debugging... just coding in the most 
pure spartan-programmers way. Are you ready to learn? Jump to [code examples!](http://www.raylib.com/examples.html)

[![Build Status](https://travis-ci.org/raysan5/raylib.svg?branch=develop)](https://travis-ci.org/raysan5/raylib)
[![https://ci.appveyor.com/api/projects/status/github/raysan5/raylib?svg=true](https://ci.appveyor.com/api/projects/status/github/raysan5/raylib?svg=true)](https://ci.appveyor.com/project/raysan5/raylib)
[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/raylib/Lobby?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge)
[![Chat on Discord](https://img.shields.io/discord/308323056592486420.svg?logo=discord)](https://discord.gg/VkzNHUE)
[![License](https://img.shields.io/badge/license-zlib%2Flibpng-blue.svg)](LICENSE.md)
[![Twitter URL](https://img.shields.io/twitter/url/http/shields.io.svg?style=social&label=Follow)](https://twitter.com/raysan5)

features
--------

   *  Written in plain C code (C99) in PascalCase/camelCase notation
   *  Hardware accelerated with OpenGL (1.1, 2.1, 3.3 or ES2 - choose at compile)
   *  Unique OpenGL abstraction layer (usable as standalone module): [rlgl](https://github.com/raysan5/raylib/blob/master/src/rlgl.c)
   *  Powerful fonts module with SpriteFonts support (XNA fonts, AngelCode fonts, TTF)
   *  Outstanding texture formats support, including compressed formats (DXT, ETC, ASTC)
   *  Full 3d support for 3d Shapes, Models, Billboards, Heightmaps and more!
   *  Flexible Materials system, supporting classic maps and PBR maps
   *  Shaders support, including Model shaders and Postprocessing shaders
   *  Powerful math module for Vector, Matrix and Quaternion operations: [raymath](https://github.com/raysan5/raylib/blob/master/src/raymath.h)
   *  Audio loading and playing with streaming support (WAV, OGG, FLAC, XM, MOD)
   *  Multiple platforms support: Windows, Linux, FreeBSD, OpenBSD, NetBSD, DragonFly, MacOS, UWP, Android, Raspberry Pi, HTML5.
   *  VR stereo rendering with configurable HMD device parameters
   *  NO external dependencies, all required libraries included with raylib
   *  Complete bindings to LUA ([raylib-lua](https://github.com/raysan5/raylib-lua)) and Go ([raylib-go](https://github.com/gen2brain/raylib-go))

raylib uses on its [core](https://github.com/raysan5/raylib/blob/master/src/core.c) module the outstanding [GLFW3](http://www.glfw.org/) library, embedded inside raylib in the form of [rglfw](https://github.com/raysan5/raylib/blob/master/src/rglfw.c) module, avoiding that way external dependencies.

raylib uses on its [audio](https://github.com/raysan5/raylib/blob/master/src/audio.c) module, the amazing [mini_al](https://github.com/dr-soft/mini_al) audio library, single-file header-only and supporting multiple platforms and multiple audio backends.

raylib uses internally multiple single-file header-only libraries to support multiple fileformats loading and saving, all those libraries are embedded with raylib and available in [src/external](https://github.com/raysan5/raylib/tree/master/src/external) directory.

*On Android, `native_app_glue module` (provided by Android NDK) and native Android libraries are used to manage window/context, inputs and activity life cycle.*

*On Raspberry Pi, Videocore API and EGL libraries are used for window/context management and raw inputs reading.*

build and installation
----------------------

Binary releases for Windows, Linux and macOS are available at the [Github Releases](https://github.com/raysan5/raylib/releases) page. raylib is also available via multiple package managers on multiple OS distributions. For more info check [raylib Wiki](https://github.com/raysan5/raylib/wiki).

To build raylib yourself, check out also the [raylib Wiki](https://github.com/raysan5/raylib/wiki) for detailed instructions.

raylib has been developed using exclusively two tools: 

   * Notepad++ (text editor) - [http://notepad-plus-plus.org/](http://notepad-plus-plus.org/)
   * MinGW (GCC compiler) - [http://www.mingw.org/](http://www.mingw.org/)
   
Those are the tools recommended to develop with raylib, in fact, those are the tools my students use. 

contact
-------

   * Webpage: [http://www.raylib.com](http://www.raylib.com)
   * Twitter: [http://www.twitter.com/raysan5](http://www.twitter.com/raysan5)
   * Facebook: [http://www.facebook.com/raylibgames](http://www.facebook.com/raylibgames)
   * Reddit: [https://www.reddit.com/r/raylib](https://www.reddit.com/r/raylib)
   * Twitch: [http://www.twitch.tv/raysan5](http://www.twitch.tv/raysan5)
   * Patreon: [https://www.patreon.com/raylib](https://www.patreon.com/raylib)
   * YouTube: [https://www.youtube.com/channel/raylib](https://www.youtube.com/channel/UC8WIBkhYb5sBNqXO1mZ7WSQ)

If you are using raylib and you enjoy it, please, [let me know][raysan5].

If you feel you can help, then, [helpme!](http://www.raylib.com/helpme.html)

license
-------

raylib is licensed under an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. Check [LICENSE](LICENSE.md) for further details.

[raysan5]: mailto:ray@raylib.com "Ramon Santamaria - Ray San"


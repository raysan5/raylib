<img align="left" src="https://github.com/raysan5/raylib/blob/master/logo/logo256x256.png" width=256>

**raylib is a simple and easy-to-use library to learn videogames programming.**

raylib is highly inspired by Borland BGI graphics lib and by XNA framework.

raylib could be useful for prototyping, tools development, graphic applications, embedded systems and education.

NOTE for ADVENTURERS: raylib is a programming library to learn videogames programming; 
no fancy interface, no visual helpers, no auto-debugging... just coding in the most 
pure spartan-programmers way. Are you ready to learn? Jump to [code examples!](http://www.raylib.com/examples.html)

features
--------
 
   *  Written in plain C code (C99) in PascalCase/camelCase notation
   *  Hardware accelerated with OpenGL (1.1, 2.1, 3.3 or ES2)
   *  Unique OpenGL abstraction layer (usable as standalone module): [rlgl](https://github.com/raysan5/raylib/blob/master/src/rlgl.c)
   *  Powerful fonts module with SpriteFonts support (XNA bitmap fonts, AngelCode fonts, TTF)
   *  Outstanding texture formats support, including compressed formats (DXT, ETC, PVRT, ASTC)
   *  Basic 3d support for Geometrics, Models, Billboards, Heightmaps and Cubicmaps
   *  Flexible Materials system, supporting classic maps and PBR maps
   *  Shaders support, including Model shaders and Postprocessing shaders
   *  Powerful math module for Vector, Matrix and Quaternion operations: [raymath](https://github.com/raysan5/raylib/blob/master/src/raymath.h)
   *  Audio loading and playing with streaming support (WAV, OGG, FLAC, XM, MOD)
   *  Multiple platforms support: Windows, Linux, Mac, **Android**, **Raspberry Pi** and **HTML5**
   *  VR stereo rendering support with configurable HMD device parameters
   *  Minimal external dependencies (GLFW3, OpenGL, OpenAL)
   *  Complete bindings to LUA ([raylib-lua](https://github.com/raysan5/raylib-lua)) and Go ([raylib-go](https://github.com/gen2brain/raylib-go))

raylib uses on its core module the outstanding [GLFW3](http://www.glfw.org/) library. The best option I found for 
multiplatform (Windows, Linux, Mac) window/context and input management (clean, focused, great license, well documented, modern, maintained, ...).

raylib uses on its [audio](https://github.com/raysan5/raylib/blob/master/src/audio.c) module, [OpenAL Soft](http://kcat.strangesoft.net/openal.html) audio library, in multiple flavours,
to accomodate to Android, Raspberry Pi and HTML5.

*On Android, `native_app_glue module` (provided by Android NDK) and native Android libraries are used to manage window/context, inputs and activity life cycle.*

*On Raspberry Pi, Videocore API and EGL libraries are used for window/context management and raw inputs reading.*

building
--------

For detailed building instructions, check [raylib Wiki](https://github.com/raysan5/raylib/wiki).

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
   * Patreon: [https://www.patreon.com/raysan5](https://www.patreon.com/raysan5)

If you are using raylib and you enjoy it, please, [let me know][raysan5].

If you feel you can help, then, [helpme!](http://www.raylib.com/helpme.html)

license
-------

raylib is licensed under an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. Check [LICENSE](LICENSE.md) for further details.

[raysan5]: mailto:ray@raylib.com "Ramon Santamaria - Ray San"


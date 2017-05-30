<img src="https://github.com/raysan5/raylib/blob/master/logo/logo256x256.png" width=256>

about
-----

raylib is a simple and easy-to-use library to learn videogames programming.

raylib is highly inspired by Borland BGI graphics lib and by XNA framework. 

NOTE for ADVENTURERS: raylib is a programming library to learn videogames programming; 
no fancy interface, no visual helpers, no auto-debugging... just coding in the most 
pure spartan-programmers way. Are you ready to learn? Jump to [code examples!](http://www.raylib.com/examples.html)

features
--------
 
   *  Written in plain C code (C99)
   *  Uses PascalCase/camelCase notation
   *  Hardware accelerated with OpenGL (1.1, 2.1, 3.3 or ES2)
   *  Unique OpenGL abstraction layer (usable as standalone module): [rlgl](https://github.com/raysan5/raylib/blob/master/src/rlgl.c)
   *  Powerful fonts module with SpriteFonts support (XNA bitmap fonts, AngelCode fonts, TTF)
   *  Outstanding texture formats support, including compressed formats (DXT, ETC, PVRT, ASTC)
   *  Basic 3d support for Geometrics, Models, Billboards, Heightmaps and Cubicmaps
   *  Flexible Materials system, supporting by default diffuse, normal and specular maps
   *  Shaders support, including Model shaders and Postprocessing shaders
   *  Powerful math module for Vector, Matrix and Quaternion operations: [raymath](https://github.com/raysan5/raylib/blob/master/src/raymath.h)
   *  Audio loading and playing with streaming support (WAV, OGG, FLAC, XM, MOD)
   *  Multiple platforms support: Windows, Linux, Mac, **Android**, **Raspberry Pi** and **HTML5**
   *  VR stereo rendering support with configurable HMD device parameters
   *  Custom color palette for fancy visuals on raywhite background
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

acknowledgements
---------------

I believe that time is the most valuable resource and the following people have invested part of their time
contributing (in some way or another) to make raylib project better. Huge thanks!

 - [Zopokx](https://github.com/Zopokx) for testing the web.
 - [Elendow](http://www.elendow.com) for testing and helping on web development.
 - Victor Dual for implementing and testing 3D shapes functions.
 - Marc Palau for implementing and testing 3D shapes functions and contribute on camera and gestures modules.
 - Kevin Gato for improving texture internal formats support and helping on raygui development. 
 - Daniel Nicolas for improving texture internal formats support and helping on raygui development. 
 - Marc Agüera for testing and using raylib on a real product ([Koala Seasons](http://www.koalaseasons.com))
 - Daniel Moreno for testing and using raylib on a real product ([Koala Seasons](http://www.koalaseasons.com))
 - Daniel Gomez for testing and using raylib on a real product ([Koala Seasons](http://www.koalaseasons.com))
 - [Sergio Martinez](https://github.com/anidealgift) for helping on raygui development and tools development (raygui_styler).
 - [Victor Fisac](https://github.com/victorfisac) for developing physics raylib module (physac) and implementing materials and lighting systems... among multiple other improvements and multiple tools and games. 
 - Albert Martos for helping on raygui and porting examples and game-templates to Android and HTML5.
 - Ian Eito for helping on raygui and porting examples and game-templates to Android and HTML5.
 - [procedural](https://github.com/procedural) for testing raylib on Linux, correcting some bugs and adding several mouse functions.
 - [Chris Hemingway](https://github.com/cHemingway) for improving raylib on OSX build system.
 - [Emanuele Petriglia](https://github.com/LelixSuper) for working on multiple GNU/Linux improvements and developing [TicTacToe](https://github.com/LelixSuper/TicTacToe) raylib game.
 - [Joshua Reisenauer](https://github.com/kd7tck) for adding audio modules support (XM, MOD) and reviewing audio system.
 - [Marcelo Paez](https://github.com/paezao) for his help on OSX to solve High DPI display issue. Thanks Marcelo!
 - [Ghassan Al-Mashareqa](https://github.com/ghassanpl) for his amazing contribution with raylib Lua module, I just work over his code to implement [rlua](https://github.com/raysan5/raylib/blob/master/src/rlua.h)
 - [Teodor Stoenescu](https://github.com/teodor-stoenescu) for his improvements on OBJ object loading.
 - [RDR8](https://github.com/RDR8) for helping with Linux build improvements
 - [Saggi Mizrahi](https://github.com/ficoos) for multiple fixes on Linux and audio system
 - [Daniel Lemos](https://github.com/xspager) for fixing issues on Linux games building
 - [Joel Davis](https://github.com/joeld42) for adding raycast picking utilities and a [great example](https://github.com/raysan5/raylib/blob/master/examples/models/models_mesh_picking.c)
 - [Richard Goodwin](https://github.com/AudioMorphology) for adding RPI touchscreen support
 - [Milan Nikolic](https://github.com/gen2brain) for adding Android build support with custom standalone toolchain
 
Please, if I forget someone in this list, excuse me and write me an email to remind me to add you!
	
license
-------

raylib is licensed under an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. Check [LICENSE](LICENSE.md) for further details.

[raysan5]: mailto:ray@raylib.com "Ramon Santamaria - Ray San"


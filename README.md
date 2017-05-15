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
   *  Powerful fonts module with multiple SpriteFonts formats support (XNA bitmap fonts, AngelCode fonts, TTF)
   *  Outstanding texture formats support, including compressed formats (DXT, ETC, PVRT, ASTC)
   *  Basic 3d support for Shapes, Models, Billboards, Heightmaps and Cubicmaps
   *  Materials (diffuse, normal, specular) and Lighting (point, directional, spot) support
   *  Shaders support, including Model shaders and Postprocessing shaders
   *  Powerful math module for Vector and Matrix operations: [raymath](https://github.com/raysan5/raylib/blob/master/src/raymath.c) 
   *  Audio loading and playing with streaming support and mixing channels (WAV, OGG, FLAC, XM, MOD)
   *  Multiple platforms support: Windows, Linux, Mac, **Android**, **Raspberry Pi** and **HTML5**
   *  VR stereo rendering support with configurable HMD device parameters
   *  Custom color palette for fancy visuals on raywhite background
   *  Minimal external dependencies (GLFW3, OpenGL, OpenAL)
   *  Complete binding to LUA: [raylib-lua](https://github.com/raysan5/raylib-lua)

raylib uses on its core module the outstanding [GLFW3](http://www.glfw.org/) library. The best option I found for 
multiplatform (Windows, Linux, Mac) window/context and input management (clean, focused, great license, well documented, modern, maintained, ...).

raylib uses on its [audio](https://github.com/raysan5/raylib/blob/master/src/audio.c) module, [OpenAL Soft](http://kcat.strangesoft.net/openal.html) audio library, in multiple flavours,
to accomodate to Android, Raspberry Pi and HTML5.

On Android, raylib uses `native_app_glue module` (provided by Android NDK) and native Android libraries to manage window/context, inputs and activity life cycle.

On Raspberry Pi, raylib uses Videocore API and EGL for window/context management and raw inputs reading.

raylib is licensed under a zlib/libpng license. View [LICENSE](https://github.com/raysan5/raylib/blob/master/LICENSE.md).

tools requirements
------------------

raylib has been developed using exclusively two tools: 

   * Notepad++ (text editor) - [http://notepad-plus-plus.org/](http://notepad-plus-plus.org/)
   * MinGW (GCC compiler) - [http://www.mingw.org/](http://www.mingw.org/)
   
Those are the tools I recommend to develop with raylib, in fact, those are the tools my students use. 
I believe those are the best tools to train spartan-programmers.

Someone could argue about debugging. raylib is a library intended for learning and I think C it's a clear enough language
to allow writing small-mid size programs with a printf-based debugging. All raylib examples have also been written this way.

Since raylib v1.1, you can download a Windows Installer package for easy installation and configuration. Check [raylib Webpage](http://www.raylib.com/)

building source (generate libraylib.a)
--------------------------------------

Check raylib wiki page: [Compile for...](https://github.com/raysan5/raylib/wiki)

building examples
-----------------

Check raylib wiki page: [Compile for...](https://github.com/raysan5/raylib/wiki)

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
 - Sergio Martinez for helping on raygui development and tools development (raygui_styler).
 - [Victor Fisac](https://github.com/victorfisac) for developing physics raylib module (physac) and implementing materials and lighting systems... among multiple other improvements and multiple tools and games. 
 - Albert Martos for helping on raygui and porting examples and game-templates to Android and HTML5.
 - Ian Eito for helping on raygui and porting examples and game-templates to Android and HTML5.
 - [procedural](https://github.com/procedural) for testing raylib on Linux, correcting some bugs and adding several mouse functions.
 - [Chris Hemingway](https://github.com/cHemingway) for improving raylib on OSX build system.
 - [Emanuele Petriglia](https://github.com/LelixSuper) for working on multiple GNU/Linux improvements and developing [TicTacToe](https://github.com/LelixSuper/TicTacToe) raylib game.
 - [Joshua Reisenauer](https://github.com/kd7tck) for adding audio modules support (XM, MOD) and reviewing audio system.
 - Marcelo Paez (paezao) for his help on OSX to solve High DPI display issue. Thanks Marcelo!
 - [Ghassan Al-Mashareqa](https://github.com/ghassanpl) for his amazing contribution with raylib Lua module, I just work over his code to implement [rlua](https://github.com/raysan5/raylib/blob/master/src/rlua.h)
 - [Teodor Stoenescu](https://github.com/teodor-stoenescu) for his improvements on OBJ object loading.
 
Please, if I forget someone in this list, excuse me and write me an email to remind me to add you!
	
[raysan5]: mailto:ray@raylib.com "Ramon Santamaria - Ray San"

license
-------

raylib is licensed under an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. Check [LICENSE](LICENSE.md) for further details.

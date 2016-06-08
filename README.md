<img src="http://www.raylib.com/img/fb_raylib_logo.png" width=256>

about
-----

raylib is a simple and easy-to-use library to learn videogames programming.

raylib is highly inspired by Borland BGI graphics lib and by XNA framework. 

NOTE for ADVENTURERS: raylib is a programming library to learn videogames programming; 
no fancy interface, no visual helpers, no auto-debugging... just coding in the most 
pure spartan-programmers way. Are you ready to learn? Jump to [code examples!](http://www.raylib.com/examples.htm)

history
-------

I've developed videogames for some years and 4 years ago I started teaching videogames development 
to young people with artistic profile, most of them had never written a single line of code.

I started with C language basis and, after searching for the most simple and easy-to-use library to teach 
videogames programming, I found WinBGI; it was great and it worked very well with students, in just a 
couple of weeks that people that had never written a single line of code were able to program (and understand) 
a simple PONG and some of them even a BREAKOUT!

But WinBGI was not the clearer and most organized lib. There were a lot of things I found useless and 
confusing and some function names were not clear enough for most of the students; not to mention points 
like no transparencies support or no hardware acceleration.

So, I decided to create my own lib, hardware accelerated, clear function names, quite organized, well structured, 
plain C coding and, the most important, primarily intended to LEARN videogames programming.

I've coded quite a lot in C# and XNA and I really love it (in fact, my students learn C# after C), 
so, I decided to use C# language notation and XNA naming conventions. That way, students can jump from 
raylib to XNA, MonoGame or similar libs extremely easily.

raylib started as a weekend project and after three months of hard work, raylib 1.0 was published on November 2013.

Enjoy it.

notes on raylib 1.1
-------------------

On April 2014, after 6 month of first raylib release, raylib 1.1 has been released. This new version presents a
complete internal redesign of the library to support OpenGL 1.1, OpenGL 3.3+ and OpenGL ES 2.0.

A new module named [rlgl](https://github.com/raysan5/raylib/blob/master/src/rlgl.h) has been added to the library. This new module translate raylib-OpenGL-style 
immediate mode functions (i.e. rlVertex3f(), rlBegin(), ...) to different versions of OpenGL (1.1, 3.3+, ES2), selectable by one define.

[rlgl](https://github.com/raysan5/raylib/blob/master/src/rlgl.h) also comes with a second new module named [raymath](https://github.com/raysan5/raylib/blob/master/src/raymath.h), which includes
a bunch of useful functions for 3d-math with vectors, matrices and quaternions.

Some other big changes of this new version have been the support for OGG files loading and stream playing, and the
support of DDS texture files (compressed and uncompressed) along with mipmaps support.

Lots of code changes and lot of testing have concluded in this amazing new raylib 1.1.

notes on raylib 1.2
-------------------

On September 2014, after 5 month of raylib 1.1 release, it comes raylib 1.2. Again, this version presents a
complete internal redesign of [core](https://github.com/raysan5/raylib/blob/master/src/core.c) module to support two new platforms: [Android](http://www.android.com/) and [Raspberry Pi](http://www.raspberrypi.org/).

It's been some month of really hard work to accomodate raylib to those new platforms while keeping it easy for the user.
On Android, raylib manages internally the activity cicle, as well as the inputs; on Raspberry Pi, a complete raw input
system has been written from scratch.

A new display initialization system has been created to support multiple resolutions, adding black bars if required;
user only defines desired screen size and it gets properly displayed.

Now raylib can easily deploy games to Android devices and Raspberry Pi (console mode).

Lots of code changes and lot of testing have concluded in this amazing new raylib 1.2.

In December 2014, new raylib 1.2.2 was published with support to compile directly for web (html5) using [emscripten](http://kripken.github.io/emscripten-site/) and [asm.js](http://asmjs.org/).

notes on raylib 1.3
-------------------

On September 2015, after 1 year of raylib 1.2 release, arrives raylib 1.3. This version adds shaders functionality,
improves tremendously textures module and also provides some new modules (camera system, gestures system, IMGUI).

Shaders support is the biggest addition to raylib 1.3, with support for easy shaders loading and use. Loaded shaders can be
attached to 3d models or used as fullscreen postrocessing effects. A bunch of postprocessing shaders are also included
in this release, check raylib/shaders folder.

Textures module has grown to support most of the internal texture formats available in OpenGL (RGB565, RGB888, RGBA5551, RGBA4444, etc.), 
including compressed texture formats (DXT, ETC1, ETC2, ASTC, PVRT); raylib 1.3 can load .dds, .pkm, .ktx, .astc and .pvr files. 

A brand new [camera](https://github.com/raysan5/raylib/blob/develop/src/camera.c) module offers to the user multiple preconfigured ready-to-use camera systems (free camera, 1st person, 3rd person).
Camera modes are very easy to use, just check examples: [core_3d_camera_free.c](https://github.com/raysan5/raylib/blob/develop/examples/core_3d_camera_free.c) and [core_3d_camera_first_person.c](https://github.com/raysan5/raylib/blob/develop/examples/core_3d_camera_first_person.c).

New [gestures](https://github.com/raysan5/raylib/blob/develop/src/gestures.h) module simplifies gestures detection on Android and HTML5 programs.

[raygui](https://github.com/raysan5/raylib/blob/develop/src/raygui.h), the new IMGUI (Immediate Mode GUI) module offers a set of functions to create simple user interfaces,
primary intended for tools development. It's still in experimental state but already fully functional.

Most of the examples have been completely rewritten and +10 new examples have been added to show the new raylib features.

Lots of code changes and lot of testing have concluded in this amazing new raylib 1.3.

notes on raylib 1.4
-------------------

On February 2016, after 4 months of raylib 1.3 release, it comes raylib 1.4. For this new version, 
lots of parts of the library have been reviewed, lots of bugs have been solved and some interesting features have been added.

First big addition is a set of [Image manipulation functions](https://github.com/raysan5/raylib/blob/develop/src/raylib.h#L673) have been added to crop, resize, colorize, flip, dither and even draw image-to-image or text-to-image.
Now a basic image processing can be done before converting the image to texture for usage.

SpriteFonts system has been improved, adding support for AngelCode fonts (.fnt) and TrueType Fonts (using [stb_truetype](https://github.com/nothings/stb/blob/master/stb_truetype.h) helper library). 
Now raylib can read standard .fnt font data and also generate at loading a SpriteFont from a TTF file.

New [physac](https://github.com/raysan5/raylib/blob/develop/src/physac.h) physics module for basic 2D physics support. Still in development but already functional. 
Module comes with some usage examples for basic jump and level interaction and also force-based physic movements.

[raymath](https://github.com/raysan5/raylib/blob/develop/src/raymath.h) module has been reviewed; some bugs have been solved and the module has been converted to a header-only file for easier portability, optionally, functions can also be used as inline. 

[gestures](https://github.com/raysan5/raylib/blob/develop/src/gestures.c) module has redesigned and simplified, now it can process touch events from any source, including mouse. 
This way, gestures system can be used on any platform providing an unified way to work with inputs and allowing the user to create multiplatform games with only one source code.

Raspberry Pi input system has been redesigned to better read raw inputs using generic Linux event handlers (keyboard:`stdin`, mouse:`/dev/input/mouse0`, gamepad:`/dev/input/js0`). 
Gamepad support has also been added (experimental).

Other important improvements are the functional raycast system for 3D picking, including some ray collision-detection functions, 
and the addition of two simple functions for persistent data storage. Now raylib user can save and load game data in a file (only some platforms supported). 
A simple [easings](https://github.com/raysan5/raylib/blob/develop/src/easings.h) module has also been added for values animation.

Up to 8 new code examples have been added to show the new raylib features and +10 complete game samples have been provided to learn 
how to create some classic games like Arkanoid, Asteroids, Missile Commander, Snake or Tetris.

Lots of code changes and lots of hours of hard work have concluded in this amazing new raylib 1.4.

notes on raylib 1.5
-------------------

On June 2016, after 4 months of raylib 1.4 release, arrives raylib 1.5. Probably this new version is the biggest boost of the library ever, 
lots of parts of the library have been redesigned, lots of bugs have been solved and some **AMAZING** new features have been added.

New platform support: **Oculus Rift CV1**. raylib introduces VR support for one the most anticipated VR devices in the market.
Supporting Oculus Rift CV1 device makes raylib the only (or one of the few) C libraries in the market to support VR out-of-the-box.

New materials system:

New lighting system:

Complete gamepad support on Raspberry Pi

Redesigned physics module: physac

Up to 8 new code examples have been added to show the new raylib features and the usage of multiple raylib modules as standalone libraries (rlgl, audio).

Lots of code changes (more than 250 commits) and lots of hours of hard work have concluded in this amazing new raylib 1.5.

features
--------
 
   *  Written in plain C code (C99)
   *  Uses C# PascalCase/camelCase notation
   *  Hardware accelerated with OpenGL (1.1, 2.1, 3.3 or ES2)
   *  Unique OpenGL abstraction layer (usable as standalone module): [rlgl](https://github.com/raysan5/raylib/blob/master/src/rlgl.c) 
   *  Powerful fonts module with multiple SpriteFonts formats support (XNA bitmap fonts, AngelCode fonts, TTF)
   *  Outstanding texture formats support, including compressed formats (DXT, ETC, PVRT, ASTC)
   *  Basic 3d support for Shapes, Models, Billboards, Heightmaps and Cubicmaps
   *  Powerful math module for Vector and Matrix operations: [raymath](https://github.com/raysan5/raylib/blob/master/src/raymath.c) 
   *  Audio loading and playing with streaming support (WAV, OGG, XM, MOD)
   *  Custom color palette for fancy visuals on raywhite background
   *  Multiple platforms support: Windows, Linux, Mac, **Android**, **Raspberry Pi**, **HTML5** and **Oculus Rift CV1**

raylib uses on its core module the outstanding [GLFW3](http://www.glfw.org/) library. The best option by far I found for 
multiplatform (Windows, Linux, Mac) window/context and input management (clean, focused, great license, well documented, modern, ...).

raylib uses on its [audio](https://github.com/raysan5/raylib/blob/master/src/audio.c) module, [OpenAL Soft](http://kcat.strangesoft.net/openal.html) audio library, in multiple flavours,
to accomodate to Android, Raspberry Pi and HTML5.

On Android, raylib uses `native_app_glue module` (provided on Android NDK) and native Android libraries to manage window/context, inputs and activity cycle.

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

Check raylib wiki page: [Building source](https://github.com/raysan5/raylib/wiki/Building-source)

building examples
-----------------

Check raylib wiki page: [Building examples](https://github.com/raysan5/raylib/wiki/Building-examples)

contact
-------

   * Webpage: [http://www.raylib.com](http://www.raylib.com)
   * Twitter: [http://www.twitter.com/raysan5](http://www.twitter.com/raysan5)
   * Facebook: [http://www.facebook.com/raylibgames](http://www.facebook.com/raylibgames)
   * Reddit: [https://www.reddit.com/r/raylib](https://www.reddit.com/r/raylib)
   * Twitch: [http://www.twitch.tv/raysan5](http://www.twitch.tv/raysan5)
   * Patreon: [https://www.patreon.com/raysan5](https://www.patreon.com/raysan5)

If you are using raylib and you enjoy it, please, [let me know][raysan5].

If you feel you can help, then, [helpme!](http://www.raylib.com/helpme.htm)

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
 
	
[raysan5]: mailto:raysan5@gmail.com "Ramon Santamaria - Ray San"

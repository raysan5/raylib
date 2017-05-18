<img src="https://github.com/raysan5/raylib/blob/master/logo/logo256x256.png" width=256>

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

A new module named [rlgl](https://github.com/raysan5/raylib/blob/master/src/rlgl.h) has been added to the library. This new module translates raylib-OpenGL-style 
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

Textures module has grown to support most of the internal texture formats available in OpenGL (RGB565, RGB888, RGBA5551, RGBA4444, etc.), including compressed texture formats (DXT, ETC1, ETC2, ASTC, PVRT); raylib 1.3 can load .dds, .pkm, .ktx, .astc and .pvr files. 
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

On July 2016, after 5 months of raylib 1.4 release, arrives raylib 1.5. This new version is the biggest boost of the library until now, lots of parts of the library have been redesigned, lots of bugs have been solved and some **AMAZING** new features have been added.

VR support: raylib supports **Oculus Rift CV1**, one of the most anticipated VR devices in the market. Additionally, raylib supports simulated VR stereo rendering, independent of the VR device; it means, raylib can generate stereo renders with custom head-mounted-display device parameteres, that way, any VR device in the market can be **simulated in any platform** just configuring device parameters (and consequently, lens distortion). To enable VR is [extremely easy](https://github.com/raysan5/raylib/blob/develop/examples/core_oculus_rift.c).

New materials system: now raylib supports standard material properties for 3D models, including diffuse-ambient-specular colors and diffuse-normal-specular textures. Just assign values to standard material and everything is processed internally.

New lighting system: added support for up to 8 configurable lights and 3 light types: **point**, **directional** and **spot** lights. Just create a light, configure its parameters and raylib manages render internally for every 3d object using standard material.

Complete gamepad support on Raspberry Pi: Gamepad system has been completely redesigned. Now multiple gamepads can be easily configured and used; gamepad data is read and processed in raw mode in a second thread.

Redesigned physics module: [physac](https://github.com/raysan5/raylib/blob/develop/src/physac.h) module has been converted to header only and usage [has been simplified](https://github.com/raysan5/raylib/blob/develop/examples/physics_basic_rigidbody.c). Performance has also been singnificantly improved, now physic objects are managed internally in a second thread.

Audio chiptunese support and mixing channels: Added support for module audio music (.xm, .mod) loading and playing. Multiple mixing channels are now also supported. All this features thanks to the amazing work of @kd7tck.

Other additions include a [2D camera system](https://github.com/raysan5/raylib/blob/develop/examples/core_2d_camera.c), render textures for offline render (and most comprehensive [postprocessing](https://github.com/raysan5/raylib/blob/develop/examples/shaders_postprocessing.c)) or support for legacy OpenGL 2.1 on desktop platforms.

This new version is so massive that is difficult to list all the improvements, most of raylib modules have been reviewed and [rlgl](https://github.com/raysan5/raylib/blob/develop/src/rlgl.c) module has been completely redesigned to accomodate to new material-lighting systems and stereo rendering. You can check [CHANGELOG](https://github.com/raysan5/raylib/blob/develop/CHANGELOG) file for a more detailed list of changes.

Up to 8 new code examples have been added to show the new raylib features and also some samples to show the usage of [rlgl](https://github.com/raysan5/raylib/blob/develop/examples/rlgl_standalone.c) and [audio](https://github.com/raysan5/raylib/blob/develop/examples/audio_standalone.c) raylib modules as standalone libraries.

Lots of code changes (+400 commits) and lots of hours of hard work have concluded in this amazing new raylib 1.5.

notes on raylib 1.6
-------------------

On November 2016, only 4 months after raylib 1.5, arrives raylib 1.6. This new version represents another big review of the library and includes some interesting additions. This version conmmemorates raylib 3rd anniversary (raylib 1.0 was published on November 2013) and it is a stepping stone for raylib future. raylib roadmap has been reviewed and redefined to focus on its primary objective: create a simple and easy-to-use library to learn videogames programming. Some of the new features:

Complete [raylib Lua binding](https://github.com/raysan5/raylib-lua). All raylib functions plus the +60 code examples have been ported to Lua, now Lua users can enjoy coding videogames in Lua while using all the internal power of raylib. This addition also open the doors to Lua scripting support for a future raylib-based engine, being able to move game logic (Init, Update, Draw, De-Init) to Lua scripts while keep using raylib functionality.

Completely redesigned [audio module](https://github.com/raysan5/raylib/blob/develop/src/audio.c). Based on the new direction taken in raylib 1.5, it has been further improved and more functionality added (+20 new functions) to allow raw audio processing and streaming. [FLAC file format support](https://github.com/raysan5/raylib/blob/develop/src/external/dr_flac.h) has also been added. In the same line, [OpenAL Soft](https://github.com/kcat/openal-soft) backend is now provided as a static library in Windows to allow static linking and get ride of OpenAL32.dll. Now raylib Windows games are completey self-contained, no external libraries required any more!

[Physac](https://github.com/victorfisac/Physac) module has been moved to its own repository and it has been improved A LOT, actually, library has been completely rewritten from scratch by [@victorfisac](https://github.com/victorfisac), multiple samples have been added together with countless new features to match current standard 2D physic libraries. Results are amazing!

Camera and gestures modules have been reviewed, highly simplified and ported to single-file header-only libraries for easier portability and usage flexibility. Consequently, camera system usage has been simplified in all examples.

Improved Gamepad support on Windows and Raspberry Pi with the addition of new functions for custom gamepad configurations but supporting by default PS3 and Xbox-based gamepads.

Improved textures and text functionality, adding new functions for texture filtering control and better TTF/AngelCode fonts loading and generation support.

Build system improvement. Added support for raylib dynamic library generation (raylib.dll) for users that prefer dynamic library linking. Also thinking on advance users, it has been added pre-configured [Visual Studio C++ 2015 solution](https://github.com/raysan5/raylib/tree/master/project/vs2015) with raylib project and C/C++ examples for users that prefer that professional IDE and compiler.

New examples, new functions, complete code-base review, multiple bugs corrected... this is raylib 1.6. Enjoy making games.

notes on raylib 1.7
-------------------

On May 2017, around 6 month after raylib 1.6, comes another raylib instalment, raylib 1.7. This time library has been improved a lot in terms of consistency and cleanness. As stated in [this patreon article](https://www.patreon.com/posts/raylib-future-7501034), this new raylib version has focused efforts in becoming more simple and easy-to-use to learn videogames programming. Some highlights of this new version are:

More than 30 new functions added to the library, functions to control Window, utils to work with filenames and extensions, functions to draw lines with custom thick, mesh loading, functions for 3d ray collisions detailed detection, funtions for VR simulation and much more... Just check [CHANGELOG](CHANGELOG) for a detailed list of additions!

Support of [configuration flags](https://github.com/raysan5/raylib/issues/200) on every raylib module. Advance users can customize raylib just choosing desired features, defining some configuration flags on modules compilation. That way users can control library size and available functionality.

Improved [build system](https://github.com/raysan5/raylib/blob/master/src/Makefile) for all supported platforms (Windows, Linux, OSX, RPI, Android, HTML5) with a unique Makefile to compile sources. Added support for Android compilation with a custom standalone toolchain and also multiple build compliation flags.

New [examples](http://www.raylib.com/examples.html) and [sample games](http://www.raylib.com/games.html) added. All samples material has been reviewed, removing useless examples and adding more comprehensive ones; all material has been ported to latest raylib version and tested in multiple platforms. Examples folder structure has been improved and also build systems.

Improved library consistency and organization in general. Functions and parameters have been renamed, some parts of the library have been cleaned and simplyfied, some functions has been moved to examples (lighting, Oculus Rift CV1 support) towards a more generic library implementation. Lots of hours have been invested in this process...

Some other features: Gamepad support on HTML5, RPI touch screen support, 32bit audio support, frames timming improvements, public log system, rres file format support, automatic GIF recording...

And here it is another version of **raylib, a simple and easy-to-use library to learn videogames programming**. Enjoy it.


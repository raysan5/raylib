![raylib logo](logo/raylib_256x256.png)

introduction
------------

I started developing videogames in 2006 and some years later I started teaching videogames development to young people with artistic profile, most of students had never written a single line of code.

I decided to start with C language basis and, after searching for the most simple and easy-to-use library to teach videogames programming, I found [WinBGI](http://www.codecutter.net/tools/winbgim/); it was great and it worked very well with students, in just a couple of weeks, those students that had never written a single line of code were able to program (and understand) a simple PONG game, some of them even a BREAKOUT!

But WinBGI was not the clearer and most organized library for my taste. There were lots of things I found confusing and some function names were not clear enough for most of the students; not to mention the lack of transparencies support and no hardware acceleration.

So, I decided to create my own library, hardware accelerated, clear function names, quite organized, well structured, plain C coding and, the most important, primarily intended to learn videogames programming.

My previous videogames development experience was mostly in C# and [XNA](https://en.wikipedia.org/wiki/Microsoft_XNA) and I really loved it, so, I decided to use C# language style notation and XNA naming conventions. That way, students were able to move from raylib to XNA, MonoGame or similar libs extremely easily.

raylib started as a weekend project and after three months of hard work, **raylib 1.0 was published on November 2013**.

Enjoy it.

notes on raylib 1.1
-------------------

On April 2014, after 6 month of first raylib release, raylib 1.1 has been released. This new version presents a complete internal redesign of the library to support OpenGL 1.1, OpenGL 3.3+ and OpenGL ES 2.0.

 - A new module named [rlgl](https://github.com/raysan5/raylib/blob/master/src/rlgl.h) has been added to the library. This new module translates raylib-OpenGL-style immediate mode functions (i.e. rlVertex3f(), rlBegin(), ...) to different versions of OpenGL (1.1, 3.3+, ES2), selectable by one define.

 - [rlgl](https://github.com/raysan5/raylib/blob/master/src/rlgl.h) also comes with a second new module named [raymath](https://github.com/raysan5/raylib/blob/master/src/raymath.h), which includes a bunch of useful functions for 3d-math with vectors, matrices and quaternions.

Some other big changes of this new version have been the support for OGG files loading and stream playing, and the support of DDS texture files (compressed and uncompressed) along with mipmaps support.

Lots of code changes and lot of testing have concluded in this amazing new raylib 1.1.

notes on raylib 1.2
-------------------

On September 2014, after 5 month of raylib 1.1 release, it comes raylib 1.2. Again, this version presents a complete internal redesign of [core](https://github.com/raysan5/raylib/blob/master/src/core.c) module to support two new platforms: [Android](http://www.android.com/) and [Raspberry Pi](http://www.raspberrypi.org/).

It's been some month of really hard work to accomodate raylib to those new platforms while keeping it easy for the users. On Android, raylib manages internally the activity cicle, as well as the inputs; on Raspberry Pi, a complete raw input system has been written from scratch.

 - A new display initialization system has been created to support multiple resolutions, adding black bars if required; user only defines desired screen size and it gets properly displayed.

 - Now raylib can easily deploy games to Android devices and Raspberry Pi (console mode).

Lots of code changes and lot of testing have concluded in this amazing new raylib 1.2.

In December 2014, new raylib 1.2.2 was published with support to compile directly for web (html5) using [emscripten](http://kripken.github.io/emscripten-site/) and [asm.js](http://asmjs.org/).

notes on raylib 1.3
-------------------

On September 2015, after 1 year of raylib 1.2 release, arrives raylib 1.3. This version adds shaders functionality, improves tremendously textures module and also provides some new modules (camera system, gestures system, immediate-mode gui).

 - Shaders support is the biggest addition to raylib 1.3, with support for easy shaders loading and use. Loaded shaders can be attached to 3d models or used as fullscreen postrocessing effects. A bunch of postprocessing shaders are also included in this release, check raylib/shaders folder.

 - Textures module has grown to support most of the internal texture formats available in OpenGL (RGB565, RGB888, RGBA5551, RGBA4444, etc.), including compressed texture formats (DXT, ETC1, ETC2, ASTC, PVRT); raylib 1.3 can load .dds, .pkm, .ktx, .astc and .pvr files. 

 - A brand new [camera](https://github.com/raysan5/raylib/blob/master/src/camera.c) module offers to the user multiple preconfigured ready-to-use camera systems (free camera, 1st person, 3rd person). Camera modes are very easy to use, just check examples: [core_3d_camera_free.c](https://github.com/raysan5/raylib/blob/master/examples/core_3d_camera_free.c) and [core_3d_camera_first_person.c](https://github.com/raysan5/raylib/blob/master/examples/core_3d_camera_first_person.c).

 - New [gestures](https://github.com/raysan5/raylib/blob/master/src/gestures.h) module simplifies gestures detection on Android and HTML5 programs.

 - [raygui](https://github.com/raysan5/raylib/blob/master/src/raygui.h), the new immediate-mode GUI module offers a set of functions to create simple user interfaces, primary intended for tools development. It's still in experimental state but already fully functional.

Most of the examples have been completely rewritten and +10 new examples have been added to show the new raylib features.

Lots of code changes and lot of testing have concluded in this amazing new raylib 1.3.

notes on raylib 1.4
-------------------

On February 2016, after 4 months of raylib 1.3 release, it comes raylib 1.4. For this new version, lots of parts of the library have been reviewed, lots of bugs have been solved and some interesting features have been added.

 - First big addition is a set of [Image manipulation functions](https://github.com/raysan5/raylib/blob/master/src/raylib.h#L673) have been added to crop, resize, colorize, flip, dither and even draw image-to-image or text-to-image. Now a basic image processing can be done before converting the image to texture for usage.

 - SpriteFonts system has been improved, adding support for AngelCode fonts (.fnt) and TrueType Fonts (using [stb_truetype](https://github.com/nothings/stb/blob/master/stb_truetype.h) helper library). Now raylib can read standard .fnt font data and also generate at loading a SpriteFont from a TTF file.

 - New [physac](https://github.com/raysan5/raylib/blob/master/src/physac.h) physics module for basic 2D physics support. Still in development but already functional. Module comes with some usage examples for basic jump and level interaction and also force-based physic movements.

 - [raymath](https://github.com/raysan5/raylib/blob/master/src/raymath.h) module has been reviewed; some bugs have been solved and the module has been converted to a header-only file for easier portability, optionally, functions can also be used as inline. 

 - [gestures](https://github.com/raysan5/raylib/blob/master/src/gestures.c) module has redesigned and simplified, now it can process touch events from any source, including mouse. This way, gestures system can be used on any platform providing an unified way to work with inputs and allowing the user to create multiplatform games with only one source code.

 - Raspberry Pi input system has been redesigned to better read raw inputs using generic Linux event handlers (keyboard:`stdin`, mouse:`/dev/input/mouse0`, gamepad:`/dev/input/js0`). Gamepad support has also been added (experimental).

Other important improvements are the functional raycast system for 3D picking, including some ray collision-detection functions, 
and the addition of two simple functions for persistent data storage. Now raylib user can save and load game data in a file (only some platforms supported). A simple [easings](https://github.com/raysan5/raylib/blob/master/src/easings.h) module has also been added for values animation.

Up to 8 new code examples have been added to show the new raylib features and +10 complete game samples have been provided to learn 
how to create some classic games like Arkanoid, Asteroids, Missile Commander, Snake or Tetris.

Lots of code changes and lots of hours of hard work have concluded in this amazing new raylib 1.4.

notes on raylib 1.5
-------------------

On July 2016, after 5 months of raylib 1.4 release, arrives raylib 1.5. This new version is the biggest boost of the library until now, lots of parts of the library have been redesigned, lots of bugs have been solved and some **AMAZING** new features have been added.

 - VR support: raylib supports **Oculus Rift CV1**, one of the most anticipated VR devices in the market. Additionally, raylib supports simulated VR stereo rendering, independent of the VR device; it means, raylib can generate stereo renders with custom head-mounted-display device parameteres, that way, any VR device in the market can be **simulated in any platform** just configuring device parameters (and consequently, lens distortion). To enable VR is [extremely easy](https://github.com/raysan5/raylib/blob/master/examples/core_oculus_rift.c).

 - New materials system: now raylib supports standard material properties for 3D models, including diffuse-ambient-specular colors and diffuse-normal-specular textures. Just assign values to standard material and everything is processed internally.

 - New lighting system: added support for up to 8 configurable lights and 3 light types: **point**, **directional** and **spot** lights. Just create a light, configure its parameters and raylib manages render internally for every 3d object using standard material.

 - Complete gamepad support on Raspberry Pi: Gamepad system has been completely redesigned. Now multiple gamepads can be easily configured and used; gamepad data is read and processed in raw mode in a second thread.

 - Redesigned physics module: [physac](https://github.com/raysan5/raylib/blob/master/src/physac.h) module has been converted to header only and usage [has been simplified](https://github.com/raysan5/raylib/blob/master/examples/physics_basic_rigidbody.c). Performance has also been singnificantly improved, now physic objects are managed internally in a second thread.

 - Audio chiptunese support and mixing channels: Added support for module audio music (.xm, .mod) loading and playing. Multiple mixing channels are now also supported. All this features thanks to the amazing work of @kd7tck.

Other additions include a [2D camera system](https://github.com/raysan5/raylib/blob/master/examples/core_2d_camera.c), render textures for offline render (and most comprehensive [postprocessing](https://github.com/raysan5/raylib/blob/master/examples/shaders_postprocessing.c)) or support for legacy OpenGL 2.1 on desktop platforms.

This new version is so massive that is difficult to list all the improvements, most of raylib modules have been reviewed and [rlgl](https://github.com/raysan5/raylib/blob/master/src/rlgl.c) module has been completely redesigned to accomodate to new material-lighting systems and stereo rendering. You can check [CHANGELOG](https://github.com/raysan5/raylib/blob/master/CHANGELOG) file for a more detailed list of changes.

Up to 8 new code examples have been added to show the new raylib features and also some samples to show the usage of [rlgl](https://github.com/raysan5/raylib/blob/master/examples/rlgl_standalone.c) and [audio](https://github.com/raysan5/raylib/blob/master/examples/audio_standalone.c) raylib modules as standalone libraries.

Lots of code changes (+400 commits) and lots of hours of hard work have concluded in this amazing new raylib 1.5.

notes on raylib 1.6
-------------------

On November 2016, only 4 months after raylib 1.5, arrives raylib 1.6. This new version represents another big review of the library and includes some interesting additions. This version conmmemorates raylib 3rd anniversary (raylib 1.0 was published on November 2013) and it is a stepping stone for raylib future. raylib roadmap has been reviewed and redefined to focus on its primary objective: create a simple and easy-to-use library to learn videogames programming. Some of the new features:

 - Complete [raylib Lua binding](https://github.com/raysan5/raylib-lua). All raylib functions plus the +60 code examples have been ported to Lua, now Lua users can enjoy coding videogames in Lua while using all the internal power of raylib. This addition also open the doors to Lua scripting support for a future raylib-based engine, being able to move game logic (Init, Update, Draw, De-Init) to Lua scripts while keep using raylib functionality.

 - Completely redesigned [audio module](https://github.com/raysan5/raylib/blob/master/src/raudio.c). Based on the new direction taken in raylib 1.5, it has been further improved and more functionality added (+20 new functions) to allow raw audio processing and streaming. [FLAC file format support](https://github.com/raysan5/raylib/blob/master/src/external/dr_flac.h) has also been added. In the same line, [OpenAL Soft](https://github.com/kcat/openal-soft) backend is now provided as a static library in Windows to allow static linking and get ride of OpenAL32.dll. Now raylib Windows games are completey self-contained, no external libraries required any more!

 - [Physac](https://github.com/victorfisac/Physac) module has been moved to its own repository and it has been improved A LOT, actually, library has been completely rewritten from scratch by [@victorfisac](https://github.com/victorfisac), multiple samples have been added together with countless new features to match current standard 2D physic libraries. Results are amazing!

 - Camera and gestures modules have been reviewed, highly simplified and ported to single-file header-only libraries for easier portability and usage flexibility. Consequently, camera system usage has been simplified in all examples.

 - Improved Gamepad support on Windows and Raspberry Pi with the addition of new functions for custom gamepad configurations but supporting by default PS3 and Xbox-based gamepads.

 - Improved textures and text functionality, adding new functions for texture filtering control and better TTF/AngelCode fonts loading and generation support.

Build system improvement. Added support for raylib dynamic library generation (raylib.dll) for users that prefer dynamic library linking. Also thinking on advance users, it has been added pre-configured [Visual Studio C++ 2015 solution](https://github.com/raysan5/raylib/tree/master/project/vs2015) with raylib project and C/C++ examples for users that prefer that professional IDE and compiler.

New examples, new functions, complete code-base review, multiple bugs corrected... this is raylib 1.6. Enjoy making games.

notes on raylib 1.7
-------------------

On May 2017, around 6 month after raylib 1.6, comes another raylib instalment, raylib 1.7. This time library has been improved a lot in terms of consistency and cleanness. As stated in [this patreon article](https://www.patreon.com/posts/raylib-future-7501034), this new raylib version has focused efforts in becoming more simple and easy-to-use to learn videogames programming. Some highlights of this new version are:

 - More than 30 new functions added to the library, functions to control Window, utils to work with filenames and extensions, functions to draw lines with custom thick, mesh loading, functions for 3d ray collisions detailed detection, funtions for VR simulation and much more... Just check [CHANGELOG](CHANGELOG) for a detailed list of additions!

 - Support of [configuration flags](https://github.com/raysan5/raylib/issues/200) on every raylib module. Advance users can customize raylib just choosing desired features, defining some configuration flags on modules compilation. That way users can control library size and available functionality.

 - Improved [build system](https://github.com/raysan5/raylib/blob/master/src/Makefile) for all supported platforms (Windows, Linux, OSX, RPI, Android, HTML5) with a unique Makefile to compile sources. Added support for Android compilation with a custom standalone toolchain and also multiple build compliation flags.

 - New [examples](http://www.raylib.com/examples.html) and [sample games](http://www.raylib.com/games.html) added. All samples material has been reviewed, removing useless examples and adding more comprehensive ones; all material has been ported to latest raylib version and tested in multiple platforms. Examples folder structure has been improved and also build systems.

 - Improved library consistency and organization in general. Functions and parameters have been renamed, some parts of the library have been cleaned and simplyfied, some functions has been moved to examples (lighting, Oculus Rift CV1 support) towards a more generic library implementation. Lots of hours have been invested in this process...

Some other features: Gamepad support on HTML5, RPI touch screen support, 32bit audio support, frames timing improvements, public log system, rres file format support, automatic GIF recording...

And here it is another version of **raylib, a simple and easy-to-use library to enjoy videogames programming**. Enjoy it.

notes on raylib 1.8
-------------------

October 2017, around 5 months after latest raylib version, another release is published: raylib 1.8. Again, several modules of the library have been reviewed and some new functionality added. Main changes of this new release are:

 - [Procedural image generation](https://github.com/raysan5/raylib/blob/master/examples/textures/textures_image_generation.c) function, a set of new functions have been added to generate gradients, checked, noise and cellular images from scratch. Image generation could be useful for certain textures or learning pourpouses.

 - [Parametric mesh generation](https://github.com/raysan5/raylib/blob/master/examples/models/models_mesh_generation.c) functions, create 3d meshes from scratch just defining a set of parameters, meshes like cube, sphere, cylinder, torus, knot and more can be very useful for prototyping or for lighting and texture testing.

 - PBR Materials support, a completely redesigned shaders and material system allows advance materials definition and usage, with fully customizable shaders. Some new functions have been added to generate the environment textures required for PBR shading and a a new complete [PBR material example](https://github.com/raysan5/raylib/blob/master/examples/models/models_material_pbr.c) is also provided for reference.

 - Custom Android APK build pipeline with [simple Makefile](https://github.com/raysan5/raylib/blob/master/templates/simple_game/Makefile). Actually, full code building mechanism based on plain Makefile has been completely reviewed and Android building has been added for sources and also for examples and templates building into final APK package. This way, raylib Android building has been greatly simplified and integrated seamlessly into standard build scripts.

 - [rlgl](https://github.com/raysan5/raylib/blob/master/src/rlgl.h) module has been completely reviewed and most of the functions renamed for consistency. This way, standalone usage of rlgl is promoted, with a [complete example provided](https://github.com/raysan5/raylib/blob/master/examples/others/rlgl_standalone.c). rlgl offers a pseudo-OpenGL 1.1 immediate-mode programming-style layer, with backends to multiple OpenGL versions.

 - [raymath](https://github.com/raysan5/raylib/blob/master/src/raymath.h) library has been also reviewed to align with other advance math libraries like [GLM](https://github.com/g-truc/glm). Matrix math has been improved and simplified, some new Quaternion functions have been added and Vector3 functions have been renamed all around the library for consistency with new Vector2 functionality.

Additionally, as always, examples and templates have been reviewed to work with new version (some new examples have been added), all external libraries have been updated to latest stable version and latest Notepad++ and MinGW have been configured to work with new raylib. For a full list of changes, just check [CHANGELOG](CHANGELOG).

New installer provided, web updated, examples re-builded, documentation reviewed... **new raylib 1.8 published**. Enjoy coding games.

notes on raylib 2.0
-------------------

It's been 9 month since last raylib version was published, a lots of things have changed since then... This new raylib version represents an inflexion point in the development of the library and so, we jump to a new major version... Here it is the result of almost **5 years and thousands of hours of hard work**... here it is... **raylib 2.0**

In **raylib 2.0** the full API has been carefully reviewed for better consistency, some new functionality has been added and the overall raylib experience has been greatly improved... The key features of new version are:

 - **Complete removal of external dependencies.** Finally, raylib does not require external libraries to be installed and linked along with raylib, all required libraries are contained and compiled within raylib. Obviously some external libraries are required but only the strictly platform-dependant ones, the ones that come installed with the OS. So, raylib becomes a self-contained platform-independent games development library. 

 - **Full redesign of audio module to use the amazing miniaudio library**, along with external dependencies removal, OpenAL library has been replaced by [miniaudio](https://github.com/dr-soft/miniaudio), this brand new library offers automatic dynamic linking with default OS audio systems. Undoubtly, the perfect low-level companion for raylib audio module!

 - **Support for continuous integration building*** through AppVeyor and Travis CI. Consequently, raylib GitHub develop branch has been removed, simplyfing the code-base to a single master branch, always stable. Every time a new commit is deployed, library is compiled for **up-to 12 different configurations**, including multiple platforms, 32bit/64bit and multiple compiler options! All those binaries are automatically attached to any new release!

 - **More platforms supported and tested**, including BSD family (FreeBSD, openBSD, NetBSD, DragonFly) and Linux-based family platforms (openSUSE, Debian, Ubuntu, Arch, NixOS...). raylib has already been added to some package managers! Oh, and last but not less important, **Android 64bit** is already supported by raylib!

 - **Support for TCC compiler!** Thanks to the lack of external dependencies, raylib can now be easily compiled with a **minimal toolchain**, like the one provide by Tiny C Compiler. It opens the door to an amazing future, allowing, for example, static linkage of libtcc for **runtime compilation of raylib-based code**... and the library itself if required! Moreover, TCC is blazing fast, it can compile all raylib in a couple of seconds!

 - Refactored all raylib configuration #defines into a **centralized `config.h` header**, with more than **40 possible configuration options** to compile a totally customizable raylib version including only desired options like supported file-formats or specific functionality support. It allows generating a trully ligth-weight version of the library if desired!

A part of that, lots of new features, like a brand **new font rendering and packaging system** for TTF fonts with **SDF support** (thanks to the amazing STB headers), new functions for **CPU image data manipulation**, new orthographic 3d camera mode, a complete review of `raymath.h` single-file header-only library for better consistency and performance, new examples and way, [way more](https://github.com/raysan5/raylib/blob/master/CHANGELOG).

Probably by now, **raylib 2.0 is the simplest and easiest-to-use library to enjoy (and learn) videogames programming**... but, undoubtly its development has exceeded any initial objective; raylib has become a simple and easy-to-use trully multiplatform portable standalone media library with thousands of possibilities... and that's just the beginning!

notes on raylib 2.5
-------------------

After almost one years since latest raylib installment, here it is **raylib 2.5**. A lot of work has been put on this new version and consequently I decided to bump versioning several digits. The complete list of changes and additions is humungous, details can be found in the [CHANGELOG](CHANGELOG), and here it is a short recap with the highlight improvements.

 - New **window management and file system functions** to query monitor information, deal with clipboard, check directory files info and even launch a URL with default system web browser. Experimental **High-DPI monitor support** has also been added through a compile flag.

 - **Redesigned Gamepad mechanism**, now generic for all platforms and gamepads, no more specific gamepad configurations.
**Redesigned UWP input system**, now raylib supports UWP seamlessly, previous implementation required a custom input system implemented in user code.

 - `rlgl` module has been redesigned to **support a unique buffer for shapes drawing batching**, including LINES, TRIANGLES, QUADS in the same indexed buffer, also added support for multi-buffering if required. Additionally, `rlPushMatrix()`/`rlPopMatrix()` functionality has been reviewed to behave exactly like OpenGL 1.1, `models_rlgl_solar_system` example has been added to illustrate this behaviour.

 - **VR simulator** has been reviewed to **allow custom configuration of Head-Mounted-Device parameters and distortion shader**, `core_vr_simulator` has been properly adapted to showcase this new functionality, now the VR simulator is a generic configurable stereo rendering system that allows any VR device simulation with just a few lines of code or even dynamic tweaking of HMD parameters.

 - Support for **Unicode text drawing**; now raylib processes UTF8 strings on drawing, supporting Unicode codepoints, allowing rendering mostly any existent language (as long as the font with the glyphs is provided). An amazing example showing this feature has also been added: `text_unicode`.

 - Brand **new text management API**, with the addition of multiple functions to deal with string data, including functionality like replace, insert, join, split, append, to uppercase, to lower... Note that most of those functions are intended for text management on rendering, using pre-loaded internal buffers, avoiding new memory allocation that user should free manually.

 - Multiple **new shapes and textures drawing functions** to support rings (`DrawRing()`, `DrawRingLines()`), circle sectors (`DrawCircleSector()`, `DrawCircleSectorLines()`), rounded rectangles (`DrawRectangleRounded()`, `DrawRectangleRoundedLines()`) and also n-patch textures (`DrawTextureNPatch()`), detailed examples have been added to illustrate all this new functionality. 

 - Experimental **cubemap support**, to automatically load multiple cubemap layouts (`LoadTextureCubemap()`). It required some internal `rlgl` redesign to allow cubemap textures.

 - **Skeletal animation support for 3d models**, this addition implied a redesign of `Model` data structure to accomodate multiple mesh/multiple materials support and bones information. Multiple models functions have been reviewed and added on this process, also **glTF models loading support** has been added.

This is a just a brief list with some of the changes of the new **raylib 2.5** but there is way more, about **70 new functions** have been added and several subsystems have been redesigned. More than **30 new examples** have been created to show the new functionalities and better illustrate already available ones.

It has been a long year of hard work to make raylib a solid technology to develop new products over it.

notes on raylib 3.0
-------------------

After **10 months of intense development**, new raylib version is ready. Despite primary intended as a minor release, the [CHANGELIST](CHANGELOG) has grown so big and the library has changed so much internally that it finally became a major release. Library **internal ABI** has reveived a big redesign and review, targeting portability, integration with other platforms and making it a perfect option for other progamming [language bindings](BINDINGS.md).

 - All **global variables** from the multiple raylib modules have been moved to a **global context state**, it has several benefits, first, better code readability with more comprehensive variables naming and categorization (organized by types, i.e. `CORE.Window.display.width`, `CORE.Input.Keyboard.currentKeyState` or `RLGL.State.modelview`). Second, it allows better memory management to load global context state dynamically when required (not at the moment), making it easy to implement a **hot-reloading mechanism** if desired.

 - All **memory allocations** on raylib and its dependencies now use `RL_MALLOC`, `RL_FREE` and similar macros. Now users can easely hook their own memory allocations mechanism if desired, having more control over memory allocated internally by the library. Additionally, it makes it easier to port the library to embedded devices where memory control is critical. For more info check raylib issue #1074.

 - All **I/O file accesses** from raylib are being moved to **memory data access**, now all I/O file access is centralized into just four functions: `LoadFileData()`, `SaveFileData()`, `LoadFileText()`, `SaveFileText()`. Users can just update those functions to any I/O file system. This change makes it easier to integrate raylib with **Virtual File Systems** or custom I/O file implementations.

 - All **raylib data structures** have been reviewed and optimized for pass-by-value usage. One of raylib distinctive design decisions is that most of its functions receive and return data by value. This design makes raylib really simple for newcomers, avoiding pointers and allowing complete access to all structures data in a simple way. The downside is that data is copied on stack every function call and that copy could be costly so, all raylib data structures have been optimized to **stay under 64 bytes** for fast copy and retrieve.

 - All **raylib tracelog messages** have been reviewd and categorized for a more comprehensive output information when developing raylib applications, now all display, input, timer, platform, auxiliar libraries, file-accesses, data loading/unloading issues are properly reported with more detailed and visual messages.

 - `raudio` module has been internally reviewed to accomodate the new `Music` structure (converted from previous pointer format) and the module has been adapted to the **highly improved** [`miniaudio v0.10`](https://github.com/dr-soft/miniaudio).

 - `text` module reviewed to **improve fonts generation** and text management functions, `Font` structure has been redesigned to better accomodate characters data, decoupling individual characters as `Image` glyphs from the font atlas parameters. Several improvements have been made to better support Unicode strings with UTF-8 encoding.

 - **Multiple new examples added** (most of them contributed by raylib users) and all examples reviewed for correct execution on most of the supported platforms, specially Web and Raspberry Pi. A detailed categorized table has been created on github for easy examples navigation and code access.

 - New **GitHub Actions CI** system has been implemented for Windows, Linux and macOS code and examples compilation on every new commit or PR to make sure library keeps stable and usable with no breaking bugs.

Note that only key changes are listed here but there is way more! About **30 new functions**, multiple functions reviewed, bindings to [+40 programming languages](https://github.com/raysan5/raylib/blob/master/BINDINGS.md) and great samples/demos/tutorials [created by the community](https://discord.gg/VkzNHUE), including raylib integration with [Spine](https://github.com/WEREMSOFT/spine-raylib-runtimes), [Unity](https://unitycoder.com/blog/2019/12/09/using-raylib-dll-in-unity/), [Tiled](https://github.com/OnACoffeeBreak/raylib_tiled_import_with_tmx), [Nuklear](http://bedroomcoders.co.uk/implementing-a-3d-gui-with-raylib/), [enet](https://github.com/nxrighthere/NetDynamics) and [more](https://github.com/raysan5/raylib/issues/1079)!

It has been **10 months of improvements** to create the best raylib ever.

Welcome to **raylib 3.0**.

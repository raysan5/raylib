<img src="http://www.raylib.com/img/fb_raylib_logo.png" width=256>

about
-----

raylib is a simple and easy-to-use library to learn videogames programming.

raylib is highly inspired by Borland BGI graphics lib and by XNA framework. 
Allegro and SDL have also been analyzed for reference.

NOTE for ADVENTURERS: raylib is a programming library to learn videogames programming; no fancy interface, no visual helpers, no auto-debugging... just coding in the most pure spartan-programmers way. Are you ready to learn? Jump to [code examples!](http://www.raylib.com/examples.htm)

Since version 1.2.2 raylib can compile directly for web (html5) using emscripten and asm.js, 
to see a demo of raylib features working on web, [check here!](http://www.raylib.com/raylib_demo.html)

history
-------

I've developed videogames for some years and last year I had to taught videogames development 
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

raylib started as a weekend project and after three months of hard work, first version was published.

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
improves textures module and provides some new modules (camera system, gestures system, IMGUI).

Shaders, the biggest addition to raylib, with support for simple and easy shaders loading and use. Loaded shaders can be
assigned to models or used as fullscreen postrocessing shaders.

Textures module has been improved to support most of the internal texture formats available in OpenGL
(RGB565, RGB888, RGBA5551, RGBA4444, etc.), including compressed texture formats (DXT, ETC1, ETC2, ASTC, PVRT).

New camera module offers the user multiple preconfigured ready-to-use camera systems (free camera, 1st person, third person),
very easy to use, just calling functions: SetCameraMode() and UpdateCamera().

New gestures module simplifies getures detection on Android and HTML5 programs.

New IMGUI (Immediate Mode GUI) module: raygui, offers a set of functions to create simple user interfaces,
primary intended for tools development, still in experimental state but already fully functional.

Lots of code changes and lot of testing have concluded in this amazing new raylib 1.3.

features
--------
 
   *  Written in plain C code (C99)
   *  Uses C# PascalCase/camelCase notation
   *  Hardware accelerated with OpenGL (1.1, 3.3+ or ES2)
   *  Unique OpenGL abstraction layer: [rlgl](https://github.com/raysan5/raylib/blob/master/src/rlgl.c) 
   *  Hardware accelerated with OpenGL (1.1, 3.3 or ES2)
   *  Unique OpenGL abstraction layer [rlgl]
   *  Powerful fonts module with SpriteFonts support
   *  Multiple textures support, including DDS, PKM and mipmaps generation
   *  Basic 3d support for Shapes, Models, Heightmaps and Billboards
   *  Powerful math module for Vector and Matrix operations: [raymath](https://github.com/raysan5/raylib/blob/master/src/raymath.c) 
   *  Audio loading and playing with streaming support (WAV and OGG)
   *  Custom color palette for fancy visuals on raywhite background
   *  Multiple platforms support: Windows, Linux, Mac, **Android**, **Raspberry Pi** and **HTML5**

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

**Building raylib sources on desktop platforms:**

_Step 1:_ Using MinGW make tool, just navigate from command line to `raylib/src/` folder and type:

    mingw32-make PLATFORM=PLATFORM_DESKTOP

NOTE: By default raylib compiles using OpenGL 1.1 to maximize compatibility; to use OpenGL 3.3 just type:

    mingw32-make PLATFORM=PLATFORM_DESKTOP GRAPHICS=GRAPHICS_API_OPENGL_33

**Building raylib sources on Raspberry Pi:**

_Step 1._ Make sure you have installed in your Raspberry Pi OpenAL Soft library for audio:

    sudo apt-get install openal1

_Step 2._ Navigate from command line to `raylib/src/` folder and type:

    make

**Building raylib sources for Android:**

_Step 1._ Make sure you have installed Android SDK, Android NDK and Apache Ant tools:

> Download and decompress on C: [Android SDK r23](http://dl.google.com/android/android-sdk_r23.0.2-windows.zip)

> Download and decompress on C: [Android NDK r10b](http://dl.google.com/android/ndk/android-ndk32-r10b-windows-x86.zip)

> Download and decompress on C: [Apache Ant 1.9.4](http://ftp.cixug.es/apache//ant/binaries/apache-ant-1.9.4-bin.zip)

_Step 2._ Create the following environment variables with the correct paths: 

    ANDROID_SDK_TOOLS = C:\android-sdk\platform-tools
    ANDROID_NDK_ROOT = C:\android-ndk-r10b
    ANT_HOME = C:\apache-ant-1.9.4

_Step 3._ Navigate from command line to folder `raylib/template_android/` and type:

    %ANDROID_NDK_ROOT%\ndk-build

NOTE: libraylib.a will be generated in folder `raylib/src_android/obj/local/armeabi/`, it must be copied
to Android project; if using `raylib/template_android` project, copy it to `raylib/template_android/jni/libs/`.

**Building raylib sources for Web (HTML5)**

_Step 1._ Make sure you have installed emscripten SDK:

> Download latest version from [here](http://kripken.github.io/emscripten-site/docs/getting_started/downloads.html). I recommend downloading the [Portable Emscripten SDK for Windows](https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.25.0-portable-64bit.zip) and decompress it in `C:\emsdk-1.25.0` folder. After that, follow the portable version installation instructions.

_Step 2._ Open `raylib/src/makefile` on Notepad++ and run the script named `raylib_makefile_emscripten`

building examples
-----------------

**Building raylib examples on desktop platforms:**

_Step 1:_ Using MinGW make tool, just navigate from command line to `raylib/examples/` folder and type:

    mingw32-make PLATFORM=PLATFORM_DESKTOP

NOTE: Make sure the following libs (and their headers) are placed on their respectibe MinGW folders:

    libglfw3.a    - GLFW3 (static version)
    libglew32.a   - GLEW, OpenGL extension loading, only required if using OpenGL 3.3+ or ES2
    libopenal32.a - OpenAL Soft, audio device management

**Building raylib examples on Raspberry Pi:**

_Step 1._ Make sure you have installed in your Raspberry Pi OpenAL Soft library for audio:

    sudo apt-get install openal1

_Step 2._ Navigate from command line to `raylib/examples/` folder and type:

    make

**Building raylib examples for HTML5 (emscripten):**

_Step 1._ Make sure you have installed emscripten SDK:

> Download latest version from [here](http://kripken.github.io/emscripten-site/docs/getting_started/downloads.html). I recommend downloading the [Portable Emscripten SDK for Windows](https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-1.25.0-portable-64bit.zip) and decompress it in `C:\emsdk-1.25.0` folder. After that, follow the portable version installation instructions.

_Step 2._ Open `raylib/examples/makefile` on Notepad++ and run the script named `raylib_makefile_emscripten`

NOTE: At this moment, raylib examples are not ready to directly compile for HTML5, code needs to be reorganized due to the way web browsers work. To see how code should be refactored to fit compilation for web, check [core_basic_window_web.c](https://github.com/raysan5/raylib/blob/master/examples/core_basic_window_web.c) example.

**Building raylib project for Android (using template):**

_Step 1._ Make sure you have installed Android SDK, Android NDK and Apache Ant tools:

> Download and decompress on C: [Android SDK r23] (http://dl.google.com/android/android-sdk_r23.0.2-windows.zip)

> Download and decompress on C: [Android NDK r10b] (http://dl.google.com/android/ndk/android-ndk32-r10b-windows-x86.zip)

> Download and decompress on C: [Apache Ant 1.9.4] (http://ftp.cixug.es/apache//ant/binaries/apache-ant-1.9.4-bin.zip)

_Step 2._ Create the following environment variables with the correct paths: 

    ANDROID_SDK_TOOLS = C:\android-sdk\platform-tools
    ANDROID_NDK_ROOT = C:\android-ndk-r10b
    ANT_HOME = C:\apache-ant-1.9.4

_Step 3._ To compile project, navigate from command line to folder `raylib/template_android/` and type:

    %ANDROID_NDK_ROOT%\ndk-build

_Step 4._ To generate APK, navigate to folder `raylib/template_android/` and type:

    %ANT_HOME%\bin\ant debug

_Step 5:_ To install APK into connected device (previously intalled drivers and activated USB debug mode on device):

    %ANT_HOME%\bin\ant installd

_Step 6:_ To view log output from device:

    %ANDROID_SDK_TOOLS%\adb logcat -c
    %ANDROID_SDK_TOOLS%\adb -d  logcat raylib:V *:S

**If you have any doubt, [just let me know][raysan5].**

contact
-------

   * Webpage: [http://www.raylib.com](http://www.raylib.com)
   * Twitter: [http://www.twitter.com/raysan5](http://www.twitter.com/raysan5)
   * Facebook: [http://www.facebook.com/raylibgames](http://www.facebook.com/raylibgames)

If you are using raylib and you enjoy it, please, [let me know][raysan5].

If you feel you can help, then, [helpme!](http://www.raylib.com/helpme.htm)

acknowledgments
---------------

The following people have contributed in some way to make raylib project a reality. Big thanks to them!

 - [Zopokx](https://github.com/Zopokx) for testing the web.
 - [Elendow](http://www.elendow.com) for testing and helping on web development.
 - Victor Dual for implementating and testing of 3D shapes functions.
 - Marc Palau for implementating and testing of 3D shapes functions and helping on development of camera and getures modules.
 - Kevin Gato for improving texture internal formats support and helping on raygui development. 
 - Daniel Nicolas for improving texture internal formats support and helping on raygui development. 
 - Marc Agüera for testing and using raylib on a real product (Koala Seasons)
 - Daniel Moreno for testing and using raylib on a real product (Koala Seasons)
 - Daniel Gomez for testing and using raylib on a real product (Koala Seasons)
 - Sergio Martinez for helping on raygui development and tools development.
 
	
[raysan5]: mailto:raysan@raysanweb.com "Ramon Santamaria - Ray San"

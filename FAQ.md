# Frequently Asked Questions

- [What is raylib?](#what-is-raylib)
- [What can I do with raylib?](#what-can-i-do-with-raylib)
- [Which kinds of games can I make with raylib?](#which-kinds-of-games-can-i-make-with-raylib)
- [Can I create non-game applications with raylib?](#can-i-create-non-game-applications-with-raylib)
- [How can I learn to use raylib? Is there some official documentation or tutorials?](#how-can-i-learn-to-use-raylib-is-there-some-official-documentation-or-tutorials)
- [How much does it cost?](#how-much-does-it-cost)
- [What is the raylib license?](#what-is-the-raylib-license)
- [What platforms are supported by raylib?](#what-platforms-are-supported-by-raylib)
- [What programming languages can I use with raylib?](#what-programming-languages-can-i-use-with-raylib)
- [Why is it coded in C?](#why-is-it-coded-in-c)
- [Is raylib a videogames engine?](#is-raylib-a-videogames-engine)
- [What does raylib provide that other engines or libraries don't?](#what-does-raylib-provide-that-other-engines-or-libraries-dont)
- [How does raylib compare to Unity/Unreal/Godot?](#how-does-raylib-compare-to-unityunrealgodot)
- [What development tools are required for raylib?](#what-development-tools-are-required-for-raylib)
- [What are raylib's external dependencies?](#what-are-raylibs-external-dependencies)
- [Can I use raylib with other technologies or libraries?](#can-i-use-raylib-with-other-technologies-or-libraries)
- [What file formats are supported by raylib?](#what-file-formats-are-supported-by-raylib)
- [Does raylib support the Vulkan API?](#does-raylib-support-the-vulkan-api)
- [What could I expect to see in raylib in the future?](#what-could-i-expect-to-see-in-raylib-in-the-future)
- [Who are the raylib developers?](#who-are-the-raylib-developers)
- [MORE QUESTIONS...](https://github.com/raysan5/raylib/wiki/Frequently-Asked-Questions)

### What is raylib?

raylib is a C programming library, designed to be simple and easy-to-use. It provides a set of functions intended for graphics/multimedia applications programming.

### What can I do with raylib?

raylib can be used to create any kind of graphics/multimedia applications: videogames, tools, mobile apps, web applications... Actually it can be used to create any application that requires something to be shown in a display with graphic hardware acceleration (OpenGL); including [IoT](https://en.wikipedia.org/wiki/Internet_of_things) devices with a graphics display.

### Which kinds of games can I make with raylib?

With enough time and effort any kind of game/application can be created but small-mid sized 2d videogames are the best fit. The raylib [examples](https://www.raylib.com/examples.html)/[games](https://www.raylib.com/games.html) and [raylibtech](https://raylibtech.itch.io/) tools are an example of what can be accomplished with raylib.

### Can I create non-game applications with raylib?

Yes, raylib can be used to create any kind of application, not just videogames. For example, it can be used to create [desktop/web tools](https://raylibtech.itch.io/) or also applications for an IoT devices like [Raspberry Pi](https://www.raspberrypi.org/).

### How can I learn to use raylib? Is there some official documentation or tutorials?

raylib does not provide a "standard" API reference documentation like other libraries, all of the raylib functionality is exposed in a simple [cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html). Most of the functions are self-explanatory and the required parameters are very intuitive. It's also highly recommended to take a look to [`raylib.h`](https://github.com/raysan5/raylib/blob/master/src/raylib.h) header file or even the source code, that is very clean and organized, intended for teaching.

raylib also provides a big [collection of examples](https://www.raylib.com/examples.html), to showcase the multiple functionality usage (+120 examples). Examples are categorized by the internal module functionality and also define an estimated level of difficulty to guide the users checking them.

There is also a [FAQ on the raylib Wiki](https://github.com/raysan5/raylib/wiki/Frequently-Asked-Questions) with common technical questions.

There are also many tutorials on the internet and YouTube created by the growing raylib community along the years.

[raylib Discord Community](https://discord.gg/raylib) is also a great place to join and ask questions, the community is very friendly and always ready to help.

### How much does it cost?

raylib is [free and open source](https://github.com/raysan5/raylib). Anyone can use raylib library for free to create games/tools/apps but also the source code of raylib is open for anyone to check it, modify it, adapt it as required or just learn how it works internally.

### What is the raylib license?

raylib source code is licensed under an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. Check [LICENSE](https://github.com/raysan5/raylib/blob/master/LICENSE) for further details.

### What platforms are supported by raylib?

raylib source code can be compiled for the following platforms:

 - Windows (7, 8.1, 10, 11)
 - Linux - Desktop (multiple distributions, X11 and Wayland based)
 - Linux - Native (no windowing system, [DRM](https://en.wikipedia.org/wiki/Direct_Rendering_Manager))
 - macOS (multiple versions, including ARM64)
 - FreeBSD, OpenBSD, NetBSD, DragonFly
 - Raspberry Pi (desktop and native)
 - Android (multiple API versions and architectures)
 - HTML5 (WebAssembly)
 - Haiku

raylib code has also been ported to several [homebrew](https://en.wikipedia.org/wiki/Homebrew_(video_games)) platforms: N3DS, Switch, PS4, PSVita.

Also note that raylib is a low-level library that can be easily ported to any platform with OpenGL support (or similar API).

### What programming languages can I use with raylib?

raylib original version is coded in C language (using some C99 features) but it has bindings to +60 programming languages. Check [BINDINGS.md](https://github.com/raysan5/raylib/blob/master/BINDINGS.md) for details.

### Why is it coded in C?

It's a simple language, no high-level code abstractions like [OOP](https://en.wikipedia.org/wiki/Object-oriented_programming), just data types and functions. It's a very enjoyable language to code.

### Is raylib a videogames engine?

I personally consider raylib a graphics library with some high-level features rather than an engine. The line that separates a library/framework from an engine could be very confusing; raylib provides all the required functionality to create simple games or small applications but it does not provide 3 elements that I personally consider any "engine" should provide: Screen manager, GameObject/Entity manager and Resource Manager. Still, most users do not need those elements or just code simple approaches on their own.

### What does raylib provide that other engines or libraries don't?

I would say "simplicity" and "enjoyment" at a really low-level of coding but actually is up to the user to discover it, to try it and to see if it fits their needs. raylib is not good for everyone but it's worth a try.

### How does raylib compare to Unity/Unreal/Godot?

Those engines are usually big and complex to use, providing lot of functionality. They require some time to learn and test, they usually abstract many parts of the game development process and they usually provide a set of tools to assist users on their creations (like a GUI editor).

raylib is a simple programming library, with no integrated tools or editors. It gives full control to users at a very low-level to create graphics applications in a more handmade way.

### What development tools are required for raylib?

To develop raylib programs you only need a text editor (with recommended code syntax highlighting) and a compiler.

A [raylib Windows Installer](https://raysan5.itch.io/raylib) package is distributed including the Notepad++ editor and MinGW (GCC) compiler pre-configured for Windows for new users as an starter-pack but for more advance configurations with other editors/compilers, [raylib Wiki](https://github.com/raysan5/raylib/wiki) provides plenty of configuration tutorials.

### What are raylib's external dependencies?

raylib is self-contained, it has no external dependencies to build it. But internally raylib uses several libraries from other developers, mostly used to load specific file-formats.

A detailed list of raylib dependencies could be found on the [raylib Wiki](https://github.com/raysan5/raylib/wiki/raylib-dependencies).

### Can I use raylib with other technologies or libraries?

Yes, raylib can be used with other libraries that provide specific functionality. There are multiple examples of raylib integrations with libraries like Spine, Tiled, Dear Imgui and several physics engines.

### What file formats are supported by raylib?

raylib can load data from multiple standard file formats:

 - Image/Textures: PNG, BMP, TGA, JPG, GIF, QOI, PSD, DDS, HDR, KTX, ASTC, PKM, PVR
 - Fonts: FNT (sprite font), TTF, OTF
 - Models/Meshes: OBJ, IQM, GLTF, VOX, M3D
 - Audio: WAV, OGG, MP3, FLAC, XM, MOD, QOA
 
### Does raylib support the Vulkan API?

No, raylib is built on top of OpenGL API, and there are currently no plans to support any other graphics APIs. In any case, raylib uses rlgl as an abstraction layer to support different OpenGL versions. If really required, a Vulkan backend equivalent could be added but creating that abstraction layer would imply a considerable amount of work.

### What could I expect to see in raylib in the future?

The main focus of the library is simplicity. Most of the efforts are invested in maintainability and bug-fixing. Despite new small features are regularly added, it's not the objective for raylib to become a full-featured engine. Personally I prefer to keep it small and enjoyable.

### Who are the raylib developers?

The main raylib developer and maintainer is [Ramon Santamaria](https://www.linkedin.com/in/raysan/) but there's 360+ contributors that have helped by adding new features, testing the library and solving issues in the 9+ years life of raylib.

The full list of raylib contributors can be seen [on GitHub](https://github.com/raysan5/raylib/graphs/contributors). 

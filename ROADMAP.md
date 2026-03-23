# raylib roadmap

Here is a wishlist with features and ideas to improve the library. Note that features listed here are usually long term improvements or just describe a route to follow for the library. There are also some additional places to look for raylib improvements and ideas:

 - [GitHub Issues](https://github.com/raysan5/raylib/issues) has several open issues for possible improvements or bugs to fix.
 - [GitHub PRs](https://github.com/raysan5/raylib/pulls) open with improvements to be reviewed.
 - [raylib source code](https://github.com/raysan5/raylib/tree/master/src) has multiple *TODO* comments around code with pending things to review or improve.
 - raylib wishlists discussions are open to everyone to ask for improvements, feel free to check and comment:
    - [raylib 6.0 wishlist](https://github.com/raysan5/raylib/discussions/4660)
    - [raylib 5.0 wishlist](https://github.com/raysan5/raylib/discussions/2952)
    - [raylib wishlist 2022](https://github.com/raysan5/raylib/discussions/2272)
    - [raylib wishlist 2021](https://github.com/raysan5/raylib/discussions/1502)

_Current version of raylib is complete and functional but there is always room for improvements._

**raylib 7.0**
 - [ ] `rcore_desktop_win32`: Improve new Windows platform backend - inputs, highdpi
 - [ ] `rcore_desktop_emscripten`: Improve new Web platform backend - inputs, highdpi
 - [ ] `rcore_desktop_cocoa`: Create additional platform backend: macOS
 - [ ] `rcore_desktop_x11`: Create additional platform backend: Linux/X11
 - [ ] `rcore_desktop_wayland`: Create additional platform backend: Linux/Wayland
 - [ ] `rcore`: Investigate alternative embedded platforms and realtime OSs
 - [ ] `rlsw`: Software renderer optimizations: mipmaps, platform-specific SIMD
 - [ ] `rtextures`: Consider moving N-patch system to separate example
 - [ ] `rtextures`: Review blending modes system, provide more options or better samples
 - [ ] `rtext`: Investigate the recently opened [`Slug`](https://sluglibrary.com/) font rendering algorithm
 - [ ] `raudio`: Support microphone input, basic API to read microphone
 - [ ] `rltexgpu`: Improve compressed textures support, loading and saving, improve KTX 2.0
 - [ ] `rlobj`: Create OBJ loader, supporting material file separately (low priority)

**raylib 6.0**
 - [x] `rlsw`: New Software Renderer backend, pseudo-OpenGL 1.1 implementation
 - [x] `rcore_emscripten`: New emscripten-only backend, avoiding GLFW dependency
 - [x] `rlgl`: Review GLSL shaders naming conventions for consistency, redesigned shader API
 - [x] `rmodels`: Improve 3d objects loading, specially animations (obj, gltf)
 - [x] `examples`: Review all examples, add more and better code explanations

**raylib 5.0**
 - [x] Split core module into separate platforms?
 - [x] Redesign gestures system, improve touch inputs management
 - [x] Redesign camera module (more flexible) ([#1143](https://github.com/raysan5/raylib/issues/1143), https://github.com/raysan5/raylib/discussions/2507)
 - [x] Better documentation and improved examples, reviewed webpage with examples complexity level
 - [x] Focus on HTML5 ([raylib 5k gamejam](https://itch.io/jam/raylib-5k-gamejam)) and embedded platforms (RPI and similar SOCs)
 - [x] Additional support libraries: [raygui](https://github.com/raysan5/raygui), [rres](https://github.com/raysan5/rres)

**raylib 4.0**
 - [x] Improved consistency and coherency in raylib API
 - [x] Continuous Deployment using GitHub Actions
 - [x] rlgl improvements for standalone usage (avoid raylib coupling)
 - Basic CPU/GPU stats system (memory, draws, time...) ([#1295](https://github.com/raysan5/raylib/issues/1295)) - _DISCARDED_
 - Software rendering backend (avoiding OpenGL) ([#1370](https://github.com/raysan5/raylib/issues/1370)) - _DISCARDED_
 - Network module (UDP): `rnet` ([#753](https://github.com/raysan5/raylib/issues/753)) - _DISCARDED_ - Use [nbnet](https://github.com/nathhB/nbnet).

 **raylib 3.0**
 - [x] Custom memory allocators support
 - [x] Global variables moved to global context
 - [x] Optimize data structures for pass-by-value
 - [x] Trace log messages redesign ([#1065](https://github.com/raysan5/raylib/issues/1065))
 - [x] Continuous Integration using GitHub Actions

**raylib 2.5**
 - [x] Support Animated models
 - [x] Support glTF models file format
 - [x] Unicode support on text drawing

**raylib 2.0**
 - [x] Removed external dependencies (GLFW3 and OpenAL)
 - [x] Support TCC compiler (32bit and 64bit)

**raylib 1.8**
 - [x] Improved Materials system with PBR support
 - [x] Procedural image generation functions (spot, gradient, noise...)
 - [x] Procedural mesh generation functions (cube, sphere...)
 - [x] Custom Android APK build pipeline (default Makefile)

**raylib 1.7**
 - [x] Support configuration flags
 - [x] Improved build system for Android
 - [x] Gamepad support on HTML5

**raylib 1.6**
 - [x] Lua scripting support (raylib Lua wrapper)
 - [x] Redesigned audio module
 - [x] Support FLAC file format

**raylib 1.5**
 - [x] Support Oculus Rift CV1 and VR stereo rendering (simulator)
 - [x] Redesign Shaders/Textures system -> New Materials system
 - [x] Support lighting: Omni, Directional and Spotlights
 - [x] Redesign physics module (physac)
 - [x] Chiptunes audio modules support

**raylib 1.4**
 - [x] TTF fonts support (using stb_truetype)
 - [x] Raycast system for 3D picking (including collisions detection)
 - [x] Floyd-Steinberg dithering on 16bit image format conversion
 - [x] Basic image manipulation functions (crop, resize, draw...)
 - [x] Storage load/save data functionality
 - [x] Add Physics module (physac)
 - [x] Remove GLEW dependency -> Replaced by GLAD
 - [x] Redesign Raspberry PI inputs system
 - [x] Redesign gestures module to be multiplatform
 - [x] Module raymath as header-only and functions inline
 - [x] Add Easings module (easings.h)

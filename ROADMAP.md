roadmap
-------

Here it is a wish-list with features and ideas to improve the library. Note that features listed here are quite high-level and could be long term additions for the library. Current version of raylib is complete and functional but there is a lot of room for improvement.

[raylib source code](https://github.com/raysan5/raylib/tree/master/src) has some *TODO* marks around code with pending things to review and improve. Check also [GitHub Issues](https://github.com/raysan5/raylib/issues) for further details!

**raylib 3.x**
 - [ ] Network module (UDP): `rnet` ([info](https://github.com/raysan5/raylib/issues/753))
 - [ ] Custom raylib resource packer: `rres` ([info](https://github.com/raysan5/rres))
 - [ ] Basic CPU/GPU stats system (memory, draws, time...)
 - [ ] Continuous Deployment using GitHub Actions
 
 **raylib 3.0**
 - [x] Custom memory allocators support
 - [x] Global variables moved to global context
 - [x] Optimize data structures for pass-by-value
 - [x] Trace log messages redesign ([info](https://github.com/raysan5/raylib/issues/1065))
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
 - [x] Support lighting: Omni, Directional and Spot lights
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

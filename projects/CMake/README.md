# raylib CMake Project

This provides a base project template which builds with [CMake](https://cmake.org).

## Usage

To compile the example, use one of the following depending on your build target...

### Desktop

Use the following to build for desktop:

``` bash
cmake -B build
cmake --build build
```

### Web

Compiling for the web requires the [Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html):

``` bash
mkdir build
cd build
emcmake cmake .. -DPLATFORM=Web -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXECUTABLE_SUFFIX=".html"
emmake make
```

### Apple platforms

Apple builds require macOS, Xcode with the target SDK, and CMake 3.22 or newer.
iOS, iPadOS, and tvOS use UIKit and OpenGL ES 3.0.

From the template directory, build the app for the iPhone or iPad simulator.

```sh
cmake -S . -B build-ios -G Xcode -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_SYSROOT=iphonesimulator \
    -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED=NO
cmake --build build-ios --config Debug --target example
```

The app is at `build-ios/Debug-iphonesimulator/example.app`. Open
`build-ios/example.xcodeproj` in Xcode and select a simulator to run it.

Use a separate build directory for each SDK. For the Apple TV simulator, set
`CMAKE_SYSTEM_NAME=tvOS` and `CMAKE_OSX_SYSROOT=appletvsimulator`. For a physical
device, use `iphoneos` or `appletvos`, omit the flag that disables code signing,
and set your signing team and bundle identifier in Xcode.

The template can use a raylib checkout or an installed static library for the
same SDK. Set `raylib_DIR` to the installed CMake package directory to select
that library.

Replace the template's `core_basic_window.c` with a game using `int main(void)`.
Only that file is compiled with `main=RaylibGameMain`. The launcher calls
`RaylibRunAppleApp()` on the main thread and the game uses the normal raylib
window and drawing loop on the rendering thread. Keep graphics and input calls
on that thread and check `WindowShouldClose()` each frame.

The template bundles files placed in its `resources/` directory as
`raylib_resources/`. The launcher exposes them at `resources/` relative to
the working directory. Write saves and screenshots elsewhere in the working
directory, which is Application Support on iOS and iPadOS and the system's
purgeable Caches directory on tvOS.
`InitWindow()` sets the logical canvas size. UIKit fits it to the drawable, whose
pixel dimensions are available through `GetRenderWidth()` and `GetRenderHeight()`.
iOS touch also drives the primary mouse button. On tvOS, the Siri Remote
directions, Select and Play/Pause map to arrow keys, Enter and Space. Back/Menu
follows system navigation. Extended gamepads supply axes and buttons.
Desktop window controls, clipboard, file drops, and gamepad mappings and
vibration are unavailable on the UIKit backend.

# raylib Breakout demo

This example is generated with CMake and opened in Xcode. Use the simulator or a real device by changing the Apple SDK you target.

## Requirements

- macOS with Xcode installed
- CMake 3.18 or newer
- iOS Simulator support from Xcode, or a connected iPhone/iPad for device testing
- An Apple developer signing team if you want to run on a real device
- Min. iOS API 13

## Generate the Xcode project

From this folder, run:

```sh
cmake -S . -B build -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphonesimulator
```

This generates `build/raylib_ios_example.xcodeproj`.

## Run in the iOS Simulator

1. Open `build/raylib_ios_example.xcodeproj` in Xcode.
2. Select an iPhone Simulator destination.
3. Build and run the `raylib-ios-example` scheme.

## Run on a real iPhone or iPad

1. Regenerate the project for the device SDK:

```sh
cmake -S . -B build -G Xcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_SYSROOT=iphoneos
```

2. Open the generated Xcode project.
3. Select your connected device.
4. Set a valid signing team in Xcode if needed.
5. Build and run the same `raylib-ios-example` scheme.



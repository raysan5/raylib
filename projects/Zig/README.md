# Starting your raylib project with Zig (0.16.0)

## How to compile and run it

To compile the project:

```sh
zig build
```

To run the project:

```sh
zig build run
```

## Compile with different optimization

To change from debug to release build you can do it with the `-Doptimze=` flag.

```
Debug
ReleaseSafe
ReleaseFast
ReleaseSmall
```

## Choose a different platform

To compile with a different platform you can use the `-Dplatform=` flag.
Here all the options:

```
glfw
rgfw
sdl
sdl2
sdl3
memory
win32
drm
android
```

In this example the platform `sdl` and `sdl2` are not supported

Important for the android platform you also have to compile for the right target

## Compile for a different target

To compile for a different [target](https://ziglang.org/download/0.16.0/release-notes.html#Support-Table) you can use the `-Dtarget=` flag.
Not all targets are supported

## Example: Compile for web and run it

To compile for the web we use emscripten and you run it like that:

```sh
zig build -Dtarget=wasm32-emscripten
```

To run it we do:

```sh
zig build run -Dtarget=wasm32-emscripten
```

And to make a relase build we do:

```sh
zig build -Dtarget=wasm32-emscripten -Doptimize=ReleaseFast
```

If we want to use rgfw for the web build we could do:

```sh
zig build -Dplatform=rgfw -Dtarget=wasm32-emscripten -Doptimize=ReleaseFast
```

## Compiling the Zig code? Just add `-Dzig` and try out zig ;)

## More Resources

See [Zig Build System](https://ziglang.org/learn/build-system/)

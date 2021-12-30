const std = @import("std");

pub fn build(b: *std.build.Builder) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    const raylib_flags = &[_][]const u8{
        "-std=gnu99",
        "-DPLATFORM_DESKTOP",
        "-DGL_SILENCE_DEPRECATION",
        "-fno-sanitize=undefined", // https://github.com/raysan5/raylib/issues/1891
    };

    const raylib = b.addStaticLibrary("raylib", "raylib.h");
    raylib.setTarget(target);
    raylib.setBuildMode(mode);
    raylib.linkLibC();

    raylib.addIncludeDir("external/glfw/include");

    raylib.addCSourceFiles(&.{
        "raudio.c",
        "rcore.c",
        "rmodels.c",
        "rshapes.c",
        "rtext.c",
        "rtextures.c",
        "utils.c",
    }, raylib_flags);

    switch (raylib.target.toTarget().os.tag) {
        .windows => {
            raylib.addCSourceFiles(&.{"rglfw.c"}, raylib_flags);
            raylib.linkSystemLibrary("winmm");
            raylib.linkSystemLibrary("gdi32");
            raylib.linkSystemLibrary("opengl32");
            raylib.addIncludeDir("external/glfw/deps/mingw");
        },
        .linux => {
            raylib.addCSourceFiles(&.{"rglfw.c"}, raylib_flags);
            raylib.linkSystemLibrary("GL");
            raylib.linkSystemLibrary("rt");
            raylib.linkSystemLibrary("dl");
            raylib.linkSystemLibrary("m");
            raylib.linkSystemLibrary("X11");
        },
        .macos => {
            // On macos rglfw.c include Objective-C files.
            const raylib_flags_extra_macos = &[_][]const u8{
                "-ObjC",
            };
            raylib.addCSourceFiles(
                &.{"rglfw.c"},
                raylib_flags ++ raylib_flags_extra_macos,
            );
            raylib.linkFramework("Foundation");
        },
        else => {
            @panic("Unsupported OS");
        },
    }

    raylib.setOutputDir("./");
    raylib.install();
}

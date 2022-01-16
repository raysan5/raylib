const std = @import("std");

pub fn Pkg(srcdir: []const u8) type {
    return struct {
        pub fn addRaylib(b: *std.build.Builder, target: std.zig.CrossTarget) *std.build.LibExeObjStep {
            // Standard release options allow the person running `zig build` to select
            // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
            const mode = b.standardReleaseOptions();

            const raylib_flags = &[_][]const u8{
                "-std=gnu99",
                "-DPLATFORM_DESKTOP",
                "-DGL_SILENCE_DEPRECATION=199309L",
                "-fno-sanitize=undefined", // https://github.com/raysan5/raylib/issues/1891
            };

            const raylib = b.addStaticLibrary("raylib", srcdir ++ "/raylib.h");
            raylib.setTarget(target);
            raylib.setBuildMode(mode);
            raylib.linkLibC();

            raylib.addIncludeDir(srcdir ++ "/external/glfw/include");

            raylib.addCSourceFiles(&.{
                srcdir ++ "/raudio.c",
                srcdir ++ "/rcore.c",
                srcdir ++ "/rmodels.c",
                srcdir ++ "/rshapes.c",
                srcdir ++ "/rtext.c",
                srcdir ++ "/rtextures.c",
                srcdir ++ "/utils.c",
            }, raylib_flags);

            switch (raylib.target.toTarget().os.tag) {
                .windows => {
                    raylib.addCSourceFiles(&.{srcdir ++ "/rglfw.c"}, raylib_flags);
                    raylib.linkSystemLibrary("winmm");
                    raylib.linkSystemLibrary("gdi32");
                    raylib.linkSystemLibrary("opengl32");
                    raylib.addIncludeDir("external/glfw/deps/mingw");
                },
                .linux => {
                    raylib.addCSourceFiles(&.{srcdir ++ "/rglfw.c"}, raylib_flags);
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
                        &.{srcdir ++ "/rglfw.c"},
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
            return raylib;
        }
    };
}

const lib = Pkg(".");

pub fn build(b: *std.build.Builder) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    _ = lib.addRaylib(b, target);
}

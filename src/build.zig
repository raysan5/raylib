const std = @import("std");

// This has been tested to work with zig master branch as of commit 87de821 or May 14 2023
pub fn addRaylib(b: *std.Build, compile: *std.Build.Step.Compile , options: Options) void {
    const raylib_flags = &[_][]const u8{
        "-std=gnu99",
        "-D_GNU_SOURCE",
        "-DGL_SILENCE_DEPRECATION=199309L",
        "-fno-sanitize=undefined", // https://github.com/raysan5/raylib/issues/1891
    };

    compile.linkLibC();

    compile.addIncludePath(srcdir ++ "/external/glfw/include");

    const c_source = &[_][]const u8{
        srcdir ++ "/raudio.c",
        srcdir ++ "/rcore.c",
        srcdir ++ "/rmodels.c",
        srcdir ++ "/rshapes.c",
        srcdir ++ "/rtext.c",
        srcdir ++ "/rtextures.c",
        srcdir ++ "/utils.c",
    };

    const rglfw = &[_][]const u8 {
        srcdir ++ "/rglfw.c",
    };

    var gen_step = std.build.Step.WriteFile.create(b);
    compile.step.dependOn(&gen_step.step);

    if (options.raygui) {
        _ = gen_step.add(srcdir ++ "/raygui.c", "#define RAYGUI_IMPLEMENTATION\n#include \"raygui.h\"\n");
        compile.addCSourceFile(srcdir ++ "/raygui.c", raylib_flags);
        compile.addIncludePath(srcdir);
        compile.addIncludePath(srcdir ++ "/../../raygui/src");
    }

    switch (compile.target.getOsTag()) {
        .windows => {
            compile.linkSystemLibrary("winmm");
            compile.linkSystemLibrary("gdi32");
            compile.linkSystemLibrary("opengl32");
            compile.addIncludePath("external/glfw/deps/mingw");
            compile.addCSourceFiles(
                c_source ++ rglfw,
                raylib_flags ++ &[_][]const u8{ "-DPLATFORM_DESKTOP" }
            );
        },
        .linux => {
            compile.linkSystemLibrary("GL");
            compile.linkSystemLibrary("rt");
            compile.linkSystemLibrary("dl");
            compile.linkSystemLibrary("m");
            compile.linkSystemLibrary("X11");
            compile.addIncludePath("/usr/include");
            compile.addCSourceFiles(
                c_source ++ rglfw,
                raylib_flags ++ &[_][]const u8{ "-DPLATFORM_DESKTOP" }
            );
        },
        .freebsd, .openbsd, .netbsd, .dragonfly => {
            compile.linkSystemLibrary("GL");
            compile.linkSystemLibrary("rt");
            compile.linkSystemLibrary("dl");
            compile.linkSystemLibrary("m");
            compile.linkSystemLibrary("X11");
            compile.linkSystemLibrary("Xrandr");
            compile.linkSystemLibrary("Xinerama");
            compile.linkSystemLibrary("Xi");
            compile.linkSystemLibrary("Xxf86vm");
            compile.linkSystemLibrary("Xcursor");
            compile.addCSourceFiles(
                c_source ++ rglfw,
                raylib_flags ++ &[_][]const u8{ "-DPLATFORM_DESKTOP" }
            );
        },
        .macos => {
            compile.linkFramework("Foundation");
            compile.linkFramework("CoreServices");
            compile.linkFramework("CoreGraphics");
            compile.linkFramework("AppKit");
            compile.linkFramework("IOKit");
            // On macos rglfw.c include Objective-C files.
            const raylib_flags_extra_macos = &[_][]const u8{
                "-ObjC",
            };
            compile.addCSourceFiles(
                rglfw,
                raylib_flags ++ raylib_flags_extra_macos,
            );
            compile.addCSourceFiles(
                c_source,
                raylib_flags ++ &[_][]const u8{ "-DPLATFORM_DESKTOP" }
            );
        },
        .emscripten => {
            compile.addCSourceFiles(
                c_source,
                raylib_flags ++ &[_][]const u8{ "PLATFORM_WEB", "GRAPHICS_API_OPENGL_ES2" }
            );

            if (b.sysroot == null) {
                @panic("Pass '--sysroot \"$EMSDK/upstream/emscripten\"'");
            }

            const cache_include = std.fs.path.join(b.allocator, &.{ b.sysroot.?, "cache", "sysroot", "include" }) catch @panic("Out of memory");
            defer b.allocator.free(cache_include);

            var dir = std.fs.openDirAbsolute(cache_include, std.fs.Dir.OpenDirOptions{ .access_sub_paths = true, .no_follow = true }) catch @panic("No emscripten cache. Generate it!");
            dir.close();

            compile.addIncludePath(cache_include);
        },
        else => {
            @panic("Unsupported OS");
        },
    }
}

const Options = struct {
    raygui: bool = false,
};

pub fn build(b: *std.Build) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});
    // Standard optimization options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall. Here we do not
    // set a preferred release mode, allowing the user to decide how to optimize.
    const optimize = b.standardOptimizeOption(.{});

    const raygui = b.option(bool, "raygui", "Compile with raygui support");

    const raylib = b.addStaticLibrary(.{
        .name = "raylib",
        .target = target,
        .optimize = optimize,
    });

    addRaylib(b, raylib, .{
        .raygui = raygui orelse false,
    });

    raylib.installHeader("src/raylib.h", "raylib.h");
    raylib.installHeader("src/raymath.h", "raymath.h");
    raylib.installHeader("src/rlgl.h", "rlgl.h");

    if (raygui orelse false) {
        raylib.installHeader("../raygui/src/raygui.h", "raygui.h");
    }

    b.installArtifact(raylib);
}

const srcdir = struct {
    fn getSrcDir() []const u8 {
        return std.fs.path.dirname(@src().file).?;
    }
}.getSrcDir();

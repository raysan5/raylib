const std = @import("std");

// This has been tested to work with zig master branch as of commit 87de821 or May 14 2023
pub fn addRaylib(b: *std.Build, target: std.zig.CrossTarget, optimize: std.builtin.OptimizeMode, options: Options) *std.Build.CompileStep {
    const raylib_flags = &[_][]const u8{
        "-std=gnu99",
        "-D_GNU_SOURCE",
        "-DGL_SILENCE_DEPRECATION=199309L",
    };

    const raylib = b.addStaticLibrary(.{
        .name = "raylib",
        .target = target,
        .optimize = optimize,
    });
    raylib.linkLibC();

    // No GLFW required on PLATFORM_DRM
    if (!options.platform_drm) {
        raylib.addIncludePath(.{ .path = srcdir ++ "/external/glfw/include" });
    }

    raylib.addCSourceFiles(&.{
        srcdir ++ "/rcore.c",
        srcdir ++ "/utils.c",
    }, raylib_flags);

    if (options.raudio) {
        raylib.addCSourceFiles(&.{
            srcdir ++ "/raudio.c",
        }, raylib_flags);
    }
    if (options.rmodels) {
        raylib.addCSourceFiles(&.{
            srcdir ++ "/rmodels.c",
        }, &[_][]const u8{
            "-fno-sanitize=undefined", // https://github.com/raysan5/raylib/issues/1891
        } ++ raylib_flags);
    }
    if (options.rshapes) {
        raylib.addCSourceFiles(&.{
            srcdir ++ "/rshapes.c",
        }, raylib_flags);
    }
    if (options.rtext) {
        raylib.addCSourceFiles(&.{
            srcdir ++ "/rtext.c",
        }, raylib_flags);
    }
    if (options.rtextures) {
        raylib.addCSourceFiles(&.{
            srcdir ++ "/rtextures.c",
        }, raylib_flags);
    }

    var gen_step = std.build.Step.WriteFile.create(b);
    raylib.step.dependOn(&gen_step.step);

    if (options.raygui) {
        _ = gen_step.add(srcdir ++ "/raygui.c", "#define RAYGUI_IMPLEMENTATION\n#include \"raygui.h\"\n");
        raylib.addCSourceFile(.{ .file = .{ .path = srcdir ++ "/raygui.c" }, .flags = raylib_flags });
        raylib.addIncludePath(.{ .path = srcdir });
        raylib.addIncludePath(.{ .path = srcdir ++ "/../../raygui/src" });
    }

    switch (target.getOsTag()) {
        .windows => {
            raylib.addCSourceFiles(&.{srcdir ++ "/rglfw.c"}, raylib_flags);
            raylib.linkSystemLibrary("winmm");
            raylib.linkSystemLibrary("gdi32");
            raylib.linkSystemLibrary("opengl32");
            raylib.addIncludePath(.{ .path = "external/glfw/deps/mingw" });

            raylib.defineCMacro("PLATFORM_DESKTOP", null);
        },
        .linux => {
            if (!options.platform_drm) {
                raylib.addCSourceFiles(&.{srcdir ++ "/rglfw.c"}, raylib_flags);
                raylib.linkSystemLibrary("GL");
                raylib.linkSystemLibrary("rt");
                raylib.linkSystemLibrary("dl");
                raylib.linkSystemLibrary("m");
                raylib.linkSystemLibrary("X11");
                raylib.addLibraryPath(.{ .path = "/usr/lib" });
                raylib.addIncludePath(.{ .path = "/usr/include" });

                raylib.defineCMacro("PLATFORM_DESKTOP", null);
            } else {
                raylib.linkSystemLibrary("GLESv2");
                raylib.linkSystemLibrary("EGL");
                raylib.linkSystemLibrary("drm");
                raylib.linkSystemLibrary("gbm");
                raylib.linkSystemLibrary("pthread");
                raylib.linkSystemLibrary("rt");
                raylib.linkSystemLibrary("m");
                raylib.linkSystemLibrary("dl");
                raylib.addIncludePath(.{ .path = "/usr/include/libdrm" });

                raylib.defineCMacro("PLATFORM_DRM", null);
                raylib.defineCMacro("GRAPHICS_API_OPENGL_ES2", null);
                raylib.defineCMacro("EGL_NO_X11", null);
                raylib.defineCMacro("DEFAULT_BATCH_BUFFER_ELEMENT", "2048");
            }
        },
        .freebsd, .openbsd, .netbsd, .dragonfly => {
            raylib.addCSourceFiles(&.{srcdir ++ "/rglfw.c"}, raylib_flags);
            raylib.linkSystemLibrary("GL");
            raylib.linkSystemLibrary("rt");
            raylib.linkSystemLibrary("dl");
            raylib.linkSystemLibrary("m");
            raylib.linkSystemLibrary("X11");
            raylib.linkSystemLibrary("Xrandr");
            raylib.linkSystemLibrary("Xinerama");
            raylib.linkSystemLibrary("Xi");
            raylib.linkSystemLibrary("Xxf86vm");
            raylib.linkSystemLibrary("Xcursor");

            raylib.defineCMacro("PLATFORM_DESKTOP", null);
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
            raylib.linkFramework("CoreServices");
            raylib.linkFramework("CoreGraphics");
            raylib.linkFramework("AppKit");
            raylib.linkFramework("IOKit");

            raylib.defineCMacro("PLATFORM_DESKTOP", null);
        },
        .emscripten => {
            raylib.defineCMacro("PLATFORM_WEB", null);
            raylib.defineCMacro("GRAPHICS_API_OPENGL_ES2", null);

            if (b.sysroot == null) {
                @panic("Pass '--sysroot \"$EMSDK/upstream/emscripten\"'");
            }

            const cache_include = std.fs.path.join(b.allocator, &.{ b.sysroot.?, "cache", "sysroot", "include" }) catch @panic("Out of memory");
            defer b.allocator.free(cache_include);

            var dir = std.fs.openDirAbsolute(cache_include, std.fs.Dir.OpenDirOptions{ .access_sub_paths = true, .no_follow = true }) catch @panic("No emscripten cache. Generate it!");
            dir.close();

            raylib.addIncludePath(.{ .path = cache_include });
        },
        else => {
            @panic("Unsupported OS");
        },
    }

    return raylib;
}

pub const Options = struct {
    raudio: bool = true,
    rmodels: bool = true,
    rshapes: bool = true,
    rtext: bool = true,
    rtextures: bool = true,
    raygui: bool = false,
    platform_drm: bool = false,
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

    const defaults = Options{};
    const options = Options{
        .platform_drm = b.option(bool, "platform_drm", "Compile raylib in native mode (no X11)") orelse defaults.platform_drm,
        .raudio = b.option(bool, "raudio", "Compile with audio support") orelse defaults.raudio,
        .rmodels = b.option(bool, "rmodels", "Compile with models support") orelse defaults.rmodels,
        .rtext = b.option(bool, "rtext", "Compile with text support") orelse defaults.rtext,
        .rtextures = b.option(bool, "rtextures", "Compile with textures support") orelse defaults.rtextures,
        .rshapes = b.option(bool, "rshapes", "Compile with shapes support") orelse defaults.rshapes,
        .raygui = b.option(bool, "raygui", "Compile with raygui support") orelse defaults.raygui,
    };

    const lib = addRaylib(b, target, optimize, options);

    lib.installHeader("src/raylib.h", "raylib.h");
    lib.installHeader("src/raymath.h", "raymath.h");
    lib.installHeader("src/rlgl.h", "rlgl.h");

    if (options.raygui) {
        lib.installHeader("../raygui/src/raygui.h", "raygui.h");
    }

    b.installArtifact(lib);
}

const srcdir = struct {
    fn getSrcDir() []const u8 {
        return std.fs.path.dirname(@src().file).?;
    }
}.getSrcDir();

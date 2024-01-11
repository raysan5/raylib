const std = @import("std");
const builtin = @import("builtin");

// This has been tested to work with zig 0.11.0 and zig 0.12.0-dev.2075+f5978181e
//
// anytype is used here to preserve compatibility, in 0.12.0dev the std.zig.CrossTarget type
// was reworked into std.Target.Query and std.Build.ResolvedTarget. Using anytype allows
// us to accept both CrossTarget and ResolvedTarget and act accordingly in getOsTagVersioned.
pub fn addRaylib(b: *std.Build, target: anytype, optimize: std.builtin.OptimizeMode, options: Options) !*std.Build.Step.Compile {
    if (comptime builtin.zig_version.minor >= 12 and @TypeOf(target) != std.Build.ResolvedTarget) {
        @compileError("Expected 'std.Build.ResolvedTarget' for argument 2 'target' in 'addRaylib', found '" ++ @typeName(@TypeOf(target)) ++ "'");
    } else if (comptime builtin.zig_version.minor == 11 and @TypeOf(target) != std.zig.CrossTarget) {
        @compileError("Expected 'std.zig.CrossTarget' for argument 2 'target' in 'addRaylib', found '" ++ @typeName(@TypeOf(target)) ++ "'");
    }

    const shared_flags = &[_][]const u8{
        "-fPIC",
        "-DBUILD_LIBTYPE_SHARED",
    };
    var raylib_flags_arr = std.ArrayList([]const u8).init(std.heap.page_allocator);
    defer raylib_flags_arr.deinit();
    try raylib_flags_arr.appendSlice(&[_][]const u8{
        "-std=gnu99",
        "-D_GNU_SOURCE",
        "-DGL_SILENCE_DEPRECATION=199309L",
        "-fno-sanitize=undefined", // https://github.com/raysan5/raylib/issues/3674
    });
    if (options.shared) {
        try raylib_flags_arr.appendSlice(shared_flags);
    }

    const raylib = if (options.shared)
        b.addSharedLibrary(.{
            .name = "raylib",
            .target = target,
            .optimize = optimize,
        })
    else
        b.addStaticLibrary(.{
            .name = "raylib",
            .target = target,
            .optimize = optimize,
        });
    raylib.linkLibC();

    // No GLFW required on PLATFORM_DRM
    if (!options.platform_drm) {
        raylib.addIncludePath(.{ .path = srcdir ++ "/external/glfw/include" });
    }

    addCSourceFilesVersioned(raylib, &.{
        srcdir ++ "/rcore.c",
        srcdir ++ "/utils.c",
    }, raylib_flags_arr.items);

    if (options.raudio) {
        addCSourceFilesVersioned(raylib, &.{
            srcdir ++ "/raudio.c",
        }, raylib_flags_arr.items);
    }
    if (options.rmodels) {
        addCSourceFilesVersioned(raylib, &.{
            srcdir ++ "/rmodels.c",
        }, raylib_flags_arr.items);
    }
    if (options.rshapes) {
        addCSourceFilesVersioned(raylib, &.{
            srcdir ++ "/rshapes.c",
        }, raylib_flags_arr.items);
    }
    if (options.rtext) {
        addCSourceFilesVersioned(raylib, &.{
            srcdir ++ "/rtext.c",
        }, raylib_flags_arr.items);
    }
    if (options.rtextures) {
        addCSourceFilesVersioned(raylib, &.{
            srcdir ++ "/rtextures.c",
        }, raylib_flags_arr.items);
    }

    var gen_step = b.addWriteFiles();
    raylib.step.dependOn(&gen_step.step);

    if (options.raygui) {
        const raygui_c_path = gen_step.add("raygui.c", "#define RAYGUI_IMPLEMENTATION\n#include \"raygui.h\"\n");
        raylib.addCSourceFile(.{ .file = raygui_c_path, .flags = raylib_flags_arr.items });
        raylib.addIncludePath(.{ .path = srcdir });
        raylib.addIncludePath(.{ .path = srcdir ++ "/../../raygui/src" });
    }

    switch (getOsTagVersioned(target)) {
        .windows => {
            addCSourceFilesVersioned(raylib, &.{
                srcdir ++ "/rglfw.c",
            }, raylib_flags_arr.items);
            raylib.linkSystemLibrary("winmm");
            raylib.linkSystemLibrary("gdi32");
            raylib.linkSystemLibrary("opengl32");
            raylib.addIncludePath(.{ .path = "external/glfw/deps/mingw" });

            raylib.defineCMacro("PLATFORM_DESKTOP", null);
        },
        .linux => {
            if (!options.platform_drm) {
                addCSourceFilesVersioned(raylib, &.{
                    srcdir ++ "/rglfw.c",
                }, raylib_flags_arr.items);
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
            addCSourceFilesVersioned(raylib, &.{
                srcdir ++ "/rglfw.c",
            }, raylib_flags_arr.items);
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
            try raylib_flags_arr.append("-ObjC");
            addCSourceFilesVersioned(raylib, &.{
                srcdir ++ "/rglfw.c",
            }, raylib_flags_arr.items);
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
    shared: bool = false,
};

pub fn build(b: *std.Build) !void {
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
        .shared = b.option(bool, "shared", "Compile as shared library") orelse defaults.shared,
    };

    const lib = try addRaylib(b, target, optimize, options);

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

fn getOsTagVersioned(target: anytype) std.Target.Os.Tag {
    if (comptime builtin.zig_version.minor >= 12) {
        return target.result.os.tag;
    } else {
        return target.getOsTag();
    }
}

fn addCSourceFilesVersioned(exe: *std.Build.Step.Compile, files: []const []const u8, flags: []const []const u8) void {
    if (comptime builtin.zig_version.minor >= 12) {
        exe.addCSourceFiles(.{
            .files = files,
            .flags = flags,
        });
    } else {
        exe.addCSourceFiles(files, flags);
    }
}

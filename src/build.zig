const std = @import("std");
const builtin = @import("builtin");

// This has been tested to work with zig 0.11.0 and zig 0.12.0-dev.1834+f36ac227b
pub fn addRaylib(
    b: *std.Build,
    target: std.zig.CrossTarget,
    optimize: std.builtin.OptimizeMode,
    options: Options,
) *std.Build.CompileStep {
    return addStaticRaylib(b, target, optimize, options);
}

pub fn addStaticRaylib(
    b: *std.Build,
    target: std.zig.CrossTarget,
    optimize: std.builtin.OptimizeMode,
    options: Options,
) *std.Build.CompileStep {
    const lib = b.addStaticLibrary(.{
        .name = "raylib",
        .target = target,
        .optimize = optimize,
    });
    if (addRaylibTo(b, lib, target, options)) {} else |err| switch (err) {
        error.OutOfMemory => {
            std.debug.print("Not enough memory!", .{});
            std.os.exit(1);
        },
    }
    return lib;
}

pub fn addSharedRaylib(
    b: *std.Build,
    target: std.zig.CrossTarget,
    optimize: std.builtin.OptimizeMode,
    options: Options,
) *std.Build.CompileStep {
    const lib = b.addSharedLibrary(.{
        .name = "raylib",
        .target = target,
        .optimize = optimize,
    });
    lib.defineCMacro("BUILD_LIBTYPE_SHARED", null);
    if (addRaylibTo(b, lib, target, options)) {} else |err| switch (err) {
        error.OutOfMemory => {
            std.debug.print("Not enough memory!", .{});
            std.os.exit(1);
        },
    }
    return lib;
}

/// Generic function for an either static, shared or object file that is passed to the `lib`
/// variable, see `addStaticRaylib` for an example.
fn addRaylibTo(
    b: *std.Build,
    lib: *std.Build.CompileStep,
    target: std.zig.CrossTarget,
    options: Options,
) !void {
    var arena = std.heap.ArenaAllocator.init(std.heap.page_allocator);
    defer arena.deinit();
    const allocator = arena.allocator();
    const common_flags = &[_][]const u8{
        "-std=gnu99",
        "-D_GNU_SOURCE",
        "-DGL_SILENCE_DEPRECATION=199309L",
    };
    var raylib_flags = std.ArrayList([]const u8).init(allocator);
    try raylib_flags.appendSlice(common_flags);
    if (lib.linkage == .dynamic) {
        switch (target.getOsTag()) {
            .linux, .freebsd, .openbsd, .netbsd, .dragonfly, .macos => {
                try raylib_flags.append("-fvisibility=hidden");
            },
            else => {},
        }
    }
    var raylib_flags_no_sanitize = try raylib_flags.clone();
    try raylib_flags_no_sanitize.append("-fno-sanitize=undefined");

    lib.linkLibC();

    // No GLFW required on PLATFORM_DRM
    if (!options.platform_drm) {
        lib.addIncludePath(.{ .path = srcdir ++ "/external/glfw/include" });
    }

    addCSourceFilesVersioned(lib, &.{
        srcdir ++ "/rcore.c",
        srcdir ++ "/utils.c",
    }, raylib_flags.items);

    if (options.raudio) {
        addCSourceFilesVersioned(lib, &.{
            srcdir ++ "/raudio.c",
        }, raylib_flags_no_sanitize.items); // https://github.com/raysan5/raylib/issues/3674
    }
    if (options.rmodels) {
        addCSourceFilesVersioned(lib, &.{
            srcdir ++ "/rmodels.c",
        }, raylib_flags_no_sanitize.items); // https://github.com/raysan5/raylib/issues/1891
    }
    if (options.rshapes) {
        addCSourceFilesVersioned(lib, &.{
            srcdir ++ "/rshapes.c",
        }, raylib_flags.items);
    }
    if (options.rtext) {
        addCSourceFilesVersioned(lib, &.{
            srcdir ++ "/rtext.c",
        }, raylib_flags.items);
    }
    if (options.rtextures) {
        addCSourceFilesVersioned(lib, &.{
            srcdir ++ "/rtextures.c",
        }, raylib_flags_no_sanitize.items); // https://github.com/raysan5/raylib/issues/3674
    }

    var gen_step = b.addWriteFiles();
    lib.step.dependOn(&gen_step.step);

    if (options.raygui) {
        const raygui_c_path = gen_step.add("raygui.c", "#define RAYGUI_IMPLEMENTATION\n#include \"raygui.h\"\n");
        lib.addCSourceFile(.{ .file = raygui_c_path, .flags = raylib_flags.items });
        lib.addIncludePath(.{ .path = srcdir });
        lib.addIncludePath(.{ .path = srcdir ++ "/../../raygui/src" });
    }

    switch (target.getOsTag()) {
        .windows => {
            addCSourceFilesVersioned(lib, &.{
                srcdir ++ "/rglfw.c",
            }, raylib_flags.items);
            lib.linkSystemLibrary("winmm");
            lib.linkSystemLibrary("gdi32");
            lib.linkSystemLibrary("opengl32");
            lib.addIncludePath(.{ .path = "external/glfw/deps/mingw" });

            lib.defineCMacro("PLATFORM_DESKTOP", null);
        },
        .linux => {
            if (!options.platform_drm) {
                addCSourceFilesVersioned(lib, &.{
                    srcdir ++ "/rglfw.c",
                }, raylib_flags.items);
                lib.linkSystemLibrary("GL");
                lib.linkSystemLibrary("rt");
                lib.linkSystemLibrary("dl");
                lib.linkSystemLibrary("m");
                lib.linkSystemLibrary("X11");
                lib.addLibraryPath(.{ .path = "/usr/lib" });
                lib.addIncludePath(.{ .path = "/usr/include" });

                lib.defineCMacro("PLATFORM_DESKTOP", null);
            } else {
                lib.linkSystemLibrary("GLESv2");
                lib.linkSystemLibrary("EGL");
                lib.linkSystemLibrary("drm");
                lib.linkSystemLibrary("gbm");
                lib.linkSystemLibrary("pthread");
                lib.linkSystemLibrary("rt");
                lib.linkSystemLibrary("m");
                lib.linkSystemLibrary("dl");
                lib.addIncludePath(.{ .path = "/usr/include/libdrm" });

                lib.defineCMacro("PLATFORM_DRM", null);
                lib.defineCMacro("GRAPHICS_API_OPENGL_ES2", null);
                lib.defineCMacro("EGL_NO_X11", null);
                lib.defineCMacro("DEFAULT_BATCH_BUFFER_ELEMENT", "2048");
            }
        },
        .freebsd, .openbsd, .netbsd, .dragonfly => {
            addCSourceFilesVersioned(lib, &.{
                srcdir ++ "/rglfw.c",
            }, raylib_flags.items);
            lib.linkSystemLibrary("GL");
            lib.linkSystemLibrary("rt");
            lib.linkSystemLibrary("dl");
            lib.linkSystemLibrary("m");
            lib.linkSystemLibrary("X11");
            lib.linkSystemLibrary("Xrandr");
            lib.linkSystemLibrary("Xinerama");
            lib.linkSystemLibrary("Xi");
            lib.linkSystemLibrary("Xxf86vm");
            lib.linkSystemLibrary("Xcursor");

            lib.defineCMacro("PLATFORM_DESKTOP", null);
        },
        .macos => {
            var new_flags = try raylib_flags.clone();
            // On macos rglfw.c include Objective-C files.
            const raylib_flags_extra_macos = &[_][]const u8{
                "-ObjC",
            };
            try new_flags.appendSlice(raylib_flags_extra_macos);
            addCSourceFilesVersioned(lib, &.{
                srcdir ++ "/rglfw.c",
            }, new_flags.items);
            lib.linkFramework("Foundation");
            lib.linkFramework("CoreServices");
            lib.linkFramework("CoreGraphics");
            lib.linkFramework("AppKit");
            lib.linkFramework("IOKit");

            lib.defineCMacro("PLATFORM_DESKTOP", null);
        },
        .emscripten => {
            lib.defineCMacro("PLATFORM_WEB", null);
            lib.defineCMacro("GRAPHICS_API_OPENGL_ES2", null);

            if (b.sysroot == null) {
                @panic("Pass '--sysroot \"$EMSDK/upstream/emscripten\"'");
            }

            const cache_include = std.fs.path.join(b.allocator, &.{ b.sysroot.?, "cache", "sysroot", "include" }) catch @panic("Out of memory");
            defer b.allocator.free(cache_include);

            var dir = std.fs.openDirAbsolute(cache_include, std.fs.Dir.OpenDirOptions{ .access_sub_paths = true, .no_follow = true }) catch @panic("No emscripten cache. Generate it!");
            dir.close();

            lib.addIncludePath(.{ .path = cache_include });
        },
        else => {
            @panic("Unsupported OS");
        },
    }
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

    const static_lib = addStaticRaylib(b, target, optimize, options);
    const shared_lib = addSharedRaylib(b, target, optimize, options);

    inline for (&[_]*std.Build.CompileStep{ static_lib, shared_lib }) |lib| {
        lib.installHeader("src/raylib.h", "raylib.h");
        lib.installHeader("src/raymath.h", "raymath.h");
        lib.installHeader("src/rlgl.h", "rlgl.h");

        if (options.raygui) {
            lib.installHeader("../raygui/src/raygui.h", "raygui.h");
        }

        b.installArtifact(lib);
    }
}

const srcdir = struct {
    fn getSrcDir() []const u8 {
        return std.fs.path.dirname(@src().file).?;
    }
}.getSrcDir();

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

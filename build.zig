const std = @import("std");

// This has been tested to work with zig 0.11.0.
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

    const build_examples = b.option(bool, "examples", "Build all examples") orelse false;
    const platform_drm = b.option(bool, "platform_drm", "Compile raylib in native mode (no X11)") orelse false;

    const raygui_path: ?[]const u8 = blk: {
        const default_path = b.pathFromRoot(b.pathJoin(&.{ "..", "raygui", "src", "raygui.h" }));
        const raygui_option = b.option([]const u8, "raygui", b.fmt("Compile with raygui support. Pass the path to raygui.h here (can be a relative path). When empty, will use '{s}'.", .{default_path}));
        if (raygui_option) |option_path| {
            if (std.mem.trimRight(u8, option_path, " ").len == 0) {
                break :blk default_path;
            }
            break :blk option_path;
        }
        break :blk null;
    };

    const raylib = addRaylib(b, target, optimize, .{
        .raygui_path = raygui_path,
        .platform_drm = platform_drm,
    });

    raylib.installHeader("src/raylib.h", "raylib.h");
    raylib.installHeader("src/raymath.h", "raymath.h");
    raylib.installHeader("src/rlgl.h", "rlgl.h");

    if (raygui_path) |raygui_h| {
        raylib.installHeader(raygui_h, "raygui.h");
    }

    b.installArtifact(raylib);

    if (build_examples) {
        var all = b.getInstallStep();
        const example_categories = &.{
            "audio",
            "core",
            "models",
            "others",
            "shaders",
            "shapes",
            "text",
            "textures",
        };
        inline for (example_categories) |example_category| {
            all.dependOn(addExample(b, target, optimize, raylib, example_category));
        }
    }
}

pub const Options = struct {
    // Path to `raygui.h`. Can be relative.
    raygui_path: ?[]const u8 = null,
    platform_drm: bool = false,
};

pub fn addRaylib(b: *std.Build, target: std.zig.CrossTarget, optimize: std.builtin.OptimizeMode, options: Options) *std.Build.CompileStep {
    const raylib_flags = &[_][]const u8{
        "-std=gnu99",
        "-D_GNU_SOURCE",
        "-DGL_SILENCE_DEPRECATION=199309L",
        "-fno-sanitize=undefined", // https://github.com/raysan5/raylib/issues/1891
    };

    const raylib = b.addStaticLibrary(.{
        .name = "raylib",
        .target = target,
        .optimize = optimize,
    });
    raylib.linkLibC();

    // No GLFW required on PLATFORM_DRM
    if (!options.platform_drm) {
        raylib.addIncludePath(.{ .path = "src/external/glfw/include" });
    }

    raylib.addCSourceFiles(&.{
        "src/raudio.c",
        "src/rcore.c",
        "src/rmodels.c",
        "src/rshapes.c",
        "src/rtext.c",
        "src/rtextures.c",
        "src/utils.c",
    }, raylib_flags);

    var gen_step = std.build.Step.WriteFile.create(b);
    raylib.step.dependOn(&gen_step.step);

    if (options.raygui_path) |raygui_path| {
        const generated_c_file = gen_step.add("raygui.c", "#define RAYGUI_IMPLEMENTATION\n#include \"raygui.h\"\n");
        raylib.addCSourceFile(.{ .file = generated_c_file, .flags = raylib_flags });
        raylib.addIncludePath(.{ .path = "src" });
        raylib.addIncludePath(.{ .path = std.fs.path.dirname(raygui_path) orelse "." });
    }

    switch (target.getOsTag()) {
        .windows => {
            raylib.addCSourceFiles(&.{"src/rglfw.c"}, raylib_flags);
            raylib.linkSystemLibrary("winmm");
            raylib.linkSystemLibrary("gdi32");
            raylib.linkSystemLibrary("opengl32");
            raylib.addIncludePath(.{ .path = "external/glfw/deps/mingw" });

            raylib.defineCMacro("PLATFORM_DESKTOP", null);
        },
        .linux => {
            if (!options.platform_drm) {
                raylib.addCSourceFiles(&.{"src/rglfw.c"}, raylib_flags);
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
            raylib.addCSourceFiles(&.{"src/rglfw.c"}, raylib_flags);
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
            raylib.addCSourceFiles(&.{"src/rglfw.c"}, raylib_flags ++ raylib_flags_extra_macos);
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

fn addExample(b: *std.Build, target: std.zig.CrossTarget, optimize: std.builtin.OptimizeMode, raylib: *std.Build.CompileStep, comptime example_category: []const u8) *std.Build.Step {
    if (target.getOsTag() == .emscripten) {
        @panic("Building examples for Emscripten Zig unsupported");
    }

    const category_dir = b.pathFromRoot(b.pathJoin(&.{ "examples", example_category }));

    const all = b.step(example_category, "All " ++ example_category ++ " examples");
    const dir = std.fs.cwd().openIterableDir(category_dir, .{}) catch |err| {
        std.log.err("Failed to iterate directory '{s}': {}", .{ category_dir, err });
        unreachable;
    };
    var iter = dir.iterate();
    while (iter.next() catch unreachable) |entry| {
        if (entry.kind != .file) continue;
        const extension_idx = std.mem.lastIndexOf(u8, entry.name, ".c") orelse continue;
        const name = entry.name[0..extension_idx];
        const path = b.pathJoin(&.{ category_dir, entry.name });

        // zig's mingw headers do not include pthread.h
        if (std.mem.eql(u8, "core_loading_thread", name) and target.getOsTag() == .windows) continue;

        const exe = b.addExecutable(.{
            .name = name,
            .target = target,
            .optimize = optimize,
        });
        exe.addCSourceFile(.{
            .file = .{
                .path = path,
            },
            .flags = &.{},
        });
        exe.linkLibC();
        exe.linkLibrary(raylib);

        exe.addIncludePath(.{ .path = "src" });
        exe.addIncludePath(.{ .path = "src/external" });
        exe.addIncludePath(.{ .path = "src/external/glfw/include" });

        switch (target.getOsTag()) {
            .windows => {
                exe.linkSystemLibrary("winmm");
                exe.linkSystemLibrary("gdi32");
                exe.linkSystemLibrary("opengl32");
                exe.addIncludePath(.{ .path = "external/glfw/deps/mingw" });

                exe.defineCMacro("PLATFORM_DESKTOP", null);
            },
            .linux => {
                exe.linkSystemLibrary("GL");
                exe.linkSystemLibrary("rt");
                exe.linkSystemLibrary("dl");
                exe.linkSystemLibrary("m");
                exe.linkSystemLibrary("X11");

                exe.defineCMacro("PLATFORM_DESKTOP", null);
            },
            .macos => {
                exe.linkFramework("Foundation");
                exe.linkFramework("Cocoa");
                exe.linkFramework("OpenGL");
                exe.linkFramework("CoreAudio");
                exe.linkFramework("CoreVideo");
                exe.linkFramework("IOKit");

                exe.defineCMacro("PLATFORM_DESKTOP", null);
            },
            else => {
                @panic("Unsupported OS");
            },
        }

        b.installArtifact(exe);
        var run_cmd = b.addRunArtifact(exe);
        run_cmd.cwd = category_dir;
        if (b.args) |args| {
            run_cmd.addArgs(args);
        }

        var run_step = b.step(name, b.fmt("Run example '{s}'", .{name}));
        run_step.dependOn(&run_cmd.step);
        all.dependOn(&exe.step);
    }
    return all;
}

const std = @import("std");
const builtin = @import("builtin");

comptime {
    if (builtin.zig_version.minor < 12) @compileError("Raylib requires zig version 0.12.0");
}

// NOTE(freakmangd): I don't like using a global here, but it prevents having to
// get the flags a second time when adding raygui
var raylib_flags_arr: std.ArrayListUnmanaged([]const u8) = .{};

/// we're not inside the actual build script recognized by the
/// zig build system; use this type where one would otherwise
/// use `@This()` when inside the actual entrypoint file.
const BuildScript = @import("../build.zig");

// This has been tested with zig version 0.12.0
pub fn addRaylib(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode, options: Options) !*std.Build.Step.Compile {
    const raylib_dep = b.dependencyFromBuildZig(BuildScript, .{
        .target = target,
        .optimize = optimize,
        .raudio = options.raudio,
        .rmodels = options.rmodels,
        .rshapes = options.rshapes,
        .rtext = options.rtext,
        .rtextures = options.rtextures,
        .platform = options.platform,
        .shared = options.shared,
        .linux_display_backend = options.linux_display_backend,
        .opengl_version = options.opengl_version,
        .config = options.config,
    });
    const raylib = raylib_dep.artifact("raylib");

    if (options.raygui) {
        const raygui_dep = b.dependency(options.raygui_dependency_name, .{});
        addRaygui(b, raylib, raygui_dep);
    }

    return raylib;
}

fn setDesktopPlatform(raylib: *std.Build.Step.Compile, platform: PlatformBackend) void {
    raylib.defineCMacro("PLATFORM_DESKTOP", null);

    switch (platform) {
        .glfw => raylib.defineCMacro("PLATFORM_DESKTOP_GLFW", null),
        .rgfw => raylib.defineCMacro("PLATFORM_DESKTOP_RGFW", null),
        .sdl => raylib.defineCMacro("PLATFORM_DESKTOP_SDL", null),
        else => {},
    }
}

fn srcDir() []const u8 {
    return std.fs.path.dirname(@src().file) orelse ".";
}

fn compileRaylib(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode, options: Options) !*std.Build.Step.Compile {
    raylib_flags_arr.clearRetainingCapacity();

    const shared_flags = &[_][]const u8{
        "-fPIC",
        "-DBUILD_LIBTYPE_SHARED",
    };
    try raylib_flags_arr.appendSlice(b.allocator, &[_][]const u8{
        "-std=gnu99",
        "-D_GNU_SOURCE",
        "-DGL_SILENCE_DEPRECATION=199309L",
        "-fno-sanitize=undefined", // https://github.com/raysan5/raylib/issues/3674
    });
    if (options.config) |config| {
        const file = b.pathJoin(&.{ srcDir(), "config.h" });
        const content = try std.fs.cwd().readFileAlloc(b.allocator, file, std.math.maxInt(usize));
        defer b.allocator.free(content);

        var lines = std.mem.splitScalar(u8, content, '\n');
        while (lines.next()) |line| {
            if (!std.mem.containsAtLeast(u8, line, 1, "SUPPORT")) continue;
            if (std.mem.startsWith(u8, line, "//")) continue;
            if (std.mem.startsWith(u8, line, "#if")) continue;

            var flag = std.mem.trimLeft(u8, line, " \t"); // Trim whitespace
            flag = flag["#define ".len - 1 ..]; // Remove #define
            flag = std.mem.trimLeft(u8, flag, " \t"); // Trim whitespace
            flag = flag[0 .. std.mem.indexOf(u8, flag, " ") orelse continue]; // Flag is only one word, so capture till space
            flag = try std.fmt.allocPrint(b.allocator, "-D{s}", .{flag}); // Prepend with -D

            // If user specifies the flag skip it
            if (std.mem.containsAtLeast(u8, config, 1, flag)) continue;

            // Append default value from config.h to compile flags
            try raylib_flags_arr.append(b.allocator, flag);
        }

        // Append config flags supplied by user to compile flags
        try raylib_flags_arr.append(b.allocator, config);

        try raylib_flags_arr.append(b.allocator, "-DEXTERNAL_CONFIG_FLAGS");
    }

    if (options.shared) {
        try raylib_flags_arr.appendSlice(b.allocator, shared_flags);
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
    if (options.platform != .drm) {
        raylib.addIncludePath(b.path(b.pathJoin(&.{ srcDir(), "external/glfw/include" })));
    }

    var c_source_files = try std.ArrayList([]const u8).initCapacity(b.allocator, 2);
    c_source_files.appendSliceAssumeCapacity(&.{ "rcore.c", "utils.c" });

    if (options.raudio) {
        try c_source_files.append("raudio.c");
    }
    if (options.rmodels) {
        try c_source_files.append("rmodels.c");
    }
    if (options.rshapes) {
        try c_source_files.append("rshapes.c");
    }
    if (options.rtext) {
        try c_source_files.append("rtext.c");
    }
    if (options.rtextures) {
        try c_source_files.append("rtextures.c");
    }

    if (options.opengl_version != .auto) {
        raylib.defineCMacro(options.opengl_version.toCMacroStr(), null);
    }

    switch (target.result.os.tag) {
        .windows => {
            try c_source_files.append("rglfw.c");
            raylib.linkSystemLibrary("winmm");
            raylib.linkSystemLibrary("gdi32");
            raylib.linkSystemLibrary("opengl32");

            setDesktopPlatform(raylib, options.platform);
        },
        .linux => {
            if (options.platform != .drm) {
                try c_source_files.append("rglfw.c");
                raylib.linkSystemLibrary("GL");
                raylib.linkSystemLibrary("rt");
                raylib.linkSystemLibrary("dl");
                raylib.linkSystemLibrary("m");

                raylib.addLibraryPath(.{ .cwd_relative = "/usr/lib" });
                raylib.addIncludePath(.{ .cwd_relative = "/usr/include" });
                if (options.linux_display_backend == .X11 or options.linux_display_backend == .Both) {
                    raylib.defineCMacro("_GLFW_X11", null);
                    raylib.linkSystemLibrary("X11");
                }

                if (options.linux_display_backend == .Wayland or options.linux_display_backend == .Both) {
                    _ = b.findProgram(&.{"wayland-scanner"}, &.{}) catch {
                        std.log.err(
                            \\ `wayland-scanner` may not be installed on the system.
                            \\ You can switch to X11 in your `build.zig` by changing `Options.linux_display_backend`
                        , .{});
                        @panic("`wayland-scanner` not found");
                    };
                    raylib.defineCMacro("_GLFW_WAYLAND", null);
                    raylib.linkSystemLibrary("wayland-client");
                    raylib.linkSystemLibrary("wayland-cursor");
                    raylib.linkSystemLibrary("wayland-egl");
                    raylib.linkSystemLibrary("xkbcommon");
                    waylandGenerate(b, raylib, "wayland.xml", "wayland-client-protocol");
                    waylandGenerate(b, raylib, "xdg-shell.xml", "xdg-shell-client-protocol");
                    waylandGenerate(b, raylib, "xdg-decoration-unstable-v1.xml", "xdg-decoration-unstable-v1-client-protocol");
                    waylandGenerate(b, raylib, "viewporter.xml", "viewporter-client-protocol");
                    waylandGenerate(b, raylib, "relative-pointer-unstable-v1.xml", "relative-pointer-unstable-v1-client-protocol");
                    waylandGenerate(b, raylib, "pointer-constraints-unstable-v1.xml", "pointer-constraints-unstable-v1-client-protocol");
                    waylandGenerate(b, raylib, "fractional-scale-v1.xml", "fractional-scale-v1-client-protocol");
                    waylandGenerate(b, raylib, "xdg-activation-v1.xml", "xdg-activation-v1-client-protocol");
                    waylandGenerate(b, raylib, "idle-inhibit-unstable-v1.xml", "idle-inhibit-unstable-v1-client-protocol");
                }
                setDesktopPlatform(raylib, options.platform);
            } else {
                if (options.opengl_version == .auto) {
                    raylib.linkSystemLibrary("GLESv2");
                    raylib.defineCMacro("GRAPHICS_API_OPENGL_ES2", null);
                }

                raylib.linkSystemLibrary("EGL");
                raylib.linkSystemLibrary("drm");
                raylib.linkSystemLibrary("gbm");
                raylib.linkSystemLibrary("pthread");
                raylib.linkSystemLibrary("rt");
                raylib.linkSystemLibrary("m");
                raylib.linkSystemLibrary("dl");
                raylib.addIncludePath(.{ .cwd_relative = "/usr/include/libdrm" });

                raylib.defineCMacro("PLATFORM_DRM", null);
                raylib.defineCMacro("EGL_NO_X11", null);
                raylib.defineCMacro("DEFAULT_BATCH_BUFFER_ELEMENT", "2048");
            }
        },
        .freebsd, .openbsd, .netbsd, .dragonfly => {
            try c_source_files.append("rglfw.c");
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

            setDesktopPlatform(raylib, options.platform);
        },
        .macos => {
            // On macos rglfw.c include Objective-C files.
            try raylib_flags_arr.append(b.allocator, "-ObjC");
            raylib.root_module.addCSourceFile(.{
                .file = b.path(b.pathJoin(&.{ srcDir(), "rglfw.c" })),
                .flags = raylib_flags_arr.items,
            });
            _ = raylib_flags_arr.pop();
            raylib.linkFramework("Foundation");
            raylib.linkFramework("CoreServices");
            raylib.linkFramework("CoreGraphics");
            raylib.linkFramework("AppKit");
            raylib.linkFramework("IOKit");

            setDesktopPlatform(raylib, options.platform);
        },
        .emscripten => {
            raylib.defineCMacro("PLATFORM_WEB", null);
            if (options.opengl_version == .auto) {
                raylib.defineCMacro("GRAPHICS_API_OPENGL_ES2", null);
            }

            if (b.sysroot == null) {
                @panic("Pass '--sysroot \"$EMSDK/upstream/emscripten\"'");
            }

            const cache_include = b.pathJoin(&.{ b.sysroot.?, "cache", "sysroot", "include" });

            var dir = std.fs.openDirAbsolute(cache_include, std.fs.Dir.OpenDirOptions{ .access_sub_paths = true, .no_follow = true }) catch @panic("No emscripten cache. Generate it!");
            dir.close();
            raylib.addIncludePath(.{ .cwd_relative = cache_include });
        },
        else => {
            @panic("Unsupported OS");
        },
    }

    raylib.root_module.addCSourceFiles(.{
        .root = b.path(srcDir()),
        .files = c_source_files.items,
        .flags = raylib_flags_arr.items,
    });

    return raylib;
}

/// This function does not need to be called if you passed .raygui = true to addRaylib
pub fn addRaygui(b: *std.Build, raylib: *std.Build.Step.Compile, raygui_dep: *std.Build.Dependency) void {
    if (raylib_flags_arr.items.len == 0) {
        @panic(
            \\argument 2 `raylib` in `addRaygui` must come from b.dependency("raylib", ...).artifact("raylib")
        );
    }

    var gen_step = b.addWriteFiles();
    raylib.step.dependOn(&gen_step.step);

    const raygui_c_path = gen_step.add("raygui.c", "#define RAYGUI_IMPLEMENTATION\n#include \"raygui.h\"\n");
    raylib.addCSourceFile(.{ .file = raygui_c_path, .flags = raylib_flags_arr.items });
    raylib.addIncludePath(raygui_dep.path("src"));

    raylib.installHeader(raygui_dep.path("src/raygui.h"), "raygui.h");
}

pub const Options = struct {
    raudio: bool = true,
    rmodels: bool = true,
    rshapes: bool = true,
    rtext: bool = true,
    rtextures: bool = true,
    raygui: bool = false,
    platform: PlatformBackend = .glfw,
    shared: bool = false,
    linux_display_backend: LinuxDisplayBackend = .Both,
    opengl_version: OpenglVersion = .auto,
    /// config should be a list of cflags, eg, "-DSUPPORT_CUSTOM_FRAME_CONTROL"
    config: ?[]const u8 = null,

    raygui_dependency_name: []const u8 = "raygui",
};

pub const OpenglVersion = enum {
    auto,
    gl_1_1,
    gl_2_1,
    gl_3_3,
    gl_4_3,
    gles_2,
    gles_3,

    pub fn toCMacroStr(self: @This()) []const u8 {
        switch (self) {
            .auto => @panic("OpenglVersion.auto cannot be turned into a C macro string"),
            .gl_1_1 => return "GRAPHICS_API_OPENGL_11",
            .gl_2_1 => return "GRAPHICS_API_OPENGL_21",
            .gl_3_3 => return "GRAPHICS_API_OPENGL_33",
            .gl_4_3 => return "GRAPHICS_API_OPENGL_43",
            .gles_2 => return "GRAPHICS_API_OPENGL_ES2",
            .gles_3 => return "GRAPHICS_API_OPENGL_ES3",
        }
    }
};

pub const LinuxDisplayBackend = enum {
    X11,
    Wayland,
    Both,
};

pub const PlatformBackend = enum {
    glfw,
    rgfw,
    sdl,
    drm,
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
        .platform = b.option(PlatformBackend, "platform", "Choose the platform backedn for desktop target") orelse defaults.platform,
        .raudio = b.option(bool, "raudio", "Compile with audio support") orelse defaults.raudio,
        .raygui = b.option(bool, "raygui", "Compile with raygui support") orelse defaults.raygui,
        .rmodels = b.option(bool, "rmodels", "Compile with models support") orelse defaults.rmodels,
        .rtext = b.option(bool, "rtext", "Compile with text support") orelse defaults.rtext,
        .rtextures = b.option(bool, "rtextures", "Compile with textures support") orelse defaults.rtextures,
        .rshapes = b.option(bool, "rshapes", "Compile with shapes support") orelse defaults.rshapes,
        .shared = b.option(bool, "shared", "Compile as shared library") orelse defaults.shared,
        .linux_display_backend = b.option(LinuxDisplayBackend, "linux_display_backend", "Linux display backend to use") orelse defaults.linux_display_backend,
        .opengl_version = b.option(OpenglVersion, "opengl_version", "OpenGL version to use") orelse defaults.opengl_version,
        .config = b.option([]const u8, "config", "Compile with custom define macros overriding config.h") orelse null,
    };

    const lib = try compileRaylib(b, target, optimize, options);

    lib.installHeader(b.path(b.pathJoin(&.{ srcDir(), "raylib.h" })), "raylib.h");
    lib.installHeader(b.path(b.pathJoin(&.{ srcDir(), "raymath.h" })), "raymath.h");
    lib.installHeader(b.path(b.pathJoin(&.{ srcDir(), "rlgl.h" })), "rlgl.h");

    b.installArtifact(lib);
}

fn waylandGenerate(
    b: *std.Build,
    raylib: *std.Build.Step.Compile,
    comptime protocol: []const u8,
    comptime basename: []const u8,
) void {
    const waylandDir = b.pathJoin(&.{ srcDir(), "external/glfw/deps/wayland" });
    const protocolDir = b.pathJoin(&.{ waylandDir, protocol });
    const clientHeader = basename ++ ".h";
    const privateCode = basename ++ "-code.h";

    const client_step = b.addSystemCommand(&.{ "wayland-scanner", "client-header" });
    client_step.addFileArg(b.path(protocolDir));
    raylib.addIncludePath(client_step.addOutputFileArg(clientHeader).dirname());

    const private_step = b.addSystemCommand(&.{ "wayland-scanner", "private-code" });
    private_step.addFileArg(b.path(protocolDir));
    raylib.addIncludePath(private_step.addOutputFileArg(privateCode).dirname());

    raylib.step.dependOn(&client_step.step);
    raylib.step.dependOn(&private_step.step);
}

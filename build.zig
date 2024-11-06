const std = @import("std");
const builtin = @import("builtin");

/// Minimum supported version of Zig
const min_ver = "0.13.0";

comptime {
    const order = std.SemanticVersion.order;
    const parse = std.SemanticVersion.parse;
    if (order(builtin.zig_version, parse(min_ver) catch unreachable) == .lt)
        @compileError("Raylib requires zig version " ++ min_ver);
}

// NOTE(freakmangd): I don't like using a global here, but it prevents having to
// get the flags a second time when adding raygui
var raylib_flags_arr: std.ArrayListUnmanaged([]const u8) = .{};

// This has been tested with zig version 0.13.0
pub fn addRaylib(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode, options: Options) !*std.Build.Step.Compile {
    const raylib_dep = b.dependencyFromBuildZig(@This(), .{
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

fn createEmsdkStep(b: *std.Build, emsdk: *std.Build.Dependency) *std.Build.Step.Run {
    if (builtin.os.tag == .windows) {
        return b.addSystemCommand(&.{emsdk.path("emsdk.bat").getPath(b)});
    } else {
        return b.addSystemCommand(&.{emsdk.path("emsdk").getPath(b)});
    }
}

fn emSdkSetupStep(b: *std.Build, emsdk: *std.Build.Dependency) !?*std.Build.Step.Run {
    const dot_emsc_path = emsdk.path(".emscripten").getPath(b);
    const dot_emsc_exists = !std.meta.isError(std.fs.accessAbsolute(dot_emsc_path, .{}));

    if (!dot_emsc_exists) {
        const emsdk_install = createEmsdkStep(b, emsdk);
        emsdk_install.addArgs(&.{ "install", "latest" });
        const emsdk_activate = createEmsdkStep(b, emsdk);
        emsdk_activate.addArgs(&.{ "activate", "latest" });
        emsdk_activate.step.dependOn(&emsdk_install.step);
        return emsdk_activate;
    } else {
        return null;
    }
}

/// A list of all flags from `src/config.h` that one may override
const config_h_flags = outer: {
    // Set this value higher if compile errors happen as `src/config.h` gets larger
    @setEvalBranchQuota(1 << 20);

    const config_h = @embedFile("src/config.h");
    var flags: [std.mem.count(u8, config_h, "\n") + 1][]const u8 = undefined;

    var i = 0;
    var lines = std.mem.tokenizeScalar(u8, config_h, '\n');
    while (lines.next()) |line| {
        if (!std.mem.containsAtLeast(u8, line, 1, "SUPPORT")) continue;
        if (std.mem.startsWith(u8, line, "//")) continue;
        if (std.mem.startsWith(u8, line, "#if")) continue;

        var flag = std.mem.trimLeft(u8, line, " \t"); // Trim whitespace
        flag = flag["#define ".len - 1 ..]; // Remove #define
        flag = std.mem.trimLeft(u8, flag, " \t"); // Trim whitespace
        flag = flag[0 .. std.mem.indexOf(u8, flag, " ") orelse continue]; // Flag is only one word, so capture till space
        flag = "-D" ++ flag; // Prepend with -D

        flags[i] = flag;
        i += 1;
    }

    // Uncomment this to check what flags normally get passed
    //@compileLog(flags[0..i].*);
    break :outer flags[0..i].*;
};

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
    if (options.config.len > 0) {
        // Sets a flag indiciating the use of a custom `config.h`
        try raylib_flags_arr.append(b.allocator, "-DEXTERNAL_CONFIG_FLAGS");

        // Splits a space-separated list of config flags into multiple flags
        //
        // Note: This means certain flags like `-x c++` won't be processed properly.
        // `-xc++` or similar should be used when possible
        var config_iter = std.mem.tokenizeScalar(u8, options.config, ' ');

        // Apply config flags supplied by the user
        while (config_iter.next()) |config_flag|
            try raylib_flags_arr.append(b.allocator, config_flag);

        // Apply all relevant configs from `src/config.h` *except* the user-specified ones
        //
        // Note: Currently using a suboptimal `O(m*n)` time algorithm where:
        // `m` corresponds roughly to the number of lines in `src/config.h`
        // `n` corresponds to the number of user-specified flags
        outer: for (config_h_flags) |flag| {
            // If a user already specified the flag, skip it
            config_iter.reset();
            while (config_iter.next()) |config_flag| {
                // For a user-specified flag to match, it must share the same prefix and have the
                // same length or be followed by an equals sign
                if (!std.mem.startsWith(u8, config_flag, flag)) continue;
                if (config_flag.len == flag.len or config_flag[flag.len] == '=') continue :outer;
            }

            // Otherwise, append default value from config.h to compile flags
            try raylib_flags_arr.append(b.allocator, flag);
        }
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
        raylib.addIncludePath(b.path("src/external/glfw/include"));
    }

    var c_source_files = try std.ArrayList([]const u8).initCapacity(b.allocator, 2);
    c_source_files.appendSliceAssumeCapacity(&.{ "src/rcore.c", "src/utils.c" });

    if (options.raudio) {
        try c_source_files.append("src/raudio.c");
    }
    if (options.rmodels) {
        try c_source_files.append("src/rmodels.c");
    }
    if (options.rshapes) {
        try c_source_files.append("src/rshapes.c");
    }
    if (options.rtext) {
        try c_source_files.append("src/rtext.c");
    }
    if (options.rtextures) {
        try c_source_files.append("src/rtextures.c");
    }

    if (options.opengl_version != .auto) {
        raylib.defineCMacro(options.opengl_version.toCMacroStr(), null);
    }

    switch (target.result.os.tag) {
        .windows => {
            try c_source_files.append("src/rglfw.c");
            raylib.linkSystemLibrary("winmm");
            raylib.linkSystemLibrary("gdi32");
            raylib.linkSystemLibrary("opengl32");

            setDesktopPlatform(raylib, options.platform);
        },
        .linux => {
            if (options.platform != .drm) {
                try c_source_files.append("src/rglfw.c");

                if (options.linux_display_backend == .X11 or options.linux_display_backend == .Both) {
                    raylib.defineCMacro("_GLFW_X11", null);
                    raylib.linkSystemLibrary("GLX");
                    raylib.linkSystemLibrary("X11");
                    raylib.linkSystemLibrary("Xcursor");
                    raylib.linkSystemLibrary("Xext");
                    raylib.linkSystemLibrary("Xfixes");
                    raylib.linkSystemLibrary("Xi");
                    raylib.linkSystemLibrary("Xinerama");
                    raylib.linkSystemLibrary("Xrandr");
                    raylib.linkSystemLibrary("Xrender");
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
                    raylib.linkSystemLibrary("EGL");
                    raylib.linkSystemLibrary("wayland-client");
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
                raylib.linkSystemLibrary("gbm");
                raylib.linkSystemLibrary2("libdrm", .{ .use_pkg_config = .force });

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
            // Include xcode_frameworks for cross compilation
            if (b.lazyDependency("xcode_frameworks", .{})) |dep| {
                raylib.addSystemFrameworkPath(dep.path("Frameworks"));
                raylib.addSystemIncludePath(dep.path("include"));
                raylib.addLibraryPath(dep.path("lib"));
            }

            // On macos rglfw.c include Objective-C files.
            try raylib_flags_arr.append(b.allocator, "-ObjC");
            raylib.root_module.addCSourceFile(.{
                .file = b.path("src/rglfw.c"),
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
            // Include emscripten for cross compilation
            if (b.lazyDependency("emsdk", .{})) |dep| {
                if (try emSdkSetupStep(b, dep)) |emSdkStep| {
                    raylib.step.dependOn(&emSdkStep.step);
                }

                raylib.addIncludePath(dep.path("upstream/emscripten/cache/sysroot/include"));
            }

            raylib.defineCMacro("PLATFORM_WEB", null);
            if (options.opengl_version == .auto) {
                raylib.defineCMacro("GRAPHICS_API_OPENGL_ES2", null);
            }
        },
        else => {
            @panic("Unsupported OS");
        },
    }

    raylib.root_module.addCSourceFiles(.{
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
    /// config should be a list of space-separated cflags, eg, "-DSUPPORT_CUSTOM_FRAME_CONTROL"
    config: []const u8 = &.{},

    raygui_dependency_name: []const u8 = "raygui",

    const defaults = Options{};

    fn getOptions(b: *std.Build) Options {
        return .{
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
            .config = b.option([]const u8, "config", "Compile with custom define macros overriding config.h") orelse &.{},
        };
    }
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

    const lib = try compileRaylib(b, target, optimize, Options.getOptions(b));

    lib.installHeader(b.path("src/raylib.h"), "raylib.h");
    lib.installHeader(b.path("src/raymath.h"), "raymath.h");
    lib.installHeader(b.path("src/rlgl.h"), "rlgl.h");

    b.installArtifact(lib);
}

fn waylandGenerate(
    b: *std.Build,
    raylib: *std.Build.Step.Compile,
    comptime protocol: []const u8,
    comptime basename: []const u8,
) void {
    const waylandDir = "src/external/glfw/deps/wayland";
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

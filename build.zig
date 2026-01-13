const std = @import("std");
const builtin = @import("builtin");

/// Minimum supported version of Zig
const min_ver = "0.15.1";

const emccOutputDir = "zig-out" ++ std.fs.path.sep_str ++ "htmlout" ++ std.fs.path.sep_str;
const emccOutputFile = "index.html";

comptime {
    const order = std.SemanticVersion.order;
    const parse = std.SemanticVersion.parse;
    if (order(builtin.zig_version, parse(min_ver) catch unreachable) == .lt)
        @compileError("Raylib requires zig version " ++ min_ver);
}

pub const emsdk = struct {
    const zemscripten = @import("zemscripten");

    pub fn shell(b: *std.Build) std.Build.LazyPath {
        return b.dependency("raylib", .{}).path("src/shell.html");
    }

    pub const FlagsOptions = struct {
        optimize: std.builtin.OptimizeMode,
        asyncify: bool = true,
    };

    pub fn emccDefaultFlags(allocator: std.mem.Allocator, options: FlagsOptions) zemscripten.EmccFlags {
        var emcc_flags = zemscripten.emccDefaultFlags(allocator, .{
            .optimize = options.optimize,
            .fsanitize = true,
        });

        if (options.asyncify)
            emcc_flags.put("-sASYNCIFY", {}) catch unreachable;

        return emcc_flags;
    }

    pub const SettingsOptions = struct {
        optimize: std.builtin.OptimizeMode,
        es3: bool = true,
        emsdk_allocator: zemscripten.EmsdkAllocator = .emmalloc,
    };

    pub fn emccDefaultSettings(allocator: std.mem.Allocator, options: SettingsOptions) zemscripten.EmccSettings {
        var emcc_settings = zemscripten.emccDefaultSettings(allocator, .{
            .optimize = options.optimize,
            .emsdk_allocator = options.emsdk_allocator,
        });

        if (options.es3)
            emcc_settings.put("FULL_ES3", "1") catch unreachable;
        emcc_settings.put("USE_GLFW", "3") catch unreachable;
        emcc_settings.put("EXPORTED_RUNTIME_METHODS", "['requestFullscreen']") catch unreachable;

        return emcc_settings;
    }

    pub fn emccStep(b: *std.Build, raylib: *std.Build.Step.Compile, wasm: *std.Build.Step.Compile, options: zemscripten.StepOptions) *std.Build.Step {
        const activate_emsdk_step = zemscripten.activateEmsdkStep(b);

        const emsdk_dep = b.dependency("emsdk", .{});
        raylib.root_module.addIncludePath(emsdk_dep.path("upstream/emscripten/cache/sysroot/include"));
        wasm.root_module.addIncludePath(emsdk_dep.path("upstream/emscripten/cache/sysroot/include"));

        const emcc_step = zemscripten.emccStep(b, wasm, options);
        emcc_step.dependOn(activate_emsdk_step);

        return emcc_step;
    }

    pub fn emrunStep(
        b: *std.Build,
        html_path: []const u8,
        extra_args: []const []const u8,
    ) *std.Build.Step {
        return zemscripten.emrunStep(b, html_path, extra_args);
    }
};

fn setDesktopPlatform(raylib: *std.Build.Step.Compile, platform: PlatformBackend) void {
    switch (platform) {
        .glfw => raylib.root_module.addCMacro("PLATFORM_DESKTOP_GLFW", ""),
        .rgfw => raylib.root_module.addCMacro("PLATFORM_DESKTOP_RGFW", ""),
        .sdl => raylib.root_module.addCMacro("PLATFORM_DESKTOP_SDL", ""),
        .android => raylib.root_module.addCMacro("PLATFORM_ANDROID", ""),
        else => {},
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
        if (std.mem.containsAtLeast(u8, line, 1, "MODULE")) continue;
        if (std.mem.startsWith(u8, line, "//")) continue;
        if (std.mem.startsWith(u8, line, "#if")) continue;

        var flag = std.mem.trimStart(u8, line, " \t"); // Trim whitespace
        flag = flag["#define ".len - 1 ..]; // Remove #define
        flag = std.mem.trimStart(u8, flag, " \t"); // Trim whitespace
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
    var raylib_flags_arr: std.ArrayList([]const u8) = .empty;
    defer raylib_flags_arr.deinit(b.allocator);

    const raylib = b.addLibrary(.{
        .name = "raylib",
        .linkage = options.linkage,
        .root_module = b.createModule(.{
            .optimize = optimize,
            .target = target,
            .link_libc = true,
        }),
    });

    try raylib_flags_arr.appendSlice(
        b.allocator,
        &[_][]const u8{
            "-std=gnu99",
            "-D_GNU_SOURCE",
            "-DGL_SILENCE_DEPRECATION=199309L",
            "-fno-sanitize=undefined", // https://github.com/raysan5/raylib/issues/3674
        },
    );

    if (options.linkage == .dynamic) {
        try raylib_flags_arr.appendSlice(
            b.allocator,
            &[_][]const u8{
                "-fPIC",
                "-DBUILD_LIBTYPE_SHARED",
            },
        );
    }

    if (options.config.len > 0) {
        // Sets a flag indicating the use of a custom `config.h`
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
    } else {
        // Set default config if no custom config got set
        try raylib_flags_arr.appendSlice(b.allocator, &config_h_flags);
    }

    // No GLFW required on PLATFORM_DRM
    if (options.platform != .drm) {
        raylib.root_module.addIncludePath(b.path("src/external/glfw/include"));
    }

    var c_source_files: std.ArrayList([]const u8) = try .initCapacity(b.allocator, 2);
    c_source_files.appendSliceAssumeCapacity(&.{ "src/rcore.c" });

    if (options.rshapes) {
        try c_source_files.append(b.allocator, "src/rshapes.c");
        try raylib_flags_arr.append(b.allocator, "-DSUPPORT_MODULE_RSHAPES");
    }
    if (options.rtextures) {
        try c_source_files.append(b.allocator, "src/rtextures.c");
        try raylib_flags_arr.append(b.allocator, "-DSUPPORT_MODULE_RTEXTURES");
    }
    if (options.rtext) {
        try c_source_files.append(b.allocator, "src/rtext.c");
        try raylib_flags_arr.append(b.allocator, "-DSUPPORT_MODULE_RTEXT");
    }
    if (options.rmodels) {
        try c_source_files.append(b.allocator, "src/rmodels.c");
        try raylib_flags_arr.append(b.allocator, "-DSUPPORT_MODULE_RMODELS");
    }
    if (options.raudio) {
        try c_source_files.append(b.allocator, "src/raudio.c");
        try raylib_flags_arr.append(b.allocator, "-DSUPPORT_MODULE_RAUDIO");
    }

    if (options.opengl_version != .auto) {
        raylib.root_module.addCMacro(options.opengl_version.toCMacroStr(), "");
    }

    raylib.root_module.addIncludePath(b.path("src/platforms"));
    switch (target.result.os.tag) {
        .windows => {
            switch (options.platform) {
                .glfw => try c_source_files.append(b.allocator, "src/rglfw.c"),
                .rgfw, .sdl, .drm, .android => {},
            }

            raylib.root_module.linkSystemLibrary("winmm", .{});
            raylib.root_module.linkSystemLibrary("gdi32", .{});
            raylib.root_module.linkSystemLibrary("opengl32", .{});

            setDesktopPlatform(raylib, options.platform);
        },
        .linux => {
            if (options.platform == .drm) {
                if (options.opengl_version == .auto) {
                    raylib.root_module.linkSystemLibrary("GLESv2", .{});
                    raylib.root_module.addCMacro("GRAPHICS_API_OPENGL_ES2", "");
                }

                raylib.root_module.linkSystemLibrary("EGL", .{});
                raylib.root_module.linkSystemLibrary("gbm", .{});
                raylib.root_module.linkSystemLibrary("libdrm", .{ .use_pkg_config = .force });

                raylib.root_module.addCMacro("PLATFORM_DRM", "");
                raylib.root_module.addCMacro("EGL_NO_X11", "");
                raylib.root_module.addCMacro("DEFAULT_BATCH_BUFFER_ELEMENT", "");
            } else if (target.result.abi.isAndroid()) {

                //these are the only tag options per https://developer.android.com/ndk/guides/other_build_systems
                const hostTuple = switch (builtin.target.os.tag) {
                    .linux => "linux-x86_64",
                    .windows => "windows-x86_64",
                    .macos => "darwin-x86_64",
                    else => @panic("unsupported host OS"),
                };

                const androidTriple = switch (target.result.cpu.arch) {
                    .x86 => "i686-linux-android",
                    .x86_64 => "x86_64-linux-android",
                    .arm => "arm-linux-androideabi",
                    .aarch64 => "aarch64-linux-android",
                    .riscv64 => "riscv64-linux-android",
                    else => error.InvalidAndroidTarget,
                } catch @panic("invalid android target!");
                const androidNdkPathString: []const u8 = options.android_ndk;
                if (androidNdkPathString.len < 1) @panic("no ndk path provided and ANDROID_NDK_HOME is not set");
                const androidApiLevel: []const u8 = options.android_api_version;

                const androidSysroot = try std.fs.path.join(b.allocator, &.{ androidNdkPathString, "/toolchains/llvm/prebuilt/", hostTuple, "/sysroot" });
                const androidLibPath = try std.fs.path.join(b.allocator, &.{ androidSysroot, "/usr/lib/", androidTriple });
                const androidApiSpecificPath = try std.fs.path.join(b.allocator, &.{ androidLibPath, androidApiLevel });
                const androidIncludePath = try std.fs.path.join(b.allocator, &.{ androidSysroot, "/usr/include" });
                const androidArchIncludePath = try std.fs.path.join(b.allocator, &.{ androidIncludePath, androidTriple });
                const androidAsmPath = try std.fs.path.join(b.allocator, &.{ androidIncludePath, "/asm-generic" });
                const androidGluePath = try std.fs.path.join(b.allocator, &.{ androidNdkPathString, "/sources/android/native_app_glue/" });

                raylib.root_module.addLibraryPath(.{ .cwd_relative = androidLibPath });
                raylib.root_module.addLibraryPath(.{ .cwd_relative = androidApiSpecificPath });
                raylib.root_module.addSystemIncludePath(.{ .cwd_relative = androidIncludePath });
                raylib.root_module.addSystemIncludePath(.{ .cwd_relative = androidArchIncludePath });
                raylib.root_module.addSystemIncludePath(.{ .cwd_relative = androidAsmPath });
                raylib.root_module.addSystemIncludePath(.{ .cwd_relative = androidGluePath });

                var libcData: std.ArrayList(u8) = .empty;
                var aw: std.Io.Writer.Allocating = .fromArrayList(b.allocator, &libcData);
                try (std.zig.LibCInstallation{
                    .include_dir = androidIncludePath,
                    .sys_include_dir = androidIncludePath,
                    .crt_dir = androidApiSpecificPath,
                }).render(&aw.writer);
                const libcFile = b.addWriteFiles().add("android-libc.txt", try libcData.toOwnedSlice(b.allocator));
                raylib.setLibCFile(libcFile);

                if (options.opengl_version == .auto) {
                    raylib.root_module.linkSystemLibrary("GLESv2", .{});
                    raylib.root_module.addCMacro("GRAPHICS_API_OPENGL_ES2", "");
                }
                raylib.root_module.linkSystemLibrary("EGL", .{});

                setDesktopPlatform(raylib, .android);
            } else {
                try c_source_files.append(b.allocator, "src/rglfw.c");

                if (options.linux_display_backend == .X11 or options.linux_display_backend == .Both) {
                    raylib.root_module.addCMacro("_GLFW_X11", "");
                    raylib.root_module.linkSystemLibrary("GLX", .{});
                    raylib.root_module.linkSystemLibrary("X11", .{});
                    raylib.root_module.linkSystemLibrary("Xcursor", .{});
                    raylib.root_module.linkSystemLibrary("Xext", .{});
                    raylib.root_module.linkSystemLibrary("Xfixes", .{});
                    raylib.root_module.linkSystemLibrary("Xi", .{});
                    raylib.root_module.linkSystemLibrary("Xinerama", .{});
                    raylib.root_module.linkSystemLibrary("Xrandr", .{});
                    raylib.root_module.linkSystemLibrary("Xrender", .{});
                }

                if (options.linux_display_backend == .Wayland or options.linux_display_backend == .Both) {
                    _ = b.findProgram(&.{"wayland-scanner"}, &.{}) catch {
                        std.log.err(
                            \\ `wayland-scanner` may not be installed on the system.
                            \\ You can switch to X11 in your `build.zig` by changing `Options.linux_display_backend`
                        , .{});
                        @panic("`wayland-scanner` not found");
                    };
                    raylib.root_module.addCMacro("_GLFW_WAYLAND", "");
                    raylib.root_module.linkSystemLibrary("EGL", .{});
                    raylib.root_module.linkSystemLibrary("wayland-client", .{});
                    raylib.root_module.linkSystemLibrary("xkbcommon", .{});
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
            }
        },
        .freebsd, .openbsd, .netbsd, .dragonfly => {
            try c_source_files.append(b.allocator, "src/rglfw.c");
            raylib.root_module.linkSystemLibrary("GL", .{});
            raylib.root_module.linkSystemLibrary("rt", .{});
            raylib.root_module.linkSystemLibrary("dl", .{});
            raylib.root_module.linkSystemLibrary("m", .{});
            raylib.root_module.linkSystemLibrary("X11", .{});
            raylib.root_module.linkSystemLibrary("Xrandr", .{});
            raylib.root_module.linkSystemLibrary("Xinerama", .{});
            raylib.root_module.linkSystemLibrary("Xi", .{});
            raylib.root_module.linkSystemLibrary("Xxf86vm", .{});
            raylib.root_module.linkSystemLibrary("Xcursor", .{});

            setDesktopPlatform(raylib, options.platform);
        },
        .macos => {
            // Include xcode_frameworks for cross compilation
            if (b.lazyDependency("xcode_frameworks", .{})) |dep| {
                raylib.root_module.addSystemFrameworkPath(dep.path("Frameworks"));
                raylib.root_module.addSystemIncludePath(dep.path("include"));
                raylib.root_module.addLibraryPath(dep.path("lib"));
            }

            // On macos rglfw.c include Objective-C files.
            try raylib_flags_arr.append(b.allocator, "-ObjC");
            raylib.root_module.addCSourceFile(.{
                .file = b.path("src/rglfw.c"),
                .flags = raylib_flags_arr.items,
            });
            _ = raylib_flags_arr.pop();
            raylib.root_module.linkFramework("Foundation", .{});
            raylib.root_module.linkFramework("CoreServices", .{});
            raylib.root_module.linkFramework("CoreGraphics", .{});
            raylib.root_module.linkFramework("AppKit", .{});
            raylib.root_module.linkFramework("IOKit", .{});

            setDesktopPlatform(raylib, options.platform);
        },
        .emscripten => {
            const activate_emsdk_step = emsdk.zemscripten.activateEmsdkStep(b);
            raylib.step.dependOn(activate_emsdk_step);
            raylib.root_module.addCMacro("PLATFORM_WEB", "");
            if (options.opengl_version == .auto) {
                raylib.root_module.addCMacro("GRAPHICS_API_OPENGL_ES3", "");
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

pub fn addRaygui(b: *std.Build, raylib: *std.Build.Step.Compile, raygui_dep: *std.Build.Dependency, options: Options) void {
    const raylib_dep = b.dependencyFromBuildZig(@This(), options);
    var gen_step = b.addWriteFiles();
    raylib.step.dependOn(&gen_step.step);

    const raygui_c_path = gen_step.add("raygui.c", "#define RAYGUI_IMPLEMENTATION\n#include \"raygui.h\"\n");
    raylib.root_module.addCSourceFile(.{ .file = raygui_c_path });
    raylib.root_module.addIncludePath(raygui_dep.path("src"));
    raylib.root_module.addIncludePath(raylib_dep.path("src"));

    raylib.installHeader(raygui_dep.path("src/raygui.h"), "raygui.h");
}

pub const Options = struct {
    raudio: bool = true,
    rmodels: bool = true,
    rshapes: bool = true,
    rtext: bool = true,
    rtextures: bool = true,
    platform: PlatformBackend = .glfw,
    linkage: std.builtin.LinkMode = .static,
    linux_display_backend: LinuxDisplayBackend = .X11,
    opengl_version: OpenglVersion = .auto,
    android_ndk: []const u8 = "",
    android_api_version: []const u8 = "35",
    /// config should be a list of space-separated cflags, eg, "-DSUPPORT_CUSTOM_FRAME_CONTROL"
    config: []const u8 = &.{},

    const defaults = Options{};

    pub fn getOptions(b: *std.Build) Options {
        return .{
            .platform = b.option(PlatformBackend, "platform", "Choose the platform backend for desktop target") orelse defaults.platform,
            .raudio = b.option(bool, "raudio", "Compile with audio support") orelse defaults.raudio,
            .rmodels = b.option(bool, "rmodels", "Compile with models support") orelse defaults.rmodels,
            .rtext = b.option(bool, "rtext", "Compile with text support") orelse defaults.rtext,
            .rtextures = b.option(bool, "rtextures", "Compile with textures support") orelse defaults.rtextures,
            .rshapes = b.option(bool, "rshapes", "Compile with shapes support") orelse defaults.rshapes,
            .linkage = b.option(std.builtin.LinkMode, "linkage", "Compile as shared or static library") orelse defaults.linkage,
            .linux_display_backend = b.option(LinuxDisplayBackend, "linux_display_backend", "Linux display backend to use") orelse defaults.linux_display_backend,
            .opengl_version = b.option(OpenglVersion, "opengl_version", "OpenGL version to use") orelse defaults.opengl_version,
            .config = b.option([]const u8, "config", "Compile with custom define macros overriding config.h") orelse &.{},
            .android_ndk = b.option([]const u8, "android_ndk", "specify path to android ndk") orelse std.process.getEnvVarOwned(b.allocator, "ANDROID_NDK_HOME") catch "",
            .android_api_version = b.option([]const u8, "android_api_version", "specify target android API level") orelse defaults.android_api_version,
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
    android,
};

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const lib = try compileRaylib(b, target, optimize, Options.getOptions(b));

    lib.installHeader(b.path("src/raylib.h"), "raylib.h");
    lib.installHeader(b.path("src/rcamera.h"), "rcamera.h");
    lib.installHeader(b.path("src/raymath.h"), "raymath.h");
    lib.installHeader(b.path("src/rlgl.h"), "rlgl.h");

    b.installArtifact(lib);

    const examples = b.step("examples", "Build/Install all examples");
    examples.dependOn(try addExamples("audio", b, target, optimize, lib));
    examples.dependOn(try addExamples("core", b, target, optimize, lib));
    examples.dependOn(try addExamples("models", b, target, optimize, lib));
    examples.dependOn(try addExamples("others", b, target, optimize, lib));
    examples.dependOn(try addExamples("shaders", b, target, optimize, lib));
    examples.dependOn(try addExamples("shapes", b, target, optimize, lib));
    examples.dependOn(try addExamples("text", b, target, optimize, lib));
    examples.dependOn(try addExamples("textures", b, target, optimize, lib));
}

fn addExamples(
    comptime module: []const u8,
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    raylib: *std.Build.Step.Compile,
) !*std.Build.Step {
    const all = b.step(module, "All " ++ module ++ " examples");
    const module_subpath = b.pathJoin(&.{ "examples", module });
    var dir = try std.fs.cwd().openDir(b.pathFromRoot(module_subpath), .{ .iterate = true });
    defer dir.close();

    var iter = dir.iterate();
    while (try iter.next()) |entry| {
        if (entry.kind != .file) continue;
        const extension_idx = std.mem.lastIndexOf(u8, entry.name, ".c") orelse continue;
        const name = entry.name[0..extension_idx];
        const path = b.pathJoin(&.{ module_subpath, entry.name });

        // zig's mingw headers do not include pthread.h
        if (std.mem.eql(u8, "core_loading_thread", name) and target.result.os.tag == .windows) continue;

        const exe_mod = b.createModule(.{
            .target = target,
            .optimize = optimize,
        });
        exe_mod.addCSourceFile(.{ .file = b.path(path), .flags = &.{} });
        exe_mod.linkLibrary(raylib);

        const run_step = b.step(name, name);

        if (target.result.os.tag == .emscripten) {
            const wasm = b.addLibrary(.{
                .name = name,
                .linkage = .static,
                .root_module = exe_mod,
            });

            if (std.mem.eql(u8, name, "rlgl_standalone")) {
                //TODO: Make rlgl_standalone example work
                continue;
            }
            if (std.mem.eql(u8, name, "raylib_opengl_interop")) {
                //TODO: Make raylib_opengl_interop example work
                continue;
            }

            const emcc_flags = emsdk.emccDefaultFlags(b.allocator, .{ .optimize = optimize });
            const emcc_settings = emsdk.emccDefaultSettings(b.allocator, .{ .optimize = optimize });

            const install_dir: std.Build.InstallDir = .{ .custom = "htmlout" };
            const emcc_step = emsdk.emccStep(b, raylib, wasm, .{
                .optimize = optimize,
                .flags = emcc_flags,
                .settings = emcc_settings,
                .shell_file_path = b.path("src/shell.html"),
                .embed_paths = &.{
                    .{
                        .src_path = b.pathJoin(&.{ module_subpath, "resources" }),
                        .virtual_path = "resources",
                    },
                },
                .install_dir = install_dir,
            });

            const html_filename = try std.fmt.allocPrint(b.allocator, "{s}.html", .{wasm.name});
            const emrun_step = emsdk.emrunStep(
                b,
                b.getInstallPath(install_dir, html_filename),
                &.{"--no_browser"},
            );
            emrun_step.dependOn(emcc_step);

            run_step.dependOn(emrun_step);
            all.dependOn(emcc_step);
        } else {
            // special examples that test using these external dependencies directly
            // alongside raylib
            if (std.mem.eql(u8, name, "rlgl_standalone")) {
                exe_mod.addIncludePath(b.path("src"));
                exe_mod.addIncludePath(b.path("src/external/glfw/include"));
                if (!hasCSource(raylib.root_module, "rglfw.c")) {
                    exe_mod.addCSourceFile(.{ .file = b.path("src/rglfw.c"), .flags = &.{} });
                }
            }
            if (std.mem.eql(u8, name, "raylib_opengl_interop")) {
                exe_mod.addIncludePath(b.path("src/external"));
            }

            switch (target.result.os.tag) {
                .windows => {
                    exe_mod.linkSystemLibrary("winmm", .{});
                    exe_mod.linkSystemLibrary("gdi32", .{});
                    exe_mod.linkSystemLibrary("opengl32", .{});

                    exe_mod.addCMacro("PLATFORM_DESKTOP", "");
                },
                .linux => {
                    exe_mod.linkSystemLibrary("GL", .{});
                    exe_mod.linkSystemLibrary("rt", .{});
                    exe_mod.linkSystemLibrary("dl", .{});
                    exe_mod.linkSystemLibrary("m", .{});
                    exe_mod.linkSystemLibrary("X11", .{});

                    exe_mod.addCMacro("PLATFORM_DESKTOP", "");
                },
                .macos => {
                    exe_mod.linkFramework("Foundation", .{});
                    exe_mod.linkFramework("Cocoa", .{});
                    exe_mod.linkFramework("OpenGL", .{});
                    exe_mod.linkFramework("CoreAudio", .{});
                    exe_mod.linkFramework("CoreVideo", .{});
                    exe_mod.linkFramework("IOKit", .{});

                    exe_mod.addCMacro("PLATFORM_DESKTOP", "");
                },
                else => {
                    @panic("Unsupported OS");
                },
            }

            const exe = b.addExecutable(.{
                .name = name,
                .root_module = exe_mod,
            });

            const install_cmd = b.addInstallArtifact(exe, .{});

            const run_cmd = b.addRunArtifact(exe);
            run_cmd.cwd = b.path(module_subpath);
            run_cmd.step.dependOn(&install_cmd.step);

            run_step.dependOn(&run_cmd.step);
            all.dependOn(&install_cmd.step);
        }
    }
    return all;
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
    raylib.root_module.addIncludePath(client_step.addOutputFileArg(clientHeader).dirname());

    const private_step = b.addSystemCommand(&.{ "wayland-scanner", "private-code" });
    private_step.addFileArg(b.path(protocolDir));
    raylib.root_module.addIncludePath(private_step.addOutputFileArg(privateCode).dirname());

    raylib.step.dependOn(&client_step.step);
    raylib.step.dependOn(&private_step.step);
}

fn hasCSource(module: *std.Build.Module, name: []const u8) bool {
    for (module.link_objects.items) |o| switch (o) {
        .c_source_file => |c| if (switch (c.file) {
            .src_path => |s| std.ascii.endsWithIgnoreCase(s.sub_path, name),
            .generated, .cwd_relative, .dependency => false,
        }) return true,
        .c_source_files => |s| for (s.files) |c| if (std.ascii.endsWithIgnoreCase(c, name)) return true,
        else => {},
    };
    return false;
}

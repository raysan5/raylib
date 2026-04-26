const std = @import("std");
const builtin = @import("builtin");

pub const emsdk = struct {
    const zemscripten = @import("zemscripten");

    pub fn shell(raylib_dep: *std.Build.Dependency) std.Build.LazyPath {
        return raylib_dep.path("src/shell.html");
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
        es3: bool = false,
        glfw3: bool = true,
        memory_growth: bool = false,
        total_memory: u32 = 134217728,
        emsdk_allocator: zemscripten.EmsdkAllocator = .emmalloc,
    };

    pub fn emccDefaultSettings(allocator: std.mem.Allocator, options: SettingsOptions) zemscripten.EmccSettings {
        var emcc_settings = zemscripten.emccDefaultSettings(allocator, .{
            .optimize = options.optimize,
            .emsdk_allocator = options.emsdk_allocator,
        });

        if (options.es3) {
            emcc_settings.put("FULL_ES3", "1") catch unreachable;
            emcc_settings.put("MIN_WEBGL_VERSION", "2") catch unreachable;
            emcc_settings.put("MAX_WEBGL_VERSION", "2") catch unreachable;
        }
        if (options.glfw3) {
            emcc_settings.put("USE_GLFW", "3") catch unreachable;
        }

        const total_memory = std.fmt.allocPrint(allocator, "{d}", .{options.total_memory}) catch unreachable;

        emcc_settings.put("EXPORTED_RUNTIME_METHODS", "['requestFullscreen']") catch unreachable;
        emcc_settings.put("TOTAL_MEMORY", total_memory) catch unreachable;
        emcc_settings.put("FORCE_FILESYSTEM", "1") catch unreachable;
        emcc_settings.put("EXPORTED_RUNTIME_METHODS", "ccall") catch unreachable;

        if (options.memory_growth)
            emcc_settings.put("ALLOW_MEMORY_GROWTH", "1") catch unreachable;

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

pub fn linkWindows(mod: *std.Build.Module, opengl: bool, comptime shcore: bool) void {
    if (opengl) mod.linkSystemLibrary("opengl32", .{});
    mod.linkSystemLibrary("winmm", .{});
    mod.linkSystemLibrary("gdi32", .{});
    if (shcore) mod.linkSystemLibrary("shcore", .{});
}

fn findWaylandScanner(b: *std.Build) void {
    _ = b.findProgram(&.{"wayland-scanner"}, &.{}) catch {
        std.log.err(
            \\ `wayland-scanner` may not be installed on the system.
            \\ You can switch to X11 in your `build.zig` by changing `Options.linux_display_backend`
        , .{});
        @panic("`wayland-scanner` not found");
    };
}

pub fn linkLinux(mod: *std.Build.Module, comptime display_backend: LinuxDisplayBackend) void {
    if (display_backend == .None) {
        mod.linkSystemLibrary("GL", .{});
    }

    if (display_backend == .X11) {
        mod.linkSystemLibrary("X11", .{});
        mod.linkSystemLibrary("Xrandr", .{});
        mod.linkSystemLibrary("Xinerama", .{});
        mod.linkSystemLibrary("Xi", .{});
        mod.linkSystemLibrary("Xcursor", .{});
    }

    if (display_backend == .Wayland) {
        mod.linkSystemLibrary("wayland-client", .{});
        mod.linkSystemLibrary("wayland-cursor", .{});
        mod.linkSystemLibrary("wayland-egl", .{});
        mod.linkSystemLibrary("xkbcommon", .{});
    }
}

pub fn linkBSD(_: *std.Build, mod: *std.Build.Module) void {
    mod.linkSystemLibrary("GL", .{});
}

pub fn linkMacOS(b: *std.Build, mod: *std.Build.Module) void {
    // Include xcode_frameworks for cross compilation
    if (b.lazyDependency("xcode_frameworks", .{})) |dep| {
        mod.addSystemFrameworkPath(dep.path("Frameworks"));
        mod.addSystemIncludePath(dep.path("include"));
        mod.addLibraryPath(dep.path("lib"));
    }

    mod.linkFramework("Foundation", .{});
    mod.linkFramework("CoreServices", .{});
    mod.linkFramework("CoreGraphics", .{});
    mod.linkFramework("AppKit", .{});
    mod.linkFramework("IOKit", .{});
}

fn compileRaylib(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode, options: Options) !*std.Build.Step.Compile {
    const raylib_mod = b.createModule(.{
        .optimize = optimize,
        .target = target,
        .link_libc = true,
    });

    const raylib = b.addLibrary(.{
        .name = "raylib",
        .linkage = options.linkage,
        .root_module = raylib_mod,
    });

    raylib_mod.addCMacro("_GNU_SOURCE", "");
    raylib_mod.addCMacro("GL_SILENCE_DEPRECATION", "199309L");

    var arena: std.heap.ArenaAllocator = .init(b.allocator);
    defer arena.deinit();

    var raylib_flags_arr: std.array_list.Managed([]const u8) = .init(arena.allocator());
    var c_source_files: std.array_list.Managed([]const u8) = .init(arena.allocator());

    try c_source_files.append("src/rcore.c");

    if (target.result.os.tag == .emscripten) {
        try raylib_flags_arr.append("-std=gnu99");
    } else {
        try raylib_flags_arr.append("-std=c99");
    }

    if (options.linkage == .dynamic) {
        raylib_mod.pic = true;
        raylib_mod.addCMacro("BUILD_LIBTYPE_SHARED", "");
    }

    if (options.config.len > 0) {
        // Splits a space-separated list of config flags into multiple flags
        //
        // Note: This means certain flags like `-x c++` won't be processed properly.
        // `-xc++` or similar should be used when possible
        var config_iter = std.mem.tokenizeScalar(u8, options.config, ' ');

        // Apply config flags supplied by the user
        while (config_iter.next()) |config_flag| {
            try raylib_flags_arr.append(config_flag);
        }
    }

    raylib_mod.addCMacro("SUPPORT_MODULE_RSHAPES", &.{@as(u8, @intFromBool(options.rshapes)) + 0x30});
    if (options.rshapes) {
        try c_source_files.append("src/rshapes.c");
    }
    raylib_mod.addCMacro("SUPPORT_MODULE_RTEXTURES", &.{@as(u8, @intFromBool(options.rtextures)) + 0x30});
    if (options.rtextures) {
        try c_source_files.append("src/rtextures.c");
    }
    raylib_mod.addCMacro("SUPPORT_MODULE_RTEXT", &.{@as(u8, @intFromBool(options.rtext)) + 0x30});
    if (options.rtext) {
        try c_source_files.append("src/rtext.c");
    }
    raylib_mod.addCMacro("SUPPORT_MODULE_RMODELS", &.{@as(u8, @intFromBool(options.rmodels)) + 0x30});
    if (options.rmodels) {
        try c_source_files.append("src/rmodels.c");
    }
    raylib_mod.addCMacro("SUPPORT_MODULE_RAUDIO", &.{@as(u8, @intFromBool(options.raudio)) + 0x30});
    if (options.raudio) {
        try c_source_files.append("src/raudio.c");
    }

    raylib_mod.addIncludePath(b.path("src/platforms"));
    switch (options.platform) {
        .glfw => {
            var opengl_version: OpenglVersion = options.opengl_version;
            if (opengl_version == .gl_soft) {
                @panic("The opengl version is not supported by this platform");
            }

            raylib_mod.addIncludePath(b.path("src/external/glfw/include"));

            if (target.result.os.tag != .emscripten) {
                if (opengl_version == .auto) {
                    opengl_version = OpenglVersion.gl_3_3;
                }
                raylib_mod.addCMacro("PLATFORM_DESKTOP_GLFW", "");
                try c_source_files.append("src/rglfw.c");
            }

            switch (target.result.os.tag) {
                .windows => linkWindows(raylib_mod, true, false),
                .linux => {
                    if (target.result.abi.isAndroid()) {
                        @panic("Target is not supported with this platform");
                    }

                    linkLinux(raylib_mod, .None);

                    if (options.linux_display_backend == .X11 or options.linux_display_backend == .Both) {
                        raylib_mod.addCMacro("_GLFW_X11", "");
                        linkLinux(raylib_mod, .X11);
                    }

                    if (options.linux_display_backend == .Wayland or options.linux_display_backend == .Both) {
                        findWaylandScanner(b);

                        raylib_mod.addCMacro("_GLFW_WAYLAND", "");
                        linkLinux(raylib_mod, .Wayland);
                        try waylandGenerate(b, raylib, "src/external/glfw/deps/wayland/", false);
                    }
                },
                .freebsd, .openbsd, .netbsd, .dragonfly => linkBSD(b, raylib_mod),
                .macos => {
                    // On macos rglfw.c include Objective-C files.
                    _ = c_source_files.pop();
                    try raylib_flags_arr.append("-ObjC");
                    raylib_mod.addCSourceFile(.{
                        .file = b.path("src/rglfw.c"),
                        .flags = raylib_flags_arr.items,
                    });
                    _ = raylib_flags_arr.pop();

                    linkMacOS(b, raylib_mod);
                },
                .emscripten => {
                    switch (opengl_version) {
                        .auto => opengl_version = OpenglVersion.gles_2,
                        .gles_2, .gles_3, .gl_soft => {},
                        else => @panic("opengl version not supported"),
                    }

                    raylib_mod.addCMacro("PLATFORM_WEB", "");

                    const activate_emsdk_step = emsdk.zemscripten.activateEmsdkStep(b);
                    raylib.step.dependOn(activate_emsdk_step);
                },
                else => @panic("Target is not supported with this platform"),
            }
            raylib_mod.addCMacro(opengl_version.toCMacroStr(), "");
        },
        .rgfw => {
            var opengl_version: OpenglVersion = options.opengl_version;

            if (target.result.os.tag != .emscripten) {
                if (opengl_version == .auto) {
                    opengl_version = OpenglVersion.gl_3_3;
                }
                raylib_mod.addCMacro("PLATFORM_DESKTOP_RGFW", "");
            }

            switch (target.result.os.tag) {
                .windows => linkWindows(raylib_mod, true, false),
                .linux => {
                    if (target.result.abi.isAndroid()) {
                        @panic("Target is not supported with this platform");
                    }

                    linkLinux(raylib_mod, .None);

                    if (options.linux_display_backend == .X11 or options.linux_display_backend == .Both) {
                        raylib_mod.addCMacro("RGFW_X11", "");
                        raylib_mod.addCMacro("RGFW_UNIX", "");

                        linkLinux(raylib_mod, .X11);
                    }

                    if (options.linux_display_backend == .Wayland or options.linux_display_backend == .Both) {
                        findWaylandScanner(b);

                        if (options.linux_display_backend != .Both) {
                            raylib_mod.addCMacro("RGFW_NO_X11", "");
                        }

                        raylib_mod.addCMacro("RGFW_WAYLAND", "");
                        raylib_mod.addCMacro("EGLAPIENTRY", "");

                        linkLinux(raylib_mod, .Wayland);

                        try waylandGenerate(b, raylib, "src/external/RGFW/deps/wayland/", true);
                    }
                },
                .freebsd, .openbsd, .netbsd, .dragonfly => linkBSD(b, raylib_mod),
                .macos => linkMacOS(b, raylib_mod),
                .emscripten => {
                    switch (opengl_version) {
                        .auto => opengl_version = OpenglVersion.gles_2,
                        .gles_2, .gles_3, .gl_soft => {},
                        else => @panic("opengl version not supported"),
                    }

                    raylib_mod.addCMacro("PLATFORM_WEB_RGFW", "");
                    const activate_emsdk_step = emsdk.zemscripten.activateEmsdkStep(b);
                    raylib.step.dependOn(activate_emsdk_step);
                },
                else => @panic("Target is not supported with this platform"),
            }

            raylib_mod.addCMacro(opengl_version.toCMacroStr(), "");
        },
        .sdl, .sdl2, .sdl3 => {
            if (options.opengl_version == .auto) {
                raylib_mod.addCMacro(OpenglVersion.gl_3_3.toCMacroStr(), "");
            } else {
                raylib_mod.addCMacro(options.opengl_version.toCMacroStr(), "");
            }

            raylib_mod.addCMacro("PLATFORM_DESKTOP_SDL", "");

            if (options.platform == .sdl2) {
                raylib_mod.addCMacro("USING_SDL2_PACKAGE", "");
            }
            if (options.platform == .sdl3) {
                raylib_mod.addCMacro("USING_SDL3_PACKAGE", "");
            }
        },
        .memory => {
            if (options.opengl_version != .auto and options.opengl_version != .gl_soft) {
                @panic("The opengl version is not supported by this platform");
            }
            raylib_mod.addCMacro(OpenglVersion.gl_soft.toCMacroStr(), "");
            raylib_mod.addCMacro("PLATFORM_MEMORY", "");
        },
        .win32 => {
            if (target.result.os.tag != .windows) {
                @panic("Target is not supported with this platform");
            }

            if (options.opengl_version == .auto) {
                raylib_mod.addCMacro(OpenglVersion.gl_3_3.toCMacroStr(), "");
            } else {
                raylib_mod.addCMacro(options.opengl_version.toCMacroStr(), "");
            }

            raylib_mod.addCMacro("PLATFORM_DESKTOP_WIN32", "");

            linkWindows(raylib_mod, options.opengl_version != .gl_soft, true);
        },
        .drm => {
            if (target.result.os.tag != .linux) {
                @panic("Target is not supported with this platform");
            }

            raylib_mod.addCMacro("PLATFORM_DRM", "");
            raylib_mod.addCMacro("EGL_NO_X11", "");
            raylib_mod.addCMacro("DEFAULT_BATCH_BUFFER_ELEMENT", "");

            try raylib_flags_arr.append("-Werror=implicit-function-declaration");

            raylib_mod.linkSystemLibrary("libdrm", .{ .use_pkg_config = .force });
            raylib_mod.linkSystemLibrary("drm", .{});
            raylib_mod.linkSystemLibrary("gbm", .{});

            switch (options.opengl_version) {
                .auto, .gles_2 => {
                    raylib_mod.addCMacro(OpenglVersion.gles_2.toCMacroStr(), "");
                    raylib_mod.linkSystemLibrary("GLESv2", .{});
                    raylib_mod.linkSystemLibrary("EGL", .{});
                },
                .gl_soft => {},
                else => @panic("The opengl version is not supported by this platform"),
            }
        },
        .android => {
            if (!target.result.abi.isAndroid()) {
                @panic("Target is not supported with this platform");
            }

            raylib_mod.addCMacro("PLATFORM_ANDROID", "");

            raylib_mod.linkSystemLibrary("EGL", .{});
            switch (options.opengl_version) {
                .auto, .gles_2 => {
                    raylib_mod.addCMacro(OpenglVersion.gles_2.toCMacroStr(), "");
                    raylib_mod.linkSystemLibrary("GLESv2", .{});
                },
                else => @panic("The opengl version is not supported by this platform"),
            }

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

            raylib_mod.addLibraryPath(.{ .cwd_relative = androidLibPath });
            raylib_mod.addLibraryPath(.{ .cwd_relative = androidApiSpecificPath });
            raylib_mod.addSystemIncludePath(.{ .cwd_relative = androidIncludePath });
            raylib_mod.addSystemIncludePath(.{ .cwd_relative = androidArchIncludePath });
            raylib_mod.addSystemIncludePath(.{ .cwd_relative = androidAsmPath });
            raylib_mod.addSystemIncludePath(.{ .cwd_relative = androidGluePath });

            const libc_data = try std.fmt.allocPrint(b.allocator,
                \\include_dir={0s}/sysroot/usr/include
                \\sys_include_dir={0s}/sysroot/usr/include/aarch64-linux-android
                \\crt_dir={0s}/sysroot/usr/lib/aarch64-linux-android/24
                \\static_lib_dir={0s}/sysroot/usr/lib/aarch64-linux-android/24
                \\msvc_lib_dir=
                \\kernel32_lib_dir=
                \\gcc_dir=
                \\
            , .{androidNdkPathString});
            const write_step = b.addWriteFiles();
            const libcFile = write_step.add("android-libc.txt", libc_data);
            raylib.setLibCFile(libcFile);
        },
    }

    raylib_mod.addCSourceFiles(.{
        .files = c_source_files.items,
        .flags = raylib_flags_arr.items,
    });

    return raylib;
}

fn addRaygui(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode, raylib: *std.Build.Step.Compile) void {
    if (b.lazyDependency("raygui", .{
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    })) |raygui_dep| {
        var gen_step = b.addWriteFiles();
        raylib.step.dependOn(&gen_step.step);

        const raygui_c_path = gen_step.add("raygui.c", "#define RAYGUI_IMPLEMENTATION\n#include \"raygui.h\"\n");
        raylib.root_module.addCSourceFile(.{ .file = raygui_c_path });
        raylib.root_module.addIncludePath(raygui_dep.path("src"));
        raylib.root_module.addIncludePath(b.path("src"));

        raylib.installHeader(raygui_dep.path("src/raygui.h"), "raygui.h");

        const c = b.addTranslateC(.{
            .root_source_file = raygui_dep.path("src/raygui.h"),
            .target = target,
            .optimize = optimize,
            .link_libc = true,
        });
        c.addIncludePath(b.path("src"));
        const c_mod = c.createModule();
        c_mod.linkLibrary(raylib);
        b.modules.put(b.graph.arena, "raygui", c_mod) catch @panic("OOM");
    }
}

pub const Options = struct {
    raudio: bool = true,
    rmodels: bool = true,
    rshapes: bool = true,
    rtext: bool = true,
    rtextures: bool = true,
    raygui: bool = false,
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
            .raygui = b.option(bool, "raygui", "Include raygui") orelse defaults.raygui,
            .linkage = b.option(std.builtin.LinkMode, "linkage", "Compile as shared or static library") orelse defaults.linkage,
            .linux_display_backend = b.option(LinuxDisplayBackend, "linux_display_backend", "Linux display backend to use") orelse defaults.linux_display_backend,
            .opengl_version = b.option(OpenglVersion, "opengl_version", "OpenGL version to use") orelse defaults.opengl_version,
            .config = b.option([]const u8, "config", "Compile with custom define macros overriding config.h") orelse &.{},
            .android_ndk = b.option([]const u8, "android_ndk", "specify path to android ndk") orelse b.graph.environ_map.get("ANDROID_NDK_HOME") orelse "",
            .android_api_version = b.option([]const u8, "android_api_version", "specify target android API level") orelse defaults.android_api_version,
        };
    }
};

pub const OpenglVersion = enum {
    auto,
    gl_soft,
    gl_1_1,
    gl_2_1,
    gl_3_3,
    gl_4_3,
    gles_2,
    gles_3,

    pub fn toCMacroStr(self: @This()) []const u8 {
        switch (self) {
            .auto => @panic("OpenglVersion.auto cannot be turned into a C macro string"),
            .gl_soft => return "GRAPHICS_API_OPENGL_SOFTWARE",
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
    None,
    X11,
    Wayland,
    Both,
};

pub const PlatformBackend = enum {
    glfw,
    rgfw,
    sdl,
    sdl2,
    sdl3,
    memory,
    win32,
    drm,
    android,
};

fn translateCMod(
    comptime header: []const u8,
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    raylib: *std.Build.Step.Compile,
) void {
    const c = b.addTranslateC(.{
        .root_source_file = b.path("src/" ++ header ++ ".h"),
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    const c_mod = c.createModule();
    c_mod.linkLibrary(raylib);
    b.modules.put(b.graph.arena, header, c_mod) catch @panic("OOM");
}

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const options: Options = .getOptions(b);

    const lib = try compileRaylib(b, target, optimize, options);

    lib.installHeader(b.path("src/raylib.h"), "raylib.h");
    lib.installHeader(b.path("src/rcamera.h"), "rcamera.h");
    lib.installHeader(b.path("src/raymath.h"), "raymath.h");
    lib.installHeader(b.path("src/rlgl.h"), "rlgl.h");

    b.installArtifact(lib);

    translateCMod("raylib", b, target, optimize, lib);
    translateCMod("rcamera", b, target, optimize, lib);
    translateCMod("raymath", b, target, optimize, lib);
    translateCMod("rlgl", b, target, optimize, lib);

    if (options.raygui) {
        addRaygui(b, target, optimize, lib);
    }

    const examples = b.step("examples", "build/install all examples");
    examples.dependOn(try addExamples("core", b, target, optimize, lib, options.platform));
    examples.dependOn(try addExamples("audio", b, target, optimize, lib, options.platform));
    examples.dependOn(try addExamples("models", b, target, optimize, lib, options.platform));
    examples.dependOn(try addExamples("shaders", b, target, optimize, lib, options.platform));
    examples.dependOn(try addExamples("shapes", b, target, optimize, lib, options.platform));
    examples.dependOn(try addExamples("text", b, target, optimize, lib, options.platform));
    examples.dependOn(try addExamples("textures", b, target, optimize, lib, options.platform));
    examples.dependOn(try addExamples("others", b, target, optimize, lib, options.platform));
}

fn addExamples(
    comptime module: []const u8,
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    raylib: *std.Build.Step.Compile,
    platform: PlatformBackend,
) !*std.Build.Step {
    const all = b.step(module, "All " ++ module ++ " examples");
    const module_subpath = b.pathJoin(&.{ "examples", module });

    var dir = try b.build_root.handle.openDir(b.graph.io, module_subpath, .{ .iterate = true });
    defer dir.close(b.graph.io);

    var iter = dir.iterate();
    while (try iter.next(b.graph.io)) |entry| {
        if (entry.kind != .file) continue;

        const filetype = std.fs.path.extension(entry.name);
        if (!std.mem.eql(u8, filetype, ".c")) continue;

        const filename = std.fs.path.stem(entry.name);
        const path = b.pathJoin(&.{ module_subpath, entry.name });

        const exe_mod = b.createModule(.{
            .target = target,
            .optimize = optimize,
            .link_libc = true,
        });
        exe_mod.addCSourceFile(.{ .file = b.path(path) });
        exe_mod.linkLibrary(raylib);

        if (platform == .sdl) {
            exe_mod.linkSystemLibrary("SDL2", .{});
            exe_mod.linkSystemLibrary("SDL3", .{});
        }
        if (platform == .sdl2) {
            exe_mod.linkSystemLibrary("SDL2", .{});
        }
        if (platform == .sdl3) {
            exe_mod.linkSystemLibrary("SDL3", .{});
        }

        if (std.mem.eql(u8, filename, "rlgl_standalone")) {
            if (platform != .glfw) continue;
            exe_mod.addIncludePath(b.path("src"));
            exe_mod.addIncludePath(b.path("src/external/glfw/include"));
        }
        if (std.mem.eql(u8, filename, "raylib_opengl_interop")) {
            if (platform == .drm) continue;
            if (target.result.os.tag == .macos) continue;
            exe_mod.addIncludePath(b.path("src/external"));
        }

        const run_step = b.step(filename, filename);

        // web exports are completely separate
        if (target.query.os_tag == .emscripten) {
            exe_mod.addCMacro("PLATFORM_WEB", "");

            const wasm = b.addLibrary(.{
                .name = filename,
                .root_module = exe_mod,
            });

            const install_dir: std.Build.InstallDir = .{ .custom = b.fmt("web/{s}/{s}", .{ module, filename }) };
            const emcc_flags = emsdk.emccDefaultFlags(b.allocator, .{ .optimize = optimize });
            const emcc_settings = emsdk.emccDefaultSettings(b.allocator, .{ .optimize = optimize });

            const emcc_step = emsdk.emccStep(b, raylib, wasm, .{
                .optimize = optimize,
                .flags = emcc_flags,
                .settings = emcc_settings,
                .preload_paths = emcc_examples_preloads_map.get(filename) orelse &.{},
                .shell_file_path = b.path("src/shell.html"),
                .install_dir = install_dir,
            });
            b.getInstallStep().dependOn(emcc_step);

            const html_filename = try std.fmt.allocPrint(b.allocator, "{s}.html", .{wasm.name});
            const emrun_step = emsdk.emrunStep(
                b,
                b.getInstallPath(install_dir, html_filename),
                &.{},
            );

            emrun_step.dependOn(emcc_step);
            run_step.dependOn(emrun_step);
            all.dependOn(emcc_step);
        } else {
            exe_mod.addCMacro("PLATFORM_DESKTOP", "");

            const exe = b.addExecutable(.{
                .name = filename,
                .root_module = exe_mod,
                .use_lld = target.result.os.tag == .windows,
            });
            b.installArtifact(exe);

            const install_cmd = b.addInstallArtifact(exe, .{ .dest_sub_path = b.fmt("{s}/{s}", .{ module, filename }) });

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
    comptime waylandDir: []const u8,
    comptime source: bool,
) !void {
    const dir = try b.build_root.handle.openDir(b.graph.io, waylandDir, .{ .iterate = true });
    defer dir.close(b.graph.io);

    var iter = dir.iterate();
    while (try iter.next(b.graph.io)) |entry| {
        if (entry.kind != .file) continue;
        const protocolDir = b.pathJoin(&.{ waylandDir, entry.name });

        const filename = std.fs.path.stem(entry.name);

        const clientHeader = b.fmt("{s}-client-protocol.h", .{filename});
        const client_step = b.addSystemCommand(&.{ "wayland-scanner", "client-header" });
        client_step.addFileArg(b.path(protocolDir));
        raylib.root_module.addIncludePath(client_step.addOutputFileArg(clientHeader).dirname());
        raylib.step.dependOn(&client_step.step);

        if (comptime source) {
            const privateCode = b.fmt("{s}-client-protocol-code.c", .{filename});
            const private_step = b.addSystemCommand(&.{ "wayland-scanner", "private-code" });
            private_step.addFileArg(b.path(protocolDir));
            raylib.root_module.addCSourceFile(.{
                .file = private_step.addOutputFileArg(privateCode),
                .flags = &.{ "-std=c99", "-O2" },
            });
            raylib.step.dependOn(&private_step.step);
        } else {
            const privateCodeHeader = b.fmt("{s}-client-protocol-code.h", .{filename});
            const private_head_step = b.addSystemCommand(&.{ "wayland-scanner", "private-code" });
            private_head_step.addFileArg(b.path(protocolDir));
            raylib.root_module.addIncludePath(private_head_step.addOutputFileArg(privateCodeHeader).dirname());
            raylib.step.dependOn(&private_head_step.step);
        }
    }
}

const EmccExamplesPreloadMap = std.static_string_map.StaticStringMap([]const emsdk.zemscripten.EmccFilePath);
const EmccExamplesPreloadKV = struct { []const u8, []const emsdk.zemscripten.EmccFilePath };
const emcc_examples_preloads = [_]EmccExamplesPreloadKV{
    .{
        "core_input_gamepad",
        &.{
            .{
                .src_path = "examples/core/resources/ps3.png",
                .virtual_path = "resources/ps3.png",
            },
            .{
                .src_path = "examples/core/resources/xbox.png",
                .virtual_path = "resources/xbox.png",
            },
        },
    },
    .{
        "core_text_file_loading",
        &.{
            .{
                .src_path = "examples/core/resources/text_file.txt",
                .virtual_path = "resources/text_file.txt",
            },
        },
    },
    .{
        "core_vr_simulator",
        &.{
            .{
                .src_path = "examples/core/resources/shaders/glsl100/distortion.fs",
                .virtual_path = "resources/shaders/glsl100/distortion.fs",
            },
        },
    },
    .{
        "textures_background_scrolling",
        &.{
            .{
                .src_path = "examples/textures/resources/cyberpunk_street_background.png",
                .virtual_path = "resources/cyberpunk_street_background.png",
            },
            .{
                .src_path = "examples/textures/resources/cyberpunk_street_midground.png",
                .virtual_path = "resources/cyberpunk_street_midground.png",
            },
            .{
                .src_path = "examples/textures/resources/cyberpunk_street_foreground.png",
                .virtual_path = "resources/cyberpunk_street_foreground.png",
            },
        },
    },
    .{
        "textures_blend_modes",
        &.{
            .{
                .src_path = "examples/textures/resources/cyberpunk_street_background.png",
                .virtual_path = "resources/cyberpunk_street_background.png",
            },
            .{
                .src_path = "examples/textures/resources/cyberpunk_street_foreground.png",
                .virtual_path = "resources/cyberpunk_street_foreground.png",
            },
        },
    },
    .{
        "textures_bunnymark",
        &.{
            .{
                .src_path = "examples/textures/resources/raybunny.png",
                .virtual_path = "resources/raybunny.png",
            },
        },
    },
    .{
        "textures_gif_player",
        &.{
            .{
                .src_path = "examples/textures/resources/scarfy_run.gif",
                .virtual_path = "resources/scarfy_run.gif",
            },
        },
    },
    .{
        "textures_image_channel",
        &.{
            .{
                .src_path = "examples/textures/resources/fudesumi.png",
                .virtual_path = "resources/fudesumi.png",
            },
        },
    },
    .{
        "textures_image_drawing",
        &.{
            .{
                .src_path = "examples/textures/resources/cat.png",
                .virtual_path = "resources/cat.png",
            },
            .{
                .src_path = "examples/textures/resources/parrots.png",
                .virtual_path = "resources/parrots.png",
            },
            .{
                .src_path = "examples/textures/resources/custom_jupiter_crash.png",
                .virtual_path = "resources/custom_jupiter_crash.png",
            },
        },
    },
    .{
        "textures_image_kernel",
        &.{
            .{
                .src_path = "examples/textures/resources/cat.png",
                .virtual_path = "resources/cat.png",
            },
        },
    },
    .{
        "textures_image_loading",
        &.{
            .{
                .src_path = "examples/textures/resources/raylib_logo.png",
                .virtual_path = "resources/raylib_logo.png",
            },
        },
    },
    .{
        "textures_image_processing",
        &.{
            .{
                .src_path = "examples/textures/resources/parrots.png",
                .virtual_path = "resources/parrots.png",
            },
        },
    },
    .{
        "textures_image_rotate",
        &.{
            .{
                .src_path = "examples/textures/resources/raylib_logo.png",
                .virtual_path = "resources/raylib_logo.png",
            },
        },
    },
    .{
        "textures_image_text",
        &.{
            .{
                .src_path = "examples/textures/resources/parrots.png",
                .virtual_path = "resources/parrots.png",
            },
            .{
                .src_path = "examples/textures/resources/KAISG.ttf",
                .virtual_path = "resources/KAISG.ttf",
            },
        },
    },
    .{
        "textures_logo_raylib",
        &.{
            .{
                .src_path = "examples/textures/resources/raylib_logo.png",
                .virtual_path = "resources/raylib_logo.png",
            },
        },
    },
    .{
        "textures_magnifying_glass",
        &.{
            .{
                .src_path = "examples/textures/resources/raybunny.png",
                .virtual_path = "resources/raybunny.png",
            },
            .{
                .src_path = "examples/textures/resources/parrots.png",
                .virtual_path = "resources/parrots.png",
            },
        },
    },
    .{
        "textures_npatch_drawing",
        &.{
            .{
                .src_path = "examples/textures/resources/ninepatch_button.png",
                .virtual_path = "resources/ninepatch_button.png",
            },
        },
    },
    .{
        "textures_particles_blending",
        &.{
            .{
                .src_path = "examples/textures/resources/spark_flame.png",
                .virtual_path = "resources/spark_flame.png",
            },
        },
    },
    .{
        "textures_polygon_drawing",
        &.{
            .{
                .src_path = "examples/textures/resources/cat.png",
                .virtual_path = "resources/cat.png",
            },
        },
    },
    .{
        "textures_raw_data",
        &.{
            .{
                .src_path = "examples/textures/resources/fudesumi.raw",
                .virtual_path = "resources/fudesumi.raw",
            },
        },
    },
    .{
        "textures_sprite_animation",
        &.{
            .{
                .src_path = "examples/textures/resources/scarfy.png",
                .virtual_path = "resources/scarfy.png",
            },
        },
    },
    .{
        "textures_sprite_button",
        &.{
            .{
                .src_path = "examples/textures/resources/buttonfx.wav",
                .virtual_path = "resources/buttonfx.wav",
            },
            .{
                .src_path = "examples/textures/resources/button.png",
                .virtual_path = "resources/button.png",
            },
        },
    },
    .{
        "textures_sprite_explosion",
        &.{
            .{
                .src_path = "examples/textures/resources/boom.wav",
                .virtual_path = "resources/boom.wav",
            },
            .{
                .src_path = "examples/textures/resources/explosion.png",
                .virtual_path = "resources/explosion.png",
            },
        },
    },
    .{
        "textures_sprite_stacking",
        &.{
            .{
                .src_path = "examples/textures/resources/booth.png",
                .virtual_path = "resources/booth.png",
            },
        },
    },
    .{
        "textures_srcrec_dstrec",
        &.{
            .{
                .src_path = "examples/textures/resources/scarfy.png",
                .virtual_path = "resources/scarfy.png",
            },
        },
    },
    .{
        "textures_textured_curve",
        &.{
            .{
                .src_path = "examples/textures/resources/road.png",
                .virtual_path = "resources/road.png",
            },
        },
    },
    .{
        "textures_tiled_drawing",
        &.{
            .{
                .src_path = "examples/textures/resources/patterns.png",
                .virtual_path = "resources/patterns.png",
            },
        },
    },
    .{
        "textures_to_image",
        &.{
            .{
                .src_path = "examples/textures/resources/raylib_logo.png",
                .virtual_path = "resources/raylib_logo.png",
            },
        },
    },
    .{
        "text_3d_drawing",
        &.{
            .{
                .src_path = "examples/text/resources/shaders/glsl100/alpha_discard.fs",
                .virtual_path = "resources/shaders/glsl100/alpha_discard.fs",
            },
        },
    },
    .{
        "text_codepoints_loading",
        &.{
            .{
                .src_path = "examples/text/resources/DotGothic16-Regular.ttf",
                .virtual_path = "resources/DotGothic16-Regular.ttf",
            },
        },
    },
    .{
        "text_font_filters",
        &.{
            .{
                .src_path = "examples/text/resources/KAISG.ttf",
                .virtual_path = "resources/KAISG.ttf",
            },
        },
    },
    .{
        "text_font_loading",
        &.{
            .{
                .src_path = "examples/text/resources/pixantiqua.fnt",
                .virtual_path = "resources/pixantiqua.fnt",
            },
            .{
                .src_path = "examples/text/resources/pixantiqua.png",
                .virtual_path = "resources/pixantiqua.png",
            },
            .{
                .src_path = "examples/text/resources/pixantiqua.ttf",
                .virtual_path = "resources/pixantiqua.ttf",
            },
        },
    },
    .{
        "text_font_sdf",
        &.{
            .{
                .src_path = "examples/text/resources/anonymous_pro_bold.ttf",
                .virtual_path = "resources/anonymous_pro_bold.ttf",
            },
            .{
                .src_path = "examples/text/resources/shaders/glsl100/sdf.fs",
                .virtual_path = "resources/shaders/glsl100/sdf.fs",
            },
        },
    },
    .{
        "text_font_spritefont",
        &.{
            .{
                .src_path = "examples/text/resources/custom_mecha.png",
                .virtual_path = "resources/custom_mecha.png",
            },
            .{
                .src_path = "examples/text/resources/custom_alagard.png",
                .virtual_path = "resources/custom_alagard.png",
            },
            .{
                .src_path = "examples/text/resources/custom_jupiter_crash.png",
                .virtual_path = "resources/custom_jupiter_crash.png",
            },
        },
    },
    .{
        "text_sprite_fonts",
        &.{
            .{
                .src_path = "examples/text/resources/sprite_fonts/alagard.png",
                .virtual_path = "resources/sprite_fonts/alagard.png",
            },
            .{
                .src_path = "examples/text/resources/sprite_fonts/pixelplay.png",
                .virtual_path = "resources/sprite_fonts/pixelplay.png",
            },
            .{
                .src_path = "examples/text/resources/sprite_fonts/mecha.png",
                .virtual_path = "resources/sprite_fonts/mecha.png",
            },
            .{
                .src_path = "examples/text/resources/sprite_fonts/setback.png",
                .virtual_path = "resources/sprite_fonts/setback.png",
            },
            .{
                .src_path = "examples/text/resources/sprite_fonts/romulus.png",
                .virtual_path = "resources/sprite_fonts/romulus.png",
            },
            .{
                .src_path = "examples/text/resources/sprite_fonts/pixantiqua.png",
                .virtual_path = "resources/sprite_fonts/pixantiqua.png",
            },
            .{
                .src_path = "examples/text/resources/sprite_fonts/alpha_beta.png",
                .virtual_path = "resources/sprite_fonts/alpha_beta.png",
            },
            .{
                .src_path = "examples/text/resources/sprite_fonts/jupiter_crash.png",
                .virtual_path = "resources/sprite_fonts/jupiter_crash.png",
            },
        },
    },
    .{
        "text_unicode_emojis",
        &.{
            .{
                .src_path = "examples/text/resources/dejavu.fnt",
                .virtual_path = "resources/dejavu.fnt",
            },
            .{
                .src_path = "examples/text/resources/dejavu.png",
                .virtual_path = "resources/dejavu.png",
            },
            .{
                .src_path = "examples/text/resources/noto_cjk.fnt",
                .virtual_path = "resources/noto_cjk.fnt",
            },
            .{
                .src_path = "examples/text/resources/noto_cjk.png",
                .virtual_path = "resources/noto_cjk.png",
            },
            .{
                .src_path = "examples/text/resources/symbola.fnt",
                .virtual_path = "resources/symbola.fnt",
            },
            .{
                .src_path = "examples/text/resources/symbola.png",
                .virtual_path = "resources/symbola.png",
            },
        },
    },
    .{
        "text_unicode_ranges",
        &.{
            .{
                .src_path = "examples/text/resources/NotoSansTC-Regular.ttf",
                .virtual_path = "resources/NotoSansTC-Regular.ttf",
            },
        },
    },
    .{
        "models_animation_blend_custom",
        &.{
            .{
                .src_path = "examples/models/resources/models/gltf/greenman.glb",
                .virtual_path = "resources/models/gltf/greenman.glb",
            },
            .{
                .src_path = "examples/models/resources/shaders/glsl100/skinning.vs",
                .virtual_path = "resources/shaders/glsl100/skinning.vs",
            },
            .{
                .src_path = "examples/models/resources/shaders/glsl100/skinning.fs",
                .virtual_path = "resources/shaders/glsl100/skinning.fs",
            },
        },
    },
    .{
        "models_animation_blending",
        &.{
            .{
                .src_path = "examples/models/resources/models/gltf/robot.glb",
                .virtual_path = "resources/models/gltf/robot.glb",
            },
            .{
                .src_path = "examples/models/resources/shaders/glsl100/skinning.vs",
                .virtual_path = "resources/shaders/glsl100/skinning.vs",
            },
            .{
                .src_path = "examples/models/resources/shaders/glsl100/skinning.fs",
                .virtual_path = "resources/shaders/glsl100/skinning.fs",
            },
        },
    },
    .{
        "models_animation_gpu_skinning",
        &.{
            .{
                .src_path = "examples/models/resources/models/gltf/greenman.glb",
                .virtual_path = "resources/models/gltf/greenman.glb",
            },
            .{
                .src_path = "examples/models/resources/shaders/glsl100/skinning.vs",
                .virtual_path = "resources/shaders/glsl100/skinning.vs",
            },
            .{
                .src_path = "examples/models/resources/shaders/glsl100/skinning.fs",
                .virtual_path = "resources/shaders/glsl100/skinning.fs",
            },
        },
    },
    .{
        "models_animation_timing",
        &.{
            .{
                .src_path = "examples/models/resources/models/gltf/robot.glb",
                .virtual_path = "resources/models/gltf/robot.glb",
            },
        },
    },
    .{
        "models_billboard_rendering",
        &.{
            .{
                .src_path = "examples/models/resources/billboard.png",
                .virtual_path = "resources/billboard.png",
            },
        },
    },
    .{
        "models_bone_socket",
        &.{
            .{
                .src_path = "examples/models/resources/models/gltf/greenman.glb",
                .virtual_path = "resources/models/gltf/greenman.glb",
            },
            .{
                .src_path = "examples/models/resources/models/gltf/greenman_hat.glb",
                .virtual_path = "resources/models/gltf/greenman_hat.glb",
            },
            .{
                .src_path = "examples/models/resources/models/gltf/greenman_sword.glb",
                .virtual_path = "resources/models/gltf/greenman_sword.glb",
            },
            .{
                .src_path = "examples/models/resources/models/gltf/greenman_shield.glb",
                .virtual_path = "resources/models/gltf/greenman_shield.glb",
            },
        },
    },
    .{
        "models_cubicmap_rendering",
        &.{
            .{
                .src_path = "examples/models/resources/cubicmap.png",
                .virtual_path = "resources/cubicmap.png",
            },
            .{
                .src_path = "examples/models/resources/cubicmap_atlas.png",
                .virtual_path = "resources/cubicmap_atlas.png",
            },
        },
    },
    .{
        "models_decals",
        &.{
            .{
                .src_path = "examples/models/resources/models/obj/character.obj",
                .virtual_path = "resources/models/obj/character.obj",
            },
            .{
                .src_path = "examples/models/resources/models/obj/character_diffuse.png",
                .virtual_path = "resources/models/obj/character_diffuse.png",
            },
            .{
                .src_path = "examples/models/resources/raylib_logo.png",
                .virtual_path = "resources/raylib_logo.png",
            },
        },
    },
    .{
        "models_directional_billboard",
        &.{
            .{
                .src_path = "examples/models/resources/skillbot.png",
                .virtual_path = "resources/skillbot.png",
            },
        },
    },
    .{
        "models_first_person_maze",
        &.{
            .{
                .src_path = "examples/models/resources/cubicmap.png",
                .virtual_path = "resources/cubicmap.png",
            },
            .{
                .src_path = "examples/models/resources/cubicmap_atlas.png",
                .virtual_path = "resources/cubicmap_atlas.png",
            },
        },
    },
    .{
        "models_heightmap_rendering",
        &.{
            .{
                .src_path = "examples/models/resources/heightmap.png",
                .virtual_path = "resources/heightmap.png",
            },
        },
    },
    .{
        "models_loading",
        &.{
            .{
                .src_path = "examples/models/resources/models/obj/castle.obj",
                .virtual_path = "resources/models/obj/castle.obj",
            },
            .{
                .src_path = "examples/models/resources/models/obj/castle_diffuse.png",
                .virtual_path = "resources/models/obj/castle_diffuse.png",
            },
        },
    },
    .{
        "models_loading_gltf",
        &.{
            .{
                .src_path = "examples/models/resources/models/gltf/robot.glb",
                .virtual_path = "resources/models/gltf/robot.glb",
            },
        },
    },
    .{
        "models_loading_iqm",
        &.{
            .{
                .src_path = "examples/models/resources/models/iqm/guy.iqm",
                .virtual_path = "resources/models/iqm/guy.iqm",
            },
            .{
                .src_path = "examples/models/resources/models/iqm/guytex.png",
                .virtual_path = "resources/models/iqm/guytex.png",
            },
            .{
                .src_path = "examples/models/resources/models/iqm/guyanim.iqm",
                .virtual_path = "resources/models/iqm/guyanim.iqm",
            },
        },
    },
    .{
        "models_loading_m3d",
        &.{
            .{
                .src_path = "examples/models/resources/models/m3d/cesium_man.m3d",
                .virtual_path = "resources/models/m3d/cesium_man.m3d",
            },
        },
    },
    .{
        "models_loading_vox",
        &.{
            .{
                .src_path = "examples/models/resources/models/vox/chr_knight.vox",
                .virtual_path = "resources/models/vox/chr_knight.vox",
            },
            .{
                .src_path = "examples/models/resources/models/vox/chr_sword.vox",
                .virtual_path = "resources/models/vox/chr_sword.vox",
            },
            .{
                .src_path = "examples/models/resources/models/vox/monu9.vox",
                .virtual_path = "resources/models/vox/monu9.vox",
            },
            .{
                .src_path = "examples/models/resources/models/vox/fez.vox",
                .virtual_path = "resources/models/vox/fez.vox",
            },
            .{
                .src_path = "examples/models/resources/shaders/glsl100/voxel_lighting.vs",
                .virtual_path = "resources/shaders/glsl100/voxel_lighting.vs",
            },
            .{
                .src_path = "examples/models/resources/shaders/glsl100/voxel_lighting.fs",
                .virtual_path = "resources/shaders/glsl100/voxel_lighting.fs",
            },
        },
    },
    .{
        "models_mesh_picking",
        &.{
            .{
                .src_path = "examples/models/resources/models/obj/turret.obj",
                .virtual_path = "resources/models/obj/turret.obj",
            },
            .{
                .src_path = "examples/models/resources/models/obj/turret_diffuse.png",
                .virtual_path = "resources/models/obj/turret_diffuse.png",
            },
        },
    },
    .{
        "models_rotating_cube",
        &.{
            .{
                .src_path = "examples/models/resources/cubicmap_atlas.png",
                .virtual_path = "resources/cubicmap_atlas.png",
            },
        },
    },
    .{
        "models_skybox_rendering",
        &.{
            .{
                .src_path = "examples/models/resources/shaders/glsl100/skybox.vs",
                .virtual_path = "resources/shaders/glsl100/skybox.vs",
            },
            .{
                .src_path = "examples/models/resources/shaders/glsl100/skybox.fs",
                .virtual_path = "resources/shaders/glsl100/skybox.fs",
            },
            .{
                .src_path = "examples/models/resources/shaders/glsl100/cubemap.vs",
                .virtual_path = "resources/shaders/glsl100/cubemap.vs",
            },
            .{
                .src_path = "examples/models/resources/shaders/glsl100/cubemap.fs",
                .virtual_path = "resources/shaders/glsl100/cubemap.fs",
            },
            .{
                .src_path = "examples/models/resources/dresden_square_2k.hdr",
                .virtual_path = "resources/dresden_square_2k.hdr",
            },
            .{
                .src_path = "examples/models/resources/skybox.png",
                .virtual_path = "resources/skybox.png",
            },
        },
    },
    .{
        "models_textured_cube",
        &.{
            .{
                .src_path = "examples/models/resources/cubicmap_atlas.png",
                .virtual_path = "resources/cubicmap_atlas.png",
            },
        },
    },
    .{
        "models_yaw_pitch_roll",
        &.{
            .{
                .src_path = "examples/models/resources/models/obj/plane.obj",
                .virtual_path = "resources/models/obj/plane.obj",
            },
            .{
                .src_path = "examples/models/resources/models/obj/plane_diffuse.png",
                .virtual_path = "resources/models/obj/plane_diffuse.png",
            },
        },
    },
    .{
        "shaders_ascii_rendering",
        &.{
            .{
                .src_path = "examples/shaders/resources/fudesumi.png",
                .virtual_path = "resources/fudesumi.png",
            },
            .{
                .src_path = "examples/shaders/resources/raysan.png",
                .virtual_path = "resources/raysan.png",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/ascii.fs",
                .virtual_path = "resources/shaders/glsl100/ascii.fs",
            },
        },
    },
    .{
        "shaders_basic_lighting",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/lighting.vs",
                .virtual_path = "resources/shaders/glsl100/lighting.vs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/lighting.fs",
                .virtual_path = "resources/shaders/glsl100/lighting.fs",
            },
        },
    },
    .{
        "shaders_basic_pbr",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/pbr.vs",
                .virtual_path = "resources/shaders/glsl100/pbr.vs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/pbr.fs",
                .virtual_path = "resources/shaders/glsl100/pbr.fs",
            },
            .{
                .src_path = "examples/shaders/resources/models/old_car_new.glb",
                .virtual_path = "resources/models/old_car_new.glb",
            },
            .{
                .src_path = "examples/shaders/resources/old_car_d.png",
                .virtual_path = "resources/old_car_d.png",
            },
            .{
                .src_path = "examples/shaders/resources/old_car_mra.png",
                .virtual_path = "resources/old_car_mra.png",
            },
            .{
                .src_path = "examples/shaders/resources/old_car_n.png",
                .virtual_path = "resources/old_car_n.png",
            },
            .{
                .src_path = "examples/shaders/resources/old_car_e.png",
                .virtual_path = "resources/old_car_e.png",
            },
            .{
                .src_path = "examples/shaders/resources/models/plane.glb",
                .virtual_path = "resources/models/plane.glb",
            },
            .{
                .src_path = "examples/shaders/resources/road_a.png",
                .virtual_path = "resources/road_a.png",
            },
            .{
                .src_path = "examples/shaders/resources/road_mra.png",
                .virtual_path = "resources/road_mra.png",
            },
            .{
                .src_path = "examples/shaders/resources/road_n.png",
                .virtual_path = "resources/road_n.png",
            },
        },
    },
    .{
        "shaders_cel_shading",
        &.{
            .{
                .src_path = "examples/shaders/resources/models/old_car_new.glb",
                .virtual_path = "resources/models/old_car_new.glb",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/cel.vs",
                .virtual_path = "resources/shaders/glsl100/cel.vs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/cel.fs",
                .virtual_path = "resources/shaders/glsl100/cel.fs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/outline_hull.vs",
                .virtual_path = "resources/shaders/glsl100/outline_hull.vs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/outline_hull.fs",
                .virtual_path = "resources/shaders/glsl100/outline_hull.fs",
            },
        },
    },
    .{
        "shaders_color_correction",
        &.{
            .{
                .src_path = "examples/shaders/resources/parrots.png",
                .virtual_path = "resources/parrots.png",
            },
            .{
                .src_path = "examples/shaders/resources/cat.png",
                .virtual_path = "resources/cat.png",
            },
            .{
                .src_path = "examples/shaders/resources/mandrill.png",
                .virtual_path = "resources/mandrill.png",
            },
            .{
                .src_path = "examples/shaders/resources/fudesumi.png",
                .virtual_path = "resources/fudesumi.png",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/color_correction.fs",
                .virtual_path = "resources/shaders/glsl100/color_correction.fs",
            },
        },
    },
    .{
        "shaders_custom_uniform",
        &.{
            .{
                .src_path = "examples/shaders/resources/models/barracks.obj",
                .virtual_path = "resources/models/barracks.obj",
            },
            .{
                .src_path = "examples/shaders/resources/models/barracks_diffuse.png",
                .virtual_path = "resources/models/barracks_diffuse.png",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/swirl.fs",
                .virtual_path = "resources/shaders/glsl100/swirl.fs",
            },
        },
    },
    .{
        "shaders_deferred_rendering",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/gbuffer.vs",
                .virtual_path = "resources/shaders/glsl100/gbuffer.vs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/gbuffer.fs",
                .virtual_path = "resources/shaders/glsl100/gbuffer.fs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/deferred_shading.vs",
                .virtual_path = "resources/shaders/glsl100/deferred_shading.vs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/deferred_shading.fs",
                .virtual_path = "resources/shaders/glsl100/deferred_shading.fs",
            },
        },
    },
    .{
        "shaders_depth_rendering",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/depth_render.fs",
                .virtual_path = "resources/shaders/glsl100/depth_render.fs",
            },
        },
    },
    .{
        "shaders_depth_writing",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/depth_write.fs",
                .virtual_path = "resources/shaders/glsl100/depth_write.fs",
            },
        },
    },
    .{
        "shaders_eratosthenes_sieve",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/eratosthenes.fs",
                .virtual_path = "resources/shaders/glsl100/eratosthenes.fs",
            },
        },
    },
    .{
        "shaders_fog_rendering",
        &.{
            .{
                .src_path = "examples/shaders/resources/texel_checker.png",
                .virtual_path = "resources/texel_checker.png",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/lighting.vs",
                .virtual_path = "resources/shaders/glsl100/lighting.vs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/fog.fs",
                .virtual_path = "resources/shaders/glsl100/fog.fs",
            },
        },
    },
    .{
        "shaders_game_of_life",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/game_of_life.fs",
                .virtual_path = "resources/shaders/glsl100/game_of_life.fs",
            },
            .{
                .src_path = "examples/shaders/resources/game_of_life/r_pentomino.png",
                .virtual_path = "resources/game_of_life/r_pentomino.png",
            },
            .{
                .src_path = "examples/shaders/resources/game_of_life/glider.png",
                .virtual_path = "resources/game_of_life/glider.png",
            },
            .{
                .src_path = "examples/shaders/resources/game_of_life/acorn.png",
                .virtual_path = "resources/game_of_life/acorn.png",
            },
            .{
                .src_path = "examples/shaders/resources/game_of_life/spaceships.png",
                .virtual_path = "resources/game_of_life/spaceships.png",
            },
            .{
                .src_path = "examples/shaders/resources/game_of_life/still_lifes.png",
                .virtual_path = "resources/game_of_life/still_lifes.png",
            },
            .{
                .src_path = "examples/shaders/resources/game_of_life/oscillators.png",
                .virtual_path = "resources/game_of_life/oscillators.png",
            },
            .{
                .src_path = "examples/shaders/resources/game_of_life/puffer_train.png",
                .virtual_path = "resources/game_of_life/puffer_train.png",
            },
            .{
                .src_path = "examples/shaders/resources/game_of_life/glider_gun.png",
                .virtual_path = "resources/game_of_life/glider_gun.png",
            },
            .{
                .src_path = "examples/shaders/resources/game_of_life/breeder.png",
                .virtual_path = "resources/game_of_life/breeder.png",
            },
        },
    },
    .{
        "shaders_hot_reloading",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/reload.fs",
                .virtual_path = "resources/shaders/glsl100/reload.fs",
            },
        },
    },
    .{
        "shaders_hybrid_rendering",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/hybrid_raymarch.fs",
                .virtual_path = "resources/shaders/glsl100/hybrid_raymarch.fs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/hybrid_raster.fs",
                .virtual_path = "resources/shaders/glsl100/hybrid_raster.fs",
            },
        },
    },
    .{
        "shaders_julia_set",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/julia_set.fs",
                .virtual_path = "resources/shaders/glsl100/julia_set.fs",
            },
        },
    },
    .{
        "shaders_lightmap_rendering",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/lightmap.vs",
                .virtual_path = "resources/shaders/glsl100/lightmap.vs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/lightmap.fs",
                .virtual_path = "resources/shaders/glsl100/lightmap.fs",
            },
            .{
                .src_path = "examples/shaders/resources/cubicmap_atlas.png",
                .virtual_path = "resources/cubicmap_atlas.png",
            },
            .{
                .src_path = "examples/shaders/resources/spark_flame.png",
                .virtual_path = "resources/spark_flame.png",
            },
        },
    },
    .{
        "shaders_mandelbrot_set",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/mandelbrot_set.fs",
                .virtual_path = "resources/shaders/glsl100/mandelbrot_set.fs",
            },
        },
    },
    .{
        "shaders_mesh_instancing",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/lighting_instancing.vs",
                .virtual_path = "resources/shaders/glsl100/lighting_instancing.vs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/lighting.fs",
                .virtual_path = "resources/shaders/glsl100/lighting.fs",
            },
        },
    },
    .{
        "shaders_model_shader",
        &.{
            .{
                .src_path = "examples/shaders/resources/models/watermill.obj",
                .virtual_path = "resources/models/watermill.obj",
            },
            .{
                .src_path = "examples/shaders/resources/models/watermill_diffuse.png",
                .virtual_path = "resources/models/watermill_diffuse.png",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/grayscale.fs",
                .virtual_path = "resources/shaders/glsl100/grayscale.fs",
            },
        },
    },
    .{
        "shaders_multi_sample2d",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/color_mix.fs",
                .virtual_path = "resources/shaders/glsl100/color_mix.fs",
            },
        },
    },
    .{
        "shaders_normalmap_rendering",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/normalmap.vs",
                .virtual_path = "resources/shaders/glsl100/normalmap.vs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/normalmap.fs",
                .virtual_path = "resources/shaders/glsl100/normalmap.fs",
            },
            .{
                .src_path = "examples/shaders/resources/models/plane.glb",
                .virtual_path = "resources/models/plane.glb",
            },
            .{
                .src_path = "examples/shaders/resources/tiles_diffuse.png",
                .virtual_path = "resources/tiles_diffuse.png",
            },
            .{
                .src_path = "examples/shaders/resources/tiles_normal.png",
                .virtual_path = "resources/tiles_normal.png",
            },
        },
    },
    .{
        "shaders_palette_switch",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/palette_switch.fs",
                .virtual_path = "resources/shaders/glsl100/palette_switch.fs",
            },
        },
    },
    .{
        "shaders_postprocessing",
        &.{
            .{
                .src_path = "examples/shaders/resources/models/church.obj",
                .virtual_path = "resources/models/church.obj",
            },
            .{
                .src_path = "examples/shaders/resources/models/church_diffuse.png",
                .virtual_path = "resources/models/church_diffuse.png",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/grayscale.fs",
                .virtual_path = "resources/shaders/glsl100/grayscale.fs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/posterization.fs",
                .virtual_path = "resources/shaders/glsl100/posterization.fs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/dream_vision.fs",
                .virtual_path = "resources/shaders/glsl100/dream_vision.fs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/pixelizer.fs",
                .virtual_path = "resources/shaders/glsl100/pixelizer.fs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/cross_hatching.fs",
                .virtual_path = "resources/shaders/glsl100/cross_hatching.fs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/cross_stitching.fs",
                .virtual_path = "resources/shaders/glsl100/cross_stitching.fs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/predator.fs",
                .virtual_path = "resources/shaders/glsl100/predator.fs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/scanlines.fs",
                .virtual_path = "resources/shaders/glsl100/scanlines.fs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/fisheye.fs",
                .virtual_path = "resources/shaders/glsl100/fisheye.fs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/sobel.fs",
                .virtual_path = "resources/shaders/glsl100/sobel.fs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/bloom.fs",
                .virtual_path = "resources/shaders/glsl100/bloom.fs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/blur.fs",
                .virtual_path = "resources/shaders/glsl100/blur.fs",
            },
        },
    },
    .{
        "shaders_raymarching_rendering",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/raymarching.fs",
                .virtual_path = "resources/shaders/glsl100/raymarching.fs",
            },
        },
    },
    .{
        "shaders_rounded_rectangle",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/base.vs",
                .virtual_path = "resources/shaders/glsl100/base.vs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/rounded_rectangle.fs",
                .virtual_path = "resources/shaders/glsl100/rounded_rectangle.fs",
            },
        },
    },
    .{
        "shaders_shadowmap_rendering",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/shadowmap.vs",
                .virtual_path = "resources/shaders/glsl100/shadowmap.vs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/shadowmap.fs",
                .virtual_path = "resources/shaders/glsl100/shadowmap.fs",
            },
            .{
                .src_path = "examples/shaders/resources/models/robot.glb",
                .virtual_path = "resources/models/robot.glb",
            },
        },
    },
    .{
        "shaders_shapes_textures",
        &.{
            .{
                .src_path = "examples/shaders/resources/fudesumi.png",
                .virtual_path = "resources/fudesumi.png",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/grayscale.fs",
                .virtual_path = "resources/shaders/glsl100/grayscale.fs",
            },
        },
    },
    .{
        "shaders_simple_mask",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/mask.fs",
                .virtual_path = "resources/shaders/glsl100/mask.fs",
            },
            .{
                .src_path = "examples/shaders/resources/plasma.png",
                .virtual_path = "resources/plasma.png",
            },
            .{
                .src_path = "examples/shaders/resources/mask.png",
                .virtual_path = "resources/mask.png",
            },
        },
    },
    .{
        "shaders_spotlight_rendering",
        &.{
            .{
                .src_path = "examples/shaders/resources/raysan.png",
                .virtual_path = "resources/raysan.png",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/spotlight.fs",
                .virtual_path = "resources/shaders/glsl100/spotlight.fs",
            },
        },
    },
    .{
        "shaders_texture_outline",
        &.{
            .{
                .src_path = "examples/shaders/resources/fudesumi.png",
                .virtual_path = "resources/fudesumi.png",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/outline.fs",
                .virtual_path = "resources/shaders/glsl100/outline.fs",
            },
        },
    },
    .{
        "shaders_texture_rendering",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/cubes_panning.fs",
                .virtual_path = "resources/shaders/glsl100/cubes_panning.fs",
            },
        },
    },
    .{
        "shaders_texture_tiling",
        &.{
            .{
                .src_path = "examples/shaders/resources/cubicmap_atlas.png",
                .virtual_path = "resources/cubicmap_atlas.png",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/tiling.fs",
                .virtual_path = "resources/shaders/glsl100/tiling.fs",
            },
        },
    },
    .{
        "shaders_texture_waves",
        &.{
            .{
                .src_path = "examples/shaders/resources/space.png",
                .virtual_path = "resources/space.png",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/wave.fs",
                .virtual_path = "resources/shaders/glsl100/wave.fs",
            },
        },
    },
    .{
        "shaders_vertex_displacement",
        &.{
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/vertex_displacement.vs",
                .virtual_path = "resources/shaders/glsl100/vertex_displacement.vs",
            },
            .{
                .src_path = "examples/shaders/resources/shaders/glsl100/vertex_displacement.fs",
                .virtual_path = "resources/shaders/glsl100/vertex_displacement.fs",
            },
        },
    },
    .{
        "audio_mixed_processor",
        &.{
            .{
                .src_path = "examples/audio/resources/country.mp3",
                .virtual_path = "resources/country.mp3",
            },
            .{
                .src_path = "examples/audio/resources/coin.wav",
                .virtual_path = "resources/coin.wav",
            },
        },
    },
    .{
        "audio_module_playing",
        &.{
            .{
                .src_path = "examples/audio/resources/mini1111.xm",
                .virtual_path = "resources/mini1111.xm",
            },
        },
    },
    .{
        "audio_music_stream",
        &.{
            .{
                .src_path = "examples/audio/resources/country.mp3",
                .virtual_path = "resources/country.mp3",
            },
        },
    },
    .{
        "audio_sound_loading",
        &.{
            .{
                .src_path = "examples/audio/resources/sound.wav",
                .virtual_path = "resources/sound.wav",
            },
            .{
                .src_path = "examples/audio/resources/target.ogg",
                .virtual_path = "resources/target.ogg",
            },
        },
    },
    .{
        "audio_sound_multi",
        &.{
            .{
                .src_path = "examples/audio/resources/sound.wav",
                .virtual_path = "resources/sound.wav",
            },
        },
    },
    .{
        "audio_sound_positioning",
        &.{
            .{
                .src_path = "examples/audio/resources/coin.wav",
                .virtual_path = "resources/coin.wav",
            },
        },
    },
    .{
        "audio_spectrum_visualizer",
        &.{
            .{
                .src_path = "examples/audio/resources/shaders/glsl100/fft.fs",
                .virtual_path = "resources/shaders/glsl100/fft.fs",
            },
            .{
                .src_path = "examples/audio/resources/country.mp3",
                .virtual_path = "resources/country.mp3",
            },
        },
    },
    .{
        "audio_stream_effects",
        &.{
            .{
                .src_path = "examples/audio/resources/country.mp3",
                .virtual_path = "resources/country.mp3",
            },
        },
    },
};
const emcc_examples_preloads_map = EmccExamplesPreloadMap.initComptime(emcc_examples_preloads);

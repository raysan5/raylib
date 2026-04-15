const std = @import("std");
const builtin = @import("builtin");

const emccOutputDir = "zig-out" ++ std.fs.path.sep_str ++ "htmlout" ++ std.fs.path.sep_str;
const emccOutputFile = "index.html";

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
    try raylib_flags_arr.append("-std=c99");

    if (options.linkage == .dynamic) {
        try raylib_flags_arr.append("-fPIC");
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
            raylib_mod.addIncludePath(b.path("src/external/glfw/include"));
            try c_source_files.append("src/rglfw.c");

            switch (target.result.os.tag) {
                .windows => {
                    if (opengl_version == .auto) {
                        opengl_version = OpenglVersion.gl_3_3;
                    }
                    raylib_mod.addCMacro("PLATFORM_DESKTOP_GLFW", "");

                    raylib_mod.linkSystemLibrary("winmm", .{});
                    raylib_mod.linkSystemLibrary("gdi32", .{});
                    raylib_mod.linkSystemLibrary("opengl32", .{});
                },
                .linux => {
                    if (opengl_version == .auto) {
                        opengl_version = OpenglVersion.gl_3_3;
                    }
                    raylib_mod.addCMacro("PLATFORM_DESKTOP_GLFW", "");

                    if (options.linux_display_backend == .X11 or options.linux_display_backend == .Both) {
                        raylib_mod.addCMacro("_GLFW_X11", "");
                        raylib_mod.linkSystemLibrary("GLX", .{});
                        raylib_mod.linkSystemLibrary("X11", .{});
                        raylib_mod.linkSystemLibrary("Xcursor", .{});
                        raylib_mod.linkSystemLibrary("Xext", .{});
                        raylib_mod.linkSystemLibrary("Xfixes", .{});
                        raylib_mod.linkSystemLibrary("Xi", .{});
                        raylib_mod.linkSystemLibrary("Xinerama", .{});
                        raylib_mod.linkSystemLibrary("Xrandr", .{});
                        raylib_mod.linkSystemLibrary("Xrender", .{});
                    }

                    if (options.linux_display_backend == .Wayland or options.linux_display_backend == .Both) {
                        _ = b.findProgram(&.{"wayland-scanner"}, &.{}) catch {
                            std.log.err(
                                \\ `wayland-scanner` may not be installed on the system.
                                \\ You can switch to X11 in your `build.zig` by changing `Options.linux_display_backend`
                            , .{});
                            @panic("`wayland-scanner` not found");
                        };
                        raylib_mod.addCMacro("_GLFW_WAYLAND", "");
                        raylib_mod.linkSystemLibrary("wayland-client", .{});
                        raylib_mod.linkSystemLibrary("wayland-cursor", .{});
                        raylib_mod.linkSystemLibrary("wayland-egl", .{});
                        raylib_mod.linkSystemLibrary("xkbcommon", .{});
                        try waylandGenerate(b, raylib, "src/external/glfw/deps/wayland/", false);
                    }
                },
                .freebsd, .openbsd, .netbsd, .dragonfly => {
                    if (opengl_version == .auto) {
                        opengl_version = OpenglVersion.gl_3_3;
                    }
                    raylib_mod.addCMacro("PLATFORM_DESKTOP_GLFW", "");

                    raylib_mod.linkSystemLibrary("GL", .{});
                    raylib_mod.linkSystemLibrary("rt", .{});
                    raylib_mod.linkSystemLibrary("dl", .{});
                    raylib_mod.linkSystemLibrary("m", .{});
                    raylib_mod.linkSystemLibrary("X11", .{});
                    raylib_mod.linkSystemLibrary("Xrandr", .{});
                    raylib_mod.linkSystemLibrary("Xinerama", .{});
                    raylib_mod.linkSystemLibrary("Xi", .{});
                    raylib_mod.linkSystemLibrary("Xxf86vm", .{});
                    raylib_mod.linkSystemLibrary("Xcursor", .{});
                },
                .macos => {
                    if (opengl_version == .auto) {
                        opengl_version = OpenglVersion.gl_3_3;
                    }

                    // On macos rglfw.c include Objective-C files.
                    try raylib_flags_arr.append("-ObjC");
                    raylib_mod.addCSourceFile(.{
                        .file = b.path("src/rglfw.c"),
                        .flags = raylib_flags_arr.items,
                    });
                    _ = raylib_flags_arr.pop();

                    // Include xcode_frameworks for cross compilation
                    if (b.lazyDependency("xcode_frameworks", .{})) |dep| {
                        raylib_mod.addSystemFrameworkPath(dep.path("Frameworks"));
                        raylib_mod.addSystemIncludePath(dep.path("include"));
                        raylib_mod.addLibraryPath(dep.path("lib"));
                    }

                    raylib_mod.linkFramework("Foundation", .{});
                    raylib_mod.linkFramework("CoreServices", .{});
                    raylib_mod.linkFramework("CoreGraphics", .{});
                    raylib_mod.linkFramework("AppKit", .{});
                    raylib_mod.linkFramework("IOKit", .{});
                },
                .emscripten => {
                    if (opengl_version == .auto) {
                        opengl_version = OpenglVersion.gles_2;
                    }
                    raylib_mod.addCMacro("PLATFORM_WEB", "");

                    const activate_emsdk_step = emsdk.zemscripten.activateEmsdkStep(b);
                    raylib.step.dependOn(activate_emsdk_step);
                    if (options.opengl_version == .auto) {
                        raylib_mod.addCMacro("GRAPHICS_API_OPENGL_ES3", "");
                    }
                },
                else => {
                    if (options.opengl_version == .auto) {
                        raylib_mod.addCMacro(OpenglVersion.gl_3_3.toCMacroStr(), "");
                    }
                    try c_source_files.append("src/rglfw.c");
                },
            }
            raylib_mod.addCMacro(opengl_version.toCMacroStr(), "");
        },
        .rgfw => {
            var opengl_version: OpenglVersion = options.opengl_version;

            switch (target.result.os.tag) {
                .linux => {
                    if (opengl_version == .auto) {
                        opengl_version = OpenglVersion.gl_3_3;
                    }
                    raylib_mod.addCMacro("PLATFORM_DESKTOP_RGFW", "");

                    if (options.linux_display_backend == .X11 or options.linux_display_backend == .Both) {
                        raylib_mod.addCMacro("RGFW_X11", "");
                        raylib_mod.addCMacro("RGFW_UNIX", "");

                        raylib_mod.linkSystemLibrary("GL", .{});
                        raylib_mod.linkSystemLibrary("X11", .{});
                        raylib_mod.linkSystemLibrary("Xrandr", .{});
                        raylib_mod.linkSystemLibrary("Xi", .{});
                        raylib_mod.linkSystemLibrary("Xcursor", .{});
                        raylib_mod.linkSystemLibrary("pthread", .{});
                        raylib_mod.linkSystemLibrary("dl", .{});
                        raylib_mod.linkSystemLibrary("rt", .{});
                    }

                    if (options.linux_display_backend == .Wayland or options.linux_display_backend == .Both) {
                        if (options.linux_display_backend != .Both) {
                            raylib_mod.addCMacro("RGFW_NO_X11", "");
                        }

                        _ = b.findProgram(&.{"wayland-scanner"}, &.{}) catch {
                            std.log.err(
                                \\ `wayland-scanner` may not be installed on the system.
                                \\ You can switch to X11 in your `build.zig` by changing `Options.linux_display_backend`
                            , .{});
                            @panic("`wayland-scanner` not found");
                        };
                        raylib_mod.addCMacro("RGFW_WAYLAND", "");
                        raylib_mod.addCMacro("EGLAPIENTRY", "");
                        raylib_mod.linkSystemLibrary("wayland-client", .{});
                        raylib_mod.linkSystemLibrary("wayland-cursor", .{});
                        raylib_mod.linkSystemLibrary("wayland-egl", .{});
                        raylib_mod.linkSystemLibrary("xkbcommon", .{});
                        try waylandGenerate(b, raylib, "src/external/RGFW/deps/wayland/", true);
                    }
                },
                .emscripten => raylib_mod.addCMacro("PLATFORM_WEB_RGFW", ""),
                else => raylib_mod.addCMacro("PLATFORM_DESKTOP_RGFW", ""),
            }

            raylib_mod.addCMacro(opengl_version.toCMacroStr(), "");
        },
        .sdl => {
            if (options.opengl_version == .auto) {
                raylib_mod.addCMacro(OpenglVersion.gl_3_3.toCMacroStr(), "");
            }
            raylib_mod.addCMacro("PLATFORM_DESKTOP_SDL", "");
        },
        .memory => {
            if (options.opengl_version == .auto) {
                raylib_mod.addCMacro(OpenglVersion.gl_soft.toCMacroStr(), "");
            }
            raylib_mod.addCMacro("PLATFORM_MEMORY", "");
        },
        .win32 => {
            if (options.opengl_version == .auto) {
                raylib_mod.addCMacro(OpenglVersion.gl_3_3.toCMacroStr(), "");
            }
            raylib_mod.addCMacro("PLATFORM_DESKTOP_WIN32", "");
        },
        .drm => {
            switch (options.opengl_version) {
                .auto => {
                    raylib_mod.addCMacro(OpenglVersion.gles_2.toCMacroStr(), "");
                    raylib_mod.linkSystemLibrary("GLESv2", .{});
                    raylib_mod.linkSystemLibrary("EGL", .{});
                    raylib_mod.linkSystemLibrary("gbm", .{});
                },
                .gl_soft => {},
                else => {
                    raylib_mod.linkSystemLibrary("EGL", .{});
                    raylib_mod.linkSystemLibrary("gbm", .{});
                },
            }

            raylib_mod.addCMacro("PLATFORM_DRM", "");
            raylib_mod.linkSystemLibrary("libdrm", .{ .use_pkg_config = .force });
            raylib_mod.addCMacro("EGL_NO_X11", "");
            raylib_mod.addCMacro("DEFAULT_BATCH_BUFFER_ELEMENT", "");
        },
        .android => {
            if (options.opengl_version == .auto) {
                raylib_mod.addCMacro(OpenglVersion.gles_2.toCMacroStr(), "");
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

            var libcData: std.ArrayList(u8) = .empty;
            var aw: std.Io.Writer.Allocating = .fromArrayList(b.allocator, &libcData);
            try (std.zig.LibCInstallation{
                .include_dir = androidIncludePath,
                .sys_include_dir = androidIncludePath,
                .crt_dir = androidApiSpecificPath,
            }).render(&aw.writer);
            const libcFile = b.addWriteFiles().add("android-libc.txt", try libcData.toOwnedSlice(arena.allocator()));
            raylib.setLibCFile(libcFile);

            if (options.opengl_version == .auto) {
                raylib_mod.linkSystemLibrary("GLESv2", .{});
                raylib_mod.addCMacro("GRAPHICS_API_OPENGL_ES2", "");
            }
            raylib_mod.linkSystemLibrary("EGL", .{});

            raylib_mod.addCMacro("PLATFORM_ANDROID", "");
        },
        //.web_emscripten => raylib_mod.addCMacro("PLATFORM_WEB_EMSCRIPTEN", ""),
    }

    raylib_mod.addCSourceFiles(.{
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
    memory,
    win32,
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

    //const examples = b.step("examples", "Build/Install all examples");
    //examples.dependOn(try addExamples("audio", b, target, optimize, lib));
    //examples.dependOn(try addExamples("core", b, target, optimize, lib));
    //examples.dependOn(try addExamples("models", b, target, optimize, lib));
    //examples.dependOn(try addExamples("others", b, target, optimize, lib));
    //examples.dependOn(try addExamples("shaders", b, target, optimize, lib));
    //examples.dependOn(try addExamples("shapes", b, target, optimize, lib));
    //examples.dependOn(try addExamples("text", b, target, optimize, lib));
    //examples.dependOn(try addExamples("textures", b, target, optimize, lib));
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

    var dir = try b.build_root.handle.openDir(b.graph.io, module_subpath, .{ .iterate = true });
    defer dir.close(b.graph.io);

    var iter = dir.iterate();
    while (try iter.next(b.graph.io)) |entry| {
        if (entry.kind != .file) continue;
        const extension_idx = std.mem.lastIndexOf(u8, entry.name, ".c") orelse continue;
        const name = entry.name[0..extension_idx];
        const filename = try std.fmt.allocPrint(b.allocator, "{s}.c", .{name});
        const path = b.pathJoin(&.{ module_subpath, filename });

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
                exe_mod.addIncludePath(b.path("src"));
                exe_mod.addIncludePath(b.path("src/external/glfw/include"));
            }
            if (std.mem.eql(u8, name, "raylib_opengl_interop")) {
                exe_mod.addIncludePath(b.path("src/external"));
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

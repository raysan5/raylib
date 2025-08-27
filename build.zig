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

fn setDesktopPlatform(raylib: *std.Build.Step.Compile, platform: PlatformBackend) void {
    switch (platform) {
        .glfw => raylib.root_module.addCMacro("PLATFORM_DESKTOP_GLFW", ""),
        .rgfw => raylib.root_module.addCMacro("PLATFORM_DESKTOP_RGFW", ""),
        .sdl => raylib.root_module.addCMacro("PLATFORM_DESKTOP_SDL", ""),
        .android => raylib.root_module.addCMacro("PLATFORM_ANDROID", ""),
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

// Adapted from Not-Nik/raylib-zig
fn emscriptenRunStep(b: *std.Build, emsdk: *std.Build.Dependency, examplePath: []const u8) !*std.Build.Step.Run {
    const dot_emsc_path = emsdk.path("upstream/emscripten/cache/sysroot/include").getPath(b);
    // If compiling on windows , use emrun.bat.
    const emrunExe = switch (builtin.os.tag) {
        .windows => "emrun.bat",
        else => "emrun",
    };
    var emrun_run_arg = try b.allocator.alloc(u8, dot_emsc_path.len + emrunExe.len + 1);
    defer b.allocator.free(emrun_run_arg);

    if (b.sysroot == null) {
        emrun_run_arg = try std.fmt.bufPrint(emrun_run_arg, "{s}" ++ std.fs.path.sep_str ++ "{s}", .{ emsdk.path("upstream/emscripten").getPath(b), emrunExe });
    } else {
        emrun_run_arg = try std.fmt.bufPrint(emrun_run_arg, "{s}" ++ std.fs.path.sep_str ++ "{s}", .{ dot_emsc_path, emrunExe });
    }
    const run_cmd = b.addSystemCommand(&.{ emrun_run_arg, examplePath });
    return run_cmd;
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

pub fn compileRaylib(b: *std.Build, target: std.Build.ResolvedTarget, optimize: std.builtin.OptimizeMode, options: Options) !*std.Build.Step.Compile {
    var raylib_flags_arr: std.ArrayList([]const u8) = .empty;
    defer raylib_flags_arr.deinit(b.allocator);

    try raylib_flags_arr.appendSlice(
        b.allocator,
        &[_][]const u8{
            "-std=gnu99",
            "-D_GNU_SOURCE",
            "-DGL_SILENCE_DEPRECATION=199309L",
            "-fno-sanitize=undefined", // https://github.com/raysan5/raylib/issues/3674
        },
    );

    if (options.shared) {
        try raylib_flags_arr.appendSlice(
            b.allocator,
            &[_][]const u8{
                "-fPIC",
                "-DBUILD_LIBTYPE_SHARED",
            },
        );
    }

    // Sets a flag indiciating the use of a custom `config.h`
    try raylib_flags_arr.append(b.allocator, "-DEXTERNAL_CONFIG_FLAGS");
    if (options.config.len > 0) {
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
        // Set default config if no custome config got set
        try raylib_flags_arr.appendSlice(b.allocator, &config_h_flags);
    }

    const raylib = b.addLibrary(.{
        .name = "raylib",
        .linkage = if (options.shared) .dynamic else .static,
        .root_module = b.createModule(.{
            .target = target,
            .optimize = optimize,
        }),
    });
    raylib.linkLibC();

    // No GLFW required on PLATFORM_DRM
    if (options.platform != .drm) {
        raylib.addIncludePath(b.path("src/external/glfw/include"));
    }

    var c_source_files: std.ArrayList([]const u8) = try .initCapacity(b.allocator, 2);
    c_source_files.appendSliceAssumeCapacity(&.{ "src/rcore.c", "src/utils.c" });

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

    raylib.addIncludePath(b.path("src/platforms"));
    switch (target.result.os.tag) {
        .windows => {
            switch (options.platform) {
                .glfw => try c_source_files.append(b.allocator, "src/rglfw.c"),
                .rgfw, .sdl, .drm, .android => {},
            }

            raylib.linkSystemLibrary("winmm");
            raylib.linkSystemLibrary("gdi32");
            raylib.linkSystemLibrary("opengl32");

            setDesktopPlatform(raylib, options.platform);
        },
        .linux => {
            if (options.platform == .drm) {
                if (options.opengl_version == .auto) {
                    raylib.linkSystemLibrary("GLESv2");
                    raylib.root_module.addCMacro("GRAPHICS_API_OPENGL_ES2", "");
                }

                raylib.linkSystemLibrary("EGL");
                raylib.linkSystemLibrary("gbm");
                raylib.linkSystemLibrary2("libdrm", .{ .use_pkg_config = .force });

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

                raylib.addLibraryPath(.{ .cwd_relative = androidLibPath });
                raylib.root_module.addLibraryPath(.{ .cwd_relative = androidApiSpecificPath });
                raylib.addSystemIncludePath(.{ .cwd_relative = androidIncludePath });
                raylib.addSystemIncludePath(.{ .cwd_relative = androidArchIncludePath });
                raylib.addSystemIncludePath(.{ .cwd_relative = androidAsmPath });
                raylib.addSystemIncludePath(.{ .cwd_relative = androidGluePath });

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
                    raylib.root_module.addCMacro("_GLFW_WAYLAND", "");
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
            }
        },
        .freebsd, .openbsd, .netbsd, .dragonfly => {
            try c_source_files.append(b.allocator, "rglfw.c");
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
            if (b.lazyDependency("emsdk", .{})) |dep| {
                if (try emSdkSetupStep(b, dep)) |emSdkStep| {
                    raylib.step.dependOn(&emSdkStep.step);
                }

                raylib.addIncludePath(dep.path("upstream/emscripten/cache/sysroot/include"));
            }

            raylib.root_module.addCMacro("PLATFORM_WEB", "");
            if (options.opengl_version == .auto) {
                raylib.root_module.addCMacro("GRAPHICS_API_OPENGL_ES2", "");
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
    raylib.addCSourceFile(.{ .file = raygui_c_path });
    raylib.addIncludePath(raygui_dep.path("src"));
    raylib.addIncludePath(raylib_dep.path("src"));

    raylib.installHeader(raygui_dep.path("src/raygui.h"), "raygui.h");
}

pub const Options = struct {
    raudio: bool = true,
    rmodels: bool = true,
    rshapes: bool = true,
    rtext: bool = true,
    rtextures: bool = true,
    platform: PlatformBackend = .glfw,
    shared: bool = false,
    linux_display_backend: LinuxDisplayBackend = .Both,
    opengl_version: OpenglVersion = .auto,
    android_ndk: []const u8 = "",
    android_api_version: []const u8 = "35",
    /// config should be a list of space-separated cflags, eg, "-DSUPPORT_CUSTOM_FRAME_CONTROL"
    config: []const u8 = &.{},

    const defaults = Options{};

    pub fn getOptions(b: *std.Build) Options {
        return .{
            .platform = b.option(PlatformBackend, "platform", "Choose the platform backedn for desktop target") orelse defaults.platform,
            .raudio = b.option(bool, "raudio", "Compile with audio support") orelse defaults.raudio,
            .rmodels = b.option(bool, "rmodels", "Compile with models support") orelse defaults.rmodels,
            .rtext = b.option(bool, "rtext", "Compile with text support") orelse defaults.rtext,
            .rtextures = b.option(bool, "rtextures", "Compile with textures support") orelse defaults.rtextures,
            .rshapes = b.option(bool, "rshapes", "Compile with shapes support") orelse defaults.rshapes,
            .shared = b.option(bool, "shared", "Compile as shared library") orelse defaults.shared,
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

fn addExamples(
    comptime module: []const u8,
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    raylib: *std.Build.Step.Compile,
) !*std.Build.Step {
    const all = b.step(module, "All " ++ module ++ " examples");
    const module_subpath = b.pathJoin(&.{ "examples", module });
    const module_resources = b.pathJoin(&.{ module_subpath, "resources@resources" });
    var dir = try std.fs.cwd().openDir(b.pathFromRoot(module_subpath), .{ .iterate = true });
    defer if (comptime builtin.zig_version.minor >= 12) dir.close();

    var iter = dir.iterate();
    while (try iter.next()) |entry| {
        if (entry.kind != .file) continue;
        const extension_idx = std.mem.lastIndexOf(u8, entry.name, ".c") orelse continue;
        const name = entry.name[0..extension_idx];
        const path = b.pathJoin(&.{ module_subpath, entry.name });

        // zig's mingw headers do not include pthread.h
        if (std.mem.eql(u8, "core_loading_thread", name) and target.result.os.tag == .windows) continue;

        if (target.result.os.tag == .emscripten) {
            const exe_lib = b.addLibrary(.{
                .name = name,
                .linkage = .static,
                .root_module = b.createModule(.{
                    .target = target,
                    .optimize = optimize,
                }),
            });
            exe_lib.addCSourceFile(.{
                .file = b.path(path),
                .flags = &.{},
            });
            exe_lib.linkLibC();

            if (std.mem.eql(u8, name, "rlgl_standalone")) {
                //TODO: Make rlgl_standalone example work
                continue;
            }
            if (std.mem.eql(u8, name, "raylib_opengl_interop")) {
                //TODO: Make raylib_opengl_interop example work
                continue;
            }

            exe_lib.linkLibrary(raylib);

            // Include emscripten for cross compilation
            if (b.lazyDependency("emsdk", .{})) |emsdk_dep| {
                if (try emSdkSetupStep(b, emsdk_dep)) |emSdkStep| {
                    exe_lib.step.dependOn(&emSdkStep.step);
                }

                exe_lib.addIncludePath(emsdk_dep.path("upstream/emscripten/cache/sysroot/include"));

                // Create the output directory because emcc can't do it.
                const emccOutputDirExample = b.pathJoin(&.{ emccOutputDir, name, std.fs.path.sep_str });
                const mkdir_command = switch (builtin.os.tag) {
                    .windows => b.addSystemCommand(&.{ "cmd.exe", "/c", "if", "not", "exist", emccOutputDirExample, "mkdir", emccOutputDirExample }),
                    else => b.addSystemCommand(&.{ "mkdir", "-p", emccOutputDirExample }),
                };

                const emcc_exe = switch (builtin.os.tag) {
                    .windows => "emcc.bat",
                    else => "emcc",
                };

                const emcc_exe_path = b.pathJoin(&.{ emsdk_dep.path("upstream/emscripten").getPath(b), emcc_exe });
                const emcc_command = b.addSystemCommand(&[_][]const u8{emcc_exe_path});
                emcc_command.step.dependOn(&mkdir_command.step);
                const emccOutputDirExampleWithFile = b.pathJoin(&.{ emccOutputDir, name, std.fs.path.sep_str, emccOutputFile });
                emcc_command.addArgs(&[_][]const u8{
                    "-o",
                    emccOutputDirExampleWithFile,
                    "-sFULL-ES3=1",
                    "-sUSE_GLFW=3",
                    "-sSTACK_OVERFLOW_CHECK=1",
                    "-sEXPORTED_RUNTIME_METHODS=['requestFullscreen']",
                    "-sASYNCIFY",
                    "-O0",
                    "--emrun",
                    "--preload-file",
                    module_resources,
                    "--shell-file",
                    b.path("src/shell.html").getPath(b),
                });

                const link_items: []const *std.Build.Step.Compile = &.{
                    raylib,
                    exe_lib,
                };
                for (link_items) |item| {
                    emcc_command.addFileArg(item.getEmittedBin());
                    emcc_command.step.dependOn(&item.step);
                }

                const run_step = try emscriptenRunStep(b, emsdk_dep, emccOutputDirExampleWithFile);
                run_step.step.dependOn(&emcc_command.step);
                run_step.addArg("--no_browser");
                const run_option = b.step(name, name);

                run_option.dependOn(&run_step.step);

                all.dependOn(&emcc_command.step);
            }
        } else {
            const exe = b.addExecutable(.{
                .name = name,
                .root_module = b.createModule(.{
                    .target = target,
                    .optimize = optimize,
                }),
            });
            exe.addCSourceFile(.{ .file = b.path(path), .flags = &.{} });
            exe.linkLibC();

            // special examples that test using these external dependencies directly
            // alongside raylib
            if (std.mem.eql(u8, name, "rlgl_standalone")) {
                exe.addIncludePath(b.path("src"));
                exe.addIncludePath(b.path("src/external/glfw/include"));
                if (!hasCSource(raylib.root_module, "rglfw.c")) {
                    exe.addCSourceFile(.{ .file = b.path("src/rglfw.c"), .flags = &.{} });
                }
            }
            if (std.mem.eql(u8, name, "raylib_opengl_interop")) {
                exe.addIncludePath(b.path("src/external"));
            }

            exe.linkLibrary(raylib);

            switch (target.result.os.tag) {
                .windows => {
                    exe.linkSystemLibrary("winmm");
                    exe.linkSystemLibrary("gdi32");
                    exe.linkSystemLibrary("opengl32");

                    exe.root_module.addCMacro("PLATFORM_DESKTOP", "");
                },
                .linux => {
                    exe.linkSystemLibrary("GL");
                    exe.linkSystemLibrary("rt");
                    exe.linkSystemLibrary("dl");
                    exe.linkSystemLibrary("m");
                    exe.linkSystemLibrary("X11");

                    exe.root_module.addCMacro("PLATFORM_DESKTOP", "");
                },
                .macos => {
                    exe.linkFramework("Foundation");
                    exe.linkFramework("Cocoa");
                    exe.linkFramework("OpenGL");
                    exe.linkFramework("CoreAudio");
                    exe.linkFramework("CoreVideo");
                    exe.linkFramework("IOKit");

                    exe.root_module.addCMacro("PLATFORM_DESKTOP", "");
                },
                else => {
                    @panic("Unsupported OS");
                },
            }

            const install_cmd = b.addInstallArtifact(exe, .{});

            const run_cmd = b.addRunArtifact(exe);
            run_cmd.cwd = b.path(module_subpath);
            run_cmd.step.dependOn(&install_cmd.step);

            const run_step = b.step(name, name);
            run_step.dependOn(&run_cmd.step);

            all.dependOn(&install_cmd.step);
        }
    }
    return all;
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

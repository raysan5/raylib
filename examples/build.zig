const std = @import("std");

const rl = @import("raylib");

const Options = rl.Options;
const PlatformBackend = rl.PlatformBackend;
const emsdk = rl.emsdk;

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const options: Options = .getOptions(b);

    const raylib_dep = b.dependency("raylib", .{
        .target = target,
        .optimize = optimize,
        .platform = options.platform,
        .raudio = options.raudio,
        .rmodels = options.rmodels,
        .rtext = options.rtext,
        .rtextures = options.rtextures,
        .rshapes = options.rshapes,
        .raygui = options.raygui,
        .linkage = options.linkage,
        .linux_display_backend = options.linux_display_backend,
        .opengl_version = options.opengl_version,
        .config = options.config,
        .android_ndk = options.android_ndk,
        .android_api_version = options.android_api_version,
    });

    const lib = raylib_dep.artifact("raylib");

    const raylib_b = raylib_dep.builder;

    b.default_step.dependOn(try addExamples("core", b, target, optimize, lib, raylib_b, options.platform));
    b.default_step.dependOn(try addExamples("audio", b, target, optimize, lib, raylib_b, options.platform));
    b.default_step.dependOn(try addExamples("models", b, target, optimize, lib, raylib_b, options.platform));
    b.default_step.dependOn(try addExamples("shaders", b, target, optimize, lib, raylib_b, options.platform));
    b.default_step.dependOn(try addExamples("shapes", b, target, optimize, lib, raylib_b, options.platform));
    b.default_step.dependOn(try addExamples("text", b, target, optimize, lib, raylib_b, options.platform));
    b.default_step.dependOn(try addExamples("textures", b, target, optimize, lib, raylib_b, options.platform));
    b.default_step.dependOn(try addExamples("others", b, target, optimize, lib, raylib_b, options.platform));
}

fn addExamples(
    comptime module: []const u8,
    b: *std.Build,
    target: std.Build.ResolvedTarget,
    optimize: std.builtin.OptimizeMode,
    raylib: *std.Build.Step.Compile,
    raylib_b: *std.Build,
    platform: PlatformBackend,
) !*std.Build.Step {
    const all = b.step(module, "All " ++ module ++ " examples");
    const module_subpath = module;

    var dir = try b.root.openDir(b.graph.io, module_subpath, .{ .iterate = true });
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
            exe_mod.addIncludePath(raylib_b.path("src"));
            exe_mod.addIncludePath(raylib_b.path("src/external/glfw/include"));
        }
        if (std.mem.eql(u8, filename, "raylib_opengl_interop")) {
            if (platform == .drm) continue;
            if (target.result.os.tag == .macos) continue;
            exe_mod.addIncludePath(raylib_b.path("src/external"));
        }

        const run_step = b.step(filename, filename);

        // web exports are completely separate
        if (target.query.os_tag == .emscripten) {
            exe_mod.addCMacro("PLATFORM_WEB", "");

            const wasm = b.addLibrary(.{
                .name = b.fmt("{s}.html", .{filename}),
                .root_module = exe_mod,
            });

            const install_dir: std.Build.InstallDir = .{ .custom = b.fmt("web/{s}/{s}", .{ module, filename }) };
            const emcc_flags = emsdk.emccDefaultFlags(b.allocator, .{ .optimize = optimize });
            const emcc_settings = emsdk.emccDefaultSettings(b.allocator, .{ .optimize = optimize });

            const SerialResourceFile = struct { src_path: []const u8, virtual_path: []const u8 };
            const EmccExamplesPreloadMap = std.static_string_map.StaticStringMap([]const SerialResourceFile);
            const EmccExamplesPreloadSerial = struct { []const u8, []const SerialResourceFile };
            const emcc_examples_preloads_serial: []const EmccExamplesPreloadSerial = @import("example_resources.zon");
            const emcc_examples_preloads_map = EmccExamplesPreloadMap.initComptime(emcc_examples_preloads_serial);
            const preload_paths: ?[]emsdk.ResourceFile = blk: {
                if (emcc_examples_preloads_map.get(filename)) |resource_files| {
                    var rfs = try b.allocator.alloc(emsdk.ResourceFile, resource_files.len);
                    for (resource_files, 0..) |resource_file, rfidx| {
                        rfs[rfidx] = .{
                            .src_path = b.path(resource_file.src_path),
                            .virtual_path = resource_file.virtual_path,
                        };
                    }
                    break :blk rfs;
                } else break :blk null;
            };

            const emcc_step = try emsdk.emccStep(b, &.{}, &.{ raylib, wasm }, .{
                .optimize = optimize,
                .flags = emcc_flags,
                .settings = emcc_settings,
                .preload_paths = preload_paths,
                .shell_file_path = raylib_b.path("src/shell.html"),
                .install_dir = install_dir,
                .out_file_name = wasm.name,
            });

            const emrun_step = try emsdk.emrunStep(
                b,
                b.graph.path(.install_prefix, b.fmt(
                    "web/{s}/{s}/{s}",
                    .{ module, filename, wasm.name },
                )),
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

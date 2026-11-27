const std = @import("std");
const rl = @import("raylib");

pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});
    const platform = b.option(rl.PlatformBackend, "platform", "select the platform") orelse rl.PlatformBackend.glfw;
    const zig = b.option(bool, "zig", "compile zig code") orelse false;

    const raylib_dep = b.dependency("raylib", .{
        .target = target,
        .optimize = optimize,
        .platform = platform,
    });
    const raylib_artifact = raylib_dep.artifact("raylib");

    if (platform == .sdl3) {
        if (b.lazyDependency("sdl3", .{ .optimize = optimize, .target = target })) |dep| {
            raylib_artifact.root_module.linkLibrary(dep.artifact("SDL3"));
        }
    }

    var exe_mod: *std.Build.Module = undefined;

    if (zig) {
        exe_mod = b.createModule(.{
            .root_source_file = b.path("src/core_basic_window.zig"),
            .target = target,
            .optimize = optimize,
        });
        exe_mod.addImport("raylib", raylib_dep.module("raylib"));
    } else {
        exe_mod = b.createModule(.{
            .target = target,
            .optimize = optimize,
            .link_libc = true,
        });
        exe_mod.addCSourceFile(.{ .file = b.path("src/core_basic_window.c") });
        exe_mod.linkLibrary(raylib_artifact);
    }

    const run_step = b.step("run", "Run the app");

    // web exports are completely separate
    if (target.query.os_tag == .emscripten) {
        const emsdk = rl.emsdk;
        const wasm = b.addLibrary(.{
            .name = "core_basic_window_web",
            .root_module = exe_mod,
        });

        const install_dir: std.Build.InstallDir = .{ .custom = "web" };
        const emcc_flags = emsdk.emccDefaultFlags(b.allocator, .{ .optimize = optimize });
        const emcc_settings = emsdk.emccDefaultSettings(b.allocator, .{ .optimize = optimize });

        const emcc_step = emsdk.emccStep(b, raylib_artifact, wasm, .{
            .optimize = optimize,
            .flags = emcc_flags,
            .settings = emcc_settings,
            .shell_file_path = emsdk.shell(raylib_dep),
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
    } else {
        const exe = b.addExecutable(.{
            .name = "core_basic_window",
            .root_module = exe_mod,
            .use_lld = target.result.os.tag == .windows,
        });
        b.installArtifact(exe);

        const run_cmd = b.addRunArtifact(exe);
        run_cmd.step.dependOn(b.getInstallStep());

        run_step.dependOn(&run_cmd.step);
    }
}

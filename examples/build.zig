const std = @import("std");
const builtin = @import("builtin");

// This has been tested to work with zig 0.11.0 (67709b6, Aug 4 2023)
fn add_module(comptime module: []const u8, b: *std.Build, target: std.zig.CrossTarget, optimize: std.builtin.OptimizeMode) !*std.Build.Step {
    if (target.getOsTag() == .emscripten) {
        @panic("Emscripten building via Zig unsupported");
    }

    const all = b.step(module, "All " ++ module ++ " examples");
    const dir = try std.fs.cwd().openIterableDir(module, .{});
    var iter = dir.iterate();
    while (try iter.next()) |entry| {
        if (entry.kind != .file) continue;
        const extension_idx = std.mem.lastIndexOf(u8, entry.name, ".c") orelse continue;
        const name = entry.name[0..extension_idx];
        const path = try std.fs.path.join(b.allocator, &.{ module, entry.name });

        // zig's mingw headers do not include pthread.h
        if (std.mem.eql(u8, "core_loading_thread", name) and target.getOsTag() == .windows) continue;

        const exe = b.addExecutable(.{
            .name = name,
            .target = target,
            .optimize = optimize,
        });
        exe.addCSourceFile(.{ .file = .{ .path = path }, .flags = &.{} });
        exe.linkLibC();
        exe.addObjectFile(switch (target.getOsTag()) {
            .windows => .{ .path = "../zig-out/lib/raylib.lib" },
            .linux => .{ .path = "../zig-out/lib/libraylib.a" },
            .macos => .{ .path = "../zig-out/lib/libraylib.a" },
            .emscripten => .{ .path = "../zig-out/lib/libraylib.a" },
            else => @panic("Unsupported OS"),
        });

        exe.addIncludePath(.{ .path = "../src" });
        exe.addIncludePath(.{ .path = "../src/external" });
        exe.addIncludePath(.{ .path = "../src/external/glfw/include" });

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

        const install_cmd = b.addInstallArtifact(exe, .{});

        const run_cmd = b.addRunArtifact(exe);
        run_cmd.step.dependOn(&install_cmd.step);

        const run_step = b.step(name, name);
        run_step.dependOn(&run_cmd.step);

        all.dependOn(&install_cmd.step);
    }
    return all;
}

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

    const all = b.getInstallStep();

    all.dependOn(try add_module("audio", b, target, optimize));
    all.dependOn(try add_module("core", b, target, optimize));
    all.dependOn(try add_module("models", b, target, optimize));
    all.dependOn(try add_module("others", b, target, optimize));
    all.dependOn(try add_module("shaders", b, target, optimize));
    all.dependOn(try add_module("shapes", b, target, optimize));
    all.dependOn(try add_module("text", b, target, optimize));
    all.dependOn(try add_module("textures", b, target, optimize));
}

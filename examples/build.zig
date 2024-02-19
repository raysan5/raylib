const std = @import("std");
const builtin = @import("builtin");

// This has been tested to work with zig 0.11.0 (67709b6, Aug 4 2023) and zig 0.12.0-dev.2075+f5978181e (Jan 8 2024)
//
// anytype is used here to preserve compatibility, in 0.12.0dev the std.zig.CrossTarget type
// was reworked into std.Target.Query and std.Build.ResolvedTarget. Using anytype allows
// us to accept both CrossTarget and ResolvedTarget and act accordingly in getOsTagVersioned.
fn add_module(comptime module: []const u8, b: *std.Build, target: anytype, optimize: std.builtin.OptimizeMode) !*std.Build.Step {
    if (comptime builtin.zig_version.minor >= 12 and @TypeOf(target) != std.Build.ResolvedTarget) {
        @compileError("Expected 'std.Build.ResolvedTarget' for argument 2 'target' in 'add_module', found '" ++ @typeName(@TypeOf(target)) ++ "'");
    } else if (comptime builtin.zig_version.minor == 11 and @TypeOf(target) != std.zig.CrossTarget) {
        @compileError("Expected 'std.zig.CrossTarget' for argument 2 'target' in 'add_module', found '" ++ @typeName(@TypeOf(target)) ++ "'");
    }

    if (getOsTagVersioned(target) == .emscripten) {
        @panic("Emscripten building via Zig unsupported");
    }

    const all = b.step(module, "All " ++ module ++ " examples");
    var dir = try openIterableDirVersioned(std.fs.cwd(), module);
    defer if (comptime builtin.zig_version.minor >= 12) dir.close();

    var iter = dir.iterate();
    while (try iter.next()) |entry| {
        if (entry.kind != .file) continue;
        const extension_idx = std.mem.lastIndexOf(u8, entry.name, ".c") orelse continue;
        const name = entry.name[0..extension_idx];
        const path = try std.fs.path.join(b.allocator, &.{ module, entry.name });

        // zig's mingw headers do not include pthread.h
        if (std.mem.eql(u8, "core_loading_thread", name) and getOsTagVersioned(target) == .windows) continue;

        const exe = b.addExecutable(.{
            .name = name,
            .target = target,
            .optimize = optimize,
        });
        exe.addCSourceFile(.{ .file = .{ .path = path }, .flags = &.{} });
        exe.linkLibC();
        exe.addObjectFile(switch (getOsTagVersioned(target)) {
            .windows => .{ .path = "../zig-out/lib/raylib.lib" },
            .linux => .{ .path = "../zig-out/lib/libraylib.a" },
            .macos => .{ .path = "../zig-out/lib/libraylib.a" },
            .emscripten => .{ .path = "../zig-out/lib/libraylib.a" },
            else => @panic("Unsupported OS"),
        });

        exe.addIncludePath(.{ .path = "../src" });
        exe.addIncludePath(.{ .path = "../src/external" });
        exe.addIncludePath(.{ .path = "../src/external/glfw/include" });

        switch (getOsTagVersioned(target)) {
            .windows => {
                exe.linkSystemLibrary("winmm");
                exe.linkSystemLibrary("gdi32");
                exe.linkSystemLibrary("opengl32");

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

fn getOsTagVersioned(target: anytype) std.Target.Os.Tag {
    if (comptime builtin.zig_version.minor >= 12) {
        return target.result.os.tag;
    } else {
        return target.getOsTag();
    }
}

fn openIterableDirVersioned(dir: std.fs.Dir, path: []const u8) !(if (builtin.zig_version.minor >= 12) std.fs.Dir else std.fs.IterableDir) {
    if (comptime builtin.zig_version.minor >= 12) {
        return dir.openDir(path, .{ .iterate = true });
    } else {
        return dir.openIterableDir(path, .{});
    }
}

const std = @import("std");
const builtin = @import("builtin");

fn add_module(comptime module: []const u8, b: *std.build.Builder, target: std.zig.CrossTarget) !*std.build.Step {
    // Standard release options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall.
    const mode = b.standardReleaseOptions();

    const all = b.step(module, "All " ++ module ++ " examples");
    const dir = try std.fs.cwd().openIterableDir(module, .{});
    var iter = dir.iterate();
    while (try iter.next()) |entry| {
        if (entry.kind != .File) continue;
        const extension_idx = std.mem.lastIndexOf(u8, entry.name, ".c") orelse continue;
        const name = entry.name[0..extension_idx];
        const path = try std.fs.path.join(b.allocator, &.{ module, entry.name });

        // zig's mingw headers do not include pthread.h
        if (std.mem.eql(u8, "core_loading_thread", name) and target.getOsTag() == .windows) continue;

        const exe = b.addExecutable(name, null);
        exe.addCSourceFile(path, switch (target.getOsTag()) {
            .windows => &[_][]const u8{},
            .linux => &[_][]const u8{},
            .macos => &[_][]const u8{"-DPLATFORM_DESKTOP"},
            else => @panic("Unsupported OS"),
        });
        exe.setTarget(target);
        exe.setBuildMode(mode);
        exe.linkLibC();
        exe.addObjectFile(switch (target.getOsTag()) {
            .windows => "../src/raylib.lib",
            .linux => "../src/libraylib.a",
            .macos => "../src/libraylib.a",
            else => @panic("Unsupported OS"),
        });

        exe.addIncludePath("../src");
        exe.addIncludePath("../src/external");
        exe.addIncludePath("../src/external/glfw/include");

        switch (exe.target.toTarget().os.tag) {
            .windows => {
                exe.linkSystemLibrary("winmm");
                exe.linkSystemLibrary("gdi32");
                exe.linkSystemLibrary("opengl32");
                exe.addIncludePath("external/glfw/deps/mingw");
            },
            .linux => {
                exe.linkSystemLibrary("GL");
                exe.linkSystemLibrary("rt");
                exe.linkSystemLibrary("dl");
                exe.linkSystemLibrary("m");
                exe.linkSystemLibrary("X11");
            },
            .macos => {
                exe.linkFramework("Foundation");
                exe.linkFramework("Cocoa");
                exe.linkFramework("OpenGL");
                exe.linkFramework("CoreAudio");
                exe.linkFramework("CoreVideo");
                exe.linkFramework("IOKit");
            },
            else => {
                @panic("Unsupported OS");
            },
        }

        exe.setOutputDir(module);

        var run = exe.run();
        run.step.dependOn(&b.addInstallArtifact(exe).step);
        run.cwd = module;
        b.step(name, name).dependOn(&run.step);
        all.dependOn(&exe.step);
    }
    return all;
}

pub fn build(b: *std.build.Builder) !void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    const all = b.getInstallStep();

    all.dependOn(try add_module("audio", b, target));
    all.dependOn(try add_module("core", b, target));
    all.dependOn(try add_module("models", b, target));
    all.dependOn(try add_module("others", b, target));
    all.dependOn(try add_module("shaders", b, target));
    all.dependOn(try add_module("shapes", b, target));
    all.dependOn(try add_module("text", b, target));
    all.dependOn(try add_module("textures", b, target));
}

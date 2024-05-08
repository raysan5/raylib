const std = @import("std");
const raylib = @import("src/build.zig");

// This has been tested to work with zig 0.12.0
pub fn build(b: *std.Build) !void {
    try raylib.build(b);
}

// expose helper functions to user's build.zig
pub const addRaylib = raylib.addRaylib;
pub const addRaygui = raylib.addRaygui;

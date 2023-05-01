const std = @import("std");
const raylib = @import("src/build.zig");

pub fn build(b: *std.Build) void {
    raylib.build(b);
}

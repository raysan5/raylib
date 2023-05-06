const std = @import("std");
const raylib = @import("src/build.zig");

pub fn build(b: *std.build.Builder) void {
    raylib.build(b);
}

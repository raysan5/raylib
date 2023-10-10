const std = @import("std");
const raylib = @import("src/build.zig");

// This has been tested to work with zig 0.11.0 (67709b6, Aug 4 2023)
pub fn build(b: *std.Build) void {
    raylib.build(b);
}

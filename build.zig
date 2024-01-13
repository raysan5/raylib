const std = @import("std");
const raylib = @import("src/build.zig");

// This has been tested to work with zig 0.11.0 and zig 0.12.0-dev.2075+f5978181e
pub fn build(b: *std.Build) !void {
    try raylib.build(b);
}

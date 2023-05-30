const std = @import("std");
const raylib = @import("src/build.zig");

// This has been tested to work with zig master branch as of commit 87de821 or May 14 2023
pub fn build(b: *std.Build) void {
    raylib.build(b);
}

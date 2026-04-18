//*******************************************************************************************
//*
//*   raylib [core] example - Basic window (adapted for HTML5 platform)
//*
//*   This example is prepared to compile for PLATFORM_WEB and PLATFORM_DESKTOP
//*   As you will notice, code structure is slightly different to the other examples...
//*   To compile it for PLATFORM_WEB just uncomment #define PLATFORM_WEB at beginning
//*
//*   This example has been created using raylib 6.0 (www.raylib.com)
//*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
//*
//*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
//*   Rewrite in Zig by HaxSam (@haxsam)
//*
//*******************************************************************************************

const rl = @import("raylib");
const std = @import("std");
const builtin = @import("builtin");

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
const screenWidth: c_int = 800;
const screenHeight: c_int = 450;

//----------------------------------------------------------------------------------
// Program main entry point
//----------------------------------------------------------------------------------
pub fn main() void {
    // Initialization
    //--------------------------------------------------------------------------------------
    rl.InitWindow(screenWidth, screenHeight, "raylib [core] example - basic window");

    if (builtin.os.tag == .emscripten) {
        std.os.emscripten.emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
    } else {
        rl.SetTargetFPS(60); // Set our game to run at 60 frames-per-second
        //--------------------------------------------------------------------------------------

        // Main game loop
        while (!rl.WindowShouldClose()) // Detect window close button or ESC key
        {
            UpdateDrawFrame();
        }
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    rl.CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
fn UpdateDrawFrame() callconv(.c) void {
    // Update
    //----------------------------------------------------------------------------------
    // TODO: Update your variables here
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    rl.BeginDrawing();

    rl.ClearBackground(rl.RAYWHITE);

    rl.DrawText("Congrats! You created your first window!", 190, 200, 20, rl.LIGHTGRAY);

    rl.EndDrawing();
    //----------------------------------------------------------------------------------
}

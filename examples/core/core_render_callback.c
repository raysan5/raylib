/*******************************************************************************************
*
*   raylib [core] example - basic render callback
*
*   Example complexity rating: [★★☆☆] 2/4
*
*   Welcome to raylib!
*
*   To test examples, just press F6 and execute 'raylib_compile_execute' script
*   Note that compiled executable is placed in the same folder as .c file
*
*   To test the examples on Web, press F6 and execute 'raylib_compile_execute_web' script
*   Web version of the program is generated in the same folder as .c file
*
*   You can find all basic examples on C:\raylib\raylib\examples folder or
*   raylib official webpage: www.raylib.com
*
*   Enjoy using raylib. :)
*
*   Example originally created with raylib 6.1, last time updated with raylib 6.1
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2013-2026 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"
#include <math.h>

// called by raylib once per frame. On web will use emcscrpten callback
bool RunFrame(void)
{
    // return false if you want to exit
    if (WindowShouldClose())
        return false;

    ClearBackground(BLACK);

    // animate the text so you can see it move while the window is resized or dragged
    float offset = cosf((float)GetTime() * 2.0f) *60.0f;
    DrawTextEx(GetFontDefault(), "Hello Render Callback, drag the window!", (Vector2) { 190, 200 + offset}, 20, 2, LIGHTGRAY);
    return true;
}

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_HIGHDPI | FLAG_VSYNC_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [core] example - basic render callback");

    SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Run a game loop using the provided callback function, which will be called every frame
    RunGameLoop(RunFrame);

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}
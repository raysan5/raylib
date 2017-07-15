/*******************************************************************************************
*
*   raylib [shapes] example - Cubic-bezier lines
*
*   This example has been created using raylib 1.7 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2017 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Global Variables Definition
//----------------------------------------------------------------------------------
int screenWidth = 800;
int screenHeight = 450;

Vector2 start = { 0, 0 };
Vector2 end = { 0, 0 };

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void);     // Update and Draw one frame

//----------------------------------------------------------------------------------
// Main Enry Point
//----------------------------------------------------------------------------------
int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - cubic-bezier lines");
    
    end = (Vector2){ screenWidth, screenHeight };
    
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) start = GetMousePosition();
    else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON)) end = GetMousePosition();
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);
        
        DrawText("USE MOUSE LEFT-RIGHT CLICK to DEFINE LINE START and END POINTS", 15, 20, 20, GRAY);

        DrawLineBezier(start, end, 2.0f, RED);
        
    EndDrawing();
    //----------------------------------------------------------------------------------
}

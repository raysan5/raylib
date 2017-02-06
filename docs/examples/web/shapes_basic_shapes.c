/*******************************************************************************************
*
*   raylib [shapes] example - Draw basic shapes 2d (rectangle, circle, line...) (adapted for HTML5 platform)
*
*   This example has been created using raylib 1.3 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2015 Ramon Santamaria (@raysan5)
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
    InitWindow(screenWidth, screenHeight, "raylib [shapes] example - basic shapes drawing");
    
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
    // TODO: Update your variables here
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawText("some basic shapes available on raylib", 20, 20, 20, DARKGRAY);

        DrawLine(18, 42, screenWidth - 18, 42, BLACK);

        DrawCircle(screenWidth/4, 120, 35, DARKBLUE);
        DrawCircleGradient(screenWidth/4, 220, 60, GREEN, SKYBLUE);
        DrawCircleLines(screenWidth/4, 340, 80, DARKBLUE);

        DrawRectangle(screenWidth/4*2 - 60, 100, 120, 60, RED);
        DrawRectangleGradient(screenWidth/4*2 - 90, 170, 180, 130, MAROON, GOLD);
        DrawRectangleLines(screenWidth/4*2 - 40, 320, 80, 60, ORANGE);

        DrawTriangle((Vector2){screenWidth/4*3, 80},
                     (Vector2){screenWidth/4*3 - 60, 150},
                     (Vector2){screenWidth/4*3 + 60, 150}, VIOLET);

        DrawTriangleLines((Vector2){screenWidth/4*3, 160},
                          (Vector2){screenWidth/4*3 - 20, 230},
                          (Vector2){screenWidth/4*3 + 20, 230}, DARKBLUE);

        DrawPoly((Vector2){screenWidth/4*3, 320}, 6, 80, 0, BROWN);

    EndDrawing();
    //----------------------------------------------------------------------------------
}
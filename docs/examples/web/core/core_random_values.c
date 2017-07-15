/*******************************************************************************************
*
*   raylib [core] example - Generate random values (adapted for HTML5 platform)
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

int framesCounter = 0;  // Variable used to count frames
int randValue;

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
    InitWindow(screenWidth, screenHeight, "raylib [core] example - generate random values");
    
    randValue = GetRandomValue(-8, 5);   // Get a random integer number between -8 and 5 (both included)
    
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
    framesCounter++;

    // Every two seconds (120 frames) a new random value is generated
    if (((framesCounter/120)%2) == 1)
    {
        randValue = GetRandomValue(-8, 5);
        framesCounter = 0;
    }
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);

        DrawText("Every 2 seconds a new random value is generated:", 130, 100, 20, MAROON);

        DrawText(FormatText("%i", randValue), 360, 180, 80, LIGHTGRAY);

    EndDrawing();
    //----------------------------------------------------------------------------------
}
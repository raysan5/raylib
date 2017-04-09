/*******************************************************************************************
*
*   raylib [core] example - Windows drop files (adapted for HTML5 platform)
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

int count = 0;
char **droppedFiles;


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
    InitWindow(screenWidth, screenHeight, "raylib [core] example - drop files");
    
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
    ClearDroppedFiles();    // Clear internal buffers
    
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
    if (IsFileDropped())
    {
        droppedFiles = GetDroppedFiles(&count);
    }
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();

        ClearBackground(RAYWHITE);

        if (count == 0) DrawText("Drop your files to this window!", 100, 40, 20, DARKGRAY);
        else
        {
            DrawText("Dropped files:", 100, 40, 20, DARKGRAY);
            
            for (int i = 0; i < count; i++)
            {
                if (i%2 == 0) DrawRectangle(0, 85 + 40*i, screenWidth, 40, Fade(LIGHTGRAY, 0.5f));
                else DrawRectangle(0, 85 + 40*i, screenWidth, 40, Fade(LIGHTGRAY, 0.3f));
                
                DrawText(droppedFiles[i], 120, 100 + 40*i, 10, GRAY);
            }
            
            DrawText("Drop new files...", 100, 110 + 40*count, 20, DARKGRAY);
        }

    EndDrawing();
    //----------------------------------------------------------------------------------
}
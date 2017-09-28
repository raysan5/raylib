/*******************************************************************************************
*
*   raylib - Android Basic Game template
*
*   <Game title>
*   <Game description>
*
*   This game has been created using raylib v1.2 (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2014 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include "android_native_app_glue.h"

//----------------------------------------------------------------------------------
// Android Main entry point
//----------------------------------------------------------------------------------
void android_main(struct android_app *app) 
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, app);
    
    // TODO: Initialize all required variables and load all required data here!

    SetTargetFPS(60);               // Not required on Android, already locked to 60 fps
    //--------------------------------------------------------------------------------------
    
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // ...
        //----------------------------------------------------------------------------------
        
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
        
            ClearBackground(RAYWHITE);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();          // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
}